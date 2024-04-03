#include "hdf5_plugin.h"
#include "Exceptions.h"
#include "Types.h"
#include <hdf5_plugin_imp.h>
#include <hdf5_plugin_util.h>
#include <hdf5_plugin_pandas.h>
#include "ddbplugin/CommonInterface.h"


static Mutex hdf5Mutex;

ConstantSP h5ls(const ConstantSP &h5_path) {
    string syntax{"hdf5::ls(fileName) "};
    LockGuard<Mutex> guard{&hdf5Mutex};
    if (h5_path->getType() != DT_STRING || h5_path->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, syntax + "filename must be a string scalar.");

    H5::Exception::dontPrint();

    vector<string> objNames, objTypes;
    H5PluginImp::h5ls(h5_path->getString(), objNames, objTypes);
    size_t objNum = objNames.size();
    TableSP lsResult = Util::createTable({"objName", "objType"}, {DT_STRING, DT_STRING}, (INDEX) objNum, (INDEX) objNum);

    VectorSP h5ObjectName = lsResult->getColumn(0);
    VectorSP h5ObjectType = lsResult->getColumn(1);
    h5ObjectName->setString(0, (int) objNum, objNames.data());
    h5ObjectType->setString(0, (int) objNum, objTypes.data());

    return lsResult;
}

ConstantSP h5lsTable(const ConstantSP &filename) {
    string syntax{"hdf5::lsTable(fileName) "};
    LockGuard<Mutex> guard{&hdf5Mutex};
    if (filename->getType() != DT_STRING)
        throw IllegalArgumentException(__FUNCTION__, syntax + "fileName must be a string scalar.");

    H5::Exception::dontPrint();

    vector<string> datasetName, datasetDims, dataType;
    H5PluginImp::h5lsTable(filename->getString(), datasetName, datasetDims, dataType);
    size_t tableNum = datasetName.size();

    vector<string> resultColName{"tableName", "tableDims", "tableType"};
    vector<DATA_TYPE> resultColType{DT_STRING, DT_STRING, DT_STRING};

    VectorSP tableName = Util::createVector(DT_STRING, 0);
    VectorSP tableDims = Util::createVector(DT_STRING, 0);
    VectorSP tableType = Util::createVector(DT_STRING, 0);

    tableName->appendString(datasetName.data(), (int)tableNum);
    tableDims->appendString(datasetDims.data(), (int)tableNum);
    tableType->appendString(dataType.data(), (int)tableNum);

    return Util::createTable(resultColName, {tableName, tableDims, tableType});
}

ConstantSP extractHDF5Schema(const ConstantSP& filename, const ConstantSP& datasetName) {
    string syntax{"hdf5::extractHDF5Schema(fileName, datasetName) "};
    LockGuard<Mutex> guard{&hdf5Mutex};
    if (filename->getType() != DT_STRING || datasetName->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, syntax + "fileName must be a string scalar.");
    if(datasetName->getType() != DT_STRING || filename->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, syntax + "datasetName must be a string scalar.");

    H5::Exception::dontPrint();
    return H5PluginImp::extractHDF5Schema(filename->getString(), datasetName->getString());
}

ConstantSP loadHDF5(Heap *heap, vector<ConstantSP> &arguments) {
    string syntax{"hdf5::loadHDF5(fileName,datasetName,[schema],[startRow],[rowNum]) "};
    LockGuard<Mutex> guard{&hdf5Mutex};
    ConstantSP filename;
    ConstantSP datasetName;
    ConstantSP schema;
    size_t startRow;
    size_t rowNum;
    checkHDF5Parameter(heap, arguments, filename, datasetName, schema, startRow, rowNum, syntax);
    H5::Exception::dontPrint();
    return H5PluginImp::loadHDF5(filename->getString(), datasetName->getString(), schema, startRow, rowNum);
}

