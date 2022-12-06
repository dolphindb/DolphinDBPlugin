#pragma once

#include <limits>
#include <string>
#include <sstream>
#include <iomanip>
#include <map>

#include "CoreConcept.h"
#include "Util.h"

//========================================================================
// Decimal implementation details
//========================================================================
namespace decimal_util {

template <typename T>
struct MinPrecision { static constexpr int value = 1; };

template <typename T>
struct MaxPrecision { static constexpr int value = 0; };

template <> struct MaxPrecision<int32_t> { static constexpr int value = 9; };
template <> struct MaxPrecision<int64_t> { static constexpr int value = 18; };


inline int exp10_i32(int x) {
    constexpr int values[] = {
        1,
        10,
        100,
        1000,
        10000,
        100000,
        1000000,
        10000000,
        100000000,
        1000000000
    };
    return values[x];
}

inline int64_t exp10_i64(int x) {
    constexpr int64_t values[] = {
        1LL,
        10LL,
        100LL,
        1000LL,
        10000LL,
        100000LL,
        1000000LL,
        10000000LL,
        100000000LL,
        1000000000LL,
        10000000000LL,
        100000000000LL,
        1000000000000LL,
        10000000000000LL,
        100000000000000LL,
        1000000000000000LL,
        10000000000000000LL,
        100000000000000000LL,
        1000000000000000000LL
    };
    return values[x];
}

template <typename T>
inline T scaleMultiplier(int scale);

template <>
inline int32_t scaleMultiplier<int32_t>(int scale) {
        return exp10_i32(scale);
}

template <>
inline int64_t scaleMultiplier<int64_t>(int scale) {
    return exp10_i64(scale);
}


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

} // namespace decimal_util


