#ifndef CONVERTER_TYPEHELPER_H_
#define CONVERTER_TYPEHELPER_H_


#include "TypeDefine.h"
#include "TypeConverter.h"

#include "functional"


namespace converter {

#define CONVERTER_BUF_SIZE 8192

static inline std::string py2str(const py::handle &obj) { return py::str(obj).cast<std::string>(); }
static inline std::string py2str(const py::object &obj) { return py::str(obj).cast<std::string>(); }
static inline std::vector<std::string> py2liststr(const py::handle &obj, const std::string &errMsg="") {
    std::vector<std::string> res;
    if (py::isinstance<py::str>(obj)) {
        res.push_back(py::cast<std::string>(obj));
    }
    else if (py::isinstance<py::list>(obj)) {
        res.reserve(py::len(obj));
        for (const auto &item : py::reinterpret_borrow<py::list>(obj)) {
            res.push_back(py::cast<std::string>(item));
        }
    }
    else {
        if (errMsg.empty())
            throw BindProgrammingException("Cannot convert " +
                py::str(obj).cast<std::string>() + " to string vector.");
        else
            throw BindProgrammingException(errMsg);
    }
    return res;
}
PyObject *decodeUtf8Text(const char *pchar,int charsize);


static inline Type createType(DATA_TYPE type, int exparam=EXPARAM_DEFAULT) { return std::make_pair(static_cast<HELPER_TYPE>(type), exparam); }
static inline Type createType(const ConstantSP &data) { return {(HELPER_TYPE)data->getType(), data->getExtraParamForType()}; }
Type createType(const std::string &data);
Type createType(const py::handle &data);

ConstantSP packDataTypeEnum(Type type);
Type unpackDataTypeEnum(ConstantSP &obj);

bool canConvertTo(const Type &src, const Type &dst, Type &res);

DATA_CATEGORY getCategory(Type type);

std::string getDataTypeString(Type type);

ConstantSP getConstantSP_VOID();
ConstantSP getConstantSP_DFLT();
ConstantSP getConstantSP_NULL();

class ConvertErrorInfo {
public:
    enum ErrorCode {
        EC_Ok = 0,
        EC_InvalidTarget = 1,
        EC_InvalidData = 2,
    };
    ConvertErrorInfo() {}
    void set(ErrorCode code, const std::string &info) {
        errorCode = code;
        errorInfo = info;
    }
public:
    ErrorCode   errorCode;
    std::string errorInfo;
};

void SetOrThrowErrorInfo(ConvertErrorInfo *error, ConvertErrorInfo::ErrorCode errorCode, const std::string &errorInfo);
void throwExceptionAboutNumpyVersion(int rowIndex, const std::string &value, const std::string &type, const VectorInfo &info);
/**
 * Cannot create a Vector / ... + msg
 */
void throwExceptionAboutChildOption(const CHILD_VECTOR_OPTION &option, const std::string &msg);

#define ERROR_INVALIDDATA_EXCEED_RANGE(typeName, type, error) \
    SetOrThrowErrorInfo( \
        error, ConvertErrorInfo::EC_InvalidData, \
        std::string(typeName) + " cannot be converted because it exceeds the range of " + converter::getDataTypeString(type) + "." \
    );
#define ERROR_INVALIDTARGET(typeName, type, error) \
    SetOrThrowErrorInfo( \
        error, ConvertErrorInfo::EC_InvalidTarget, \
        "Cannot convert " + std::string(typeName) + " to " + converter::getDataTypeString(type) + "." \
    );
#define ERROR_INVALIDDATA(typeName, type, error) \
    SetOrThrowErrorInfo( \
        error, ConvertErrorInfo::EC_InvalidData, \
        "Cannot convert " + std::string(typeName) + " to " + converter::getDataTypeString(type) + "." \
    );


Constant* createNullConstant(Type type);
Constant* createBool(char val);
Constant* createChar(char val);
Constant* createShort(short val);
Constant* createInt(int val);
Constant* createLong(long long val);
Constant* createFloat(float val);
Constant* createDouble(double val);
Constant* createString(const std::string &val);
Constant* createSymbol(const std::string &val);
Constant* createBlob(const std::string &val);
Constant* createDate(int year, int month, int day);
Constant* createDate(int days);
Constant* createMonth(int year, int month);
Constant* createMonth(int months);
Constant* createNanoTime(int hour, int minute, int second, int nanosecond);
Constant* createNanoTime(long long nanoseconds);
Constant* createTime(int hour, int minute, int second, int millisecond);
Constant* createTime(int milliseconds);
Constant* createSecond(int hour, int minute, int second);
Constant* createSecond(int seconds);
Constant* createMinute(int hour, int minute);
Constant* createMinute(int minutes);
Constant* createNanoTimestamp(int year, int month, int day, int hour, int minute, int second, int nanosecond);
Constant* createNanoTimestamp(long long nanoseconds);
Constant* createTimestamp(int year, int month, int day, int hour, int minute, int second, int millisecond);
Constant* createTimestamp(long long milliseconds);
Constant* createDateTime(int year, int month, int day, int hour, int minute, int second);
Constant* createDateTime(int seconds);
Constant* createDateHour(int hours);
Constant* createDateHour(int year, int month, int day, int hour);
Constant* createDecimal32(int scale, int value, bool isRaw=false);
Constant* createDecimal32(int scale, float value);
Constant* createDecimal32(int scale, double value);
Constant* createDecimal32(int scale, const char* value);
Constant* createDecimal64(int scale, long long value, bool isRaw=false);
Constant* createDecimal64(int scale, float value);
Constant* createDecimal64(int scale, double value);
Constant* createDecimal64(int scale, const char* value);
Constant* createDecimal128(int scale, int128 value, bool isRaw=false);
Constant* createDecimal128(int scale, float value);
Constant* createDecimal128(int scale, double value);
Constant* createDecimal128(int scale, const char* value);


Constant* createValue(Type type, int128 val, bool isRaw=false);
Constant* createValue(Type type, long long val, bool isRaw=false);
Constant* createValue(Type type, int val, bool isRaw=false);
Constant* createValue(Type type, short val, bool isRaw=false);
Constant* createValue(Type type, char val, bool isRaw=false);
Constant* createValue(Type type, double val, bool checkDecimal=false);
Constant* createValue(Type type, float val, bool checkDecimal=false);


template <typename T>
ConstantSP createObject(Type type, const char *name, T val, ConvertErrorInfo *error = NULL, bool isRaw = false) {
    SetOrThrowErrorInfo(error, ConvertErrorInfo::EC_InvalidData, "It cannot be converted to " + getDataTypeString(type));
    return NULL;
}


ConstantSP createObject(Type type, const char *name, std::nullptr_t val, ConvertErrorInfo *error = NULL, bool isRaw = false);
ConstantSP createObject(Type type, const char *name, Constant* val, ConvertErrorInfo *error = NULL, bool isRaw = false);
ConstantSP createObject(Type type, const char *name, ConstantSP val, ConvertErrorInfo *error = NULL, bool isRaw = false);
ConstantSP createObject(Type type, const char *name, bool val, ConvertErrorInfo *error = NULL, bool isRaw = false);
ConstantSP createObject(Type type, const char *name, char val, ConvertErrorInfo *error = NULL, bool isRaw = false);
ConstantSP createObject(Type type, const char *name, short val, ConvertErrorInfo *error = NULL, bool isRaw = false);
ConstantSP createObject(Type type, const char *name, int val, ConvertErrorInfo *error = NULL, bool isRaw = false);
ConstantSP createObject(Type type, const char *name, long long val, ConvertErrorInfo *error = NULL, bool isRaw = false);
ConstantSP createObject(Type type, const char *name, int128 val, ConvertErrorInfo *error = NULL, bool isRaw = false);
ConstantSP createObject(Type type, const char *name, float val, ConvertErrorInfo *error = NULL, bool isRaw = false);
ConstantSP createObject(Type type, const char *name, double val, ConvertErrorInfo *error = NULL, bool isRaw = false);
ConstantSP createObject(Type type, const char *name, const char * val, ConvertErrorInfo *error = NULL, bool isRaw = false);
ConstantSP createObject(Type type, const char *name, const std::string &val, ConvertErrorInfo *error = NULL, bool isRaw = false);


py::object convertTemporalToNumpy(const ConstantSP &obj);
py::object convertTemporalToPython(const ConstantSP &obj);
ConstantSP convertTemporalFromPyDateTime(const py::handle &obj, Type type = {HT_UNK, EXPARAM_DEFAULT});
ConstantSP convertTemporalFromPyDate(const py::handle &obj, Type type = {HT_UNK, EXPARAM_DEFAULT});
ConstantSP convertTemporalFromPyTime(const py::handle &obj, Type type = {HT_UNK, EXPARAM_DEFAULT});


bool isNULL(const py::handle &data);
bool isNULL(const py::handle &data, int &pri, Type &nullType);

#define NP_NULL_INT64           (int64_t)(0x8000000000000000)
#define NP_NULL_FLOAT32         (uint32_t)(2143289344)
#define NP_NULL_FLOAT64         (uint64_t)(9221120237041090560LL)

static inline void SET_NPNULL(long long *p, size_t len = 1) { std::fill((long long *)p, ((long long *)p) + len, NP_NULL_INT64); }
static inline void SET_NPNULL(double *p, size_t len = 1) { std::fill((uint64_t *)p, ((uint64_t *)p) + len, NP_NULL_FLOAT64); }
static inline void SET_NPNULL(float *p, size_t len = 1) { std::fill((uint32_t *)p, ((uint32_t *)p) + len, NP_NULL_FLOAT32); }

#define ISNUMPYNULL_INT64(value)        (value == NP_NULL_INT64)
#define ISNUMPYNULL_FLOAT32(value)      (std::isnan(value) || std::isinf(value))
#define ISNUMPYNULL_FLOAT64(value)      (std::isnan(value) || std::isinf(value))


ConstantSP castTemporal(const ConstantSP &obj, Type type);
ConstantSP castTemporal(const ConstantSP &obj, Type type, bool &changed);

bool checkAnyVector(const ConstantSP &obj);
bool checkArrayVector(const ConstantSP &obj);

Vector* createPair(const ConstantSP &a, const ConstantSP &b);
Vector* createVector(Type type, INDEX size = 0, INDEX capacity = 0);
Vector* createArrayVector(Type type, INDEX size = 0, INDEX valueSize = 0, INDEX capacity = 0, INDEX valueCapacity = 0);
Vector* createIndexVector(INDEX start, INDEX length);
Vector* createIndexVector(INDEX length, bool arrayOnly);

ConstantSP createArrayVectorWithIndexAndValue(const ConstantSP &index, const ConstantSP &value);

Vector* createAllNullVector(Type type, INDEX size = 0, INDEX capacity = 0);
Vector* createMatrix(Type type, INDEX cols, INDEX rows, INDEX colCapacity = 0);
Vector* createMatrixWithVector(ConstantSP ddbVec, INDEX cols, INDEX rows);
Vector* createMatrixWithVectors(const std::vector<ConstantSP> &columns, Type type, INDEX cols, INDEX rows);
Vector* createAllNullMatrix(Type type, INDEX cols, INDEX rows, INDEX colCapacity = 0);
Set* createSet(const ConstantSP& val);
Set* createSet(Type type, size_t capacity = 0);
Dictionary* createDictionary(const ConstantSP& key, const ConstantSP& val, bool isOrdered=true);
Dictionary* createDictionary(Type keyType, Type valType, bool isOrdered=true);
Table* createTable(const std::vector<std::string> &colNames, const std::vector<ConstantSP> &cols);
Table* createTable(const std::vector<std::string> &colNames, const std::vector<Type> &colTypes, INDEX size = 0, INDEX capacity = 0);
Table* createTable(const TableChecker &types, INDEX size = 0, INDEX capacity = 0);

void VectorFillNull(VectorSP &ddbVec, INDEX start, INDEX len);
void VectorFillConstant(VectorSP &ddbVec, INDEX start, INDEX len, const ConstantSP &value);

bool VectorAppendChar(VectorSP &ddbVec, char *buf, int len);
bool VectorAppendShort(VectorSP &ddbVec, short *buf, int len);
bool VectorAppendInt(VectorSP &ddbVec, int *buf, int len);
bool VectorAppendLong(VectorSP &ddbVec, long long *buf, int len);
bool VectorAppendFloat(VectorSP &ddbVec, float *buf, int len);
bool VectorAppendDouble(VectorSP &ddbVec, double *buf, int len);
void VectorAppendDecimal32(VectorSP &ddbVec, int *buf, int len, int scale);
void VectorAppendDecimal64(VectorSP &ddbVec, long long *buf, int len, int scale);
void VectorAppendDecimal128(VectorSP &ddbVec, int128 *buf, int len, int scale);
bool VectorAppendChild(VectorSP &ddbVec, const ConstantSP &child);

bool VectorHasNull(VectorSP &ddbVec);


void appendStringtoVector(VectorSP &ddbVec, const py::array &pyVec, size_t size, size_t offset, const VectorInfo &info);
void setBlobVector(VectorSP &ddbVec, const py::array &pyVec, size_t size, size_t offset, const VectorInfo &info);


void VectorEnumString(
    const VectorSP &ddbVec,
    std::function<bool(char **, INDEX, INDEX)> func,
    INDEX offset = 0
);


template <typename T>
void VectorEnumNumeric(
    const VectorSP &ddbVec,
    const T* (Vector::*getConst)(INDEX, int, T*) const,
    std::function<bool(const T *, INDEX, INDEX)> func,
    INDEX offset = 0
) {
    T buffer[CONVERTER_BUF_SIZE];
    const T *pbuf;
    INDEX startIndex = offset;
    int size;
    INDEX leftSize = ddbVec->size() - startIndex;
    while (leftSize > 0) {
        size = std::min(leftSize, CONVERTER_BUF_SIZE);
        pbuf = (ddbVec.get()->*getConst)(startIndex, size, buffer);
        if (!func(pbuf, startIndex, size)) break;
        leftSize -= size;
        startIndex += size;
    }
}


template <typename T>
void VectorEnumBinary(
    const VectorSP &ddbVec,
    std::function<bool(const T *, INDEX, int)> func,
    INDEX offset = 0
) {
    std::vector<T> buffer(CONVERTER_BUF_SIZE);
    const T* pbuf;
    INDEX startIndex = offset;
    int size;
    INDEX leftSize = ddbVec->size() - startIndex;
    while (leftSize > 0) {
        size = std::min(leftSize, CONVERTER_BUF_SIZE);
        pbuf = (const T*)ddbVec->getBinaryConst(startIndex, size, sizeof(T), (unsigned char*)buffer.data());
        if (!func(pbuf, startIndex, size)) break;
        leftSize -= size;
        startIndex += size;
    }
}


template <typename T>
void processData(T *psrcData, size_t size, std::function<void(T *, int)> f) {
    int bufsize = std::min(CONVERTER_BUF_SIZE, (int)size);
    std::unique_ptr<T[]> bufsp(new T[bufsize]);
    T *buf = bufsp.get();
    size_t startIndex = 0, len;
    while(startIndex < size){
        len = std::min((int)(size-startIndex), bufsize);
        memcpy(buf, psrcData+startIndex, sizeof(T)*len);
        f(buf,len);
        startIndex += len;
    }
}


bool isArrayLike(const py::handle &obj);





} /* namespace converter */




#endif
