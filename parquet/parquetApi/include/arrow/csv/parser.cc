// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#include "arrow/csv/parser.h"

#include <algorithm>
#include <cstdio>
#include <limits>
#include <utility>

#include "arrow/memory_pool.h"
#include "arrow/result.h"
#include "arrow/status.h"
#include "arrow/util/logging.h"

namespace arrow {
namespace csv {

static Status ParseError(const char* message) {
  return Status::Invalid("CSV parse error: ", message);
}

static Status MismatchingColumns(int32_t expected, int32_t actual) {
  char s[50];
  snprintf(s, sizeof(s), "Expected %d columns, got %d", expected, actual);
  return ParseError(s);
}

static inline bool IsControlChar(uint8_t c) { return c < ' '; }

int32_t SkipRows(const uint8_t* data, uint32_t size, int32_t num_rows,
                 const uint8_t** out_data) {
  const auto end = data + size;
  int32_t skipped_rows = 0;
  *out_data = data;

  for (; skipped_rows < num_rows; ++skipped_rows) {
    uint8_t c;
    do {
      while (ARROW_PREDICT_FALSE(data < end && !IsControlChar(*data))) {
        ++data;
      }
      if (ARROW_PREDICT_FALSE(data == end)) {
        return skipped_rows;
      }
      c = *data++;
    } while (c != '\r' && c != '\n');
    if (c == '\r' && data < end && *data == '\n') {
      ++data;
    }
    *out_data = data;
  }

  return skipped_rows;
}

template <bool Quoting, bool Escaping>
class SpecializedOptions {
 public:
  static constexpr bool quoting = Quoting;
  static constexpr bool escaping = Escaping;
};

// A helper class allocating the buffer for parsed values and writing into it
// without any further resizes, except at the end.
class BlockParser::PresizedParsedWriter {
 public:
  PresizedParsedWriter(MemoryPool* pool, uint32_t size)
      : parsed_size_(0), parsed_capacity_(size) {
    parsed_buffer_ = *AllocateResizableBuffer(parsed_capacity_, pool);
    parsed_ = parsed_buffer_->mutable_data();
  }

  void Finish(std::shared_ptr<Buffer>* out_parsed) {
    ARROW_CHECK_OK(parsed_buffer_->Resize(parsed_size_));
    *out_parsed = parsed_buffer_;
  }

  void BeginLine() { saved_parsed_size_ = parsed_size_; }

  void PushFieldChar(char c) {
    DCHECK_LT(parsed_size_, parsed_capacity_);
    parsed_[parsed_size_++] = static_cast<uint8_t>(c);
  }

  // Rollback the state that was saved in BeginLine()
  void RollbackLine() { parsed_size_ = saved_parsed_size_; }

  int64_t size() { return parsed_size_; }

 protected:
  std::shared_ptr<ResizableBuffer> parsed_buffer_;
  uint8_t* parsed_;
  int64_t parsed_size_;
  int64_t parsed_capacity_;
  // Checkpointing, for when an incomplete line is encountered at end of block
  int64_t saved_parsed_size_;
};

// A helper class handling a growable buffer for values offsets.  This class is
// used when the number of columns is not yet known and we therefore cannot
// efficiently presize the target area for a given number of rows.
class BlockParser::ResizableValuesWriter {
 public:
  explicit ResizableValuesWriter(MemoryPool* pool)
      : values_size_(0), values_capacity_(256) {
    values_buffer_ = *AllocateResizableBuffer(values_capacity_ * sizeof(*values_), pool);
    values_ = reinterpret_cast<ValueDesc*>(values_buffer_->mutable_data());
  }

  template <typename ParsedWriter>
  void Start(ParsedWriter& parsed_writer) {
    PushValue({static_cast<uint32_t>(parsed_writer.size()) & 0x7fffffffU, false});
  }

  void Finish(std::shared_ptr<Buffer>* out_values) {
    ARROW_CHECK_OK(values_buffer_->Resize(values_size_ * sizeof(*values_)));
    *out_values = values_buffer_;
  }

  void BeginLine() { saved_values_size_ = values_size_; }

  void StartField(bool quoted) { quoted_ = quoted; }

  template <typename ParsedWriter>
  void FinishField(ParsedWriter* parsed_writer) {
    PushValue({static_cast<uint32_t>(parsed_writer->size()) & 0x7fffffffU, quoted_});
  }

  // Rollback the state that was saved in BeginLine()
  void RollbackLine() { values_size_ = saved_values_size_; }

