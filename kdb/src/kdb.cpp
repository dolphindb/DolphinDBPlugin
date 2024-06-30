#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <string>
#include <exception>
#include <vector>
#include <iostream>
#include <map>
#include <ctime>
#include <zlib.h>

#include "Logger.h"
#include "Concurrent.h"
#include "Exceptions.h"
#include "ScalarImp.h"
#include "SpecialConstant.h"
#include "Util.h"

#include "kdb.h"
#include "q2ddb.h"
#include "qfile.h"

using namespace std;

//////////////////////////////////////////////////////////////////////
// Constants

const char PATH_SEP = '/';

// kdb+ IPC parameters
const int KDB_TIMEOUT = 1 * 1000;   // 1 sec
const int KDB_CAPABILITY = 1;       // 1 TB limit

enum KDB_IPC_error: int {
    KDB_IPC_AUTH_ERROR     =  0,
    KDB_IPC_CONN_ERROR     = -1,
    KDB_IPC_TIMEOUT_ERROR  = -2,
    KDB_IPC_SSL_INIT_ERROR = -3,
};

//////////////////////////////////////////////////////////////////////
// DolphinDB Inter-ops
namespace /*anonymous*/ {

    Mutex LOCK_KDB;

    template<typename Fun>
    void SingleThread(Fun&& fun) {
        LockGuard<Mutex> guard(&LOCK_KDB);
        fun();
    }

    int arg2Int(const ConstantSP& arg, const char* argName = nullptr,
        const string& usage = "", const char* caller = nullptr
    ) {
        assert(arg.get());
        if(arg->getType() != DT_INT || arg->getForm() != DF_SCALAR) {
            const auto ref = caller  ? caller  : __FUNCTION__;
            const auto var = argName ? argName : "Arg";
            throw IllegalArgumentException(ref,
                usage + PLUGIN_NAME ": " + var + " should be an integer scalar.");
        }
        return arg->getInt();
    }

    string arg2String(const ConstantSP& arg, const char* argName = nullptr,
        const string& usage = "", const char* caller = nullptr
    ) {
        assert(arg.get());
        if(arg->getType() != DT_STRING || arg->getForm() != DF_SCALAR) {
            const auto ref = caller  ? caller  : __FUNCTION__;
            const auto var = argName ? argName : "Arg";
            throw IllegalArgumentException(ref,
                usage + PLUGIN_NAME ": " + var + " should be a string scalar.");
        }
        return arg->getString();
    }

    Connection* arg2Connection(const ConstantSP& arg,
        const string& usage = "", const char* caller = nullptr
    ) {
        assert(arg.get());
        const auto ref = caller ? caller : __FUNCTION__;
        if(arg->getType() != DT_RESOURCE) {
            throw IllegalArgumentException(ref,
                usage + PLUGIN_NAME ": Invalid connection object.");
        }
        const string desc = arg->getString();
        if(desc.find(Connection::MARKER) == desc.npos) {
            throw IllegalArgumentException(ref,
                usage + PLUGIN_NAME ": Invalid kdb+ connection object.");
        }
        Connection* conn = reinterpret_cast<Connection*>(arg->getLong());
        return conn;
    }

    string& normalizePath(string& path) {
#       ifdef WINDOWS
        // Replace backward slash with forward slash
        path = Util::replace(path, '\\', PATH_SEP);
#       endif
        while(!path.empty() && path.back() == PATH_SEP) {
            path.pop_back();
        }
        return path;
    }

    ConstantSP safeOp(const ConstantSP &arg,
        std::function<ConstantSP(Connection *)> &&fun
    ) {
        Connection* conn = arg2Connection(arg, "", __FUNCTION__);
        if(!conn) {
            throw IllegalArgumentException(__FUNCTION__,
                PLUGIN_NAME ": Connection object already closed.");
        }
        ConstantSP result;
        SingleThread([&](){ result = fun(conn); });
        return result;
    }

    void kdbConnectionOnClose(Heap *heap, vector<ConstantSP> &args) {
        assert(args.size() >= 1);
        try {
            // Use unique_ptr<> to manage conn until it is reset.
            unique_ptr<Connection> conn{reinterpret_cast<Connection*>(args[0]->getLong())};
            if(conn) {
                args[0]->setLong(0);
            }
        }
        catch(const TraceableException& te) {
            LOG_ERR(PLUGIN_NAME ": "
                + string{__FUNCTION__} + " error: " + te.what());
        }
    }

}//namespace /*anonymouse*/

