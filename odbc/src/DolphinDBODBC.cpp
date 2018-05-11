/*
 * DolphinDBODBC.cpp
 *
 *  Created on: May 26, 2017
 *      Author: xinjing.zhou
 */
#include "DolphinDBODBC.h"
#include "Util.h"
#ifndef LINUX
#include <windows.h>
#endif
#include <nanodbc/nanodbc.h>
#include <sql.h>
#include <sqlext.h>
#include <cassert>
#include <climits>
#include <codecvt>
#include <cstdio>
#include <iostream>
#include <locale>
#include <vector>
using namespace std;
ConstantSP minmax(const ConstantSP &a, const ConstantSP &b)
{
    if (!a->isScalar() && !a->isArray())
        throw IllegalArgumentException(
            "minmax",
            "The argument for minmax function must be a scalar or vector.");
    ConstantSP result = Util::createVector(a->getType(), 2);
    if (a->isScalar())
    {
        result->set(0, a);
        result->set(1, a);
    }
    else
    {
        result->set(0, ((Vector *)a.get())->min());
        result->set(1, ((Vector *)a.get())->max());
    }
    return result;
}

static void odbcConnectionOnClose(Heap *heap, vector<ConstantSP> &args)
{
    nanodbc::connection *cp = (nanodbc::connection *)(args[0]->getLong());
    if (cp != nullptr)
    {
        delete cp;
        args[0]->setLong(0);
    }
    else
    {

    }
}

static const int useStringSizeThreshold = 10;

static inline DATA_TYPE sqltype2DataType(int sqltype, long colsize)
{
    // https://docs.microsoft.com/en-us/sql/odbc/reference/appendixes/c-data-types
    switch (sqltype)
    {
    case SQL_BIT:
        return DT_BOOL;
    case SQL_TINYINT:
    case SQL_SMALLINT:
        return DT_SHORT;
    case SQL_INTEGER:
        return DT_INT;
    case SQL_BIGINT:
        return DT_LONG;
    case SQL_REAL:
        return DT_FLOAT;
    case SQL_FLOAT:
    case SQL_DOUBLE:
    case SQL_DECIMAL:
    case SQL_NUMERIC:
        return DT_DOUBLE;
    case SQL_DATE:
    case SQL_TYPE_DATE:
        return DT_DATE;
        break;
    case SQL_TIME:
    case SQL_TYPE_TIME:
        return DT_NANOTIME;
    case SQL_TIMESTAMP:
    case SQL_TYPE_TIMESTAMP:
        return DT_NANOTIMESTAMP;
    case SQL_CHAR:
        if (colsize == 1)
        {
            return DT_CHAR;
        }
    default:
        if ((sqltype == SQL_CHAR || sqltype == SQL_VARCHAR) &&
            colsize <= useStringSizeThreshold)
        {
            return DT_SYMBOL;
        }
        return DT_STRING;
    }
}

static bool compatible(DATA_TYPE dolphinType, int sqlType, int colsize)
{
    switch (dolphinType)
    {
    case DT_BOOL:
    case DT_CHAR:
    case DT_SHORT:
    case DT_INT:
    case DT_LONG:
    case DT_FLOAT:
    case DT_DOUBLE:
        switch (sqlType)
        {
        case SQL_BIT:
        case SQL_TINYINT:
        case SQL_SMALLINT:
        case SQL_INTEGER:
        case SQL_BIGINT:
        case SQL_REAL:
        case SQL_FLOAT:
        case SQL_DOUBLE:
        case SQL_NUMERIC:
        case SQL_DECIMAL:
            return true;
        case SQL_CHAR:
            if (colsize == 1)
                return true;
        default:
            return false;
        }
    case DT_DATE:
    case DT_TIMESTAMP:
    case DT_NANOTIMESTAMP:
        switch (sqlType)
        {
        case SQL_DATE:
        case SQL_TYPE_DATE:
        case SQL_TIMESTAMP:
        case SQL_TYPE_TIMESTAMP:
            return true;
        default:
            return false;
        }
    case DT_TIME:
    case DT_NANOTIME:
        switch (sqlType)
        {
        case SQL_TYPE_TIME:
        case SQL_TIME:
            return true;
        default:
            return false;
        }
    case DT_STRING:
    case DT_SYMBOL:
        return true;
    default:
        return false;
    }
}

