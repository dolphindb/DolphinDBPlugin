/*
 * DolphinDBODBC.cpp
 *
 *  Created on: May 26, 2017
 *      Author: xinjing.zhou
 */
#include "DolphinDBODBC.h"
#include "ODBCConnection.h"
#include "ScalarImp.h"
#include <algorithm>
#include <map>
using namespace std;

OdbcConnectionSP safeGet(const ConstantSP &arg) {
    if(arg.isNull())
        throw RuntimeException("[PLUGIN::ODBC]: failed to get odbc connection, because handle is null. ");
    if (arg->getType() == DT_RESOURCE) {
        LockGuard<Mutex> guard(&OdbcConnection::ODBC_PLUGIN_LOCK);
        long long tag = reinterpret_cast<long long>(arg->getLong());
        if (OdbcConnection::ODBC_CONN_MAP.find(tag) == OdbcConnection::ODBC_CONN_MAP.end()) {
            if (arg->getString().find("odbc connection to") == 0) {
                throw RuntimeException("[PLUGIN::ODBC]: The odbc connection has been closed.");
            } else
                throw RuntimeException("[PLUGIN::ODBC]: connHandle must be an odbc connection");
        }
        OdbcConnectionSP odbcConn = OdbcConnection::ODBC_CONN_MAP[tag];
        return odbcConn;
    } else {
        throw RuntimeException("[PLUGIN::ODBC]: Invalid odbc connection.");
    }
}

ConstantSP odbcClose(Heap *heap, vector<ConstantSP> &args) {
    if(args.size() < 1)
        throw IllegalArgumentException("odbc::close",
                                           "[PLUGIN::ODBC]: argument size can't be lesser than 1. ");
    if(args[0].isNull())
        throw IllegalArgumentException("odbc::close",
                                           "[PLUGIN::ODBC]: connHandle can't be an Empty pointer. ");
    if (args[0]->getType() != DT_RESOURCE) {
        throw IllegalArgumentException("odbc::close",
                                       "[PLUGIN::ODBC]: connHandle must be an odbc connection handle");
    }
    ConstantSP handle = OdbcConnection::odbcGetConnection(heap, args, "odbc::close");

    LockGuard<Mutex> guard(&OdbcConnection::ODBC_PLUGIN_LOCK);
    long long tag = reinterpret_cast<long long>(args[0]->getLong());
    if (OdbcConnection::ODBC_CONN_MAP.find(tag) == OdbcConnection::ODBC_CONN_MAP.end()){
        if (args[0]->getString().find("odbc connection to") == 0) {
            throw RuntimeException("[PLUGIN::ODBC]: The odbc connection has been closed.");
        } else
            throw RuntimeException("[PLUGIN::ODBC]: connHandle must be an odbc connection");
    }
    OdbcConnection::ODBC_CONN_MAP[tag]->close(false);
    OdbcConnection::ODBC_CONN_MAP.erase(tag);
    args[0]->setLong(0);
    // can't use Expression::void_ for undefined error
    return Util::createConstant(DT_VOID);
}

ConstantSP odbcConnect(Heap *heap, vector<ConstantSP> &args) {
    return OdbcConnection::odbcGetConnection(heap, args, "odbc::connect");
}

