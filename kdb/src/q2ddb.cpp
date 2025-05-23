#include "q2ddb.h"

#include <cassert>
#include <tuple>
#include <sstream>
#include <iomanip>

#include "CoreConcept.h"
#include "Types.h"
#include "Util.h"
#include "ScalarImp.h"
#include "SpecialConstant.h"
#include "Logger.h"
#include "ddbplugin/PluginLogger.h"
#include "ddbplugin/PluginLoggerImp.h"

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

bool kdb::isEnumType(Type type) noexcept {
    return K_ENUM_MIN <= type && type <= K_ENUM_MAX;
}
bool kdb::isNestedType(Type type) noexcept {
    return K_NESTED_MIN <= type && type <= K_NESTED_MAX;
}

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
            if(isEnumType(type)) {
                return sizeof(J);
            } else
            if(isNestedType(type)) {
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

// bool kdb::isInf(const H& h) noexcept { return abs(h) == (wh); }
bool kdb::isInf(const I& i) noexcept { return abs(i) == (wi); }
bool kdb::isInf(const J& j) noexcept { return abs(j) == (wj); }
// bool kdb::isInf(const E& e) noexcept { return abs(e) == static_cast<float>((wf)); }
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
    PLUGIN_LOG(PLUGIN_NAME ": "
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

ConstantSP kdb::toDDB::fromK(K pk, const string& var, bool allowAny) {
    assert(pk);
    const auto type  = static_cast<Type>(pk->t);
    if (type < 0 && type != K_ERROR) {
        return fromScalar(type, pk, var);
    }
    switch(type) {
        case K_LIST: {
            auto ret = list(kK(pk), kK(pk) + pk->n, var, allowAny);
            ret->setNullFlag(ret->hasNull());
            return ret;
        }
        case K_DICT:
            return fromDict(type, kK(pk), var);
        case K_TABLE:
            return fromTable(type, kK(pk->k), var);
        case K_UNARY_PRIMITIVE:
            return new Void();
        default:
            auto ret = fromArray(type, kG(pk), pk->n, var);
            ret->setNullFlag(ret->hasNull());
            return ret;
    }
}

ConstantSP merge2Table(TableSP left, TableSP right) {
    vector<string> colNames;
    vector<ConstantSP> cols;
    for (int i = 0; i < left->columns(); ++i) {
        colNames.push_back(left->getColumnName(i));
        cols.push_back(left->getColumn(i));
    }
    for (int i = 0; i < right->columns(); ++i) {
        colNames.push_back(right->getColumnName(i));
        cols.push_back(right->getColumn(i));
    }
    return Util::createTable(colNames, cols);
}

ConstantSP kdb::toDDB::fromDict(
    Type type, K* data, const string& var
) {
    K keys = data[0];
    K values = data[1];
    const auto keyType  = static_cast<Type>(keys->t);
    const auto valueType  = static_cast<Type>(values->t);

    ConstantSP ddbKey = fromK(keys, "key of " + var, true);
    ConstantSP ddbValue = fromK(values, "value of " + var, true);

    DATA_TYPE keyDdbType = KDB_DDB_TYPE_MAP[keyType];
    DATA_TYPE valueDdbType = KDB_DDB_TYPE_MAP[valueType];

    // check for unsupported type
    if (keyDdbType == DT_ANY || keyDdbType == 0 || ddbKey->size() != ddbValue->size()) {
        if (ddbKey->getForm() == DF_TABLE && ddbValue->getForm() == DF_TABLE) {
            return merge2Table(ddbKey, ddbValue);
        }
        VectorSP anyVec = Util::createVector(DT_ANY, 2);
        anyVec->set(0, ddbKey);
        anyVec->set(1, ddbValue);
        return anyVec;
    }
    DictionarySP dict = Util::createDictionary(keyDdbType, NULL, valueDdbType, NULL);
    if (dict.isNull()) {
        throw RuntimeException(PLUGIN_NAME "failed to createDictionary for keyType " + std::to_string(keyDdbType));
    }
    for (int i = 0; i < ddbKey->size(); ++i) {
        dict->set(ddbKey->get(i), ddbValue->get(i));
    }
    return dict;
}

ConstantSP kdb::toDDB::fromTable(
    Type type, K* data, const string& var
) {
    VectorSP colNamesVec = fromK(data[0], "colNames of " + var);
    vector<string> colNames;
    // HACK force to getString of colNames
    for (int i = 0; i < colNamesVec->size(); ++i) {
        colNames.emplace_back(colNamesVec->getString(i));
    }

    vector<ConstantSP> cols;
    for (int i = 0 ; i < data[1]->n; ++i) {
        cols.push_back(fromK(kK(data[1])[i], "cols of " + var));
        if (cols.back()->getType() == DT_ANY && cols.back()->size() == 0) {
            ((AnyVector*)cols.back().get())->setTableColumn(true);
            ((AnyVector*)cols.back().get())->setExtraParamForType(DT_INT);
        }
    }
    return Util::createTable(colNames, cols);
}

ConstantSP kdb::toDDB::fromScalar(
    Type type, K data, const string& var
) {
    switch(-type) {
        case K_BOOL: return new Bool(data->g);
        case K_GUID: return new Uuid(kdb::byteswap<U>(*kU(data)).g);
        case K_BYTE: return new Char(data->g);
        case K_SHORT: return new Short(data->h);
        case K_INT: return new Int(data->i);
        case K_LONG: return new Long(data->j);
        case K_FLOAT:
            if (isNull(data->e)) {
                return new Float(DDB_FLOAT_NULL);
            }
            return new Float(data->e);
        case K_DOUBLE:
            if (isNull(data->f)) {
                return new Double(DDB_DOUBLE_NULL);
            }
            return new Double(data->f);
        case K_CHAR: return new Char(data->i);
        case K_STRING: return new String(data->s);
        case K_TIMESTAMP:
            if (isNull(data->j) || isInf(data->j)) {
                return new NanoTimestamp(DDB_LONG_NULL);
            }
            return new NanoTimestamp(data->j + KDB_NANOTIMESTAMP_GAP);
        case K_MONTH:
            if (isNull(data->i) || isInf(data->i)) {
                return new Month(DDB_INT_NULL);
            }
            return new Month(data->i + KDB_MONTH_GAP);
        case K_DATE:
            if (isNull(data->i) || isInf(data->i)) {
                return new Date(DDB_INT_NULL);
            }
            return new Date(data->i + KDB_DATE_GAP);
        case K_DATETIME:
            return new Timestamp(isNull(data->f) || isInf(data->f) ? DDB_LONG_NULL
            : static_cast<long long>(data->f * (SEC_PER_DAY * 1000LL) + KDB_DATETIME_GAP));
        case K_TIMESPAN:
            return new NanoTime(isNull(data->j) || isInf(data->j) ? DDB_LONG_NULL : data->j);
        case K_MINUTE:
            return new Minute(isNull(data->i) || isInf(data->i) ? DDB_INT_NULL : data->i);
        case K_SECOND:
            return new Second(isNull(data->i) || isInf(data->i) ? DDB_INT_NULL : data->i);
        case K_TIME:
            return new Time(isNull(data->i) || isInf(data->i) ? DDB_INT_NULL : data->i);
        default:
            throw RuntimeException(PLUGIN_NAME ": "
                "kdb+ object " + var + " (" + to_string(type) + ") "
                "not yet supported.");
    }
}

VectorSP kdb::toDDB::fromArray(
    Type type, byte* data, size_t count, const string& var
) {
    if(count == UNKNOWN_SIZE) {
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

template<typename T>
void checkValid(T* begin, T* end, const string &typeName, const string &var) {
    if(!(begin && begin <= end)) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Invalid kdb+ " + typeName + " for " + var + ".");
    }
}
VectorSP kdb::toDDB::list(K* begin, K* end, const string& var, bool allowAny) {
    checkValid(begin, end, "object", var);
    const auto count = end - begin;
    if(count == 0) {
        return Util::createVector(DT_ANY, 0);
    }

    K* item = begin;
    if(!*item) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Invalid kdb+ list item at first(" + var + ").");
    }

    if((*item)->t == K_CHAR) {
        return charsList(begin, end, var);
    } else {
        return nestedList(static_cast<Type>((*item)->t), begin, end, var, allowAny);
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
    checkValid(begin, end, "GUID vector", var);

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
    checkValid(begin, end, "float vector", var);
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
    checkValid(begin, end, "double vector", var);
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
    checkValid(begin, end, "timestamp vector", var);

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
    checkValid(begin, end, "month vector", var);

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
    checkValid(begin, end, "date vector", var);

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
    checkValid(begin, end, "datetime vector", var);
    const auto count = end - begin;

    // DolphinDB doesn't have +/-inf datetimes, treat them as nulls.
    using kValueT = long long;
    auto timestamps = make_unique<kValueT[]>(count);
    transform(begin, end, timestamps.get(), [](const F& z){
        return isNull(z) || isInf(z)
            ? DDB_LONG_NULL
            : static_cast<kValueT>(z * (SEC_PER_DAY * 1000LL) + KDB_DATETIME_GAP);
    });

    using ddbPtr = const long long*;
    return makeVector<kValueT, DT_TIMESTAMP, ddbPtr>(
        timestamps.get(), timestamps.get() + count,
        &Vector::appendLong, var);
}

VectorSP kdb::toDDB::timespans(J* begin, J* end, const string& var) {
    checkValid(begin, end, "timespans vector", var);

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
    checkValid(begin, end, "minutes vector", var);

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
    checkValid(begin, end, "seconds vector", var);

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
    checkValid(begin, end, "times vector", var);

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
    static_assert(
        sizeof(kValueT) == sizeof(typename remove_pointer<ddbPtr>::type),
        "kdb+ vs DolphinDB type equivalence"
    );
    if(!((begin && begin <= end) || (!begin && begin == end))) {
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
        if(!isValidListOf(*item, K_CHAR)) {
            // HACK ,if scalar,use fromK
            if ((*item)->t == -1*K_CHAR) {
                string str = fromK(*item, var + "[" + to_string(item - begin) + "].")->getString();
                charArrays.push_back(str);
                continue;
            }
            throw RuntimeException(PLUGIN_NAME ": Not a valid kdb+ string at "
                + var + "[" + to_string(item - begin) + "].");
        }
        charArrays.emplace_back(string{kC(*item), kC(*item) + (*item)->n});
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

    auto syms = make_unique<S[]>(count);
    for(auto idx = begin; idx < end; ++idx) {
        S s = nullptr;
        if(isNull(*idx)) {
            s = sym(nullptr);
        } else
        if(0 <= *idx && static_cast<size_t>(*idx) < symList.size()) {
            s = sym(symList[*idx]);
        } else {
            PLUGIN_LOG(PLUGIN_NAME ": " + var + " - "
                "sym[" + to_string(*idx) + "] not in " + symName + ".");
            s = sym(nullptr);
        }
        assert(s);
        syms[idx - begin] = s;
    }

    return strings(syms.get(), syms.get() + count, var);
}

VectorSP kdb::toDDB::nestedList(Type type,
    K* begin, K* end, const string& var, bool allowAny
) {
    assert(begin && begin <= end);
    const auto count = end - begin;
    if(count == 0) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Cannot convert empty nested list " + var + ".");
    }

    // Convert each nested list
    vector<VectorSP> list;
    list.reserve(count);
    transform(begin, end, back_inserter(list), [&var, begin](const K &item) {
        return fromK(item, var + '[' + to_string(&item - begin) + ']');
    });
    assert(list.size() == static_cast<size_t>(count));

    // Detect and check if they are all of homogeneous type
    DATA_TYPE dbType = DT_ANY;
    DATA_CATEGORY dbCategory = NOTHING;
    DATA_FORM dbForm = MAX_DATA_FORMS;
    for(auto i = 0u; i < list.size(); ++i) {
        auto const& item = list[i];
        assert(!item.isNull());
        const auto itemType = item->getType();
        const auto itemCategory = item->getCategory();
        const auto itemForm = item->getForm();

        if (i == 0) {
            dbType = itemType;
            dbCategory = itemCategory;
            dbForm = itemForm;
        }
        if (itemForm > DF_VECTOR) {
            if (allowAny) {
                VectorSP ret = Util::createVector(DT_ANY, list.size());
                for (int i = 0 ; i <int(list.size()); ++i) {
                    ret->set(i, list[i]);
                }
                return ret;
            }
            throw RuntimeException(PLUGIN_NAME ": "
                "Mixed data in a kdb+ list of type " + to_string(type) + " "
                "at " + var + "[" + to_string(i) + "] ("
                "expected=" + to_string(dbType) + " "
                "actual=" + to_string(itemType) + ").");
        }

        if(itemType != dbType) {
            if (itemCategory == dbCategory && itemCategory == LITERAL) {
                continue;
            }
            if (itemType == DT_ANY && item->size() == 0) {
                continue;
            }
            if (allowAny) {
                VectorSP ret = Util::createVector(DT_ANY, list.size());
                for (int i = 0 ; i <int(list.size()); ++i) {
                    ret->set(i, list[i]);
                }
                return ret;
            }
            throw RuntimeException(PLUGIN_NAME ": "
                "Mixed data in a kdb+ list of type " + to_string(type) + " "
                "at " + var + "[" + to_string(i) + "] ("
                "expected=" + to_string(dbType) + " "
                "actual=" + to_string(itemType) + ").");
        }
    }

    if (dbForm !=DF_SCALAR && dbForm != DF_VECTOR) {
        VectorSP ret = Util::createVector(DT_ANY, list.size());
        for (int i = 0 ; i <int(list.size()); ++i) {
            ret->set(i, list[i]);
        }
        return ret;
    }

    // Construct nested list for DolphinDB
    VectorSP vec;
    switch(dbType) {
        case DT_ANY:
            vec = Util::createVector(DT_ANY, count);
            fakeEmptyAnyColumn(vec.get(), "*", var);
            for(auto i = 1u; i < list.size(); ++i) {
                auto item = list[i];
                // assert(item->getType() == DT_ANY && !item->size());
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

namespace {
    struct assert_failure {
        template<typename Fun>
        explicit assert_failure(Fun fun) { fun(); }
    };
}

//============================================================================
/**
   00   01   02  03   04   05   06   07
+----+----+----+----+----+----+----+----+
|    item offset    | ?? | ?? | ?? |form|
+----+----+----+----+----+----+----+----+
|               enum index              |
+----+----+----+----+----+----+----+----+
 *> FIXME: only "format" of value 0x00 or 0x01 is recongized for now.
 *> A null symbol will have "index" value equal to 0Wj in q.
 */

constexpr int64_t ITEM_INDEX_MASK = 0x00FFFFFFFFFFFFFF;

struct kdb::Parser::ItemIndex {
    union {
        struct {        // For nested data map
            byte    pad_xxx[7];
            byte    format;
        };
        int64_t index;  // For enumerated symbol
    };
    constexpr int64_t getOffset() const noexcept {
        return index & ITEM_INDEX_MASK;
    }
};
static_assert(
    sizeof(kdb::Parser::ItemIndex) == 4+3+1,
    "kdb+ file - mapped list item index"
);

//============================================================================
/**
  00   01   02   03   04  05  06  07  08~~
+----+----+----+----+---+---+---+---+--~~~~~~~~~~~~~~~~~~~
| FF | 01 |type|attr|   item count  | 0-teminated symbols
+----+----+----+----+---+---+---+---+--~~~~~~~~~~~~~~~~~~~
 *> FIXME: In enum sym file, "item count" may be inaccurate sometimes!
 */
struct kdb::Parser::BaseHeader : HeaderTag<0xFF, 0x01, BaseHeader> {
    //{
    byte      tag[2];
    Type      type;
    Attribute attr;
    int32_t   ref_count;
    //}

    using HeaderTag::isValid;

    constexpr bool isValidOf(Type type) const noexcept {
        return isValid() && this->type == type;
    }
};
static_assert(
    sizeof(kdb::Parser::BaseHeader) == 2+1+1+4,
    "kdb+ file 0xFF01 header - unenumerated syms list"
);

struct kdb::Parser::SymsList : DataBlock<BaseHeader, char> {
    size_t getCount() const noexcept {
        assert(header.ref_count >= 0);
        return static_cast<size_t>(header.ref_count);
    }

    vector<string> getSyms(const byte* end) const {
        if(!header.isValidOf(K_STRING)) {
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
            if(eos >= eod) {
                throw RuntimeException(PLUGIN_NAME ": "
                    "Incomplete or truncated syms list.");
            }
            syms.emplace_back(string{next, eos});
            next = eos + 1;
        }
        if(next < eod) {
            PLUGIN_LOG(PLUGIN_NAME ": "
                "Found syms beyond designated count=" + to_string(count) + ".");
            while(next < eod) {
                const auto eos = find(next, eod, '\0');
                syms.emplace_back(string{next, eos});
                next = eos + 1;
            }
        }

        if(syms.size() > count) {
            PLUGIN_LOG(PLUGIN_NAME ": "
                "Actual syms extracted=" + to_string(syms.size()) + ".");
        }
        return syms;
    }
};

//============================================================================
/**
  00   01   02   03  04 05 06 07 08 09 0A 0B 0C 0D 0D 0F  10~~
+----+----+----+----+--+--+--+--+--+--+--+--+--+--+--+--+--~~~~~~~~~~
| FE | 20 |type|attr| ref. count|       item count      | data items
+----+----+----+----+--+--+--+--+--+--+--+--+--+--+--+--+--~~~~~~~~~~
 */
struct kdb::Parser::BaseListHeader
    :  BaseHeader, HeaderTag<0xFE, 0x20, BaseListHeader>
{
    //{
    int64_t count;
    //}

    constexpr bool isValid() const noexcept {
        return HeaderTag<0xFE, 0x20, BaseListHeader>::isValid()
            && K_LIST <= type && type <= K_NESTED_MAX;
    }
};
static_assert(
    sizeof(kdb::Parser::BaseListHeader) == 8+8,
    "kdb+ file 0xFE20 header - simple list"
);

struct kdb::Parser::BaseList : DataBlock<BaseListHeader, byte> {
    using DataBlock::isComplete;

    size_t getCount() const noexcept {
        assert(header.count >= 0);
        return static_cast<size_t>(header.count);
    }
};

//============================================================================
/**
  00   01   02   03  04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  0010~~0FEF           0FF0~~
+----+----+----+----+--+--+--+--+--+--+--+--+--+--+--+--+--~~~~~~~~~~~~~~~~--+--~~~~~~~~~~~
| FD | 20 |type|attr| ref. count|00|00|00|00|00|00|00|00| 00..00 / enum name | ext. header
+----+----+----+----+--+--+--+--+--+--+--+--+--+--+--+--+--~~~~~~~~~~~~~~~~--+--~~~~~~~~~~~
 */
struct kdb::Parser::ExtListHeader
    :  BaseListHeader, HeaderTag<0xFD, 0x20, ExtListHeader>
{
    using HeaderTag<0xFD, 0x20, ExtListHeader>::isValid;
};
static_assert(
    sizeof(kdb::Parser::ExtListHeader) == 8+8,
    "kdb+ file 0xFD20 primary header - extended list"
);

struct kdb::Parser::ExtList {
    static constexpr ptrdiff_t EXT_DATA_SEGMENT = 0x1000;

    struct {
        ExtListHeader header;
        char enum_name[EXT_DATA_SEGMENT
            - (sizeof(ExtListHeader) + sizeof(BaseListHeader))];
        BaseListHeader payload;
    };

    bool isComplete(const byte* end) const noexcept {
        return end - reinterpret_cast<const byte*>(this) >= EXT_DATA_SEGMENT;
    }

    template<typename T>
    constexpr const T* get() const noexcept {
        return reinterpret_cast<const T*>(&payload);
    }
};
static_assert(
    sizeof(kdb::Parser::ExtList) == kdb::Parser::ExtList::EXT_DATA_SEGMENT,
    "kdb+ file 0xFD20 data block - extendded list & secondary header"
);

//============================================================================
/**
  00   01   02   03  04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  10~~
+----+----+----+----+--+--+--+--+--+--+--+--+--+--+--+--+--~~~~~~~~~~
| FD | 00 |type|attr| ref. count|       item count      | data items
+----+----+----+----+--+--+--+--+--+--+--+--+--+--+--+--+--~~~~~~~~~~
 *> This is an extension header following kdb::Parser::ExtListHeader.
 *> kdb::K_LIST  <= "type" < kdb::K_ENUM_MIN
 */
struct kdb::Parser::SimpleListHeader
    :  BaseListHeader, HeaderTag<0xFD, 0x00, SimpleListHeader>
{
    constexpr bool isValid() const noexcept {
        return HeaderTag<0xFD, 0x00, SimpleListHeader>::isValid()
            && K_LIST <= type && type < K_ENUM_MIN;
    }
};
static_assert(
    sizeof(kdb::Parser::SimpleListHeader) == 8+8,
    "kdb+ file 0xFD00 secondary header - simple list"
);

struct kdb::Parser::SimpleList : DataBlock<SimpleListHeader, byte> {
    bool isComplete(const byte* end) const {
        return DataBlock::isComplete(getSize(header.type), getCount(), end);
    }

    size_t getCount() const noexcept {
        assert(header.count >= 0);
        return static_cast<size_t>(header.count);
    }

    VectorSP getVector() const {
        if(!header.isValid()) {
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
/**
  00   01   02   03  04 05 06 07 08 09 0A 0B 0C 0C 0D 0E  0F~~
+----+----+----+----+--+--+--+--+--+--+--+--+--+--+--+--+--~~~~~~~~~~~~
| FD | 00 |type|attr| ref. count|       item count      | item indices
+----+----+----+----+--+--+--+--+--+--+--+--+--+--+--+--+--~~~~~~~~~~~~
 *> This is an extension header following kdb::Parser::ExtListHeader.
 *> The "item indices" are of type kdb::Parser::ItemIndex.
 *> kdb::K_ENUM_MIN  <= "type" <= kdb::K_ENUM_MAX
 */
struct kdb::Parser::EnumSymsHeader
    :  BaseListHeader, HeaderTag<0xFD, 0x00, EnumSymsHeader>
{
    constexpr bool isValid() const noexcept {
        return HeaderTag<0xFD, 0x00, EnumSymsHeader>::isValid()
            && isEnumType(type);
    }
};
static_assert(
    sizeof(kdb::Parser::EnumSymsHeader) == 8+8,
    "kdb+ file 0xFD00 secondary header - enumerated syms list"
);

struct kdb::Parser::EnumSymsList : DataBlock<EnumSymsHeader, ItemIndex> {
    bool isComplete(const byte* end) const {
        return DataBlock::isComplete(getCount(), end);
    }

    size_t getCount() const noexcept {
        assert(header.count >= 0);
        return static_cast<size_t>(header.count);
    }

    vector<S> getSyms(const byte* end,
        const vector<string>& symList, const string& symName
    ) const {
        if(!header.isValid()) {
            throw RuntimeException(PLUGIN_NAME ": "
                "kdb+ extended file - not a valid enumerated syms list.");
        }

        auto count = getCount();
        const auto begin = get();
        //FIXME: Deal with some compressed enum sym vector with wrong count...
        const auto length = end - reinterpret_cast<const byte*>(begin);
        if(length % sizeof(data_t) == 0) {
            const auto real_count = length / sizeof(data_t);
            if(real_count > 0 && count == 0) {
                PLUGIN_LOG(PLUGIN_NAME ": "
                    "Incorrect enum sym count found "
                    "(count=" + to_string(count) + ","
                    " expected=" + to_string(real_count) + ").");
                count = real_count;
            }
        }

        vector<S> syms;
        syms.reserve(count);
        transform(begin, begin + count, back_inserter(syms),
            [begin, &symList, &symName](const ItemIndex& idx) {
                if(isNull(bit_cast<J>(idx.index))) {
                    return sym(nullptr);
                } else
                if(0 <= idx.index
                    && static_cast<size_t>(idx.index) < symList.size()
                ) {
                    return sym(symList[idx.index]);
                } else {
                    PLUGIN_LOG(PLUGIN_NAME ": Enumerated sym out of bounds "
                        + symName + "[" + to_string(idx.index) + "].");
                    return sym(nullptr);
                }
            }
        );
        return syms;
    }
};

//============================================================================
/**
  00   01   02   03  04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  10~~
+----+----+----+----+--+--+--+--+--+--+--+--+--+--+--+--+--~~~~~~~~~~~~
| FD | 01 |type| 00 |   00..00  |       item count      | item indices
+----+----+----+----+--+--+--+--+--+--+--+--+--+--+--+--+--~~~~~~~~~~~~
| FB | 00 |type|attr| ref. count|       item count      | item indices
+----+----+----+----+--+--+--+--+--+--+--+--+--+--+--+--+--~~~~~~~~~~~~
 *> This is an extension header following kdb::Parser::ExtListHeader.
 *> kdb::K_NESTED_MIN <= "type" <= kdb::K_NESTED_MAX.
 *> The "item indices" are of type kdb::Parser::ItemIndex.
 */
template<kdb::byte Tag0, kdb::byte Tag1, int32_t MinRef>
struct kdb::Parser::NestedListHeader
    : BaseListHeader, HeaderTag<Tag0, Tag1, NestedListHeader<Tag0, Tag1, MinRef>>
{
    constexpr bool isValid() const noexcept {
        return HeaderTag<Tag0, Tag1, NestedListHeader>::isValid()
            && isNestedType(type) && ref_count >= MinRef;
    }
};
static_assert(
    sizeof(kdb::Parser::NestedListHeader<0xFD, 0x01>) == 8+8,
    "kdb+ file 0xFD01/0xFB00 secondary/ternary header - nested list"
);

template<kdb::byte Tag0, kdb::byte Tag1, int32_t MinRef>
struct kdb::Parser::NestedList
    : DataBlock<NestedListHeader<Tag0, Tag1, MinRef>, ItemIndex>
{
    using base_type =
        DataBlock<NestedListHeader<Tag0, Tag1, MinRef>, ItemIndex>;

    bool isComplete(const byte* end) const {
        return base_type::isComplete(getCount(), end);
    }

    size_t getCount() const noexcept {
        assert(base_type::header.count >= 0);
        return static_cast<size_t>(base_type::header.count);
    }

    size_t guessCount(const byte* end) const {
        // Files nested more than 2 levels deep may not store the actual count!
        const auto bytes =
            end - reinterpret_cast<const byte*>(base_type::get());
        assert(bytes >= 0);
        if(bytes % sizeof(ItemIndex)) {
            throw RuntimeException(PLUGIN_NAME ": "
                "Truncated or incomplete nested list in kdb+ extended file.");
        }

        return bytes / sizeof(ItemIndex);
    }

    tuple<const ItemIndex*, const ItemIndex*> getIndices() const {
        if(!base_type::header.isValid()) {
            throw RuntimeException(PLUGIN_NAME ": "
                "kdb+ extended file - not a valid nested list.");
        }

        const auto begin = base_type::get();
        return make_tuple(begin, begin + getCount());
    }
};

//============================================================================
/**
   00    01   02~~
+-----+-----+--~~~~~~~~~~
|type0|count| data items
+-----+-----+--~~~~~~~~~~
 *> kdb::K_LIST + 0x7F <= "type0" < kdb::K_ENUM_MIN + 0x7F.
 *> Total size of "data items" must be <= 0x5F bytes.
 */
struct kdb::Parser::NestedItemHeader {
    //{
    Type   type_o;
    int8_t count;
    //}

    static constexpr int8_t TYPE_OFFSET = 0x7F;
    static constexpr int8_t MAX_LENGTH  = 0x5F;

    constexpr Type getType() const noexcept {
        return static_cast<Type>(type_o - TYPE_OFFSET);
    }

    constexpr bool isValid() const noexcept {
        return K_LIST <= getType() && getType() < K_ENUM_MIN;
    }
};
static_assert(
    sizeof(kdb::Parser::NestedItemHeader) == 1+1,
    "kdb+ file 0x7F?? ternary header - simple nested item"
);

struct kdb::Parser::NestedItem : DataBlock<NestedItemHeader, byte> {
    bool isComplete(const byte* end) const {
        const auto itemSize = getSize(header.getType());
        if(itemSize == UNKNOWN_SIZE) {
            throw RuntimeException(PLUGIN_NAME ": "
                  "Cannot handle the simple nested item "
                  "(type=" + to_string(header.type_o) + "->"
                + to_string(header.getType()) + ").");
        } else
        if(itemSize * getCount() > header_t::MAX_LENGTH) {
            throw RuntimeException(PLUGIN_NAME ": "
                  "Simple nested item overflow "
                  "(type=" + to_string(header.type_o) + "->"
                + to_string(header.getType())
                + "|count=" + to_string(getCount()) + ").");
        }

        return DataBlock::isComplete(itemSize, getCount(), end);
    }

    size_t getCount() const noexcept {
        assert(header.count >= 0);
        return static_cast<size_t>(header.count);
    }
};

//============================================================================
/**
  00   01   02   03  04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  10~~
+----+----+----+----+--+--+--+--+--+--+--+--+--+--+--+--+--~~~~~~~~~~~~
| FB | 00 |type|attr| ref. count|       item count      | data items
+----+----+----+----+--+--+--+--+--+--+--+--+--+--+--+--+--~~~~~~~~~~~~
 *> kdb::K_LIST <= "type" < kdb::K_ENUM_MIN.
 *> "ref. count" >= 1.
 *> Total size of data time >= 0x60 bytes.
 */
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
static_assert(
    sizeof(kdb::Parser::NestedItemExHeader) == 8+8,
    "kdb+ file 0xFB00 ternary header - extended nested item"
);

struct kdb::Parser::NestedItemEx : DataBlock<NestedItemExHeader, byte> {
    bool isComplete(const byte* end) const {
        const auto itemSize = getSize(header.getType());
        if(itemSize == UNKNOWN_SIZE) {
            throw RuntimeException(PLUGIN_NAME ": "
                  "Cannot handle the extended nested item "
                  "(type=" + to_string(header.getType()) + ").");
        } else
        if(itemSize * getCount() < header_t::MIN_LENGTH) {
            throw RuntimeException(PLUGIN_NAME ": "
                  "Simple nested item overflow "
                  "(type=" + to_string(header.getType())
                + "|count=" + to_string(getCount()) + ").");
        }

        return DataBlock::isComplete(itemSize, getCount(), end);
    }

    size_t getCount() const noexcept {
        assert(header.count >= 0);
        return static_cast<size_t>(header.count);
    }
};

//============================================================================
/**
   00    01   02~~
+-----+-----+--~~~~~~~~~~~~
|type0|count| item indices
+-----+-----+--~~~~~~~~~~~~
 *> kdb::ENUM_MIN + 0x7F <= "type0" <= kdb::ENUM_MAX + 0x7F.
 *> The "item indices" are of type kdb::Parser::ItemIndex.
 *> Total size of "item indices" must be <= 0x5F bytes ("count" <= 11).
 */
struct kdb::Parser::NestedEnumSymsHeader : kdb::Parser::NestedItemHeader {
    //{
    byte   pad_xxx[3];
    int8_t count_syms;
    //}

    using NestedItemHeader::TYPE_OFFSET;
    using NestedItemHeader::MAX_LENGTH;

    using NestedItemHeader::getType;

    constexpr bool isValid() const noexcept {
        return K_ENUM_MIN <= getType() && getType() <= K_ENUM_MAX;
    }
};
static_assert(
    sizeof(kdb::Parser::NestedEnumSymsHeader) == 1+4+1,
    "kdb+ file 0x93?? ternary header - simple nested enum syms"
);

struct kdb::Parser::NestedEnumSyms : DataBlock<NestedEnumSymsHeader, ItemIndex> {
    bool isComplete(const byte* end) const {
        if(sizeof(data_t) * getCount() > header_t::MAX_LENGTH) {
            throw RuntimeException(PLUGIN_NAME ": "
                  "Simple nested enum syms overflow "
                  "(type=" + to_string(header.type_o) + "->"
                + to_string(header.getType())
                + "|count=" + to_string(getCount()) + ").");
        }

        return DataBlock::isComplete(getCount(), end);
    }

    size_t getCount() const noexcept {
        assert(header.count_syms >= 0);
        return static_cast<size_t>(header.count_syms);
    }
};

//============================================================================
/**
  00   01   02   03  04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  10~~
+----+----+----+----+--+--+--+--+--+--+--+--+--+--+--+--+--~~~~~~~~~~~~
| FB | 00 |type|attr| ref. count|       item count      | item indices
+----+----+----+----+--+--+--+--+--+--+--+--+--+--+--+--+--~~~~~~~~~~~~
 *> kdb::ENUM_MIN <= "type" <= kdb::ENUM_MAX.
 *> "ref. count" >= 1.
 *> The "item indices" are of type kdb::Parser::ItemIndex.
 *> Total size of "item indices" must be > 0x60 bytes ("item count" >= 12).
 */
struct kdb::Parser::NestedEnumSymsExHeader
    : BaseListHeader, HeaderTag<0xFB, 0x00, NestedEnumSymsExHeader>
{
    static constexpr int8_t MIN_LENGTH = NestedEnumSymsHeader::MAX_LENGTH + 1;

    constexpr Type getType() const noexcept {
        return type;
    }

    constexpr bool isValid() const noexcept {
        return HeaderTag<0xFB, 0x00, NestedEnumSymsExHeader>::isValid()
            && isEnumType(getType()) && ref_count >= 1;
    }
};
static_assert(
    sizeof(kdb::Parser::NestedItemExHeader) == 8+8,
    "kdb+ file 0xFB00 ternary header - extended nested enum syms"
);

struct kdb::Parser::NestedEnumSymsEx
    : DataBlock<NestedEnumSymsExHeader, ItemIndex>
{
    bool isComplete(const byte* end) const {
        if(sizeof(data_t) * getCount() < header_t::MIN_LENGTH) {
            throw RuntimeException(PLUGIN_NAME ": "
                  "Simple nested enum syms overflow "
                  "(type=" + to_string(header.getType())
                + "|count=" + to_string(getCount()) + ").");
            return false;
        }

        return DataBlock::isComplete(getCount(), end);
    }

    size_t getCount() const noexcept {
        assert(header.count >= 0);
        return static_cast<size_t>(header.count);
    }
};

//============================================================================
/**
  00   01   02   03  04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  10~~
+----+----+----+----+--+--+--+--+--+--+--+--+--+--+--+--+--~~~~~~~~~~
| FD | 00 | 04 | 00 | ref. count|         00..00        | data items
+----+----+----+----+--+--+--+--+--+--+--+--+--+--+--+--+--~~~~~~~~~~
 */
struct kdb::Parser::NestedMapHeader
    :  BaseListHeader, HeaderTag<0xFD, 0x00, NestedMapHeader>
{
    constexpr bool isValid() const noexcept {
        return HeaderTag<0xFD, 0x00, NestedMapHeader>::isValid()
            && type == K_BYTE;
    }
};
static_assert(
    sizeof(kdb::Parser::NestedMapHeader) == 8+8,
    "kdb+ file 0xFD00 secondary header - nested data map"
);

struct kdb::Parser::NestedMap : DataBlock<NestedMapHeader, byte> {
    bool isComplete(const byte* end) const {
        return DataBlock::isComplete(0, end);
    }

    static string stringize(const ItemIndex& idx) {
        ostringstream msg;
        msg << '<' <<"0x"
            << hex << uppercase << setfill('0') << setw(16) << idx.getOffset()
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
    if(!item->isComplete(end)) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Truncated nested item " + stringize(idx) + " "
            "(type=" + to_string(type) + ").");
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
    if(!item->isComplete(end)) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Truncated simple nested enum syms " + stringize(idx) + " "
            "(type=" + to_string(item->header.getType()) + ").");
    }

    static_assert(
        sizeof(J) == sizeof(NestedEnumSyms::data_t),
        "item index mapping to enum sym index"
    );
    const auto begin = reinterpret_cast<J*>(item->get());
    return toDDB::longs(begin, begin + item->getCount(), stringize(idx));
}

ConstantSP kdb::Parser::NestedMap::getItem(
    const NestedEnumSymsEx* item, const ItemIndex& idx, const byte* end
) const {
    assert(item && item->header.isValid());
    if(!item->isComplete(end)) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Truncated extended nested enum syms " + stringize(idx) + " "
            "(type=" + to_string(item->header.getType()) + ").");
    }

    static_assert(
        sizeof(J) == sizeof(NestedEnumSyms::data_t),
        "item index mapping to enum sym index"
    );
    const auto begin = reinterpret_cast<J*>(item->get());
    return toDDB::longs(begin, begin + item->getCount(), stringize(idx));
}

ConstantSP kdb::Parser::NestedMap::getItem(
    const NestedList<0xFB, 0x00, 1>* item,
    const ItemIndex& idx, const byte* end
) const {
    assert(item && item->header.isValid());
    if(!item->isComplete(end)) {
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
    if(!header.isValid()) {
        throw RuntimeException(PLUGIN_NAME ": "
            "kdb+ nested data map - not a valid data map.");
    }

    switch(idx.format) {
        case 0x00:
            if(getAt<NestedItemExHeader>(idx.getOffset())->isValid()) {
                return make_tuple(
                    getItem(getAt<NestedItemEx>(idx.getOffset()), idx, end),
                    false);
            } else
            if(getAt<NestedListHeader<0xFB, 0x00, 1>>(idx.getOffset())->isValid()) {
                return make_tuple(
                    getItem(getAt<NestedList<0xFB, 0x00, 1>>(
                        idx.getOffset()), idx, end),
                    false);
            } else
            if(getAt<NestedEnumSymsExHeader>(idx.getOffset())->isValid()) {
                return make_tuple(
                    getItem(getAt<NestedEnumSymsEx>(idx.getOffset()), idx, end),
                    true);
            } else {
                throw RuntimeException(PLUGIN_NAME ": "
                    "kdb+ nested data map - (" + stringize(idx) +") "
                    "is not an extended nested item.");
            }
        case 0x01:
            if(getAt<NestedItemHeader>(idx.getOffset())->isValid()) {
                return make_tuple(
                    getItem(getAt<NestedItem>(idx.getOffset()), idx, end),
                    false);
            } else
            if(getAt<NestedEnumSymsHeader>(idx.getOffset())->isValid()) {
                return make_tuple(
                    getItem(getAt<NestedEnumSyms>(idx.getOffset()), idx, end),
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
/**
End of Data  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13 14 15 16 17
~~~~~~~~~~--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 data items |03|00|00|00|02|??|??|??|         offset        |     offset + 0x08     |
~~~~~~~~~~--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *> "offset" = offset of kdb::Parser::Trailer from beginning of data contents
 */
struct kdb::Parser::Trailer {
    //{
    int8_t  tag[5];
    int8_t  pad_xxx[3];
    int64_t end_offset[2];
    //}

    static const int8_t TAG[sizeof(tag)];

    bool isValid(const byte* begin) const {
        const ptrdiff_t offset0 = reinterpret_cast<const byte*>(this) - begin;
        const ptrdiff_t offset1 = offset0 + sizeof(end_offset[0]);
        return memcmp(tag, TAG, sizeof(TAG)) == 0
            && end_offset[0] == offset0 && end_offset[1] == offset1;
    }
};
static_assert(
    sizeof(kdb::Parser::Trailer) == 4*2+8+8,
    "kdb+ file trailer (optional)"
);

const int8_t kdb::Parser::Trailer::TAG[] = {
    0x03, 0x00, 0x00, 0x00, 0x02,
};

//============================================================================
/**
End of Data  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
~~~~~~~~~~--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 data items |00|00|00|00|00|00|00|00|02|00|14|02|??|??|??|??|
~~~~~~~~~~--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 */
struct kdb::Parser::IndexHeader {
    //{
    int8_t  pad_00[8];
    int8_t  tag[4];
    int32_t type;
    //}

    static constexpr int32_t INDEX_P = 0x00;
    static constexpr int32_t INDEX_G = 0x01;
    static const int8_t TAG[sizeof(tag)];

    bool isValid(const byte* begin) const {
        return memcmp(pad_00, "\0\0\0\0\0\0\0\0", sizeof(pad_00)) == 0
            && memcmp(tag, TAG, sizeof(TAG)) == 0
            && (type == INDEX_P || type == INDEX_G);
    }
};
static_assert(
    sizeof(kdb::Parser::IndexHeader) == 8+4+4,
    "kdb+ file index header"
);

const int8_t kdb::Parser::IndexHeader::TAG[] = {
    0x02, 0x00, 0x14, 0x02,
};

#pragma pack(pop)
//////////////////////////////////////////////////////////////////////////////

kdb::Parser::Parser(bool strictNested)
    : buffer_{}, end_{nullptr}, strict_{strictNested}
{}

vector<kdb::byte>& kdb::Parser::getBuffer() noexcept {
    return buffer_;
}

const kdb::byte* kdb::Parser::begin() const noexcept {
    return (assert(!buffer_.empty()), buffer_.data());
}

const kdb::byte* kdb::Parser::end() const noexcept {
    return end_ ? end_ : findEnd();
}

const kdb::byte* kdb::Parser::findEnd(const byte* start) const noexcept {
    assert(!end_);
    const ptrdiff_t end = buffer_.size();

    const ptrdiff_t from = start ? start - begin() : 0;
    const auto trailer = findObj<Trailer>(from, end);
    const auto index   = findObj<IndexHeader>(from, end);
    return end_ = begin() + min(trailer, index);
}

template<typename T>
ptrdiff_t kdb::Parser::findObj(ptrdiff_t from, ptrdiff_t to) const noexcept
{
    const ptrdiff_t end = to - sizeof(T);
    for(auto p = from; p <= end; ++p) {
        const auto obj = parse<T>(p);
        if(obj->isValid(begin())) {
            return p;
        }
    }
    return buffer_.size();
}

template<typename T>
const T* kdb::Parser::parse(ptrdiff_t index, bool allowEnd) const {
    return static_cast<const T*>(parse(sizeof(T), index, allowEnd));
}

// Allow for "end" semantics
const void* kdb::Parser::parse(
    size_t size, ptrdiff_t offset, bool allowEnd
) const {
    if(!(0 <= offset && static_cast<size_t>(offset) <= buffer_.size())) {
        throw RuntimeException(PLUGIN_NAME ": kdb+ file access out of bounds "
            "(" + to_string(offset) + ")!");
    } else
    if(!allowEnd && offset + size > buffer_.size()) {
        throw RuntimeException(PLUGIN_NAME ": kdb+ file access out of bounds "
            "(" + to_string(offset) + ":" + to_string(offset + size) + ")!");
    }

    return begin() + offset;
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
        if(data->getCount()) {
            throw RuntimeException(PLUGIN_NAME ": "
                "Unexpected non-empty kdb+ file with magic 0xFF01.");
        }
        VectorSP any = Util::createVector(DT_ANY, 0);
        fakeEmptyAnyColumn(any.get(), file, "");
        return any;
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

void checkFastVectorType(kdb::Type type, const string &file){
    if(type == kdb::K_STRING) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Unexpected un-enumerated syms list in " + file + ".");
    } else
    if(isEnumType(type)) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Unexpected enumerated syms list in " + file + ".");
    } else
    if(isNestedType(type)) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Unexpected nested data list in " + file + ".");
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

    auto count = data->getCount();
    if(!data->isComplete(itemSize, count, end())) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Truncated or incomplete kdb+ file " + file + " "
            "(" + to_string(type) + ").");
    }

    //FIXME: Deal with some compressed fast vector data that has wrong count...
    const auto length = end() - data->data;
    if(length % itemSize == 0) {
        const auto real_count = length / itemSize;
        if(real_count > count) {
            PLUGIN_LOG(PLUGIN_NAME ": "
                "Incorrect item count found in " + file + " "
                "(count=" + to_string(count) + ","
                " expected=" + to_string(real_count) + ").");
            count = real_count;
        }
    }

    checkFastVectorType(type, file);
    return toDDB::fromArray(type, data->get(), count, file);
}

VectorSP kdb::Parser::getGeneralList(const ExtList* data,
    const string& file, const vector<string>& symList, const string& symName
) const {
    assert(data);
    if(!data->isComplete(end())) {
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
    } else {
        ostringstream msg;
        msg << hex << uppercase << setfill('0')
            << setw(2) << unsigned{data->payload.tag[0]}
            << setw(2) << unsigned{data->payload.tag[1]};
        throw RuntimeException(PLUGIN_NAME ": "
            "Cannot recognize extension header in " + file + " "
            "(0x" + msg.str() + ").");
    }
}

VectorSP kdb::Parser::getEnumStrings(const EnumSymsList* data,
    const string& file, const vector<string>& symList, const string& symName
) const {
    assert(data);
    if(!data->isComplete(end())) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Truncated or incomplete kdb+ extended file " + file + " "
            "(" + to_string(data->header.type) + ").");
    }

    const auto syms = data->getSyms(end(), symList, symName);
    assert(syms.size() == data->getCount() || 0 == data->getCount());
    const auto begin = const_cast<S*>(syms.data());
    return toDDB::strings(begin, begin + syms.size(), file);
}

VectorSP kdb::Parser::getNestedLists(const NestedList<0xFD, 0x01>* data,
    const string& file, const vector<string>& symList, const string& symName
) const {
    assert(data);
    const auto type = data->header.type;
    assert(K_NESTED_MIN <= type && type <= K_NESTED_MAX);
    if(!data->isComplete(end())) {
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
        if(!mapData->isComplete(mapParser.end())) {
            throw RuntimeException(PLUGIN_NAME ": "
                "Truncated or incomplete kdb+ data map " + mapPath + " "
                "(" + to_string(mapData->header.type) + ")");
        } else
        if(!mapData->get<NestedMapHeader>()->isValid()) {
            throw RuntimeException(PLUGIN_NAME ": "
                "Cannot recognize kdb+ data map header in " + mapPath + ".");
        }
        return mapNestedLists(begin, end,
            mapData->get<NestedMap>(), mapParser.end(), mapPath,
            symList, symName);
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
    const auto form = item->getForm();
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
    } else if (form == DF_SCALAR) { // HACK only char list would return string scalar
        vec = Util::createVector(DT_STRING, 0, count);
        vec->append(item);
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

        if(item->getType() != type) {
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

    if(symName.empty()) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Enum sym has not been loaded for kdb+ extended file"
            " " + mapFile + " yet.");
    }

    const auto enumNamePath = mapFile + '#';
    BinFile enumNameFile{enumNamePath, enumNamePath};
    Parser enumNameParser;
    enumNameFile.readInto(enumNameParser.getBuffer());
    const auto enumNames = enumNameParser.getStrings(enumNamePath);
    if(enumNames.size() != 1) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Failed to identify symbol enum name from " + enumNamePath + ".");
    } else
    if(symName != enumNames.front()) {
        throw RuntimeException(PLUGIN_NAME ": "
            "kdb+ extended file " + mapFile + " "
            "was not enumerated on " + symName + ".");
    }

	const auto begin = indices->getLongConst(0, indices->size(), nullptr);
    const auto end   = begin + indices->size();
    static_assert(
        sizeof(remove_pointer<decltype(begin)>::type) == sizeof(J),
        "avoid enum sym index copy"
    );
    return toDDB::mapStrings(
        const_cast<J*>(begin), const_cast<J*>(end), symList, symName, mapFile);
}

DatabaseUpdater::DatabaseUpdater(Heap *heap, SystemHandleSP dbHandle, ConstantSP tableName, ConstantSP partitionColumns,
                                 ConstantSP sortColumns, const vector<string> &colNames,
                                 const vector<DATA_TYPE> &colTypes, FunctionDefSP transform)
    : heap_(heap),
      dbHandle_(dbHandle),
      tableName_(tableName),
      partitionColumns_(partitionColumns),
      sortColumns_(sortColumns),
      colNames_(colNames),
      colTypes_(colTypes),
      transform_(transform) {
    initConvertFuncMap(heap);

    owner_ = heap->currentSession()->getUser()->getUserId();
    domain_ = dbHandle->getDomain();
    partitionType_ = domain_->getPartitionType();
    inMemory_ = dbHandle->getDatabaseDir().empty();
    if (partitionType_ == SEQ) {
        throw RuntimeException("SEQ partition is not supported.");
    }
    appendFunc_ = heap_->currentSession()->getFunctionDef("append!");
    loadTableFunc_ = heap_->currentSession()->getFunctionDef("loadTable");
    createPartitionedTableFunc_ = heap_->currentSession()->getFunctionDef("createPartitionedTable");
    existsTableFunc_ = heap_->currentSession()->getFunctionDef("existsTable");
    dbPath_ = dbHandle_->getDatabaseDir();

    vector<ConstantSP> existsTableArgs = {new String(dbPath_), tableName_};
    bool existsTable = existsTableFunc_->call(heap_, existsTableArgs)->getBool();
    if (existsTable) {
        vector<ConstantSP> loadTableArgs = {dbHandle_, tableName_};
        destTable_ = loadTableFunc_->call(heap_, loadTableArgs);
    } else {
        if (!transform_.isNull()) {
            throw RuntimeException(PLUGIN_NAME
                                   "If a transforming function is specified, the partitioned table must be created "
                                   "before appending data to it.");
        }
        if (!colNames.empty() && !colTypes.empty()) {
            ConstantSP dummyTable = Util::createTable(colNames_, colTypes_, 0, 1);
            if (dummyTable.isNull()) {
                throw RuntimeException(PLUGIN_NAME "create table failed, invalid schema.");
            }
            vector<ConstantSP> createTableArgs = {dbHandle_, dummyTable, tableName_, partitionColumns_};
            auto engineType = dbHandle->getDomain()->getEngineType();
            if (engineType == DBENGINE_TYPE::IOT) {
                if (sortColumns.isNull()) {
                    throw RuntimeException(PLUGIN_NAME "sortColumns is needed if database engine is TSDB");
                }
                // NOTE if not OLAP, must has sortColumns or primaryKey
                createTableArgs.emplace_back(new Void());
                createTableArgs.push_back(sortColumns);
            }
            destTable_ = createPartitionedTableFunc_->call(heap_, createTableArgs);
        } else {
            tableCreateFlag_ = true;
        }
    }
}

Mutex DatabaseUpdater::convertMutex_;
unordered_map<int, FunctionDefSP> DatabaseUpdater::convertFuncMap_;

void DatabaseUpdater::initConvertFuncMap(Heap *heap) {
    LockGuard<Mutex> mutex(&convertMutex_);
    convertFuncMap_[DT_BOOL] = heap->currentSession()->getFunctionDef("bool");
    convertFuncMap_[DT_BOOL+ARRAY_TYPE_BASE] = heap->currentSession()->getFunctionDef("bool");
    convertFuncMap_[DT_UUID] = heap->currentSession()->getFunctionDef("uuid");
    convertFuncMap_[DT_UUID+ARRAY_TYPE_BASE] = heap->currentSession()->getFunctionDef("uuid");
    convertFuncMap_[DT_CHAR] = heap->currentSession()->getFunctionDef("char");
    convertFuncMap_[DT_CHAR+ARRAY_TYPE_BASE] = heap->currentSession()->getFunctionDef("char");
    convertFuncMap_[DT_SHORT] = heap->currentSession()->getFunctionDef("short");
    convertFuncMap_[DT_SHORT+ARRAY_TYPE_BASE] = heap->currentSession()->getFunctionDef("short");
    convertFuncMap_[DT_INT] = heap->currentSession()->getFunctionDef("int");
    convertFuncMap_[DT_INT+ARRAY_TYPE_BASE] = heap->currentSession()->getFunctionDef("int");
    convertFuncMap_[DT_LONG] = heap->currentSession()->getFunctionDef("long");
    convertFuncMap_[DT_LONG+ARRAY_TYPE_BASE] = heap->currentSession()->getFunctionDef("long");
    convertFuncMap_[DT_FLOAT] = heap->currentSession()->getFunctionDef("float");
    convertFuncMap_[DT_FLOAT+ARRAY_TYPE_BASE] = heap->currentSession()->getFunctionDef("float");
    convertFuncMap_[DT_DOUBLE] = heap->currentSession()->getFunctionDef("double");
    convertFuncMap_[DT_DOUBLE+ARRAY_TYPE_BASE] = heap->currentSession()->getFunctionDef("double");
    convertFuncMap_[DT_DATE] = heap->currentSession()->getFunctionDef("date");
    convertFuncMap_[DT_DATE+ARRAY_TYPE_BASE] = heap->currentSession()->getFunctionDef("date");
    convertFuncMap_[DT_MONTH] = heap->currentSession()->getFunctionDef("month");
    convertFuncMap_[DT_MONTH+ARRAY_TYPE_BASE] = heap->currentSession()->getFunctionDef("month");
    convertFuncMap_[DT_TIME] = heap->currentSession()->getFunctionDef("time");
    convertFuncMap_[DT_TIME+ARRAY_TYPE_BASE] = heap->currentSession()->getFunctionDef("time");
    convertFuncMap_[DT_MINUTE] = heap->currentSession()->getFunctionDef("minute");
    convertFuncMap_[DT_MINUTE+ARRAY_TYPE_BASE] = heap->currentSession()->getFunctionDef("minute");
    convertFuncMap_[DT_SECOND] = heap->currentSession()->getFunctionDef("second");
    convertFuncMap_[DT_SECOND+ARRAY_TYPE_BASE] = heap->currentSession()->getFunctionDef("second");
    convertFuncMap_[DT_DATETIME] = heap->currentSession()->getFunctionDef("datetime");
    convertFuncMap_[DT_DATETIME+ARRAY_TYPE_BASE] = heap->currentSession()->getFunctionDef("datetime");
    convertFuncMap_[DT_TIMESTAMP] = heap->currentSession()->getFunctionDef("timestamp");
    convertFuncMap_[DT_TIMESTAMP+ARRAY_TYPE_BASE] = heap->currentSession()->getFunctionDef("timestamp");
    convertFuncMap_[DT_NANOTIME] = heap->currentSession()->getFunctionDef("nanotime");
    convertFuncMap_[DT_NANOTIME+ARRAY_TYPE_BASE] = heap->currentSession()->getFunctionDef("nanotime");
    convertFuncMap_[DT_NANOTIMESTAMP] = heap->currentSession()->getFunctionDef("nanotimestamp");
    convertFuncMap_[DT_NANOTIMESTAMP+ARRAY_TYPE_BASE] = heap->currentSession()->getFunctionDef("nanotimestamp");
    convertFuncMap_[DT_SYMBOL] = heap->currentSession()->getFunctionDef("symbol");
    convertFuncMap_[DT_STRING] = heap->currentSession()->getFunctionDef("string");
    convertFuncMap_[DT_BLOB] = heap->currentSession()->getFunctionDef("blob");
}

// do not support concurrent append
void DatabaseUpdater::append(TableSP table) {
    // apply transform
    TableSP appendData;
    // use schema type to trans first
    if (!colTypes_.empty()) {
        if (int(colTypes_.size()) != table->columns()) {
            throw RuntimeException(PLUGIN_NAME "expect table with " + std::to_string(colTypes_.size()) +
                                   " columns, actually " + std::to_string(table->columns()));
        }
        vector<ConstantSP> cols;
        int columnSize = table->columns();
        cols.reserve(columnSize);
        for (int i = 0; i < columnSize; ++i) {
            if (table->getColumnType(i) != colTypes_[i]) {
                if (convertFuncMap_.find(colTypes_[i]) == convertFuncMap_.end()) {
                    throw RuntimeException(PLUGIN_NAME "type " + Util::getDataTypeString(colTypes_[i]) +
                                           " is not supported to convert, actual type " +
                                           Util::getDataTypeString(table->getColumnType(i)));
                }
                const FunctionDefSP &func = convertFuncMap_[colTypes_[i]];
                vector<ConstantSP> args{table->getColumn(i)};
                cols.push_back(func->call(heap_, args));
            } else {
                cols.push_back(table->getColumn(i));
            }
        }
        table = Util::createTable(colNames_, cols);
    }

    vector<ConstantSP> args{table};
    if (!transform_.isNull()) {
        appendData = transform_->call(heap_, args);
        if (appendData->getForm() != DF_TABLE) {
            throw RuntimeException("Output of transform must be a table.");
        }
    } else {
        appendData = table;
    }

    if (UNLIKELY(tableCreateFlag_)) {  // only if without schema, without transform
        colNames_.clear();
        colTypes_.clear();
        for (int i = 0; i < appendData->columns(); ++i) {
            colNames_.push_back(table->getColumnName(i));
            colTypes_.push_back(table->getColumnType(i));
        }
        ConstantSP dummyTable = Util::createTable(colNames_, colTypes_, 0, 1);
        if (dummyTable.isNull()) {
            throw RuntimeException(PLUGIN_NAME "create table failed, invalid schema.");
        }
        vector<ConstantSP> createTableArgs = {dbHandle_, dummyTable, tableName_, partitionColumns_};
        auto engineType = dbHandle_->getDomain()->getEngineType();
        if (engineType == DBENGINE_TYPE::IOT) {
            if (sortColumns_.isNull()) {
                throw RuntimeException(PLUGIN_NAME "sortColumns is needed if database engine is tsdb");
            }
            // NOTE if not OLAP, must has sortColumns or primaryKey
            createTableArgs.emplace_back(new Void());
            createTableArgs.push_back(sortColumns_);
        }
        destTable_ = createPartitionedTableFunc_->call(heap_, createTableArgs);
        tableCreateFlag_ = false;
    }

    // do append
    vector<ConstantSP> appendArgs{destTable_, appendData};
    appendFunc_->call(heap_, appendArgs);
}

ConstantSP DatabaseUpdater::getTableHandle() {
    if (destTable_.isNull()) {
        throw RuntimeException(PLUGIN_NAME "unable to get loaded table.");
    }
    return destTable_;
}

bool kdb::Parser::isValidBuffer(long long batchSize) {
    switch (parserType_) {
        case BaseArray: {
            switch(kdbType_) {
                case K_BOOL:
                    return buffer_.size() > batchSize * sizeof(G);
                case K_GUID:
                    return buffer_.size() > batchSize * sizeof(U);
                case K_BYTE:
                    return buffer_.size() > batchSize * sizeof(G);
                case K_SHORT:
                    return buffer_.size() > batchSize * sizeof(H);
                case K_INT:
                    return buffer_.size() > batchSize * sizeof(I);
                case K_LONG:
                    return buffer_.size() > batchSize * sizeof(J);
                case K_FLOAT:
                    return buffer_.size() > batchSize * sizeof(E);
                case K_DOUBLE:
                    return buffer_.size() > batchSize * sizeof(F);
                case K_CHAR:
                    return buffer_.size() > batchSize * sizeof(G);
                case K_STRING:
                    return buffer_.size() > batchSize * sizeof(S);
                case K_TIMESTAMP:
                    return buffer_.size() > batchSize * sizeof(J);
                case K_MONTH:
                    return buffer_.size() > batchSize * sizeof(I);
                case K_DATE:
                    return buffer_.size() > batchSize * sizeof(I);
                case K_DATETIME:
                    return buffer_.size() > batchSize * sizeof(F);
                case K_TIMESPAN:
                    return buffer_.size() > batchSize * sizeof(J);
                case K_MINUTE:
                    return buffer_.size() > batchSize * sizeof(I);
                case K_SECOND:
                    return buffer_.size() > batchSize * sizeof(I);
                case K_TIME:
                    return buffer_.size() > batchSize * sizeof(I);
                default:
                    throw RuntimeException(PLUGIN_NAME ": " "kdb+ object with type " + to_string(kdbType_) +
                        "not yet supported.");
            }
            break;
        }
        case EnumSym:
        case NestedArray: {
            return buffer_.size() > batchSize * sizeof(ItemIndex);
            break;
        }
        case Invalid: // HACK if invalid, must be empty
            return true;
        default:
            throw RuntimeException(PLUGIN_NAME "validate buffer failed, invalid parser type " + std::to_string(parserType_));
    }
}

bool kdb::Parser::getBatch(long long batchSize, VectorSP buffer, bool finalBatch, const vector<string> &symList,
                           const string &symName) {
    long long count = batchSize;
    long long usedSize;
    switch (parserType_) {
        case BaseArray: {
            // get real size, get buffer info
            long long bufSize = buffer_.size();
#define BUF_WRANGLE(T)                         \
    bufSize /= sizeof(T);                      \
    count = bufSize < count ? bufSize : count; \
    usedSize = count * sizeof(T);              \
    break;
            switch (kdbType_) {
                case K_BOOL:
                case K_BYTE:
                case K_CHAR:
                    BUF_WRANGLE(G)
                case K_GUID:
                    BUF_WRANGLE(U)
                case K_SHORT:
                    BUF_WRANGLE(H)
                case K_INT:
                case K_MINUTE:
                case K_SECOND:
                case K_TIME:
                case K_DATE:
                case K_MONTH:
                    BUF_WRANGLE(I)
                case K_LONG:
                case K_TIMESPAN:
                case K_TIMESTAMP:
                    BUF_WRANGLE(J)
                case K_FLOAT:
                    BUF_WRANGLE(E)
                case K_DOUBLE:
                case K_DATETIME:
                    BUF_WRANGLE(F)
                case K_STRING:
                    BUF_WRANGLE(S)
                default:
                    throw RuntimeException(PLUGIN_NAME "kdb+ object with type " +
                                           to_string(kdbType_) + "not yet supported.");
            }
            VectorSP tmp = toDDB::fromArray(kdbType_, buffer_.data(), count, to_string(kdbType_) /*TODO*/);
            buffer->append(tmp);
            break;
        }
        case EnumSym: {
            ItemIndex *begin = (ItemIndex *)buffer_.data();
            vector<S> syms;
            if (batchSize * sizeof(ItemIndex) > buffer_.size()) {
                batchSize = buffer_.size() / sizeof(ItemIndex);
            }
            usedSize = batchSize * sizeof(ItemIndex);
            syms.reserve(batchSize);
            transform(begin, begin + batchSize, back_inserter(syms), [&symList, &symName](const ItemIndex &idx) {
                if (isNull(bit_cast<J>(idx.index))) {
                    return sym(nullptr);
                } else if (0 <= idx.index && static_cast<size_t>(idx.index) < symList.size()) {
                    return sym(symList[idx.index]);
                } else {
                    PLUGIN_LOG(PLUGIN_NAME ": Enumerated sym out of bounds " + symName + "[" + to_string(idx.index) + "].");
                    return sym(nullptr);
                }
            });

            const auto symsBegin = const_cast<S *>(syms.data());
            buffer->append(toDDB::strings(symsBegin, symsBegin + syms.size(), "" /* TODO path assignment*/));
            break;
        }
        case NestedArray: {

            if (batchSize * sizeof(ItemIndex) > buffer_.size()) {
                batchSize = buffer_.size() / sizeof(ItemIndex);
            }
            usedSize = batchSize * sizeof(ItemIndex);

            ItemIndex *idx = (ItemIndex *)buffer_.data();
            auto end = idx + batchSize;
            VectorSP item;
            bool isEnumSyms;
            VectorSP tuple = Util::createVector(DT_ANY, 1);
            // Parse the remaining items in sequence
            for (; idx < end; ++idx) {
                tie(item, isEnumSyms) = mapData_->getItem(*idx, mapEnd_);
                assert(!item.isNull());
                if (isEnumSyms) {
                    item = mapEnumSyms(item, mapPath_, symList, symName);
                }

                auto colType = item->getType();
                if (isArrayVector_) {
                    colType = (DATA_TYPE)(colType + ARRAY_TYPE_BASE);
                }
                if (colType != ddbType_) {
                    throw RuntimeException(PLUGIN_NAME "Heterogeneous nested data in " + mapPath_ +
                                           " not supported, expect " + Util::getDataTypeString(ddbType_) + " actual " +
                                           Util::getDataTypeString(colType));
                } else if (isArrayVector_) {
                    if (!firstItem_.isNull() && idx == (ItemIndex *)buffer_.data()) {
                        // HACK first item use one get from getStruct
                        tuple->set(0, firstItem_);
                        buffer->append(tuple);
                        firstItem_.clear();
                    } else {
                        tuple->set(0, item);
                        buffer->append(tuple);
                    }
                } else {
                    buffer->append(item);
                }
            }
            break;
        }
        case Invalid: // HACK if invalid, must be empty
            return true;
        default:
            throw RuntimeException(PLUGIN_NAME "get batch failed, invalid parser type " + std::to_string(parserType_));
    }
    // move rest of buffer to very first
    vector<byte> remainedBuffer;
    remainedBuffer.resize(buffer_.size() - usedSize);
    std::copy(buffer_.begin() + usedSize, buffer_.end(), remainedBuffer.begin());
    buffer_ = remainedBuffer;
    return true;
}

DATA_TYPE kdb::Parser::getStruct(const std::string &file, const std::vector<std::string> &symList,
                                 const std::string &symName, size_t &count) {
    // NOTE assume parse would never out of bound check
    if (parse<BaseHeader>()->isValid()) {
        // fake any vector
        parserType_ = Invalid;
        return DT_ANY;
    } else if (parse<BaseListHeader>()->isValid()) {
        const BaseList *data = parse<BaseList>();
        const auto type = data->header.type;
        const auto itemSize = getSize(type);
        if (itemSize == UNKNOWN_SIZE) {
            throw RuntimeException(PLUGIN_NAME "Cannot recognize data type in kdb+ file " + file + " (" +
                                   to_string(type) + ").");
        }

        count = data->getCount();

        // //FIXME: Deal with some compressed fast vector data that has wrong count...
        // const auto length = end() - data->data;
        // if(length % itemSize == 0) {
        //     const auto real_count = length / itemSize;
        //     if(real_count > count) {
        //         PLUGIN_LOG(PLUGIN_NAME ": "
        //             "Incorrect item count found in " + file + " "
        //             "(count=" + to_string(count) + ","
        //             " expected=" + to_string(real_count) + ").");
        //         count = real_count;
        //     }
        // }

        checkFastVectorType(type, file);

        byte *dataBegin = data->get();
        ptrdiff_t gap = dataBegin - buffer_.data();
        vector<byte> tmp;
        tmp.reserve(buffer_.size());
        for (auto i = gap; i < (ptrdiff_t)buffer_.size(); ++i) {
            tmp.push_back(buffer_[i]);
        }
        buffer_ = tmp;
        parserType_ = BaseArray;
        kdbType_ = type;
        ddbType_ = KDB_DDB_TYPE_MAP[type];
        return ddbType_;

    } else if (parse<ExtListHeader>()->isValid()) {
        const ExtList *data = parse<ExtList>();

        if (data->get<SimpleListHeader>()->isValid()) {
            const SimpleList *simpleListPtr = data->get<SimpleList>();

            // NOTE for uuid
            count = simpleListPtr->getCount();
            if (!simpleListPtr->header.isValid()) {
                throw RuntimeException(PLUGIN_NAME "kdb+ extended file - not a valid simple list.");
            }
            ostringstream msg;
            msg << hex << uppercase << setfill('0') << setw(2) << simpleListPtr->header.type;

            byte *dataBegin = simpleListPtr->get();
            ptrdiff_t gap = dataBegin - buffer_.data();
            vector<byte> tmp;
            tmp.reserve(buffer_.size());
            for (unsigned int i = gap; i < buffer_.size(); ++i) {
                tmp.push_back(buffer_[i]);
            }
            buffer_ = tmp;
            parserType_ = BaseArray;
            kdbType_ = simpleListPtr->header.type;
            ddbType_ = KDB_DDB_TYPE_MAP[kdbType_];
            return ddbType_;

        } else if (data->get<EnumSymsHeader>()->isValid()) {
            if (UNLIKELY(symName.empty())) {
                throw RuntimeException(PLUGIN_NAME "Enum sym has not been loaded for kdb+ extended file " +
                                       file + " yet.");
            } else if (UNLIKELY(symName != data->enum_name)) {
                throw RuntimeException(PLUGIN_NAME
                                       "kdb+ extended file " +
                                       file +
                                       " was not enumerated on " +
                                       symName + ".");
            }
            const EnumSymsList *symsListPtr = data->get<EnumSymsList>();

            if (!symsListPtr->header.isValid()) {
                throw RuntimeException(PLUGIN_NAME "kdb+ extended file - not a valid enumerated syms list.");
            }

            count = symsListPtr->getCount();

            // const auto begin = symsListPtr->get();

            // //FIXME: Deal with some compressed enum sym vector with wrong count...
            // const auto length = end - reinterpret_cast<const byte*>(begin);
            // if(length % sizeof(data_t) == 0) {
            //     const auto real_count = length / sizeof(data_t);
            //     if(real_count > 0 && count == 0) {
            //         PLUGIN_LOG(PLUGIN_NAME ": "
            //             "Incorrect enum sym count found "
            //             "(count=" + to_string(count) + ","
            //             " expected=" + to_string(real_count) + ").");
            //         count = real_count;
            //     }
            // }

            byte *dataBegin = reinterpret_cast<byte *>(symsListPtr->get());
            ptrdiff_t gap = dataBegin - buffer_.data();
            vector<byte> tmp;
            tmp.reserve(buffer_.size());
            for (auto i = gap; i < (ptrdiff_t)buffer_.size(); ++i) {
                tmp.push_back(buffer_[i]);
            }
            buffer_ = tmp;
            parserType_ = EnumSym;
            kdbType_ = K_STRING;  // NOTE maybe not
            ddbType_ = DT_SYMBOL;

            return DT_SYMBOL;

        } else if (data->get<NestedListHeader<0xFD, 0x01>>()->isValid()) {
            const NestedList<0xFD, 0x01> *nestedData = data->get<NestedList<0xFD, 0x01>>();
            const auto type = nestedData->header.type;
            assert(isNestedType(type));

            const ItemIndex *begin, *end;
            tie(begin, end) = nestedData->getIndices();
            if (!nestedData->getCount()) {
                end = begin + nestedData->guessCount(this->end());
            }
            assert(begin && begin <= end);
            if (begin == end) {
                throw RuntimeException(PLUGIN_NAME "Unexpected empty nested list in " + file + ".");
            }

            const auto mapPath = file + '#';
            BinFile mapFile{mapPath, mapPath};
            mapPath_ = mapPath;
            mapParser_ = new Parser();
            mapFile.readInto(mapParser_->getBuffer());
            if (mapParser_->parse<ExtListHeader>()->isValid()) {
                const auto mapData = mapParser_->parse<ExtList>();
                if (!mapData->isComplete(mapParser_->end())) {
                    throw RuntimeException(PLUGIN_NAME "Truncated or incomplete kdb+ data map " + mapPath + " (" +
                                           to_string(mapData->header.type) + ")");
                } else if (!mapData->get<NestedMapHeader>()->isValid()) {
                    throw RuntimeException(PLUGIN_NAME "Cannot recognize kdb+ data map header in " + mapPath + ".");
                }
                mapEnd_ = mapParser_->end();
                mapData_ = mapData->get<NestedMap>();

                auto idx = begin;
                bool isEnumSyms;
                tie(firstItem_, isEnumSyms) = mapData_->getItem(*idx, mapEnd_);
                assert(!firstItem_.isNull());
                if (isEnumSyms) {
                    firstItem_ = mapEnumSyms(firstItem_, mapPath_, symList, symName);
                }

                // ... and check if we can create an array vector instead of an any vector
                const auto type = firstItem_->getType();
                const auto form = firstItem_->getForm();
                switch (type) {
                    case DT_SYMBOL:
                    case DT_STRING:
                        isArrayVector_ = false;
                        break;
                    default:
                        isArrayVector_ = true;
                }

                VectorSP vec, tuple;
                // cannot use count, because nested data count is bad
                auto count = end - begin;
                // do with first element
                if (isArrayVector_) {
                    vec = InternalUtil::createArrayVector(
                        static_cast<DATA_TYPE>(ARRAY_TYPE_BASE + firstItem_->getType()), 0, 0, count, 0);
                    tuple = Util::createVector(DT_ANY, 1);
                    tuple->set(0, firstItem_);
                    vec->append(tuple);
                    ddbType_ = DATA_TYPE(ARRAY_TYPE_BASE + firstItem_->getType());
                } else if (form == DF_SCALAR) {  // HACK only char list would return string scalar
                    vec = Util::createVector(DT_STRING, 0, count);
                    vec->append(firstItem_);
                    ddbType_ = DT_STRING;
                } else {
                    vec = Util::createVector(DT_ANY, 0, count);
                    vec->append(firstItem_);
                    ddbType_ = DT_ANY;
                }

                byte *dataBegin = (byte *)begin;
                ptrdiff_t gap = dataBegin - buffer_.data();
                vector<byte> remainedBuffer;
                remainedBuffer.resize(buffer_.size() - gap);
                std::copy(buffer_.begin() + gap, buffer_.end(), remainedBuffer.begin());
                buffer_ = remainedBuffer;
                parserType_ = NestedArray;
                // kdbType_ = type;
                return ddbType_;
            } else {
                throw RuntimeException(PLUGIN_NAME "Cannot recognize kdb+ data map file header in " + mapPath + ".");
            }
        } else {
            ostringstream msg;
            msg << hex << uppercase << setfill('0') << setw(2) << unsigned{data->payload.tag[0]} << setw(2)
                << unsigned{data->payload.tag[1]};
            throw RuntimeException(PLUGIN_NAME "Cannot recognize extension header in " + file +
                                   " (0x" + msg.str() + ").");
        }
    } else {
        ostringstream msg;
        if (buffer_.size() >= 2) {
            msg << "0x" << hex << uppercase << setfill('0') << setw(2) << unsigned{buffer_[0]} << setw(2)
                << unsigned{buffer_[1]};
        } else {
            msg << "insufficient bytes";
        }
        throw RuntimeException(PLUGIN_NAME "<nyi> kdb+ file magic in " + file + " (" + msg.str() + ").");
    }
}
//////////////////////////////////////////////////////////////////////////////
