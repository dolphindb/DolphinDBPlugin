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

#include "parquet/column_writer.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "arrow/array.h"
#include "arrow/buffer_builder.h"
#include "arrow/compute/api.h"
#include "arrow/status.h"
#include "arrow/type.h"
#include "arrow/type_traits.h"
#include "arrow/util/bit_stream_utils.h"
#include "arrow/util/checked_cast.h"
#include "arrow/util/compression.h"
#include "arrow/util/logging.h"
#include "arrow/util/rle_encoding.h"
#include "parquet/column_page.h"
#include "parquet/encoding.h"
#include "parquet/encryption_internal.h"
#include "parquet/internal_file_encryptor.h"
#include "parquet/metadata.h"
#include "parquet/platform.h"
#include "parquet/properties.h"
#include "parquet/schema.h"
#include "parquet/statistics.h"
#include "parquet/thrift_internal.h"
#include "parquet/types.h"

using arrow::Datum;
using arrow::Status;
using arrow::BitUtil::BitWriter;
using arrow::internal::checked_cast;
using arrow::util::RleEncoder;

namespace parquet {

namespace {

inline const int16_t* AddIfNotNull(const int16_t* base, int64_t offset) {
  if (base != nullptr) {
    return base + offset;
  }
  return nullptr;
}

}  // namespace

LevelEncoder::LevelEncoder() {}
LevelEncoder::~LevelEncoder() {}

void LevelEncoder::Init(Encoding::type encoding, int16_t max_level,
                        int num_buffered_values, uint8_t* data, int data_size) {
  bit_width_ = BitUtil::Log2(max_level + 1);
  encoding_ = encoding;
  switch (encoding) {
    case Encoding::RLE: {
      rle_encoder_.reset(new RleEncoder(data, data_size, bit_width_));
      break;
    }
    case Encoding::BIT_PACKED: {
      int num_bytes =
          static_cast<int>(BitUtil::BytesForBits(num_buffered_values * bit_width_));
      bit_packed_encoder_.reset(new BitWriter(data, num_bytes));
      break;
    }
    default:
      throw ParquetException("Unknown encoding type for levels.");
  }
}

int LevelEncoder::MaxBufferSize(Encoding::type encoding, int16_t max_level,
                                int num_buffered_values) {
  int bit_width = BitUtil::Log2(max_level + 1);
  int num_bytes = 0;
  switch (encoding) {
    case Encoding::RLE: {
      // TODO: Due to the way we currently check if the buffer is full enough,
      // we need to have MinBufferSize as head room.
      num_bytes = RleEncoder::MaxBufferSize(bit_width, num_buffered_values) +
                  RleEncoder::MinBufferSize(bit_width);
      break;
    }
    case Encoding::BIT_PACKED: {
      num_bytes =
          static_cast<int>(BitUtil::BytesForBits(num_buffered_values * bit_width));
      break;
    }
    default:
      throw ParquetException("Unknown encoding type for levels.");
  }
  return num_bytes;
}

int LevelEncoder::Encode(int batch_size, const int16_t* levels) {
  int num_encoded = 0;
  if (!rle_encoder_ && !bit_packed_encoder_) {
    throw ParquetException("Level encoders are not initialized.");
  }

  if (encoding_ == Encoding::RLE) {
    for (int i = 0; i < batch_size; ++i) {
      if (!rle_encoder_->Put(*(levels + i))) {
        break;
      }
      ++num_encoded;
    }
    rle_encoder_->Flush();
    rle_length_ = rle_encoder_->len();
  } else {
    for (int i = 0; i < batch_size; ++i) {
      if (!bit_packed_encoder_->PutValue(*(levels + i), bit_width_)) {
        break;
      }
      ++num_encoded;
    }
    bit_packed_encoder_->Flush();
  }
  return num_encoded;
}

// ----------------------------------------------------------------------
// PageWriter implementation

// This subclass delimits pages appearing in a serialized stream, each preceded
// by a serialized Thrift format::PageHeader indicating the type of each page
// and the page metadata.
class SerializedPageWriter : public PageWriter {
 public:
  SerializedPageWriter(std::shared_ptr<ArrowOutputStream> sink, Compression::type codec,
                       int compression_level, ColumnChunkMetaDataBuilder* metadata,
                       int16_t row_group_ordinal, int16_t column_chunk_ordinal,
                       MemoryPool* pool = ::arrow::default_memory_pool(),
                       std::shared_ptr<Encryptor> meta_encryptor = nullptr,
                       std::shared_ptr<Encryptor> data_encryptor = nullptr)
      : sink_(std::move(sink)),
        metadata_(metadata),
        pool_(pool),
        num_values_(0),
        dictionary_page_offset_(0),
        data_page_offset_(0),
        total_uncompressed_size_(0),
        total_compressed_size_(0),
        page_ordinal_(0),
        row_group_ordinal_(row_group_ordinal),
        column_ordinal_(column_chunk_ordinal),
        meta_encryptor_(std::move(meta_encryptor)),
        data_encryptor_(std::move(data_encryptor)),
        encryption_buffer_(AllocateBuffer(pool, 0)) {
    if (data_encryptor_ != nullptr || meta_encryptor_ != nullptr) {
      InitEncryption();
    }
    compressor_ = internal::GetWriteCodec(codec, compression_level);
    thrift_serializer_.reset(new ThriftSerializer);
  }

  int64_t WriteDictionaryPage(const DictionaryPage& page) override {
    int64_t uncompressed_size = page.size();
    std::shared_ptr<Buffer> compressed_data;
    if (has_compressor()) {
      auto buffer = std::static_pointer_cast<ResizableBuffer>(
          AllocateBuffer(pool_, uncompressed_size));
      Compress(*(page.buffer().get()), buffer.get());
      compressed_data = std::static_pointer_cast<Buffer>(buffer);
    } else {
      compressed_data = page.buffer();
    }

    format::DictionaryPageHeader dict_page_header;
    dict_page_header.__set_num_values(page.num_values());
    dict_page_header.__set_encoding(ToThrift(page.encoding()));
    dict_page_header.__set_is_sorted(page.is_sorted());

    const uint8_t* output_data_buffer = compressed_data->data();
    int32_t output_data_len = static_cast<int32_t>(compressed_data->size());

    if (data_encryptor_.get()) {
      UpdateEncryption(encryption::kDictionaryPage);
      PARQUET_THROW_NOT_OK(encryption_buffer_->Resize(
          data_encryptor_->CiphertextSizeDelta() + output_data_len, false));
      output_data_len = data_encryptor_->Encrypt(compressed_data->data(), output_data_len,
                                                 encryption_buffer_->mutable_data());
      output_data_buffer = encryption_buffer_->data();
    }

    format::PageHeader page_header;
    page_header.__set_type(format::PageType::DICTIONARY_PAGE);
    page_header.__set_uncompressed_page_size(static_cast<int32_t>(uncompressed_size));
    page_header.__set_compressed_page_size(static_cast<int32_t>(output_data_len));
    page_header.__set_dictionary_page_header(dict_page_header);
    // TODO(PARQUET-594) crc checksum

    PARQUET_ASSIGN_OR_THROW(int64_t start_pos, sink_->Tell());
    if (dictionary_page_offset_ == 0) {
      dictionary_page_offset_ = start_pos;
    }

    if (meta_encryptor_) {
      UpdateEncryption(encryption::kDictionaryPageHeader);
    }
    int64_t header_size =
        thrift_serializer_->Serialize(&page_header, sink_.get(), meta_encryptor_);

    PARQUET_THROW_NOT_OK(sink_->Write(output_data_buffer, output_data_len));

    total_uncompressed_size_ += uncompressed_size + header_size;
    total_compressed_size_ += output_data_len + header_size;
    ++dict_encoding_stats_[page.encoding()];

    PARQUET_ASSIGN_OR_THROW(int64_t final_pos, sink_->Tell());
    return final_pos - start_pos;
  }

  void Close(bool has_dictionary, bool fallback) override {
    if (meta_encryptor_ != nullptr) {
      UpdateEncryption(encryption::kColumnMetaData);
    }
    // index_page_offset = -1 since they are not supported
    metadata_->Finish(num_values_, dictionary_page_offset_, -1, data_page_offset_,
                      total_compressed_size_, total_uncompressed_size_, has_dictionary,
                      fallback, dict_encoding_stats_, data_encoding_stats_,
                      meta_encryptor_);
    // Write metadata at end of column chunk
    metadata_->WriteTo(sink_.get());
  }

  /**
   * Compress a buffer.
   */
  void Compress(const Buffer& src_buffer, ResizableBuffer* dest_buffer) override {
    DCHECK(compressor_ != nullptr);

    // Compress the data
    int64_t max_compressed_size =
        compressor_->MaxCompressedLen(src_buffer.size(), src_buffer.data());

    // Use Arrow::Buffer::shrink_to_fit = false
    // underlying buffer only keeps growing. Resize to a smaller size does not reallocate.
    PARQUET_THROW_NOT_OK(dest_buffer->Resize(max_compressed_size, false));

    PARQUET_ASSIGN_OR_THROW(
        int64_t compressed_size,
        compressor_->Compress(src_buffer.size(), src_buffer.data(), max_compressed_size,
                              dest_buffer->mutable_data()));
    PARQUET_THROW_NOT_OK(dest_buffer->Resize(compressed_size, false));
  }

