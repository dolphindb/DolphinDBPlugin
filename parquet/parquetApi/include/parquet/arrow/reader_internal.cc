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

#include "parquet/arrow/reader_internal.h"

#include <algorithm>
#include <climits>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "arrow/array.h"
#include "arrow/builder.h"
#include "arrow/datum.h"
#include "arrow/extension_type.h"
#include "arrow/io/memory.h"
#include "arrow/ipc/reader.h"
#include "arrow/ipc/writer.h"
#include "arrow/scalar.h"
#include "arrow/status.h"
#include "arrow/table.h"
#include "arrow/type.h"
#include "arrow/type_traits.h"
#include "arrow/util/base64.h"
#include "arrow/util/checked_cast.h"
#include "arrow/util/int_util_internal.h"
#include "arrow/util/logging.h"
#include "arrow/util/string_view.h"
#include "arrow/util/ubsan.h"
#include "arrow/visitor_inline.h"
#include "parquet/arrow/reader.h"
#include "parquet/arrow/schema.h"
#include "parquet/arrow/schema_internal.h"
#include "parquet/column_reader.h"
#include "parquet/platform.h"
#include "parquet/properties.h"
#include "parquet/schema.h"
#include "parquet/statistics.h"
#include "parquet/types.h"
// Required after "arrow/util/int_util_internal.h" (for OPTIONAL)
#include "parquet/windows_compatibility.h"

using arrow::Array;
using arrow::BooleanArray;
using arrow::ChunkedArray;
using arrow::DataType;
using arrow::Datum;
using arrow::Field;
using arrow::Int32Array;
using arrow::ListArray;
using arrow::MemoryPool;
using arrow::ResizableBuffer;
using arrow::Status;
using arrow::StructArray;
using arrow::Table;
using arrow::TimestampArray;

using ::arrow::BitUtil::FromBigEndian;
using ::arrow::internal::checked_cast;
using ::arrow::internal::checked_pointer_cast;
using ::arrow::internal::SafeLeftShift;
using ::arrow::util::SafeLoadAs;

using parquet::internal::BinaryRecordReader;
using parquet::internal::DictionaryRecordReader;
using parquet::internal::RecordReader;
using parquet::schema::GroupNode;
using parquet::schema::Node;
using parquet::schema::PrimitiveNode;
using ParquetType = parquet::Type;

