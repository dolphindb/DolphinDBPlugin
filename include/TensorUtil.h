#pragma once

#include "Types.h"
#include "Util.h"

#include <cstdint>

#include <array>
#include <limits>
#include <numeric>
#include <type_traits>
#include <vector>

namespace tensor_util {
// ref: https://github.com/pytorch/pytorch/blob/a7f6b0ab4fa4e43a2fa7bcb53825277db88b55f0/torch/lib/TH/generic/THTensor.c#L639-L654
inline bool isContiguous(const std::vector<int64_t> &shape, const std::vector<int64_t> &strides) {
    if (shape.size() != strides.size()) {
        return false;
    }

    int64_t z = 1;
    const auto ndim = static_cast<int64_t>(shape.size());
    for (int64_t d = ndim - 1; d >= 0; --d) {
        if (shape[d] != 1) {
            if (shape[d] == 0) {
                return true;
            }
            if (strides[d] != z) {
                return false;
            }
            z *= shape[d];
        }
    }
    return true;
}

inline std::vector<int64_t> makeContiguousStrides(const std::vector<int64_t> &shape) {
    std::vector<int64_t> strides;

    const auto ndim = static_cast<int64_t>(shape.size());
    if (0 == ndim) {
        return strides;
    }

    strides.resize(ndim);
    strides[ndim-1] = 1;
    for (int64_t d = ndim - 2; d >= 0; --d) {
        if (0 == shape[d+1]) {
            strides[d] = strides[d+1];
        } else {
            strides[d] = strides[d+1] * shape[d+1];
        }
    }

    return strides;
}

inline int64_t computeSize(const std::vector<int64_t> &shape) {
    if (shape.empty()) {
        return 0;
    }
    return std::accumulate(shape.begin(), shape.end(), int64_t(1), std::multiplies<int64_t>{});
}

inline bool isDataTypeSupported(DATA_TYPE type) {
    switch (type) {
        case DT_BOOL:
        case DT_CHAR:
        case DT_SHORT:
        case DT_INT:
        case DT_LONG:
        case DT_FLOAT:
        case DT_DOUBLE:
            return true;
        default:
            break;
    }
    return false;
}

template <DATA_TYPE D_TYPE> struct DDBPrimitiveType;
template <> struct DDBPrimitiveType<DT_BOOL> {
    static constexpr DATA_TYPE dtype = DT_BOOL;
    using ctype = char;
    using ptype = bool;
    static constexpr ctype Null = CHAR_MIN;
};
template <> struct DDBPrimitiveType<DT_CHAR> {
    static constexpr DATA_TYPE dtype = DT_CHAR;
    using ctype = char;
    using ptype = ctype;
    static constexpr ctype Null = CHAR_MIN;
};
template <> struct DDBPrimitiveType<DT_SHORT> {
    static constexpr DATA_TYPE dtype = DT_SHORT;
    using ctype = short;
    using ptype = ctype;
    static constexpr ctype Null = SHRT_MIN;
};
template <> struct DDBPrimitiveType<DT_INT> {
    static constexpr DATA_TYPE dtype = DT_INT;
    using ctype = int;
    using ptype = ctype;
    static constexpr ctype Null = INT_MIN;
};
template <> struct DDBPrimitiveType<DT_LONG> {
    static constexpr DATA_TYPE dtype = DT_LONG;
    using ctype = long long;
    using ptype = ctype;
    static constexpr ctype Null = LLONG_MIN;
};
template <> struct DDBPrimitiveType<DT_FLOAT> {
    static constexpr DATA_TYPE dtype = DT_FLOAT;
    using ctype = float;
    using ptype = ctype;
    static constexpr ctype Null = -FLT_MAX; // std::numeric_limits<ctype>::lowest();
};
template <> struct DDBPrimitiveType<DT_DOUBLE> {
    static constexpr DATA_TYPE dtype = DT_DOUBLE;
    using ctype = double;
    using ptype = ctype;
    static constexpr ctype Null = -DBL_MAX; // std::numeric_limits<ctype>::lowest();
};

/// Primary Class Template.
template <template <DATA_TYPE...> class Functor, size_t DEPTH, typename SFINAE = void, DATA_TYPE... Types>
struct TypeDispatcher {
    typedef TypeDispatcher<Functor, DEPTH, SFINAE, Types...> Self;

    template <DATA_TYPE D_TYPE>
    using Append = TypeDispatcher<Functor, DEPTH, SFINAE, Types..., D_TYPE>;

