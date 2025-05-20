#include "parser.h"

#include "json.hpp"
#include "ddbplugin/Plugin.h"

#if defined(__GNUC__) && __GNUC__ >= 4
#define LIKELY(x) (__builtin_expect((x), 1))
#define UNLIKELY(x) (__builtin_expect((x), 0))
#else
#define LIKELY(x) (x)
#define UNLIKELY(x) (x)
#endif

static ConstantSP const void_ = Util::createConstant(DT_VOID);
void setData(nlohmann::json &data, vector<ConstantSP> &cols, std::map<string, int> &colIdx, vector<DATA_TYPE> &dt,
             int i, Heap *heap) {
    for (auto it = data.begin(); it != data.end(); ++it) {
        if (colIdx.find(it.key()) == colIdx.end()) {
            PLUGIN_LOG_ERR(
                "error ocurred when parse json data in subscribe callback of mqtt. the error message is the given JSON "
                "data does not have a key named:",
                it.key());
            throw RuntimeException("The given JSON data does not have a key named " + it.key());
        }
        int curCol = colIdx[it.key()];
        std::string colCurData = it->get<string>();
        try {
            if (colCurData.empty()) {
                cols[curCol]->setNull(i);
                continue;
            }
            switch (dt[curCol]) {
                case DT_BOOL:  // true, false, True, False
                {
                    cols[curCol]->setBool(i, colCurData == "1");
                } break;
                case DT_CHAR: {
                    int charData = 0;
                    charData = std::stoi(colCurData);
                    cols[curCol]->setChar(i, (char)charData);  // todo: should be char
                } break;
                case DT_SHORT:
                    cols[curCol]->setShort(i, std::stoi(colCurData));
                    break;
                case DT_INT:
                    cols[curCol]->setInt(i, std::stoi(colCurData));
                    break;
                case DT_LONG:
                    cols[curCol]->setLong(i, std::stol(colCurData));
                    break;
                case DT_DATE: {
                    static FunctionDefSP const date = Util::getFuncDefFromHeap(heap, "date");
                    ConstantSP d = date->call(heap, new String(colCurData), void_);
                    cols[curCol]->setInt(i, d->getInt());
                    break;
                }
                case DT_MONTH: {
                    cols[curCol]->setInt(i, std::stoi(colCurData));
                    break;
                }
                case DT_TIME: {
                    static FunctionDefSP const date = Util::getFuncDefFromHeap(heap, "time");
                    ConstantSP d = date->call(heap, new String(colCurData), void_);
                    cols[curCol]->setInt(i, d->getInt());
                    break;
                }
                case DT_MINUTE: {
                    static FunctionDefSP const date = Util::getFuncDefFromHeap(heap, "minute");
                    ConstantSP d = date->call(heap, new String(colCurData), void_);
                    cols[curCol]->setInt(i, d->getInt());
                    break;
                }
                case DT_SECOND: {
                    static FunctionDefSP const date = Util::getFuncDefFromHeap(heap, "second");
                    ConstantSP d = date->call(heap, new String(colCurData), void_);
                    cols[curCol]->setInt(i, d->getInt());
                    break;
                }
                case DT_DATETIME: {
                    static FunctionDefSP const date = Util::getFuncDefFromHeap(heap, "datetime");
                    ConstantSP d = date->call(heap, new String(colCurData), void_);
                    cols[curCol]->setLong(i, d->getLong());
                    break;
                }
                case DT_TIMESTAMP: {
                    std::string dateStr = it->get<string>();
                    if (colCurData.empty()) {
                        cols[curCol]->setNull(i);
                    } else {
                        static FunctionDefSP const date = Util::getFuncDefFromHeap(heap, "timestamp");
                        ConstantSP d = date->call(heap, new String(colCurData), void_);
                        cols[curCol]->setLong(i, d->getLong());
                    }
                    break;
                }
                case DT_NANOTIME: {
                    static FunctionDefSP const date = Util::getFuncDefFromHeap(heap, "nanotime");
                    ConstantSP d = date->call(heap, new String(colCurData), void_);
                    cols[curCol]->setLong(i, d->getLong());
                    break;
                }
                case DT_NANOTIMESTAMP: {
                    static FunctionDefSP const date = Util::getFuncDefFromHeap(heap, "nanotimestamp");
                    ConstantSP d = date->call(heap, new String(colCurData), void_);
                    cols[curCol]->setLong(i, d->getLong());
                    break;
                }
                case DT_FLOAT:
                    cols[curCol]->setFloat(i, std::stof(colCurData));
                    break;
                case DT_DOUBLE:
                    cols[curCol]->setFloat(i, std::stod(colCurData));
                    break;
                case DT_SYMBOL:
                case DT_STRING:
                    cols[curCol]->setString(i, colCurData);
                    break;
                case DT_INT128: {
                    static FunctionDefSP const date = Util::getFuncDefFromHeap(heap, "int128");
                    ConstantSP d = date->call(heap, new String(colCurData), void_);
                    cols[curCol]->set(i, d);
                    break;
                }
                case DT_UUID: {
                    static FunctionDefSP const date = Util::getFuncDefFromHeap(heap, "uuid");
                    ConstantSP d = date->call(heap, new String(colCurData), void_);
                    cols[curCol]->set(i, d);
                    break;
                }
                case DT_IP: {
                    static FunctionDefSP const date = Util::getFuncDefFromHeap(heap, "ipaddr");
                    ConstantSP d = date->call(heap, new String(colCurData), void_);
                    cols[curCol]->set(i, d);
                    break;
                }
                default:
                    throw RuntimeException("The schema type in position " + std::to_string(curCol + 1) +
                                           " does not support");
            }
        } catch (exception &e) {
            throw RuntimeException(LOG_PRE_STR + " set data failed when parse csv or json, error message is <" +
                                   string(e.what()) + ">" + " data type is <" + Util::getDataTypeString(dt[curCol]) +
                                   ">, data content is:" + colCurData);
        }
    }
}

