//
// Created by lin on 2021/2/23.
//

#include "PluginHbase.h"
#include "Logger.h"
#include "ScalarImp.h"
#include "Util.h"

#include "ddbplugin/Plugin.h"

#include <protocol/TBinaryProtocol.h>
#include <transport/TTransportUtils.h>


using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::hadoop::hbase::thrift;

const static string HBASE_PREFIX = "[Plugin::HBase]";
const static string HBASE_CONNECTION_DESC = "hbase connection";

dolphindb::ResourceMap<HBaseConnect> HBASE_CONNECTION_MAP(HBASE_PREFIX, HBASE_CONNECTION_DESC);

/* INTERFACES */

ConstantSP connectH(Heap *heap, vector<ConstantSP> &args) {
    string usage = "Usage: connect(host, port, [isFramed], [timeout]). ";

    bool isFramed = false;
    int timeout = 5000;//default is 5000ms

    if (args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "host must be a string!");
    }
    if (args[1]->getType() != DT_INT || args[1]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "port must be an integer!");
    }
    if (args.size() >= 3) {
        if (args[2]->getType() != DT_BOOL || args[2]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, usage + "isFramed must be a bool!");
        }
        if (args[2]->isNull()) {
            throw IllegalArgumentException(__FUNCTION__, usage + "isFramed is provided but is Null.");
        }
        isFramed = args[2]->getBool();
    }
    if (args.size() == 4) {
        if (args[3]->getType() != DT_INT || args[3]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, usage + "timeout must be an integer!");
        }
        timeout = args[3]->getInt();
    }

    SmartPointer<HBaseConnect> conn = new HBaseConnect(args[0]->getString(), args[1]->getInt(), isFramed, timeout);
    FunctionDefSP onClose(Util::createSystemProcedure(
            "hbase connection onClose()", connectionOnCloseH, 1, 1));
    ConstantSP resource = Util::createResource(reinterpret_cast<long long>(conn.get()), HBASE_CONNECTION_DESC, onClose, heap->currentSession());
    HBASE_CONNECTION_MAP.safeAdd(resource, conn);

    return resource;
}

ConstantSP showTablesH(Heap *heap, vector<ConstantSP> &args) {
    string usage = "Usage: showTables(hbaseConnection). ";

    auto conn = HBASE_CONNECTION_MAP.safeGet(args[0]);
    return conn->showTablesH();
}

ConstantSP loadH(Heap *heap, vector<ConstantSP> &args) {
    string usage = "Usage: load(hbaseConnection, tableName, [schema]). ";

    auto conn = HBASE_CONNECTION_MAP.safeGet(args[0]);

    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "tableName must be a string!");
    }
    if (args.size() == 3) {
        if (args[2]->getForm() != DF_TABLE) {
            throw IllegalArgumentException(__FUNCTION__, usage + "schema must be a table!");
        }
        return conn->loadH(args[1]->getString(), args[2]);
    }
    return conn->loadH(args[1]->getString());
}

ConstantSP deleteTableH(Heap *heap, vector<ConstantSP> &args) {
    string usage = "Usage: deleteTable(hbaseConnection, tableName). ";

    auto conn = HBASE_CONNECTION_MAP.safeGet(args[0]);

    if ((args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR) && args[1]->getForm() != DF_VECTOR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "tableName must be a string or string vector!");
    }
    if (args[1]->getForm() == DF_VECTOR) {
        for (int i = 0; i < args[1]->size(); ++i) {
            conn->deleteTableH(args[1]->getString(i));
        }
    } else {
        conn->deleteTableH(args[1]->getString());
    }
    return new Void();
}

ConstantSP getRowH(Heap *heap, vector<ConstantSP> &args) {
    string usage = "Usage: getRow(hbaseConnection, tableName, rowKey, [columnName]). ";

    vector<string> columnNames;
    auto conn = HBASE_CONNECTION_MAP.safeGet(args[0]);

    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "tableName must be a string!");
    }
    if (args[2]->getType() != DT_STRING || args[2]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "rowKey must be a string!");
    }
    if (args.size() == 4) {
        if (args[3]->getType() != DT_STRING && (args[3]->getForm() != DF_SCALAR || args[3]->getForm() != DF_VECTOR))
            throw IllegalArgumentException(__FUNCTION__, usage + "columnName must be a string or string vector!");
        if (args[3]->getForm() == DF_SCALAR) {
            columnNames.emplace_back(args[3]->getString());
        } else {
            int columnSize = args[3]->size();
            for (int i = 0; i < columnSize; ++i) {
                columnNames.emplace_back(args[3]->getString(i));
            }
        }
    }

    return conn->getRowH(args[1]->getString(), args[2]->getString(), columnNames);
}


