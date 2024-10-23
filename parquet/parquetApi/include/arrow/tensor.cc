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

#include "arrow/tensor.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <numeric>
#include <string>
#include <type_traits>
#include <vector>

#include "arrow/status.h"
#include "arrow/type.h"
#include "arrow/type_traits.h"
#include "arrow/util/checked_cast.h"
#include "arrow/util/logging.h"
#include "arrow/visitor_inline.h"

namespace arrow {

using internal::checked_cast;

namespace internal {

void ComputeRowMajorStrides(const FixedWidthType& type, const std::vector<int64_t>& shape,
                            std::vector<int64_t>* strides) {
  const int byte_width = GetByteWidth(type);
  int64_t remaining = byte_width;
  for (int64_t dimsize : shape) {
    remaining *= dimsize;
  }

  if (remaining == 0) {
    strides->assign(shape.size(), byte_width);
    return;
  }

  for (int64_t dimsize : shape) {
    remaining /= dimsize;
    strides->push_back(remaining);
  }
}

}  // namespace internal

static void ComputeColumnMajorStrides(const FixedWidthType& type,
                                      const std::vector<int64_t>& shape,
                                      std::vector<int64_t>* strides) {
  const int byte_width = internal::GetByteWidth(type);
  int64_t total = byte_width;
  for (int64_t dimsize : shape) {
    if (dimsize == 0) {
      strides->assign(shape.size(), byte_width);
      return;
    }
  }
  for (int64_t dimsize : shape) {
    strides->push_back(total);
    total *= dimsize;
  }
}

namespace {

inline bool IsTensorStridesRowMajor(const std::shared_ptr<DataType>& type,
                                    const std::vector<int64_t>& shape,
                                    const std::vector<int64_t>& strides) {
  std::vector<int64_t> c_strides;
  const auto& fw_type = checked_cast<const FixedWidthType&>(*type);
  internal::ComputeRowMajorStrides(fw_type, shape, &c_strides);
  return strides == c_strides;
}

inline bool IsTensorStridesColumnMajor(const std::shared_ptr<DataType>& type,
                                       const std::vector<int64_t>& shape,
                                       const std::vector<int64_t>& strides) {
  std::vector<int64_t> f_strides;
  const auto& fw_type = checked_cast<const FixedWidthType&>(*type);
  ComputeColumnMajorStrides(fw_type, shape, &f_strides);
  return strides == f_strides;
}

inline Status CheckTensorValidity(const std::shared_ptr<DataType>& type,
                                  const std::shared_ptr<Buffer>& data,
                                  const std::vector<int64_t>& shape) {
  if (!type) {
    return Status::Invalid("Null type is supplied");
  }
  if (!is_tensor_supported(type->id())) {
    return Status::Invalid(type->ToString(), " is not valid data type for a tensor");
  }
  if (!data) {
    return Status::Invalid("Null data is supplied");
  }
  if (!std::all_of(shape.begin(), shape.end(), [](int64_t x) { return x >= 0; })) {
    return Status::Invalid("Shape elements must be positive");
  }
  return Status::OK();
}

Status CheckTensorStridesValidity(const std::shared_ptr<Buffer>& data,
                                  const std::vector<int64_t>& shape,
                                  const std::vector<int64_t>& strides,
                                  const std::shared_ptr<DataType>& type) {
  if (strides.size() != shape.size()) {
    return Status::Invalid("strides must have the same length as shape");
  }
  if (data->size() == 0 && std::find(shape.begin(), shape.end(), 0) != shape.end()) {
    return Status::OK();
  }

  std::vector<int64_t> last_index(shape);
  const int64_t n = static_cast<int64_t>(shape.size());
  for (int64_t i = 0; i < n; ++i) {
    --last_index[i];
  }
  int64_t last_offset = Tensor::CalculateValueOffset(strides, last_index);
  const int byte_width = internal::GetByteWidth(*type);
  if (last_offset + byte_width > data->size()) {
    return Status::Invalid("strides must not involve buffer over run");
  }
  return Status::OK();
}

}  // namespace

namespace internal {

bool IsTensorStridesContiguous(const std::shared_ptr<DataType>& type,
                               const std::vector<int64_t>& shape,
                               const std::vector<int64_t>& strides) {
  return IsTensorStridesRowMajor(type, shape, strides) ||
         IsTensorStridesColumnMajor(type, shape, strides);
}

Status ValidateTensorParameters(const std::shared_ptr<DataType>& type,
                                const std::shared_ptr<Buffer>& data,
                                const std::vector<int64_t>& shape,
                                const std::vector<int64_t>& strides,
                                const std::vector<std::string>& dim_names) {
  RETURN_NOT_OK(CheckTensorValidity(type, data, shape));
  if (!strides.empty()) {
    RETURN_NOT_OK(CheckTensorStridesValidity(data, shape, strides, type));
  }
  if (dim_names.size() > shape.size()) {
    return Status::Invalid("too many dim_names are supplied");
  }
  return Status::OK();
}

}  // namespace internal

/// Constructor with strides and dimension names
Tensor::Tensor(const std::shared_ptr<DataType>& type, const std::shared_ptr<Buffer>& data,
               const std::vector<int64_t>& shape, const std::vector<int64_t>& strides,
               const std::vector<std::string>& dim_names)
    : type_(type), data_(data), shape_(shape), strides_(strides), dim_names_(dim_names) {
  ARROW_CHECK(is_tensor_supported(type->id()));
  if (shape.size() > 0 && strides.size() == 0) {
    internal::ComputeRowMajorStrides(checked_cast<const FixedWidthType&>(*type_), shape,
                                     &strides_);
  }
}

Tensor::Tensor(const std::shared_ptr<DataType>& type, const std::shared_ptr<Buffer>& data,
               const std::vector<int64_t>& shape, const std::vector<int64_t>& strides)
    : Tensor(type, data, shape, strides, {}) {}

Tensor::Tensor(const std::shared_ptr<DataType>& type, const std::shared_ptr<Buffer>& data,
               const std::vector<int64_t>& shape)
    : Tensor(type, data, shape, {}, {}) {}

const std::string& Tensor::dim_name(int i) const {
  static const std::string kEmpty = "";
  if (dim_names_.size() == 0) {
    return kEmpty;
  } else {
    ARROW_CHECK_LT(i, static_cast<int>(dim_names_.size()));
    return dim_names_[i];
  }
}

int64_t Tensor::size() const {
  return std::accumulate(shape_.begin(), shape_.end(), 1LL, std::multiplies<int64_t>());
}

bool Tensor::is_contiguous() const {
  return internal::IsTensorStridesContiguous(type_, shape_, strides_);
}

bool Tensor::is_row_major() const {
  return IsTensorStridesRowMajor(type_, shape_, strides_);
}

bool Tensor::is_column_major() const {
  return IsTensorStridesColumnMajor(type_, shape_, strides_);
}

Type::type Tensor::type_id() const { return type_->id(); }

bool Tensor::Equals(const Tensor& other, const EqualOptions& opts) const {
  return TensorEquals(*this, other, opts);
}

namespace {

template <typename TYPE>
int64_t StridedTensorCountNonZero(int dim_index, int64_t offset, const Tensor& tensor) {
  using c_type = typename TYPE::c_type;
  c_type const zero = c_type(0);
  int64_t nnz = 0;
  if (dim_index == tensor.ndim() - 1) {
    for (int64_t i = 0; i < tensor.shape()[dim_index]; ++i) {
      auto const* ptr = tensor.raw_data() + offset + i * tensor.strides()[dim_index];
      auto& elem = *reinterpret_cast<c_type const*>(ptr);
      if (elem != zero) ++nnz;
    }
    return nnz;
  }
  for (int64_t i = 0; i < tensor.shape()[dim_index]; ++i) {
    nnz += StridedTensorCountNonZero<TYPE>(dim_index + 1, offset, tensor);
    offset += tensor.strides()[dim_index];
  }
  return nnz;
}

template <typename TYPE>
int64_t ContiguousTensorCountNonZero(const Tensor& tensor) {
  using c_type = typename TYPE::c_type;
  auto* data = reinterpret_cast<c_type const*>(tensor.raw_data());
  return std::count_if(data, data + tensor.size(),
                       [](c_type const& x) { return x != 0; });
}

template <typename TYPE>
inline int64_t TensorCountNonZero(const Tensor& tensor) {
  if (tensor.is_contiguous()) {
    return ContiguousTensorCountNonZero<TYPE>(tensor);
  } else {
    return StridedTensorCountNonZero<TYPE>(0, 0, tensor);
  }
}

struct NonZeroCounter {
  explicit NonZeroCounter(const Tensor& tensor) : tensor_(tensor) {}

  template <typename TYPE>
  enable_if_number<TYPE, Status> Visit(const TYPE& type) {
    result = TensorCountNonZero<TYPE>(tensor_);
    return Status::OK();
  }

  Status Visit(const DataType& type) {
    ARROW_CHECK(!is_tensor_supported(type.id()));
    return Status::NotImplemented("Tensor of ", type.ToString(), " is not implemented");
  }

  const Tensor& tensor_;
  int64_t result;
};

}  // namespace

Result<int64_t> Tensor::CountNonZero() const {
  NonZeroCounter counter(*this);
  RETURN_NOT_OK(VisitTypeInline(*type(), &counter));
  return counter.result;
}

}  // namespace arrow
