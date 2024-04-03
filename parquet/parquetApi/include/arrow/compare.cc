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

// Functions for comparing Arrow data structures

#include "arrow/compare.h"

#include <climits>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "arrow/array.h"
#include "arrow/array/diff.h"
#include "arrow/buffer.h"
#include "arrow/scalar.h"
#include "arrow/sparse_tensor.h"
#include "arrow/status.h"
#include "arrow/tensor.h"
#include "arrow/type.h"
#include "arrow/type_traits.h"
#include "arrow/util/bit_util.h"
#include "arrow/util/bitmap_ops.h"
#include "arrow/util/checked_cast.h"
#include "arrow/util/logging.h"
#include "arrow/util/macros.h"
#include "arrow/util/memory.h"
#include "arrow/visitor_inline.h"

namespace arrow {

using internal::BitmapEquals;
using internal::checked_cast;

// ----------------------------------------------------------------------
// Public method implementations

namespace {

// These helper functions assume we already checked the arrays have equal
// sizes and null bitmaps.

template <typename ArrowType, typename EqualityFunc>
inline bool BaseFloatingEquals(const NumericArray<ArrowType>& left,
                               const NumericArray<ArrowType>& right,
                               EqualityFunc&& equals) {
  using T = typename ArrowType::c_type;

  const T* left_data = left.raw_values();
  const T* right_data = right.raw_values();

  if (left.null_count() > 0) {
    for (int64_t i = 0; i < left.length(); ++i) {
      if (left.IsNull(i)) continue;
      if (!equals(left_data[i], right_data[i])) {
        return false;
      }
    }
  } else {
    for (int64_t i = 0; i < left.length(); ++i) {
      if (!equals(left_data[i], right_data[i])) {
        return false;
      }
    }
  }
  return true;
}

template <typename ArrowType>
inline bool FloatingEquals(const NumericArray<ArrowType>& left,
                           const NumericArray<ArrowType>& right,
                           const EqualOptions& opts) {
  using T = typename ArrowType::c_type;

  if (opts.nans_equal()) {
    return BaseFloatingEquals<ArrowType>(left, right, [](T x, T y) -> bool {
      return (x == y) || (std::isnan(x) && std::isnan(y));
    });
  } else {
    return BaseFloatingEquals<ArrowType>(left, right,
                                         [](T x, T y) -> bool { return x == y; });
  }
}

template <typename ArrowType>
inline bool FloatingApproxEquals(const NumericArray<ArrowType>& left,
                                 const NumericArray<ArrowType>& right,
                                 const EqualOptions& opts) {
  using T = typename ArrowType::c_type;
  const T epsilon = static_cast<T>(opts.atol());

  if (opts.nans_equal()) {
    return BaseFloatingEquals<ArrowType>(left, right, [epsilon](T x, T y) -> bool {
      return (fabs(x - y) <= epsilon) || (x == y) || (std::isnan(x) && std::isnan(y));
    });
  } else {
    return BaseFloatingEquals<ArrowType>(left, right, [epsilon](T x, T y) -> bool {
      return (fabs(x - y) <= epsilon) || (x == y);
    });
  }
}

// RangeEqualsVisitor assumes the range sizes are equal

class RangeEqualsVisitor {
 public:
  RangeEqualsVisitor(const Array& right, int64_t left_start_idx, int64_t left_end_idx,
                     int64_t right_start_idx)
      : right_(right),
        left_start_idx_(left_start_idx),
        left_end_idx_(left_end_idx),
        right_start_idx_(right_start_idx),
        result_(false) {}

  template <typename ArrayType>
  inline Status CompareValues(const ArrayType& left) {
    const auto& right = checked_cast<const ArrayType&>(right_);

    for (int64_t i = left_start_idx_, o_i = right_start_idx_; i < left_end_idx_;
         ++i, ++o_i) {
      const bool is_null = left.IsNull(i);
      if (is_null != right.IsNull(o_i) ||
          (!is_null && left.Value(i) != right.Value(o_i))) {
        result_ = false;
        return Status::OK();
      }
    }
    result_ = true;
    return Status::OK();
  }

  template <typename ArrayType, typename CompareValuesFunc>
  bool CompareWithOffsets(const ArrayType& left,
                          CompareValuesFunc&& compare_values) const {
    const auto& right = checked_cast<const ArrayType&>(right_);

    for (int64_t i = left_start_idx_, o_i = right_start_idx_; i < left_end_idx_;
         ++i, ++o_i) {
      const bool is_null = left.IsNull(i);
      if (is_null != right.IsNull(o_i)) {
        return false;
      }
      if (is_null) continue;
      const auto begin_offset = left.value_offset(i);
      const auto end_offset = left.value_offset(i + 1);
      const auto right_begin_offset = right.value_offset(o_i);
      const auto right_end_offset = right.value_offset(o_i + 1);
      // Underlying can't be equal if the size isn't equal
      if (end_offset - begin_offset != right_end_offset - right_begin_offset) {
        return false;
      }

      if (!compare_values(left, right, begin_offset, right_begin_offset,
                          end_offset - begin_offset)) {
        return false;
      }
    }
    return true;
  }

  template <typename BinaryArrayType>
  bool CompareBinaryRange(const BinaryArrayType& left) const {
    using offset_type = typename BinaryArrayType::offset_type;

    auto compare_values = [](const BinaryArrayType& left, const BinaryArrayType& right,
                             offset_type left_offset, offset_type right_offset,
                             offset_type nvalues) {
      if (nvalues == 0) {
        return true;
      }
      return std::memcmp(left.value_data()->data() + left_offset,
                         right.value_data()->data() + right_offset,
                         static_cast<size_t>(nvalues)) == 0;
    };
    return CompareWithOffsets(left, compare_values);
  }