ConstantSP odbcQuery(Heap *heap, vector<ConstantSP> &args) {
    FunctionDefSP transform;
    int batchSize = -1;
    if(args.size() < 2)
        throw IllegalArgumentException("odbc::query", "[PLUGIN::ODBC]: the size of arguments must be greater than 1. ");
    if(args[1].isNull()){
        throw IllegalArgumentException("odbc::query", "[PLUGIN::ODBC]: querySql can't be an empty pointer. ");
    }
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException("odbc::query", "[PLUGIN::ODBC]: querySql must be a string scalar.");
    }
    TableSP appendedTable;
    if (args.size() >= 3){
        if(args[2]->getForm() != DF_TABLE) {
            throw IllegalArgumentException("odbc::query", "[PLUGIN::ODBC]: t must be a table.");
        }
        appendedTable = args[2];
    }
    if (args.size() >= 4 && !args[3]->isNothing()) {
        if (args[3]->getForm() != DF_SCALAR || args[3]->getType() != DT_INT)
            throw RuntimeException("[PLUGIN::ODBC]: batchSize must be a int scalar. ");
        batchSize = args[3]->getInt();
        if (batchSize <= 0) throw RuntimeException("[PLUGIN::ODBC]: batchSize must be greater than 0. ");
    }

    if (args.size() >= 5) {
        if (args[4]->getForm() != DF_SCALAR || args[4]->getType() != DT_FUNCTIONDEF)
            throw RuntimeException("[PLUGIN::ODBC]: transform must be a fuction scalar. ");
        transform = args[4];
    }

    string querySql = args[1]->getString();
    u16string querySqlU16 = utf8_to_utf16(querySql);

    vector<ConstantSP> connArgs;
    connArgs.emplace_back(args[0]);
    ConstantSP csp = OdbcConnection::odbcGetConnection(heap, connArgs, "odbc::query");
    OdbcConnectionSP odbcConn = safeGet(csp);
    return odbcConn->odbcQuery(heap, appendedTable, transform, batchSize, querySql);
}

ConstantSP odbcExecute(Heap *heap, vector<ConstantSP> &args) {
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException("odbc::execute", "SQLstatements must be a string scalar.");
    }
    string querySql = args[1]->getString();
    vector<ConstantSP> connArgs;
    connArgs.emplace_back(args[0]);
    ConstantSP csp = OdbcConnection::odbcGetConnection(heap, connArgs, "odbc::execute");
    OdbcConnectionSP odbcConn = safeGet(csp);
    odbcConn->odbcExecute(querySql, heap);
    return Util::createConstant(DT_VOID);
}

ConstantSP odbcAppend(Heap *heap, vector<ConstantSP> &args) {
    vector<ConstantSP> connArgs;
    connArgs.emplace_back(args[0]);
    ConstantSP csp = OdbcConnection::odbcGetConnection(heap, connArgs, "odbc::append");
    OdbcConnectionSP odbcConn = safeGet(csp);
    if (args[1]->getForm() != DF_TABLE)
        throw IllegalArgumentException("odbc::append", "[PLUGIN:ODBC]: tableData must be a table");
    TableSP t = args[1];
    if (args[2]->getType() != DT_STRING || args[2]->getForm() != DF_SCALAR)
        throw IllegalArgumentException("odbc::append", "[PLUGIN:ODBC]: tableName must be a string scalar");
    string tableName = args[2]->getString();
    if (tableName == "")
        throw IllegalArgumentException("odbc::append", "[PLUGIN:ODBC]: tableName cannot be empty");
    bool createTable = true;
    if (args.size() > 3 && !args[3]->isNothing()) {
        if (args[3]->getType() != DT_BOOL || args[3]->getForm() != DF_SCALAR)
            throw IllegalArgumentException("odbc::append",
                                           "[PLUGIN:ODBC]: createTableIfNotExist must be a bool scalar");
        if (args[3]->isNull())
            throw IllegalArgumentException("odbc::append",
                                           "[PLUGIN:ODBC]: createTableIfNotExist can't be null");
        createTable = args[3]->getBool();
    }
    bool insertIgnore = false;
    if (args.size() > 4 && !args[4]->isNothing()) {
        if (args[4]->getType() != DT_BOOL || args[4]->getForm() != DF_SCALAR)
            throw IllegalArgumentException("odbc::append",
                                           "[PLUGIN:ODBC]: insertIgnore must be a bool scalar");
        if (args[4]->isNull())
            throw IllegalArgumentException("odbc::append", "[PLUGIN:ODBC]: insertIgnore can't be null");
        insertIgnore = args[4]->getBool();
    }
    return odbcConn->odbcAppend(heap, t, tableName, createTable, insertIgnore);
}