/* HBASECONNECT */

HBaseConnect::HBaseConnect(const string &hostname, const int port, bool isFramed, int timeout) {
    apache::thrift::GlobalOutput.setOutputFunction(customThriftLogFunction);

    socket_ = std::make_shared<apache::thrift::transport::TSocket>(hostname, port);
    socket_->setConnTimeout(timeout);
    socket_->setRecvTimeout(timeout);

    if (isFramed) {
        transport_ = std::make_shared<apache::thrift::transport::TFramedTransport>(socket_);
    } else {
        transport_ = std::make_shared<apache::thrift::transport::TBufferedTransport>(socket_);
    }

    auto protocol = std::make_shared<TBinaryProtocol>(transport_);
    client_ = std::make_shared<HbaseClient>(protocol);

    try {
        transport_->open();

        if (!transport_->isOpen()) {
            throw RuntimeException(string("HBase: ") + "Failed to connect to the HBase Thrift server");
        }
    } catch (const TException &tx) {
        throw RuntimeException(string("HBase: ") + tx.what());
    }

    try {
        // Fetch table names to check if the port is right
        vector<string> tableNames;
        client_->getTableNames(tableNames);
    } catch (TException &tx) {
        if (isFramed && string(tx.what()) == "THRIFT_EAGAIN (timed out)") {
            throw RuntimeException(string("HBase: ") + tx.what() + "\nThe HBase Thrift server probably is not using `TFramedTransport`. Add `--framed` when start the server.");
        }
        throw RuntimeException(string("HBase: ") + tx.what() + "\nThe port number may be wrong (not for HBase Thrift server, default is 9090).");
    }
}

ConstantSP HBaseConnect::showTablesH() {
    LockGuard<Mutex> lk(&mtx_);
    vector<string> tables;
    try {
        client_->getTableNames(tables);
    } catch (TException &tx) {
        throw RuntimeException(string("HBase getTableNames error: ") + tx.what());
    }
    VectorSP ret = Util::createVector(DT_STRING, 0, (INDEX) tables.size());
    ret->appendString(tables.data(), (INDEX) tables.size());
    return ret;
}

ConstantSP HBaseConnect::loadH(const string &tableName) {
    LockGuard<Mutex> lk(&mtx_);
    vector<string> tables;
    try {
        client_->getTableNames(tables);
    } catch (TException &tx) {
        throw RuntimeException(string("HBase getTableNames error: ") + tx.what());
    }
    const std::map<Text, Text> dummyAttributes;
    vector<string> columnNames;
    int scanner;
    for (const auto &table: tables) {
        if (tableName == table) {
            TableSP result;
            try {
                scanner = client_->scannerOpen(tableName, "", columnNames, dummyAttributes);

                vector<string> colNames = {"row"};
                bool first = true;

                while (true) {
                    vector<TRowResult> values;
                    client_->scannerGetList(values, scanner, 1024);
                    if (values.empty()) {
                        if (first) {
                            return new Void();
                        }
                        break;
                    }
                    vector<ConstantSP> columns;
                    for (auto &val: values) {
                        vector<ConstantSP> dataToAppend;
                        if (first) {
                            columns.emplace_back(new String(val.row));
                        } else {
                            dataToAppend.emplace_back(new String(val.row));
                        }

                        for (auto &column: val.columns) {
                            if (first) {
                                colNames.emplace_back(column.first);
                                columns.emplace_back(new String(column.second.value));
                            } else {
                                dataToAppend.emplace_back(new String(column.second.value));
                            }
                        }
                        if (first) {
                            result = Util::createTable(colNames, columns);
                            first = false;
                        } else {
                            INDEX insertedRows;
                            string errMsg;
                            bool success = result->append(dataToAppend, insertedRows, errMsg);
                            if (!success) {
                                std::cerr << errMsg << std::endl;
                                LOG_ERR(errMsg);
                            }
                        }
                    }
                }

                client_->scannerClose(scanner);
                return result;
            } catch (const TException &tx) {
                throw RuntimeException(string("HBase scanner error: ") + tx.what());
            }
        }
    }

    throw RuntimeException("Table " + tableName + " is not found!");
}