  template <typename ListArrayType>
  bool CompareLists(const ListArrayType& left) {
    using offset_type = typename ListArrayType::offset_type;
    const auto& right = checked_cast<const ListArrayType&>(right_);
    const std::shared_ptr<Array>& left_values = left.values();
    const std::shared_ptr<Array>& right_values = right.values();

    auto compare_values = [&](const ListArrayType& left, const ListArrayType& right,
                              offset_type left_offset, offset_type right_offset,
                              offset_type nvalues) {
      if (nvalues == 0) {
        return true;
      }
      return left_values->RangeEquals(left_offset, left_offset + nvalues, right_offset,
                                      right_values);
    };
    return CompareWithOffsets(left, compare_values);
  }

  bool CompareMaps(const MapArray& left) {
    // We need a specific comparison helper for maps to avoid comparing
    // struct field names (which are indifferent for maps)
    using offset_type = typename MapArray::offset_type;
    const auto& right = checked_cast<const MapArray&>(right_);
    const auto left_keys = left.keys();
    const auto left_items = left.items();
    const auto right_keys = right.keys();
    const auto right_items = right.items();

    auto compare_values = [&](const MapArray& left, const MapArray& right,
                              offset_type left_offset, offset_type right_offset,
                              offset_type nvalues) {
      if (nvalues == 0) {
        return true;
      }
      return left_keys->RangeEquals(left_offset, left_offset + nvalues, right_offset,
                                    right_keys) &&
             left_items->RangeEquals(left_offset, left_offset + nvalues, right_offset,
                                     right_items);
    };
    return CompareWithOffsets(left, compare_values);
  }

  bool CompareStructs(const StructArray& left) {
    const auto& right = checked_cast<const StructArray&>(right_);
    bool equal_fields = true;
    for (int64_t i = left_start_idx_, o_i = right_start_idx_; i < left_end_idx_;
         ++i, ++o_i) {
      if (left.IsNull(i) != right.IsNull(o_i)) {
        return false;
      }
      if (left.IsNull(i)) continue;
      for (int j = 0; j < left.num_fields(); ++j) {
        // TODO: really we should be comparing stretches of non-null data rather
        // than looking at one value at a time.
        equal_fields = left.field(j)->RangeEquals(i, i + 1, o_i, right.field(j));
        if (!equal_fields) {
          return false;
        }
      }
    }
    return true;
  }

  bool CompareUnions(const UnionArray& left) const {
    const auto& right = checked_cast<const UnionArray&>(right_);

    const UnionMode::type union_mode = left.mode();
    if (union_mode != right.mode()) {
      return false;
    }

    const auto& left_type = checked_cast<const UnionType&>(*left.type());

    const std::vector<int>& child_ids = left_type.child_ids();

    const int8_t* left_codes = left.raw_type_codes();
    const int8_t* right_codes = right.raw_type_codes();

    for (int64_t i = left_start_idx_, o_i = right_start_idx_; i < left_end_idx_;
         ++i, ++o_i) {
      if (left.IsNull(i) != right.IsNull(o_i)) {
        return false;
      }
      if (left.IsNull(i)) continue;
      if (left_codes[i] != right_codes[o_i]) {
        return false;
      }

      auto child_num = child_ids[left_codes[i]];

      // TODO(wesm): really we should be comparing stretches of non-null data
      // rather than looking at one value at a time.
      if (union_mode == UnionMode::SPARSE) {
        if (!left.field(child_num)->RangeEquals(i, i + 1, o_i, right.field(child_num))) {
          return false;
        }
      } else {
        const int32_t offset =
            checked_cast<const DenseUnionArray&>(left).raw_value_offsets()[i];
        const int32_t o_offset =
            checked_cast<const DenseUnionArray&>(right).raw_value_offsets()[o_i];
        if (!left.field(child_num)->RangeEquals(offset, offset + 1, o_offset,
                                                right.field(child_num))) {
          return false;
        }
      }
    }
    return true;
  }

  Status Visit(const BinaryArray& left) {
    result_ = CompareBinaryRange(left);
    return Status::OK();
  }

  Status Visit(const LargeBinaryArray& left) {
    result_ = CompareBinaryRange(left);
    return Status::OK();
  }

  Status Visit(const FixedSizeBinaryArray& left) {
    const auto& right = checked_cast<const FixedSizeBinaryArray&>(right_);

    int32_t width = left.byte_width();

    const uint8_t* left_data = nullptr;
    const uint8_t* right_data = nullptr;

    if (left.values()) {
      left_data = left.raw_values();
    }

    if (right.values()) {
      right_data = right.raw_values();
    }

    for (int64_t i = left_start_idx_, o_i = right_start_idx_; i < left_end_idx_;
         ++i, ++o_i) {
      const bool is_null = left.IsNull(i);
      if (is_null != right.IsNull(o_i)) {
        result_ = false;
        return Status::OK();
      }
      if (is_null) continue;

      if (std::memcmp(left_data + width * i, right_data + width * o_i, width)) {
        result_ = false;
        return Status::OK();
      }
    }
    result_ = true;
    return Status::OK();
  }

  Status Visit(const Decimal128Array& left) {
    return Visit(checked_cast<const FixedSizeBinaryArray&>(left));
  }

  Status Visit(const NullArray& left) {
    ARROW_UNUSED(left);
    result_ = true;
    return Status::OK();
  }

  template <typename T>
  typename std::enable_if<std::is_base_of<PrimitiveArray, T>::value, Status>::type Visit(
      const T& left) {
    return CompareValues<T>(left);
  }

  Status Visit(const ListArray& left) {
    result_ = CompareLists(left);
    return Status::OK();
  }

  Status Visit(const LargeListArray& left) {
    result_ = CompareLists(left);
    return Status::OK();
  }