  int64_t WriteDataPage(const DataPage& page) override {
    int64_t uncompressed_size = page.uncompressed_size();
    std::shared_ptr<Buffer> compressed_data = page.buffer();
    const uint8_t* output_data_buffer = compressed_data->data();
    int32_t output_data_len = static_cast<int32_t>(compressed_data->size());

    if (data_encryptor_.get()) {
      PARQUET_THROW_NOT_OK(encryption_buffer_->Resize(
          data_encryptor_->CiphertextSizeDelta() + output_data_len, false));
      UpdateEncryption(encryption::kDataPage);
      output_data_len = data_encryptor_->Encrypt(compressed_data->data(), output_data_len,
                                                 encryption_buffer_->mutable_data());
      output_data_buffer = encryption_buffer_->data();
    }

    format::PageHeader page_header;
    page_header.__set_uncompressed_page_size(static_cast<int32_t>(uncompressed_size));
    page_header.__set_compressed_page_size(static_cast<int32_t>(output_data_len));
    // TODO(PARQUET-594) crc checksum

    if (page.type() == PageType::DATA_PAGE) {
      const DataPageV1& v1_page = checked_cast<const DataPageV1&>(page);
      SetDataPageHeader(page_header, v1_page);
    } else if (page.type() == PageType::DATA_PAGE_V2) {
      const DataPageV2& v2_page = checked_cast<const DataPageV2&>(page);
      SetDataPageV2Header(page_header, v2_page);
    } else {
      throw ParquetException("Unexpected page type");
    }

    PARQUET_ASSIGN_OR_THROW(int64_t start_pos, sink_->Tell());
    if (page_ordinal_ == 0) {
      data_page_offset_ = start_pos;
    }

    if (meta_encryptor_) {
      UpdateEncryption(encryption::kDataPageHeader);
    }
    int64_t header_size =
        thrift_serializer_->Serialize(&page_header, sink_.get(), meta_encryptor_);
    PARQUET_THROW_NOT_OK(sink_->Write(output_data_buffer, output_data_len));

    total_uncompressed_size_ += uncompressed_size + header_size;
    total_compressed_size_ += output_data_len + header_size;
    num_values_ += page.num_values();
    ++data_encoding_stats_[page.encoding()];
    ++page_ordinal_;
    PARQUET_ASSIGN_OR_THROW(int64_t current_pos, sink_->Tell());
    return current_pos - start_pos;
  }

  void SetDataPageHeader(format::PageHeader& page_header, const DataPageV1& page) {
    format::DataPageHeader data_page_header;
    data_page_header.__set_num_values(page.num_values());
    data_page_header.__set_encoding(ToThrift(page.encoding()));
    data_page_header.__set_definition_level_encoding(
        ToThrift(page.definition_level_encoding()));
    data_page_header.__set_repetition_level_encoding(
        ToThrift(page.repetition_level_encoding()));
    data_page_header.__set_statistics(ToThrift(page.statistics()));

    page_header.__set_type(format::PageType::DATA_PAGE);
    page_header.__set_data_page_header(data_page_header);
  }

  void SetDataPageV2Header(format::PageHeader& page_header, const DataPageV2 page) {
    format::DataPageHeaderV2 data_page_header;
    data_page_header.__set_num_values(page.num_values());
    data_page_header.__set_num_nulls(page.num_nulls());
    data_page_header.__set_num_rows(page.num_rows());
    data_page_header.__set_encoding(ToThrift(page.encoding()));

    data_page_header.__set_definition_levels_byte_length(
        page.definition_levels_byte_length());
    data_page_header.__set_repetition_levels_byte_length(
        page.repetition_levels_byte_length());

    data_page_header.__set_is_compressed(page.is_compressed());
    data_page_header.__set_statistics(ToThrift(page.statistics()));

    page_header.__set_type(format::PageType::DATA_PAGE_V2);
    page_header.__set_data_page_header_v2(data_page_header);
  }

  bool has_compressor() override { return (compressor_ != nullptr); }

  int64_t num_values() { return num_values_; }

  int64_t dictionary_page_offset() { return dictionary_page_offset_; }

  int64_t data_page_offset() { return data_page_offset_; }

  int64_t total_compressed_size() { return total_compressed_size_; }

  int64_t total_uncompressed_size() { return total_uncompressed_size_; }

 private:
  // To allow UpdateEncryption on Close
  friend class BufferedPageWriter;

  void InitEncryption() {
    // Prepare the AAD for quick update later.
    if (data_encryptor_ != nullptr) {
      data_page_aad_ = encryption::CreateModuleAad(
          data_encryptor_->file_aad(), encryption::kDataPage, row_group_ordinal_,
          column_ordinal_, kNonPageOrdinal);
    }
    if (meta_encryptor_ != nullptr) {
      data_page_header_aad_ = encryption::CreateModuleAad(
          meta_encryptor_->file_aad(), encryption::kDataPageHeader, row_group_ordinal_,
          column_ordinal_, kNonPageOrdinal);
    }
  }

  void UpdateEncryption(int8_t module_type) {
    switch (module_type) {
      case encryption::kColumnMetaData: {
        meta_encryptor_->UpdateAad(encryption::CreateModuleAad(
            meta_encryptor_->file_aad(), module_type, row_group_ordinal_, column_ordinal_,
            kNonPageOrdinal));
        break;
      }
      case encryption::kDataPage: {
        encryption::QuickUpdatePageAad(data_page_aad_, page_ordinal_);
        data_encryptor_->UpdateAad(data_page_aad_);
        break;
      }
      case encryption::kDataPageHeader: {
        encryption::QuickUpdatePageAad(data_page_header_aad_, page_ordinal_);
        meta_encryptor_->UpdateAad(data_page_header_aad_);
        break;
      }
      case encryption::kDictionaryPageHeader: {
        meta_encryptor_->UpdateAad(encryption::CreateModuleAad(
            meta_encryptor_->file_aad(), module_type, row_group_ordinal_, column_ordinal_,
            kNonPageOrdinal));
        break;
      }
      case encryption::kDictionaryPage: {
        data_encryptor_->UpdateAad(encryption::CreateModuleAad(
            data_encryptor_->file_aad(), module_type, row_group_ordinal_, column_ordinal_,
            kNonPageOrdinal));
        break;
      }
      default:
        throw ParquetException("Unknown module type in UpdateEncryption");
    }
  }

  std::shared_ptr<ArrowOutputStream> sink_;
  ColumnChunkMetaDataBuilder* metadata_;
  MemoryPool* pool_;
  int64_t num_values_;
  int64_t dictionary_page_offset_;
  int64_t data_page_offset_;
  int64_t total_uncompressed_size_;
  int64_t total_compressed_size_;
  int16_t page_ordinal_;
  int16_t row_group_ordinal_;
  int16_t column_ordinal_;

  std::unique_ptr<ThriftSerializer> thrift_serializer_;

  // Compression codec to use.
  std::unique_ptr<::arrow::util::Codec> compressor_;

  std::string data_page_aad_;
  std::string data_page_header_aad_;

  std::shared_ptr<Encryptor> meta_encryptor_;
  std::shared_ptr<Encryptor> data_encryptor_;

  std::shared_ptr<ResizableBuffer> encryption_buffer_;

  std::map<Encoding::type, int32_t> dict_encoding_stats_;
  std::map<Encoding::type, int32_t> data_encoding_stats_;
};

// This implementation of the PageWriter writes to the final sink on Close .
class BufferedPageWriter : public PageWriter {
 public:
  BufferedPageWriter(std::shared_ptr<ArrowOutputStream> sink, Compression::type codec,
                     int compression_level, ColumnChunkMetaDataBuilder* metadata,
                     int16_t row_group_ordinal, int16_t current_column_ordinal,
                     MemoryPool* pool = ::arrow::default_memory_pool(),
                     std::shared_ptr<Encryptor> meta_encryptor = nullptr,
                     std::shared_ptr<Encryptor> data_encryptor = nullptr)
      : final_sink_(std::move(sink)), metadata_(metadata), has_dictionary_pages_(false) {
    in_memory_sink_ = CreateOutputStream(pool);
    pager_ = std::unique_ptr<SerializedPageWriter>(
        new SerializedPageWriter(in_memory_sink_, codec, compression_level, metadata,
                                 row_group_ordinal, current_column_ordinal, pool,
                                 std::move(meta_encryptor), std::move(data_encryptor)));
  }

  int64_t WriteDictionaryPage(const DictionaryPage& page) override {
    has_dictionary_pages_ = true;
    return pager_->WriteDictionaryPage(page);
  }

  void Close(bool has_dictionary, bool fallback) override {
    if (pager_->meta_encryptor_ != nullptr) {
      pager_->UpdateEncryption(encryption::kColumnMetaData);
    }
    // index_page_offset = -1 since they are not supported
    PARQUET_ASSIGN_OR_THROW(int64_t final_position, final_sink_->Tell());
    // dictionary page offset should be 0 iff there are no dictionary pages
    auto dictionary_page_offset =
        has_dictionary_pages_ ? pager_->dictionary_page_offset() + final_position : 0;
    metadata_->Finish(pager_->num_values(), dictionary_page_offset, -1,
                      pager_->data_page_offset() + final_position,
                      pager_->total_compressed_size(), pager_->total_uncompressed_size(),
                      has_dictionary, fallback, pager_->dict_encoding_stats_,
                      pager_->data_encoding_stats_, pager_->meta_encryptor_);

    // Write metadata at end of column chunk
    metadata_->WriteTo(in_memory_sink_.get());

    // flush everything to the serialized sink
    PARQUET_ASSIGN_OR_THROW(auto buffer, in_memory_sink_->Finish());
    PARQUET_THROW_NOT_OK(final_sink_->Write(buffer));
  }

  int64_t WriteDataPage(const DataPage& page) override {
    return pager_->WriteDataPage(page);
  }

  void Compress(const Buffer& src_buffer, ResizableBuffer* dest_buffer) override {
    pager_->Compress(src_buffer, dest_buffer);
  }

  bool has_compressor() override { return pager_->has_compressor(); }

