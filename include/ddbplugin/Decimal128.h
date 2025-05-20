#pragma once

#include "WideInteger.h"
namespace {

/**
 * calculate greatest common divisor
 */
template <typename T>
inline T gcd(T a, T b) {
    T c;
    while (a != 0) {
        c = a;
        a = b % a;
        b = c;
    }
    return b;
}

// since we use min() as nullValue, so a+b=min() cause overflow
template <typename T>
inline bool addOverflow(T a, T b, T &res) {
    res = a + b;
    if (b > 0 && a > std::numeric_limits<T>::max() - b) {
        return true;
    }
    if (b < 0 && a <= std::numeric_limits<T>::min() - b) {
        return true;
    }
    return false;
}

// since we use min() as nullValue, so a-b=min() cause overflow
template <typename T>
inline bool subOverflow(T a, T b, T &res) {
    res = a - b;
    if (b < 0 && a > std::numeric_limits<T>::max() + b) {
        return true;
    }
    if (b > 0 && a <= std::numeric_limits<T>::min() + b) {
        return true;
    }
    return false;
}

template <typename T, typename U, typename R = typename std::conditional<sizeof(T) >= sizeof(U), T, U>::type>
inline bool mulOverflow(T a, U b, R &result) {
    result = a * b;
    if (a == 0 || b == 0) {
        return false;
    }

    if ((a < 0) != (b < 0)) { // different sign
        if (a == std::numeric_limits<R>::min()) {
            return b > 1;
        } else if (b == std::numeric_limits<R>::min()) {
            return a > 1;
        }
        if (a < 0) {
            return (-a) > std::numeric_limits<R>::max() / b;
        }
        if (b < 0) {
            return a > std::numeric_limits<R>::max() / (-b);
        }
    } else if (a < 0 && b < 0) {
        if (a == std::numeric_limits<R>::min()) {
            return b <= -1;
        } else if (b == std::numeric_limits<R>::min()) {
            return a <= -1;
        }
        return (-a) > std::numeric_limits<R>::max() / (-b);
    }

    return a > std::numeric_limits<R>::max() / b;
}

/**
 * @brief result = value1 * value2 / divisor
 *
 * @return overflow or not
 */
template <typename T>
inline bool mulDivOverflow(T value1, T value2, T divisor, T &result) {
    // minimize value1 & divisor
    {
        T c = gcd(value1, divisor);
        if (c != 1) {
            value1 /= c;
            divisor /= c;
        }
    }
    // minimize value2 & divisor
    {
        T c = gcd(value2, divisor);
        if (c != 1) {
            value2 /= c;
            divisor /= c;
        }
    }

    bool overflow = mulOverflow(value1, value2, result);
    result /= divisor;
    return overflow;
}

}

namespace ddb {

class decimal128
{
public:
    using valueT = int128;
    static constexpr int scale{8};
    static constexpr valueT one{100000000};

    decimal128() : value(0) {}
    explicit decimal128(valueT v)
        : value(v) { }
    explicit decimal128(int v)
        : value(v * one) { }
    explicit decimal128(double v)
        : value(v * double(one)) { }
    bool operator==(const decimal128 y) const
    {
        return value == y.value;
    }
    bool operator!=(const decimal128 y) const
    {
        return value != y.value;
    }
    bool operator>(const decimal128 y) const
    {
        return value > y.value;
    }
    bool operator>=(const decimal128 y) const
    {
        return value >= y.value;
    }
    bool operator<(const decimal128 y) const
    {
        return value < y.value;
    }
    decimal128 operator-() const
    {
        return decimal128(-value);
    }
    decimal128 operator+(const decimal128 y) const
    {
        checkNull(y);
        valueT res;
        if (addOverflow(value, y.value, res)) {
            throw RuntimeException("[PLUGIN::BACKTEST] Decimal add overflow.");
        }
        return decimal128(res);
    }
    decimal128& operator+=(const decimal128 y)
    {
        *this = *this + y;
        return *this;
    }
    decimal128 operator-(const decimal128 y) const
    {
        checkNull(y);
        valueT res;
        if (subOverflow(value, y.value, res)) {
            throw RuntimeException("[PLUGIN::BACKTEST] Decimal sub overflow.");
        }
        return decimal128(res);
    }
    decimal128& operator-=(const decimal128 y)
    {
        *this = *this - y;
        return *this;
    }
    decimal128 operator*(const decimal128 y) const
    {
        checkNull(y);
        valueT res;
        if (mulOverflow(value, y.value, res)) {
            throw RuntimeException("[PLUGIN::BACKTEST] Decimal mul overflow.");
        }
        res /= CRYPTO_NUMERAL_FACTOR;
        return decimal128(res);
    }
    decimal128 operator/(const decimal128 y) const
    {
        if (y.value == 0) {
            throw RuntimeException("[PLUGIN::BACKTEST] decimal128 division by zero");
        }
        checkNull(y);
        valueT res;
        if (mulDivOverflow(value, CRYPTO_NUMERAL_FACTOR, y.value, res)) {
            throw RuntimeException("[PLUGIN::BACKTEST] Decimal div overflow.");
        }
        return decimal128(res);
    }
    explicit operator valueT() const
    {
        return value;
    }
    explicit operator double() const
    {
        return (double)value / (double)CRYPTO_NUMERAL_FACTOR;
    }
private:

    // the raw data of 1 in decimal128 with scale 8
    static const valueT CRYPTO_NUMERAL_FACTOR = 100000000;

    valueT value{0};

    void checkNull(decimal128 y) const
    {
        // null value of decimal128 type in DDB
        if (y.value == std::numeric_limits<valueT>::min()) {
            throw RuntimeException("[PLUGIN::BACKTEST] null value in decimal128 operations");
        }
    }
};

// the scale of all decimal values in backtest are 8
static const int CRYPTO_SCALE = 8;
constexpr int decimalScale{8};

}  // namespace ddb
