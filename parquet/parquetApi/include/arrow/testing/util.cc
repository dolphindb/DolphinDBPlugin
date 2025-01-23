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

#include "arrow/testing/util.h"

#include <chrono>
#include <cstring>
#include <random>

#ifdef _WIN32
// clang-format off
// (prevent include reordering)
#include "arrow/util/windows_compatibility.h"
#include <winsock2.h>
// clang-format on
#else
#include <arpa/inet.h>   // IWYU pragma: keep
#include <netinet/in.h>  // IWYU pragma: keep
#include <sys/socket.h>  // IWYU pragma: keep
#include <sys/stat.h>    // IWYU pragma: keep
#include <sys/types.h>   // IWYU pragma: keep
#include <sys/wait.h>    // IWYU pragma: keep
#include <unistd.h>      // IWYU pragma: keep
#endif

#include "arrow/table.h"
#include "arrow/testing/random.h"
#include "arrow/util/io_util.h"
#include "arrow/util/logging.h"

namespace arrow {

uint64_t random_seed() {
  return std::chrono::high_resolution_clock::now().time_since_epoch().count();
}

void random_null_bytes(int64_t n, double pct_null, uint8_t* null_bytes) {
  const int random_seed = 0;
  std::default_random_engine gen(random_seed);
  std::uniform_real_distribution<double> d(0.0, 1.0);
  std::generate(null_bytes, null_bytes + n,
                [&d, &gen, &pct_null] { return d(gen) > pct_null; });
}

void random_is_valid(int64_t n, double pct_null, std::vector<bool>* is_valid,
                     int random_seed) {
  std::default_random_engine gen(random_seed);
  std::uniform_real_distribution<double> d(0.0, 1.0);
  is_valid->resize(n, false);
  std::generate(is_valid->begin(), is_valid->end(),
                [&d, &gen, &pct_null] { return d(gen) > pct_null; });
}

void random_bytes(int64_t n, uint32_t seed, uint8_t* out) {
  std::default_random_engine gen(seed);
  std::uniform_int_distribution<uint32_t> d(0, std::numeric_limits<uint8_t>::max());
  std::generate(out, out + n, [&d, &gen] { return static_cast<uint8_t>(d(gen)); });
}

std::string random_string(int64_t n, uint32_t seed) {
  std::string s;
  s.resize(static_cast<size_t>(n));
  random_bytes(n, seed, reinterpret_cast<uint8_t*>(&s[0]));
  return s;
}

int32_t DecimalSize(int32_t precision) {
  DCHECK_GE(precision, 1) << "decimal precision must be greater than or equal to 1, got "
                          << precision;
  DCHECK_LE(precision, 38) << "decimal precision must be less than or equal to 38, got "
                           << precision;

  switch (precision) {
    case 1:
    case 2:
      return 1;  // 127
    case 3:
    case 4:
      return 2;  // 32,767
    case 5:
    case 6:
      return 3;  // 8,388,607
    case 7:
    case 8:
    case 9:
      return 4;  // 2,147,483,427
    case 10:
    case 11:
      return 5;  // 549,755,813,887
    case 12:
    case 13:
    case 14:
      return 6;  // 140,737,488,355,327
    case 15:
    case 16:
      return 7;  // 36,028,797,018,963,967
    case 17:
    case 18:
      return 8;  // 9,223,372,036,854,775,807
    case 19:
    case 20:
    case 21:
      return 9;  // 2,361,183,241,434,822,606,847
    case 22:
    case 23:
      return 10;  // 604,462,909,807,314,587,353,087
    case 24:
    case 25:
    case 26:
      return 11;  // 154,742,504,910,672,534,362,390,527
    case 27:
    case 28:
      return 12;  // 39,614,081,257,132,168,796,771,975,167
    case 29:
    case 30:
    case 31:
      return 13;  // 10,141,204,801,825,835,211,973,625,643,007
    case 32:
    case 33:
      return 14;  // 2,596,148,429,267,413,814,265,248,164,610,047
    case 34:
    case 35:
      return 15;  // 664,613,997,892,457,936,451,903,530,140,172,287
    case 36:
    case 37:
    case 38:
      return 16;  // 170,141,183,460,469,231,731,687,303,715,884,105,727
    default:
      DCHECK(false);
      break;
  }
  return -1;
}

void random_decimals(int64_t n, uint32_t seed, int32_t precision, uint8_t* out) {
  std::default_random_engine gen(seed);
  std::uniform_int_distribution<uint32_t> d(0, std::numeric_limits<uint8_t>::max());
  const int32_t required_bytes = DecimalSize(precision);
  constexpr int32_t byte_width = 16;
  std::fill(out, out + byte_width * n, '\0');

  for (int64_t i = 0; i < n; ++i, out += byte_width) {
    std::generate(out, out + required_bytes,
                  [&d, &gen] { return static_cast<uint8_t>(d(gen)); });

    // sign extend if the sign bit is set for the last byte generated
    // 0b10000000 == 0x80 == 128
    if ((out[required_bytes - 1] & '\x80') != 0) {
      std::fill(out + required_bytes, out + byte_width, '\xFF');
    }
  }
}

void random_ascii(int64_t n, uint32_t seed, uint8_t* out) {
  rand_uniform_int(n, seed, static_cast<int32_t>('A'), static_cast<int32_t>('z'), out);
}

int64_t CountNulls(const std::vector<uint8_t>& valid_bytes) {
  return static_cast<int64_t>(std::count(valid_bytes.cbegin(), valid_bytes.cend(), '\0'));
}

Status MakeRandomByteBuffer(int64_t length, MemoryPool* pool,
                            std::shared_ptr<ResizableBuffer>* out, uint32_t seed) {
  ARROW_ASSIGN_OR_RAISE(auto result, AllocateResizableBuffer(length, pool));
  random_bytes(length, seed, result->mutable_data());
  *out = std::move(result);
  return Status::OK();
}

Status GetTestResourceRoot(std::string* out) {
  const char* c_root = std::getenv("ARROW_TEST_DATA");
  if (!c_root) {
    return Status::IOError(
        "Test resources not found, set ARROW_TEST_DATA to <repo root>/testing/data");
  }
  *out = std::string(c_root);
  return Status::OK();
}

int GetListenPort() {
  // Get a new available port number by binding a socket to an ephemeral port
  // and then closing it.  Since ephemeral port allocation tends to avoid
  // reusing port numbers, this should give a different port number
  // every time, even across processes.
  struct sockaddr_in sin;
#ifdef _WIN32
  SOCKET sock_fd;
  auto sin_len = static_cast<int>(sizeof(sin));
  auto errno_message = []() -> std::string {
    return internal::WinErrorMessage(WSAGetLastError());
  };
#else
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
  int sock_fd;
  auto sin_len = static_cast<socklen_t>(sizeof(sin));
  auto errno_message = []() -> std::string { return internal::ErrnoMessage(errno); };
#endif

#ifdef _WIN32
  WSADATA wsa_data;
  if (WSAStartup(0x0202, &wsa_data) != 0) {
    ARROW_LOG(FATAL) << "Failed to initialize Windows Sockets";
  }
#endif

  sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sock_fd == INVALID_SOCKET) {
    Status::IOError("Failed to create TCP socket: ", errno_message()).Abort();
  }
  // First bind to ('0.0.0.0', 0)
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  if (bind(sock_fd, reinterpret_cast<struct sockaddr*>(&sin), sin_len) == SOCKET_ERROR) {
    Status::IOError("bind() failed: ", errno_message()).Abort();
  }
  // Then get actual bound port number
  if (getsockname(sock_fd, reinterpret_cast<struct sockaddr*>(&sin), &sin_len) ==
      SOCKET_ERROR) {
    Status::IOError("getsockname() failed: ", errno_message()).Abort();
  }
  int port = ntohs(sin.sin_port);
#ifdef _WIN32
  closesocket(sock_fd);
#else
  close(sock_fd);
#endif

  return port;
}

const std::vector<std::shared_ptr<DataType>>& all_dictionary_index_types() {
  static std::vector<std::shared_ptr<DataType>> types = {
      int8(), uint8(), int16(), uint16(), int32(), uint32(), int64(), uint64()};
  return types;
}

}  // namespace arrow
