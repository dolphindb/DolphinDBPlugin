#ifndef _Q2DDB_H_
#define _Q2DDB_H_

#include <type_traits>
#include <string>
#include <vector>

#include "CoreConcept.h"

#include "k.h"

namespace kdb {

    enum Type: std::int8_t {
        K_LIST      =    0,
        K_BOOL      = (KB),
        K_GUID      = (UU),
        K_BYTE      = (KG),
        K_SHORT     = (KH),
        K_INT       = (KI),
        K_LONG      = (KJ),
        K_FLOAT     = (KE),
        K_DOUBLE    = (KF),
        K_CHAR      = (KC),
        K_STRING    = (KS),
        K_TIMESTAMP = (KP),
        K_MONTH     = (KM),
        K_DATE      = (KD),
        K_DATETIME  = (KZ),
        K_TIMESPAN  = (KN),
        K_MINUTE    = (KU),
        K_SECOND    = (KV),
        K_TIME      = (KT),
        K_ENUM_MIN   =  20, K_ENUM_MAX   =  76,
        K_NESTED_MIN =  77, K_NESTED_MAX =  97,
        K_TABLE     = (XT),
        K_DICT      = (XD),
        K_FUNC_MIN   = 100, K_FUNC_MAX   = 112,
        K_ERROR     = -128,
    };

    enum Attribute: std::uint8_t {
        K_ATTR_NONE    = 0u,
        K_ATTR_SORTED  = 1u,
        K_ATTR_UNIQUE  = 2u,
        K_ATTR_PARTED  = 3u,
        K_ATTR_GROUPED = 4u,
    };

    constexpr auto UNKNOWN_SIZE = static_cast<std::size_t>(-1);
    std::size_t getSize(Type type) noexcept;

    bool isNull(const H& h) noexcept;
    bool isNull(const I& i) noexcept;
    bool isNull(const J& j) noexcept;
    bool isNull(const E& e) noexcept;
    bool isNull(const F& f) noexcept;

    bool isInf(const H& h) noexcept;
    bool isInf(const I& i) noexcept;
    bool isInf(const J& j) noexcept;
    bool isInf(const E& e) noexcept;
    bool isInf(const F& f) noexcept;

    bool isValidList(const K k) noexcept;
    bool isValidListOf(const K k, Type type) noexcept;

    S sym(const char* str) noexcept;
    S sym(const std::string& str) noexcept;

    void fakeEmptyAnyColumn(Vector* colVal,
        const std::string& tableName, const std::string& colName,
        DATA_TYPE dummyTYpe = DT_INT);

    struct toDDB {
        static ConstantSP fromK(K pk, const std::string& var);

        static VectorSP fromArray(Type type, byte* begin, std::size_t count,
            const std::string& var);

        // Generic kdb+ list
        static VectorSP list(K* begin, K* end, const std::string& var);
        // kdb+ bool (b) list
        static VectorSP bools(G* begin, G* end, const std::string& var);
        // kdb+ GUID (g) list
        static VectorSP GUIDs(U* begin, U* end, const std::string& var);
        // kdb+ byte (x) list
        static VectorSP bytes(G* begin, G* end, const std::string& var);
        // kdb+ short (h) list
        static VectorSP shorts(H* begin, H* end, const std::string& var);
        // kdb+ int (i) list
        static VectorSP ints(I* begin, I* end, const std::string& var);
        // kdb+ long (j) list
        static VectorSP longs(J* begin, J* end, const std::string& var);
        // kdb+ real (e) list
        static VectorSP floats(E* begin, E* end, const std::string& var);
        // kdb+ float (f) list
        static VectorSP doubles(F* begin, F* end, const std::string& var);
        // kdb+ char (c) list
        static VectorSP chars(G* begin, G* end, const std::string& var);
        // kdb+ string (C) list
        static VectorSP charsList(K* begin, K* end, const std::string& var);
        // kdb+ symbol (s) list
        static VectorSP strings(S* begin, S* end, const std::string& var);
        // kdb+ timestamp (p) list
        static VectorSP timestamps(J* begin, J* end, const std::string& var);
        // kdb+ month (m) list
        static VectorSP months(I* begin, I* end, const std::string& var);
        // kdb+ date (d) list
        static VectorSP dates(I* begin, I* end, const std::string& var);
        // kdb+ datetime (z) list
        static VectorSP datetimes(F* begin, F* end, const std::string& var);
        // kdb+ timespan (n) list
        static VectorSP timespans(J* begin, J* end, const std::string& var);
        // kdb+ minute (u) list
        static VectorSP minutes(I* begin, I* end, const std::string& var);
        // kdb+ second (v) list
        static VectorSP seconds(I* begin, I* end, const std::string& var);
        // kdb+ time (t) list
        static VectorSP times(I* begin, I* end, const std::string& var);

        // kdb+ enumerated symbol (s) list
        static VectorSP mapStrings(J* begin, J* end,
            const std::vector<std::string>& symList, const std::string& symName,
            const std::string& var);

        // kdb+ nested (B|G|X|H|I|J|E|F|CC|S|P|M|D|Z|N|U|V|T) list
        //  FIXME: Only support single level of nesting for now...
        static VectorSP nestedList(Type type,
            K* begin, K* end, const std::string& var);

    private:
        // kdb+ vector of a primitive numreic type
        template<typename kValueT, DATA_TYPE ddbType, typename ddbPtr>
        static VectorSP makeVector(
            ddbPtr begin, ddbPtr end,
            bool(Vector::*append)(ddbPtr, int),
            const std::string& var);

