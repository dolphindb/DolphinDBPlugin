#include "q2ddb.h"

#include <cassert>
#include <tuple>
#include <sstream>
#include <iomanip>

#include "Util.h"
#include "ScalarImp.h"
#include "SpecialConstant.h"
#include "Logger.h"

#include "endian.h"
#include "kdb.h"
#include "qfile.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////////

// kdb+ FP nulls -- cannot compare FPs directly, as NaN==NaN is always false!
const unsigned long long KDB_DOUBLE_NULL = 0xfff8000000000000ULL;//q) 0x0 vs 0n
const unsigned int       KDB_FLOAT_NULL  = 0xffc00000U;          //q) 0x0 vs 0Ne

// kdb+ time starts at 2000.01.01, while DolphinDB start at 1970.01.01
const I SEC_PER_DAY   = 24 * 60 * 60;
const I KDB_MONTH_GAP = 24000;  //q) neg["i"$0001.01m] + 12i
const I KDB_DATE_GAP  = 10957;  //q) neg["i"$1970.01.01]
const J KDB_DATETIME_GAP      = KDB_DATE_GAP * SEC_PER_DAY * 1000LL;
const J KDB_NANOTIMESTAMP_GAP = KDB_DATETIME_GAP * 1000000LL;

// DolphinDB's numeric values has different special values than kdb+
const short     DDB_SHORT_NULL  =  numeric_limits<short>::min();
const int       DDB_INT_NULL    =  numeric_limits<int>::min();
const long long DDB_LONG_NULL   =  numeric_limits<long long>::min();
const float     DDB_FLOAT_NULL  =  (FLT_NMIN);
const float     DDB_FLOAT_INF   =  numeric_limits<float>::infinity();
const float     DDB_FLOAT_NINF  = -numeric_limits<float>::infinity();
const double    DDB_DOUBLE_NULL =  (DBL_NMIN);
const double    DDB_DOUBLE_INF  =  numeric_limits<double>::infinity();
const double    DDB_DOUBLE_NINF = -numeric_limits<double>::infinity();

//////////////////////////////////////////////////////////////////////////////

size_t kdb::getSize(Type type) noexcept {
    switch(type) {
        case K_BOOL:
        case K_BYTE:
        case K_CHAR:
            return sizeof(G);
        case K_GUID:
            return sizeof(U);
        case K_SHORT:
            return sizeof(H);
        case K_INT:
        case K_MONTH:
        case K_DATE:
        case K_MINUTE:
        case K_SECOND:
        case K_TIME:
            return sizeof(I);
        case K_LONG:
        case K_TIMESTAMP:
        case K_TIMESPAN:
            return sizeof(J);
        case K_FLOAT:
            return sizeof(E);
        case K_DOUBLE:
        case K_DATETIME:
            return sizeof(F);
        default:
            if(K_ENUM_MIN <= type && type <= K_ENUM_MAX) {
                return sizeof(J);
            } else
            if(K_NESTED_MIN <= type && type <= K_NESTED_MAX) {
                return sizeof(J);
            } else {
                return UNKNOWN_SIZE;
            }
    }
}

bool kdb::isNull(const H& h) noexcept { return h == (nh); }
bool kdb::isNull(const I& i) noexcept { return i == (ni); }
bool kdb::isNull(const J& j) noexcept { return j == (nj); }
bool kdb::isNull(const E& e) noexcept { return bit_equal(e, KDB_FLOAT_NULL); }
bool kdb::isNull(const F& f) noexcept { return bit_equal(f, KDB_DOUBLE_NULL); }

bool kdb::isInf(const H& h) noexcept { return abs(h) == (wh); }
bool kdb::isInf(const I& i) noexcept { return abs(i) == (wi); }
bool kdb::isInf(const J& j) noexcept { return abs(j) == (wj); }
bool kdb::isInf(const E& e) noexcept { return abs(e) == static_cast<float>((wf)); }
bool kdb::isInf(const F& f) noexcept { return abs(f) == (wf); }

bool kdb::isValidList(const K k) noexcept {
    return k && (k->n >= 0) && (K_LIST <= k->t) && (k->t <= K_NESTED_MAX);
}
bool kdb::isValidListOf(const K k, Type type) noexcept {
    return k && (k->n >= 0) && (k->t == type);
}

S kdb::sym(const char* str) noexcept {
    static const char NULL_SYM[] = { '\0' };
    return const_cast<S>((str && *str != '\0') ? str : NULL_SYM);
}
S kdb::sym(const string& str) noexcept {
    return kdb::sym(str.c_str());
}

//FIXME: DolphinDB does not allow [].setColumnarTuple!()
//  However, we can try to fake it here...
void kdb::fakeEmptyAnyColumn(Vector* colVal,
    const string& tableName, const string& colName, DATA_TYPE dummyType
) {
    assert(colVal);
    LOG(PLUGIN_NAME ": "
            "DolphinDB does not support empty ANY VECTOR in "
        + tableName + "." + colName + " as a table column! "
            "We'll try to fake one instead...");
    const auto any = dynamic_cast<AnyVector*>(colVal);
    assert(any);
    any->setTableColumn(true);
    // Just a dummy type, will accept any type as of v2.00.10.
    any->setExtraParamForType(dummyType);
}

//////////////////////////////////////////////////////////////////////////////

ConstantSP kdb::toDDB::fromK(K pk, const string& var) {
    assert(pk);
    const auto type  = static_cast<Type>(pk->t);
    switch(type) {
        case K_LIST:
            return list(kK(pk), kK(pk) + pk->n, var);
        default:
            return fromArray(type, kG(pk), pk->n, var);
    }
}

VectorSP kdb::toDDB::fromArray(
    Type type, byte* data, size_t count, const string& var
) {
    if(UNLIKELY(count == UNKNOWN_SIZE)) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Unknown kdb+ list length for " + var + ".");
    }

    void* const p = data;
    switch(type) {
        case K_BOOL: return bools(
                static_cast<G*>(p), static_cast<G*>(p) + count, var);
        case K_GUID: return GUIDs(
                static_cast<U*>(p), static_cast<U*>(p) + count, var);
        case K_BYTE: return bytes(
                static_cast<G*>(p), static_cast<G*>(p) + count, var);
        case K_SHORT: return shorts(
                static_cast<H*>(p), static_cast<H*>(p) + count, var);
        case K_INT: return ints(
                static_cast<I*>(p), static_cast<I*>(p) + count, var);
        case K_LONG: return longs(
                static_cast<J*>(p), static_cast<J*>(p) + count, var);
        case K_FLOAT: return floats(
                static_cast<E*>(p), static_cast<E*>(p) + count, var);
        case K_DOUBLE: return doubles(
                static_cast<F*>(p), static_cast<F*>(p) + count, var);
        case K_CHAR: return chars(
                static_cast<G*>(p), static_cast<G*>(p) + count, var);
        case K_STRING: return strings(
                static_cast<S*>(p), static_cast<S*>(p) + count, var);
        case K_TIMESTAMP: return timestamps(
                static_cast<J*>(p), static_cast<J*>(p) + count, var);
        case K_MONTH: return months(
                static_cast<I*>(p), static_cast<I*>(p) + count, var);
        case K_DATE: return dates(
                static_cast<I*>(p), static_cast<I*>(p) + count, var);
        case K_DATETIME: return datetimes(
                static_cast<F*>(p), static_cast<F*>(p) + count, var);
        case K_TIMESPAN: return timespans(
                static_cast<J*>(p), static_cast<J*>(p) + count, var);
        case K_MINUTE: return minutes(
                static_cast<I*>(p), static_cast<I*>(p) + count, var);
        case K_SECOND: return seconds(
                static_cast<I*>(p), static_cast<I*>(p) + count, var);
        case K_TIME: return times(
                static_cast<I*>(p), static_cast<I*>(p) + count, var);
        default:
            throw RuntimeException(PLUGIN_NAME ": "
                "kdb+ object " + var + " (" + to_string(type) + ") "
                "not yet supported.");
    }
}

