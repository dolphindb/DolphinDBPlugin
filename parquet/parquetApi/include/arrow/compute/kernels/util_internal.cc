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

#include "arrow/compute/kernels/util_internal.h"

#include <cstdint>

#include "arrow/array/data.h"
#include "arrow/type.h"
#include "arrow/util/checked_cast.h"

namespace arrow {

using internal::checked_cast;

namespace compute {
namespace internal {

const uint8_t* GetValidityBitmap(const ArrayData& data) {
  const uint8_t* bitmap = nullptr;
  if (data.buffers[0]) {
    bitmap = data.buffers[0]->data();
  }
  return bitmap;
}

int GetBitWidth(const DataType& type) {
  return checked_cast<const FixedWidthType&>(type).bit_width();
}

PrimitiveArg GetPrimitiveArg(const ArrayData& arr) {
  PrimitiveArg arg;
  arg.is_valid = GetValidityBitmap(arr);
  arg.data = arr.buffers[1]->data();
  arg.bit_width = GetBitWidth(*arr.type);
  arg.offset = arr.offset;
  arg.length = arr.length;
  if (arg.bit_width > 1) {
    arg.data += arr.offset * arg.bit_width / 8;
  }
  // This may be kUnknownNullCount
  arg.null_count = arr.null_count.load();
  return arg;
}

}  // namespace internal
}  // namespace compute
}  // namespace arrow
