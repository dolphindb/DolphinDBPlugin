#include "hdf5_plugin.h"

ConstantSP h5ls(const ConstantSP &h5_path)
{
    if (h5_path->getType() != DT_STRING)
        throw IllegalArgumentException(__FUNCTION__, "The argument for h5ls must be a string.");

    vector<string> objsName, objsType;
    H5PluginImp::h5ls(h5_path->getString(), objsName, objsType);

    size_t objsNum = objsName.size();

    TableSP lsResult = Util::createTable({"objName", "objType"}, {DT_STRING, DT_STRING}, objsNum, objsNum);

    VectorSP h5ObjectName = lsResult->getColumn(0);
    VectorSP h5ObjectType = lsResult->getColumn(1);

    h5ObjectName->setString(0, objsNum, objsName.data());
    h5ObjectType->setString(0, objsNum, objsType.data());

    return lsResult;
}

ConstantSP h5lsTable(const ConstantSP &filename)
{
    if (filename->getType() != DT_STRING)
        throw IllegalArgumentException(__FUNCTION__, "The argument for h5ls must be a string.");

    vector<string> datasetName, datasetDims, dataType;
    H5PluginImp::h5lsTable(filename->getString(), datasetName, datasetDims, dataType);

    size_t tableNum = datasetName.size();

    vector<string> resultColName{"tableName", "tableDims", "tableType"};
    vector<DATA_TYPE> resultColType{DT_STRING, DT_STRING, DT_STRING};

    VectorSP tableName = Util::createVector(DT_STRING, 0);
    VectorSP tableDims = Util::createVector(DT_STRING, 0);
    VectorSP tableType = Util::createVector(DT_STRING, 0);

    tableName->appendString(datasetName.data(), tableNum);
    tableDims->appendString(datasetDims.data(), tableNum);
    tableType->appendString(dataType.data(), tableNum);

    return Util::createTable(resultColName, {tableName, tableDims, tableType});
}

ConstantSP extractHDF5Schema(const ConstantSP& filename, const ConstantSP& datasetName) {
    if (filename->getType() != DT_STRING || datasetName->getType() != DT_STRING)
        throw IllegalArgumentException(__FUNCTION__, "The filename and dataset must be a string.");

    ConstantSP schema = H5PluginImp::extractHDF5Schema(filename->getString(), datasetName->getString());

    return schema;
}

ConstantSP loadHDF5(Heap *heap, vector<ConstantSP>& arguments)
{
    ConstantSP filename = arguments[0];
    ConstantSP datasetName = arguments[1];

    size_t startRow = 0;
    size_t rowNum = 0;
    ConstantSP schema = H5PluginImp::nullSP;

    if (filename->getType() != DT_STRING || datasetName->getType() != DT_STRING)
        throw IllegalArgumentException(__FUNCTION__, "The filename and dataset must be a string.");
    if (arguments.size() >= 3) {
        if (arguments[2]->isNull())
            schema = H5PluginImp::nullSP;
        else if (!arguments[2]->isTable())
            throw IllegalArgumentException(__FUNCTION__, "schema must be a table containing column names and types.");
        else
            schema = arguments[2];
    }
    if (arguments.size() >= 4) {
        if (arguments[3]->isScalar() && arguments[3]->isNumber())
            startRow = arguments[3]->getLong();
        else if (arguments[3]->isNull())
            startRow = 0;
        else
            throw IllegalArgumentException(__FUNCTION__, "startRow must be an integer.");

        if (startRow < 0)
            throw IllegalArgumentException(__FUNCTION__, "startRow must be positive.");
    }
    if (arguments.size() >= 5) {
        if (arguments[4]->isScalar() && arguments[4]->isNumber())
            rowNum = arguments[4]->getLong();
        else if (arguments[4]->isNull())
            rowNum = 0;
        else
            throw IllegalArgumentException(__FUNCTION__, "rowNum must be an integer.");

        if (rowNum < 0)
            throw IllegalArgumentException(__FUNCTION__, "rowNum must be positive.");
    }

    return H5PluginImp::loadHDF5(filename->getString(), datasetName->getString(), schema, startRow, rowNum);
}

ConstantSP loadHDF5Ex(Heap *heap, vector<ConstantSP>& arguments)
{
    ConstantSP db = arguments[0];
    ConstantSP tableName = arguments[1];
    ConstantSP partitionColumnNames = arguments[2];
    ConstantSP filename = arguments[3];
    ConstantSP datasetName = arguments[4];

    size_t startRow = 0;
    size_t rowNum = 0;
    ConstantSP schema = H5PluginImp::nullSP;

    if (!db->isDatabase())
        throw IllegalArgumentException(__FUNCTION__, "dbHandle must be a database handle.");
    if (tableName->getType() != DT_STRING)
        throw IllegalArgumentException(__FUNCTION__, "tableName must be a string.");
    if (!partitionColumnNames->isNull() && partitionColumnNames->getType() != DT_STRING)
        throw IllegalArgumentException(__FUNCTION__, "The partition columns must be in string or string vector.");
    if (filename->getType() != DT_STRING || datasetName->getType() != DT_STRING)
        throw IllegalArgumentException(__FUNCTION__, "The filename and dataset for h5read must be a string.");
    if (arguments.size() >= 6) {
        if (arguments[5]->isNull())
            schema = H5PluginImp::nullSP;
        else if (!arguments[5]->isTable())
            throw IllegalArgumentException(__FUNCTION__, "schema must be a table containing column names and types.");
        else
            schema = arguments[5];
    }
    if (arguments.size() >= 7) {
        if (arguments[6]->isScalar() && arguments[6]->isNumber())
            startRow = arguments[6]->getLong();
        else if (arguments[6]->isNull())
            startRow = 0;
        else
            throw IllegalArgumentException(__FUNCTION__, "startRow must be an integer.");

        if (startRow < 0)
            throw IllegalArgumentException(__FUNCTION__, "startRow must be nonnegative.");
    }
    if (arguments.size() >= 8) {
        if (arguments[7]->isScalar() && arguments[7]->isNumber())
            rowNum = arguments[7]->getLong();
        else if (arguments[7]->isNull())
            rowNum = 0;
        else
            throw IllegalArgumentException(__FUNCTION__, "rowNum must be an integer.");

        if (rowNum < 0)
            throw IllegalArgumentException(__FUNCTION__, "rowNum must be nonnegative.");
    }

    return H5PluginImp::loadHDF5Ex(heap, db, tableName->getString(), partitionColumnNames,
                                   filename->getString(), datasetName->getString(),
                                   schema, startRow, rowNum);
}

ConstantSP HDF5DS(Heap *heap, vector<ConstantSP>& arguments)
{
    ConstantSP filename = arguments[0];
    ConstantSP datasetName = arguments[1];

    size_t dsNum = 1;
    ConstantSP schema = H5PluginImp::nullSP;

    if (filename->getType() != DT_STRING || datasetName->getType() != DT_STRING)
        throw IllegalArgumentException(__FUNCTION__, "The filename and dataset for h5read must be a string.");
    if (arguments.size() >= 3) {
        if (arguments[2]->isNull())
            schema = H5PluginImp::nullSP;
        else if (!arguments[2]->isTable())
            throw IllegalArgumentException(__FUNCTION__, "schema must be a table containing column names and types.");
        else
            schema = arguments[2];
    }
    if (arguments.size() >= 4) {
        if (arguments[3]->isScalar() && arguments[3]->isNumber())
            dsNum = arguments[3]->getLong();
        else if (arguments[3]->isNull())
            dsNum = 1;
        else
            throw IllegalArgumentException(__FUNCTION__, "dsNum must be an integer.");
        
        if (dsNum < 1)
            throw IllegalArgumentException(__FUNCTION__, "dsNum must be positive.");
    }

    return H5PluginImp::HDF5DS(filename, datasetName, schema, dsNum);
}

