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

#define ARROW_VERSION_MAJOR 9
#define ARROW_VERSION_MINOR 0
#define ARROW_VERSION_PATCH 0
#define ARROW_VERSION ((ARROW_VERSION_MAJOR * 1000) + ARROW_VERSION_MINOR) * 1000 + ARROW_VERSION_PATCH

#define ARROW_VERSION_STRING "9.0.0"

#define ARROW_SO_VERSION "900"
#define ARROW_FULL_SO_VERSION "900.0.0"

#define ARROW_CXX_COMPILER_ID "GNU"
#define ARROW_CXX_COMPILER_VERSION "4.8.5"
#define ARROW_CXX_COMPILER_FLAGS " -O3 -DNDEBUG"

#define ARROW_BUILD_TYPE "RELEASE"

#define ARROW_GIT_ID "634bce383eb521f3d6dc16865d56ddfd7c370d16"
#define ARROW_GIT_DESCRIPTION ""

#define ARROW_PACKAGE_KIND ""

/* #undef ARROW_COMPUTE */
/* #undef ARROW_CSV */
/* #undef ARROW_CUDA */
/* #undef ARROW_DATASET */
/* #undef ARROW_FILESYSTEM */
/* #undef ARROW_FLIGHT */
#define ARROW_IPC
#define ARROW_JEMALLOC
#define ARROW_JEMALLOC_VENDORED
/* #undef ARROW_JSON */

/* #undef ARROW_GCS */
/* #undef ARROW_S3 */
#define ARROW_USE_NATIVE_INT128
/* #undef ARROW_WITH_OPENTELEMETRY */
/* #undef ARROW_WITH_UCX */

/* #undef GRPCPP_PP_INCLUDE */