ConstantSP HBaseConnect::loadH(const string &tableName, const TableSP &schema) {
    LockGuard<Mutex> lk(&mtx_);

    VectorSP vecName = schema->getColumn("name");
    if (vecName == nullptr) {
        throw IllegalArgumentException(__FUNCTION__, "There is no column \"name\" in schema table");
    }
    if (vecName->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, "The schema table column \"name\" type must be STRING");
    }

    VectorSP vecType = schema->getColumn("type");
    if (vecType == nullptr) {
        throw IllegalArgumentException(__FUNCTION__, "There is no column \"type\" in schema table");
    }
    if (vecType->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, "The schema table column \"type\" type must be STRING");
    }
    if (vecName->size() != vecType->size()) {
        throw IllegalArgumentException(__FUNCTION__, "The schema table column \"name\" and \"type\" size are not equal");
    }

    int colNums = vecName->size();
    vector<string> colNames{"row"};
    vector<string> columnNames;
    vector<ConstantSP> cols;
    vector<DATA_TYPE> colTypes;
    colTypes.emplace_back(DT_STRING);
    cols.resize(colNums + 1);
    cols[0] = Util::createVector(DT_STRING, 0);

    for (int i = 1; i < colNums + 1; ++i) {
        colNames.emplace_back(vecName->getString(i - 1));
        columnNames.emplace_back(vecName->getString(i - 1));
        string sType = vecType->getString(i - 1);
        std::transform(sType.begin(), sType.end(), sType.begin(), ::toupper);
        if (sType == "BOOL") {
            colTypes.push_back(DT_BOOL);
            cols[i] = Util::createVector(DT_BOOL, 0);
        } else if (sType == "CHAR") {
            colTypes.push_back(DT_CHAR);
            cols[i] = Util::createVector(DT_CHAR, 0);
        } else if (sType == "SHORT") {
            colTypes.push_back(DT_SHORT);
            cols[i] = Util::createVector(DT_SHORT, 0);
        } else if (sType == "INT") {
            colTypes.push_back(DT_INT);
            cols[i] = Util::createVector(DT_INT, 0);
        } else if (sType == "LONG") {
            colTypes.push_back(DT_LONG);
            cols[i] = Util::createVector(DT_LONG, 0);
        } else if (sType == "DATE") {
            colTypes.push_back(DT_DATE);
            cols[i] = Util::createVector(DT_DATE, 0);
        } else if (sType == "MONTH") {
            colTypes.push_back(DT_MONTH);
            cols[i] = Util::createVector(DT_MONTH, 0);
        } else if (sType == "TIME") {
            colTypes.push_back(DT_TIME);
            cols[i] = Util::createVector(DT_TIME, 0);
        } else if (sType == "MINUTE") {
            colTypes.push_back(DT_MINUTE);
            cols[i] = Util::createVector(DT_MINUTE, 0);
        } else if (sType == "SECOND") {
            colTypes.push_back(DT_SECOND);
            cols[i] = Util::createVector(DT_SECOND, 0);
        } else if (sType == "DATETIME") {
            colTypes.push_back(DT_DATETIME);
            cols[i] = Util::createVector(DT_DATETIME, 0);
        } else if (sType == "TIMESTAMP") {
            colTypes.push_back(DT_TIMESTAMP);
            cols[i] = Util::createVector(DT_TIMESTAMP, 0);
        } else if (sType == "NANOTIME") {
            colTypes.push_back(DT_NANOTIME);
            cols[i] = Util::createVector(DT_NANOTIME, 0);
        } else if (sType == "NANOTIMESTAMP") {
            colTypes.push_back(DT_NANOTIMESTAMP);
            cols[i] = Util::createVector(DT_NANOTIMESTAMP, 0);
        } else if (sType == "FLOAT") {
            colTypes.push_back(DT_FLOAT);
            cols[i] = Util::createVector(DT_FLOAT, 0);
        } else if (sType == "DOUBLE") {
            colTypes.push_back(DT_DOUBLE);
            cols[i] = Util::createVector(DT_DOUBLE, 0);
        } else if (sType == "SYMBOL") {
            colTypes.push_back(DT_SYMBOL);
            cols[i] = Util::createVector(DT_SYMBOL, 0);
        } else if (sType == "STRING") {
            colTypes.push_back(DT_STRING);
            cols[i] = Util::createVector(DT_STRING, 0);
        } else {
            throw IllegalArgumentException(__FUNCTION__, "The Type " + sType + " is not supported");
        }
    }

    TableSP result = Util::createTable(colNames, colTypes, 0, 10);
    vector<string> tables;

    try {
        client_->getTableNames(tables);
    } catch (TException &tx) {
        throw RuntimeException(string("HBase getTableNames error: ") + tx.what());
    }

    const std::map<Text, Text> dummyAttributes;
    int scanner;
    vector<string> emptyVec;
    for (const auto &table: tables) {
        if (tableName == table) {
            try {
                scanner = client_->scannerOpen(tableName, "", emptyVec, dummyAttributes);
            } catch (TException &tx) {
                throw RuntimeException(string("HBase scannerOpen error: ") + tx.what());
            }

            while (true) {
                vector<TRowResult> values;
                try {
                    client_->scannerGetList(values, scanner, 1024);
                } catch (TException &tx) {
                    throw RuntimeException(string("HBase scannerGetList error: ") + tx.what());
                }
                if (values.empty())
                    break;
                for (auto &val: values) {
                    vector<ConstantSP> dataToAppend;
                    dataToAppend.emplace_back(new String(val.row));
                    for (auto i = 1; i < colNums + 1; ++i) {
                        auto cell = val.columns[colNames[i]];
                        if (cell.value.empty()) {
                            dataToAppend.emplace_back(new Void());
                            continue;
                        }
                        switch (colTypes[i]) {
                            case DT_BOOL: {
                                string tem(cell.value);
                                std::transform(tem.begin(), tem.end(), tem.begin(), ::toupper);
                                if (tem == "TRUE" || tem == "1") {
                                    dataToAppend.emplace_back(new Bool(1));
                                    break;
                                } else if (tem == "FALSE" || tem == "0") {
                                    dataToAppend.emplace_back(new Bool(0));
                                    break;
                                } else {
                                    dataToAppend.emplace_back(new Void());
                                    break;
                                }
                            }
                            case DT_CHAR: {
                                if (cell.value.length() > 1) {
                                    dataToAppend.emplace_back(new Void());
                                } else {
                                    dataToAppend.emplace_back(new Char(cell.value[0]));
                                }
                                break;
                            }
                            case DT_SHORT: {
                                char *pEnd;
                                auto tem = (short) std::strtol(cell.value.c_str(), &pEnd, 10);
                                if (pEnd == cell.value.c_str()) {
                                    dataToAppend.emplace_back(new Void());
                                } else {
                                    dataToAppend.emplace_back(new Short(tem));
                                }
                                break;
                            }
                            case DT_INT: {
                                char *pEnd;
                                auto tem = (int) std::strtol(cell.value.c_str(), &pEnd, 10);
                                if (pEnd == cell.value.c_str()) {
                                    dataToAppend.emplace_back(new Void());
                                } else {
                                    dataToAppend.emplace_back(new Int(tem));
                                }
                                break;
                            }
                            case DT_LONG: {
                                char *pEnd;
                                auto tem = (long long) std::strtoll(cell.value.c_str(), &pEnd, 10);
                                if (pEnd == cell.value.c_str()) {
                                    dataToAppend.emplace_back(new Void());
                                } else {
                                    dataToAppend.emplace_back(new Long(tem));
                                }
                                break;
                            }
                            case DT_FLOAT: {
                                char *pEnd;
                                auto tem = std::strtof(cell.value.c_str(), &pEnd);
                                if (pEnd == cell.value.c_str()) {
                                    dataToAppend.emplace_back(new Void());
                                } else {
                                    dataToAppend.emplace_back(new Float(tem));
                                }
                                break;
                            }
                            case DT_DOUBLE: {
                                char *pEnd;
                                auto tem = std::strtod(cell.value.c_str(), &pEnd);
                                if (pEnd == cell.value.c_str()) {
                                    dataToAppend.emplace_back(new Void());
                                } else {
                                    dataToAppend.emplace_back(new Double(tem));
                                }
                                break;
                            }
                            case DT_SYMBOL:
                            case DT_STRING: {
                                dataToAppend.emplace_back(new String(cell.value));
                                break;
                            }
                            case DT_TIMESTAMP: {
                                long long tem;
                                if (timestampParserH(cell.value, tem)) {
                                    dataToAppend.emplace_back(new Timestamp(tem));
                                } else {
                                    dataToAppend.emplace_back(new Void());
                                }
                                break;
                            }
                            case DT_NANOTIME: {
                                long long tem;
                                if (nanoTimeParserH(cell.value, tem)) {
                                    dataToAppend.emplace_back(new NanoTime(tem));
                                } else {
                                    dataToAppend.emplace_back(new Void());
                                }
                                break;
                            }
                            case DT_NANOTIMESTAMP: {
                                long long tem;
                                if (nanoTimestampParserH(cell.value, tem)) {
                                    dataToAppend.emplace_back(new NanoTimestamp(tem));
                                } else {
                                    dataToAppend.emplace_back(new Void());
                                }
                                break;
                            }
                            case DT_DATETIME: {
                                int tem;
                                if (dateTimeParserH(cell.value, tem)) {
                                    dataToAppend.emplace_back(new DateTime(tem));
                                } else {
                                    dataToAppend.emplace_back(new Void());
                                }
                                break;
                            }
                            case DT_MINUTE: {
                                int tem;
                                if (minuteParserH(cell.value, tem)) {
                                    dataToAppend.emplace_back(new Minute(tem));
                                } else {
                                    dataToAppend.emplace_back(new Void());
                                }
                                break;
                            }
                            case DT_SECOND: {
                                int tem;
                                if (secondParserH(cell.value, tem)) {
                                    dataToAppend.emplace_back(new Second(tem));
                                } else {
                                    dataToAppend.emplace_back(new Void());
                                }
                                break;
                            }
                            case DT_TIME: {
                                int tem;
                                if (timeParserH(cell.value, tem)) {
                                    dataToAppend.emplace_back(new Time(tem));
                                } else {
                                    dataToAppend.emplace_back(new Void());
                                }
                                break;
                            }
                            case DT_MONTH: {
                                int tem;
                                if (monthParserH(cell.value, tem)) {
                                    dataToAppend.emplace_back(new Month(tem));
                                } else {
                                    dataToAppend.emplace_back(new Void());
                                }
                                break;
                            }
                            case DT_DATE: {
                                int tem;
                                if (dateParserH(cell.value, tem)) {
                                    dataToAppend.emplace_back(new Date(tem));
                                } else {
                                    dataToAppend.emplace_back(new Void());
                                }
                                break;
                            }
                            default: {
                                client_->scannerClose(scanner);
                                throw IllegalArgumentException(__FUNCTION__, "The Type " + vecType->getString(i - 1) + " is not supported");
                            }
                        }
                    }

                    INDEX insertedRows;
                    string errMsg;
                    bool success = result->append(dataToAppend, insertedRows, errMsg);
                    if (!success) {
                        client_->scannerClose(scanner);
                        throw RuntimeException("Error when append table: " + errMsg);
                    }
                }
            }

            client_->scannerClose(scanner);
            return result;
        }
    }

    throw RuntimeException("Table " + tableName + " is not found!");
}