  Status Visit(const FixedSizeListArray& left) {
    const auto& right = checked_cast<const FixedSizeListArray&>(right_);
    result_ = left.values()->RangeEquals(
        left.value_offset(left_start_idx_), left.value_offset(left_end_idx_),
        right.value_offset(right_start_idx_), right.values());
    return Status::OK();
  }

  Status Visit(const MapArray& left) {
    result_ = CompareMaps(left);
    return Status::OK();
  }

  Status Visit(const StructArray& left) {
    result_ = CompareStructs(left);
    return Status::OK();
  }

  Status Visit(const UnionArray& left) {
    result_ = CompareUnions(left);
    return Status::OK();
  }

  Status Visit(const DictionaryArray& left) {
    const auto& right = checked_cast<const DictionaryArray&>(right_);
    if (!left.dictionary()->Equals(right.dictionary())) {
      result_ = false;
      return Status::OK();
    }
    result_ = left.indices()->RangeEquals(left_start_idx_, left_end_idx_,
                                          right_start_idx_, right.indices());
    return Status::OK();
  }

  Status Visit(const ExtensionArray& left) {
    result_ = (right_.type()->Equals(*left.type()) &&
               ArrayRangeEquals(*left.storage(),
                                *static_cast<const ExtensionArray&>(right_).storage(),
                                left_start_idx_, left_end_idx_, right_start_idx_));
    return Status::OK();
  }

  bool result() const { return result_; }

 protected:
  const Array& right_;
  int64_t left_start_idx_;
  int64_t left_end_idx_;
  int64_t right_start_idx_;

  bool result_;
};

static bool IsEqualPrimitive(const PrimitiveArray& left, const PrimitiveArray& right) {
  const int byte_width = internal::GetByteWidth(*left.type());

  const uint8_t* left_data = nullptr;
  const uint8_t* right_data = nullptr;

  if (left.values()) {
    left_data = left.values()->data() + left.offset() * byte_width;
  }

  if (right.values()) {
    right_data = right.values()->data() + right.offset() * byte_width;
  }

  if (byte_width == 0) {
    // Special case 0-width data, as the data pointers may be null
    for (int64_t i = 0; i < left.length(); ++i) {
      if (left.IsNull(i) != right.IsNull(i)) {
        return false;
      }
    }
    return true;
  } else if (left.null_count() > 0) {
    for (int64_t i = 0; i < left.length(); ++i) {
      const bool left_null = left.IsNull(i);
      const bool right_null = right.IsNull(i);
      if (left_null != right_null) {
        return false;
      }
      if (!left_null && memcmp(left_data, right_data, byte_width) != 0) {
        return false;
      }
      left_data += byte_width;
      right_data += byte_width;
    }
    return true;
  } else {
    auto number_of_bytes_to_compare = static_cast<size_t>(byte_width * left.length());
    return memcmp(left_data, right_data, number_of_bytes_to_compare) == 0;
  }
}

// A bit confusing: ArrayEqualsVisitor inherits from RangeEqualsVisitor but
// doesn't share the same preconditions.
// When RangeEqualsVisitor is called, we only know the range sizes equal.
// When ArrayEqualsVisitor is called, we know the sizes and null bitmaps are equal.

class ArrayEqualsVisitor : public RangeEqualsVisitor {
 public:
  explicit ArrayEqualsVisitor(const Array& right, const EqualOptions& opts)
      : RangeEqualsVisitor(right, 0, right.length(), 0), opts_(opts) {}

  Status Visit(const NullArray& left) {
    ARROW_UNUSED(left);
    result_ = true;
    return Status::OK();
  }

  Status Visit(const BooleanArray& left) {
    const auto& right = checked_cast<const BooleanArray&>(right_);

    if (left.null_count() > 0) {
      const uint8_t* left_data = left.values()->data();
      const uint8_t* right_data = right.values()->data();

      for (int64_t i = 0; i < left.length(); ++i) {
        if (left.IsValid(i) && BitUtil::GetBit(left_data, i + left.offset()) !=
                                   BitUtil::GetBit(right_data, i + right.offset())) {
          result_ = false;
          return Status::OK();
        }
      }
      result_ = true;
    } else {
      result_ = BitmapEquals(left.values()->data(), left.offset(), right.values()->data(),
                             right.offset(), left.length());
    }
    return Status::OK();
  }

  template <typename T>
  typename std::enable_if<std::is_base_of<PrimitiveArray, T>::value &&
                              !std::is_base_of<FloatArray, T>::value &&
                              !std::is_base_of<DoubleArray, T>::value &&
                              !std::is_base_of<BooleanArray, T>::value,
                          Status>::type
  Visit(const T& left) {
    result_ = IsEqualPrimitive(left, checked_cast<const PrimitiveArray&>(right_));
    return Status::OK();
  }

  // TODO nan-aware specialization for half-floats

  Status Visit(const FloatArray& left) {
    result_ =
        FloatingEquals<FloatType>(left, checked_cast<const FloatArray&>(right_), opts_);
    return Status::OK();
  }

  Status Visit(const DoubleArray& left) {
    result_ =
        FloatingEquals<DoubleType>(left, checked_cast<const DoubleArray&>(right_), opts_);
    return Status::OK();
  }

  template <typename ArrayType>
  bool ValueOffsetsEqual(const ArrayType& left) {
    using offset_type = typename ArrayType::offset_type;

    const auto& right = checked_cast<const ArrayType&>(right_);

    if (left.offset() == 0 && right.offset() == 0) {
      return left.value_offsets()->Equals(*right.value_offsets(),
                                          (left.length() + 1) * sizeof(offset_type));
    } else {
      // One of the arrays is sliced; logic is more complicated because the
      // value offsets are not both 0-based
      auto left_offsets =
          reinterpret_cast<const offset_type*>(left.value_offsets()->data()) +
          left.offset();
      auto right_offsets =
          reinterpret_cast<const offset_type*>(right.value_offsets()->data()) +
          right.offset();

      for (int64_t i = 0; i < left.length() + 1; ++i) {
        if (left_offsets[i] - left_offsets[0] != right_offsets[i] - right_offsets[0]) {
          return false;
        }
      }
      return true;
    }
  }