 protected:
  void PushValue(ValueDesc v) {
    if (ARROW_PREDICT_FALSE(values_size_ == values_capacity_)) {
      values_capacity_ = values_capacity_ * 2;
      ARROW_CHECK_OK(values_buffer_->Resize(values_capacity_ * sizeof(*values_)));
      values_ = reinterpret_cast<ValueDesc*>(values_buffer_->mutable_data());
    }
    values_[values_size_++] = v;
  }

  std::shared_ptr<ResizableBuffer> values_buffer_;
  ValueDesc* values_;
  int64_t values_size_;
  int64_t values_capacity_;
  bool quoted_;
  // Checkpointing, for when an incomplete line is encountered at end of block
  int64_t saved_values_size_;
};

// A helper class allocating the buffer for values offsets and writing into it
// without any further resizes, except at the end.  This class is used once the
// number of columns is known, as it eliminates resizes and generates simpler,
// faster CSV parsing code.
class BlockParser::PresizedValuesWriter {
 public:
  PresizedValuesWriter(MemoryPool* pool, int32_t num_rows, int32_t num_cols)
      : values_size_(0), values_capacity_(1 + num_rows * num_cols) {
    values_buffer_ = *AllocateResizableBuffer(values_capacity_ * sizeof(*values_), pool);
    values_ = reinterpret_cast<ValueDesc*>(values_buffer_->mutable_data());
  }

  template <typename ParsedWriter>
  void Start(ParsedWriter& parsed_writer) {
    PushValue({static_cast<uint32_t>(parsed_writer.size()) & 0x7fffffffU, false});
  }

  void Finish(std::shared_ptr<Buffer>* out_values) {
    ARROW_CHECK_OK(values_buffer_->Resize(values_size_ * sizeof(*values_)));
    *out_values = values_buffer_;
  }

  void BeginLine() { saved_values_size_ = values_size_; }

  void StartField(bool quoted) { quoted_ = quoted; }

  template <typename ParsedWriter>
  void FinishField(ParsedWriter* parsed_writer) {
    PushValue({static_cast<uint32_t>(parsed_writer->size()) & 0x7fffffffU, quoted_});
  }

  // Rollback the state that was saved in BeginLine()
  void RollbackLine() { values_size_ = saved_values_size_; }

 protected:
  void PushValue(ValueDesc v) {
    DCHECK_LT(values_size_, values_capacity_);
    values_[values_size_++] = v;
  }