VectorSP kdb::toDDB::list(K* begin, K* end, const string& var) {
    if(UNLIKELY(!(begin && begin <= end))) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Invalid kdb+ object for " + var + ".");
    }

    const auto count = end - begin;
    if(count == 0) {
        return Util::createVector(DT_ANY, 0);
    }

    K* item = begin;
    if(UNLIKELY(!*item)) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Invalid kdb+ list item at first(" + var + ").");
    } else
    if((*item)->t == K_CHAR) {
        return charsList(begin, end, var);
    } else {
        return nestedList(static_cast<Type>((*item)->t), begin, end, var);
    }
}

VectorSP kdb::toDDB::bools(G* begin, G* end, const string& var) {
    using ddbPtr = const char*;
    const auto from = reinterpret_cast<ddbPtr>(begin),
               to   = reinterpret_cast<ddbPtr>(end);
    return makeVector<G, DT_BOOL>(
        from, to, &Vector::appendBool, var);
}

VectorSP kdb::toDDB::GUIDs(U* begin, U* end, const string& var) {
    if(UNLIKELY(!(begin && begin <= end))) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Invalid kdb+ GUID vector for " + var + ".");
    }

    using ddbPtr = const Guid*;
    return buildVector<U, DT_UUID, ddbPtr>(
        [begin, end](ddbPtr& from, ddbPtr& to) {
            transform(begin, end, begin, &kdb::byteswap<U>);
            from = reinterpret_cast<ddbPtr>(begin),
            to = reinterpret_cast<ddbPtr>(end);
        },
        &Vector::appendGuid, var);
}

VectorSP kdb::toDDB::bytes(G* begin, G* end, const string& var) {
    // DolphinDB doesn't have type of byte, so convert byte to char instead.
    using ddbPtr = const char*;
    const auto from = reinterpret_cast<ddbPtr>(begin),
               to   = reinterpret_cast<ddbPtr>(end);
    return makeVector<G, DT_CHAR>(
        from, to, &Vector::appendChar, var);
}

VectorSP kdb::toDDB::shorts(H* begin, H* end, const string& var) {
    // DolphinDB doesn't have short +/-inf values, treat them as normal shorts.
    assert(isNull(DDB_SHORT_NULL)
        &&"kdb+ vs DolphinDB short null equivalence");

    using ddbPtr = const short*;
    return makeVector<H, DT_SHORT, ddbPtr>(
        begin, end, &Vector::appendShort, var);
}

VectorSP kdb::toDDB::ints(I* begin, I* end, const string& var) {
    // DolphinDB doesn't have int +/-inf values, treat them as normal ints.
    assert(isNull(DDB_INT_NULL)
        &&"kdb+ vs DolphinDB int null equivalence");

    using ddbPtr = const int*;
    return makeVector<I, DT_INT, ddbPtr>(
        begin, end, &Vector::appendInt, var);
}

VectorSP kdb::toDDB::longs(J* begin, J* end, const string& var) {
    // DolphinDB doesn't have long +/-inf valuen, treat them as normal longs.
    assert(isNull(DDB_LONG_NULL)
        &&"kdb+ vs DolphinDB long null equivalence");

    using ddbPtr = const long long*;
    return makeVector<J, DT_LONG, ddbPtr>(
        begin, end, &Vector::appendLong, var);
}

VectorSP kdb::toDDB::floats(E* begin, E* end, const string& var) {
    if(UNLIKELY(!(begin && begin <= end))) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Invalid kdb+ real vector for " + var + ".");
    }
    assert(static_cast<float>( (wf)) == DDB_FLOAT_INF
        && static_cast<float>(-(wf)) == DDB_FLOAT_NINF
        &&"kdb+ vs DolphinDB float +/-inf equivalence");

    using ddbPtr = const float*;
    return buildVector<E, DT_FLOAT, ddbPtr>(
        [begin, end](ddbPtr& from, ddbPtr& to) {
            transform(begin, end, begin, [](E e){
                return isNull(e) ? DDB_FLOAT_NULL : e;
            });
            from = begin;
            to = end;
        },
        &Vector::appendFloat, var);
}

VectorSP kdb::toDDB::doubles(F* begin, F* end, const string& var) {
    if(UNLIKELY(!(begin && begin <= end))) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Invalid kdb+ float vector for " + var + ".");
    }
    assert((wf) == DDB_DOUBLE_INF && -(wf) == DDB_DOUBLE_NINF
        &&"kdb+ vs DolphinDB double +/-inf equivalence");

    using ddbPtr = const double*;
    return buildVector<F, DT_DOUBLE, ddbPtr>(
        [begin, end](ddbPtr& from, ddbPtr& to) {
            transform(begin, end, begin, [](F f){
                return isNull(f) ? DDB_DOUBLE_NULL : f;
            });
            from = begin;
            to = end;
        },
        &Vector::appendDouble, var);
}

VectorSP kdb::toDDB::chars(G* begin, G* end, const string& var) {
    // kdb+ treats " " as char null -- weird comparing to -128c in DolphinDB
    using ddbPtr = const char*;
    const auto from = reinterpret_cast<ddbPtr>(begin),
               to   = reinterpret_cast<ddbPtr>(end);
    return makeVector<G, DT_CHAR>(from, to, &Vector::appendChar, var);
}

VectorSP kdb::toDDB::strings(S* begin, S* end, const string& var) {
    using ddbPtr = const char**;
    const auto from = const_cast<ddbPtr>(begin),
               to   = const_cast<ddbPtr>(end);
    return makeVector<S*, DT_SYMBOL>(from, to, &Vector::appendString, var);
}

VectorSP kdb::toDDB::timestamps(J* begin, J* end, const string& var) {
    if(UNLIKELY(!(begin && begin <= end))) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Invalid kdb+ timestamp vector for " + var + ".");
    }

    // DolphinDB doesn't have +/-inf timestamps, treat them as nulls.
    using ddbPtr = const long long*;
    return buildVector<J, DT_NANOTIMESTAMP, ddbPtr>(
        [begin, end](ddbPtr& from, ddbPtr& to) {
            transform(begin, end, begin, [](J p){
                return isNull(p) || isInf(p)
                    ? DDB_LONG_NULL : p + KDB_NANOTIMESTAMP_GAP;
            });
            from = begin;
            to = end;
        },
        &Vector::appendLong, var);
}

VectorSP kdb::toDDB::months(I* begin, I* end, const string& var) {
    if(UNLIKELY(!(begin && begin <= end))) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Invalid kdb+ month vector for " + var + ".");
    }

    // DolphinDB doesn't have +/-inf months, treat them as nulls.
    using ddbPtr = const int*;
    return buildVector<I, DT_MONTH, ddbPtr>(
        [begin, end](ddbPtr& from, ddbPtr& to) {
            transform(begin, end, begin, [](I m){
                return isNull(m) || isInf(m)
                    ? DDB_INT_NULL : m + KDB_MONTH_GAP;
            });
            from = begin;
            to = end;
        },
        &Vector::appendInt, var);
}

