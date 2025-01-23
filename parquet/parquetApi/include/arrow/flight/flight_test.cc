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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "arrow/flight/api.h"
#include "arrow/ipc/test_common.h"
#include "arrow/status.h"
#include "arrow/testing/generator.h"
#include "arrow/testing/gtest_util.h"
#include "arrow/testing/util.h"
#include "arrow/util/logging.h"
#include "arrow/util/make_unique.h"

#ifdef GRPCPP_GRPCPP_H
#error "gRPC headers should not be in public API"
#endif

#include "arrow/flight/internal.h"
#include "arrow/flight/middleware_internal.h"
#include "arrow/flight/test_util.h"

namespace pb = arrow::flight::protocol;

namespace arrow {
namespace flight {

void AssertEqual(const ActionType& expected, const ActionType& actual) {
  ASSERT_EQ(expected.type, actual.type);
  ASSERT_EQ(expected.description, actual.description);
}

void AssertEqual(const FlightDescriptor& expected, const FlightDescriptor& actual) {
  ASSERT_TRUE(expected.Equals(actual));
}

void AssertEqual(const Ticket& expected, const Ticket& actual) {
  ASSERT_EQ(expected.ticket, actual.ticket);
}

void AssertEqual(const Location& expected, const Location& actual) {
  ASSERT_EQ(expected, actual);
}

void AssertEqual(const std::vector<FlightEndpoint>& expected,
                 const std::vector<FlightEndpoint>& actual) {
  ASSERT_EQ(expected.size(), actual.size());
  for (size_t i = 0; i < expected.size(); ++i) {
    AssertEqual(expected[i].ticket, actual[i].ticket);

    ASSERT_EQ(expected[i].locations.size(), actual[i].locations.size());
    for (size_t j = 0; j < expected[i].locations.size(); ++j) {
      AssertEqual(expected[i].locations[j], actual[i].locations[j]);
    }
  }
}

template <typename T>
void AssertEqual(const std::vector<T>& expected, const std::vector<T>& actual) {
  ASSERT_EQ(expected.size(), actual.size());
  for (size_t i = 0; i < expected.size(); ++i) {
    AssertEqual(expected[i], actual[i]);
  }
}

void AssertEqual(const FlightInfo& expected, const FlightInfo& actual) {
  std::shared_ptr<Schema> ex_schema, actual_schema;
  ipc::DictionaryMemo expected_memo;
  ipc::DictionaryMemo actual_memo;
  ASSERT_OK(expected.GetSchema(&expected_memo, &ex_schema));
  ASSERT_OK(actual.GetSchema(&actual_memo, &actual_schema));

  AssertSchemaEqual(*ex_schema, *actual_schema);
  ASSERT_EQ(expected.total_records(), actual.total_records());
  ASSERT_EQ(expected.total_bytes(), actual.total_bytes());

  AssertEqual(expected.descriptor(), actual.descriptor());
  AssertEqual(expected.endpoints(), actual.endpoints());
}

TEST(TestFlightDescriptor, Basics) {
  auto a = FlightDescriptor::Command("select * from table");
  auto b = FlightDescriptor::Command("select * from table");
  auto c = FlightDescriptor::Command("select foo from table");
  auto d = FlightDescriptor::Path({"foo", "bar"});
  auto e = FlightDescriptor::Path({"foo", "baz"});
  auto f = FlightDescriptor::Path({"foo", "baz"});

  ASSERT_EQ(a.ToString(), "FlightDescriptor<cmd = 'select * from table'>");
  ASSERT_EQ(d.ToString(), "FlightDescriptor<path = 'foo/bar'>");
  ASSERT_TRUE(a.Equals(b));
  ASSERT_FALSE(a.Equals(c));
  ASSERT_FALSE(a.Equals(d));
  ASSERT_FALSE(d.Equals(e));
  ASSERT_TRUE(e.Equals(f));
}

// This tests the internal protobuf types which don't get exported in the Flight DLL.
#ifndef _WIN32
TEST(TestFlightDescriptor, ToFromProto) {
  FlightDescriptor descr_test;
  pb::FlightDescriptor pb_descr;

  FlightDescriptor descr1{FlightDescriptor::PATH, "", {"foo", "bar"}};
  ASSERT_OK(internal::ToProto(descr1, &pb_descr));
  ASSERT_OK(internal::FromProto(pb_descr, &descr_test));
  AssertEqual(descr1, descr_test);

  FlightDescriptor descr2{FlightDescriptor::CMD, "command", {}};
  ASSERT_OK(internal::ToProto(descr2, &pb_descr));
  ASSERT_OK(internal::FromProto(pb_descr, &descr_test));
  AssertEqual(descr2, descr_test);
}
#endif

TEST(TestFlight, DISABLED_StartStopTestServer) {
  TestServer server("flight-test-server");
  server.Start();
  ASSERT_TRUE(server.IsRunning());

  std::this_thread::sleep_for(std::chrono::duration<double>(0.2));

  ASSERT_TRUE(server.IsRunning());
  int exit_code = server.Stop();
#ifdef _WIN32
  // We do a hard kill on Windows
  ASSERT_EQ(259, exit_code);
#else
  ASSERT_EQ(0, exit_code);
#endif
  ASSERT_FALSE(server.IsRunning());
}

// ARROW-6017: we should be able to construct locations for unknown
// schemes
TEST(TestFlight, UnknownLocationScheme) {
  Location location;
  ASSERT_OK(Location::Parse("s3://test", &location));
  ASSERT_OK(Location::Parse("https://example.com/foo", &location));
}

TEST(TestFlight, ConnectUri) {
  TestServer server("flight-test-server");
  server.Start();
  ASSERT_TRUE(server.IsRunning());

  std::stringstream ss;
  ss << "grpc://localhost:" << server.port();
  std::string uri = ss.str();

  std::unique_ptr<FlightClient> client;
  Location location1;
  Location location2;
  ASSERT_OK(Location::Parse(uri, &location1));
  ASSERT_OK(Location::Parse(uri, &location2));
  ASSERT_OK(FlightClient::Connect(location1, &client));
  ASSERT_OK(FlightClient::Connect(location2, &client));
}

TEST(TestFlight, RoundTripTypes) {
  Ticket ticket{"foo"};
  std::string ticket_serialized;
  Ticket ticket_deserialized;
  ASSERT_OK(ticket.SerializeToString(&ticket_serialized));
  ASSERT_OK(Ticket::Deserialize(ticket_serialized, &ticket_deserialized));
  ASSERT_EQ(ticket.ticket, ticket_deserialized.ticket);

  FlightDescriptor desc = FlightDescriptor::Command("select * from foo;");
  std::string desc_serialized;
  FlightDescriptor desc_deserialized;
  ASSERT_OK(desc.SerializeToString(&desc_serialized));
  ASSERT_OK(FlightDescriptor::Deserialize(desc_serialized, &desc_deserialized));
  ASSERT_TRUE(desc.Equals(desc_deserialized));

  desc = FlightDescriptor::Path({"a", "b", "test.arrow"});
  ASSERT_OK(desc.SerializeToString(&desc_serialized));
  ASSERT_OK(FlightDescriptor::Deserialize(desc_serialized, &desc_deserialized));
  ASSERT_TRUE(desc.Equals(desc_deserialized));

  FlightInfo::Data data;
  std::shared_ptr<Schema> schema =
      arrow::schema({field("a", int64()), field("b", int64()), field("c", int64()),
                     field("d", int64())});
  Location location1, location2, location3;
  ASSERT_OK(Location::ForGrpcTcp("localhost", 10010, &location1));
  ASSERT_OK(Location::ForGrpcTls("localhost", 10010, &location2));
  ASSERT_OK(Location::ForGrpcUnix("/tmp/test.sock", &location3));
  std::vector<FlightEndpoint> endpoints{FlightEndpoint{ticket, {location1, location2}},
                                        FlightEndpoint{ticket, {location3}}};
  ASSERT_OK(MakeFlightInfo(*schema, desc, endpoints, -1, -1, &data));
  std::unique_ptr<FlightInfo> info = std::unique_ptr<FlightInfo>(new FlightInfo(data));
  std::string info_serialized;
  std::unique_ptr<FlightInfo> info_deserialized;
  ASSERT_OK(info->SerializeToString(&info_serialized));
  ASSERT_OK(FlightInfo::Deserialize(info_serialized, &info_deserialized));
  ASSERT_TRUE(info->descriptor().Equals(info_deserialized->descriptor()));
  ASSERT_EQ(info->endpoints(), info_deserialized->endpoints());
  ASSERT_EQ(info->total_records(), info_deserialized->total_records());
  ASSERT_EQ(info->total_bytes(), info_deserialized->total_bytes());
}

TEST(TestFlight, RoundtripStatus) {
  // Make sure status codes round trip through our conversions

  std::shared_ptr<FlightStatusDetail> detail;
  detail = FlightStatusDetail::UnwrapStatus(
      MakeFlightError(FlightStatusCode::Internal, "Test message"));
  ASSERT_NE(nullptr, detail);
  ASSERT_EQ(FlightStatusCode::Internal, detail->code());

  detail = FlightStatusDetail::UnwrapStatus(
      MakeFlightError(FlightStatusCode::TimedOut, "Test message"));
  ASSERT_NE(nullptr, detail);
  ASSERT_EQ(FlightStatusCode::TimedOut, detail->code());

  detail = FlightStatusDetail::UnwrapStatus(
      MakeFlightError(FlightStatusCode::Cancelled, "Test message"));
  ASSERT_NE(nullptr, detail);
  ASSERT_EQ(FlightStatusCode::Cancelled, detail->code());

  detail = FlightStatusDetail::UnwrapStatus(
      MakeFlightError(FlightStatusCode::Unauthenticated, "Test message"));
  ASSERT_NE(nullptr, detail);
  ASSERT_EQ(FlightStatusCode::Unauthenticated, detail->code());

  detail = FlightStatusDetail::UnwrapStatus(
      MakeFlightError(FlightStatusCode::Unauthorized, "Test message"));
  ASSERT_NE(nullptr, detail);
  ASSERT_EQ(FlightStatusCode::Unauthorized, detail->code());

  detail = FlightStatusDetail::UnwrapStatus(
      MakeFlightError(FlightStatusCode::Unavailable, "Test message"));
  ASSERT_NE(nullptr, detail);
  ASSERT_EQ(FlightStatusCode::Unavailable, detail->code());

  Status status = internal::FromGrpcStatus(
      internal::ToGrpcStatus(Status::NotImplemented("Sentinel")));
  ASSERT_TRUE(status.IsNotImplemented());
  ASSERT_THAT(status.message(), ::testing::HasSubstr("Sentinel"));

  status = internal::FromGrpcStatus(internal::ToGrpcStatus(Status::Invalid("Sentinel")));
  ASSERT_TRUE(status.IsInvalid());
  ASSERT_THAT(status.message(), ::testing::HasSubstr("Sentinel"));

  status = internal::FromGrpcStatus(internal::ToGrpcStatus(Status::KeyError("Sentinel")));
  ASSERT_TRUE(status.IsKeyError());
  ASSERT_THAT(status.message(), ::testing::HasSubstr("Sentinel"));

  status =
      internal::FromGrpcStatus(internal::ToGrpcStatus(Status::AlreadyExists("Sentinel")));
  ASSERT_TRUE(status.IsAlreadyExists());
  ASSERT_THAT(status.message(), ::testing::HasSubstr("Sentinel"));
}

TEST(TestFlight, GetPort) {
  Location location;
  std::unique_ptr<FlightServerBase> server = ExampleTestServer();

  ASSERT_OK(Location::ForGrpcTcp("localhost", 0, &location));
  FlightServerOptions options(location);
  ASSERT_OK(server->Init(options));
  ASSERT_GT(server->port(), 0);
}

TEST(TestFlight, BuilderHook) {
  Location location;
  std::unique_ptr<FlightServerBase> server = ExampleTestServer();

  ASSERT_OK(Location::ForGrpcTcp("localhost", 0, &location));
  FlightServerOptions options(location);
  bool builder_hook_run = false;
  options.builder_hook = [&builder_hook_run](void* builder) {
    ASSERT_NE(nullptr, builder);
    builder_hook_run = true;
  };
  ASSERT_OK(server->Init(options));
  ASSERT_TRUE(builder_hook_run);
  ASSERT_GT(server->port(), 0);
  ASSERT_OK(server->Shutdown());
}

// ----------------------------------------------------------------------
// Client tests

// Helper to initialize a server and matching client with callbacks to
// populate options.
template <typename T, typename... Args>
Status MakeServer(std::unique_ptr<FlightServerBase>* server,
                  std::unique_ptr<FlightClient>* client,
                  std::function<Status(FlightServerOptions*)> make_server_options,
                  std::function<Status(FlightClientOptions*)> make_client_options,
                  Args&&... server_args) {
  Location location;
  RETURN_NOT_OK(Location::ForGrpcTcp("localhost", 0, &location));
  *server = arrow::internal::make_unique<T>(std::forward<Args>(server_args)...);
  FlightServerOptions server_options(location);
  RETURN_NOT_OK(make_server_options(&server_options));
  RETURN_NOT_OK((*server)->Init(server_options));
  Location real_location;
  RETURN_NOT_OK(Location::ForGrpcTcp("localhost", (*server)->port(), &real_location));
  FlightClientOptions client_options = FlightClientOptions::Defaults();
  RETURN_NOT_OK(make_client_options(&client_options));
  return FlightClient::Connect(real_location, client_options, client);
}

class TestFlightClient : public ::testing::Test {
 public:
  void SetUp() {
    server_ = ExampleTestServer();

    Location location;
    ASSERT_OK(Location::ForGrpcTcp("localhost", 0, &location));
    FlightServerOptions options(location);
    ASSERT_OK(server_->Init(options));

    ASSERT_OK(ConnectClient());
  }