ConstantSP HBaseConnect::getRowH(const string &tableName, const string &rowKey, const vector<string> &columnNames) {
    LockGuard<Mutex> lk(&mtx_);

    try {
        vector<string> tables;
        client_->getTableNames(tables);
        for (const auto &table: tables) {
            if (tableName == table) {
                vector<TRowResult> rowResult;
                const std::map<Text, Text> dummyAttributes;
                if (columnNames.empty()) {
                    client_->getRow(rowResult, tableName, rowKey, dummyAttributes);
                } else {
                    client_->getRowWithColumns(rowResult, tableName, rowKey, columnNames, dummyAttributes);
                }
                vector<string> colNames = {"row"};
                vector<ConstantSP> columns;
                for (auto &res: rowResult) {
                    columns.emplace_back(new String(res.row));
                    for (auto &column: res.columns) {
                        colNames.emplace_back(column.first);
                        columns.emplace_back(new String(column.second.value));
                    }
                }
                if(columns.empty()){
                    throw RuntimeException("HBase: A table has least one column");
                }
                return Util::createTable(colNames, columns);
            }
        }
    } catch (const TException &tx) {
        throw RuntimeException(string("HBase: ") + tx.what());
    }

    throw RuntimeException("Table " + tableName + " is not found!");
}

void HBaseConnect::deleteTableH(const string &tableName) {
    LockGuard<Mutex> lk(&mtx_);
    bool found = false;
    try {
        vector<string> tables;
        client_->getTableNames(tables);
        for (const auto &table: tables) {
            if (tableName == table) {
                if (client_->isTableEnabled(table)) {
                    client_->disableTable(table);
                }
                client_->deleteTable(table);
                found = true;
                break;
            }
        }
    } catch (const TException &tx) {
        throw RuntimeException(string("HBase: ") + tx.what());
    }
    if (!found) {
        throw RuntimeException("Table " + tableName + " is not found!");
    }
}