VectorSP kdb::toDDB::dates(I* begin, I* end, const string& var) {
    if(UNLIKELY(!(begin && begin <= end))) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Invalid kdb+ date vector for " + var + ".");
    }

    // DolphinDB doesn't have +/-inf dates, treat them as nulls.
    using ddbPtr = const int*;
    return buildVector<I, DT_DATE, ddbPtr>(
        [begin, end](ddbPtr& from, ddbPtr& to) {
            transform(begin, end, begin, [](I& d){
                return isNull(d) || isInf(d)
                    ? DDB_INT_NULL : d + KDB_DATE_GAP;
            });
            from = begin;
            to = end;
        },
        &Vector::appendInt, var);
}

VectorSP kdb::toDDB::datetimes(F* begin, F* end, const string& var) {
    if(UNLIKELY(!(begin && begin <= end))) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Invalid kdb+ datetime vector for " + var + ".");
    }
    const auto count = end - begin;

    // DolphinDB doesn't have +/-inf datetimes, treat them as nulls.
    using kValueT2 = long long;
    /*
    //FIXME: make_unique<>() is available only after C++14...
    auto timestamps = make_unique<KValueT2[]>(count);
    /*/
    unique_ptr<kValueT2[]> timestamps{ new kValueT2[count] };
    //*/
    transform(begin, end, timestamps.get(), [](const F& z){
        return isNull(z) || isInf(z)
            ? DDB_LONG_NULL : static_cast<kValueT2>(
                        z * (SEC_PER_DAY * 1000LL) + KDB_DATETIME_GAP);
    });

    using ddbPtr = const long long*;
    return makeVector<kValueT2, DT_TIMESTAMP, ddbPtr>(
        timestamps.get(), timestamps.get() + count,
        &Vector::appendLong, var);
}

VectorSP kdb::toDDB::timespans(J* begin, J* end, const string& var) {
    if(UNLIKELY(!(begin && begin <= end))) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Invalid kdb+ timespans for " + var + ".");
    }

    // DolphinDB doesn't have +/-inf timespans, treat them as nulls.
    using ddbPtr = const long long*;
    return buildVector<J, DT_NANOTIME, ddbPtr>(
        [begin, end](ddbPtr& from, ddbPtr& to) {
            transform(begin, end, begin, [](J& n){
                return isNull(n) || isInf(n) ? DDB_LONG_NULL : n;
            });
            from = begin;
            to = end;
        },
        &Vector::appendLong, var);
}

VectorSP kdb::toDDB::minutes(I* begin, I* end, const string& var) {
    if(UNLIKELY(!(begin && begin <= end))) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Invalid kdb+ minutes for " + var + ".");
    }

    // DolphinDB doesn't have +/-inf minutes, treat them as nulls.
    using ddbPtr = const int*;
    return buildVector<I, DT_MINUTE, ddbPtr>(
        [begin, end](ddbPtr& from, ddbPtr& to) {
            transform(begin, end, begin, [](I& u){
                return isNull(u) || isInf(u) ? DDB_INT_NULL : u;
            });
            from = begin;
            to = end;
        },
        &Vector::appendInt, var);
}

VectorSP kdb::toDDB::seconds(I* begin, I* end, const string& var) {
    if(UNLIKELY(!(begin && begin <= end))) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Invalid kdb+ seconds for " + var + ".");
    }

    // DolphinDB doesn't have +/-inf seconds, treat them as nulls.
    using ddbPtr = const int*;
    return buildVector<I, DT_SECOND, ddbPtr>(
        [begin, end](ddbPtr& from, ddbPtr& to) {
            transform(begin, end, begin, [](I& v){
                return isNull(v) || isInf(v) ? DDB_INT_NULL : v;
            });
            from = begin;
            to = end;
        },
        &Vector::appendInt, var);
}

VectorSP kdb::toDDB::times(I* begin, I* end, const string& var) {
    if(UNLIKELY(!(begin && begin <= end))) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Invalid kdb+ times for " + var + ".");
    }

    // DolphinDB doesn't have +/-inf times, treat them as nulls.
    using ddbPtr = const int*;
    return buildVector<I, DT_TIME, ddbPtr>(
        [begin, end](ddbPtr& from, ddbPtr& to) {
            transform(begin, end, begin, [](I& v){
                return isNull(v) || isInf(v) ? DDB_INT_NULL : v;
            });
            from = begin;
            to = end;
        },
        &Vector::appendInt, var);
}

template<typename kValueT, DATA_TYPE ddbType, typename ddbPtr>
VectorSP kdb::toDDB::makeVector(
    ddbPtr begin, ddbPtr end,
    bool(Vector::*append)(ddbPtr, int),
    const string& var
) {
    static_assert(sizeof(kValueT)
            == sizeof(typename remove_pointer<ddbPtr>::type),
        "kdb+ vs DolphinDB type equivalence");

    if(UNLIKELY(!((begin && begin <= end) || (!begin && begin == end)))) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Invalid kdb+ vector for " + var + ".");
    }
    const auto count = end - begin;

    VectorSP vec{ Util::createVector(ddbType, 0, count) };
    assert(!vec.isNull());
    if(ddbType == DT_SYMBOL) {
        vec->initialize();  // initialize symbol base
    }
    (*vec.*append)(begin, count);
    return vec;
}

template<typename kValueT, DATA_TYPE ddbType, typename ddbPtr>
VectorSP kdb::toDDB::buildVector(
    function<void(ddbPtr&, ddbPtr&)>&& build,
    bool(Vector::*append)(ddbPtr, int),
    const string& var
) {
    ddbPtr begin, end;
    build(begin, end);
    return makeVector<kValueT, ddbType>(begin, end, append, var);
}

VectorSP kdb::toDDB::charsList(K* begin, K* end, const string& var) {
    assert(begin && begin <= end);
    const auto count = end - begin;

    vector<string> charArrays;
    charArrays.reserve(count);
    for(auto item = begin; item < end; ++item) {
        if(LIKELY(isValidListOf(*item, K_CHAR))) {
            charArrays.emplace_back(string{kC(*item), kC(*item) + (*item)->n});
        } else {
            throw RuntimeException(PLUGIN_NAME ": Not a valid kdb+ string at "
                + var + "[" + to_string(item - begin) + "].");
        }
    }
    assert(charArrays.size() == static_cast<size_t>(count));

    VectorSP vec{ Util::createVector(DT_STRING, 0, count) };
    vec->appendString(charArrays.data(), count);
    return vec;
}

VectorSP kdb::toDDB::mapStrings(J* begin, J* end,
    const vector<string>& symList, const string& symName, const string& var
) {
    assert(begin && begin <= end);
    const auto count = end - begin;

    /*
    //FIXME: make_unique<>() is available only after C++14...
    auto syms = make_unique<S[]>(count);
    /*/
    unique_ptr<S[]> syms{ new S[count] };
    //*/
    for(auto idx = begin; idx < end; ++idx) {
        S s = nullptr;
        if(UNLIKELY(isNull(*idx))) {
            s = sym(nullptr);
        } else
        if(LIKELY(0 <= *idx && static_cast<size_t>(*idx) < symList.size())) {
            s = sym(symList[*idx]);
        } else {
            LOG(PLUGIN_NAME ": " + var + " - "
                "sym[" + to_string(*idx) + "] not in " + symName + ".");
            s = sym(nullptr);
        }
        assert(s);
        syms[idx - begin] = s;
    }

    return strings(syms.get(), syms.get() + count, var);
}