//========================================================================
// Decimal <=> String
//========================================================================
namespace decimal_util {

template <typename T>
inline std::string toString(int scale, T rawData) {
    std::stringstream ss;

    if (scale == 0) {
        ss << rawData;
    } else {
        auto multiplier = scaleMultiplier<T>(scale);

        T integer = rawData / multiplier;
        if (rawData < 0 && integer == 0) {
            ss << '-';
        }
        ss << integer;

        int sign = rawData < 0 ? -1 : 1;
        auto frac = rawData % multiplier * sign;
        ss << "." << std::setw(scale) << std::setfill('0') << std::right << frac;
    }

    return ss.str();
}

/**
 * only support string like "0.0000000123"
 * not support "15e16"|"-0x1afp-2"|"inF"|"Nan"|"invalid"
 */
template <typename T>
inline bool parseString(const char *str, size_t str_len, T &rawData, int scale, string &errMsg) {
    const char dec_point = '.';

    enum StateEnum { IN_SIGN, IN_BEFORE_FIRST_DIG, IN_BEFORE_DEC, IN_AFTER_DEC, IN_END } state = IN_SIGN;
    enum ErrorCodes {
        ERR_WRONG_CHAR = -1,
        ERR_NO_DIGITS = -2,
        ERR_WRONG_STATE = -3,
        ERR_SCALE_ERROR = -4,
        ERR_OVERFLOW = -5,
    };
    static std::map<int, std::string> msg = {
        {ERR_WRONG_STATE, " not illegal format "},
        {ERR_NO_DIGITS, " no digits "},
        {ERR_WRONG_CHAR, " not illegal format "},
        {ERR_SCALE_ERROR, " input number contains more than scale"},
        {ERR_OVERFLOW, " Decimal math overflow"},
    };
    StateEnum prevState = IN_SIGN;
    rawData = 0;
    int sign = 1;
    int error = 0;
    int digitsCount = 0; // including '+' '-'
    int noneZeroDigitsCount = 0; // ignore zero before numbers ( which in left of '.' or '1-9')
    int afterDigitCount = 0;
    char c;
    size_t i = 0;

    while ((i < str_len) && (state != IN_END))  // loop while extraction from file is possible
    {
        c = str[i++];

        switch (state) {
            case IN_SIGN:
                if (c == '-') {
                    sign = -1;
                    state = IN_BEFORE_FIRST_DIG;
                    digitsCount++;
                } else if (c == '+') {
                    state = IN_BEFORE_FIRST_DIG;
                    digitsCount++;
                } else if ((c >= '0') && (c <= '9')) {
                    state = IN_BEFORE_DEC;
                    rawData = static_cast<int>(c - '0');
                    digitsCount++;
                    if (c != '0') {
                        noneZeroDigitsCount++;
                    }              
                } else if (c == dec_point) {
                    state = IN_AFTER_DEC;
                } else if ((c != ' ') && (c != '\t')) {
                    error = ERR_WRONG_CHAR;
                    state = IN_END;
                    prevState = IN_SIGN;
                }
                // else ignore char
                break;
            case IN_BEFORE_FIRST_DIG:
                if ((c >= '0') && (c <= '9')) {
                    if (noneZeroDigitsCount + 1 > decimal_util::MaxPrecision<T>::value) {
                        error = ERR_OVERFLOW;
                        state = IN_END;
                        break;
                    }
                    digitsCount++;
                    if (c != '0') {
                        noneZeroDigitsCount++;
                    }
                    rawData = 10 * rawData + static_cast<int>(c - '0');
                    state = IN_BEFORE_DEC;
                } else if (c == dec_point) {
                    state = IN_AFTER_DEC;
                } else if ((c != ' ') && (c != '\t')) {
                    error = ERR_WRONG_CHAR;
                    state = IN_END;
                    prevState = IN_BEFORE_FIRST_DIG;
                }
                break;
            case IN_BEFORE_DEC:
                if ((c >= '0') && (c <= '9')) {
                    if (noneZeroDigitsCount + 1 > decimal_util::MaxPrecision<T>::value) {
                        error = ERR_OVERFLOW;
                        state = IN_END;
                        break;
                    }
                    digitsCount++;
                    if (noneZeroDigitsCount != 0 || c != '0') {
                        noneZeroDigitsCount++;
                    }                    
                    rawData = 10 * rawData + static_cast<int>(c - '0');
                } else if (c == dec_point) {
                    state = IN_AFTER_DEC;
                } else if ((c != ' ') && (c != '\t')) {
                    error = ERR_WRONG_CHAR;
                    state = IN_END;
                    prevState = IN_BEFORE_DEC;
                }
                break;
            case IN_AFTER_DEC:
                if ((c >= '0') && (c <= '9')) {
                    if (afterDigitCount + 1 > scale) {
                        // error = ERR_SCALE_ERROR;
                        state = IN_END;
                        break;
                    }
                    if (noneZeroDigitsCount + 1 > decimal_util::MaxPrecision<T>::value) {
                        error = ERR_OVERFLOW;
                        state = IN_END;
                        break;
                    }
                    digitsCount++;
                    noneZeroDigitsCount++;
                    afterDigitCount++;
                    rawData = 10 * rawData + static_cast<int>(c - '0');
                } else if ((c != ' ') && (c != '\t')) {
                    error = ERR_WRONG_CHAR;
                    state = IN_END;
                    prevState = IN_AFTER_DEC;
                }
                break;
            default:
                error = ERR_WRONG_STATE;
                state = IN_END;
                break;
        }  // switch state
    }
    // std::cout << " noneZeroDigitsCount " << noneZeroDigitsCount << " scale " << scale << " afterDigitCount" << afterDigitCount << std::endl;

    if (error >= 0 || ( error == ERR_WRONG_CHAR && prevState != IN_SIGN)) {
        if (digitsCount == 0) {
            rawData = std::numeric_limits<T>::min();
            return true;
        }
        if (scale > afterDigitCount) {
            if (noneZeroDigitsCount + scale - afterDigitCount > decimal_util::MaxPrecision<T>::value) {
                errMsg = msg[ERR_OVERFLOW];
                return false;
            } else {
                rawData = rawData * decimal_util::scaleMultiplier<T>(scale - afterDigitCount);
            }
        };
        if (sign < 0) {
            rawData = -rawData;
        }
        error = 0;
    } else {
        if (error == ERR_WRONG_CHAR &&  prevState == IN_SIGN) {
            rawData = std::numeric_limits<T>::min();
            return true;
        }
        rawData = 0;
        errMsg = msg[error];
    }

    return (error >= 0);
}

template <typename T>
inline bool parseString(const string &str, T &rawData, int scale, string &errMsg) {
    return parseString(str.c_str(), str.size(), rawData, scale, errMsg);
}

// For example, Decimal32(4) can contain numbers from -99999.9999 to 99999.9999 with 0.0001 step.
template <typename T>
inline void toDecimal(const string &str, int scale, T& rawData) {
    if (scale < 0 || scale > decimal_util::MaxPrecision<T>::value) {
        throw RuntimeException("Scale out of bound (valid range: [0, " +
                std::to_string(decimal_util::MaxPrecision<T>::value) + "], but get: " +
                std::to_string(scale) + ")");
    }

    rawData = 0;
    std::string errMsg;
    if (!parseString(str, rawData, scale, errMsg)) {
        throw RuntimeException("Convert string to DECIMAL failed: " + errMsg);
    }
}

}  // namespace decimal_util