void HBaseConnect::closeH() {
    LockGuard<Mutex> lk(&mtx_);
    try {
        transport_->close();
        socket_->close();
    } catch (const TException &tx) {
        throw RuntimeException(string("HBase: ") + tx.what());
    }
}


/* HELPERS */

bool HBaseConnect::partialDateParserH(const string &str, bool containDelimitor, int &part1, int &part2) {
    if (str.length() < 3)
        return false;
    unsigned start = 0;
    if (Util::isLetter(str[0])) {
        return false;
    } else {
        part1 = str[0] - '0';
        if (Util::isDigit(str[1])) {
            part1 = part1 * 10 + str[1] - '0';
            start = containDelimitor ? 3 : 2;
        } else
            start = 2;
    }

    part2 = 0;
    while (start < str.length())
        part2 = part2 * 10 + str[start++] - '0';
    return true;
}

bool HBaseConnect::dateParserH(const string &str, int &intVal) {
    intVal = INT_MIN;
    if (str.length() < 6)
        return false;
    int year = 0, month = 0, day = 0;
    //year in the first
    year = (str[0] - '0') * 10 + str[1] - '0';
    if (Util::isDateDelimitor(str[2])) {
        //date=yy-m-d yy-mm-dd
        if (year < 20)
            year += 2000;
        else
            year += 1900;
        partialDateParserH(str.substr(3), true, month, day);
    } else if (str.length() == 6) {
        //date=yymmdd
        if (year < 20)
            year += 2000;
        else
            year += 1900;
        month = (str[2] - '0') * 10 + str[3] - '0';
        day = (str[4] - '0') * 10 + str[5] - '0';
    } else {
        year = year * 100 + (str[2] - '0') * 10 + str[3] - '0';
        if (Util::isDateDelimitor(str[4]))
            partialDateParserH(str.substr(5), true, month, day);
        else
            partialDateParserH(str.substr(4), false, month, day);
    }
    intVal = Util::countDays(year, month, day);
    return true;
}