namespace H5PluginImp
{

ConstantSP nullSP = Util::createNullConstant(DT_VOID);

std::string getDatasetDimsStr(hid_t loc_id, const char *name)
{

    H5DataSet dset(name, loc_id);
    H5DataSpace dspace(dset.id());

    H5S_class_t spaceClass = H5Sget_simple_extent_type(dspace.id());
    if (spaceClass == H5S_NULL)
        return "NULL";
    if (spaceClass == H5S_SCALAR)
        return "0";
    if (spaceClass == H5S_NO_CLASS)
        return "";

    std::vector<hsize_t> dims;
    dspace.currentDims(dims);

    if (dims.size() == 0)
        return "0";

    auto combineDimsOp = [](std::string &a, hsize_t b) -> std::string & {
        return a.append(1, ',').append(std::to_string(b));
    };

    std::string dimsInfo = std::to_string(dims.back());
    return std::accumulate(dims.rbegin() + 1, dims.rend(), dimsInfo, combineDimsOp);
}

std::string getDatasetNativeTypeStr(hid_t loc_id, const char *name)
{
    H5DataSet dset(name, loc_id);
    H5DataType dtype;
    dtype.openFromDataset(dset.id());
    return getHdf5NativeTypeStr(dtype);
}

std::string getHdf5ObjectTypeInfoStr(hid_t loc_id, const char *name, H5O_type_t type)
{
    if (loc_id < 0)
        return "";

    if (type == H5O_TYPE_GROUP)
        return "Group";
    else if (type == H5O_TYPE_DATASET)
        return "DataSet{(" + getDatasetDimsStr(loc_id, name) + ")}";
    else if (type == H5O_TYPE_NAMED_DATATYPE)
        return "NamedDataType";

    return "";
}

void h5lsTable(const string &filename, vector<string> &datasetName, vector<string> &datasetDims,
               vector<string> &dataType)
{
    H5ReadOnlyFile file(filename);
    struct h5_dataset_table_info
    {
        std::vector<string> nameVal;
        std::vector<string> dimsVal;
        std::vector<string> typeVal;
    } datasetTableInfo;

    H5O_iterate_t callback = [](hid_t obj, const char *name, const H5O_info_t *info, void *op_data) -> herr_t {
        if (info->type != H5O_TYPE_DATASET)
            return 0;

        h5_dataset_table_info &dataset_info = *(static_cast<h5_dataset_table_info *>(op_data));

        dataset_info.nameVal.push_back(name[0] == '.' ? "/" : std::string("/") + name);
        dataset_info.dimsVal.push_back(getDatasetDimsStr(obj, name));
        dataset_info.typeVal.push_back(getDatasetNativeTypeStr(obj, name));

        return 0;
    };

    H5Ovisit(file.id(), H5_INDEX_CRT_ORDER, H5_ITER_NATIVE, callback, &datasetTableInfo);

    datasetName = std::move(datasetTableInfo.nameVal);
    datasetDims = std::move(datasetTableInfo.dimsVal);
    dataType = std::move(datasetTableInfo.typeVal);
}

void h5ls(const string &filename, vector<string> &objsName, vector<string> &objsType)
{
    H5ReadOnlyFile file(filename);

    struct h5_objects_name_and_type
    {
        std::vector<string> nameVal;
        std::vector<string> typeVal;
    } nameAndType;

    H5O_iterate_t callback = [](hid_t obj, const char *name, const H5O_info_t *info, void *op_data) -> herr_t {
        using std::vector;
        h5_objects_name_and_type &name_and_type = *(static_cast<h5_objects_name_and_type *>(op_data));

        name_and_type.nameVal.push_back(name[0] == '.' ? "/" : std::string("/") + name);
        name_and_type.typeVal.push_back(getHdf5ObjectTypeInfoStr(obj, name, info->type));

        return 0;
    };

    H5Ovisit(file.id(), H5_INDEX_CRT_ORDER, H5_ITER_NATIVE, callback, &nameAndType);

    objsName = std::move(nameAndType.nameVal);
    objsType = std::move(nameAndType.typeVal);
}

std::string getLayoutColumnType(hdf5_type_layout &layout)
{
    switch (layout.flag)
    {
    case IS_FIXED_STR:
    case IS_VARIABLE_STR:
        return "STRING";
    case IS_ENUM:
        return "SYMBOL";
    case IS_S_CHAR_INTEGER:
        return "CHAR";
    case IS_U_CHAR_INTEGER:
    case IS_S_SHORT_INTEGER:
        return "SHORT";
    case IS_S_INT_INTEGER:
    case IS_U_SHORT_INTEGER:
        return "INT";
    case IS_U_INT_INTEGER:
    case IS_S_LLONG_INTEGER:
        return "LONG";
    case IS_FLOAT_FLOAT:
        return "FLOAT";
    case IS_DOUBLE_FLOAT:
        return "DOUBLE";
    case IS_UNIX_TIME:
        return "TIMESTAMP";
    case IS_U_LLONG_INTEGER:
    case IS_COMPOUND:
    case IS_ARRAY:
    default:
        throw RuntimeException("unsupported data type");
    }
}

TableSP generateSimpleDatasetSchema(const int colNum, H5DataType &srcType) {
    hid_t t = -1;
    H5DataType srcNativeType;
    hdf5_type_layout srcNativeTypeLayout;

    hid_t sid = srcType.id();

    bool isTime = isClassEqual(sid, H5T_TIME);

    if (!isTime && acceptNativeType(srcNativeType, sid))
        t = srcNativeType.id();
    else
        t = srcType.id();

    if (!getHdf5SimpleLayout(t, srcNativeTypeLayout))
        return nullSP;

    string destType = getLayoutColumnType(srcNativeTypeLayout);

    vector<ConstantSP> cols(2);
    cols[0] = Util::createVector(DT_STRING, colNum, colNum);
    cols[1] = Util::createVector(DT_STRING, colNum, colNum);

    ConstantSP type = new String(destType);

    for (int i = 0; i < colNum; i++) {
        cols[0]->set(i, new String("col_" + std::to_string(i)));
        cols[1]->set(i, type);
    }

    vector<string> colNames(2);
    colNames[0] = "name";
    colNames[1] = "type";

    return Util::createTable(colNames, cols);
}

void generateComplexTypeBasedColsNameAndColsType(vector<string> &colsName, vector<string> &colsType, hid_t type);

TableSP generateCompoundDatasetSchema(H5DataType &type) {
    std::vector<H5ColumnSP> h5Cols;
    H5DataType convertedType;

    if (!TypeColumn::createComplexColumns(type, h5Cols, convertedType))
        throw RuntimeException("unsupported data type");

    vector<string> names, types;
    generateComplexTypeBasedColsNameAndColsType(names, types, convertedType.id());
    const int colNum = names.size();

    vector<ConstantSP> cols(2);
    cols[0] = Util::createVector(DT_STRING, colNum, colNum);
    cols[1] = Util::createVector(DT_STRING, colNum, colNum);

    for (int i = 0; i < colNum; i++)
    {
        ConstantSP name = new String(names[i]);
        ConstantSP type = new String(types[i]);
        type->setString(types[i]);
        cols[0]->set(i, name);
        cols[1]->set(i, type);
    }

    vector<string> colNames(2);
    colNames[0] = "name";
    colNames[1] = "type";

    return Util::createTable(colNames, cols);
}

TableSP extractHDF5Schema(const string &filename, const string &datasetName)
{
    H5ReadOnlyFile f(filename);
    H5DataSet s(datasetName, f.id());
    H5DataSpace dspace(s.id());

    const int rank = dspace.rank();
    int colNum;

    if (rank > 2)
        throw RuntimeException("rank of dataspace > 2");

    vector<hsize_t> dims;
    dspace.currentDims(dims);

    H5DataType t;
    t.openFromDataset(s.id());

    registerUnixTimeConvert();

    switch (H5Tget_class(t.id()))
    {
    case H5T_INTEGER:
    case H5T_FLOAT:
    case H5T_TIME:
    case H5T_STRING:
    case H5T_ENUM:
        if (rank <= 1)
            colNum = 1;
        else
            colNum = dims[1];
        return generateSimpleDatasetSchema(colNum, t);
    case H5T_COMPOUND:
    case H5T_ARRAY:
        return generateCompoundDatasetSchema(t);
    default:
        throw RuntimeException("unsupported type");
    }
}

int _generateColNameFromArrayInCompound(vector<string> &colsName, vector<string> &colsType, vector<hdf5_type_layout> &layout,
                                        const string &prefix, const string &suffix, size_t start, int belong_to);

int _generateColNameFromCompoundMember(vector<string> &colsName, vector<string> &colsType, vector<hdf5_type_layout> &layout,
                                       const string &prefix, const string &suffix, size_t start, int belong_to)
{
    for (size_t i = start; i < layout.size(); i++)
    {

        if (layout[i].flag == IS_COMPOUND)
        {
            std::string p = prefix + "_C" + layout[i].name;
            i = _generateColNameFromCompoundMember(colsName, colsType, layout, p, suffix, i + 1, i) - 1;
            continue;
        }
        else if (layout[i].flag == IS_ARRAY)
        {
            std::string p = prefix + "_A" + layout[i].name;
            i = _generateColNameFromArrayInCompound(colsName, colsType, layout, p, suffix, i + 1, i) - 1;
            continue;
        }

        if (layout[i].belong_to != belong_to)
            return i;

        colsName.push_back(prefix + "_" + layout[i].name + suffix);
        colsType.push_back(getLayoutColumnType(layout[i]));
    }
    return layout.size();
}

int _generateColNameFromArrayInCompound(vector<string> &colsName, vector<string> &colsType, vector<hdf5_type_layout> &layout,
                                        const string &prefix, const string &suffix, size_t start, int belong_to)
{
    for (size_t i = start; i < layout.size(); i++)
    {
        std::string s = std::to_string(i - start) + suffix;
        if (layout[i].flag == IS_COMPOUND)
        {
            std::string p = prefix + "_C" + layout[i].name;
            i = _generateColNameFromCompoundMember(colsName, colsType, layout, p, s, i + 1, i) - 1;
            continue;
        }
        else if (layout[i].flag == IS_ARRAY)
        {
            std::string p = prefix + "_A" + layout[i].name;
            i = _generateColNameFromArrayInCompound(colsName, colsType, layout, p, suffix, i + 1, i) - 1;
            continue;
        }

        if (layout[i].belong_to < belong_to)
            return i;
        colsName.push_back(prefix + "_" + s);
        colsType.push_back(getLayoutColumnType(layout[i]));
    }
    return layout.size();
}

void generateColNameFromCompoundMember(vector<string> &colsName, vector<string> &colsType, hid_t type)
{
    vector<hdf5_type_layout> layout;
    getHdf5ComplexLayout(type, layout);
    for (size_t i = 0; i < layout.size(); i++)
    {
        if (layout[i].flag == IS_COMPOUND)
        {
            i = _generateColNameFromCompoundMember(colsName, colsType, layout, "C" + layout[i].name, "", i + 1, i) - 1;
            continue;
        }
        else if (layout[i].flag == IS_ARRAY)
        {
            i = _generateColNameFromArrayInCompound(colsName, colsType, layout, "A" + layout[i].name, "", i + 1, i) - 1;
            continue;
        }

        colsName.push_back(layout[i].name);
        colsType.push_back(getLayoutColumnType(layout[i]));
    }
}

void generateColNameFromArrayMember(vector<string> &colsName, vector<string> &colsType, hid_t array_type)
{
    H5DataType base_type;
    base_type.accept(H5Tget_super(array_type));

    size_t cols_begin = colsName.size();

    hid_t sid = base_type.id();
    hid_t t = -1;
    H5DataType srcNativeType;
    hdf5_type_layout srcNativeTypeLayout;
    acceptNativeType(srcNativeType, sid);
    t = srcNativeType.id();
    getHdf5SimpleLayout(t, srcNativeTypeLayout);
    string type = getLayoutColumnType(srcNativeTypeLayout);

    if (isClassEqual(base_type.id(), H5T_COMPOUND))
        generateColNameFromCompoundMember(colsName, colsType, base_type.id());
    else if (isClassEqual(base_type.id(), H5T_ARRAY))
        generateColNameFromArrayMember(colsName, colsType, base_type.id());
    else {
        colsName.push_back("array");
        colsType.push_back(type);
    }

    int n = getArrayElementNum(array_type);
    size_t ele_cols_num = colsName.size() - cols_begin;

    for (int i = 1; i < n; i++)
        for (size_t k = 0; k != ele_cols_num; k++) {
            colsName.push_back(colsName[k]);
            colsType.push_back(type);
        }

    for (int i = 0; i < n; i++)
        for (size_t k = 0; k != ele_cols_num; k++)
            colsName[cols_begin + k + i * ele_cols_num].append("_").append(std::to_string(i));
}

void generateComplexTypeBasedColsName(vector<string> &cols_name, hid_t type)
{
    vector<string> colsType;
    if (isClassEqual(type, H5T_COMPOUND))
        return generateColNameFromCompoundMember(cols_name, colsType, type);
    if (isClassEqual(type, H5T_ARRAY))
        return generateColNameFromArrayMember(cols_name, colsType, type);
}

void generateComplexTypeBasedColsNameAndColsType(vector<string> &colsName, vector<string> &colsType, hid_t type)
{
    if (isClassEqual(type, H5T_COMPOUND))
        return generateColNameFromCompoundMember(colsName, colsType, type);
    if (isClassEqual(type, H5T_ARRAY))
        return generateColNameFromArrayMember(colsName, colsType, type);
}

bool colsNumEqual(TableSP t, int ncols)
{
    return t->columns() == ncols;
}

void generateIncrementedColsName(std::vector<string> &cols, int size)
{
    cols.resize(size, "col_");
    for (size_t i = 0; i != cols.size(); i++)
        cols[i].append(std::to_string(i));
}

inline std::string typeIncompatibleErrorMsg(int idx, DATA_TYPE src, VectorSP destVec)
{
    DATA_TYPE dest = (destVec == nullptr) ? src : destVec->getType();

    return "uncompatible type in column " + std::to_string(idx) + " " +
           Util::getDataTypeString(src) + "->" + Util::getDataTypeString(dest);
}

bool createColumnVec(vector<H5ColumnSP> &cols, size_t num, size_t cap, H5DataType &src, TableSP dest)
{
    cols.resize(num);
    for (size_t i = 0; i != num; i++)
    {
        cols[i] = TypeColumn::createNewColumn(src);
        if (cols[i] == nullptr)
            throw RuntimeException("can't create new column");

        VectorSP destVec = dest->isNull() ? nullSP : dest->getColumn(i);
        VectorSP v = cols[i]->createDolphinDBColumnVector(destVec, 0, cap);

        if (v->isNull())
            throw RuntimeException(typeIncompatibleErrorMsg(i, cols[i]->srcType(), destVec));
    }
    return true;
}

bool createColumnVec(vector<H5ColumnSP> &cols, size_t cap, TableSP dest)
{
    for (size_t i = 0; i != cols.size(); i++)
    {
        if (cols[i] == nullptr)
            throw RuntimeException("unknown error:col can't be null"); //it should't be executed....just check

        VectorSP destVec = dest->isNull() ? nullSP : dest->getColumn(i);
        VectorSP v = cols[i]->createDolphinDBColumnVector(destVec, 0, cap);

        if (v->isNull())
            throw RuntimeException(typeIncompatibleErrorMsg(i, cols[i]->srcType(), destVec));
    }
    return true;
}

typedef SmartPointer<DatasetAppender> DatasetAppenderSP;

TableSP appendColumnVecToTable(TableSP tb, vector<ConstantSP> &colVec)
{
    if (tb->isNull())
        return tb;

    string errMsg;
    INDEX insertedRows = 0;
    if (!tb->append(colVec, insertedRows, errMsg))
        throw TableRuntimeException(errMsg);
    return tb;
}

void doReadDataset(H5GeneralDataReader &reader, DatasetAppenderSP appender,
                   vector<H5ColumnSP> &cols, vector<ConstantSP> &colVec)
{
    size_t eleNum = 0;
    size_t eleByteLength = reader.elementByteLength();

    appender->setColumns(cols.data(), cols.size(), eleByteLength);

    while ((eleNum = reader.read()) != 0)
    {
        appender->updateRawData(reader.rawData<char *>(), eleNum);
        appender->append();
    }

    colVec.resize(cols.size());
    for (size_t i = 0; i != colVec.size(); i++)
        colVec[i] = cols[i]->colVec();
}

void doReadDataset_concurrent(H5GeneralDataReader &reader, DatasetAppenderSP appender,
                              vector<H5ColumnSP> &cols, vector<ConstantSP> &colVec)
{
    size_t eleNum = 0;
    size_t eleByteLength = reader.elementByteLength();

    appender->setColumns(cols.data(), cols.size(), eleByteLength);

    vlen_mem backMem;
    backMem.pre_alloc_buffer.resize(reader.preAllocVlenBufferByteLength());
    std::vector<char> backBuffer(reader.bufferByteLength());

    Thread t(appender);

    while ((eleNum = reader.read()) != 0)
    {
        t.join();

        freeVlenMemory(backMem);
        reader.swap_buffer(backBuffer, backMem);
        appender->updateRawData(backBuffer.data(), eleNum);

        t.start();
    }
    t.join();
    freeVlenMemory(backMem);

    colVec.resize(cols.size());
    for (size_t i = 0; i != colVec.size(); i++)
        colVec[i] = cols[i]->colVec();
}

void getRowAndColNum(const hid_t set, vector<size_t> &rowAndColNum) {
    H5DataSpace dspace(set);
    std::vector<hsize_t> dims;
    dspace.currentDims(dims);
    if (dspace.rank() == 0) {
        dims.push_back(1);
        dims.push_back(1);
    }
    if (dspace.rank() == 1)
        dims.push_back(1);
    rowAndColNum.push_back(dims[0]);
    rowAndColNum.push_back(dims[1]);
}

void getRowAndColNum(const string &filename, const string &datasetName, vector<size_t> &rowAndColNum) {
    H5ReadOnlyFile f(filename);
    H5DataSet set(datasetName, f.id());
    getRowAndColNum(set.id(), rowAndColNum);
}

size_t getDatasetSize(const hid_t set) {
    H5DataType t;
    t.openFromDataset(set);

    registerUnixTimeConvert();

    switch (H5Tget_class(t.id()))
    {
    case H5T_INTEGER:
    case H5T_FLOAT:
    case H5T_TIME:
    case H5T_STRING:
    case H5T_ENUM:
    {
        H5DataType convertedType;

        if (!TypeColumn::convertHdf5SimpleType(t, convertedType))
            throw RuntimeException("unsupported data type");

        H5GeneralDataReader reader(set, 1024 * 1024 * 32, 1024 * 1024 * 32, convertedType.id());
        return reader.totalSize();
    }
    case H5T_COMPOUND:
    case H5T_ARRAY:
    {
        std::vector<H5ColumnSP> cols;
        H5DataType convertedType;

        if (!TypeColumn::createComplexColumns(t, cols, convertedType))
            throw RuntimeException("unsupported data type");

        H5GeneralDataReader reader(set, 1024 * 1024 * 32, 1024 * 1024 * 32, convertedType.id());
        return reader.totalSize();
    }
    default:
        throw RuntimeException("unsupported type");
    }
}

size_t getDatasetSize(const string &filename, const string &datasetName) {
    H5ReadOnlyFile f(filename);
    H5DataSet set(datasetName, f.id());

    return getDatasetSize(set.id());
}

TableSP readSimpleDataset(const hid_t set, H5DataType &type, TableSP tb, size_t startRow, size_t readRowNum)
{
    H5DataType convertedType;

    if (!TypeColumn::convertHdf5SimpleType(type, convertedType))
        throw RuntimeException("unsupported data type");

    vector<size_t> rowAndColNum;
    getRowAndColNum(set, rowAndColNum);
    size_t rowNum = rowAndColNum[0];
    size_t colNum = rowAndColNum[1];
    size_t startElement = startRow * colNum;
    size_t endElement;
    if (readRowNum == 0 || readRowNum > rowNum)
        endElement = rowNum * colNum;
    else
        endElement = (startRow + readRowNum) * colNum;

    H5BlockDataReader reader(set, startElement, endElement,
                             1024 * 1024 * 32, 1024 * 1024 * 32, convertedType.id());

    if (!reader.columnNum() || !reader.rowNum())
        throw RuntimeException("empty dataset!");

    if (!tb->isNull() && !colsNumEqual(tb, reader.columnNum()))
        throw RuntimeException("the columns of the table don't match the dataset");

    vector<H5ColumnSP> cols;
    createColumnVec(cols, reader.columnNum(), reader.rowNum(), type, tb);

    vector<ConstantSP> colVec;
    DatasetAppenderSP appender = new SimpleDatasetAppender();
    doReadDataset(reader, appender, cols, colVec);

    if (tb->isNull())
    {
        vector<string> colsName;
        generateIncrementedColsName(colsName, colVec.size());
        return Util::createTable(colsName, colVec);
    }

    return appendColumnVecToTable(tb, colVec);
}

TableSP readComplexDataset(const hid_t set, H5DataType &type, TableSP tb, size_t startRow, size_t readRowNum)
{
    std::vector<H5ColumnSP> cols;
    H5DataType convertedType;

    if (!TypeColumn::createComplexColumns(type, cols, convertedType))
        throw RuntimeException("unsupported data type");

    vector<size_t> rowAndColNum;
    getRowAndColNum(set, rowAndColNum);
    size_t rowNum = rowAndColNum[0];
    size_t colNum = rowAndColNum[1];
    size_t startElement = startRow * colNum;
    size_t endElement;
    if (readRowNum == 0 || readRowNum > rowNum)
        endElement = rowNum * colNum;
    else
        endElement = (startRow + readRowNum) * colNum;

    H5BlockDataReader reader(set, startElement, endElement,
                             1024 * 1024 * 32, 1024 * 1024 * 32, convertedType.id());

    if (!reader.columnNum() || !reader.rowNum())
        throw RuntimeException("empty dataset!");

    if (!tb->isNull() && !colsNumEqual(tb, cols.size()))
        throw RuntimeException("the columns of the table don't match the dataset");

    createColumnVec(cols, reader.rowNum() * reader.columnNum(), tb);

    vector<ConstantSP> colVec;
    DatasetAppenderSP appender = new ComplexDatasetAppender();
    doReadDataset(reader, appender, cols, colVec);

    if (tb->isNull())
    {
        vector<string> colsName;
        generateComplexTypeBasedColsName(colsName, convertedType.id());
        return Util::createTable(colsName, colVec);
    }

    return appendColumnVecToTable(tb, colVec);
}

ConstantSP loadHDF5(const hid_t set, const ConstantSP &schema, const size_t startRow, const size_t rowNum) {
    H5DataType t;
    t.openFromDataset(set);

    registerUnixTimeConvert();

    TableSP tableWithSchema = schema->isNull() ?
        static_cast<TableSP>(nullSP) : DBFileIO::createEmptyTableFromSchema(schema);

    switch (H5Tget_class(t.id()))
    {
    case H5T_INTEGER:
    case H5T_FLOAT:
    case H5T_TIME:
    case H5T_STRING:
    case H5T_ENUM:
        return readSimpleDataset(set, t, tableWithSchema, startRow, rowNum);
    case H5T_COMPOUND:
    case H5T_ARRAY:
        return readComplexDataset(set, t, tableWithSchema, startRow, rowNum);

    default:
        throw RuntimeException("unsupported type");
    }
}

ConstantSP loadHDF5(const string &filename, const string &datasetName,
                    const ConstantSP &schema, const size_t startRow, const size_t rowNum)
{
    H5ReadOnlyFile f(filename);
    H5DataSet set(datasetName, f.id());
    return loadHDF5(set.id(), schema, startRow, rowNum);
}

ConstantSP loadFromH5ToDatabase(Heap *heap, vector<ConstantSP> &arguments)
{
    hid_t setId = static_cast<hid_t>(arguments[0]->getLong());
    TableSP schema = static_cast<TableSP>(arguments[1]);
    size_t startRow = arguments[2]->getLong();
    size_t rowNum = arguments[3]->getLong();
    SystemHandleSP db = static_cast<SystemHandleSP>(arguments[4]);
    string tableName = arguments[5]->getString();

    bool diskSeqMode = !db->getDatabaseDir().empty() &&
                       db->getDomain()->getPartitionType() == SEQ;

    TableSP loadedTable = loadHDF5(setId, schema, startRow, rowNum);

    if (diskSeqMode) {
        string id = db->getDomain()->getPartition(arguments[6]->getInt())->getPath();
        string directory = db->getDatabaseDir() + "/" + id;
        if (!DBFileIO::saveBasicTable(heap->currentSession(), directory, loadedTable.get(), tableName, NULL, true, 1, false))
            throw RuntimeException("Failed to save the table to directory " + directory);
        return new Long(loadedTable->rows());
    }
    else
        return loadedTable;
}

void getColNamesAndTypesFromSchema(const TableSP &schema, vector<string> &colNames,
                                   vector<DATA_TYPE> &types)
{
    ConstantSP vecName = schema->getColumn(0);
    ConstantSP vecType = schema->getColumn(1);
    int rows = vecName->size();
    for (int i = 0; i < rows; ++i) {
        colNames.push_back(vecName->getString(i));
        types.push_back(Util::getDataType(vecType->getString(i)));
        if (types.back() <= DT_VOID || types.back() > DT_STRING)
            throw RuntimeException("Invalid data type '" + vecType->getString(i) + "'");
    }
}

vector<DistributedCallSP> generateH5Tasks(Heap* heap, const hid_t set,
    const TableSP &schema, const size_t startRow, const size_t rowNum,
    const SystemHandleSP &db, const string &tableName)
{
    vector<string> colNames;
    vector<DATA_TYPE> types;

    if (!schema->isNull())
		getColNamesAndTypesFromSchema(schema, colNames, types);

    vector<size_t> rowAndColNum;
    getRowAndColNum(set, rowAndColNum);

    size_t estimatedRows = rowAndColNum[0] - startRow;
    if (rowNum != 0)
        estimatedRows = std::min(rowNum, estimatedRows);

    int partitions = 0;
    DomainSP domain = db->getDomain();

    if (domain->getPartitionType() == SEQ) {
        partitions = domain->getPartitionCount();
        if (partitions <= 1)
            throw IOException("The database must have at least two partitions.");
        if ((estimatedRows / partitions) < 65536)
            throw IOException("The number of rows per partition is too small (<65,536) and the hdf5 file can't be partitioned.");
    }
    if (partitions == 0) {
        long long fileSize = getDatasetSize(set);
        long long maxSizePerPartition = 128 * 1024 * 1024;
        int defaultPartitions = 4;//GOContainer::LOCAL_EXECUTOR_SIZE + 1;
        long long sizePerPartition = fileSize / defaultPartitions;
        if (sizePerPartition < 16 * 1024 * 1024)
            partitions = (fileSize / (16 * 1024 * 1024)) + 1;
        else if (sizePerPartition > maxSizePerPartition)
            partitions = (fileSize / maxSizePerPartition) + 1;
        else
            partitions = defaultPartitions;
    }

    int columns=colNames.size();
	vector<ConstantSP> cols(columns);

    vector<DistributedCallSP> tasks;

    size_t rowIncrement = std::ceil(static_cast<double>(estimatedRows) / partitions);
    size_t currentStartRow = startRow;
    ConstantSP _rowIncrement = new Long(rowIncrement);
    ConstantSP setId = new Long(set);
    ConstantSP _tableName = new String(tableName);
    FunctionDefSP func = Util::createSystemFunction("loadFromH5ToDatabase", loadFromH5ToDatabase, 7, 7, false);
    for (int i = 0; i < partitions; i++) {
        ConstantSP _startRow = new Long(currentStartRow);
        ConstantSP id = new Int(i);
        vector<ConstantSP> args{setId, schema,
                                _startRow, _rowIncrement, db, _tableName, id};
        ObjectSP call = Util::createRegularFunctionCall(func, args);
        DistributedCallSP task = new DistributedCall(call, true);
        tasks.push_back(task);
        currentStartRow += rowIncrement;
    }
    return tasks;
}

TableSP generateInMemoryParitionedTable(Heap *heap, const SystemHandleSP &db,
                                        const ConstantSP &tables, const ConstantSP &partitionNames)
{
    FunctionDefSP createPartitionedTable = heap->currentSession()->getFunctionDef("createPartitionedTable");
    ConstantSP emptyString = new String("");
    vector<ConstantSP> args{db, tables, emptyString, partitionNames};
    return createPartitionedTable->call(heap, args);
}

ConstantSP loadHDF5Ex(Heap *heap, const SystemHandleSP &db, const string &tableName, const ConstantSP &partitionColumns,
                      const string &filename, const string &datasetName, const TableSP &schema,
                      const size_t startRow, const size_t rowNum)
{
    const TableSP convertedSchema = schema->isNull() ? extractHDF5Schema(filename, datasetName) : schema;
    H5ReadOnlyFile f(filename);
    H5DataSet dataset(datasetName, f.id());
    vector<DistributedCallSP> tasks = generateH5Tasks(heap, dataset.id(), convertedSchema, startRow, rowNum, db, tableName);
    int partitions = tasks.size();

    StaticStageExecutor executor(true, false, false);
    executor.execute(heap, tasks);
    for (int i = 0; i < partitions; i++) {
        const string &errMsg = tasks[i]->getErrorMessage();
        if (!errMsg.empty())
            throw RuntimeException(errMsg);
    }

    string owner = heap->currentSession()->getUser()->getUserId();
    DomainSP domain = db->getDomain();
    bool seqDomain = domain->getPartitionType() == SEQ;
    bool inMemory = db->getDatabaseDir().empty();
    ConstantSP tableName_ = new String(tableName);

    if (seqDomain) {
        if (inMemory) {
            ConstantSP tmpTables = Util::createVector(DT_ANY, partitions);
            for (int i = 0; i < partitions; i++)
                tmpTables->set(i, tasks[i]->getResultObject());
            ConstantSP partitionNames = new String("");
            return generateInMemoryParitionedTable(heap, db, tmpTables, partitionNames);
        }
        else {
            vector<int> partitionColumnIndices(1, -1);
            vector<int> baseIds;
            int baseId = -1;
            // db->getSymbolBaseManager()->getSymbolBases(baseIds);
            // if (!baseIds.empty()) {
            //     string errMsg;
            //     baseId = baseIds.back();
            //     if (!db->getSymbolBaseManager()->saveSymbolBase(db->getSymbolBaseManager()->findAndLoad(baseId), errMsg))
            //         throw IOException("Failed to save symbol base: " + errMsg);
            // }

            string tableFile = db->getDatabaseDir() + "/" + tableName + ".tbl";
            vector<ColumnDesc> cols;
            int columns = convertedSchema->rows();
            for(int i=0; i<columns; ++i){
                string name = convertedSchema->getColumn(0)->getString(i);
                DATA_TYPE type = Util::getDataType(convertedSchema->getColumn(1)->getString(i));
                int extra = type == DT_SYMBOL ? baseId : -1;
				cols.push_back(ColumnDesc(name, type, extra));
			}

            if (!DBFileIO::saveTableHeader(owner, cols, partitionColumnIndices, 0, tableFile, NULL))
                throw IOException("Failed to save table header " + tableFile);
            if (!DBFileIO::saveDatabase(db.get()))
                throw IOException("Failed to save database " + db->getDatabaseDir());
            db->getDomain()->addTable(tableName, owner, cols, partitionColumnIndices);
            vector<ConstantSP> loadTableArgs = {db, tableName_};
            return heap->currentSession()->getFunctionDef("loadTable")->call(heap, loadTableArgs);
            // return DBFileIO::loadTable(heap->currentSession(), db.get(), tableName, nullSP, SEGTBL, false);
        }
    }
    else {
        string dbPath = db->getDatabaseDir();
        vector<ConstantSP> existsTableArgs = {new String(dbPath), tableName_};
        bool existsTable = heap->currentSession()->getFunctionDef("existsTable")->call(heap, existsTableArgs)->getBool();
        ConstantSP result;

        if (existsTable) {
            vector<ConstantSP> loadTableArgs = {db, tableName_};
            result = heap->currentSession()->getFunctionDef("loadTable")->call(heap, loadTableArgs);
        }
        else {
            TableSP schema = extractHDF5Schema(filename, datasetName);
            ConstantSP dummyTable = DBFileIO::createEmptyTableFromSchema(schema);
            vector<ConstantSP> createTableArgs = {db, dummyTable, tableName_, partitionColumns};
            result = heap->currentSession()->getFunctionDef("createPartitionedTable")->call(heap, createTableArgs);
        }

        FunctionDefSP append = heap->currentSession()->getFunctionDef("append!");
        for (int i = 0; i < partitions; ++i) {
            ConstantSP table = tasks[i]->getResultObject();
            vector<ConstantSP> appendArgs = {result, table};
            append->call(heap, appendArgs);
        }
        if (!inMemory) {
            vector<ConstantSP> loadTableArgs = {db, tableName_};
            result = heap->currentSession()->getFunctionDef("loadTable")->call(heap, loadTableArgs);
        }
        return result;
    }
}

ConstantSP HDF5DS(const ConstantSP &filename, const ConstantSP &datasetName,
                  const ConstantSP &schema, const size_t dsNum)
{
    vector<size_t> rowAndColNum;
    getRowAndColNum(filename->getString(), datasetName->getString(), rowAndColNum);
    size_t rowNum = rowAndColNum[0];
    size_t rowIncrement = std::ceil(static_cast<double>(rowNum) / dsNum);
    ConstantSP _rowIncrement = new Long(rowIncrement);

    ConstantSP dataSources = Util::createVector(DT_ANY, dsNum);
    size_t startRow = 0;

    FunctionDefSP _loadHDF5 = Util::createSystemFunction("loadHDF5", ::loadHDF5, 2, 5, false);
    for (size_t i = 0; i < dsNum; i++)
    {
        ConstantSP _startRow = new Long(startRow);
        vector<ConstantSP> args{filename, datasetName, schema, _startRow, _rowIncrement};
        ObjectSP code = Util::createRegularFunctionCall(_loadHDF5, args);
        ConstantSP ds = new DataSource(code);
        dataSources->set(i, ds);
        startRow += rowIncrement;
    }

    return dataSources;
}

void groupBySorting(vector<ConstantSP> &vecs, VectorSP &indices, vector<INDEX> &groups) {
    VectorSP vec = vecs[0];
    vec->sort(true, indices.get());
    vector<pair<INDEX, INDEX>> duplicates;
    vector<pair<INDEX, INDEX>> uniques;
    vec->findDuplicatedElements(0, vec->size(), duplicates);
    vec->findUniqueElements(0, vec->size(), uniques);
    auto duplicateIt = duplicates.begin();
    auto uniqueIt = uniques.begin();
    while (duplicateIt != duplicates.end() || uniqueIt != uniques.end()) {
        if (duplicateIt == duplicates.end())
            for (; uniqueIt != uniques.end(); uniqueIt++)
                groups.push_back((*uniqueIt).first + 1);
        else if (uniqueIt == uniques.end())
            for (; duplicateIt != duplicates.end(); duplicateIt++)
                groups.push_back((*duplicateIt).first + (*duplicateIt).second);
        else {
            INDEX duplicateStart = (*duplicateIt).first;
            INDEX uniqueStart = (*uniqueIt).first;
            if (duplicateStart < uniqueStart) {
                groups.push_back((*duplicateIt).first + (*duplicateIt).second);
                duplicateIt++;
            }
            else {
                groups.push_back((*uniqueIt).first + 1);
                uniqueIt++;
            }
        }
    }
}

void SegmentedInMemoryTableBuiler::insert(TableSP& curTable){
	DomainSP domain = db_->getDomain();
	vector<DomainSP> targetDomains;
	PARTITION_TYPE dbPartitionType = domain->getPartitionType();
	int dimCount = domain->getPartitionDimensions();
	if(dbPartitionType == HIER){
		// HierarchicalDomain* hier = (HierarchicalDomain*)domain.get();
		// for(int i=0; i<dimCount; ++i)
		// 	targetDomains.push_back(hier->getDimensionalDomain(i));
        throw RuntimeException("HIER domain not supported.");
	}
	else
		targetDomains.push_back(domain);

	vector<ConstantSP> vecs;
	for(int j=0; j<dimCount; ++j) {
		vecs.push_back(targetDomains[j]->getPartitionKey(curTable->getColumn(partitionColumnIndices_[j])));
	}
    VectorSP indices(Util::createIndexVector(0, curTable->size()));
	vector<INDEX> groups;
    // SQLAlgo::groupBySorting(vecs, indices, true, groups);
    groupBySorting(vecs, indices, groups);
	int groupNum = groups.size();
	vector<DomainPartitionSP> partitions;

    INDEX start = 0;
	for(int k=0; k<groupNum; ++k){
		bool skip = false;
		if(dbPartitionType == HIER){
            // for(int j=0; j<dimCount; ++j){
            // 	PARTITION_TYPE curType = targetDomains[j]->getPartitionType();
            // 	INDEX index = (j == 0 ? start : indices->getIndex(start));
            // 	if((curType != VALUE && vecs[j]->getInt(index) < 0) || (curType == VALUE && vecs[j]->get(index)->isNull())){
            // 		skip = true;
            // 		break;
            // 	}
            // }
            throw RuntimeException("HIER domain not supported.");
        }
		else if((dbPartitionType != VALUE && vecs[0]->getInt(start) < 0) || (dbPartitionType == VALUE && vecs[0]->get(start)->isNull())){
			skip = true;
		}
		if(skip){
			start = groups[k];
			continue;
		}

		TableSP curPartition;
		if(groupNum == 1)
			curPartition = curTable;
		else
			curPartition = curTable->get(indices->getSubVector(start, groups[k] - start));
		string path;
		int key;
		for(int j=0; j<dimCount; ++j){
			partitions.clear();
			targetDomains[j]->retrievePartitionAt(curPartition->getColumn(partitionColumnIndices_[j])->get(0), partitions, false);
			if(partitions.empty()){
				break;
			}
			if(j > 0)
				path.append(1,'/');
			path.append(partitions[0]->getPath());
			key = partitions[0]->getKey();
		}
		if(partitions.empty())
			continue;

		string errMsg;
		int inserted;
		if(dimCount == 1){
            unordered_map<int, int>::const_iterator it = keyMap_.find(key);
            if(it == keyMap_.end()){
				int index = partitions_.size();
				partitions_.push_back(curPartition);
				segmentKeys_.push_back(key);
				keyMap_[key] = index;
			}
			else{
				vector<ConstantSP> args(1, curPartition);
				if(!partitions_[it->second]->append(args, inserted, errMsg))
					throw RuntimeException(errMsg);
			}
		}
		else {
            throw RuntimeException("HIER domain not supported.");
            // unordered_map<string, int>::const_iterator it = pathMap_.find(path);
			// if(it == pathMap_.end()){
			// 	int index = partitions_.size();
			// 	partitions_.push_back(curPartition);
			// 	segmentPaths_.push_back(path);
			// 	pathMap_[path] = index;
			// }
			// else{
			// 	vector<ConstantSP> args(1, curPartition);
			// 	if(!partitions_[it->second]->append(args, inserted, errMsg))
			// 		throw RuntimeException(errMsg);
			// }
		}
		start = groups[k];
	}
}

ConstantSP SegmentedInMemoryTableBuiler::getSegmentedTable() {
    int partitionCount = db_->getDomain()->getPartitionCount();
    ConstantSP tables = Util::createVector(DT_ANY, partitionCount);
    for (int i = 0; i < partitionCount; i++) {
        int key = db_->getDomain()->getPartition(i)->getKey();
        auto it = keyMap_.find(key);
        if (it == keyMap_.end()) {
            TableSP emptyTable = DBFileIO::createEmptyTableFromSchema(schema_);
            tables->set(i, emptyTable);
        }
        else {
            tables->set(i, partitions_[it->second]);
        }
    }
    return generateInMemoryParitionedTable(heap_, db_, tables, partitionColumnNames_);
}

#define TO_STRING(s) #s

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