//////////////////////////////////////////////////////////////////////
// Class Implementation

Connection::Connection(
    const string& hostStr, const int port, const string& usernamePassword)
  : host_{hostStr}, port_{port}, handle_{0}
{
    const auto handle = khpunc(
        kdb::sym(hostStr), port, kdb::sym(usernamePassword),
        KDB_TIMEOUT, KDB_CAPABILITY);
    switch(handle) {
        case KDB_IPC_AUTH_ERROR:
            throw RuntimeException(PLUGIN_NAME ": Authentication error.");
        case KDB_IPC_CONN_ERROR:
            throw RuntimeException(PLUGIN_NAME ": Connection error.");
        case KDB_IPC_TIMEOUT_ERROR:
            throw RuntimeException(PLUGIN_NAME ": Connection time out.");
        default:
            if(handle < 0) {
                throw RuntimeException(PLUGIN_NAME ": q-IPC error.");
            }
    }
    assert(handle > 0);
    handle_ = handle;
    LOG(PLUGIN_NAME ": Connection `:" + str() + " opened.");
}

Connection::~Connection() {
    if(handle_) {
        assert(handle_ > 0);
        LOG(PLUGIN_NAME ": Connection `:" + str() + " closed.");
        kclose(handle_);
    }
}

KPtr Connection::kExec(const string& command) const {
    KPtr res{ k(handle_, kdb::sym(command), nullptr) };
    if(!res) {
        throw RuntimeException(PLUGIN_NAME ": "
            "kdb+ network error: " + command + '(' + strerror(errno) + ").");
    } else
    if(res->t == kdb::K_ERROR) {
        const string errMsg = res->s;
        throw RuntimeException(PLUGIN_NAME ": "
            "kdb+ execution error: " + command + "('" + errMsg + ").");
    }
    return res;
}

// must use the file name to load sym in kdb+
// otherwise the enumerated data could not find its sym list
string Connection::loadSymFile(const string& symFilePath) const {
    if(symFilePath.empty()) {
        return "";
    }

    const auto fields = Util::split(symFilePath, PATH_SEP);
    if(fields.empty()) {
        throw RuntimeException(PLUGIN_NAME ": Invalid symPath.");
    }
    const string symName = fields.back();

    // load sym to kdb
    const string command = symName + R"(:get hsym`$")" + symFilePath + '"';
    kExec(command);

    LOG(PLUGIN_NAME ": Loaded enum sym `" + symName + " in kdb+");
    return symName;
}

ConstantSP Connection::loadColumn(
    const string& tableName, const string& colName
) const {
    assert(!tableName.empty());
    if(colName.empty()) {
        throw RuntimeException(PLUGIN_NAME ": invalid column name.");
    }

    const string queryCommand = tableName + "`" + colName;
    KPtr colRes{ kExec(queryCommand) };
    if(!kdb::isValidList(colRes.get())) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Failed to load column " + tableName + "." + colName + ".");
    }

    auto colVal = kdb::toDDB::fromK(colRes.get(), colName);
    auto col = dynamic_cast<Vector*>(colVal.get());
    assert(col);
    col->setNullFlag(col->hasNull());
    if(!col->size() && col->getType() == DT_ANY) {
        kdb::fakeEmptyAnyColumn(col, tableName, colName);
    }

    LOG(PLUGIN_NAME ": Loaded column " + tableName + "." + colName
        + " (" + to_string(static_cast<kdb::Type>(colRes->t))
        + "->" + to_string(col->getType()) + ") "
          "size=" + to_string(col->size()));
    return colVal;
}

string getUniqueTableName() {
    Guid guid = Guid(true);
    vector<string> guidVec = Util::split(guid.getString(), '-');
    string uniqueTableName = "dolphindb";
    for (string &s: guidVec) {
        uniqueTableName += s;
    }
    return uniqueTableName;
}

