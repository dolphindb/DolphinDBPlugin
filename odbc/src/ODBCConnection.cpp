#include "ODBCConnection.h"
#include <cstddef>
#include <cstring>
#include <string>
#include "Exceptions.h"
#include "ScalarImp.h"
#include "Types.h"
#include "cvt.h"
#include "nanodbc/nanodbc.h"
using namespace std;

static void odbcConnectionOnClose(Heap *heap, vector<ConstantSP> &args) {
    if(args.size() < 1){
        LOG_ERR("[PLUGIN::ODBC]: ", "the count of param for odbcConnectionOnClose can't be less than 1");
        return;
    }
    LockGuard<Mutex> guard(&OdbcConnection::ODBC_PLUGIN_LOCK);
    long long cp = args[0]->getLong();
    if (cp == 0 || OdbcConnection::ODBC_CONN_MAP.find(cp) == OdbcConnection::ODBC_CONN_MAP.end()){
        LOG_ERR(args[0]->getString());
        if(args[0]->getString().find("odbc connection to") == string::npos)
            LOG_ERR("[PLUGIN::ODBC]: odbcConnectionOnClose handle is not an odbc connection. ");
        return;
    }
    OdbcConnectionSP data = OdbcConnection::ODBC_CONN_MAP[cp];
    try{
        data->close(true);
    }
    catch(exception& e){
        LOG_ERR("[PLUGIN::ODBC]: failed to close odbc connection: ", e.what());
    }
    OdbcConnection::ODBC_CONN_MAP.erase(cp);
    args[0]->setLong(0);
}

DATA_TYPE OdbcConnection::sqlType2DataType(int sqlType, long colSize, int cType) {
    // cout << sqlType << " " << colSize << " " << cType << endl;
    // https://docs.microsoft.com/en-us/sql/odbc/reference/appendixes/c-data-types
    switch (sqlType) {
        case SQL_BIT: return DT_BOOL;
        case SQL_TINYINT:
        case SQL_SMALLINT: return DT_SHORT;
        case SQL_INTEGER: return DT_INT;
        case SQL_BIGINT: return DT_LONG;
        case SQL_REAL: return DT_FLOAT;
        case SQL_FLOAT:
        case SQL_DOUBLE:
        case SQL_DECIMAL:
        case SQL_NUMERIC: return DT_DOUBLE;
        case SQL_DATE:
        case SQL_TYPE_DATE: return DT_DATE;
        case SQL_TIME:
        case SQL_TYPE_TIME: return DT_SECOND;
        case SQL_TIMESTAMP:
        case SQL_TYPE_TIMESTAMP: return DT_NANOTIMESTAMP;
        case SQL_IS_SMALLINT:  // aka char
        case SQL_CHAR:
            if (colSize == 1) { return DT_CHAR; }
        // fall-through
        default:
            // if(cType == SQL_C_BINARY)
            //   return DT_BLOB;
            // if ((sqlType == SQL_CHAR || sqlType == SQL_VARCHAR) &&
            //     colSize <= useStringSizeThreshold) {
            //   return DT_SYMBOL;
            // }
            return DT_STRING;
    }
}

bool OdbcConnection::compatible(DATA_TYPE dolphinType, int sqlType, int colSize) {
    switch (dolphinType) {
        case DT_BOOL:
        case DT_CHAR:
        case DT_SHORT:
        case DT_INT:
        case DT_LONG:
        case DT_FLOAT:
        case DT_DOUBLE:
            switch (sqlType) {
                case SQL_BIT:
                case SQL_TINYINT:
                case SQL_SMALLINT:
                case SQL_INTEGER:
                case SQL_BIGINT:
                case SQL_REAL:
                case SQL_FLOAT:
                case SQL_DOUBLE:
                case SQL_NUMERIC:
                case SQL_DECIMAL: return true;
                case SQL_IS_SMALLINT:
                case SQL_CHAR:
                    if (colSize == 1) return true;
                default: return false;
            }
        case DT_DATE:
        case DT_TIMESTAMP:
        case DT_NANOTIMESTAMP:
            switch (sqlType) {
                case SQL_DATE:
                case SQL_TYPE_DATE:
                case SQL_TIMESTAMP:
                case SQL_TYPE_TIMESTAMP: return true;
                default: return false;
            }
        case DT_SECOND:
        case DT_TIME:
        case DT_NANOTIME:
            switch (sqlType) {
                case SQL_TYPE_TIME:
                case SQL_TIME: return true;
                default: return false;
            }
        case DT_STRING:
        case DT_SYMBOL:
        case DT_BLOB: return true;
        default: return false;
    }
}

ODBCDataBaseType OdbcConnection::getDataBaseType(const string &dataBase) {
    if (DBT_MAP.find(dataBase) == DBT_MAP.end())
        throw RuntimeException("[PLUGIN::ODBC]: The dataBaseType " + dataBase + "is not supported. ");
    return DBT_MAP[dataBase];
}

void OdbcConnection::getColNames(const nanodbc::result &results, vector<std::string> &columnNames) {
    short columns = results.columns();
    columnNames.resize(columns);
    for (short i = 0; i < columns; ++i) { columnNames[i] = utf16_to_utf8(results.column_name(i)); }
}

void OdbcConnection::getColVecAndType(const nanodbc::result &results, vector<ConstantSP> &columnVecs,
                      vector<DATA_TYPE> &columnTypes) {
    const short columns = results.columns();
    columnTypes.resize(columns);
    columnVecs.resize(columns);
    for (short i = 0; i < columns; ++i) {
        columnTypes[i] = sqlType2DataType(results.column_datatype(i), results.column_size(i),
                                          results.column_c_datatype(i));
        columnVecs[i] = Util::createVector(columnTypes[i], 0);
    }
}


