#ifndef _Q2DDB_H_
#define _Q2DDB_H_

#include <type_traits>
#include <string>
#include <vector>

#include "CoreConcept.h"

#include "k.h"

namespace kdb {

    enum Type: H {
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

    enum Attribute: G {
        K_ATTR_NONE    = 0u,
        K_ATTR_SORTED  = 1u,
        K_ATTR_UNIQUE  = 2u,
        K_ATTR_PARTED  = 3u,
        K_ATTR_GROUPED = 4u,
    };

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
        static constexpr auto UNKNOWN_COUNT = static_cast<std::size_t>(-1);

        // kdb+ new data format - data segment offset
        static constexpr std::ptrdiff_t NEW_DATA_OFFSET = 0x1000;

#       pragma pack(push, 1)

        struct Header {
            union {
                char magic[8];
                struct {
                    byte format[2];
                    byte type;  //--> kdb::Type
                    Attribute attr;
                    int pad_or_count;  // padding/map-level? or count
                };
            };
        };
        static_assert(sizeof(Header) == 8,
            "kdb+ binary file header");

        struct HeaderEx : Header {
            std::size_t count;
        };
        static_assert(sizeof(HeaderEx) == 8+8,
            "kdb+ binary file extended header");

        struct Trailer {
            int pad_03;
            int pad_02;
            std::ptrdiff_t last_addr;
            std::ptrdiff_t trailer_addr;

            bool valid(std::ptrdiff_t tail, std::size_t itemSize) const;
        };
        static_assert(sizeof(Trailer) == 4*2+8*2,
            "kdb+ binary file padding trailer");

        struct ItemIndex {
            int offset;
            byte pad_xxx[3];
            byte format;
        };
        static_assert(sizeof(ItemIndex) == 4+3+1,
            "kdb+ binary file nested item index");

        struct ItemHeader {
            byte type;
            byte count;

            static constexpr byte TYPE_OFFSET = 0x7F;
            static constexpr byte MAX_COUNT   = 0x5F;
        };
        static_assert(sizeof(ItemHeader) == 1+1,
            "kdb+ binary file# nested item header");

        using ItemHeaderEx = HeaderEx;
        static_assert(sizeof(ItemHeaderEx) == 16,
            "kdb+ binary file# nested item extended header");

        struct SymItemHeader {
            byte type;
            byte pad_xxx[4];
            byte count;

            static constexpr byte TYPE_OFFSET = ItemHeader::TYPE_OFFSET;
            static constexpr byte MAX_COUNT   = ItemHeader::MAX_COUNT;
        };
        static_assert(sizeof(SymItemHeader) == 1+4+1,
            "kdb+ binary file# nested symbol header");

        using SymItemHeaderEx = ItemHeaderEx;
        static_assert(sizeof(ItemHeaderEx) == 16,
            "kdb+ binary file# nested symbol extended header");

#       pragma pack(pop)

    public:
        std::vector<byte>& getBuffer() noexcept;

        const Header* getHeader() const;
        const HeaderEx* getHeaderEx() const;

        template<typename T>
        const T* get(std::ptrdiff_t offset = 0, bool end = false) const {
            return const_cast<Parser*>(this)->get<T>(offset, end);
        }
        template<typename T>
        T* get(std::ptrdiff_t offset = 0, bool end = false) {
            return static_cast<T*>(get(offset, sizeof(T), end));
        }

        void initialize(bool force = false);

        std::vector<std::string> strings(const std::string& file);

        VectorSP strings(const std::string& file,
            const std::vector<std::string>& symList,
            const std::string& symName);

        VectorSP vector(const std::string& file);

        VectorSP nestedList(Parser& mapParser,
            const std::vector<std::string>& symList, const std::string& symName,
            const std::string& file,const std::string& mapFile);

    private:
        using k_type = std::remove_pointer<K>::type;

        struct State {
            Type type;
            Attribute attr;
            std::size_t count;
            std::ptrdiff_t header, data, end, enumName;

            State();
        };

    private:
        void* get(std::ptrdiff_t offset, std::size_t size, bool end = false);

        bool initialized() const;

        template<typename HeaderT, typename CountT>
        void initialize(std::ptrdiff_t header, CountT(HeaderT::*count));

        static std::size_t itemSize(Type type);

        std::size_t guessItemCount(std::size_t itemSize) const;
        void verifyItemCount(std::size_t itemSize) const;

        VectorSP mapNestedList(Parser& mapParser,
            const std::vector<std::string>& symList, const std::string& symName,
            const std::string& file, const std::string& mapFile);

        ConstantSP mapNestedItem(const ItemIndex* idx, Parser& mapParser,
            const std::vector<std::string>& symList, const std::string& symName,
            const std::string& file, const std::string& mapFile);

        template<byte ItemFormat>
        ConstantSP mapNestedItem(std::ptrdiff_t offset, Parser& mapParser,
            const std::vector<std::string>& symList, const std::string& symName,
            const std::string& mapFile);

        template<byte ItemFormat>
        VectorSP mapNestedStrings(std::ptrdiff_t offset, Parser& mapParser,
            const std::vector<std::string>& symList, const std::string& symName,
            const std::string& mapFile);

    private:
        std::vector<byte> buffer_;
        State state_;

    };//class Parser

}//namespace kdb

#endif//_Q2DDB_H_