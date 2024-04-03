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

#include "arrow/flight/platform.h"

#ifdef __APPLE__
#include <limits.h>
#include <mach-o/dyld.h>
#endif

#include <cstdlib>
#include <sstream>

#include <boost/filesystem.hpp>
// We need BOOST_USE_WINDOWS_H definition with MinGW when we use
// boost/process.hpp. See ARROW_BOOST_PROCESS_COMPILE_DEFINITIONS in
// cpp/cmake_modules/BuildUtils.cmake for details.
#include <boost/process.hpp>

#include <gtest/gtest.h>

#include "arrow/ipc/test_common.h"
#include "arrow/testing/generator.h"
#include "arrow/testing/gtest_util.h"
#include "arrow/testing/util.h"
#include "arrow/util/logging.h"

#include "arrow/flight/api.h"
#include "arrow/flight/internal.h"
#include "arrow/flight/test_util.h"

namespace arrow {
namespace flight {

namespace bp = boost::process;
namespace fs = boost::filesystem;

namespace {

Status ResolveCurrentExecutable(fs::path* out) {
  // See https://stackoverflow.com/a/1024937/10194 for various
  // platform-specific recipes.

  boost::system::error_code ec;

#if defined(__linux__)
  *out = fs::canonical("/proc/self/exe", ec);
#elif defined(__APPLE__)
  char buf[PATH_MAX + 1];
  uint32_t bufsize = sizeof(buf);
  if (_NSGetExecutablePath(buf, &bufsize) < 0) {
    return Status::Invalid("Can't resolve current exe: path too large");
  }
  *out = fs::canonical(buf, ec);
#elif defined(_WIN32)
  char buf[MAX_PATH + 1];
  if (!GetModuleFileNameA(NULL, buf, sizeof(buf))) {
    return Status::Invalid("Can't get executable file path");
  }
  *out = fs::canonical(buf, ec);
#else
  ARROW_UNUSED(ec);
  return Status::NotImplemented("Not available on this system");
#endif
  if (ec) {
    // XXX fold this into the Status class?
    return Status::IOError("Can't resolve current exe: ", ec.message());
  } else {
    return Status::OK();
  }
}

}  // namespace

void TestServer::Start() {
  namespace fs = boost::filesystem;

  std::string str_port = std::to_string(port_);
  std::vector<fs::path> search_path = ::boost::this_process::path();
  // If possible, prepend current executable directory to search path,
  // since it's likely that the test server executable is located in
  // the same directory as the running unit test.
  fs::path current_exe;
  Status st = ResolveCurrentExecutable(&current_exe);
  if (st.ok()) {
    search_path.insert(search_path.begin(), current_exe.parent_path());
  } else if (st.IsNotImplemented()) {
    ARROW_CHECK(st.IsNotImplemented()) << st.ToString();
  }

  try {
    server_process_ = std::make_shared<bp::child>(
        bp::search_path(executable_name_, search_path), "-port", str_port);
  } catch (...) {
    std::stringstream ss;
    ss << "Failed to launch test server '" << executable_name_ << "', looked in ";
    for (const auto& path : search_path) {
      ss << path << " : ";
    }
    ARROW_LOG(FATAL) << ss.str();
    throw;
  }
  std::cout << "Server running with pid " << server_process_->id() << std::endl;
}

int TestServer::Stop() {
  if (server_process_ && server_process_->valid()) {
#ifndef _WIN32
    kill(server_process_->id(), SIGTERM);
#else
    // This would use SIGKILL on POSIX, which is more brutal than SIGTERM
    server_process_->terminate();
#endif
    server_process_->wait();
    return server_process_->exit_code();
  } else {
    // Presumably the server wasn't able to start
    return -1;
  }
}

bool TestServer::IsRunning() { return server_process_->running(); }

int TestServer::port() const { return port_; }

Status GetBatchForFlight(const Ticket& ticket, std::shared_ptr<RecordBatchReader>* out) {
  if (ticket.ticket == "ticket-ints-1") {
    BatchVector batches;
    RETURN_NOT_OK(ExampleIntBatches(&batches));
    *out = std::make_shared<BatchIterator>(batches[0]->schema(), batches);
    return Status::OK();
  } else if (ticket.ticket == "ticket-floats-1") {
    BatchVector batches;
    RETURN_NOT_OK(ExampleFloatBatches(&batches));
    *out = std::make_shared<BatchIterator>(batches[0]->schema(), batches);
    return Status::OK();
  } else if (ticket.ticket == "ticket-dicts-1") {
    BatchVector batches;
    RETURN_NOT_OK(ExampleDictBatches(&batches));
    *out = std::make_shared<BatchIterator>(batches[0]->schema(), batches);
    return Status::OK();
  } else if (ticket.ticket == "ticket-large-batch-1") {
    BatchVector batches;
    RETURN_NOT_OK(ExampleLargeBatches(&batches));
    *out = std::make_shared<BatchIterator>(batches[0]->schema(), batches);
    return Status::OK();
  } else {
    return Status::NotImplemented("no stream implemented for ticket: " + ticket.ticket);
  }
}

class FlightTestServer : public FlightServerBase {
  Status ListFlights(const ServerCallContext& context, const Criteria* criteria,
                     std::unique_ptr<FlightListing>* listings) override {
    std::vector<FlightInfo> flights = ExampleFlightInfo();
    if (criteria && criteria->expression != "") {
      // For test purposes, if we get criteria, return no results
      flights.clear();
    }
    *listings = std::unique_ptr<FlightListing>(new SimpleFlightListing(flights));
    return Status::OK();
  }