 private:
  std::shared_ptr<ArrowOutputStream> final_sink_;
  ColumnChunkMetaDataBuilder* metadata_;
  std::shared_ptr<::arrow::io::BufferOutputStream> in_memory_sink_;
  std::unique_ptr<SerializedPageWriter> pager_;
  bool has_dictionary_pages_;
};

std::unique_ptr<PageWriter> PageWriter::Open(
    std::shared_ptr<ArrowOutputStream> sink, Compression::type codec,
    int compression_level, ColumnChunkMetaDataBuilder* metadata,
    int16_t row_group_ordinal, int16_t column_chunk_ordinal, MemoryPool* pool,
    bool buffered_row_group, std::shared_ptr<Encryptor> meta_encryptor,
    std::shared_ptr<Encryptor> data_encryptor) {
  if (buffered_row_group) {
    return std::unique_ptr<PageWriter>(
        new BufferedPageWriter(std::move(sink), codec, compression_level, metadata,
                               row_group_ordinal, column_chunk_ordinal, pool,
                               std::move(meta_encryptor), std::move(data_encryptor)));
  } else {
    return std::unique_ptr<PageWriter>(
        new SerializedPageWriter(std::move(sink), codec, compression_level, metadata,
                                 row_group_ordinal, column_chunk_ordinal, pool,
                                 std::move(meta_encryptor), std::move(data_encryptor)));
  }
}

// ----------------------------------------------------------------------
// ColumnWriter

const std::shared_ptr<WriterProperties>& default_writer_properties() {
  static std::shared_ptr<WriterProperties> default_writer_properties =
      WriterProperties::Builder().build();
  return default_writer_properties;
}

class ColumnWriterImpl {
 public:
  ColumnWriterImpl(ColumnChunkMetaDataBuilder* metadata,
                   std::unique_ptr<PageWriter> pager, const bool use_dictionary,
                   Encoding::type encoding, const WriterProperties* properties)
      : metadata_(metadata),
        descr_(metadata->descr()),
        pager_(std::move(pager)),
        has_dictionary_(use_dictionary),
        encoding_(encoding),
        properties_(properties),
        allocator_(properties->memory_pool()),
        num_buffered_values_(0),
        num_buffered_encoded_values_(0),
        rows_written_(0),
        total_bytes_written_(0),
        total_compressed_bytes_(0),
        closed_(false),
        fallback_(false),
        definition_levels_sink_(allocator_),
        repetition_levels_sink_(allocator_) {
    definition_levels_rle_ =
        std::static_pointer_cast<ResizableBuffer>(AllocateBuffer(allocator_, 0));
    repetition_levels_rle_ =
        std::static_pointer_cast<ResizableBuffer>(AllocateBuffer(allocator_, 0));
    uncompressed_data_ =
        std::static_pointer_cast<ResizableBuffer>(AllocateBuffer(allocator_, 0));
    if (pager_->has_compressor()) {
      compressor_temp_buffer_ =
          std::static_pointer_cast<ResizableBuffer>(AllocateBuffer(allocator_, 0));
    }
  }

  virtual ~ColumnWriterImpl() = default;

  int64_t Close();

 protected:
  virtual std::shared_ptr<Buffer> GetValuesBuffer() = 0;

  // Serializes Dictionary Page if enabled
  virtual void WriteDictionaryPage() = 0;

  // Plain-encoded statistics of the current page
  virtual EncodedStatistics GetPageStatistics() = 0;

  // Plain-encoded statistics of the whole chunk
  virtual EncodedStatistics GetChunkStatistics() = 0;

  // Merges page statistics into chunk statistics, then resets the values
  virtual void ResetPageStatistics() = 0;

  // Adds Data Pages to an in memory buffer in dictionary encoding mode
  // Serializes the Data Pages in other encoding modes
  void AddDataPage();

  void BuildDataPageV1(int64_t definition_levels_rle_size,
                       int64_t repetition_levels_rle_size, int64_t uncompressed_size,
                       const std::shared_ptr<Buffer>& values);
  void BuildDataPageV2(int64_t definition_levels_rle_size,
                       int64_t repetition_levels_rle_size, int64_t uncompressed_size,
                       const std::shared_ptr<Buffer>& values);

  // Serializes Data Pages
  void WriteDataPage(const DataPage& page) {
    total_bytes_written_ += pager_->WriteDataPage(page);
  }

  // Write multiple definition levels
  void WriteDefinitionLevels(int64_t num_levels, const int16_t* levels) {
    DCHECK(!closed_);
    PARQUET_THROW_NOT_OK(
        definition_levels_sink_.Append(levels, sizeof(int16_t) * num_levels));
  }

  // Write multiple repetition levels
  void WriteRepetitionLevels(int64_t num_levels, const int16_t* levels) {
    DCHECK(!closed_);
    PARQUET_THROW_NOT_OK(
        repetition_levels_sink_.Append(levels, sizeof(int16_t) * num_levels));
  }

  // RLE encode the src_buffer into dest_buffer and return the encoded size
  int64_t RleEncodeLevels(const void* src_buffer, ResizableBuffer* dest_buffer,
                          int16_t max_level, bool include_length_prefix = true);

  // Serialize the buffered Data Pages
  void FlushBufferedDataPages();

  ColumnChunkMetaDataBuilder* metadata_;
  const ColumnDescriptor* descr_;

  std::unique_ptr<PageWriter> pager_;

  bool has_dictionary_;
  Encoding::type encoding_;
  const WriterProperties* properties_;

  LevelEncoder level_encoder_;

  MemoryPool* allocator_;

  // The total number of values stored in the data page. This is the maximum of
  // the number of encoded definition levels or encoded values. For
  // non-repeated, required columns, this is equal to the number of encoded
  // values. For repeated or optional values, there may be fewer data values
  // than levels, and this tells you how many encoded levels there are in that
  // case.
  int64_t num_buffered_values_;

  // The total number of stored values. For repeated or optional values, this
  // number may be lower than num_buffered_values_.
  int64_t num_buffered_encoded_values_;

  // Total number of rows written with this ColumnWriter
  int rows_written_;

  // Records the total number of bytes written by the serializer
  int64_t total_bytes_written_;

  // Records the current number of compressed bytes in a column
  int64_t total_compressed_bytes_;

  // Flag to check if the Writer has been closed
  bool closed_;

  // Flag to infer if dictionary encoding has fallen back to PLAIN
  bool fallback_;

  ::arrow::BufferBuilder definition_levels_sink_;
  ::arrow::BufferBuilder repetition_levels_sink_;

  std::shared_ptr<ResizableBuffer> definition_levels_rle_;
  std::shared_ptr<ResizableBuffer> repetition_levels_rle_;

  std::shared_ptr<ResizableBuffer> uncompressed_data_;
  std::shared_ptr<ResizableBuffer> compressor_temp_buffer_;

  std::vector<std::unique_ptr<DataPage>> data_pages_;

 private:
  void InitSinks() {
    definition_levels_sink_.Rewind(0);
    repetition_levels_sink_.Rewind(0);
  }