VectorSP kdb::toDDB::nestedList(Type type,
    K* begin, K* end, const string& var
) {
    assert(begin && begin <= end);
    const auto count = end - begin;
    if(UNLIKELY(count == 0)) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Cannot convert empty nested list " + var + ".");
    }

    // Convert each nested list
    vector<VectorSP> list;
    list.reserve(count);
    transform(begin, end, back_inserter(list), [&var, begin](const K item) {
        return fromK(item, var + '[' + to_string(&item - begin) + ']');
    });
    assert(list.size() == static_cast<size_t>(count));

    // Detect and check if they are all of homogeneous type
    DATA_TYPE dbType = DT_ANY;
    for(auto i = 0u; i < list.size(); ++i) {
        auto const& item = list[i];
        assert(!item.isNull());
        const auto itemType = item->getType();
        if(dbType == DT_ANY) {
            if(itemType != DT_ANY) {
                dbType = itemType;
            }
        } else
        if(itemType == DT_ANY) {
            if(item->size()) {
                throw RuntimeException(PLUGIN_NAME ": "
                    "Unknown data type in a kdb+ list of type"
                    " " + to_string(type) + " "
                    "at " + var + "[" + to_string(i) + "].");
            }
        } else
        if(itemType != dbType) {
            throw RuntimeException(PLUGIN_NAME ": "
                "Mixed data in a kdb+ list of type " + to_string(type) + " "
                "at " + var + "[" + to_string(i) + "] ("
                "expected=" + to_string(dbType) + " "
                "actual=" + to_string(itemType) + ").");
        }
    }

    // Construct nested list for DolphinDB
    VectorSP vec;
    switch(dbType) {
        case DT_ANY:
            vec = Util::createVector(DT_ANY, count);
            fakeEmptyAnyColumn(vec.get(), "*", var);
            for(auto i = 1u; i < list.size(); ++i) {
                auto item = list[i];
                assert(item->getType() == DT_ANY && !item->size());
                vec->set(i, item);
            }
            break;
        case DT_SYMBOL:
        case DT_STRING:
            vec = Util::createVector(DT_ANY, 0, count);
            for_each(list.begin(), list.end(), [&vec, dbType](VectorSP& item) {
                if(item->getType() == DT_ANY) {
                    item = Util::createVector(dbType, 0, 0);
                }
                vec->append(item);
            });
            break;
        default:
            vec = InternalUtil::createArrayVector(
                bit_cast<DATA_TYPE>(ARRAY_TYPE_BASE + dbType),
                0, 0, count, 0);
            for_each(list.begin(), list.end(), [&vec](VectorSP& item) {
                VectorSP tuple = Util::createVector(DT_ANY, 1);
                tuple->set(0, item);
                vec->append(tuple);
            });
    }
    assert(!vec.isNull() && vec->size() == count);

    return vec;
}

//////////////////////////////////////////////////////////////////////////////
#pragma pack(push, 1)

//============================================================================
struct kdb::Parser::ItemIndex {
    union {
        struct {        // For nested data map
            int32_t offset;
            byte    pad_xxx[3];
            byte    format;
        };
        int64_t index;  // For enumerated symbol
    };
};
static_assert(sizeof(kdb::Parser::ItemIndex) == 4+3+1,
    "kdb+ file - mapped list item index");

//============================================================================
struct kdb::Parser::BaseHeader : HeaderTag<0xFF, 0x01, BaseHeader> {
    struct {
        byte      tag[2];
        Type      type;
        Attribute attr;
        int32_t   ref_count;
    };

    using HeaderTag::isValid;

    constexpr bool isValidOf(Type type) const noexcept {
        return isValid() && this->type == type;
    }
};
static_assert(sizeof(kdb::Parser::BaseHeader) == 2+1+1+4,
    "kdb+ file 0xFF01 header - unenumerated syms list");

struct kdb::Parser::SymsList : DataBlock<BaseHeader, char> {
    constexpr size_t getCount() const noexcept {
        return static_cast<size_t>(
            (assert(header.ref_count >= 0), header.ref_count));
    }

    vector<string> getSyms(const byte* end) const {
        if(UNLIKELY(!header.isValidOf(K_STRING))) {
            throw RuntimeException(PLUGIN_NAME ": "
                "kdb+ file - not a valid syms list.");
        }

        vector<string> syms;
        const auto count = getCount() ? getCount() : UNKNOWN_SIZE;
        if(count != UNKNOWN_SIZE) {
            syms.reserve(count);
        }


        const auto eod = reinterpret_cast<const char*>(end);
        const char* next = get();
        while(next < eod && syms.size() < count) {
            const auto eos = find(next, eod, '\0');
            if(UNLIKELY(eos >= eod)) {
                throw RuntimeException(PLUGIN_NAME ": "
                    "Incomplete or truncated syms list.");
            }
            syms.emplace_back(string{next, eos});
            next = eos + 1;
        }
        if(next < eod) {
            LOG(PLUGIN_NAME ": "
                "Found syms beyond designated count=" + to_string(count) + ".");
            while(next < eod) {
                const auto eos = find(next, eod, '\0');
                syms.emplace_back(string{next, eos});
                next = eos + 1;
            }
        }

        assert(syms.size() >= count || count == UNKNOWN_SIZE);
        if(syms.size() > count) {
            LOG(PLUGIN_NAME ": "
                "Actual syms extracted=" + to_string(syms.size()) + ".");
        }
        return syms;
    }
};

//============================================================================
struct kdb::Parser::BaseListHeader
    :  BaseHeader, HeaderTag<0xFE, 0x20, BaseListHeader>
{
    struct {
        int64_t count;
    };

    constexpr bool isValid() const noexcept {
        return HeaderTag<0xFE, 0x20, BaseListHeader>::isValid()
            && K_LIST <= type && type <= K_NESTED_MAX;
    }
};
static_assert(sizeof(kdb::Parser::BaseListHeader) == 8+8,
    "kdb+ file 0xFE20 header - simple list");

struct kdb::Parser::BaseList : DataBlock<BaseListHeader, byte> {
    using DataBlock::isComplete;

    constexpr size_t getCount() const noexcept {
        return static_cast<size_t>((assert(header.count >= 0), header.count));
    }
};

//============================================================================
struct kdb::Parser::ExtListHeader
    :  BaseListHeader, HeaderTag<0xFD, 0x20, ExtListHeader>
{
    using HeaderTag<0xFD, 0x20, ExtListHeader>::isValid;
};
static_assert(sizeof(kdb::Parser::ExtListHeader) == 8+8,
    "kdb+ file 0xFD20 primary header - extended list");

struct kdb::Parser::ExtList {
    static constexpr ptrdiff_t EXT_DATA_SEGMENT = 0x1000;

    struct {
        ExtListHeader header;
        char enum_name[EXT_DATA_SEGMENT
            - (sizeof(ExtListHeader) + sizeof(BaseListHeader))];
        BaseListHeader payload;
    };

    constexpr bool isComplete(const byte* end) const noexcept {
        return end - reinterpret_cast<const byte*>(this) >= EXT_DATA_SEGMENT;
    }

    template<typename T>
    constexpr const T* get() const noexcept {
        return reinterpret_cast<const T*>(&payload);
    }
};
static_assert(sizeof(kdb::Parser::ExtList)
        == kdb::Parser::ExtList::EXT_DATA_SEGMENT,
    "kdb+ file 0xFD20 data block - extendded list & secondary header");

//============================================================================
struct kdb::Parser::SimpleListHeader
    :  BaseListHeader, HeaderTag<0xFD, 0x00, SimpleListHeader>
{
    constexpr bool isValid() const noexcept {
        return HeaderTag<0xFD, 0x00, SimpleListHeader>::isValid()
            && K_LIST <= type && type < K_ENUM_MIN;
    }
};
static_assert(sizeof(kdb::Parser::SimpleListHeader) == 8+8,
    "kdb+ file 0xFD00 secondary header - simple list");

