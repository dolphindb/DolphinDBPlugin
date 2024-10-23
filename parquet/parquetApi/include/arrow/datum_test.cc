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

#include <memory>
#include <string>

#include <gtest/gtest.h>

#include "arrow/array/array_base.h"
#include "arrow/chunked_array.h"
#include "arrow/datum.h"
#include "arrow/scalar.h"
#include "arrow/table.h"
#include "arrow/testing/gtest_common.h"
#include "arrow/testing/gtest_util.h"
#include "arrow/type_fwd.h"
#include "arrow/util/checked_cast.h"

namespace arrow {

class BinaryArray;
class RecordBatch;

using internal::checked_cast;

// ----------------------------------------------------------------------
// Datum

template <typename T>
void CheckImplicitConstructor(Datum::Kind expected_kind) {
  std::shared_ptr<T> value;
  Datum datum = value;
  ASSERT_EQ(expected_kind, datum.kind());
}

TEST(Datum, ImplicitConstructors) {
  CheckImplicitConstructor<Scalar>(Datum::SCALAR);

  CheckImplicitConstructor<Array>(Datum::ARRAY);

  // Instantiate from array subclass
  CheckImplicitConstructor<BinaryArray>(Datum::ARRAY);

  CheckImplicitConstructor<ChunkedArray>(Datum::CHUNKED_ARRAY);
  CheckImplicitConstructor<RecordBatch>(Datum::RECORD_BATCH);

  CheckImplicitConstructor<Table>(Datum::TABLE);
}

TEST(Datum, Constructors) {
  Datum val(std::make_shared<Int64Scalar>(1));
  ASSERT_EQ(ValueDescr::SCALAR, val.shape());
  AssertTypeEqual(*int64(), *val.type());
  ASSERT_TRUE(val.is_scalar());
  ASSERT_FALSE(val.is_array());
  ASSERT_EQ(1, val.length());

  const Int64Scalar& val_as_i64 = checked_cast<const Int64Scalar&>(*val.scalar());
  const Int64Scalar& val_as_i64_2 = val.scalar_as<Int64Scalar>();
  ASSERT_EQ(1, val_as_i64.value);
  ASSERT_EQ(1, val_as_i64_2.value);

  auto arr = ArrayFromJSON(int64(), "[1, 2, 3, 4]");
  auto sel_indices = ArrayFromJSON(int32(), "[0, 3]");

  Datum val2(arr);
  ASSERT_EQ(Datum::ARRAY, val2.kind());
  ASSERT_EQ(ValueDescr::ARRAY, val2.shape());
  AssertTypeEqual(*int64(), *val2.type());
  AssertArraysEqual(*arr, *val2.make_array());
  ASSERT_TRUE(val2.is_array());
  ASSERT_FALSE(val2.is_scalar());
  ASSERT_EQ(arr->length(), val2.length());

  auto Check = [&](const Datum& v) { AssertArraysEqual(*arr, *v.make_array()); };

  // Copy constructor
  Datum val3 = val2;
  Check(val3);

  // Copy assignment
  Datum val4;
  val4 = val2;
  Check(val4);

  // Move constructor
  Datum val5 = std::move(val2);
  Check(val5);

  // Move assignment
  Datum val6;
  val6 = std::move(val4);
  Check(val6);
}

TEST(Datum, NullCount) {
  Datum val1(std::make_shared<Int8Scalar>(1));
  ASSERT_EQ(0, val1.null_count());

  Datum val2(MakeNullScalar(int8()));
  ASSERT_EQ(1, val2.null_count());

  Datum val3(ArrayFromJSON(int8(), "[1, null, null, null]"));
  ASSERT_EQ(3, val3.null_count());
}

TEST(Datum, MutableArray) {
  auto arr = ArrayFromJSON(int8(), "[1, 2, 3, 4]");

  Datum val(arr);

  val.mutable_array()->length = 0;
  ASSERT_EQ(0, val.array()->length);
}

TEST(Datum, ToString) {
  auto arr = ArrayFromJSON(int8(), "[1, 2, 3, 4]");

  Datum v1(arr);
  Datum v2(std::make_shared<Int8Scalar>(1));
  ASSERT_EQ("Array", v1.ToString());
  ASSERT_EQ("Scalar", v2.ToString());
}

TEST(ValueDescr, Basics) {
  ValueDescr d1(utf8(), ValueDescr::SCALAR);
  ValueDescr d2 = ValueDescr::Any(utf8());
  ValueDescr d3 = ValueDescr::Scalar(utf8());
  ValueDescr d4 = ValueDescr::Array(utf8());

  ASSERT_EQ(ValueDescr::SCALAR, d1.shape);
  AssertTypeEqual(*utf8(), *d1.type);
  ASSERT_EQ(ValueDescr::Scalar(utf8()), d1);

  ASSERT_EQ(ValueDescr::ANY, d2.shape);
  AssertTypeEqual(*utf8(), *d2.type);
  ASSERT_EQ(ValueDescr::Any(utf8()), d2);
  ASSERT_NE(ValueDescr::Any(int32()), d2);

  ASSERT_EQ(ValueDescr::SCALAR, d3.shape);
  ASSERT_EQ(ValueDescr::ARRAY, d4.shape);

  ASSERT_EQ("scalar[string]", d1.ToString());
  ASSERT_EQ("any[string]", d2.ToString());
  ASSERT_EQ("scalar[string]", d3.ToString());
  ASSERT_EQ("array[string]", d4.ToString());
}

}  // namespace arrow