bool HBaseConnect::monthParserH(const string &str, int &intVal) {
    intVal = INT_MIN;
    if (str.length() < 6)
        return false;
    int year, month;
    if (str.length() == 6) {
        char *pEnd;
        int tem = (int) std::strtol(str.c_str(), &pEnd, 10);
        if (*pEnd == '\0') {
            month = tem % 100;
            if (month > 12)
                return false;
            year = tem / 100;
            intVal = year * 12 + month - 1;
            return true;
        } else {
            return false;
        }
    }
    year = (str[0] - '0') * 1000 + (str[1] - '0') * 100 + (str[2] - '0') * 10 + str[3] - '0';
    if (str[4] != '.')
        return false;
    month = (str[5] - '0') * 10 + str[6] - '0';
    if (month > 12)
        return false;
    intVal = year * 12 + month - 1;
    return true;
}

bool HBaseConnect::timeParserH(const string &str, int &intVal) {
    intVal = INT_MIN;
    if (str.length() != 12 && str.length() != 9)
        return false;
    int hour, minute, second, millisecond;
    if (str.length() == 9) {
        char *pEnd;
        long long tem = std::strtoll(str.c_str(), &pEnd, 10);
        if (*pEnd == '\0') {
            millisecond = int(tem % 1000);
            tem /= 1000;
            second = int(tem % 100);
            tem /= 100;
            minute = int(tem % 100);
            hour = int(tem / 100);
            intVal = ((hour * 60 + minute) * 60 + second) * 1000 + millisecond;
            return true;
        } else {
            return false;
        }
    }
    hour = (str[0] - '0') * 10 + str[1] - '0';
    minute = (str[3] - '0') * 10 + str[4] - '0';
    second = (str[6] - '0') * 10 + str[7] - '0';
    if (hour >= 24 || minute >= 60 || second >= 60)
        return false;
    millisecond = stoi(str.substr(9));
    intVal = ((hour * 60 + minute) * 60 + second) * 1000 + millisecond;
    return true;
}