ConstantSP Connection::extractSchema(const std::string& tablePath, const std::string &symPath) const {
    loadSymFile(symPath);
    string uniqueTableName = getUniqueTableName();
    string mapLine = uniqueTableName + ":get `:" + tablePath;
    kExec(mapLine);
    string countLine = "count " + uniqueTableName;
    ConstantSP kExecRet = execute(countLine);
    long long count = kExecRet->getLong();

    // HACK if count=0, return table, if > 0 return table[0 til 1]
    TableSP dummyTable;
    if (count == 0) {
        // BUG empty execute would have problem and other problem dummyTable = execute(uniqueTableName);
        dummyTable = getTable(tablePath, symPath);
    } else {
        string dummyLine = uniqueTableName + "[0 + til 1]";
        dummyTable = execute(dummyLine);
    }
    // clean job for kdb env
    string cleanCommand = uniqueTableName + ":0";
    kExec(cleanCommand);

    int colNums = dummyTable->columns();
    VectorSP names = Util::createVector(DT_STRING, 0, colNums);
    VectorSP typeString = Util::createVector(DT_STRING, 0, colNums);
    VectorSP typeInt = Util::createVector(DT_INT, 0, colNums);

    for (int i = 0; i < dummyTable->columns(); ++i) {
        string name = dummyTable->getColumnName(i);
        DATA_TYPE colType = dummyTable->getColumnType(i);
        string typeStr = Util::getDataTypeString(colType);
        int colTypeInt = colType;
        names->appendString(&name, 1);
        typeString->appendString(&typeStr, 1);
        typeInt->appendInt(&colTypeInt, 1);
    }
    vector<string> colNames{"name", "typeString", "typeInt"};
    vector<ConstantSP> cols{names, typeString, typeInt};
    return Util::createTable(colNames, cols);
}

ConstantSP Connection::execute(const std::string& qScript) const {
    KPtr data = kExec(qScript);
    return kdb::toDDB::fromK(data.get(), qScript);
}

ConstantSP Connection::loadTableEx(Heap *heap, ConstantSP dbHandle, ConstantSP tableName, ConstantSP partitionColumns, ConstantSP schema, long long batchSize, FunctionDefSP transform, ConstantSP sortColumns, const string &pathOrScript, const string &symPath) {
    // if pathOrScript
    string keyCommand = "key`:" + pathOrScript;
    ConstantSP kExecRet;
    try {
        kExecRet = execute(keyCommand);
    } catch (...) {
        LOG_WARN(PLUGIN_NAME, "failed to execute '", keyCommand, "' in kdb+");
    }

    // if (schema.isNull() && !kExecRet->isNull()) {
    //     extractSchema(pathOrScript, symPath);
    // }

    DatabaseUpdater dbUpdater = DatabaseUpdater(heap, dbHandle, tableName, partitionColumns, sortColumns, schema, transform);
    if (!kExecRet->isNull()) {
        LOG_INFO(PLUGIN_NAME, "load table from tablePath '", pathOrScript, "'");
        loadSymFile(symPath);
        string uniqueTableName = getUniqueTableName();
        string mapLine = uniqueTableName + ":get `:" + pathOrScript;
        kExecRet = execute(mapLine);
        string countLine = "count " + uniqueTableName;
        kExecRet = execute(countLine);
        long long count = kExecRet->getLong();
        long long cursor = 0;
        while (cursor < count) {
            long long size = count - cursor > batchSize ? batchSize: count - cursor;
            string batchLine = uniqueTableName + "[" + std::to_string(cursor) + " + til " + std::to_string(size) + "]";
            kExecRet = execute(batchLine);
            dbUpdater.append(kExecRet);
            cursor += size;
        }
        // clean job for kdb env
        string cleanCommand = uniqueTableName + ":0";
        kExec(cleanCommand);
    } else {
        LOG_INFO(PLUGIN_NAME, "load table from script '",pathOrScript, "' result");
        // script insert
        // FIXME may decrease use of memory
        ConstantSP retData = execute(pathOrScript);
        dbUpdater.append(retData);
    }
    vector<ConstantSP> loadTableArgs{dbHandle, tableName};
    return heap->currentSession()->getFunctionDef("loadTable")->call(heap, loadTableArgs);
}

