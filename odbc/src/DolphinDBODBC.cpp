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
#include <sql.h>
#include <sqlext.h>
#include <cassert>
#include <climits>
#include <cstdio>
#include <iostream>
#include <locale>
#include <vector>
#include "cvt.h"
#include "nanodbc/nanodbc.h"

#include <fstream>
#include <iostream>
//#define DEBUGlog
#ifdef DEBUGlog
std::ofstream sqlodbcfs;
string sqlodbcfslog = "/home/ybzhang/DolphinDBPlugin/odbctable.log";
#endif

using namespace std;

static void odbcConnectionOnClose(Heap* heap, vector<ConstantSP>& args) {
  nanodbc::connection* cp = (nanodbc::connection*)(args[0]->getLong());
  if (cp != nullptr) {
    delete cp;
    args[0]->setLong(0);
  }
}

static const int useStringSizeThreshold = 30;

static inline DATA_TYPE sqltype2DataType(int sqltype, long colsize) {
  // https://docs.microsoft.com/en-us/sql/odbc/reference/appendixes/c-data-types
  switch (sqltype) {
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
      return DT_SECOND;
    case SQL_TIMESTAMP:
    case SQL_TYPE_TIMESTAMP:
      return DT_NANOTIMESTAMP;
    case SQL_CHAR:
      if (colsize == 1) {
        return DT_CHAR;
      }
    // fall-through
    default:
      if ((sqltype == SQL_CHAR || sqltype == SQL_VARCHAR) &&
          colsize <= useStringSizeThreshold) {
        return DT_SYMBOL;
      }
      return DT_STRING;
  }
}