        template<typename kValueT, DATA_TYPE ddbType, typename ddbPtr>
        static VectorSP buildVector(
            std::function<void(ddbPtr&, ddbPtr&)>&& build,
            bool(Vector::*append)(ddbPtr, int),
            const std::string& var);

    };//struct toDDB

    //@see https://code.kx.com/q/kb/serialization/
    class Parser {
    public:
        template<byte Tag0, byte Tag1, typename HeaderT>
        struct HeaderTag;
        template<typename HeaderT,
            typename DataT = byte, std::size_t MinCount = 0>
        struct DataBlock;

        // 0xFF01
        struct BaseHeader;
        struct SymsList;

        // 0xFE20
        struct BaseListHeader;
        struct BaseList;

        // 0xFD20
        struct ExtListHeader;
        struct ExtList;

        // 0xFD0000~~0xFD0013
        struct SimpleListHeader;
        struct SimpleList;

        // 0xFD0014~~0xFD004C
        struct EnumSymsHeader;
        struct EnumSymsList;

        // 0xFD014D00000000~~0xFD016100000000
        // 0xFB004Dnnnnnnnn~~0xFB0061nnnnnnnn
        template<byte Tag0, byte Tag1, std::int32_t MinRef = 0>
        struct NestedListHeader;
        template<byte Tag0, byte Tag1, std::int32_t MinRef = 0>
        struct NestedList;

        // 0xHHHHHHHHnnnnnn00
        // 0xHHHHHHHHnnnnnn01
        // 0xHHHHHHHHHHHHHHHH
        struct ItemIndex;

        // 0xFD0004
        struct NestedMapHeader;
        struct NestedMap;

        // 0x7F+type
        struct NestedItemHeader;
        struct NestedItem;

        // 0xFB0000~~0xFB0013
        struct NestedItemExHeader;
        struct NestedItemEx;

        // 0x93~~0xCB
        struct NestedEnumSymsHeader;
        struct NestedEnumSyms;

        // 0xFB0014~~0xFB004C
        struct NestedEnumSymsExHeader;
        struct NestedEnumSymsEx;

        // 0x030000000200000000+addr0+addr1
        struct Trailer;

    public:
        Parser();

        std::vector<byte>& getBuffer() noexcept;

        std::vector<std::string> getStrings(const std::string& file) const;

        VectorSP getVector(const std::string& file,
            const std::vector<std::string>& symList,
            const std::string& symName) const;

    protected:
        const byte* begin() const noexcept;
        const byte* end() const noexcept;

        // Allow for "end" iterator semantics
        template<typename T>
        const T* parse(std::ptrdiff_t offset = 0,
            bool allowEnd = false) const;
        const void* parse(std::size_t offset, std::ptrdiff_t index,
            bool allowEnd = false) const;

    private:
        const byte* findEnd( const byte* start = nullptr) const noexcept;

        VectorSP getFastVector(const BaseList* data,
            const std::string& file,
            const std::vector<std::string>& symList,
            const std::string& symName) const;

        VectorSP getGeneralList(const ExtList* data,
            const std::string& file,
            const std::vector<std::string>& symList,
            const std::string& symName) const;

        VectorSP getEnumStrings(const EnumSymsList* data,
            const std::string& file,
            const std::vector<std::string>& symList,
            const std::string& symName) const;

        VectorSP getNestedLists(const NestedList<0xFD, 0x01>* data,
            const std::string& file,
            const std::vector<std::string>& symList,
            const std::string& symName) const;

        VectorSP mapNestedLists(const ItemIndex* begin, const ItemIndex* end,
            const NestedMap* mapData, const byte* mapEnd,
            const std::string& mapFile,
            const std::vector<std::string>& symList,
            const std::string& symName) const;

        static VectorSP mapEnumSyms(const VectorSP& indices,
            const std::string& mapFile,
            const std::vector<std::string>& symList,
            const std::string& symName);

    private:
        std::vector<byte> buffer_;
        mutable const byte* end_;

    };//class Parser

    //////////////////////////////////////////////////////////////////////////

#   pragma pack(push, 1)

    template<byte Tag0, byte Tag1, typename HeaderT>
    struct Parser::HeaderTag {
    protected:
        constexpr bool isValid() const noexcept {
            return static_cast<const HeaderT*>(this)->tag[0] == Tag0
                && static_cast<const HeaderT*>(this)->tag[1] == Tag1;
        }
    };

    template<typename HeaderT, typename DataT, std::size_t MinCount>
    struct Parser::DataBlock {
        using header_t = HeaderT;
        using data_t   = DataT;

        struct {
            header_t header;
            data_t   data[MinCount];
        };

        constexpr DataT* get(std::ptrdiff_t index = 0) const noexcept {
            return const_cast<DataT*>(data + index);
        }

        template<typename T>
        constexpr T* getAt(std::ptrdiff_t offset = 0) const noexcept {
            return const_cast<T*>(reinterpret_cast<const T*>(data + offset));
        }

    protected:
        constexpr bool isComplete(std::size_t count, const byte* end
        ) const noexcept {
            return isComplete(sizeof(data_t), count, end);
        }

        constexpr bool isComplete(
            std::size_t itemSize, std::size_t count, const byte* end
        ) const noexcept {
            return reinterpret_cast<const byte*>(data) + itemSize * count
                    <= end;
        }
    };

#   pragma pack(pop)

}//namespace kdb

#endif//_Q2DDB_H_