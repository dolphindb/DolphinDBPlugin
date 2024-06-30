#include <hdf5_plugin_imp.h>
#include <hdf5_plugin_pandas.h>
#include <hdf5_plugin.h>
#include <list>
#include <numeric>
#include "Exceptions.h"

/* H5_PLUGIN_IMP */

static InitHdf5Filter initHdf5Filter;

namespace H5PluginImp {

std::string getDatasetDimsStr(hid_t loc_id, const char *name)
{
    H5DataSet dSet(name, loc_id);
    H5DataSpace dSpace(dSet.id());

    H5S_class_t spaceClass = H5Sget_simple_extent_type(dSpace.id());
    if (spaceClass == H5S_NULL)
        return "NULL";
    if (spaceClass == H5S_SCALAR)
        return "0";
    if (spaceClass == H5S_NO_CLASS)
        return "";

    std::vector<hsize_t> dims;
    dSpace.currentDims(dims);

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
    H5DataSet dSet(name, loc_id);
    H5DataType dType;
    dType.openFromDataset(dSet.id());
    return getHdf5NativeTypeStr(dType);
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
        try {

            if (info->type != H5O_TYPE_DATASET)
                return 0;

            h5_dataset_table_info &dataset_info = *(static_cast<h5_dataset_table_info *>(op_data));

            dataset_info.nameVal.push_back(name[0] == '.' ? "/" : std::string("/") + name);
            dataset_info.dimsVal.push_back(getDatasetDimsStr(obj, name));
            dataset_info.typeVal.push_back(getDatasetNativeTypeStr(obj, name));
            return 0;
        } catch(...) {
            return -1;
        }
    };

    int ret = H5Ovisit(file.id(), H5_INDEX_CRT_ORDER, H5_ITER_NATIVE, callback, &datasetTableInfo, H5O_INFO_BASIC);
    checkFailAndThrowRuntimeException(ret < 0, "Failed to list the tables in file.");

    datasetName = std::move(datasetTableInfo.nameVal);
    datasetDims = std::move(datasetTableInfo.dimsVal);
    dataType = std::move(datasetTableInfo.typeVal);
}

