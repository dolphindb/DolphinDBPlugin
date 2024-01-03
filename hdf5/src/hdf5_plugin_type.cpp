#include "hdf5_plugin_type.h"
#include <hdf5_plugin_imp.h>

namespace H5PluginImp
{

#define TO_STRING(s) #s
typedef SmartPointer<TypeColumn> H5ColumnSP;

const char *getHdf5NativeTypeStr(H5DataType &type)
{
    hid_t id = type.id();
    H5T_class_t typeClass = H5Tget_class(type.id());

    static const char *ERROR_STR = "";

    if (typeClass == H5T_NO_CLASS)
        return ERROR_STR;

    // check time class
    if (typeClass == H5T_TIME)
    {
        if (H5Tequal(id, H5T_UNIX_D32BE) > 0)
            return TO_STRING(H5T_UNIX_D32BE);
        if (H5Tequal(id, H5T_UNIX_D32LE) > 0)
            return TO_STRING(H5T_UNIX_D32LE);
        if (H5Tequal(id, H5T_UNIX_D64BE) > 0)
            return TO_STRING(H5T_UNIX_D64BE);
        if (H5Tequal(id, H5T_UNIX_D64LE) > 0)
            return TO_STRING(H5T_UNIX_D64LE);
        return TO_STRING(UNKNOWN_TIME_TYPE);
    }

    // check the class we don't need to show the detail
    switch (typeClass)
    {
    case H5T_BITFIELD:
        return TO_STRING(H5T_BITFIELD);
    case H5T_OPAQUE:
        return TO_STRING(H5T_OPAQUE);
    case H5T_REFERENCE:
        return TO_STRING(H5T_REFERENCE);
    case H5T_VLEN:
        return TO_STRING(H5T_VLEN);
    case H5T_COMPOUND:
        return TO_STRING(H5T_COMPOUND);
    case H5T_ARRAY:
        return TO_STRING(H5T_ARRAY);
    default:
        break;
    }

    //check the class we need to show the detail
    H5DataType nativeType;
    if (!acceptNativeType(nativeType, id))
        return ERROR_STR;
    hdf5_type_layout layout;
    if (!getHdf5SimpleLayout(nativeType.id(), layout))
        return ERROR_STR;

    switch (layout.flag)
    {
    case IS_FIXED_STR:
    case IS_VARIABLE_STR:
        return TO_STRING(H5T_STRING);
    case IS_ENUM:
        return TO_STRING(H5T_ENUM);
    case IS_S_CHAR_INTEGER:
        return TO_STRING(H5T_NATIVE_SCHAR);
    case IS_U_CHAR_INTEGER:
        return TO_STRING(H5T_NATIVE_UCHAR);
    case IS_S_SHORT_INTEGER:
        return TO_STRING(H5T_NATIVE_SHORT);
    case IS_U_SHORT_INTEGER:
        return TO_STRING(H5T_NATIVE_USHORT);
    case IS_S_INT_INTEGER:
        return TO_STRING(H5T_NATIVE_INT);
    case IS_U_INT_INTEGER:
        return TO_STRING(H5T_NATIVE_UINT);
    case IS_S_LLONG_INTEGER:
        return TO_STRING(H5T_NATIVE_LLONG);
    case IS_U_LLONG_INTEGER:
        return TO_STRING(H5T_NATIVE_ULLONG);
    case IS_FLOAT_FLOAT:
        return TO_STRING(H5T_NATIVE_FLOAT);
    case IS_DOUBLE_FLOAT:
        return TO_STRING(H5T_NATIVE_DOUBLE);
    default:
        return ERROR_STR;
    }
}

#undef TO_STRING


H5ColumnSP TypeColumn::createNewColumn(H5DataType &srcType)
{
    H5ColumnSP csp;
    hid_t t = -1;
    H5DataType srcNativeType;
    hdf5_type_layout srcNativeTypeLayout;

    hid_t sid = srcType.id();

    bool isTime = isClassEqual(sid, H5T_TIME);

    if (!isTime && acceptNativeType(srcNativeType, sid))
        t = srcNativeType.id();
    else
        t = srcType.id();

    getHdf5SimpleLayout(t, srcNativeTypeLayout);
        // return nullptr;

    switch (srcNativeTypeLayout.flag)
    {
    case IS_FIXED_STR:
        csp = new FixedStringColumn(H5Tget_size(srcNativeType.id()) + 1); //one byte for '\0'
        break;
    case IS_VARIABLE_STR:
        csp = new VariableStringColumn();
        break;
    case IS_ENUM:
        csp = new SymbolColumn(srcNativeType.id());
        break;
    case IS_S_CHAR_INTEGER:
        csp = new CharColumn();
        break;
    case IS_U_CHAR_INTEGER:
        csp = new ShortColumn();
        break;
    case IS_S_SHORT_INTEGER:
        csp = new ShortColumn();
        break;
    case IS_U_SHORT_INTEGER:
        csp = new IntColumn();
        break;
    case IS_S_INT_INTEGER:
        csp = new IntColumn();
        break;
    case IS_U_INT_INTEGER:
        csp = new LLongColumn();
        break;
    case IS_S_LLONG_INTEGER:
        csp = new LLongColumn();
        break;
    case IS_U_LLONG_INTEGER:
        csp = new LLongColumn();
        break;
    case IS_FLOAT_FLOAT:
        csp = new FloatColumn();
        break;
    case IS_DOUBLE_FLOAT:
        csp = new DoubleColumn();
        break;
    case IS_UNIX_TIME:
        csp = new UNIX64BitTimestampColumn();
        break;
    case IS_BIT:
        csp = new BoolColumn();
        break;
    default:
        csp = nullptr;
        break;
    }

    return csp;
}

bool TypeColumn::convertHdf5SimpleType(H5DataType &srcType,
                                       H5DataType &convertedType)
{
    hid_t cid = -1;
    hid_t sid = srcType.id();
    hid_t t = -1;
    H5DataType srcNativeType;
    hdf5_type_layout srcNativeTypeLayout;

    bool isTime = isClassEqual(sid, H5T_TIME);

    if (!isTime && acceptNativeType(srcNativeType, sid))
        t = srcNativeType.id();
    else
        t = srcType.id();

    getHdf5SimpleLayout(t, srcNativeTypeLayout);
        // return false;

    switch (srcNativeTypeLayout.flag)
    {
    case IS_FIXED_STR:
        cid = H5Tcopy(H5T_C_S1);
        H5Tset_size(cid, srcNativeTypeLayout.size + 1); //one byte for '\0'
        break;
    case IS_VARIABLE_STR:
        cid = H5Tcopy(H5T_C_S1);
        H5Tset_size(cid, H5T_VARIABLE);
        break;
    case IS_ENUM:
        cid = H5Tcopy(srcNativeType.id());
        break;
    case IS_S_CHAR_INTEGER:
        cid = H5T_NATIVE_CHAR;
        break;
    case IS_U_CHAR_INTEGER:
        cid = H5T_NATIVE_SHORT;
        break;
    case IS_S_SHORT_INTEGER:
        cid = H5T_NATIVE_SHORT;
        break;
    case IS_U_SHORT_INTEGER:
        cid = H5T_NATIVE_INT;
        break;
    case IS_S_INT_INTEGER:
        cid = H5T_NATIVE_INT;
        break;
    case IS_U_INT_INTEGER:
        cid = H5T_NATIVE_LLONG;
        break;
    case IS_S_LLONG_INTEGER:
        cid = H5T_NATIVE_LLONG;
        break;
    case IS_U_LLONG_INTEGER:
        cid = H5T_NATIVE_LLONG; //unsupported
        break;
    case IS_FLOAT_FLOAT:
        cid = H5T_NATIVE_FLOAT;
        break;
    case IS_DOUBLE_FLOAT:
        cid = H5T_NATIVE_DOUBLE;
        break;
    case IS_UNIX_TIME:
        cid = Util::isLittleEndian() ? H5T_UNIX_D64LE : H5T_UNIX_D64BE;
        break;
    case IS_BIT:
        cid = H5T_NATIVE_B8;
        break;
    default:
        cid = -1;
        break;
    }
    convertedType.accept(cid);
    return cid != -1;
}

bool TypeColumn::createHdf5TypeColumns(H5DataType &srcType, vector<H5ColumnSP> &cols,
                                       H5DataType &convertedType)
{
    H5T_class_t srcClass = H5Tget_class(srcType.id());
    if (srcClass == H5T_COMPOUND || srcClass == H5T_ARRAY)
    {
        if(!createComplexColumns(srcType, cols, convertedType))
            return false;
    }
    else
    {
        H5ColumnSP csp = createNewColumn(srcType);
        bool ok = convertHdf5SimpleType(srcType, convertedType);

        if (!ok || csp == nullptr)
            return false;

        cols.push_back(csp);
    }
    return true;
}

bool TypeColumn::createCompoundColumns(H5DataType &srcType, vector<H5ColumnSP> &cols,
                                       H5DataType &convertedType)
{
    if(!isClassEqual(srcType.id(), H5T_COMPOUND))
        return false;

    size_t memNum = H5Tget_nmembers(srcType.id());
    H5DataType memType;
    H5DataType memTypeConverted;
    convertedType.accept(H5Tcreate(H5T_COMPOUND, 2 * H5Tget_size(srcType.id())));
    size_t offset = 0;

    for (size_t i = 0; i < memNum; i++)
    {
        memType.accept(H5Tget_member_type(srcType.id(), i));

        if (!createHdf5TypeColumns(memType, cols, memTypeConverted))
            return false;

        char *memName = H5Tget_member_name(srcType.id(), i);
        H5Tinsert(convertedType.id(), memName, offset, memTypeConverted.id());
        offset += H5Tget_size(memTypeConverted.id());
        H5free_memory(memName);
    }

    H5Tpack(convertedType.id());
    return true;
}

bool TypeColumn::createArrayColumns(H5DataType &srcType, vector<H5ColumnSP> &cols,
                                    H5DataType &convertedType)
{

    if (!isClassEqual(srcType.id(), H5T_ARRAY))
        return false;

    H5DataType base = H5Tget_super(srcType.id());

    H5DataType baseConverted;
    size_t dimsNum = H5Tget_array_ndims(srcType.id());
    if(dimsNum < 0) {
        return false;
    }

    std::vector<hsize_t> dims(dimsNum);
    H5Tget_array_dims(srcType.id(), dims.data());

    hsize_t eleNum = 1;
    for (size_t i = 0; i != dimsNum; i++)
        eleNum *= dims[i];

    for (size_t i = 0; i != eleNum; i++)
        if(!createHdf5TypeColumns(base, cols, baseConverted))
            return false;

    convertedType.accept(H5Tarray_create(baseConverted.id(), dims.size(), dims.data()));
    return convertedType.id() > 0;
}

bool TypeColumn::createComplexColumns(H5DataType &srcType, vector<H5ColumnSP> &cols, H5DataType &convertedType)
{
    H5T_class_t c = H5Tget_class(srcType.id());

    if (c == H5T_COMPOUND)
        return createCompoundColumns(srcType, cols, convertedType);
    if (c == H5T_ARRAY)
        return createArrayColumns(srcType, cols, convertedType);
    return false;
}

VectorSP TypeColumn::createDolphinDBColumnVector(const VectorSP& destVec, int size, int cap)
{
    DATA_TYPE destType = (destVec->isNull()) ? srcType() : destVec->getType();

    if (!compatible(destType))
        return nullSP;
    colVec_ = createCompatibleVector(destVec, destType, size, cap);
    destType_ = destType;
    destTypeSize_ = (destType == DT_STRING) ? sizeof(char *) : Util::getDataTypeSize(destType);
    return colVec_;
}

VectorSP TypeColumn::createCompatibleVector(VectorSP destVec, DATA_TYPE destType, int size, int cap)
{
    return Util::createVector(destType, size, cap);
}

VectorSP TypeColumn::createDolphinDBColumnVector(int size, int cap)
{
    return createDolphinDBColumnVector(nullSP, size, cap);
}

int TypeColumn::appendData(char *raw_data, int offset, int stride, int len, vector<char> &buffer)
{
    buffer.resize(len * destTypeSize_);

    pack_info_t t;
    t.buffer = buffer.data();
    t.len = len;
    t.raw_data = raw_data + offset;
    t.stride = stride;

    DATA_TYPE basicType = packData(t);

    return doAppend(t.buffer, t.len, basicType);
}

int TypeColumn::doAppend(void *data, int len, DATA_TYPE basicType)
{
    bool r = true;
    VectorSP v = colVec_;

    switch (basicType)
    {
    case DT_CHAR:
        r = v->appendChar((char *)data, len);
        break;
    case DT_SHORT:
        r = v->appendShort((short *)data, len);
        break;
    case DT_INT:
        r = v->appendInt((int *)data, len);
        break;
    case DT_LONG:
        r = v->appendLong((long long *)data, len);
        break;
    case DT_FLOAT:
        r = v->appendFloat((float *)data, len);
        break;
    case DT_DOUBLE:
        r = v->appendDouble((double *)data, len);
        break;
    case DT_STRING:
        r = v->appendString((char **)data, len);
        break;
    case DT_TIMESTAMP:
        r = v->appendInt((int *)data, len);
        break;
    default:
        r = false;
    }
    if (!r)
        throw RuntimeException(HDF5_LOG_PREFIX + "append data to vector failed");

    return len;
}

template <class src_t>
void numeric_pack_to_bool(pack_info_t t)
{
    static_assert(std::is_arithmetic<src_t>::value, "src type is not integer or float");
    //static_assert(std::is_signed<src_t>::value, "src type is not signed");

    char *buf = (char *)t.buffer;
    for (int i = 0; i != t.len; i++)
    {
        const src_t *n = (const src_t *)t.raw_data;
        buf[i] = static_cast<char>(*n != 0);
        t.raw_data += t.stride;
    }
}

template <class src_type, class dest_type>
void numeric_upper_pack(pack_info_t t)
{
    dest_type *buf = (dest_type *)t.buffer;

    for (int i = 0; i != t.len; i++)
    {
        const src_type *n = (const src_type *)(t.raw_data);
        buf[i] = static_cast<dest_type>(*n);
        t.raw_data += t.stride;
    }
}

#define is_sintegral_v(type) (std::is_integral<type>::value && std::is_signed<type>::value)
#define is_fp_v(type) (std::is_floating_point<type>::value)
#define is_signed_v(type) (std::is_signed<type>::value)
#define is_same_v(type1, type2) (std::is_same<type1, type2>::value)

template <bool cond, typename t = void>
using _enable_if_t = typename std::enable_if<cond, t>::type;

#define enable_if_2cond_t(cond1, cond2) _enable_if_t<cond1 && cond2>

//floating point -> signed integral
template <class src_t, class dest_t>
enable_if_2cond_t(is_fp_v(src_t), is_sintegral_v(dest_t))
    numeric_lower_pack(pack_info_t t)
{
    dest_t *buf = (dest_t *)t.buffer;

    dest_t destMax = std::numeric_limits<dest_t>::max();
    dest_t destMin = std::numeric_limits<dest_t>::min() + 1;
    // min value in dolphindb = min value in c + 1

    for (int i = 0; i != t.len; i++)
    {
        const src_t *n = (const src_t *)(t.raw_data);
        dest_t v;

        if (*n >= destMax) //avoid overflow
            v = destMax;
        else if (*n <= destMin)
            v = destMin;
        else
            v = *n >= 0 ? (*n + 0.5) : (*n - 0.5); //round to nearest integral

        buf[i] = v;
        t.raw_data += t.stride;
    }
}

//signed integral-> signed integral
template <class src_t, class dest_t>
enable_if_2cond_t(is_sintegral_v(src_t), is_sintegral_v(dest_t))
    numeric_lower_pack(pack_info_t t)
{
    dest_t *buf = (dest_t *)t.buffer;

    dest_t destMax = std::numeric_limits<dest_t>::max();
    dest_t destMin = std::numeric_limits<dest_t>::min() + 1;

    for (int i = 0; i != t.len; i++)
    {
        const src_t *n = (const src_t *)(t.raw_data);
        dest_t v;

        if (*n > destMax)
            v = destMax;
        else if (*n < destMin)
            v = destMin;
        else
            v = *n;

        buf[i] = v;
        t.raw_data += t.stride;
    }
}

//floating point -> floating point
//double -> float
template <class src_t, class dest_t>
enable_if_2cond_t(is_fp_v(src_t), is_fp_v(dest_t))
    numeric_lower_pack(pack_info_t t)
{

    assert(is_same_v(src_t, double) && is_same_v(dest_t, float));

    dest_t *buf = (dest_t *)t.buffer;

    dest_t destMax = std::numeric_limits<dest_t>::max();
    dest_t destMin = -340282326638528860000000000000000000000.0f;

    for (int i = 0; i != t.len; i++)
    {
        const src_t *n = (const src_t *)(t.raw_data);
        dest_t v;

        if (std::isinf(*n))
            v = INFINITY;
        else if (std::isnan(*n))
            v = NAN;
        else if (*n > destMax)
            v = destMax;
        else if (*n < destMin)
            v = destMin;
        else
            v = *n;

        buf[i] = v;
        t.raw_data += t.stride;
    }
}

//signed integral -> floating point
//nothing
template <class src_t, class dest_t>
enable_if_2cond_t(is_sintegral_v(src_t), is_fp_v(dest_t))
    numeric_lower_pack(pack_info_t t)
{
    assert(0);
}

template <class src_t, class dest_t>
struct numeric_pack
{
    void operator()(pack_info_t t) const
    {

        static_assert(std::is_arithmetic<src_t>::value, "src type is not integer or float");
        static_assert(std::is_arithmetic<dest_t>::value, "dest type is not integer or float");
        static_assert(std::is_signed<src_t>::value, "src type is not signed");
        static_assert(std::is_signed<dest_t>::value, "dest type is not signed");

        if (is_same_v(src_t, dest_t))
            return numeric_upper_pack<src_t, dest_t>(t);

        constexpr dest_t destMax = std::numeric_limits<dest_t>::max();
        constexpr src_t srcMax = std::numeric_limits<src_t>::max();

        if (srcMax <= destMax) // calculate in compile-time
            numeric_upper_pack<src_t, dest_t>(t);
        else
            numeric_lower_pack<src_t, dest_t>(t);
    }
};

template <class src_t>
struct numeric_pack<src_t, bool>
{
    void operator()(pack_info_t t) const { return numeric_pack_to_bool<src_t>(t); }
};

/*
*    pack from char
*/

template <class dest_type>
void packCharTo(pack_info_t t)
{
    numeric_pack<char, dest_type>()(t);
}

DATA_TYPE CharColumn::packData(pack_info_t t)
{
    switch (destType_)
    {
    case DT_BOOL:
        packCharTo<bool>(t);
        return DT_CHAR;
    case DT_CHAR:
        packCharTo<char>(t);
        return DT_CHAR;
    case DT_SHORT:
        packCharTo<short>(t);
        return DT_SHORT;
    case DT_INT:
        packCharTo<int>(t);
        return DT_INT;
    case DT_LONG:
        packCharTo<long long>(t);
        return DT_LONG;
    case DT_FLOAT:
        packCharTo<float>(t);
        return DT_FLOAT;
    case DT_DOUBLE:
        packCharTo<double>(t);
        return DT_DOUBLE;
    case DT_MONTH:
        packCharTo<int>(t);
        return DT_INT;
    case DT_TIME:
        packCharTo<int>(t);
        return DT_INT;
    case DT_MINUTE:
        packCharTo<int>(t);
        return DT_INT;
    case DT_SECOND:
        packCharTo<int>(t);
        return DT_INT;
    case DT_NANOTIME:
        packCharTo<int>(t);
        return DT_INT;
    case DT_DATE:
        packCharTo<int>(t);
        return DT_INT;
    case DT_DATETIME:
        packCharTo<int>(t);
        return DT_INT;
    case DT_TIMESTAMP:
        packCharTo<int>(t);
        return DT_INT;
    case DT_NANOTIMESTAMP:
        packCharTo<int>(t);
        return DT_INT;
    default:
        return DT_VOID;
    }
}

/*
*    pack from short
*/

template <class dest_type>
void packShortTo(pack_info_t t)
{
    numeric_pack<short, dest_type>()(t);
}

DATA_TYPE ShortColumn::packData(pack_info_t t)
{
    switch (destType_)
    {
    case DT_BOOL:
        packShortTo<bool>(t);
        return DT_CHAR;
    case DT_CHAR:
        packShortTo<char>(t);
        return DT_CHAR;
    case DT_SHORT:
        packShortTo<short>(t);
        return DT_SHORT;
    case DT_INT:
        packShortTo<int>(t);
        return DT_INT;
    case DT_LONG:
        packShortTo<long long>(t);
        return DT_LONG;
    case DT_FLOAT:
        packShortTo<float>(t);
        return DT_FLOAT;
    case DT_DOUBLE:
        packShortTo<double>(t);
        return DT_DOUBLE;
    case DT_MONTH:
        packShortTo<int>(t);
        return DT_INT;
    case DT_TIME:
        packShortTo<int>(t);
        return DT_INT;
    case DT_MINUTE:
        packShortTo<int>(t);
        return DT_INT;
    case DT_SECOND:
        packShortTo<int>(t);
        return DT_INT;
    case DT_NANOTIME:
        packShortTo<int>(t);
        return DT_INT;
    case DT_DATE:
        packShortTo<int>(t);
        return DT_INT;
    case DT_DATETIME:
        packShortTo<int>(t);
        return DT_INT;
    case DT_TIMESTAMP:
        packShortTo<int>(t);
        return DT_INT;
    case DT_NANOTIMESTAMP:
        packShortTo<int>(t);
        return DT_INT;
    default:
        return DT_VOID;
    }
}

template <class dest_type>
void packIntTo(pack_info_t t)
{
    numeric_pack<int, dest_type>()(t);
}

DATA_TYPE IntColumn::packData(pack_info_t t)
{
    switch (destType_)
    {
    case DT_BOOL:
        packIntTo<bool>(t);
        return DT_CHAR;
    case DT_CHAR:
        packIntTo<char>(t);
        return DT_CHAR;
    case DT_SHORT:
        packIntTo<short>(t);
        return DT_SHORT;
    case DT_INT:
        packIntTo<int>(t);
        return DT_INT;
    case DT_LONG:
        packIntTo<long long>(t);
        return DT_LONG;
    case DT_FLOAT:
        packIntTo<float>(t);
        return DT_FLOAT;
    case DT_DOUBLE:
        packIntTo<double>(t);
        return DT_DOUBLE;
    case DT_DATE:
        packIntTo<int>(t);
        return DT_INT;
    case DT_MONTH:
        packIntTo<int>(t);
        return DT_INT;
    case DT_TIME:
        packIntTo<int>(t);
        return DT_INT;
    case DT_MINUTE:
        packIntTo<int>(t);
        return DT_INT;
    case DT_SECOND:
        packIntTo<int>(t);
        return DT_INT;
    case DT_DATETIME:
        packIntTo<int>(t);
        return DT_INT;
    case DT_TIMESTAMP:
        packIntTo<int>(t);
        return DT_INT;
    case DT_NANOTIMESTAMP:
        packIntTo<int>(t);
        return DT_INT;
    case DT_NANOTIME:
        packIntTo<int>(t);
        return DT_INT;
    default:
        return DT_VOID;
    }
}

template <class dest_type>
void packLLongTo(pack_info_t t)
{
    numeric_pack<long long, dest_type>()(t);
}

DATA_TYPE LLongColumn::packData(pack_info_t t)
{
    switch (destType_)
    {
    case DT_BOOL:
        packLLongTo<bool>(t);
        return DT_CHAR;
    case DT_CHAR:
        packLLongTo<char>(t);
        return DT_CHAR;
    case DT_SHORT:
        packLLongTo<short>(t);
        return DT_SHORT;
    case DT_INT:
        packLLongTo<int>(t);
        return DT_INT;
    case DT_LONG:
        packLLongTo<long long>(t);
        return DT_LONG;
    case DT_FLOAT:
        packLLongTo<float>(t);
        return DT_FLOAT;
    case DT_DOUBLE:
        packLLongTo<double>(t);
        return DT_DOUBLE;
    case DT_DATE:
        packLLongTo<int>(t);
        return DT_INT;
    case DT_DATETIME:
        packLLongTo<int>(t);
        return DT_INT;
    case DT_MONTH:
        packLLongTo<int>(t);
        return DT_INT;
    case DT_TIME:
        packLLongTo<int>(t);
        return DT_INT;
    case DT_MINUTE:
        packLLongTo<int>(t);
        return DT_INT;
    case DT_SECOND:
        packLLongTo<int>(t);
        return DT_INT;
    case DT_TIMESTAMP:
        packLLongTo<long long>(t);
        return DT_LONG;
    case DT_NANOTIME:
        packLLongTo<long long>(t);
        return DT_LONG;
    case DT_NANOTIMESTAMP:
        packLLongTo<long long>(t);
        return DT_LONG;
    default:
        return DT_VOID;
    }
}

template <class dest_type>
void packFloatTo(pack_info_t t)
{
    numeric_pack<float, dest_type>()(t);
}

DATA_TYPE FloatColumn::packData(pack_info_t t)
{
    switch (destType_)
    {
    case DT_BOOL:
        packFloatTo<bool>(t);
        return DT_CHAR;
    case DT_CHAR:
        packFloatTo<char>(t);
        return DT_CHAR;
    case DT_SHORT:
        packFloatTo<short>(t);
        return DT_SHORT;
    case DT_INT:
        packFloatTo<int>(t);
        return DT_INT;
    case DT_LONG:
        packFloatTo<long long>(t);
        return DT_LONG;
    case DT_FLOAT:
        packFloatTo<float>(t);
        return DT_FLOAT;
    case DT_DOUBLE:
        packFloatTo<double>(t);
        return DT_DOUBLE;
    default:
        return DT_VOID;
    }
}

template <class dest_type>
void pack_from_double_to(pack_info_t t)
{
    numeric_pack<double, dest_type>()(t);
}

DATA_TYPE DoubleColumn::packData(pack_info_t t)
{
    switch (destType_)
    {
    case DT_BOOL:
        pack_from_double_to<bool>(t);
        return DT_CHAR;
    case DT_CHAR:
        pack_from_double_to<char>(t);
        return DT_CHAR;
    case DT_SHORT:
        pack_from_double_to<short>(t);
        return DT_SHORT;
    case DT_INT:
        pack_from_double_to<int>(t);
        return DT_INT;
    case DT_LONG:
        pack_from_double_to<long long>(t);
        return DT_LONG;
    case DT_FLOAT:
        pack_from_double_to<float>(t);
        return DT_FLOAT;
    case DT_DOUBLE:
        pack_from_double_to<double>(t);
        return DT_DOUBLE;
    default:
        return DT_VOID;
    }
}

bool StringColumn::compatible(DATA_TYPE destType) const
{
    return destType == DT_STRING || destType == DT_SYMBOL;
}

VectorSP StringColumn::createCompatibleVector(VectorSP destVec, DATA_TYPE destType, int size, int cap)
{
    if (destType == DT_STRING)
        return Util::createVector(destType, size, cap);
    else if (destType == DT_SYMBOL)
    {
        symBaseSP_ = destVec->getSymbolBase();
        return Util::createSymbolVector(symBaseSP_, size, cap);
    }

    return nullSP;
}

DATA_TYPE FixedStringColumn::packData(pack_info_t t)
{
    if (destType_ == DT_STRING)
    {
        char **buf = (char **)t.buffer;
        for (int i = 0; i != t.len; i++)
        {
            buf[i] = t.raw_data;
            t.raw_data += t.stride;
        }
        return DT_STRING;
    }
    else if (destType_ == DT_SYMBOL)
    {
        int *buf = (int *)t.buffer;
        for (int i = 0; i != t.len; i++)
        {
            buf[i] = symBaseSP_->findAndInsert(t.raw_data);
            t.raw_data += t.stride;
        }
        return DT_INT; //raw data of symbol
    }
    return DT_VOID;
}

DATA_TYPE VariableStringColumn::packData(pack_info_t t)
{
    if (destType_ == DT_STRING)
    {
        char **buf = (char **)t.buffer;
        static char empty = '\0';
        for (int i = 0; i != t.len; i++)
        {
            char **n = (char **)(t.raw_data);
            buf[i] = (*n == nullptr) ? &empty : *n;
            t.raw_data += t.stride;
        }
        return DT_STRING;
    }
    else if (destType_ == DT_SYMBOL)
    {
        int *buf = (int *)t.buffer;
        for (int i = 0; i != t.len; i++)
        {
            char **n = (char **)(t.raw_data);
            buf[i] = symBaseSP_->findAndInsert(*n == nullptr ? "" : *n);
            t.raw_data += t.stride;
        }
        return DT_INT; //raw data of symbol
    }
    return DT_VOID;
}

DATA_TYPE SymbolColumn::packData(pack_info_t t)
{
    if (destType_ == DT_STRING)
    {
        char **buf = (char **)t.buffer;
        long long value = 0;

        for (int i = 0; i != t.len; i++)
        {
            memcpy(&value, t.raw_data, baseSize_);
            buf[i] = &enumMap_[value][0]; //the buf won't be modified
            t.raw_data += t.stride;
        }
        return DT_STRING;
    }
    else if (destType_ == DT_SYMBOL)
    {
        int *buf = (int *)t.buffer;
        long long value = 0;
        for (int i = 0; i != t.len; i++)
        {
            memcpy(&value, t.raw_data, baseSize_);
            buf[i] = symbolMap_[value];
            t.raw_data += t.stride;
        }
        return DT_INT;
    }
    return DT_VOID;
}

VectorSP SymbolColumn::createCompatibleVector(VectorSP destVec, DATA_TYPE destType, int size, int cap)
{
    if (destType == DT_STRING)
        return Util::createVector(destType, size, cap);
    else if (destType == DT_SYMBOL)
    {
        symbolMap_.clear();
        symBaseSP_ = (destVec->isNull()) ? new SymbolBase() : destVec->getSymbolBase();

        auto iter = enumMap_.begin();
        for (; iter != enumMap_.end(); iter++)
        {
            long long enumValue = iter.key();
            const string &enumName = iter.value();

            int symValue = symBaseSP_->findAndInsert(enumName);
            symbolMap_.insert(enumValue, symValue);
        }
        return Util::createSymbolVector(symBaseSP_, size, cap);
    }
    return nullSP;
}

void SymbolColumn::createEnumMap(hid_t nativeEnumId)
{
    H5DataType base = H5Tget_super(nativeEnumId);

    if (base.id() < 0 || nativeEnumId < 0)
        throw RuntimeException(HDF5_LOG_PREFIX + "error when convert hdf5 enum");

    baseSize_ = H5Tget_size(base.id());

    int memNum = H5Tget_nmembers(nativeEnumId);
    if (memNum <= 0)
        throw RuntimeException(HDF5_LOG_PREFIX + "invalid member num:" + std::to_string(memNum));

    enumMap_.clear();
    for (int i = 0; i < memNum; i++)
    {
        long long value = 0;
        H5Tget_member_value(nativeEnumId, i, &value);
        char *name = H5Tget_member_name(nativeEnumId, i);
        enumMap_.insert(value, name);
        H5free_memory(name);
    }
}

bool UNIX64BitTimestampColumn::compatible(DATA_TYPE destType) const
{
    switch (destType)
    {
    case DT_DATE:
    case DT_MONTH:
    case DT_TIME:
    case DT_MINUTE:
    case DT_SECOND:
    case DT_DATETIME:
    case DT_TIMESTAMP:
    case DT_NANOTIME:
    case DT_NANOTIMESTAMP:
        return true;
    default:
        return false;
    }
    return false;
}

void pack64timestampToDTdate(pack_info_t t)
{
    int *buf = (int *)t.buffer;
    tm gmtBuf;
    for (int i = 0; i != t.len; i++)
    {
        long long *n = (long long *)t.raw_data;
        time_t ts = (*n) / 1000;

        tm *gmt = gmtime_r(&ts,&gmtBuf);
        buf[i] = (gmt==nullptr) ? 0 : Util::countDays(gmt->tm_year + 1900, gmt->tm_mon + 1, gmt->tm_mday);
        t.raw_data += t.stride;
    }
}

void pack64timestampToDTdatetime(pack_info_t t)
{
    int *buf = (int *)t.buffer;
    for (int i = 0; i != t.len; i++)
    {
        long long *n = (long long *)t.raw_data;
        buf[i] = (*n) / 1000;
        t.raw_data += t.stride;
    }
}

void pack64timestampToDTtimestamp(pack_info_t t)
{
    long long *buf = (long long *)t.buffer;
    for (int i = 0; i != t.len; i++)
    {
        long long *n = (long long *)t.raw_data;
        buf[i] = *n;
        t.raw_data += t.stride;
    }
}

void pack64timestampToDTnanotime(pack_info_t t)
{
    long long *buf = (long long *)t.buffer;
    tm gmtBuf;
    for (int i = 0; i != t.len; i++)
    {
        long long *n = (long long *)t.raw_data;
        time_t ts = (*n) / 1000;
        tm *gmt = gmtime_r(&ts,&gmtBuf);
        buf[i] = (gmt==nullptr) ? 0 : (((gmt->tm_hour * 60 + gmt->tm_min) * 60 + gmt->tm_sec) * 1000 + (*n) % 1000) * 1000000;
        t.raw_data += t.stride;
    }
}

void pack64timestampToDTnanotimestamp(pack_info_t t)
{
    long long *buf = (long long *)t.buffer;
    for (int i = 0; i != t.len; i++)
    {
        long long *n = (long long *)t.raw_data;
        buf[i] = *n * 1000000;
        t.raw_data += t.stride;
    }
}

void pack64timestampToDTmonth(pack_info_t t)
{
    int *buf = (int *)t.buffer;
    tm gmtBuf;
    for (int i = 0; i != t.len; i++)
    {
        long long *n = (long long *)t.raw_data;
        time_t ts = (*n) / 1000;
        tm *gmt = gmtime_r(&ts,&gmtBuf);
        buf[i] = (gmt==nullptr) ? 0 : (gmt->tm_year + 1900) * 12 + gmt->tm_mon;
        t.raw_data += t.stride;
    }
}

void pack64timestampToDTtime(pack_info_t t)
{
    int *buf = (int *)t.buffer;
    tm gmtBuf;
    for (int i = 0; i != t.len; i++)
    {
        long long *n = (long long *)t.raw_data;
        time_t ts = (*n) / 1000;
        tm *gmt = gmtime_r(&ts,&gmtBuf);
        buf[i] = (gmt==nullptr) ? 0 : ((gmt->tm_hour * 60 + gmt->tm_min) * 60 + gmt->tm_sec) * 1000 + (*n) % 1000;
        t.raw_data += t.stride;
    }
}

void pack64timestampToDTminute(pack_info_t t)
{
    int *buf = (int *)t.buffer;
    tm gmtBuf;
    for (int i = 0; i != t.len; i++)
    {
        long long *n = (long long *)t.raw_data;
        time_t ts = (*n) / 1000;
        tm *gmt = gmtime_r(&ts,&gmtBuf);
        buf[i] = (gmt==nullptr) ? 0 : gmt->tm_hour * 60 + gmt->tm_min;
        t.raw_data += t.stride;
    }
}

void pack64timestampToDTsecond(pack_info_t t)
{
    int *buf = (int *)t.buffer;
    tm gmtBuf;
    for (int i = 0; i != t.len; i++)
    {
        long long *n = (long long *)t.raw_data;
        time_t ts = (*n) / 1000;
        tm *gmt = gmtime_r(&ts,&gmtBuf);
        buf[i] = (gmt==nullptr) ? 0 : (gmt->tm_hour * 60 + gmt->tm_min) * 60 + gmt->tm_sec;
        t.raw_data += t.stride;
    }
}

DATA_TYPE UNIX64BitTimestampColumn::packData(pack_info_t t)
{
    switch (destType_)
    {
    case DT_DATE:
        pack64timestampToDTdate(t);
        return DT_INT;
    case DT_MONTH:
        pack64timestampToDTmonth(t);
        return DT_INT;
    case DT_TIME:
        pack64timestampToDTtime(t);
        return DT_INT;
    case DT_MINUTE:
        pack64timestampToDTminute(t);
        return DT_INT;
    case DT_SECOND:
        pack64timestampToDTsecond(t);
        return DT_INT;
    case DT_DATETIME:
        pack64timestampToDTdatetime(t);
        return DT_INT;
    case DT_TIMESTAMP:
        pack64timestampToDTtimestamp(t);
        return DT_LONG;
    case DT_NANOTIME:
        pack64timestampToDTnanotime(t);
        return DT_LONG;
    case DT_NANOTIMESTAMP:
        pack64timestampToDTnanotimestamp(t);
        return DT_LONG;
    default:
        return DT_VOID;
    }
}

bool IntegerColumn::compatible(DATA_TYPE destType) const
{
    switch (destType)
    {
    case DT_BOOL:
    case DT_CHAR:
    case DT_SHORT:
    case DT_INT:
    case DT_LONG:
    case DT_FLOAT:
    case DT_DOUBLE:
    case DT_DATE:
    case DT_MONTH:
    case DT_TIME:
    case DT_MINUTE:
    case DT_SECOND:
    case DT_DATETIME:
    case DT_TIMESTAMP:
    case DT_NANOTIME:
    case DT_NANOTIMESTAMP:
        return true;
    default:
        return false;
    }
}

bool FloatPointNumColumn::compatible(DATA_TYPE destType) const
{

    switch (destType)
    {
    case DT_BOOL:
    case DT_CHAR:
    case DT_SHORT:
    case DT_INT:
    case DT_LONG:
    case DT_FLOAT:
    case DT_DOUBLE:
        return true;
    default:
        return false;
    }
}

bool BoolColumn::compatible(DATA_TYPE destType) const
{
    switch (destType)
    {
    case DT_BOOL:
    case DT_CHAR:
    case DT_SHORT:
    case DT_INT:
    case DT_LONG:
    case DT_FLOAT:
    case DT_DOUBLE:
        return true;
    default:
        return false;
    }
}

template <class dest_type>
void packBoolTo(pack_info_t t)
{
    numeric_pack_to_bool<dest_type>(t);
}

DATA_TYPE BoolColumn::packData(pack_info_t t)
{
    switch (destType_)
    {
    case DT_BOOL:
        packBoolTo<bool>(t);
        return DT_CHAR;
    case DT_CHAR:
        packBoolTo<char>(t);
        return DT_CHAR;
    case DT_SHORT:
        packBoolTo<short>(t);
        return DT_SHORT;
    case DT_INT:
        packBoolTo<int>(t);
        return DT_INT;
    case DT_LONG:
        packBoolTo<long long>(t);
        return DT_LONG;
    case DT_FLOAT:
        packBoolTo<float>(t);
        return DT_FLOAT;
    case DT_DOUBLE:
        packBoolTo<double>(t);
        return DT_DOUBLE;
    case DT_MONTH:
        packBoolTo<int>(t);
        return DT_INT;
    case DT_TIME:
        packBoolTo<int>(t);
        return DT_INT;
    case DT_MINUTE:
        packBoolTo<int>(t);
        return DT_INT;
    case DT_SECOND:
        packBoolTo<int>(t);
        return DT_INT;
    case DT_NANOTIME:
        packBoolTo<int>(t);
        return DT_INT;
    case DT_DATE:
        packBoolTo<int>(t);
        return DT_INT;
    case DT_TIMESTAMP:
        packBoolTo<int>(t);
        return DT_INT;
    case DT_NANOTIMESTAMP:
        packBoolTo<int>(t);
        return DT_INT;
    default:
        return DT_VOID;
    }
}
}