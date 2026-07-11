//
// Created by lin on 2021/2/23.
//

#include "PluginHbase.h"
#include "ScalarImp.h"
#include "Util.h"

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/protocol/TCompactProtocol.h>
#include <thrift/transport/TTransportUtils.h>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::hadoop::hbase::thrift;

using namespace ddb;

namespace {

std::string schemaColumnHint() {
    return " This may be caused by invalid column names in schema.";
}

bool hasColumnName(const vector<string> &colNames, const string &columnName) {
    return std::find(colNames.begin() + 1, colNames.end(), columnName) != colNames.end();
}

void appendStringCellsByColumnNames(const TRowResult &row, const vector<string> &colNames, vector<ConstantSP> &dataToAppend) {
    for (size_t i = 1; i < colNames.size(); ++i) {
        auto cell = row.columns.find(colNames[i]);
        if (cell == row.columns.end()) {
            dataToAppend.emplace_back(new Void());
        } else {
            dataToAppend.emplace_back(new String(cell->second.value));
        }
    }
}

const char *protocolName(bool useCompactProtocol) {
    return useCompactProtocol ? "TCompactProtocol" : "TBinaryProtocol";
}

const char *transportName(HBaseTransportMode transportMode) {
    return transportMode == HBaseTransportMode::Framed ? "TFramedTransport" : "TBufferedTransport";
}

string connectionModeName(HBaseTransportMode transportMode, bool useCompactProtocol) {
    return string(transportName(transportMode)) + " + " + protocolName(useCompactProtocol);
}
}

HBaseConnect::HBaseConnect(const string &hostname, const int port, HBaseTransportMode transportMode, int timeout)
{
    struct ConnectCandidate {
        HBaseTransportMode transportMode;
        bool useCompactProtocol;
    };

    vector<ConnectCandidate> candidates;
    if (transportMode == HBaseTransportMode::Auto) {
        candidates = {
            {HBaseTransportMode::Framed, true},
            {HBaseTransportMode::Buffered, false},
            {HBaseTransportMode::Framed, false},
            {HBaseTransportMode::Buffered, true},
        };
    } else {
        candidates = {
            {transportMode, false},
            {transportMode, true},
        };
    }

    vector<string> errors;
    for (const auto &candidate: candidates) {
        try {
            connectWithProtocol(hostname, port, candidate.transportMode, timeout, candidate.useCompactProtocol);
            return;
        } catch (const TException &tx) {
            errors.emplace_back(connectionModeName(candidate.transportMode, candidate.useCompactProtocol) + " error: " + tx.what());
            closeConnectionQuietly();
        }
    }

    string msg = "HBase: failed to connect to the HBase Thrift server.";
    for (const auto &error: errors) {
        msg += "\n" + error;
    }
    msg += "\nThe port number may be wrong (not for HBase Thrift server, default is 9090), or the server transport/protocol does not match the client.";
    throw RuntimeException(msg);
}

void HBaseConnect::connectWithProtocol(const string &hostname, int port, HBaseTransportMode transportMode, int timeout, bool useCompactProtocol) {
    socket_ = std::make_shared<apache::thrift::transport::TSocket>(hostname, port);
    socket_->setConnTimeout(timeout);
    socket_->setRecvTimeout(timeout);

    if (transportMode == HBaseTransportMode::Framed) {
        transport_ = std::make_shared<apache::thrift::transport::TFramedTransport>(socket_);
    } else {
        transport_ = std::make_shared<apache::thrift::transport::TBufferedTransport>(socket_);
    }

    std::shared_ptr<TProtocol> protocol;
    if (useCompactProtocol) {
        protocol = std::make_shared<TCompactProtocol>(transport_);
    } else {
        protocol = std::make_shared<TBinaryProtocol>(transport_);
    }
    client_ = std::make_shared<HbaseClient>(protocol);

    transport_->open();

    if (!transport_->isOpen()) {
        throw TException("Failed to connect to the HBase Thrift server");
    }

    // Fetch table names to verify that transport and protocol match the server.
    vector<string> tableNames;
    try {
        client_->getTableNames(tableNames);
    } catch (const TException &tx) {
        throw TException(string(protocolName(useCompactProtocol)) + " probe failed: " + tx.what());
    }
}

