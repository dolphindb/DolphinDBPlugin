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
#include <limits>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include <gtest/gtest.h>

#include "arrow/array.h"
#include "arrow/chunked_array.h"
#include "arrow/compute/api_aggregate.h"
#include "arrow/compute/kernels/aggregate_internal.h"
#include "arrow/compute/kernels/test_util.h"
#include "arrow/type.h"
#include "arrow/type_traits.h"
#include "arrow/util/checked_cast.h"

#include "arrow/testing/gtest_common.h"
#include "arrow/testing/gtest_util.h"
#include "arrow/testing/random.h"

namespace arrow {

using internal::checked_cast;
using internal::checked_pointer_cast;

namespace compute {

//
// Sum
//

template <typename ArrowType>
using SumResult =
    std::pair<typename FindAccumulatorType<ArrowType>::Type::c_type, size_t>;

template <typename ArrowType>
static SumResult<ArrowType> NaiveSumPartial(const Array& array) {
  using ArrayType = typename TypeTraits<ArrowType>::ArrayType;
  using ResultType = SumResult<ArrowType>;

  ResultType result;

  auto data = array.data();
  const auto& array_numeric = reinterpret_cast<const ArrayType&>(array);
  const auto values = array_numeric.raw_values();

  if (array.null_count() != 0) {
    internal::BitmapReader reader(array.null_bitmap_data(), array.offset(),
                                  array.length());
    for (int64_t i = 0; i < array.length(); i++) {
      if (reader.IsSet()) {
        result.first += values[i];
        result.second++;
      }

      reader.Next();
    }
  } else {
    for (int64_t i = 0; i < array.length(); i++) {
      result.first += values[i];
      result.second++;
    }
  }

  return result;
}

template <typename ArrowType>
static Datum NaiveSum(const Array& array) {
  using SumType = typename FindAccumulatorType<ArrowType>::Type;
  using SumScalarType = typename TypeTraits<SumType>::ScalarType;

  auto result = NaiveSumPartial<ArrowType>(array);
  bool is_valid = result.second > 0;

  if (!is_valid) return Datum(std::make_shared<SumScalarType>());
  return Datum(std::make_shared<SumScalarType>(result.first));
}

template <typename ArrowType>
void ValidateSum(const Array& input, Datum expected) {
  using OutputType = typename FindAccumulatorType<ArrowType>::Type;

  ASSERT_OK_AND_ASSIGN(Datum result, Sum(input));
  DatumEqual<OutputType>::EnsureEqual(result, expected);
}

template <typename ArrowType>
void ValidateSum(const std::shared_ptr<ChunkedArray>& input, Datum expected) {
  using OutputType = typename FindAccumulatorType<ArrowType>::Type;

  ASSERT_OK_AND_ASSIGN(Datum result, Sum(input));
  DatumEqual<OutputType>::EnsureEqual(result, expected);
}

template <typename ArrowType>
void ValidateSum(const char* json, Datum expected) {
  auto array = ArrayFromJSON(TypeTraits<ArrowType>::type_singleton(), json);
  ValidateSum<ArrowType>(*array, expected);
}

template <typename ArrowType>
void ValidateSum(const std::vector<std::string>& json, Datum expected) {
  auto array = ChunkedArrayFromJSON(TypeTraits<ArrowType>::type_singleton(), json);
  ValidateSum<ArrowType>(array, expected);
}

template <typename ArrowType>
void ValidateSum(const Array& array) {
  ValidateSum<ArrowType>(array, NaiveSum<ArrowType>(array));
}

using UnaryOp = Result<Datum>(const Datum&, ExecContext*);

template <UnaryOp& Op, typename ScalarType>
void ValidateBooleanAgg(const std::string& json,
                        const std::shared_ptr<ScalarType>& expected) {
  auto array = ArrayFromJSON(boolean(), json);
  auto exp = Datum(expected);
  ASSERT_OK_AND_ASSIGN(Datum result, Op(array, nullptr));
  ASSERT_TRUE(result.Equals(exp));
}

TEST(TestBooleanAggregation, Sum) {
  ValidateBooleanAgg<Sum>("[]", std::make_shared<UInt64Scalar>());
  ValidateBooleanAgg<Sum>("[null]", std::make_shared<UInt64Scalar>());
  ValidateBooleanAgg<Sum>("[null, false]", std::make_shared<UInt64Scalar>(0));
  ValidateBooleanAgg<Sum>("[true]", std::make_shared<UInt64Scalar>(1));
  ValidateBooleanAgg<Sum>("[true, false, true]", std::make_shared<UInt64Scalar>(2));
  ValidateBooleanAgg<Sum>("[true, false, true, true, null]",
                          std::make_shared<UInt64Scalar>(3));
}

TEST(TestBooleanAggregation, Mean) {
  ValidateBooleanAgg<Mean>("[]", std::make_shared<DoubleScalar>());
  ValidateBooleanAgg<Mean>("[null]", std::make_shared<DoubleScalar>());
  ValidateBooleanAgg<Mean>("[null, false]", std::make_shared<DoubleScalar>(0));
  ValidateBooleanAgg<Mean>("[true]", std::make_shared<DoubleScalar>(1));
  ValidateBooleanAgg<Mean>("[true, false, true, false]",
                           std::make_shared<DoubleScalar>(0.5));
  ValidateBooleanAgg<Mean>("[true, null]", std::make_shared<DoubleScalar>(1));
  ValidateBooleanAgg<Mean>("[true, null, false, true, true]",
                           std::make_shared<DoubleScalar>(0.75));
  ValidateBooleanAgg<Mean>("[true, null, false, false, false]",
                           std::make_shared<DoubleScalar>(0.25));
}

template <typename ArrowType>
class TestNumericSumKernel : public ::testing::Test {};

TYPED_TEST_SUITE(TestNumericSumKernel, NumericArrowTypes);
TYPED_TEST(TestNumericSumKernel, SimpleSum) {
  using SumType = typename FindAccumulatorType<TypeParam>::Type;
  using ScalarType = typename TypeTraits<SumType>::ScalarType;
  using T = typename TypeParam::c_type;

  ValidateSum<TypeParam>("[]", Datum(std::make_shared<ScalarType>()));

  ValidateSum<TypeParam>("[null]", Datum(std::make_shared<ScalarType>()));

  ValidateSum<TypeParam>("[0, 1, 2, 3, 4, 5]",
                         Datum(std::make_shared<ScalarType>(static_cast<T>(5 * 6 / 2))));

  std::vector<std::string> chunks = {"[0, 1, 2, 3, 4, 5]"};
  ValidateSum<TypeParam>(chunks,
                         Datum(std::make_shared<ScalarType>(static_cast<T>(5 * 6 / 2))));

  chunks = {"[0, 1, 2]", "[3, 4, 5]"};
  ValidateSum<TypeParam>(chunks,
                         Datum(std::make_shared<ScalarType>(static_cast<T>(5 * 6 / 2))));

  chunks = {"[0, 1, 2]", "[]", "[3, 4, 5]"};
  ValidateSum<TypeParam>(chunks,
                         Datum(std::make_shared<ScalarType>(static_cast<T>(5 * 6 / 2))));

  chunks = {};
  ValidateSum<TypeParam>(chunks,
                         Datum(std::make_shared<ScalarType>()));  // null

  const T expected_result = static_cast<T>(14);
  ValidateSum<TypeParam>("[1, null, 3, null, 3, null, 7]",
                         Datum(std::make_shared<ScalarType>(expected_result)));
}

template <typename ArrowType>
class TestRandomNumericSumKernel : public ::testing::Test {};

TYPED_TEST_SUITE(TestRandomNumericSumKernel, NumericArrowTypes);
TYPED_TEST(TestRandomNumericSumKernel, RandomArraySum) {
  auto rand = random::RandomArrayGenerator(0x5487655);
  // Test size up to 1<<13 (8192).
  for (size_t i = 3; i < 14; i += 2) {
    for (auto null_probability : {0.0, 0.001, 0.1, 0.5, 0.999, 1.0}) {
      for (auto length_adjust : {-2, -1, 0, 1, 2}) {
        int64_t length = (1UL << i) + length_adjust;
        auto array = rand.Numeric<TypeParam>(length, 0, 100, null_probability);
        ValidateSum<TypeParam>(*array);
      }
    }
  }
}

TYPED_TEST_SUITE(TestRandomNumericSumKernel, NumericArrowTypes);
TYPED_TEST(TestRandomNumericSumKernel, RandomArraySumOverflow) {
  using CType = typename TypeParam::c_type;
  using SumCType = typename FindAccumulatorType<TypeParam>::Type::c_type;
  if (sizeof(CType) == sizeof(SumCType)) {
    // Skip if accumulator type is same to original type
    return;
  }

  CType max = std::numeric_limits<CType>::max();
  CType min = std::numeric_limits<CType>::min();
  int64_t length = 1024;

  auto rand = random::RandomArrayGenerator(0x5487655);
  for (auto null_probability : {0.0, 0.1, 0.5, 1.0}) {
    // Test overflow on the original type
    auto array = rand.Numeric<TypeParam>(length, max - 200, max - 100, null_probability);
    ValidateSum<TypeParam>(*array);
    array = rand.Numeric<TypeParam>(length, min + 100, min + 200, null_probability);
    ValidateSum<TypeParam>(*array);
  }
}

TYPED_TEST(TestRandomNumericSumKernel, RandomSliceArraySum) {
  auto arithmetic = ArrayFromJSON(TypeTraits<TypeParam>::type_singleton(),
                                  "[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16]");
  ValidateSum<TypeParam>(*arithmetic);
  for (size_t i = 1; i < 15; i++) {
    auto slice = arithmetic->Slice(i, 16);
    ValidateSum<TypeParam>(*slice);
  }

  // Trigger ConsumeSparse with different slice offsets.
  auto rand = random::RandomArrayGenerator(0xfa432643);
  const int64_t length = 1U << 5;
  auto array = rand.Numeric<TypeParam>(length, 0, 10, 0.5);
  for (size_t i = 1; i < 16; i++) {
    for (size_t j = 1; j < 16; j++) {
      auto slice = array->Slice(i, length - j);
      ValidateSum<TypeParam>(*slice);
    }
  }
}

//
// Count
//

using CountPair = std::pair<int64_t, int64_t>;

static CountPair NaiveCount(const Array& array) {
  CountPair count;

  count.first = array.length() - array.null_count();
  count.second = array.null_count();

  return count;
}

void ValidateCount(const Array& input, CountPair expected) {
  CountOptions all = CountOptions(CountOptions::COUNT_NON_NULL);
  CountOptions nulls = CountOptions(CountOptions::COUNT_NULL);

  ASSERT_OK_AND_ASSIGN(Datum result, Count(input, all));
  AssertDatumsEqual(result, Datum(expected.first));

  ASSERT_OK_AND_ASSIGN(result, Count(input, nulls));
  AssertDatumsEqual(result, Datum(expected.second));
}

template <typename ArrowType>
void ValidateCount(const char* json, CountPair expected) {
  auto array = ArrayFromJSON(TypeTraits<ArrowType>::type_singleton(), json);
  ValidateCount(*array, expected);
}

void ValidateCount(const Array& input) { ValidateCount(input, NaiveCount(input)); }

template <typename ArrowType>
class TestCountKernel : public ::testing::Test {};

TYPED_TEST_SUITE(TestCountKernel, NumericArrowTypes);
TYPED_TEST(TestCountKernel, SimpleCount) {
  ValidateCount<TypeParam>("[]", {0, 0});
  ValidateCount<TypeParam>("[null]", {0, 1});
  ValidateCount<TypeParam>("[1, null, 2]", {2, 1});
  ValidateCount<TypeParam>("[null, null, null]", {0, 3});
  ValidateCount<TypeParam>("[1, 2, 3, 4, 5, 6, 7, 8, 9]", {9, 0});
}

template <typename ArrowType>
class TestRandomNumericCountKernel : public ::testing::Test {};

TYPED_TEST_SUITE(TestRandomNumericCountKernel, NumericArrowTypes);
TYPED_TEST(TestRandomNumericCountKernel, RandomArrayCount) {
  auto rand = random::RandomArrayGenerator(0x1205643);
  for (size_t i = 3; i < 10; i++) {
    for (auto null_probability : {0.0, 0.01, 0.1, 0.25, 0.5, 1.0}) {
      for (auto length_adjust : {-2, -1, 0, 1, 2}) {
        int64_t length = (1UL << i) + length_adjust;
        auto array = rand.Numeric<TypeParam>(length, 0, 100, null_probability);
        ValidateCount(*array);
      }
    }
  }
}

//
// Mean
//

template <typename ArrowType>
static Datum NaiveMean(const Array& array) {
  using MeanScalarType = typename TypeTraits<DoubleType>::ScalarType;

  const auto result = NaiveSumPartial<ArrowType>(array);
  const double mean = static_cast<double>(result.first) /
                      static_cast<double>(result.second ? result.second : 1UL);
  const bool is_valid = result.second > 0;

  if (!is_valid) return Datum(std::make_shared<MeanScalarType>());
  return Datum(std::make_shared<MeanScalarType>(mean));
}

template <typename ArrowType>
void ValidateMean(const Array& input, Datum expected) {
  using OutputType = typename FindAccumulatorType<DoubleType>::Type;

  ASSERT_OK_AND_ASSIGN(Datum result, Mean(input));
  DatumEqual<OutputType>::EnsureEqual(result, expected);
}

template <typename ArrowType>
void ValidateMean(const char* json, Datum expected) {
  auto array = ArrayFromJSON(TypeTraits<ArrowType>::type_singleton(), json);
  ValidateMean<ArrowType>(*array, expected);
}

template <typename ArrowType>
void ValidateMean(const Array& array) {
  ValidateMean<ArrowType>(array, NaiveMean<ArrowType>(array));
}

template <typename ArrowType>
class TestMeanKernelNumeric : public ::testing::Test {};

TYPED_TEST_SUITE(TestMeanKernelNumeric, NumericArrowTypes);
TYPED_TEST(TestMeanKernelNumeric, SimpleMean) {
  using ScalarType = typename TypeTraits<DoubleType>::ScalarType;

  ValidateMean<TypeParam>("[]", Datum(std::make_shared<ScalarType>()));

  ValidateMean<TypeParam>("[null]", Datum(std::make_shared<ScalarType>()));

  ValidateMean<TypeParam>("[1, null, 1]", Datum(std::make_shared<ScalarType>(1.0)));

  ValidateMean<TypeParam>("[1, 2, 3, 4, 5, 6, 7, 8]",
                          Datum(std::make_shared<ScalarType>(4.5)));

  ValidateMean<TypeParam>("[0, 0, 0, 0, 0, 0, 0, 0]",
                          Datum(std::make_shared<ScalarType>(0.0)));

  ValidateMean<TypeParam>("[1, 1, 1, 1, 1, 1, 1, 1]",
                          Datum(std::make_shared<ScalarType>(1.0)));
}

template <typename ArrowType>
class TestRandomNumericMeanKernel : public ::testing::Test {};

TYPED_TEST_SUITE(TestRandomNumericMeanKernel, NumericArrowTypes);
TYPED_TEST(TestRandomNumericMeanKernel, RandomArrayMean) {
  auto rand = random::RandomArrayGenerator(0x8afc055);
  // Test size up to 1<<13 (8192).
  for (size_t i = 3; i < 14; i += 2) {
    for (auto null_probability : {0.0, 0.001, 0.1, 0.5, 0.999, 1.0}) {
      for (auto length_adjust : {-2, -1, 0, 1, 2}) {
        int64_t length = (1UL << i) + length_adjust;
        auto array = rand.Numeric<TypeParam>(length, 0, 100, null_probability);
        ValidateMean<TypeParam>(*array);
      }
    }
  }
}

TYPED_TEST_SUITE(TestRandomNumericMeanKernel, NumericArrowTypes);
TYPED_TEST(TestRandomNumericMeanKernel, RandomArrayMeanOverflow) {
  using CType = typename TypeParam::c_type;
  using SumCType = typename FindAccumulatorType<TypeParam>::Type::c_type;
  if (sizeof(CType) == sizeof(SumCType)) {
    // Skip if accumulator type is same to original type
    return;
  }

  CType max = std::numeric_limits<CType>::max();
  CType min = std::numeric_limits<CType>::min();
  int64_t length = 1024;

  auto rand = random::RandomArrayGenerator(0x8afc055);
  for (auto null_probability : {0.0, 0.1, 0.5, 1.0}) {
    // Test overflow on the original type
    auto array = rand.Numeric<TypeParam>(length, max - 200, max - 100, null_probability);
    ValidateMean<TypeParam>(*array);
    array = rand.Numeric<TypeParam>(length, min + 100, min + 200, null_probability);
    ValidateMean<TypeParam>(*array);
  }
}

//
// Min / Max
//

template <typename ArrowType>
class TestPrimitiveMinMaxKernel : public ::testing::Test {
  using Traits = TypeTraits<ArrowType>;
  using ArrayType = typename Traits::ArrayType;
  using c_type = typename ArrowType::c_type;
  using ScalarType = typename Traits::ScalarType;