    if (!getHdf5SimpleLayout(t, srcNativeTypeLayout))
        return nullptr;

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
        csp = nullptr; //unsupported;
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

    if (!getHdf5SimpleLayout(t, srcNativeTypeLayout))
        return false;

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
        cid = -1; //unsupported
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
        if (!createComplexColumns(srcType, cols, convertedType))
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
    if (!isClassEqual(srcType.id(), H5T_COMPOUND))
        return false;

    size_t memNum = H5Tget_nmembers(srcType.id());
    H5DataType memType;
    H5DataType memTypeConverted;
    convertedType.accept(H5Tcreate(H5T_COMPOUND, 2 * H5Tget_size(srcType.id())));
    size_t offset = 0;

    for (size_t i = 0; i != memNum; i++)
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

    std::vector<hsize_t> dims(dimsNum);
    H5Tget_array_dims(srcType.id(), dims.data());

    hsize_t eleNum = 1;
    for (size_t i = 0; i != dimsNum; i++)
        eleNum *= dims[i];

    for (size_t i = 0; i != eleNum; i++)
        if (!createHdf5TypeColumns(base, cols, baseConverted))
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

VectorSP TypeColumn::createDolphinDBColumnVector(VectorSP destVec, int size, int cap)
{
    DATA_TYPE destType = (destVec->isNull()) ? srcType() : destVec->getType();

    if (!compatible(destType))
        return nullSP;

    colVec_ = createCompatiableVector(destVec, destType, size, cap);
    destType_ = destType;
    destTypeSize_ = (destType == DT_STRING) ? sizeof(char *) : Util::getDataTypeSize(destType);
    return colVec_;
}

VectorSP TypeColumn::createCompatiableVector(VectorSP destVec, DATA_TYPE destType, int size, int cap)
{
    (void)destVec;
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
    default:
        r = false;
    }
    if (!r)
        throw RuntimeException("append data to vector failed");

