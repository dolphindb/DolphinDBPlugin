#include "parser.h"
#include "ScalarImp.h"
#include "Util.h"
#include "json.hpp"
#include "Logger.h"
#include "ddbplugin/PluginLogger.h"
#if defined(__GNUC__) && __GNUC__ >= 4
#define LIKELY(x) (__builtin_expect((x), 1))
#define UNLIKELY(x) (__builtin_expect((x), 0))
#else
#define LIKELY(x) (x)
#define UNLIKELY(x) (x)
#endif

static ConstantSP const void_ = Util::createConstant(DT_VOID);
ConstantSP parseJSON(Heap* heap, vector<ConstantSP>& args) {
    using namespace nlohmann;
    VectorSP schema = args[0];
    VectorSP colNames = args[1];
    std::map<string, int> colIdx;
    for (int i = 0; i < colNames->size(); ++i) {
        colIdx.emplace(colNames->getString(i), i);
    }

    string original = args[2]->getString();
    auto parsedObj = json::parse(original);

    if(parsedObj.type_name() != string("array")){
        throw RuntimeException(PLUGIN_ZMQ_PARSERS_PREFIX+"The given JSON data must be a array. ");
    }
    size_t nRows = parsedObj.size();
    if (nRows < 1){
        throw RuntimeException(PLUGIN_ZMQ_PARSERS_PREFIX+"The JSON array must have at least one element");
    }
    size_t nCols = parsedObj.at(0).size();

    vector<DATA_TYPE> dt(nCols);
    vector<ConstantSP> cols(nCols);

    for (size_t i = 0; i < nCols; ++i) {
        dt[i] = (DATA_TYPE)schema->getInt(i);
        if (dt[i] == DT_SYMBOL)
            dt[i] = DT_STRING;
        cols[i] = Util::createVector((DATA_TYPE)schema->getInt(i), nRows, nRows);
    }

    for (size_t i = 0; i < nRows; ++i) {
        auto& row = parsedObj[i];
        for (auto it = row.begin(); it != row.end(); ++it) {
            auto colIdxIter = colIdx.find(it.key());
            if (colIdxIter == colIdx.end()){
                PLUGIN_LOG_ERR(PLUGIN_ZMQ_PARSERS_PREFIX+": The json key["+it.key()+"] does not exist in the table schema");
                continue;
            }
            int curCol = colIdxIter->second;
            //std::cout << curCol << " " << dt[curCol] << " " << it.key() << " " << it.value() << std::endl;
            if(it->is_null()){
                cols[curCol]->setNull(i);
                continue;
            }
            switch (dt[curCol]) {
                case DT_BOOL:                                    // true, false, True, False
                    cols[curCol]->setBool(i, it->get<int>());    // todo: should be bool
                    break;
                case DT_CHAR:
                    cols[curCol]->setChar(i, it->get<string>()[0]);    // todo: should be char
                    break;
                case DT_SHORT:
                    cols[curCol]->setShort(i, it->get<short>());
                    break;
                case DT_INT:
                    cols[curCol]->setInt(i, it->get<int>());
                    break;
                case DT_LONG:
                    cols[curCol]->setLong(i, it->get<long long>());
                    break;
                case DT_DATE: {
                    static FunctionDefSP const date = heap->currentSession()->getFunctionDef("date");
                    ConstantSP d = date->call(heap, new String(it->get<string>()), void_);
                    cols[curCol]->setInt(i, d->getInt());
                    break;
                }
                case DT_MONTH: {
                    static FunctionDefSP const date = heap->currentSession()->getFunctionDef("month");
                    ConstantSP d = date->call(heap, new String(it->get<string>()), void_);
                    cols[curCol]->setInt(i, d->getInt());
                    break;
                }
                case DT_TIME: {
                    static FunctionDefSP const date = heap->currentSession()->getFunctionDef("time");
                    ConstantSP d = date->call(heap, new String(it->get<string>()), void_);
                    cols[curCol]->setInt(i, d->getInt());
                    break;
                }
                case DT_MINUTE: {
                    static FunctionDefSP const date = heap->currentSession()->getFunctionDef("minute");
                    ConstantSP d = date->call(heap, new String(it->get<string>()), void_);
                    cols[curCol]->setInt(i, d->getInt());
                    break;
                }
                case DT_SECOND: {
                    static FunctionDefSP const date = heap->currentSession()->getFunctionDef("second");
                    ConstantSP d = date->call(heap, new String(it->get<string>()), void_);
                    cols[curCol]->setInt(i, d->getInt());
                    break;
                }
                case DT_DATETIME: {
                    static FunctionDefSP const date = heap->currentSession()->getFunctionDef("datetime");
                    ConstantSP d = date->call(heap, new String(it->get<string>()), void_);
                    cols[curCol]->setLong(i, d->getLong());
                    break;
                }
                case DT_TIMESTAMP: {
                    static FunctionDefSP const date = heap->currentSession()->getFunctionDef("timestamp");
                    ConstantSP d = date->call(heap, new String(it->get<string>()), void_);
                    cols[curCol]->setLong(i, d->getLong());
                    break;
                }
                case DT_NANOTIME: {
                    static FunctionDefSP const date = heap->currentSession()->getFunctionDef("nanotime");
                    ConstantSP d = date->call(heap, new String(it->get<string>()), void_);
                    cols[curCol]->setLong(i, d->getLong());
                    break;
                }
                case DT_NANOTIMESTAMP: {
                    static FunctionDefSP const date = heap->currentSession()->getFunctionDef("nanotimestamp");
                    ConstantSP d = date->call(heap, new String(it->get<string>()), void_);
                    cols[curCol]->setLong(i, d->getLong());
                    break;
                }
                case DT_FLOAT:
                    cols[curCol]->setFloat(i, it->get<float>());
                    break;
                case DT_DOUBLE:
                    cols[curCol]->setDouble(i, it->get<double>());
                    break;
                case DT_STRING:
                    cols[curCol]->setString(i, it->get<string>());
                    break;
                case DT_INT128: {
                    static FunctionDefSP const date = heap->currentSession()->getFunctionDef("int128");
                    ConstantSP d = date->call(heap, new String(it->get<string>()), void_);
                    cols[curCol]->set(i, d);
                    break;
                }
                case DT_UUID: {
                    static FunctionDefSP const date = heap->currentSession()->getFunctionDef("uuid");
                    ConstantSP d = date->call(heap, new String(it->get<string>()), void_);
                    cols[curCol]->set(i, d);
                    break;
                }
                case DT_IP: {
                    static FunctionDefSP const date = heap->currentSession()->getFunctionDef("ipaddr");
                    ConstantSP d = date->call(heap, new String(it->get<string>()), void_);
                    cols[curCol]->set(i, d);
                    break;
                }
                default:
                    throw RuntimeException("The schema type in position " + std::to_string(curCol + 1) + "does net support");
            }
        }
    }
    vector<string> colNames_(colNames->size());
    for (unsigned int i = 0; i < colNames_.size(); ++i) {
        colNames_[i] = colNames->getString(i);
    }
    return Util::createTable(colNames_, cols);
}