ConstantSP parseJson(Heap *heap, vector<ConstantSP> &args) {
    using namespace nlohmann;
    VectorSP schema = args[0];
    VectorSP colNames = args[1];
    std::map<string, int> colIdx;
    for (int i = 0; i < colNames->size(); ++i) {
        colIdx.emplace(colNames->getString(i), i);
    }
    string original = args[2]->getString();
    json parsedObj = json::parse(original);
    bool isJsonArray = parsedObj.is_array();
    int nCols = isJsonArray ? parsedObj[0].size() : parsedObj.size();
    int nRows = isJsonArray ? parsedObj.size() : 1;
    vector<DATA_TYPE> dt(nCols);
    vector<ConstantSP> cols(nCols);
    if (nCols != schema->size()) {
        throw RuntimeException(
            LOG_PRE_STR +
            "parse json data to ddb table failed, column number of data parsed from jsom not equal to schema");
    }

    for (int i = 0; i < nCols; ++i) {
        dt[i] = (DATA_TYPE)schema->getInt(i);
        if (dt[i] == DT_SYMBOL) dt[i] = DT_STRING;
        cols[i] = Util::createVector((DATA_TYPE)schema->getInt(i), nRows, nRows);
    }

    if (isJsonArray) {
        for (int i = 0; i < nRows; ++i) {
            auto &row = parsedObj[i];
            setData(row, cols, colIdx, dt, i, heap);
        }
    } else {
        setData(parsedObj, cols, colIdx, dt, 0, heap);
    }
    vector<string> colNames_(colNames->size());
    for (unsigned int i = 0; i < colNames_.size(); ++i) {
        colNames_[i] = colNames->getString(i);
    }
    return Util::createTable(colNames_, cols);
}

