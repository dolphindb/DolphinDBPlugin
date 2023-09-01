#include "q2ddb.h"

#include <cassert>
#include <sstream>
#include <iomanip>

#include "Util.h"
#include "ScalarImp.h"
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

S kdb::sym(const char* str) noexcept { return const_cast<S>(str); }
S kdb::sym(const string& str) noexcept { return kdb::sym(str.c_str()); }

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
    if(UNLIKELY(count == Parser::UNKNOWN_COUNT)) {
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
    return buildVector<I, DT_MINUTE, ddbPtr>(
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

    if(UNLIKELY(!(begin && begin <= end))) {
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
            s = sym("");
        } else
        if(LIKELY(0 <= *idx && static_cast<size_t>(*idx) < symList.size())) {
            s = sym(symList[*idx].c_str());
        } else {
            LOG(PLUGIN_NAME ": " + var + " - "
                "sym[" + to_string(*idx) + "] not in " + symName + ".");
            s = sym("");
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

    auto item = begin;
    VectorSP val = fromK(*item, "first(" + var + ")");
    assert(!val.isNull());
    bool isArrayVector;
    switch(val->getType()) {
        case DT_SYMBOL:
        case DT_STRING:
            isArrayVector = false;
            break;
        default:
            isArrayVector = true;
    }

    VectorSP vec, tuple;
    if(isArrayVector) {
        vec = InternalUtil::createArrayVector(
                bit_cast<DATA_TYPE>(ARRAY_TYPE_BASE + val->getType()),
                0, 0, count, 0);
        tuple = Util::createVector(DT_ANY, 1);
        tuple->set(0, val);
        vec->append(tuple);
    } else {
        vec = Util::createVector(DT_ANY, 0, count);
        vec->append(val);
    }

    for(++item; item < end; ++item) {
        const string itemName = var + "[" + to_string(item - begin) + "]";
        if(LIKELY(isValidListOf(*item, type))) {
            val = fromK(*item, itemName);
            assert(!val.isNull());
            if(isArrayVector) {
                tuple->set(0, val);
                vec->append(tuple);
            } else {
                vec->append(val);
            }
        } else {
            throw RuntimeException(PLUGIN_NAME ": Not a valid kdb+ list "
                "of type " + to_string(type) + " at " + itemName + ".");
        }
    }

    assert(vec->size() == count);
    return vec;
}

//////////////////////////////////////////////////////////////////////////////

kdb::Parser::State::State()
  : type{K_ERROR}, attr{K_ATTR_NONE}, count{UNKNOWN_COUNT},
    header{0}, data{0}, end{0}, enumName{0}
{}

vector<kdb::byte>& kdb::Parser::getBuffer() noexcept {
    return buffer_;
}

const kdb::Parser::Header* kdb::Parser::getHeader() const {
    return get<Header>(state_.header);
}

const kdb::Parser::HeaderEx* kdb::Parser::getHeaderEx() const {
    return get<HeaderEx>(state_.header);
}

// Allow for "end" semantics
void* kdb::Parser::get(ptrdiff_t offset, size_t size, bool end) {
    const auto max = static_cast<ptrdiff_t>(buffer_.size());
    if(UNLIKELY(!(0 <= offset && offset <= max))) {
        throw RuntimeException(PLUGIN_NAME ": kdb+ file access out of bounds "
            "(" + to_string(offset) + ")!");
    } else
    if(UNLIKELY(!end && static_cast<ptrdiff_t>(offset + size) > max)) {
        throw RuntimeException(PLUGIN_NAME ": kdb+ file access out of bounds "
            "(" + to_string(offset) + ":" + to_string(offset + size) + ")!");
    } else {
        return buffer_.data() + offset;
    }
}

bool kdb::Parser::initialized() const {
    return state_.data > 0;
}

void kdb::Parser::initialize(bool force) {
    if(initialized() && !force) {
        return;
    }

    const auto header = get<Header>();
    switch(header->format[0]) {
        case 0xFF:
            if(LIKELY(header->format[1] == 0x01)) {
                return initialize(0, &Header::pad_or_count);
            }
            break;
        case 0xFE:
            if(LIKELY(header->format[1] == 0x20)) {
                return initialize(0, &HeaderEx::count);
            }
            break;
        case 0xFD:
            if(LIKELY(header->format[1] == 0x20)) {
                return initialize(
                    NEW_DATA_OFFSET - sizeof(HeaderEx), &HeaderEx::count);
            }
            break;
        default:
            ;//no-op
    }
    ostringstream msg;
    msg << "(0x" << hex << uppercase << setfill('0')
        << setw(2) << unsigned{header->format[0]}
        << setw(2) << unsigned{header->format[1]}
        << ")";
    throw RuntimeException(PLUGIN_NAME ": "
        "Unknown kdb+ file magic " + msg.str() + ".");
}

/**
 *   00   01   02   03   04   05   06   07   08~0F
 * +----+----+----+----+----+----+----+----+
 * | FF | 01 |type|attr| count / 0000 0000 |
 * +----+----+----+----+----+----+----+----+--------------------------------+
 * | FE | 20 |type|attr| 00 | 00 | 00 | 00 |   count / 0000 0000 0000 0000  |
 * +----+----+----+----+----+----+----+----+--------------------------------+
 * 
 *   00   01    02~0F       10~0FEF     0FF0 0FF1  0FF2~0FFF  1000
 * +----+----+---...---+-----.....-----+----+----+---.....---+---...
 * | FD | 20 |  00..00 | enum / 00..00 | FD | 00 |   .....   |   ...
 * +----+----+---...---+-----.....-----+----+----+---.....---+---...
 * | FD | 20 |  00..00 |     00..00    | FD | 01 |   .....   |   ...
 * +----+----+---...---+-----.....-----+----+----+---.....---+---...
 */
template<typename HeaderT, typename CountT>
void kdb::Parser::initialize(ptrdiff_t offset, CountT(HeaderT::*count)) {
    const auto header = get<HeaderT>(offset);
    state_.type = static_cast<Type>(header->type);
    state_.attr = static_cast<Attribute>(header->attr);
    state_.count = static_cast<size_t>(header->*count);
    state_.header = offset;
    state_.enumName = 0 + sizeof(HeaderT);
    state_.data = offset + sizeof(HeaderT);
    assert(buffer_.size() <= numeric_limits<decltype(State::end)>::max());
    state_.end = buffer_.size();
    assert(state_.data <= state_.end);

    const auto item = itemSize(state_.type);
    if(state_.count == 0) {
        if(item == UNKNOWN_COUNT) {
            LOG(PLUGIN_NAME ": Cannot guess list item count for "
                + to_string(state_.type) + ".");
            state_.count = UNKNOWN_COUNT;
        } else {
            state_.count = guessItemCount(item);
        }
    }

    if(state_.count != UNKNOWN_COUNT && item != UNKNOWN_COUNT) {
        const auto expected = state_.data + item * state_.count;
        if(UNLIKELY(static_cast<ptrdiff_t>(expected) > state_.end)) {
            throw RuntimeException(PLUGIN_NAME ": "
                "Truncated or incomplete kdb+ list.");
        }
    }
}

size_t kdb::Parser::itemSize(Type type) {
    switch(abs(type)) {
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
                return sizeof(ItemIndex);
            } else {
                return UNKNOWN_COUNT;
            }
    }
}

size_t kdb::Parser::guessItemCount(size_t itemSize) const {
    assert(itemSize != UNKNOWN_COUNT);

    auto dataLen = state_.end - state_.data;
    const auto end = static_cast<ptrdiff_t>(state_.end - sizeof(Trailer));
    for(auto p = state_.data; p <= end; ++p) {
        const auto trailer = get<Trailer>(p);
        if(trailer->valid(p + sizeof(Header), itemSize)) {
            dataLen = p - state_.data;
            break;
        }
    }

    if(UNLIKELY(dataLen % itemSize)) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Truncated or incomplete kdb+ list file "
            "(" + to_string(state_.type) + ").");
    } else {
        return dataLen / itemSize;
    }
}