// creates a new native odbc connection if the first argument is a connection
// string. returns the native odbc connection if the first argument is a
// DT_RESOURCE type.
static ConstantSP odbcGetConnection(Heap *heap,
                                    vector<ConstantSP> &args,
                                    const string &funcName)
{
    if (args[0]->getType() == DT_STRING)
    {
        string connStr = args[0]->getString();
        try
        {
            unique_ptr<nanodbc::connection> cup(
                new nanodbc::connection(connStr));

            const char *fmt = "odbc connection to [%s]";
            SmartPointer<char> descBufP(new char[connStr.size() + strlen(fmt)]);
            sprintf(descBufP.get(), fmt, connStr.c_str());

            FunctionDefSP onClose(Util::createSystemProcedure(
                "odbc connection onClose()", odbcConnectionOnClose, 1, 1));

            Constant *res =
                Util::createResource((long long)cup.release(), descBufP.get(),
                                     onClose, heap->currentSession());
            return res;
        }
        catch (const runtime_error &e)
        {
            const char *fmt = "error connecting to [%s]: %s";
            SmartPointer<char> errorMsgBufP(
                new char[connStr.size() + strlen(e.what()) + strlen(fmt)]);
            sprintf(errorMsgBufP.get(), fmt, connStr.c_str(), e.what());
            throw TableRuntimeException(errorMsgBufP.get());
        }
    }
    else if (args[0]->getType() == DT_RESOURCE)
    {
        return args[0];
    }
    else
    {
        throw IllegalArgumentException(funcName,
                                       "Unknown argument type, must be a "
                                       "connection string or odbc connection "
                                       "handle");
    }
    return nullptr;
}

static TableSP odbcGetOrCreateTable(const Heap *heap,
                                    const vector<ConstantSP> &args,
                                    const nanodbc::result &results,
                                    vector<ConstantSP> &columnVecs)
{
    int symbolCount = 0;
    if (args.size() >= 3)
    {
        if (args[2]->getForm() != DF_TABLE)
        {
            throw TableRuntimeException("The 3rd argument must be a table");
        }
        TableSP tblSP(args[2]);
        INDEX columns = tblSP->columns();
        if (columns != results.columns())
        {
            throw TableRuntimeException(
                "The given table schema is incompatible with the returned "
                "table from ODBC (different column count)");
        }
        for (INDEX i = 0; i < columns; ++i)
        {
            Vector *col = (Vector *)tblSP->getColumn(i).get();
            DATA_TYPE dolphinType = col->getType();
            if (!compatible(dolphinType, results.column_datatype(i),
                            results.column_size(i)))
            {
                const char *fmt =
                    "The given table schema is incompatible with the returned "
                    "table from ODBC at column %d[%s]";
                SmartPointer<char> descBufP(
                    new char[10 + strlen(fmt) + results.column_name(i).size()]);
                sprintf(descBufP.get(), fmt, i, results.column_name(i).c_str());
                throw TableRuntimeException(descBufP.get());
            }
            if (dolphinType == DT_SYMBOL)
            {
                columnVecs[i] =
                    Util::createSymbolVector(col->getSymbolBase(), 0);
                symbolCount++;
            }
            else
            {
                columnVecs[i] = Util::createVector(dolphinType, 0);
            }
        }
        return tblSP;
    }
    else
    {
        short columns = results.columns();
        vector<string> columnNames(columns);
        vector<DATA_TYPE> columnTypes(columns);

        for (short i = 0; i < columns; ++i)
        {
            columnNames[i] = results.column_name(i);
            columnTypes[i] = sqltype2DataType(results.column_datatype(i),
                                              results.column_size(i));
            if (columnTypes[i] == DT_SYMBOL)
            {
                symbolCount++;
            }
        }
        // multiple columns share the same symbolbase
        SymbolBaseSP symbolBaseSP;
        if (symbolCount > 0)
        {
            symbolBaseSP = new SymbolBase();
        }
        for (short i = 0; i < columns; ++i)
        {
            if (columnTypes[i] == DT_SYMBOL)
            {
                columnVecs[i] = Util::createSymbolVector(symbolBaseSP, 0);
            }
            else
            {
                columnVecs[i] = Util::createVector(columnTypes[i], 0);
            }
        }
        return Util::createTable(columnNames, columnTypes, 0, 1);
    }
}

