#ifndef _ENDIAN_H_
#define _ENDIAN_H_

#include <type_traits>
#include <algorithm>

#if __GNUG__ && __GNUG__ < 5
//NOTE: std::is_trivially_copyable & std::is_trivially_constructible, etc.
//      from C++11 are missing in g++ <5.0
//@see https://stackoverflow.com/questions/25123458/is-trivially-copyable-is-not-a-member-of-std
namespace std {
    template<typename T>
    struct is_trivially_copyable {
        static constexpr bool value = __has_trivial_copy(T);
    };

    template<typename T>
    struct is_trivially_constructible {
        static constexpr bool value = __has_trivial_constructor(T);
    };
}
#endif//__GNUG__ && __GNUG__ < 5

namespace kdb {

    using byte = std::uint8_t;
    static_assert(sizeof(byte) == 1, "basic data unit");

    //////////////////////////////////////////////////////////////////////////

    //FIXME: Available as std::bit_cast<To, From> since C++20
    template<typename To, class From>
    typename std::enable_if<
        sizeof(To) == sizeof(From) &&
        std::is_trivially_copyable<From>::value &&
        std::is_trivially_copyable<To>::value,
        To>::type
    bit_cast(const From& from) noexcept {
        static_assert(std::is_trivially_constructible<To>::value,
            "need to default-construct To");
        To to;
        std::memcpy(&to, &from, sizeof(To));
        return to;
    }

    //FIXME: Available as std::byteswap<T> since C++23
    template<typename T>
    T& byteswap(T& val) noexcept {
        const auto bytes = bit_cast<std::array<byte, sizeof(T)>>(val);
        std::copy(bytes.rbegin(), bytes.rend(),
            static_cast<byte*>(static_cast<void*>(&val)));
        return val;
    }

    // Floating-point NaNs cannot compare to each other, compare bit-by-bit
    template<typename Flt, typename Int>
    bool bit_equal(const Flt& f, const Int& i) noexcept {
        return bit_cast<Int>(f) == i;
    }

}//namespace kdb

namespace endian {

    constexpr int lsb_detector{ 0x01 };
    static_assert(CHAR_BIT == 8, "byte size");

    enum Endianness { LITTLE, BIG };

    constexpr Endianness native() {
        return *static_cast<const kdb::byte*>(
            static_cast<const void*>(&lsb_detector)
        ) == 0x01 ? LITTLE : BIG;
    }

#if 0
    template<typename T>
    constexpr
    typename std::enable_if<
        native() == LITTLE && (sizeof(T) > 1) && std::is_integral<T>::value, T
    >::type norm(T value) noexcept {
        return value;
    }

    template<typename T>
    typename std::enable_if<
        native() == BIG && (sizeof(T) > 1) && std::is_integral<T>::value, T
    >::type norm(T value) noexcept {
        return kdb::byteswap(value);
    }
#endif

}//namespace endian

#endif//_ENDIAN_H_