ConstantSP loadPandasHDF5(Heap *heap, vector<ConstantSP>& arguments){
    string syntax{"hdf5::loadPandasHDF5(fileName,datasetName,[schema],[startRow],[rowNum]) "};
    LockGuard<Mutex> guard{&hdf5Mutex};
    ConstantSP filename;
    ConstantSP groupname;
    ConstantSP schema;
    size_t startRow;
    size_t rowNum;
    checkHDF5Parameter(heap, arguments, filename, groupname, schema, startRow, rowNum, syntax);
    H5::Exception::dontPrint();
    return H5PluginImp::loadPandasHDF5(filename->getString(), groupname->getString(), schema, startRow, rowNum);
}

ConstantSP loadHDF5Ex(Heap *heap, vector<ConstantSP>& arguments) {
    string syntax{"hdf5::loadHDF5Ex(dbHandle, tableName, [partitionColumns], fileName, datasetName, [schema], [startRow], [rowNum], [transform]) "};
    LockGuard<Mutex> guard{&hdf5Mutex};
    ConstantSP db = arguments[0];
    ConstantSP tableName = arguments[1];
    ConstantSP partitionColumnNames = arguments[2];
    ConstantSP filename = arguments[3];
    ConstantSP datasetName = arguments[4];

    size_t startRow = 0;
    size_t rowNum = 0;
    ConstantSP schema = H5PluginImp::nullSP;
    H5::Exception::dontPrint();

    if (!db->isDatabase())
        throw IllegalArgumentException(__FUNCTION__, syntax + "dbHandle must be a database handle.");
    if (tableName->getType() != DT_STRING || tableName->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, syntax + "tableName must be a string scalar.");
    if (!partitionColumnNames->isNull() && (partitionColumnNames->getType() != DT_STRING ||
        !(partitionColumnNames->isVector() || partitionColumnNames->isScalar())))
        throw IllegalArgumentException(__FUNCTION__, syntax + "The partition columns must be string or string vector.");
    if (filename->getType() != DT_STRING || filename->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, syntax + "The filename  must be a string scalar.");
    if (datasetName->getType() != DT_STRING || datasetName->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, syntax + "The datasetName must be a string scalar.");
    if (arguments.size() >= 6) {
        if (arguments[5]->isNull())
            schema = H5PluginImp::nullSP;
        else if (!arguments[5]->isTable())
            throw IllegalArgumentException(__FUNCTION__, syntax + "schema must be a table containing column names and types.");
        else
            schema = arguments[5];
    }
    if (arguments.size() >= 7) {
        if (arguments[6]->isScalar() && arguments[6]->isNumber()) {
            if (arguments[6]->getLong() < 0) {
                throw IllegalArgumentException(__FUNCTION__, syntax + "startRow must be nonnegative.");
            }
            startRow = arguments[6]->getLong();
        } else if (arguments[6]->isNull())
            startRow = 0;
        else
            throw IllegalArgumentException(__FUNCTION__, syntax + "startRow must be an integer scalar.");


    }
    if (arguments.size() >= 8) {
        if (arguments[7]->isScalar() && arguments[7]->isNumber()) {
            if (arguments[7]->getLong() < 0)
                throw IllegalArgumentException(__FUNCTION__, syntax + "rowNum must be nonnegative.");
            rowNum = arguments[7]->getLong();
        } else if (arguments[7]->isNull())
            rowNum = 0;
        else
            throw IllegalArgumentException(__FUNCTION__, syntax + "rowNum must be an integer scalar.");

    }
    FunctionDefSP transform;
    if (arguments.size() >= 9) {
        if (arguments[8]->getType() != DT_FUNCTIONDEF)
            throw IllegalArgumentException(__FUNCTION__, syntax + "transform must be a function.");
        transform = FunctionDefSP(arguments[8]);
        return H5PluginImp::loadHDF5Ex(heap, db, tableName->getString(), partitionColumnNames,
                                   filename->getString(), datasetName->getString(),
                                   schema, startRow, rowNum, transform);
    } else {
        return H5PluginImp::loadHDF5Ex(heap, db, tableName->getString(), partitionColumnNames,
                                   filename->getString(), datasetName->getString(),
                                   schema, startRow, rowNum);
    }
}