TableSP Connection::getTable(
    const string& tablePath, const string& symFilePath
) const {
    // load symbol
    const string symName = loadSymFile(symFilePath);

    // load table
    const string loadCommand = R"(\l )" + tablePath;
    kExec(loadCommand);

    // split table path, get table name, get cols
    const auto pathVec = Util::split(tablePath, PATH_SEP);
    if(pathVec.empty()) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Invalid file path " + tablePath + ".");
    }
    const string tableName = pathVec.back();
    const string colsCommand = "cols " + tableName;
    KPtr colsRes = kExec(colsCommand);
    if(!kdb::isValidListOf(colsRes.get(), kdb::K_STRING)) {
        throw RuntimeException(PLUGIN_NAME ": Failed to get table cols.");
    }

    // load each column
    const size_t colNum = static_cast<size_t>(colsRes->n);
    vector<string> colNames(colNum);
    transform(kS(colsRes.get()), kS(colsRes.get()) + colNum, colNames.begin(),
        static_cast<S(*)(const string&)>(&kdb::sym)
    );

    vector<ConstantSP> cols(colNum);
    transform(colNames.cbegin(), colNames.cend(), cols.begin(),
        [&](const string& colName) { return loadColumn(tableName, colName); }
    );

    // drop table & sym, release memory in kdb+
    string dropCommand = "![`.;();0b;] (),`" + tableName;
    if(!symName.empty()) {
        dropCommand += "`" + symName;
    }
    KPtr dropRes{ kExec(dropCommand) };

    // create table in DolphinDB
    return Util::createTable(colNames, cols);
}

string Connection::str() const {
    return host_ + ":" + to_string(port_);
}

//////////////////////////////////////////////////////////////////////
// Load kdb+ splayed tables directly from directory

vector<string> loadSymList(const string& symPath) {
    kdb::BinFile symFile{symPath, symPath};
    if(!symFile) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Open sym file " + symPath + " failed.");
    }

    kdb::Parser parser;
    symFile.readInto(parser.getBuffer());
    const auto symList = parser.getStrings(symPath);

    LOG(PLUGIN_NAME ": Loaded enum sym from " + symPath + " "
        "size=" + to_string(symList.size()));
    return symList;
}

VectorSP loadSplayedColumn(const string& colPath,
    const vector<string>& symList, const string& symName
) {
    kdb::BinFile colFile{colPath, colPath};
    if(!colFile) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Open column " + colPath + " failed.");
    }

    kdb::Parser parser;
    colFile.readInto(parser.getBuffer());
    VectorSP col = parser.getVector(colPath, symList, symName);
    assert(!col.isNull());

    LOG(PLUGIN_NAME ": Loaded splayed column from " + colPath + " "
        + "type=" + to_string(col->getType()) + " "
          "size=" + to_string(col->size()));
    col->setNullFlag(col->hasNull());
    col->setTemporary(true);
    return col;
}

#define KDB_READ_PARALLEL 1
#if KDB_READ_PARALLEL
class SplayedColumnLoader: public Runnable {
public:
    SplayedColumnLoader(ConstantSP& col, const string& colPath,
        const vector<string>& symList, const string& symName)
      : colPath_{colPath}, symName_{symName}, symList_{symList}, col_{col}
    {}

    void run() override {
        try {
            col_.get() = loadSplayedColumn(colPath_, symList_.get(), symName_);
        }
        catch(const RuntimeException& rex) {
            col_.get() = new String{rex.what()};
        }
        catch(const exception& ex) {
            LOG_ERR(PLUGIN_NAME ": "
                "<BUG> failed to load kdb+ column file"
                " " + colPath_ + ": " + ex.what());
        }
        catch(...) {
            LOG_ERR(PLUGIN_NAME ": "
                "<BUG> unknown error while loading kdb+ column file"
                " " + colPath_ + ".");
        }
    }

private:
    string colPath_;
    string symName_;
    reference_wrapper<const vector<string>> symList_;
    reference_wrapper<ConstantSP> col_;
};

using SplayedColumnLoaderSP = SmartPointer<SplayedColumnLoader>;

#endif//KDB_READ_PARALLEL

TableSP extractFileSchema(const string &tablePath, const string &symPath) {
}

