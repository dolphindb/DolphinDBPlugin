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

#include "arrow/engine/substrait/serde.h"

#include "arrow/engine/substrait/expression_internal.h"
#include "arrow/engine/substrait/plan_internal.h"
#include "arrow/engine/substrait/relation_internal.h"
#include "arrow/engine/substrait/type_internal.h"
#include "arrow/util/string_view.h"

#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/message.h>
#include <google/protobuf/util/json_util.h>
#include <google/protobuf/util/message_differencer.h>
#include <google/protobuf/util/type_resolver_util.h>

namespace arrow {
namespace engine {

Status ParseFromBufferImpl(const Buffer& buf, const std::string& full_name,
                           google::protobuf::Message* message) {
  google::protobuf::io::ArrayInputStream buf_stream{buf.data(),
                                                    static_cast<int>(buf.size())};

  if (message->ParseFromZeroCopyStream(&buf_stream)) {
    return Status::OK();
  }
  return Status::IOError("ParseFromZeroCopyStream failed for ", full_name);
}

template <typename Message>
Result<Message> ParseFromBuffer(const Buffer& buf) {
  Message message;
  ARROW_RETURN_NOT_OK(
      ParseFromBufferImpl(buf, Message::descriptor()->full_name(), &message));
  return message;
}

Result<compute::Declaration> DeserializeRelation(const Buffer& buf,
                                                 const ExtensionSet& ext_set) {
  ARROW_ASSIGN_OR_RAISE(auto rel, ParseFromBuffer<substrait::Rel>(buf));
  ARROW_ASSIGN_OR_RAISE(auto decl_info, FromProto(rel, ext_set));
  return std::move(decl_info.declaration);
}

using DeclarationFactory = std::function<Result<compute::Declaration>(
    compute::Declaration, std::vector<std::string> names)>;

namespace {

DeclarationFactory MakeConsumingSinkDeclarationFactory(
    const ConsumerFactory& consumer_factory) {
  return [&consumer_factory](
             compute::Declaration input,
             std::vector<std::string> names) -> Result<compute::Declaration> {
    std::shared_ptr<compute::SinkNodeConsumer> consumer = consumer_factory();
    if (consumer == NULLPTR) {
      return Status::Invalid("consumer factory is exhausted");
    }
    std::shared_ptr<compute::ExecNodeOptions> options =
        std::make_shared<compute::ConsumingSinkNodeOptions>(
            compute::ConsumingSinkNodeOptions{consumer_factory(), std::move(names)});
    return compute::Declaration::Sequence(
        {std::move(input), {"consuming_sink", options}});
  };
}

compute::Declaration ProjectByNamesDeclaration(compute::Declaration input,
                                               std::vector<std::string> names) {
  int names_size = static_cast<int>(names.size());
  if (names_size == 0) {
    return input;
  }
  std::vector<compute::Expression> expressions;
  for (int i = 0; i < names_size; i++) {
    expressions.push_back(compute::field_ref(FieldRef(i)));
  }
  return compute::Declaration::Sequence(
      {std::move(input),
       {"project",
        compute::ProjectNodeOptions{std::move(expressions), std::move(names)}}});
}

DeclarationFactory MakeWriteDeclarationFactory(
    const WriteOptionsFactory& write_options_factory) {
  return [&write_options_factory](
             compute::Declaration input,
             std::vector<std::string> names) -> Result<compute::Declaration> {
    std::shared_ptr<dataset::WriteNodeOptions> options = write_options_factory();
    if (options == NULLPTR) {
      return Status::Invalid("write options factory is exhausted");
    }
    compute::Declaration projected = ProjectByNamesDeclaration(input, names);
    return compute::Declaration::Sequence(
        {std::move(projected), {"write", std::move(*options)}});
  };
}

Result<std::vector<compute::Declaration>> DeserializePlans(
    const Buffer& buf, DeclarationFactory declaration_factory,
    const ExtensionIdRegistry* registry, ExtensionSet* ext_set_out) {
  ARROW_ASSIGN_OR_RAISE(auto plan, ParseFromBuffer<substrait::Plan>(buf));

  ARROW_ASSIGN_OR_RAISE(auto ext_set, GetExtensionSetFromPlan(plan, registry));

  std::vector<compute::Declaration> sink_decls;
  for (const substrait::PlanRel& plan_rel : plan.relations()) {
    ARROW_ASSIGN_OR_RAISE(
        auto decl_info,
        FromProto(plan_rel.has_root() ? plan_rel.root().input() : plan_rel.rel(),
                  ext_set));
    std::vector<std::string> names;
    if (plan_rel.has_root()) {
      names.assign(plan_rel.root().names().begin(), plan_rel.root().names().end());
    }

    // pipe each relation
    ARROW_ASSIGN_OR_RAISE(
        auto sink_decl,
        declaration_factory(std::move(decl_info.declaration), std::move(names)));
    sink_decls.push_back(std::move(sink_decl));
  }

  if (ext_set_out) {
    *ext_set_out = std::move(ext_set);
  }
  return sink_decls;
}

}  // namespace

Result<std::vector<compute::Declaration>> DeserializePlans(
    const Buffer& buf, const ConsumerFactory& consumer_factory,
    const ExtensionIdRegistry* registry, ExtensionSet* ext_set_out) {
  return DeserializePlans(buf, MakeConsumingSinkDeclarationFactory(consumer_factory),
                          registry, ext_set_out);
}

Result<std::vector<compute::Declaration>> DeserializePlans(
    const Buffer& buf, const WriteOptionsFactory& write_options_factory,
    const ExtensionIdRegistry* registry, ExtensionSet* ext_set_out) {
  return DeserializePlans(buf, MakeWriteDeclarationFactory(write_options_factory),
                          registry, ext_set_out);
}

namespace {

Result<compute::ExecPlan> MakeSingleDeclarationPlan(
    std::vector<compute::Declaration> declarations) {
  if (declarations.size() > 1) {
    return Status::Invalid("DeserializePlan does not support multiple root relations");
  } else {
    ARROW_ASSIGN_OR_RAISE(auto plan, compute::ExecPlan::Make());
    std::ignore = declarations[0].AddToPlan(plan.get());
    return *std::move(plan);
  }
}

}  // namespace

Result<compute::ExecPlan> DeserializePlan(
    const Buffer& buf, const std::shared_ptr<compute::SinkNodeConsumer>& consumer,
    const ExtensionIdRegistry* registry, ExtensionSet* ext_set_out) {
  bool factory_done = false;
  auto single_consumer = [&factory_done, &consumer] {
    if (factory_done) {
      return std::shared_ptr<compute::SinkNodeConsumer>{};
    }
    factory_done = true;
    return consumer;
  };
  ARROW_ASSIGN_OR_RAISE(auto declarations,
                        DeserializePlans(buf, single_consumer, registry, ext_set_out));
  return MakeSingleDeclarationPlan(declarations);
}

Result<compute::ExecPlan> DeserializePlan(
    const Buffer& buf, const std::shared_ptr<dataset::WriteNodeOptions>& write_options,
    const ExtensionIdRegistry* registry, ExtensionSet* ext_set_out) {
  bool factory_done = false;
  auto single_write_options = [&factory_done, &write_options] {
    if (factory_done) {
      return std::shared_ptr<dataset::WriteNodeOptions>{};
    }
    factory_done = true;
    return write_options;
  };
  ARROW_ASSIGN_OR_RAISE(auto declarations, DeserializePlans(buf, single_write_options,
                                                            registry, ext_set_out));
  return MakeSingleDeclarationPlan(declarations);
}

Result<std::shared_ptr<Schema>> DeserializeSchema(const Buffer& buf,
                                                  const ExtensionSet& ext_set) {
  ARROW_ASSIGN_OR_RAISE(auto named_struct, ParseFromBuffer<substrait::NamedStruct>(buf));
  return FromProto(named_struct, ext_set);
}

Result<std::shared_ptr<Buffer>> SerializeSchema(const Schema& schema,
                                                ExtensionSet* ext_set) {
  ARROW_ASSIGN_OR_RAISE(auto named_struct, ToProto(schema, ext_set));
  std::string serialized = named_struct->SerializeAsString();
  return Buffer::FromString(std::move(serialized));
}

Result<std::shared_ptr<DataType>> DeserializeType(const Buffer& buf,
                                                  const ExtensionSet& ext_set) {
  ARROW_ASSIGN_OR_RAISE(auto type, ParseFromBuffer<substrait::Type>(buf));
  ARROW_ASSIGN_OR_RAISE(auto type_nullable, FromProto(type, ext_set));
  return std::move(type_nullable.first);
}

Result<std::shared_ptr<Buffer>> SerializeType(const DataType& type,
                                              ExtensionSet* ext_set) {
  ARROW_ASSIGN_OR_RAISE(auto st_type, ToProto(type, /*nullable=*/true, ext_set));
  std::string serialized = st_type->SerializeAsString();
  return Buffer::FromString(std::move(serialized));
}

Result<compute::Expression> DeserializeExpression(const Buffer& buf,
                                                  const ExtensionSet& ext_set) {
  ARROW_ASSIGN_OR_RAISE(auto expr, ParseFromBuffer<substrait::Expression>(buf));
  return FromProto(expr, ext_set);
}

Result<std::shared_ptr<Buffer>> SerializeExpression(const compute::Expression& expr,
                                                    ExtensionSet* ext_set) {
  ARROW_ASSIGN_OR_RAISE(auto st_expr, ToProto(expr, ext_set));
  std::string serialized = st_expr->SerializeAsString();
  return Buffer::FromString(std::move(serialized));
}

namespace internal {

template <typename Message>
static Status CheckMessagesEquivalent(const Buffer& l_buf, const Buffer& r_buf) {
  ARROW_ASSIGN_OR_RAISE(auto l, ParseFromBuffer<Message>(l_buf));
  ARROW_ASSIGN_OR_RAISE(auto r, ParseFromBuffer<Message>(r_buf));

  using google::protobuf::util::MessageDifferencer;

  std::string out;
  google::protobuf::io::StringOutputStream out_stream{&out};
  MessageDifferencer::StreamReporter reporter{&out_stream};

  MessageDifferencer differencer;
  differencer.set_message_field_comparison(MessageDifferencer::EQUIVALENT);
  differencer.ReportDifferencesTo(&reporter);

  if (differencer.Compare(l, r)) {
    return Status::OK();
  }
  return Status::Invalid("Messages were not equivalent: ", out);
}

Status CheckMessagesEquivalent(util::string_view message_name, const Buffer& l_buf,
                               const Buffer& r_buf) {
  if (message_name == "Type") {
    return CheckMessagesEquivalent<substrait::Type>(l_buf, r_buf);
  }

  if (message_name == "NamedStruct") {
    return CheckMessagesEquivalent<substrait::NamedStruct>(l_buf, r_buf);
  }

  if (message_name == "Schema") {
    return Status::Invalid(
        "There is no substrait message named Schema. The substrait message type which "
        "corresponds to Schemas is NamedStruct");
  }

  if (message_name == "Expression") {
    return CheckMessagesEquivalent<substrait::Expression>(l_buf, r_buf);
  }

  if (message_name == "Rel") {
    return CheckMessagesEquivalent<substrait::Rel>(l_buf, r_buf);
  }

  if (message_name == "Relation") {
    return Status::Invalid(
        "There is no substrait message named Relation. You probably meant \"Rel\"");
  }

  return Status::Invalid("Unsupported message name ", message_name,
                         " for CheckMessagesEquivalent");
}

inline google::protobuf::util::TypeResolver* GetGeneratedTypeResolver() {
  static std::unique_ptr<google::protobuf::util::TypeResolver> type_resolver;
  if (!type_resolver) {
    type_resolver.reset(google::protobuf::util::NewTypeResolverForDescriptorPool(
        /*url_prefix=*/"", google::protobuf::DescriptorPool::generated_pool()));
  }
  return type_resolver.get();
}

Result<std::shared_ptr<Buffer>> SubstraitFromJSON(util::string_view type_name,
                                                  util::string_view json) {
  std::string type_url = "/substrait." + type_name.to_string();

  google::protobuf::io::ArrayInputStream json_stream{json.data(),
                                                     static_cast<int>(json.size())};

  std::string out;
  google::protobuf::io::StringOutputStream out_stream{&out};
  google::protobuf::util::JsonParseOptions json_opts;
  json_opts.ignore_unknown_fields = true;
  auto status = google::protobuf::util::JsonToBinaryStream(
      GetGeneratedTypeResolver(), type_url, &json_stream, &out_stream,
      std::move(json_opts));

  if (!status.ok()) {
    return Status::Invalid("JsonToBinaryStream returned ", status);
  }
  return Buffer::FromString(std::move(out));
}

Result<std::string> SubstraitToJSON(util::string_view type_name, const Buffer& buf) {
  std::string type_url = "/substrait." + type_name.to_string();

  google::protobuf::io::ArrayInputStream buf_stream{buf.data(),
                                                    static_cast<int>(buf.size())};

  std::string out;
  google::protobuf::io::StringOutputStream out_stream{&out};

  auto status = google::protobuf::util::BinaryToJsonStream(
      GetGeneratedTypeResolver(), type_url, &buf_stream, &out_stream);
  if (!status.ok()) {
    return Status::Invalid("BinaryToJsonStream returned ", status);
  }
  return out;
}

}  // namespace internal
}  // namespace engine
}  // namespace arrow