  Status GetFlightInfo(const ServerCallContext& context, const FlightDescriptor& request,
                       std::unique_ptr<FlightInfo>* out) override {
    // Test that Arrow-C++ status codes can make it through gRPC
    if (request.type == FlightDescriptor::DescriptorType::CMD &&
        request.cmd == "status-outofmemory") {
      return Status::OutOfMemory("Sentinel");
    }

    std::vector<FlightInfo> flights = ExampleFlightInfo();

    for (const auto& info : flights) {
      if (info.descriptor().Equals(request)) {
        *out = std::unique_ptr<FlightInfo>(new FlightInfo(info));
        return Status::OK();
      }
    }
    return Status::Invalid("Flight not found: ", request.ToString());
  }

  Status DoGet(const ServerCallContext& context, const Ticket& request,
               std::unique_ptr<FlightDataStream>* data_stream) override {
    // Test for ARROW-5095
    if (request.ticket == "ARROW-5095-fail") {
      return Status::UnknownError("Server-side error");
    }
    if (request.ticket == "ARROW-5095-success") {
      return Status::OK();
    }

    std::shared_ptr<RecordBatchReader> batch_reader;
    RETURN_NOT_OK(GetBatchForFlight(request, &batch_reader));

    *data_stream = std::unique_ptr<FlightDataStream>(new RecordBatchStream(batch_reader));
    return Status::OK();
  }

  Status DoExchange(const ServerCallContext& context,
                    std::unique_ptr<FlightMessageReader> reader,
                    std::unique_ptr<FlightMessageWriter> writer) override {
    // Test various scenarios for a DoExchange
    if (reader->descriptor().type != FlightDescriptor::DescriptorType::CMD) {
      return Status::Invalid("Must provide a command descriptor");
    }

    const std::string& cmd = reader->descriptor().cmd;
    if (cmd == "error") {
      // Immediately return an error to the client.
      return Status::NotImplemented("Expected error");
    } else if (cmd == "get") {
      return RunExchangeGet(std::move(reader), std::move(writer));
    } else if (cmd == "put") {
      return RunExchangePut(std::move(reader), std::move(writer));
    } else if (cmd == "counter") {
      return RunExchangeCounter(std::move(reader), std::move(writer));
    } else if (cmd == "total") {
      return RunExchangeTotal(std::move(reader), std::move(writer));
    } else if (cmd == "echo") {
      return RunExchangeEcho(std::move(reader), std::move(writer));
    } else {
      return Status::NotImplemented("Scenario not implemented: ", cmd);
    }
  }