    return len;
}

template <class src_t>
void numeric_pack_to_bool(pack_info_t t)
{
    static_assert(std::is_arithmetic<src_t>::value, "src type is not integer or float");
    static_assert(std::is_signed<src_t>::value, "src type is not signed");

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

        if (srcMax <= destMax) // calcuate in compile-time
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

VectorSP StringColumn::createCompatiableVector(VectorSP destVec, DATA_TYPE destType, int size, int cap)
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

VectorSP SymbolColumn::createCompatiableVector(VectorSP destVec, DATA_TYPE destType, int size, int cap)
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
        throw RuntimeException("error when convert hdf5 enum");

    baseSize_ = H5Tget_size(base.id());

    int memNum = H5Tget_nmembers(nativeEnumId);
    if (memNum <= 0)
        throw RuntimeException("invalid member num:" + std::to_string(memNum));

    enumMap_.clear();
    for (int i = 0; i != memNum; i++)
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

char *H5Object::getName(hid_t id)
{
    static char n[30];
    int ret = H5Iget_name(id, n, 29);
    return ret > 0 ? n : nullptr;
}

hid_t H5ReadOnlyFile::open(const std::string &filename)
{
    close();

    htri_t r = H5Fis_hdf5(filename.c_str());
    if (r == 0)
        throw IOException(filename + " is not an HDF5 file", INVALIDDATA);
    if (r == -1)
        throw IOException("check " + filename + " failed,it may not exist");

    if ((id_ = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT)) <= 0)
        throw IOException("can't open file" + filename);

    return id_;
}