  // Concatenate the encoded levels and values into one buffer
  void ConcatenateBuffers(int64_t definition_levels_rle_size,
                          int64_t repetition_levels_rle_size,
                          const std::shared_ptr<Buffer>& values, uint8_t* combined) {
    memcpy(combined, repetition_levels_rle_->data(), repetition_levels_rle_size);
    combined += repetition_levels_rle_size;
    memcpy(combined, definition_levels_rle_->data(), definition_levels_rle_size);
    combined += definition_levels_rle_size;
    memcpy(combined, values->data(), values->size());
  }
};

// return the size of the encoded buffer
int64_t ColumnWriterImpl::RleEncodeLevels(const void* src_buffer,
                                          ResizableBuffer* dest_buffer, int16_t max_level,
                                          bool include_length_prefix) {
  // V1 DataPage includes the length of the RLE level as a prefix.
  int32_t prefix_size = include_length_prefix ? sizeof(int32_t) : 0;

  // TODO: This only works with due to some RLE specifics
  int64_t rle_size = LevelEncoder::MaxBufferSize(Encoding::RLE, max_level,
                                                 static_cast<int>(num_buffered_values_)) +
                     prefix_size;

  // Use Arrow::Buffer::shrink_to_fit = false
  // underlying buffer only keeps growing. Resize to a smaller size does not reallocate.
  PARQUET_THROW_NOT_OK(dest_buffer->Resize(rle_size, false));

  level_encoder_.Init(Encoding::RLE, max_level, static_cast<int>(num_buffered_values_),
                      dest_buffer->mutable_data() + prefix_size,
                      static_cast<int>(dest_buffer->size() - prefix_size));
  int encoded = level_encoder_.Encode(static_cast<int>(num_buffered_values_),
                                      reinterpret_cast<const int16_t*>(src_buffer));
  DCHECK_EQ(encoded, num_buffered_values_);

  if (include_length_prefix) {
    reinterpret_cast<int32_t*>(dest_buffer->mutable_data())[0] = level_encoder_.len();
  }

  return level_encoder_.len() + prefix_size;
}

void ColumnWriterImpl::AddDataPage() {
  int64_t definition_levels_rle_size = 0;
  int64_t repetition_levels_rle_size = 0;

  std::shared_ptr<Buffer> values = GetValuesBuffer();
  bool is_v1_data_page = properties_->data_page_version() == ParquetDataPageVersion::V1;

  if (descr_->max_definition_level() > 0) {
    definition_levels_rle_size = RleEncodeLevels(
        definition_levels_sink_.data(), definition_levels_rle_.get(),
        descr_->max_definition_level(), /*include_length_prefix=*/is_v1_data_page);
  }

  if (descr_->max_repetition_level() > 0) {
    repetition_levels_rle_size = RleEncodeLevels(
        repetition_levels_sink_.data(), repetition_levels_rle_.get(),
        descr_->max_repetition_level(), /*include_length_prefix=*/is_v1_data_page);
  }

  int64_t uncompressed_size =
      definition_levels_rle_size + repetition_levels_rle_size + values->size();

  if (is_v1_data_page) {
    BuildDataPageV1(definition_levels_rle_size, repetition_levels_rle_size,
                    uncompressed_size, values);
  } else {
    BuildDataPageV2(definition_levels_rle_size, repetition_levels_rle_size,
                    uncompressed_size, values);
  }

  // Re-initialize the sinks for next Page.
  InitSinks();
  num_buffered_values_ = 0;
  num_buffered_encoded_values_ = 0;
}

void ColumnWriterImpl::BuildDataPageV1(int64_t definition_levels_rle_size,
                                       int64_t repetition_levels_rle_size,
                                       int64_t uncompressed_size,
                                       const std::shared_ptr<Buffer>& values) {
  // Use Arrow::Buffer::shrink_to_fit = false
  // underlying buffer only keeps growing. Resize to a smaller size does not reallocate.
  PARQUET_THROW_NOT_OK(uncompressed_data_->Resize(uncompressed_size, false));
  ConcatenateBuffers(definition_levels_rle_size, repetition_levels_rle_size, values,
                     uncompressed_data_->mutable_data());

  EncodedStatistics page_stats = GetPageStatistics();
  page_stats.ApplyStatSizeLimits(properties_->max_statistics_size(descr_->path()));
  page_stats.set_is_signed(SortOrder::SIGNED == descr_->sort_order());
  ResetPageStatistics();

  std::shared_ptr<Buffer> compressed_data;
  if (pager_->has_compressor()) {
    pager_->Compress(*(uncompressed_data_.get()), compressor_temp_buffer_.get());
    compressed_data = compressor_temp_buffer_;
  } else {
    compressed_data = uncompressed_data_;
  }

  // Write the page to OutputStream eagerly if there is no dictionary or
  // if dictionary encoding has fallen back to PLAIN
  if (has_dictionary_ && !fallback_) {  // Save pages until end of dictionary encoding
    PARQUET_ASSIGN_OR_THROW(
        auto compressed_data_copy,
        compressed_data->CopySlice(0, compressed_data->size(), allocator_));
    std::unique_ptr<DataPage> page_ptr(new DataPageV1(
        compressed_data_copy, static_cast<int32_t>(num_buffered_values_), encoding_,
        Encoding::RLE, Encoding::RLE, uncompressed_size, page_stats));
    total_compressed_bytes_ += page_ptr->size() + sizeof(format::PageHeader);

    data_pages_.push_back(std::move(page_ptr));
  } else {  // Eagerly write pages
    DataPageV1 page(compressed_data, static_cast<int32_t>(num_buffered_values_),
                    encoding_, Encoding::RLE, Encoding::RLE, uncompressed_size,
                    page_stats);
    WriteDataPage(page);
  }
}

void ColumnWriterImpl::BuildDataPageV2(int64_t definition_levels_rle_size,
                                       int64_t repetition_levels_rle_size,
                                       int64_t uncompressed_size,
                                       const std::shared_ptr<Buffer>& values) {
  // Compress the values if needed. Repetition and definition levels are uncompressed in
  // V2.
  std::shared_ptr<Buffer> compressed_values;
  if (pager_->has_compressor()) {
    pager_->Compress(*values, compressor_temp_buffer_.get());
    compressed_values = compressor_temp_buffer_;
  } else {
    compressed_values = values;
  }

  // Concatenate uncompressed levels and the possibly compressed values
  int64_t combined_size =
      definition_levels_rle_size + repetition_levels_rle_size + compressed_values->size();
  std::shared_ptr<ResizableBuffer> combined = AllocateBuffer(allocator_, combined_size);

  ConcatenateBuffers(definition_levels_rle_size, repetition_levels_rle_size,
                     compressed_values, combined->mutable_data());

  EncodedStatistics page_stats = GetPageStatistics();
  page_stats.ApplyStatSizeLimits(properties_->max_statistics_size(descr_->path()));
  page_stats.set_is_signed(SortOrder::SIGNED == descr_->sort_order());
  ResetPageStatistics();

  int32_t num_values = static_cast<int32_t>(num_buffered_values_);
  int32_t null_count = static_cast<int32_t>(page_stats.null_count);
  int32_t def_levels_byte_length = static_cast<int32_t>(definition_levels_rle_size);
  int32_t rep_levels_byte_length = static_cast<int32_t>(repetition_levels_rle_size);

  // Write the page to OutputStream eagerly if there is no dictionary or
  // if dictionary encoding has fallen back to PLAIN
  if (has_dictionary_ && !fallback_) {  // Save pages until end of dictionary encoding
    PARQUET_ASSIGN_OR_THROW(auto data_copy,
                            combined->CopySlice(0, combined->size(), allocator_));
    std::unique_ptr<DataPage> page_ptr(new DataPageV2(
        combined, num_values, null_count, num_values, encoding_, def_levels_byte_length,
        rep_levels_byte_length, uncompressed_size, pager_->has_compressor()));
    total_compressed_bytes_ += page_ptr->size() + sizeof(format::PageHeader);
    data_pages_.push_back(std::move(page_ptr));
  } else {
    DataPageV2 page(combined, num_values, null_count, num_values, encoding_,
                    def_levels_byte_length, rep_levels_byte_length, uncompressed_size);
    WriteDataPage(page);
  }
}

int64_t ColumnWriterImpl::Close() {
  if (!closed_) {
    closed_ = true;
    if (has_dictionary_ && !fallback_) {
      WriteDictionaryPage();
    }

    FlushBufferedDataPages();

    EncodedStatistics chunk_statistics = GetChunkStatistics();
    chunk_statistics.ApplyStatSizeLimits(
        properties_->max_statistics_size(descr_->path()));
    chunk_statistics.set_is_signed(SortOrder::SIGNED == descr_->sort_order());

    // Write stats only if the column has at least one row written
    if (rows_written_ > 0 && chunk_statistics.is_set()) {
      metadata_->SetStatistics(chunk_statistics);
    }
    pager_->Close(has_dictionary_, fallback_);
  }

  return total_bytes_written_;
}

void ColumnWriterImpl::FlushBufferedDataPages() {
  // Write all outstanding data to a new page
  if (num_buffered_values_ > 0) {
    AddDataPage();
  }
  for (const auto& page_ptr : data_pages_) {
    WriteDataPage(*page_ptr);
  }
  data_pages_.clear();
  total_compressed_bytes_ = 0;
}

// ----------------------------------------------------------------------
// TypedColumnWriter

template <typename Action>
inline void DoInBatches(int64_t total, int64_t batch_size, Action&& action) {
  int64_t num_batches = static_cast<int>(total / batch_size);
  for (int round = 0; round < num_batches; round++) {
    action(round * batch_size, batch_size);
  }
  // Write the remaining values
  if (total % batch_size > 0) {
    action(num_batches * batch_size, total % batch_size);
  }
}

bool DictionaryDirectWriteSupported(const ::arrow::Array& array) {
  DCHECK_EQ(array.type_id(), ::arrow::Type::DICTIONARY);
  const ::arrow::DictionaryType& dict_type =
      static_cast<const ::arrow::DictionaryType&>(*array.type());
  auto id = dict_type.value_type()->id();
  return id == ::arrow::Type::BINARY || id == ::arrow::Type::STRING;
}

Status ConvertDictionaryToDense(const ::arrow::Array& array, MemoryPool* pool,
                                std::shared_ptr<::arrow::Array>* out) {
  const ::arrow::DictionaryType& dict_type =
      static_cast<const ::arrow::DictionaryType&>(*array.type());

  // TODO(ARROW-1648): Remove this special handling once we require an Arrow
  // version that has this fixed.
  if (dict_type.value_type()->id() == ::arrow::Type::NA) {
    *out = std::make_shared<::arrow::NullArray>(array.length());
    return Status::OK();
  }

  ::arrow::compute::ExecContext ctx(pool);
  ARROW_ASSIGN_OR_RAISE(Datum cast_output,
                        ::arrow::compute::Cast(array.data(), dict_type.value_type(),
                                               ::arrow::compute::CastOptions(), &ctx));
  *out = cast_output.make_array();
  return Status::OK();
}

static inline bool IsDictionaryEncoding(Encoding::type encoding) {
  return encoding == Encoding::PLAIN_DICTIONARY;
}

template <typename DType>
class TypedColumnWriterImpl : public ColumnWriterImpl, public TypedColumnWriter<DType> {
 public:
  using T = typename DType::c_type;

  TypedColumnWriterImpl(ColumnChunkMetaDataBuilder* metadata,
                        std::unique_ptr<PageWriter> pager, const bool use_dictionary,
                        Encoding::type encoding, const WriterProperties* properties)
      : ColumnWriterImpl(metadata, std::move(pager), use_dictionary, encoding,
                         properties) {
    current_encoder_ = MakeEncoder(DType::type_num, encoding, use_dictionary, descr_,
                                   properties->memory_pool());

    if (properties->statistics_enabled(descr_->path()) &&
        (SortOrder::UNKNOWN != descr_->sort_order())) {
      page_statistics_ = MakeStatistics<DType>(descr_, allocator_);
      chunk_statistics_ = MakeStatistics<DType>(descr_, allocator_);
    }
  }

  int64_t Close() override { return ColumnWriterImpl::Close(); }

