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

#include <cmath>
#include <locale>
#include <stdexcept>
#include <string>

#include <gtest/gtest.h>

#include "arrow/status.h"
#include "arrow/testing/gtest_util.h"
#include "arrow/type.h"
#include "arrow/util/formatting.h"

namespace arrow {

using internal::StringFormatter;

class StringAppender {
 public:
  Status operator()(util::string_view v) {
    string_.append(v.data(), v.size());
    return Status::OK();
  }

  std::string string() const { return string_; }

 protected:
  std::string string_;
};

template <typename FormatterType, typename C_TYPE = typename FormatterType::value_type>
void AssertFormatting(FormatterType& formatter, C_TYPE value,
                      const std::string& expected) {
  StringAppender appender;
  ASSERT_OK(formatter(value, appender));
  ASSERT_EQ(appender.string(), expected) << "Formatting failed (value = " << value << ")";
}

TEST(Formatting, Boolean) {
  StringFormatter<BooleanType> formatter;

  AssertFormatting(formatter, true, "true");
  AssertFormatting(formatter, false, "false");
}

template <typename FormatterType>
void TestAnyIntUpTo8(FormatterType& formatter) {
  AssertFormatting(formatter, 0, "0");
  AssertFormatting(formatter, 1, "1");
  AssertFormatting(formatter, 9, "9");
  AssertFormatting(formatter, 10, "10");
  AssertFormatting(formatter, 99, "99");
  AssertFormatting(formatter, 100, "100");
  AssertFormatting(formatter, 127, "127");
}

template <typename FormatterType>
void TestAnyIntUpTo16(FormatterType& formatter) {
  TestAnyIntUpTo8(formatter);
  AssertFormatting(formatter, 999, "999");
  AssertFormatting(formatter, 1000, "1000");
  AssertFormatting(formatter, 9999, "9999");
  AssertFormatting(formatter, 10000, "10000");
  AssertFormatting(formatter, 32767, "32767");
}

template <typename FormatterType>
void TestAnyIntUpTo32(FormatterType& formatter) {
  TestAnyIntUpTo16(formatter);
  AssertFormatting(formatter, 99999, "99999");
  AssertFormatting(formatter, 100000, "100000");
  AssertFormatting(formatter, 999999, "999999");
  AssertFormatting(formatter, 1000000, "1000000");
  AssertFormatting(formatter, 9999999, "9999999");
  AssertFormatting(formatter, 10000000, "10000000");
  AssertFormatting(formatter, 99999999, "99999999");
  AssertFormatting(formatter, 100000000, "100000000");
  AssertFormatting(formatter, 999999999, "999999999");
  AssertFormatting(formatter, 1000000000, "1000000000");
  AssertFormatting(formatter, 1234567890, "1234567890");
  AssertFormatting(formatter, 2147483647, "2147483647");
}

template <typename FormatterType>
void TestAnyIntUpTo64(FormatterType& formatter) {
  TestAnyIntUpTo32(formatter);
  AssertFormatting(formatter, 9999999999ULL, "9999999999");
  AssertFormatting(formatter, 10000000000ULL, "10000000000");
  AssertFormatting(formatter, 99999999999ULL, "99999999999");
  AssertFormatting(formatter, 100000000000ULL, "100000000000");
  AssertFormatting(formatter, 999999999999ULL, "999999999999");
  AssertFormatting(formatter, 1000000000000ULL, "1000000000000");
  AssertFormatting(formatter, 9999999999999ULL, "9999999999999");
  AssertFormatting(formatter, 10000000000000ULL, "10000000000000");
  AssertFormatting(formatter, 99999999999999ULL, "99999999999999");
  AssertFormatting(formatter, 1000000000000000000ULL, "1000000000000000000");
  AssertFormatting(formatter, 9223372036854775807ULL, "9223372036854775807");
}

template <typename FormatterType>
void TestUIntUpTo8(FormatterType& formatter) {
  TestAnyIntUpTo8(formatter);
  AssertFormatting(formatter, 128, "128");
  AssertFormatting(formatter, 255, "255");
}

template <typename FormatterType>
void TestUIntUpTo16(FormatterType& formatter) {
  TestAnyIntUpTo16(formatter);
  AssertFormatting(formatter, 32768, "32768");
  AssertFormatting(formatter, 65535, "65535");
}

template <typename FormatterType>
void TestUIntUpTo32(FormatterType& formatter) {
  TestAnyIntUpTo32(formatter);
  AssertFormatting(formatter, 2147483648U, "2147483648");
  AssertFormatting(formatter, 4294967295U, "4294967295");
}

template <typename FormatterType>
void TestUIntUpTo64(FormatterType& formatter) {
  TestAnyIntUpTo64(formatter);
  AssertFormatting(formatter, 9999999999999999999ULL, "9999999999999999999");
  AssertFormatting(formatter, 10000000000000000000ULL, "10000000000000000000");
  AssertFormatting(formatter, 12345678901234567890ULL, "12345678901234567890");
  AssertFormatting(formatter, 18446744073709551615ULL, "18446744073709551615");
}

TEST(Formatting, UInt8) {
  StringFormatter<UInt8Type> formatter;

  TestUIntUpTo8(formatter);
}

TEST(Formatting, UInt16) {
  StringFormatter<UInt16Type> formatter;

  TestUIntUpTo16(formatter);
}

TEST(Formatting, UInt32) {
  StringFormatter<UInt32Type> formatter;

  TestUIntUpTo32(formatter);
}

TEST(Formatting, UInt64) {
  StringFormatter<UInt64Type> formatter;

  TestUIntUpTo64(formatter);
}

template <typename FormatterType>
void TestIntUpTo8(FormatterType& formatter) {
  TestAnyIntUpTo8(formatter);
  AssertFormatting(formatter, -1, "-1");
  AssertFormatting(formatter, -9, "-9");
  AssertFormatting(formatter, -10, "-10");
  AssertFormatting(formatter, -99, "-99");
  AssertFormatting(formatter, -100, "-100");
  AssertFormatting(formatter, -127, "-127");
  AssertFormatting(formatter, -128, "-128");
}

template <typename FormatterType>
void TestIntUpTo16(FormatterType& formatter) {
  TestAnyIntUpTo16(formatter);
  TestIntUpTo8(formatter);
  AssertFormatting(formatter, -129, "-129");
  AssertFormatting(formatter, -999, "-999");
  AssertFormatting(formatter, -1000, "-1000");
  AssertFormatting(formatter, -9999, "-9999");
  AssertFormatting(formatter, -10000, "-10000");
  AssertFormatting(formatter, -32768, "-32768");
}

template <typename FormatterType>
void TestIntUpTo32(FormatterType& formatter) {
  TestAnyIntUpTo32(formatter);
  TestIntUpTo16(formatter);
  AssertFormatting(formatter, -32769, "-32769");
  AssertFormatting(formatter, -99999, "-99999");
  AssertFormatting(formatter, -1000000000, "-1000000000");
  AssertFormatting(formatter, -1234567890, "-1234567890");
  AssertFormatting(formatter, -2147483647, "-2147483647");
  AssertFormatting(formatter, -2147483647 - 1, "-2147483648");
}

template <typename FormatterType>
void TestIntUpTo64(FormatterType& formatter) {
  TestAnyIntUpTo64(formatter);
  TestIntUpTo32(formatter);
  AssertFormatting(formatter, -2147483649LL, "-2147483649");
  AssertFormatting(formatter, -9999999999LL, "-9999999999");
  AssertFormatting(formatter, -1000000000000000000LL, "-1000000000000000000");
  AssertFormatting(formatter, -9012345678901234567LL, "-9012345678901234567");
  AssertFormatting(formatter, -9223372036854775807LL, "-9223372036854775807");
  AssertFormatting(formatter, -9223372036854775807LL - 1, "-9223372036854775808");
}

TEST(Formatting, Int8) {
  StringFormatter<Int8Type> formatter;

  TestIntUpTo8(formatter);
}

TEST(Formatting, Int16) {
  StringFormatter<Int16Type> formatter;

  TestIntUpTo16(formatter);
}

TEST(Formatting, Int32) {
  StringFormatter<Int32Type> formatter;

  TestIntUpTo32(formatter);
}

TEST(Formatting, Int64) {
  StringFormatter<Int64Type> formatter;

  TestIntUpTo64(formatter);
}

TEST(Formatting, Float) {
  StringFormatter<FloatType> formatter;

  AssertFormatting(formatter, 0.0f, "0");
  AssertFormatting(formatter, -0.0f, "-0");
  AssertFormatting(formatter, 1.5f, "1.5");
  AssertFormatting(formatter, 0.0001f, "0.0001");
  AssertFormatting(formatter, 1234.567f, "1234.567");
  AssertFormatting(formatter, 1e9f, "1000000000");
  AssertFormatting(formatter, 1e10f, "1e+10");
  AssertFormatting(formatter, 1e20f, "1e+20");
  AssertFormatting(formatter, 1e-6f, "0.000001");
  AssertFormatting(formatter, 1e-7f, "1e-7");
  AssertFormatting(formatter, 1e-20f, "1e-20");

  AssertFormatting(formatter, std::nanf(""), "nan");
  AssertFormatting(formatter, HUGE_VALF, "inf");
  AssertFormatting(formatter, -HUGE_VALF, "-inf");
}

TEST(Formatting, Double) {
  StringFormatter<DoubleType> formatter;

  AssertFormatting(formatter, 0.0, "0");
  AssertFormatting(formatter, -0.0, "-0");
  AssertFormatting(formatter, 1.5, "1.5");
  AssertFormatting(formatter, 0.0001, "0.0001");
  AssertFormatting(formatter, 1234.567, "1234.567");
  AssertFormatting(formatter, 1e9, "1000000000");
  AssertFormatting(formatter, 1e10, "1e+10");
  AssertFormatting(formatter, 1e20, "1e+20");
  AssertFormatting(formatter, 1e-6, "0.000001");
  AssertFormatting(formatter, 1e-7, "1e-7");
  AssertFormatting(formatter, 1e-20, "1e-20");

  AssertFormatting(formatter, std::nan(""), "nan");
  AssertFormatting(formatter, HUGE_VAL, "inf");
  AssertFormatting(formatter, -HUGE_VAL, "-inf");
}

TEST(Formatting, Date32) {
  StringFormatter<Date32Type> formatter;

  AssertFormatting(formatter, 0, "1970-01-01");
  AssertFormatting(formatter, 1, "1970-01-02");
  AssertFormatting(formatter, 30, "1970-01-31");
  AssertFormatting(formatter, 30 + 1, "1970-02-01");
  AssertFormatting(formatter, 30 + 28, "1970-02-28");
  AssertFormatting(formatter, 30 + 28 + 1, "1970-03-01");
  AssertFormatting(formatter, -1, "1969-12-31");
  AssertFormatting(formatter, 365, "1971-01-01");
  AssertFormatting(formatter, 2 * 365, "1972-01-01");
  AssertFormatting(formatter, 2 * 365 + 30 + 28 + 1, "1972-02-29");
}

TEST(Formatting, Date64) {
  StringFormatter<Date64Type> formatter;

  constexpr int64_t kMillisInDay = 24 * 60 * 60 * 1000;
  AssertFormatting(formatter, kMillisInDay * (0), "1970-01-01");
  AssertFormatting(formatter, kMillisInDay * (1), "1970-01-02");
  AssertFormatting(formatter, kMillisInDay * (30), "1970-01-31");
  AssertFormatting(formatter, kMillisInDay * (30 + 1), "1970-02-01");
  AssertFormatting(formatter, kMillisInDay * (30 + 28), "1970-02-28");
  AssertFormatting(formatter, kMillisInDay * (30 + 28 + 1), "1970-03-01");
  AssertFormatting(formatter, kMillisInDay * (-1), "1969-12-31");
  AssertFormatting(formatter, kMillisInDay * (365), "1971-01-01");
  AssertFormatting(formatter, kMillisInDay * (2 * 365), "1972-01-01");
  AssertFormatting(formatter, kMillisInDay * (2 * 365 + 30 + 28 + 1), "1972-02-29");
}

TEST(Formatting, Time32) {
  {
    StringFormatter<Time32Type> formatter(time32(TimeUnit::SECOND));

    AssertFormatting(formatter, 0, "00:00:00");
    AssertFormatting(formatter, 1, "00:00:01");
    AssertFormatting(formatter, ((12) * 60 + 34) * 60 + 56, "12:34:56");
    AssertFormatting(formatter, 24 * 60 * 60 - 1, "23:59:59");
  }

  {
    StringFormatter<Time32Type> formatter(time32(TimeUnit::MILLI));

    AssertFormatting(formatter, 0, "00:00:00.000");
    AssertFormatting(formatter, 1, "00:00:00.001");
    AssertFormatting(formatter, 1000, "00:00:01.000");
    AssertFormatting(formatter, (((12) * 60 + 34) * 60 + 56) * 1000 + 789,
                     "12:34:56.789");
    AssertFormatting(formatter, 24 * 60 * 60 * 1000 - 1, "23:59:59.999");
  }
}

TEST(Formatting, Time64) {
  {
    StringFormatter<Time64Type> formatter(time64(TimeUnit::MICRO));

    AssertFormatting(formatter, 0, "00:00:00.000000");
    AssertFormatting(formatter, 1, "00:00:00.000001");
    AssertFormatting(formatter, 1000000, "00:00:01.000000");
    AssertFormatting(formatter, (((12) * 60 + 34) * 60 + 56) * 1000000LL + 789000,
                     "12:34:56.789000");
    AssertFormatting(formatter, (24 * 60 * 60) * 1000000LL - 1, "23:59:59.999999");
  }

  {
    StringFormatter<Time64Type> formatter(time64(TimeUnit::NANO));

    AssertFormatting(formatter, 0, "00:00:00.000000000");
    AssertFormatting(formatter, 1, "00:00:00.000000001");
    AssertFormatting(formatter, 1000000000LL, "00:00:01.000000000");
    AssertFormatting(formatter, (((12) * 60 + 34) * 60 + 56) * 1000000000LL + 789000000LL,
                     "12:34:56.789000000");
    AssertFormatting(formatter, (24 * 60 * 60) * 1000000000LL - 1, "23:59:59.999999999");
  }
}

TEST(Formatting, Timestamp) {
  {
    StringFormatter<TimestampType> formatter(timestamp(TimeUnit::SECOND));

    AssertFormatting(formatter, 0, "1970-01-01 00:00:00");
    AssertFormatting(formatter, 1, "1970-01-01 00:00:01");
    AssertFormatting(formatter, 24 * 60 * 60, "1970-01-02 00:00:00");
    AssertFormatting(formatter, 616377600, "1989-07-14 00:00:00");
    AssertFormatting(formatter, 951782400, "2000-02-29 00:00:00");
    AssertFormatting(formatter, 63730281600LL, "3989-07-14 00:00:00");
    AssertFormatting(formatter, -2203977600LL, "1900-02-28 00:00:00");

    AssertFormatting(formatter, 1542129070, "2018-11-13 17:11:10");
    AssertFormatting(formatter, -2203932304LL, "1900-02-28 12:34:56");
  }

  {
    StringFormatter<TimestampType> formatter(timestamp(TimeUnit::MILLI));

    AssertFormatting(formatter, 0, "1970-01-01 00:00:00.000");
    AssertFormatting(formatter, 1000L + 1, "1970-01-01 00:00:01.001");
    AssertFormatting(formatter, 24 * 60 * 60 * 1000LL + 2, "1970-01-02 00:00:00.002");
    AssertFormatting(formatter, 616377600 * 1000LL + 3, "1989-07-14 00:00:00.003");
    AssertFormatting(formatter, 951782400 * 1000LL + 4, "2000-02-29 00:00:00.004");
    AssertFormatting(formatter, 63730281600LL * 1000LL + 5, "3989-07-14 00:00:00.005");
    AssertFormatting(formatter, -2203977600LL * 1000LL + 6, "1900-02-28 00:00:00.006");

    AssertFormatting(formatter, 1542129070LL * 1000LL + 7, "2018-11-13 17:11:10.007");
    AssertFormatting(formatter, -2203932304LL * 1000LL + 8, "1900-02-28 12:34:56.008");
  }

  {
    StringFormatter<TimestampType> formatter(timestamp(TimeUnit::MICRO));

    AssertFormatting(formatter, 0, "1970-01-01 00:00:00.000000");
    AssertFormatting(formatter, 1000000LL + 1, "1970-01-01 00:00:01.000001");
    AssertFormatting(formatter, 24 * 60 * 60 * 1000000LL + 2,
                     "1970-01-02 00:00:00.000002");
    AssertFormatting(formatter, 616377600 * 1000000LL + 3, "1989-07-14 00:00:00.000003");
    AssertFormatting(formatter, 951782400 * 1000000LL + 4, "2000-02-29 00:00:00.000004");
    AssertFormatting(formatter, 63730281600LL * 1000000LL + 5,
                     "3989-07-14 00:00:00.000005");
    AssertFormatting(formatter, -2203977600LL * 1000000LL + 6,
                     "1900-02-28 00:00:00.000006");

    AssertFormatting(formatter, 1542129070 * 1000000LL + 7, "2018-11-13 17:11:10.000007");
    AssertFormatting(formatter, -2203932304LL * 1000000LL + 8,
                     "1900-02-28 12:34:56.000008");
  }

  {
    StringFormatter<TimestampType> formatter(timestamp(TimeUnit::NANO));

    AssertFormatting(formatter, 0, "1970-01-01 00:00:00.000000000");
    AssertFormatting(formatter, 1000000000LL + 1, "1970-01-01 00:00:01.000000001");
    AssertFormatting(formatter, 24 * 60 * 60 * 1000000000LL + 2,
                     "1970-01-02 00:00:00.000000002");
    AssertFormatting(formatter, 616377600 * 1000000000LL + 3,
                     "1989-07-14 00:00:00.000000003");
    AssertFormatting(formatter, 951782400 * 1000000000LL + 4,
                     "2000-02-29 00:00:00.000000004");
    AssertFormatting(formatter, -2203977600LL * 1000000000LL + 6,
                     "1900-02-28 00:00:00.000000006");

    AssertFormatting(formatter, 1542129070 * 1000000000LL + 7,
                     "2018-11-13 17:11:10.000000007");
    AssertFormatting(formatter, -2203932304LL * 1000000000LL + 8,
                     "1900-02-28 12:34:56.000000008");
  }
}

}  // namespace arrow