void H5ReadOnlyFile::close()
{
    if (valid())
        H5Fclose(id_);

    id_ = H5I_INVALID_HID;
}

//H5DataSet imp

hid_t H5DataSet::open(const std::string &dset_name, hid_t loc_id)
{
    close();
    if ((id_ = H5Dopen(loc_id, dset_name.c_str(), H5P_DEFAULT)) <= 0)
        throw IOException("can't open dataset " + dset_name, INVALIDDATA);

    return id_;
}

void H5DataSet::close()
{
    if (valid())
        H5Dclose(id_);

    id_ = H5I_INVALID_HID;
}

//H5DatSpace imp

hid_t H5DataSpace::openFromDataset(hid_t dset_id)
{
    if ((id_ = H5Dget_space(dset_id)) <= 0)
        throw IOException("can't open dataspace", INVALIDDATA);

    return id_;
}

hid_t H5DataSpace::create(int rank, const hsize_t *current_dims, const hsize_t *maximum_dims)
{
    if ((id_ = H5Screate_simple(rank, current_dims, maximum_dims)) < 0)
        throw RuntimeException("create hdf5 dataspace failed");

    return id_;
}

void H5DataSpace::close()
{
    if (valid())
        H5Sclose(id_);

    id_ = H5I_INVALID_HID;
}