bool HBaseConnect::secondParserH(const string &str, int &intVal) {
    intVal = INT_MIN;
    if (str.length() < 6)
        return false;
    int hour, minute, second;
    if (str.length() == 6) {
        char *pEnd;
        int tem = (int) std::strtol(str.c_str(), &pEnd, 10);
        if (*pEnd == '\0') {
            second = tem % 100;
            tem /= 100;
            minute = tem % 100;
            hour = tem / 100;
            if (hour >= 24 || minute >= 60 || second >= 60)
                return false;
        } else {
            return false;
        }
    } else if (str.length() == 7) {
        if (str[1] != ':' || str[4] != ':') return false;
        hour = (str[0] - '0');
        minute = (str[2] - '0') * 10 + str[3] - '0';
        second = (str[5] - '0') * 10 + str[6] - '0';
    } else {
        if (str[2] != ':' || str[5] != ':') return false;
        hour = (str[0] - '0') * 10 + str[1] - '0';
        minute = (str[3] - '0') * 10 + str[4] - '0';
        second = (str[6] - '0') * 10 + str[7] - '0';
    }
    if (hour >= 24 || minute >= 60 || second >= 60)
        return false;
    intVal = (hour * 60 + minute) * 60 + second;
    return true;
}

bool HBaseConnect::minuteParserH(const string &str, int &intVal) {
    intVal = INT_MIN;
    auto len = str.length();
    if (len != 4 && len != 5)
        return false;
    int hour, minute;
    if (len == 4) {
        char *pEnd;
        int tem = (int) std::strtol(str.c_str(), &pEnd, 10);
        if (tem < 0) return false;
        if (*pEnd == '\0') {
            minute = tem % 100;
            hour = tem / 100;
            intVal = hour * 60 + minute;
            if (hour >= 24 || minute >= 60)
                return false;
            return true;
        } else {
            return false;
        }
    }
    if (str[2] != ':') return false;
    hour = (str[0] - '0') * 10 + str[1] - '0';
    minute = (str[3] - '0') * 10 + str[4] - '0';
    if (hour >= 24 || minute >= 60)
        return false;
    intVal = hour * 60 + minute;
    return true;
}

bool HBaseConnect::dateTimeParserH(const string &str, int &intVal) {
    intVal = INT_MIN;
    auto len = str.length();
    if (len != 19 && len != 14)
        return false;
    int hour, minute, second;
    if (len == 14) {
        char *pEnd;
        long long tem = std::strtoll(str.c_str(), &pEnd, 10);
        if (*pEnd == '\0') {
            second = int(tem % 100);
            tem = tem / 100;
            minute = int(tem % 100);
            tem = tem / 100;
            hour = int(tem % 100);
            dateParserH(str.substr(0, 8), intVal);
        }
    } else {
        int start = (int) str.length() - 8;
        while (start >= 0 && (str[start] != ' ' && str[start] != 'T')) --start;
        if (start < 0)
            return false;
        int end = start - 1;
        while (end >= 0 && (str[end] == ' ' || str[end] == 'T')) --end;
        if (end < 0)
            return false;
        dateParserH(str.substr(0, end + 1), intVal);
        string t = str.substr(start + 1);
        hour = (t[0] - '0') * 10 + t[1] - '0';
        minute = (t[3] - '0') * 10 + t[4] - '0';
        second = (t[6] - '0') * 10 + t[7] - '0';
    }
    if (intVal == INT_MIN)
        return false;
    if (hour >= 24 || minute >= 60 || second >= 60) {
        intVal = INT_MIN;
        return false;
    }
    intVal = intVal * 86400 + (hour * 60 + minute) * 60 + second;
    return true;
}

bool HBaseConnect::nanoTimeParserH(const string &str, long long &longVal) {
    longVal = LLONG_MIN;
    if (str.length() == 15 && str[2] != ':') {
        int hour, minute, second, nanosecond = 0;
        hour = (str[0] - '0') * 10 + str[1] - '0';
        minute = (str[2] - '0') * 10 + str[3] - '0';
        second = (str[4] - '0') * 10 + str[5] - '0';
        for (int i = 6; i < 15; ++i)
            nanosecond = nanosecond * 10 + str[i] - '0';
        if (hour >= 24 || minute >= 60 || second >= 60)
            return false;
        longVal = ((hour * 60 + minute) * 60 + second) * 1000000000ll + nanosecond;
    } else if (str.length() == 15 || str.length() == 18) {
        int hour, minute, second, nanosecond = 0;
        hour = (str[0] - '0') * 10 + str[1] - '0';
        minute = (str[3] - '0') * 10 + str[4] - '0';
        second = (str[6] - '0') * 10 + str[7] - '0';
        if (hour >= 24 || minute >= 60 || second >= 60)
            return false;

        nanosecond = stoi(str.substr(9));
        if (str.length() == 15)
            nanosecond *= 1000;
        longVal = ((hour * 60 + minute) * 60 + second) * 1000000000ll + nanosecond;
    }
    return true;
}

