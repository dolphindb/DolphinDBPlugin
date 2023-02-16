/*
 * DolphinDBODBC.cpp
 *
 *  Created on: May 26, 2017
 *      Author: xinjing.zhou
 */
#include "DolphinDBODBC.h"
#include <map>
#include <algorithm>

// std::ifstream typeTran;
// std::string typeTranLog = "./plugins/odbc/typeTran.txt";

using namespace std;

static map<long long, bool> connMap;
static Mutex odbcMutex;

static void odbcConnectionOnClose(Heap* heap, vector<ConstantSP>& args) {
  OdbcConnection* cp = (OdbcConnection*)(args[0]->getLong());
  if (cp != nullptr) {

    LockGuard<Mutex> guard(&odbcMutex);
    connMap.erase(args[0]->getLong());

    delete cp;
    args[0]->setLong(0);
  }
}

nanodbc::connection* safeGet(const ConstantSP& arg) {
  if (arg->getType() == DT_RESOURCE && ((OdbcConnection *)(arg->getLong()) != nullptr)) {
      auto conn = (OdbcConnection *)(arg->getLong());
      nanodbc::connection* nanoConn = conn->getConnection();
      if (nanoConn->connected()) {
        return nanoConn;
      } else {
        throw RuntimeException("The odbc connection has been closed.");
      }
  } else {
    throw RuntimeException("Invalid odbc connection.");
  }
}

static const int useStringSizeThreshold = 30;

static inline DATA_TYPE sqlType2DataType(int sqlType, long colSize, int cType) {
  // cout << sqlType << " " << colSize << " " << cType << endl;
  // https://docs.microsoft.com/en-us/sql/odbc/reference/appendixes/c-data-types
  switch (sqlType) {
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
    case SQL_TIME:
    case SQL_TYPE_TIME:
      return DT_SECOND;
    case SQL_TIMESTAMP:
    case SQL_TYPE_TIMESTAMP:
      return DT_NANOTIMESTAMP;
    case SQL_IS_SMALLINT: // aka char
    case SQL_CHAR:
      if (colSize == 1) {
        return DT_CHAR;
      }
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

static bool compatible(DATA_TYPE dolphinType, int sqlType, int colSize) {
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
        case SQL_DECIMAL:
          return true;
        case SQL_IS_SMALLINT:
        case SQL_CHAR:
          if (colSize == 1) return true;
        default:
          return false;
      }
    case DT_DATE:
    case DT_TIMESTAMP:
    case DT_NANOTIMESTAMP:
      switch (sqlType) {
        case SQL_DATE:
        case SQL_TYPE_DATE: 
        case SQL_TIMESTAMP:
        case SQL_TYPE_TIMESTAMP:
          return true;
        default:
          return false;
      }
    case DT_SECOND:
    case DT_TIME:
    case DT_NANOTIME:
      switch (sqlType) {
        case SQL_TYPE_TIME:
        case SQL_TIME:
          return true;
        default:
          return false;
      }
    case DT_STRING:
    case DT_SYMBOL:
    case DT_BLOB:
      return true;
    default:
      return false;
  }
}

