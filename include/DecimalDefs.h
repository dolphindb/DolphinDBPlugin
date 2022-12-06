#pragma once

#include <cassert>


namespace decimal_util {

enum class CompareType {
    eq, ne, lt, gt, le, ge, between
};

inline CompareType reverseCompareType(CompareType type) {
    switch (type) {
        case CompareType::eq: return CompareType::eq;  // a == b => b == a
        case CompareType::ne: return CompareType::ne;  // a != b => b != a
        case CompareType::lt: return CompareType::gt;  // a < b => b > a
        case CompareType::gt: return CompareType::lt;  // a > b => b < a
        case CompareType::le: return CompareType::ge;  // a <= b => b >= a
        case CompareType::ge: return CompareType::le;  // a >= b => b <= a
        default: assert("unreachable" && 0);
    }
}

} // namespace decimal_util