void h5ls(const string &filename, vector<string> &objNames, vector<string> &objTypes) {

    struct h5_objects_names_and_types {
        std::vector<string> names;
        std::vector<string> types;
    } namesAndTypes;

    H5O_iterate_t callback = [](hid_t obj, const char *name, const H5O_info_t *info, void *op_data) -> herr_t {
        using std::vector;
        h5_objects_names_and_types &names_and_types = *(static_cast<h5_objects_names_and_types *>(op_data));

        names_and_types.names.push_back(name[0] == '.' ? "/" : std::string("/") + name);
        names_and_types.types.push_back(getHdf5ObjectTypeInfoStr(obj, name, info->type));

        return 0;
    };

    try {
        H5ReadOnlyFile file(filename);
        int ret = H5Ovisit(file.id(), H5_INDEX_CRT_ORDER, H5_ITER_NATIVE, callback, &namesAndTypes, H5O_INFO_BASIC);
        checkFailAndThrowRuntimeException(ret < 0, "Failed to list the file content.");
    } catch (H5::Exception &error) {
        throw RuntimeException(HDF5_LOG_PREFIX + "Error listing the file content: " + error.getDetailMsg());
    }

    objNames = std::move(namesAndTypes.names);
    objTypes = std::move(namesAndTypes.types);
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
    case IS_U_LLONG_INTEGER:
        return "LONG";
    case IS_FLOAT_FLOAT:
        return "FLOAT";
    case IS_DOUBLE_FLOAT:
        return "DOUBLE";
    case IS_UNIX_TIME:
        return "TIMESTAMP";
    case IS_BIT:
        return "BOOL";
    case IS_COMPOUND:
    case IS_ARRAY:
    default:
        throw RuntimeException(HDF5_LOG_PREFIX + "unsupported data type");
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

TableSP generateCompoundDatasetSchema(H5DataType &type) {
    std::vector<H5ColumnSP> h5Cols;
    H5DataType convertedType;
    checkFailAndThrowRuntimeException(!TypeColumn::createComplexColumns(type, h5Cols, convertedType), "unsupported data type");

    vector<string> names, types;
    generateComplexTypeBasedColsNameAndColsType(names, types, convertedType.id());
    const int colNum = names.size();

    vector<ConstantSP> cols(2);
    cols[0] = Util::createVector(DT_STRING, colNum, colNum);
    cols[1] = Util::createVector(DT_STRING, colNum, colNum);

    for (int i = 0; i < colNum; i++)
    {
        ConstantSP name = new String(names[i]);
        ConstantSP t = new String(types[i]);
        t->setString(types[i]);
        cols[0]->set(i, name);
        cols[1]->set(i, t);
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
    H5DataSpace dSpace(s.id());

    const int rank = dSpace.rank();
    int colNum;

    if (rank > 2)
        throw RuntimeException(HDF5_LOG_PREFIX + "rank of dataspace > 2");
    else if(rank < 0) {
        throw RuntimeException(HDF5_LOG_PREFIX + "Failed to get the rank of dataspace");
    }

    vector<hsize_t> dims;
    dSpace.currentDims(dims);

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
        throw RuntimeException(HDF5_LOG_PREFIX + "unsupported type");
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

bool createColumnVec(vector<H5ColumnSP> &cols, size_t num, size_t cap, H5DataType &src, const TableSP& dest)
{
    cols.resize(num);
    for (size_t i = 0; i < num; i++)
    {
        cols[i] = TypeColumn::createNewColumn(src);
        checkFailAndThrowRuntimeException(cols[i] == nullptr, "can't create new column");

        VectorSP destVec = dest->isNull() ? nullSP : dest->getColumn(i);
        VectorSP v = cols[i]->createDolphinDBColumnVector(destVec, 0, cap);
        checkFailAndThrowRuntimeException(v->isNull(), typeIncompatibleErrorMsg(i, cols[i]->srcType(), destVec));
    }
    return true;
}

bool createColumnVec(vector<H5ColumnSP> &cols, size_t cap, const TableSP& dest)
{
    for (size_t i = 0; i != cols.size(); i++)
    {
        checkFailAndThrowRuntimeException(cols[i] == nullptr, "unknown error:col can't be null"); //it should't be executed....just check

        VectorSP destVec = dest->isNull() ? nullSP : dest->getColumn(i);
        VectorSP v = cols[i]->createDolphinDBColumnVector(destVec, 0, cap);

        checkFailAndThrowRuntimeException(v->isNull(), typeIncompatibleErrorMsg(i, cols[i]->srcType(), destVec));
    }
    return true;
}


TableSP appendColumnVecToTable(const TableSP& tb, vector<ConstantSP> &colVec)
{
    if (tb->isNull())
        return tb;

    string errMsg;
    INDEX insertedRows = 0;
    checkFailAndThrowRuntimeException(!tb->append(colVec, insertedRows, errMsg), errMsg);
    return tb;
}

void doReadDataset_concurrent(H5GeneralDataReader &reader, const DatasetAppendRunnerSP& appendRunner,
                              vector<H5ColumnSP> &cols, vector<ConstantSP> &colVec)
{
    size_t eleNum = 0;
    size_t eleByteLength = reader.elementByteLength();

    appendRunner->setColumns(cols.data(), cols.size(), eleByteLength);

    vlen_mem backMem;
    backMem.pre_alloc_buffer.resize(reader.preAllocVlenBufferByteLength());
    std::vector<char> backBuffer(reader.bufferByteLength());

    Thread t(appendRunner);

    while ((eleNum = reader.read()) != 0)
    {
        t.join();

        freeVlenMemory(backMem);
        reader.swap_buffer(backBuffer, backMem);
        appendRunner->updateRawData(backBuffer.data(), eleNum);

        t.start();
    }
    t.join();
    freeVlenMemory(backMem);

    // handle null conditions
    colVec.resize(cols.size());
    for (size_t i = 0; i != colVec.size(); i++) {
        colVec[i] = cols[i]->colVec();
        if (colVec[i]->getType() == DT_FLOAT) {
            float buf[Util::BUF_SIZE];
            INDEX start = 0;
            INDEX len = Util::BUF_SIZE;
            int totalLength = colVec[i]->size();
            while(start < totalLength) {
                INDEX step = std::min(len, totalLength - start);
                float * floatBuf = colVec[i]->getFloatBuffer(start, step, buf);
                for(int j = 0; j < step; ++j) {
                    if ((*((uint32_t *)&(floatBuf[j]))) == 0x7fc00000) {
                        floatBuf[j] = FLT_NMIN;
                    }
                }
                colVec[i]->setFloat(start, step, floatBuf);
                start += step;
            }
        }
        else if (colVec[i]->getType() == DT_DOUBLE) {
            double buf[Util::BUF_SIZE];
            INDEX start = 0;
            INDEX len = Util::BUF_SIZE;
            int totalLength = colVec[i]->size();
            while(start < totalLength) {
                INDEX step = std::min(len, totalLength - start);
                double * doubleBuf = colVec[i]->getDoubleBuffer(start, step, buf);
                for(int j = 0; j < step; ++j) {
                    if ((*((uint64_t *)&(doubleBuf[j]))) == 0x7FF8000000000000) {
                        doubleBuf[j] = DBL_NMIN;
                    }
                }
                colVec[i]->setDouble(start, step, doubleBuf);
                start += step;
            }
        }
        colVec[i]->setNullFlag(true);
    }
}

void getRowAndColNum(const hid_t set, vector<size_t> &rowAndColNum) {
    H5DataSpace dSpace(set);
    std::vector<hsize_t> dims;
    dSpace.currentDims(dims);
    int rank = dSpace.rank();
    checkFailAndThrowRuntimeException(rank < 0, "Failed to get the rank of dataspace.");

    if (rank == 0) {
        dims.push_back(1);
        dims.push_back(1);
    }
    if (rank == 1)
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

        checkFailAndThrowRuntimeException(!TypeColumn::convertHdf5SimpleType(t, convertedType), "unsupported data type");

        H5GeneralDataReader reader(set, 1024 * 1024 * 32, 1024 * 1024 * 32, convertedType.id());
        return reader.totalSize();
    }
    case H5T_COMPOUND:
    case H5T_ARRAY:
    {
        std::vector<H5ColumnSP> cols;
        H5DataType convertedType;

        checkFailAndThrowRuntimeException(!TypeColumn::createComplexColumns(t, cols, convertedType), "unsupported data type");

        H5GeneralDataReader reader(set, 1024 * 1024 * 32, 1024 * 1024 * 32, convertedType.id());
        return reader.totalSize();
    }
    default:
        throw RuntimeException(HDF5_LOG_PREFIX + "unsupported type");
    }
}

void readCheck(size_t colNum, size_t rowNum, const TableSP& tb, size_t colSize) {
    checkFailAndThrowRuntimeException(!colNum, !rowNum, "empty dataset!");
    checkFailAndThrowRuntimeException(!tb->isNull() && !colsNumEqual(tb, colSize), "the columns of the table don't match the dataset");
}

TableSP readSimpleDataset(const hid_t set, H5DataType &type, const TableSP& tb, size_t startRow, size_t readRowNum, GroupInfo &groupInfo)
{
    H5DataType convertedType;

    checkFailAndThrowRuntimeException(!TypeColumn::convertHdf5SimpleType(type, convertedType), "unsupported data type");
    vector<size_t> rowAndColNum;
    getRowAndColNum(set, rowAndColNum);
    size_t rowNum = rowAndColNum[0];
    size_t colNum = rowAndColNum[1];
    size_t startElement = startRow * colNum;
    size_t endElement;
    if (readRowNum == 0 || readRowNum > rowNum) {
        endElement = rowNum * colNum;
    } else {
        endElement = (startRow + readRowNum) * colNum;
    }

    H5BlockDataReader reader(set, startElement, endElement, 1024 * 1024 * 32, 1024 * 1024 * 32, convertedType.id());

    readCheck(reader.columnNum(), reader.rowNum(), tb, reader.columnNum());

    vector<H5ColumnSP> cols;
    createColumnVec(cols, reader.columnNum(), std::min(readRowNum,rowNum), type, tb);
    vector<ConstantSP> colVec;
    DatasetAppendRunnerSP appendRunner = new SimpleDatasetAppendRunner();
    doReadDataset_concurrent(reader, appendRunner, cols, colVec);

    if (tb->isNull())
    {
        vector<string> realColsName;
        if(!strcmp(groupInfo.dataType.c_str(), "normal")){
            generateIncrementedColsName(realColsName, colVec.size());
            return Util::createTable(realColsName, colVec);
        }
        else{
            realColsName = *(groupInfo.colsName);
            vector<ConstantSP> realColVec;
            //match the col name with the col data. jump index col.
            realColVec.push_back(colVec[0]);
            for(size_t i = 1; i < realColsName.size(); i++){
                size_t index = find(groupInfo.kindColsName->begin(), groupInfo.kindColsName->end(), realColsName[i]) - groupInfo.kindColsName->begin();
                realColVec.push_back(colVec[index + 1]);
            }
        }
        return Util::createTable(realColsName, colVec);
    }

    return appendColumnVecToTable(tb, colVec);
}
TableSP readComplexDataset(const hid_t set, H5DataType &type, const TableSP& tb, size_t startRow, size_t readRowNum, GroupInfo &groupInfo)
{
    std::vector<H5ColumnSP> cols;
    H5DataType convertedType;

    checkFailAndThrowRuntimeException(!TypeColumn::createComplexColumns(type, cols, convertedType), "unsupported data type");

    vector<size_t> rowAndColNum;
    getRowAndColNum(set, rowAndColNum);
    size_t rowNum = rowAndColNum[0];
    size_t colNum = rowAndColNum[1];
    size_t startElement = startRow * colNum;
    size_t endElement;
    if (readRowNum == 0/* || readRowNum > rowNum*/)
        endElement = rowNum * colNum;
    else
        endElement = (startRow + readRowNum) * colNum;

    H5BlockDataReader reader(set, startElement, endElement, 1024 * 1024 * 32, 1024 * 1024 * 32, convertedType.id());

    readCheck(reader.columnNum(), reader.rowNum(), tb, cols.size());

    createColumnVec(cols, std::min(endElement,colNum * rowNum)-startElement, tb);

    vector<ConstantSP> colVec;
    DatasetAppendRunnerSP appendRunner = new ComplexDatasetAppendRunner();
    doReadDataset_concurrent(reader, appendRunner, cols, colVec);

    if (tb->isNull())
    {
        vector<string> realColsName;
        if(!strcmp(groupInfo.dataType.c_str(),"normal")){
            generateComplexTypeBasedColsName(realColsName, convertedType.id());
            return Util::createTable(realColsName, colVec);
        }else{
            realColsName = *(groupInfo.colsName);
            vector<ConstantSP> realColVec;
            // match the col name with the col data. jump index col.

            vector<string> res = *(groupInfo.kindColsName);
            realColVec.push_back(colVec[0]);
            for(size_t i = 1; i < realColsName.size(); i++){
                size_t index = find(groupInfo.kindColsName->begin(), groupInfo.kindColsName->end(), realColsName[i]) - groupInfo.kindColsName->begin();
                realColVec.push_back(colVec[index + 1]);
            }
            return Util::createTable(realColsName, realColVec);
        }
    }

    return appendColumnVecToTable(tb, colVec);
}
ConstantSP loadHDF5(const hid_t set, const ConstantSP &schema, const size_t startRow, const size_t rowNum) {
    H5DataType t;
    t.openFromDataset(set);

    registerUnixTimeConvert();

    TableSP tableWithSchema = schema->isNull() ?
        static_cast<TableSP>(nullSP) : DBFileIO::createEmptyTableFromSchema(schema);

    string dType = "normal";
    GroupInfo info(dType, nullptr, 0);
    switch (H5Tget_class(t.id()))
    {
    case H5T_INTEGER:
    case H5T_FLOAT:
    case H5T_TIME:
    case H5T_STRING:
    case H5T_ENUM:
    case H5T_BITFIELD:
        return readSimpleDataset(set, t, tableWithSchema, startRow, rowNum, info);
    case H5T_COMPOUND:
    case H5T_ARRAY:
        return readComplexDataset(set, t, tableWithSchema, startRow, rowNum, info);
    default:
        throw RuntimeException(HDF5_LOG_PREFIX + "unsupported type");
    }
}

ConstantSP loadHDF5(const string &filename, const string &datasetName,
                    const ConstantSP &schema, const size_t startRow, const size_t rowNum)
{
    H5ReadOnlyFile f(filename);
    H5DataSet set(datasetName, f.id());
    return loadHDF5(set.id(), schema, startRow, rowNum);
}

void getGroupAttribute(const H5::Group& group, const string& attribute, string& value){
    checkFailAndThrowRuntimeException(!group.attrExists(attribute), "attribute [" + attribute + "] doesn't exist in group [" +
                               group.getObjName() + "]");
    H5::Attribute attr;
    HDF5_SAFE_EXECUTE(attr = group.openAttribute(attribute));
    H5::DataType type = attr.getDataType();
    HDF5_SAFE_EXECUTE(attr.read(type, value));
}

void getDataSetAttribute(const H5::DataSet& dataset, const string& attribute, string& value){
    checkFailAndThrowRuntimeException(!dataset.attrExists(attribute), "attribute [" + attribute + "] doesn't exist in dataset [" +
                               dataset.getObjName() + "]");

    H5::Attribute attr;
    HDF5_SAFE_EXECUTE(attr = dataset.openAttribute(attribute));
    H5::DataType type = attr.getDataType();
    HDF5_SAFE_EXECUTE(attr.read(type, value));
}

template <typename T>
void getGroupAttribute(const H5::Group& group, const string& attribute, T* value){
    checkFailAndThrowRuntimeException(!group.attrExists(attribute), "attribute [" + attribute + "] doesn't exist in group [" +
                               group.getObjName() + "]");
    H5::Attribute attr;
    HDF5_SAFE_EXECUTE(attr = group.openAttribute(attribute));
    H5::DataType type = attr.getDataType();
    HDF5_SAFE_EXECUTE(attr.read(type, value));
}

ConstantSP loadFromH5ToDatabase(Heap *heap, vector<ConstantSP> &arguments)
{
    //hid_t setId = static_cast<hid_t>(arguments[0]->getLong());
    TableSP schema = static_cast<TableSP>(arguments[2]);
    size_t startRow = arguments[3]->getLong();
    size_t rowNum = arguments[4]->getLong();
    SystemHandleSP db = static_cast<SystemHandleSP>(arguments[5]);
    string tableName = arguments[6]->getString();
    FunctionDefSP transform = (FunctionDefSP)arguments[8];
    bool diskSeqMode = !db->getDatabaseDir().empty() &&
                       db->getDomain()->getPartitionType() == SEQ;
    ConstantSP tempTable = loadHDF5(arguments[0]->getString(), arguments[1]->getString(), schema, startRow, rowNum);
    TableSP loadedTable;
    if(!transform->isNull()){
        vector<ConstantSP> args = {tempTable};
        loadedTable = transform->call(heap, args);
    }else{
        loadedTable = tempTable;
    }

    if (diskSeqMode) {
        string id = db->getDomain()->getPartition(arguments[7]->getInt())->getPath();
        string directory = db->getDatabaseDir() + "/" + id;
        checkFailAndThrowRuntimeException(!DBFileIO::saveBasicTable(heap->currentSession(), directory, loadedTable.get(), tableName,
                          NULL, true, 1, false), "Failed to save the table to directory " + directory);
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
        // if (types.back() <= DT_VOID || types.back() > DT_STRING)
        //     throw RuntimeException(HDF5_LOG_PREFIX + "Invalid data type '" + vecType->getString(i) + "'");
    }
}

vector<DistributedCallSP> generateH5Tasks(Heap* heap, const string &fileName, const string &datasetName,
    const TableSP &schema, const size_t startRow, const size_t rowNum,
    const SystemHandleSP &db, const string &tableName, const FunctionDefSP &transform)
{
    vector<string> colNames;
    vector<DATA_TYPE> types;

    H5ReadOnlyFile f(fileName);
    H5DataSet dataset(datasetName, f.id());
    if (!schema->isNull())
		getColNamesAndTypesFromSchema(schema, colNames, types);

    vector<size_t> rowAndColNum;
    getRowAndColNum(dataset.id(), rowAndColNum);

    size_t estimatedRows = rowAndColNum[0] - startRow;
    if (rowNum != 0)
        estimatedRows = std::min(rowNum, estimatedRows);

    int partitions = 0;
    DomainSP domain = db->getDomain();

    if (domain->getPartitionType() == SEQ) {
        partitions = domain->getPartitionCount();
        checkFailAndThrowIOException(partitions <= 1, "The database must have at least two partitions.");
        checkFailAndThrowIOException((estimatedRows / partitions) < 65536, "The number of rows per partition is too small (<65,536) and the hdf5 file can't be partitioned.");
    }
    if (partitions == 0) {
        double fileSize = (double)getDatasetSize(dataset.id());
        long long maxSizePerPartition = 128 * 1024 * 1024;
        fileSize = (((double)estimatedRows / rowAndColNum[0])) * fileSize;
        int defaultPartitions = 4;//GOContainer::LOCAL_EXECUTOR_SIZE + 1;
        long long sizePerPartition = fileSize / defaultPartitions;
        if (sizePerPartition < 16 * 1024 * 1024)
            partitions = (fileSize / (16 * 1024 * 1024)) + 1;
        else if (sizePerPartition > maxSizePerPartition)
            partitions = (fileSize / maxSizePerPartition) + 1;
        else
            partitions = defaultPartitions;
    }
    dataset.close();
    f.close();

    int columns=colNames.size();
	vector<ConstantSP> cols(columns);

    vector<DistributedCallSP> tasks;

    size_t rowIncrement = std::ceil(static_cast<double>(estimatedRows) / partitions);
    size_t currentStartRow = startRow;
    ConstantSP _rowIncrement = new Long(rowIncrement);
    ConstantSP file = new String(fileName);
    ConstantSP set = new String(datasetName);
    ConstantSP _tableName = new String(tableName);
    FunctionDefSP func = Util::createSystemFunction("loadFromH5ToDatabase", loadFromH5ToDatabase, 9, 9, false);
    for (int i = 0; i < partitions; i++) {
        ConstantSP _startRow = new Long(currentStartRow);
        ConstantSP id = new Int(i);
        vector<ConstantSP> args{file, set, schema,
                                _startRow, _rowIncrement, db, _tableName, id, transform};
        ObjectSP call = Util::createRegularFunctionCall(func, args);
        DistributedCallSP task = new DistributedCall(call, true);
        tasks.push_back(task);
        currentStartRow += rowIncrement;
    }
    return tasks;
}

TableSP generateInMemoryPartitionedTable(Heap *heap, const SystemHandleSP &db,
                                        const ConstantSP &tables, const ConstantSP &partitionNames)
{
    FunctionDefSP createPartitionedTable = heap->currentSession()->getFunctionDef("createPartitionedTable");
    ConstantSP emptyString = new String("");
    vector<ConstantSP> args{db, tables, emptyString, partitionNames};
    return createPartitionedTable->call(heap, args);
}

ConstantSP savePartition(Heap *heap, vector<ConstantSP> &arguments){
    SystemHandleSP db = arguments[0];
    ConstantSP tb = arguments[1];
    ConstantSP tbInMemory = arguments[2];
    string dbPath = db->getDatabaseDir();
    FunctionDefSP append = heap->currentSession()->getFunctionDef("append!");
    vector<ConstantSP> appendArgs = {tb, tbInMemory};
    append->call(heap, appendArgs);
    return new Void();
}

ConstantSP loadHDF5Ex(Heap *heap, const SystemHandleSP &db, const string &tableName, const ConstantSP &partitionColumns,
                      const string &filename, const string &datasetName, const TableSP &schema,
                      const size_t startRow, const size_t rowNum, const FunctionDefSP &transform)
{
    const TableSP convertedSchema = schema->isNull() ? extractHDF5Schema(filename, datasetName) : schema;
    vector<DistributedCallSP> tasks = generateH5Tasks(heap, filename, datasetName, convertedSchema, startRow, rowNum, db, tableName, transform);
    int partitions = tasks.size();


    string owner = heap->currentSession()->getUser()->getUserId();
    DomainSP domain = db->getDomain();
    bool seqDomain = domain->getPartitionType() == SEQ;
    bool inMemory = db->getDatabaseDir().empty();
    ConstantSP tableName_ = new String(tableName);

    if (seqDomain) {
        StaticStageExecutor executor(false, false, false);
        executor.execute(heap, tasks);
        for (int i = 0; i < partitions; i++) {
            const string &errMsg = tasks[i]->getErrorMessage();
            checkFailAndThrowRuntimeException(!errMsg.empty(), errMsg);
        }
        if (inMemory) {
            ConstantSP tmpTables = Util::createVector(DT_ANY, partitions);
            for (int i = 0; i < partitions; i++)
                tmpTables->set(i, tasks[i]->getResultObject());
            ConstantSP partitionNames = new String("");
            return generateInMemoryPartitionedTable(heap, db, tmpTables, partitionNames);
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

            string physicalIndex = tableName;
            checkFailAndThrowIOException(!DBFileIO::saveTableHeader(owner, physicalIndex, cols, partitionColumnIndices, 0, tableFile, NULL), "Failed to save table header " + tableFile);
            vector<FunctionDefSP> partitionFuncs{};
            checkFailAndThrowIOException(!DBFileIO::saveDatabase(db.get()), "Failed to save database " + db->getDatabaseDir());
            db->getDomain()->addTable(tableName, owner, physicalIndex, cols, partitionColumnIndices, partitionFuncs);
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
            //TableSP schema = extractHDF5Schema(filename, datasetName);
            ConstantSP dummyTable = DBFileIO::createEmptyTableFromSchema(convertedSchema);
            vector<ConstantSP> createTableArgs = {db, dummyTable, tableName_, partitionColumns};
            result = heap->currentSession()->getFunctionDef("createPartitionedTable")->call(heap, createTableArgs);
        }
        vector<FunctionDefSP> functors;
        FunctionDefSP func = Util::createSystemFunction("savePartition",&savePartition, 3, 3, false);
        vector<ConstantSP> args(3);
        args[0] = db;
        args[1] = result;
        args[2] = new Void();
        functors.push_back(Util::createPartialFunction(func, args));
        //int parallel = 10;

        PipelineStageExecutor executor(functors, false, 2, 1);

        executor.execute(heap, tasks);
        for(int i=0; i<partitions; ++i){
            if(!tasks[i]->getErrorMessage().empty()){
                string errMsg;
                errMsg = tasks[i]->getErrorMessage();
                throw RuntimeException(HDF5_LOG_PREFIX + errMsg);
            }
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
    checkFailAndThrowRuntimeException((memTypeId < 0), "unknown memory data_type");

    H5DataSpace dspace(locId);
    int rank = dspace.rank();
    checkFailAndThrowRuntimeException(rank > 2, "rank of dataspace > 2");
    checkFailAndThrowRuntimeException(rank < 0, "Failed to get the rank of dataspace");

    elementByteLength_ = H5Tget_size(memTypeId);
    memTypeId_ = memTypeId;
    assert(elementByteLength_ > 0);

    spaceClass_ = H5Sget_simple_extent_type(dspace.id());
    checkFailAndThrowRuntimeException(spaceClass_ == H5S_NO_CLASS, "unknown class of dataspace");

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

    hasVlenString_ = discoverVlenString(memTypeId);

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
    return this->readByRow(offsetRow_, offsetCol_);
}

size_t H5BlockDataReader::read()
{
    return this->readByRow(offsetRow_, offsetCol_);
}

void H5GeneralDataReader::swap_buffer(std::vector<char> &buf, vlen_mem &vm)
{
    buffer_.swap(buf);
    assert(vlenMem_.pre_alloc_buffer.capacity() == vm.pre_alloc_buffer.capacity());
    vlenMem_.pre_alloc_buffer.swap(vm.pre_alloc_buffer);
    vlenMem_.nature_alloc_ptr.swap(vm.nature_alloc_ptr);
}



size_t H5GeneralDataReader::readByRow(size_t offsetRow, size_t offsetCol)
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

    elementNum = prepareToRead(mem_space, file_space, offsetRow_, offsetCol_);

    if (elementNum != 0)
        doRead(mem_space.id(), file_space.id());

    offsetRow_ += (offsetCol_ + elementNum) / colNum_;
    offsetCol_ = (offsetCol_ + elementNum) % colNum_;
    return elementNumReadLast_ = elementNum;
}

size_t H5GeneralDataReader::prepareToRead(H5DataSpace &mem_space, H5DataSpace &file_space,
                                              size_t offsetRow, size_t offsetCol) const
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
    checkFailAndThrowRuntimeException(elementNumReadMax <= 0, "Block size too small");

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
        H5Sselect_hyperslab(file_space.id(), H5S_SELECT_OR, offset, nullptr, count, nullptr);
    }

    hsize_t dims[2];
    dims[0] = 1;
    dims[1] = elementNum;

    mem_space.create(2, dims, nullptr);
    H5Sselect_all(mem_space.id());

    return elementNum;
}

