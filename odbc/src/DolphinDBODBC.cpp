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
#include "ddbplugin/CommonInterface.h"
#include "ddbplugin/PluginLoggerImp.h"

using namespace std;

ODBCBaseConnectionSP safeGet(const ConstantSP &arg) {
    if(arg.isNull())
        throw RuntimeException(PLUGIN_ODBC_STRING_PREFIX + "Failed to get ODBC connection, because conn is null. ");
    if (arg->getType() == DT_RESOURCE) {
        LockGuard<Mutex> guard(&ODBCBaseConnection::ODBC_PLUGIN_LOCK);
        long long tag = reinterpret_cast<long long>(arg->getLong());
        if (ODBCBaseConnection::ODBC_CONN_MAP.find(tag) == ODBCBaseConnection::ODBC_CONN_MAP.end()) {
            if (arg->getString().find("ODBC connection to") == 0) {
                throw RuntimeException(PLUGIN_ODBC_STRING_PREFIX + "conn has been closed.");
            } else
                throw RuntimeException(PLUGIN_ODBC_STRING_PREFIX + "conn must be an ODBC connection");
        }
        ODBCBaseConnectionSP odbcConn = ODBCBaseConnection::ODBC_CONN_MAP[tag];
        return odbcConn;
    } else {
        throw RuntimeException(PLUGIN_ODBC_STRING_PREFIX + "Invalid ODBC connection.");
    }
}

ConstantSP odbcClose(Heap *heap, vector<ConstantSP> &args) {
    if (args[0]->getType() != DT_RESOURCE) {
        throw IllegalArgumentException("odbc::close",
                                       PLUGIN_ODBC_STRING_PREFIX + "conn must be an ODBC connection handle");
    }
    ConstantSP handle = ODBCBaseConnection::odbcGetConnection(heap, args, "odbc::close");

    LockGuard<Mutex> guard(&ODBCBaseConnection::ODBC_PLUGIN_LOCK);
    long long tag = reinterpret_cast<long long>(args[0]->getLong());
    if (ODBCBaseConnection::ODBC_CONN_MAP.find(tag) == ODBCBaseConnection::ODBC_CONN_MAP.end()){
        if (args[0]->getString().find("ODBC connection to") == 0) {
            throw RuntimeException(PLUGIN_ODBC_STRING_PREFIX + "The ODBC connection has been closed.");
        } else
            throw RuntimeException(PLUGIN_ODBC_STRING_PREFIX + "conn must be an ODBC connection");
    }
    ODBCBaseConnection::ODBC_CONN_MAP[tag]->close(false);
    ODBCBaseConnection::ODBC_CONN_MAP.erase(tag);
    args[0]->setLong(0);
    // can't use Expression::void_ for undefined error
    return Util::createConstant(DT_VOID);
}

ConstantSP odbcConnect(Heap *heap, vector<ConstantSP> &args) {
    return ODBCBaseConnection::odbcGetConnection(heap, args, "odbc::connect");
}

ConstantSP odbcQuery(Heap *heap, vector<ConstantSP> &args) {
    FunctionDefSP transform;
    int batchSize = -1;
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException("odbc::query", PLUGIN_ODBC_STRING_PREFIX + "sqlQuery must be a string scalar.");
    }
    TableSP appendedTable;
    if (args.size() >= 3){
        if(args[2]->getForm() != DF_TABLE) {
            throw IllegalArgumentException("odbc::query", PLUGIN_ODBC_STRING_PREFIX + "resultTable must be a table.");
        }
        appendedTable = args[2];
    }
    if (args.size() >= 4 && !args[3]->isNothing()) {
        if (args[3]->getForm() != DF_SCALAR || args[3]->getType() != DT_INT)
            throw RuntimeException(PLUGIN_ODBC_STRING_PREFIX + "batchSize must be a int scalar. ");
        batchSize = args[3]->getInt();
        if (batchSize <= 0) throw RuntimeException(PLUGIN_ODBC_STRING_PREFIX + "batchSize must be greater than 0. ");
    }

    if (args.size() >= 5) {
        if (args[4]->getForm() != DF_SCALAR || args[4]->getType() != DT_FUNCTIONDEF)
            throw RuntimeException(PLUGIN_ODBC_STRING_PREFIX + "transform must be a fuction. ");
        transform = args[4];
    }

    string querySql = args[1]->getString();
    if(querySql.empty()){
        throw IllegalArgumentException("odbc::query", PLUGIN_ODBC_STRING_PREFIX + "sqlQuery must be not empty. ");
    }

    vector<ConstantSP> connArgs;
    connArgs.emplace_back(args[0]);
    ConstantSP csp = ODBCBaseConnection::odbcGetConnection(heap, connArgs, "odbc::query");
    ODBCBaseConnectionSP odbcConn = safeGet(csp);
    return odbcConn->odbcQuery(heap, appendedTable, transform, batchSize, querySql);
}