 public:
  void AssertMinMaxIs(const Datum& array, c_type expected_min, c_type expected_max,
                      const MinMaxOptions& options) {
    ASSERT_OK_AND_ASSIGN(Datum out, MinMax(array, options));
    const StructScalar& value = out.scalar_as<StructScalar>();

    const auto& out_min = checked_cast<const ScalarType&>(*value.value[0]);
    ASSERT_EQ(expected_min, out_min.value);

    const auto& out_max = checked_cast<const ScalarType&>(*value.value[1]);
    ASSERT_EQ(expected_max, out_max.value);
  }

  void AssertMinMaxIs(const std::string& json, c_type expected_min, c_type expected_max,
                      const MinMaxOptions& options) {
    auto array = ArrayFromJSON(type_singleton(), json);
    AssertMinMaxIs(array, expected_min, expected_max, options);
  }

  void AssertMinMaxIs(const std::vector<std::string>& json, c_type expected_min,
                      c_type expected_max, const MinMaxOptions& options) {
    auto array = ChunkedArrayFromJSON(type_singleton(), json);
    AssertMinMaxIs(array, expected_min, expected_max, options);
  }

  void AssertMinMaxIsNull(const Datum& array, const MinMaxOptions& options) {
    ASSERT_OK_AND_ASSIGN(Datum out, MinMax(array, options));

    const StructScalar& value = out.scalar_as<StructScalar>();
    for (const auto& val : value.value) {
      ASSERT_FALSE(val->is_valid);
    }
  }