int H5DataSpace::rank() const
{
    return H5Sget_simple_extent_ndims(id_);
}

int H5DataSpace::currentDims(std::vector<hsize_t> &dims) const
{
    int rank = this->rank();
    dims.resize(rank);
    return H5Sget_simple_extent_dims(id_, dims.data(), nullptr);
}

//H5DataType imp

size_t H5DataType::size() const
{
    return H5Tget_size(id_);
}

hid_t H5DataType::openFromDataset(hid_t dset_id)
{
    if ((id_ = H5Dget_type(dset_id)) <= 0)
        throw IOException("can't open dataspace", INVALIDDATA);

    return id_;
}

void H5DataType::close()
{
    if (valid())
        H5Tclose(id_);
    id_ = H5I_INVALID_HID;
}

void H5Property::close()
{
    if (valid())
        H5Pclose(id_);

    id_ = H5I_INVALID_HID;
}

H5BlockDataReader::H5BlockDataReader(hid_t locId, size_t startElement, size_t endElement,
                                     int bufferByteLength, int vlenStrBufferByteLength, hid_t memTypeId)
    : H5GeneralDataReader(locId, bufferByteLength, vlenStrBufferByteLength, memTypeId),
      startElement_(startElement), endElement_(std::min(endElement, rowNum_ * colNum_))
{
    offsetRow_ = startElement_ / colNum_;
    offsetCol_ = startElement_ % colNum_;
    endRow_ = endElement_ / colNum_;
    endCol_ = endElement_ % colNum_;
}

H5GeneralDataReader::H5GeneralDataReader(hid_t locId, int bufferByteLength, int vlenStrBufferByteLength, hid_t memTypeId)
{
    open(locId, bufferByteLength, vlenStrBufferByteLength, memTypeId);
}

void H5GeneralDataReader::open(hid_t locId, int bufferByteLength, int vlenStrBufferByteLength, hid_t memTypeId)
{
    if (memTypeId < 0)
        throw RuntimeException("unknown memory data_type");

    H5DataSpace dspace(locId);
    int rank = dspace.rank();
    if (rank > 2)
        throw RuntimeException("rank of dataspace > 2");

    elementByteLength_ = H5Tget_size(memTypeId);
    memTypeId_ = memTypeId;
    assert(elementByteLength_ > 0);

    spaceClass_ = H5Sget_simple_extent_type(dspace.id());
    if (spaceClass_ == H5S_NO_CLASS)
        throw RuntimeException("unknown class of dataspace");

    std::vector<hsize_t> dims;

    if (spaceClass_ == H5S_SIMPLE)
    {
        dspace.currentDims(dims);
        assert(dims.size() == (size_t)rank);
        if (rank == 0) {
            dims.push_back(1);
            dims.push_back(1);
        }
        if (rank == 1)
            dims.push_back(1);
    }
    else if (spaceClass_ == H5S_SCALAR)
        dims.assign(2, 1);
    else if (spaceClass_ == H5S_NULL)
        dims.assign(2, 0);

    locId_ = locId;
    offsetRow_ = 0;
    offsetCol_ = 0;

    rowNum_ = dims[0];
    colNum_ = dims[1];

    size_t buf_len;
    //round buffer len
    buf_len = std::max(bufferByteLength, (int)elementByteLength_);
    buf_len = std::min(buf_len, rowNum_ * colNum_ * elementByteLength_);
    buf_len -= buf_len % elementByteLength_;

    buffer_.resize(buf_len, 0);
    buffer_.shrink_to_fit();

    hasVlenString_ = discoveVlenString(memTypeId);

    if (hasVlenString_)
        vlenMem_.pre_alloc_buffer.resize(vlenStrBufferByteLength);
    setXferProperty(hasVlenString_);
}

void H5BlockDataReader::open(hid_t locId, int bufferByteLength, int vlenStrBufferByteLength, hid_t memTypeId) {
    H5GeneralDataReader::open(locId, bufferByteLength, vlenStrBufferByteLength, memTypeId);

    size_t buf_len;
    buf_len = std::max(bufferByteLength, (int)elementByteLength_);
    buf_len = std::min(buf_len, endElement_ - startElement_);
    buf_len -= buf_len % elementByteLength_;

    buffer_.resize(buf_len, 0);
    buffer_.shrink_to_fit();
}

size_t H5GeneralDataReader::read()
{
    return this->readbyRow(offsetRow_, offsetCol_);
}

size_t H5BlockDataReader::read()
{
    return this->readbyRow(offsetRow_, offsetCol_);
}

void H5GeneralDataReader::swap_buffer(std::vector<char> &buf, vlen_mem &vm)
{
    buffer_.swap(buf);
    assert(vlenMem_.pre_alloc_buffer.capacity() == vm.pre_alloc_buffer.capacity());
    vlenMem_.pre_alloc_buffer.swap(vm.pre_alloc_buffer);
    vlenMem_.nature_alloc_ptr.swap(vm.nature_alloc_ptr);
}

size_t H5GeneralDataReader::testRead(size_t offsetRow, size_t offsetCol, size_t &row_offset_after, size_t &col_offset_after)
{
    row_offset_after = offsetRow;
    col_offset_after = offsetCol;

    if (reachRowEnd(offsetRow, offsetCol) || reachColumnEnd(offsetRow, offsetCol))
        return 0;

    H5DataSpace file_space;
    H5DataSpace mem_space;
    size_t elementNum;

    elementNum = prepareToRead(mem_space, file_space, offsetRow, offsetCol, true);

    row_offset_after = offsetRow + (offsetCol + elementNum) / colNum_;
    col_offset_after = (offsetCol + elementNum) % colNum_;

    return elementNum;
}

size_t H5GeneralDataReader::readbyRow(size_t offsetRow, size_t offsetCol)
{
    offsetRow_ = offsetRow;
    offsetCol_ = offsetCol;

    if (reachEnd())
        return 0;

    H5DataSpace file_space;
    H5DataSpace mem_space;
    size_t elementNum;

    if (hasVlenString_)
        freeVlenMemory(vlenMem_);

    elementNum = prepareToRead(mem_space, file_space, offsetRow_, offsetCol_, false);

    if (elementNum != 0)
        doRead(mem_space.id(), file_space.id());

    offsetRow_ += (offsetCol_ + elementNum) / colNum_;
    offsetCol_ = (offsetCol_ + elementNum) % colNum_;
    return elementNumReadLast_ = elementNum;
}

