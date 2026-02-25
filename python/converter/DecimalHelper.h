#ifndef DECIMAL_HELPER_H_
#define DECIMAL_HELPER_H_


#include "TypeDefine.h"
#include "Types.h"
#include "Util.h"


namespace _decimal_util {

inline bool isDecimalType(DATA_TYPE type) {
    if ((int)type >= ARRAY_TYPE_BASE) {
        type = static_cast<DATA_TYPE>(type - ARRAY_TYPE_BASE);
    }
    return (Util::getCategory(type) == DENARY);
}

inline int packDecimalTypeAndScale(DATA_TYPE type, int scale) {
    if (false == isDecimalType(type)) {
        return static_cast<int>(type);
    }
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
        scale = (value & (~0x80000000)) >> 16;
        type = static_cast<DATA_TYPE>(value & 0xffff);
        if ((int)type >= ARRAY_TYPE_BASE) {
            if (Util::getCategory(static_cast<DATA_TYPE>(static_cast<int>(type) - ARRAY_TYPE_BASE)) != DENARY) {
                type = DT_VOID;
            }
        } else {
            if (Util::getCategory(type) != DENARY) {
                type = DT_VOID;
            }
        }
    }
    return {type, scale};
}


inline int128 exp10_i128(int x) {
    using int128 = int128;
    constexpr int128 values[] =
    {
        static_cast<int128>(1LL),
        static_cast<int128>(10LL),
        static_cast<int128>(100LL),
        static_cast<int128>(1000LL),
        static_cast<int128>(10000LL),
        static_cast<int128>(100000LL),
        static_cast<int128>(1000000LL),
        static_cast<int128>(10000000LL),
        static_cast<int128>(100000000LL),
        static_cast<int128>(1000000000LL),
        static_cast<int128>(10000000000LL),
        static_cast<int128>(100000000000LL),
        static_cast<int128>(1000000000000LL),
        static_cast<int128>(10000000000000LL),
        static_cast<int128>(100000000000000LL),
        static_cast<int128>(1000000000000000LL),
        static_cast<int128>(10000000000000000LL),
        static_cast<int128>(100000000000000000LL),
        static_cast<int128>(1000000000000000000LL),
        static_cast<int128>(1000000000000000000LL) * 10LL,
        static_cast<int128>(1000000000000000000LL) * 100LL,
        static_cast<int128>(1000000000000000000LL) * 1000LL,
        static_cast<int128>(1000000000000000000LL) * 10000LL,
        static_cast<int128>(1000000000000000000LL) * 100000LL,
        static_cast<int128>(1000000000000000000LL) * 1000000LL,
        static_cast<int128>(1000000000000000000LL) * 10000000LL,
        static_cast<int128>(1000000000000000000LL) * 100000000LL,
        static_cast<int128>(1000000000000000000LL) * 1000000000LL,
        static_cast<int128>(1000000000000000000LL) * 10000000000LL,
        static_cast<int128>(1000000000000000000LL) * 100000000000LL,
        static_cast<int128>(1000000000000000000LL) * 1000000000000LL,
        static_cast<int128>(1000000000000000000LL) * 10000000000000LL,
        static_cast<int128>(1000000000000000000LL) * 100000000000000LL,
        static_cast<int128>(1000000000000000000LL) * 1000000000000000LL,
        static_cast<int128>(1000000000000000000LL) * 10000000000000000LL,
        static_cast<int128>(1000000000000000000LL) * 100000000000000000LL,
        static_cast<int128>(1000000000000000000LL) * 100000000000000000LL * 10LL,
        static_cast<int128>(1000000000000000000LL) * 100000000000000000LL * 100LL,
        static_cast<int128>(1000000000000000000LL) * 100000000000000000LL * 1000LL
    };
    assert(x >= 0 && static_cast<size_t>(x) < sizeof(values)/sizeof(values[0]));
    return values[x];
}


} // namespace _decimal_util


#endif  // DECIMAL_HELPER_H_