  template <typename BinaryArrayType>
  bool CompareBinary(const BinaryArrayType& left) {
    const auto& right = checked_cast<const BinaryArrayType&>(right_);

    bool equal_offsets = ValueOffsetsEqual<BinaryArrayType>(left);
    if (!equal_offsets) {
      return false;
    }

    if (!left.value_data() && !(right.value_data())) {
      return true;
    }
    if (left.value_offset(left.length()) == left.value_offset(0)) {
      return true;
    }

    const uint8_t* left_data = left.value_data()->data();
    const uint8_t* right_data = right.value_data()->data();

    if (left.null_count() == 0) {
      // Fast path for null count 0, single memcmp
      if (left.offset() == 0 && right.offset() == 0) {
        return std::memcmp(left_data, right_data,
                           left.raw_value_offsets()[left.length()]) == 0;
      } else {
        const int64_t total_bytes =
            left.value_offset(left.length()) - left.value_offset(0);
        return std::memcmp(left_data + left.value_offset(0),
                           right_data + right.value_offset(0),
                           static_cast<size_t>(total_bytes)) == 0;
      }
    } else {
      // ARROW-537: Only compare data in non-null slots
      auto left_offsets = left.raw_value_offsets();
      auto right_offsets = right.raw_value_offsets();
      for (int64_t i = 0; i < left.length(); ++i) {
        if (left.IsNull(i)) {
          continue;
        }
        if (std::memcmp(left_data + left_offsets[i], right_data + right_offsets[i],
                        left.value_length(i))) {
          return false;
        }
      }
      return true;
    }
  }

  template <typename ListArrayType>
  bool CompareList(const ListArrayType& left) {
    const auto& right = checked_cast<const ListArrayType&>(right_);

    bool equal_offsets = ValueOffsetsEqual<ListArrayType>(left);
    if (!equal_offsets) {
      return false;
    }

    return left.values()->RangeEquals(left.value_offset(0),
                                      left.value_offset(left.length()),
                                      right.value_offset(0), right.values());
  }

  Status Visit(const BinaryArray& left) {
    result_ = CompareBinary(left);
    return Status::OK();
  }

  Status Visit(const LargeBinaryArray& left) {
    result_ = CompareBinary(left);
    return Status::OK();
  }

  Status Visit(const ListArray& left) {
    result_ = CompareList(left);
    return Status::OK();
  }

  Status Visit(const LargeListArray& left) {
    result_ = CompareList(left);
    return Status::OK();
  }

  Status Visit(const FixedSizeListArray& left) {
    const auto& right = checked_cast<const FixedSizeListArray&>(right_);
    result_ =
        left.values()->RangeEquals(left.value_offset(0), left.value_offset(left.length()),
                                   right.value_offset(0), right.values());
    return Status::OK();
  }

  Status Visit(const DictionaryArray& left) {
    const auto& right = checked_cast<const DictionaryArray&>(right_);
    if (!left.dictionary()->Equals(right.dictionary())) {
      result_ = false;
    } else {
      result_ = left.indices()->Equals(right.indices());
    }
    return Status::OK();
  }

  template <typename T>
  typename std::enable_if<std::is_base_of<NestedType, typename T::TypeClass>::value,
                          Status>::type
  Visit(const T& left) {
    return RangeEqualsVisitor::Visit(left);
  }

  Status Visit(const ExtensionArray& left) {
    result_ = (right_.type()->Equals(*left.type()) &&
               ArrayEquals(*left.storage(),
                           *static_cast<const ExtensionArray&>(right_).storage()));
    return Status::OK();
  }

 protected:
  const EqualOptions opts_;
};

class ApproxEqualsVisitor : public ArrayEqualsVisitor {
 public:
  explicit ApproxEqualsVisitor(const Array& right, const EqualOptions& opts)
      : ArrayEqualsVisitor(right, opts) {}

  using ArrayEqualsVisitor::Visit;

  // TODO half-floats

  Status Visit(const FloatArray& left) {
    result_ = FloatingApproxEquals<FloatType>(
        left, checked_cast<const FloatArray&>(right_), opts_);
    return Status::OK();
  }

  Status Visit(const DoubleArray& left) {
    result_ = FloatingApproxEquals<DoubleType>(
        left, checked_cast<const DoubleArray&>(right_), opts_);
    return Status::OK();
  }
};

static bool BaseDataEquals(const Array& left, const Array& right) {
  if (left.length() != right.length() || left.null_count() != right.null_count() ||
      left.type_id() != right.type_id()) {
    return false;
  }
  // ARROW-2567: Ensure that not only the type id but also the type equality
  // itself is checked.
  if (!TypeEquals(*left.type(), *right.type(), false /* check_metadata */)) {
    return false;
  }
  if (left.null_count() > 0 && left.null_count() < left.length()) {
    return BitmapEquals(left.null_bitmap()->data(), left.offset(),
                        right.null_bitmap()->data(), right.offset(), left.length());
  }
  return true;
}

template <typename VISITOR, typename... Extra>
inline bool ArrayEqualsImpl(const Array& left, const Array& right, Extra&&... extra) {
  bool are_equal;
  // The arrays are the same object
  if (&left == &right) {
    are_equal = true;
  } else if (!BaseDataEquals(left, right)) {
    are_equal = false;
  } else if (left.length() == 0) {
    are_equal = true;
  } else if (left.null_count() == left.length()) {
    are_equal = true;
  } else {
    VISITOR visitor(right, std::forward<Extra>(extra)...);
    auto error = VisitArrayInline(left, &visitor);
    if (!error.ok()) {
      DCHECK(false) << "Arrays are not comparable: " << error.ToString();
    }
    are_equal = visitor.result();
  }
  return are_equal;
}

class TypeEqualsVisitor {
 public:
  explicit TypeEqualsVisitor(const DataType& right, bool check_metadata)
      : right_(right), check_metadata_(check_metadata), result_(false) {}

