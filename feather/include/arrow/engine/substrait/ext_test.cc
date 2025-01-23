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

#include "arrow/engine/substrait/extension_set.h"
#include "arrow/engine/substrait/util.h"

#include <google/protobuf/descriptor.h>
#include <google/protobuf/util/json_util.h>
#include <google/protobuf/util/type_resolver_util.h>
#include <gtest/gtest.h>

#include "arrow/testing/gtest_util.h"
#include "arrow/testing/matchers.h"

using testing::ElementsAre;
using testing::Eq;
using testing::HasSubstr;
using testing::UnorderedElementsAre;

namespace arrow {

using internal::checked_cast;

namespace engine {

// an extension-id-registry provider to be used as a test parameter
//
// we cannot pass a pointer to a nested registry as a test parameter because the
// shared_ptr in which it is made would not be held and get destructed too early,
// nor can we pass a shared_ptr to the default nested registry as a test parameter
// because it is global and must never be cleaned up, so we pass a shared_ptr to a
// provider that either owns or does not own the registry it provides, depending
// on the case.
struct ExtensionIdRegistryProvider {
  virtual ExtensionIdRegistry* get() const = 0;
};

struct DefaultExtensionIdRegistryProvider : public ExtensionIdRegistryProvider {
  virtual ~DefaultExtensionIdRegistryProvider() {}
  ExtensionIdRegistry* get() const override { return default_extension_id_registry(); }
};

struct NestedExtensionIdRegistryProvider : public ExtensionIdRegistryProvider {
  virtual ~NestedExtensionIdRegistryProvider() {}
  std::shared_ptr<ExtensionIdRegistry> registry_ = substrait::MakeExtensionIdRegistry();
  ExtensionIdRegistry* get() const override { return &*registry_; }
};

using Id = ExtensionIdRegistry::Id;

bool operator==(const Id& id1, const Id& id2) {
  return id1.uri == id2.uri && id1.name == id2.name;
}

bool operator!=(const Id& id1, const Id& id2) { return !(id1 == id2); }

struct TypeName {
  std::shared_ptr<DataType> type;
  util::string_view name;
};

static const std::vector<TypeName> kTypeNames = {
    TypeName{uint8(), "u8"},
    TypeName{uint16(), "u16"},
    TypeName{uint32(), "u32"},
    TypeName{uint64(), "u64"},
    TypeName{float16(), "fp16"},
    TypeName{null(), "null"},
    TypeName{month_interval(), "interval_month"},
    TypeName{day_time_interval(), "interval_day_milli"},
    TypeName{month_day_nano_interval(), "interval_month_day_nano"},
};

static const std::vector<util::string_view> kFunctionNames = {
    "add",
};

static const std::vector<util::string_view> kTempFunctionNames = {
    "temp_func_1",
    "temp_func_2",
};

static const std::vector<TypeName> kTempTypeNames = {
    TypeName{timestamp(TimeUnit::SECOND, "temp_tz_1"), "temp_type_1"},
    TypeName{timestamp(TimeUnit::SECOND, "temp_tz_2"), "temp_type_2"},
};

static Id kNonExistentId{kArrowExtTypesUri, "non_existent"};
static TypeName kNonExistentTypeName{timestamp(TimeUnit::SECOND, "non_existent_tz_1"),
                                     "non_existent_type_1"};

using ExtensionIdRegistryParams =
    std::tuple<std::shared_ptr<ExtensionIdRegistryProvider>, std::string>;

struct ExtensionIdRegistryTest
    : public testing::TestWithParam<ExtensionIdRegistryParams> {};

TEST_P(ExtensionIdRegistryTest, GetTypes) {
  auto provider = std::get<0>(GetParam());
  auto registry = provider->get();

  for (TypeName e : kTypeNames) {
    auto id = Id{kArrowExtTypesUri, e.name};
    for (auto typerec_opt : {registry->GetType(id), registry->GetType(*e.type)}) {
      ASSERT_TRUE(typerec_opt);
      auto typerec = typerec_opt.value();
      ASSERT_EQ(id, typerec.id);
      ASSERT_EQ(*e.type, *typerec.type);
    }
  }
  ASSERT_FALSE(registry->GetType(kNonExistentId));
  ASSERT_FALSE(registry->GetType(*kNonExistentTypeName.type));
}

TEST_P(ExtensionIdRegistryTest, ReregisterTypes) {
  auto provider = std::get<0>(GetParam());
  auto registry = provider->get();

  for (TypeName e : kTypeNames) {
    auto id = Id{kArrowExtTypesUri, e.name};
    ASSERT_RAISES(Invalid, registry->CanRegisterType(id, e.type));
    ASSERT_RAISES(Invalid, registry->RegisterType(id, e.type));
  }
}

TEST_P(ExtensionIdRegistryTest, GetFunctions) {
  auto provider = std::get<0>(GetParam());
  auto registry = provider->get();

  for (util::string_view name : kFunctionNames) {
    auto id = Id{kArrowExtTypesUri, name};
    for (auto funcrec_opt : {registry->GetFunction(id), registry->GetFunction(name)}) {
      ASSERT_TRUE(funcrec_opt);
      auto funcrec = funcrec_opt.value();
      ASSERT_EQ(id, funcrec.id);
      ASSERT_EQ(name, funcrec.function_name);
    }
  }
  ASSERT_FALSE(registry->GetType(kNonExistentId));
  ASSERT_FALSE(registry->GetType(*kNonExistentTypeName.type));
}

TEST_P(ExtensionIdRegistryTest, ReregisterFunctions) {
  auto provider = std::get<0>(GetParam());
  auto registry = provider->get();

  for (util::string_view name : kFunctionNames) {
    auto id = Id{kArrowExtTypesUri, name};
    ASSERT_RAISES(Invalid, registry->CanRegisterFunction(id, name.to_string()));
    ASSERT_RAISES(Invalid, registry->RegisterFunction(id, name.to_string()));
  }
}

INSTANTIATE_TEST_SUITE_P(
    Substrait, ExtensionIdRegistryTest,
    testing::Values(
        std::make_tuple(std::make_shared<DefaultExtensionIdRegistryProvider>(),
                        "default"),
        std::make_tuple(std::make_shared<NestedExtensionIdRegistryProvider>(),
                        "nested")));

TEST(ExtensionIdRegistryTest, RegisterTempTypes) {
  auto default_registry = default_extension_id_registry();
  constexpr int rounds = 3;
  for (int i = 0; i < rounds; i++) {
    auto registry = substrait::MakeExtensionIdRegistry();

    for (TypeName e : kTempTypeNames) {
      auto id = Id{kArrowExtTypesUri, e.name};
      ASSERT_OK(registry->CanRegisterType(id, e.type));
      ASSERT_OK(registry->RegisterType(id, e.type));
      ASSERT_RAISES(Invalid, registry->CanRegisterType(id, e.type));
      ASSERT_RAISES(Invalid, registry->RegisterType(id, e.type));
      ASSERT_OK(default_registry->CanRegisterType(id, e.type));
    }
  }
}

TEST(ExtensionIdRegistryTest, RegisterTempFunctions) {
  auto default_registry = default_extension_id_registry();
  constexpr int rounds = 3;
  for (int i = 0; i < rounds; i++) {
    auto registry = substrait::MakeExtensionIdRegistry();

    for (util::string_view name : kTempFunctionNames) {
      auto id = Id{kArrowExtTypesUri, name};
      ASSERT_OK(registry->CanRegisterFunction(id, name.to_string()));
      ASSERT_OK(registry->RegisterFunction(id, name.to_string()));
      ASSERT_RAISES(Invalid, registry->CanRegisterFunction(id, name.to_string()));
      ASSERT_RAISES(Invalid, registry->RegisterFunction(id, name.to_string()));
      ASSERT_OK(default_registry->CanRegisterFunction(id, name.to_string()));
    }
  }
}

TEST(ExtensionIdRegistryTest, RegisterNestedTypes) {
  std::shared_ptr<DataType> type1 = kTempTypeNames[0].type;
  std::shared_ptr<DataType> type2 = kTempTypeNames[1].type;
  auto id1 = Id{kArrowExtTypesUri, kTempTypeNames[0].name};
  auto id2 = Id{kArrowExtTypesUri, kTempTypeNames[1].name};

  auto default_registry = default_extension_id_registry();
  constexpr int rounds = 3;
  for (int i = 0; i < rounds; i++) {
    auto registry1 = nested_extension_id_registry(default_registry);

    ASSERT_OK(registry1->CanRegisterType(id1, type1));
    ASSERT_OK(registry1->RegisterType(id1, type1));

    for (int j = 0; j < rounds; j++) {
      auto registry2 = nested_extension_id_registry(&*registry1);

      ASSERT_OK(registry2->CanRegisterType(id2, type2));
      ASSERT_OK(registry2->RegisterType(id2, type2));
      ASSERT_RAISES(Invalid, registry2->CanRegisterType(id2, type2));
      ASSERT_RAISES(Invalid, registry2->RegisterType(id2, type2));
      ASSERT_OK(default_registry->CanRegisterType(id2, type2));
    }

    ASSERT_RAISES(Invalid, registry1->CanRegisterType(id1, type1));
    ASSERT_RAISES(Invalid, registry1->RegisterType(id1, type1));
    ASSERT_OK(default_registry->CanRegisterType(id1, type1));
  }
}

TEST(ExtensionIdRegistryTest, RegisterNestedFunctions) {
  util::string_view name1 = kTempFunctionNames[0];
  util::string_view name2 = kTempFunctionNames[1];
  auto id1 = Id{kArrowExtTypesUri, name1};
  auto id2 = Id{kArrowExtTypesUri, name2};

  auto default_registry = default_extension_id_registry();
  constexpr int rounds = 3;
  for (int i = 0; i < rounds; i++) {
    auto registry1 = substrait::MakeExtensionIdRegistry();

    ASSERT_OK(registry1->CanRegisterFunction(id1, name1.to_string()));
    ASSERT_OK(registry1->RegisterFunction(id1, name1.to_string()));

    for (int j = 0; j < rounds; j++) {
      auto registry2 = substrait::MakeExtensionIdRegistry();

      ASSERT_OK(registry2->CanRegisterFunction(id2, name2.to_string()));
      ASSERT_OK(registry2->RegisterFunction(id2, name2.to_string()));
      ASSERT_RAISES(Invalid, registry2->CanRegisterFunction(id2, name2.to_string()));
      ASSERT_RAISES(Invalid, registry2->RegisterFunction(id2, name2.to_string()));
      ASSERT_OK(default_registry->CanRegisterFunction(id2, name2.to_string()));
    }

    ASSERT_RAISES(Invalid, registry1->CanRegisterFunction(id1, name1.to_string()));
    ASSERT_RAISES(Invalid, registry1->RegisterFunction(id1, name1.to_string()));
    ASSERT_OK(default_registry->CanRegisterFunction(id1, name1.to_string()));
  }
}

}  // namespace engine
}  // namespace arrow