  void AssertMinMaxIsNull(const std::string& json, const MinMaxOptions& options) {
    auto array = ArrayFromJSON(type_singleton(), json);
    AssertMinMaxIsNull(array, options);
  }

  void AssertMinMaxIsNull(const std::vector<std::string>& json,
                          const MinMaxOptions& options) {
    auto array = ChunkedArrayFromJSON(type_singleton(), json);
    AssertMinMaxIsNull(array, options);
  }

  std::shared_ptr<DataType> type_singleton() { return Traits::type_singleton(); }
};

template <typename ArrowType>
class TestIntegerMinMaxKernel : public TestPrimitiveMinMaxKernel<ArrowType> {};

template <typename ArrowType>
class TestFloatingMinMaxKernel : public TestPrimitiveMinMaxKernel<ArrowType> {};

class TestBooleanMinMaxKernel : public TestPrimitiveMinMaxKernel<BooleanType> {};

TEST_F(TestBooleanMinMaxKernel, Basics) {
  MinMaxOptions options;
  std::vector<std::string> chunked_input0 = {"[]", "[]"};
  std::vector<std::string> chunked_input1 = {"[true, true, null]", "[true, null]"};
  std::vector<std::string> chunked_input2 = {"[false, false, false]", "[false]"};
  std::vector<std::string> chunked_input3 = {"[true, null]", "[null, false]"};

  // SKIP nulls by default
  this->AssertMinMaxIsNull("[]", options);
  this->AssertMinMaxIsNull("[null, null, null]", options);
  this->AssertMinMaxIs("[false, false, false]", false, false, options);
  this->AssertMinMaxIs("[false, false, false, null]", false, false, options);
  this->AssertMinMaxIs("[true, null, true, true]", true, true, options);
  this->AssertMinMaxIs("[true, null, true, true]", true, true, options);
  this->AssertMinMaxIs("[true, null, false, true]", false, true, options);
  this->AssertMinMaxIsNull(chunked_input0, options);
  this->AssertMinMaxIs(chunked_input1, true, true, options);
  this->AssertMinMaxIs(chunked_input2, false, false, options);
  this->AssertMinMaxIs(chunked_input3, false, true, options);

  options = MinMaxOptions(MinMaxOptions::OUTPUT_NULL);
  this->AssertMinMaxIsNull("[]", options);
  this->AssertMinMaxIsNull("[null, null, null]", options);
  this->AssertMinMaxIsNull("[false, null, false]", options);
  this->AssertMinMaxIsNull("[true, null]", options);
  this->AssertMinMaxIs("[true, true, true]", true, true, options);
  this->AssertMinMaxIs("[false, false]", false, false, options);
  this->AssertMinMaxIs("[false, true]", false, true, options);
  this->AssertMinMaxIsNull(chunked_input0, options);
  this->AssertMinMaxIsNull(chunked_input1, options);
  this->AssertMinMaxIs(chunked_input2, false, false, options);
  this->AssertMinMaxIsNull(chunked_input3, options);
}

TYPED_TEST_SUITE(TestIntegerMinMaxKernel, IntegralArrowTypes);
TYPED_TEST(TestIntegerMinMaxKernel, Basics) {
  MinMaxOptions options;
  std::vector<std::string> chunked_input1 = {"[5, 1, 2, 3, 4]", "[9, 1, null, 3, 4]"};
  std::vector<std::string> chunked_input2 = {"[5, null, 2, 3, 4]", "[9, 1, 2, 3, 4]"};
  std::vector<std::string> chunked_input3 = {"[5, 1, 2, 3, null]", "[9, 1, null, 3, 4]"};

  // SKIP nulls by default
  this->AssertMinMaxIsNull("[]", options);
  this->AssertMinMaxIsNull("[null, null, null]", options);
  this->AssertMinMaxIs("[5, 1, 2, 3, 4]", 1, 5, options);
  this->AssertMinMaxIs("[5, null, 2, 3, 4]", 2, 5, options);
  this->AssertMinMaxIs(chunked_input1, 1, 9, options);
  this->AssertMinMaxIs(chunked_input2, 1, 9, options);
  this->AssertMinMaxIs(chunked_input3, 1, 9, options);

  options = MinMaxOptions(MinMaxOptions::OUTPUT_NULL);
  this->AssertMinMaxIs("[5, 1, 2, 3, 4]", 1, 5, options);
  // output null
  this->AssertMinMaxIsNull("[5, null, 2, 3, 4]", options);
  // output null
  this->AssertMinMaxIsNull(chunked_input1, options);
  this->AssertMinMaxIsNull(chunked_input2, options);
  this->AssertMinMaxIsNull(chunked_input3, options);
}

TYPED_TEST_SUITE(TestFloatingMinMaxKernel, RealArrowTypes);
TYPED_TEST(TestFloatingMinMaxKernel, Floats) {
  MinMaxOptions options;
  std::vector<std::string> chunked_input1 = {"[5, 1, 2, 3, 4]", "[9, 1, null, 3, 4]"};
  std::vector<std::string> chunked_input2 = {"[5, null, 2, 3, 4]", "[9, 1, 2, 3, 4]"};
  std::vector<std::string> chunked_input3 = {"[5, 1, 2, 3, null]", "[9, 1, null, 3, 4]"};

  this->AssertMinMaxIs("[5, 1, 2, 3, 4]", 1, 5, options);
  this->AssertMinMaxIs("[5, 1, 2, 3, 4]", 1, 5, options);
  this->AssertMinMaxIs("[5, null, 2, 3, 4]", 2, 5, options);
  this->AssertMinMaxIs("[5, Inf, 2, 3, 4]", 2.0, INFINITY, options);
  this->AssertMinMaxIs("[5, NaN, 2, 3, 4]", 2, 5, options);
  this->AssertMinMaxIs("[5, -Inf, 2, 3, 4]", -INFINITY, 5, options);
  this->AssertMinMaxIs(chunked_input1, 1, 9, options);
  this->AssertMinMaxIs(chunked_input2, 1, 9, options);
  this->AssertMinMaxIs(chunked_input3, 1, 9, options);

  options = MinMaxOptions(MinMaxOptions::OUTPUT_NULL);
  this->AssertMinMaxIs("[5, 1, 2, 3, 4]", 1, 5, options);
  this->AssertMinMaxIs("[5, -Inf, 2, 3, 4]", -INFINITY, 5, options);
  // output null
  this->AssertMinMaxIsNull("[5, null, 2, 3, 4]", options);
  // output null
  this->AssertMinMaxIsNull("[5, -Inf, null, 3, 4]", options);
  // output null
  this->AssertMinMaxIsNull(chunked_input1, options);
  this->AssertMinMaxIsNull(chunked_input2, options);
  this->AssertMinMaxIsNull(chunked_input3, options);
}

TYPED_TEST(TestFloatingMinMaxKernel, DefaultOptions) {
  auto values = ArrayFromJSON(this->type_singleton(), "[0, 1, 2, 3, 4]");

  ASSERT_OK_AND_ASSIGN(auto no_options_provided, CallFunction("min_max", {values}));

  auto default_options = MinMaxOptions::Defaults();
  ASSERT_OK_AND_ASSIGN(auto explicit_defaults,
                       CallFunction("min_max", {values}, &default_options));

  AssertDatumsEqual(explicit_defaults, no_options_provided);
}

template <typename ArrowType>
struct MinMaxResult {
  using T = typename ArrowType::c_type;