struct kdb::Parser::SimpleList : DataBlock<SimpleListHeader, byte> {
    bool isComplete(const byte* end) const noexcept {
        return DataBlock::isComplete(getSize(header.type), getCount(), end);
    }

    constexpr size_t getCount() const noexcept {
        return static_cast<size_t>((assert(header.count >= 0), header.count));
    }

    VectorSP getVector() const {
        if(UNLIKELY(!header.isValid())) {
            throw RuntimeException(PLUGIN_NAME ": "
                "kdb+ extended file - not a valid simple list.");
        }

        ostringstream msg;
        msg << hex << uppercase << setfill('0') << setw(2) << header.type;
        return toDDB::fromArray(header.type, get(), getCount(),
            "0xFD00" + msg.str());
    }
};

//============================================================================
struct kdb::Parser::EnumSymsHeader
    :  BaseListHeader, HeaderTag<0xFD, 0x00, EnumSymsHeader>
{
    constexpr bool isValid() const noexcept {
        return HeaderTag<0xFD, 0x00, EnumSymsHeader>::isValid()
            && K_ENUM_MIN <= type && type <= K_ENUM_MAX;
    }
};
static_assert(sizeof(kdb::Parser::EnumSymsHeader) == 8+8,
    "kdb+ file 0xFD00 secondary header - enumerated syms list");

struct kdb::Parser::EnumSymsList : DataBlock<EnumSymsHeader, ItemIndex> {
    constexpr bool isComplete(const byte* end) const noexcept {
        return DataBlock::isComplete(getCount(), end);
    }

    constexpr size_t getCount() const noexcept {
        return static_cast<size_t>((assert(header.count >= 0), header.count));
    }

    vector<S> getSyms(
        const vector<string>& symList, const string& symName
    ) const {
        if(UNLIKELY(!header.isValid())) {
            throw RuntimeException(PLUGIN_NAME ": "
                "kdb+ extended file - not a valid enumerated syms list.");
        }

        vector<S> syms;
        syms.reserve(getCount());
        const auto begin = get();
        transform(begin, begin + getCount(), back_inserter(syms),
            [begin,
            &symList, &symName](const ItemIndex& idx) {
                if(isNull(idx.index)) {
                    return sym(nullptr);
                } else
                if(0 <= idx.index
                    && static_cast<size_t>(idx.index) <= symList.size()
                ) {
                    return sym(symList[idx.index]);
                } else {
                    LOG(PLUGIN_NAME ": Enumerated sym out of bounds "
                        + symName + "[" + to_string(idx.index) + "].");
                    return sym(nullptr);
                }
            }
        );
        return syms;
    }
};

//============================================================================
template<byte Tag0, byte Tag1, int32_t MinRef>
struct kdb::Parser::NestedListHeader
    : BaseListHeader, HeaderTag<Tag0, Tag1, NestedListHeader<Tag0, Tag1, MinRef>>
{
    constexpr bool isValid() const noexcept {
        return HeaderTag<Tag0, Tag1, NestedListHeader>::isValid()
            && K_NESTED_MIN <= type && type <= K_NESTED_MAX
            && ref_count >= MinRef;
    }
};
static_assert(sizeof(kdb::Parser::NestedListHeader<0xFD, 0x01>) == 8+8,
    "kdb+ file 0xFD01/0xFB00 secondary/ternary header - nested list");

template<byte Tag0, byte Tag1, int32_t MinRef>
struct kdb::Parser::NestedList
    : DataBlock<NestedListHeader<Tag0, Tag1, MinRef>, ItemIndex>
{
    using base_type =
        DataBlock<NestedListHeader<Tag0, Tag1, MinRef>, ItemIndex>;

    constexpr bool isComplete(const byte* end) const noexcept {
        return base_type::isComplete(getCount(), end);
    }

    constexpr size_t getCount() const noexcept {
        return static_cast<size_t>(
            (assert(base_type::header.count >= 0), base_type::header.count));
    }

    size_t guessCount(const byte* end) const noexcept {
        // Files nested more than 2 levels deep may not store the actual count!
        const auto bytes =
            end - reinterpret_cast<const byte*>(base_type::get());
        assert(bytes >= 0);
        if(UNLIKELY(bytes % sizeof(ItemIndex))) {
            throw RuntimeException(PLUGIN_NAME ": "
                "Truncated or incomplete nested list in kdb+ extended file.");
        }

        return bytes / sizeof(ItemIndex);
    }

    tuple<const ItemIndex*, const ItemIndex*> getIndices() const {
        if(UNLIKELY(!base_type::header.isValid())) {
            throw RuntimeException(PLUGIN_NAME ": "
                "kdb+ extended file - not a valid nested list.");
        }

        const auto begin = base_type::get();
        return make_tuple(begin, begin + getCount());
    }
};

//============================================================================
struct kdb::Parser::NestedItemHeader {
    struct {
        Type   type_o;
        int8_t count;
    };

    static constexpr int8_t TYPE_OFFSET = 0x7F;
    static constexpr int8_t MAX_LENGTH  = 0x5F;

    constexpr Type getType() const noexcept {
        return static_cast<Type>(type_o - TYPE_OFFSET);
    }

    constexpr bool isValid() const noexcept {
        return K_LIST <= getType() && getType() < K_ENUM_MIN;
    }
};
static_assert(sizeof(kdb::Parser::NestedItemHeader) == 1+1,
    "kdb+ file 0x7F?? ternary header - simple nested item");

struct kdb::Parser::NestedItem : DataBlock<NestedItemHeader, byte> {
    bool isComplete(const byte* end) const noexcept {
        const auto itemSize = getSize(header.getType());
        if(UNLIKELY(itemSize == UNKNOWN_SIZE)) {
            throw RuntimeException(PLUGIN_NAME ": "
                  "Cannot handle the simple nested item "
                  "(type=" + to_string(header.type_o) + "->"
                + to_string(header.getType()) + ").");
        } else
        if(UNLIKELY(itemSize * getCount() > header_t::MAX_LENGTH)) {
            throw RuntimeException(PLUGIN_NAME ": "
                  "Simple nested item overflow "
                  "(type=" + to_string(header.type_o) + "->"
                + to_string(header.getType())
                + "|count=" + to_string(getCount()) + ").");
        } else {
            return DataBlock::isComplete(itemSize, getCount(), end);
        }
    }

    constexpr size_t getCount() const noexcept {
        return static_cast<size_t>((assert(header.count >= 0), header.count));
    }
};

//============================================================================
struct kdb::Parser::NestedItemExHeader
    : BaseListHeader, HeaderTag<0xFB, 0x00, NestedItemExHeader>
{
    static constexpr int8_t MIN_LENGTH = NestedItemHeader::MAX_LENGTH + 1;

    constexpr Type getType() const noexcept {
        return type;
    }

    constexpr bool isValid() const noexcept {
        return HeaderTag<0xFB, 0x00, NestedItemExHeader>::isValid()
            && K_LIST <= getType() && getType() < K_ENUM_MIN
            && ref_count >= 1;
    }
};
static_assert(sizeof(kdb::Parser::NestedItemExHeader) == 8+8,
    "kdb+ file 0xFB00 ternary header - extended nested item");

