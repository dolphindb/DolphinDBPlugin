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

// Dummy file for checking if TlsCredentialsOptions exists in
// the grpc::experimental namespace. gRPC versions 1.32 and higher
// put it here. This is for supporting disabling server
// validation when using TLS.

#include <grpc/grpc_security_constants.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/security/tls_credentials_options.h>

static grpc_tls_server_verification_option check(
    const grpc::experimental::TlsCredentialsOptions* options) {
  grpc_tls_server_verification_option server_opt = options->server_verification_option();
  return server_opt;
}

int main(int argc, const char** argv) {
  grpc_tls_server_verification_option opt = check(nullptr);
  return 0;
}