    template <class Iter, class... Args>
    static auto call(Iter &&iter, Args &&...args)
        -> decltype(Self::Append<DT_BOOL>::call(std::next(iter), std::forward<Args>(args)...))
    {
        const auto type = *iter;
        switch (type) {
            case DT_BOOL:
                return Self::Append<DT_BOOL>::call(std::next(iter), std::forward<Args>(args)...);
            case DT_CHAR:
                return Self::Append<DT_CHAR>::call(std::next(iter), std::forward<Args>(args)...);
            case DT_SHORT:
                return Self::Append<DT_SHORT>::call(std::next(iter), std::forward<Args>(args)...);
            case DT_INT:
                return Self::Append<DT_INT>::call(std::next(iter), std::forward<Args>(args)...);
            case DT_LONG:
                return Self::Append<DT_LONG>::call(std::next(iter), std::forward<Args>(args)...);
            case DT_FLOAT:
                return Self::Append<DT_FLOAT>::call(std::next(iter), std::forward<Args>(args)...);
            case DT_DOUBLE:
                return Self::Append<DT_DOUBLE>::call(std::next(iter), std::forward<Args>(args)...);
            default:
                break;
        }
        throw RuntimeException("Failed to dispatch: unsupported data type: " + Util::getDataTypeString(type));
    }
};

/// Specialized Class Template.
template <template <DATA_TYPE...> class Functor, size_t DEPTH, DATA_TYPE... Types>
struct TypeDispatcher<Functor, DEPTH, typename std::enable_if<sizeof...(Types) == DEPTH>::type, Types...> {
    template <class Iter, class... Args>
    static auto call(Iter &&iter, Args &&...args)
        -> decltype(Functor<Types...>{}(std::forward<Args>(args)...))
    {
        return Functor<Types...>{}(std::forward<Args>(args)...);
    }
};

template <template <DATA_TYPE...> class Functor, size_t DEPTH, class... Args>
inline auto dispatch(const std::array<DATA_TYPE, DEPTH> &types, Args &&...args)
    -> decltype(TypeDispatcher<Functor, DEPTH>::call(types.begin(), std::forward<Args>(args)...))
{
    return TypeDispatcher<Functor, DEPTH>::call(types.begin(), std::forward<Args>(args)...);
}

template <template <DATA_TYPE> class Functor, class... Args>
inline auto dispatch(DATA_TYPE type, Args &&...args)
    -> decltype(dispatch<Functor>(std::array<DATA_TYPE, 1>{type}, std::forward<Args>(args)...))
{
    return dispatch<Functor>(std::array<DATA_TYPE, 1>{type}, std::forward<Args>(args)...);
}

/// Convert C++ primitive data type to DDB data type.
template <typename T, typename = void> struct Cpp2DDBPrimitiveType;
template <> struct Cpp2DDBPrimitiveType<bool> {
    static constexpr DATA_TYPE dtype = DT_BOOL;
    using type = DDBPrimitiveType<dtype>;
};
template <> struct Cpp2DDBPrimitiveType<char> {
    static constexpr DATA_TYPE dtype = DT_CHAR;
    using type = DDBPrimitiveType<dtype>;
};
template <> struct Cpp2DDBPrimitiveType<short> {
    static constexpr DATA_TYPE dtype = DT_SHORT;
    using type = DDBPrimitiveType<dtype>;
};
template <> struct Cpp2DDBPrimitiveType<int> {
    static constexpr DATA_TYPE dtype = DT_INT;
    using type = DDBPrimitiveType<dtype>;
};
template <> struct Cpp2DDBPrimitiveType<long long> {
    static constexpr DATA_TYPE dtype = DT_LONG;
    using type = DDBPrimitiveType<dtype>;
};
template <> struct Cpp2DDBPrimitiveType<float> {
    static constexpr DATA_TYPE dtype = DT_FLOAT;
    using type = DDBPrimitiveType<dtype>;
};
template <> struct Cpp2DDBPrimitiveType<double> {
    static constexpr DATA_TYPE dtype = DT_DOUBLE;
    using type = DDBPrimitiveType<dtype>;
};
template <typename T>
struct Cpp2DDBPrimitiveType<T, typename std::enable_if<std::is_same<T, int64_t>::value &&
                                                   not std::is_same<T, long long>::value>::type> {
    static constexpr DATA_TYPE dtype = DT_LONG;
    using type = DDBPrimitiveType<dtype>;
};

struct TensorFactory {
    using DeviceType = Tensor::DeviceType;
    using TensorType = Tensor::TensorType;

    /**
     * Convert DDB Constant to Tensor:
     * - Scalar => 1-dim Tensor.
     * - Vector => 1-dim Tensor.
     * - Matrix => 2-dim Tensor.
     * - Table => 2-dim Tensor.
     * - Tuple of Vector => 2-dim Tensor.
     * - Tuple of Matrix => 3-dim Tensor
     * - Tuple of (Tuple of Vector) => 3-dim Tensor.
     * - Tuple of (Tuple of (Tuple of ...)) => n-dim Tensor.
     *
     * @note DDB Matrix is column-major, Tensor is row-major.
     */
    static Tensor * makeTensor(const ConstantSP &data, TensorType tensorType = TensorType::Basic,
                               DeviceType deviceType = DeviceType::CPU);

    /// Equivalent to `torch::from_blob`.
    /// ref: https://pytorch.org/cppdocs/api/function_namespacetorch_1ac009244049812a3efdf4605d19c5e79b.html
    static Tensor * makeTensor(DATA_TYPE dataType, void *ptr, const std::vector<int64_t> &shape,
                               const std::vector<int64_t> &strides = {}, TensorType tensorType = TensorType::Basic,
                               DeviceType deviceType = DeviceType::CPU);

    /// Same as above, except without specifying the data type (automatically detect the type according to `T`).
    template <typename T>
    static Tensor * makeTensor(T *ptr, const std::vector<int64_t> &shape, const std::vector<int64_t> &strides = {},
                               TensorType tensorType = TensorType::Basic, DeviceType deviceType = DeviceType::CPU) {
        return makeTensor(tensor_util::Cpp2DDBPrimitiveType<T>::dtype, ptr, shape, strides, tensorType, deviceType);
    }
};
}  // namespace tensor_util