string OdbcConnection::sqlTypeName(DATA_TYPE t, ODBCDataBaseType databaseType) {
    switch (t) {
        case DT_BOOL:
            if (databaseType == ODBC_DBT_POSTGRSQL)
                return "boolean";
            else if (databaseType == ODBC_DBT_CLICKHOUSE)
                return "Bool";
            else if (databaseType == ODBC_DBT_ORACLE)
                return "char(1)";
            else
                return "bit";
        case DT_CHAR: return "char(1)";
        case DT_SHORT: return "smallint";
        case DT_INT: return "int";
        case DT_LONG:
            if (databaseType == ODBC_DBT_ORACLE) return "number(19)";
            return "bigint";
        case DT_DATE: return "date";
        case DT_MONTH: return "date";
        case DT_TIME:
            if (databaseType == ODBC_DBT_ORACLE)
                throw RuntimeException("[PLUGIN:ODBC]: Unsupported data type for Oracle: " + Util::getDataTypeString(t));
            return "time";
        case DT_MINUTE:
            if (databaseType == ODBC_DBT_ORACLE)
                throw RuntimeException("[PLUGIN:ODBC]: Unsupported data type for Oracle: " + Util::getDataTypeString(t));
            return "time";
        case DT_SECOND:
            if (databaseType == ODBC_DBT_ORACLE)
                throw RuntimeException("[PLUGIN:ODBC]: Unsupported data type for Oracle: " + Util::getDataTypeString(t));
            return "time";
        case DT_DATETIME:
            if (databaseType == ODBC_DBT_POSTGRSQL)
                return "timestamp";
            else if (databaseType == ODBC_DBT_ORACLE)
                return "date";
            else if (databaseType == ODBC_DBT_CLICKHOUSE)
                return "datetime64";
            else
                return "datetime";

        case DT_TIMESTAMP:
            if (databaseType == ODBC_DBT_POSTGRSQL)
                return "timestamp";
            else if (databaseType == ODBC_DBT_CLICKHOUSE)
                return "datetime64";
            else if (databaseType == ODBC_DBT_ORACLE)
                return "timestamp";
            else if (databaseType == ODBC_DBT_SQLSERVER)
                return "datetime2(3)";
            else
                return "datetime";
        case DT_NANOTIME:
            if (databaseType == ODBC_DBT_ORACLE)
                throw RuntimeException("[PLUGIN:ODBC]: Unsupported data type for Oracle: " + Util::getDataTypeString(t));
            return "time";
        case DT_NANOTIMESTAMP:
            if (databaseType == ODBC_DBT_POSTGRSQL)
                return "timestamp";
            else if (databaseType == ODBC_DBT_CLICKHOUSE)
                return "datetime64";
            else if (databaseType == ODBC_DBT_ORACLE)
                return "timestamp(9)";
            else if (databaseType == ODBC_DBT_SQLSERVER)
                return "datetime2(7)";
            else
                return "datetime";
        case DT_FLOAT:
            if (databaseType == ODBC_DBT_SQLSERVER)
                return "float(24)";
            else
                return "float";
        case DT_DOUBLE:
            if (databaseType == ODBC_DBT_POSTGRSQL)
                return "double precision";
            else if (databaseType == ODBC_DBT_SQLSERVER)
                return "float(53)";
            else if (databaseType == ODBC_DBT_ORACLE)
                return "binary_double";
            else
                return "double";
        case DT_SYMBOL: return "varchar(255)";
        case DT_STRING: return "varchar(255)";

        default:
            throw RuntimeException("[PLUGIN:ODBC]: The data type " + Util::getDataTypeString(t) +
                                   " of DolphinDB is not supported. ");
    }
}

string OdbcConnection::getValueStr(string p, DATA_TYPE t, ODBCDataBaseType databaseType) {
    if (p == "") return "null";
    switch (t) {
        case DT_BOOL:
            if (databaseType == ODBC_DBT_POSTGRSQL) {
                if (p == "1")
                    return "true";
                else
                    return "false";
            } else {
                return p;
            }

        case DT_CHAR:
            if (databaseType == ODBC_DBT_SQLITE) {
                if (p == "\'") {
                    return "\'\'\'\'";
                } else if (p == "\\") {
                    return "\'\\\'";
                }
            }
            return "\'" + p + "\'";

        case DT_DATE:
        {
            for (size_t i = 0; i < p.length(); i++)
                if (p[i] == '.') p[i] = '-';
            string ret;
            if (databaseType == ODBC_DBT_ORACLE) {

                ret.reserve(p.size() + 30);
                ret += "to_date(";
                ret += '\'';
                ret += p;
                ret += '\'';
                ret +=  ", 'YYYY-MM-DD')";
            } else {
                ret.reserve(p.size() + 2);
                ret += '\'';
                ret += p;
                ret += '\'';
            }
            return ret;
        }

        case DT_MONTH:
            for (size_t i = 0; i < p.length(); i++)
                if (p[i] == '.') p[i] = '-';
            p.erase(p.length() - 1, 1);
            p += "-01";
            p = "\'" + p + "\'";
            if (databaseType == ODBC_DBT_ORACLE) { p = "to_date(" + p + ", 'YYYY-MM-DD')"; }
            return p;

        case DT_TIME:
            // s = s.substr(0, 8);
            p = "\'" + p + "\'";
            return p;

        case DT_MINUTE:
            if(p.size() < 5)
                throw RuntimeException("[PLUGIN::ODBC]: Minute data string size must be granter than 4");
            p = p.substr(0, 5);
            p += ":00";
            p = "\'" + p + "\'";
            return p;

        case DT_SECOND:
            p = "\'" + p + "\'";
            return p;

        case DT_DATETIME:
            for (size_t i = 0; i < p.length(); i++) {
                if (p[i] == '.') p[i] = '-';
                if (p[i] == 'T') p[i] = ' ';
            }

            if (databaseType == ODBC_DBT_ORACLE) {
                p = "\'" + p + "\'";
                p = "to_date(" + p + ", 'YYYY-MM-DD HH24:MI:SS')";
            } else {
                p += ".000";
                p = "\'" + p + "\'";
            }
            return p;

        case DT_TIMESTAMP:
        {
            for (size_t i = 0; i + 5 < p.length(); i++) {
                if (p[i] == '.') p[i] = '-';
                if (p[i] == 'T') p[i] = ' ';
            }
            if(UNLIKELY(p.size() < 23))
                throw RuntimeException("[PLUGIN::ODBC]: Timestamp data string size must be granter than 22");
            p = p.substr(0, 23);
            string ret;
            ret.clear();
            if (databaseType == ODBC_DBT_ORACLE) {
                if (ret.capacity() < p.size() + 50) ret.reserve(p.size() + 50);
                ret += "to_timestamp(";
                ret += '\'';
                ret += p;
                ret += '\'';
                ret += ", 'YYYY-MM-DD HH24:MI:SS.ff')";
            } else {
                ret.reserve(p.size() + 2);
                ret += '\'';
                ret += p;
                ret += '\'';
            }
            return ret;
        }

        case DT_NANOTIME:
            // s = s.substr(0, 15);
            p = "\'" + p + "\'";
            return p;

        case DT_NANOTIMESTAMP:
            // cout << s << endl;
            if (databaseType == ODBC_DBT_ORACLE) {
                for (size_t i = 0; i + 12 < p.length(); i++) {
                    if (p[i] == '.') p[i] = '-';
                    if (p[i] == 'T') p[i] = ' ';
                }
            } else {
                if(p.size() < 23)
                    throw RuntimeException("[PLUGIN::ODBC]: NanoTimestamp data string size must be granter than 22");
                p = p.substr(0, 23);
                for (size_t i = 0; i + 5 < p.length(); i++) {
                    if (p[i] == '.') p[i] = '-';
                    if (p[i] == 'T') p[i] = ' ';
                }
            }
            p = "\'" + p + "\'";
            if (databaseType == ODBC_DBT_ORACLE) {
                p = "to_timestamp(" + p + ", 'YYYY-MM-DD HH24:MI:SS.ff9')";
            }
            return p;

        case DT_SYMBOL:
        case DT_STRING:
        {
            string ret;
            ret.clear();
            ret += "\'";
            if (databaseType == ODBC_DBT_POSTGRSQL || databaseType == ODBC_DBT_SQLITE ||
                databaseType == ODBC_DBT_SQLSERVER) {
                for (auto i : p) {
                    if (UNLIKELY(i == '\'')) ret += "\'";
                    ret += i;
                }
            } else {
                if(ret.capacity() < p.size()*2) ret.reserve(p.size()*2);
                for (auto i : p) {
                    if (UNLIKELY(i == '\\' || i == '\'')) ret += "\\";
                    ret += i;
                }
            }
            ret += "\'";
            return ret;
        }
        default: return p;
    }
    return "";
}