ConstantSP HDF5DS(Heap *heap, vector<ConstantSP>& arguments) {
    string syntax{"hdf5::HDF5DS(fileName,datasetName,[schema],[dsNum]) "};
    LockGuard<Mutex> guard{&hdf5Mutex};
    ConstantSP filename = arguments[0];
    ConstantSP datasetName = arguments[1];

    int dsNum = 1;
    ConstantSP schema = H5PluginImp::nullSP;

    if (filename->getType() != DT_STRING || filename->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, syntax + "The filename  must be a string scalar.");
    if (datasetName->getType() != DT_STRING || datasetName->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, syntax + "The datasetName must be a string scalar.");
    if (arguments.size() >= 3) {
        if (arguments[2]->isNull())
            schema = H5PluginImp::nullSP;
        else if (!arguments[2]->isTable())
            throw IllegalArgumentException(__FUNCTION__, syntax + "schema must be a table containing column names and types.");
        else
            schema = arguments[2];
    }
    if (arguments.size() >= 4) {
        if (arguments[3]->isScalar() && arguments[3]->getType() == DT_INT) {
            dsNum = arguments[3]->getInt();
        }
        else if (arguments[3]->isNull())
            dsNum = 1;
        else
            throw IllegalArgumentException(__FUNCTION__, syntax + "dsNum must be an integer.");
        if (dsNum < 1)
            throw IllegalArgumentException(__FUNCTION__, syntax + "dsNum must be positive.");
    }
    H5::Exception::dontPrint();
    return H5PluginImp::HDF5DS(filename, datasetName, schema, (size_t)dsNum);
}

ConstantSP saveHDF5(Heap *heap, vector<ConstantSP> &arguments) {
    string syntax{"hdf5::saveHDF5(table, fileName, datasetName, [append], [stringMaxLength]) "};
    LockGuard<Mutex> guard{&hdf5Mutex};
    TableSP table = arguments[0];
    ConstantSP fileName = arguments[1];
    ConstantSP datasetName = arguments[2];
    bool append = false;
    unsigned stringMaxLength = 16;
    if(arguments[0]->getForm() != DF_TABLE){
        throw IllegalArgumentException(__FUNCTION__, "table must be a table type.");
    }
    if(arguments[1]->getType() != DT_STRING || arguments[1]->getForm() != DF_SCALAR){
        throw IllegalArgumentException(__FUNCTION__, "fileName must be a string scalar.");
    }
    if(arguments[2]->getType() != DT_STRING || arguments[2]->getForm() != DF_SCALAR){
        throw IllegalArgumentException(__FUNCTION__, "datasetName must be a string scalar.");
    }
    if(arguments.size() > 3 && !arguments[3]->isNull()){
        if(arguments[3]->getType() != DT_BOOL || arguments[3]->getForm() != DF_SCALAR){
            throw IllegalArgumentException(__FUNCTION__, "append must be a boolean scalar.");
        }
        append = arguments[3]->getBool();
    }
    if(arguments.size() > 4 && !arguments[4]->isNull()){
        if(arguments[4]->getCategory() != INTEGRAL || arguments[4]->getForm() != DF_SCALAR){
            throw IllegalArgumentException(__FUNCTION__, "stringMaxLength must be a integer scalar.");
        }
        stringMaxLength = arguments[4]->getInt();
        if(stringMaxLength <= 0){
            throw IllegalArgumentException(__FUNCTION__, "stringMaxLength must be positive.");
        }
    }
    H5::Exception::dontPrint();
    return H5PluginImp::saveHDF5(table, fileName->getString(), datasetName->getString(), append, stringMaxLength);
}