  // A simple example - act like DoGet.
  Status RunExchangeGet(std::unique_ptr<FlightMessageReader> reader,
                        std::unique_ptr<FlightMessageWriter> writer) {
    RETURN_NOT_OK(writer->Begin(ExampleIntSchema()));
    BatchVector batches;
    RETURN_NOT_OK(ExampleIntBatches(&batches));
    for (const auto& batch : batches) {
      RETURN_NOT_OK(writer->WriteRecordBatch(*batch));
    }
    return Status::OK();
  }

  // A simple example - act like DoPut
  Status RunExchangePut(std::unique_ptr<FlightMessageReader> reader,
                        std::unique_ptr<FlightMessageWriter> writer) {
    ARROW_ASSIGN_OR_RAISE(auto schema, reader->GetSchema());
    if (!schema->Equals(ExampleIntSchema(), false)) {
      return Status::Invalid("Schema is not as expected");
    }
    BatchVector batches;
    RETURN_NOT_OK(ExampleIntBatches(&batches));
    FlightStreamChunk chunk;
    for (const auto& batch : batches) {
      RETURN_NOT_OK(reader->Next(&chunk));
      if (!chunk.data) {
        return Status::Invalid("Expected another batch");
      }
      if (!batch->Equals(*chunk.data)) {
        return Status::Invalid("Batch does not match");
      }
    }
    RETURN_NOT_OK(reader->Next(&chunk));
    if (chunk.data || chunk.app_metadata) {
      return Status::Invalid("Too many batches");
    }

    RETURN_NOT_OK(writer->WriteMetadata(Buffer::FromString("done")));
    return Status::OK();
  }

  // Read some number of record batches from the client, send a
  // metadata message back with the count, then echo the batches back.
  Status RunExchangeCounter(std::unique_ptr<FlightMessageReader> reader,
                            std::unique_ptr<FlightMessageWriter> writer) {
    std::vector<std::shared_ptr<RecordBatch>> batches;
    FlightStreamChunk chunk;
    int chunks = 0;
    while (true) {
      RETURN_NOT_OK(reader->Next(&chunk));
      if (!chunk.data && !chunk.app_metadata) {
        break;
      }
      if (chunk.data) {
        batches.push_back(chunk.data);
        chunks++;
      }
    }

    // Echo back the number of record batches read.
    std::shared_ptr<Buffer> buf = Buffer::FromString(std::to_string(chunks));
    RETURN_NOT_OK(writer->WriteMetadata(buf));
    // Echo the record batches themselves.
    if (chunks > 0) {
      ARROW_ASSIGN_OR_RAISE(auto schema, reader->GetSchema());
      RETURN_NOT_OK(writer->Begin(schema));

      for (const auto& batch : batches) {
        RETURN_NOT_OK(writer->WriteRecordBatch(*batch));
      }
    }

    return Status::OK();
  }

  // Read int64 batches from the client, each time sending back a
  // batch with a running sum of columns.
  Status RunExchangeTotal(std::unique_ptr<FlightMessageReader> reader,
                          std::unique_ptr<FlightMessageWriter> writer) {
    FlightStreamChunk chunk{};
    ARROW_ASSIGN_OR_RAISE(auto schema, reader->GetSchema());
    // Ensure the schema contains only int64 columns
    for (const auto& field : schema->fields()) {
      if (field->type()->id() != Type::type::INT64) {
        return Status::Invalid("Field is not INT64: ", field->name());
      }
    }
    std::vector<int64_t> sums(schema->num_fields());
    std::vector<std::shared_ptr<Array>> columns(schema->num_fields());
    RETURN_NOT_OK(writer->Begin(schema));
    while (true) {
      RETURN_NOT_OK(reader->Next(&chunk));
      if (!chunk.data && !chunk.app_metadata) {
        break;
      }
      if (chunk.data) {
        if (!chunk.data->schema()->Equals(schema, false)) {
          // A compliant client implementation would make this impossible
          return Status::Invalid("Schemas are incompatible");
        }

        // Update the running totals
        auto builder = std::make_shared<Int64Builder>();
        int col_index = 0;
        for (const auto& column : chunk.data->columns()) {
          auto arr = std::dynamic_pointer_cast<Int64Array>(column);
          if (!arr) {
            return MakeFlightError(FlightStatusCode::Internal, "Could not cast array");
          }
          for (int row = 0; row < column->length(); row++) {
            if (!arr->IsNull(row)) {
              sums[col_index] += arr->Value(row);
            }
          }

          builder->Reset();
          RETURN_NOT_OK(builder->Append(sums[col_index]));
          RETURN_NOT_OK(builder->Finish(&columns[col_index]));

          col_index++;
        }

        // Echo the totals to the client
        auto response = RecordBatch::Make(schema, /* num_rows */ 1, columns);
        RETURN_NOT_OK(writer->WriteRecordBatch(*response));
      }
    }
    return Status::OK();
  }