// creates a new native odbc connection if the first argument is a connection
// string. returns the native odbc connection if the first argument is a
// DT_RESOURCE type.
static ConstantSP odbcGetConnection(Heap* heap, vector<ConstantSP>& args,
                                    const string& funcName) {
  if (args[0]->isScalar() && args[0]->getType() == DT_STRING) {
    u16string connStr = utf8_to_utf16(args[0]->getString());
    string dataBaseType;
    if(args.size() >= 2){
      if(args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(
            funcName,
            "DataBaseType must be a string scalar. ");
      dataBaseType = args[1]->getString();
      std::transform(dataBaseType.begin(), dataBaseType.end(), dataBaseType.begin(), ::tolower);

      if(dataBaseType != "postgresql" && dataBaseType != "mysql" 
         && dataBaseType != "sqlserver" && dataBaseType != "clickhouse"
         && dataBaseType != "sqlite" && dataBaseType != "oracle")
        throw IllegalArgumentException(
            funcName,
            "DataBaseType must be PostgreSQL, SQLServer, MySQL, ClickHouse, SQLite, or Oracle. ");
    }
    try {
      unique_ptr<OdbcConnection> cup(new OdbcConnection(nanodbc::connection(connStr), dataBaseType));
      const char* fmt = "odbc connection to [%s]";
      vector<char> descBuf(connStr.size() + strlen(fmt));
	  sprintf(descBuf.data(), fmt, connStr.c_str());
      FunctionDefSP onClose(Util::createSystemProcedure(
          "odbc connection onClose()", odbcConnectionOnClose, 1, 1));
      ConstantSP res =
          Util::createResource((long long)cup.release(), descBuf.data(),
                               onClose, heap->currentSession());
      
	  {
      LockGuard<Mutex> guard(&odbcMutex);
      connMap[res->getLong()] = true;
	  }

      if(dataBaseType == "clickhouse") {
		// can't use new String("select 1") for undefined error
		ConstantSP select1 = Util::createConstant(DT_STRING);
        select1->setString("select 1");
        vector<ConstantSP> qArgs = {res, select1};
        ConstantSP ret = odbcQuery(heap, qArgs);
      }
      return res;
    } catch (const runtime_error& e) {
      const char* fmt = "error connecting to [%s]: %s";
      vector<char> errorMsgBuf(connStr.size() + strlen(e.what()) + strlen(fmt));
      sprintf(errorMsgBuf.data(), fmt, connStr.c_str(), e.what());
      throw TableRuntimeException(errorMsgBuf.data());
    }
  } else if (args[0]->getType() == DT_RESOURCE) {
    return args[0];
  } else {
    throw IllegalArgumentException(
        funcName,
        "Unknown argument type, must be a connection string or odbc connection "
        "handle");
  }
}

void getColNames(const nanodbc::result& results, vector<std::string>& columnNames){
  short columns = results.columns();
  for (short i = 0; i < columns; ++i) {
    columnNames[i] = utf16_to_utf8(results.column_name(i)); 
  }
}

void getColVecAndType(const nanodbc::result& results, vector<ConstantSP>& columnVecs, vector<DATA_TYPE>& columnTypes){
  int symbolCount = 0;
  short columns = results.columns();
  for (short i = 0; i < columns; ++i) {
      columnTypes[i] =
          sqlType2DataType(results.column_datatype(i), results.column_size(i), results.column_c_datatype(i));
      if (columnTypes[i] == DT_SYMBOL) {
        symbolCount++;
      }
  }
  // multiple columns share the same symbol base
  SymbolBaseSP symbolBaseSP = nullptr;
  if (symbolCount > 0) {
    symbolBaseSP = new SymbolBase();
  }
  for (short i = 0; i < columns; ++i) {
    if (columnTypes[i] == DT_SYMBOL) {
      columnVecs[i] = Util::createSymbolVector(symbolBaseSP, 0);
    } else {
      columnVecs[i] = Util::createVector(columnTypes[i], 0);
    }
  }
}

static TableSP odbcGetOrCreateTable(const Heap* heap,
                                    const vector<ConstantSP>& args,
                                    const nanodbc::result& results,
                                    vector<ConstantSP>& columnVecs,
                                    vector<string>& colNames,
                                    bool compatibility) {
  int symbolCount = 0;
  getColNames(results, colNames);
  if (args.size() >= 3 && args[2]->getType() != DT_VOID) {
    if (args[2]->getForm() != DF_TABLE) {
      throw TableRuntimeException("The 3rd argument must be a table");
    }
    TableSP tblSP(args[2]);
    if(compatibility){
      INDEX columns = tblSP->columns();
      if (columns != results.columns()) {
        throw TableRuntimeException(
            "The given table schema is incompatible with the returned table from "
            "ODBC (different column count)");
      }
      for (INDEX i = 0; i < columns; ++i) {
      
        DATA_TYPE dolphinType = tblSP->getColumnType(i);
        if (!compatible(dolphinType, results.column_datatype(i),
                        results.column_size(i))) {
          const char* fmt =
              "The given table schema is incompatible with the returned table "
              "from ODBC at column %d[%s]";
          vector<char> descBuf(10 + strlen(fmt) + results.column_name(i).size());
          sprintf(descBuf.data(), fmt, i, results.column_name(i).c_str());
          throw TableRuntimeException(descBuf.data());
        }
        if (dolphinType == DT_SYMBOL) {
          //                if(!tblSP->isSegmentedTable())
          columnVecs[i] = Util::createSymbolVector(nullptr, 0);
          symbolCount++;
        } else {
          columnVecs[i] = Util::createVector(dolphinType, 0);
        }
      }
    }else{
      short columns = results.columns();
      vector<DATA_TYPE> columnTypes(columns);
      getColVecAndType(results, columnVecs, columnTypes);
    }
    return tblSP;
  } else {
    short columns = results.columns();
    vector<DATA_TYPE> columnTypes(columns);

    getColVecAndType(results, columnVecs, columnTypes);
    return Util::createTable(colNames, columnTypes, 0, 1);
  }
}

ConstantSP odbcClose(Heap* heap, vector<ConstantSP>& args) {
  assert(heap);
  assert(args.size() == 1);
  if (args[0]->getType() != DT_RESOURCE) {
    throw IllegalArgumentException(
        "odbc::close",
        "Unknown argument type, must be an odbc connection handle");
  }
  ConstantSP csp = odbcGetConnection(heap, args, "odbc::close");
  nanodbc::connection* cp = safeGet(csp);
  cp->disconnect();
  cp->deallocate();

  OdbcConnection* cspConn = (OdbcConnection*)(csp->getLong());
  if (cspConn != nullptr) {

    LockGuard<Mutex> guard(&odbcMutex);
    connMap.erase(csp->getLong());

    delete cspConn;
    csp->setLong(0);
  }
  // can't use Expression::void_ for undefined error
  return Util::createConstant(DT_VOID);
}

ConstantSP odbcConnect(Heap* heap, vector<ConstantSP>& args) {
  if (!heap)
	throw RuntimeException("heap can't be null.");
  if (args.size() > 2)
	throw RuntimeException("arguments size can't greater than 2.");
  if (args[0]->getType() != DT_STRING) {
    throw IllegalArgumentException(
        "odbc::connect",
        "Unknown argument type, must be an odbc connection string");
  }
  return odbcGetConnection(heap, args, "odbc::connect");
}

ConstantSP odbcQuery(Heap* heap, vector<ConstantSP>& args) {
  const static int nanodbc_rowset_size = 1;
  if (!heap)
	throw RuntimeException("heap can't be null.");
  if (args.size() < 2)
	throw RuntimeException("arguments size can't less than 2.");
  FunctionDefSP tranform;
  int batchSize = -1;
  if(args.size() >= 4 && !args[3]->isNothing()){
    if(args[3]->getForm() != DF_SCALAR || args[3]->getType() != DT_INT)
      throw RuntimeException("batchSize must be a int scalar. ");
      batchSize = args[3]->getInt();
      if(batchSize <= 0)
        throw RuntimeException("batchSize must be greater than 0. ");
  }

  if(args.size() >= 5){
    if(args[4]->getForm() != DF_SCALAR || args[4]->getType() != DT_FUNCTIONDEF)
      throw RuntimeException("tranform must be a fuction scalar. ");
      tranform = args[4];
  }

  u16string querySql = utf8_to_utf16(args[1]->getString());
  vector<ConstantSP> connArgs;
  connArgs.emplace_back(args[0]);
  ConstantSP csp = odbcGetConnection(heap, connArgs, "odbc::query");
  nanodbc::connection* cp = safeGet(csp);
  
  nanodbc::result results;
  nanodbc::statement hStmt;
  try {
     results = hStmt.execute_direct(*cp, querySql, nanodbc_rowset_size);

  } catch (const runtime_error& e) {
	const char* fmt = "Executed query [%s]: %s";
    vector<char> errorMsgBuf(args[1]->getString().size() + strlen(e.what()) + strlen(fmt));
    sprintf(errorMsgBuf.data(), fmt, args[1]->getString().c_str(), e.what());
    string content = string(errorMsgBuf.data());

    vector<ConstantSP> closeArg = {csp};
    if(content.find("Syntax error") != string::npos && content.find("query, subquery, possibly with UNION, SELECT subquery, SELECT") != string::npos) {
      odbcConnectionOnClose(heap, closeArg);
    }
    throw TableRuntimeException(content);
  }

  const short columns = results.columns();
  vector<ConstantSP> columnVecs(columns);
  vector<Vector*> arrCol(columns);
  vector<DATA_TYPE> columnTypes(columns);
  vector<std::string> colNames(columns);

  TableSP table = odbcGetOrCreateTable(heap, args, results, columnVecs, colNames, tranform.isNull());
  for (short i = 0; i < columns; ++i) {
    arrCol[i] = (Vector*)columnVecs[i].get();
    columnTypes[i] = arrCol[i]->getType();
  }

  const int BUF_SIZE = batchSize == -1 ? 256 * 1024 : batchSize;
  vector<vector<U8>> buffers =
      vector<vector<U8>>(columns, vector<U8>(BUF_SIZE));
  int curLine = 0;
  vector<char> charVector(BUF_SIZE);
  vector<short> shortVector(BUF_SIZE);
  vector<U4> U4Vector(BUF_SIZE);
  char* charBuf = charVector.data();
  short* shortBuf = shortVector.data();
  U4* buf = U4Vector.data();
  bool hasNext = results.next();
  while (hasNext) {
    for (short col = 0; col < columns; ++col) {
      // nanodbc stores numeric & decimal types as strings
      if (results.column_datatype(col) == SQL_DECIMAL ||
          results.column_datatype(col) == SQL_NUMERIC) {
        u16string _str;
        results.get_ref<u16string>(col, u"", _str);
        string str = utf16_to_utf8(_str);
        if (str != "")
          sscanf(str.c_str(), "%lf", &(buffers[col][curLine].doubleVal));
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
          case DT_CHAR:
            buffers[col][curLine].charVal = results.get<char>(col, CHAR_MIN);
            break;
          case DT_SHORT:
            buffers[col][curLine].shortVal = results.get<short>(col, SHRT_MIN);
            break;
          case DT_INT:
            buffers[col][curLine].intVal = results.get<int>(col, INT_MIN);
            break;
          case DT_LONG:
            buffers[col][curLine].longVal =
                results.get<int64_t>(col, LLONG_MIN);
            break;
          case DT_FLOAT:
            buffers[col][curLine].floatVal = results.get<float>(col, FLT_NMIN);
            break;
          case DT_DOUBLE:
            buffers[col][curLine].doubleVal =
                results.get<double>(col, DBL_NMIN);
            break;
          case DT_DATE: {
            nanodbc::date fallback = {-1, -1, -1};
            nanodbc::date date = {0, 0, 0};
            results.get_ref<nanodbc::date>(col, fallback, date);
            if (memcmp(&date, &fallback, sizeof(nanodbc::date)) == 0) {
              buffers[col][curLine].intVal = INT_MIN;
            } else {
              buffers[col][curLine].intVal =
                  Util::countDays(date.year, date.month, date.day);
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
                  ((ts.hour * 60 + ts.min) * 60 + ts.sec) * 1000 +
                  ts.fract / 1000000;
            }
          } break;
          case DT_SECOND: {
            nanodbc::time fallback = {-1, -1, -1};
            nanodbc::time ts = {0, 0, 0};
            results.get_ref<nanodbc::time>(col, fallback, ts);
            if (memcmp(&ts, &fallback, sizeof(nanodbc::time)) == 0) {
              buffers[col][curLine].intVal = INT_MIN;
            } else {
              buffers[col][curLine].intVal =
                  (ts.hour * 60 + ts.min) * 60 + ts.sec;
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
              buffers[col][curLine].intVal =
                  ((ts.hour * 60 + ts.min) * 60 + ts.sec) * 1000;
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
                  ((ts.hour * 60 + ts.min) * 60 + ts.sec) * 1000000000ll +
                  ts.fract;
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
                  Util::countDays(ts.year, ts.month, ts.day) *
                      86400000000000ll +
                  ((ts.hour * 60 + ts.min) * 60 + ts.sec) * 1000000000ll +
                  ts.fract;
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
            if(SQL_C_BINARY == results.column_c_datatype(col)){
              vector<uint8_t> ret = results.get<std::vector<std::uint8_t>>(col);
              buffers[col][curLine].pointer = (char*)(new string((const char*)ret.data(), ret.size()));
              // buffers[col][curLine].pointer = new char[ret.size() + 1];
              // memcpy(buffers[col][curLine].pointer, ret.data(), ret.size());
              // buffers[col][curLine].pointer[ret.size()] = '\0';
            }else{
              u16string _str;
              results.get_ref<u16string>(col, u"", _str);
              string str = utf16_to_utf8(_str);

              buffers[col][curLine].pointer = (char*)(new string(str));
              //buffers[col][curLine].pointer = new char[str.size() + 1];
              // if (str == "") {
              //   *(buffers[col][curLine].pointer) = '\0';
              // } else {
              //   strcpy(buffers[col][curLine].pointer, str.c_str());
              // }
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
        vector<U8>& colBuf = buffers[col];
        switch (rawType) {
          case DT_BOOL:
            if (sqlType == SQL_DECIMAL || sqlType == SQL_NUMERIC) {
              for (int j = 0; j < curLine; ++j)
                charBuf[j] = colBuf[j].doubleVal == DBL_NMIN ? CHAR_MIN : (bool)colBuf[j].doubleVal;
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
                shortBuf[j] = colBuf[j].doubleVal == DBL_NMIN ? SHRT_MIN : colBuf[j].doubleVal;
            } else {
              for (int j = 0; j < curLine; ++j)
                shortBuf[j] = colBuf[j].shortVal;
            }
            arrCol[col]->appendShort(shortBuf, curLine);
            break;
          case DT_INT:
            if (sqlType == SQL_DECIMAL || sqlType == SQL_NUMERIC) {
              for (int j = 0; j < curLine; ++j)
                buf[j].intVal = colBuf[j].doubleVal == DBL_NMIN ? INT_MIN : colBuf[j].doubleVal;
            } else {
              for (int j = 0; j < curLine; ++j)
                buf[j].intVal = colBuf[j].intVal;
            }

            arrCol[col]->appendInt((int*)buf, curLine);
            break;
          case DT_LONG:
            if (sqlType == SQL_DECIMAL || sqlType == SQL_NUMERIC) {
              for (int j = 0; j < curLine; ++j)
                colBuf[j].longVal = colBuf[j].doubleVal == DBL_NMIN ? LONG_LONG_MIN : colBuf[j].doubleVal;
            }
            arrCol[col]->appendLong((long long*)(&colBuf[0]), curLine);
            break;
          case DT_FLOAT:
            if (sqlType == SQL_DECIMAL || sqlType == SQL_NUMERIC) {
              for (int j = 0; j < curLine; ++j)
                buf[j].floatVal = colBuf[j].doubleVal == DBL_NMIN ? FLT_NMIN : colBuf[j].doubleVal;
            } else {
              for (int j = 0; j < curLine; ++j)
                buf[j].floatVal = colBuf[j].floatVal;
            }
            arrCol[col]->appendFloat((float*)buf, curLine);
            break;
          case DT_DOUBLE:
            arrCol[col]->appendDouble((double*)(&colBuf[0]), curLine);
            break;
          case DT_STRING:
          case DT_BLOB:
//The processing for 64-bit is the same as for 32-bit.
//  
// #ifdef BIT32
//             if (sqlType == SQL_DECIMAL || sqlType == SQL_NUMERIC) {
//               for (int j = 0; j < curLine; ++j) {
//                 char strBuf[120];
//                 int nbytes = snprintf(strBuf, sizeof(strBuf), "%lf",
//                                       colBuf[j].doubleVal);
//                 if (colBuf[j].doubleVal != DBL_NMIN) {
//                   buf[j].pointer = new char[nbytes + 1];
//                   strcpy(buf[j].pointer, strBuf);
//                 } else {
//                   buf[j].pointer = new char[1];
//                   (*buf[j].pointer) = '\0'
//                 }
//               }
//             } else {
//               for (int j = 0; j < curLine; ++j)
//                 buf[j].pointer = colBuf[j].pointer;
//             }
//             arrCol[col]->appendString((const char**)buf, curLine);
//             for (int j = 0; j < curLine; ++j) delete[] buf[j].pointer;
// #else
            //If it's a String, we've already saved the data in Pointer. So we don't need this step.
            // if (sqlType == SQL_DECIMAL || sqlType == SQL_NUMERIC) {
            //   for (int j = 0; j < curLine; ++j) {
            //     char strBuf[120];
            //     int nbytes = snprintf(strBuf, sizeof(strBuf), "%lf",
            //                           colBuf[j].doubleVal);
            //     if (colBuf[j].doubleVal != DBL_NMIN) {
            //       buffers[col][j].pointer = new char[nbytes + 1];
            //       strcpy(buffers[col][j].pointer, strBuf);
            //     } else {
            //       buffers[col][j].pointer = new char[1];
            //       *(buffers[col][j].pointer) = '\0';
            //     }
            //   }
            // }
            {
            vector<string> stringBuffer;
            stringBuffer.reserve(curLine);
            for (int j = 0; j < curLine; ++j) stringBuffer.push_back((*(string*)(buffers[col][j].pointer)));
            arrCol[col]->appendString(stringBuffer.data(), curLine);
            for (int j = 0; j < curLine; ++j) delete ((string*)buffers[col][j].pointer);
            }
//#endif
            break;
          default:
            assert(false);
        }
      }
      curLine = 0;
      for (short i = 0; i < columns; ++i) {
        arrCol[i]->setNullFlag(arrCol[i]->hasNull());
      }

      if (arrCol[0]->size() > 0) {
        TableSP tmpTable = Util::createTable(colNames, columnVecs);
        if(!tranform.isNull()){
          vector<ConstantSP> arg{tmpTable};
          tmpTable = tranform->call(heap, arg);
        }
        vector<ConstantSP> _{table, tmpTable};
        static const FunctionDefSP func =
            heap->currentSession()->getFunctionDef("append!");
        func->call(heap, _);
      }

      for (short i = 0; i < columns; ++i) {
        arrCol[i]->clear();
      }
    }
  }
  return table;
}

ConstantSP odbcExecute(Heap* heap, vector<ConstantSP>& args) {
  if (!heap)
	throw RuntimeException("heap can't be null.");
  if (args.size() < 2)
	throw RuntimeException("arguments size can't less than 2.");

  u16string querySql = utf8_to_utf16(args[1]->getString());
  vector<ConstantSP> connArgs;
  connArgs.emplace_back(args[0]);
  ConstantSP csp = odbcGetConnection(heap, connArgs, "odbc::execute");
  nanodbc::connection* cp = safeGet(csp);

  try {
    nanodbc::just_execute(*cp, querySql);
  } catch (const runtime_error& e) {
	const char* fmt = "Executed query [%s]: %s";
    vector<char> errorMsgBuf(querySql.size() + strlen(e.what()) + strlen(fmt));
    sprintf(errorMsgBuf.data(), fmt, querySql.c_str(), e.what());
    string content = string(errorMsgBuf.data());
    vector<ConstantSP> closeArg = {csp};
    if(content.find("Syntax error") != string::npos && content.find("query, subquery, possibly with UNION, SELECT subquery, SELECT") != string::npos) {
      odbcConnectionOnClose(heap, closeArg);
    }
    throw TableRuntimeException(errorMsgBuf.data());
  }

  return Util::createConstant(DT_VOID);
}

#define TO_STRING(s) #s

static string sqlTypeName(DATA_TYPE t, const string& databaseType) {

  switch (t) {
    case DT_BOOL:
      if (databaseType == "postgresql")
        return "boolean";
      else if (databaseType == "clickhouse")
        return "Bool";
      else if (databaseType == "oracle")
        return "char(1)";
      else
        return "bit";
    case DT_CHAR:
      return "char(1)";
    case DT_SHORT:
      return "smallint";
    case DT_INT:
      return "int";
    case DT_LONG:
    if (databaseType == "oracle")
        return "number";
      return "bigint";
    case DT_DATE:
      return "date";
    case DT_MONTH:
      return "date";
    case DT_TIME:
      return "time";
    case DT_MINUTE:
      return "time";
    case DT_SECOND:
      return "time";
    case DT_DATETIME:
      if(databaseType == "postgresql")
        return "timestamp";
      else if(databaseType == "oracle")
        return "date";
      else if (databaseType == "clickhouse")
        return "datetime64";
      else
        return "datetime";

    case DT_TIMESTAMP:
      if(databaseType == "postgresql")
        return "timestamp";
      else if (databaseType == "clickhouse")
        return "datetime64";
      else if (databaseType == "oracle")
        return "timestamp";
      else
        return "datetime";
    case DT_NANOTIME:
      return "time";
    case DT_NANOTIMESTAMP:
      if(databaseType == "postgresql")
        return "timestamp";
      else if (databaseType == "clickhouse")
        return "datetime64";
      else if (databaseType == "oracle")
        return "timestamp(9)";
      else
        return "datetime";
    case DT_FLOAT:
      if(databaseType == "sqlserver")
        return "float(24)";
      else
        return "float";
    case DT_DOUBLE:
      if(databaseType == "postgresql")
        return "double precision";
      else if(databaseType == "sqlserver")
        return "float(53)";
      else if(databaseType == "oracle")
        return "binary_double";
      else 
        return "double";
    case DT_SYMBOL:
      return "varchar(255)";
    case DT_STRING:
      return "varchar(255)";

    default:
      throw RuntimeException("The data type " + Util::getDataTypeString(t) +" of DolphinDB is not supported. ");
  }
}

static string getValueStr(const ConstantSP& p, DATA_TYPE t, const string& databaseType) {  
  if (p->isNull()) return "null";
  string s;
  switch (t) {
    case DT_BOOL:
      if(databaseType == "postgresql"){
        if (p->getBool())
          return "true";
        else
          return "false";
      }
      else{
        if (p->getBool())
          return "1";
        else
          return "0";
      }

    case DT_CHAR:
      if(databaseType == "sqlite") {
        if(p->getString() == "\'") {
          return "\'\'\'\'";
        } else if(p->getString()=="\\") {
          return "\'\\\'";
        }
      }
      return "\'" + p->getString() + "\'";
     
    case DT_DATE:
      s = p->getString();
      for (size_t i = 0; i < s.length(); i++)
        if (s[i] == '.') s[i] = '-';
      s = "\'" + s + "\'";
      if(databaseType == "oracle") {
        s = "to_date(" + s + ", 'YYYY-MM-DD')";
      }
      return s;

    case DT_MONTH:
      s = p->getString();
      for (size_t i = 0; i < s.length(); i++)
        if (s[i] == '.') s[i] = '-';
      s.erase(s.length() - 1, 1);
      s += "-01";
      s = "\'" + s + "\'";
      return s;

    case DT_TIME:
      s = p->getString();
      // s = s.substr(0, 8);
      s = "\'" + s + "\'";
      return s;

    case DT_MINUTE:
      s = p->getString();
      s = s.substr(0, 5);
      s += ":00";
      s = "\'" + s + "\'";
      return s;

    case DT_SECOND:
      s = p->getString();
      s = "\'" + s + "\'";
      return s;

    case DT_DATETIME:
      s = p->getString();
      for (size_t i = 0; i < s.length(); i++) {
        if (s[i] == '.') s[i] = '-';
        if (s[i] == 'T') s[i] = ' ';
      }
      
      if(databaseType == "oracle") {
        s = "\'" + s + "\'";
        s = "to_date(" + s + ", 'YYYY-MM-DD HH24:MI:SS')";
      } else {
        s += ".000";
        s = "\'" + s + "\'";
      }
      return s;

    case DT_TIMESTAMP:
      s = p->getString();
      for (size_t i = 0; i < s.length() - 5; i++) {
        if (s[i] == '.') s[i] = '-';
        if (s[i] == 'T') s[i] = ' ';
      }
      s = s.substr(0, 23);
      s = "\'" + s + "\'";
      if(databaseType == "oracle") {
        s = "to_timestamp(" + s + ", 'YYYY-MM-DD HH24:MI:SS.ff')";
      }
      return s;

    case DT_NANOTIME:
      s = p->getString();
      // s = s.substr(0, 15);
      s = "\'" + s + "\'";
      return s;

    case DT_NANOTIMESTAMP:
      s = p->getString();
      // cout << s << endl;
      if(databaseType == "oracle"){
        for (size_t i = 0; i < s.length() - 12; i++) {
          if (s[i] == '.') s[i] = '-';
          if (s[i] == 'T') s[i] = ' ';
        }
      }
      else{
        s = s.substr(0, 23);
        for (size_t i = 0; i < s.length() - 5; i++) {
          if (s[i] == '.') s[i] = '-';
          if (s[i] == 'T') s[i] = ' ';
        }
      }
      s = "\'" + s + "\'";
      if(databaseType == "oracle") {
        s = "to_timestamp(" + s + ", 'YYYY-MM-DD HH24:MI:SS.ff9')";
      }
      return s;
      
    case DT_SYMBOL:
    case DT_STRING:
      s+="\'";
      if (databaseType == "postgresql" || databaseType == "sqlite" || databaseType == "sqlserver")
      {
        for (auto i : p->getString())
        {
          if (i == '\'')
            s += "\'";
          s += i;
        }
      }
      else
      {
        for (auto i : p->getString())
        {
          if (i == '\\' || i == '\'')
            s += "\\";
          s += i;
        }
      }
      s+="\'";
      return s;

    default:
      return p->getString();
  }
  return "";
}
ConstantSP odbcAppend(Heap* heap, vector<ConstantSP>& args) {
  if (!heap)
	throw RuntimeException("heap can't be null.");
  if (args.size() < 2)
	throw RuntimeException("arguments size can't less than 2.");

  vector<ConstantSP> connArgs;
  connArgs.emplace_back(args[0]);
  ConstantSP csp = odbcGetConnection(heap, connArgs, "odbc::append");
  nanodbc::transaction cp(*(safeGet(csp)));
  if(args[1]->getForm() != DF_TABLE)
    throw IllegalArgumentException("odbc::append", "tableData must be a table");
  TableSP t = args[1];
  if(args[2]->getType() != DT_STRING || args[2]->getForm() != DF_SCALAR)
    throw IllegalArgumentException("odbc::append", "tableName must be a string scalar");
  string tableName = args[2]->getString();
  if(tableName == "")
    throw IllegalArgumentException("odbc::append", "tableName cannot be empty");
  bool flag = true;
  if (args.size() > 3) {
    if(args[3]->getType() != DT_BOOL || args[3]->getForm() != DF_SCALAR)
      throw IllegalArgumentException("odbc::append", "createTableIfNotExist must be a bool scalar");
    flag = args[3]->getBool();
  }
  bool insertIgnore = false;
  if (args.size() > 4) {
    if(args[4]->getType() != DT_BOOL || args[4]->getForm() != DF_SCALAR)
      throw IllegalArgumentException("odbc::append", "insertIgnore must be a bool scalar");
    insertIgnore = args[4]->getBool();
  }

  string databaseType = ((OdbcConnection*)csp->getLong())->getDataBaseType();
  if (insertIgnore && (databaseType == "" || databaseType != "mysql"))
      throw IllegalArgumentException("odbc::append",
                                     "insert ignore can only use in MySQL");

  int inRows = 3000;
  if(databaseType == "sqlite")
    inRows = 500;
  vector<VectorSP> cols;
  vector<DATA_TYPE> colType;

  for (int i = 0; i < t->columns(); i++) {
    cols.push_back(t->getColumn(i));
    colType.push_back(t->getColumnType(i));
  }

  u16string querySql;

  if (flag) {
    string createString = "create table " + tableName + "(";
    for (int i = 0; i < t->columns(); i++) {
	  if (i)
	  	createString += ",";
      createString += t->getColumnName(i) + " " + sqlTypeName(colType[i], databaseType);
    }
    createString += ")";

    if(databaseType == "mysql") {
      createString += "DEFAULT CHARSET=utf8";
    } else if(databaseType == "clickhouse") {
      createString += "engine=Log;";
      std::cout << "use Log as table engine." << std::endl;
    }
    // cout << createString << endl;
    querySql = utf8_to_utf16(createString);
    try {
      nanodbc::just_execute(cp, querySql);
    } catch (const exception& e) {
      const char* fmt = "Executed query [%s]: %s";
      vector<char> errorMsgBuf(querySql.size() + strlen(e.what()) +
                               strlen(fmt));
      sprintf(errorMsgBuf.data(), fmt, querySql.c_str(), e.what());
      string content = string(errorMsgBuf.data());
      vector<ConstantSP> closeArg = {csp};
      if(content.find("Syntax error") != string::npos && content.find("query, subquery, possibly with UNION, SELECT subquery, SELECT") != string::npos) {
        odbcConnectionOnClose(heap, closeArg);
      }
      throw TableRuntimeException(content);
    }
  }
  int rows = t->rows();
  string q;
  if(databaseType == "oracle") {
    q = "INSERT ALL"; 
    string prefix = " INTO " + tableName + " VALUES ";

    for (int i = 0; i < rows; i++) {
      string insertString = "(";
      for (int j = 0; j < t->columns(); j++) {
		if (j)
	  	  insertString += ",";
        insertString += getValueStr(cols[j]->get(i), colType[j], databaseType);
      }
      insertString += ")";

      q += prefix;
      q += insertString;
      if (i % inRows == inRows - 1) {
        q += " select * from dual;";
        // cout << q << endl;
        u16string querySql = utf8_to_utf16(q);
        try {
          nanodbc::just_execute(cp, querySql);
        } catch (const runtime_error& e) {
          const char* fmt = "Executed query [%s]: %s";
          vector<char> errorMsgBuf(querySql.size() + strlen(e.what()) + strlen(fmt));
          sprintf(errorMsgBuf.data(), fmt, querySql.c_str(), e.what());
          throw TableRuntimeException(errorMsgBuf.data());
        }
          q = "INSERT ALL "; 
      }
    }
  } else {
    if (insertIgnore)
      q = "INSERT IGNORE INTO " + tableName + " VALUES ";
    else
      q = "INSERT INTO " + tableName + " VALUES ";

    for (int i = 0; i < rows; i++) {
      string insertString = "(";
      for (int j = 0; j < t->columns(); j++) {
		if (j)
			insertString += ",";
        insertString += getValueStr(cols[j]->get(i), colType[j], databaseType);
      }
      insertString += ")";
      if (i % inRows != inRows - 1) {
        insertString += ",";
      }

      q += insertString;
      if (i % inRows == inRows - 1) {
        q += ";";
        // cout << q << endl;
        u16string querySql = utf8_to_utf16(q);
        try {
          nanodbc::just_execute(cp, querySql);
        } catch (const runtime_error& e) {
          const char* fmt = "Executed query [%s]: %s";
          vector<char> errorMsgBuf(querySql.size() + strlen(e.what()) +
                                  strlen(fmt));
          sprintf(errorMsgBuf.data(), fmt, querySql.c_str(), e.what());
          string content = string(errorMsgBuf.data());
          vector<ConstantSP> closeArg = {csp};
          if(content.find("Syntax error") != string::npos && content.find("query, subquery, possibly with UNION, SELECT subquery, SELECT") != string::npos) {
            odbcConnectionOnClose(heap, closeArg);
          }
          throw TableRuntimeException(content);
        }
        if (insertIgnore)
          q = "INSERT IGNORE INTO " + tableName + " VALUES ";
        else
          q = "INSERT INTO " + tableName + " VALUES ";
      }
    }
  }


  if (rows % inRows != 0) {
    if(databaseType == "oracle") {
      q += " select * from dual;";
    } else {
      q[q.length() - 1] = ';';
    }
    
    // cout << q << endl;
    querySql = utf8_to_utf16(q);
    try {
      nanodbc::just_execute(cp, querySql);
    } catch (const runtime_error& e) {
      const char* fmt = "Executed query [%s]: %s";
      vector<char> errorMsgBuf(querySql.size() + strlen(e.what()) +
                               strlen(fmt));
      sprintf(errorMsgBuf.data(), fmt, querySql.c_str(), e.what());
      string content = string(errorMsgBuf.data());
      vector<ConstantSP> closeArg = {csp};
      if(content.find("Syntax error") != string::npos && content.find("query, subquery, possibly with UNION, SELECT subquery, SELECT") != string::npos) {
        odbcConnectionOnClose(heap, closeArg);
      }
      throw TableRuntimeException(content);
    }
  }
  try{
    cp.commit();
  }catch(const runtime_error &e){
      const char* fmt = "Failed to append data : %s";
      vector<char> errorMsgBuf(strlen(e.what()) + strlen(fmt));
      sprintf(errorMsgBuf.data(), fmt, e.what()); 
      throw TableRuntimeException(errorMsgBuf.data());
  }
  return Util::createConstant(DT_VOID);
}