ConstantSP odbcExecute(Heap *heap, vector<ConstantSP> &args) {
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException("odbc::execute", PLUGIN_ODBC_STRING_PREFIX + "sqlStatement must be a string scalar.");
    }
    string querySql = args[1]->getString();
    if(querySql.empty()){
        throw IllegalArgumentException("odbc::query", PLUGIN_ODBC_STRING_PREFIX + "sqlStatement must be not empty. ");
    }
    vector<ConstantSP> connArgs;
    connArgs.emplace_back(args[0]);
    ConstantSP csp = ODBCBaseConnection::odbcGetConnection(heap, connArgs, "odbc::execute");
    ODBCBaseConnectionSP odbcConn = safeGet(csp);
    odbcConn->odbcExecute(querySql, heap);
    return Util::createConstant(DT_VOID);
}

ConstantSP odbcAppend(Heap *heap, vector<ConstantSP> &args) {
    vector<ConstantSP> connArgs;
    connArgs.emplace_back(args[0]);
    ConstantSP csp = ODBCBaseConnection::odbcGetConnection(heap, connArgs, "odbc::append");
    ODBCBaseConnectionSP odbcConn = safeGet(csp);
    if (args[1]->getForm() != DF_TABLE)
        throw IllegalArgumentException("odbc::append", PLUGIN_ODBC_STRING_PREFIX + "ddbTable must be a table");
    TableSP t = args[1];
    if (args[2]->getType() != DT_STRING || args[2]->getForm() != DF_SCALAR)
        throw IllegalArgumentException("odbc::append", PLUGIN_ODBC_STRING_PREFIX + "tableName must be a string scalar");
    string tableName = args[2]->getString();
    if (tableName == "")
        throw IllegalArgumentException("odbc::append", PLUGIN_ODBC_STRING_PREFIX + "tableName cannot be empty");
    bool createTable = true;
    if (args.size() > 3 && !args[3]->isNothing()) {
        if (args[3]->getType() != DT_BOOL || args[3]->getForm() != DF_SCALAR)
            throw IllegalArgumentException("odbc::append",
                                           PLUGIN_ODBC_STRING_PREFIX + "createTableIfNotExist must be a bool scalar");
        if (args[3]->isNull())
            throw IllegalArgumentException("odbc::append",
                                           PLUGIN_ODBC_STRING_PREFIX + "createTableIfNotExist can't be null");
        createTable = args[3]->getBool();
    }
    bool insertIgnore = false;
    if (args.size() > 4 && !args[4]->isNothing()) {
        if (args[4]->getType() != DT_BOOL || args[4]->getForm() != DF_SCALAR)
            throw IllegalArgumentException("odbc::append",
                                           PLUGIN_ODBC_STRING_PREFIX + "ignoreDuplicates must be a bool");
        if (args[4]->isNull())
            throw IllegalArgumentException("odbc::append", PLUGIN_ODBC_STRING_PREFIX + "ignoreDuplicates can't be null");
        insertIgnore = args[4]->getBool();
    }
    return odbcConn->odbcAppend(heap, t, tableName, createTable, insertIgnore);
}