struct kdb::Parser::NestedItemEx : DataBlock<NestedItemExHeader, byte> {
    bool isComplete(const byte* end) const noexcept {
        const auto itemSize = getSize(header.getType());
        if(UNLIKELY(itemSize == UNKNOWN_SIZE)) {
            throw RuntimeException(PLUGIN_NAME ": "
                  "Cannot handle the extended nested item "
                  "(type=" + to_string(header.getType()) + ").");
        } else
        if(UNLIKELY(itemSize * getCount() < header_t::MIN_LENGTH)) {
            throw RuntimeException(PLUGIN_NAME ": "
                  "Simple nested item overflow "
                  "(type=" + to_string(header.getType())
                + "|count=" + to_string(getCount()) + ").");
        } else {
            return DataBlock::isComplete(itemSize, getCount(), end);
        }
    }

    constexpr size_t getCount() const noexcept {
        return static_cast<size_t>((assert(header.count >= 0), header.count));
    }
};

//============================================================================
struct kdb::Parser::NestedEnumSymsHeader : kdb::Parser::NestedItemHeader {
    struct {
        byte   pad_xxx[3];
        int8_t count_syms;
    };

    using NestedItemHeader::TYPE_OFFSET;
    using NestedItemHeader::MAX_LENGTH;

    using NestedItemHeader::getType;

    constexpr bool isValid() const noexcept {
        return K_ENUM_MIN <= getType() && getType() <= K_ENUM_MAX;
    }
};
static_assert(sizeof(kdb::Parser::NestedEnumSymsHeader) == 1+4+1,
    "kdb+ file 0x93?? ternary header - simple nested enum syms");

struct kdb::Parser::NestedEnumSyms : DataBlock<NestedEnumSymsHeader, int64_t> {
    bool isComplete(const byte* end) const noexcept {
        if(UNLIKELY(sizeof(data_t) * getCount() > header_t::MAX_LENGTH)) {
            throw RuntimeException(PLUGIN_NAME ": "
                  "Simple nested enum syms overflow "
                  "(type=" + to_string(header.type_o) + "->"
                + to_string(header.getType())
                + "|count=" + to_string(getCount()) + ").");
        } else {
            return DataBlock::isComplete(getCount(), end);
        }
    }

    constexpr size_t getCount() const noexcept {
        return static_cast<size_t>(
            (assert(header.count_syms >= 0), header.count_syms));
    }
};

//============================================================================
struct kdb::Parser::NestedEnumSymsExHeader
    : BaseListHeader, HeaderTag<0xFB, 0x00, NestedEnumSymsExHeader>
{
    static constexpr int8_t MIN_LENGTH = NestedEnumSymsHeader::MAX_LENGTH + 1;

    constexpr Type getType() const noexcept {
        return type;
    }

    constexpr bool isValid() const noexcept {
        return HeaderTag<0xFB, 0x00, NestedEnumSymsExHeader>::isValid()
            && K_ENUM_MIN <= getType() && getType() <= K_ENUM_MAX
            && ref_count >= 1;
    }
};
static_assert(sizeof(kdb::Parser::NestedItemExHeader) == 8+8,
    "kdb+ file 0xFB00 ternary header - extended nested enum syms");

struct kdb::Parser::NestedEnumSymsEx
    : DataBlock<NestedEnumSymsExHeader, int64_t>
{
    bool isComplete(const byte* end) const noexcept {
        if(UNLIKELY(sizeof(data_t) * getCount() < header_t::MIN_LENGTH)) {
            throw RuntimeException(PLUGIN_NAME ": "
                  "Simple nested enum syms overflow "
                  "(type=" + to_string(header.getType())
                + "|count=" + to_string(getCount()) + ").");
        } else {
            return DataBlock::isComplete(getCount(), end);
        }
    }

    constexpr size_t getCount() const noexcept {
        return static_cast<size_t>((assert(header.count >= 0), header.count));
    }
};

//============================================================================
struct kdb::Parser::NestedMapHeader
    :  BaseListHeader, HeaderTag<0xFD, 0x00, NestedMapHeader>
{
    constexpr bool isValid() const noexcept {
        return HeaderTag<0xFD, 0x00, NestedMapHeader>::isValid()
            && type == K_BYTE;
    }
};
static_assert(sizeof(kdb::Parser::NestedMapHeader) == 8+8,
    "kdb+ file 0xFD00 secondary header - nested data map");

struct kdb::Parser::NestedMap : DataBlock<NestedMapHeader, byte> {
    constexpr bool isComplete(const byte* end) const noexcept {
        return DataBlock::isComplete(0, end);
    }

    static string stringize(const ItemIndex& idx) {
        ostringstream msg;
        msg << '<' <<"0x"
            << hex << uppercase << setfill('0') << setw(8) << idx.offset
            << '|' << setw(2) << unsigned{idx.format} << '>';
        return msg.str();
    }

    // is enumerated?
    tuple<ConstantSP, bool> getItem(
        const ItemIndex& idx, const byte* end) const;

    ConstantSP getItem(const NestedEnumSyms* item,
        const ItemIndex& idx, const byte* end) const;
    ConstantSP getItem(const NestedEnumSymsEx* item,
        const ItemIndex& idx, const byte* end) const;
    ConstantSP getItem(const NestedList<0xFB, 0x00, 1>* item,
        const ItemIndex& idx, const byte* end) const;

    template<typename ItemT>
    ConstantSP getItem(const ItemT* item,
        const ItemIndex& idx, const byte* end) const;
};

template<typename ItemT>
ConstantSP kdb::Parser::NestedMap::getItem(
    const ItemT* item, const ItemIndex& idx, const byte* end
) const {
    assert(item && item->header.isValid());
    const auto type = item->header.getType();
    if(UNLIKELY(!item->isComplete(end))) {
        throw RuntimeException(PLUGIN_NAME ": "
                "Truncated nested item "
            + stringize(idx) + " (type=" + to_string(type) + ").");
    }

    switch(type) {
        case K_CHAR:
            return new String{ string(
                reinterpret_cast<char*>(item->get()), item->getCount()) };
        case K_STRING:
            throw RuntimeException(PLUGIN_NAME ": "
                    "Unexpected unenumerated nested item "
                + stringize(idx) + " (type=" + to_string(type) + ").");
        default:
            return toDDB::fromArray(type, item->get(), item->getCount(),
                stringize(idx));
    }
}

ConstantSP kdb::Parser::NestedMap::getItem(
    const NestedEnumSyms* item, const ItemIndex& idx, const byte* end
) const {
    assert(item && item->header.isValid());
    if(UNLIKELY(!item->isComplete(end))) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Truncated simple nested enum syms " + stringize(idx) + " "
            "(type=" + to_string(item->header.getType()) + ").");
    }

    return toDDB::longs(item->get(), item->get() + item->getCount(),
        stringize(idx));
}

ConstantSP kdb::Parser::NestedMap::getItem(
    const NestedEnumSymsEx* item, const ItemIndex& idx, const byte* end
) const {
    assert(item && item->header.isValid());
    if(UNLIKELY(!item->isComplete(end))) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Truncated extended nested enum syms " + stringize(idx) + " "
            "(type=" + to_string(item->header.getType()) + ").");
    }

    return toDDB::longs(item->get(), item->get() + item->getCount(),
        stringize(idx));
}