  int64_t WriteBatch(int64_t num_values, const int16_t* def_levels,
                     const int16_t* rep_levels, const T* values) override {
    // We check for DataPage limits only after we have inserted the values. If a user
    // writes a large number of values, the DataPage size can be much above the limit.
    // The purpose of this chunking is to bound this. Even if a user writes large number
    // of values, the chunking will ensure the AddDataPage() is called at a reasonable
    // pagesize limit
    int64_t value_offset = 0;
    auto WriteChunk = [&](int64_t offset, int64_t batch_size) {
      int64_t values_to_write = WriteLevels(batch_size, AddIfNotNull(def_levels, offset),
                                            AddIfNotNull(rep_levels, offset));
      // PARQUET-780
      if (values_to_write > 0) {
        DCHECK_NE(nullptr, values);
      }
      WriteValues(values + value_offset, values_to_write, batch_size - values_to_write);
      CommitWriteAndCheckPageLimit(batch_size, values_to_write);
      value_offset += values_to_write;

      // Dictionary size checked separately from data page size since we
      // circumvent this check when writing ::arrow::DictionaryArray directly
      CheckDictionarySizeLimit();
    };
    DoInBatches(num_values, properties_->write_batch_size(), WriteChunk);
    return value_offset;
  }

  void WriteBatchSpaced(int64_t num_values, const int16_t* def_levels,
                        const int16_t* rep_levels, const uint8_t* valid_bits,
                        int64_t valid_bits_offset, const T* values) override {
    // Like WriteBatch, but for spaced values
    int64_t value_offset = 0;
    auto WriteChunk = [&](int64_t offset, int64_t batch_size) {
      int64_t batch_num_values = 0;
      int64_t batch_num_spaced_values = 0;
      WriteLevelsSpaced(batch_size, AddIfNotNull(def_levels, offset),
                        AddIfNotNull(rep_levels, offset), &batch_num_values,
                        &batch_num_spaced_values);
      WriteValuesSpaced(values + value_offset, batch_num_values, batch_num_spaced_values,
                        valid_bits, valid_bits_offset + value_offset);
      CommitWriteAndCheckPageLimit(batch_size, batch_num_spaced_values);
      value_offset += batch_num_spaced_values;

      // Dictionary size checked separately from data page size since we
      // circumvent this check when writing ::arrow::DictionaryArray directly
      CheckDictionarySizeLimit();
    };
    DoInBatches(num_values, properties_->write_batch_size(), WriteChunk);
  }

  Status WriteArrow(const int16_t* def_levels, const int16_t* rep_levels,
                    int64_t num_levels, const ::arrow::Array& array,
                    ArrowWriteContext* ctx) override {
    if (array.type()->id() == ::arrow::Type::DICTIONARY) {
      return WriteArrowDictionary(def_levels, rep_levels, num_levels, array, ctx);
    } else {
      return WriteArrowDense(def_levels, rep_levels, num_levels, array, ctx);
    }
  }

  int64_t EstimatedBufferedValueBytes() const override {
    return current_encoder_->EstimatedDataEncodedSize();
  }

 protected:
  std::shared_ptr<Buffer> GetValuesBuffer() override {
    return current_encoder_->FlushValues();
  }

  // Internal function to handle direct writing of ::arrow::DictionaryArray,
  // since the standard logic concerning dictionary size limits and fallback to
  // plain encoding is circumvented
  Status WriteArrowDictionary(const int16_t* def_levels, const int16_t* rep_levels,
                              int64_t num_levels, const ::arrow::Array& array,
                              ArrowWriteContext* context);

  Status WriteArrowDense(const int16_t* def_levels, const int16_t* rep_levels,
                         int64_t num_levels, const ::arrow::Array& array,
                         ArrowWriteContext* context);

  void WriteDictionaryPage() override {
    // We have to dynamic cast here because of TypedEncoder<Type> as
    // some compilers don't want to cast through virtual inheritance
    auto dict_encoder = dynamic_cast<DictEncoder<DType>*>(current_encoder_.get());
    DCHECK(dict_encoder);
    std::shared_ptr<ResizableBuffer> buffer =
        AllocateBuffer(properties_->memory_pool(), dict_encoder->dict_encoded_size());
    dict_encoder->WriteDict(buffer->mutable_data());

    DictionaryPage page(buffer, dict_encoder->num_entries(),
                        properties_->dictionary_page_encoding());
    total_bytes_written_ += pager_->WriteDictionaryPage(page);
  }

  EncodedStatistics GetPageStatistics() override {
    EncodedStatistics result;
    if (page_statistics_) result = page_statistics_->Encode();
    return result;
  }

  EncodedStatistics GetChunkStatistics() override {
    EncodedStatistics result;
    if (chunk_statistics_) result = chunk_statistics_->Encode();
    return result;
  }

  void ResetPageStatistics() override {
    if (chunk_statistics_ != nullptr) {
      chunk_statistics_->Merge(*page_statistics_);
      page_statistics_->Reset();
    }
  }

  Type::type type() const override { return descr_->physical_type(); }

  const ColumnDescriptor* descr() const override { return descr_; }

  int64_t rows_written() const override { return rows_written_; }

  int64_t total_compressed_bytes() const override { return total_compressed_bytes_; }

  int64_t total_bytes_written() const override { return total_bytes_written_; }

  const WriterProperties* properties() override { return properties_; }

 private:
  using ValueEncoderType = typename EncodingTraits<DType>::Encoder;
  using TypedStats = TypedStatistics<DType>;
  std::unique_ptr<Encoder> current_encoder_;
  std::shared_ptr<TypedStats> page_statistics_;
  std::shared_ptr<TypedStats> chunk_statistics_;

  // If writing a sequence of ::arrow::DictionaryArray to the writer, we keep the
  // dictionary passed to DictEncoder<T>::PutDictionary so we can check
  // subsequent array chunks to see either if materialization is required (in
  // which case we call back to the dense write path)
  std::shared_ptr<::arrow::Array> preserved_dictionary_;

  int64_t WriteLevels(int64_t num_values, const int16_t* def_levels,
                      const int16_t* rep_levels) {
    int64_t values_to_write = 0;
    // If the field is required and non-repeated, there are no definition levels
    if (descr_->max_definition_level() > 0) {
      for (int64_t i = 0; i < num_values; ++i) {
        if (def_levels[i] == descr_->max_definition_level()) {
          ++values_to_write;
        }
      }

      WriteDefinitionLevels(num_values, def_levels);
    } else {
      // Required field, write all values
      values_to_write = num_values;
    }

    // Not present for non-repeated fields
    if (descr_->max_repetition_level() > 0) {
      // A row could include more than one value
      // Count the occasions where we start a new row
      for (int64_t i = 0; i < num_values; ++i) {
        if (rep_levels[i] == 0) {
          rows_written_++;
        }
      }

      WriteRepetitionLevels(num_values, rep_levels);
    } else {
      // Each value is exactly one row
      rows_written_ += static_cast<int>(num_values);
    }
    return values_to_write;
  }

  void WriteLevelsSpaced(int64_t num_levels, const int16_t* def_levels,
                         const int16_t* rep_levels, int64_t* out_values_to_write,
                         int64_t* out_spaced_values_to_write) {
    int64_t values_to_write = 0;
    int64_t spaced_values_to_write = 0;
    // If the field is required and non-repeated, there are no definition levels
    if (descr_->max_definition_level() > 0) {
      // Minimal definition level for which spaced values are written
      int16_t min_spaced_def_level = descr_->max_definition_level();
      const ::parquet::schema::Node* node = descr_->schema_node().get();
      while (node != nullptr && !node->is_repeated()) {
        if (node->is_optional()) {
          min_spaced_def_level--;
        }
        node = node->parent();
      }
      for (int64_t i = 0; i < num_levels; ++i) {
        if (def_levels[i] == descr_->max_definition_level()) {
          ++values_to_write;
        }
        if (def_levels[i] >= min_spaced_def_level) {
          ++spaced_values_to_write;
        }
      }
      WriteDefinitionLevels(num_levels, def_levels);
    } else {
      // Required field, write all values
      values_to_write = num_levels;
      spaced_values_to_write = num_levels;
    }

    // Not present for non-repeated fields
    if (descr_->max_repetition_level() > 0) {
      // A row could include more than one value
      // Count the occasions where we start a new row
      for (int64_t i = 0; i < num_levels; ++i) {
        if (rep_levels[i] == 0) {
          rows_written_++;
        }
      }

      WriteRepetitionLevels(num_levels, rep_levels);
    } else {
      // Each value is exactly one row
      rows_written_ += static_cast<int>(num_levels);
    }

    *out_values_to_write = values_to_write;
    *out_spaced_values_to_write = spaced_values_to_write;
  }

  void CommitWriteAndCheckPageLimit(int64_t num_levels, int64_t num_values) {
    num_buffered_values_ += num_levels;
    num_buffered_encoded_values_ += num_values;

    if (current_encoder_->EstimatedDataEncodedSize() >= properties_->data_pagesize()) {
      AddDataPage();
    }
  }

  void FallbackToPlainEncoding() {
    if (IsDictionaryEncoding(current_encoder_->encoding())) {
      WriteDictionaryPage();
      // Serialize the buffered Dictionary Indices
      FlushBufferedDataPages();
      fallback_ = true;
      // Only PLAIN encoding is supported for fallback in V1
      current_encoder_ = MakeEncoder(DType::type_num, Encoding::PLAIN, false, descr_,
                                     properties_->memory_pool());
      encoding_ = Encoding::PLAIN;
    }
  }

  // Checks if the Dictionary Page size limit is reached
  // If the limit is reached, the Dictionary and Data Pages are serialized
  // The encoding is switched to PLAIN
  //
  // Only one Dictionary Page is written.
  // Fallback to PLAIN if dictionary page limit is reached.
  void CheckDictionarySizeLimit() {
    if (!has_dictionary_ || fallback_) {
      // Either not using dictionary encoding, or we have already fallen back
      // to PLAIN encoding because the size threshold was reached
      return;
    }

    // We have to dynamic cast here because TypedEncoder<Type> as some compilers
    // don't want to cast through virtual inheritance
    auto dict_encoder = dynamic_cast<DictEncoder<DType>*>(current_encoder_.get());
    if (dict_encoder->dict_encoded_size() >= properties_->dictionary_pagesize_limit()) {
      FallbackToPlainEncoding();
    }
  }