//@see https://code.kx.com/q/database/object/
//@see https://code.kx.com/q/kb/splayed-tables/
TableSP loadSplayedTable(string tablePath,
    vector<string>& symList, string symName
) {
    if(!tablePath.empty() && tablePath.back() != PATH_SEP) {
        tablePath.push_back(PATH_SEP);
    }

    // Read .d file, get column names
    const string dotD = tablePath + ".d";
    vector<string> colNames = loadSymList(dotD);

    const size_t colNum = colNames.size();
    vector<ConstantSP> cols;
    cols.reserve(colNum);

#if KDB_READ_PARALLEL
    vector<ThreadSP> colThreads;
    colThreads.reserve(colNum);
    cols.resize(colNum);
    transform(colNames.cbegin(), colNames.cend(), cols.begin(),
        back_inserter(colThreads),
        [&](const string& colName, ConstantSP& col) {
            return new Thread{SplayedColumnLoaderSP{
                new SplayedColumnLoader{
                    col, tablePath + colName, symList, symName
                }
            }};
        }
    );
    assert(cols.size() == colNum && colThreads.size() == colNum);

    for(auto thread : colThreads) {
        if(!thread->isStarted()) {
            thread->start();
        }
    }
    for(auto thread : colThreads) {
        thread->join();
    }
#else//KDB_READ_PARALLEL
    transform(colNames.cbegin(), colNames.cend(), back_inserter(cols),
        [&tablePath, &symList, &symName](const string& colName) {
            return loadSplayedColumn(tablePath + colName, symList, symName);
        }
    );
#endif//KDB_READ_PARALLEL

    for(auto i = 0u; i < colNum; ++i) {
        if(cols[i].isNull()) {
            throw RuntimeException(PLUGIN_NAME ": "
                "<BUG> unexpected error while loading "
                " " + tablePath + colNames[i] + ","
                " check debug log for details.");
        } else
        if(cols[i]->getForm() != DF_VECTOR) {
            if(cols[i]->getType() == DT_STRING) {
                throw RuntimeException(cols[i]->getString());
            }
            else {
                throw RuntimeException(PLUGIN_NAME ": "
                    "<BUG> unexpected data loaded from"
                    " " + tablePath + colNames[i] + ":"
                    " " + cols[i]->getString());
            }
        }
    }
    return Util::createTable(colNames, cols);
}

//////////////////////////////////////////////////////////////////////////////
// DolphinDB Plugin API

ConstantSP kdbConnect(Heap *heap, vector<ConstantSP> &args){
    const string usage = "Usage: connect(host, port, usernamePassword). ";
    assert(args.size() >= 3-1);

    const string hostStr = arg2String(args[0], "host", usage, __FUNCTION__);
    const int port = arg2Int(args[1], "port", usage, __FUNCTION__);

    string usrStr = "";
    if(args.size() >= 3) {
        usrStr = arg2String(args[2], "usernamePassword", usage, __FUNCTION__);
    }

    // Use unique_ptr<> to manage cup until Util::createResource() takes over.
    unique_ptr<Connection> cup;
    SingleThread([&](){
        cup = make_unique<Connection>(hostStr, port, usrStr);
    });

    const string desc = Connection::MARKER + (" to [" + cup->str() + "]");
    FunctionDefSP onClose{
        Util::createSystemProcedure(
            "kdb+ connection onClose()", &kdbConnectionOnClose, 1, 1)
    };
    // FIXME: Still not quite safe!
    //  If Util::createResource throws, cup will be dangling forever.
    static_assert(
        sizeof(long long) >= sizeof(Connection*),
        "ensure enough space to store the pointer"
    );
    return Util::createResource(reinterpret_cast<long long>(cup.release()),
        desc, onClose, heap->currentSession());
}

ConstantSP kdbLoadTable(Heap *heap, vector<ConstantSP> &args){
    const string usage = "Usage: loadTable(handle, tablePath, symPath). ";
    assert(args.size() >= 3-1);

    string tablePath = arg2String(args[1], "tablePath", usage, __FUNCTION__);
    tablePath = normalizePath(tablePath);

    string symFilePath = "";
    if(args.size() >= 3) {
        symFilePath = arg2String(args[2], "symPath", usage, __FUNCTION__);
    }
    symFilePath = normalizePath(symFilePath);

    return safeOp(args[0], [&](Connection *conn){
        return conn->getTable(tablePath, symFilePath);
    });
}