  Status VisitChildren(const DataType& left) {
    if (left.num_fields() != right_.num_fields()) {
      result_ = false;
      return Status::OK();
    }

    for (int i = 0; i < left.num_fields(); ++i) {
      if (!left.field(i)->Equals(right_.field(i), check_metadata_)) {
        result_ = false;
        return Status::OK();
      }
    }
    result_ = true;
    return Status::OK();
  }

  template <typename T>
  enable_if_t<is_null_type<T>::value || is_primitive_ctype<T>::value ||
                  is_base_binary_type<T>::value,
              Status>
  Visit(const T&) {
    result_ = true;
    return Status::OK();
  }

  template <typename T>
  enable_if_interval<T, Status> Visit(const T& left) {
    const auto& right = checked_cast<const IntervalType&>(right_);
    result_ = right.interval_type() == left.interval_type();
    return Status::OK();
  }

  template <typename T>
  enable_if_t<is_time_type<T>::value || is_date_type<T>::value ||
                  is_duration_type<T>::value,
              Status>
  Visit(const T& left) {
    const auto& right = checked_cast<const T&>(right_);
    result_ = left.unit() == right.unit();
    return Status::OK();
  }

  Status Visit(const TimestampType& left) {
    const auto& right = checked_cast<const TimestampType&>(right_);
    result_ = left.unit() == right.unit() && left.timezone() == right.timezone();
    return Status::OK();
  }

  Status Visit(const FixedSizeBinaryType& left) {
    const auto& right = checked_cast<const FixedSizeBinaryType&>(right_);
    result_ = left.byte_width() == right.byte_width();
    return Status::OK();
  }

  Status Visit(const Decimal128Type& left) {
    const auto& right = checked_cast<const Decimal128Type&>(right_);
    result_ = left.precision() == right.precision() && left.scale() == right.scale();
    return Status::OK();
  }

  template <typename T>
  enable_if_t<is_list_like_type<T>::value || is_struct_type<T>::value, Status> Visit(
      const T& left) {
    return VisitChildren(left);
  }

  Status Visit(const MapType& left) {
    const auto& right = checked_cast<const MapType&>(right_);
    if (left.keys_sorted() != right.keys_sorted()) {
      result_ = false;
      return Status::OK();
    }
    result_ = left.key_type()->Equals(*right.key_type(), check_metadata_) &&
              left.item_type()->Equals(*right.item_type(), check_metadata_);
    return Status::OK();
  }

  Status Visit(const UnionType& left) {
    const auto& right = checked_cast<const UnionType&>(right_);

    if (left.mode() != right.mode() || left.type_codes() != right.type_codes()) {
      result_ = false;
      return Status::OK();
    }

    result_ = std::equal(
        left.fields().begin(), left.fields().end(), right.fields().begin(),
        [this](const std::shared_ptr<Field>& l, const std::shared_ptr<Field>& r) {
          return l->Equals(r, check_metadata_);
        });
    return Status::OK();
  }

  Status Visit(const DictionaryType& left) {
    const auto& right = checked_cast<const DictionaryType&>(right_);
    result_ = left.index_type()->Equals(right.index_type()) &&
              left.value_type()->Equals(right.value_type()) &&
              (left.ordered() == right.ordered());
    return Status::OK();
  }

  Status Visit(const ExtensionType& left) {
    result_ = left.ExtensionEquals(static_cast<const ExtensionType&>(right_));
    return Status::OK();
  }

  bool result() const { return result_; }

 protected:
  const DataType& right_;
  bool check_metadata_;
  bool result_;
};

class ScalarEqualsVisitor {
 public:
  explicit ScalarEqualsVisitor(const Scalar& right,
                               const EqualOptions& opts = EqualOptions::Defaults())
      : right_(right), result_(false), options_(opts) {}

  Status Visit(const NullScalar& left) {
    result_ = true;
    return Status::OK();
  }

  Status Visit(const BooleanScalar& left) {
    const auto& right = checked_cast<const BooleanScalar&>(right_);
    result_ = left.value == right.value;
    return Status::OK();
  }

  template <typename T>
  typename std::enable_if<std::is_base_of<FloatScalar, T>::value ||
                              std::is_base_of<DoubleScalar, T>::value,
                          Status>::type
  Visit(const T& left_) {
    const auto& right = checked_cast<const T&>(right_);
    if (options_.nans_equal()) {
      result_ = right.value == left_.value ||
                (std::isnan(right.value) && std::isnan(left_.value));
    } else {
      result_ = right.value == left_.value;
    }
    return Status::OK();
  }

  template <typename T>
  typename std::enable_if<
      (std::is_base_of<internal::PrimitiveScalar<typename T::TypeClass>, T>::value &&
       !std::is_base_of<FloatScalar, T>::value &&
       !std::is_base_of<DoubleScalar, T>::value) ||
          std::is_base_of<TemporalScalar<typename T::TypeClass>, T>::value,
      Status>::type
  Visit(const T& left_) {
    const auto& right = checked_cast<const T&>(right_);
    result_ = right.value == left_.value;
    return Status::OK();
  }

  template <typename T>
  typename std::enable_if<std::is_base_of<BaseBinaryScalar, T>::value, Status>::type
  Visit(const T& left) {
    const auto& right = checked_cast<const BaseBinaryScalar&>(right_);
    result_ = internal::SharedPtrEquals(left.value, right.value);
    return Status::OK();
  }

  Status Visit(const Decimal128Scalar& left) {
    const auto& right = checked_cast<const Decimal128Scalar&>(right_);
    result_ = left.value == right.value;
    return Status::OK();
  }