  void WriteValues(const T* values, int64_t num_values, int64_t num_nulls) {
    dynamic_cast<ValueEncoderType*>(current_encoder_.get())
        ->Put(values, static_cast<int>(num_values));
    if (page_statistics_ != nullptr) {
      page_statistics_->Update(values, num_values, num_nulls);
    }
  }

  void WriteValuesSpaced(const T* values, int64_t num_values, int64_t num_spaced_values,
                         const uint8_t* valid_bits, int64_t valid_bits_offset) {
    if (descr_->schema_node()->is_optional()) {
      dynamic_cast<ValueEncoderType*>(current_encoder_.get())
          ->PutSpaced(values, static_cast<int>(num_spaced_values), valid_bits,
                      valid_bits_offset);
    } else {
      dynamic_cast<ValueEncoderType*>(current_encoder_.get())
          ->Put(values, static_cast<int>(num_values));
    }
    if (page_statistics_ != nullptr) {
      const int64_t num_nulls = num_spaced_values - num_values;
      page_statistics_->UpdateSpaced(values, valid_bits, valid_bits_offset, num_values,
                                     num_nulls);
    }
  }
};

template <typename DType>
Status TypedColumnWriterImpl<DType>::WriteArrowDictionary(const int16_t* def_levels,
                                                          const int16_t* rep_levels,
                                                          int64_t num_levels,
                                                          const ::arrow::Array& array,
                                                          ArrowWriteContext* ctx) {
  // If this is the first time writing a DictionaryArray, then there's
  // a few possible paths to take:
  //
  // - If dictionary encoding is not enabled, convert to densely
  //   encoded and call WriteArrow
  // - Dictionary encoding enabled
  //   - If this is the first time this is called, then we call
  //     PutDictionary into the encoder and then PutIndices on each
  //     chunk. We store the dictionary that was written in
  //     preserved_dictionary_ so that subsequent calls to this method
  //     can make sure the dictionary has not changed
  //   - On subsequent calls, we have to check whether the dictionary
  //     has changed. If it has, then we trigger the varying
  //     dictionary path and materialize each chunk and then call
  //     WriteArrow with that
  auto WriteDense = [&] {
    std::shared_ptr<::arrow::Array> dense_array;
    RETURN_NOT_OK(
        ConvertDictionaryToDense(array, properties_->memory_pool(), &dense_array));
    return WriteArrowDense(def_levels, rep_levels, num_levels, *dense_array, ctx);
  };

  if (!IsDictionaryEncoding(current_encoder_->encoding()) ||
      !DictionaryDirectWriteSupported(array)) {
    // No longer dictionary-encoding for whatever reason, maybe we never were
    // or we decided to stop. Note that WriteArrow can be invoked multiple
    // times with both dense and dictionary-encoded versions of the same data
    // without a problem. Any dense data will be hashed to indices until the
    // dictionary page limit is reached, at which everything (dictionary and
    // dense) will fall back to plain encoding
    return WriteDense();
  }

  auto dict_encoder = dynamic_cast<DictEncoder<DType>*>(current_encoder_.get());
  const auto& data = checked_cast<const ::arrow::DictionaryArray&>(array);
  std::shared_ptr<::arrow::Array> dictionary = data.dictionary();
  std::shared_ptr<::arrow::Array> indices = data.indices();

  int64_t value_offset = 0;
  auto WriteIndicesChunk = [&](int64_t offset, int64_t batch_size) {
    int64_t batch_num_values = 0;
    int64_t batch_num_spaced_values = 0;
    WriteLevelsSpaced(batch_size, AddIfNotNull(def_levels, offset),
                      AddIfNotNull(rep_levels, offset), &batch_num_values,
                      &batch_num_spaced_values);
    dict_encoder->PutIndices(*indices->Slice(value_offset, batch_num_spaced_values));
    CommitWriteAndCheckPageLimit(batch_size, batch_num_values);
    value_offset += batch_num_spaced_values;
  };

  // Handle seeing dictionary for the first time
  if (!preserved_dictionary_) {
    // It's a new dictionary. Call PutDictionary and keep track of it
    PARQUET_CATCH_NOT_OK(dict_encoder->PutDictionary(*dictionary));

    // TODO(wesm): If some dictionary values are unobserved, then the
    // statistics will be inaccurate. Do we care enough to fix it?
    if (page_statistics_ != nullptr) {
      PARQUET_CATCH_NOT_OK(page_statistics_->Update(*dictionary));
    }
    preserved_dictionary_ = dictionary;
  } else if (!dictionary->Equals(*preserved_dictionary_)) {
    // Dictionary has changed
    PARQUET_CATCH_NOT_OK(FallbackToPlainEncoding());
    return WriteDense();
  }

  PARQUET_CATCH_NOT_OK(
      DoInBatches(num_levels, properties_->write_batch_size(), WriteIndicesChunk));
  return Status::OK();
}

// ----------------------------------------------------------------------
// Direct Arrow write path

template <typename ParquetType, typename ArrowType, typename Enable = void>
struct SerializeFunctor {
  using ArrowCType = typename ArrowType::c_type;
  using ArrayType = typename ::arrow::TypeTraits<ArrowType>::ArrayType;
  using ParquetCType = typename ParquetType::c_type;
  Status Serialize(const ArrayType& array, ArrowWriteContext*, ParquetCType* out) {
    const ArrowCType* input = array.raw_values();
    if (array.null_count() > 0) {
      for (int i = 0; i < array.length(); i++) {
        out[i] = static_cast<ParquetCType>(input[i]);
      }
    } else {
      std::copy(input, input + array.length(), out);
    }
    return Status::OK();
  }
};

template <typename ParquetType, typename ArrowType>
Status WriteArrowSerialize(const ::arrow::Array& array, int64_t num_levels,
                           const int16_t* def_levels, const int16_t* rep_levels,
                           ArrowWriteContext* ctx,
                           TypedColumnWriter<ParquetType>* writer) {
  using ParquetCType = typename ParquetType::c_type;
  using ArrayType = typename ::arrow::TypeTraits<ArrowType>::ArrayType;

  ParquetCType* buffer = nullptr;
  PARQUET_THROW_NOT_OK(ctx->GetScratchData<ParquetCType>(array.length(), &buffer));

  bool no_nulls =
      writer->descr()->schema_node()->is_required() || (array.null_count() == 0);

  SerializeFunctor<ParquetType, ArrowType> functor;
  RETURN_NOT_OK(functor.Serialize(checked_cast<const ArrayType&>(array), ctx, buffer));

  if (no_nulls) {
    PARQUET_CATCH_NOT_OK(writer->WriteBatch(num_levels, def_levels, rep_levels, buffer));
  } else {
    PARQUET_CATCH_NOT_OK(writer->WriteBatchSpaced(num_levels, def_levels, rep_levels,
                                                  array.null_bitmap_data(),
                                                  array.offset(), buffer));
  }
  return Status::OK();
}

template <typename ParquetType>
Status WriteArrowZeroCopy(const ::arrow::Array& array, int64_t num_levels,
                          const int16_t* def_levels, const int16_t* rep_levels,
                          ArrowWriteContext* ctx,
                          TypedColumnWriter<ParquetType>* writer) {
  using T = typename ParquetType::c_type;
  const auto& data = static_cast<const ::arrow::PrimitiveArray&>(array);
  const T* values = nullptr;
  // The values buffer may be null if the array is empty (ARROW-2744)
  if (data.values() != nullptr) {
    values = reinterpret_cast<const T*>(data.values()->data()) + data.offset();
  } else {
    DCHECK_EQ(data.length(), 0);
  }
  if (writer->descr()->schema_node()->is_required() || (data.null_count() == 0)) {
    PARQUET_CATCH_NOT_OK(writer->WriteBatch(num_levels, def_levels, rep_levels, values));
  } else {
    PARQUET_CATCH_NOT_OK(writer->WriteBatchSpaced(num_levels, def_levels, rep_levels,
                                                  data.null_bitmap_data(), data.offset(),
                                                  values));
  }
  return Status::OK();
}

#define WRITE_SERIALIZE_CASE(ArrowEnum, ArrowType, ParquetType)  \
  case ::arrow::Type::ArrowEnum:                                 \
    return WriteArrowSerialize<ParquetType, ::arrow::ArrowType>( \
        array, num_levels, def_levels, rep_levels, ctx, this);

#define WRITE_ZERO_COPY_CASE(ArrowEnum, ArrowType, ParquetType)                       \
  case ::arrow::Type::ArrowEnum:                                                      \
    return WriteArrowZeroCopy<ParquetType>(array, num_levels, def_levels, rep_levels, \
                                           ctx, this);

#define ARROW_UNSUPPORTED()                                          \
  std::stringstream ss;                                              \
  ss << "Arrow type " << array.type()->ToString()                    \
     << " cannot be written to Parquet type " << descr_->ToString(); \
  return Status::Invalid(ss.str());

// ----------------------------------------------------------------------
// Write Arrow to BooleanType

template <>
struct SerializeFunctor<BooleanType, ::arrow::BooleanType> {
  Status Serialize(const ::arrow::BooleanArray& data, ArrowWriteContext*, bool* out) {
    for (int i = 0; i < data.length(); i++) {
      *out++ = data.Value(i);
    }
    return Status::OK();
  }
};

template <>
Status TypedColumnWriterImpl<BooleanType>::WriteArrowDense(const int16_t* def_levels,
                                                           const int16_t* rep_levels,
                                                           int64_t num_levels,
                                                           const ::arrow::Array& array,
                                                           ArrowWriteContext* ctx) {
  if (array.type_id() != ::arrow::Type::BOOL) {
    ARROW_UNSUPPORTED();
  }
  return WriteArrowSerialize<BooleanType, ::arrow::BooleanType>(
      array, num_levels, def_levels, rep_levels, ctx, this);
}

// ----------------------------------------------------------------------
// Write Arrow types to INT32

template <>
struct SerializeFunctor<Int32Type, ::arrow::Date64Type> {
  Status Serialize(const ::arrow::Date64Array& array, ArrowWriteContext*, int32_t* out) {
    const int64_t* input = array.raw_values();
    for (int i = 0; i < array.length(); i++) {
      *out++ = static_cast<int32_t>(*input++ / 86400000);
    }
    return Status::OK();
  }
};

template <>
struct SerializeFunctor<Int32Type, ::arrow::Time32Type> {
  Status Serialize(const ::arrow::Time32Array& array, ArrowWriteContext*, int32_t* out) {
    const int32_t* input = array.raw_values();
    const auto& type = static_cast<const ::arrow::Time32Type&>(*array.type());
    if (type.unit() == ::arrow::TimeUnit::SECOND) {
      for (int i = 0; i < array.length(); i++) {
        out[i] = input[i] * 1000;
      }
    } else {
      std::copy(input, input + array.length(), out);
    }
    return Status::OK();
  }
};

template <>
Status TypedColumnWriterImpl<Int32Type>::WriteArrowDense(const int16_t* def_levels,
                                                         const int16_t* rep_levels,
                                                         int64_t num_levels,
                                                         const ::arrow::Array& array,
                                                         ArrowWriteContext* ctx) {
  switch (array.type()->id()) {
    case ::arrow::Type::NA: {
      PARQUET_CATCH_NOT_OK(WriteBatch(num_levels, def_levels, rep_levels, nullptr));
    } break;
      WRITE_SERIALIZE_CASE(INT8, Int8Type, Int32Type)
      WRITE_SERIALIZE_CASE(UINT8, UInt8Type, Int32Type)
      WRITE_SERIALIZE_CASE(INT16, Int16Type, Int32Type)
      WRITE_SERIALIZE_CASE(UINT16, UInt16Type, Int32Type)
      WRITE_SERIALIZE_CASE(UINT32, UInt32Type, Int32Type)
      WRITE_ZERO_COPY_CASE(INT32, Int32Type, Int32Type)
      WRITE_ZERO_COPY_CASE(DATE32, Date32Type, Int32Type)
      WRITE_SERIALIZE_CASE(DATE64, Date64Type, Int32Type)
      WRITE_SERIALIZE_CASE(TIME32, Time32Type, Int32Type)
    default:
      ARROW_UNSUPPORTED()
  }
  return Status::OK();
}

// ----------------------------------------------------------------------
// Write Arrow to Int64 and Int96

#define INT96_CONVERT_LOOP(ConversionFunction) \
  for (int64_t i = 0; i < array.length(); i++) ConversionFunction(input[i], &out[i]);

template <>
struct SerializeFunctor<Int96Type, ::arrow::TimestampType> {
  Status Serialize(const ::arrow::TimestampArray& array, ArrowWriteContext*, Int96* out) {
    const int64_t* input = array.raw_values();
    const auto& type = static_cast<const ::arrow::TimestampType&>(*array.type());
    switch (type.unit()) {
      case ::arrow::TimeUnit::NANO:
        INT96_CONVERT_LOOP(internal::NanosecondsToImpalaTimestamp);
        break;
      case ::arrow::TimeUnit::MICRO:
        INT96_CONVERT_LOOP(internal::MicrosecondsToImpalaTimestamp);
        break;
      case ::arrow::TimeUnit::MILLI:
        INT96_CONVERT_LOOP(internal::MillisecondsToImpalaTimestamp);
        break;
      case ::arrow::TimeUnit::SECOND:
        INT96_CONVERT_LOOP(internal::SecondsToImpalaTimestamp);
        break;
    }
    return Status::OK();
  }
};

#define COERCE_DIVIDE -1
#define COERCE_INVALID 0
#define COERCE_MULTIPLY +1

static std::pair<int, int64_t> kTimestampCoercionFactors[4][4] = {
    // from seconds ...
    {{COERCE_INVALID, 0},                      // ... to seconds
     {COERCE_MULTIPLY, 1000},                  // ... to millis
     {COERCE_MULTIPLY, 1000000},               // ... to micros
     {COERCE_MULTIPLY, INT64_C(1000000000)}},  // ... to nanos
    // from millis ...
    {{COERCE_INVALID, 0},
     {COERCE_MULTIPLY, 1},
     {COERCE_MULTIPLY, 1000},
     {COERCE_MULTIPLY, 1000000}},
    // from micros ...
    {{COERCE_INVALID, 0},
     {COERCE_DIVIDE, 1000},
     {COERCE_MULTIPLY, 1},
     {COERCE_MULTIPLY, 1000}},
    // from nanos ...
    {{COERCE_INVALID, 0},
     {COERCE_DIVIDE, 1000000},
     {COERCE_DIVIDE, 1000},
     {COERCE_MULTIPLY, 1}}};

template <>
struct SerializeFunctor<Int64Type, ::arrow::TimestampType> {
  Status Serialize(const ::arrow::TimestampArray& array, ArrowWriteContext* ctx,
                   int64_t* out) {
    const auto& source_type = static_cast<const ::arrow::TimestampType&>(*array.type());
    auto source_unit = source_type.unit();
    const int64_t* values = array.raw_values();

    ::arrow::TimeUnit::type target_unit = ctx->properties->coerce_timestamps_unit();
    auto target_type = ::arrow::timestamp(target_unit);
    bool truncation_allowed = ctx->properties->truncated_timestamps_allowed();

    auto DivideBy = [&](const int64_t factor) {
      for (int64_t i = 0; i < array.length(); i++) {
        if (!truncation_allowed && array.IsValid(i) && (values[i] % factor != 0)) {
          return Status::Invalid("Casting from ", source_type.ToString(), " to ",
                                 target_type->ToString(),
                                 " would lose data: ", values[i]);
        }
        out[i] = values[i] / factor;
      }
      return Status::OK();
    };

    auto MultiplyBy = [&](const int64_t factor) {
      for (int64_t i = 0; i < array.length(); i++) {
        out[i] = values[i] * factor;
      }
      return Status::OK();
    };

    const auto& coercion = kTimestampCoercionFactors[static_cast<int>(source_unit)]
                                                    [static_cast<int>(target_unit)];

    // .first -> coercion operation; .second -> scale factor
    DCHECK_NE(coercion.first, COERCE_INVALID);
    return coercion.first == COERCE_DIVIDE ? DivideBy(coercion.second)
                                           : MultiplyBy(coercion.second);
  }
};

#undef COERCE_DIVIDE
#undef COERCE_INVALID
#undef COERCE_MULTIPLY

Status WriteTimestamps(const ::arrow::Array& values, int64_t num_levels,
                       const int16_t* def_levels, const int16_t* rep_levels,
                       ArrowWriteContext* ctx, TypedColumnWriter<Int64Type>* writer) {
  const auto& source_type = static_cast<const ::arrow::TimestampType&>(*values.type());

  auto WriteCoerce = [&](const ArrowWriterProperties* properties) {
    ArrowWriteContext temp_ctx = *ctx;
    temp_ctx.properties = properties;
    return WriteArrowSerialize<Int64Type, ::arrow::TimestampType>(
        values, num_levels, def_levels, rep_levels, &temp_ctx, writer);
  };

  if (ctx->properties->coerce_timestamps_enabled()) {
    // User explicitly requested coercion to specific unit
    if (source_type.unit() == ctx->properties->coerce_timestamps_unit()) {
      // No data conversion necessary
      return WriteArrowZeroCopy<Int64Type>(values, num_levels, def_levels, rep_levels,
                                           ctx, writer);
    } else {
      return WriteCoerce(ctx->properties);
    }
  } else if (writer->properties()->version() == ParquetVersion::PARQUET_1_0 &&
             source_type.unit() == ::arrow::TimeUnit::NANO) {
    // Absent superseding user instructions, when writing Parquet version 1.0 files,
    // timestamps in nanoseconds are coerced to microseconds
    std::shared_ptr<ArrowWriterProperties> properties =
        (ArrowWriterProperties::Builder())
            .coerce_timestamps(::arrow::TimeUnit::MICRO)
            ->disallow_truncated_timestamps()
            ->build();
    return WriteCoerce(properties.get());
  } else if (source_type.unit() == ::arrow::TimeUnit::SECOND) {
    // Absent superseding user instructions, timestamps in seconds are coerced to
    // milliseconds
    std::shared_ptr<ArrowWriterProperties> properties =
        (ArrowWriterProperties::Builder())
            .coerce_timestamps(::arrow::TimeUnit::MILLI)
            ->build();
    return WriteCoerce(properties.get());
  } else {
    // No data conversion necessary
    return WriteArrowZeroCopy<Int64Type>(values, num_levels, def_levels, rep_levels, ctx,
                                         writer);
  }
}

template <>
Status TypedColumnWriterImpl<Int64Type>::WriteArrowDense(const int16_t* def_levels,
                                                         const int16_t* rep_levels,
                                                         int64_t num_levels,
                                                         const ::arrow::Array& array,
                                                         ArrowWriteContext* ctx) {
  switch (array.type()->id()) {
    case ::arrow::Type::TIMESTAMP:
      return WriteTimestamps(array, num_levels, def_levels, rep_levels, ctx, this);
      WRITE_ZERO_COPY_CASE(INT64, Int64Type, Int64Type)
      WRITE_SERIALIZE_CASE(UINT32, UInt32Type, Int64Type)
      WRITE_SERIALIZE_CASE(UINT64, UInt64Type, Int64Type)
      WRITE_ZERO_COPY_CASE(TIME64, Time64Type, Int64Type)
    default:
      ARROW_UNSUPPORTED();
  }
}

template <>
Status TypedColumnWriterImpl<Int96Type>::WriteArrowDense(const int16_t* def_levels,
                                                         const int16_t* rep_levels,
                                                         int64_t num_levels,
                                                         const ::arrow::Array& array,
                                                         ArrowWriteContext* ctx) {
  if (array.type_id() != ::arrow::Type::TIMESTAMP) {
    ARROW_UNSUPPORTED();
  }
  return WriteArrowSerialize<Int96Type, ::arrow::TimestampType>(
      array, num_levels, def_levels, rep_levels, ctx, this);
}

// ----------------------------------------------------------------------
// Floating point types

template <>
Status TypedColumnWriterImpl<FloatType>::WriteArrowDense(const int16_t* def_levels,
                                                         const int16_t* rep_levels,
                                                         int64_t num_levels,
                                                         const ::arrow::Array& array,
                                                         ArrowWriteContext* ctx) {
  if (array.type_id() != ::arrow::Type::FLOAT) {
    ARROW_UNSUPPORTED();
  }
  return WriteArrowZeroCopy<FloatType>(array, num_levels, def_levels, rep_levels, ctx,
                                       this);
}

template <>
Status TypedColumnWriterImpl<DoubleType>::WriteArrowDense(const int16_t* def_levels,
                                                          const int16_t* rep_levels,
                                                          int64_t num_levels,
                                                          const ::arrow::Array& array,
                                                          ArrowWriteContext* ctx) {
  if (array.type_id() != ::arrow::Type::DOUBLE) {
    ARROW_UNSUPPORTED();
  }
  return WriteArrowZeroCopy<DoubleType>(array, num_levels, def_levels, rep_levels, ctx,
                                        this);
}

// ----------------------------------------------------------------------
// Write Arrow to BYTE_ARRAY

template <>
Status TypedColumnWriterImpl<ByteArrayType>::WriteArrowDense(const int16_t* def_levels,
                                                             const int16_t* rep_levels,
                                                             int64_t num_levels,
                                                             const ::arrow::Array& array,
                                                             ArrowWriteContext* ctx) {
  if (array.type()->id() != ::arrow::Type::BINARY &&
      array.type()->id() != ::arrow::Type::STRING) {
    ARROW_UNSUPPORTED();
  }

  int64_t value_offset = 0;
  auto WriteChunk = [&](int64_t offset, int64_t batch_size) {
    int64_t batch_num_values = 0;
    int64_t batch_num_spaced_values = 0;
    WriteLevelsSpaced(batch_size, AddIfNotNull(def_levels, offset),
                      AddIfNotNull(rep_levels, offset), &batch_num_values,
                      &batch_num_spaced_values);
    std::shared_ptr<::arrow::Array> data_slice =
        array.Slice(value_offset, batch_num_spaced_values);
    current_encoder_->Put(*data_slice);
    if (page_statistics_ != nullptr) {
      page_statistics_->Update(*data_slice);
    }
    CommitWriteAndCheckPageLimit(batch_size, batch_num_values);
    CheckDictionarySizeLimit();
    value_offset += batch_num_spaced_values;
  };

  PARQUET_CATCH_NOT_OK(
      DoInBatches(num_levels, properties_->write_batch_size(), WriteChunk));
  return Status::OK();
}

// ----------------------------------------------------------------------
// Write Arrow to FIXED_LEN_BYTE_ARRAY

template <typename ParquetType, typename ArrowType>
struct SerializeFunctor<
    ParquetType, ArrowType,
    ::arrow::enable_if_t<::arrow::is_fixed_size_binary_type<ArrowType>::value &&
                         !::arrow::is_decimal_type<ArrowType>::value>> {
  Status Serialize(const ::arrow::FixedSizeBinaryArray& array, ArrowWriteContext*,
                   FLBA* out) {
    if (array.null_count() == 0) {
      // no nulls, just dump the data
      // todo(advancedxy): use a writeBatch to avoid this step
      for (int64_t i = 0; i < array.length(); i++) {
        out[i] = FixedLenByteArray(array.GetValue(i));
      }
    } else {
      for (int64_t i = 0; i < array.length(); i++) {
        if (array.IsValid(i)) {
          out[i] = FixedLenByteArray(array.GetValue(i));
        }
      }
    }
    return Status::OK();
  }
};

// ----------------------------------------------------------------------
// Write Arrow to Decimal128

using ::arrow::internal::checked_pointer_cast;

// Requires a custom serializer because decimal128 in parquet are in big-endian
// format. Thus, a temporary local buffer is required.
template <typename ParquetType, typename ArrowType>
struct SerializeFunctor<ParquetType, ArrowType, ::arrow::enable_if_decimal<ArrowType>> {
  Status Serialize(const ::arrow::Decimal128Array& array, ArrowWriteContext* ctx,
                   FLBA* out) {
    AllocateScratch(array, ctx);
    auto offset = Offset(array);

    if (array.null_count() == 0) {
      for (int64_t i = 0; i < array.length(); i++) {
        out[i] = FixDecimalEndianess(array.GetValue(i), offset);
      }
    } else {
      for (int64_t i = 0; i < array.length(); i++) {
        out[i] = array.IsValid(i) ? FixDecimalEndianess(array.GetValue(i), offset)
                                  : FixedLenByteArray();
      }
    }

    return Status::OK();
  }