ConstantSP kdb::Parser::NestedMap::getItem(
    const NestedList<0xFB, 0x00, 1>* item,
    const ItemIndex& idx, const byte* end
) const {
    assert(item && item->header.isValid());
    if(UNLIKELY(!item->isComplete(end))) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Truncated nested item list " + stringize(idx) + " "
            "(type=" + to_string(item->header.type) + ").");
    }

    const ItemIndex *from, *to;
    tie(from, to) = item->getIndices();
    assert(from && from <= to);

    VectorSP vec = Util::createVector(DT_STRING, 0, item->getCount());
    for(auto i = from; i < to; ++i) {
        ConstantSP entry;
        bool isEnumSyms;
        tie(entry, isEnumSyms) = getItem(*i, end);
        assert(!entry.isNull());

        switch(entry->getType()) {
            case DT_STRING:
                vec->append(entry);
                break;
            default:
                throw RuntimeException(PLUGIN_NAME ": "
                    "Non-string nested list " + stringize(idx) + " "
                    "(type=" + to_string(entry->getType()) + ") "
                    "not supported.");
        }
    }

    assert(static_cast<size_t>(vec->size()) == item->getCount());
    return vec;
}

tuple<ConstantSP, bool> kdb::Parser::NestedMap::getItem(
    const ItemIndex& idx, const byte* end
) const {
    if(UNLIKELY(!header.isValid())) {
        throw RuntimeException(PLUGIN_NAME ": "
            "kdb+ nested data map - not a valid data map.");
    }

    switch(idx.format) {
        case 0x00:
            if(getAt<NestedItemExHeader>(idx.offset)->isValid()) {
                return make_tuple(
                    getItem(getAt<NestedItemEx>(idx.offset), idx, end),
                    false);
            } else
            if(getAt<NestedListHeader<0xFB, 0x00, 1>>(idx.offset)->isValid()) {
                return make_tuple(
                    getItem(getAt<NestedList<0xFB, 0x00, 1>>(
                        idx.offset), idx, end),
                    false);
            } else
            if(getAt<NestedEnumSymsExHeader>(idx.offset)->isValid()) {
                return make_tuple(
                    getItem(getAt<NestedEnumSymsEx>(idx.offset), idx, end),
                    true);
            } else {
                throw RuntimeException(PLUGIN_NAME ": "
                    "kdb+ nested data map - (" + stringize(idx) +") "
                    "is not an extended nested item.");
            }
        case 0x01:
            if(getAt<NestedItemHeader>(idx.offset)->isValid()) {
                return make_tuple(
                    getItem(getAt<NestedItem>(idx.offset), idx, end),
                    false);
            } else
            if(getAt<NestedEnumSymsHeader>(idx.offset)->isValid()) {
                return make_tuple(
                    getItem(getAt<NestedEnumSyms>(idx.offset), idx, end),
                    true);
            } else {
                throw RuntimeException(PLUGIN_NAME ": "
                    "kdb+ nested data map - (" + stringize(idx) +") "
                    "is not a simple nested item.");
            }
        default:
            throw RuntimeException(PLUGIN_NAME ": "
                "kdb+ nested data map - "
                "cannot recognize nested item (" + stringize(idx) + ").");
    }
}

//============================================================================
struct kdb::Parser::Trailer {
    int32_t  tag[2];
    int64_t end_offset[2];

    static constexpr int32_t TAG[] = { 0x03, 0x02 };

    constexpr bool isValid(const byte* begin) const {
        return tag[0] == TAG[0] && tag[1] == TAG[1]
            && end_offset[0] == reinterpret_cast<const byte*>(this) - begin
            && end_offset[1] == static_cast<int64_t>(
                    end_offset[0] + sizeof(end_offset[0]));
    }
};
static_assert(sizeof(kdb::Parser::Trailer) == 4*2+8+8,
    "kdb+ file trailer (optional)");

#pragma pack(pop)
//////////////////////////////////////////////////////////////////////////////

vector<kdb::byte>& kdb::Parser::getBuffer() noexcept {
    return buffer_;
}

const byte* kdb::Parser::begin() const noexcept {
    return (assert(!buffer_.empty()), buffer_.data());
}

const byte* kdb::Parser::end() const noexcept {
    return begin() + buffer_.size();
}

const byte* kdb::Parser::findEnd(const byte* start) const noexcept {
    assert(begin() <= start && start < end());
    const auto from = start ? start : begin();
    const auto to   = end() - sizeof(Trailer);
    for(auto p = from; p <= to; ++p) {
        const auto trailer = reinterpret_cast<const Trailer*>(p);
        if(trailer->isValid(begin())) {
            return p;
        }
    }
    return end();
}

template<typename T>
const T* kdb::Parser::parse(ptrdiff_t index, bool allowEnd) const {
    return static_cast<const T*>(parse(sizeof(T), index, allowEnd));
}

// Allow for "end" semantics
const void* kdb::Parser::parse(
    size_t size, ptrdiff_t offset, bool allowEnd
) const {
    if(UNLIKELY(
        !(0 <= offset && static_cast<size_t>(offset) <= buffer_.size())
    )) {
        throw RuntimeException(PLUGIN_NAME ": kdb+ file access out of bounds "
            "(" + to_string(offset) + ")!");
    } else
    if(UNLIKELY(!allowEnd && offset + size > buffer_.size())) {
        throw RuntimeException(PLUGIN_NAME ": kdb+ file access out of bounds "
            "(" + to_string(offset) + ":" + to_string(offset + size) + ")!");
    } else {
        return begin() + offset;
    }
}

vector<string> kdb::Parser::getStrings(const string& file) const {
    const auto symsList = parse<SymsList>();
    return symsList->getSyms(end());
}

VectorSP kdb::Parser::getVector(
    const string& file, const vector<string>& symList, const string& symName
) const {
    if(parse<BaseHeader>()->isValid()) {
        const auto data = parse<SymsList>();
        if(UNLIKELY(data->getCount())) {
            throw RuntimeException(PLUGIN_NAME ": "
                "Unexpected non-empty kdb+ file with magic 0xFF01.");
        } else {
            VectorSP any = Util::createVector(DT_ANY, 0);
            fakeEmptyAnyColumn(any.get(), file, "");
            return any;
        }
    } else
    if(parse<BaseListHeader>()->isValid()) {
        return getFastVector(parse<BaseList>(), file, symList, symName);
    } else
    if(parse<ExtListHeader>()->isValid()) {
        return getGeneralList(parse<ExtList>(), file, symList, symName);
    } else {
        ostringstream msg;
        if(buffer_.size() >= 2) {
            msg << "0x" << hex << uppercase << setfill('0')
                << setw(2) << unsigned{buffer_[0]}
                << setw(2) << unsigned{buffer_[1]};
        } else {
            msg << "insufficient bytes";
        }
        throw RuntimeException(PLUGIN_NAME ": "
            "<nyi> kdb+ file magic in " + file + " (" + msg.str() + ").");
    }
}

VectorSP kdb::Parser::getFastVector(const BaseList* data,
    const string& file, const vector<string>& symList, const string& symName
) const {
    assert(data);
    const auto type = data->header.type;
    const auto itemSize = getSize(type);
    if(itemSize == UNKNOWN_SIZE) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Cannot recognize data type in kdb+ file " + file + " "
            "(" + to_string(type) + ").");
    }

    const auto count = data->getCount();
    if(UNLIKELY(!data->isComplete(itemSize, count, end()))) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Truncated or incomplete kdb+ file " + file + " "
            "(" + to_string(type) + ").");
    }

    if(UNLIKELY(type == K_STRING)) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Unexpected unenumerated syms list in " + file + ".");
    } else
    if(K_ENUM_MIN <= type && type <= K_ENUM_MAX) {
//        return getEnumStrings(data, file, symList, symName);
throw RuntimeException(__FILE__ ":" + to_string(__LINE__));
    } else
    if(K_NESTED_MIN <= type && type <= K_NESTED_MAX) {
//        return getNestedVector(data, file, symList, symName);
throw RuntimeException(__FILE__ ":" + to_string(__LINE__));
    } else {
        return toDDB::fromArray(type, data->get(), count, file);
    }
}