  Status Visit(const ListScalar& left) {
    const auto& right = checked_cast<const ListScalar&>(right_);
    result_ = internal::SharedPtrEquals(left.value, right.value);
    return Status::OK();
  }

  Status Visit(const LargeListScalar& left) {
    const auto& right = checked_cast<const LargeListScalar&>(right_);
    result_ = internal::SharedPtrEquals(left.value, right.value);
    return Status::OK();
  }

  Status Visit(const MapScalar& left) {
    const auto& right = checked_cast<const MapScalar&>(right_);
    result_ = internal::SharedPtrEquals(left.value, right.value);
    return Status::OK();
  }

  Status Visit(const FixedSizeListScalar& left) {
    const auto& right = checked_cast<const FixedSizeListScalar&>(right_);
    result_ = internal::SharedPtrEquals(left.value, right.value);
    return Status::OK();
  }

  Status Visit(const StructScalar& left) {
    const auto& right = checked_cast<const StructScalar&>(right_);

    if (right.value.size() != left.value.size()) {
      result_ = false;
    } else {
      bool all_equals = true;
      for (size_t i = 0; i < left.value.size() && all_equals; i++) {
        all_equals &= internal::SharedPtrEquals(left.value[i], right.value[i]);
      }
      result_ = all_equals;
    }

    return Status::OK();
  }

  Status Visit(const UnionScalar& left) {
    const auto& right = checked_cast<const UnionScalar&>(right_);
    if (left.is_valid && right.is_valid) {
      result_ = left.value->Equals(*right.value);
    } else if (!left.is_valid && !right.is_valid) {
      result_ = true;
    } else {
      result_ = false;
    }
    return Status::OK();
  }

  Status Visit(const DictionaryScalar& left) {
    const auto& right = checked_cast<const DictionaryScalar&>(right_);
    result_ = left.value.index->Equals(right.value.index) &&
              left.value.dictionary->Equals(right.value.dictionary);
    return Status::OK();
  }

  Status Visit(const ExtensionScalar& left) {
    return Status::NotImplemented("extension");
  }

  bool result() const { return result_; }