bool HBaseConnect::nanoTimestampParserH(const string &str, long long &longVal) {
    longVal = LLONG_MIN;
    auto len = str.length();
    if (len != 23 && len != 29)
        return false;
    if (len == 23) {
        char *pEnd;
        long long tem = std::strtoll(str.c_str() + 8, &pEnd, 10);
        if (*pEnd == '\0') {
            int hour, minute, second, nanosecond;
            nanosecond = int(tem % 1000000000ll);
            tem = tem / 1000000000ll;
            second = int(tem % 100);
            tem = tem / 100;
            minute = int(tem % 100);
            tem = tem / 100;
            hour = int(tem % 100);
            int intVal;
            dateParserH(str.substr(0, 8), intVal);
            if (intVal == INT_MIN || hour >= 24 || minute >= 60 || second >= 60) {
                return false;
            }
            longVal = intVal * 86400000000000ll + ((hour * 60 + minute) * 60 + second) * 1000000000ll + nanosecond;
            return true;
        } else {
            return false;
        }
    }
    int dateLen = (int) str.length() - 16;
    if (str[dateLen] != ' ' && str[dateLen] != 'T') {
        dateLen -= 3;
        if (str[dateLen] != ' ' && str[dateLen] != 'T')
            return false;
    }
    int intVal;
    dateParserH(str.substr(0, dateLen), intVal);
    if (intVal == INT_MIN) {
        longVal = LLONG_MIN;
        return false;
    }

    int hour, minute, second, nanosecond = 0;
    string t = str.substr(dateLen + 1);
    hour = (t[0] - '0') * 10 + t[1] - '0';
    minute = (t[3] - '0') * 10 + t[4] - '0';
    second = (t[6] - '0') * 10 + t[7] - '0';
    if (hour >= 24 || minute >= 60 || second >= 60) {
        longVal = LLONG_MIN;
        return false;
    }
    nanosecond = stoi(t.substr(9));
    if (t.length() - dateLen == 16)
        nanosecond *= 1000;
    longVal = intVal * 86400000000000ll + ((hour * 60 + minute) * 60 + second) * 1000000000ll + nanosecond;
    return true;
}

bool HBaseConnect::timestampParserH(const string &str, long long &longVal) {
    longVal = LLONG_MIN;
    auto len = str.length();
    if (len != 17 && len != 23)
        return false;
    if (len == 17) {
        char *pEnd;
        long long tem = std::strtoll(str.c_str(), &pEnd, 10);
        if (*pEnd == '\0') {
            int hour, minute, second, millisecond;
            millisecond = int(tem % 1000);
            tem = tem / 1000;
            second = int(tem % 100);
            tem = tem / 100;
            minute = int(tem % 100);
            tem = tem / 100;
            hour = int(tem % 100);
            int intVal;
            dateParserH(str.substr(0, 8), intVal);
            if (intVal == INT_MIN || hour >= 24 || minute >= 60 || second >= 60) {
                return false;
            }
            longVal = intVal * 86400000ll + ((hour * 60 + minute) * 60 + second) * 1000 + millisecond;
            return true;
        } else {
            return false;
        }
    }

    int start = (int) str.length() - 12;
    while (start >= 0 && (str[start] != ' ' && str[start] != 'T')) --start;
    if (start < 0)
        return false;
    int intVal;
    dateParserH(str.substr(0, 10), intVal);
    if (intVal == INT_MIN) {
        longVal = LLONG_MIN;
        return false;
    }
    int hour, minute, second, millisecond = 0;
    string t = str.substr(start + 1);
    hour = (t[0] - '0') * 10 + t[1] - '0';
    minute = (t[3] - '0') * 10 + t[4] - '0';
    second = (t[6] - '0') * 10 + t[7] - '0';
    if (hour >= 24 || minute >= 60 || second >= 60) {
        longVal = LLONG_MIN;
        return false;
    }
    if (t[8] == '.')
        millisecond = stoi(t.substr(9));
    else
        return false;
    longVal = intVal * 86400000ll + ((hour * 60 + minute) * 60 + second) * 1000 + millisecond;
    return true;
}

void connectionOnCloseH(Heap *heap, vector<ConstantSP> &args) {
  auto conn = HBASE_CONNECTION_MAP.safeGet(args[0]);
  if (conn.get()) {
    conn->closeH();
    conn.clear();
  }
}

void HBaseConnect::customThriftLogFunction(const char *message) {}


