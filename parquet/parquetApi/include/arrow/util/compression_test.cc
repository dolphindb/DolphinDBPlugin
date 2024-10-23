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

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <memory>
#include <ostream>
#include <random>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "arrow/result.h"
#include "arrow/testing/gtest_util.h"
#include "arrow/testing/util.h"
#include "arrow/util/compression.h"

namespace arrow {
namespace util {

std::vector<uint8_t> MakeRandomData(int data_size) {
  std::vector<uint8_t> data(data_size);
  random_bytes(data_size, 1234, data.data());
  return data;
}

std::vector<uint8_t> MakeCompressibleData(int data_size) {
  std::string base_data =
      "Apache Arrow is a cross-language development platform for in-memory data";
  int nrepeats = static_cast<int>(1 + data_size / base_data.size());

  std::vector<uint8_t> data(base_data.size() * nrepeats);
  for (int i = 0; i < nrepeats; ++i) {
    std::memcpy(data.data() + i * base_data.size(), base_data.data(), base_data.size());
  }
  data.resize(data_size);
  return data;
}

// Check roundtrip of one-shot compression and decompression functions.
void CheckCodecRoundtrip(std::unique_ptr<Codec>& c1, std::unique_ptr<Codec>& c2,
                         const std::vector<uint8_t>& data) {
  int max_compressed_len =
      static_cast<int>(c1->MaxCompressedLen(data.size(), data.data()));
  std::vector<uint8_t> compressed(max_compressed_len);
  std::vector<uint8_t> decompressed(data.size());

  // compress with c1
  int64_t actual_size;
  ASSERT_OK_AND_ASSIGN(actual_size, c1->Compress(data.size(), data.data(),
                                                 max_compressed_len, compressed.data()));
  compressed.resize(actual_size);

  // decompress with c2
  int64_t actual_decompressed_size;
  ASSERT_OK_AND_ASSIGN(actual_decompressed_size,
                       c2->Decompress(compressed.size(), compressed.data(),
                                      decompressed.size(), decompressed.data()));

  ASSERT_EQ(data, decompressed);
  ASSERT_EQ(data.size(), actual_decompressed_size);

  // compress with c2
  ASSERT_EQ(max_compressed_len,
            static_cast<int>(c2->MaxCompressedLen(data.size(), data.data())));
  // Resize to prevent ASAN from detecting container overflow.
  compressed.resize(max_compressed_len);

  int64_t actual_size2;
  ASSERT_OK_AND_ASSIGN(actual_size2, c2->Compress(data.size(), data.data(),
                                                  max_compressed_len, compressed.data()));
  ASSERT_EQ(actual_size2, actual_size);
  compressed.resize(actual_size2);

  // decompress with c1
  int64_t actual_decompressed_size2;
  ASSERT_OK_AND_ASSIGN(actual_decompressed_size2,
                       c1->Decompress(compressed.size(), compressed.data(),
                                      decompressed.size(), decompressed.data()));

  ASSERT_EQ(data, decompressed);
  ASSERT_EQ(data.size(), actual_decompressed_size2);
}

// Check the streaming compressor against one-shot decompression

void CheckStreamingCompressor(Codec* codec, const std::vector<uint8_t>& data) {
  std::shared_ptr<Compressor> compressor;
  ASSERT_OK_AND_ASSIGN(compressor, codec->MakeCompressor());

  std::vector<uint8_t> compressed;
  int64_t compressed_size = 0;
  const uint8_t* input = data.data();
  int64_t remaining = data.size();

  compressed.resize(10);
  bool do_flush = false;

  while (remaining > 0) {
    // Feed a small amount each time
    int64_t input_len = std::min(remaining, static_cast<int64_t>(1111));
    int64_t output_len = compressed.size() - compressed_size;
    uint8_t* output = compressed.data() + compressed_size;
    ASSERT_OK_AND_ASSIGN(auto result,
                         compressor->Compress(input_len, input, output_len, output));
    ASSERT_LE(result.bytes_read, input_len);
    ASSERT_LE(result.bytes_written, output_len);
    compressed_size += result.bytes_written;
    input += result.bytes_read;
    remaining -= result.bytes_read;
    if (result.bytes_read == 0) {
      compressed.resize(compressed.capacity() * 2);
    }
    // Once every two iterations, do a flush
    if (do_flush) {
      Compressor::FlushResult result;
      do {
        output_len = compressed.size() - compressed_size;
        output = compressed.data() + compressed_size;
        ASSERT_OK_AND_ASSIGN(result, compressor->Flush(output_len, output));
        ASSERT_LE(result.bytes_written, output_len);
        compressed_size += result.bytes_written;
        if (result.should_retry) {
          compressed.resize(compressed.capacity() * 2);
        }
      } while (result.should_retry);
    }
    do_flush = !do_flush;
  }

  // End the compressed stream
  Compressor::EndResult result;
  do {
    int64_t output_len = compressed.size() - compressed_size;
    uint8_t* output = compressed.data() + compressed_size;
    ASSERT_OK_AND_ASSIGN(result, compressor->End(output_len, output));
    ASSERT_LE(result.bytes_written, output_len);
    compressed_size += result.bytes_written;
    if (result.should_retry) {
      compressed.resize(compressed.capacity() * 2);
    }
  } while (result.should_retry);

  // Check decompressing the compressed data
  std::vector<uint8_t> decompressed(data.size());
  ASSERT_OK(codec->Decompress(compressed_size, compressed.data(), decompressed.size(),
                              decompressed.data()));

  ASSERT_EQ(data, decompressed);
}

// Check the streaming decompressor against one-shot compression

void CheckStreamingDecompressor(Codec* codec, const std::vector<uint8_t>& data) {
  // Create compressed data
  int64_t max_compressed_len = codec->MaxCompressedLen(data.size(), data.data());
  std::vector<uint8_t> compressed(max_compressed_len);
  int64_t compressed_size;
  ASSERT_OK_AND_ASSIGN(
      compressed_size,
      codec->Compress(data.size(), data.data(), max_compressed_len, compressed.data()));
  compressed.resize(compressed_size);

  // Run streaming decompression
  std::shared_ptr<Decompressor> decompressor;
  ASSERT_OK_AND_ASSIGN(decompressor, codec->MakeDecompressor());

  std::vector<uint8_t> decompressed;
  int64_t decompressed_size = 0;
  const uint8_t* input = compressed.data();
  int64_t remaining = compressed.size();

  decompressed.resize(10);
  while (!decompressor->IsFinished()) {
    // Feed a small amount each time
    int64_t input_len = std::min(remaining, static_cast<int64_t>(23));
    int64_t output_len = decompressed.size() - decompressed_size;
    uint8_t* output = decompressed.data() + decompressed_size;
    ASSERT_OK_AND_ASSIGN(auto result,
                         decompressor->Decompress(input_len, input, output_len, output));
    ASSERT_LE(result.bytes_read, input_len);
    ASSERT_LE(result.bytes_written, output_len);
    ASSERT_TRUE(result.need_more_output || result.bytes_written > 0 ||
                result.bytes_read > 0)
        << "Decompression not progressing anymore";
    if (result.need_more_output) {
      decompressed.resize(decompressed.capacity() * 2);
    }
    decompressed_size += result.bytes_written;
    input += result.bytes_read;
    remaining -= result.bytes_read;
  }
  ASSERT_TRUE(decompressor->IsFinished());
  ASSERT_EQ(remaining, 0);

  // Check the decompressed data
  decompressed.resize(decompressed_size);
  ASSERT_EQ(data.size(), decompressed_size);
  ASSERT_EQ(data, decompressed);
}

// Check the streaming compressor and decompressor together

void CheckStreamingRoundtrip(std::shared_ptr<Compressor> compressor,
                             std::shared_ptr<Decompressor> decompressor,
                             const std::vector<uint8_t>& data) {
  std::default_random_engine engine(42);
  std::uniform_int_distribution<int> buf_size_distribution(10, 40);

  auto make_buf_size = [&]() -> int64_t { return buf_size_distribution(engine); };

  // Compress...

  std::vector<uint8_t> compressed(1);
  int64_t compressed_size = 0;
  {
    const uint8_t* input = data.data();
    int64_t remaining = data.size();

    while (remaining > 0) {
      // Feed a varying amount each time
      int64_t input_len = std::min(remaining, make_buf_size());
      int64_t output_len = compressed.size() - compressed_size;
      uint8_t* output = compressed.data() + compressed_size;
      ASSERT_OK_AND_ASSIGN(auto result,
                           compressor->Compress(input_len, input, output_len, output));
      ASSERT_LE(result.bytes_read, input_len);
      ASSERT_LE(result.bytes_written, output_len);
      compressed_size += result.bytes_written;
      input += result.bytes_read;
      remaining -= result.bytes_read;
      if (result.bytes_read == 0) {
        compressed.resize(compressed.capacity() * 2);
      }
    }
    // End the compressed stream
    Compressor::EndResult result;
    do {
      int64_t output_len = compressed.size() - compressed_size;
      uint8_t* output = compressed.data() + compressed_size;
      ASSERT_OK_AND_ASSIGN(result, compressor->End(output_len, output));
      ASSERT_LE(result.bytes_written, output_len);
      compressed_size += result.bytes_written;
      if (result.should_retry) {
        compressed.resize(compressed.capacity() * 2);
      }
    } while (result.should_retry);

    compressed.resize(compressed_size);
  }

  // Then decompress...

  std::vector<uint8_t> decompressed(2);
  int64_t decompressed_size = 0;
  {
    const uint8_t* input = compressed.data();
    int64_t remaining = compressed.size();

    while (!decompressor->IsFinished()) {
      // Feed a varying amount each time
      int64_t input_len = std::min(remaining, make_buf_size());
      int64_t output_len = decompressed.size() - decompressed_size;
      uint8_t* output = decompressed.data() + decompressed_size;
      ASSERT_OK_AND_ASSIGN(
          auto result, decompressor->Decompress(input_len, input, output_len, output));
      ASSERT_LE(result.bytes_read, input_len);
      ASSERT_LE(result.bytes_written, output_len);
      ASSERT_TRUE(result.need_more_output || result.bytes_written > 0 ||
                  result.bytes_read > 0)
          << "Decompression not progressing anymore";
      if (result.need_more_output) {
        decompressed.resize(decompressed.capacity() * 2);
      }
      decompressed_size += result.bytes_written;
      input += result.bytes_read;
      remaining -= result.bytes_read;
    }
    ASSERT_EQ(remaining, 0);
    decompressed.resize(decompressed_size);
  }

  ASSERT_EQ(data.size(), decompressed.size());
  ASSERT_EQ(data, decompressed);
}

void CheckStreamingRoundtrip(Codec* codec, const std::vector<uint8_t>& data) {
  std::shared_ptr<Compressor> compressor;
  std::shared_ptr<Decompressor> decompressor;
  ASSERT_OK_AND_ASSIGN(compressor, codec->MakeCompressor());
  ASSERT_OK_AND_ASSIGN(decompressor, codec->MakeDecompressor());

  CheckStreamingRoundtrip(compressor, decompressor, data);
}

class CodecTest : public ::testing::TestWithParam<Compression::type> {
 protected:
  Compression::type GetCompression() { return GetParam(); }