 protected:
  const Scalar& right_;
  bool result_;
  const EqualOptions options_;
};

Status PrintDiff(const Array& left, const Array& right, std::ostream* os) {
  if (os == nullptr) {
    return Status::OK();
  }

  if (!left.type()->Equals(right.type())) {
    *os << "# Array types differed: " << *left.type() << " vs " << *right.type()
        << std::endl;
    return Status::OK();
  }

  if (left.type()->id() == Type::DICTIONARY) {
    *os << "# Dictionary arrays differed" << std::endl;

    const auto& left_dict = checked_cast<const DictionaryArray&>(left);
    const auto& right_dict = checked_cast<const DictionaryArray&>(right);

    *os << "## dictionary diff";
    auto pos = os->tellp();
    RETURN_NOT_OK(PrintDiff(*left_dict.dictionary(), *right_dict.dictionary(), os));
    if (os->tellp() == pos) {
      *os << std::endl;
    }

    *os << "## indices diff";
    pos = os->tellp();
    RETURN_NOT_OK(PrintDiff(*left_dict.indices(), *right_dict.indices(), os));
    if (os->tellp() == pos) {
      *os << std::endl;
    }
    return Status::OK();
  }

  ARROW_ASSIGN_OR_RAISE(auto edits, Diff(left, right, default_memory_pool()));
  ARROW_ASSIGN_OR_RAISE(auto formatter, MakeUnifiedDiffFormatter(*left.type(), os));
  return formatter(*edits, left, right);
}

}  // namespace

bool ArrayEquals(const Array& left, const Array& right, const EqualOptions& opts) {
  bool are_equal = ArrayEqualsImpl<ArrayEqualsVisitor>(left, right, opts);
  if (!are_equal) {
    ARROW_IGNORE_EXPR(PrintDiff(left, right, opts.diff_sink()));
  }
  return are_equal;
}

bool ArrayApproxEquals(const Array& left, const Array& right, const EqualOptions& opts) {
  bool are_equal = ArrayEqualsImpl<ApproxEqualsVisitor>(left, right, opts);
  if (!are_equal) {
    DCHECK_OK(PrintDiff(left, right, opts.diff_sink()));
  }
  return are_equal;
}

bool ArrayRangeEquals(const Array& left, const Array& right, int64_t left_start_idx,
                      int64_t left_end_idx, int64_t right_start_idx) {
  bool are_equal;
  if (&left == &right) {
    are_equal = true;
  } else if (left.type_id() != right.type_id() ||
             !TypeEquals(*left.type(), *right.type(), false /* check_metadata */)) {
    are_equal = false;
  } else if (left.length() == 0) {
    are_equal = true;
  } else {
    RangeEqualsVisitor visitor(right, left_start_idx, left_end_idx, right_start_idx);
    auto error = VisitArrayInline(left, &visitor);
    if (!error.ok()) {
      DCHECK(false) << "Arrays are not comparable: " << error.ToString();
    }
    are_equal = visitor.result();
  }
  return are_equal;
}

namespace {

bool StridedIntegerTensorContentEquals(const int dim_index, int64_t left_offset,
                                       int64_t right_offset, int elem_size,
                                       const Tensor& left, const Tensor& right) {
  const auto n = left.shape()[dim_index];
  const auto left_stride = left.strides()[dim_index];
  const auto right_stride = right.strides()[dim_index];
  if (dim_index == left.ndim() - 1) {
    for (int64_t i = 0; i < n; ++i) {
      if (memcmp(left.raw_data() + left_offset + i * left_stride,
                 right.raw_data() + right_offset + i * right_stride, elem_size) != 0) {
        return false;
      }
    }
    return true;
  }
  for (int64_t i = 0; i < n; ++i) {
    if (!StridedIntegerTensorContentEquals(dim_index + 1, left_offset, right_offset,
                                           elem_size, left, right)) {
      return false;
    }
    left_offset += left_stride;
    right_offset += right_stride;
  }
  return true;
}

bool IntegerTensorEquals(const Tensor& left, const Tensor& right) {
  bool are_equal;
  // The arrays are the same object
  if (&left == &right) {
    are_equal = true;
  } else {
    const bool left_row_major_p = left.is_row_major();
    const bool left_column_major_p = left.is_column_major();
    const bool right_row_major_p = right.is_row_major();
    const bool right_column_major_p = right.is_column_major();

    if (!(left_row_major_p && right_row_major_p) &&
        !(left_column_major_p && right_column_major_p)) {
      const auto& type = checked_cast<const FixedWidthType&>(*left.type());
      are_equal = StridedIntegerTensorContentEquals(0, 0, 0, internal::GetByteWidth(type),
                                                    left, right);
    } else {
      const int byte_width = internal::GetByteWidth(*left.type());
      DCHECK_GT(byte_width, 0);

      const uint8_t* left_data = left.data()->data();
      const uint8_t* right_data = right.data()->data();

      are_equal = memcmp(left_data, right_data,
                         static_cast<size_t>(byte_width * left.size())) == 0;
    }
  }
  return are_equal;
}

template <typename DataType>
bool StridedFloatTensorContentEquals(const int dim_index, int64_t left_offset,
                                     int64_t right_offset, const Tensor& left,
                                     const Tensor& right, const EqualOptions& opts) {
  using c_type = typename DataType::c_type;
  static_assert(std::is_floating_point<c_type>::value,
                "DataType must be a floating point type");

  const auto n = left.shape()[dim_index];
  const auto left_stride = left.strides()[dim_index];
  const auto right_stride = right.strides()[dim_index];
  if (dim_index == left.ndim() - 1) {
    auto left_data = left.raw_data();
    auto right_data = right.raw_data();
    if (opts.nans_equal()) {
      for (int64_t i = 0; i < n; ++i) {
        c_type left_value =
            *reinterpret_cast<const c_type*>(left_data + left_offset + i * left_stride);
        c_type right_value = *reinterpret_cast<const c_type*>(right_data + right_offset +
                                                              i * right_stride);
        if (left_value != right_value &&
            !(std::isnan(left_value) && std::isnan(right_value))) {
          return false;
        }
      }
    } else {
      for (int64_t i = 0; i < n; ++i) {
        c_type left_value =
            *reinterpret_cast<const c_type*>(left_data + left_offset + i * left_stride);
        c_type right_value = *reinterpret_cast<const c_type*>(right_data + right_offset +
                                                              i * right_stride);
        if (left_value != right_value) {
          return false;
        }
      }
    }
    return true;
  }
  for (int64_t i = 0; i < n; ++i) {
    if (!StridedFloatTensorContentEquals<DataType>(dim_index + 1, left_offset,
                                                   right_offset, left, right, opts)) {
      return false;
    }
    left_offset += left_stride;
    right_offset += right_stride;
  }
  return true;
}

template <typename DataType>
bool FloatTensorEquals(const Tensor& left, const Tensor& right,
                       const EqualOptions& opts) {
  return StridedFloatTensorContentEquals<DataType>(0, 0, 0, left, right, opts);
}

}  // namespace

bool TensorEquals(const Tensor& left, const Tensor& right, const EqualOptions& opts) {
  if (left.type_id() != right.type_id()) {
    return false;
  } else if (left.size() == 0 && right.size() == 0) {
    return true;
  } else if (left.shape() != right.shape()) {
    return false;
  }

  switch (left.type_id()) {
    // TODO: Support half-float tensors
    // case Type::HALF_FLOAT:
    case Type::FLOAT:
      return FloatTensorEquals<FloatType>(left, right, opts);

    case Type::DOUBLE:
      return FloatTensorEquals<DoubleType>(left, right, opts);

    default:
      return IntegerTensorEquals(left, right);
  }
}

namespace {

template <typename LeftSparseIndexType, typename RightSparseIndexType>
struct SparseTensorEqualsImpl {
  static bool Compare(const SparseTensorImpl<LeftSparseIndexType>& left,
                      const SparseTensorImpl<RightSparseIndexType>& right,
                      const EqualOptions&) {
    // TODO(mrkn): should we support the equality among different formats?
    return false;
  }
};

bool IntegerSparseTensorDataEquals(const uint8_t* left_data, const uint8_t* right_data,
                                   const int byte_width, const int64_t length) {
  if (left_data == right_data) {
    return true;
  }
  return memcmp(left_data, right_data, static_cast<size_t>(byte_width * length)) == 0;
}

template <typename DataType>
bool FloatSparseTensorDataEquals(const typename DataType::c_type* left_data,
                                 const typename DataType::c_type* right_data,
                                 const int64_t length, const EqualOptions& opts) {
  using c_type = typename DataType::c_type;
  static_assert(std::is_floating_point<c_type>::value,
                "DataType must be a floating point type");
  if (opts.nans_equal()) {
    if (left_data == right_data) {
      return true;
    }

    for (int64_t i = 0; i < length; ++i) {
      const auto left = left_data[i];
      const auto right = right_data[i];
      if (left != right && !(std::isnan(left) && std::isnan(right))) {
        return false;
      }
    }
  } else {
    for (int64_t i = 0; i < length; ++i) {
      if (left_data[i] != right_data[i]) {
        return false;
      }
    }
  }
  return true;
}

template <typename SparseIndexType>
struct SparseTensorEqualsImpl<SparseIndexType, SparseIndexType> {
  static bool Compare(const SparseTensorImpl<SparseIndexType>& left,
                      const SparseTensorImpl<SparseIndexType>& right,
                      const EqualOptions& opts) {
    DCHECK(left.type()->id() == right.type()->id());
    DCHECK(left.shape() == right.shape());

    const auto length = left.non_zero_length();
    DCHECK(length == right.non_zero_length());

    const auto& left_index = checked_cast<const SparseIndexType&>(*left.sparse_index());
    const auto& right_index = checked_cast<const SparseIndexType&>(*right.sparse_index());

    if (!left_index.Equals(right_index)) {
      return false;
    }

    const int byte_width = internal::GetByteWidth(*left.type());
    DCHECK_GT(byte_width, 0);

    const uint8_t* left_data = left.data()->data();
    const uint8_t* right_data = right.data()->data();
    switch (left.type()->id()) {
      // TODO: Support half-float tensors
      // case Type::HALF_FLOAT:
      case Type::FLOAT:
        return FloatSparseTensorDataEquals<FloatType>(
            reinterpret_cast<const float*>(left_data),
            reinterpret_cast<const float*>(right_data), length, opts);

      case Type::DOUBLE:
        return FloatSparseTensorDataEquals<DoubleType>(
            reinterpret_cast<const double*>(left_data),
            reinterpret_cast<const double*>(right_data), length, opts);

      default:  // Integer cases
        return IntegerSparseTensorDataEquals(left_data, right_data, byte_width, length);
    }
  }
};

template <typename SparseIndexType>
inline bool SparseTensorEqualsImplDispatch(const SparseTensorImpl<SparseIndexType>& left,
                                           const SparseTensor& right,
                                           const EqualOptions& opts) {
  switch (right.format_id()) {
    case SparseTensorFormat::COO: {
      const auto& right_coo =
          checked_cast<const SparseTensorImpl<SparseCOOIndex>&>(right);
      return SparseTensorEqualsImpl<SparseIndexType, SparseCOOIndex>::Compare(
          left, right_coo, opts);
    }

    case SparseTensorFormat::CSR: {
      const auto& right_csr =
          checked_cast<const SparseTensorImpl<SparseCSRIndex>&>(right);
      return SparseTensorEqualsImpl<SparseIndexType, SparseCSRIndex>::Compare(
          left, right_csr, opts);
    }

    case SparseTensorFormat::CSC: {
      const auto& right_csc =
          checked_cast<const SparseTensorImpl<SparseCSCIndex>&>(right);
      return SparseTensorEqualsImpl<SparseIndexType, SparseCSCIndex>::Compare(
          left, right_csc, opts);
    }

    case SparseTensorFormat::CSF: {
      const auto& right_csf =
          checked_cast<const SparseTensorImpl<SparseCSFIndex>&>(right);
      return SparseTensorEqualsImpl<SparseIndexType, SparseCSFIndex>::Compare(
          left, right_csf, opts);
    }

    default:
      return false;
  }
}

}  // namespace

bool SparseTensorEquals(const SparseTensor& left, const SparseTensor& right,
                        const EqualOptions& opts) {
  if (left.type()->id() != right.type()->id()) {
    return false;
  } else if (left.size() == 0 && right.size() == 0) {
    return true;
  } else if (left.shape() != right.shape()) {
    return false;
  } else if (left.non_zero_length() != right.non_zero_length()) {
    return false;
  }

  switch (left.format_id()) {
    case SparseTensorFormat::COO: {
      const auto& left_coo = checked_cast<const SparseTensorImpl<SparseCOOIndex>&>(left);
      return SparseTensorEqualsImplDispatch(left_coo, right, opts);
    }

    case SparseTensorFormat::CSR: {
      const auto& left_csr = checked_cast<const SparseTensorImpl<SparseCSRIndex>&>(left);
      return SparseTensorEqualsImplDispatch(left_csr, right, opts);
    }

    case SparseTensorFormat::CSC: {
      const auto& left_csc = checked_cast<const SparseTensorImpl<SparseCSCIndex>&>(left);
      return SparseTensorEqualsImplDispatch(left_csc, right, opts);
    }

    case SparseTensorFormat::CSF: {
      const auto& left_csf = checked_cast<const SparseTensorImpl<SparseCSFIndex>&>(left);
      return SparseTensorEqualsImplDispatch(left_csf, right, opts);
    }

    default:
      return false;
  }
}

bool TypeEquals(const DataType& left, const DataType& right, bool check_metadata) {
  // The arrays are the same object
  if (&left == &right) {
    return true;
  } else if (left.id() != right.id()) {
    return false;
  } else {
    // First try to compute fingerprints
    if (check_metadata) {
      const auto& left_metadata_fp = left.metadata_fingerprint();
      const auto& right_metadata_fp = right.metadata_fingerprint();
      if (left_metadata_fp != right_metadata_fp) {
        return false;
      }
    }

    const auto& left_fp = left.fingerprint();
    const auto& right_fp = right.fingerprint();
    if (!left_fp.empty() && !right_fp.empty()) {
      return left_fp == right_fp;
    }

    // TODO remove check_metadata here?
    TypeEqualsVisitor visitor(right, check_metadata);
    auto error = VisitTypeInline(left, &visitor);
    if (!error.ok()) {
      DCHECK(false) << "Types are not comparable: " << error.ToString();
    }
    return visitor.result();
  }
}

bool ScalarEquals(const Scalar& left, const Scalar& right, const EqualOptions& options) {
  bool are_equal = false;
  if (&left == &right) {
    are_equal = true;
  } else if (!left.type->Equals(right.type)) {
    are_equal = false;
  } else if (left.is_valid != right.is_valid) {
    are_equal = false;
  } else {
    ScalarEqualsVisitor visitor(right, options);
    auto error = VisitScalarInline(left, &visitor);
    DCHECK_OK(error);
    are_equal = visitor.result();
  }
  return are_equal;
}

}  // namespace arrow