  T min = 0;
  T max = 0;
  bool is_valid = false;
};

template <typename ArrowType>
static enable_if_integer<ArrowType, MinMaxResult<ArrowType>> NaiveMinMax(
    const Array& array) {
  using T = typename ArrowType::c_type;
  using ArrayType = typename TypeTraits<ArrowType>::ArrayType;

  MinMaxResult<ArrowType> result;

  const auto& array_numeric = reinterpret_cast<const ArrayType&>(array);
  const auto values = array_numeric.raw_values();

  if (array.length() <= array.null_count()) {  // All null values
    return result;
  }

  T min = std::numeric_limits<T>::max();
  T max = std::numeric_limits<T>::min();
  if (array.null_count() != 0) {  // Some values are null
    internal::BitmapReader reader(array.null_bitmap_data(), array.offset(),
                                  array.length());
    for (int64_t i = 0; i < array.length(); i++) {
      if (reader.IsSet()) {
        min = std::min(min, values[i]);
        max = std::max(max, values[i]);
      }
      reader.Next();
    }
  } else {  // All true values
    for (int64_t i = 0; i < array.length(); i++) {
      min = std::min(min, values[i]);
      max = std::max(max, values[i]);
    }
  }

  result.min = min;
  result.max = max;
  result.is_valid = true;
  return result;
}

template <typename ArrowType>
static enable_if_floating_point<ArrowType, MinMaxResult<ArrowType>> NaiveMinMax(
    const Array& array) {
  using T = typename ArrowType::c_type;
  using ArrayType = typename TypeTraits<ArrowType>::ArrayType;

  MinMaxResult<ArrowType> result;

  const auto& array_numeric = reinterpret_cast<const ArrayType&>(array);
  const auto values = array_numeric.raw_values();

  if (array.length() <= array.null_count()) {  // All null values
    return result;
  }

  T min = std::numeric_limits<T>::infinity();
  T max = -std::numeric_limits<T>::infinity();
  if (array.null_count() != 0) {  // Some values are null
    internal::BitmapReader reader(array.null_bitmap_data(), array.offset(),
                                  array.length());
    for (int64_t i = 0; i < array.length(); i++) {
      if (reader.IsSet()) {
        min = std::fmin(min, values[i]);
        max = std::fmax(max, values[i]);
      }
      reader.Next();
    }
  } else {  // All true values
    for (int64_t i = 0; i < array.length(); i++) {
      min = std::fmin(min, values[i]);
      max = std::fmax(max, values[i]);
    }
  }

  result.min = min;
  result.max = max;
  result.is_valid = true;
  return result;
}

template <typename ArrowType>
void ValidateMinMax(const Array& array) {
  using Traits = TypeTraits<ArrowType>;
  using ScalarType = typename Traits::ScalarType;

  ASSERT_OK_AND_ASSIGN(Datum out, MinMax(array));
  const StructScalar& value = out.scalar_as<StructScalar>();

  auto expected = NaiveMinMax<ArrowType>(array);
  const auto& out_min = checked_cast<const ScalarType&>(*value.value[0]);
  const auto& out_max = checked_cast<const ScalarType&>(*value.value[1]);

  if (expected.is_valid) {
    ASSERT_TRUE(out_min.is_valid);
    ASSERT_TRUE(out_max.is_valid);
    ASSERT_EQ(expected.min, out_min.value);
    ASSERT_EQ(expected.max, out_max.value);
  } else {  // All null values
    ASSERT_FALSE(out_min.is_valid);
    ASSERT_FALSE(out_max.is_valid);
  }
}

template <typename ArrowType>
class TestRandomNumericMinMaxKernel : public ::testing::Test {};

TYPED_TEST_SUITE(TestRandomNumericMinMaxKernel, NumericArrowTypes);
TYPED_TEST(TestRandomNumericMinMaxKernel, RandomArrayMinMax) {
  auto rand = random::RandomArrayGenerator(0x8afc055);
  // Test size up to 1<<11 (2048).
  for (size_t i = 3; i < 12; i += 2) {
    for (auto null_probability : {0.0, 0.01, 0.1, 0.5, 0.99, 1.0}) {
      int64_t base_length = (1UL << i) + 2;
      auto array = rand.Numeric<TypeParam>(base_length, 0, 100, null_probability);
      for (auto length_adjust : {-2, -1, 0, 1, 2}) {
        int64_t length = (1UL << i) + length_adjust;
        ValidateMinMax<TypeParam>(*array->Slice(0, length));
      }
    }
  }
}

//
// Mode
//

template <typename T>
class TestPrimitiveModeKernel : public ::testing::Test {
 public:
  using ArrowType = T;
  using Traits = TypeTraits<ArrowType>;
  using c_type = typename ArrowType::c_type;
  using ModeType = typename Traits::ScalarType;
  using CountType = typename TypeTraits<Int64Type>::ScalarType;

