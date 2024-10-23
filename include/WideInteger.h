///
/// ref: https://github.com/abseil/abseil-cpp/blob/master/absl/numeric/int128.h
///
#pragma once
#include <functional>
#include <limits>
#include <iosfwd>
#include <type_traits>

#include <stdint.h>

namespace wide_integer {

#ifndef __SIZEOF_INT128__
    #error "Current compiler does not support __int128"
#endif

using int128 = __int128;
using uint128 = unsigned __int128;

static_assert(sizeof(int128) == 16, "");
static_assert(sizeof(uint128) == 16, "");

constexpr uint128 makeUint128(uint64_t high, uint64_t low);
constexpr int128 makeInt128(int64_t high, uint64_t low);

constexpr uint128 uint128MaxValue();
constexpr uint128 uint128MinValue();

constexpr int128 int128MaxValue();
constexpr int128 int128MinValue();

}  // namespace wide_integer


namespace wide_integer {
namespace internal {
// Casts from unsigned to signed while preserving the underlying binary
// representation.
constexpr int64_t BitCastToSigned(uint64_t v) {
    // Casting an unsigned integer to a signed integer of the same
    // width is implementation defined behavior if the source value would not fit
    // in the destination type. We step around it with a roundtrip bitwise not
    // operation to make sure this function remains constexpr. Clang, GCC, and
    // MSVC optimize this to a no-op on x86-64.
    return v & (uint64_t{1} << 63) ? ~static_cast<int64_t>(~v)
                                    : static_cast<int64_t>(v);
}
constexpr __int128_t BitCastToSigned(__uint128_t v) {
    return v & (static_cast<__uint128_t>(1) << 127)
                ? ~static_cast<__int128_t>(~v)
                : static_cast<__int128_t>(v);
}

constexpr inline uint64_t Uint128High64(uint128 v) {
    return static_cast<uint64_t>(v >> 64);
}
constexpr inline uint64_t Uint128Low64(uint128 v) {
    return static_cast<uint64_t>(v);
}
constexpr uint64_t Int128Low64(int128 v) {
    return static_cast<uint64_t>(v & ~uint64_t{0});
}
constexpr int64_t Int128High64(int128 v) {
    // Initially cast to unsigned to prevent a right shift on a negative value.
    return wide_integer::internal::BitCastToSigned(
            static_cast<uint64_t>(static_cast<uint128>(v) >> 64));
}

// ref: https://www.boost.org/doc/libs/1_35_0/doc/html/boost/hash_combine_id241013.html
template <class T>
inline void hash_combine(std::size_t &seed, const T &v) {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}
}  // namespace internal

constexpr uint128 makeUint128(uint64_t high, uint64_t low) {
    return uint128((static_cast<__uint128_t>(high) << 64) | low);
}

constexpr int128 makeInt128(int64_t high, uint64_t low) {
    return int128(internal::BitCastToSigned(static_cast<__uint128_t>(high) << 64) | low);
}

constexpr uint128 uint128MaxValue() {
    return makeUint128(std::numeric_limits<uint64_t>::max(),
                       std::numeric_limits<uint64_t>::max());
}

constexpr uint128 uint128MinValue() {
    return uint128(0);
}

constexpr int128 int128MaxValue() {
    return makeInt128(std::numeric_limits<int64_t>::max(),
                      std::numeric_limits<uint64_t>::max());
}

constexpr int128 int128MinValue() {
    return makeInt128(std::numeric_limits<int64_t>::min(), 0);
}

constexpr int128 INT128_MIN = int128MinValue();

static_assert(int128MinValue() == (-int128MaxValue() - 1), "");
static_assert(uint128(int128MinValue() | int128MaxValue()) == uint128MaxValue(), "");
}  // namespace wide_integer


std::ostream& operator<<(std::ostream &os, wide_integer::uint128 v);
std::ostream& operator<<(std::ostream &os, wide_integer::int128 v);

/*
    -std=c++11 && (
        (clang < 12) ||
        (gcc < 10.3)
    )
*/
#if !(!defined(__STRICT_ANSI__) && defined(_GLIBCXX_USE_INT128)) && \
        ( \
            (defined(__clang__) && (__clang_major__ < 12)) || \
            (!defined(__clang__) && (__GNUC__ < 10 || (__GNUC__ == 10 && __GNUC_MINOR__ < 3))) \
        )
// Specialized numeric_limits for uint128 and int128.
namespace std {
template <>
class numeric_limits<wide_integer::uint128> {
public:
    static constexpr bool is_specialized = true;
    static constexpr bool is_signed = false;
    static constexpr bool is_integer = true;
    static constexpr bool is_exact = true;
    static constexpr bool has_infinity = false;
    static constexpr bool has_quiet_NaN = false;
    static constexpr bool has_signaling_NaN = false;
    static constexpr float_denorm_style has_denorm = denorm_absent;
    static constexpr bool has_denorm_loss = false;
    static constexpr float_round_style round_style = round_toward_zero;
    static constexpr bool is_iec559 = false;
    static constexpr bool is_bounded = true;
    static constexpr bool is_modulo = true;
    static constexpr int digits = 128;
    static constexpr int digits10 = 38;
    static constexpr int max_digits10 = 0;
    static constexpr int radix = 2;
    static constexpr int min_exponent = 0;
    static constexpr int min_exponent10 = 0;
    static constexpr int max_exponent = 0;
    static constexpr int max_exponent10 = 0;
    static constexpr bool tinyness_before = false;