  // Echo the client's messages back.
  Status RunExchangeEcho(std::unique_ptr<FlightMessageReader> reader,
                         std::unique_ptr<FlightMessageWriter> writer) {
    FlightStreamChunk chunk;
    bool begun = false;
    while (true) {
      RETURN_NOT_OK(reader->Next(&chunk));
      if (!chunk.data && !chunk.app_metadata) {
        break;
      }
      if (!begun && chunk.data) {
        begun = true;
        RETURN_NOT_OK(writer->Begin(chunk.data->schema()));
      }
      if (chunk.data && chunk.app_metadata) {
        RETURN_NOT_OK(writer->WriteWithMetadata(*chunk.data, chunk.app_metadata));
      } else if (chunk.data) {
        RETURN_NOT_OK(writer->WriteRecordBatch(*chunk.data));
      } else if (chunk.app_metadata) {
        RETURN_NOT_OK(writer->WriteMetadata(chunk.app_metadata));
      }
    }
    return Status::OK();
  }

  Status RunAction1(const Action& action, std::unique_ptr<ResultStream>* out) {
    std::vector<Result> results;
    for (int i = 0; i < 3; ++i) {
      Result result;
      std::string value = action.body->ToString() + "-part" + std::to_string(i);
      result.body = Buffer::FromString(std::move(value));
      results.push_back(result);
    }
    *out = std::unique_ptr<ResultStream>(new SimpleResultStream(std::move(results)));
    return Status::OK();
  }

  Status RunAction2(std::unique_ptr<ResultStream>* out) {
    // Empty
    *out = std::unique_ptr<ResultStream>(new SimpleResultStream({}));
    return Status::OK();
  }

  Status DoAction(const ServerCallContext& context, const Action& action,
                  std::unique_ptr<ResultStream>* out) override {
    if (action.type == "action1") {
      return RunAction1(action, out);
    } else if (action.type == "action2") {
      return RunAction2(out);
    } else {
      return Status::NotImplemented(action.type);
    }
  }

  Status ListActions(const ServerCallContext& context,
                     std::vector<ActionType>* out) override {
    std::vector<ActionType> actions = ExampleActionTypes();
    *out = std::move(actions);
    return Status::OK();
  }