void kdb::Parser::verifyItemCount(size_t itemSize) const {
    const auto dataLen = state_.end - state_.data;
    if (UNLIKELY(state_.count == UNKNOWN_COUNT)) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Unknown kdb+ list length in file.");
    } else
    if(UNLIKELY(dataLen / itemSize < state_.count)) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Truncated or incomplete kdb+ list file.");
    }
}

bool kdb::Parser::Trailer::valid(ptrdiff_t tail, size_t itemSize) const {
    return pad_03 == 3 && pad_02 == 2
        && last_addr == static_cast<decltype(last_addr)>(tail - itemSize)
        && trailer_addr == tail;
}

vector<string> kdb::Parser::strings(const string& file) {
    assert(initialized());
    if(UNLIKELY(state_.type != K_STRING)) {
        throw RuntimeException(PLUGIN_NAME ": "
            + file + " is not a symbol list "
              "(" + to_string(state_.type) + ").");
    }

    std::vector<string> syms;
    const bool knownCount = state_.count != UNKNOWN_COUNT;
    if(knownCount) {
        syms.reserve(state_.count);
    }

    for(auto next = state_.data; next < state_.end; ) {
        const auto eos = find(
                get<char>(next), get<char>(state_.end, true), '\0');
        const auto end = eos - reinterpret_cast<char*>(buffer_.data());
        if(UNLIKELY(end >= state_.end)) {
            throw RuntimeException(PLUGIN_NAME ": "
                "Truncated symbol in " + file + ".");
        }
        syms.emplace_back(string{get<char>(next), eos});
        if(syms.size() == state_.count) {
            break;
        }
        next = end + 1;
    }

    if(UNLIKELY(knownCount && syms.size() != state_.count)) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Incomplete symbol list " + file + ".");
    }
    return syms;
}