//========================================================================
// Misc
//========================================================================
namespace decimal_util {

inline std::string categoryToString(const DATA_CATEGORY cat) {
#define NORMAL_CASE(tag) case tag: return #tag;

    switch (cat) {
        NORMAL_CASE(LOGICAL);
        NORMAL_CASE(INTEGRAL);
        NORMAL_CASE(FLOATING);
        NORMAL_CASE(TEMPORAL);
        NORMAL_CASE(LITERAL);
        NORMAL_CASE(SYSTEM);
        NORMAL_CASE(MIXED);
        NORMAL_CASE(BINARY);
        NORMAL_CASE(COMPLEX);
        NORMAL_CASE(ARRAY);
        NORMAL_CASE(DENARY);
        default: return "UNKNOWN(" + std::to_string(static_cast<int>(cat)) + ")";
    }

#undef NORMAL_CASE
}

inline void checkArithmeticOperation(const DATA_CATEGORY cat) {
    if (cat != NOTHING && cat != INTEGRAL && cat != FLOATING && cat != DENARY) {
        throw RuntimeException("Not allow to perform arithmetic operation between DECIMAL and " +
                categoryToString(cat));
    }
}

inline void checkComparison(const DATA_CATEGORY cat) {
    if (cat != NOTHING && cat != INTEGRAL && cat != FLOATING && cat != DENARY) {
        throw RuntimeException("Not allow to perform comparison between DECIMAL and " + categoryToString(cat));
    }
}

/**
 * @brief valid str: "DECIMAL32(2)", "decimal64(8)", or "decimal128(10)"
 * 
 * @return std::pair<DATA_TYPE, int> second: scale
 */
inline std::pair<DATA_TYPE, int> parseDecimalType(const std::string &str) {
    std::pair<DATA_TYPE, int> ret{DT_VOID, 0};

    if (false == Util::startWith(Util::lower(str), "decimal")) {
        return ret;
    }

    constexpr size_t kDecLen = sizeof("decimal") - 1;

    if (str.size() < kDecLen + 2) {
        throw RuntimeException("Invalid decimal data type");
    }

    size_t i = kDecLen;

    if (str[i] == '3' && str[i+1] == '2') {
        ret.first = DT_DECIMAL32;
        i += 2;
    } else if (str[i] == '6' && str[i+1] == '4') {
        ret.first = DT_DECIMAL64;
        i += 2;
    } else if (str[i] == '1' && str[i+1] == '2') {
        if (str.size() < kDecLen + 3 || str[i+2] != '8') {
            throw RuntimeException("Invalid decimal data type");
        }
        ret.first = DT_DECIMAL128;
        i += 3;
    } else {
        throw RuntimeException("Invalid decimal data type");
    }

    if (str.size() < i + 3 || str[i] != '(' || str[str.size()-1] != ')') {
        throw RuntimeException("Invalid decimal data type");
    }
    ++i;

    char digits[4] = {0};
    for (size_t j = 0; j < 3 && i < str.size() - 1; ++j) {
        if (false == std::isdigit(str[i])) {
            throw RuntimeException("Invalid scale for decimal data type");
        }
        digits[j] = str[i];
        ++i;
    }

    if (str[i] != ')') {
        throw RuntimeException("Invalid decimal data type");
    }

    ret.second = std::strtol(digits, nullptr, 10);

    return ret;
}

inline int packDecimalTypeAndScale(DATA_TYPE type, int scale) {
    ASSERT(Util::getCategory(type) == DENARY);
    /*
     *     31          16            0
     * +---+-----------+-------------+
     * | 1 |   scale   |  data type  |
     * +---+-----------+-------------+
     */
    return ((scale << 16) | 0x80000000) | (type & 0xffff);
}

inline std::pair<DATA_TYPE, int> unpackDecimalTypeAndScale(const int value) {
    DATA_TYPE type = static_cast<DATA_TYPE>(value);
    int scale = 0;
    if (value & 0x80000000) {
        /*
         *     31          16            0
         * +---+-----------+-------------+
         * | 1 |   scale   |  data type  |
         * +---+-----------+-------------+
         */
        type = static_cast<DATA_TYPE>(value & 0xffff);
        scale = (value & (~0x80000000)) >> 16;
        if (Util::getCategory(type) != DENARY) {
            type = DT_VOID;
        }
    }
    return {type, scale};
}

/**
 *  32                24      0
 *  +-----------------+-------+
 *  | compress method | scale |
 *  +-----------------+-------+
 */
inline int getScaleFromExtraParam(int extra) {
    return extra & 0x00ffffff;
}

} // namespace decimal_util
