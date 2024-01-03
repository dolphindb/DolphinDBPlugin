#include <hdf5_plugin_imp.h>
#include <hdf5_plugin_pandas.h>
#include <unordered_map>

#include "Exceptions.h"
#include "H5DataSet.h"
#include "H5Ipublic.h"

namespace H5PluginImp {
void getColName(const std::string &str, char delim, std::vector<std::string> &colsName) {
    std::stringstream ss(str);
    std::string item;
    int lineIndex = 0;
    while (std::getline(ss, item, delim)) {
        if (!item.empty()) {
            if (lineIndex == 0 || lineIndex == 1 || lineIndex == 2) {
                lineIndex++;
                continue;
            }
            if (lineIndex == 3) {
                colsName.push_back(item.substr(1, item.length() - 1));
                lineIndex++;
                continue;
            }
            if (item[0] == 'a' && item[1] != '.') colsName.push_back(item.substr(2, item.length() - 2));
            // if(lineIndex % 2 == 1){
            //    colsName.push_back(item.substr(2, item.length() - 2));
            //}
        }
        lineIndex++;
    }
    // FIXME potential bug.
    colsName.insert(colsName.begin(), "index");
    colsName.pop_back();
}

void getKindColName(const std::string &str, char delim, std::vector<std::string> &colsName) {
    std::stringstream ss(str);
    std::string item;
    int lineIndex = 0;
    while (std::getline(ss, item, delim)) {
        if (!item.empty()) {
            if (lineIndex == 0) {
                lineIndex++;
                continue;
            }
            if (lineIndex == 1) {
                colsName.push_back(item.substr(1, item.length() - 1));
                lineIndex++;
                continue;
            }
            if (item[0] == 'a' && item[1] != '.') {
                colsName.push_back(item.substr(2, item.length() - 2));
            }
        }
        lineIndex++;
    }
}

ConstantSP loadPandasHDF5(const string &fileName, const string &groupName, const ConstantSP &schema,
                          const size_t startRow, const size_t rowNum) {
    H5::Exception::dontPrint();
    H5::H5File f(fileName, H5F_ACC_RDONLY);
    if (!f.nameExists(groupName))
        throw RuntimeException(HDF5_LOG_PREFIX + "The group [" + groupName + "] is not exist.");

    H5::Group group;
    HDF5_SAFE_EXECUTE(group = f.openGroup(groupName));

    // check pandas_type.
    if (!group.attrExists("pandas_type"))
        throw RuntimeException(HDF5_LOG_PREFIX + "The file name: " + fileName +
                               " is not pandas hdf5 file. Try loadHDF5 function.");

    // check table type.
    string tableTypeInfo;
    if (group.attrExists("table_type")) {
        getGroupAttribute(group, "table_type", tableTypeInfo);
        unordered_set<string> tableTypeSet{"appendable_frame", "appendable_series", "appendable_multiframe",
                                           "appendable_multiseries"};
        if(tableTypeSet.find(tableTypeInfo) == tableTypeSet.end()) {
            throw RuntimeException(HDF5_LOG_PREFIX +
                                   "The type: " + tableTypeInfo + " is not support now. Try loadHDF5 function.");
        }

        GroupInfo info;
        string colValueInfo;
        getGroupAttribute(group, "values_cols", colValueInfo);

        vector<string> colValueArray;
        getKindColName(colValueInfo, '\n', colValueArray);
        vector<string> dataKindArray;

        H5::DataSet dSet;
        HDF5_SAFE_EXECUTE(dSet = group.openDataSet("table"));
        for (size_t i = 0; i < colValueArray.size(); i++) {
            string attributeName = colValueArray[i] + "_kind";
            if (dSet.attrExists(attributeName)) {
                string kindValue;
                getDataSetAttribute(dSet, attributeName, kindValue);
                vector<string> kindArray;
                getKindColName(kindValue, '\n', kindArray);
                dataKindArray.insert(dataKindArray.end(), kindArray.begin(), kindArray.end());
            } else
                break;
        }
        info.kindColsName = &dataKindArray;
        info.dataType = tableTypeInfo;

        // get col line.
        string colsInfo;
        getGroupAttribute(group, "non_index_axes", colsInfo);
        std::vector<std::string> col_name;
        getColName(colsInfo, '\n', col_name);
        info.colsName = &col_name;

        // pandas hdf5 file dataset name is table.
        return loadPandasHDF5(dSet.getId(), schema, startRow, rowNum, info);
    } else {
        string pandasTypeInfo;
        getGroupAttribute(group, "pandas_type", pandasTypeInfo);
        // handle frame fixed h5 file.
        if (pandasTypeInfo == "frame") {
            return loadFrameTypeHDF5(group, schema, startRow, rowNum, groupName);
        } else if (pandasTypeInfo == "series") {
            return loadSeriesTypeHDF5(group, schema, startRow, rowNum, groupName);
        } else {
            throw RuntimeException(HDF5_LOG_PREFIX + "The type: " + pandasTypeInfo +
                                   " is not support now. Try loadHDF5 function.");
        }
    }
}
ConstantSP combineLabelLevel(ConstantSP label, ConstantSP level) {
    INDEX labelSize = label->size();
    vector<long long> labelIndex;
    labelIndex.reserve(labelSize);

    INDEX start = 0, count;
    switch (label->getType()) {
        case DT_CHAR: {
            char buf[Util::BUF_SIZE];
            const char *pbuf;
            while (start < labelSize) {
                count = std::min(Util::BUF_SIZE, labelSize - start);
                pbuf = label->getCharConst(start, count, buf);
                labelIndex.insert(labelIndex.end(), pbuf, pbuf + count);
                start += count;
            }
        } break;
        case DT_SHORT: {
            short buf[Util::BUF_SIZE];
            const short *pbuf;
            while (start < labelSize) {
                count = std::min(Util::BUF_SIZE, labelSize - start);
                pbuf = label->getShortConst(start, count, buf);
                labelIndex.insert(labelIndex.end(), pbuf, pbuf + count);
                start += count;
            }
        } break;
        case DT_INT: {
            int buf[Util::BUF_SIZE];
            const int *pbuf;
            while (start < labelSize) {
                count = std::min(Util::BUF_SIZE, labelSize - start);
                pbuf = label->getIntConst(start, count, buf);
                labelIndex.insert(labelIndex.end(), pbuf, pbuf + count);
                start += count;
            }
        } break;
        case DT_LONG: {
            long long buf[Util::BUF_SIZE];
            const long long *pbuf;
            while (start < labelSize) {
                count = std::min(Util::BUF_SIZE, labelSize - start);
                pbuf = label->getLongConst(start, count, buf);
                labelIndex.insert(labelIndex.end(), pbuf, pbuf + count);
                start += count;
            }
        } break;
        default:
            throw RuntimeException(HDF5_LOG_PREFIX + "index form process don't support label type: " +
                                   Util::getDataTypeString(level->getType()));
    }

    INDEX levelSize = level->size();
    start = 0;
    switch (level->getType()) {
        case DT_CHAR: {
            vector<char> levelVec;
            levelVec.reserve(levelSize);
            char buf[Util::BUF_SIZE];
            const char *pbuf;
            while (start < levelSize) {
                count = std::min(Util::BUF_SIZE, levelSize - start);
                pbuf = level->getCharConst(start, count, buf);
                levelVec.insert(levelVec.end(), pbuf, pbuf + count);
                start += count;
            }
            vector<char> data;
            data.reserve(labelSize);
            for (unsigned int i = 0; i < labelIndex.size(); ++i) {
                unsigned int index = labelIndex[i];
                if (index < 0 || index >= levelVec.size()) {
                    throw RuntimeException(HDF5_LOG_PREFIX + "index form process failed due to index exceed");
                }
                data.emplace_back(levelVec[labelIndex[i]]);
            }
            VectorSP ret = Util::createVector(level->getType(), 0, labelSize);
            ret->appendChar(data.data(), labelSize);
            return ret;
        }
        case DT_SHORT: {
            vector<short> levelVec;
            levelVec.reserve(levelSize);
            short buf[Util::BUF_SIZE];
            const short *pbuf;
            while (start < levelSize) {
                count = std::min(Util::BUF_SIZE, levelSize - start);
                pbuf = level->getShortConst(start, count, buf);
                levelVec.insert(levelVec.end(), pbuf, pbuf + count);
                start += count;
            }
            vector<short> data;
            data.reserve(labelSize);
            for (unsigned int i = 0; i < labelIndex.size(); ++i) {
                unsigned int index = labelIndex[i];
                if (index < 0 || index >= levelVec.size()) {
                    throw RuntimeException(HDF5_LOG_PREFIX + "index form process failed due to index exceed");
                }
                data.emplace_back(levelVec[labelIndex[i]]);
            }
            VectorSP ret = Util::createVector(level->getType(), 0, labelSize);
            ret->appendShort(data.data(), labelSize);
            return ret;
        }
        case DT_INT: {
            vector<int> levelVec;
            levelVec.reserve(levelSize);
            int buf[Util::BUF_SIZE];
            const int *pbuf;
            while (start < levelSize) {
                count = std::min(Util::BUF_SIZE, levelSize - start);
                pbuf = level->getIntConst(start, count, buf);
                levelVec.insert(levelVec.end(), pbuf, pbuf + count);
                start += count;
            }
            vector<int> data;
            data.reserve(labelSize);
            for (unsigned int i = 0; i < labelIndex.size(); ++i) {
                unsigned int index = labelIndex[i];
                if (index < 0 || index >= levelVec.size()) {
                    throw RuntimeException(HDF5_LOG_PREFIX + "index form process failed due to index exceed");
                }
                data.emplace_back(levelVec[labelIndex[i]]);
            }
            VectorSP ret = Util::createVector(level->getType(), 0, labelSize);
            ret->appendInt(data.data(), labelSize);
            return ret;
        }
        case DT_LONG: {
            vector<long long> levelVec;
            levelVec.reserve(levelSize);
            long long buf[Util::BUF_SIZE];
            const long long *pbuf;
            while (start < levelSize) {
                count = std::min(Util::BUF_SIZE, levelSize - start);
                pbuf = level->getLongConst(start, count, buf);
                levelVec.insert(levelVec.end(), pbuf, pbuf + count);
                start += count;
            }
            vector<long long> data;
            data.reserve(labelSize);
            for (unsigned int i = 0; i < labelIndex.size(); ++i) {
                unsigned int index = labelIndex[i];
                if (index < 0 || index >= levelVec.size()) {
                    throw RuntimeException(HDF5_LOG_PREFIX + "index form process failed due to index exceed");
                }
                data.emplace_back(levelVec[labelIndex[i]]);
            }
            VectorSP ret = Util::createVector(level->getType(), 0, labelSize);
            ret->appendLong(data.data(), labelSize);
            return ret;
        }
        case DT_TIMESTAMP: {
            vector<long long> levelVec;
            levelVec.reserve(levelSize);
            long long buf[Util::BUF_SIZE];
            const long long *pbuf;
            while (start < levelSize) {
                count = std::min(Util::BUF_SIZE, levelSize - start);
                pbuf = level->getLongConst(start, count, buf);
                levelVec.insert(levelVec.end(), pbuf, pbuf + count);
                start += count;
            }
            vector<long long> data;
            data.reserve(labelSize);
            for (unsigned int i = 0; i < labelIndex.size(); ++i) {
                unsigned int index = labelIndex[i];
                if (index < 0 || index >= levelVec.size()) {
                    throw RuntimeException(HDF5_LOG_PREFIX + "index form process failed due to index exceed");
                }
                data.emplace_back(levelVec[labelIndex[i]]);
            }
            VectorSP ret = Util::createVector(level->getType(), 0, labelSize);
            ret->appendLong(data.data(), labelSize);
            return ret;
        }
        case DT_STRING:
        case DT_SYMBOL: {
            vector<string> levelVec;
            levelVec.reserve(levelSize);
            for (INDEX i = 0; i < level->size(); ++i) {
                levelVec.push_back(level->getString(i));
            }
            vector<string> data;
            data.reserve(labelSize);
            for (unsigned int i = 0; i < labelIndex.size(); ++i) {
                unsigned int index = labelIndex[i];
                if (index < 0 || index >= levelVec.size()) {
                    throw RuntimeException(HDF5_LOG_PREFIX + "index form process failed due to index exceed");
                }
                data.emplace_back(levelVec[labelIndex[i]]);
            }
            VectorSP ret = Util::createVector(level->getType(), 0, labelSize);
            ret->appendString(data.data(), labelSize);
            return ret;
        }
        case DT_FLOAT: {
            vector<float> levelVec;
            levelVec.reserve(levelSize);
            float buf[Util::BUF_SIZE];
            const float *pbuf;
            while (start < levelSize) {
                count = std::min(Util::BUF_SIZE, levelSize - start);
                pbuf = level->getFloatConst(start, count, buf);
                levelVec.insert(levelVec.end(), pbuf, pbuf + count);
                start += count;
            }
            vector<float> data;
            data.reserve(labelSize);
            for (unsigned int i = 0; i < labelIndex.size(); ++i) {
                unsigned int index = labelIndex[i];
                if (index < 0 || index >= levelVec.size()) {
                    throw RuntimeException(HDF5_LOG_PREFIX + "index form process failed due to index exceed");
                }
                data.emplace_back(levelVec[labelIndex[i]]);
            }
            VectorSP ret = Util::createVector(level->getType(), 0, labelSize);
            ret->appendFloat(data.data(), labelSize);
            return ret;
        }
        case DT_DOUBLE: {
            vector<double> levelVec;
            levelVec.reserve(levelSize);
            double buf[Util::BUF_SIZE];
            const double *pbuf;
            while (start < levelSize) {
                count = std::min(Util::BUF_SIZE, levelSize - start);
                pbuf = level->getDoubleConst(start, count, buf);
                levelVec.insert(levelVec.end(), pbuf, pbuf + count);
                start += count;
            }
            vector<double> data;
            data.reserve(labelSize);
            for (unsigned int i = 0; i < labelIndex.size(); ++i) {
                unsigned int index = labelIndex[i];
                if (index < 0 || index >= levelVec.size()) {
                    throw RuntimeException(HDF5_LOG_PREFIX + "index form process failed due to index exceed");
                }
                data.emplace_back(levelVec[labelIndex[i]]);
            }
            VectorSP ret = Util::createVector(level->getType(), 0, labelSize);
            ret->appendDouble(data.data(), labelSize);
            return ret;
        }
        default:
            throw RuntimeException(HDF5_LOG_PREFIX + "index form process don't support level type: " +
                                   Util::getDataTypeString(level->getType()));
    }
    return label;
}
vector<ConstantSP> readSimpleDataFromDataSet(hid_t set, H5DataType &type, const TableSP &schema, const size_t startRow,
                                             const size_t readRowNum) {
    H5DataType convertedType;
    if (!TypeColumn::convertHdf5SimpleType(type, convertedType))
        throw RuntimeException(HDF5_LOG_PREFIX + "unsupported data type");

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
    if (!reader.columnNum() || !reader.rowNum()) throw RuntimeException(HDF5_LOG_PREFIX + "empty dataset!");

    vector<H5ColumnSP> cols;
    createColumnVec(cols, reader.columnNum(), std::min(readRowNum, rowNum), convertedType, schema);

    vector<ConstantSP> colVec;
    DatasetAppendRunnerSP appendRunner = new SimpleDatasetAppendRunner();
    doReadDataset_concurrent(reader, appendRunner, cols, colVec);
    return colVec;
}

vector<ConstantSP> loadDataSet(const hid_t set, const TableSP &schema, size_t startRow, size_t readRowNum) {
    H5DataType t;
    t.openFromDataset(set);
    registerUnixTimeConvert();
    return readSimpleDataFromDataSet(set, t, schema, startRow, readRowNum);
}

ConstantSP loadPandasHDF5(const hid_t set, const ConstantSP &schema, const size_t startRow, const size_t rowNum,
                          GroupInfo &info) {
    H5DataType t;
    t.openFromDataset(set);
    registerUnixTimeConvert();

    TableSP tableWithSchema =
        schema->isNull() ? static_cast<TableSP>(nullSP) : DBFileIO::createEmptyTableFromSchema(schema);

    switch (H5Tget_class(t.id())) {
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

TableSP loadFrameTypeHDF5(const H5::Group &group, const ConstantSP &schema, size_t startRow, size_t readRowNum,
                          const string &groupName) {
    TableSP tableWithSchema =
        schema->isNull() ? static_cast<TableSP>(nullSP) : DBFileIO::createEmptyTableFromSchema(schema);
    string fileName;
    HDF5_SAFE_EXECUTE(fileName = group.getFileName());
    string parseFailPrefix = "group [" + groupName + "] in file [" + fileName + "] parsed failed. ";
    // global variable
    int nDim;
    int nBlocks;
    getGroupAttribute<int>(group, "ndim", &nDim);
    getGroupAttribute<int>(group, "nblocks", &nBlocks);
    if (nDim != 2) {
        throw RuntimeException(
            HDF5_LOG_PREFIX + parseFailPrefix +
            "only pandas HDF5 data with '2' ndim is supported. Current ndim: " + std::to_string(nDim));
    }
    string axis0_variety;
    string axis1_variety;
    getGroupAttribute(group, "axis0_variety", axis0_variety);
    getGroupAttribute(group, "axis1_variety", axis1_variety);
    if (axis0_variety != "regular") {
        throw RuntimeException(
            HDF5_LOG_PREFIX + parseFailPrefix +
            "only pandas HDF5 data with 'regular' axis0_variety is supported. Current axis0_variety type: " +
            axis0_variety);
    }
    if (axis1_variety == "regular") {
        string colNameStr = "axis0";
        if (!group.nameExists(colNameStr)) {
            throw RuntimeException(HDF5_LOG_PREFIX + parseFailPrefix + colNameStr + " doesn't exist in group.");
        }
        H5::DataSet colNameDataSet;
        HDF5_SAFE_EXECUTE(colNameDataSet = group.openDataSet(colNameStr));
        // parse cols name dataset.
        ConstantSP colNameSP = loadDataSet(colNameDataSet.getId(), nullSP, 0, 0)[0];
        vector<size_t> rowAndColNum;
        getRowAndColNum(colNameDataSet.getId(), rowAndColNum);
        vector<string> nameArray;
        nameArray.push_back("index");
        size_t colsNameNum = rowAndColNum[0];
        for (size_t i = 0; i < colsNameNum; i++) {
            nameArray.push_back(colNameSP->getString(i));
        }

        // parse row index.
        string colIndexStr = "axis1";
        if (!group.nameExists(colIndexStr)) {
            throw RuntimeException(HDF5_LOG_PREFIX + parseFailPrefix + colIndexStr + " doesn't exists in group.");
        }
        H5::DataSet rowIndexDataSet;
        HDF5_SAFE_EXECUTE(rowIndexDataSet = group.openDataSet(colIndexStr));
        ConstantSP rowIndex = loadDataSet(rowIndexDataSet.getId(), nullSP, startRow, readRowNum)[0];
        vector<ConstantSP> dataCols;
        dataCols.push_back(rowIndex);
        vector<string> dataColsName;
        for (int i = 0; i < nBlocks; i++) {
            // get data col name order.
            string blockItem = "block" + std::to_string(i) + "_items";
            H5::DataSet itemSet;
            HDF5_SAFE_EXECUTE(itemSet = group.openDataSet(blockItem));
            hid_t itemId = itemSet.getId();

            ConstantSP colsName = loadDataSet(itemId, nullSP, 0, 0)[0];
            rowAndColNum.clear();
            getRowAndColNum(itemId, rowAndColNum);
            for (size_t index = 0; index < rowAndColNum[0]; index++) {
                dataColsName.push_back(colsName->getString(index));
            }
            // get data.
            string blockValue = "block" + std::to_string(i) + "_values";
            H5::DataSet valueSet;
            HDF5_SAFE_EXECUTE(valueSet = group.openDataSet(blockValue));
            hid_t valueId = valueSet.getId();
            vector<ConstantSP> data = loadDataSet(valueId, schema, startRow, readRowNum);
            dataCols.insert(dataCols.end(), data.begin(), data.end());
        }
        if (nameArray.size() != dataCols.size()) {
            throw RuntimeException(HDF5_LOG_PREFIX + parseFailPrefix + "colNames size(" +
                                   std::to_string(nameArray.size()) + ") is not equal to col size(" +
                                   std::to_string(dataCols.size()) + ").");
        }

        vector<ConstantSP> finalDataCols;
        // push back index cols.
        finalDataCols.push_back(dataCols[0]);
        for (size_t i = 1; i < nameArray.size(); i++) {
            size_t index = find(dataColsName.begin(), dataColsName.end(), nameArray[i]) - dataColsName.begin();
            finalDataCols.push_back(dataCols[index + 1]);
        }
        if (tableWithSchema.isNull() || tableWithSchema->isNull()) {
            return Util::createTable(nameArray, finalDataCols);
        }
        return appendColumnVecToTable(tableWithSchema, finalDataCols);
    } else {
        int axis1NLevels;
        getGroupAttribute<int>(group, "axis1_nlevels", &axis1NLevels);

        string colNameStr = "axis0";
        if (!group.nameExists(colNameStr)) {
            throw RuntimeException(HDF5_LOG_PREFIX + parseFailPrefix + colNameStr + " doesn't exist in group.");
        }
        H5::DataSet colNameDataSet;
        HDF5_SAFE_EXECUTE(colNameDataSet = group.openDataSet(colNameStr));
        // parse cols name dataset.
        ConstantSP colNameSP = loadDataSet(colNameDataSet.getId(), nullSP, 0, 0)[0];
        vector<size_t> rowAndColNum;
        getRowAndColNum(colNameDataSet.getId(), rowAndColNum);

        vector<string> nameArray;
        vector<ConstantSP> dataCols;

        vector<string> indexNames;
        for (int i = 0; i < axis1NLevels; ++i) {
            // get label data
            string colLabelStr = "axis1_label" + std::to_string(i);
            if (!group.nameExists(colLabelStr)) {
                throw RuntimeException(HDF5_LOG_PREFIX + parseFailPrefix + colLabelStr + " doesn't exist in group.");
            }
            H5::DataSet rowLabelDataSet;
            HDF5_SAFE_EXECUTE(rowLabelDataSet = group.openDataSet(colLabelStr));
            ConstantSP rowLabel = loadDataSet(rowLabelDataSet.getId(), nullSP, startRow, readRowNum)[0];

            // get index data
            string colIndexStr = "axis1_level" + std::to_string(i);
            if (!group.nameExists(colIndexStr)) {
                throw RuntimeException(HDF5_LOG_PREFIX + parseFailPrefix + colIndexStr + " doesn't exist in group.");
            }
            H5::DataSet rowIndexDataSet;
            HDF5_SAFE_EXECUTE(rowIndexDataSet = group.openDataSet(colIndexStr));
            ConstantSP rowIndex = loadDataSet(rowIndexDataSet.getId(), nullSP, startRow, readRowNum)[0];

            // push back name into nameArray
            try {
                // in case of 'name' attribute not found.
                string name;
                getDataSetAttribute(rowIndexDataSet, "name", name);
                indexNames.push_back(name);
            } catch (...) {
                indexNames.emplace_back("index_" + std::to_string(i));
            }

            // form final index column & push back into dataCols
            ConstantSP transCol = combineLabelLevel(rowLabel, rowIndex);
            dataCols.push_back(transCol);
        }
        unordered_set<string> indexNameSet;
        for (string &name : indexNames) {
            indexNameSet.insert(name);
        }
        if (indexNameSet.size() == indexNames.size()) {
            for (string &name : indexNames) {
                nameArray.push_back(name);
            }
        } else {
            for (unsigned int i = 0; i < indexNames.size(); ++i) {
                nameArray.emplace_back("index_" + std::to_string(i));
            }
        }

        size_t colsNameNum = rowAndColNum[0];
        for (size_t i = 0; i < colsNameNum; i++) {
            nameArray.push_back(colNameSP->getString(i));
        }

        // parse row index.
        vector<string> dataColsName;
        for (int i = 0; i < nBlocks; ++i) {
            // get data col name order.
            string blockItem = "block" + std::to_string(i) + "_items";
            H5::DataSet itemSet;
            HDF5_SAFE_EXECUTE(itemSet = group.openDataSet(blockItem));
            ConstantSP colsName = loadDataSet(itemSet.getId(), nullSP, 0, 0)[0];
            rowAndColNum.clear();
            getRowAndColNum(itemSet.getId(), rowAndColNum);
            for (size_t index = 0; index < rowAndColNum[0]; index++) {
                dataColsName.push_back(colsName->getString(index));
            }
            // get data.
            string blockValue = "block" + std::to_string(i) + "_values";
            H5::DataSet valueSet;
            HDF5_SAFE_EXECUTE(valueSet = group.openDataSet(blockValue).getId());
            vector<ConstantSP> data = loadDataSet(valueSet.getId(), schema, startRow, readRowNum);
            dataCols.insert(dataCols.end(), data.begin(), data.end());
        }
        if (nameArray.size() != dataCols.size()) {
            throw RuntimeException(HDF5_LOG_PREFIX + parseFailPrefix + "colNames size(" +
                                   std::to_string(nameArray.size()) + ") is not equal to col size(" +
                                   std::to_string(dataCols.size()) + ").");
        }
        // push back index cols.
        vector<ConstantSP> finalDataCols;
        for (int i = 0; i < axis1NLevels; ++i) {
            finalDataCols.push_back(dataCols[i]);
        }
        for (size_t i = axis1NLevels; i < nameArray.size(); i++) {
            size_t index = find(dataColsName.begin(), dataColsName.end(), nameArray[i]) - dataColsName.begin();
            if (index == dataColsName.size()) {
                throw RuntimeException(HDF5_LOG_PREFIX + parseFailPrefix + "col " + nameArray[i] +
                                       " not found in HDF5 dataSet, parse failed.");
            }
            finalDataCols.push_back(dataCols[index + axis1NLevels]);
        }
        if (tableWithSchema.isNull() || tableWithSchema->isNull()) {
            return Util::createTable(nameArray, finalDataCols);
        }
        return appendColumnVecToTable(tableWithSchema, finalDataCols);
    }
}

TableSP loadSeriesTypeHDF5(const H5::Group &group, const ConstantSP &schema, size_t startRow, size_t readRowNum,
                           const string &groupName) {
    TableSP tableWithSchema =
        schema->isNull() ? static_cast<TableSP>(nullSP) : DBFileIO::createEmptyTableFromSchema(schema);
    string fileName;
    HDF5_SAFE_EXECUTE(fileName = group.getFileName());
    string parseFailPrefix = "group [" + groupName + "] in file [" + fileName + "] parsed failed. ";

    string indexVariety;
    getGroupAttribute(group, "index_variety", indexVariety);
    if (indexVariety == "regular") {
        vector<string> nameArray = {"index", "value"};
        H5::DataSet indexSet;
        HDF5_SAFE_EXECUTE(indexSet = group.openDataSet("index"));
        H5::DataSet valueSet;
        HDF5_SAFE_EXECUTE(valueSet = group.openDataSet("values"));
        ConstantSP indexColSP = loadDataSet(indexSet.getId(), nullSP, startRow, readRowNum)[0];
        ConstantSP valueColSP = loadDataSet(valueSet.getId(), nullSP, startRow, readRowNum)[0];
        vector<ConstantSP> cols;
        cols.push_back(indexColSP);
        cols.push_back(valueColSP);
        if (tableWithSchema.isNull() || tableWithSchema->isNull()) {
            return Util::createTable(nameArray, cols);
        }
        return appendColumnVecToTable(tableWithSchema, cols);
    } else if (indexVariety == "multi") {
        int indexNLevel;
        getGroupAttribute<int>(group, "index_nlevels", &indexNLevel);

        vector<string> nameArray;
        vector<ConstantSP> cols;
        for (int i = 0; i < indexNLevel; ++i) {
            string colLabelStr = "index_label" + std::to_string(i);
            H5::DataSet rowLabelDataSet;
            HDF5_SAFE_EXECUTE(rowLabelDataSet = group.openDataSet(colLabelStr));
            ConstantSP rowLabel = loadDataSet(rowLabelDataSet.getId(), nullSP, startRow, readRowNum)[0];

            // get index data
            string colIndexStr = "index_level" + std::to_string(i);
            if (!group.nameExists(colIndexStr)) {
                throw RuntimeException(HDF5_LOG_PREFIX + parseFailPrefix + colIndexStr + " doesn't exist in group.");
            }
            H5::DataSet rowIndexDataSet;
            HDF5_SAFE_EXECUTE(rowIndexDataSet = group.openDataSet(colIndexStr));
            ConstantSP rowIndex = loadDataSet(rowIndexDataSet.getId(), nullSP, startRow, readRowNum)[0];

            // push back name into nameArray
            try {
                // in case of 'name' attribute not found.
                string name;
                getDataSetAttribute(rowIndexDataSet, "name", name);
                nameArray.push_back(name);
            } catch (...) {
                nameArray.emplace_back("index_" + std::to_string(i));
            }

            // form final index column & push back into dataCols
            ConstantSP transCol = combineLabelLevel(rowLabel, rowIndex);
            cols.push_back(transCol);
        }

        H5::DataSet valueSet;
        HDF5_SAFE_EXECUTE(valueSet = group.openDataSet("values"));
        ConstantSP valueColSP = loadDataSet(valueSet.getId(), nullSP, startRow, readRowNum)[0];
        nameArray.emplace_back("value");
        cols.push_back(valueColSP);
        if (tableWithSchema.isNull() || tableWithSchema->isNull()) {
            return Util::createTable(nameArray, cols);
        }
        return appendColumnVecToTable(tableWithSchema, cols);
    } else {
        throw RuntimeException(HDF5_LOG_PREFIX + parseFailPrefix + "unsupported index_variety type: " + indexVariety);
    }
    return nullSP;
}

}  // namespace H5PluginImp