  void TearDown() { ASSERT_OK(server_->Shutdown()); }

  Status ConnectClient() {
    Location location;
    RETURN_NOT_OK(Location::ForGrpcTcp("localhost", server_->port(), &location));
    return FlightClient::Connect(location, &client_);
  }

  template <typename EndpointCheckFunc>
  void CheckDoGet(const FlightDescriptor& descr, const BatchVector& expected_batches,
                  EndpointCheckFunc&& check_endpoints) {
    auto expected_schema = expected_batches[0]->schema();

    std::unique_ptr<FlightInfo> info;
    ASSERT_OK(client_->GetFlightInfo(descr, &info));
    check_endpoints(info->endpoints());

    std::shared_ptr<Schema> schema;
    ipc::DictionaryMemo dict_memo;
    ASSERT_OK(info->GetSchema(&dict_memo, &schema));
    AssertSchemaEqual(*expected_schema, *schema);

    // By convention, fetch the first endpoint
    Ticket ticket = info->endpoints()[0].ticket;
    CheckDoGet(ticket, expected_batches);
  }

  void CheckDoGet(const Ticket& ticket, const BatchVector& expected_batches) {
    auto num_batches = static_cast<int>(expected_batches.size());
    ASSERT_GE(num_batches, 2);

    std::unique_ptr<FlightStreamReader> stream;
    ASSERT_OK(client_->DoGet(ticket, &stream));

    FlightStreamChunk chunk;
    for (int i = 0; i < num_batches; ++i) {
      ASSERT_OK(stream->Next(&chunk));
      ASSERT_NE(nullptr, chunk.data);
#if !defined(__MINGW32__)
      ASSERT_BATCHES_EQUAL(*expected_batches[i], *chunk.data);
#else
      // In MINGW32, the following code does not have the reproducibility at the LSB
      // even when this is called twice with the same seed.
      // As a workaround, use approxEqual
      //   /* from GenerateTypedData in random.cc */
      //   std::default_random_engine rng(seed);  // seed = 282475250
      //   std::uniform_real_distribution<double> dist;
      //   std::generate(data, data + n,          // n = 10
      //                 [&dist, &rng] { return static_cast<ValueType>(dist(rng)); });
      //   /* data[1] = 0x40852cdfe23d3976 or 0x40852cdfe23d3975 */
      ASSERT_BATCHES_APPROX_EQUAL(*expected_batches[i], *chunk.data);
#endif
    }

    // Stream exhausted
    ASSERT_OK(stream->Next(&chunk));
    ASSERT_EQ(nullptr, chunk.data);
  }

 protected:
  std::unique_ptr<FlightClient> client_;
  std::unique_ptr<FlightServerBase> server_;
};

class AuthTestServer : public FlightServerBase {
  Status DoAction(const ServerCallContext& context, const Action& action,
                  std::unique_ptr<ResultStream>* result) override {
    auto buf = Buffer::FromString(context.peer_identity());
    auto peer = Buffer::FromString(context.peer());
    *result = std::unique_ptr<ResultStream>(
        new SimpleResultStream({Result{buf}, Result{peer}}));
    return Status::OK();
  }
};

class TlsTestServer : public FlightServerBase {
  Status DoAction(const ServerCallContext& context, const Action& action,
                  std::unique_ptr<ResultStream>* result) override {
    auto buf = Buffer::FromString("Hello, world!");
    *result = std::unique_ptr<ResultStream>(new SimpleResultStream({Result{buf}}));
    return Status::OK();
  }
};

class DoPutTestServer : public FlightServerBase {
 public:
  Status DoPut(const ServerCallContext& context,
               std::unique_ptr<FlightMessageReader> reader,
               std::unique_ptr<FlightMetadataWriter> writer) override {
    descriptor_ = reader->descriptor();
    return reader->ReadAll(&batches_);
  }

 protected:
  FlightDescriptor descriptor_;
  BatchVector batches_;

  friend class TestDoPut;
};

class MetadataTestServer : public FlightServerBase {
  Status DoGet(const ServerCallContext& context, const Ticket& request,
               std::unique_ptr<FlightDataStream>* data_stream) override {
    BatchVector batches;
    if (request.ticket == "dicts") {
      RETURN_NOT_OK(ExampleDictBatches(&batches));
    } else if (request.ticket == "floats") {
      RETURN_NOT_OK(ExampleFloatBatches(&batches));
    } else {
      RETURN_NOT_OK(ExampleIntBatches(&batches));
    }
    std::shared_ptr<RecordBatchReader> batch_reader =
        std::make_shared<BatchIterator>(batches[0]->schema(), batches);

    *data_stream = std::unique_ptr<FlightDataStream>(new NumberingStream(
        std::unique_ptr<FlightDataStream>(new RecordBatchStream(batch_reader))));
    return Status::OK();
  }

  Status DoPut(const ServerCallContext& context,
               std::unique_ptr<FlightMessageReader> reader,
               std::unique_ptr<FlightMetadataWriter> writer) override {
    FlightStreamChunk chunk;
    int counter = 0;
    while (true) {
      RETURN_NOT_OK(reader->Next(&chunk));
      if (chunk.data == nullptr) break;
      if (chunk.app_metadata == nullptr) {
        return Status::Invalid("Expected application metadata to be provided");
      }
      if (std::to_string(counter) != chunk.app_metadata->ToString()) {
        return Status::Invalid("Expected metadata value: " + std::to_string(counter) +
                               " but got: " + chunk.app_metadata->ToString());
      }
      auto metadata = Buffer::FromString(std::to_string(counter));
      RETURN_NOT_OK(writer->WriteMetadata(*metadata));
      counter++;
    }
    return Status::OK();
  }
};

// Server for testing custom IPC options support
class OptionsTestServer : public FlightServerBase {
  Status DoGet(const ServerCallContext& context, const Ticket& request,
               std::unique_ptr<FlightDataStream>* data_stream) override {
    BatchVector batches;
    RETURN_NOT_OK(ExampleNestedBatches(&batches));
    auto reader = std::make_shared<BatchIterator>(batches[0]->schema(), batches);
    *data_stream = std::unique_ptr<FlightDataStream>(new RecordBatchStream(reader));
    return Status::OK();
  }

  // Just echo the number of batches written. The client will try to
  // call this method with different write options set.
  Status DoPut(const ServerCallContext& context,
               std::unique_ptr<FlightMessageReader> reader,
               std::unique_ptr<FlightMetadataWriter> writer) override {
    FlightStreamChunk chunk;
    int counter = 0;
    while (true) {
      RETURN_NOT_OK(reader->Next(&chunk));
      if (chunk.data == nullptr) break;
      counter++;
    }
    auto metadata = Buffer::FromString(std::to_string(counter));
    return writer->WriteMetadata(*metadata);
  }