ConstantSP kdbLoadTableEx(Heap *heap, vector<ConstantSP> &arguments) {
    string syntax("loadTableEx(dbHandle, tableName, partitionColumns, handle,"
                  "tablePath|qScript, [symPath], [schema], [batchSize=0], "
                  "[transform], [sortColumns])");

    ConstantSP dbHandle = arguments[0];
    ConstantSP tableName = arguments[1];
    ConstantSP partitionColumns = arguments[2];
    ConstantSP schema;
    long long batchSize = 0;
    FunctionDefSP transform;
    ConstantSP sortColumns;

    if (!dbHandle->isDatabase()) {
        throw IllegalArgumentException(__FUNCTION__,
                                       syntax +
                                       "dbHandle must be a database handle.");
    }
    if (tableName->getType() != DT_STRING ||
        tableName->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__,
                                       syntax +
                                       "tableName must be a string scalar.");
    }
    if (!partitionColumns->isNull() &&
        (partitionColumns->getType() != DT_STRING ||
        !(partitionColumns->isVector() || partitionColumns->isScalar()))) {
        throw IllegalArgumentException(__FUNCTION__, syntax +
             "The partition columns must be string scalar or string vector.");
    }

    ConstantSP handle = arguments[3];

    if (arguments[4]->getType() != DT_STRING ||
        arguments[4]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, syntax + "tablePath|qScript must be a string scalar.");
    }
    string pathOrScript = arguments[4]->getString();

    string symPath;
    if (arguments.size() > 5 && !arguments[5]->isNull()) {
        if (arguments[5]->getType() != DT_STRING || arguments[5]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, syntax + "symPath must be a string scalar.");
        }
        symPath = arguments[5]->getString();
    }
    if (arguments.size() > 6 && !arguments[6]->isNull()) {
        if (arguments[6]->getForm() == DF_TABLE) {
            schema = arguments[6];
            string errMsg =
                "schema must be a table with at least two columns. First column is name, second column is typeString.";
            if (schema->columns() < 2) {
                throw IllegalArgumentException(__FUNCTION__, syntax + errMsg);
            }
            if (((TableSP)schema)->getColumnName(0) != "name" || (((TableSP)schema)->getColumnName(1) != "typeString")) {
                throw IllegalArgumentException(__FUNCTION__, syntax + errMsg);
            }
            if (((TableSP)schema)->getColumnType(0) != DT_STRING || ((TableSP)schema)->getColumnType(1) != DT_STRING) {
                throw IllegalArgumentException(__FUNCTION__, syntax + "first two schema columns' type must be string.");
            }
        } else {
            throw IllegalArgumentException(__FUNCTION__, syntax + "schema must be a table with at least two columns. ");
        }
    }
    if (arguments.size() > 7 && !arguments[7]->isNull()) {
        if (arguments[7]->getCategory() != INTEGRAL || arguments[7]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, syntax + "batchSize must be an integer scalar.");
        }
        batchSize = arguments[7]->getLong();
        if (batchSize < 0) {
            throw IllegalArgumentException(__FUNCTION__, syntax + "batchSize must be an positive integer scalar.");
        }
    }
    if (arguments.size() > 8 && !arguments[8]->isNull()) {
        if (arguments[8]->getType() != DT_FUNCTIONDEF || (FunctionDefSP(arguments[8]))->getParamCount() != 1) {
            throw IllegalArgumentException(__FUNCTION__, syntax + "transform must be a function with one parameter.");
        }
        transform = arguments[8];
    }
    if (arguments.size() > 9 && !arguments[9]->isNull()) {
        if (((SystemHandleSP)dbHandle)->getDomain()->getEngineType() != DBENGINE_TYPE::IOT) {
            throw IllegalArgumentException(__FUNCTION__, syntax + "sortColumns only support tsdb dbHandle.");
        }
        if (arguments[9]->getType() != DT_STRING || (!arguments[9]->isVector() && !arguments[9]->isScalar())) {
            sortColumns = arguments[9];
        }
    }

    return safeOp(handle, [&](Connection *conn){
        return conn->loadTableEx(heap, dbHandle, tableName, partitionColumns, schema, batchSize, transform, sortColumns, pathOrScript, symPath);
    });
}