ConstantSP odbcClose(Heap *heap, vector<ConstantSP> &args)
{
    assert(heap);
    assert(args.size() == 1);
    if (args[0]->getType() != DT_RESOURCE)
    {
        throw IllegalArgumentException(
            "odbc::close",
            "Unknown argument type, must be an odbc connection handle");
    }
    ConstantSP csp = odbcGetConnection(heap, args, "odbc::close");
    nanodbc::connection *cp = (nanodbc::connection *)(csp->getLong());
    cp->disconnect();
    return Util::createConstant(DT_VOID);
}

ConstantSP odbcConnect(Heap *heap, vector<ConstantSP> &args)
{
    assert(heap);
    assert(args.size() == 1);
    if (args[0]->getType() != DT_STRING)
    {
        throw IllegalArgumentException(
            "odbc::connect",
            "Unknown argument type, must be an odbc connection string");
    }
    return odbcGetConnection(heap, args, "odbc::connect");
}

ConstantSP odbcQuery(Heap *heap, vector<ConstantSP> &args)
{
    assert(heap);
    assert(args.size() >= 2);

    string querySql = args[1]->getString();
    ConstantSP csp = odbcGetConnection(heap, args, "odbc::query");
    nanodbc::connection *cp = (nanodbc::connection *)(csp->getLong());

    nanodbc::result results;
    try
    {
        results = nanodbc::execute(*cp, querySql);
    }
    catch (const runtime_error &e)
    {
        const char *fmt = "error executing query [%s]: %s";
        SmartPointer<char> errorMsgBufP(
            new char[querySql.size() + strlen(e.what()) + strlen(fmt)]);
        sprintf(errorMsgBufP.get(), fmt, querySql.c_str(), e.what());
        throw TableRuntimeException(errorMsgBufP.get());
    }

    const short columns = results.columns();
    vector<ConstantSP> columnVecs(columns);
    vector<Vector *> arrCol(columns);
    vector<DATA_TYPE> columnTypes(columns);

    TableSP table = odbcGetOrCreateTable(heap, args, results, columnVecs);
    for (short i = 0; i < columns; ++i)
    {
        arrCol[i] = (Vector *)columnVecs[i].get();
        columnTypes[i] = arrCol[i]->getType();
    }

    vector<vector<U8>> buffers =
        vector<vector<U8>>(columns, vector<U8>(Util::BUF_SIZE));
    int curLine = 0, rowIdx = 0;
    char charBuf[Util::BUF_SIZE];
    short shortBuf[Util::BUF_SIZE];
    U4 buf[Util::BUF_SIZE];
    bool hasNext = results.next();
    while (hasNext)
    {
        for (short col = 0; col < columns; ++col)
        {
            // nanodbc stores numeric & decimal types as strings
            if (results.column_datatype(col) == SQL_DECIMAL ||
                results.column_datatype(col) == SQL_NUMERIC)
            {
                string str;
                results.get_ref<string>(col, "", str);
                if (str != "")
                    sscanf(str.c_str(), "%lf",
                           &(buffers[col][curLine].doubleVal));
                else
                    buffers[col][curLine].doubleVal = DBL_NMIN;
            }
            else
            {
                switch (columnTypes[col])
                {
                case DT_BOOL:
                    buffers[col][curLine].charVal =
                        results.get<char>(col, CHAR_MIN);
                    if (buffers[col][curLine].charVal != CHAR_MIN)
                    {
                        buffers[col][curLine].charVal =
                            !!buffers[col][curLine].charVal;
                    }
                    break;
                case DT_CHAR:
                    buffers[col][curLine].charVal =
                        results.get<char>(col, CHAR_MIN);
                    break;
                case DT_SHORT:
                    buffers[col][curLine].shortVal =
                        results.get<short>(col, SHRT_MIN);
                    break;
                case DT_INT:
                    buffers[col][curLine].intVal =
                        results.get<int>(col, INT_MIN);
                    break;
                case DT_LONG:
                    buffers[col][curLine].longVal =
                        results.get<int64_t>(col, LLONG_MIN);
                    break;
                case DT_FLOAT:
                    buffers[col][curLine].floatVal =
                        results.get<float>(col, FLT_MIN);
                    break;
                case DT_DOUBLE:
                    buffers[col][curLine].doubleVal =
                        results.get<double>(col, DBL_NMIN);
                    break;
                case DT_DATE:
                {
                    nanodbc::date fallback = {0, 0, 0};
                    nanodbc::date date = {0, 0, 0};
                    results.get_ref<nanodbc::date>(col, fallback, date);
                    if (memcmp(&date, &fallback, sizeof(nanodbc::date)) ==
                        0)
                    {
                        buffers[col][curLine].intVal = INT_MIN;
                    }
                    else
                    {
                        buffers[col][curLine].intVal = Util::countDays(
                            date.year, date.month, date.day);
                    }
                }
                break;
                case DT_TIMESTAMP:
                {
                    nanodbc::timestamp fallback = {0, 0, 0, 0, 0, 0, 0};
                    nanodbc::timestamp ts = {0, 0, 0, 0, 0, 0, 0};
                    results.get_ref<nanodbc::timestamp>(col, fallback, ts);
                    if (memcmp(&ts, &fallback,
                               sizeof(nanodbc::timestamp)) == 0)
                    {
                        buffers[col][curLine].longVal = LLONG_MIN;
                    }
                    else
                    {
                        // ts.fract is the number of billionths of a second.
                        // https://docs.microsoft.com/en-us/sql/odbc/reference/appendixes/c-data-types
                        buffers[col][curLine].longVal =
                            Util::countDays(ts.year, ts.month, ts.day) *
                                86400000ll +
                            ((ts.hour * 60 + ts.min) * 60 + ts.sec) * 1000 +
                            ts.fract / 1000000;
                    }
                }
                break;
                case DT_TIME:
                {
                    nanodbc::timestamp fallback = {0, 0, 0, 0, 0, 0, 0};
                    nanodbc::timestamp ts = {0, 0, 0, 0, 0, 0, 0};
                    results.get_ref<nanodbc::timestamp>(col, fallback, ts);
                    if (memcmp(&ts, &fallback,
                               sizeof(nanodbc::timestamp)) == 0)
                    {
                        buffers[col][curLine].intVal = INT_MIN;
                    }
                    else
                    {
                        buffers[col][curLine].intVal =
                            ((ts.hour * 60 + ts.min) * 60 + ts.sec) * 1000 +
                            ts.fract / 1000000;
                    }
                }
                break;
                case DT_NANOTIME:
                {
                    nanodbc::timestamp fallback = {0, 0, 0, 0, 0, 0, 0};
                    nanodbc::timestamp ts = {0, 0, 0, 0, 0, 0, 0};
                    results.get_ref<nanodbc::timestamp>(col, fallback, ts);
                    if (memcmp(&ts, &fallback,
                               sizeof(nanodbc::timestamp)) == 0)
                    {
                        buffers[col][curLine].longVal = LLONG_MIN;
                    }
                    else
                    {
                        buffers[col][curLine].longVal =
                            ((ts.hour * 60 + ts.min) * 60 + ts.sec) *
                                1000000000ll +
                            ts.fract;
                    }
                }
                break;
                case DT_NANOTIMESTAMP:
                {
                    nanodbc::timestamp fallback = {0, 0, 0, 0, 0, 0, 0};
                    nanodbc::timestamp ts = {0, 0, 0, 0, 0, 0, 0};
                    results.get_ref<nanodbc::timestamp>(col, fallback, ts);
                    if (memcmp(&ts, &fallback,
                               sizeof(nanodbc::timestamp)) == 0)
                    {
                        buffers[col][curLine].longVal = LLONG_MIN;
                    }
                    else
                    {
                        buffers[col][curLine].longVal =
                            Util::countDays(ts.year, ts.month, ts.day) *
                                86400000000000ll +
                            ((ts.hour * 60 + ts.min) * 60 + ts.sec) *
                                1000000000ll +
                            ts.fract;
                    }
                }
                break;
                case DT_SYMBOL:
                {
                    SymbolBaseSP symbolBase = arrCol[col]->getSymbolBase();
                    string str;
                    results.get_ref<string>(col, "", str);
                    buffers[col][curLine].intVal =
                        symbolBase->findAndInsert(str);
                }
                break;
                default:
                    string str;
                    results.get_ref<string>(col, "", str);
                    buffers[col][curLine].pointer =
                        new char[str.size() + 1];
                    if (str == "")
                    {
                        *(buffers[col][curLine].pointer) = '\0';
                    }
                    else
                    {
                        strcpy(buffers[col][curLine].pointer, str.c_str());
                    }
                }
            }
        }
        ++curLine;
        rowIdx++;
        hasNext = results.next();
        if (hasNext == false || curLine == Util::BUF_SIZE)
        {
            for (short col = 0; col < columns; ++col)
            {
                DATA_TYPE rawType = arrCol[col]->getRawType();
                int sqltype = results.column_datatype(col);
                vector<U8> &colBuf = buffers[col];
                switch (rawType)
                {
                case DT_BOOL:
                    if (sqltype == SQL_DECIMAL || sqltype == SQL_NUMERIC)
                    {
                        for (int j = 0; j < curLine; ++j)
                            charBuf[j] = (bool)colBuf[j].doubleVal;
                    }
                    else
                    {
                        for (int j = 0; j < curLine; ++j)
                            charBuf[j] = colBuf[j].charVal;
                    }
                    arrCol[col]->appendBool(charBuf, curLine);
                    break;
                case DT_CHAR:
                    if (sqltype == SQL_DECIMAL || sqltype == SQL_NUMERIC)
                    {
                        for (int j = 0; j < curLine; ++j)
                            charBuf[j] = (char)colBuf[j].doubleVal;
                    }
                    else
                    {
                        for (int j = 0; j < curLine; ++j)
                            charBuf[j] = colBuf[j].charVal;
                    }
                    arrCol[col]->appendChar(charBuf, curLine);
                    break;
                case DT_SHORT:
                    if (sqltype == SQL_DECIMAL || sqltype == SQL_NUMERIC)
                    {
                        for (int j = 0; j < curLine; ++j)
                            shortBuf[j] = (short)colBuf[j].doubleVal;
                    }
                    else
                    {
                        for (int j = 0; j < curLine; ++j)
                            shortBuf[j] = colBuf[j].shortVal;
                    }
                    arrCol[col]->appendShort(shortBuf, curLine);
                    break;
                case DT_INT:
                    if (sqltype == SQL_DECIMAL || sqltype == SQL_NUMERIC)
                    {
                        for (int j = 0; j < curLine; ++j)
                            buf[j].intVal = (int)colBuf[j].doubleVal;
                    }
                    else
                    {
                        for (int j = 0; j < curLine; ++j)
                            buf[j].intVal = colBuf[j].intVal;
                    }

                    arrCol[col]->appendInt((int *)buf, curLine);
                    break;
                case DT_LONG:
                    if (sqltype == SQL_DECIMAL || sqltype == SQL_NUMERIC)
                    {
                        for (int j = 0; j < curLine; ++j)
                            colBuf[j].longVal =
                                (long long)colBuf[j].doubleVal;
                    }
                    arrCol[col]->appendLong((long long *)(&colBuf[0]),
                                            curLine);
                    break;
                case DT_FLOAT:
                    if (sqltype == SQL_DECIMAL || sqltype == SQL_NUMERIC)
                    {
                        for (int j = 0; j < curLine; ++j)
                            buf[j].floatVal =
                                (long long)colBuf[j].doubleVal;
                    }
                    else
                    {
                        for (int j = 0; j < curLine; ++j)
                            buf[j].floatVal = colBuf[j].floatVal;
                    }
                    arrCol[col]->appendFloat((float *)buf, curLine);
                    break;
                case DT_DOUBLE:
                    arrCol[col]->appendDouble((double *)(&colBuf[0]),
                                              curLine);
                    break;
                case DT_STRING:
#ifdef BIT32
                    if (sqltype == SQL_DECIMAL || sqltype == SQL_NUMERIC)
                    {
                        for (int j = 0; j < curLine; ++j)
                        {
                            char strBuf[120];
                            int nbytes =
                                snprintf(strBuf, sizeof(strBuf), "%lf",
                                         colBuf[j].doubleVal);
                            buf[j].pointer = new char[nbytes + 1];
                            if (colBuf[j].doubleVal == DBL_NMIN)
                                buf[j].pointer = nullptr;
                            strcpy(buf[j].pointer, strBuf);
                        }
                    }
                    else
                    {
                        for (int j = 0; j < curLine; ++j)
                            buf[j].pointer = colBuf[j].pointer;
                    }
                    arrCol[col]->appendString((char **)buf, curLine);
                    for (int j = 0; j < curLine; ++j)
                        delete[] buf[j].pointer;
#else
                    if (sqltype == SQL_DECIMAL || sqltype == SQL_NUMERIC)
                    {
                        for (int j = 0; j < curLine; ++j)
                        {
                            char strBuf[120];
                            int nbytes =
                                snprintf(strBuf, sizeof(strBuf), "%lf",
                                         colBuf[j].doubleVal);
                            buffers[col][j].pointer = new char[nbytes + 1];
                            if (colBuf[j].doubleVal == DBL_NMIN)
                                buffers[col][j].pointer = nullptr;
                            strcpy(buffers[col][j].pointer, strBuf);
                        }
                    }
                    arrCol[col]->appendString((char **)(&buffers[col][0]),
                                              curLine);
                    for (int j = 0; j < curLine; ++j)
                        delete[] buffers[col][j].pointer;
#endif
                    break;
                default:
                    assert(false);
                }
            }
            curLine = 0;
        }
    }
    for (short i = 0; i < columns; ++i)
    {
        arrCol[i]->setNullFlag(arrCol[i]->hasNull());
    }

    if (arrCol[0]->size() > 0)
    {
        string errMsg;
        INDEX insertedRows = 0;
        bool good = table->append(columnVecs, insertedRows, errMsg);
        if (good == false)
        {
            throw TableRuntimeException(errMsg);
        }
    }
    return table;
}

ConstantSP odbcExecute(Heap *heap, vector<ConstantSP> &args)
{
    assert(heap);
    assert(args.size() >= 2);

    string querySql = args[1]->getString();
    ConstantSP csp = odbcGetConnection(heap, args, "odbc::query");
    nanodbc::connection *cp = (nanodbc::connection *)(csp->getLong());

    nanodbc::result results;
    try
    {
        results = nanodbc::execute(*cp, querySql);
    }
    catch (const runtime_error &e)
    {
        const char *fmt = "error executing query [%s]: %s";
        SmartPointer<char> errorMsgBufP(
            new char[querySql.size() + strlen(e.what()) + strlen(fmt)]);
        sprintf(errorMsgBufP.get(), fmt, querySql.c_str(), e.what());
        throw TableRuntimeException(errorMsgBufP.get());
    }

    return Util::createConstant(DT_VOID);
}