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

#include <cstdint>
#include <vector>

#include "benchmark/benchmark.h"
#include "parquet/level_conversion.h"

constexpr int64_t kLevelCount = 2048;
// Def level indicating the element is missing from the leaf
// array (a parent repeated element was empty).
constexpr int16_t kMissingDefLevel = 0;

// Definition Level indicating the values has an entry in the leaf element.
constexpr int16_t kPresentDefLevel = 2;

// A repition level that indicates a repeated element.
constexpr int16_t kHasRepeatedElements = 1;

std::vector<uint8_t> RunDefinitionLevelsToBitmap(const std::vector<int16_t>& def_levels,
                                                 ::benchmark::State* state) {
  int64_t values_read = 0;
  int64_t null_count = 0;
  std::vector<uint8_t> bitmap(/*count=*/def_levels.size(), 0);
  int rep = 0;
  for (auto _ : *state) {
    parquet::internal::DefinitionLevelsToBitmap(
        def_levels.data(), def_levels.size(), /*max_definition_level=*/kPresentDefLevel,
        /*max_repetition_level=*/kHasRepeatedElements, &values_read, &null_count,
        bitmap.data(), /*valid_bits_offset=*/(rep++ % 8) * def_levels.size());
  }
  state->SetBytesProcessed(int64_t(state->iterations()) * def_levels.size());
  return bitmap;
}

void BM_DefinitionLevelsToBitmapRepeatedAllMissing(::benchmark::State& state) {
  std::vector<int16_t> def_levels(/*count=*/kLevelCount, kMissingDefLevel);
  auto result = RunDefinitionLevelsToBitmap(def_levels, &state);
  ::benchmark::DoNotOptimize(result);
}

BENCHMARK(BM_DefinitionLevelsToBitmapRepeatedAllMissing);

void BM_DefinitionLevelsToBitmapRepeatedAllPresent(::benchmark::State& state) {
  std::vector<int16_t> def_levels(/*count=*/kLevelCount, kPresentDefLevel);
  auto result = RunDefinitionLevelsToBitmap(def_levels, &state);
  ::benchmark::DoNotOptimize(result);
}

BENCHMARK(BM_DefinitionLevelsToBitmapRepeatedAllPresent);

void BM_DefinitionLevelsToBitmapRepeatedMostPresent(::benchmark::State& state) {
  std::vector<int16_t> def_levels(/*count=*/kLevelCount, kPresentDefLevel);
  for (size_t x = 0; x < def_levels.size(); x++) {
    if (x % 10 == 0) {
      def_levels[x] = kMissingDefLevel;
    }
  }
  auto result = RunDefinitionLevelsToBitmap(def_levels, &state);
  ::benchmark::DoNotOptimize(result);
}

BENCHMARK(BM_DefinitionLevelsToBitmapRepeatedMostPresent);