ConstantSP parseCsv(Heap *heap, vector<ConstantSP> &args) {
    VectorSP schema = args[0];
    char delimiter = args[1]->getChar();
    char rowDelimiter = args[2]->getChar();
    string original = args[3]->getString();  // row1 [rowDelimiter] row2 ...

    vector<string> data = Util::split(original, rowDelimiter);
    int nCols = schema->size();
    int nRows = data.size();
    vector<DATA_TYPE> dt(nCols);
    vector<ConstantSP> cols(nCols);

    for (int i = 0; i < nCols; ++i) {
        dt[i] = (DATA_TYPE)schema->getInt(i);
        if (dt[i] == DT_SYMBOL) dt[i] = DT_STRING;
        cols[i] = Util::createVector((DATA_TYPE)schema->getInt(i), nRows, nRows);
    }
    for (int i = 0; i < nRows; ++i) {
        vector<string> raw = Util::split(data[i], delimiter);
        for (int j = 0; j < nCols; ++j) {
            if ((size_t)j >= raw.size() || raw[j].empty()) {
                cols[j]->setNull(i);
                continue;
            }
            try {
                switch (dt[j]) {
                    case DT_BOOL:
                        cols[j]->setBool(i, raw[j][0] == '1');  // true, false, True, False
                        break;
                    case DT_CHAR: {
                        int charData = std::stoi(raw[j]);
                        // set char from the value of ASCII
                        cols[j]->setChar(i, (char)charData);
                    } break;
                    case DT_SHORT:
                        cols[j]->setShort(i, std::stoi(raw[j]));
                        break;
                    case DT_INT:
                        cols[j]->setInt(i, std::stoi(raw[j]));
                        break;
                    case DT_LONG:
                        cols[j]->setLong(i, std::stol(raw[j]));
                        break;
                    case DT_DATE: {
                        static FunctionDefSP const date = Util::getFuncDefFromHeap(heap, "date");
                        ConstantSP d = date->call(heap, new String(raw[j]), void_);
                        cols[j]->setInt(i, d->getInt());
                        break;
                    }
                    case DT_MONTH: {
                        // todo: 2001.01M could not be converted
                        cols[j]->setInt(i, std::stoi(raw[j]));
                        break;
                    }
                    case DT_TIME: {
                        static FunctionDefSP const date = Util::getFuncDefFromHeap(heap, "time");
                        ConstantSP d = date->call(heap, new String(raw[j]), void_);
                        cols[j]->setInt(i, d->getInt());
                        break;
                    }
                    case DT_MINUTE: {
                        static FunctionDefSP const date = Util::getFuncDefFromHeap(heap, "minute");
                        ConstantSP d = date->call(heap, new String(raw[j]), void_);
                        cols[j]->setInt(i, d->getInt());
                        break;
                    }
                    case DT_SECOND: {
                        static FunctionDefSP const date = Util::getFuncDefFromHeap(heap, "second");
                        ConstantSP d = date->call(heap, new String(raw[j]), void_);
                        cols[j]->setInt(i, d->getInt());
                        break;
                    }
                    case DT_DATETIME: {
                        static FunctionDefSP const date = Util::getFuncDefFromHeap(heap, "datetime");
                        ConstantSP d = date->call(heap, new String(raw[j]), void_);
                        cols[j]->setLong(i, d->getLong());
                        break;
                    }
                    case DT_TIMESTAMP: {
                        static FunctionDefSP const date = Util::getFuncDefFromHeap(heap, "timestamp");
                        ConstantSP d = date->call(heap, new String(raw[j]), void_);
                        cols[j]->setLong(i, d->getLong());
                        break;
                    }
                    case DT_NANOTIME: {
                        static FunctionDefSP const date = Util::getFuncDefFromHeap(heap, "nanotime");
                        ConstantSP d = date->call(heap, new String(raw[j]), void_);
                        cols[j]->setLong(i, d->getLong());
                        break;
                    }
                    case DT_NANOTIMESTAMP: {
                        static FunctionDefSP const date = Util::getFuncDefFromHeap(heap, "nanotimestamp");
                        ConstantSP d = date->call(heap, new String(raw[j]), void_);
                        cols[j]->setLong(i, d->getLong());
                        break;
                    }
                    case DT_FLOAT:
                        cols[j]->setFloat(i, std::stof(raw[j]));
                        break;
                    case DT_DOUBLE:
                        cols[j]->setDouble(i, std::stod(raw[j]));
                        break;
                    case DT_SYMBOL:
                    case DT_STRING:
                        cols[j]->setString(i, raw[j]);
                        break;
                    case DT_INT128: {
                        static FunctionDefSP const date = Util::getFuncDefFromHeap(heap, "int128");
                        ConstantSP d = date->call(heap, new String(raw[j]), void_);
                        cols[j]->set(i, d);
                        break;
                    }
                    case DT_UUID: {
                        static FunctionDefSP const date = Util::getFuncDefFromHeap(heap, "uuid");
                        ConstantSP d = date->call(heap, new String(raw[j]), void_);
                        cols[j]->set(i, d);
                        break;
                    }
                    case DT_IP: {
                        static FunctionDefSP const date = Util::getFuncDefFromHeap(heap, "ipaddr");
                        ConstantSP d = date->call(heap, new String(raw[j]), void_);
                        cols[j]->set(i, d);
                        break;
                    }
                    default:
                        PLUGIN_LOG_ERR(LOG_PRE_STR, "the data type of schema in position <", j + 1, "> is ", dt[j],
                                "which is not supported when execute csv parser");
                }
            } catch (exception &e) {
                cols[j]->setNull(i);
                PLUGIN_LOG_ERR(LOG_PRE_STR, "error occured when executed csv parser, err msg is ", string(e.what()));
            }
        }
    }
    vector<string> colNames(nCols);
    for (int i = 0; i < nCols; ++i) colNames[i] = string("c").append(Util::convert(i));
    return Util::createTable(colNames, cols);
}

