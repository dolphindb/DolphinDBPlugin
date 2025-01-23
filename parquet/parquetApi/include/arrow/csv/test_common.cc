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

#include "arrow/csv/test_common.h"
#include "arrow/testing/gtest_util.h"

namespace arrow {
namespace csv {

std::string MakeCSVData(std::vector<std::string> lines) {
  std::string s;
  for (const auto& line : lines) {
    s += line;
  }
  return s;
}

void MakeCSVParser(std::vector<std::string> lines, ParseOptions options, int32_t num_cols,
                   std::shared_ptr<BlockParser>* out) {
  auto csv = MakeCSVData(lines);
  auto parser = std::make_shared<BlockParser>(options, num_cols);
  uint32_t out_size;
  ASSERT_OK(parser->Parse(util::string_view(csv), &out_size));
  ASSERT_EQ(out_size, csv.size()) << "trailing CSV data not parsed";
  *out = parser;
}

void MakeCSVParser(std::vector<std::string> lines, ParseOptions options,
                   std::shared_ptr<BlockParser>* out) {
  return MakeCSVParser(lines, options, -1, out);
}

void MakeCSVParser(std::vector<std::string> lines, std::shared_ptr<BlockParser>* out) {
  MakeCSVParser(lines, ParseOptions::Defaults(), out);
}

void MakeColumnParser(std::vector<std::string> items, std::shared_ptr<BlockParser>* out) {
  auto options = ParseOptions::Defaults();
  // Need this to test for null (empty) values
  options.ignore_empty_lines = false;
  std::vector<std::string> lines;
  for (const auto& item : items) {
    lines.push_back(item + '\n');
  }
  MakeCSVParser(lines, options, 1, out);
  ASSERT_EQ((*out)->num_cols(), 1) << "Should have seen only 1 CSV column";
  ASSERT_EQ((*out)->num_rows(), items.size());
}

}  // namespace csv
}  // namespace arrow