ConstantSP parseCSV(Heap* heap, vector<ConstantSP>& args) {
    VectorSP schema = args[0];
    char delimiter = args[1]->getChar();
    char rowDelimiter = args[2]->getChar();
    string original = args[3]->getString();    // row1 [rowDelimiter] row2 ...

    vector<string> data = Util::split(original, rowDelimiter);
    size_t nCols = schema->size();
    int nRows = data.size();

    vector<DATA_TYPE> dt(nCols);
    vector<ConstantSP> cols(nCols);

    for (size_t i = 0; i < nCols; ++i) {
        dt[i] = (DATA_TYPE)schema->getInt(i);
        if (dt[i] == DT_SYMBOL)
            dt[i] = DT_STRING;
        cols[i] = Util::createVector((DATA_TYPE)schema->getInt(i), nRows, nRows);
    }
    for (int i = 0; i < nRows; ++i) {
        vector<string> raw = Util::split(data[i], delimiter);
        if(raw.size() < nCols)
            throw RuntimeException(PLUGIN_ZMQ_PARSERS_PREFIX+"The number of elements in row " + std::to_string(i + 1) + " of the original data is less than the number of schemas. ");
        for (size_t j = 0; j < nCols; ++j) {
            switch (dt[j]) {
                case DT_BOOL:                                 // true, false, True, False
                    if(raw[j].size() < 1){
                        throw RuntimeException(PLUGIN_ZMQ_PARSERS_PREFIX+"empty string at row [" + std::to_string(i + 1) + "] column"+std::to_string(j + 1)+"]");
                    }
                    cols[j]->setBool(i, raw[j][0] == '1');    // todo: should be 0 or 1
                    break;
                case DT_CHAR:
                    if(raw[j].size() < 1){
                        throw RuntimeException(PLUGIN_ZMQ_PARSERS_PREFIX+"empty string at row [" + std::to_string(i + 1) + "] column ["+std::to_string(j + 1)+"]");
                    }
                    cols[j]->setChar(i, raw[j][0]);
                    break;
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
                    static FunctionDefSP const date = heap->currentSession()->getFunctionDef("date");
                    ConstantSP d = date->call(heap, new String(raw[j]), void_);
                    cols[j]->setInt(i, d->getInt());
                    break;
                }
                case DT_MONTH: {
                    // todo: 2001.01M could not be converted
                    static FunctionDefSP const date = heap->currentSession()->getFunctionDef("month");
                    ConstantSP d = date->call(heap, new String(raw[j].substr(0, raw[j].size() - 1)), void_);
                    cols[j]->setInt(i, d->getInt());
                    break;
                }
                case DT_TIME: {
                    static FunctionDefSP const date = heap->currentSession()->getFunctionDef("time");
                    ConstantSP d = date->call(heap, new String(raw[j]), void_);
                    cols[j]->setInt(i, d->getInt());
                    break;
                }
                case DT_MINUTE: {
                    static FunctionDefSP const date = heap->currentSession()->getFunctionDef("minute");
                    ConstantSP d = date->call(heap, new String(raw[j]), void_);
                    cols[j]->setInt(i, d->getInt());
                    break;
                }
                case DT_SECOND: {
                    static FunctionDefSP const date = heap->currentSession()->getFunctionDef("second");
                    ConstantSP d = date->call(heap, new String(raw[j]), void_);
                    cols[j]->setInt(i, d->getInt());
                    break;
                }
                case DT_DATETIME: {
                    static FunctionDefSP const date = heap->currentSession()->getFunctionDef("datetime");
                    ConstantSP d = date->call(heap, new String(raw[j]), void_);
                    cols[j]->setLong(i, d->getLong());
                    break;
                }
                case DT_TIMESTAMP: {
                    static FunctionDefSP const date = heap->currentSession()->getFunctionDef("timestamp");
                    ConstantSP d = date->call(heap, new String(raw[j]), void_);
                    cols[j]->setLong(i, d->getLong());
                    break;
                }
                case DT_NANOTIME: {
                    static FunctionDefSP const date = heap->currentSession()->getFunctionDef("nanotime");
                    ConstantSP d = date->call(heap, new String(raw[j]), void_);
                    cols[j]->setLong(i, d->getLong());
                    break;
                }
                case DT_NANOTIMESTAMP: {
                    static FunctionDefSP const date = heap->currentSession()->getFunctionDef("nanotimestamp");
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
                case DT_STRING:
                    cols[j]->setString(i, raw[j]);
                    break;
                case DT_INT128: {
                    static FunctionDefSP const date = heap->currentSession()->getFunctionDef("int128");
                    ConstantSP d = date->call(heap, new String(raw[j]), void_);
                    cols[j]->setLong(i, d->getLong());
                    break;
                }
                case DT_UUID: {
                    static FunctionDefSP const date = heap->currentSession()->getFunctionDef("uuid");
                    ConstantSP d = date->call(heap, new String(raw[j]), void_);
                    cols[j]->setLong(i, d->getLong());
                    break;
                }
                case DT_IP: {
                    static FunctionDefSP const date = heap->currentSession()->getFunctionDef("ipaddr");
                    ConstantSP d = date->call(heap, new String(raw[j]), void_);
                    cols[j]->setLong(i, d->getLong());
                    break;
                }
                default:
                    throw RuntimeException("The schema type in position " + std::to_string(j + 1) + "does net support");
            }
        }
    }
    vector<string> colNames(nCols);
    for (size_t i = 0; i < nCols; ++i)
        colNames[i] = string("c").append(Util::convert(i));
    return Util::createTable(colNames, cols);
}