  // Parquet's Decimal are stored with FixedLength values where the length is
  // proportional to the precision. Arrow's Decimal are always stored with 16
  // bytes. Thus the internal FLBA pointer must be adjusted by the offset calculated
  // here.
  int32_t Offset(const ::arrow::Decimal128Array& array) {
    auto decimal_type = checked_pointer_cast<::arrow::Decimal128Type>(array.type());
    return decimal_type->byte_width() - internal::DecimalSize(decimal_type->precision());
  }

  void AllocateScratch(const ::arrow::Decimal128Array& array, ArrowWriteContext* ctx) {
    int64_t non_null_count = array.length() - array.null_count();
    int64_t size = non_null_count * 16;
    scratch_buffer = AllocateBuffer(ctx->memory_pool, size);
    scratch = reinterpret_cast<int64_t*>(scratch_buffer->mutable_data());
  }

  FixedLenByteArray FixDecimalEndianess(const uint8_t* in, int64_t offset) {
    const auto* u64_in = reinterpret_cast<const int64_t*>(in);
    auto out = reinterpret_cast<const uint8_t*>(scratch) + offset;
    *scratch++ = ::arrow::BitUtil::ToBigEndian(u64_in[1]);
    *scratch++ = ::arrow::BitUtil::ToBigEndian(u64_in[0]);
    return FixedLenByteArray(out);
  }