VectorSP kdb::Parser::strings(const string& file,
    const std::vector<string>& symList, const string& symName
) {
    assert(initialized());
    if(UNLIKELY(!(K_ENUM_MIN <= state_.type && state_.type <= K_ENUM_MAX))) {
        throw RuntimeException(PLUGIN_NAME ": "
            + file + " is not an enumerated symbol list "
              "(" + to_string(state_.type) + ").");
    }

    const string enumName{get<char>(state_.enumName)};
    if(symName.empty()) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Enum sym file not yet loaded.");
    }
     else
    if(enumName != symName) {
        throw RuntimeException(PLUGIN_NAME ": "
            + file + " was not enumerated on " + symName + ".");
    }

    verifyItemCount(sizeof(J));
    const auto begin = get<J>(state_.data);
    const auto end   = get<J>(state_.data + sizeof(J) * state_.count, true);
    return toDDB::mapStrings(begin, end, symList, symName, file);
}

VectorSP kdb::Parser::vector(const string& file) {
    assert(initialized());
    verifyItemCount(1);
    return toDDB::fromArray(state_.type,
        buffer_.data() + state_.data, state_.count, file);
}

namespace kdb {

    template<>
    ConstantSP Parser::mapNestedItem<0x00>(ptrdiff_t, Parser&,
        const std::vector<string>&, const string&, const string&);
    template<>
    ConstantSP Parser::mapNestedItem<0x01>(ptrdiff_t, Parser&,
        const std::vector<string>&, const string&, const string&);

    template<>
    VectorSP Parser::mapNestedStrings<0x00>(ptrdiff_t, Parser&,
        const std::vector<string>&, const string&, const string&);
    template<>
    VectorSP Parser::mapNestedStrings<0x01>(ptrdiff_t, Parser&,
        const std::vector<string>&, const string&, const string&);

}//namespace kdb