ConstantSP formatCSV(Heap* heap, vector<ConstantSP>& args) {
    string func = "formatCSV";
    string usage = "formatCSV([format], delimiter=',', rowDelimiter=';', Table)";
    char delimiter = args[1]->getChar();
    char rowDelimiter = args[2]->getChar();
    if (!args[3]->isTable()) {
        throw IllegalArgumentException(func, usage + " the 4th argument must be a table");
    }
    string ret;

    static const FunctionDefSP string = heap->currentSession()->getFunctionDef("string");
    static const FunctionDefSP format = heap->currentSession()->getFunctionDef("format");

    TableSP t = args[3];
    int nRows = t->getColumn(0)->rows();
    int nCols = t->columns();
    if (args[0]->size() > 0 && args[0]->size() != nCols) {
        throw IllegalArgumentException(
                func, usage + " the size of first argument must be the same as the number of columns of the input table");
    }
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

    for (int row = 0; row < nRows; ++row) {
        for (int col = 0; col < nCols; ++col) {
            ret.append(str[col]->get(row)->getString());
            if (LIKELY(col < nCols - 1)) {
                ret += delimiter;
            }
        }
        ret += rowDelimiter;
    }
    return new String(ret);
}

namespace {
    ConstantSP createParser(Heap* heap, vector<ConstantSP>& args, FunctionDefSP def, const string& usage) {
        //    if (!args[0]->isVector() || args[0]->getType() != DT_INT) {
        //        throw IllegalArgumentException(usage, "schema must be a vector of data types");
        //    }
        //    VectorSP schema = args[0];
        //    for (decltype(schema->size()) i = 0; i < schema->size(); ++i) {
        //        if (schema->getInt(i) < DT_BOOL || schema->getInt(i) > DT_STRING) {
        //            throw IllegalArgumentException(usage, "the " + Util::convert(i) + " data type is not valid");
        //        }
        //    }
        FunctionDefSP defNew = Util::createPartialFunction(def, args);
        return defNew;
    }
}    // namespace