  void AssertModeIs(const Datum& array, c_type expected_mode, int64_t expected_count) {
    ASSERT_OK_AND_ASSIGN(Datum out, Mode(array));
    const StructScalar& value = out.scalar_as<StructScalar>();

    const auto& out_mode = checked_cast<const ModeType&>(*value.value[0]);
    ASSERT_EQ(expected_mode, out_mode.value);

    const auto& out_count = checked_cast<const CountType&>(*value.value[1]);
    ASSERT_EQ(expected_count, out_count.value);
  }

  void AssertModeIs(const std::string& json, c_type expected_mode,
                    int64_t expected_count) {
    auto array = ArrayFromJSON(type_singleton(), json);
    AssertModeIs(array, expected_mode, expected_count);
  }

  void AssertModeIsNull(const Datum& array) {
    ASSERT_OK_AND_ASSIGN(Datum out, Mode(array));
    const StructScalar& value = out.scalar_as<StructScalar>();

    for (const auto& val : value.value) {
      ASSERT_FALSE(val->is_valid);
    }
  }

  void AssertModeIsNull(const std::string& json) {
    auto array = ArrayFromJSON(type_singleton(), json);
    AssertModeIsNull(array);
  }

  void AssertModeIsNaN(const Datum& array, int64_t expected_count) {
    ASSERT_OK_AND_ASSIGN(Datum out, Mode(array));
    const StructScalar& value = out.scalar_as<StructScalar>();

    const auto& out_mode = checked_cast<const ModeType&>(*value.value[0]);
    ASSERT_NE(out_mode.value, out_mode.value);  // NaN != NaN

    const auto& out_count = checked_cast<const CountType&>(*value.value[1]);
    ASSERT_EQ(expected_count, out_count.value);
  }