ConstantSP formatCsv(Heap *heap, vector<ConstantSP> &args) {
    string func = "formatCsv";
    string usage = "formatCsv([format], delimiter=',', rowDelimiter=';', Table)";
    char delimiter = args[1]->getChar();
    char rowDelimiter = args[2]->getChar();
    if (args[3].isNull() || !args[3]->isTable()) {
        throw IllegalArgumentException(func, usage + " the 4th argument must be a table");
    }
    string ret;
    static const FunctionDefSP string = Util::getFuncDefFromHeap(heap, "string");
    static const FunctionDefSP format = Util::getFuncDefFromHeap(heap, "format");

    TableSP t = args[3];
    int nRows = t->getColumn(0)->rows();
    int nCols = t->columns();
    if (args[0]->size() > 0 && args[0]->size() != nCols) {
        throw IllegalArgumentException(
            func, usage + " the size of first argument must be the same as the number of columns of the input table");
    }
    try {
        vector<VectorSP> str(nCols);
        for (int i = 0; i < nCols; ++i) {
            if (args[0]->size() == 0 || args[0]->getString(i).empty()) {
                vector<ConstantSP> args_{t->getColumn(i)};
                str[i] = string->call(heap, args_);
            } else {
                vector<ConstantSP> args_{t->getColumn(i), args[0]->get(i)};
                str[i] = format->call(heap, args_);
            }
        }
        
        vector<char> charBuf(nRows);
        
        for (int row = 0; row < nRows; ++row) {
            for (int col = 0; col < nCols; ++col) {
                std::string rowData = str[col]->get(row)->getString();
                if (t->getColumnType(col) == DT_CHAR) {
                    char *charData;
                    charData = (char *)(t->getColumn(col)->getDataBuffer(0, nRows, charBuf.data()));
                    // transform the char to value of ASCII
                    rowData = std::to_string((int)charData[row]);
                } else if (t->getColumnType(col) == DT_MONTH) {
                    int monthData = t->getColumn(col)->getInt(row);
                    rowData = std::to_string(monthData);
                }
                ret.append(rowData);
                if (LIKELY(col < nCols - 1)) {
                    ret += delimiter;
                }
            }
            ret += rowDelimiter;
        }
    } catch (exception &e) {
        std::string errMsg(e.what());
        throw RuntimeException(LOG_PRE_STR + " err occured when call fortmat csv function, message is <" + errMsg +
                               ">");
    }
    return new String(ret);
}

ConstantSP formatJson(Heap *heap, vector<ConstantSP> &args) {
    using namespace nlohmann;
    string func = "formatJson";
    string usage = "formatJson(Table)";
    if (args[0].isNull() || !args[0]->isTable()) {
        throw IllegalArgumentException(func, usage + " the argument must be a table");
    }
    static const FunctionDefSP string = Util::getFuncDefFromHeap(heap, "string");
    TableSP t = args[0];
    int nRows = t->getColumn(0)->rows();
    int nCols = t->columns();
    json result;
    try {
        vector<VectorSP> str(nCols);
        for (int i = 0; i < nCols; ++i) {
            vector<ConstantSP> args_{t->getColumn(i)};
            str[i] = string->call(heap, args_);
        }
        vector<char> charBuf(nRows);
        for (int row = 0; row < nRows; ++row) {
            json jsonRowData;
            for (int col = 0; col < nCols; ++col) {
                std::string rowData = str[col]->get(row)->getString();
                if (t->getColumnType(col) == DT_CHAR) {
                    char *charData;
                    charData = (char *)(t->getColumn(col)->getDataBuffer(0, nRows, charBuf.data()));
                    // transform the char to value of ASCII
                    rowData = std::to_string((int)charData[row]);
                } else if (t->getColumnType(col) == DT_MONTH) {
                    int monthData = 0;
                    monthData = t->getColumn(col)->getInt(row);
                    rowData = std::to_string(monthData);
                }
                jsonRowData[t->getColumnName(col)] = rowData;
            }
            result.push_back(jsonRowData);
        }
    } catch (exception &e) {
        std::string errMsg(e.what());
        throw RuntimeException(LOG_PRE_STR + " err occured when call fortmat csv function, message is <" + errMsg +
                               ">");
    }
    return new String(result.dump());
}