void OdbcConnection::odbcExecute(nanodbc::transaction &cp, const string &querySql, Heap *heap) {
    u16string querySqlU16 = utf8_to_utf16(querySql);
    try {
        nanodbc::just_execute(cp, querySqlU16);
    } catch (const runtime_error &e) {
        std::string errorMsg = "Executed query [" + querySql + "]: " + e.what();
        if (errorMsg.find("Syntax error") != string::npos &&
            errorMsg.find("query, subquery, possibly with UNION, SELECT subquery, SELECT") != string::npos) {
            this->close(false);
        }
        throw RuntimeException(string("[PLUGIN:ODBC]: ") + e.what());
    }
}

void OdbcConnection::odbcExecute(const string &querySql, Heap *heap) {
    LockGuard<Mutex> lockGuard(&lock);
    if(closed_)
        throw RuntimeException("[PLUGIN::ODBC]: odbc connection is closed. ");
    u16string querySqlU16 = utf8_to_utf16(querySql);
    try {
        nanodbc::just_execute(*nanoConn_, querySqlU16);
    } catch (const runtime_error &e) {
        std::string errorMsg = "Executed query [" + querySql + "]: " + e.what();
        if (errorMsg.find("Syntax error") != string::npos &&
            errorMsg.find("query, subquery, possibly with UNION, SELECT subquery, SELECT") != string::npos) {
            this->close(false);
        }
        throw RuntimeException(string("[PLUGIN:ODBC]: ") + e.what());
    }
}

// creates a new native odbc connection if the first argument is a connection
// string. returns the native odbc connection if the first argument is a
// DT_RESOURCE type.
// make sure
ConstantSP OdbcConnection::odbcGetConnection(Heap *heap, vector<ConstantSP> &args, const string &funcName) {
    if (args.size() == 0)
        throw IllegalArgumentException(funcName,
                                       "[PLUGIN::ODBC]: The first argument must be a connection string "
                                       "scalar or an odbc connection handle");
    if (args[0]->isScalar() && args[0]->getType() == DT_STRING) {
        string connStr = args[0]->getString();
        if (connStr == "")
            throw IllegalArgumentException(funcName, "[PLUGIN::ODBC]: connStr can't be an empty string. ");
        u16string connStrU16 = utf8_to_utf16(connStr);
        string dataBaseType;
        if (args.size() >= 2) {
            if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR)
                throw IllegalArgumentException(funcName,
                                               "[PLUGIN::ODBC]: DataBaseType must be a string scalar. ");
            dataBaseType = args[1]->getString();
            std::transform(dataBaseType.begin(), dataBaseType.end(), dataBaseType.begin(), ::tolower);

            if (dataBaseType != "postgresql" && dataBaseType != "mysql" && dataBaseType != "sqlserver" &&
                dataBaseType != "clickhouse" && dataBaseType != "sqlite" && dataBaseType != "oracle")
                throw IllegalArgumentException(funcName,
                                               "[PLUGIN::ODBC]: DataBaseType must be PostgreSQL, SQLServer, "
                                               "MySQL, ClickHouse, SQLite, or Oracle. ");
        }

        OdbcConnectionSP cup(
            new OdbcConnection(new nanodbc::connection(connStrU16), getDataBaseType(dataBaseType)));

        if (dataBaseType == "clickhouse") {
            // can't use new String("select 1") for undefined error
            string sqlString = "select 1";
            cup->odbcExecute(sqlString, heap);
        }
        string desc = "odbc connection to [" + connStr + "]";
        FunctionDefSP onClose(
            Util::createSystemProcedure("odbc connection onClose()", odbcConnectionOnClose, 1, 1));
        ConstantSP res =
            Util::createResource((long long)cup.get(), desc, onClose, heap->currentSession());
        LockGuard<Mutex> guard(&ODBC_PLUGIN_LOCK);
        long long conn = reinterpret_cast<long long>(cup.get());
        ODBC_CONN_MAP[conn] = cup;

        return res;
    } else if (args[0]->getType() == DT_RESOURCE) {
        LockGuard<Mutex> lock(&ODBC_PLUGIN_LOCK);
        long long conn = args[0]->getLong();
        if (ODBC_CONN_MAP.find(conn) == ODBC_CONN_MAP.end()){
            if(args[0]->getString().find("odbc connection") == 0) {
                throw RuntimeException("[PLUGIN::ODBC]: The odbc connection has been closed.");
            }
            else
                throw IllegalArgumentException(funcName,
                                            "[PLUGIN::ODBC]: connHandle must be an odbc connection");
        }
        return args[0];
    } else {
        throw IllegalArgumentException(funcName,
                                       "[PLUGIN::ODBC]: The first argument must be a connection string "
                                       "scalar or an odbc connection handle");
    }
}

TableSP OdbcConnection::odbcGetOrCreateTable(Heap *heap, const TableSP& schemaTable,
                                    const nanodbc::result &results, vector<ConstantSP> &columnVecs,
                                    vector<string> &colNames, bool compatibility) {
    getColNames(results, colNames);
    const short columns = results.columns();
    if (!schemaTable.isNull()) {
        TableSP tblSP = schemaTable;
        if (compatibility) {
            if (columns != tblSP->columns()) {
                throw TableRuntimeException(
                    "[PLUGIN::ODBC]: The given table schema is incompatible with the returned table from "
                    "ODBC");
            }
            columnVecs.resize(columns);
            for (INDEX i = 0; i < columns; ++i) {
                DATA_TYPE dolphinType = tblSP->getColumnType(i);
                if (!compatible(dolphinType, results.column_datatype(i), results.column_size(i))) {
                    throw TableRuntimeException(
                        "[PLUGIN::ODBC]: The given table schema is incompatible with the returned table from "
                        "ODBC at column " +
                        to_string(i) + "[" + utf16_to_utf8(results.column_name(i)) + "]");
                }
                columnVecs[i] = Util::createVector(dolphinType, 0);
            }
        } else {
            vector<DATA_TYPE> columnTypes;
            getColVecAndType(results, columnVecs, columnTypes);
        }
        return tblSP;
    } else {
        vector<DATA_TYPE> columnTypes;
        getColVecAndType(results, columnVecs, columnTypes);
        return Util::createTable(colNames, columnTypes, 0, 0);
    }
}

