///
/// ref: https://github.com/abseil/abseil-cpp/blob/master/absl/numeric/int128.h
///
#pragma once
#include <functional>
#include <limits>
#include <iosfwd>
#include <type_traits>
#include <stdint.h>

#include "Types.h"
#include "Util.h"


#ifndef __SIZEOF_INT128__
    #error "Current compiler does not support __int128"
#endif

namespace ddb {
constexpr long long bitCastToSigned(unsigned long long v) {
    // Casting an unsigned integer to a signed integer of the same
    // width is implementation defined behavior if the source value would not fit
    // in the destination type. We step around it with a roundtrip bitwise not
    // operation to make sure this function remains constexpr. Clang, GCC, and
    // MSVC optimize this to a no-op on x86-64.
    return v & (((unsigned long long)1) << 63) ? ~static_cast<long long>(~v) : static_cast<long long>(v);
}

constexpr inline unsigned long long uint128High64(uint128 v) {
    return static_cast<unsigned long long>(v >> 64);
}
constexpr inline unsigned long long uint128Low64(uint128 v) {
    return static_cast<unsigned long long>(v);
}
constexpr unsigned long long int128Low64(int128 v) {
    return static_cast<unsigned long long>(v & ~((unsigned long long)0));
}
constexpr long long int128High64(int128 v) {
    // Initially cast to unsigned to prevent a right shift on a negative value.
    return bitCastToSigned(static_cast<unsigned long long>(static_cast<uint128>(v) >> 64));
}
} // namespace ddb

namespace std {
using ddb::int128;
using ddb::uint128;

std::ostream& operator<<(std::ostream &os, ddb::uint128 v);
std::ostream& operator<<(std::ostream &os, ddb::int128 v);

/*
    -std=c++11 && ((clang < 12) ||(gcc < 10.3))
*/
#if !(!defined(__STRICT_ANSI__) && defined(_GLIBCXX_USE_INT128)) && \
        ( \
            (defined(__clang__) && (__clang_major__ < 12)) || \
            (!defined(__clang__) && (__GNUC__ < 10 || (__GNUC__ == 10 && __GNUC_MINOR__ < 3))) \
        )
// Specialized numeric_limits for uint128 and int128.
template <>
class numeric_limits<ddb::uint128> {
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

    static constexpr uint128 min() { return 0; }
    static constexpr uint128 lowest() { return 0; }
    static constexpr uint128 max() { return ddb::uint128MaxValue(); }
    static constexpr uint128 epsilon() { return 0; }
    static constexpr uint128 round_error() { return 0; }
    static constexpr uint128 infinity() { return 0; }
    static constexpr uint128 quiet_NaN() { return 0; }
    static constexpr uint128 signaling_NaN() { return 0; }
    static constexpr uint128 denorm_min() { return 0; }
};

template <>
class numeric_limits<ddb::int128> {
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

    static constexpr int128 min() { return ddb::int128MinValue(); }
    static constexpr int128 lowest() { return ddb::int128MinValue(); }
    static constexpr int128 max() { return ddb::int128MaxValue(); }
    static constexpr int128 epsilon() { return 0; }
    static constexpr int128 round_error() { return 0; }
    static constexpr int128 infinity() { return 0; }
    static constexpr int128 quiet_NaN() { return 0; }
    static constexpr int128 signaling_NaN() { return 0; }
    static constexpr int128 denorm_min() { return 0; }
};

template <>
struct make_unsigned<ddb::int128> {
    typedef uint128 type;
};

template <>
struct add_pointer<ddb::uint128> {
    typedef uint128* type;
};

template <>
struct is_integral<ddb::int128> : public true_type {};

template <>
struct is_integral<ddb::uint128> : public true_type {};
#endif

}  // namespace std

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
	(!defined(__clang__) && ( (defined(__STRICT_ANSI__) ) || (__GNUC__ < 6) ) )

namespace std {
// Specialized std::hash for uint128 and int128.
template <>
struct hash<ddb::int128> {
    size_t operator()(ddb::int128 v) const noexcept {
        size_t seed = 0;
        ddb::hashCombine(seed, ddb::int128High64(v));
        ddb::hashCombine(seed, ddb::int128Low64(v));
        return seed;
    }
};

template <>
struct hash<ddb::uint128> {
    size_t operator()(ddb::uint128 v) const noexcept {
        size_t seed = 0;
        ddb::hashCombine(seed, ddb::uint128High64(v));
        ddb::hashCombine(seed, ddb::uint128Low64(v));
        return seed;
    }
};

}  // namespace std
#endif

namespace std {

ddb::int128 pow(ddb::int128 x, size_t y);
ddb::int128 trunc(ddb::int128 x);

} // namespace std