ConstantSP kdbExtractFileSchema(Heap *heap, vector<ConstantSP> &args) {
    const string usage = "Usage: extractFileSchema(tablePath, [symPath]). ";

    string tablePath = arg2String(args[0], "tablePath", usage, __FUNCTION__);

    // TODO find out why normalize is required
    tablePath = normalizePath(tablePath);
    string symPath;
    if (args.size() > 1 && !args[1]->isNull()) {
        symPath = arg2String(args[1], "symPath", usage, __FUNCTION__);
        symPath = normalizePath(symPath);
    }

    return extractFileSchema(tablePath, symPath);
}

ConstantSP kdbExtractTableSchema(Heap *heap, vector<ConstantSP> &args) {
    const string usage = "Usage: extractTableSchema(handle, tablePath, [symPath]). ";

    string tablePath = arg2String(args[1], "tablePath", usage, __FUNCTION__);
    tablePath = normalizePath(tablePath);
    string symPath;
    if (args.size() > 2 && !args[2]->isNull()) {
        symPath = arg2String(args[2], "symPath", usage, __FUNCTION__);
        symPath = normalizePath(symPath);
    }

    return safeOp(args[0], [&](Connection *conn){
        return conn->extractSchema(tablePath, symPath);
    });
}

ConstantSP kdbExecute(Heap *heap, vector<ConstantSP> &args) {
    const string usage = "Usage: execute(handle, qScript). ";
    assert(args.size() >= 3-1);

    string qScript = arg2String(args[1], "qScript", usage, __FUNCTION__);

    return safeOp(args[0], [&](Connection *conn){
        return conn->execute(qScript);
    });
}

ConstantSP kdbLoadFile(Heap *heap, vector<ConstantSP> &args){
    const auto usage = string("Usage: loadFile(tablePath, symPath). ");
    assert(args.size() >= 2-1);

    string tablePath = arg2String(args[0], "tablePath", usage, __FUNCTION__);
    tablePath = normalizePath(tablePath);
    if(!(Util::exists(tablePath) || Util::existsDir(tablePath))) {
        throw IllegalArgumentException(__FUNCTION__,
            usage + PLUGIN_NAME ": "
            "tablePath [" + tablePath + "] does not exist.");
    }

    string symFilePath = "";
    if(args.size() >= 2) {
        symFilePath = arg2String(args[1], "symPath", usage, __FUNCTION__);
    }
    symFilePath = normalizePath(symFilePath);
    if(!(symFilePath.empty() || Util::exists(symFilePath))) {
        const char* extra = Util::existsDir(symFilePath)
            ? "should be a file, not a directory"
            : "does not exist";
        throw IllegalArgumentException(__FUNCTION__,
            usage + PLUGIN_NAME ": "
            "symPath [" + symFilePath + "] " + extra + '.');
    }

    vector<string> symList;
    if(!symFilePath.empty()) {
        symList = loadSymList(symFilePath);
    }

    vector<string> fields;
    Util::split(symFilePath, PATH_SEP, fields);
    string symName = "";
    if(!fields.empty()) {
        symName = fields.back();
    }

    return loadSplayedTable(tablePath, symList, symName);
}

ConstantSP kdbClose(Heap *heap, vector<ConstantSP> &args) {
    const string usage = "Usage: close(handle). ";
    assert(args.size() >= 1);

    // Use unique_ptr<> to manage cup until reset.
    unique_ptr<Connection> cup{
        arg2Connection(args[0], usage, __FUNCTION__)
    };
    if(cup) {
        SingleThread([&cup](){ cup.reset(); });
        args[0]->setLong(0);
    }
    return new Void();
}