void HBaseConnect::closeConnectionQuietly() {
    try {
        if (transport_ != nullptr) {
            transport_->close();
        }
        if (socket_ != nullptr) {
            socket_->close();
        }
    } catch (const TException &) {
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

HBaseScanOptions HBaseConnect::parseScanOptions(const ConstantSP &config) {
    if (config == nullptr || config->isNull() || config->isNothing()) {
        return HBaseScanOptions();
    }
    if (config->getForm() != DF_DICTIONARY) {
        throw IllegalArgumentException(__FUNCTION__, "scanOptions must be a dictionary.");
    }
    DictionarySP dict = config;
    if (dict->getKeyType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, "scanOptions must be a dictionary whose key type is STRING.");
    }
    VectorSP keys = dict->keys();
    for (int i = 0; i < keys->size(); ++i) {
        string key = keys->getString(i);
        if (key != "startRow" && key != "stopRow" && key != "columns" &&
            key != "caching" && key != "filterString") {
            throw IllegalArgumentException(__FUNCTION__,
                "Unsupported scanOptions key \"" + key +
                "\". Supported keys are: startRow, stopRow, columns, caching, filterString.");
        }
    }

    HBaseScanOptions options;

    ConstantSP startRow = dict->getMember("startRow");
    if (!startRow->isNull()) {
        if (startRow->getType() != DT_STRING || startRow->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, "scanOptions startRow must be a STRING scalar.");
        }
        options.hasStartRow = true;
        options.startRow = startRow->getString();
    }

    ConstantSP stopRow = dict->getMember("stopRow");
    if (!stopRow->isNull()) {
        if (stopRow->getType() != DT_STRING || stopRow->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, "scanOptions stopRow must be a STRING scalar.");
        }
        options.hasStopRow = true;
        options.stopRow = stopRow->getString();
    }

    ConstantSP columns = dict->getMember("columns");
    if (!columns->isNull()) {
        if (columns->getType() != DT_STRING || (columns->getForm() != DF_SCALAR && columns->getForm() != DF_VECTOR)) {
            throw IllegalArgumentException(__FUNCTION__, "scanOptions columns must be a STRING scalar or STRING vector.");
        }
        options.hasColumns = true;
        if (columns->getForm() == DF_SCALAR) {
            options.columns.emplace_back(columns->getString());
        } else {
            int size = columns->size();
            options.columns.reserve(size);
            for (int i = 0; i < size; ++i) {
                options.columns.emplace_back(columns->getString(i));
            }
        }
    }

    ConstantSP caching = dict->getMember("caching");
    if (!caching->isNull()) {
        if (caching->getForm() != DF_SCALAR || caching->getType() != DT_INT) {
            throw IllegalArgumentException(__FUNCTION__, "scanOptions caching must be a numeric scalar.");
        }
        int value = caching->getInt();
        if (value <= 0) {
            throw IllegalArgumentException(__FUNCTION__, "scanOptions caching must be a positive integer.");
        }
        options.hasCaching = true;
        options.caching = value;
    }

    ConstantSP filterString = dict->getMember("filterString");
    if (!filterString->isNull()) {
        if (filterString->getType() != DT_STRING || filterString->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, "scanOptions filterString must be a STRING scalar.");
        }
        options.hasFilterString = true;
        options.filterString = filterString->getString();
    }

    return options;
}

int HBaseConnect::openScanner(const std::string &tableName, const std::vector<std::string> &columns, const HBaseScanOptions *scanOptions) {
    const std::map<Text, Text> dummyAttributes;
    if (scanOptions == nullptr || (!scanOptions->hasStartRow && !scanOptions->hasStopRow &&
        !scanOptions->hasColumns && !scanOptions->hasCaching && !scanOptions->hasFilterString)) {
        return client_->scannerOpen(tableName, "", columns, dummyAttributes);
    }

    TScan scan;
    if (scanOptions->hasStartRow) {
        scan.__set_startRow(scanOptions->startRow);
    }
    if (scanOptions->hasStopRow) {
        scan.__set_stopRow(scanOptions->stopRow);
    }
    if (scanOptions->hasColumns) {
        scan.__set_columns(scanOptions->columns);
    } else if (!columns.empty()) {
        scan.__set_columns(columns);
    }
    if (scanOptions->hasCaching) {
        scan.__set_caching(scanOptions->caching);
    }
    if (scanOptions->hasFilterString) {
        scan.__set_filterString(scanOptions->filterString);
    }

    return client_->scannerOpenWithScan(tableName, scan, dummyAttributes);
}