  std::unique_ptr<Codec> MakeCodec() { return *Codec::Create(GetCompression()); }
};

TEST(TestCodecMisc, GetCodecAsString) {
  ASSERT_EQ("UNCOMPRESSED", Codec::GetCodecAsString(Compression::UNCOMPRESSED));
  ASSERT_EQ("SNAPPY", Codec::GetCodecAsString(Compression::SNAPPY));
  ASSERT_EQ("GZIP", Codec::GetCodecAsString(Compression::GZIP));
  ASSERT_EQ("LZO", Codec::GetCodecAsString(Compression::LZO));
  ASSERT_EQ("BROTLI", Codec::GetCodecAsString(Compression::BROTLI));
  ASSERT_EQ("LZ4_RAW", Codec::GetCodecAsString(Compression::LZ4));
  ASSERT_EQ("LZ4", Codec::GetCodecAsString(Compression::LZ4_FRAME));
  ASSERT_EQ("ZSTD", Codec::GetCodecAsString(Compression::ZSTD));
  ASSERT_EQ("BZ2", Codec::GetCodecAsString(Compression::BZ2));
}

TEST(TestCodecMisc, GetCompressionType) {
  ASSERT_OK_AND_EQ(Compression::UNCOMPRESSED, Codec::GetCompressionType("UNCOMPRESSED"));
  ASSERT_OK_AND_EQ(Compression::SNAPPY, Codec::GetCompressionType("SNAPPY"));
  ASSERT_OK_AND_EQ(Compression::GZIP, Codec::GetCompressionType("GZIP"));
  ASSERT_OK_AND_EQ(Compression::LZO, Codec::GetCompressionType("LZO"));
  ASSERT_OK_AND_EQ(Compression::BROTLI, Codec::GetCompressionType("BROTLI"));
  ASSERT_OK_AND_EQ(Compression::LZ4, Codec::GetCompressionType("LZ4_RAW"));
  ASSERT_OK_AND_EQ(Compression::LZ4_FRAME, Codec::GetCompressionType("LZ4"));
  ASSERT_OK_AND_EQ(Compression::ZSTD, Codec::GetCompressionType("ZSTD"));
  ASSERT_OK_AND_EQ(Compression::BZ2, Codec::GetCompressionType("BZ2"));

  ASSERT_RAISES(Invalid, Codec::GetCompressionType("unk"));
  ASSERT_RAISES(Invalid, Codec::GetCompressionType("snappy"));
}

TEST_P(CodecTest, CodecRoundtrip) {
  const auto compression = GetCompression();
  if (compression == Compression::BZ2) {
    // SKIP: BZ2 doesn't support one-shot compression
    return;
  }

  int sizes[] = {0, 10000, 100000};

  // create multiple compressors to try to break them
  std::unique_ptr<Codec> c1, c2;
  ASSERT_OK_AND_ASSIGN(c1, Codec::Create(compression));
  ASSERT_OK_AND_ASSIGN(c2, Codec::Create(compression));

  for (int data_size : sizes) {
    std::vector<uint8_t> data = MakeRandomData(data_size);
    CheckCodecRoundtrip(c1, c2, data);

    data = MakeCompressibleData(data_size);
    CheckCodecRoundtrip(c1, c2, data);
  }
}

TEST(TestCodecMisc, SpecifyCompressionLevel) {
  struct CombinationOption {
    Compression::type codec;
    int level;
    bool expect_success;
  };
  constexpr CombinationOption combinations[] = {
      {Compression::GZIP, 2, true},     {Compression::BROTLI, 10, true},
      {Compression::ZSTD, 4, true},     {Compression::LZ4, -10, false},
      {Compression::LZO, -22, false},   {Compression::UNCOMPRESSED, 10, false},
      {Compression::SNAPPY, 16, false}, {Compression::GZIP, -992, false}};

  std::vector<uint8_t> data = MakeRandomData(2000);
  for (const auto& combination : combinations) {
    const auto compression = combination.codec;
    if (!Codec::IsAvailable(compression)) {
      // Support for this codec hasn't been built
      continue;
    }
    const auto level = combination.level;
    const auto expect_success = combination.expect_success;
    auto result1 = Codec::Create(compression, level);
    auto result2 = Codec::Create(compression, level);
    ASSERT_EQ(expect_success, result1.ok());
    ASSERT_EQ(expect_success, result2.ok());
    if (expect_success) {
      CheckCodecRoundtrip(*result1, *result2, data);
    }
  }
}

TEST_P(CodecTest, OutputBufferIsSmall) {
  auto type = GetCompression();
  if (type != Compression::SNAPPY) {
    return;
  }

  ASSERT_OK_AND_ASSIGN(auto codec, Codec::Create(type));

  std::vector<uint8_t> data = MakeRandomData(10);
  auto max_compressed_len = codec->MaxCompressedLen(data.size(), data.data());
  std::vector<uint8_t> compressed(max_compressed_len);
  std::vector<uint8_t> decompressed(data.size() - 1);

  int64_t actual_size;
  ASSERT_OK_AND_ASSIGN(
      actual_size,
      codec->Compress(data.size(), data.data(), max_compressed_len, compressed.data()));
  compressed.resize(actual_size);

  std::stringstream ss;
  ss << "Invalid: Output buffer size (" << decompressed.size() << ") must be "
     << data.size() << " or larger.";
  ASSERT_RAISES_WITH_MESSAGE(Invalid, ss.str(),
                             codec->Decompress(compressed.size(), compressed.data(),
                                               decompressed.size(), decompressed.data()));
}

TEST_P(CodecTest, StreamingCompressor) {
  if (GetCompression() == Compression::SNAPPY) {
    // SKIP: snappy doesn't support streaming compression
    return;
  }
  if (GetCompression() == Compression::BZ2) {
    // SKIP: BZ2 doesn't support one-shot decompression
    return;
  }
  if (GetCompression() == Compression::LZ4) {
    // SKIP: LZ4 raw format doesn't support streaming compression.
    return;
  }

  int sizes[] = {0, 10, 100000};
  for (int data_size : sizes) {
    auto codec = MakeCodec();

    std::vector<uint8_t> data = MakeRandomData(data_size);
    CheckStreamingCompressor(codec.get(), data);

    data = MakeCompressibleData(data_size);
    CheckStreamingCompressor(codec.get(), data);
  }
}

TEST_P(CodecTest, StreamingDecompressor) {
  if (GetCompression() == Compression::SNAPPY) {
    // SKIP: snappy doesn't support streaming decompression
    return;
  }
  if (GetCompression() == Compression::BZ2) {
    // SKIP: BZ2 doesn't support one-shot compression
    return;
  }
  if (GetCompression() == Compression::LZ4) {
    // SKIP: LZ4 raw format doesn't support streaming decompression.
    return;
  }

  int sizes[] = {0, 10, 100000};
  for (int data_size : sizes) {
    auto codec = MakeCodec();

    std::vector<uint8_t> data = MakeRandomData(data_size);
    CheckStreamingDecompressor(codec.get(), data);

    data = MakeCompressibleData(data_size);
    CheckStreamingDecompressor(codec.get(), data);
  }
}

TEST_P(CodecTest, StreamingRoundtrip) {
  if (GetCompression() == Compression::SNAPPY) {
    // SKIP: snappy doesn't support streaming decompression
    return;
  }
  if (GetCompression() == Compression::LZ4) {
    // SKIP: LZ4 raw format doesn't support streaming compression.
    return;
  }

  int sizes[] = {0, 10, 100000};
  for (int data_size : sizes) {
    auto codec = MakeCodec();

    std::vector<uint8_t> data = MakeRandomData(data_size);
    CheckStreamingRoundtrip(codec.get(), data);

    data = MakeCompressibleData(data_size);
    CheckStreamingRoundtrip(codec.get(), data);
  }
}

TEST_P(CodecTest, StreamingDecompressorReuse) {
  if (GetCompression() == Compression::SNAPPY) {
    // SKIP: snappy doesn't support streaming decompression
    return;
  }
  if (GetCompression() == Compression::LZ4) {
    // SKIP: LZ4 raw format doesn't support streaming decompression.
    return;
  }

  auto codec = MakeCodec();
  std::shared_ptr<Compressor> compressor;
  std::shared_ptr<Decompressor> decompressor;
  ASSERT_OK_AND_ASSIGN(compressor, codec->MakeCompressor());
  ASSERT_OK_AND_ASSIGN(decompressor, codec->MakeDecompressor());

  std::vector<uint8_t> data = MakeRandomData(100);
  CheckStreamingRoundtrip(compressor, decompressor, data);
  // Decompressor::Reset() should allow reusing decompressor for a new stream
  ASSERT_OK_AND_ASSIGN(compressor, codec->MakeCompressor());
  ASSERT_OK(decompressor->Reset());
  data = MakeRandomData(200);
  CheckStreamingRoundtrip(compressor, decompressor, data);
}

#ifdef ARROW_WITH_ZLIB
INSTANTIATE_TEST_SUITE_P(TestGZip, CodecTest, ::testing::Values(Compression::GZIP));
#endif

#ifdef ARROW_WITH_SNAPPY
INSTANTIATE_TEST_SUITE_P(TestSnappy, CodecTest, ::testing::Values(Compression::SNAPPY));
#endif

#ifdef ARROW_WITH_LZ4
INSTANTIATE_TEST_SUITE_P(TestLZ4, CodecTest, ::testing::Values(Compression::LZ4));
#endif

#ifdef ARROW_WITH_LZ4
INSTANTIATE_TEST_SUITE_P(TestLZ4Frame, CodecTest,
                         ::testing::Values(Compression::LZ4_FRAME));
#endif

#ifdef ARROW_WITH_BROTLI
INSTANTIATE_TEST_SUITE_P(TestBrotli, CodecTest, ::testing::Values(Compression::BROTLI));
#endif

#if ARROW_WITH_BZ2
INSTANTIATE_TEST_SUITE_P(TestBZ2, CodecTest, ::testing::Values(Compression::BZ2));
#endif

#ifdef ARROW_WITH_ZSTD
INSTANTIATE_TEST_SUITE_P(TestZSTD, CodecTest, ::testing::Values(Compression::ZSTD));
#endif

}  // namespace util
}  // namespace arrow