VectorSP kdb::Parser::nestedList(Parser& mapParser,
    const std::vector<string>& symList, const string& symName,
    const string& file, const string& mapFile
) {
    assert(initialized() && mapParser.initialized());

    const auto mapHeader = mapParser.getHeaderEx();
    switch(mapHeader->format[0]) {
        case 0xFD:
            if(LIKELY(mapHeader->format[1] == 0x00)) {
                if(LIKELY(mapHeader->type == K_BYTE)) {
                    return mapNestedList(
                        mapParser, symList, symName, file, mapFile);
                }
            }
            break;
        default:
            ;//no-op
    }
    ostringstream msg;
    msg << "(0x" << hex << uppercase << setfill('0')
        << setw(2) << mapHeader->format[0] << setw(2) << mapHeader->format[1]
        << "|" << "type=" << dec << unsigned{mapHeader->type} << ")";
    throw RuntimeException(PLUGIN_NAME ": "
          "Unsupported kdb+ nested list# extended header "
        + msg.str() + " in " + mapFile + ".");
}

VectorSP kdb::Parser::mapNestedList(Parser& mapParser,
    const std::vector<string>& symList, const string& symName,
    const string& file, const string& mapFile
) {
    verifyItemCount(sizeof(ItemIndex));

    if(UNLIKELY(state_.count == 0)) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Cannot parse empty nested list in " + file + ".");
    }

    const auto begin = get<ItemIndex>(state_.data);
    const auto end   = begin + state_.count;
    auto idx = begin;
    ConstantSP val = mapNestedItem(
        idx, mapParser, symList, symName, file, mapFile);
    assert(!val.isNull());

    bool isArrayVector;
    switch(val->getType()) {
        case DT_SYMBOL:
        case DT_STRING:
            isArrayVector = false;
            break;
        default:
            isArrayVector = true;
    }

    VectorSP vec, tuple;
    if(isArrayVector) {
        vec = InternalUtil::createArrayVector(
                bit_cast<DATA_TYPE>(ARRAY_TYPE_BASE + val->getType()),
                0, 0, state_.count, 0);
        tuple = Util::createVector(DT_ANY, 1);
        tuple->set(0, val);
        vec->append(tuple);
    } else {
        vec = Util::createVector(DT_ANY, 0, state_.count);
        vec->append(val);
    }

    for(++idx; idx < end; ++idx) {
        val = mapNestedItem(idx, mapParser, symList, symName, file, mapFile);
        assert(!val.isNull());
        if(isArrayVector) {
            tuple->set(0, val);
            vec->append(tuple);
        } else {
            vec->append(val);
        }
    }

    assert(static_cast<size_t>(vec->size()) == state_.count);
    return vec;
}

ConstantSP kdb::Parser::mapNestedItem(const ItemIndex* idx, Parser& mapParser,
    const std::vector<string>& symList, const string& symName,
    const string& file, const string& mapFile
) {
    switch(idx->format) {
        case 0x00:
            return mapNestedItem<0x00>(
                idx->offset, mapParser, symList, symName, mapFile);
        case 0x01:
            return mapNestedItem<0x01>(
                idx->offset, mapParser, symList, symName, mapFile);
        default:
            ;//no-op
    }
    ostringstream msg;
    msg << "(0x" << hex << uppercase << setfill('0')
        << setw(2) << unsigned{idx->format} << ")";
    throw RuntimeException(PLUGIN_NAME ": "
          "Unsupported nested list item index in "
        + file + " " + msg.str() + ".");
}