VectorSP kdb::Parser::getGeneralList(const ExtList* data,
    const string& file, const vector<string>& symList, const string& symName
) const {
    assert(data);
    if(UNLIKELY(!data->isComplete(end()))) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Truncated or incomplete kdb+ extended file " + file + ".");
    }

    if(data->get<SimpleListHeader>()->isValid()) {
        return data->get<SimpleList>()->getVector();
    } else
    if(data->get<EnumSymsHeader>()->isValid()) {
        if(UNLIKELY(symName.empty())) {
            throw RuntimeException(PLUGIN_NAME ": "
                "Enum sym has not been loaded for kdb+ extended file"
                " " + file + " yet.");
        } else
        if(UNLIKELY(symName != data->enum_name)) {
            throw RuntimeException(PLUGIN_NAME ": "
                "kdb+ extended file " + file + " "
                "was not enumerated on " + symName + ".");
        }
        return getEnumStrings(data->get<EnumSymsList>(),
            file, symList, symName);
    } else
    if(data->get<NestedListHeader<0xFD, 0x01>>()->isValid()) {
        return getNestedLists(
            data->get<NestedList<0xFD, 0x01>>(), file, symList, symName);
    }

    ostringstream msg;
    msg << hex << uppercase << setfill('0')
        << setw(2) << unsigned{data->payload.tag[0]}
        << setw(2) << unsigned{data->payload.tag[1]};
    throw RuntimeException(PLUGIN_NAME ": "
        "Cannot recognize extension header in " + file + " "
        "(0x" + msg.str() + ").");
}

VectorSP kdb::Parser::getEnumStrings(const EnumSymsList* data,
    const string& file, const vector<string>& symList, const string& symName
) const {
    assert(data);
    if(UNLIKELY(!data->isComplete(end()))) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Truncated or incomplete kdb+ extended file " + file + " "
            "(" + to_string(data->header.type) + ").");
    }

    const auto syms = data->getSyms(symList, symName);
    assert(syms.size() == data->getCount());
    const auto begin = const_cast<S*>(syms.data());
    return toDDB::strings(begin, begin + syms.size(), file);
}

VectorSP kdb::Parser::getNestedLists(const NestedList<0xFD, 0x01>* data,
    const string& file, const vector<string>& symList, const string& symName
) const {
    assert(data);
    const auto type = data->header.type;
    assert(K_NESTED_MIN <= type && type <= K_NESTED_MAX);
    if(UNLIKELY(!data->isComplete(end()))) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Truncated or incomplete kdb+ extended file " + file + " "
            "(" + to_string(type) + ").");
    }

    const ItemIndex *begin, *end;
    tie(begin, end) = data->getIndices();
    if(!data->getCount()) {
        end = begin + data->guessCount(this->end());
    }
    assert(begin && begin <= end);
    if(begin == end) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Unexpected empty nested list in " + file + ".");
    }

    const auto mapPath = file + '#';
    BinFile mapFile{mapPath, mapPath};
    Parser mapParser;
    mapFile.readInto(mapParser.getBuffer());
    if(mapParser.parse<ExtListHeader>()->isValid()) {
        const auto mapData = mapParser.parse<ExtList>();
        if(UNLIKELY(!mapData->isComplete(mapParser.end()))) {
            throw RuntimeException(PLUGIN_NAME ": "
                "Truncated or incomplete kdb+ data map " + mapPath + " "
                "(" + to_string(mapData->header.type) + ")");
        } else
        if(UNLIKELY(!mapData->get<NestedMapHeader>()->isValid())) {
            throw RuntimeException(PLUGIN_NAME ": "
                "Cannot recognize kdb+ data map header in " + mapPath + ".");
        } else {
            return mapNestedLists(begin, end,
                mapData->get<NestedMap>(), mapParser.end(), mapPath,
                symList, symName);
        }
    } else {
        throw RuntimeException(PLUGIN_NAME ": "
            "Cannot recognize kdb+ data map file header in " + mapPath + ".");
    }
}

VectorSP kdb::Parser::mapNestedLists(
    const ItemIndex* begin, const ItemIndex* end,
    const NestedMap* mapData, const byte* mapEnd, const string& mapFile,
    const vector<string>& symList, const string& symName
) const {
    assert(begin && begin < end);
    assert(mapData);

    // Parse the first item...
    auto idx = begin;
    VectorSP item;
    bool isEnumSyms;
    tie(item, isEnumSyms) = mapData->getItem(*idx, mapEnd);
    assert(!item.isNull());
    if(isEnumSyms) {
        item = mapEnumSyms(item, mapFile, symList, symName);
    }

    // ... and check if we can create an array vector instead of an any vector
    const auto type = item->getType();
    bool isArrayVector;
    switch(type) {
        case DT_SYMBOL:
        case DT_STRING:
            isArrayVector = false;
            break;
        default:
            isArrayVector = true;
    }

    VectorSP vec, tuple;
    const auto count = end - begin;
    if(isArrayVector) {
        vec = InternalUtil::createArrayVector(
            static_cast<DATA_TYPE>(ARRAY_TYPE_BASE + item->getType()),
            0, 0, count, 0);
        tuple = Util::createVector(DT_ANY, 1);
        tuple->set(0, item);
        vec->append(tuple);
    } else {
        vec = Util::createVector(DT_ANY, 0, count);
        vec->append(item);
    }

    // Parse the remaining items in sequence
    for(++idx; idx < end; ++idx) {
        tie(item, isEnumSyms) = mapData->getItem(*idx, mapEnd);
        assert(!item.isNull());
        if(isEnumSyms) {
            item = mapEnumSyms(item, mapFile, symList, symName);
        }

        if(UNLIKELY(item->getType() != type)) {
            throw RuntimeException(PLUGIN_NAME ": "
                "Heterogeneous nested data in " + mapFile + " not supported.");
        } else
        if(isArrayVector) {
            tuple->set(0, item);
            vec->append(tuple);
        } else {
            vec->append(item);
        }
    }

    assert(vec->size() == count);
    return vec;
}

VectorSP kdb::Parser::mapEnumSyms(const VectorSP& indices,
    const string& mapFile, const vector<string>& symList, const string& symName
) {
    assert(indices.get() && indices->getType() == DT_LONG);

    if(UNLIKELY(symName.empty())) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Enum sym has not been loaded for kdb+ extended file"
            " " + mapFile + " yet.");
    }

    const auto enumNamePath = mapFile + '#';
    BinFile enumNameFile{enumNamePath, enumNamePath};
    Parser enumNameParser;
    enumNameFile.readInto(enumNameParser.getBuffer());
    const auto enumNames = enumNameParser.getStrings(enumNamePath);
    if(UNLIKELY(enumNames.size() != 1)) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Failed to identify symbol enum name from " + enumNamePath + ".");
    } else
    if(UNLIKELY(symName != enumNames.front())) {
        throw RuntimeException(PLUGIN_NAME ": "
            "kdb+ extended file " + mapFile + " "
            "was not enumerated on " + symName + ".");
    }

	const auto begin = indices->getLongConst(0, indices->size(), nullptr);
    const auto end   = begin + indices->size();
    static_assert(sizeof(remove_pointer<decltype(begin)>::type) == sizeof(J),
        "avoid enum sym index copy");
    return toDDB::mapStrings(
        const_cast<J*>(begin), const_cast<J*>(end), symList, symName, mapFile);
}

//////////////////////////////////////////////////////////////////////////////