size_t H5GeneralDataReader::prepareToRead(H5DataSpace &mem_space, H5DataSpace &file_space,
                                              size_t offsetRow, size_t offsetCol, bool test) const
{
    if (spaceClass_ == H5S_NULL)
        return 0;
    if (spaceClass_ == H5S_SCALAR)
    {
        mem_space.accept(H5S_ALL);
        file_space.accept(H5S_ALL);
        return 1;
    }

    hsize_t offset[2];
    hsize_t count[2];

    file_space.openFromDataset(locId_);
    H5Sselect_none(file_space.id());

    size_t elementNum = 0;
    size_t elementNumReadMax = elementNumReadOnceMax();
    if (elementNumReadMax <= 0)
        throw RuntimeException("Block size too small");

    size_t iRow = offsetRow;
    size_t iCol = offsetCol;

    //select the first line
    offset[0] = iRow;
    offset[1] = iCol;
    count[0] = 1;
    count[1] = std::min(elementNumReadMax, colNum_ - iCol);
    elementNum += count[1];

    if (elementNumReadMax > elementNum)
    {
        iRow++;
        iCol = 0;
    }

    assert(iCol == 0 || (iCol != 0 && elementNum == elementNumReadMax));

    if (!test)
        H5Sselect_hyperslab(file_space.id(), H5S_SELECT_OR, offset, nullptr, count, nullptr);

    //select the middle lines
    if (elementNumReadMax - elementNum >= colNum_ && iRow < rowNum_)
    {
        size_t numOfEntireRowCanBeRead = (elementNumReadMax - elementNum) / colNum_;
        numOfEntireRowCanBeRead = std::min(numOfEntireRowCanBeRead, rowNum_ - iRow);

        offset[0] = iRow;
        offset[1] = iCol;
        count[0] = numOfEntireRowCanBeRead;
        count[1] = colNum_;
        elementNum += count[0] * count[1];

        iRow += numOfEntireRowCanBeRead;
        iCol = 0;
        if (!test)
            H5Sselect_hyperslab(file_space.id(), H5S_SELECT_OR, offset, nullptr, count, nullptr);
    }

    //select the last line
    if (elementNumReadMax > elementNum && iRow < rowNum_)
    {
        assert(elementNumReadMax - elementNum < colNum_);
        offset[0] = iRow;
        offset[1] = iCol;
        count[0] = 1;
        count[1] = elementNumReadMax - elementNum;
        elementNum += count[1];
        if (!test)
            H5Sselect_hyperslab(file_space.id(), H5S_SELECT_OR, offset, nullptr, count, nullptr);
    }

    hsize_t dims[2];
    dims[0] = 1;
    dims[1] = elementNum;

    mem_space.create(2, dims, nullptr);
    if (!test)
        H5Sselect_all(mem_space.id());

    return elementNum;
}

size_t H5GeneralDataReader::readbyCol()
{
    if (reachRowEnd() || reachColumnEnd())
        return 0;

    hsize_t offset[2];
    hsize_t count[2];

    H5DataSpace file_space(locId_);
    H5Sselect_none(file_space.id());

    size_t elementNum = 0;
    size_t elementNumReadMax = elementNumReadOnceMax();
    assert(elementNumReadMax > 0);

    size_t iRow = offsetRow_;
    size_t iCol = offsetCol_;

    //select the first line
    offset[0] = iRow;
    offset[1] = iCol;
    count[0] = std::min(elementNumReadMax, rowNum_ - iRow);
    count[1] = 1;
    elementNum += count[0];

    if (elementNumReadMax > elementNum)
    {
        iRow = 0;
        iCol++;
    }

    assert(iRow == 0 || (iRow != 0 && elementNum == elementNumReadMax));
    H5Sselect_hyperslab(file_space.id(), H5S_SELECT_OR, offset, nullptr, count, nullptr);

    //select the middle lines
    if (elementNumReadMax - elementNum >= rowNum_ && iCol < colNum_)
    {
        size_t numOfEntireColCanBeRead = (elementNumReadMax - elementNum) / rowNum_;
        numOfEntireColCanBeRead = std::min(numOfEntireColCanBeRead, colNum_ - iCol);

        offset[0] = iRow;
        offset[1] = iCol;
        count[0] = rowNum_;
        count[1] = numOfEntireColCanBeRead;
        elementNum += count[0] * count[1];

        iRow = 0;
        iCol += numOfEntireColCanBeRead;

        H5Sselect_hyperslab(file_space.id(), H5S_SELECT_OR, offset, nullptr, count, nullptr);
    }

    //select the last line
    if (elementNumReadMax > elementNum && iCol < colNum_)
    {
        assert(elementNumReadMax - elementNum < rowNum_);
        offset[0] = iRow;
        offset[1] = iCol;
        count[0] = elementNumReadMax - elementNum;
        count[1] = 1;
        elementNum += count[0];

        H5Sselect_hyperslab(file_space.id(), H5S_SELECT_OR, offset, nullptr, count, nullptr);
    }

    hsize_t dims[2];
    dims[0] = 1;
    dims[1] = elementNum;

    H5DataSpace mem_space(2, dims, dims);
    H5Sselect_all(mem_space.id());

    doRead(mem_space.id(), file_space.id());

    offsetRow_ = (offsetRow_ + elementNum) % rowNum_;
    offsetCol_ += (offsetRow_ + elementNum) / rowNum_;
    return elementNumReadLast_ = elementNum;
}

void H5GeneralDataReader::doRead(hid_t mem_space_id, hid_t file_space_id)
{
    herr_t state = H5Dread(locId_, memTypeId_, mem_space_id, file_space_id, xferProperty_.id(), buffer_.data());

    if (state < 0)
        throw RuntimeException("H5Dread return " + std::to_string(state));
}

size_t H5GeneralDataReader::switchColumn(size_t col_idx)
{
    assert(col_idx < colNum_);
    size_t old_idx = offsetCol_;
    offsetCol_ = col_idx;
    offsetRow_ = 0;
    return old_idx;
}

size_t H5GeneralDataReader::switchRow(size_t row_idx)
{
    assert(row_idx < rowNum_);
    size_t old_idx = offsetRow_;
    offsetRow_ = row_idx;
    offsetCol_ = 0;
    return old_idx;
}

bool H5GeneralDataReader::discoveVlenString(hid_t type) const
{

    H5T_class_t tc = H5Tget_class(type);

    if (tc == H5T_STRING)
        return H5Tis_variable_str(type) > 0;

    if (tc != H5T_COMPOUND && type != H5T_ARRAY)
        return false;

    std::vector<hdf5_type_layout> layout;
    getHdf5ComplexLayout(type, layout);

    for (size_t i = 0; i != layout.size(); i++)
        if (layout[i].flag == IS_VARIABLE_STR)
            return true;

    return false;
}

void H5GeneralDataReader::setXferProperty(bool has_vlen_str)
{
    xferProperty_.accept(H5Pcopy(H5P_DATASET_XFER_DEFAULT));

    if (!has_vlen_str)
        return;

    H5MM_allocate_t vlenAlloc = [](size_t size, void *alloc_info) -> void * {

        vlen_mem *vm = (vlen_mem *)alloc_info;
        std::vector<char> &pre_alloc_buf = vm->pre_alloc_buffer;
        std::list<char *> &malloc_ptr = vm->nature_alloc_ptr;

        char *ptr = nullptr;

        if (pre_alloc_buf.capacity() >= (pre_alloc_buf.size() + size))
        {
            ptr = pre_alloc_buf.data() + pre_alloc_buf.size();
            pre_alloc_buf.insert(pre_alloc_buf.end(), size, 0);
        }
        else
        {
            ptr = (char *)malloc(size * sizeof(char));
            malloc_ptr.push_back(ptr);
        }

        return ptr;
    };
    H5MM_free_t vlenFree = [](void *mem, void *free_info) -> void { return; };

    herr_t status = H5Pset_vlen_mem_manager(xferProperty_.id(), vlenAlloc, &vlenMem_, vlenFree, nullptr);
    if (status < 0)
        throw RuntimeException("can't set hdf5 dataset transfer property");
}

void freeVlenMemory(vlen_mem &vm)
{
    vm.pre_alloc_buffer.clear();
    while (!vm.nature_alloc_ptr.empty())
    {
        free(vm.nature_alloc_ptr.front());
        vm.nature_alloc_ptr.pop_front();
    }
}


inline bool acceptNativeType(H5DataType &type, hid_t src_type)
{
	return type.accept(H5Tget_native_type(src_type, H5T_DIR_ASCEND)) > 0;
}

int addTypeFlag(hid_t type_id, H5T_class_t type_class)
{
    switch (type_class)
    {
    case H5T_ENUM:
        return h5_type_flag::IS_ENUM;
    case H5T_COMPOUND:
        return h5_type_flag::IS_COMPOUND;
    case H5T_ARRAY:
        return h5_type_flag::IS_ARRAY;
    case H5T_STRING:
        return H5Tis_variable_str(type_id) > 0 ? IS_VARIABLE_STR : IS_FIXED_STR;
    case H5T_INTEGER:
    {
        hid_t t = type_id;
        int size = H5Tget_size(t);
        H5T_sign_t hs = H5Tget_sign(t);

        if (hs == H5T_SGN_ERROR)
            return -1;

        bool sign = hs == H5T_SGN_2;

        if (size == sizeof(char))
            return sign ? IS_S_CHAR_INTEGER : IS_U_CHAR_INTEGER;
        else if (size == sizeof(short))
            return sign ? IS_S_SHORT_INTEGER : IS_U_SHORT_INTEGER;
        else if (size == sizeof(int))
            return sign ? IS_S_INT_INTEGER : IS_U_INT_INTEGER;
        else if (size == sizeof(long long))
            return sign ? IS_S_LLONG_INTEGER : IS_U_LLONG_INTEGER;

        return -1;
    }
    case H5T_FLOAT:
    {
        hid_t t = type_id;

        int size = H5Tget_size(t);

        if (size == sizeof(float))
            return IS_FLOAT_FLOAT;
        else if (size == sizeof(double))
            return IS_DOUBLE_FLOAT;

        return -1;
    }
    case H5T_TIME:
        return IS_UNIX_TIME;
    default:
        return -1;
    }
}

void _getMemoryLayoutInArrayType(hid_t type, std::vector<hdf5_type_layout> &layout, int belong_to);