ConstantSP OdbcConnection::odbcAppend(Heap* heap, TableSP t, const string& tableName, bool createTable, bool insertIgnore){
    LockGuard<Mutex> lockGuard(&lock);
    if(closed_)
        throw RuntimeException("[PLUGIN::ODBC]: odbc connection is closed. ");
    nanodbc::transaction tranConn(*nanoConn_);
    ODBCDataBaseType databaseType = getDataBaseType();
    if (insertIgnore && databaseType != ODBC_DBT_MYSQL)
        throw IllegalArgumentException("odbc::append", "[PLUGIN:ODBC]: insert ignore can only use in MySQL");

    int inRows = 3000;
    if (databaseType == ODBC_DBT_SQLITE) inRows = 500;
    vector<VectorSP> cols;
    vector<DATA_TYPE> colType;

    for (int i = 0; i < t->columns(); i++) {
        cols.push_back(t->getColumn(i));
        colType.push_back(t->getColumnType(i));
    }

    u16string querySql;

    if (createTable) {
        string createString = "create table " + tableName + "(";
        for (int i = 0; i < t->columns(); i++) {
            if (i) createString += ",";
            if(databaseType == ODBC_DBT_ORACLE && colType[i]==DT_TIMESTAMP) {
                createString += t->getColumnName(i) + " " + sqlTypeName(colType[i], databaseType) + "(9)";
            } else {
                createString += t->getColumnName(i) + " " + sqlTypeName(colType[i], databaseType);
            }
        }
        createString += ")";

        if (databaseType == ODBC_DBT_MYSQL) {
            createString += "DEFAULT CHARSET=utf8";
        } else if (databaseType == ODBC_DBT_CLICKHOUSE) {
            createString += "engine=Log;";
            // std::cout << "use Log as table engine." << std::endl;
        }
        LOG("[PLUGIN:ODBC]: " + createString);
        odbcExecute(tranConn, createString, heap);
    }
    int rows = t->rows();
    int columns = t->columns();
    string sql;
    if (databaseType == ODBC_DBT_ORACLE) {
        INDEX batchSize = 1000000;
        INDEX start = 0;
        vector<char> dataNullVec;
        bool nullPtr[batchSize];
        dataNullVec.reserve(batchSize);

        vector<char *> dataCharPtrVec;
        vector<char> dataCharVec;
        vector<short> dataShortVec;
        vector<int> dataIntVec;
        vector<long long> dataLongVec;
        vector<float> dataFloatVec;
        vector<double> dataDoubleVec;
        vector<string> dataStringVec;
        for (size_t i = 0; i < cols.size(); ++i) {
            switch (cols[i]->getType()) {
                case DT_BOOL:
                case DT_CHAR:
                    dataCharVec.reserve(batchSize);
                    break;
                case DT_SHORT:
                    dataShortVec.reserve(batchSize);
                    break;
                case DT_INT:
                    dataIntVec.reserve(batchSize);
                    break;
                case DT_LONG:
                    dataLongVec.reserve(batchSize);
                    break;
                case DT_DATE:
                case DT_MONTH:
                case DT_TIME:
                case DT_MINUTE:
                case DT_SECOND:
                case DT_DATETIME:
                    dataIntVec.reserve(batchSize);
                    dataStringVec.reserve(batchSize);
                    break;
                case DT_TIMESTAMP:
                case DT_NANOTIME:
                case DT_NANOTIMESTAMP:
                    dataLongVec.reserve(batchSize);
                    dataStringVec.reserve(batchSize);
                    break;
                case DT_FLOAT:
                    dataFloatVec.reserve(batchSize);
                    break;
                case DT_DOUBLE:
                    dataDoubleVec.reserve(batchSize);
                    break;
                case DT_SYMBOL:
                    dataStringVec.reserve(batchSize);
                    break;
                case DT_STRING:
                    dataCharPtrVec.reserve(batchSize);
                    break;
                default:
                    throw RuntimeException("[PLUGIN::ODBC]: Unsupported type " +
                        Util::getDataTypeString(cols[i]->getType()) + " of column " + to_string(i) +".");
            }
        }
        while(start < rows) {
            INDEX end = std::min(start + batchSize, rows);

            nanodbc::statement statement(*nanoConn_);
            string prepareSql = "insert into ";
            prepareSql += tableName;
            prepareSql += " (";
            string questionMarks = "(";
            for (int i = 0; i < t->columns(); i++) {
                if (i) {
                    prepareSql += " ,";
                    questionMarks += " ,";
                }
                prepareSql += t->getColumnName(i);

                if (databaseType == ODBC_DBT_ORACLE) {
                    switch (colType[i]) {
                        case DT_LONG:
                            questionMarks += "TO_NUMBER(?)";
                            break;
                        case DT_DATE:
                            questionMarks += "TO_DATE(?, \'YYYY-MM-DD\')";
                            break;
                        case DT_MONTH:
                            questionMarks += "TO_DATE(?, \'YYYY-MM-DD\')";
                            break;
                        case DT_DATETIME:
                            questionMarks += "TO_DATE(?, \'YYYY-MM-DD HH24:MI:SS')";
                            break;
                        case DT_TIMESTAMP:
                            questionMarks += "TO_TIMESTAMP(?, \'YYYY-MM-DD HH24:MI:SS.FF3\')";
                            break;
                        case DT_NANOTIMESTAMP:
                            questionMarks += "TO_TIMESTAMP(?, \'YYYY-MM-DD HH24:MI:SS.FF9\')";
                            break;
                        default:
                            questionMarks += '?';
                    }
                } else {
                    questionMarks += '?';
                }
            }
            questionMarks += ')';
            prepareSql += ") values ";
            prepareSql += questionMarks;
            prepareSql += "";
            LOG("[PLUGIN:ODBC]: " + prepareSql);
            nanodbc::prepare(statement, utf8_to_utf16(prepareSql));

            const size_t elements = end - start;

            for (size_t i = 0; i < cols.size(); ++i) {
                dataNullVec.clear();
                cols[i]->isNull(start, elements, dataNullVec.data());
                for(size_t j = 0; j < elements; ++j){
                    nullPtr[j] = dataNullVec[j];
                }

                switch (cols[i]->getType()) {
                    case DT_BOOL:
                    case DT_CHAR: {
                        dataCharVec.clear();
                        const char *bindData = cols[i]->getCharConst(start, elements, dataCharVec.data());
                        statement.bind(i, bindData, elements, nullPtr);
                    } break;
                    case DT_SHORT: {
                        dataShortVec.clear();
                        const short *bindData = cols[i]->getShortConst(start, elements, dataShortVec.data());
                        statement.bind(i, bindData, elements, nullPtr);
                    } break;
                    case DT_INT: {
                        dataIntVec.clear();
                        const int *bindData = cols[i]->getIntConst(start, elements, dataIntVec.data());
                        statement.bind(i, bindData, elements, nullPtr);
                    } break;
                    case DT_LONG: {
                        dataLongVec.clear();
                        const long long *bindData = cols[i]->getLongConst(start, elements, dataLongVec.data());
                        if (databaseType == ODBC_DBT_ORACLE) {
                            dataStringVec.clear();
                            for(size_t j = 0; j < elements; ++j) {
                                if(nullPtr[j]) {
                                    dataStringVec.emplace_back("");
                                } else {
                                    dataStringVec.emplace_back(std::to_string(bindData[j]));
                                }
                            }
                            statement.bind_oracle_time(i, dataStringVec);
                        } else {
                            statement.bind(i, bindData, elements, nullPtr);
                        }
                    } break;

                    case DT_DATE: {
                        if (databaseType == ODBC_DBT_ORACLE) {
                            dataStringVec.clear();
                            for (INDEX j = start; j < end; ++j) {
                                string p = cols[i]->getString(j);
                                for (size_t k = 0; k < p.length(); k++) {
                                    if (p[k] == '.') p[k] = '-';
                                }
                                dataStringVec.emplace_back(std::move(p));
                            }
                            statement.bind_oracle_time(i, dataStringVec);
                        } else {
                            nanodbc::date bindData[elements];
                            const int *data = cols[i]->getIntConst(start, elements, dataIntVec.data());

                            for (size_t j = 0; j < elements; ++j) {
                                int value = data[j];
                                int year, month, day;
                                Util::parseDate(value, year, month, day);
                                bindData[j].year = year;
                                bindData[j].month = month;
                                bindData[j].day = day;
                            }
                            statement.bind(i, bindData, elements, nullPtr);
                        }
                    } break;

                    case DT_MONTH: {
                        if (databaseType == ODBC_DBT_ORACLE) {
                            dataIntVec.clear();
                            dataStringVec.clear();
                            const int *data = cols[i]->getIntConst(start, elements, dataIntVec.data());
                            string p;
                            p.reserve(11);
                            for (size_t j = 0; j < elements; ++j) {
                                p += std::to_string(data[j] / 12);
                                p += '-';
                                p += std::to_string(data[j] % 12 + 1);
                                p += "-01";
                                dataStringVec.emplace_back(std::move(p));
                            }
                            statement.bind_oracle_time(i, dataStringVec);
                        } else {
                            nanodbc::date bindData[elements];
                            const int *data = cols[i]->getIntConst(start, elements, dataIntVec.data());

                            for (size_t j = 0; j < elements; ++j) {
                                int value = data[j];
                                int year, month, day;
                                Util::parseDate(value, year, month, day);
                                bindData[j].year = year;
                                bindData[j].month = month;
                                bindData[j].day = 1;
                            }
                            statement.bind(i, bindData, elements, nullPtr);
                        }
                    } break;

                    case DT_TIME: {
                        if (databaseType == ODBC_DBT_ORACLE) {
                            // TODO
                        } else {
                            dataIntVec.clear();
                            nanodbc::time bindData[elements];
                            const int *data = cols[i]->getIntConst(start, elements, dataIntVec.data());
                            for (size_t j = 0; j < elements; ++j) {
                                int value = data[j] / 1000;
                                bindData[j].sec = value % 60;
                                value /= 60;
                                bindData[j].min = value % 60;
                                value /= 60;
                                bindData[j].hour = value;
                            }
                            statement.bind(i, bindData, elements, nullPtr);
                        }
                    } break;

                    case DT_MINUTE: {
                        if (databaseType == ODBC_DBT_ORACLE) {
                            // TODO
                        } else {
                            nanodbc::time bindData[elements];
                            dataIntVec.clear();
                            const int *data = cols[i]->getIntConst(start, elements, dataIntVec.data());
                            for (size_t j = 0; j < elements; ++j) {
                                int value = data[j];
                                bindData[j].sec = 0;
                                value /= 60;
                                bindData[j].min = value % 60;
                                value /= 60;
                                bindData[j].hour = value;
                            }
                            statement.bind(i, bindData, elements, nullPtr);
                        }
                    } break;

                    case DT_SECOND: {
                        if (databaseType == ODBC_DBT_ORACLE) {
                            // TODO
                        } else {
                            nanodbc::time bindData[elements];
                            dataIntVec.clear();
                            const int *data = cols[i]->getIntConst(start, elements, dataIntVec.data());
                            for (size_t j = 0; j < elements; ++j) {
                                int value = data[j];
                                bindData[j].sec = value % 60;
                                value /= 60;
                                bindData[j].min = value % 60;
                                value /= 60;
                                bindData[j].hour = value;
                            }
                            statement.bind(i, bindData, elements, nullPtr);
                        }
                    } break;

                    case DT_DATETIME: {
                        if (databaseType == ODBC_DBT_ORACLE) {
                            dataStringVec.clear();
                            for (INDEX j = start; j < end; ++j) {
                                string p = cols[i]->getString(j);
                                for (size_t k = 0; k < p.length(); k++) {
                                    if (p[k] == '.') p[k] = '-';
                                    if (p[k] == 'T') p[k] = ' ';
                                }
                                dataStringVec.emplace_back(std::move(p));
                            }
                            statement.bind_oracle_time(i, dataStringVec);
                        } else {
                            nanodbc::timestamp bindData[elements];
                            dataIntVec.clear();
                            const int *data = cols[i]->getIntConst(start, elements, dataIntVec.data());

                            for (size_t j = 0; j < elements; ++j) {
                                int value = data[j];
                                bindData[j].fract = 0;
                                bindData[j].sec = value % 60;
                                value /= 60;
                                bindData[j].min = value % 60;
                                value /= 60;
                                bindData[j].hour = value % 24;
                                value /= 24;
                                int year, month, day;
                                Util::parseDate(value, year, month, day);
                                bindData[j].year = year;
                                bindData[j].month = month;
                                bindData[j].day = day;
                            }
                            statement.bind(i, bindData, elements, nullPtr);
                        }
                    } break;

                    case DT_TIMESTAMP: {
                        if (databaseType == ODBC_DBT_ORACLE) {
                            dataStringVec.clear();

                            for (INDEX j = start; j < end; ++j) {
                                string p = cols[i]->getString(j);
                                // std::cout << p << "  ";
                                // std::cout << p.size() << "\n";
                                for (size_t k = 0; k +5 < p.length(); k++) {
                                    if (p[k] == '.')
                                        p[k] = '-';
                                    if (p[k] == 'T') {
                                        p[k] = ' ';
                                    }
                                }
                                dataStringVec.emplace_back(std::move(p));
                            }
                            statement.bind_oracle_time(i, dataStringVec);
                        } else {
                            nanodbc::timestamp bindData[elements];
                            dataLongVec.clear();
                            const long long *data = cols[i]->getLongConst(start, elements, dataLongVec.data());

                            for (size_t j = 0; j < elements; ++j) {
                                long long value = data[j];
                                bindData[j].fract = value % 1000 * 1000000;
                                value /= 1000;
                                bindData[j].sec = value % 60;
                                value /= 60;
                                bindData[j].min = value % 60;
                                value /= 60;
                                bindData[j].hour = value % 24;
                                value /= 24;
                                int year, month, day;
                                Util::parseDate(value, year, month, day);
                                bindData[j].year = year;
                                bindData[j].month = month;
                                bindData[j].day = day;
                            }
                            statement.bind(i, bindData, elements, nullPtr);
                        }
                    } break;
                    case DT_NANOTIME: {
                        if (databaseType == ODBC_DBT_ORACLE) {
                            // TODO
                        } else {
                            nanodbc::time bindData[elements];
                            const long long *data = cols[i]->getLongConst(start, elements, dataLongVec.data());
                            for (size_t j = 0; j < elements; ++j) {
                                long long value = data[j] / 1000000000;
                                bindData[j].sec = value % 60;
                                value /= 60;
                                bindData[j].min = value % 60;
                                value /= 60;
                                bindData[j].hour = value;
                            }
                            statement.bind(i, bindData, elements, nullPtr);
                        }
                    } break;
                    case DT_NANOTIMESTAMP: {
                        if (databaseType == ODBC_DBT_ORACLE) {
                            dataStringVec.clear();
                            for (INDEX j = start; j < end; ++j) {
                                string p = cols[i]->getString(j);
                                for (size_t k = 0; k + 10< p.length(); k++) {
                                    if (p[k] == '.') p[k] = '-';
                                    if (p[k] == 'T') p[k] = ' ';
                                }
                                dataStringVec.emplace_back(std::move(p));
                            }
                            statement.bind_oracle_time(i, dataStringVec);
                        } else {
                            nanodbc::timestamp bindData[elements];
                            dataLongVec.clear();
                            const long long *data = cols[i]->getLongConst(start, elements, dataLongVec.data());

                            for (size_t j = 0; j < elements; ++j) {
                                long long value = data[j];
                                bindData[j].fract = value % 1000000000;
                                value /= 1000000000;
                                bindData[j].sec = value % 60;
                                value /= 60;
                                bindData[j].min = value % 60;
                                value /= 60;
                                bindData[j].hour = value % 24;
                                value /= 24;
                                int year, month, day;
                                Util::parseDate(value, year, month, day);
                                bindData[j].year = year;
                                bindData[j].month = month;
                                bindData[j].day = day;
                            }
                            statement.bind(i, bindData, elements, nullPtr);
                        }
                    } break;
                    case DT_FLOAT: {
                        dataFloatVec.clear();
                        const float *bindData = cols[i]->getFloatConst(start, elements, dataFloatVec.data());
                        statement.bind(i, bindData, elements, nullPtr);
                    } break;
                    case DT_DOUBLE: {
                        dataDoubleVec.clear();
                        const double *bindData = cols[i]->getDoubleConst(start, elements, dataDoubleVec.data());
                        statement.bind(i, bindData, elements, nullPtr);
                    } break;
                    case DT_SYMBOL: {
                        dataStringVec.clear();
                        for (INDEX j = start; j < end; ++j) {
                            dataStringVec.emplace_back(cols[i]->getString(j));
                        }
                        statement.bind_strings(i, dataStringVec);
                    } break;
                    case DT_STRING: {
                        dataCharPtrVec.clear();
                        dataStringVec.clear();
                        char **bindData = cols[i]->getStringConst(start, elements, dataCharPtrVec.data());
                        for (size_t j = 0; j < elements; ++j) {
                            dataStringVec.emplace_back(bindData[j]);
                        }
                        statement.bind_strings(i, dataStringVec);
                    } break;

                    default:
                        throw RuntimeException("[PLUGIN::ODBC]: Unsupported type " +
                            Util::getDataTypeString(cols[i]->getType()) + " of column " + to_string(i) +".");
                }
            }
            transact(statement, elements);
            start = end;
        }

    } else {
        if (insertIgnore)
            sql = "INSERT IGNORE INTO " + tableName + " VALUES ";
        else
            sql = "INSERT INTO " + tableName + " VALUES ";

        for (int row = 0; row < rows; row++) {
            string insertString = "(";
            for (int col = 0; col < columns; col++) {
                if (col > 0) insertString += ",";
                insertString += getValueStr(cols[col]->getString(row), colType[col], databaseType);
            }
            insertString += ")";
            if (row % inRows != inRows - 1) { insertString += ","; }

            sql += insertString;
            if (row % inRows == inRows - 1) {
                sql += ";";
                // cout << q << endl;
                odbcExecute(tranConn, sql, heap);
                if (insertIgnore)
                    sql = "INSERT IGNORE INTO " + tableName + " VALUES ";
                else
                    sql = "INSERT INTO " + tableName + " VALUES ";
            }
        }
        if (rows % inRows != 0) {
            if (databaseType == ODBC_DBT_ORACLE) {
                sql += " select * from dual;";
            } else {
                sql[sql.length() - 1] = ';';
            }
            odbcExecute(tranConn, sql, heap);
        }
    }

    try {
        tranConn.commit();
    } catch (const runtime_error &e) {
        throw RuntimeException(string("[PLUGIN::ODBC]: Failed to append data : ") + e.what());
    }
    return Util::createConstant(DT_VOID);
}