static bool compatible(DATA_TYPE dolphinType, int sqlType, int colsize) {
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
        case SQL_CHAR:
          if (colsize == 1) return true;
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
  if (args[0]->getType() == DT_STRING) {
    u16string connStr = utf8_to_utf16(args[0]->getString());
    try {
      unique_ptr<nanodbc::connection> cup(new nanodbc::connection(connStr));
      const char* fmt = "odbc connection to [%s]";
      vector<char> descBuf(connStr.size() + strlen(fmt));
      sprintf(descBuf.data(), fmt, connStr.c_str());
      FunctionDefSP onClose(Util::createSystemProcedure(
          "odbc connection onClose()", odbcConnectionOnClose, 1, 1));
      Constant* res =
          Util::createResource((long long)cup.release(), descBuf.data(),
                               onClose, heap->currentSession());
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

static TableSP odbcGetOrCreateTable(const Heap* heap,
                                    const vector<ConstantSP>& args,
                                    const nanodbc::result& results,
                                    vector<ConstantSP>& columnVecs) {
  int symbolCount = 0;
  if (args.size() >= 3 && args[2]->getType() != DT_VOID) {
    if (args[2]->getForm() != DF_TABLE) {
      throw TableRuntimeException("The 3rd argument must be a table");
    }
    TableSP tblSP(args[2]);
    INDEX columns = tblSP->columns();
    if (columns != results.columns()) {
      throw TableRuntimeException(
          "The given table schema is incompatible with the returned table from "
          "ODBC (different column count)");
    }
    for (INDEX i = 0; i < columns; ++i) {
      //            Vector* col = (Vector*)tblSP->getColumn(i).get();
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
    return tblSP;
  } else {
    short columns = results.columns();
    vector<string> columnNames(columns);
    vector<DATA_TYPE> columnTypes(columns);

    for (short i = 0; i < columns; ++i) {
      columnNames[i] = utf16_to_utf8(results.column_name(i));
      columnTypes[i] =
          sqltype2DataType(results.column_datatype(i), results.column_size(i));
      if (columnTypes[i] == DT_SYMBOL) {
        symbolCount++;
      }
    }
    // multiple columns share the same symbolbase
    SymbolBaseSP symbolBaseSP;
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
    return Util::createTable(columnNames, columnTypes, 0, 1);
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
  nanodbc::connection* cp = (nanodbc::connection*)(csp->getLong());
  cp->disconnect();
  return Util::createConstant(DT_VOID);
}

ConstantSP odbcConnect(Heap* heap, vector<ConstantSP>& args) {
  assert(heap);
  assert(args.size() == 1);
  if (args[0]->getType() != DT_STRING) {
    throw IllegalArgumentException(
        "odbc::connect",
        "Unknown argument type, must be an odbc connection string");
  }
  return odbcGetConnection(heap, args, "odbc::connect");
}

ConstantSP odbcQuery(Heap* heap, vector<ConstantSP>& args) {
  const static int nanodbc_rowset_size = 4096;
  assert(heap);
  assert(args.size() >= 2);
  // Encoding srcEncoding = Encoding::UTF8;
#ifdef WINDOWS
  if (args.size() == 4 && args[3]->getType() != DT_VOID) {
    if (args[3]->getType() != DT_STRING || args[3]->getForm() != DF_SCALAR) {
      throw IllegalArgumentException(
          "odbc::query",
          "The 4th argument must be the name of encoding, e.g.(GB2312)");
    }
    if (Util::lower(args[3]->getString()) == "gb2312") {
      auto srcEncoding = Encoding::GB2312;
    } else {
      throw IllegalArgumentException(
          "odbc::query",
          "The 4th argument must be the name of encoding, e.g.(GB2312)");
    }
  }
#endif

  u16string querySql = utf8_to_utf16(args[1]->getString());
  ConstantSP csp = odbcGetConnection(heap, args, "odbc::query");
  nanodbc::connection* cp = (nanodbc::connection*)(csp->getLong());

  nanodbc::result results;
  try {
    results = nanodbc::execute(*cp, querySql, nanodbc_rowset_size);
  } catch (const runtime_error& e) {
    const char* fmt = "Executed query [%s]: %s";
    vector<char> errorMsgBuf(querySql.size() + strlen(e.what()) + strlen(fmt));
    sprintf(errorMsgBuf.data(), fmt, querySql.c_str(), e.what());
    throw TableRuntimeException(errorMsgBuf.data());
  }

  const short columns = results.columns();
  vector<ConstantSP> columnVecs(columns);
  vector<Vector*> arrCol(columns);
  vector<DATA_TYPE> columnTypes(columns);

  TableSP table = odbcGetOrCreateTable(heap, args, results, columnVecs);
  for (short i = 0; i < columns; ++i) {
    arrCol[i] = (Vector*)columnVecs[i].get();
    columnTypes[i] = arrCol[i]->getType();
  }

  const int BUF_SIZE = 256 * 1024;
  vector<vector<U8>> buffers =
      vector<vector<U8>>(columns, vector<U8>(BUF_SIZE));
  int curLine = 0, rowIdx = 0;
  char charBuf[BUF_SIZE];
  short shortBuf[BUF_SIZE];
  U4 buf[BUF_SIZE];
  bool hasNext = results.next();
  vector<std::string> colNames(columns);
  for (short i = 0; i < columns; ++i) {
    colNames[i] = "col" + std::to_string(i);
  }
  bool isSegmentedTable = table->isSegmentedTable() || table->isDFSTable();
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
            nanodbc::date fallback = {0, 0, 0};
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
            nanodbc::timestamp fallback = {0, 0, 0, 0, 0, 0, 0};
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
            nanodbc::time fallback = {0, 0, 0};
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
            nanodbc::time fallback = {0, 0, 0};
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
            nanodbc::timestamp fallback = {0, 0, 0, 0, 0, 0, 0};
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
            nanodbc::timestamp fallback = {0, 0, 0, 0, 0, 0, 0};
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
            // cout << "{" << str << "} " << str.size() << " " <<
            // symbolBase->findAndInsert(str) << endl;
            buffers[col][curLine].intVal = symbolBase->findAndInsert(str);
          } break;
          default:
            u16string _str;
            results.get_ref<u16string>(col, u"", _str);
            string str = utf16_to_utf8(_str);
            buffers[col][curLine].pointer = new char[str.size() + 1];
            if (str == "") {
              *(buffers[col][curLine].pointer) = '\0';
            } else {
              strcpy(buffers[col][curLine].pointer, str.c_str());
            }
        }
      }
    }
    ++curLine;
    rowIdx++;
    hasNext = results.next();
    if (hasNext == false || curLine == BUF_SIZE) {
      for (short col = 0; col < columns; ++col) {
        DATA_TYPE rawType = arrCol[col]->getRawType();
        int sqltype = results.column_datatype(col);
        vector<U8>& colBuf = buffers[col];
        switch (rawType) {
          case DT_BOOL:
            if (sqltype == SQL_DECIMAL || sqltype == SQL_NUMERIC) {
              for (int j = 0; j < curLine; ++j)
                charBuf[j] = (bool)colBuf[j].doubleVal;
            } else {
              for (int j = 0; j < curLine; ++j) charBuf[j] = colBuf[j].charVal;
            }
            arrCol[col]->appendBool(charBuf, curLine);
            break;
          case DT_CHAR:
            if (sqltype == SQL_DECIMAL || sqltype == SQL_NUMERIC) {
              for (int j = 0; j < curLine; ++j)
                charBuf[j] = (char)colBuf[j].doubleVal;
            } else {
              for (int j = 0; j < curLine; ++j) charBuf[j] = colBuf[j].charVal;
            }
            arrCol[col]->appendChar(charBuf, curLine);
            break;
          case DT_SHORT:
            if (sqltype == SQL_DECIMAL || sqltype == SQL_NUMERIC) {
              for (int j = 0; j < curLine; ++j)
                shortBuf[j] = (short)colBuf[j].doubleVal;
            } else {
              for (int j = 0; j < curLine; ++j)
                shortBuf[j] = colBuf[j].shortVal;
            }
            arrCol[col]->appendShort(shortBuf, curLine);
            break;
          case DT_INT:
            if (sqltype == SQL_DECIMAL || sqltype == SQL_NUMERIC) {
              for (int j = 0; j < curLine; ++j)
                buf[j].intVal = (int)colBuf[j].doubleVal;
            } else {
              for (int j = 0; j < curLine; ++j)
                buf[j].intVal = colBuf[j].intVal;
            }

            arrCol[col]->appendInt((int*)buf, curLine);
            break;
          case DT_LONG:
            if (sqltype == SQL_DECIMAL || sqltype == SQL_NUMERIC) {
              for (int j = 0; j < curLine; ++j)
                colBuf[j].longVal = (long long)colBuf[j].doubleVal;
            }
            arrCol[col]->appendLong((long long*)(&colBuf[0]), curLine);
            break;
          case DT_FLOAT:
            if (sqltype == SQL_DECIMAL || sqltype == SQL_NUMERIC) {
              for (int j = 0; j < curLine; ++j)
                buf[j].floatVal = (long long)colBuf[j].doubleVal;
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
#ifdef BIT32
            if (sqltype == SQL_DECIMAL || sqltype == SQL_NUMERIC) {
              for (int j = 0; j < curLine; ++j) {
                char strBuf[120];
                int nbytes = snprintf(strBuf, sizeof(strBuf), "%lf",
                                      colBuf[j].doubleVal);
                if (colBuf[j].doubleVal != DBL_NMIN) {
                  buf[j].pointer = new char[nbytes + 1];
                  strcpy(buf[j].pointer, strBuf);
                } else {
                  buf[j].pointer = new char[1];
                  (*buf[j].pointer) = '\0'
                }
              }
            } else {
              for (int j = 0; j < curLine; ++j)
                buf[j].pointer = colBuf[j].pointer;
            }
            arrCol[col]->appendString((char**)buf, curLine);
            for (int j = 0; j < curLine; ++j) delete[] buf[j].pointer;
#else
            if (sqltype == SQL_DECIMAL || sqltype == SQL_NUMERIC) {
              for (int j = 0; j < curLine; ++j) {
                char strBuf[120];
                int nbytes = snprintf(strBuf, sizeof(strBuf), "%lf",
                                      colBuf[j].doubleVal);
                if (colBuf[j].doubleVal != DBL_NMIN) {
                  buffers[col][j].pointer = new char[nbytes + 1];
                  strcpy(buffers[col][j].pointer, strBuf);
                } else {
                  buffers[col][j].pointer = new char[1];
                  *(buffers[col][j].pointer) = '\0';
                }
              }
            }
            arrCol[col]->appendString((char**)(&buffers[col][0]), curLine);
            for (int j = 0; j < curLine; ++j) delete[] buffers[col][j].pointer;
#endif
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
        if (isSegmentedTable) {
          TableSP tmpTable = Util::createTable(colNames, columnVecs);
          vector<ConstantSP> _{table, tmpTable};
          static const FunctionDefSP func =
              heap->currentSession()->getFunctionDef("append!");
          func->call(heap, _);
        } else {
          string errMsg;
          INDEX insertedRows = 0;
          bool good = table->append(columnVecs, insertedRows, errMsg);
          if (!good) {
            throw TableRuntimeException(errMsg);
          }
        }
      }

      for (short i = 0; i < columns; ++i) {
        arrCol[i]->clear();
      }
    }
  }
  return table;
}

ConstantSP odbcExecute(Heap* heap, vector<ConstantSP>& args) {
  assert(heap);
  assert(args.size() >= 2);

  u16string querySql = utf8_to_utf16(args[1]->getString());
  ConstantSP csp = odbcGetConnection(heap, args, "odbc::query");
  nanodbc::connection* cp = (nanodbc::connection*)(csp->getLong());

  try {
    nanodbc::just_execute(*cp, querySql);
  } catch (const runtime_error& e) {
    const char* fmt = "Executed query [%s]: %s";
    vector<char> errorMsgBuf(querySql.size() + strlen(e.what()) + strlen(fmt));
    sprintf(errorMsgBuf.data(), fmt, querySql.c_str(), e.what());
    throw TableRuntimeException(errorMsgBuf.data());
  }

  return Util::createConstant(DT_VOID);
}

string sqltypename(DATA_TYPE t) {
  enum DATA_TYPE {
    DT_VOID,
    DT_BOOL,
    DT_CHAR,
    DT_SHORT,
    DT_INT,
    DT_LONG,
    DT_DATE,
    DT_MONTH,
    DT_TIME,
    DT_MINUTE,
    DT_SECOND,
    DT_DATETIME,
    DT_TIMESTAMP,
    DT_NANOTIME,
    DT_NANOTIMESTAMP,
    DT_FLOAT,
    DT_DOUBLE,
    DT_SYMBOL,
    DT_STRING,
    DT_UUID,
    DT_FUNCTIONDEF,
    DT_HANDLE,
    DT_CODE,
    DT_DATASOURCE,
    DT_RESOURCE,
    DT_ANY,
    DT_COMPRESS,
    DT_DICTIONARY,
    DT_DATEHOUR,
    DT_DATEMINUTE,
    DT_OBJECT
  };

  switch (t) {
    case DT_BOOL:
      return "bit";
      break;
    case DT_CHAR:
      return "char(1)";
      break;
    case DT_SHORT:
      return "smallint";
      break;
    case DT_INT:
      return "int";
      break;
    case DT_LONG:
      return "bigint";
      break;
    case DT_DATE:
      return "date";
      break;
    case DT_MONTH:
      return "date";
      break;
    case DT_TIME:
      return "time";
      break;
    case DT_MINUTE:
      return "time";
      break;
    case DT_SECOND:
      return "time";
      break;
    case DT_DATETIME:
      return "datetime";
      break;
    case DT_TIMESTAMP:
      return "datetime";
      break;
    case DT_NANOTIME:
      return "time";
      break;
    case DT_NANOTIMESTAMP:
      return "datetime";
      break;
    case DT_FLOAT:
      return "float";
      break;
    case DT_DOUBLE:
      return "double";
      break;
    case DT_SYMBOL:
      return "varchar(255)";
      break;
    case DT_STRING:
      return "varchar(255)";
      break;

    default:
      break;
  }
  return "";
}

string getvaluestr(ConstantSP p, DATA_TYPE t) {
  enum DATA_TYPE {
    DT_VOID,
    DT_BOOL,
    DT_CHAR,
    DT_SHORT,
    DT_INT,
    DT_LONG,
    DT_DATE,
    DT_MONTH,
    DT_TIME,
    DT_MINUTE,
    DT_SECOND,
    DT_DATETIME,
    DT_TIMESTAMP,
    DT_NANOTIME,
    DT_NANOTIMESTAMP,
    DT_FLOAT,
    DT_DOUBLE,
    DT_SYMBOL,
    DT_STRING,
    DT_UUID,
    DT_FUNCTIONDEF,
    DT_HANDLE,
    DT_CODE,
    DT_DATASOURCE,
    DT_RESOURCE,
    DT_ANY,
    DT_COMPRESS,
    DT_DICTIONARY,
    DT_DATEHOUR,
    DT_DATEMINUTE,
    DT_OBJECT
  };
  if (p->isNull()) return "null";
  string s;
  switch (t) {
    case DT_BOOL:
      if (p->getBool())
        return "1";
      else
        return "0";

      break;
    case DT_CHAR:
      return "\'" + p->getString() + "\'";
      break;
      ///   case DT_SHORT:
      ///    return "smallint";
      ///    break;
      ///  case DT_INT:
      ///   return "int";
      ///   break;
      ///  case DT_LONG:
      ///   return "bigint";
      ///   break;
    case DT_DATE:
      s = p->getString();
      for (size_t i = 0; i < s.length(); i++)
        if (s[i] == '.') s[i] = '-';
      s = "\'" + s + "\'";
      return s;
      break;

    case DT_MONTH:
      s = p->getString();
      for (size_t i = 0; i < s.length(); i++)
        if (s[i] == '.') s[i] = '-';
      s.erase(s.length() - 1, 1);
      s += "-01";
      s = "\'" + s + "\'";
      return s;
      break;

    case DT_TIME:
      s = p->getString();
      s = s.substr(0, 8);
      s = "\'" + s + "\'";
      return s;
      break;

    case DT_MINUTE:
      s = p->getString();
      s = s.substr(0, 5);
      s += ":00";
      s = "\'" + s + "\'";
      return s;
      break;

    case DT_SECOND:
      s = p->getString();
      s = "\'" + s + "\'";
      return s;
      break;

    case DT_DATETIME:
      s = p->getString();
      for (size_t i = 0; i < s.length(); i++) {
        if (s[i] == '.') s[i] = '-';
        if (s[i] == 'T') s[i] = ' ';
      }
      s += ".000";
      s = "\'" + s + "\'";
      return s;
      break;

    case DT_TIMESTAMP:
      s = p->getString();
      for (size_t i = 0; i < s.length() - 5; i++) {
        if (s[i] == '.') s[i] = '-';
        if (s[i] == 'T') s[i] = ' ';
      }
      s = s.substr(0, 23);
      s = "\'" + s + "\'";
      return s;
      break;

    case DT_NANOTIME:
      s = p->getString();
      s = s.substr(0, 8);
      s = "\'" + s + "\'";
      return s;
      break;

    case DT_NANOTIMESTAMP:
      s = p->getString();
      s = s.substr(0, 23);
      for (size_t i = 0; i < s.length() - 5; i++) {
        if (s[i] == '.') s[i] = '-';
        if (s[i] == 'T') s[i] = ' ';
      }

      s = "\'" + s + "\'";
      return s;
      break;
      //  case DT_FLOAT:
      //    return "float";
      //    break;
      ///   case DT_DOUBLE:
      ///    return "double";
      ///     break;
    case DT_SYMBOL:
      return "\'" + p->getString() + "\'";
      break;
    case DT_STRING:
      return "\'" + p->getString() + "\'";
      break;

    default:
      return p->getString();
      break;
  }
  return "";
}
ConstantSP odbcAppend(Heap* heap, vector<ConstantSP>& args) {
#ifdef DEBUGlog
  sqlodbcfs.open(sqlodbcfslog, std::ios::app);
  sqlodbcfs << "function odbcExportTable" << std::endl;
  sqlodbcfs.close();
#endif

  assert(heap);
  assert(args.size() >= 2);

  // u16string querySql = utf8_to_utf16(args[1]->getString());
  ConstantSP csp = odbcGetConnection(heap, args, "odbc::query");
  nanodbc::connection* cp = (nanodbc::connection*)(csp->getLong());
  TableSP t = args[1];
  string tablename = args[2]->getString();
  bool flag = true;
  if (args.size() > 3) flag = args[3]->getBool();
  bool insertIgnore = false;
  if (args.size() > 4) insertIgnore = args[4]->getBool();
  int inrows = 3000;
  vector<VectorSP> cols;
  vector<DATA_TYPE> coltype;
#ifdef DEBUGlog
  sqlodbcfs.open(sqlodbcfslog, std::ios::app);
  sqlodbcfs << "function odbcExportTable get connection success" << std::endl;
  sqlodbcfs.close();
#endif
#ifdef DEBUGlog
  sqlodbcfs.open(sqlodbcfslog, std::ios::app);
  sqlodbcfs << "t is " << t->getString() << std::endl;
  sqlodbcfs.close();
#endif
  for (int i = 0; i < t->columns(); i++) {
    cols.push_back(t->getColumn(i));
    coltype.push_back(t->getColumnType(i));
  }

  u16string querySql;

  if (flag) {
    string createstring = "create table " + tablename + "(";
    for (int i = 0; i < t->columns(); i++) {
      createstring += t->getColumnName(i) + " " + sqltypename(coltype[i]);
      if (i < t->columns() - 1) createstring += ",";
    }
    createstring += ")";

#ifdef DEBUGlog
    sqlodbcfs.open(sqlodbcfslog, std::ios::app);
    sqlodbcfs << createstring << std::endl;
    sqlodbcfs.close();
#endif

    querySql = utf8_to_utf16(createstring);
    try {
      nanodbc::just_execute(*cp, querySql);
    } catch (const runtime_error& e) {
      const char* fmt = "Executed query [%s]: %s";
      vector<char> errorMsgBuf(querySql.size() + strlen(e.what()) +
                               strlen(fmt));
      sprintf(errorMsgBuf.data(), fmt, querySql.c_str(), e.what());
      throw TableRuntimeException(errorMsgBuf.data());
    }
  }
  int rows = t->rows();
  string q;
  if (insertIgnore)
    q = "INSERT IGNORE INTO " + tablename + " VALUES ";
  else
    q = "INSERT INTO " + tablename + " VALUES ";

  for (int i = 0; i < rows; i++) {
    string insertstring = "(";
    for (int j = 0; j < t->columns(); j++) {
      insertstring += getvaluestr(cols[j]->get(i), coltype[j]);
      if (j < t->columns() - 1) insertstring += ",";
    }
    insertstring += ")";
    if (i % inrows != inrows - 1) {
      insertstring += ",";
    }
#ifdef DEBUGlog
    sqlodbcfs.open(sqlodbcfslog, std::ios::app);
    sqlodbcfs << insertstring << std::endl;
    sqlodbcfs.close();
#endif
    q += insertstring;
    if (i % inrows == inrows - 1) {
      q += ";";
      u16string querySql = utf8_to_utf16(q);
      try {
        nanodbc::just_execute(*cp, querySql);
      } catch (const runtime_error& e) {
        const char* fmt = "Executed query [%s]: %s";
        vector<char> errorMsgBuf(querySql.size() + strlen(e.what()) +
                                 strlen(fmt));
        sprintf(errorMsgBuf.data(), fmt, querySql.c_str(), e.what());
        throw TableRuntimeException(errorMsgBuf.data());
      }
      q = "INSERT INTO " + tablename + " VALUES ";
    }
  }
  if (rows % inrows != 0) {
    q[q.length() - 1] = ';';
    querySql = utf8_to_utf16(q);
    try {
      nanodbc::just_execute(*cp, querySql);
    } catch (const runtime_error& e) {
      const char* fmt = "Executed query [%s]: %s";
      vector<char> errorMsgBuf(querySql.size() + strlen(e.what()) +
                               strlen(fmt));
      sprintf(errorMsgBuf.data(), fmt, querySql.c_str(), e.what());
      throw TableRuntimeException(errorMsgBuf.data());
    }
  }
  return Util::createConstant(DT_VOID);
}