void H5GeneralDataReader::doRead(hid_t mem_space_id, hid_t file_space_id)
{
    herr_t state = H5Dread(locId_, memTypeId_, H5S_BLOCK, file_space_id, xferProperty_.id(), buffer_.data());

    checkFailAndThrowRuntimeException(state < 0, "H5Dread return " + std::to_string(state));
}


bool H5GeneralDataReader::discoverVlenString(hid_t type) const
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
    checkFailAndThrowRuntimeException(status < 0, "can't set hdf5 dataset transfer property");
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
    case H5T_BITFIELD:
        return IS_BIT;
    default:
        return -1;
    }
}

void _getMemoryLayoutInCompoundType(hid_t type, std::vector<hdf5_type_layout> &layout, int belong_to)
{
    int num_of_members = H5Tget_nmembers(type);
    int upper_offset = belong_to == -1 ? 0 : layout[belong_to].offset;
    for (int i = 0; i < num_of_members; i++)
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

ConstantSP saveHDF5(const TableSP &table, const string &fileName, const string &datasetName, bool append, unsigned stringMaxLength){
    if(append){
        appendHDF5(table, fileName, datasetName, stringMaxLength);
    }
    else{
        writeHDF5(table, fileName, datasetName, stringMaxLength);
    }
    return new Void();
}

void appendHDF5(const TableSP &table, const string &fileName, const string &datasetName, unsigned stringMaxLength){
    H5::H5File file;
    try {
        file.openFile(fileName.c_str(), H5F_ACC_RDWR, H5P_DEFAULT);
    } catch (const H5::FileIException &e) {
        throw RuntimeException(HDF5_LOG_PREFIX + "Error opening file: " + e.getDetailMsg());
    }

    hsize_t nRecords = table->getColumn(0)->size();
    size_t type_size;
    size_t *field_offset = new size_t[table->columns()];
    size_t *dst_sizes = new size_t[table->columns()];
    extractDolphinDBSchema(table, type_size, field_offset, dst_sizes, stringMaxLength);
    char *buf = new char[nRecords * type_size];
    extractDolphinDBData(table, type_size, field_offset, buf, stringMaxLength);

    H5TBappend_records(file.getId(), datasetName.c_str(), nRecords, type_size, field_offset, dst_sizes, buf);

    delete[] field_offset;
    delete[] dst_sizes;
    delete[] buf;

    file.close();
}

void writeHDF5(const TableSP &table, const string &fileName, const string &datasetName, unsigned stringMaxLength){
    H5::H5File file;
    if(H5Fis_hdf5(fileName.c_str()) > 0){
        file = H5::H5File(fileName.c_str(), H5F_ACC_RDWR, H5P_DEFAULT, H5P_DEFAULT);
        hsize_t nFields;
        hsize_t nRecords;
        if(H5TBget_table_info(file.getId(), datasetName.c_str(), &nFields, &nRecords) >= 0){
            H5TBdelete_record(file.getId(), datasetName.c_str(), 0, nRecords);
            file.unlink(datasetName.c_str(), H5P_DEFAULT);
        }
    }
    else{
        file = H5::H5File(fileName.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    }

    hsize_t nFields = table->columns();
    hsize_t nRecords = table->getColumn(0)->size();
    size_t type_size;
    char **field_names = new char*[nFields];
    size_t *field_offset = new size_t[nFields];
    hid_t *field_types = new hid_t[nFields];
    extractDolphinDBSchema(table, type_size, field_names, field_offset, field_types, stringMaxLength);
    char *buf = new char[nRecords * type_size];
    extractDolphinDBData(table, type_size, field_offset, buf, stringMaxLength);

    H5TBmake_table(datasetName.c_str(), file.getId(), datasetName.c_str(), nFields, nRecords, type_size, const_cast<const char**>(field_names), field_offset, field_types, 10, nullptr, 0, buf);

    for(hsize_t i = 0; i < nFields; ++i){
        delete[] field_names[i];
    }
    delete[] field_names;
    delete[] field_offset;
    delete[] field_types;
    delete[] buf;

    file.close();
}

void extractDolphinDBSchema(const TableSP &table, size_t &type_size, char *field_names[], size_t *field_offset, hid_t *field_types, unsigned stringMaxLength){
    type_size = 0;
    auto extractType = [&](int index, size_t size, hid_t type){
        field_offset[index] = type_size;
        field_types[index] = type;
        type_size += size;
    };
    for(int i = 0; i < table->columns(); ++i){
        switch(table->getColumnType(i)){
        case DT_BOOL:
            extractType(i, sizeof(bool), H5T_NATIVE_HBOOL);
            break;
        case DT_CHAR:
            extractType(i, sizeof(char), H5T_NATIVE_CHAR);
            break;
        case DT_SHORT:
            extractType(i, sizeof(short), H5T_NATIVE_SHORT);
            break;
        case DT_INT:
        case DT_DATE:
        case DT_MONTH:
        case DT_TIME:
        case DT_MINUTE:
        case DT_SECOND:
        case DT_DATETIME:
            extractType(i, sizeof(int), H5T_NATIVE_INT);
            break;
        case DT_LONG:
        case DT_TIMESTAMP:
        case DT_NANOTIME:
        case DT_NANOTIMESTAMP:
            extractType(i, sizeof(long long), H5T_NATIVE_LLONG);
            break;
        case DT_FLOAT:
            extractType(i, sizeof(float), H5T_NATIVE_FLOAT);
            break;
        case DT_DOUBLE:
            extractType(i, sizeof(double), H5T_NATIVE_DOUBLE);
            break;
        case DT_STRING:
        case DT_SYMBOL:
        {
            // H5::DataType string_type(H5T_STRING, stringMaxLength);
            hid_t string_type = H5Tcopy(H5T_C_S1);
            H5Tset_size(string_type, stringMaxLength);
            extractType(i, stringMaxLength, string_type);
            break;
        }
        default:
            throw RuntimeException(HDF5_LOG_PREFIX + "unsupported type.");
        }
        string name = table->getColumnName(i);
        checkFailAndThrowRuntimeException(name.length() + 1 > stringMaxLength, "string length out of stringMaxLength.");
        field_names[i] = new char[stringMaxLength];
        strcpy(field_names[i], name.c_str());
    }
}

void extractDolphinDBSchema(const TableSP &table, size_t &type_size, size_t *field_offset, size_t *field_sizes, unsigned stringMaxLength){
    type_size = 0;
    auto extractType = [&](int index, size_t size){
        field_offset[index] = type_size;
        field_sizes[index] = size;
        type_size += size;
        return;
    };
    for(int i = 0; i < table->columns(); ++i){
        switch(table->getColumnType(i)){
        case DT_BOOL:
            extractType(i, sizeof(bool));
            break;
        case DT_CHAR:
            extractType(i, sizeof(char));
            break;
        case DT_SHORT:
            extractType(i, sizeof(short));
            break;
        case DT_INT:
        case DT_DATE:
        case DT_MONTH:
        case DT_TIME:
        case DT_MINUTE:
        case DT_SECOND:
        case DT_DATETIME:
            extractType(i, sizeof(int));
            break;
        case DT_LONG:
        case DT_TIMESTAMP:
        case DT_NANOTIME:
        case DT_NANOTIMESTAMP:
            extractType(i, sizeof(long long));
            break;
        case DT_FLOAT:
            extractType(i, sizeof(float));
            break;
        case DT_DOUBLE:
            extractType(i, sizeof(double));
            break;
        case DT_STRING:
        case DT_SYMBOL:
            extractType(i, stringMaxLength);
            break;
        default:
            throw RuntimeException(HDF5_LOG_PREFIX + "unsupported type.");
        }
    }
}

void extractDolphinDBData(const TableSP &table, const size_t &type_size, const size_t *field_offset, char *buf, unsigned stringMaxLength){
    for(int i = 0; i < table->columns(); ++i){
        VectorSP dolphindbCol = table->getColumn(i);
        if(dolphindbCol->size() == 0){
            return;
        }
        switch(dolphindbCol->get(0)->getType()){
        case DT_BOOL:
            for(int j = 0; j < dolphindbCol->size(); ++j){
                bool *ptr = (bool*)(buf + type_size * j + field_offset[i]);
                ConstantSP value = dolphindbCol->get(j);
                *ptr = value->isNull() ? false : value->getBool();
            }
            break;
        case DT_CHAR:
            for(int j = 0; j < dolphindbCol->size(); ++j){
                char *ptr = (char*)(buf + type_size * j + field_offset[i]);
                ConstantSP value = dolphindbCol->get(j);
                *ptr = value->isNull() ? '\0' : value->getChar();
            }
            break;
        case DT_SHORT:
            for(int j = 0; j < dolphindbCol->size(); ++j){
                short *ptr = (short*)(buf + type_size * j + field_offset[i]);
                ConstantSP value = dolphindbCol->get(j);
                *ptr = value->isNull() ? 0 : value->getShort();
            }
            break;
        case DT_INT:
        case DT_DATE:
        case DT_MONTH:
        case DT_TIME:
        case DT_MINUTE:
        case DT_SECOND:
        case DT_DATETIME:
            for(int j = 0; j < dolphindbCol->size(); ++j){
                int *ptr = (int*)(buf + type_size * j + field_offset[i]);
                ConstantSP value = dolphindbCol->get(j);
                *ptr = value->isNull() ? 0 : value->getInt();
            }
            break;
        case DT_LONG:
        case DT_TIMESTAMP:
        case DT_NANOTIME:
        case DT_NANOTIMESTAMP:
            for(int j = 0; j < dolphindbCol->size(); ++j){
                long long *ptr = (long long*)(buf + type_size * j + field_offset[i]);
                ConstantSP value = dolphindbCol->get(j);
                *ptr = value->isNull() ? 0 : value->getLong();
            }
            break;
        case DT_FLOAT:
            for(int j = 0; j < dolphindbCol->size(); ++j){
                float *ptr = (float*)(buf + type_size * j + field_offset[i]);
                ConstantSP value = dolphindbCol->get(j);
                uint32_t nan = 0x7fc00000;
                *ptr = value->isNull() ? (*((float *)&nan)) : value->getFloat();
            }
            break;
        case DT_DOUBLE:
            for(int j = 0; j < dolphindbCol->size(); ++j){
                double *ptr = (double*)(buf + type_size * j + field_offset[i]);
                ConstantSP value = dolphindbCol->get(j);
                uint64_t nan = 0xffffffff0000ffff;
                *ptr = value->isNull() ? (*((double *)&nan)) : value->getDouble();
            }
            break;
        case DT_STRING:
        case DT_SYMBOL:
            for(int j = 0; j < dolphindbCol->size(); ++j){
                char *ptr = buf + type_size * j + field_offset[i];
                ConstantSP value = dolphindbCol->get(j);
                string str = value->isNull() ? "" : value->getString();
                checkFailAndThrowRuntimeException(str.length() + 1 > stringMaxLength, "string length out of stringMaxLength.");

                strcpy(ptr, str.c_str());
            }
            break;
        default:
            throw RuntimeException(HDF5_LOG_PREFIX + "unsupported type");
        }
    }
}

/* unused function

    size_t getDatasetSize(const string &filename, const string &datasetName) {
        H5ReadOnlyFile f(filename);
        H5DataSet set(datasetName, f.id());

        return getDatasetSize(set.id());
    }

    void getDataColName(const std::string& str, char delim, std::vector<std::string>& colsName){
        std::stringstream ss(str);
        std::string item;
        int lineIndex = 0;
        while (std::getline(ss, item, delim)) {
            if (!item.empty()) {
                if(lineIndex == 0){
                    lineIndex++;
                    continue;
                }

                if(lineIndex == 1){
                    colsName.push_back(item.substr(1, item.length() - 1));
                    lineIndex++;
                    continue;
                }
                if(item[0] == 'a' && item[1] != '.'){
                    colsName.push_back(item.substr(2, item.length() - 2));
                }
            }
            lineIndex++;
        }
        colsName.pop_back();
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

    size_t H5GeneralDataReader::readByCol()
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
*/

template void getGroupAttribute(const H5::Group& group, const string& attribute, int* value);

} // namespace H5PluginImp