ConstantSP OdbcConnection::odbcQuery(Heap* heap, const TableSP& schemaTable, const FunctionDefSP& transform, int batchSize, const string& querySql){
    LockGuard<Mutex> lockGuard(&lock);
    if(closed_)
        throw RuntimeException("[PLUGIN::ODBC]: odbc connection is closed. ");
    const static int nanodbc_rowset_size = 1;
    u16string querySqlU16 = utf8_to_utf16(querySql);
    if (querySqlU16 == u"")
        throw IllegalArgumentException("odbc::query", "[PLUGIN::ODBC]: querySql can't be an empty string.");

    nanodbc::result results;
    nanodbc::statement hStmt;
    try {
        results = hStmt.execute_direct(*nanoConn_, querySqlU16, nanodbc_rowset_size);
    } catch (const runtime_error &e) {
        string content = "[PLUGIN::ODBC]: Executed query [" + querySql + "]: " + e.what();
        if (content.find("Syntax error") != string::npos &&
            content.find("query, subquery, possibly with UNION, SELECT subquery, SELECT") != string::npos) {
            this->close(false);
        }
        throw TableRuntimeException(content);
    }

    const short columns = results.columns();
    vector<ConstantSP> columnVecs;
    vector<std::string> colNames;

    TableSP appendTable = odbcGetOrCreateTable(heap, schemaTable, results, columnVecs, colNames, transform.isNull());

    vector<Vector *> arrCol(columns);
    vector<DATA_TYPE> columnTypes(columns);
    for (short i = 0; i < columns; ++i) {
        arrCol[i] = (Vector *)columnVecs[i].get();
        columnTypes[i] = arrCol[i]->getType();
    }

    const int BUF_SIZE = batchSize == -1 ? 256 * 1024 : batchSize;
    vector<vector<U8ForStringPoint>> buffers =
        vector<vector<U8ForStringPoint>>(columns, vector<U8ForStringPoint>(BUF_SIZE));
    int curLine = 0;
    vector<char> charVector(BUF_SIZE);
    vector<short> shortVector(BUF_SIZE);
    vector<U4> U4Vector(BUF_SIZE);
    char *charBuf = charVector.data();
    short *shortBuf = shortVector.data();
    U4 *buf = U4Vector.data();
    bool hasNext = results.next();
    while (hasNext) {
        for (short col = 0; col < columns; ++col) {
            // nanodbc stores numeric & decimal types as strings
            if (results.column_datatype(col) == SQL_DECIMAL || results.column_datatype(col) == SQL_NUMERIC) {
                u16string _str;
                results.get_ref<u16string>(col, u"", _str);
                string str = utf16_to_utf8(_str);
                if (str != "")
                    try{
                        buffers[col][curLine].doubleVal = stod(str);
                    }catch(...){
                        buffers[col][curLine].doubleVal = DBL_NMIN;
                    }
                else
                    buffers[col][curLine].doubleVal = DBL_NMIN;
            } else {
                switch (columnTypes[col]) {
                    case DT_BOOL:
                        buffers[col][curLine].charVal = results.get<char>(col, CHAR_MIN);
                        if (buffers[col][curLine].charVal != CHAR_MIN) {
                            buffers[col][curLine].charVal = !!buffers[col][curLine].charVal;
                        }
                        break;
                    case DT_CHAR: buffers[col][curLine].charVal = results.get<char>(col, CHAR_MIN); break;
                    case DT_SHORT: buffers[col][curLine].shortVal = results.get<short>(col, SHRT_MIN); break;
                    case DT_INT: buffers[col][curLine].intVal = results.get<int>(col, INT_MIN); break;
                    case DT_LONG: buffers[col][curLine].longVal = results.get<int64_t>(col, LLONG_MIN); break;
                    case DT_FLOAT: buffers[col][curLine].floatVal = results.get<float>(col, FLT_NMIN); break;
                    case DT_DOUBLE:
                        buffers[col][curLine].doubleVal = results.get<double>(col, DBL_NMIN);
                        break;
                    case DT_DATE: {
                        nanodbc::date fallback = {-1, -1, -1};
                        nanodbc::date date = {0, 0, 0};
                        results.get_ref<nanodbc::date>(col, fallback, date);
                        if (memcmp(&date, &fallback, sizeof(nanodbc::date)) == 0) {
                            buffers[col][curLine].intVal = INT_MIN;
                        } else {
                            buffers[col][curLine].intVal = Util::countDays(date.year, date.month, date.day);
                        }
                    } break;
                    case DT_TIMESTAMP: {
                        nanodbc::timestamp fallback = {-1, -1, -1, -1, -1, -1, -1};
                        nanodbc::timestamp ts = {0, 0, 0, 0, 0, 0, 0};
                        results.get_ref<nanodbc::timestamp>(col, fallback, ts);
                        if (memcmp(&ts, &fallback, sizeof(nanodbc::timestamp)) == 0) {
                            buffers[col][curLine].longVal = LLONG_MIN;
                        } else {
                            // ts.fract is the number of billionths of a second.
                            // https://docs.microsoft.com/en-us/sql/odbc/reference/appendixes/c-data-types
                            buffers[col][curLine].longVal =
                                Util::countDays(ts.year, ts.month, ts.day) * 86400000ll +
                                ((ts.hour * 60 + ts.min) * 60 + ts.sec) * 1000 + ts.fract / 1000000;
                        }
                    } break;
                    case DT_SECOND: {
                        nanodbc::time fallback = {-1, -1, -1};
                        nanodbc::time ts = {0, 0, 0};
                        results.get_ref<nanodbc::time>(col, fallback, ts);
                        if (memcmp(&ts, &fallback, sizeof(nanodbc::time)) == 0) {
                            buffers[col][curLine].intVal = INT_MIN;
                        } else {
                            buffers[col][curLine].intVal = (ts.hour * 60 + ts.min) * 60 + ts.sec;
                        }
                    } break;
                    case DT_TIME: {
                        nanodbc::time fallback = {-1, -1, -1};
                        nanodbc::time ts = {0, 0, 0};
                        results.get_ref<nanodbc::time>(col, fallback, ts);
                        if (memcmp(&ts, &fallback, sizeof(nanodbc::time)) == 0) {
                            buffers[col][curLine].intVal = INT_MIN;
                        } else {
                            // ts.fract is the number of billionths of a second.
                            // https://docs.microsoft.com/en-us/sql/odbc/reference/appendixes/c-data-types
                            buffers[col][curLine].intVal = ((ts.hour * 60 + ts.min) * 60 + ts.sec) * 1000;
                        }
                    } break;
                    case DT_NANOTIME: {
                        nanodbc::timestamp fallback = {-1, -1, -1, -1, -1, -1, -1};
                        nanodbc::timestamp ts = {0, 0, 0, 0, 0, 0, 0};
                        results.get_ref<nanodbc::timestamp>(col, fallback, ts);
                        if (memcmp(&ts, &fallback, sizeof(nanodbc::timestamp)) == 0) {
                            buffers[col][curLine].longVal = LLONG_MIN;
                        } else {
                            // ts.fract is the number of billionths of a second.
                            // https://docs.microsoft.com/en-us/sql/odbc/reference/appendixes/c-data-types
                            buffers[col][curLine].longVal =
                                ((ts.hour * 60 + ts.min) * 60 + ts.sec) * 1000000000ll + ts.fract;
                        }
                    } break;
                    case DT_NANOTIMESTAMP: {
                        nanodbc::timestamp fallback = {-1, -1, -1, -1, -1, -1, -1};
                        nanodbc::timestamp ts = {0, 0, 0, 0, 0, 0, 0};
                        results.get_ref<nanodbc::timestamp>(col, fallback, ts);
                        if (memcmp(&ts, &fallback, sizeof(nanodbc::timestamp)) == 0) {
                            buffers[col][curLine].longVal = LLONG_MIN;
                        } else {
                            // ts.fract is the number of billionths of a second.
                            // https://docs.microsoft.com/en-us/sql/odbc/reference/appendixes/c-data-types
                            buffers[col][curLine].longVal =
                                Util::countDays(ts.year, ts.month, ts.day) * 86400000000000ll +
                                ((ts.hour * 60 + ts.min) * 60 + ts.sec) * 1000000000ll + ts.fract;
                        }
                    } break;
                    case DT_SYMBOL: {
                        SymbolBaseSP symbolBase = arrCol[col]->getSymbolBase();
                        u16string _str;
                        results.get_ref<u16string>(col, u"", _str);
                        string str = utf16_to_utf8(_str);
                        buffers[col][curLine].intVal = symbolBase->findAndInsert(str);
                    } break;
                    default:
                        if (SQL_C_BINARY == results.column_c_datatype(col)) {
                            vector<uint8_t> ret = results.get<std::vector<std::uint8_t>>(col);
                            buffers[col][curLine].pointer =
                                new string(reinterpret_cast<char *>(ret.data()), ret.size());
                            // buffers[col][curLine].pointer = new char[ret.size() + 1];
                            // memcpy(buffers[col][curLine].pointer, ret.data(), ret.size());
                            // buffers[col][curLine].pointer[ret.size()] = '\0';
                        } else {
                            u16string _str;
                            results.get_ref<u16string>(col, u"", _str);
                            string str = utf16_to_utf8(_str);

                            buffers[col][curLine].pointer = new string(str);
                            // buffers[col][curLine].pointer = new char[str.size() + 1];
                            //  if (str == "") {
                            //    *(buffers[col][curLine].pointer) = '\0';
                            //  } else {
                            //    strcpy(buffers[col][curLine].pointer, str.c_str());
                            //  }
                        }
                }
            }
        }
        ++curLine;
        hasNext = results.next();
        if (!hasNext || curLine == BUF_SIZE) {
            for (short col = 0; col < columns; ++col) {
                DATA_TYPE rawType = arrCol[col]->getRawType();
                int sqlType = results.column_datatype(col);
                vector<U8ForStringPoint> &colBuf = buffers[col];
                switch (rawType) {
                    case DT_BOOL:
                        if (sqlType == SQL_DECIMAL || sqlType == SQL_NUMERIC) {
                            for (int j = 0; j < curLine; ++j)
                                charBuf[j] =
                                    colBuf[j].doubleVal == DBL_NMIN ? CHAR_MIN : (bool)colBuf[j].doubleVal;
                        } else {
                            for (int j = 0; j < curLine; ++j) charBuf[j] = colBuf[j].charVal;
                        }
                        arrCol[col]->appendBool(charBuf, curLine);
                        break;
                    case DT_CHAR:
                        if (sqlType == SQL_DECIMAL || sqlType == SQL_NUMERIC) {
                            for (int j = 0; j < curLine; ++j)
                                charBuf[j] = colBuf[j].doubleVal == DBL_NMIN ? CHAR_MIN : colBuf[j].doubleVal;
                        } else {
                            for (int j = 0; j < curLine; ++j) charBuf[j] = colBuf[j].charVal;
                        }
                        arrCol[col]->appendChar(charBuf, curLine);
                        break;
                    case DT_SHORT:
                        if (sqlType == SQL_DECIMAL || sqlType == SQL_NUMERIC) {
                            for (int j = 0; j < curLine; ++j)
                                shortBuf[j] =
                                    colBuf[j].doubleVal == DBL_NMIN ? SHRT_MIN : colBuf[j].doubleVal;
                        } else {
                            for (int j = 0; j < curLine; ++j) shortBuf[j] = colBuf[j].shortVal;
                        }
                        arrCol[col]->appendShort(shortBuf, curLine);
                        break;
                    case DT_INT:
                        if (sqlType == SQL_DECIMAL || sqlType == SQL_NUMERIC) {
                            for (int j = 0; j < curLine; ++j)
                                buf[j].intVal =
                                    colBuf[j].doubleVal == DBL_NMIN ? INT_MIN : colBuf[j].doubleVal;
                        } else {
                            for (int j = 0; j < curLine; ++j) buf[j].intVal = colBuf[j].intVal;
                        }

                        arrCol[col]->appendInt(reinterpret_cast<int *>(buf), curLine);
                        break;
                    case DT_LONG:
                        if (sqlType == SQL_DECIMAL || sqlType == SQL_NUMERIC) {
                            for (int j = 0; j < curLine; ++j)
                                colBuf[j].longVal =
                                    colBuf[j].doubleVal == DBL_NMIN ? LONG_LONG_MIN : colBuf[j].doubleVal;
                        }
                        arrCol[col]->appendLong(reinterpret_cast<long long int *>(colBuf.data()), curLine);
                        break;
                    case DT_FLOAT:
                        if (sqlType == SQL_DECIMAL || sqlType == SQL_NUMERIC) {
                            for (int j = 0; j < curLine; ++j)
                                buf[j].floatVal =
                                    colBuf[j].doubleVal == DBL_NMIN ? FLT_NMIN : colBuf[j].doubleVal;
                        } else {
                            for (int j = 0; j < curLine; ++j) buf[j].floatVal = colBuf[j].floatVal;
                        }
                        arrCol[col]->appendFloat(reinterpret_cast<float *>(buf), curLine);
                        break;
                    case DT_DOUBLE:
                        arrCol[col]->appendDouble(reinterpret_cast<double *>(colBuf.data()), curLine);
                        break;
                    case DT_STRING:
                    case DT_BLOB:
                        // The processing for 64-bit is the same as for 32-bit.
                        //
                        //  #ifdef BIT32
                        //              if (sqlType == SQL_DECIMAL || sqlType == SQL_NUMERIC) {
                        //                for (int j = 0; j < curLine; ++j) {
                        //                  char strBuf[120];
                        //                  int nbytes = snprintf(strBuf, sizeof(strBuf), "%lf",
                        //                                        colBuf[j].doubleVal);
                        //                  if (colBuf[j].doubleVal != DBL_NMIN) {
                        //                    buf[j].pointer = new char[nbytes + 1];
                        //                    strcpy(buf[j].pointer, strBuf);
                        //                  } else {
                        //                    buf[j].pointer = new char[1];
                        //                    (*buf[j].pointer) = '\0'
                        //                  }
                        //                }
                        //              } else {
                        //                for (int j = 0; j < curLine; ++j)
                        //                  buf[j].pointer = colBuf[j].pointer;
                        //              }
                        //              arrCol[col]->appendString((const char**)buf, curLine);
                        //              for (int j = 0; j < curLine; ++j) delete[] buf[j].pointer;
                        //  #else
                        // If it's a String, we've already saved the data in Pointer. So we don't need this
                        // step.
                        //  if (sqlType == SQL_DECIMAL || sqlType == SQL_NUMERIC) {
                        //    for (int j = 0; j < curLine; ++j) {
                        //      char strBuf[120];
                        //      int nbytes = snprintf(strBuf, sizeof(strBuf), "%lf",
                        //                            colBuf[j].doubleVal);
                        //      if (colBuf[j].doubleVal != DBL_NMIN) {
                        //        buffers[col][j].pointer = new char[nbytes + 1];
                        //        strcpy(buffers[col][j].pointer, strBuf);
                        //      } else {
                        //        buffers[col][j].pointer = new char[1];
                        //        *(buffers[col][j].pointer) = '\0';
                        //      }
                        //    }
                        //  }
                        {
                            vector<string> stringBuffer;
                            stringBuffer.reserve(curLine);
                            for (int j = 0; j < curLine; ++j)
                                stringBuffer.push_back((*(buffers[col][j].pointer)));
                            arrCol[col]->appendString(stringBuffer.data(), curLine);
                            for (int j = 0; j < curLine; ++j) delete (buffers[col][j].pointer);
                        }
                        // #endif
                        break;
                    default:
                        throw RuntimeException("[PLUGIN:ODBC]: The DolphinDB type " +
                                            Util::getDataTypeString(rawType) + " is not supported. ");
                }
            }
            curLine = 0;
            for (short i = 0; i < columns; ++i) { arrCol[i]->setNullFlag(arrCol[i]->hasNull()); }

            if (arrCol[0]->size() > 0) {
                TableSP tmpTable = Util::createTable(colNames, columnVecs);
                if (!transform.isNull()) {
                    vector<ConstantSP> arg{tmpTable};
                    try{
                        tmpTable = transform->call(heap, arg);
                    }catch(exception &e){
                        throw RuntimeException("[PLUGIN::ODBC]: call transform error: " + string(e.what()));
                    }catch(...){
                        throw RuntimeException("[PLUGIN::ODBC]: call transform error. ");
                    }
                }
                vector<ConstantSP> _{appendTable, tmpTable};
                static const FunctionDefSP func = heap->currentSession()->getFunctionDef("append!");
                try{
                        func->call(heap, _);
                }catch(exception &e){
                    throw RuntimeException("[PLUGIN::ODBC]: append error: " + string(e.what()));
                }catch(...){
                        throw RuntimeException("[PLUGIN::ODBC]: call transform error. ");
                }

            }

            for (short i = 0; i < columns; ++i) { arrCol[i]->clear(); }
        }
    }
    return appendTable;
}

unordered_map<string, ODBCDataBaseType> OdbcConnection::DBT_MAP = {{"", ODBC_DBT_VOID},
                                                                   {"mysql", ODBC_DBT_MYSQL},
                                                                   {"sqlserver", ODBC_DBT_SQLSERVER},
                                                                   {"sqlite", ODBC_DBT_SQLITE},
                                                                   {"clickhouse", ODBC_DBT_CLICKHOUSE},
                                                                   {"oracle", ODBC_DBT_ORACLE},
                                                                   {"postgresql", ODBC_DBT_POSTGRSQL}};

Mutex OdbcConnection::ODBC_PLUGIN_LOCK;
unordered_map<long long, OdbcConnectionSP> OdbcConnection::ODBC_CONN_MAP;