ConstantSP kdbLoadFileEx(Heap *heap, vector<ConstantSP> &arguments) {

    string syntax("loadFileEx(dbHandle, tableName, partitionColumns,"
                  "tablePath, [symPath], [schema], [batchSize=0], "
                  "[transform], [sortColumns]) ");

    ConstantSP dbHandle = arguments[0];
    ConstantSP tableName = arguments[1];
    ConstantSP partitionColumns = arguments[2];
    ConstantSP schema;
    long long batchSize = 0;
    FunctionDefSP transform;
    ConstantSP sortColumns;

    if (!dbHandle->isDatabase()) {
        throw IllegalArgumentException(__FUNCTION__,
                                       syntax +
                                       "dbHandle must be a database handle.");
    }
    if (tableName->getType() != DT_STRING ||
        tableName->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__,
                                       syntax +
                                       "tableName must be a string scalar.");
    }
    if (!partitionColumns->isNull() &&
        (partitionColumns->getType() != DT_STRING ||
        !(partitionColumns->isVector() || partitionColumns->isScalar()))) {
        throw IllegalArgumentException(__FUNCTION__, syntax +
             "The partition columns must be string scalar or string vector.");
    }

    if (arguments[3]->getType() != DT_STRING ||
        arguments[3]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, syntax + "tablePath must be a string scalar.");
    }
    string tablePath = arguments[3]->getString();

    string symFilePath;
    if (arguments.size() > 4 && !arguments[4]->isNull()) {
        if (arguments[4]->getType() != DT_STRING || arguments[4]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, syntax + "symPath must be a string scalar.");
        }
        symFilePath = arguments[4]->getString();
    }
    if (arguments.size() > 5 && !arguments[5]->isNull()) {
        if (arguments[5]->getForm() == DF_TABLE) {
            schema = arguments[5];
            string errMsg =
                "schema must be a table with at least two columns. First column is name, second column is typeString.";
            if (schema->columns() < 2) {
                throw IllegalArgumentException(__FUNCTION__, syntax + errMsg);
            }
            if (((TableSP)schema)->getColumnName(0) != "name" || (((TableSP)schema)->getColumnName(1) != "typeString")) {
                throw IllegalArgumentException(__FUNCTION__, syntax + errMsg);
            }
            if (((TableSP)schema)->getColumnType(0) != DT_STRING || ((TableSP)schema)->getColumnType(1) != DT_STRING) {
                throw IllegalArgumentException(__FUNCTION__, syntax + "first two schema columns' type must be string.");
            }
        } else {
            throw IllegalArgumentException(__FUNCTION__, syntax + "schema must be a table with at least two columns. ");
        }
    }
    if (arguments.size() > 6 && !arguments[6]->isNull()) {
        if (arguments[6]->getCategory() != INTEGRAL || arguments[6]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, syntax + "batchSize must be an integer scalar.");
        }
        batchSize = arguments[6]->getLong();
        if (batchSize < 0) {
            throw IllegalArgumentException(__FUNCTION__, syntax + "batchSize must be an positive integer scalar.");
        }
    }
    if (arguments.size() > 7 && !arguments[7]->isNull()) {
        if (arguments[7]->getType() != DT_FUNCTIONDEF || (FunctionDefSP(arguments[7]))->getParamCount() != 1) {
            throw IllegalArgumentException(__FUNCTION__, syntax + "transform must be a function with one parameter.");
        }
        transform = arguments[7];
    }
    if (arguments.size() > 8 && !arguments[8]->isNull()) {
        if (((SystemHandleSP)dbHandle)->getDomain()->getEngineType() != DBENGINE_TYPE::IOT) {
            throw IllegalArgumentException(__FUNCTION__, syntax + "sortColumns only support tsdb dbHandle.");
        }
        if (arguments[8]->getType() != DT_STRING || (!arguments[8]->isVector() && !arguments[8]->isScalar())) {
            sortColumns = arguments[8];
        }
    }


    if(!(symFilePath.empty() || Util::exists(symFilePath))) {
        const char* extra = Util::existsDir(symFilePath)
            ? "should be a file, not a directory"
            : "does not exist";
        throw IllegalArgumentException(__FUNCTION__,
            syntax + PLUGIN_NAME ": "
            "symPath [" + symFilePath + "] " + extra + '.');
    }

    vector<string> symList;
    if(!symFilePath.empty()) {
        symList = loadSymList(symFilePath);
    }

    vector<string> fields;
    Util::split(symFilePath, PATH_SEP, fields);
    string symName = "";
    if(!fields.empty()) {
        symName = fields.back();
    }
    DatabaseUpdater dbUpdater{heap, dbHandle, tableName, partitionColumns, sortColumns, schema, transform};


    if(!tablePath.empty() && tablePath.back() != PATH_SEP) {
        tablePath.push_back(PATH_SEP);
    }

    // Read .d file, get column names
    const string dotD = tablePath + ".d";
    vector<string> colNames = loadSymList(dotD);

    return kdb::loadFileEx(dbUpdater, tablePath, symList, symName, colNames);
}