  void AssertModeIsNaN(const std::string& json, int64_t expected_count) {
    auto array = ArrayFromJSON(type_singleton(), json);
    AssertModeIsNaN(array, expected_count);
  }

  std::shared_ptr<DataType> type_singleton() { return Traits::type_singleton(); }
};

template <typename ArrowType>
class TestIntegerModeKernel : public TestPrimitiveModeKernel<ArrowType> {};

template <typename ArrowType>
class TestFloatingModeKernel : public TestPrimitiveModeKernel<ArrowType> {};

class TestBooleanModeKernel : public TestPrimitiveModeKernel<BooleanType> {};

class TestInt8ModeKernelValueRange : public TestPrimitiveModeKernel<Int8Type> {};

class TestInt32ModeKernel : public TestPrimitiveModeKernel<Int32Type> {};

TEST_F(TestBooleanModeKernel, Basics) {
  this->AssertModeIs("[false, false]", false, 2);
  this->AssertModeIs("[false, false, true, true, true]", true, 3);
  this->AssertModeIs("[true, false, false, true, true]", true, 3);
  this->AssertModeIs("[false, false, true, true, true, false]", false, 3);

  this->AssertModeIs("[true, null, false, false, null, true, null, null, true]", true, 3);
  this->AssertModeIsNull("[null, null, null]");
  this->AssertModeIsNull("[]");
}

TYPED_TEST_SUITE(TestIntegerModeKernel, IntegralArrowTypes);
TYPED_TEST(TestIntegerModeKernel, Basics) {
  this->AssertModeIs("[5, 1, 1, 5, 5]", 5, 3);
  this->AssertModeIs("[5, 1, 1, 5, 5, 1]", 1, 3);
  this->AssertModeIs("[127, 0, 127, 127, 0, 1, 0, 127]", 127, 4);

  this->AssertModeIs("[null, null, 2, null, 1]", 1, 1);
  this->AssertModeIsNull("[null, null, null]");
  this->AssertModeIsNull("[]");
}

TYPED_TEST_SUITE(TestFloatingModeKernel, RealArrowTypes);
TYPED_TEST(TestFloatingModeKernel, Floats) {
  this->AssertModeIs("[5, 1, 1, 5, 5]", 5, 3);
  this->AssertModeIs("[5, 1, 1, 5, 5, 1]", 1, 3);
  this->AssertModeIs("[Inf, 100, Inf, 100, Inf]", INFINITY, 3);
  this->AssertModeIs("[Inf, -Inf, Inf, -Inf]", -INFINITY, 2);

  this->AssertModeIs("[null, null, 2, null, 1]", 1, 1);
  this->AssertModeIs("[NaN, NaN, 1, null, 1]", 1, 2);

  this->AssertModeIsNull("[null, null, null]");
  this->AssertModeIsNull("[]");

  this->AssertModeIsNaN("[NaN, NaN, 1]", 2);
  this->AssertModeIsNaN("[NaN, NaN, null]", 2);
  this->AssertModeIsNaN("[NaN, NaN, NaN]", 3);
}

TEST_F(TestInt8ModeKernelValueRange, Basics) {
  this->AssertModeIs("[0, 127, -128, -128]", -128, 2);
  this->AssertModeIs("[127, 127, 127]", 127, 3);
}

template <typename ArrowType>
struct ModeResult {
  using T = typename ArrowType::c_type;