template<>
ConstantSP kdb::Parser::mapNestedItem<0x00>(
    ptrdiff_t offset, Parser& mapParser,
    const std::vector<string>& symList, const string& symName,
    const string& mapFile
) {
    const auto base = mapParser.state_.data;
    const auto header = mapParser.get<ItemHeaderEx>(base + offset);

    const char* err = "Unknown kdb+ nested list# item magic";
    switch(header->format[0]) {
        case 0xFB:
            if(header->format[1] == 0x00) {
                err = nullptr;
            }
            break;
        default:
            ;//no-op
    }
    if(UNLIKELY(!!err)) {
        ostringstream msg;
        msg << err << " (0x" << hex << uppercase << setfill('0')
            << setw(2) << unsigned{header->format[0]}
            << setw(2) << unsigned{header->format[1]}
            << ")";
        throw RuntimeException(PLUGIN_NAME ": "
            + msg.str() + " in " + mapFile + ".");
    }

    const auto type = static_cast<Type>(header->type);
    const auto size = itemSize(type);
    if(UNLIKELY(size != UNKNOWN_COUNT
        && size * header->count <= ItemHeader::MAX_COUNT
    )) {
        ostringstream msg;
        msg << "0x" << hex << uppercase << setfill('0')
            << setw(8) << offset;
        throw RuntimeException(PLUGIN_NAME ": "
                "Nested list# item length underflow in "
            + mapFile + " at " + msg.str()
            + " (" + to_string(header->count)
            + "|type=" + to_string(type) + ").");
    }

    switch(type) {
        case K_CHAR:
            return new String{ string(
                reinterpret_cast<const char*>(header + 1), header->count) };
        case K_STRING:
            throw RuntimeException(PLUGIN_NAME ": "
                + mapFile + " is not an enumerated symbol list "
                  "(" + to_string(state_.type) + ").");
        default:
            if(K_ENUM_MIN <= type && type <= K_ENUM_MAX) {
                if(UNLIKELY(symName.empty())) {
                    throw RuntimeException(PLUGIN_NAME ": "
                        "Enum sym file not yet loaded.");
                }
                return mapNestedStrings<0x00>(
                    offset, mapParser, symList, symName, mapFile);
            } else {
                return toDDB::fromArray(type,
                    reinterpret_cast<byte*>(header + 1), header->count, mapFile);
            }
    }
}

template<>
ConstantSP kdb::Parser::mapNestedItem<0x01>(
    ptrdiff_t offset, Parser& mapParser,
    const std::vector<string>& symList, const string& symName,
    const string& mapFile
) {
    const auto base = mapParser.state_.data;
    const auto header = mapParser.get<ItemHeader>(base + offset);

    const auto type = static_cast<Type>(header->type - ItemHeader::TYPE_OFFSET);
    const auto size = itemSize(type);
    if(UNLIKELY(
        size != UNKNOWN_COUNT && size * header->count > ItemHeader::MAX_COUNT
    )) {
        ostringstream msg;
        msg << "0x" << hex << uppercase << setfill('0')
            << setw(8) << offset;
        throw RuntimeException(PLUGIN_NAME ": "
                "Nested list# item length overflow in "
            + mapFile + " at " + msg.str()
            + " (" + to_string(unsigned{header->count})
            + "|type=" + to_string(type) + ").");
    }

    switch(type) {
        case K_CHAR:
            return new String{ string(
                reinterpret_cast<const char*>(header + 1), header->count) };
        case K_STRING:
            throw RuntimeException(PLUGIN_NAME ": "
                + mapFile + " is not an enumerated symbol list "
                  "(" + to_string(state_.type) + ").");
        default:
            if(K_ENUM_MIN <= type && type <= K_ENUM_MAX) {
                if(UNLIKELY(symName.empty())) {
                    throw RuntimeException(PLUGIN_NAME ": "
                        "Enum sym file not yet loaded.");
                }
                return mapNestedStrings<0x01>(
                    offset, mapParser, symList, symName, mapFile);
            } else {
                return toDDB::fromArray(type,
                    reinterpret_cast<byte*>(header + 1), header->count, mapFile);
            }
    }
}

namespace /*anonymous*/ {