namespace {
ConstantSP createParser(Heap *heap, vector<ConstantSP> &args, FunctionDefSP def, const string &usage) {
    std::ignore = heap;
    std::ignore = usage;
    FunctionDefSP defNew = Util::createPartialFunction(def, args);
    return defNew;
}
}  // namespace

ConstantSP createJsonParser(Heap *heap, vector<ConstantSP> &args) {
    static FunctionDefSP const def = Util::createSystemFunction("parseJson", parseJson, 3, 3, 0);
    string usage = "createJsonParser(schema, colNames)";
    if (args[1]->getType() != DT_STRING || !args[1]->isVector()) {
        throw IllegalArgumentException(usage, "colNames must be a string vector");
    }
    if ((args[0]->getType() != DT_INT) || (args[0]->size() != args[1]->size())) {
        throw IllegalArgumentException(usage, "the schema should be DATA_TYPE and have same size with colNames.");
    }

    return createParser(heap, args, def, usage);
}

ConstantSP createJsonFormatter(Heap *heap, vector<ConstantSP> &args) {
    std::ignore = heap;
    std::ignore = args;
    static FunctionDefSP const formatter = Util::createSystemFunction("formatJson", formatJson, 1, 1, 0);
    return formatter;
}

ConstantSP createCsvParser(Heap *heap, vector<ConstantSP> &args) {
    string usage = "createCsvParser(schema, [delimiter=','], [rowDelimiter=';']). ";
    char delimiter = ',';
    char rowDelimiter = ';';
    if (args.size() > 1) {
        if (args[1]->getType() != DT_CHAR || args[1]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(usage, "the second argument must be a char");
        }
        delimiter = args[1]->getChar();
    }
    if (args.size() > 2) {
        if (args[2]->getType() != DT_CHAR || args[2]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(usage, "the third argument must be a char");
        }
        rowDelimiter = args[2]->getChar();
    }
    vector<ConstantSP> argsNew{args[0], new Char(delimiter), new Char(rowDelimiter)};
    static FunctionDefSP const def = Util::createSystemFunction("parseCsv", parseCsv, 4, 4, 0);
    return createParser(heap, argsNew, def, usage);
}

ConstantSP createCsvFormatter(Heap *heap, vector<ConstantSP> &args) {
    string usage = "createCsvFormatter(format, [delimiter=','], [rowDelimiter=';']). ";
    char delimiter = ',';
    char rowDelimiter = ';';
    if (!args.empty()) {
        if (args[0]->getType() != DT_VOID && (args[0]->getType() != DT_STRING || !args[0]->isVector())) {
            throw IllegalArgumentException(usage, "the first argument must be a tuple of string ");
        }
    } else {
        throw IllegalArgumentException(usage, "the first argument must be a tuple of string ");
    }

    if (args.size() > 1) {
        if (args[1]->getType() != DT_CHAR || args[1]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(usage, "the second argument must be a char");
        }
        delimiter = args[1]->getChar();
    }
    if (args.size() > 2) {
        if (args[2]->getType() != DT_CHAR || args[2]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(usage, "the third argument must be a char");
        }
        rowDelimiter = args[2]->getChar();
    }
    vector<ConstantSP> argsNew{args[0], new Char(delimiter), new Char(rowDelimiter)};
    static FunctionDefSP const def = Util::createSystemFunction("formatCsv", formatCsv, 4, 4, 0);
    return createParser(heap, argsNew, def, usage);
}