  std::shared_ptr<ResizableBuffer> scratch_buffer;
  int64_t* scratch;
};

template <>
Status TypedColumnWriterImpl<FLBAType>::WriteArrowDense(const int16_t* def_levels,
                                                        const int16_t* rep_levels,
                                                        int64_t num_levels,
                                                        const ::arrow::Array& array,
                                                        ArrowWriteContext* ctx) {
  switch (array.type()->id()) {
    WRITE_SERIALIZE_CASE(FIXED_SIZE_BINARY, FixedSizeBinaryType, FLBAType)
    WRITE_SERIALIZE_CASE(DECIMAL, Decimal128Type, FLBAType)
    default:
      break;
  }
  return Status::OK();
}

// ----------------------------------------------------------------------
// Dynamic column writer constructor

std::shared_ptr<ColumnWriter> ColumnWriter::Make(ColumnChunkMetaDataBuilder* metadata,
                                                 std::unique_ptr<PageWriter> pager,
                                                 const WriterProperties* properties) {
  const ColumnDescriptor* descr = metadata->descr();
  const bool use_dictionary = properties->dictionary_enabled(descr->path()) &&
                              descr->physical_type() != Type::BOOLEAN;
  Encoding::type encoding = properties->encoding(descr->path());
  if (use_dictionary) {
    encoding = properties->dictionary_index_encoding();
  }
  switch (descr->physical_type()) {
    case Type::BOOLEAN:
      return std::make_shared<TypedColumnWriterImpl<BooleanType>>(
          metadata, std::move(pager), use_dictionary, encoding, properties);
    case Type::INT32:
      return std::make_shared<TypedColumnWriterImpl<Int32Type>>(
          metadata, std::move(pager), use_dictionary, encoding, properties);
    case Type::INT64:
      return std::make_shared<TypedColumnWriterImpl<Int64Type>>(
          metadata, std::move(pager), use_dictionary, encoding, properties);
    case Type::INT96:
      return std::make_shared<TypedColumnWriterImpl<Int96Type>>(
          metadata, std::move(pager), use_dictionary, encoding, properties);
    case Type::FLOAT:
      return std::make_shared<TypedColumnWriterImpl<FloatType>>(
          metadata, std::move(pager), use_dictionary, encoding, properties);
    case Type::DOUBLE:
      return std::make_shared<TypedColumnWriterImpl<DoubleType>>(
          metadata, std::move(pager), use_dictionary, encoding, properties);
    case Type::BYTE_ARRAY:
      return std::make_shared<TypedColumnWriterImpl<ByteArrayType>>(
          metadata, std::move(pager), use_dictionary, encoding, properties);
    case Type::FIXED_LEN_BYTE_ARRAY:
      return std::make_shared<TypedColumnWriterImpl<FLBAType>>(
          metadata, std::move(pager), use_dictionary, encoding, properties);
    default:
      ParquetException::NYI("type reader not implemented");
  }
  // Unreachable code, but suppress compiler warning
  return std::shared_ptr<ColumnWriter>(nullptr);
}

}  // namespace parquet