  // Echo client data, but with write options set to limit the nesting
  // level.
  Status DoExchange(const ServerCallContext& context,
                    std::unique_ptr<FlightMessageReader> reader,
                    std::unique_ptr<FlightMessageWriter> writer) override {
    FlightStreamChunk chunk;
    auto options = ipc::IpcWriteOptions::Defaults();
    options.max_recursion_depth = 1;
    bool begun = false;
    while (true) {
      RETURN_NOT_OK(reader->Next(&chunk));
      if (!chunk.data && !chunk.app_metadata) {
        break;
      }
      if (!begun && chunk.data) {
        begun = true;
        RETURN_NOT_OK(writer->Begin(chunk.data->schema(), options));
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
};

class TestMetadata : public ::testing::Test {
 public:
  void SetUp() {
    ASSERT_OK(MakeServer<MetadataTestServer>(
        &server_, &client_, [](FlightServerOptions* options) { return Status::OK(); },
        [](FlightClientOptions* options) { return Status::OK(); }));
  }

  void TearDown() { ASSERT_OK(server_->Shutdown()); }

 protected:
  std::unique_ptr<FlightClient> client_;
  std::unique_ptr<FlightServerBase> server_;
};

class TestOptions : public ::testing::Test {
 public:
  void SetUp() {
    ASSERT_OK(MakeServer<OptionsTestServer>(
        &server_, &client_, [](FlightServerOptions* options) { return Status::OK(); },
        [](FlightClientOptions* options) { return Status::OK(); }));
  }

  void TearDown() { ASSERT_OK(server_->Shutdown()); }

 protected:
  std::unique_ptr<FlightClient> client_;
  std::unique_ptr<FlightServerBase> server_;
};

class TestAuthHandler : public ::testing::Test {
 public:
  void SetUp() {
    ASSERT_OK(MakeServer<AuthTestServer>(
        &server_, &client_,
        [](FlightServerOptions* options) {
          options->auth_handler = std::unique_ptr<ServerAuthHandler>(
              new TestServerAuthHandler("user", "p4ssw0rd"));
          return Status::OK();
        },
        [](FlightClientOptions* options) { return Status::OK(); }));
  }

  void TearDown() { ASSERT_OK(server_->Shutdown()); }

 protected:
  std::unique_ptr<FlightClient> client_;
  std::unique_ptr<FlightServerBase> server_;
};

class TestBasicAuthHandler : public ::testing::Test {
 public:
  void SetUp() {
    ASSERT_OK(MakeServer<AuthTestServer>(
        &server_, &client_,
        [](FlightServerOptions* options) {
          options->auth_handler = std::unique_ptr<ServerAuthHandler>(
              new TestServerBasicAuthHandler("user", "p4ssw0rd"));
          return Status::OK();
        },
        [](FlightClientOptions* options) { return Status::OK(); }));
  }

  void TearDown() { ASSERT_OK(server_->Shutdown()); }

 protected:
  std::unique_ptr<FlightClient> client_;
  std::unique_ptr<FlightServerBase> server_;
};

class TestDoPut : public ::testing::Test {
 public:
  void SetUp() {
    ASSERT_OK(MakeServer<DoPutTestServer>(
        &server_, &client_, [](FlightServerOptions* options) { return Status::OK(); },
        [](FlightClientOptions* options) { return Status::OK(); }));
    do_put_server_ = (DoPutTestServer*)server_.get();
  }

  void TearDown() { ASSERT_OK(server_->Shutdown()); }

  void CheckBatches(FlightDescriptor expected_descriptor,
                    const BatchVector& expected_batches) {
    ASSERT_TRUE(do_put_server_->descriptor_.Equals(expected_descriptor));
    ASSERT_EQ(do_put_server_->batches_.size(), expected_batches.size());
    for (size_t i = 0; i < expected_batches.size(); ++i) {
      ASSERT_BATCHES_EQUAL(*do_put_server_->batches_[i], *expected_batches[i]);
    }
  }

  void CheckDoPut(FlightDescriptor descr, const std::shared_ptr<Schema>& schema,
                  const BatchVector& batches) {
    std::unique_ptr<FlightStreamWriter> stream;
    std::unique_ptr<FlightMetadataReader> reader;
    ASSERT_OK(client_->DoPut(descr, schema, &stream, &reader));
    for (const auto& batch : batches) {
      ASSERT_OK(stream->WriteRecordBatch(*batch));
    }
    ASSERT_OK(stream->DoneWriting());
    ASSERT_OK(stream->Close());

    CheckBatches(descr, batches);
  }

 protected:
  std::unique_ptr<FlightClient> client_;
  std::unique_ptr<FlightServerBase> server_;
  DoPutTestServer* do_put_server_;
};

class TestTls : public ::testing::Test {
 public:
  void SetUp() {
    // Manually initialize gRPC to try to ensure some thread-locals
    // get initialized.
    // https://github.com/grpc/grpc/issues/13856
    // https://github.com/grpc/grpc/issues/20311
    // In general, gRPC on MacOS struggles with TLS (both in the sense
    // of thread-locals and encryption)
    grpc_init();

    server_.reset(new TlsTestServer);

    Location location;
    ASSERT_OK(Location::ForGrpcTls("localhost", 0, &location));
    FlightServerOptions options(location);
    ASSERT_RAISES(UnknownError, server_->Init(options));
    ASSERT_OK(ExampleTlsCertificates(&options.tls_certificates));
    ASSERT_OK(server_->Init(options));

    ASSERT_OK(Location::ForGrpcTls("localhost", server_->port(), &location_));
    ASSERT_OK(ConnectClient());
  }

  void TearDown() {
    ASSERT_OK(server_->Shutdown());
    grpc_shutdown();
  }

  Status ConnectClient() {
    auto options = FlightClientOptions();
    CertKeyPair root_cert;
    RETURN_NOT_OK(ExampleTlsCertificateRoot(&root_cert));
    options.tls_root_certs = root_cert.pem_cert;
    return FlightClient::Connect(location_, options, &client_);
  }

 protected:
  Location location_;
  std::unique_ptr<FlightClient> client_;
  std::unique_ptr<FlightServerBase> server_;
};

// A server middleware that rejects all calls.
class RejectServerMiddlewareFactory : public ServerMiddlewareFactory {
  Status StartCall(const CallInfo& info, const CallHeaders& incoming_headers,
                   std::shared_ptr<ServerMiddleware>* middleware) override {
    return MakeFlightError(FlightStatusCode::Unauthenticated, "All calls are rejected");
  }
};

// A server middleware that counts the number of successful and failed
// calls.
class CountingServerMiddleware : public ServerMiddleware {
 public:
  CountingServerMiddleware(std::atomic<int>* successful, std::atomic<int>* failed)
      : successful_(successful), failed_(failed) {}
  void SendingHeaders(AddCallHeaders* outgoing_headers) override {}
  void CallCompleted(const Status& status) override {
    if (status.ok()) {
      ARROW_IGNORE_EXPR((*successful_)++);
    } else {
      ARROW_IGNORE_EXPR((*failed_)++);
    }
  }

  std::string name() const override { return "CountingServerMiddleware"; }

 private:
  std::atomic<int>* successful_;
  std::atomic<int>* failed_;
};

class CountingServerMiddlewareFactory : public ServerMiddlewareFactory {
 public:
  CountingServerMiddlewareFactory() : successful_(0), failed_(0) {}

  Status StartCall(const CallInfo& info, const CallHeaders& incoming_headers,
                   std::shared_ptr<ServerMiddleware>* middleware) override {
    *middleware = std::make_shared<CountingServerMiddleware>(&successful_, &failed_);
    return Status::OK();
  }

  std::atomic<int> successful_;
  std::atomic<int> failed_;
};

// The current span ID, used to emulate OpenTracing style distributed
// tracing. Only used for communication between application code and
// client middleware.
static thread_local std::string current_span_id = "";

// A server middleware that stores the current span ID, in an
// emulation of OpenTracing style distributed tracing.
class TracingServerMiddleware : public ServerMiddleware {
 public:
  explicit TracingServerMiddleware(const std::string& current_span_id)
      : span_id(current_span_id) {}
  void SendingHeaders(AddCallHeaders* outgoing_headers) override {}
  void CallCompleted(const Status& status) override {}

  std::string name() const override { return "TracingServerMiddleware"; }

  std::string span_id;
};

class TracingServerMiddlewareFactory : public ServerMiddlewareFactory {
 public:
  TracingServerMiddlewareFactory() {}

  Status StartCall(const CallInfo& info, const CallHeaders& incoming_headers,
                   std::shared_ptr<ServerMiddleware>* middleware) override {
    const std::pair<CallHeaders::const_iterator, CallHeaders::const_iterator>& iter_pair =
        incoming_headers.equal_range("x-tracing-span-id");
    if (iter_pair.first != iter_pair.second) {
      const util::string_view& value = (*iter_pair.first).second;
      *middleware = std::make_shared<TracingServerMiddleware>(std::string(value));
    }
    return Status::OK();
  }
};

// A client middleware that adds a thread-local "request ID" to
// outgoing calls as a header, and keeps track of the status of
// completed calls. NOT thread-safe.
class PropagatingClientMiddleware : public ClientMiddleware {
 public:
  explicit PropagatingClientMiddleware(std::atomic<int>* received_headers,
                                       std::vector<Status>* recorded_status)
      : received_headers_(received_headers), recorded_status_(recorded_status) {}

  void SendingHeaders(AddCallHeaders* outgoing_headers) {
    // Pick up the span ID from thread locals. We have to use a
    // thread-local for communication, since we aren't even
    // instantiated until after the application code has already
    // started the call (and so there's no chance for application code
    // to pass us parameters directly).
    outgoing_headers->AddHeader("x-tracing-span-id", current_span_id);
  }

  void ReceivedHeaders(const CallHeaders& incoming_headers) { (*received_headers_)++; }

  void CallCompleted(const Status& status) { recorded_status_->push_back(status); }

 private:
  std::atomic<int>* received_headers_;
  std::vector<Status>* recorded_status_;
};

class PropagatingClientMiddlewareFactory : public ClientMiddlewareFactory {
 public:
  void StartCall(const CallInfo& info, std::unique_ptr<ClientMiddleware>* middleware) {
    recorded_calls_.push_back(info.method);
    *middleware = arrow::internal::make_unique<PropagatingClientMiddleware>(
        &received_headers_, &recorded_status_);
  }

  void Reset() {
    recorded_calls_.clear();
    recorded_status_.clear();
    received_headers_.fetch_and(0);
  }

  std::vector<FlightMethod> recorded_calls_;
  std::vector<Status> recorded_status_;
  std::atomic<int> received_headers_;
};

class ReportContextTestServer : public FlightServerBase {
  Status DoAction(const ServerCallContext& context, const Action& action,
                  std::unique_ptr<ResultStream>* result) override {
    std::shared_ptr<Buffer> buf;
    const ServerMiddleware* middleware = context.GetMiddleware("tracing");
    if (middleware == nullptr || middleware->name() != "TracingServerMiddleware") {
      buf = Buffer::FromString("");
    } else {
      buf = Buffer::FromString(((const TracingServerMiddleware*)middleware)->span_id);
    }
    *result = std::unique_ptr<ResultStream>(new SimpleResultStream({Result{buf}}));
    return Status::OK();
  }
};

class ErrorMiddlewareServer : public FlightServerBase {
  Status DoAction(const ServerCallContext& context, const Action& action,
                  std::unique_ptr<ResultStream>* result) override {
    std::string msg = "error_message";
    auto buf = Buffer::FromString("");

    std::shared_ptr<FlightStatusDetail> flightStatusDetail(
        new FlightStatusDetail(FlightStatusCode::Failed, msg));
    *result = std::unique_ptr<ResultStream>(new SimpleResultStream({Result{buf}}));
    return Status(StatusCode::ExecutionError, "test failed", flightStatusDetail);
  }
};

class PropagatingTestServer : public FlightServerBase {
 public:
  explicit PropagatingTestServer(std::unique_ptr<FlightClient> client)
      : client_(std::move(client)) {}

  Status DoAction(const ServerCallContext& context, const Action& action,
                  std::unique_ptr<ResultStream>* result) override {
    const ServerMiddleware* middleware = context.GetMiddleware("tracing");
    if (middleware == nullptr || middleware->name() != "TracingServerMiddleware") {
      current_span_id = "";
    } else {
      current_span_id = ((const TracingServerMiddleware*)middleware)->span_id;
    }

    return client_->DoAction(action, result);
  }

 private:
  std::unique_ptr<FlightClient> client_;
};

class TestRejectServerMiddleware : public ::testing::Test {
 public:
  void SetUp() {
    ASSERT_OK(MakeServer<MetadataTestServer>(
        &server_, &client_,
        [](FlightServerOptions* options) {
          options->middleware.push_back(
              {"reject", std::make_shared<RejectServerMiddlewareFactory>()});
          return Status::OK();
        },
        [](FlightClientOptions* options) { return Status::OK(); }));
  }

  void TearDown() { ASSERT_OK(server_->Shutdown()); }

 protected:
  std::unique_ptr<FlightClient> client_;
  std::unique_ptr<FlightServerBase> server_;
};

class TestCountingServerMiddleware : public ::testing::Test {
 public:
  void SetUp() {
    request_counter_ = std::make_shared<CountingServerMiddlewareFactory>();
    ASSERT_OK(MakeServer<MetadataTestServer>(
        &server_, &client_,
        [&](FlightServerOptions* options) {
          options->middleware.push_back({"request_counter", request_counter_});
          return Status::OK();
        },
        [](FlightClientOptions* options) { return Status::OK(); }));
  }

  void TearDown() { ASSERT_OK(server_->Shutdown()); }

 protected:
  std::shared_ptr<CountingServerMiddlewareFactory> request_counter_;
  std::unique_ptr<FlightClient> client_;
  std::unique_ptr<FlightServerBase> server_;
};

// Setup for this test is 2 servers
// 1. Client makes request to server A with a request ID set
// 2. server A extracts the request ID and makes a request to server B
//    with the same request ID set
// 3. server B extracts the request ID and sends it back
// 4. server A returns the response of server B
// 5. Client validates the response
class TestPropagatingMiddleware : public ::testing::Test {
 public:
  void SetUp() {
    server_middleware_ = std::make_shared<TracingServerMiddlewareFactory>();
    second_client_middleware_ = std::make_shared<PropagatingClientMiddlewareFactory>();
    client_middleware_ = std::make_shared<PropagatingClientMiddlewareFactory>();

    std::unique_ptr<FlightClient> server_client;
    ASSERT_OK(MakeServer<ReportContextTestServer>(
        &second_server_, &server_client,
        [&](FlightServerOptions* options) {
          options->middleware.push_back({"tracing", server_middleware_});
          return Status::OK();
        },
        [&](FlightClientOptions* options) {
          options->middleware.push_back(second_client_middleware_);
          return Status::OK();
        }));

    ASSERT_OK(MakeServer<PropagatingTestServer>(
        &first_server_, &client_,
        [&](FlightServerOptions* options) {
          options->middleware.push_back({"tracing", server_middleware_});
          return Status::OK();
        },
        [&](FlightClientOptions* options) {
          options->middleware.push_back(client_middleware_);
          return Status::OK();
        },
        std::move(server_client)));
  }

  void ValidateStatus(const Status& status, const FlightMethod& method) {
    ASSERT_EQ(1, client_middleware_->received_headers_);
    ASSERT_EQ(method, client_middleware_->recorded_calls_.at(0));
    ASSERT_EQ(status.code(), client_middleware_->recorded_status_.at(0).code());
  }

  void TearDown() {
    ASSERT_OK(first_server_->Shutdown());
    ASSERT_OK(second_server_->Shutdown());
  }

  void CheckHeader(const std::string& header, const std::string& value,
                   const CallHeaders::const_iterator& it) {
    // Construct a string_view before comparison to satisfy MSVC
    util::string_view header_view(header.data(), header.length());
    util::string_view value_view(value.data(), value.length());
    ASSERT_EQ(header_view, (*it).first);
    ASSERT_EQ(value_view, (*it).second);
  }

 protected:
  std::unique_ptr<FlightClient> client_;
  std::unique_ptr<FlightServerBase> first_server_;
  std::unique_ptr<FlightServerBase> second_server_;
  std::shared_ptr<TracingServerMiddlewareFactory> server_middleware_;
  std::shared_ptr<PropagatingClientMiddlewareFactory> second_client_middleware_;
  std::shared_ptr<PropagatingClientMiddlewareFactory> client_middleware_;
};

class TestErrorMiddleware : public ::testing::Test {
 public:
  void SetUp() {
    ASSERT_OK(MakeServer<ErrorMiddlewareServer>(
        &server_, &client_, [](FlightServerOptions* options) { return Status::OK(); },
        [](FlightClientOptions* options) { return Status::OK(); }));
  }

  void TearDown() { ASSERT_OK(server_->Shutdown()); }

 protected:
  std::unique_ptr<FlightClient> client_;
  std::unique_ptr<FlightServerBase> server_;
};

TEST_F(TestErrorMiddleware, TestMetadata) {
  Action action;
  std::unique_ptr<ResultStream> stream;

  // Run action1
  action.type = "action1";

  action.body = Buffer::FromString("action1-content");
  Status s = client_->DoAction(action, &stream);
  ASSERT_FALSE(s.ok());
  std::shared_ptr<FlightStatusDetail> flightStatusDetail =
      FlightStatusDetail::UnwrapStatus(s);
  ASSERT_TRUE(flightStatusDetail);
  ASSERT_EQ(flightStatusDetail->extra_info(), "error_message");
}

TEST_F(TestFlightClient, ListFlights) {
  std::unique_ptr<FlightListing> listing;
  ASSERT_OK(client_->ListFlights(&listing));
  ASSERT_TRUE(listing != nullptr);

  std::vector<FlightInfo> flights = ExampleFlightInfo();

  std::unique_ptr<FlightInfo> info;
  for (const FlightInfo& flight : flights) {
    ASSERT_OK(listing->Next(&info));
    AssertEqual(flight, *info);
  }
  ASSERT_OK(listing->Next(&info));
  ASSERT_TRUE(info == nullptr);

  ASSERT_OK(listing->Next(&info));
  ASSERT_TRUE(info == nullptr);
}

TEST_F(TestFlightClient, ListFlightsWithCriteria) {
  std::unique_ptr<FlightListing> listing;
  ASSERT_OK(client_->ListFlights(FlightCallOptions(), {"foo"}, &listing));
  std::unique_ptr<FlightInfo> info;
  ASSERT_OK(listing->Next(&info));
  ASSERT_TRUE(info == nullptr);
}

TEST_F(TestFlightClient, GetFlightInfo) {
  auto descr = FlightDescriptor::Path({"examples", "ints"});
  std::unique_ptr<FlightInfo> info;

  ASSERT_OK(client_->GetFlightInfo(descr, &info));
  ASSERT_NE(info, nullptr);

  std::vector<FlightInfo> flights = ExampleFlightInfo();
  AssertEqual(flights[0], *info);
}

TEST_F(TestFlightClient, GetSchema) {
  auto descr = FlightDescriptor::Path({"examples", "ints"});
  std::unique_ptr<SchemaResult> schema_result;
  std::shared_ptr<Schema> schema;
  ipc::DictionaryMemo dict_memo;

  ASSERT_OK(client_->GetSchema(descr, &schema_result));
  ASSERT_NE(schema_result, nullptr);
  ASSERT_OK(schema_result->GetSchema(&dict_memo, &schema));
}

TEST_F(TestFlightClient, GetFlightInfoNotFound) {
  auto descr = FlightDescriptor::Path({"examples", "things"});
  std::unique_ptr<FlightInfo> info;
  // XXX Ideally should be Invalid (or KeyError), but gRPC doesn't support
  // multiple error codes.
  auto st = client_->GetFlightInfo(descr, &info);
  ASSERT_RAISES(Invalid, st);
  ASSERT_NE(st.message().find("Flight not found"), std::string::npos);
}

TEST_F(TestFlightClient, DoGetInts) {
  auto descr = FlightDescriptor::Path({"examples", "ints"});
  BatchVector expected_batches;
  ASSERT_OK(ExampleIntBatches(&expected_batches));

  auto check_endpoints = [](const std::vector<FlightEndpoint>& endpoints) {
    // Two endpoints in the example FlightInfo
    ASSERT_EQ(2, endpoints.size());
    AssertEqual(Ticket{"ticket-ints-1"}, endpoints[0].ticket);
  };

  CheckDoGet(descr, expected_batches, check_endpoints);
}

TEST_F(TestFlightClient, DoGetFloats) {
  auto descr = FlightDescriptor::Path({"examples", "floats"});
  BatchVector expected_batches;
  ASSERT_OK(ExampleFloatBatches(&expected_batches));

  auto check_endpoints = [](const std::vector<FlightEndpoint>& endpoints) {
    // One endpoint in the example FlightInfo
    ASSERT_EQ(1, endpoints.size());
    AssertEqual(Ticket{"ticket-floats-1"}, endpoints[0].ticket);
  };

  CheckDoGet(descr, expected_batches, check_endpoints);
}

TEST_F(TestFlightClient, DoGetDicts) {
  auto descr = FlightDescriptor::Path({"examples", "dicts"});
  BatchVector expected_batches;
  ASSERT_OK(ExampleDictBatches(&expected_batches));

  auto check_endpoints = [](const std::vector<FlightEndpoint>& endpoints) {
    // One endpoint in the example FlightInfo
    ASSERT_EQ(1, endpoints.size());
    AssertEqual(Ticket{"ticket-dicts-1"}, endpoints[0].ticket);
  };

  CheckDoGet(descr, expected_batches, check_endpoints);
}

// Ensure the gRPC client is configured to allow large messages
// Tests a 32 MiB batch
TEST_F(TestFlightClient, DoGetLargeBatch) {
  BatchVector expected_batches;
  ASSERT_OK(ExampleLargeBatches(&expected_batches));
  Ticket ticket{"ticket-large-batch-1"};
  CheckDoGet(ticket, expected_batches);
}

TEST_F(TestFlightClient, DoExchange) {
  auto descr = FlightDescriptor::Command("counter");
  BatchVector batches;
  auto a1 = ArrayFromJSON(int32(), "[4, 5, 6, null]");
  auto schema = arrow::schema({field("f1", a1->type())});
  batches.push_back(RecordBatch::Make(schema, a1->length(), {a1}));
  std::unique_ptr<FlightStreamReader> reader;
  std::unique_ptr<FlightStreamWriter> writer;
  ASSERT_OK(client_->DoExchange(descr, &writer, &reader));
  ASSERT_OK(writer->Begin(schema));
  for (const auto& batch : batches) {
    ASSERT_OK(writer->WriteRecordBatch(*batch));
  }
  ASSERT_OK(writer->DoneWriting());
  FlightStreamChunk chunk;
  ASSERT_OK(reader->Next(&chunk));
  ASSERT_NE(nullptr, chunk.app_metadata);
  ASSERT_EQ(nullptr, chunk.data);
  ASSERT_EQ("1", chunk.app_metadata->ToString());
  ASSERT_OK_AND_ASSIGN(auto server_schema, reader->GetSchema());
  AssertSchemaEqual(schema, server_schema);
  for (const auto& batch : batches) {
    ASSERT_OK(reader->Next(&chunk));
    ASSERT_BATCHES_EQUAL(*batch, *chunk.data);
  }
  ASSERT_OK(writer->Close());
}

// Test pure-metadata DoExchange to ensure nothing blocks waiting for
// schema messages
TEST_F(TestFlightClient, DoExchangeNoData) {
  auto descr = FlightDescriptor::Command("counter");
  std::unique_ptr<FlightStreamReader> reader;
  std::unique_ptr<FlightStreamWriter> writer;
  ASSERT_OK(client_->DoExchange(descr, &writer, &reader));
  ASSERT_OK(writer->DoneWriting());
  FlightStreamChunk chunk;
  ASSERT_OK(reader->Next(&chunk));
  ASSERT_EQ(nullptr, chunk.data);
  ASSERT_NE(nullptr, chunk.app_metadata);
  ASSERT_EQ("0", chunk.app_metadata->ToString());
  ASSERT_OK(writer->Close());
}

// Test sending a schema without any data, as this hits an edge case
// in the client-side writer.
TEST_F(TestFlightClient, DoExchangeWriteOnlySchema) {
  auto descr = FlightDescriptor::Command("counter");
  std::unique_ptr<FlightStreamReader> reader;
  std::unique_ptr<FlightStreamWriter> writer;
  ASSERT_OK(client_->DoExchange(descr, &writer, &reader));
  auto schema = arrow::schema({field("f1", arrow::int32())});
  ASSERT_OK(writer->Begin(schema));
  ASSERT_OK(writer->WriteMetadata(Buffer::FromString("foo")));
  ASSERT_OK(writer->DoneWriting());
  FlightStreamChunk chunk;
  ASSERT_OK(reader->Next(&chunk));
  ASSERT_EQ(nullptr, chunk.data);
  ASSERT_NE(nullptr, chunk.app_metadata);
  ASSERT_EQ("0", chunk.app_metadata->ToString());
  ASSERT_OK(writer->Close());
}

// Emulate DoGet
TEST_F(TestFlightClient, DoExchangeGet) {
  auto descr = FlightDescriptor::Command("get");
  std::unique_ptr<FlightStreamReader> reader;
  std::unique_ptr<FlightStreamWriter> writer;
  ASSERT_OK(client_->DoExchange(descr, &writer, &reader));
  ASSERT_OK_AND_ASSIGN(auto server_schema, reader->GetSchema());
  AssertSchemaEqual(*ExampleIntSchema(), *server_schema);
  BatchVector batches;
  ASSERT_OK(ExampleIntBatches(&batches));
  FlightStreamChunk chunk;
  for (const auto& batch : batches) {
    ASSERT_OK(reader->Next(&chunk));
    ASSERT_NE(nullptr, chunk.data);
    AssertBatchesEqual(*batch, *chunk.data);
  }
  ASSERT_OK(reader->Next(&chunk));
  ASSERT_EQ(nullptr, chunk.data);
  ASSERT_EQ(nullptr, chunk.app_metadata);
  ASSERT_OK(writer->Close());
}

// Emulate DoPut
TEST_F(TestFlightClient, DoExchangePut) {
  auto descr = FlightDescriptor::Command("put");
  std::unique_ptr<FlightStreamReader> reader;
  std::unique_ptr<FlightStreamWriter> writer;
  ASSERT_OK(client_->DoExchange(descr, &writer, &reader));
  ASSERT_OK(writer->Begin(ExampleIntSchema()));
  BatchVector batches;
  ASSERT_OK(ExampleIntBatches(&batches));
  for (const auto& batch : batches) {
    ASSERT_OK(writer->WriteRecordBatch(*batch));
  }
  ASSERT_OK(writer->DoneWriting());
  FlightStreamChunk chunk;
  ASSERT_OK(reader->Next(&chunk));
  ASSERT_NE(nullptr, chunk.app_metadata);
  AssertBufferEqual(*chunk.app_metadata, "done");
  ASSERT_OK(reader->Next(&chunk));
  ASSERT_EQ(nullptr, chunk.data);
  ASSERT_EQ(nullptr, chunk.app_metadata);
  ASSERT_OK(writer->Close());
}

// Test the echo server
TEST_F(TestFlightClient, DoExchangeEcho) {
  auto descr = FlightDescriptor::Command("echo");
  std::unique_ptr<FlightStreamReader> reader;
  std::unique_ptr<FlightStreamWriter> writer;
  ASSERT_OK(client_->DoExchange(descr, &writer, &reader));
  ASSERT_OK(writer->Begin(ExampleIntSchema()));
  BatchVector batches;
  FlightStreamChunk chunk;
  ASSERT_OK(ExampleIntBatches(&batches));
  for (const auto& batch : batches) {
    ASSERT_OK(writer->WriteRecordBatch(*batch));
    ASSERT_OK(reader->Next(&chunk));
    ASSERT_NE(nullptr, chunk.data);
    ASSERT_EQ(nullptr, chunk.app_metadata);
    AssertBatchesEqual(*batch, *chunk.data);
  }
  for (int i = 0; i < 10; i++) {
    const auto buf = Buffer::FromString(std::to_string(i));
    ASSERT_OK(writer->WriteMetadata(buf));
    ASSERT_OK(reader->Next(&chunk));
    ASSERT_EQ(nullptr, chunk.data);
    ASSERT_NE(nullptr, chunk.app_metadata);
    AssertBufferEqual(*buf, *chunk.app_metadata);
  }
  int index = 0;
  for (const auto& batch : batches) {
    const auto buf = Buffer::FromString(std::to_string(index));
    ASSERT_OK(writer->WriteWithMetadata(*batch, buf));
    ASSERT_OK(reader->Next(&chunk));
    ASSERT_NE(nullptr, chunk.data);
    ASSERT_NE(nullptr, chunk.app_metadata);
    AssertBatchesEqual(*batch, *chunk.data);
    AssertBufferEqual(*buf, *chunk.app_metadata);
    index++;
  }
  ASSERT_OK(writer->DoneWriting());
  ASSERT_OK(reader->Next(&chunk));
  ASSERT_EQ(nullptr, chunk.data);
  ASSERT_EQ(nullptr, chunk.app_metadata);
  ASSERT_OK(writer->Close());
}

// Test interleaved reading/writing
TEST_F(TestFlightClient, DoExchangeTotal) {
  auto descr = FlightDescriptor::Command("total");
  std::unique_ptr<FlightStreamReader> reader;
  std::unique_ptr<FlightStreamWriter> writer;
  {
    auto a1 = ArrayFromJSON(arrow::int32(), "[4, 5, 6, null]");
    auto schema = arrow::schema({field("f1", a1->type())});
    // XXX: as noted in flight/client.cc, Begin() is lazy and the
    // schema message won't be written until some data is also
    // written. There's also timing issues; hence we check each status
    // here.
    EXPECT_RAISES_WITH_MESSAGE_THAT(
        Invalid, ::testing::HasSubstr("Field is not INT64: f1"), ([&]() {
          RETURN_NOT_OK(client_->DoExchange(descr, &writer, &reader));
          RETURN_NOT_OK(writer->Begin(schema));
          auto batch = RecordBatch::Make(schema, /* num_rows */ 4, {a1});
          RETURN_NOT_OK(writer->WriteRecordBatch(*batch));
          return writer->Close();
        })());
  }
  {
    auto a1 = ArrayFromJSON(arrow::int64(), "[1, 2, null, 3]");
    auto a2 = ArrayFromJSON(arrow::int64(), "[null, 4, 5, 6]");
    auto schema = arrow::schema({field("f1", a1->type()), field("f2", a2->type())});
    ASSERT_OK(client_->DoExchange(descr, &writer, &reader));
    ASSERT_OK(writer->Begin(schema));
    auto batch = RecordBatch::Make(schema, /* num_rows */ 4, {a1, a2});
    FlightStreamChunk chunk;
    ASSERT_OK(writer->WriteRecordBatch(*batch));
    ASSERT_OK_AND_ASSIGN(auto server_schema, reader->GetSchema());
    AssertSchemaEqual(*schema, *server_schema);

    ASSERT_OK(reader->Next(&chunk));
    ASSERT_NE(nullptr, chunk.data);
    auto expected1 = RecordBatch::Make(
        schema, /* num_rows */ 1,
        {ArrayFromJSON(arrow::int64(), "[6]"), ArrayFromJSON(arrow::int64(), "[15]")});
    AssertBatchesEqual(*expected1, *chunk.data);

    ASSERT_OK(writer->WriteRecordBatch(*batch));
    ASSERT_OK(reader->Next(&chunk));
    ASSERT_NE(nullptr, chunk.data);
    auto expected2 = RecordBatch::Make(
        schema, /* num_rows */ 1,
        {ArrayFromJSON(arrow::int64(), "[12]"), ArrayFromJSON(arrow::int64(), "[30]")});
    AssertBatchesEqual(*expected2, *chunk.data);

    ASSERT_OK(writer->Close());
  }
}

// Ensure server errors get propagated no matter what we try
TEST_F(TestFlightClient, DoExchangeError) {
  auto descr = FlightDescriptor::Command("error");
  std::unique_ptr<FlightStreamReader> reader;
  std::unique_ptr<FlightStreamWriter> writer;
  {
    ASSERT_OK(client_->DoExchange(descr, &writer, &reader));
    auto status = writer->Close();
    EXPECT_RAISES_WITH_MESSAGE_THAT(
        NotImplemented, ::testing::HasSubstr("Expected error"), writer->Close());
  }
  {
    ASSERT_OK(client_->DoExchange(descr, &writer, &reader));
    FlightStreamChunk chunk;
    EXPECT_RAISES_WITH_MESSAGE_THAT(
        NotImplemented, ::testing::HasSubstr("Expected error"), reader->Next(&chunk));
  }
  {
    ASSERT_OK(client_->DoExchange(descr, &writer, &reader));
    EXPECT_RAISES_WITH_MESSAGE_THAT(
        NotImplemented, ::testing::HasSubstr("Expected error"), reader->GetSchema());
  }
  // writer->Begin isn't tested here because, as noted in client.cc,
  // OpenRecordBatchWriter lazily writes the initial message - hence
  // Begin() won't fail. Additionally, it appears gRPC may buffer
  // writes - a write won't immediately fail even when the server
  // immediately fails.
}

TEST_F(TestFlightClient, ListActions) {
  std::vector<ActionType> actions;
  ASSERT_OK(client_->ListActions(&actions));

  std::vector<ActionType> expected = ExampleActionTypes();
  AssertEqual(expected, actions);
}

TEST_F(TestFlightClient, DoAction) {
  Action action;
  std::unique_ptr<ResultStream> stream;
  std::unique_ptr<Result> result;

  // Run action1
  action.type = "action1";

  const std::string action1_value = "action1-content";
  action.body = Buffer::FromString(action1_value);
  ASSERT_OK(client_->DoAction(action, &stream));

  for (int i = 0; i < 3; ++i) {
    ASSERT_OK(stream->Next(&result));
    std::string expected = action1_value + "-part" + std::to_string(i);
    ASSERT_EQ(expected, result->body->ToString());
  }

  // stream consumed
  ASSERT_OK(stream->Next(&result));
  ASSERT_EQ(nullptr, result);

  // Run action2, no results
  action.type = "action2";
  ASSERT_OK(client_->DoAction(action, &stream));

  ASSERT_OK(stream->Next(&result));
  ASSERT_EQ(nullptr, result);
}

TEST_F(TestFlightClient, RoundTripStatus) {
  const auto descr = FlightDescriptor::Command("status-outofmemory");
  std::unique_ptr<FlightInfo> info;
  const auto status = client_->GetFlightInfo(descr, &info);
  ASSERT_RAISES(OutOfMemory, status);
}

TEST_F(TestFlightClient, Issue5095) {
  // Make sure the server-side error message is reflected to the
  // client
  Ticket ticket1{"ARROW-5095-fail"};
  std::unique_ptr<FlightStreamReader> stream;
  Status status = client_->DoGet(ticket1, &stream);
  ASSERT_RAISES(UnknownError, status);
  ASSERT_THAT(status.message(), ::testing::HasSubstr("Server-side error"));

  Ticket ticket2{"ARROW-5095-success"};
  status = client_->DoGet(ticket2, &stream);
  ASSERT_RAISES(KeyError, status);
  ASSERT_THAT(status.message(), ::testing::HasSubstr("No data"));
}

// Test setting generic transport options by configuring gRPC to fail
// all calls.
TEST_F(TestFlightClient, GenericOptions) {
  std::unique_ptr<FlightClient> client;
  auto options = FlightClientOptions::Defaults();
  // Set a very low limit at the gRPC layer to fail all calls
  options.generic_options.emplace_back(GRPC_ARG_MAX_RECEIVE_MESSAGE_LENGTH, 32);
  Location location;
  ASSERT_OK(Location::ForGrpcTcp("localhost", server_->port(), &location));
  ASSERT_OK(FlightClient::Connect(location, options, &client));
  auto descr = FlightDescriptor::Path({"examples", "ints"});
  std::unique_ptr<SchemaResult> schema_result;
  std::shared_ptr<Schema> schema;
  ipc::DictionaryMemo dict_memo;
  auto status = client->GetSchema(descr, &schema_result);
  ASSERT_RAISES(Invalid, status);
  ASSERT_THAT(status.message(), ::testing::HasSubstr("resource exhausted"));
}

TEST_F(TestFlightClient, TimeoutFires) {
  // Server does not exist on this port, so call should fail
  std::unique_ptr<FlightClient> client;
  Location location;
  ASSERT_OK(Location::ForGrpcTcp("localhost", 30001, &location));
  ASSERT_OK(FlightClient::Connect(location, &client));
  FlightCallOptions options;
  options.timeout = TimeoutDuration{0.2};
  std::unique_ptr<FlightInfo> info;
  auto start = std::chrono::system_clock::now();
  Status status = client->GetFlightInfo(options, FlightDescriptor{}, &info);
  auto end = std::chrono::system_clock::now();
#ifdef ARROW_WITH_TIMING_TESTS
  EXPECT_LE(end - start, std::chrono::milliseconds{400});
#else
  ARROW_UNUSED(end - start);
#endif
  ASSERT_RAISES(IOError, status);
}

TEST_F(TestFlightClient, NoTimeout) {
  // Call should complete quickly, so timeout should not fire
  FlightCallOptions options;
  options.timeout = TimeoutDuration{5.0};  // account for slow server process startup
  std::unique_ptr<FlightInfo> info;
  auto start = std::chrono::system_clock::now();
  auto descriptor = FlightDescriptor::Path({"examples", "ints"});
  Status status = client_->GetFlightInfo(options, descriptor, &info);
  auto end = std::chrono::system_clock::now();
#ifdef ARROW_WITH_TIMING_TESTS
  EXPECT_LE(end - start, std::chrono::milliseconds{600});
#else
  ARROW_UNUSED(end - start);
#endif
  ASSERT_OK(status);
  ASSERT_NE(nullptr, info);
}

TEST_F(TestDoPut, DoPutInts) {
  auto descr = FlightDescriptor::Path({"ints"});
  BatchVector batches;
  auto a0 = ArrayFromJSON(int8(), "[0, 1, 127, -128, null]");
  auto a1 = ArrayFromJSON(uint8(), "[0, 1, 127, 255, null]");
  auto a2 = ArrayFromJSON(int16(), "[0, 258, 32767, -32768, null]");
  auto a3 = ArrayFromJSON(uint16(), "[0, 258, 32767, 65535, null]");
  auto a4 = ArrayFromJSON(int32(), "[0, 65538, 2147483647, -2147483648, null]");
  auto a5 = ArrayFromJSON(uint32(), "[0, 65538, 2147483647, 4294967295, null]");
  auto a6 = ArrayFromJSON(
      int64(), "[0, 4294967298, 9223372036854775807, -9223372036854775808, null]");
  auto a7 = ArrayFromJSON(
      uint64(), "[0, 4294967298, 9223372036854775807, 18446744073709551615, null]");
  auto schema = arrow::schema({field("f0", a0->type()), field("f1", a1->type()),
                               field("f2", a2->type()), field("f3", a3->type()),
                               field("f4", a4->type()), field("f5", a5->type()),
                               field("f6", a6->type()), field("f7", a7->type())});
  batches.push_back(
      RecordBatch::Make(schema, a0->length(), {a0, a1, a2, a3, a4, a5, a6, a7}));

  CheckDoPut(descr, schema, batches);
}

TEST_F(TestDoPut, DoPutFloats) {
  auto descr = FlightDescriptor::Path({"floats"});
  BatchVector batches;
  auto a0 = ArrayFromJSON(float32(), "[0, 1.2, -3.4, 5.6, null]");
  auto a1 = ArrayFromJSON(float64(), "[0, 1.2, -3.4, 5.6, null]");
  auto schema = arrow::schema({field("f0", a0->type()), field("f1", a1->type())});
  batches.push_back(RecordBatch::Make(schema, a0->length(), {a0, a1}));

  CheckDoPut(descr, schema, batches);
}

TEST_F(TestDoPut, DoPutEmptyBatch) {
  // Sending and receiving a 0-sized batch shouldn't fail
  auto descr = FlightDescriptor::Path({"ints"});
  BatchVector batches;
  auto a1 = ArrayFromJSON(int32(), "[]");
  auto schema = arrow::schema({field("f1", a1->type())});
  batches.push_back(RecordBatch::Make(schema, a1->length(), {a1}));

  CheckDoPut(descr, schema, batches);
}

TEST_F(TestDoPut, DoPutDicts) {
  auto descr = FlightDescriptor::Path({"dicts"});
  BatchVector batches;
  auto dict_values = ArrayFromJSON(utf8(), "[\"foo\", \"bar\", \"quux\"]");
  auto ty = dictionary(int8(), dict_values->type());
  auto schema = arrow::schema({field("f1", ty)});
  // Make several batches
  for (const char* json : {"[1, 0, 1]", "[null]", "[null, 1]"}) {
    auto indices = ArrayFromJSON(int8(), json);
    auto dict_array = std::make_shared<DictionaryArray>(ty, indices, dict_values);
    batches.push_back(RecordBatch::Make(schema, dict_array->length(), {dict_array}));
  }

  CheckDoPut(descr, schema, batches);
}

// Ensure the gRPC server is configured to allow large messages
// Tests a 32 MiB batch
TEST_F(TestDoPut, DoPutLargeBatch) {
  auto descr = FlightDescriptor::Path({"large-batches"});
  auto schema = ExampleLargeSchema();
  BatchVector batches;
  ASSERT_OK(ExampleLargeBatches(&batches));
  CheckDoPut(descr, schema, batches);
}

TEST_F(TestDoPut, DoPutSizeLimit) {
  const int64_t size_limit = 4096;
  Location location;
  ASSERT_OK(Location::ForGrpcTcp("localhost", server_->port(), &location));
  FlightClientOptions client_options;
  client_options.write_size_limit_bytes = size_limit;
  std::unique_ptr<FlightClient> client;
  ASSERT_OK(FlightClient::Connect(location, client_options, &client));

  auto descr = FlightDescriptor::Path({"ints"});
  // Batch is too large to fit in one message
  auto schema = arrow::schema({field("f1", arrow::int64())});
  auto batch = arrow::ConstantArrayGenerator::Zeroes(768, schema);
  BatchVector batches;
  batches.push_back(batch->Slice(0, 384));
  batches.push_back(batch->Slice(384));

  std::unique_ptr<FlightStreamWriter> stream;
  std::unique_ptr<FlightMetadataReader> reader;
  ASSERT_OK(client->DoPut(descr, schema, &stream, &reader));

  // Large batch will exceed the limit
  const auto status = stream->WriteRecordBatch(*batch);
  EXPECT_RAISES_WITH_MESSAGE_THAT(Invalid, ::testing::HasSubstr("exceeded soft limit"),
                                  status);
  auto detail = FlightWriteSizeStatusDetail::UnwrapStatus(status);
  ASSERT_NE(nullptr, detail);
  ASSERT_EQ(size_limit, detail->limit());
  ASSERT_GT(detail->actual(), size_limit);

  // But we can retry with a smaller batch
  for (const auto& batch : batches) {
    ASSERT_OK(stream->WriteRecordBatch(*batch));
  }

  ASSERT_OK(stream->DoneWriting());
  ASSERT_OK(stream->Close());
  CheckBatches(descr, batches);
}

TEST_F(TestAuthHandler, PassAuthenticatedCalls) {
  ASSERT_OK(client_->Authenticate(
      {},
      std::unique_ptr<ClientAuthHandler>(new TestClientAuthHandler("user", "p4ssw0rd"))));

  Status status;
  std::unique_ptr<FlightListing> listing;
  status = client_->ListFlights(&listing);
  ASSERT_RAISES(NotImplemented, status);

  std::unique_ptr<ResultStream> results;
  Action action;
  action.type = "";
  action.body = Buffer::FromString("");
  status = client_->DoAction(action, &results);
  ASSERT_OK(status);

  std::vector<ActionType> actions;
  status = client_->ListActions(&actions);
  ASSERT_RAISES(NotImplemented, status);

  std::unique_ptr<FlightInfo> info;
  status = client_->GetFlightInfo(FlightDescriptor{}, &info);
  ASSERT_RAISES(NotImplemented, status);

  std::unique_ptr<FlightStreamReader> stream;
  status = client_->DoGet(Ticket{}, &stream);
  ASSERT_RAISES(NotImplemented, status);

  std::unique_ptr<FlightStreamWriter> writer;
  std::unique_ptr<FlightMetadataReader> reader;
  std::shared_ptr<Schema> schema = arrow::schema({});
  status = client_->DoPut(FlightDescriptor{}, schema, &writer, &reader);
  ASSERT_OK(status);
  status = writer->Close();
  ASSERT_RAISES(NotImplemented, status);
}

TEST_F(TestAuthHandler, FailUnauthenticatedCalls) {
  Status status;
  std::unique_ptr<FlightListing> listing;
  status = client_->ListFlights(&listing);
  ASSERT_RAISES(IOError, status);
  ASSERT_THAT(status.message(), ::testing::HasSubstr("Invalid token"));

  std::unique_ptr<ResultStream> results;
  Action action;
  action.type = "";
  action.body = Buffer::FromString("");
  status = client_->DoAction(action, &results);
  ASSERT_RAISES(IOError, status);
  ASSERT_THAT(status.message(), ::testing::HasSubstr("Invalid token"));

  std::vector<ActionType> actions;
  status = client_->ListActions(&actions);
  ASSERT_RAISES(IOError, status);
  ASSERT_THAT(status.message(), ::testing::HasSubstr("Invalid token"));

  std::unique_ptr<FlightInfo> info;
  status = client_->GetFlightInfo(FlightDescriptor{}, &info);
  ASSERT_RAISES(IOError, status);
  ASSERT_THAT(status.message(), ::testing::HasSubstr("Invalid token"));

  std::unique_ptr<FlightStreamReader> stream;
  status = client_->DoGet(Ticket{}, &stream);
  ASSERT_RAISES(IOError, status);
  ASSERT_THAT(status.message(), ::testing::HasSubstr("Invalid token"));

  std::unique_ptr<FlightStreamWriter> writer;
  std::unique_ptr<FlightMetadataReader> reader;
  std::shared_ptr<Schema> schema(
      (new arrow::Schema(std::vector<std::shared_ptr<Field>>())));
  status = client_->DoPut(FlightDescriptor{}, schema, &writer, &reader);
  ASSERT_OK(status);
  status = writer->Close();
  ASSERT_RAISES(IOError, status);
  // ARROW-7583: don't check the error message here.
  // Because gRPC reports errors in some paths with booleans, instead
  // of statuses, we can fail the call without knowing why it fails,
  // instead reporting a generic error message. This is
  // nondeterministic, so don't assert any particular message here.
}

TEST_F(TestAuthHandler, CheckPeerIdentity) {
  ASSERT_OK(client_->Authenticate(
      {},
      std::unique_ptr<ClientAuthHandler>(new TestClientAuthHandler("user", "p4ssw0rd"))));

  Action action;
  action.type = "who-am-i";
  action.body = Buffer::FromString("");
  std::unique_ptr<ResultStream> results;
  ASSERT_OK(client_->DoAction(action, &results));
  ASSERT_NE(results, nullptr);

  std::unique_ptr<Result> result;
  ASSERT_OK(results->Next(&result));
  ASSERT_NE(result, nullptr);
  // Action returns the peer identity as the result.
  ASSERT_EQ(result->body->ToString(), "user");

  ASSERT_OK(results->Next(&result));
  ASSERT_NE(result, nullptr);
  // Action returns the peer address as the result.
  ASSERT_NE(result->body->ToString(), "");
}

TEST_F(TestBasicAuthHandler, PassAuthenticatedCalls) {
  ASSERT_OK(
      client_->Authenticate({}, std::unique_ptr<ClientAuthHandler>(
                                    new TestClientBasicAuthHandler("user", "p4ssw0rd"))));

  Status status;
  std::unique_ptr<FlightListing> listing;
  status = client_->ListFlights(&listing);
  ASSERT_RAISES(NotImplemented, status);

  std::unique_ptr<ResultStream> results;
  Action action;
  action.type = "";
  action.body = Buffer::FromString("");
  status = client_->DoAction(action, &results);
  ASSERT_OK(status);

  std::vector<ActionType> actions;
  status = client_->ListActions(&actions);
  ASSERT_RAISES(NotImplemented, status);

  std::unique_ptr<FlightInfo> info;
  status = client_->GetFlightInfo(FlightDescriptor{}, &info);
  ASSERT_RAISES(NotImplemented, status);

  std::unique_ptr<FlightStreamReader> stream;
  status = client_->DoGet(Ticket{}, &stream);
  ASSERT_RAISES(NotImplemented, status);

  std::unique_ptr<FlightStreamWriter> writer;
  std::unique_ptr<FlightMetadataReader> reader;
  std::shared_ptr<Schema> schema = arrow::schema({});
  status = client_->DoPut(FlightDescriptor{}, schema, &writer, &reader);
  ASSERT_OK(status);
  status = writer->Close();
  ASSERT_RAISES(NotImplemented, status);
}

TEST_F(TestBasicAuthHandler, FailUnauthenticatedCalls) {
  Status status;
  std::unique_ptr<FlightListing> listing;
  status = client_->ListFlights(&listing);
  ASSERT_RAISES(IOError, status);
  ASSERT_THAT(status.message(), ::testing::HasSubstr("Invalid token"));

  std::unique_ptr<ResultStream> results;
  Action action;
  action.type = "";
  action.body = Buffer::FromString("");
  status = client_->DoAction(action, &results);
  ASSERT_RAISES(IOError, status);
  ASSERT_THAT(status.message(), ::testing::HasSubstr("Invalid token"));

  std::vector<ActionType> actions;
  status = client_->ListActions(&actions);
  ASSERT_RAISES(IOError, status);
  ASSERT_THAT(status.message(), ::testing::HasSubstr("Invalid token"));

  std::unique_ptr<FlightInfo> info;
  status = client_->GetFlightInfo(FlightDescriptor{}, &info);
  ASSERT_RAISES(IOError, status);
  ASSERT_THAT(status.message(), ::testing::HasSubstr("Invalid token"));

  std::unique_ptr<FlightStreamReader> stream;
  status = client_->DoGet(Ticket{}, &stream);
  ASSERT_RAISES(IOError, status);
  ASSERT_THAT(status.message(), ::testing::HasSubstr("Invalid token"));

  std::unique_ptr<FlightStreamWriter> writer;
  std::unique_ptr<FlightMetadataReader> reader;
  std::shared_ptr<Schema> schema(
      (new arrow::Schema(std::vector<std::shared_ptr<Field>>())));
  status = client_->DoPut(FlightDescriptor{}, schema, &writer, &reader);
  ASSERT_OK(status);
  status = writer->Close();
  ASSERT_RAISES(IOError, status);
  ASSERT_THAT(status.message(), ::testing::HasSubstr("Invalid token"));
}

TEST_F(TestBasicAuthHandler, CheckPeerIdentity) {
  ASSERT_OK(
      client_->Authenticate({}, std::unique_ptr<ClientAuthHandler>(
                                    new TestClientBasicAuthHandler("user", "p4ssw0rd"))));

  Action action;
  action.type = "who-am-i";
  action.body = Buffer::FromString("");
  std::unique_ptr<ResultStream> results;
  ASSERT_OK(client_->DoAction(action, &results));
  ASSERT_NE(results, nullptr);

  std::unique_ptr<Result> result;
  ASSERT_OK(results->Next(&result));
  ASSERT_NE(result, nullptr);
  // Action returns the peer identity as the result.
  ASSERT_EQ(result->body->ToString(), "user");
}

TEST_F(TestTls, DoAction) {
  FlightCallOptions options;
  options.timeout = TimeoutDuration{5.0};
  Action action;
  action.type = "test";
  action.body = Buffer::FromString("");
  std::unique_ptr<ResultStream> results;
  ASSERT_OK(client_->DoAction(options, action, &results));
  ASSERT_NE(results, nullptr);

  std::unique_ptr<Result> result;
  ASSERT_OK(results->Next(&result));
  ASSERT_NE(result, nullptr);
  ASSERT_EQ(result->body->ToString(), "Hello, world!");
}

TEST_F(TestTls, OverrideHostname) {
  std::unique_ptr<FlightClient> client;
  auto client_options = FlightClientOptions();
  client_options.override_hostname = "fakehostname";
  CertKeyPair root_cert;
  ASSERT_OK(ExampleTlsCertificateRoot(&root_cert));
  client_options.tls_root_certs = root_cert.pem_cert;
  ASSERT_OK(FlightClient::Connect(location_, client_options, &client));

  FlightCallOptions options;
  options.timeout = TimeoutDuration{5.0};
  Action action;
  action.type = "test";
  action.body = Buffer::FromString("");
  std::unique_ptr<ResultStream> results;
  ASSERT_RAISES(IOError, client->DoAction(options, action, &results));
}

// Test the facility for setting generic transport options.
TEST_F(TestTls, OverrideHostnameGeneric) {
  std::unique_ptr<FlightClient> client;
  auto client_options = FlightClientOptions();
  client_options.generic_options.emplace_back(GRPC_SSL_TARGET_NAME_OVERRIDE_ARG,
                                              "fakehostname");
  CertKeyPair root_cert;
  ASSERT_OK(ExampleTlsCertificateRoot(&root_cert));
  client_options.tls_root_certs = root_cert.pem_cert;
  ASSERT_OK(FlightClient::Connect(location_, client_options, &client));

  FlightCallOptions options;
  options.timeout = TimeoutDuration{5.0};
  Action action;
  action.type = "test";
  action.body = Buffer::FromString("");
  std::unique_ptr<ResultStream> results;
  ASSERT_RAISES(IOError, client->DoAction(options, action, &results));
  // Could check error message for the gRPC error message but it isn't
  // necessarily stable
}

TEST_F(TestMetadata, DoGet) {
  Ticket ticket{""};
  std::unique_ptr<FlightStreamReader> stream;
  ASSERT_OK(client_->DoGet(ticket, &stream));

  BatchVector expected_batches;
  ASSERT_OK(ExampleIntBatches(&expected_batches));

  FlightStreamChunk chunk;
  auto num_batches = static_cast<int>(expected_batches.size());
  for (int i = 0; i < num_batches; ++i) {
    ASSERT_OK(stream->Next(&chunk));
    ASSERT_NE(nullptr, chunk.data);
    ASSERT_NE(nullptr, chunk.app_metadata);
    ASSERT_BATCHES_EQUAL(*expected_batches[i], *chunk.data);
    ASSERT_EQ(std::to_string(i), chunk.app_metadata->ToString());
  }
  ASSERT_OK(stream->Next(&chunk));
  ASSERT_EQ(nullptr, chunk.data);
}

// Test dictionaries. This tests a corner case in the reader:
// dictionary batches come in between the schema and the first record
// batch, so the server must take care to read application metadata
// from the record batch, and not one of the dictionary batches.
TEST_F(TestMetadata, DoGetDictionaries) {
  Ticket ticket{"dicts"};
  std::unique_ptr<FlightStreamReader> stream;
  ASSERT_OK(client_->DoGet(ticket, &stream));

  BatchVector expected_batches;
  ASSERT_OK(ExampleDictBatches(&expected_batches));

  FlightStreamChunk chunk;
  auto num_batches = static_cast<int>(expected_batches.size());
  for (int i = 0; i < num_batches; ++i) {
    ASSERT_OK(stream->Next(&chunk));
    ASSERT_NE(nullptr, chunk.data);
    ASSERT_NE(nullptr, chunk.app_metadata);
    ASSERT_BATCHES_EQUAL(*expected_batches[i], *chunk.data);
    ASSERT_EQ(std::to_string(i), chunk.app_metadata->ToString());
  }
  ASSERT_OK(stream->Next(&chunk));
  ASSERT_EQ(nullptr, chunk.data);
}

TEST_F(TestMetadata, DoPut) {
  std::unique_ptr<FlightStreamWriter> writer;
  std::unique_ptr<FlightMetadataReader> reader;
  std::shared_ptr<Schema> schema = ExampleIntSchema();
  ASSERT_OK(client_->DoPut(FlightDescriptor{}, schema, &writer, &reader));

  BatchVector expected_batches;
  ASSERT_OK(ExampleIntBatches(&expected_batches));

  std::shared_ptr<RecordBatch> chunk;
  std::shared_ptr<Buffer> metadata;
  auto num_batches = static_cast<int>(expected_batches.size());
  for (int i = 0; i < num_batches; ++i) {
    ASSERT_OK(writer->WriteWithMetadata(*expected_batches[i],
                                        Buffer::FromString(std::to_string(i))));
  }
  // This eventually calls grpc::ClientReaderWriter::Finish which can
  // hang if there are unread messages. So make sure our wrapper
  // around this doesn't hang (because it drains any unread messages)
  ASSERT_OK(writer->Close());
}

// Test DoPut() with dictionaries. This tests a corner case in the
// server-side reader; see DoGetDictionaries above.
TEST_F(TestMetadata, DoPutDictionaries) {
  std::unique_ptr<FlightStreamWriter> writer;
  std::unique_ptr<FlightMetadataReader> reader;
  BatchVector expected_batches;
  ASSERT_OK(ExampleDictBatches(&expected_batches));
  // ARROW-8749: don't get the schema via ExampleDictSchema because
  // DictionaryMemo uses field addresses to determine whether it's
  // seen a field before. Hence, if we use a schema that is different
  // (identity-wise) than the schema of the first batch we write,
  // we'll end up generating a duplicate set of dictionaries that
  // confuses the reader.
  ASSERT_OK(client_->DoPut(FlightDescriptor{}, expected_batches[0]->schema(), &writer,
                           &reader));
  std::shared_ptr<RecordBatch> chunk;
  std::shared_ptr<Buffer> metadata;
  auto num_batches = static_cast<int>(expected_batches.size());
  for (int i = 0; i < num_batches; ++i) {
    ASSERT_OK(writer->WriteWithMetadata(*expected_batches[i],
                                        Buffer::FromString(std::to_string(i))));
  }
  ASSERT_OK(writer->Close());
}

TEST_F(TestMetadata, DoPutReadMetadata) {
  std::unique_ptr<FlightStreamWriter> writer;
  std::unique_ptr<FlightMetadataReader> reader;
  std::shared_ptr<Schema> schema = ExampleIntSchema();
  ASSERT_OK(client_->DoPut(FlightDescriptor{}, schema, &writer, &reader));

  BatchVector expected_batches;
  ASSERT_OK(ExampleIntBatches(&expected_batches));

  std::shared_ptr<RecordBatch> chunk;
  std::shared_ptr<Buffer> metadata;
  auto num_batches = static_cast<int>(expected_batches.size());
  for (int i = 0; i < num_batches; ++i) {
    ASSERT_OK(writer->WriteWithMetadata(*expected_batches[i],
                                        Buffer::FromString(std::to_string(i))));
    ASSERT_OK(reader->ReadMetadata(&metadata));
    ASSERT_NE(nullptr, metadata);
    ASSERT_EQ(std::to_string(i), metadata->ToString());
  }
  // As opposed to DoPutDrainMetadata, now we've read the messages, so
  // make sure this still closes as expected.
  ASSERT_OK(writer->Close());
}

TEST_F(TestOptions, DoGetReadOptions) {
  // Call DoGet, but with a very low read nesting depth set to fail the call.
  Ticket ticket{""};
  auto options = FlightCallOptions();
  options.read_options.max_recursion_depth = 1;
  std::unique_ptr<FlightStreamReader> stream;
  ASSERT_OK(client_->DoGet(options, ticket, &stream));
  FlightStreamChunk chunk;
  ASSERT_RAISES(Invalid, stream->Next(&chunk));
}

TEST_F(TestOptions, DoPutWriteOptions) {
  // Call DoPut, but with a very low write nesting depth set to fail the call.
  std::unique_ptr<FlightStreamWriter> writer;
  std::unique_ptr<FlightMetadataReader> reader;
  BatchVector expected_batches;
  ASSERT_OK(ExampleNestedBatches(&expected_batches));

  auto options = FlightCallOptions();
  options.write_options.max_recursion_depth = 1;
  ASSERT_OK(client_->DoPut(options, FlightDescriptor{}, expected_batches[0]->schema(),
                           &writer, &reader));
  for (const auto& batch : expected_batches) {
    ASSERT_RAISES(Invalid, writer->WriteRecordBatch(*batch));
  }
}

TEST_F(TestOptions, DoExchangeClientWriteOptions) {
  // Call DoExchange and write nested data, but with a very low nesting depth set to
  // fail the call.
  auto options = FlightCallOptions();
  options.write_options.max_recursion_depth = 1;
  auto descr = FlightDescriptor::Command("");
  std::unique_ptr<FlightStreamReader> reader;
  std::unique_ptr<FlightStreamWriter> writer;
  ASSERT_OK(client_->DoExchange(options, descr, &writer, &reader));
  BatchVector batches;
  ASSERT_OK(ExampleNestedBatches(&batches));
  ASSERT_OK(writer->Begin(batches[0]->schema()));
  for (const auto& batch : batches) {
    ASSERT_RAISES(Invalid, writer->WriteRecordBatch(*batch));
  }
  ASSERT_OK(writer->DoneWriting());
  ASSERT_OK(writer->Close());
}

TEST_F(TestOptions, DoExchangeClientWriteOptionsBegin) {
  // Call DoExchange and write nested data, but with a very low nesting depth set to
  // fail the call. Here the options are set explicitly when we write data and not in the
  // call options.
  auto descr = FlightDescriptor::Command("");
  std::unique_ptr<FlightStreamReader> reader;
  std::unique_ptr<FlightStreamWriter> writer;
  ASSERT_OK(client_->DoExchange(descr, &writer, &reader));
  BatchVector batches;
  ASSERT_OK(ExampleNestedBatches(&batches));
  auto options = ipc::IpcWriteOptions::Defaults();
  options.max_recursion_depth = 1;
  ASSERT_OK(writer->Begin(batches[0]->schema(), options));
  for (const auto& batch : batches) {
    ASSERT_RAISES(Invalid, writer->WriteRecordBatch(*batch));
  }
  ASSERT_OK(writer->DoneWriting());
  ASSERT_OK(writer->Close());
}

TEST_F(TestOptions, DoExchangeServerWriteOptions) {
  // Call DoExchange and write nested data, but with a very low nesting depth set to fail
  // the call. (The low nesting depth is set on the server side.)
  auto descr = FlightDescriptor::Command("");
  std::unique_ptr<FlightStreamReader> reader;
  std::unique_ptr<FlightStreamWriter> writer;
  ASSERT_OK(client_->DoExchange(descr, &writer, &reader));
  BatchVector batches;
  ASSERT_OK(ExampleNestedBatches(&batches));
  ASSERT_OK(writer->Begin(batches[0]->schema()));
  FlightStreamChunk chunk;
  ASSERT_OK(writer->WriteRecordBatch(*batches[0]));
  ASSERT_OK(writer->DoneWriting());
  ASSERT_RAISES(Invalid, writer->Close());
}

TEST_F(TestRejectServerMiddleware, Rejected) {
  std::unique_ptr<FlightInfo> info;
  const auto& status = client_->GetFlightInfo(FlightDescriptor{}, &info);
  ASSERT_RAISES(IOError, status);
  ASSERT_THAT(status.message(), ::testing::HasSubstr("All calls are rejected"));
}

TEST_F(TestCountingServerMiddleware, Count) {
  std::unique_ptr<FlightInfo> info;
  const auto& status = client_->GetFlightInfo(FlightDescriptor{}, &info);
  ASSERT_RAISES(NotImplemented, status);

  Ticket ticket{""};
  std::unique_ptr<FlightStreamReader> stream;
  ASSERT_OK(client_->DoGet(ticket, &stream));

  ASSERT_EQ(1, request_counter_->failed_);

  while (true) {
    FlightStreamChunk chunk;
    ASSERT_OK(stream->Next(&chunk));
    if (chunk.data == nullptr) {
      break;
    }
  }

  ASSERT_EQ(1, request_counter_->successful_);
  ASSERT_EQ(1, request_counter_->failed_);
}

TEST_F(TestPropagatingMiddleware, Propagate) {
  Action action;
  std::unique_ptr<ResultStream> stream;
  std::unique_ptr<Result> result;

  current_span_id = "trace-id";
  client_middleware_->Reset();

  action.type = "action1";
  action.body = Buffer::FromString("action1-content");
  ASSERT_OK(client_->DoAction(action, &stream));

  ASSERT_OK(stream->Next(&result));
  ASSERT_EQ("trace-id", result->body->ToString());
  ValidateStatus(Status::OK(), FlightMethod::DoAction);
}

// For each method, make sure that the client middleware received
// headers from the server and that the proper method enum value was
// passed to the interceptor
TEST_F(TestPropagatingMiddleware, ListFlights) {
  client_middleware_->Reset();
  std::unique_ptr<FlightListing> listing;
  const Status status = client_->ListFlights(&listing);
  ASSERT_RAISES(NotImplemented, status);
  ValidateStatus(status, FlightMethod::ListFlights);
}

TEST_F(TestPropagatingMiddleware, GetFlightInfo) {
  client_middleware_->Reset();
  auto descr = FlightDescriptor::Path({"examples", "ints"});
  std::unique_ptr<FlightInfo> info;
  const Status status = client_->GetFlightInfo(descr, &info);
  ASSERT_RAISES(NotImplemented, status);
  ValidateStatus(status, FlightMethod::GetFlightInfo);
}

TEST_F(TestPropagatingMiddleware, GetSchema) {
  client_middleware_->Reset();
  auto descr = FlightDescriptor::Path({"examples", "ints"});
  std::unique_ptr<SchemaResult> result;
  const Status status = client_->GetSchema(descr, &result);
  ASSERT_RAISES(NotImplemented, status);
  ValidateStatus(status, FlightMethod::GetSchema);
}

TEST_F(TestPropagatingMiddleware, ListActions) {
  client_middleware_->Reset();
  std::vector<ActionType> actions;
  const Status status = client_->ListActions(&actions);
  ASSERT_RAISES(NotImplemented, status);
  ValidateStatus(status, FlightMethod::ListActions);
}

TEST_F(TestPropagatingMiddleware, DoGet) {
  client_middleware_->Reset();
  Ticket ticket1{"ARROW-5095-fail"};
  std::unique_ptr<FlightStreamReader> stream;
  Status status = client_->DoGet(ticket1, &stream);
  ASSERT_RAISES(NotImplemented, status);
  ValidateStatus(status, FlightMethod::DoGet);
}

TEST_F(TestPropagatingMiddleware, DoPut) {
  client_middleware_->Reset();
  auto descr = FlightDescriptor::Path({"ints"});
  auto a1 = ArrayFromJSON(int32(), "[4, 5, 6, null]");
  auto schema = arrow::schema({field("f1", a1->type())});

  std::unique_ptr<FlightStreamWriter> stream;
  std::unique_ptr<FlightMetadataReader> reader;
  ASSERT_OK(client_->DoPut(descr, schema, &stream, &reader));
  const Status status = stream->Close();
  ASSERT_RAISES(NotImplemented, status);
  ValidateStatus(status, FlightMethod::DoPut);
}

}  // namespace flight
}  // namespace arrow