ConstantSP HBaseConnect::loadH(const string &tableName, const TableSP &schema, const HBaseScanOptions &scanOptions) {
    LockGuard<Mutex> lk(&mtx_);

    bool hasSchema = !schema.isNull();
    vector<string> colNames{"row"};
    vector<string> columnNames;
    vector<DATA_TYPE> colTypes;
    TableSP result;
    vector<TRowResult> scannedRows;

    if (hasSchema) {
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

        colTypes.emplace_back(DT_STRING);
        for (int i = 0; i < vecName->size(); ++i) {
            string columnName = vecName->getString(i);
            colNames.emplace_back(columnName);
            columnNames.emplace_back(columnName);

            string sType = vecType->getString(i);
            std::transform(sType.begin(), sType.end(), sType.begin(), ::toupper);
            if (sType == "BOOL") {
                colTypes.push_back(DT_BOOL);
            } else if (sType == "CHAR") {
                colTypes.push_back(DT_CHAR);
            } else if (sType == "SHORT") {
                colTypes.push_back(DT_SHORT);
            } else if (sType == "INT") {
                colTypes.push_back(DT_INT);
            } else if (sType == "LONG") {
                colTypes.push_back(DT_LONG);
            } else if (sType == "DATE") {
                colTypes.push_back(DT_DATE);
            } else if (sType == "MONTH") {
                colTypes.push_back(DT_MONTH);
            } else if (sType == "TIME") {
                colTypes.push_back(DT_TIME);
            } else if (sType == "MINUTE") {
                colTypes.push_back(DT_MINUTE);
            } else if (sType == "SECOND") {
                colTypes.push_back(DT_SECOND);
            } else if (sType == "DATETIME") {
                colTypes.push_back(DT_DATETIME);
            } else if (sType == "TIMESTAMP") {
                colTypes.push_back(DT_TIMESTAMP);
            } else if (sType == "NANOTIME") {
                colTypes.push_back(DT_NANOTIME);
            } else if (sType == "NANOTIMESTAMP") {
                colTypes.push_back(DT_NANOTIMESTAMP);
            } else if (sType == "FLOAT") {
                colTypes.push_back(DT_FLOAT);
            } else if (sType == "DOUBLE") {
                colTypes.push_back(DT_DOUBLE);
            } else if (sType == "SYMBOL") {
                colTypes.push_back(DT_SYMBOL);
            } else if (sType == "STRING") {
                colTypes.push_back(DT_STRING);
            } else {
                throw IllegalArgumentException(__FUNCTION__, "The Type " + sType + " is not supported");
            }
        }

        result = Util::createTable(colNames, colTypes, 0, 10);
    }

    vector<string> tables;
    try {
        client_->getTableNames(tables);
    } catch (TException &tx) {
        throw RuntimeException(string("HBase getTableNames error: ") + tx.what());
    }

    int scanner;
    for (const auto &table: tables) {
        if (tableName != table) {
            continue;
        }

        try {
            try {
                scanner = openScanner(tableName, columnNames, &scanOptions);
            } catch (const TException &tx) {
                string msg = string("HBase scannerOpen error: ") + tx.what();
                if (hasSchema) {
                    msg += schemaColumnHint();
                }
                throw RuntimeException(msg);
            }

            while (true) {
                vector<TRowResult> values;
                try {
                    client_->scannerGetList(values, scanner, 1024);
                } catch (const TException &tx) {
                    client_->scannerClose(scanner);
                    string msg = string("HBase scannerGetList error: ") + tx.what();
                    if (hasSchema) {
                        msg += schemaColumnHint();
                    }
                    throw RuntimeException(msg);
                }
                if (values.empty()) {
                    break;
                }

                for (auto &val: values) {
                    if (!hasSchema) {
                        scannedRows.emplace_back(val);
                        for (auto &column: val.columns) {
                            if (!hasColumnName(colNames, column.first)) {
                                colNames.emplace_back(column.first);
                            }
                        }
                    } else {
                        vector<ConstantSP> dataToAppend;
                        dataToAppend.emplace_back(new String(val.row));
                        for (size_t i = 1; i < colNames.size(); ++i) {
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
                                    } else if (tem == "FALSE" || tem == "0") {
                                        dataToAppend.emplace_back(new Bool(0));
                                    } else {
                                        dataToAppend.emplace_back(new Void());
                                    }
                                    break;
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
                                default:
                                    client_->scannerClose(scanner);
                                    throw RuntimeException("Impossible type is parsed.");
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
            }

            client_->scannerClose(scanner);
            LOG_INFO("[PluginHbase] Load Success");
            if (!hasSchema) {
                colTypes.assign(colNames.size(), DT_STRING);
                result = Util::createTable(colNames, colTypes, 0, 10);
                for (const auto &row: scannedRows) {
                    vector<ConstantSP> dataToAppend;
                    dataToAppend.emplace_back(new String(row.row));
                    appendStringCellsByColumnNames(row, colNames, dataToAppend);

                    INDEX insertedRows;
                    string errMsg;
                    bool success = result->append(dataToAppend, insertedRows, errMsg);
                    if (!success) {
                        throw RuntimeException("Error when append table: " + errMsg);
                    }
                }
            }
            return result;
        } catch (const TException &tx) {
            string msg = string("HBase scanner error: ") + tx.what();
            if (hasSchema) {
                msg += schemaColumnHint();
            }
            throw RuntimeException(msg);
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
    int hour{0}, minute{0}, second{0};
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

void connectionOnCloseH(Heap *heap, vector<ConstantSP> &args)
{
    std::ignore = heap;
  auto conn = HBASE_CONNECTION_MAP.safeGet(args[0]);
  if (conn.get()) {
    conn->closeH();
    conn.clear();
  }
}