  Status GetSchema(const ServerCallContext& context, const FlightDescriptor& request,
                   std::unique_ptr<SchemaResult>* schema) override {
    std::vector<FlightInfo> flights = ExampleFlightInfo();

    for (const auto& info : flights) {
      if (info.descriptor().Equals(request)) {
        *schema =
            std::unique_ptr<SchemaResult>(new SchemaResult(info.serialized_schema()));
        return Status::OK();
      }
    }
    return Status::Invalid("Flight not found: ", request.ToString());
  }
};

std::unique_ptr<FlightServerBase> ExampleTestServer() {
  return std::unique_ptr<FlightServerBase>(new FlightTestServer);
}

Status MakeFlightInfo(const Schema& schema, const FlightDescriptor& descriptor,
                      const std::vector<FlightEndpoint>& endpoints, int64_t total_records,
                      int64_t total_bytes, FlightInfo::Data* out) {
  out->descriptor = descriptor;
  out->endpoints = endpoints;
  out->total_records = total_records;
  out->total_bytes = total_bytes;
  return internal::SchemaToString(schema, &out->schema);
}

NumberingStream::NumberingStream(std::unique_ptr<FlightDataStream> stream)
    : counter_(0), stream_(std::move(stream)) {}

std::shared_ptr<Schema> NumberingStream::schema() { return stream_->schema(); }

Status NumberingStream::GetSchemaPayload(FlightPayload* payload) {
  return stream_->GetSchemaPayload(payload);
}

Status NumberingStream::Next(FlightPayload* payload) {
  RETURN_NOT_OK(stream_->Next(payload));
  if (payload && payload->ipc_message.type == ipc::MessageType::RECORD_BATCH) {
    payload->app_metadata = Buffer::FromString(std::to_string(counter_));
    counter_++;
  }
  return Status::OK();
}

std::shared_ptr<Schema> ExampleIntSchema() {
  auto f0 = field("f0", int8());
  auto f1 = field("f1", uint8());
  auto f2 = field("f2", int16());
  auto f3 = field("f3", uint16());
  auto f4 = field("f4", int32());
  auto f5 = field("f5", uint32());
  auto f6 = field("f6", int64());
  auto f7 = field("f7", uint64());
  return ::arrow::schema({f0, f1, f2, f3, f4, f5, f6, f7});
}

std::shared_ptr<Schema> ExampleFloatSchema() {
  auto f0 = field("f0", float16());
  auto f1 = field("f1", float32());
  auto f2 = field("f2", float64());
  return ::arrow::schema({f0, f1, f2});
}

std::shared_ptr<Schema> ExampleStringSchema() {
  auto f0 = field("f0", utf8());
  auto f1 = field("f1", binary());
  return ::arrow::schema({f0, f1});
}

std::shared_ptr<Schema> ExampleDictSchema() {
  std::shared_ptr<RecordBatch> batch;
  ABORT_NOT_OK(ipc::test::MakeDictionary(&batch));
  return batch->schema();
}

std::shared_ptr<Schema> ExampleLargeSchema() {
  std::vector<std::shared_ptr<arrow::Field>> fields;
  for (int i = 0; i < 128; i++) {
    const auto field_name = "f" + std::to_string(i);
    fields.push_back(arrow::field(field_name, arrow::float64()));
  }
  return arrow::schema(fields);
}

std::vector<FlightInfo> ExampleFlightInfo() {
  Location location1;
  Location location2;
  Location location3;
  Location location4;
  Location location5;
  ARROW_EXPECT_OK(Location::ForGrpcTcp("foo1.bar.com", 12345, &location1));
  ARROW_EXPECT_OK(Location::ForGrpcTcp("foo2.bar.com", 12345, &location2));
  ARROW_EXPECT_OK(Location::ForGrpcTcp("foo3.bar.com", 12345, &location3));
  ARROW_EXPECT_OK(Location::ForGrpcTcp("foo4.bar.com", 12345, &location4));
  ARROW_EXPECT_OK(Location::ForGrpcTcp("foo5.bar.com", 12345, &location5));

  FlightInfo::Data flight1, flight2, flight3, flight4;

  FlightEndpoint endpoint1({{"ticket-ints-1"}, {location1}});
  FlightEndpoint endpoint2({{"ticket-ints-2"}, {location2}});
  FlightEndpoint endpoint3({{"ticket-cmd"}, {location3}});
  FlightEndpoint endpoint4({{"ticket-dicts-1"}, {location4}});
  FlightEndpoint endpoint5({{"ticket-floats-1"}, {location5}});

  FlightDescriptor descr1{FlightDescriptor::PATH, "", {"examples", "ints"}};
  FlightDescriptor descr2{FlightDescriptor::CMD, "my_command", {}};
  FlightDescriptor descr3{FlightDescriptor::PATH, "", {"examples", "dicts"}};
  FlightDescriptor descr4{FlightDescriptor::PATH, "", {"examples", "floats"}};

  auto schema1 = ExampleIntSchema();
  auto schema2 = ExampleStringSchema();
  auto schema3 = ExampleDictSchema();
  auto schema4 = ExampleFloatSchema();

  ARROW_EXPECT_OK(
      MakeFlightInfo(*schema1, descr1, {endpoint1, endpoint2}, 1000, 100000, &flight1));
  ARROW_EXPECT_OK(MakeFlightInfo(*schema2, descr2, {endpoint3}, 1000, 100000, &flight2));
  ARROW_EXPECT_OK(MakeFlightInfo(*schema3, descr3, {endpoint4}, -1, -1, &flight3));
  ARROW_EXPECT_OK(MakeFlightInfo(*schema4, descr4, {endpoint5}, 1000, 100000, &flight4));
  return {FlightInfo(flight1), FlightInfo(flight2), FlightInfo(flight3),
          FlightInfo(flight4)};
}

Status ExampleIntBatches(BatchVector* out) {
  std::shared_ptr<RecordBatch> batch;
  for (int i = 0; i < 5; ++i) {
    // Make all different sizes, use different random seed
    RETURN_NOT_OK(ipc::test::MakeIntBatchSized(10 + i, &batch, i));
    out->push_back(batch);
  }
  return Status::OK();
}

Status ExampleFloatBatches(BatchVector* out) {
  std::shared_ptr<RecordBatch> batch;
  for (int i = 0; i < 5; ++i) {
    // Make all different sizes, use different random seed
    RETURN_NOT_OK(ipc::test::MakeFloatBatchSized(10 + i, &batch, i));
    out->push_back(batch);
  }
  return Status::OK();
}

Status ExampleDictBatches(BatchVector* out) {
  // Just the same batch, repeated a few times
  std::shared_ptr<RecordBatch> batch;
  for (int i = 0; i < 3; ++i) {
    RETURN_NOT_OK(ipc::test::MakeDictionary(&batch));
    out->push_back(batch);
  }
  return Status::OK();
}

Status ExampleNestedBatches(BatchVector* out) {
  std::shared_ptr<RecordBatch> batch;
  for (int i = 0; i < 3; ++i) {
    RETURN_NOT_OK(ipc::test::MakeListRecordBatch(&batch));
    out->push_back(batch);
  }
  return Status::OK();
}

Status ExampleLargeBatches(BatchVector* out) {
  const auto array_length = 32768;
  std::shared_ptr<RecordBatch> batch;
  std::vector<std::shared_ptr<arrow::Array>> arrays;
  const auto arr = arrow::ConstantArrayGenerator::Float64(array_length, 1.0);
  for (int i = 0; i < 128; i++) {
    arrays.push_back(arr);
  }
  auto schema = ExampleLargeSchema();
  out->push_back(RecordBatch::Make(schema, array_length, arrays));
  out->push_back(RecordBatch::Make(schema, array_length, arrays));
  return Status::OK();
}

std::vector<ActionType> ExampleActionTypes() {
  return {{"drop", "drop a dataset"}, {"cache", "cache a dataset"}};
}

TestServerAuthHandler::TestServerAuthHandler(const std::string& username,
                                             const std::string& password)
    : username_(username), password_(password) {}

TestServerAuthHandler::~TestServerAuthHandler() {}

Status TestServerAuthHandler::Authenticate(ServerAuthSender* outgoing,
                                           ServerAuthReader* incoming) {
  std::string token;
  RETURN_NOT_OK(incoming->Read(&token));
  if (token != password_) {
    return MakeFlightError(FlightStatusCode::Unauthenticated, "Invalid token");
  }
  RETURN_NOT_OK(outgoing->Write(username_));
  return Status::OK();
}

Status TestServerAuthHandler::IsValid(const std::string& token,
                                      std::string* peer_identity) {
  if (token != password_) {
    return MakeFlightError(FlightStatusCode::Unauthenticated, "Invalid token");
  }
  *peer_identity = username_;
  return Status::OK();
}

TestServerBasicAuthHandler::TestServerBasicAuthHandler(const std::string& username,
                                                       const std::string& password) {
  basic_auth_.username = username;
  basic_auth_.password = password;
}

TestServerBasicAuthHandler::~TestServerBasicAuthHandler() {}

Status TestServerBasicAuthHandler::Authenticate(ServerAuthSender* outgoing,
                                                ServerAuthReader* incoming) {
  std::string token;
  RETURN_NOT_OK(incoming->Read(&token));
  BasicAuth incoming_auth;
  RETURN_NOT_OK(BasicAuth::Deserialize(token, &incoming_auth));
  if (incoming_auth.username != basic_auth_.username ||
      incoming_auth.password != basic_auth_.password) {
    return MakeFlightError(FlightStatusCode::Unauthenticated, "Invalid token");
  }
  RETURN_NOT_OK(outgoing->Write(basic_auth_.username));
  return Status::OK();
}

Status TestServerBasicAuthHandler::IsValid(const std::string& token,
                                           std::string* peer_identity) {
  if (token != basic_auth_.username) {
    return MakeFlightError(FlightStatusCode::Unauthenticated, "Invalid token");
  }
  *peer_identity = basic_auth_.username;
  return Status::OK();
}

TestClientAuthHandler::TestClientAuthHandler(const std::string& username,
                                             const std::string& password)
    : username_(username), password_(password) {}

TestClientAuthHandler::~TestClientAuthHandler() {}

Status TestClientAuthHandler::Authenticate(ClientAuthSender* outgoing,
                                           ClientAuthReader* incoming) {
  RETURN_NOT_OK(outgoing->Write(password_));
  std::string username;
  RETURN_NOT_OK(incoming->Read(&username));
  if (username != username_) {
    return MakeFlightError(FlightStatusCode::Unauthenticated, "Invalid token");
  }
  return Status::OK();
}

Status TestClientAuthHandler::GetToken(std::string* token) {
  *token = password_;
  return Status::OK();
}

TestClientBasicAuthHandler::TestClientBasicAuthHandler(const std::string& username,
                                                       const std::string& password) {
  basic_auth_.username = username;
  basic_auth_.password = password;
}

TestClientBasicAuthHandler::~TestClientBasicAuthHandler() {}

Status TestClientBasicAuthHandler::Authenticate(ClientAuthSender* outgoing,
                                                ClientAuthReader* incoming) {
  std::string pb_result;
  RETURN_NOT_OK(BasicAuth::Serialize(basic_auth_, &pb_result));
  RETURN_NOT_OK(outgoing->Write(pb_result));
  RETURN_NOT_OK(incoming->Read(&token_));
  return Status::OK();
}

Status TestClientBasicAuthHandler::GetToken(std::string* token) {
  *token = token_;
  return Status::OK();
}

Status ExampleTlsCertificates(std::vector<CertKeyPair>* out) {
  std::string root;
  RETURN_NOT_OK(GetTestResourceRoot(&root));

  *out = std::vector<CertKeyPair>();
  for (int i = 0; i < 2; i++) {
    try {
      std::stringstream cert_path;
      cert_path << root << "/flight/cert" << i << ".pem";
      std::stringstream key_path;
      key_path << root << "/flight/cert" << i << ".key";

      std::ifstream cert_file(cert_path.str());
      if (!cert_file) {
        return Status::IOError("Could not open certificate: " + cert_path.str());
      }
      std::stringstream cert;
      cert << cert_file.rdbuf();

      std::ifstream key_file(key_path.str());
      if (!key_file) {
        return Status::IOError("Could not open key: " + key_path.str());
      }
      std::stringstream key;
      key << key_file.rdbuf();

      out->push_back(CertKeyPair{cert.str(), key.str()});
    } catch (const std::ifstream::failure& e) {
      return Status::IOError(e.what());
    }
  }
  return Status::OK();
}

Status ExampleTlsCertificateRoot(CertKeyPair* out) {
  std::string root;
  RETURN_NOT_OK(GetTestResourceRoot(&root));

  std::stringstream path;
  path << root << "/flight/root-ca.pem";

  try {
    std::ifstream cert_file(path.str());
    if (!cert_file) {
      return Status::IOError("Could not open certificate: " + path.str());
    }
    std::stringstream cert;
    cert << cert_file.rdbuf();
    out->pem_cert = cert.str();
    out->pem_key = "";
    return Status::OK();
  } catch (const std::ifstream::failure& e) {
    return Status::IOError(e.what());
  }
}

}  // namespace flight
}  // namespace arrow