    string getEnumName(const string& mapFile) {
        const auto enumNamePath = mapFile + '#';
        kdb::BinFile enumNameFile{enumNamePath, enumNamePath};
        if(UNLIKELY(!enumNameFile)) {
            throw RuntimeException(PLUGIN_NAME ": "
                "Open nested symbol enum name " + enumNamePath + " failed.");
        }

        kdb::Parser enumNameParser;
        enumNameFile.readInto(enumNameParser.getBuffer());
        enumNameParser.initialize();

        const auto syms = enumNameParser.strings(enumNamePath);
        if(UNLIKELY(syms.size() != 1)) {
            throw RuntimeException(PLUGIN_NAME ": "
                "Cannot identify symbol enum name from " + enumNamePath + ".");
        }
        return syms.front();
    }

}//namspace /*anonymous*/

template<>
VectorSP kdb::Parser::mapNestedStrings<0x00>(
    ptrdiff_t offset, Parser& mapParser,
    const std::vector<string>& symList, const string& symName,
    const string& mapFile
) {
    const auto base = mapParser.state_.data;
    const auto header = mapParser.get<SymItemHeaderEx>(base + offset);

    const char* err = "Unknown kdb+ nested list# symbol magic";
    switch(header->format[0]) {
        case 0xFB:
            if(header->format[1] == 0x00) {
                err = nullptr;
            }
            break;
        default:
            ;//no-op
    }
    if(UNLIKELY(!!err)) {
        ostringstream msg;
        msg << err << " (0x" << hex << uppercase << setfill('0')
            << setw(2) << unsigned{header->format[0]}
            << setw(2) << unsigned{header->format[1]}
            << ")";
        throw RuntimeException(PLUGIN_NAME ": "
            + msg.str() + " in " + mapFile + ".");
    }

    const auto type = static_cast<Type>(header->type);
    assert(K_ENUM_MIN <= type && type <= K_ENUM_MAX);
    const auto size = itemSize(type);
    if(UNLIKELY(size != UNKNOWN_COUNT
        && size * header->count <= SymItemHeader::MAX_COUNT
    )) {
        ostringstream msg;
        msg << "0x" << hex << uppercase << setfill('0')
            << setw(8) << offset;
        throw RuntimeException(PLUGIN_NAME ": "
                "Nested list# symbol length underflow in "
            + mapFile + " at " + msg.str()
            + " (" + to_string(header->count)
            + "|type=" + to_string(type) + ").");
    }

    const auto enumName = getEnumName(mapFile);
    if(UNLIKELY(enumName != symName)) {
        throw RuntimeException(PLUGIN_NAME ": "
            + mapFile + " was not enumerated on " + symName + ".");
    }

    const auto begin = reinterpret_cast<J*>(header + 1);
    const auto end   = begin + header->count;
    return toDDB::mapStrings(begin, end, symList, symName, mapFile);
}

template<>
VectorSP kdb::Parser::mapNestedStrings<0x01>(
    ptrdiff_t offset, Parser& mapParser,
    const std::vector<string>& symList, const string& symName,
    const string& mapFile
) {
    const auto base = mapParser.state_.data;
    const auto header = mapParser.get<SymItemHeader>(base + offset);

    const auto type = static_cast<Type>(header->type - ItemHeader::TYPE_OFFSET);
    assert(K_ENUM_MIN <= type && type <= K_ENUM_MAX);
    const auto size = itemSize(type);
    if(UNLIKELY(size * header->count > ItemHeader::MAX_COUNT)) {
        ostringstream msg;
        msg << "0x" << hex << uppercase << setfill('0')
            << setw(8) << offset;
        throw RuntimeException(PLUGIN_NAME ": "
                "Nested symbol list# length overflow in "
            + mapFile + " at " + msg.str()
            + " (" + to_string(unsigned{header->count})
            + "|type=" + to_string(type) + ").");
    }

    const auto enumName = getEnumName(mapFile);
    if(UNLIKELY(enumName != symName)) {
        throw RuntimeException(PLUGIN_NAME ": "
            + mapFile + " was not enumerated on " + symName + ".");
    }

    const auto begin = reinterpret_cast<J*>(header + 1);
    const auto end   = begin + header->count;
    return toDDB::mapStrings(begin, end, symList, symName, mapFile);
}

//////////////////////////////////////////////////////////////////////////////