ConstantSP createJSONParser(Heap* heap, vector<ConstantSP>& args) {
    static FunctionDefSP const def = Util::createSystemFunction("parseJSON", parseJSON, 3, 3, 0);
    string usage = "createJSONParser(schema, colNames)";
    if (args[0]->getType() != DT_INT || args[0]->getForm() != DF_VECTOR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "schema must be a vector of types!");
    }
    if (args[1]->getType() != DT_STRING || !args[1]->isVector()) {
        throw IllegalArgumentException(usage, "colNames must be a string vector");
    }
    if (args[0]->size() != args[1]->size()) {
        throw IllegalArgumentException(usage, "schema and colNames must have the same size.");
    }
    return createParser(heap, args, def, usage);
}

ConstantSP createJSONFormatter(Heap* heap, vector<ConstantSP>& args) {
    static FunctionDefSP const formatter = heap->currentSession()->getFunctionDef("toStdJson");
    return formatter;
}

ConstantSP createCSVParser(Heap* heap, vector<ConstantSP>& args) {
    // p = createCSVParser([INT, DOUBLE, STRING], ',', ';' )
    // p("1,2,3;4,5,6")
    string usage = "createCSVParser(schema, [delimiter=','], [rowDelimiter=';']). ";
    char delimiter = ',';
    char rowDelimiter = ';';
    if (args[0]->getType() != DT_INT || args[0]->getForm() != DF_VECTOR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "schema must be a vector of types!");
    }
    if (args.size() > 1) {
        if (args[1]->getType() != DT_CHAR || args[1]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(usage, "delimiter must be a char");
        }
        delimiter = args[1]->getChar();
    }
    if (args.size() > 2) {
        if (args[2]->getType() != DT_CHAR || args[2]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(usage, "rowDelimiter must be a char");
        }
        rowDelimiter = args[2]->getChar();
    }
    vector<ConstantSP> argsNew{args[0], new Char(delimiter), new Char(rowDelimiter)};
    static FunctionDefSP const def = Util::createSystemFunction("parseCSV", parseCSV, 4, 4, 0);
    return createParser(heap, argsNew, def, usage);
}

ConstantSP createCSVFormatter(Heap* heap, vector<ConstantSP>& args) {
    string usage = "createCSVFormatter(format, [delimiter=','], [rowDelimiter=';']). ";
    char delimiter = ',';
    char rowDelimiter = ';';
    if(args[0]->getType() != DT_STRING || !args[0]->isVector()) {
        throw IllegalArgumentException(usage, "format must be a tuple of string ");
    }

    if (args.size() > 1) {
        if (args[1]->getType() != DT_CHAR || args[1]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(usage, "delimiter must be a char"  );
        }
        delimiter = args[1]->getChar();
    }
    if (args.size() > 2) {
        if (args[2]->getType() != DT_CHAR || args[2]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(usage, "rowDelimiter must be a char");
        }
        rowDelimiter = args[2]->getChar();
    }
    vector<ConstantSP> argsNew{args[0], new Char(delimiter), new Char(rowDelimiter)};
    static FunctionDefSP const def = Util::createSystemFunction("formatCSV", formatCSV, 4, 4, 0);
    return createParser(heap, argsNew, def, usage);
}