void _getMemoryLayoutInCompoundType(hid_t type, std::vector<hdf5_type_layout> &layout, int belong_to)
{
    int num_of_members = H5Tget_nmembers(type);
    int upper_offset = belong_to == -1 ? 0 : layout[belong_to].offset;
    for (int i = 0; i != num_of_members; i++)
    {
        H5T_class_t type_class = H5Tget_member_class(type, i);
        hdf5_type_layout cml;
        char *name_buf = H5Tget_member_name(type, i);
        cml.name = name_buf;
        H5free_memory(name_buf);
        cml.offset = H5Tget_member_offset(type, i) + upper_offset;
        cml.belong_to = belong_to;

        H5DataType d;
        d.accept(H5Tget_member_type(type, i));
        cml.flag = addTypeFlag(d.id(), type_class);
        cml.size = H5Tget_size(d.id());

        layout.push_back(std::move(cml));

        if (type_class == H5T_COMPOUND)
            _getMemoryLayoutInCompoundType(d.id(), layout, layout.size() - 1);
        else if (type_class == H5T_ARRAY)
            _getMemoryLayoutInArrayType(d.id(), layout, layout.size() - 1);
    }
}

bool getMemoryLayoutInCompoundType(hid_t type, std::vector<hdf5_type_layout> &layout)
{
    if (!isClassEqual(type, H5T_COMPOUND))
        return false;

    layout.clear();
    _getMemoryLayoutInCompoundType(type, layout, -1);
    return true;
}

bool getMemoryLayoutInArrayType(hid_t type, std::vector<hdf5_type_layout> &layout);

bool getHdf5ComplexLayout(hid_t t, std::vector<hdf5_type_layout> &layout)
{
    if (isClassEqual(t, H5T_COMPOUND))
        return getMemoryLayoutInCompoundType(t, layout);
    if (isClassEqual(t, H5T_ARRAY))
        return getMemoryLayoutInArrayType(t, layout);
    return false;
}

bool getHdf5SimpleLayout(hid_t t, hdf5_type_layout &layout)
{
    H5T_class_t tclass = H5Tget_class(t);
    layout.name = "";
    layout.belong_to = -1;
    layout.offset = 0;
    layout.size = H5Tget_size(t);
    layout.flag = addTypeFlag(t, tclass);

    return tclass != H5T_COMPOUND && tclass != H5T_ARRAY && layout.flag >= 0 && t > 0;
}

void _getMemoryLayoutInArrayType(hid_t type, std::vector<hdf5_type_layout> &layout, int belong_to)
{
    H5DataType base;
    size_t array_len;
    size_t base_type_size;
    int upper_offset;
    H5T_class_t base_class;

    array_len = getArrayElementNum(type);
    base.accept(H5Tget_super(type));
    upper_offset = belong_to == -1 ? 0 : layout[belong_to].offset;
    base_class = H5Tget_class(base.id());
    base_type_size = H5Tget_size(base.id());

    for (size_t i = 0; i != array_len; i++)
    {
        hdf5_type_layout aml;
        aml.name = "";
        aml.offset = base_type_size * i + upper_offset;
        aml.belong_to = belong_to;
        aml.flag = addTypeFlag(base.id(), base_class);
        aml.size = base_type_size;

        layout.push_back(std::move(aml));

        if (base_class == H5T_COMPOUND)
            _getMemoryLayoutInCompoundType(base.id(), layout, layout.size() - 1);
        else if (base_class == H5T_ARRAY)
            _getMemoryLayoutInArrayType(base.id(), layout, layout.size() - 1);
    }
}

bool getMemoryLayoutInArrayType(hid_t type, std::vector<hdf5_type_layout> &layout)
{
    if (!isClassEqual(type, H5T_ARRAY))
        return false;

    layout.clear();
    _getMemoryLayoutInArrayType(type, layout, -1);
    return true;
}

size_t getArrayElementNum(hid_t array)
{
    if (!isClassEqual(array, H5T_ARRAY))
        return -1;

    int rank = H5Tget_array_dims(array, nullptr);
    std::vector<hsize_t> dims(rank, 0);

    H5Tget_array_dims(array, dims.data());
    size_t n = 1, i = 0;
    while (i != dims.size())
        n *= dims[i++];
    return n;
}

uint32_t swapEndian32(uint32_t n)
{
    return ((n & 0x000000FFu) << 24) |
           ((n & 0x0000FF00u) << 8) |
           ((n & 0x00FF0000u) >> 8) |
           ((n & 0xFF000000u) >> 24);
}

uint64_t swapEndian64(uint64_t n)
{
    return ((n & 0x00000000000000ffULL) << 56) |
           ((n & 0x000000000000ff00ULL) << 40) |
           ((n & 0x0000000000ff0000ULL) << 24) |
           ((n & 0x00000000ff000000ULL) << 8) |
           ((n & 0x000000ff00000000ULL) >> 8) |
           ((n & 0x0000ff0000000000ULL) >> 24) |
           ((n & 0x00ff000000000000ULL) >> 40) |
           ((n & 0xff00000000000000ULL) >> 56);
}

void registerUnixTimeConvert()
{
    static bool done = false;
    if (done)
        return;

    H5T_conv_t v = [](hid_t src_id,
                      hid_t dst_id,
                      H5T_cdata_t *cdata,
                      size_t nelmts,
                      size_t buf_stride,
                      size_t bkg_stride,
                      void *buf,
                      void *bkg,
                      hid_t dset_xfer_plist) -> herr_t {
        if (cdata->command == H5T_CONV_INIT)
        {
            cdata->need_bkg = H5T_BKG_NO;
            if (H5Tequal(src_id, H5T_UNIX_D32BE) && H5Tequal(dst_id, H5T_UNIX_D32LE))
                return 0;
            if (H5Tequal(src_id, H5T_UNIX_D32LE) && H5Tequal(dst_id, H5T_UNIX_D32BE))
                return 0;
            return -1;
        }
        else if (cdata->command == H5T_CONV_CONV)
        {
            buf_stride = buf_stride ? buf_stride : H5Tget_size(dst_id);
            for (size_t i = 0; i != nelmts; i++)
            {
                char *b = (char *)buf + buf_stride * i;
                uint32_t *n = (uint32_t *)b;
                *n = swapEndian32(*n);
            }
        }

        return 0;
    };

    H5Tregister(H5T_PERS_SOFT, "a", H5T_UNIX_D32BE, H5T_UNIX_D32LE, v);
    H5Tregister(H5T_PERS_SOFT, "b", H5T_UNIX_D32LE, H5T_UNIX_D32BE, v);

    H5T_conv_t p = [](hid_t src_id,
                      hid_t dst_id,
                      H5T_cdata_t *cdata,
                      size_t nelmts,
                      size_t buf_stride,
                      size_t bkg_stride,
                      void *buf,
                      void *bkg,
                      hid_t dset_xfer_plist) -> herr_t {
        if (cdata->command == H5T_CONV_INIT)
        {
            cdata->need_bkg = H5T_BKG_NO;
            if (H5Tequal(src_id, H5T_UNIX_D64BE) && H5Tequal(dst_id, H5T_UNIX_D64LE))
                return 0;
            if (H5Tequal(src_id, H5T_UNIX_D64LE) && H5Tequal(dst_id, H5T_UNIX_D64BE))
                return 0;
            return -1;
        }
        else if (cdata->command == H5T_CONV_CONV)
        {
            buf_stride = buf_stride ? buf_stride : H5Tget_size(dst_id);
            for (size_t i = 0; i != nelmts; i++)
            {
                char *b = (char *)buf + buf_stride * i;
                uint64_t *n = (uint64_t *)b;
                *n = swapEndian64(*n);
            }
        }

        return 0;
    };

    H5Tregister(H5T_PERS_SOFT, "c", H5T_UNIX_D64LE, H5T_UNIX_D64BE, p);
    H5Tregister(H5T_PERS_SOFT, "d", H5T_UNIX_D64BE, H5T_UNIX_D64LE, p);

    H5T_conv_t kk = [](hid_t src_id,
                       hid_t dst_id,
                       H5T_cdata_t *cdata,
                       size_t nelmts,
                       size_t buf_stride,
                       size_t bkg_stride,
                       void *buf,
                       void *bkg,
                       hid_t dset_xfer_plist) -> herr_t {
        if (cdata->command == H5T_CONV_INIT)
        {
            cdata->need_bkg = H5T_BKG_NO;
            bool src_ok = H5Tequal(src_id, H5T_UNIX_D32LE) || H5Tequal(src_id, H5T_UNIX_D32BE);
            bool dst_ok = H5Tequal(dst_id, H5T_UNIX_D64LE) || H5Tequal(dst_id, H5T_UNIX_D64BE);

            return src_ok && dst_ok ? 0 : -1;
        }
        else if (cdata->command == H5T_CONV_CONV)
        {
            H5T_order_t st = H5Tget_order(src_id);
            H5T_order_t dt = H5Tget_order(dst_id);

            bool is_little_endian = Util::isLittleEndian();

            size_t src_stride = buf_stride ? buf_stride : H5Tget_size(src_id);
            size_t dest_stride = buf_stride ? buf_stride : H5Tget_size(dst_id);

            for (int i = int(nelmts - 1); i >= 0; i--)
            {
                char *b = (char *)buf + src_stride * i;
                uint32_t n = *(uint32_t *)b;
                if (is_little_endian && st == H5T_ORDER_BE)
                    n = swapEndian32(n);
                else if (!is_little_endian && st == H5T_ORDER_LE)
                    n = swapEndian32(n);

                uint64_t m = (uint64_t)n * 1000; //seconds to millseconds;

                if (is_little_endian && dt == H5T_ORDER_BE)
                    m = swapEndian64(m);
                else if (!is_little_endian && dt == H5T_ORDER_LE)
                    m = swapEndian64(m);

                b = (char *)buf + dest_stride * i;
                *(uint64_t *)b = m;
            }
        }

        return 0;
    };

    H5Tregister(H5T_PERS_HARD, "e", H5T_UNIX_D32LE, H5T_UNIX_D64LE, kk);
    H5Tregister(H5T_PERS_HARD, "f", H5T_UNIX_D32LE, H5T_UNIX_D64BE, kk);
    H5Tregister(H5T_PERS_HARD, "g", H5T_UNIX_D32BE, H5T_UNIX_D64LE, kk);
    H5Tregister(H5T_PERS_HARD, "h", H5T_UNIX_D32BE, H5T_UNIX_D64BE, kk);

    done = true;
}
}