  std::shared_ptr<ResizableBuffer> values_buffer_;
  ValueDesc* values_;
  int64_t values_size_;
  const int64_t values_capacity_;
  bool quoted_;
  // Checkpointing, for when an incomplete line is encountered at end of block
  int64_t saved_values_size_;
};

template <typename SpecializedOptions, typename ValuesWriter, typename ParsedWriter>
Status BlockParser::ParseLine(ValuesWriter* values_writer, ParsedWriter* parsed_writer,
                              const char* data, const char* data_end, bool is_final,
                              const char** out_data) {
  int32_t num_cols = 0;
  char c;

  DCHECK_GT(data_end, data);

  auto FinishField = [&]() { values_writer->FinishField(parsed_writer); };

  values_writer->BeginLine();
  parsed_writer->BeginLine();

  // The parsing state machine

  // Special case empty lines: do we start with a newline separator?
  c = *data;
  if (ARROW_PREDICT_FALSE(IsControlChar(c))) {
    if (c == '\r') {
      data++;
      if (data < data_end && *data == '\n') {
        data++;
      }
      goto EmptyLine;
    }
    if (c == '\n') {
      data++;
      goto EmptyLine;
    }
  }

FieldStart:
  // At the start of a field
  // Quoting is only recognized at start of field
  if (SpecializedOptions::quoting && ARROW_PREDICT_FALSE(*data == options_.quote_char)) {
    ++data;
    values_writer->StartField(true /* quoted */);
    goto InQuotedField;
  } else {
    values_writer->StartField(false /* quoted */);
    goto InField;
  }

InField:
  // Inside a non-quoted part of a field
  if (ARROW_PREDICT_FALSE(data == data_end)) {
    goto AbortLine;
  }
  c = *data++;
  if (SpecializedOptions::escaping && ARROW_PREDICT_FALSE(c == options_.escape_char)) {
    if (ARROW_PREDICT_FALSE(data == data_end)) {
      goto AbortLine;
    }
    c = *data++;
    parsed_writer->PushFieldChar(c);
    goto InField;
  }
  if (ARROW_PREDICT_FALSE(c == options_.delimiter)) {
    goto FieldEnd;
  }
  if (ARROW_PREDICT_FALSE(IsControlChar(c))) {
    if (c == '\r') {
      // In the middle of a newline separator?
      if (ARROW_PREDICT_TRUE(data < data_end) && *data == '\n') {
        data++;
      }
      goto LineEnd;
    }
    if (c == '\n') {
      goto LineEnd;
    }
  }
  parsed_writer->PushFieldChar(c);
  goto InField;

InQuotedField:
  // Inside a quoted part of a field
  if (ARROW_PREDICT_FALSE(data == data_end)) {
    goto AbortLine;
  }
  c = *data++;
  if (SpecializedOptions::escaping && ARROW_PREDICT_FALSE(c == options_.escape_char)) {
    if (ARROW_PREDICT_FALSE(data == data_end)) {
      goto AbortLine;
    }
    c = *data++;
    parsed_writer->PushFieldChar(c);
    goto InQuotedField;
  }
  if (ARROW_PREDICT_FALSE(c == options_.quote_char)) {
    if (options_.double_quote && ARROW_PREDICT_TRUE(data < data_end) &&
        ARROW_PREDICT_FALSE(*data == options_.quote_char)) {
      // Double-quoting
      ++data;
    } else {
      // End of single-quoting
      goto InField;
    }
  }
  parsed_writer->PushFieldChar(c);
  goto InQuotedField;

FieldEnd:
  // At the end of a field
  FinishField();
  ++num_cols;
  if (ARROW_PREDICT_FALSE(data == data_end)) {
    goto AbortLine;
  }
  goto FieldStart;

LineEnd:
  // At the end of line
  FinishField();
  ++num_cols;
  if (ARROW_PREDICT_FALSE(num_cols != num_cols_)) {
    if (num_cols_ == -1) {
      num_cols_ = num_cols;
    } else {
      return MismatchingColumns(num_cols_, num_cols);
    }
  }
  ++num_rows_;
  *out_data = data;
  return Status::OK();

AbortLine:
  // Not a full line except perhaps if in final block
  if (is_final) {
    FinishField();
    ++num_cols;
    if (num_cols_ == -1) {
      num_cols_ = num_cols;
    } else if (num_cols != num_cols_) {
      return MismatchingColumns(num_cols_, num_cols);
    }
    ++num_rows_;
    *out_data = data;
    return Status::OK();
  }
  // Truncated line at end of block, rewind parsed state
  values_writer->RollbackLine();
  parsed_writer->RollbackLine();
  return Status::OK();

EmptyLine:
  if (!options_.ignore_empty_lines) {
    if (num_cols_ == -1) {
      // Consider as single value
      num_cols_ = 1;
    }
    // Record as row of empty (null?) values
    while (num_cols++ < num_cols_) {
      values_writer->StartField(false /* quoted */);
      FinishField();
    }
    ++num_rows_;
  }
  *out_data = data;
  return Status::OK();
}

template <typename SpecializedOptions, typename ValuesWriter, typename ParsedWriter>
Status BlockParser::ParseChunk(ValuesWriter* values_writer, ParsedWriter* parsed_writer,
                               const char* data, const char* data_end, bool is_final,
                               int32_t rows_in_chunk, const char** out_data,
                               bool* finished_parsing) {
  int32_t num_rows_deadline = num_rows_ + rows_in_chunk;

  while (data < data_end && num_rows_ < num_rows_deadline) {
    const char* line_end = data;
    RETURN_NOT_OK(ParseLine<SpecializedOptions>(values_writer, parsed_writer, data,
                                                data_end, is_final, &line_end));
    if (line_end == data) {
      // Cannot parse any further
      *finished_parsing = true;
      break;
    }
    data = line_end;
  }
  // Append new buffers and update size
  std::shared_ptr<Buffer> values_buffer;
  values_writer->Finish(&values_buffer);
  if (values_buffer->size() > 0) {
    values_size_ += static_cast<int32_t>(values_buffer->size() / sizeof(ValueDesc) - 1);
    values_buffers_.push_back(std::move(values_buffer));
  }
  *out_data = data;
  return Status::OK();
}

template <typename SpecializedOptions>
Status BlockParser::DoParseSpecialized(const std::vector<util::string_view>& views,
                                       bool is_final, uint32_t* out_size) {
  num_rows_ = 0;
  values_size_ = 0;
  parsed_size_ = 0;
  values_buffers_.clear();
  parsed_buffer_.reset();
  parsed_ = nullptr;

  size_t total_view_length = 0;
  for (const auto& view : views) {
    total_view_length += view.length();
  }
  if (total_view_length > std::numeric_limits<uint32_t>::max()) {
    return Status::Invalid("CSV block too large");
  }

  PresizedParsedWriter parsed_writer(pool_, static_cast<uint32_t>(total_view_length));
  uint32_t total_parsed_length = 0;

  for (const auto& view : views) {
    const char* data = view.data();
    const char* data_end = view.data() + view.length();
    bool finished_parsing = false;

    if (num_cols_ == -1) {
      // Can't presize values when the number of columns is not known, first parse
      // a single line
      const int32_t rows_in_chunk = 1;
      ResizableValuesWriter values_writer(pool_);
      values_writer.Start(parsed_writer);

      RETURN_NOT_OK(ParseChunk<SpecializedOptions>(&values_writer, &parsed_writer, data,
                                                   data_end, is_final, rows_in_chunk,
                                                   &data, &finished_parsing));
      if (num_cols_ == -1) {
        return ParseError("Empty CSV file or block: cannot infer number of columns");
      }
    }

    while (!finished_parsing && data < data_end && num_rows_ < max_num_rows_) {
      // We know the number of columns, so can presize a values array for
      // a given number of rows
      DCHECK_GE(num_cols_, 0);

      int32_t rows_in_chunk;
      constexpr int32_t kTargetChunkSize = 32768;
      if (num_cols_ > 0) {
        rows_in_chunk = std::min(std::max(kTargetChunkSize / num_cols_, 512),
                                 max_num_rows_ - num_rows_);
      } else {
        rows_in_chunk = std::min(kTargetChunkSize, max_num_rows_ - num_rows_);
      }

      PresizedValuesWriter values_writer(pool_, rows_in_chunk, num_cols_);
      values_writer.Start(parsed_writer);

      RETURN_NOT_OK(ParseChunk<SpecializedOptions>(&values_writer, &parsed_writer, data,
                                                   data_end, is_final, rows_in_chunk,
                                                   &data, &finished_parsing));
    }
    DCHECK_GE(data, view.data());
    DCHECK_LE(data, data_end);
    total_parsed_length += static_cast<uint32_t>(data - view.data());

    if (data < data_end) {
      // Stopped early, for some reason
      break;
    }
  }

  parsed_writer.Finish(&parsed_buffer_);
  parsed_size_ = static_cast<int32_t>(parsed_buffer_->size());
  parsed_ = parsed_buffer_->data();

  DCHECK_EQ(values_size_, num_rows_ * num_cols_);
  if (num_cols_ == -1) {
    DCHECK_EQ(num_rows_, 0);
  }
#ifndef NDEBUG
  if (num_rows_ > 0) {
    DCHECK_GT(values_buffers_.size(), 0);
    auto& last_values_buffer = values_buffers_.back();
    auto last_values = reinterpret_cast<const ValueDesc*>(last_values_buffer->data());
    auto last_values_size = last_values_buffer->size() / sizeof(ValueDesc);
    auto check_parsed_size =
        static_cast<int32_t>(last_values[last_values_size - 1].offset);
    DCHECK_EQ(parsed_size_, check_parsed_size);
  } else {
    DCHECK_EQ(parsed_size_, 0);
  }
#endif
  *out_size = static_cast<uint32_t>(total_parsed_length);
  return Status::OK();
}

Status BlockParser::DoParse(const std::vector<util::string_view>& data, bool is_final,
                            uint32_t* out_size) {
  if (options_.quoting) {
    if (options_.escaping) {
      return DoParseSpecialized<SpecializedOptions<true, true>>(data, is_final, out_size);
    } else {
      return DoParseSpecialized<SpecializedOptions<true, false>>(data, is_final,
                                                                 out_size);
    }
  } else {
    if (options_.escaping) {
      return DoParseSpecialized<SpecializedOptions<false, true>>(data, is_final,
                                                                 out_size);
    } else {
      return DoParseSpecialized<SpecializedOptions<false, false>>(data, is_final,
                                                                  out_size);
    }
  }
}

Status BlockParser::Parse(const std::vector<util::string_view>& data,
                          uint32_t* out_size) {
  return DoParse(data, false /* is_final */, out_size);
}

Status BlockParser::ParseFinal(const std::vector<util::string_view>& data,
                               uint32_t* out_size) {
  return DoParse(data, true /* is_final */, out_size);
}

Status BlockParser::Parse(util::string_view data, uint32_t* out_size) {
  return DoParse({data}, false /* is_final */, out_size);
}

Status BlockParser::ParseFinal(util::string_view data, uint32_t* out_size) {
  return DoParse({data}, true /* is_final */, out_size);
}

BlockParser::BlockParser(MemoryPool* pool, ParseOptions options, int32_t num_cols,
                         int32_t max_num_rows)
    : pool_(pool),
      options_(options),
      num_rows_(-1),
      num_cols_(num_cols),
      max_num_rows_(max_num_rows) {}

BlockParser::BlockParser(ParseOptions options, int32_t num_cols, int32_t max_num_rows)
    : BlockParser(default_memory_pool(), options, num_cols, max_num_rows) {}

}  // namespace csv
}  // namespace arrow