  T mode = std::numeric_limits<T>::min();
  int64_t count = 0;
};

template <typename ArrowType>
ModeResult<ArrowType> NaiveMode(const Array& array) {
  using ArrayType = typename TypeTraits<ArrowType>::ArrayType;
  using CTYPE = typename ArrowType::c_type;

  std::unordered_map<CTYPE, int64_t> value_counts;

  const auto& array_numeric = reinterpret_cast<const ArrayType&>(array);
  const auto values = array_numeric.raw_values();
  internal::BitmapReader reader(array.null_bitmap_data(), array.offset(), array.length());
  for (int64_t i = 0; i < array.length(); ++i) {
    if (reader.IsSet()) {
      ++value_counts[values[i]];
    }
    reader.Next();
  }

  ModeResult<ArrowType> result;
  for (const auto& value_count : value_counts) {
    auto value = value_count.first;
    auto count = value_count.second;
    if (count > result.count || (count == result.count && value < result.mode)) {
      result.count = count;
      result.mode = value;
    }
  }

  return result;
}

template <typename ArrowType, typename CTYPE = typename ArrowType::c_type>
void CheckModeWithRange(CTYPE range_min, CTYPE range_max) {
  using ModeScalar = typename TypeTraits<ArrowType>::ScalarType;
  using CountScalar = typename TypeTraits<Int64Type>::ScalarType;

  auto rand = random::RandomArrayGenerator(0x5487655);
  // 32K items (>= counting mode cutoff) within range, 10% null
  auto array = rand.Numeric<ArrowType>(32 * 1024, range_min, range_max, 0.1);

  auto expected = NaiveMode<ArrowType>(*array);
  ASSERT_OK_AND_ASSIGN(Datum out, Mode(array));
  const StructScalar& value = out.scalar_as<StructScalar>();

  ASSERT_TRUE(value.is_valid);
  const auto& out_mode = checked_cast<const ModeScalar&>(*value.value[0]);
  const auto& out_count = checked_cast<const CountScalar&>(*value.value[1]);
  ASSERT_EQ(out_mode.value, expected.mode);
  ASSERT_EQ(out_count.value, expected.count);
}

TEST_F(TestInt32ModeKernel, SmallValueRange) {
  // Small value range => should exercise counter-based Mode implementation
  CheckModeWithRange<ArrowType>(-100, 100);
}

TEST_F(TestInt32ModeKernel, LargeValueRange) {
  // Large value range => should exercise hashmap-based Mode implementation
  CheckModeWithRange<ArrowType>(-10000000, 10000000);
}

}  // namespace compute
}  // namespace arrow
