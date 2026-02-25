#include <cstring>
#include <string>
#include <cmath>
#include <ctime>
#include <regex>

#include "TypeHelper.h"
#ifdef PYTHON_SWORDFISH
#include "TypeBinding.h"
#endif
#include "TypeConverter.h"
#include "DecimalHelper.h"
#include "OperatorImp.h"

#include "ScalarImp.h"

#include <modsupport.h>
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/chrono.h>
#include <pybind11/stl.h>


using namespace pybind11::literals;

#ifdef PYTHON_SWORDFISH
using pybind_dolphindb::SwordfishDataType;
#endif


namespace converter {


PyObject *decodeUtf8Text(const char *pchar,int charsize) {
    PyObject *tmp = PyUnicode_DecodeUTF8Stateful(pchar,charsize,NULL,NULL);
    if (PyErr_Occurred() && tmp == NULL) {
        PyErr_Clear();
        tmp = PyUnicode_DecodeUTF8Stateful(pchar,charsize,"ignore",NULL);
        std::string error_str(pchar, charsize);
#ifdef PYTHON_SWORDFISH
        LOG_ERR("Cannot decode data: " + error_str + ", Please encode the string using the UTF-8 format.");
#else
        throw RuntimeException("Cannot decode data: " + error_str + ", Please encode the string using the UTF-8 format.");
#endif
    }
    return tmp;
}


static inline Type _extractTypeFromString(const std::string &val) {
    std::string name = Util::upper(val);
    if (Util::endWith(name, "[]")) {
        Type elem = _extractTypeFromString(name.substr(0, name.size() - 2));
        return Type((HELPER_TYPE)(elem.first + ARRAY_TYPE_BASE), elem.second);
    }
    static const std::unordered_map<int, HELPER_TYPE> bit_type_map = {
        {32, HELPER_TYPE::HT_DECIMAL32},
        {64, HELPER_TYPE::HT_DECIMAL64},
        {128, HELPER_TYPE::HT_DECIMAL128},
    };
    std::regex pattern(R"(^DECIMAL(\d+)\((\d+)\)$)");
    std::smatch match;
    if (std::regex_match(name, match, pattern)) {
        int bit_num = std::stoi(match[1].str());
        int scale = std::stoi(match[2].str());
        auto it = bit_type_map.find(bit_num);
        if (it != bit_type_map.end()) {
            return Type(it->second, scale);
        }
        throw ConversionException("[TODO] Invalid type string. unsupport DECIMAL" + std::to_string(bit_num) + ".");
    }
    HELPER_TYPE type = (HELPER_TYPE)Util::getDataType(name);
    return Type(type, EXPARAM_DEFAULT);
}

Type createType(const std::string &data) { return _extractTypeFromString(data); }

Type createType(const py::handle &data) {
    Type type;
    if (data.is_none()) {
        type = {HT_UNK, EXPARAM_DEFAULT};
    }
    // else if (py::isinstance<SwordfishDataType>(data)) {
    //     type = (py::cast<SwordfishDataType*>(data))->unpack();
    // }
    else if (CHECK_INS(data, py_int_)) {
        // change int to Type
        type = std::make_pair(static_cast<HELPER_TYPE>(py::cast<int>(data)), EXPARAM_DEFAULT);
    }
    else if (CHECK_INS(data, py_list_)) {
        // change [int, int] to Type
        py::list tmplist = py::reinterpret_borrow<py::list>(data);
        if (tmplist.size() <=0 || tmplist.size() > 2)
            throw ConversionException("Conversion failed. Specify a valid type.");
        HELPER_TYPE datatype = static_cast<HELPER_TYPE>(py::cast<int>(tmplist[0]));
        int exparam = py::cast<int>(tmplist[1]);
        type = std::make_pair(datatype, exparam);
    }
    else {
        throw ConversionException("Conversion failed. Specify a valid type.");
    }
    return type;
}


ConstantSP packDataTypeEnum(Type type) {
    return new pybind_dolphindb::Int(_decimal_util::packDecimalTypeAndScale((DATA_TYPE)type.first, EXPARM_VALUE(type.second)));
}
Type unpackDataTypeEnum(ConstantSP &obj) {
    int val = obj->getInt();
    auto type = _decimal_util::unpackDecimalTypeAndScale(val);
    return {(HELPER_TYPE)type.first, type.second};
}


bool canConvertTo(const Type &src, const Type &dst, Type &res) {
    if (src.first == HT_VOID && dst.first == HT_VOID) {
        res = {HT_ANY, EXPARAM_DEFAULT};
        return true;
    }
    if (src.first == HT_VOID) {
        res = dst;
        return true;
    }
    if (dst.first == HT_VOID) {
        res = src;
        return true;
    }
    if (src.first == HT_ANY || dst.first == HT_ANY) {
        res = {HT_ANY, EXPARAM_DEFAULT};
        return true;
    }
    if (src.first == dst.first) {
        if (EXPARM_VALUE(src.second) != EXPARM_VALUE(dst.second)) {
            res = {HT_ANY, EXPARAM_DEFAULT};
            return false;
        }
        if (src.second != EXPARAM_DEFAULT) res = src;
        else res = dst;
        return true;
    }
    DATA_CATEGORY src_category = getCategory(src);
    DATA_CATEGORY dst_category = getCategory(dst);
    if (src_category != dst_category) {
        if ((src_category == DATA_CATEGORY::INTEGRAL || src_category == DATA_CATEGORY::FLOATING) &&
            (dst_category == DATA_CATEGORY::INTEGRAL || dst_category == DATA_CATEGORY::FLOATING)) {
            res = {std::max(src.first, dst.first), EXPARAM_DEFAULT};
            return true;
        }
        res = {HT_ANY, EXPARAM_DEFAULT};
        return false;
    }
    if (src_category == DATA_CATEGORY::TEMPORAL) {
        res = {HT_ANY, EXPARAM_DEFAULT};
        return false;
    }
    if (src_category == DATA_CATEGORY::INTEGRAL) {
        res = {std::max(src.first, dst.first), EXPARAM_DEFAULT};
        return true;
    }
    if (src_category == DATA_CATEGORY::LOGICAL) {
        res = {HT_BOOL, EXPARAM_DEFAULT};
        return true;
    }
    if (src_category == DATA_CATEGORY::FLOATING) {
        res = {std::max(src.first, dst.first), EXPARAM_DEFAULT};
        return true;
    }
    if (src_category == DATA_CATEGORY::LITERAL || src_category == DATA_CATEGORY::BINARY || src_category == DATA_CATEGORY::MIXED) {
        res = {HT_ANY, EXPARAM_DEFAULT};
        return false;
    }
    if (src_category == DATA_CATEGORY::NOTHING) {
        res = {HT_ANY, EXPARAM_DEFAULT};
        return false;
    }
    if (src_category == DATA_CATEGORY::DENARY) {
        res = {HT_ANY, EXPARAM_DEFAULT};
        return false;
    }
    res = {HT_ANY, EXPARAM_DEFAULT};
    return false;
}

#ifdef PYTHON_SWORDFISH
ConstantSP getConstantSP_VOID() { return Expression::void_; }
#else
ConstantSP getConstantSP_VOID()
{
    static ConstantSP void_ = new Void();
    return void_;
}
#endif

#ifdef PYTHON_SWORDFISH
ConstantSP getConstantSP_DFLT() { return Expression::default_; }
#else
ConstantSP getConstantSP_DFLT()
{
    static ConstantSP dflt_ = new Void(false, true);
    return dflt_;
}
#endif

#ifdef PYTHON_SWORDFISH
ConstantSP getConstantSP_NULL() { return Expression::null_; }
#else
ConstantSP getConstantSP_NULL()
{
    static ConstantSP null_ = new Void(true, false);
    return null_;
}
#endif

DATA_CATEGORY getCategory(Type type) {
    return Util::getCategory((DATA_TYPE)type.first);
}

std::string getDataTypeString(Type type) {
    if (type.first == HELPER_TYPE::HT_EXTENSION) {
        return "extension";
    }
    if (type.first == HELPER_TYPE::HT_UNK) {
        return "unknown";
    }
    return Util::getDataTypeString((DATA_TYPE)type.first);
}


void SetOrThrowErrorInfo(ConvertErrorInfo *error, ConvertErrorInfo::ErrorCode errorCode, const std::string &errorInfo) {
    if (error != NULL) {
        error->set(errorCode, errorInfo);
    }
    throw ConversionException(errorInfo);
}


void throwExceptionAboutNumpyVersion(int rowIndex, const std::string &value, const std::string &type, const VectorInfo &info) {
    std::string errInfo;
    if (info.option == VectorInfo::COLNAME)
        errInfo = "The value " + value + " (column \"" + info.colName + "\", row " + std::to_string(rowIndex) + ") must be of " + type + " type.";
    else if (info.option == VectorInfo::COLID) {
        if (info.isCol)
            errInfo = "The value " + value + " (column " + std::to_string(info.colId) + ", row " + std::to_string(rowIndex) + ") must be of " + type + " type.";
        else
            errInfo = "The value " + value + " (column " + std::to_string(rowIndex) + ", col " + std::to_string(info.colId) + ") must be of " + type + " type.";
    }
    else
        errInfo = "The value " + value + " (at index " + std::to_string(rowIndex) + ") must be of " + type + " type.";
    if (PyObjs::cache_->np_above_1_20_) {
        throw ConversionException(errInfo);
    }
    else {
        throw ConversionException(errInfo + " Also check if the object is of an unsupported numpy dtype.");
    }
}


void throwExceptionAboutChildOption(const CHILD_VECTOR_OPTION &option, const std::string &msg) {
    if (CHILD_VECTOR_OPTION::NORMAL_VECTOR == option)
        throw ConversionException("Cannot create a Vector " + msg);
    if (CHILD_VECTOR_OPTION::ANY_VECTOR == option)
        throw ConversionException("Cannot create an AnyVector " + msg);
    if (CHILD_VECTOR_OPTION::ARRAY_VECTOR == option)
        throw ConversionException("Cannot create an ArrayVector " + msg);
    if (CHILD_VECTOR_OPTION::SET_VECTOR == option)
        throw ConversionException("Cannot create a Set " + msg);
    throw ConversionException("Cannot create a Dictionary key " + msg);
}


#define CHECK_OUT_OF_BOUNDS_CHAR(val)       (val < SCHAR_MIN || val > SCHAR_MAX)
#define CHECK_OUT_OF_BOUNDS_SHORT(val)      (val < SHRT_MIN || val > SHRT_MAX)
#define CHECK_OUT_OF_BOUNDS_INT(val)        (val < INT_MIN || val > INT_MAX)
#define CHECK_OUT_OF_BOUNDS_LONG(val)       (val < LLONG_MIN || val > LLONG_MAX)


Constant* createNullConstant(Type type) {
    if (type.first == HT_UNK || type.first == HT_VOID)
        return new Void(true);
    return Util::createNullConstant((DATA_TYPE)type.first, EXPARM_VALUE(type.second));
}

Constant* createBool(char val) {
    return new Bool(val);
}
Constant* createChar(char val) {
    return new Char(val);
}
Constant* createShort(short val) {
    return new Short(val);
}
Constant* createInt(int val) {
    return new Int(val);
}
Constant* createLong(long long val) {
    return new Long(val);
}
Constant* createFloat(float val) {
    return new Float(val);
}
Constant* createDouble(double val) {
    return new Double(val);
}
Constant* createString(const std::string &val) {
    return new String(val);
}
Constant* createSymbol(const std::string &val) {
    Constant* tmp = Util::createConstant(DATA_TYPE::DT_SYMBOL);
    tmp->setString(val);
    return tmp;
}
Constant* createBlob(const std::string &val) {
    return new String(val, true);
}
Constant* createDate(int year, int month, int day) {
    return new Date(year, month, day);
}
Constant* createDate(int days) {
    return new Date(days);
}
Constant* createMonth(int year, int month) {
    return new Month(year, month);
}
Constant* createMonth(int months) {
    return new Month(months);
}
Constant* createNanoTime(int hour, int minute, int second, int nanosecond) {
    return new NanoTime(hour, minute, second, nanosecond);
}
Constant* createNanoTime(long long nanoseconds) {
    return new NanoTime(nanoseconds);
}
Constant* createTime(int hour, int minute, int second, int millisecond) {
    return new Time(hour, minute, second, millisecond);
}
Constant* createTime(int milliseconds) {
    return new Time(milliseconds);
}
Constant* createSecond(int hour, int minute, int second) {
    return new Second(hour, minute, second);
}
Constant* createSecond(int seconds) {
    return new Second(seconds);
}
Constant* createMinute(int hour, int minute) {
    return new Minute(hour, minute);
}
Constant* createMinute(int minutes) {
    return new Minute(minutes);
}
Constant* createNanoTimestamp(int year, int month, int day, int hour, int minute, int second, int nanosecond) {
    return new NanoTimestamp(year, month, day, hour, minute, second, nanosecond);
}
Constant* createNanoTimestamp(long long nanoseconds) {
    return new NanoTimestamp(nanoseconds);
}
Constant* createTimestamp(int year, int month, int day, int hour, int minute, int second, int millisecond) {
    return new Timestamp(year, month, day, hour, minute, second, millisecond);
}
Constant* createTimestamp(long long milliseconds) {
    return new Timestamp(milliseconds);
}
Constant* createDateTime(int year, int month, int day, int hour, int minute, int second) {
    return new DateTime(year, month, day, hour, minute, second);
}
Constant* createDateTime(int seconds) {
    return new DateTime(seconds);
}
Constant* createDateHour(int hours) {
    return new DateHour(hours);
}
Constant* createDateHour(int year, int month, int day, int hour) {
    return new DateHour(year, month, day, hour);
}

int _getDecimalPlaces(const char* value) {
    if (!value) {
        return 0;
    } else {
        const char* dot = std::strchr(value, '.');
        return (dot) ? std::strlen(dot + 1) : 0;
    }
}
Constant* createDecimal32(int scale, int value, bool isRaw) {
    if (isRaw) {
        if (value == std::numeric_limits<int>::min()) {
            return Util::createNullConstant(DATA_TYPE::DT_DECIMAL32, EXPARM_VALUE(scale));
        }
        return new Decimal32(EXPARM_VALUE(scale), value);
    }
    ConstantSP data = new Int(value);
    ConstantSP res = OperatorImp::asDecimal32(data, new Int(EXPARM_VALUE(scale)));
    return new Decimal32(scale, res->getDecimal32(scale));
}
Constant* createDecimal32(int scale, float value) {
    ConstantSP data = new Float(value);
    ConstantSP res = OperatorImp::asDecimal32(data, new Int(EXPARM_VALUE(scale)));
    return new Decimal32(scale, res->getDecimal32(scale));
}
Constant* createDecimal32(int scale, double value) {
    ConstantSP data = new Double(value);
    ConstantSP res = OperatorImp::asDecimal32(data, new Int(EXPARM_VALUE(scale)));
    return new Decimal32(scale, res->getDecimal32(scale));
}
Constant* createDecimal32(int scale, const char* value) {
    ConstantSP data = new String(value);
    ConstantSP res = OperatorImp::asDecimal32(data, new Int(EXPARM_VALUE(scale)));
    return new Decimal32(scale, res->getDecimal32(scale));
}
Constant* createDecimal64(int scale, long long value, bool isRaw) {
    if (isRaw) {
        if (value == std::numeric_limits<long long>::min()) {
            return Util::createNullConstant(DATA_TYPE::DT_DECIMAL64, EXPARM_VALUE(scale));
        }
        return new Decimal64(EXPARM_VALUE(scale), value);
    }
    ConstantSP data = new Long(value);
    ConstantSP res = OperatorImp::asDecimal64(data, new Int(EXPARM_VALUE(scale)));
    return new Decimal64(scale, res->getDecimal64(scale));
}
Constant* createDecimal64(int scale, float value) {
    ConstantSP data = new Float(value);
    ConstantSP res = OperatorImp::asDecimal64(data, new Int(EXPARM_VALUE(scale)));
    return new Decimal64(scale, res->getDecimal64(scale));
}
Constant* createDecimal64(int scale, double value) {
    ConstantSP data = new Double(value);
    ConstantSP res = OperatorImp::asDecimal64(data, new Int(EXPARM_VALUE(scale)));
    return new Decimal64(scale, res->getDecimal64(scale));
}
Constant* createDecimal64(int scale, const char* value) {
    ConstantSP data = new String(value);
    ConstantSP res = OperatorImp::asDecimal64(data, new Int(EXPARM_VALUE(scale)));
    return new Decimal64(scale, res->getDecimal64(scale));
}
Constant* createDecimal128(int scale, int128 value, bool isRaw) {
    if (isRaw) {
        if (value == std::numeric_limits<int128>::min()) {
            return Util::createNullConstant(DATA_TYPE::DT_DECIMAL128, EXPARM_VALUE(scale));
        }
        return new Decimal128(EXPARM_VALUE(scale), value);
    }
    value *= _decimal_util::exp10_i128(scale);
    return new Decimal128(EXPARM_VALUE(scale), value);
}
Constant* createDecimal128(int scale, float value) {
    ConstantSP data = new Float(value);
    ConstantSP res = OperatorImp::asDecimal128(data, new Int(EXPARM_VALUE(scale)));
    return new Decimal128(scale, res->getDecimal128(scale));
}
Constant* createDecimal128(int scale, double value) {
    ConstantSP data = new Double(value);
    ConstantSP res = OperatorImp::asDecimal128(data, new Int(EXPARM_VALUE(scale)));
    return new Decimal128(scale, res->getDecimal128(scale));
}
Constant* createDecimal128(int scale, const char* value) {
    ConstantSP data = new String(value);
    ConstantSP res = OperatorImp::asDecimal128(data, new Int(EXPARM_VALUE(scale)));
    return new Decimal128(scale, res->getDecimal128(scale));
}

Constant* createValue(Type type, int128 val, bool isRaw) {
    switch (type.first)
    {
    case HELPER_TYPE::HT_DECIMAL128:
        return createDecimal128(EXPARM_VALUE(type.second), val, isRaw);
    default:
        return NULL;
    }
}

Constant* createValue(Type type, long long val, bool isRaw) {
    switch (type.first)
    {
    case HELPER_TYPE::HT_LONG:
        return createLong(val);
    case HELPER_TYPE::HT_NANOTIME:
        return createNanoTime(val);
    case HELPER_TYPE::HT_NANOTIMESTAMP:
        return createNanoTimestamp(val);
    case HELPER_TYPE::HT_TIMESTAMP:
        return createTimestamp(val);
    case HELPER_TYPE::HT_DECIMAL64: {
        return createDecimal64(EXPARM_VALUE(type.second), val, isRaw);
    }
    default:
        return NULL;
    }
}

Constant* createValue(Type type, int val, bool isRaw) {
    switch (type.first)
    {
    case HELPER_TYPE::HT_INT:
        return createInt(val);
    case HELPER_TYPE::HT_DATE:
        return createDate(val);
    case HELPER_TYPE::HT_MONTH:
        return createMonth(val);
    case HELPER_TYPE::HT_TIME:
        return createTime(val);
    case HELPER_TYPE::HT_SECOND:
        return createSecond(val);
    case HELPER_TYPE::HT_MINUTE:
        return createMinute(val);
    case HELPER_TYPE::HT_DATETIME:
        return createDateTime(val);
    case HELPER_TYPE::HT_DATEHOUR:
        return createDateHour(val);
    case HELPER_TYPE::HT_DECIMAL32: {
        return createDecimal32(EXPARM_VALUE(type.second), val, isRaw);
    }
    default:
        return NULL;
    }
}

Constant* createValue(Type type, short val, bool isRaw) {
    std::ignore = isRaw;
    switch (type.first)
    {
    case HELPER_TYPE::HT_SHORT:
        return createShort(val);
    default:
        return NULL;
    }
}

Constant* createValue(Type type, char val, bool isRaw) {
    std::ignore = isRaw;
    switch (type.first)
    {
    case HELPER_TYPE::HT_BOOL:
        return createBool(val);
    case HELPER_TYPE::HT_CHAR:
        return createChar(val);
    default:
        return NULL;
    }
}

Constant* createValue(Type type, double val, bool checkDecimal) {
    switch (type.first)
    {
    case HELPER_TYPE::HT_DOUBLE:
        return createDouble(val);
    case HELPER_TYPE::HT_DECIMAL32: {
        if (!checkDecimal) return NULL;
        return createDecimal32(type.second, val);
    }
    case HELPER_TYPE::HT_DECIMAL64: {
        if (!checkDecimal) return NULL;
        return createDecimal64(type.second, val);
    }
    case HELPER_TYPE::HT_DECIMAL128: {
        if (!checkDecimal) return NULL;
        return createDecimal128(type.second, val);
    }
    default:
        return NULL;
    }
}

Constant* createValue(Type type, float val, bool checkDecimal) {
    switch (type.first)
    {
    case HELPER_TYPE::HT_FLOAT:
        return createFloat(val);
    case HELPER_TYPE::HT_DECIMAL32: {
        if (!checkDecimal) return NULL;
        return createDecimal32(type.second, val);
    }
    case HELPER_TYPE::HT_DECIMAL64: {
        if (!checkDecimal) return NULL;
        return createDecimal64(type.second, val);
    }
    case HELPER_TYPE::HT_DECIMAL128: {
        if (!checkDecimal) return NULL;
        return createDecimal128(type.second, val);
    }
    default:
        return NULL;
    }
}


ConstantSP createObject(Type type, const char* name, std::nullptr_t val, ConvertErrorInfo *error, bool isRaw) {
    std::ignore = name;
    std::ignore = val;
    std::ignore = error;
    std::ignore = isRaw;
    return createNullConstant(type);
}

ConstantSP createObject(Type type, const char* name, Constant* val, ConvertErrorInfo *error, bool isRaw) {
    std::ignore = type;
    std::ignore = name;
    std::ignore = error;
    std::ignore = isRaw;
    return val;
}

ConstantSP createObject(Type type, const char* name, ConstantSP val, ConvertErrorInfo *error, bool isRaw) {
    std::ignore = type;
    std::ignore = name;
    std::ignore = error;
    std::ignore = isRaw;
    return val;
}

ConstantSP createObject(Type type, const char* name, bool val, ConvertErrorInfo *error, bool isRaw) {
    std::ignore = isRaw;
    switch (type.first)
    {
    case HELPER_TYPE::HT_BOOL:
        return createBool(val);
    default:
        ERROR_INVALIDTARGET(name, type, error);
    }
    return NULL;
}

ConstantSP createObject(Type type, const char* name, char val, ConvertErrorInfo *error, bool isRaw) {
    Constant* tmp;
    tmp = createValue(type, val, isRaw);
    if (tmp != nullptr) return tmp;

    tmp = createValue(type, (short)val, isRaw);
    if (tmp != nullptr) return tmp;

    tmp = createValue(type, (int)val, isRaw);
    if (tmp != nullptr) return tmp;

    tmp = createValue(type, (long long)val, isRaw);
    if (tmp != nullptr) return tmp;

    tmp = createValue(type, (int128)val, isRaw);
    if (tmp != nullptr) return tmp;

    tmp = createValue(type, static_cast<float>(val));
    if (tmp != nullptr) return tmp;

    tmp = createValue(type, static_cast<double>(val));
    if (tmp != nullptr) return tmp;

    ERROR_INVALIDTARGET(name, type, error);
    return NULL;
}

ConstantSP createObject(Type type, const char* name, short val, ConvertErrorInfo *error, bool isRaw) {
    Constant* tmp;
    tmp = createValue(type, val, isRaw);
    if (tmp != nullptr) return tmp;

    tmp = createValue(type, (int)val, isRaw);
    if (tmp != nullptr) return tmp;

    tmp = createValue(type, (long long)val, isRaw);
    if (tmp != nullptr) return tmp;

    tmp = createValue(type, (int128)val, isRaw);
    if (tmp != nullptr) return tmp;

    tmp = createValue(type, static_cast<float>(val));
    if (tmp != nullptr) return tmp;

    tmp = createValue(type, static_cast<double>(val));
    if (tmp != nullptr) return tmp;

    if (CHECK_OUT_OF_BOUNDS_CHAR(val))
        ERROR_INVALIDDATA_EXCEED_RANGE(name, type, error);
    tmp = createValue(type, static_cast<char>(val), isRaw);
    if (tmp != nullptr) return tmp;

    ERROR_INVALIDTARGET(name, type, error);
    return NULL;
}

ConstantSP createObject(Type type, const char* name, int val, ConvertErrorInfo *error, bool isRaw) {
    Constant* tmp;
    tmp = createValue(type, val, isRaw);
    if (tmp != nullptr) return tmp;

    tmp = createValue(type, (long long)val, isRaw);
    if (tmp != nullptr) return tmp;
    
    tmp = createValue(type, (int128)val, isRaw);
    if (tmp != nullptr) return tmp;
    
    tmp = createValue(type, static_cast<float>(val));
    if (tmp != nullptr) return tmp;

    tmp = createValue(type, static_cast<double>(val));
    if (tmp != nullptr) return tmp;

    if (CHECK_OUT_OF_BOUNDS_SHORT(val)) {
        ERROR_INVALIDDATA_EXCEED_RANGE(name, type, error);
    }
    tmp = createValue(type, static_cast<short>(val), isRaw);
    if (tmp != nullptr) return tmp;

    if (CHECK_OUT_OF_BOUNDS_CHAR(val)) {
        ERROR_INVALIDDATA_EXCEED_RANGE(name, type, error);
    }
    tmp = createValue(type, static_cast<char>(val), isRaw);
    if (tmp != nullptr) return tmp;

    ERROR_INVALIDTARGET(name, type, error);
    return NULL;
}

ConstantSP createObject(Type type, const char* name, long long val, ConvertErrorInfo *error, bool isRaw) {
    Constant* tmp;
    tmp = createValue(type, val, isRaw);
    if (tmp != nullptr) return tmp;

    tmp = createValue(type, (int128)val, isRaw);
    if (tmp != nullptr) return tmp;
    
    tmp = createValue(type, static_cast<float>(val));
    if (tmp != nullptr) return tmp;

    tmp = createValue(type, static_cast<double>(val));
    if (tmp != nullptr) return tmp;

    if (CHECK_OUT_OF_BOUNDS_INT(val)) {
        ERROR_INVALIDDATA_EXCEED_RANGE(name, type, error);
    }
    tmp = createValue(type, static_cast<int>(val), isRaw);
    if (tmp != nullptr) return tmp;

    if (CHECK_OUT_OF_BOUNDS_SHORT(val)) {
        ERROR_INVALIDDATA_EXCEED_RANGE(name, type, error);
    }
    tmp = createValue(type, static_cast<short>(val), isRaw);
    if (tmp != nullptr) return tmp;

    if (CHECK_OUT_OF_BOUNDS_CHAR(val)) {
        ERROR_INVALIDDATA_EXCEED_RANGE(name, type, error);
    }
    tmp = createValue(type, static_cast<char>(val), isRaw);
    if (tmp != nullptr) return tmp;

    ERROR_INVALIDTARGET(name, type, error);
    return NULL;
}

ConstantSP createObject(Type type, const char* name, int128 val, ConvertErrorInfo *error, bool isRaw) {
    Constant* tmp;
    tmp = createValue(type, val, isRaw);
    if (tmp != nullptr) return tmp;

    tmp = createValue(type, static_cast<float>(val));
    if (tmp != nullptr) return tmp;

    tmp = createValue(type, static_cast<double>(val));
    if (tmp != nullptr) return tmp;

    if (CHECK_OUT_OF_BOUNDS_LONG(val)) {
        ERROR_INVALIDDATA_EXCEED_RANGE(name, type, error);
    }
    tmp = createValue(type, static_cast<long long>(val), isRaw);
    if (tmp != nullptr) return tmp;

    if (CHECK_OUT_OF_BOUNDS_INT(val)) {
        ERROR_INVALIDDATA_EXCEED_RANGE(name, type, error);
    }
    tmp = createValue(type, static_cast<int>(val), isRaw);
    if (tmp != nullptr) return tmp;

    if (CHECK_OUT_OF_BOUNDS_SHORT(val)) {
        ERROR_INVALIDDATA_EXCEED_RANGE(name, type, error);
    }
    tmp = createValue(type, static_cast<short>(val), isRaw);
    if (tmp != nullptr) return tmp;

    if (CHECK_OUT_OF_BOUNDS_CHAR(val)) {
        ERROR_INVALIDDATA_EXCEED_RANGE(name, type, error);
    }
    tmp = createValue(type, static_cast<char>(val), isRaw);
    if (tmp != nullptr) return tmp;

    ERROR_INVALIDTARGET(name, type, error);
    return NULL;
}

ConstantSP createObject(Type type, const char* name, float val, ConvertErrorInfo *error, bool isRaw) {
    std::ignore = isRaw;
    if (ISNUMPYNULL_FLOAT32(val)) {
        return createNullConstant(type);
    }
    Constant* tmp;
    tmp = createValue(type, val, true);
    if (tmp != nullptr) return tmp;

    tmp = createValue(type, static_cast<double>(val), true);
    if (tmp != nullptr) return tmp;

    ERROR_INVALIDTARGET(name, type, error);
    return NULL;
}

ConstantSP createObject(Type type, const char* name, double val, ConvertErrorInfo *error, bool isRaw) {
    std::ignore = isRaw;
    if (ISNUMPYNULL_FLOAT64(val)) {
        return createNullConstant(type);
    }
    Constant* tmp;
    tmp = createValue(type, val, true);
    if (tmp != nullptr) return tmp;

    tmp = createValue(type, static_cast<float>(val), true);
    if (tmp != nullptr) return tmp;

    ERROR_INVALIDTARGET(name, type, error);
    return NULL;
}

ConstantSP createObject(Type type, const char* name, const char * val, ConvertErrorInfo *error, bool isRaw) {
    std::ignore = isRaw;
    if (val == (const char*) 0) {
        return createNullConstant(type);
    }
    size_t valLen = strlen(val);
    if (valLen == 0)
        return createNullConstant(type);
    switch (type.first)
    {
    case HELPER_TYPE::HT_CHAR: {
        if (valLen != 1)
            ERROR_INVALIDDATA(name, type, error);
        return createChar(val[0]);
    }
    case HELPER_TYPE::HT_DATE: {
        Date *pdate = Date::parseDate(val);
        if (pdate == nullptr)
            ERROR_INVALIDDATA(name, type, error);
        return pdate;
    }
    case HELPER_TYPE::HT_MONTH: {
        Month *pmonth = Month::parseMonth(val);
        if (pmonth == nullptr)
            ERROR_INVALIDDATA(name, type, error);
        return pmonth;
    }
    case HELPER_TYPE::HT_TIME: {
        Time *ptime = Time::parseTime(val);
        if (ptime == nullptr)
            ERROR_INVALIDDATA(name, type, error);
        return ptime;
    }
    case HELPER_TYPE::HT_MINUTE: {
        Minute *pminute = Minute::parseMinute(val);
        if (pminute == nullptr)
            ERROR_INVALIDDATA(name, type, error);
        return pminute;
    }
    case HELPER_TYPE::HT_SECOND: {
        Second *psecond = Second::parseSecond(val);
        if (psecond == nullptr)
            ERROR_INVALIDDATA(name, type, error);
        return psecond;
    }
    case HELPER_TYPE::HT_DATETIME: {
        DateTime *pdatetime = DateTime::parseDateTime(val);
        if (pdatetime == nullptr)
            ERROR_INVALIDDATA(name, type, error);
        return pdatetime;
    }
    case HELPER_TYPE::HT_TIMESTAMP: {
        Timestamp *ptimestamp = Timestamp::parseTimestamp(val);
        if (ptimestamp == nullptr)
            ERROR_INVALIDDATA(name, type, error);
        return ptimestamp;
    }
    case HELPER_TYPE::HT_NANOTIME: {
        NanoTime *pnanotime = NanoTime::parseNanoTime(val);
        if (pnanotime == nullptr)
            ERROR_INVALIDDATA(name, type, error);
        return pnanotime;
    }
    case HELPER_TYPE::HT_NANOTIMESTAMP: {
        NanoTimestamp *pnanotimestamp = NanoTimestamp::parseNanoTimestamp(val);
        if (pnanotimestamp == nullptr)
            ERROR_INVALIDDATA(name, type, error);
        return pnanotimestamp;
    }
    case HELPER_TYPE::HT_DATEHOUR: {
        DateHour *pdatehour = DateHour::parseDateHour(val);
        if (pdatehour == nullptr)
            ERROR_INVALIDDATA(name, type, error);
        return pdatehour;
    }
    case HELPER_TYPE::HT_INT128: {
        Int128 *pint128 = Int128::parseInt128(val, valLen);
        if (pint128 == nullptr)
            ERROR_INVALIDDATA(name, type, error);
        return pint128;
    }
    case HELPER_TYPE::HT_UUID: {
        Uuid *puuid = Uuid::parseUuid(val, valLen);
        if (puuid == nullptr)
            ERROR_INVALIDDATA(name, type, error);
        return puuid;
    }
    case HELPER_TYPE::HT_IP: {
        IPAddr *pip = IPAddr::parseIPAddr(val, valLen);
        if (pip == nullptr)
            ERROR_INVALIDDATA(name, type, error);
        return pip;
    }
    case HELPER_TYPE::HT_STRING: {
        return createString(val);
    }
    case HELPER_TYPE::HT_SYMBOL: {
        return createSymbol(val);
    }
    case HELPER_TYPE::HT_BLOB: {
        return createBlob(val);
    }
    case HELPER_TYPE::HT_DURATION: {
        Duration* pduration = Duration::parseDuration(val);
        if (pduration == nullptr)
            ERROR_INVALIDDATA(name, type, error);
        return pduration;
    }
    case HELPER_TYPE::HT_DECIMAL32: {
        return createDecimal32(type.second, val);
    }
    case HELPER_TYPE::HT_DECIMAL64: {
        return createDecimal64(type.second, val);
    }
    case HELPER_TYPE::HT_DECIMAL128: {
        return createDecimal128(type.second, val);
    }
    default:
        ERROR_INVALIDTARGET(name, type, error);
    }
    return NULL;
}

ConstantSP createObject(Type type, const char* name, const std::string & val, ConvertErrorInfo *error, bool isRaw) {
    std::ignore = isRaw;
    return createObject(type, name, (const char *)val.data(), error);
}


py::object convertTemporalToNumpy(const ConstantSP &obj) {
    Type type = createType(obj);
    switch (type.first) {
        case HT_DATE:
            return PyObjs::cache_->np_datetime64_type_(obj->getLong(), "D");
        case HT_MONTH:
            return PyObjs::cache_->np_datetime64_type_(obj->getLong() - 23640, "M");
        case HT_TIME:
            return PyObjs::cache_->np_datetime64_type_(obj->getLong(), "ms");
        case HT_MINUTE:
            return PyObjs::cache_->np_datetime64_type_(obj->getLong(), "m");
        case HT_SECOND:
            return PyObjs::cache_->np_datetime64_type_(obj->getLong(), "s");
        case HT_DATETIME:
            return PyObjs::cache_->np_datetime64_type_(obj->getLong(), "s");
        case HT_DATEHOUR:
            return PyObjs::cache_->np_datetime64_type_(obj->getLong(), "h");
        case HT_TIMESTAMP:
            return PyObjs::cache_->np_datetime64_type_(obj->getLong(), "ms");
        case HT_NANOTIME:
            return PyObjs::cache_->np_datetime64_type_(obj->getLong(), "ns");
        case HT_NANOTIMESTAMP:
            return PyObjs::cache_->np_datetime64_type_(obj->getLong(), "ns");
        default:
            return py::none();
    }
}


py::object convertTemporalToPython(const ConstantSP &obj) {
    Type type = createType(obj);
    switch (type.first)
    {
    case HT_DATE: {
        /**
         * 2012.01.01 -> datetime.date(2012, 1, 1)
        */
        if (obj->isNull()) return py::none();
        int days = obj->getInt();
        if (days < -719162) return py::int_(days);
        return converter::PyObjs::cache_->py_date_(1970, 1, 1) + converter::PyObjs::cache_->py_timedelta_("days"_a=days);
    }
    case HT_MONTH: {
        /**
         * 2012.01M -> datetime.date(2012, 1, 1)
        */
        if (obj->isNull()) return py::none();
        int months = obj->getInt();
        int m_year = months / 12;
        int m_month = months % 12 + 1;
        int m_day = 1;
        if (m_year < 1) return py::int_(months);
        return converter::PyObjs::cache_->py_date_(m_year, m_month, m_day);
    }
    case HT_TIME: {
        /**
         * 13:30:10.008 -> datetime.time(13, 30, 10, 8000)
        */
        if (obj->isNull()) return py::none();
        int milliseconds = obj->getInt();
        int m_ms = milliseconds % 1000;
        milliseconds = milliseconds / 1000;
        int m_s = milliseconds % 60;
        milliseconds = milliseconds / 60;
        int m_m = milliseconds % 60;
        int m_h = milliseconds / 60;
        return converter::PyObjs::cache_->py_time_(m_h, m_m, m_s, m_ms * 1000);
    }
    case HT_MINUTE: {
        /**
         * 13:30m -> datetime.time(13, 30, 0, 0)
        */
        if (obj->isNull()) return py::none();
        int minutes = obj->getInt();
        return converter::PyObjs::cache_->py_time_(minutes / 60, minutes % 60, 0, 0);
    }
    case HT_SECOND: {
        /**
         * 13:30:10 -> datetime.time(13, 30, 10, 0)
        */
        if (obj->isNull()) return py::none();
        int seconds = obj->getInt();
        int minutes = seconds / 60;
        int hour = minutes / 60;
        return converter::PyObjs::cache_->py_time_(hour, minutes % 60, seconds % 60, 0);
    }
    case HT_DATETIME: {
        /**
         * 2012.06.13T13:30:10 -> datetime.datetime(2012, 6, 13, 13, 30, 10)
        */
        // int days = val_->getInt();
        // if (days < -719162) return py::int_(days);
        // return converter::PyObjs::cache_->py_date_(1970, 1, 1) + converter::PyObjs::cache_->py_timedelta_("days"_a=days);
        if (obj->isNull()) return py::none();
        int seconds = obj->getInt();
        int days = seconds / (24*60*60);
        // if (days < -719162) return py::int_(seconds);
        // DATETIME: 4B INT
        return converter::PyObjs::cache_->py_datetime_(1970, 1, 1) + converter::PyObjs::cache_->py_timedelta_("days"_a=days, "seconds"_a=seconds % (24*60*60));
    }
    case HT_TIMESTAMP: {
        /**
         * 2012.06.13T13:30:10.1213 -> datetime.datetime(2012, 6, 13, 13, 30, 10, 123000)
        */
        // int days = val_->getInt();
        // if (days < -719162) return py::int_(days);
        // return converter::PyObjs::cache_->py_date_(1970, 1, 1) + converter::PyObjs::cache_->py_timedelta_("days"_a=days);
        if (obj->isNull()) return py::none();
        long long milliseconds = obj->getLong();
        // long long seconds = milliseconds / 1000;
        // long long days = seconds / (24*60*60);
        
        // if (days < -719162) return py::int_(seconds);
        // DATETIME: 4B INT
        return converter::PyObjs::cache_->py_datetime_(1970, 1, 1) + \
               converter::PyObjs::cache_->py_timedelta_("microseconds"_a = milliseconds * 1000);
    }
    case HT_NANOTIME: {
        /**
         * 12:34:56.123000456 -> 45296123000456
        */
        // int days = val_->getInt();
        // if (days < -719162) return py::int_(days);
        // return converter::PyObjs::cache_->py_date_(1970, 1, 1) + converter::PyObjs::cache_->py_timedelta_("days"_a=days);
        if (obj->isNull()) return py::none();
        long long data = obj->getLong();
        return py::int_(data);
    }
    case HT_NANOTIMESTAMP: {
        /**
         * 2001.06.13T13:30:10.123456789 -> 992439010123456789
        */
        // int days = val_->getInt();
        // if (days < -719162) return py::int_(days);
        // return converter::PyObjs::cache_->py_date_(1970, 1, 1) + converter::PyObjs::cache_->py_timedelta_("days"_a=days);
        if (obj->isNull()) return py::none();
        long long data = obj->getLong();
        return py::int_(data);
    }
    case HT_DATEHOUR: {
        /**
         * 2012.06.13T13 -> datetime.datetime(2012, 6, 13, 13, 0, 0)
        */
        // int days = val_->getInt();
        // if (days < -719162) return py::int_(days);
        // return converter::PyObjs::cache_->py_date_(1970, 1, 1) + converter::PyObjs::cache_->py_timedelta_("days"_a=days);
        if (obj->isNull()) return py::none();
        int hours = obj->getInt();
        int days = hours / 24;
        if (days == -719162 && hours != 0) return py::int_(hours);
        if (days < -719162) return py::int_(hours);
        return converter::PyObjs::cache_->py_datetime_(1970, 1, 1) + converter::PyObjs::cache_->py_timedelta_("hours"_a=hours);
    }
    default:
        throw ConversionException("Unsupported conversion from " + getDataTypeString(type) + " to python datetime.datetime/time/date.");
    }
}





Constant* _createTemporalWithTMAndMicrosecond(const std::tm &cal, const int &msecs, const Type &type) {
    switch (type.first)
    {
    case HT_DATE: {
        return createDate(cal.tm_year, cal.tm_mon, cal.tm_mday);
    }
    case HT_MONTH: {
        if (cal.tm_year == 0 && cal.tm_mon == 0) return createNullConstant(type);
        return createMonth(cal.tm_year, cal.tm_mon);
    }
    case HT_TIME: {
        return createTime(cal.tm_hour, cal.tm_min, cal.tm_sec, msecs / 1000);
    }
    case HT_MINUTE: {
        return createMinute(cal.tm_hour, cal.tm_min);
    }
    case HT_SECOND: {
        return createSecond(cal.tm_hour, cal.tm_min, cal.tm_sec);
    }
    case HT_DATETIME: {
        return createDateTime(cal.tm_year, cal.tm_mon, cal.tm_mday, cal.tm_hour, cal.tm_min, cal.tm_sec);
    }
    case HT_TIMESTAMP: {
        return createTimestamp(cal.tm_year, cal.tm_mon, cal.tm_mday, cal.tm_hour, cal.tm_min, cal.tm_sec, msecs / 1000);
    }
    case HT_NANOTIME: {
        return createNanoTime(cal.tm_hour, cal.tm_min, cal.tm_sec, msecs * 1000);
    }
    case HT_NANOTIMESTAMP: {
        return createNanoTimestamp(cal.tm_year, cal.tm_mon, cal.tm_mday, cal.tm_hour, cal.tm_min, cal.tm_sec, msecs * 1000);
    }
    case HT_DATEHOUR: {
        return createDateHour(cal.tm_year, cal.tm_mon, cal.tm_mday, cal.tm_hour);
    }
    default:
        return nullptr;
    }
}


ConstantSP convertTemporalFromPyDateTime(const py::handle &obj, Type type) {
    std::tm cal;
    int msecs;

    cal.tm_sec = PyDateTime_DATE_GET_SECOND(obj.ptr());
    cal.tm_min = PyDateTime_DATE_GET_MINUTE(obj.ptr());
    cal.tm_hour = PyDateTime_DATE_GET_HOUR(obj.ptr());
    cal.tm_mday = PyDateTime_GET_DAY(obj.ptr());
    cal.tm_mon = PyDateTime_GET_MONTH(obj.ptr());
    cal.tm_year = PyDateTime_GET_YEAR(obj.ptr());
    cal.tm_isdst = -1;
    msecs = PyDateTime_DATE_GET_MICROSECOND(obj.ptr());

    Constant* res = _createTemporalWithTMAndMicrosecond(cal, msecs, type);
    if (nullptr == res) {
        throw ConversionException("Unsupported conversion from python datetime.datetime to " + getDataTypeString(type));
    }
    return res;
}


ConstantSP convertTemporalFromPyDate(const py::handle &obj, Type type) {
    std::tm cal;
    int msecs;

    cal.tm_sec = 0;
    cal.tm_min = 0;
    cal.tm_hour = 0;
    cal.tm_mday = PyDateTime_GET_DAY(obj.ptr());
    cal.tm_mon = PyDateTime_GET_MONTH(obj.ptr());
    cal.tm_year = PyDateTime_GET_YEAR(obj.ptr());
    cal.tm_isdst = -1;
    msecs = 0;

    Constant* res = _createTemporalWithTMAndMicrosecond(cal, msecs, type);
    if (nullptr == res) {
        throw ConversionException("Unsupported conversion from python datetime.date to " + getDataTypeString(type));
    }
    return res;
}


ConstantSP convertTemporalFromPyTime(const py::handle &obj, Type type) {
    std::tm cal;
    int msecs;

    cal.tm_sec = PyDateTime_TIME_GET_SECOND(obj.ptr());
    cal.tm_min = PyDateTime_TIME_GET_MINUTE(obj.ptr());
    cal.tm_hour = PyDateTime_TIME_GET_HOUR(obj.ptr());
    cal.tm_mday = 1;  // This date (day, month, year) = (1, 0, 1970)
    cal.tm_mon = 0;   // represents 1-Jan-1970, which is the first
    cal.tm_year = 0; // earliest available date for Python's datetime
    cal.tm_isdst = -1;
    msecs = PyDateTime_TIME_GET_MICROSECOND(obj.ptr());

    Constant* res = _createTemporalWithTMAndMicrosecond(cal, msecs, type);
    if (nullptr == res) {
        throw ConversionException("Unsupported conversion from python datetime.time to " + getDataTypeString(type));
    }
    return res;
}


ConstantSP castTemporal(const ConstantSP &obj, Type type) {
    if (type.first == HT_UNK) return obj;
    Type objType = createType(obj);
    if (objType == type) return obj;
    return OperatorImp::cast(obj, packDataTypeEnum(type));
}


ConstantSP castTemporal(const ConstantSP &obj, Type type, bool &changed) {
    if (type.first == HT_UNK) return obj;
    Type objType = createType(obj);
    if (objType == type) return obj;
    changed = true;
    return OperatorImp::cast(obj, packDataTypeEnum(type));
}


bool checkAnyVector(const ConstantSP &obj) {
    return obj->getType() == DATA_TYPE::DT_ANY;
}


bool checkArrayVector(const ConstantSP &obj) {
    return obj->getCategory() == DATA_CATEGORY::ARRAY;
}


Vector* createVector(Type type, INDEX size, INDEX capacity) {
    if (type.first == HT_UNK) throw ConversionException("Vector creation requires a specific type.");
    return Util::createVector((DATA_TYPE)type.first, size, capacity, true, EXPARM_VALUE(type.second));
}

Vector* createArrayVector(Type type, INDEX size, INDEX valueSize, INDEX capacity, INDEX valueCapacity) {
    if (type.first == HT_UNK) throw ConversionException("ArrayVector creation requires a specific type.");
    return Util::createArrayVector((DATA_TYPE)type.first, size, valueSize, capacity, valueCapacity, true, EXPARM_VALUE(type.second));
}

Vector* createIndexVector(INDEX start, INDEX length) {
    return Util::createIndexVector(start, length);
}

Vector* createIndexVector(INDEX length, bool arrayOnly) {
    return Util::createIndexVector(length, arrayOnly);
}

Vector* createPair(const ConstantSP &a, const ConstantSP &b) {
    Type type = createType(a);
    if (!canConvertTo(type, createType(b), type)) {
        throw ConversionException("Both elements of a Pair must be of the same or compatible types.");
    }
    Vector* pair = createVector(type, 2, 2);
    pair->set(0, a);
    pair->set(1, b);
    pair->setForm(DATA_FORM::DF_PAIR);
    return pair;
}

ConstantSP createArrayVectorWithIndexAndValue(const ConstantSP &index, const ConstantSP &value) {
    return OperatorImp::arrayVector(index, value);
}

Vector* createAllNullVector(Type type, INDEX size, INDEX capacity) {
    if (capacity < size) capacity = size;
    if (type.first == HT_UNK) throw ConversionException("Vector creation requires a specific type.");
    Vector* tmp = Util::createVector((DATA_TYPE)type.first, size, capacity, true, EXPARM_VALUE(type.second));
    tmp->fill(0, size, getConstantSP_NULL());
    return tmp;
}

Vector* createMatrix(Type type, INDEX cols, INDEX rows, INDEX colCapacity) {
    if (colCapacity < cols) colCapacity = cols;
    if (type.first == HT_UNK) throw ConversionException("Matrix creation requires a specific type.");
    return Util::createMatrix((DATA_TYPE)type.first, cols, rows, colCapacity, EXPARM_VALUE(type.second));
}

Vector* createMatrixWithVector(ConstantSP ddbVec, INDEX cols, INDEX rows) {
    Vector* matrix = createMatrix({(HELPER_TYPE)ddbVec->getType(), ddbVec->getExtraParamForType()}, cols, rows, cols);
    matrix->set(0, 0, ddbVec);
    return matrix;
}

Vector* createMatrixWithVectors(const std::vector<ConstantSP> &columns, Type type, INDEX cols, INDEX rows) {
    Vector* matrix = createMatrix(type, cols, rows, cols);
    for (int i = 0; i < cols; ++i) {
        matrix->set(i, 0, columns[i]);
    }
    return matrix;
}

Vector* createAllNullMatrix(Type type, INDEX cols, INDEX rows, INDEX colCapacity) {
    Vector* tmp = createMatrix(type, cols, rows, colCapacity);
    tmp->fill(0, rows * cols, getConstantSP_NULL());
    return tmp;
}

Set* createSet(const ConstantSP& val) {
    VectorSP vec = val;
    Set* set = createSet(createType(vec), vec->size());
    if (set == nullptr) {
        throw ConversionException("Cannot create a Set with data type " + getDataTypeString(createType(vec)) + ".");
    }
    for (int i = 0; i < vec->size(); ++i) {
        set->append(vec->get(i));
    }
    return set;
}

Set* createSet(Type type, size_t capacity) {
    return Util::createSet((DATA_TYPE)type.first, nullptr, capacity);
}

Dictionary* createDictionary(const ConstantSP& key, const ConstantSP& val, bool isOrdered) {
    Dictionary* dict = createDictionary(createType(key), createType(val), isOrdered);
    dict->set(key, val);
    return dict;
}

Dictionary* createDictionary(Type keyType, Type valType, bool isOrdered) {
    if (keyType.first == HT_UNK || valType.first == HT_UNK)
        throw ConversionException("Cannot create a Dictionary with keys or values of unknown type.");
    return Util::createDictionary((DATA_TYPE)keyType.first, nullptr, (DATA_TYPE)valType.first, nullptr, isOrdered, EXPARM_VALUE(keyType.second), EXPARM_VALUE(valType.second));
}

Table* createTable(const std::vector<std::string> &colNames, const std::vector<ConstantSP> &cols) {
    return Util::createTable(colNames, cols);
}

Table* createTable(const std::vector<std::string> &colNames, const std::vector<Type> &colTypes, INDEX size, INDEX capacity) {
    if (capacity < size) capacity = size;
    std::vector<ConstantSP> columns;
    columns.reserve(colTypes.size());
    for (auto &type : colTypes) {
        if ((int)(type.first) >= ARRAY_TYPE_BASE) {
            if (size != 0) {
                throw ConversionException("When a table contains an ArrayVector column, the initial size must be 0.");
            }
            columns.push_back(createArrayVector(type, size, size, capacity, capacity));
        }
        else {
            columns.push_back(createVector(type, size, capacity));
        }
        columns.back()->setNullFlag(columns.back()->hasNull());
    }
    return Util::createTable(colNames, columns);
}

Table* createTable(const TableChecker &types, INDEX size, INDEX capacity) {
    size_t len = types.size();
    std::vector<std::string> colNames;
    colNames.reserve(len);
    std::vector<Type> colTypes;
    colTypes.reserve(len);
    for (const auto &it : types) {
        colNames.emplace_back(it.first);
        colTypes.emplace_back(it.second);
    }
    return createTable(colNames, colTypes, size, capacity);
}


void VectorFillNull(VectorSP &ddbVec, INDEX start, INDEX len) {
    ddbVec->fill(start, len, getConstantSP_NULL());
}


void VectorFillConstant(VectorSP &ddbVec, INDEX start, INDEX len, const ConstantSP &value) {
    ddbVec->fill(start, len, value);
}


bool VectorAppendChar(VectorSP &ddbVec, char *buf, int len) {
    int oldLen = ddbVec->size();
    if (!ddbVec->appendChar(buf, len)) return false;
    ddbVec->setNullFlag(ddbVec->getNullFlag() || ddbVec->hasNull(oldLen, len));
    return true;
}
bool VectorAppendShort(VectorSP &ddbVec, short *buf, int len) {
    int oldLen = ddbVec->size();
    if (!ddbVec->appendShort(buf, len)) return false;
    ddbVec->setNullFlag(ddbVec->getNullFlag() || ddbVec->hasNull(oldLen, len));
    return true;
}
bool VectorAppendInt(VectorSP &ddbVec, int *buf, int len) {
    int oldLen = ddbVec->size();
    if (!ddbVec->appendInt(buf, len)) return false;
    ddbVec->setNullFlag(ddbVec->getNullFlag() || ddbVec->hasNull(oldLen, len));
    return true;
}
bool VectorAppendLong(VectorSP &ddbVec, long long *buf, int len) {
    int oldLen = ddbVec->size();
    if (!ddbVec->appendLong(buf, len)) return false;
    ddbVec->setNullFlag(ddbVec->getNullFlag() || ddbVec->hasNull(oldLen, len));
    return true;
}
bool VectorAppendFloat(VectorSP &ddbVec, float *buf, int len) {
    int oldLen = ddbVec->size();
    if (!ddbVec->appendFloat(buf, len)) return false;
    ddbVec->setNullFlag(ddbVec->getNullFlag() || ddbVec->hasNull(oldLen, len));
    return true;
}
bool VectorAppendDouble(VectorSP &ddbVec, double *buf, int len) {
    int oldLen = ddbVec->size();
    if (!ddbVec->appendDouble(buf, len)) return false;
    ddbVec->setNullFlag(ddbVec->getNullFlag() || ddbVec->hasNull(oldLen, len));
    return true;
}
void VectorAppendDecimal32(VectorSP &ddbVec, int *buf, int len, int scale) {
    int *w_buf;
    std::vector<int> tmp(len);
    INDEX start_p = ddbVec->size();
    ddbVec->resize(start_p + len);
    w_buf = ddbVec->getDecimal32Buffer(start_p, len, scale, tmp.data());
    memcpy(w_buf, buf, len * sizeof(int));
    ddbVec->setDecimal32(start_p, len, scale, w_buf);
    ddbVec->setNullFlag(ddbVec->getNullFlag() || ddbVec->hasNull(start_p, len));
}
void VectorAppendDecimal64(VectorSP &ddbVec, long long *buf, int len, int scale) {
    long long *w_buf;
    std::vector<long long> tmp(len);
    INDEX start_p = ddbVec->size();
    ddbVec->resize(start_p + len);
    w_buf = ddbVec->getDecimal64Buffer(start_p, len, scale, tmp.data());
    memcpy(w_buf, buf, len * sizeof(long long));
    ddbVec->setDecimal64(start_p, len, scale, w_buf);
    ddbVec->setNullFlag(ddbVec->getNullFlag() || ddbVec->hasNull(start_p, len));
}
void VectorAppendDecimal128(VectorSP &ddbVec, int128 *buf, int len, int scale) {
    int128 *w_buf;
    std::vector<int128> tmp(len);
    INDEX start_p = ddbVec->size();
    ddbVec->resize(start_p + len);
    w_buf = ddbVec->getDecimal128Buffer(start_p, len, scale, tmp.data());
    memcpy(w_buf, buf, len * sizeof(int128));
    ddbVec->setDecimal128(start_p, len, scale, w_buf);
    ddbVec->setNullFlag(ddbVec->getNullFlag() || ddbVec->hasNull(start_p, len));
}
bool VectorAppendChild(VectorSP &ddbVec, const ConstantSP &child) {
    VectorSP pAnyVector = createVector({HT_ANY, EXPARAM_DEFAULT}, 0, 1);
    if (!pAnyVector->append(child)) return false;
    return ddbVec->append(pAnyVector);
}

bool VectorHasNull(VectorSP &ddbVec) {
    if (!ddbVec->getNullFlag()) return false;
    return  ddbVec->hasNull();
}

void VectorEnumString(
    const VectorSP &ddbVec,
    std::function<bool(char **, INDEX, INDEX)> func,
    INDEX offset
) {
    char* buffer[CONVERTER_BUF_SIZE];
    char** pbuf;
    INDEX startIndex = offset;
    int size;
    INDEX leftSize = ddbVec->size() - startIndex;
    while (leftSize > 0) {
        size = std::min(leftSize, CONVERTER_BUF_SIZE);
        pbuf = ddbVec->getStringConst(startIndex, size, buffer);
        if (!func(pbuf, startIndex, size)) break;
        leftSize -= size;
        startIndex += size;
    }
}


#define APPEND_UUID(ddbVec, strs, addstrindex, ind)                     \
    int ni = 0;                                                         \
    unsigned char tmp[16] = {};                                         \
    bool error = false;                                                 \
    while (ni < addstrindex) {                                          \
        if ((*(strs + ni)).empty()) {                                   \
            if (!ddbVec->append(getConstantSP_NULL())) {                \
                error = true;break;                                     \
            }                                                           \
            ++ni;                                                       \
            continue;                                                   \
        }                                                               \
        if (!Guid::fromGuid((*(strs + ni)).c_str(), tmp)) {             \
            error = true;break;                                         \
        }                                                               \
        Guid guid = Guid(tmp);                                          \
        if (!ddbVec->appendGuid(&guid, 1)) {                            \
            error = true;break;                                         \
        }                                                               \
        ++ni;                                                           \
    }                                                                   \
    if (error) {                                                        \
        ind = ddbVec->size();                                           \
        throw RuntimeException("");                                     \
    }                                                                   \

#define APPEND_INT128(ddbVec, strs, addstrindex, ind)                   \
    int ni = 0;                                                         \
    unsigned char tmp[16] = {};                                         \
    bool error = false;                                                 \
    while (ni < addstrindex) {                                          \
        if ((*(strs + ni)).empty()) {                                   \
            if (!ddbVec->append(getConstantSP_NULL())) {                \
                error = true;break;                                     \
            }                                                           \
            ++ni;                                                       \
            continue;                                                   \
        }                                                               \
        Guid::fromGuid((*(strs + ni)).c_str(), tmp);                    \
        if (!Int128::parseInt128((*(strs + ni)).c_str(), (*(strs + ni)).size(), tmp)) { \
            error = true;break;                                         \
        }                                                               \
        Guid guid = Guid(tmp);                                          \
        if (!ddbVec->appendGuid(&guid, 1)) {                            \
            error = true;break;                                         \
        }                                                               \
        ++ni;                                                           \
    }                                                                   \
    if (error) {                                                        \
        ind = ddbVec->size();                                           \
        throw RuntimeException("");                                     \
    }                                                                   \

#define APPEND_IPADDR(ddbVec, strs, addstrindex, ind)                               \
    int ni = 0;                                                                     \
    while (ni < addstrindex) {                                                      \
        Guid guid = IPAddr((strs + ni)->c_str(), (strs + ni)->size()).getInt128();  \
        if (!ddbVec->appendGuid(&guid, 1)) {                                        \
            ind = ddbVec->size();                                                   \
            throw RuntimeException("");                                             \
        }                                                                           \
        ++ni;                                                                       \
    }                                                                               \

#define APPEND_DATA(ddbVec, strs, addstrindex, ind, type)               \
    if (type == HT_STRING || type == HT_SYMBOL) {                       \
        if (!ddbVec->appendString(strs, addstrindex)) {                 \
            throw RuntimeException("");                                 \
        }                                                               \
    }                                                                   \
    else if (type == HT_UUID) {                                         \
        APPEND_UUID(ddbVec, strs, addstrindex, ind);                    \
    }                                                                   \
    else if (type == HT_INT128) {                                       \
        APPEND_INT128(ddbVec, strs, addstrindex, ind);                  \
    }                                                                   \
    else if (type == HT_IP) {                                           \
        APPEND_IPADDR(ddbVec, strs, addstrindex, ind);                  \
    }                                                                   \
    else {                                                              \
        throw RuntimeException("");                                     \
    }                                                                   \


void appendStringtoVector(VectorSP &ddbVec, const py::array &pyVec, size_t size, size_t offset, const VectorInfo &info) {
    int strbufsize = 1024;      // this case for fix APY-474
    std::unique_ptr<std::string[]> bufsp(new std::string[strbufsize]);
    std::string* strs = bufsp.get();
    HELPER_TYPE type = (HELPER_TYPE)ddbVec->getType();
    int addstrindex = 0;
    int addbatchindex = 0;
    INDEX ind = -1;
    size_t i = 0;
    Py_ssize_t length;
    auto it = pyVec.begin();
    for(size_t offseti = 0; offseti < offset && it != pyVec.end(); offseti++)
        ++it;
    try {
        for(; it != pyVec.end() && i < size; ++it, i++) {
            if(isNULL(it->ptr())==false){
                PyObject *str_value=it->ptr();
                if (PyUnicode_Check(str_value)) {
                    const char* utf8_str = PyUnicode_AsUTF8AndSize(str_value, &length);
                    if (utf8_str != NULL && !PyErr_Occurred()) {
                        strs[addstrindex].assign(utf8_str, (size_t)length);
                    }
                    else {
                        if (PyErr_Occurred()) PyErr_Clear();
#ifdef PYTHON_SWORDFISH
                        LOG_INFO("Cannot parse string as UTF-8 string.");
#else
                        throw RuntimeException("Cannot parse string as UTF-8 string.");
#endif
                        strs[addstrindex].clear();
                    }
                }
                else if (PyBytes_Check(str_value)) {
                    length = PyBytes_Size(str_value);
                    strs[addstrindex].assign(PyBytes_AS_STRING(str_value), length);
                }
                else {
                    std::string data = py::str(*it);
                    strs[addstrindex].assign(data);
                }
            }else{
                strs[addstrindex].clear();
            }
            addstrindex++;
            if(addstrindex >= strbufsize){  
                APPEND_DATA(ddbVec, strs, addstrindex, ind, type);
                addstrindex=0;
                addbatchindex++;
            }
        }
        APPEND_DATA(ddbVec, strs, addstrindex, ind, type);
    }
    catch(...) {
        if (PyErr_Occurred()) PyErr_Clear();
        if(ind == -1) {         // failed to convert pystr to string
            auto errit = pyVec.begin();
            size_t row_index = offset + i;
            for(size_t offseti = 0; offseti < row_index && errit != pyVec.end(); offseti++)
                errit++;
            throwExceptionAboutNumpyVersion(offset+i, py2str(*errit), getDataTypeString({type, EXPARAM_DEFAULT}), info);
        }
        else {                  // failed to convert STRING to IP/UUID/INT128
            auto errit = pyVec.begin();
            size_t row_index = offset + addbatchindex * strbufsize + ind;
            for(size_t offseti = 0; offseti < row_index && errit != pyVec.end(); offseti++)
                errit++;
            throwExceptionAboutNumpyVersion(row_index, py2str(*errit), getDataTypeString({type, EXPARAM_DEFAULT}), info);
        }
    }
}


void setBlobVector(VectorSP &ddbVec, const py::array &pyVec, size_t size, size_t offset, const VectorInfo &info) {
    std::ignore = size;
    size_t ni = 0;
    py::object tmpObj;
    DATA_TYPE type = ddbVec->getType();
    try {
        for (auto &obj : pyVec) {
            if (ni < offset) {++ni;continue;}
            tmpObj = py::reinterpret_borrow<py::object>(obj);
            if (isNULL(tmpObj)) {
                ddbVec->setString(ni, "");
            }
            else {
                ddbVec->setString(ni, py::cast<std::string>(obj));
            }
            ++ni;
        }
    }
    catch (...) {
        throwExceptionAboutNumpyVersion(ni, py2str(tmpObj), Util::getDataTypeString(type), info);
    }
}

#undef CHECK_INS

} /* namespace converter */