    static constexpr wide_integer::uint128 min() { return 0; }
    static constexpr wide_integer::uint128 lowest() { return 0; }
    static constexpr wide_integer::uint128 max() { return wide_integer::uint128MaxValue(); }
    static constexpr wide_integer::uint128 epsilon() { return 0; }
    static constexpr wide_integer::uint128 round_error() { return 0; }
    static constexpr wide_integer::uint128 infinity() { return 0; }
    static constexpr wide_integer::uint128 quiet_NaN() { return 0; }
    static constexpr wide_integer::uint128 signaling_NaN() { return 0; }
    static constexpr wide_integer::uint128 denorm_min() { return 0; }
};

template <>
class numeric_limits<wide_integer::int128> {
public:
    static constexpr bool is_specialized = true;
    static constexpr bool is_signed = true;
    static constexpr bool is_integer = true;
    static constexpr bool is_exact = true;
    static constexpr bool has_infinity = false;
    static constexpr bool has_quiet_NaN = false;
    static constexpr bool has_signaling_NaN = false;
    static constexpr float_denorm_style has_denorm = denorm_absent;
    static constexpr bool has_denorm_loss = false;
    static constexpr float_round_style round_style = round_toward_zero;
    static constexpr bool is_iec559 = false;
    static constexpr bool is_bounded = true;
    static constexpr bool is_modulo = false;
    static constexpr int digits = 127;
    static constexpr int digits10 = 38;
    static constexpr int max_digits10 = 0;
    static constexpr int radix = 2;
    static constexpr int min_exponent = 0;
    static constexpr int min_exponent10 = 0;
    static constexpr int max_exponent = 0;
    static constexpr int max_exponent10 = 0;
    static constexpr bool tinyness_before = false;

    static constexpr wide_integer::int128 min() { return wide_integer::int128MinValue(); }
    static constexpr wide_integer::int128 lowest() { return wide_integer::int128MinValue(); }
    static constexpr wide_integer::int128 max() { return wide_integer::int128MaxValue(); }
    static constexpr wide_integer::int128 epsilon() { return 0; }
    static constexpr wide_integer::int128 round_error() { return 0; }
    static constexpr wide_integer::int128 infinity() { return 0; }
    static constexpr wide_integer::int128 quiet_NaN() { return 0; }
    static constexpr wide_integer::int128 signaling_NaN() { return 0; }
    static constexpr wide_integer::int128 denorm_min() { return 0; }
};

template <>
struct make_unsigned<wide_integer::int128> {
    typedef wide_integer::uint128 type;
};

template <>
struct add_pointer<wide_integer::uint128> {
    typedef wide_integer::uint128* type;
};

template <>
struct is_integral<wide_integer::int128> : public true_type {};

template <>
struct is_integral<wide_integer::uint128> : public true_type {};
}  // namespace std
#endif

static_assert(std::numeric_limits<wide_integer::int128>::min() ==
        (-std::numeric_limits<wide_integer::int128>::max() - 1), "");
static_assert(wide_integer::uint128(std::numeric_limits<wide_integer::int128>::min() |
        std::numeric_limits<wide_integer::int128>::max()) == std::numeric_limits<wide_integer::uint128>::max(), "");


/*
    clang && (
        (-std=c++11) ||
        (-std=gnu++11 && clang < 15)
    ) ||
    gcc && (
        (-std=c++11) ||
        (gcc < 6)
    )
*/
#if (defined(__clang__) && \
        ( \
            (defined(__STRICT_ANSI__)) || \
            (!(!defined(__STRICT_ANSI__) && defined(_GLIBCXX_USE_INT128)) && __clang_major__ < 15) \
        ) \
    ) || \
    (!defined(__clang__) && \
        ( \
            (defined(__STRICT_ANSI__)) || \
            (__GNUC__ < 6) \
        ) \
    )
namespace std {
// Specialized std::hash for uint128 and int128.
template <>
struct hash<wide_integer::int128> {
    size_t operator()(wide_integer::int128 v) const noexcept {
        size_t seed = 0;
        wide_integer::internal::hash_combine(seed, wide_integer::internal::Int128High64(v));
        wide_integer::internal::hash_combine(seed, wide_integer::internal::Int128Low64(v));
        return seed;
    }
};

template <>
struct hash<wide_integer::uint128> {
    size_t operator()(wide_integer::uint128 v) const noexcept {
        size_t seed = 0;
        wide_integer::internal::hash_combine(seed, wide_integer::internal::Uint128High64(v));
        wide_integer::internal::hash_combine(seed, wide_integer::internal::Uint128Low64(v));
        return seed;
    }
};
}  // namespace std
#endif

namespace std {
// FIXME: ???
wide_integer::int128 pow(wide_integer::int128 x, size_t y);
wide_integer::int128 trunc(wide_integer::int128 x);
};  // namespace std