namespace parquet {
namespace arrow {

template <typename ArrowType>
using ArrayType = typename ::arrow::TypeTraits<ArrowType>::ArrayType;

template <typename CType, typename StatisticsType>
Status MakeMinMaxScalar(const StatisticsType& statistics,
                        std::shared_ptr<::arrow::Scalar>* min,
                        std::shared_ptr<::arrow::Scalar>* max) {
  *min = ::arrow::MakeScalar(static_cast<CType>(statistics.min()));
  *max = ::arrow::MakeScalar(static_cast<CType>(statistics.max()));
  return Status::OK();
}

template <typename CType, typename StatisticsType>
Status MakeMinMaxTypedScalar(const StatisticsType& statistics,
                             std::shared_ptr<DataType> type,
                             std::shared_ptr<::arrow::Scalar>* min,
                             std::shared_ptr<::arrow::Scalar>* max) {
  ARROW_ASSIGN_OR_RAISE(*min, ::arrow::MakeScalar(type, statistics.min()));
  ARROW_ASSIGN_OR_RAISE(*max, ::arrow::MakeScalar(type, statistics.max()));
  return Status::OK();
}

template <typename StatisticsType>
Status MakeMinMaxIntegralScalar(const StatisticsType& statistics,
                                const ::arrow::DataType& arrow_type,
                                std::shared_ptr<::arrow::Scalar>* min,
                                std::shared_ptr<::arrow::Scalar>* max) {
  const auto column_desc = statistics.descr();
  const auto& logical_type = column_desc->logical_type();
  const auto& integer = checked_pointer_cast<const IntLogicalType>(logical_type);
  const bool is_signed = integer->is_signed();

  switch (integer->bit_width()) {
    case 8:
      return is_signed ? MakeMinMaxScalar<int8_t>(statistics, min, max)
                       : MakeMinMaxScalar<uint8_t>(statistics, min, max);
    case 16:
      return is_signed ? MakeMinMaxScalar<int16_t>(statistics, min, max)
                       : MakeMinMaxScalar<uint16_t>(statistics, min, max);
    case 32:
      return is_signed ? MakeMinMaxScalar<int32_t>(statistics, min, max)
                       : MakeMinMaxScalar<uint32_t>(statistics, min, max);
    case 64:
      return is_signed ? MakeMinMaxScalar<int64_t>(statistics, min, max)
                       : MakeMinMaxScalar<uint64_t>(statistics, min, max);
  }

  return Status::OK();
}

static Status FromInt32Statistics(const Int32Statistics& statistics,
                                  const LogicalType& logical_type,
                                  std::shared_ptr<::arrow::Scalar>* min,
                                  std::shared_ptr<::arrow::Scalar>* max) {
  ARROW_ASSIGN_OR_RAISE(auto type, FromInt32(logical_type));

  switch (logical_type.type()) {
    case LogicalType::Type::INT:
      return MakeMinMaxIntegralScalar(statistics, *type, min, max);
      break;
    case LogicalType::Type::DATE:
    case LogicalType::Type::TIME:
    case LogicalType::Type::NONE:
      return MakeMinMaxTypedScalar<int32_t>(statistics, type, min, max);
      break;
    default:
      break;
  }

  return Status::NotImplemented("Cannot extract statistics for type ");
}

static Status FromInt64Statistics(const Int64Statistics& statistics,
                                  const LogicalType& logical_type,
                                  std::shared_ptr<::arrow::Scalar>* min,
                                  std::shared_ptr<::arrow::Scalar>* max) {
  ARROW_ASSIGN_OR_RAISE(auto type, FromInt64(logical_type));

  switch (logical_type.type()) {
    case LogicalType::Type::INT:
      return MakeMinMaxIntegralScalar(statistics, *type, min, max);
      break;
    case LogicalType::Type::TIME:
    case LogicalType::Type::TIMESTAMP:
    case LogicalType::Type::NONE:
      return MakeMinMaxTypedScalar<int64_t>(statistics, type, min, max);
      break;
    default:
      break;
  }

  return Status::NotImplemented("Cannot extract statistics for type ");
}

static inline Status ByteArrayStatisticsAsScalars(const Statistics& statistics,
                                                  std::shared_ptr<::arrow::Scalar>* min,
                                                  std::shared_ptr<::arrow::Scalar>* max) {
  auto logical_type = statistics.descr()->logical_type();
  auto type = logical_type->type() == LogicalType::Type::STRING ? ::arrow::utf8()
                                                                : ::arrow::binary();

  ARROW_ASSIGN_OR_RAISE(
      *min, ::arrow::MakeScalar(type, Buffer::FromString(statistics.EncodeMin())));
  ARROW_ASSIGN_OR_RAISE(
      *max, ::arrow::MakeScalar(type, Buffer::FromString(statistics.EncodeMax())));

  return Status::OK();
}

Status StatisticsAsScalars(const Statistics& statistics,
                           std::shared_ptr<::arrow::Scalar>* min,
                           std::shared_ptr<::arrow::Scalar>* max) {
  if (!statistics.HasMinMax()) {
    return Status::Invalid("Statistics has no min max.");
  }

  auto column_desc = statistics.descr();
  if (column_desc == nullptr) {
    return Status::Invalid("Statistics carries no descriptor, can't infer arrow type.");
  }

  auto physical_type = column_desc->physical_type();
  auto logical_type = column_desc->logical_type();
  switch (physical_type) {
    case Type::BOOLEAN:
      return MakeMinMaxScalar<bool, BoolStatistics>(
          checked_cast<const BoolStatistics&>(statistics), min, max);
    case Type::FLOAT:
      return MakeMinMaxScalar<float, FloatStatistics>(
          checked_cast<const FloatStatistics&>(statistics), min, max);
    case Type::DOUBLE:
      return MakeMinMaxScalar<double, DoubleStatistics>(
          checked_cast<const DoubleStatistics&>(statistics), min, max);
    case Type::INT32:
      return FromInt32Statistics(checked_cast<const Int32Statistics&>(statistics),
                                 *logical_type, min, max);
    case Type::INT64:
      return FromInt64Statistics(checked_cast<const Int64Statistics&>(statistics),
                                 *logical_type, min, max);
    case Type::BYTE_ARRAY:
      return ByteArrayStatisticsAsScalars(statistics, min, max);
    default:
      return Status::NotImplemented("Extract statistics unsupported for physical_type ",
                                    physical_type, " unsupported.");
  }

  return Status::OK();
}

// ----------------------------------------------------------------------
// Primitive types

template <typename ArrowType, typename ParquetType>
Status TransferInt(RecordReader* reader, MemoryPool* pool,
                   const std::shared_ptr<DataType>& type, Datum* out) {
  using ArrowCType = typename ArrowType::c_type;
  using ParquetCType = typename ParquetType::c_type;
  int64_t length = reader->values_written();
  ARROW_ASSIGN_OR_RAISE(auto data,
                        ::arrow::AllocateBuffer(length * sizeof(ArrowCType), pool));

  auto values = reinterpret_cast<const ParquetCType*>(reader->values());
  auto out_ptr = reinterpret_cast<ArrowCType*>(data->mutable_data());
  std::copy(values, values + length, out_ptr);
  *out = std::make_shared<ArrayType<ArrowType>>(
      type, length, std::move(data), reader->ReleaseIsValid(), reader->null_count());
  return Status::OK();
}

std::shared_ptr<Array> TransferZeroCopy(RecordReader* reader,
                                        const std::shared_ptr<DataType>& type) {
  std::vector<std::shared_ptr<Buffer>> buffers = {reader->ReleaseIsValid(),
                                                  reader->ReleaseValues()};
  auto data = std::make_shared<::arrow::ArrayData>(type, reader->values_written(),
                                                   buffers, reader->null_count());
  return ::arrow::MakeArray(data);
}

Status TransferBool(RecordReader* reader, MemoryPool* pool, Datum* out) {
  int64_t length = reader->values_written();

  const int64_t buffer_size = BitUtil::BytesForBits(length);
  ARROW_ASSIGN_OR_RAISE(auto data, ::arrow::AllocateBuffer(buffer_size, pool));

  // Transfer boolean values to packed bitmap
  auto values = reinterpret_cast<const bool*>(reader->values());
  uint8_t* data_ptr = data->mutable_data();
  memset(data_ptr, 0, buffer_size);

  for (int64_t i = 0; i < length; i++) {
    if (values[i]) {
      ::arrow::BitUtil::SetBit(data_ptr, i);
    }
  }

  *out = std::make_shared<BooleanArray>(length, std::move(data), reader->ReleaseIsValid(),
                                        reader->null_count());
  return Status::OK();
}

Status TransferInt96(RecordReader* reader, MemoryPool* pool,
                     const std::shared_ptr<DataType>& type, Datum* out) {
  int64_t length = reader->values_written();
  auto values = reinterpret_cast<const Int96*>(reader->values());
  ARROW_ASSIGN_OR_RAISE(auto data,
                        ::arrow::AllocateBuffer(length * sizeof(int64_t), pool));
  auto data_ptr = reinterpret_cast<int64_t*>(data->mutable_data());
  for (int64_t i = 0; i < length; i++) {
    if (values[i].value[2] == 0) {
      // Happens for null entries: avoid triggering UBSAN as that Int96 timestamp
      // isn't representable as a 64-bit Unix timestamp.
      *data_ptr++ = 0;
    } else {
      *data_ptr++ = Int96GetNanoSeconds(values[i]);
    }
  }
  *out = std::make_shared<TimestampArray>(type, length, std::move(data),
                                          reader->ReleaseIsValid(), reader->null_count());
  return Status::OK();
}

Status TransferDate64(RecordReader* reader, MemoryPool* pool,
                      const std::shared_ptr<DataType>& type, Datum* out) {
  int64_t length = reader->values_written();
  auto values = reinterpret_cast<const int32_t*>(reader->values());

  ARROW_ASSIGN_OR_RAISE(auto data,
                        ::arrow::AllocateBuffer(length * sizeof(int64_t), pool));
  auto out_ptr = reinterpret_cast<int64_t*>(data->mutable_data());

  for (int64_t i = 0; i < length; i++) {
    *out_ptr++ = static_cast<int64_t>(values[i]) * kMillisecondsPerDay;
  }

  *out = std::make_shared<::arrow::Date64Array>(
      type, length, std::move(data), reader->ReleaseIsValid(), reader->null_count());
  return Status::OK();
}

// ----------------------------------------------------------------------
// Binary, direct to dictionary-encoded

Status TransferDictionary(RecordReader* reader,
                          const std::shared_ptr<DataType>& logical_value_type,
                          std::shared_ptr<ChunkedArray>* out) {
  auto dict_reader = dynamic_cast<DictionaryRecordReader*>(reader);
  DCHECK(dict_reader);
  *out = dict_reader->GetResult();
  if (!logical_value_type->Equals(*(*out)->type())) {
    ARROW_ASSIGN_OR_RAISE(*out, (*out)->View(logical_value_type));
  }
  return Status::OK();
}

Status TransferBinary(RecordReader* reader,
                      const std::shared_ptr<DataType>& logical_value_type,
                      std::shared_ptr<ChunkedArray>* out) {
  if (reader->read_dictionary()) {
    return TransferDictionary(
        reader, ::arrow::dictionary(::arrow::int32(), logical_value_type), out);
  }
  auto binary_reader = dynamic_cast<BinaryRecordReader*>(reader);
  DCHECK(binary_reader);
  auto chunks = binary_reader->GetBuilderChunks();
  for (const auto& chunk : chunks) {
    if (!chunk->type()->Equals(*logical_value_type)) {
      ARROW_ASSIGN_OR_RAISE(*out, ChunkedArray(chunks).View(logical_value_type));
      return Status::OK();
    }
  }
  *out = std::make_shared<ChunkedArray>(chunks, logical_value_type);
  return Status::OK();
}

// ----------------------------------------------------------------------
// INT32 / INT64 / BYTE_ARRAY / FIXED_LEN_BYTE_ARRAY -> Decimal128

static uint64_t BytesToInteger(const uint8_t* bytes, int32_t start, int32_t stop) {
  const int32_t length = stop - start;

  DCHECK_GE(length, 0);
  DCHECK_LE(length, 8);

  switch (length) {
    case 0:
      return 0;
    case 1:
      return bytes[start];
    case 2:
      return FromBigEndian(SafeLoadAs<uint16_t>(bytes + start));
    case 3: {
      const uint64_t first_two_bytes = FromBigEndian(SafeLoadAs<uint16_t>(bytes + start));
      const uint64_t last_byte = bytes[stop - 1];
      return first_two_bytes << 8 | last_byte;
    }
    case 4:
      return FromBigEndian(SafeLoadAs<uint32_t>(bytes + start));
    case 5: {
      const uint64_t first_four_bytes =
          FromBigEndian(SafeLoadAs<uint32_t>(bytes + start));
      const uint64_t last_byte = bytes[stop - 1];
      return first_four_bytes << 8 | last_byte;
    }
    case 6: {
      const uint64_t first_four_bytes =
          FromBigEndian(SafeLoadAs<uint32_t>(bytes + start));
      const uint64_t last_two_bytes =
          FromBigEndian(SafeLoadAs<uint16_t>(bytes + start + 4));
      return first_four_bytes << 16 | last_two_bytes;
    }
    case 7: {
      const uint64_t first_four_bytes =
          FromBigEndian(SafeLoadAs<uint32_t>(bytes + start));
      const uint64_t second_two_bytes =
          FromBigEndian(SafeLoadAs<uint16_t>(bytes + start + 4));
      const uint64_t last_byte = bytes[stop - 1];
      return first_four_bytes << 24 | second_two_bytes << 8 | last_byte;
    }
    case 8:
      return FromBigEndian(SafeLoadAs<uint64_t>(bytes + start));
    default: {
      DCHECK(false);
      return UINT64_MAX;
    }
  }
}

static constexpr int32_t kMinDecimalBytes = 1;
static constexpr int32_t kMaxDecimalBytes = 16;

/// \brief Convert a sequence of big-endian bytes to one int64_t (high bits) and one
/// uint64_t (low bits).
static void BytesToIntegerPair(const uint8_t* bytes, const int32_t length,
                               int64_t* out_high, uint64_t* out_low) {
  DCHECK_GE(length, kMinDecimalBytes);
  DCHECK_LE(length, kMaxDecimalBytes);

  // XXX This code is copied from Decimal::FromBigEndian

  int64_t high, low;

  // Bytes are coming in big-endian, so the first byte is the MSB and therefore holds the
  // sign bit.
  const bool is_negative = static_cast<int8_t>(bytes[0]) < 0;

  // 1. Extract the high bytes
  // Stop byte of the high bytes
  const int32_t high_bits_offset = std::max(0, length - 8);
  const auto high_bits = BytesToInteger(bytes, 0, high_bits_offset);

  if (high_bits_offset == 8) {
    // Avoid undefined shift by 64 below
    high = high_bits;
  } else {
    high = -1 * (is_negative && length < kMaxDecimalBytes);
    // Shift left enough bits to make room for the incoming int64_t
    high = SafeLeftShift(high, high_bits_offset * CHAR_BIT);
    // Preserve the upper bits by inplace OR-ing the int64_t
    high |= high_bits;
  }

  // 2. Extract the low bytes
  // Stop byte of the low bytes
  const int32_t low_bits_offset = std::min(length, 8);
  const auto low_bits = BytesToInteger(bytes, high_bits_offset, length);

  if (low_bits_offset == 8) {
    // Avoid undefined shift by 64 below
    low = low_bits;
  } else {
    // Sign extend the low bits if necessary
    low = -1 * (is_negative && length < 8);
    // Shift left enough bits to make room for the incoming int64_t
    low = SafeLeftShift(low, low_bits_offset * CHAR_BIT);
    // Preserve the upper bits by inplace OR-ing the int64_t
    low |= low_bits;
  }

  *out_high = high;
  *out_low = static_cast<uint64_t>(low);
}

static inline void RawBytesToDecimalBytes(const uint8_t* value, int32_t byte_width,
                                          uint8_t* out_buf) {
  // view the first 8 bytes as an unsigned 64-bit integer
  auto low = reinterpret_cast<uint64_t*>(out_buf);

  // view the second 8 bytes as a signed 64-bit integer
  auto high = reinterpret_cast<int64_t*>(out_buf + sizeof(uint64_t));

  // Convert the fixed size binary array bytes into a Decimal128 compatible layout
  BytesToIntegerPair(value, byte_width, high, low);
}

template <typename T>
Status ConvertToDecimal128(const Array& array, const std::shared_ptr<DataType>&,
                           MemoryPool* pool, std::shared_ptr<Array>*) {
  return Status::NotImplemented("not implemented");
}

template <>
Status ConvertToDecimal128<FLBAType>(const Array& array,
                                     const std::shared_ptr<DataType>& type,
                                     MemoryPool* pool, std::shared_ptr<Array>* out) {
  const auto& fixed_size_binary_array =
      static_cast<const ::arrow::FixedSizeBinaryArray&>(array);

  // The byte width of each decimal value
  const int32_t type_length =
      static_cast<const ::arrow::Decimal128Type&>(*type).byte_width();

  // number of elements in the entire array
  const int64_t length = fixed_size_binary_array.length();

  // Get the byte width of the values in the FixedSizeBinaryArray. Most of the time
  // this will be different from the decimal array width because we write the minimum
  // number of bytes necessary to represent a given precision
  const int32_t byte_width =
      static_cast<const ::arrow::FixedSizeBinaryType&>(*fixed_size_binary_array.type())
          .byte_width();
  if (byte_width < kMinDecimalBytes || byte_width > kMaxDecimalBytes) {
    return Status::Invalid("Invalid FIXED_LEN_BYTE_ARRAY length for Decimal128");
  }

  // allocate memory for the decimal array
  ARROW_ASSIGN_OR_RAISE(auto data, ::arrow::AllocateBuffer(length * type_length, pool));

  // raw bytes that we can write to
  uint8_t* out_ptr = data->mutable_data();

  // convert each FixedSizeBinary value to valid decimal bytes
  const int64_t null_count = fixed_size_binary_array.null_count();
  if (null_count > 0) {
    for (int64_t i = 0; i < length; ++i, out_ptr += type_length) {
      if (!fixed_size_binary_array.IsNull(i)) {
        RawBytesToDecimalBytes(fixed_size_binary_array.GetValue(i), byte_width, out_ptr);
      }
    }
  } else {
    for (int64_t i = 0; i < length; ++i, out_ptr += type_length) {
      RawBytesToDecimalBytes(fixed_size_binary_array.GetValue(i), byte_width, out_ptr);
    }
  }

  *out = std::make_shared<::arrow::Decimal128Array>(
      type, length, std::move(data), fixed_size_binary_array.null_bitmap(), null_count);

  return Status::OK();
}

template <>
Status ConvertToDecimal128<ByteArrayType>(const Array& array,
                                          const std::shared_ptr<DataType>& type,
                                          MemoryPool* pool, std::shared_ptr<Array>* out) {
  const auto& binary_array = static_cast<const ::arrow::BinaryArray&>(array);
  const int64_t length = binary_array.length();

  const auto& decimal_type = static_cast<const ::arrow::Decimal128Type&>(*type);
  const int64_t type_length = decimal_type.byte_width();

  ARROW_ASSIGN_OR_RAISE(auto data, ::arrow::AllocateBuffer(length * type_length, pool));

  // raw bytes that we can write to
  uint8_t* out_ptr = data->mutable_data();

  const int64_t null_count = binary_array.null_count();

  // convert each BinaryArray value to valid decimal bytes
  for (int64_t i = 0; i < length; i++, out_ptr += type_length) {
    int32_t record_len = 0;
    const uint8_t* record_loc = binary_array.GetValue(i, &record_len);

    if (record_len < 0 || record_len > type_length) {
      return Status::Invalid("Invalid BYTE_ARRAY length for Decimal128");
    }

    auto out_ptr_view = reinterpret_cast<uint64_t*>(out_ptr);
    out_ptr_view[0] = 0;
    out_ptr_view[1] = 0;

    // only convert rows that are not null if there are nulls, or
    // all rows, if there are not
    if ((null_count > 0 && !binary_array.IsNull(i)) || null_count <= 0) {
      if (record_len <= 0) {
        return Status::Invalid("Invalid BYTE_ARRAY length for Decimal128");
      }
      RawBytesToDecimalBytes(record_loc, record_len, out_ptr);
    }
  }

  *out = std::make_shared<::arrow::Decimal128Array>(
      type, length, std::move(data), binary_array.null_bitmap(), null_count);
  return Status::OK();
}

/// \brief Convert an Int32 or Int64 array into a Decimal128Array
/// The parquet spec allows systems to write decimals in int32, int64 if the values are
/// small enough to fit in less 4 bytes or less than 8 bytes, respectively.
/// This function implements the conversion from int32 and int64 arrays to decimal arrays.
template <
    typename ParquetIntegerType,
    typename = ::arrow::enable_if_t<std::is_same<ParquetIntegerType, Int32Type>::value ||
                                    std::is_same<ParquetIntegerType, Int64Type>::value>>
static Status DecimalIntegerTransfer(RecordReader* reader, MemoryPool* pool,
                                     const std::shared_ptr<DataType>& type, Datum* out) {
  DCHECK_EQ(type->id(), ::arrow::Type::DECIMAL);

  const int64_t length = reader->values_written();

  using ElementType = typename ParquetIntegerType::c_type;
  static_assert(std::is_same<ElementType, int32_t>::value ||
                    std::is_same<ElementType, int64_t>::value,
                "ElementType must be int32_t or int64_t");

  const auto values = reinterpret_cast<const ElementType*>(reader->values());

  const auto& decimal_type = static_cast<const ::arrow::Decimal128Type&>(*type);
  const int64_t type_length = decimal_type.byte_width();

  ARROW_ASSIGN_OR_RAISE(auto data, ::arrow::AllocateBuffer(length * type_length, pool));
  uint8_t* out_ptr = data->mutable_data();

  using ::arrow::BitUtil::FromLittleEndian;

  for (int64_t i = 0; i < length; ++i, out_ptr += type_length) {
    // sign/zero extend int32_t values, otherwise a no-op
    const auto value = static_cast<int64_t>(values[i]);

    auto out_ptr_view = reinterpret_cast<uint64_t*>(out_ptr);

    // No-op on little endian machines, byteswap on big endian
    out_ptr_view[0] = FromLittleEndian(static_cast<uint64_t>(value));

    // no need to byteswap here because we're sign/zero extending exactly 8 bytes
    out_ptr_view[1] = static_cast<uint64_t>(value < 0 ? -1 : 0);
  }

  if (reader->nullable_values()) {
    std::shared_ptr<ResizableBuffer> is_valid = reader->ReleaseIsValid();
    *out = std::make_shared<::arrow::Decimal128Array>(type, length, std::move(data),
                                                      is_valid, reader->null_count());
  } else {
    *out = std::make_shared<::arrow::Decimal128Array>(type, length, std::move(data));
  }
  return Status::OK();
}

/// \brief Convert an arrow::BinaryArray to an arrow::Decimal128Array
/// We do this by:
/// 1. Creating an arrow::BinaryArray from the RecordReader's builder
/// 2. Allocating a buffer for the arrow::Decimal128Array
/// 3. Converting the big-endian bytes in each BinaryArray entry to two integers
///    representing the high and low bits of each decimal value.
template <typename ParquetType>
Status TransferDecimal(RecordReader* reader, MemoryPool* pool,
                       const std::shared_ptr<DataType>& type, Datum* out) {
  DCHECK_EQ(type->id(), ::arrow::Type::DECIMAL);

  auto binary_reader = dynamic_cast<BinaryRecordReader*>(reader);
  DCHECK(binary_reader);
  ::arrow::ArrayVector chunks = binary_reader->GetBuilderChunks();
  for (size_t i = 0; i < chunks.size(); ++i) {
    std::shared_ptr<Array> chunk_as_decimal;
    RETURN_NOT_OK(
        ConvertToDecimal128<ParquetType>(*chunks[i], type, pool, &chunk_as_decimal));
    // Replace the chunk, which will hopefully also free memory as we go
    chunks[i] = chunk_as_decimal;
  }
  *out = std::make_shared<ChunkedArray>(chunks, type);
  return Status::OK();
}

Status TransferExtension(RecordReader* reader, std::shared_ptr<DataType> value_type,
                         const ColumnDescriptor* descr, MemoryPool* pool, Datum* out) {
  std::shared_ptr<ChunkedArray> result;
  auto ext_type = std::static_pointer_cast<::arrow::ExtensionType>(value_type);
  auto storage_type = ext_type->storage_type();
  RETURN_NOT_OK(TransferColumnData(reader, storage_type, descr, pool, &result));

  ::arrow::ArrayVector out_chunks(result->num_chunks());
  for (int i = 0; i < result->num_chunks(); i++) {
    auto chunk = result->chunk(i);
    auto ext_data = chunk->data()->Copy();
    ext_data->type = ext_type;
    auto ext_result = ext_type->MakeArray(ext_data);
    out_chunks[i] = ext_result;
  }
  *out = std::make_shared<ChunkedArray>(out_chunks);
  return Status::OK();
}

#define TRANSFER_INT32(ENUM, ArrowType)                                              \
  case ::arrow::Type::ENUM: {                                                        \
    Status s = TransferInt<ArrowType, Int32Type>(reader, pool, value_type, &result); \
    RETURN_NOT_OK(s);                                                                \
  } break;

#define TRANSFER_INT64(ENUM, ArrowType)                                              \
  case ::arrow::Type::ENUM: {                                                        \
    Status s = TransferInt<ArrowType, Int64Type>(reader, pool, value_type, &result); \
    RETURN_NOT_OK(s);                                                                \
  } break;

Status TransferColumnData(RecordReader* reader, std::shared_ptr<DataType> value_type,
                          const ColumnDescriptor* descr, MemoryPool* pool,
                          std::shared_ptr<ChunkedArray>* out) {
  Datum result;
  std::shared_ptr<ChunkedArray> chunked_result;
  switch (value_type->id()) {
    case ::arrow::Type::DICTIONARY: {
      RETURN_NOT_OK(TransferDictionary(reader, value_type, &chunked_result));
      result = chunked_result;
    } break;
    case ::arrow::Type::NA: {
      result = std::make_shared<::arrow::NullArray>(reader->values_written());
      break;
    }
    case ::arrow::Type::INT32:
    case ::arrow::Type::INT64:
    case ::arrow::Type::FLOAT:
    case ::arrow::Type::DOUBLE:
      result = TransferZeroCopy(reader, value_type);
      break;
    case ::arrow::Type::BOOL:
      RETURN_NOT_OK(TransferBool(reader, pool, &result));
      break;
      TRANSFER_INT32(UINT8, ::arrow::UInt8Type);
      TRANSFER_INT32(INT8, ::arrow::Int8Type);
      TRANSFER_INT32(UINT16, ::arrow::UInt16Type);
      TRANSFER_INT32(INT16, ::arrow::Int16Type);
      TRANSFER_INT32(UINT32, ::arrow::UInt32Type);
      TRANSFER_INT64(UINT64, ::arrow::UInt64Type);
      TRANSFER_INT32(DATE32, ::arrow::Date32Type);
      TRANSFER_INT32(TIME32, ::arrow::Time32Type);
      TRANSFER_INT64(TIME64, ::arrow::Time64Type);
    case ::arrow::Type::DATE64:
      RETURN_NOT_OK(TransferDate64(reader, pool, value_type, &result));
      break;
    case ::arrow::Type::FIXED_SIZE_BINARY:
    case ::arrow::Type::BINARY:
    case ::arrow::Type::STRING: {
      RETURN_NOT_OK(TransferBinary(reader, value_type, &chunked_result));
      result = chunked_result;
    } break;
    case ::arrow::Type::DECIMAL: {
      switch (descr->physical_type()) {
        case ::parquet::Type::INT32: {
          RETURN_NOT_OK(
              DecimalIntegerTransfer<Int32Type>(reader, pool, value_type, &result));
        } break;
        case ::parquet::Type::INT64: {
          RETURN_NOT_OK(
              DecimalIntegerTransfer<Int64Type>(reader, pool, value_type, &result));
        } break;
        case ::parquet::Type::BYTE_ARRAY: {
          RETURN_NOT_OK(
              TransferDecimal<ByteArrayType>(reader, pool, value_type, &result));
        } break;
        case ::parquet::Type::FIXED_LEN_BYTE_ARRAY: {
          RETURN_NOT_OK(TransferDecimal<FLBAType>(reader, pool, value_type, &result));
        } break;
        default:
          return Status::Invalid(
              "Physical type for decimal must be int32, int64, byte array, or fixed "
              "length binary");
      }
    } break;
    case ::arrow::Type::TIMESTAMP: {
      const ::arrow::TimestampType& timestamp_type =
          static_cast<::arrow::TimestampType&>(*value_type);
      switch (timestamp_type.unit()) {
        case ::arrow::TimeUnit::MILLI:
        case ::arrow::TimeUnit::MICRO: {
          result = TransferZeroCopy(reader, value_type);
        } break;
        case ::arrow::TimeUnit::NANO: {
          if (descr->physical_type() == ::parquet::Type::INT96) {
            RETURN_NOT_OK(TransferInt96(reader, pool, value_type, &result));
          } else {
            result = TransferZeroCopy(reader, value_type);
          }
        } break;
        default:
          return Status::NotImplemented("TimeUnit not supported");
      }
    } break;
    case ::arrow::Type::EXTENSION: {
      RETURN_NOT_OK(TransferExtension(reader, value_type, descr, pool, &result));
    } break;
    default:
      return Status::NotImplemented("No support for reading columns of type ",
                                    value_type->ToString());
  }

  if (result.kind() == Datum::ARRAY) {
    *out = std::make_shared<ChunkedArray>(result.make_array());
  } else if (result.kind() == Datum::CHUNKED_ARRAY) {
    *out = result.chunked_array();
  } else {
    DCHECK(false) << "Should be impossible, result was " << result.ToString();
  }

  return Status::OK();
}

Status ReconstructNestedList(const std::shared_ptr<Array>& arr,
                             std::shared_ptr<Field> field, int16_t max_def_level,
                             int16_t max_rep_level, const int16_t* def_levels,
                             const int16_t* rep_levels, int64_t total_levels,
                             ::arrow::MemoryPool* pool, std::shared_ptr<Array>* out) {
  // Walk downwards to extract nullability
  std::vector<std::string> item_names;
  std::vector<bool> nullable;
  std::vector<std::shared_ptr<const ::arrow::KeyValueMetadata>> field_metadata;
  std::vector<std::shared_ptr<::arrow::Int32Builder>> offset_builders;
  std::vector<std::shared_ptr<::arrow::BooleanBuilder>> valid_bits_builders;
  nullable.push_back(field->nullable());
  while (field->type()->num_fields() > 0) {
    if (field->type()->num_fields() > 1) {
      return Status::NotImplemented("Fields with more than one child are not supported.");
    } else {
      if (field->type()->id() != ::arrow::Type::LIST) {
        return Status::NotImplemented("Currently only nesting with Lists is supported.");
      }
      field = field->type()->field(0);
    }
    item_names.push_back(field->name());
    offset_builders.emplace_back(
        std::make_shared<::arrow::Int32Builder>(::arrow::int32(), pool));
    valid_bits_builders.emplace_back(
        std::make_shared<::arrow::BooleanBuilder>(::arrow::boolean(), pool));
    nullable.push_back(field->nullable());
    field_metadata.push_back(field->metadata());
  }

  int64_t list_depth = offset_builders.size();
  // This describes the minimal definition that describes a level that
  // reflects a value in the primitive values array.
  int16_t values_def_level = max_def_level;
  if (nullable[nullable.size() - 1]) {
    values_def_level--;
  }

  // The definition levels that are needed so that a list is declared
  // as empty and not null.
  std::vector<int16_t> empty_def_level(list_depth);
  int def_level = 0;
  for (int i = 0; i < list_depth; i++) {
    if (nullable[i]) {
      def_level++;
    }
    empty_def_level[i] = static_cast<int16_t>(def_level);
    def_level++;
  }

  int32_t values_offset = 0;
  std::vector<int64_t> null_counts(list_depth, 0);
  for (int64_t i = 0; i < total_levels; i++) {
    int16_t rep_level = rep_levels[i];
    if (rep_level < max_rep_level) {
      for (int64_t j = rep_level; j < list_depth; j++) {
        if (j == (list_depth - 1)) {
          RETURN_NOT_OK(offset_builders[j]->Append(values_offset));
        } else {
          RETURN_NOT_OK(offset_builders[j]->Append(
              static_cast<int32_t>(offset_builders[j + 1]->length())));
        }

        if (((empty_def_level[j] - 1) == def_levels[i]) && (nullable[j])) {
          RETURN_NOT_OK(valid_bits_builders[j]->Append(false));
          null_counts[j]++;
          break;
        } else {
          RETURN_NOT_OK(valid_bits_builders[j]->Append(true));
          if (empty_def_level[j] == def_levels[i]) {
            break;
          }
        }
      }
    }
    if (def_levels[i] >= values_def_level) {
      values_offset++;
    }
  }
  // Add the final offset to all lists
  for (int64_t j = 0; j < list_depth; j++) {
    if (j == (list_depth - 1)) {
      RETURN_NOT_OK(offset_builders[j]->Append(values_offset));
    } else {
      RETURN_NOT_OK(offset_builders[j]->Append(
          static_cast<int32_t>(offset_builders[j + 1]->length())));
    }
  }

  std::vector<std::shared_ptr<Buffer>> offsets;
  std::vector<std::shared_ptr<Buffer>> valid_bits;
  std::vector<int64_t> list_lengths;
  for (int64_t j = 0; j < list_depth; j++) {
    list_lengths.push_back(offset_builders[j]->length() - 1);
    std::shared_ptr<Array> array;
    RETURN_NOT_OK(offset_builders[j]->Finish(&array));
    offsets.emplace_back(std::static_pointer_cast<Int32Array>(array)->values());
    RETURN_NOT_OK(valid_bits_builders[j]->Finish(&array));
    valid_bits.emplace_back(std::static_pointer_cast<BooleanArray>(array)->values());
  }

  *out = arr;

  // TODO(wesm): Use passed-in field
  for (int64_t j = list_depth - 1; j >= 0; j--) {
    auto list_type = ::arrow::list(::arrow::field(item_names[j], (*out)->type(),
                                                  nullable[j + 1], field_metadata[j]));
    *out = std::make_shared<::arrow::ListArray>(list_type, list_lengths[j], offsets[j],
                                                *out, valid_bits[j], null_counts[j]);
  }
  return Status::OK();
}

}  // namespace arrow
}  // namespace parquet
