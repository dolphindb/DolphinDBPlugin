// SPDX-License-Identifier: Apache-2.0
// Copyright © 2025-2026 DolphinDB, Inc.
#pragma once

#include "DolphinDBEverything.h"
#include "CoreConcept.h"
#include "Hbase.h"
#include <thrift/transport/TSocket.h>
#include "ddbplugin/PluginLogger.h"
#include "ddbplugin/Plugin.h"

struct HBaseScanOptions {
    bool hasStartRow = false;
    std::string startRow;
    bool hasStopRow = false;
    std::string stopRow;
    bool hasColumns = false;
    std::vector<std::string> columns;
    bool hasCaching = false;
    int caching = 0;
    bool hasFilterString = false;
    std::string filterString;
};

enum class HBaseTransportMode {
    Auto,
    Buffered,
    Framed,
};

/* HBASECONNECT */

class HBaseConnect {
public:
    HBaseConnect(const string &hostname, int port, HBaseTransportMode transportMode, int timeout);
    ~HBaseConnect() = default;
    void closeH();
    ConstantSP showTablesH();
    ConstantSP loadH(const std::string &tableName, const TableSP &schema = TableSP(),
                          const HBaseScanOptions &scanOptions = HBaseScanOptions());
    void deleteTableH(const std::string &tableName);
    ConstantSP getRowH(const std::string &tableName, const std::string &rowKey, const std::vector<std::string> &columnNames);
    static HBaseScanOptions parseScanOptions(const ConstantSP &config);

private:
    std::shared_ptr<apache::thrift::transport::TSocket> socket_;
    std::shared_ptr<apache::thrift::transport::TTransport> transport_;
    std::shared_ptr<apache::hadoop::hbase::thrift::HbaseClient> client_;

    Mutex mtx_;

    /* HELPERS */
    static bool partialDateParserH(const string &str, bool containDelimitor, int &part1, int &part2);
    static bool dateParserH(const string &str, int &intVal);
    static bool monthParserH(const string &str, int &intVal);
    static bool timeParserH(const string &str, int &intVal);
    static bool secondParserH(const string &str, int &intVal);
    static bool minuteParserH(const string &str, int &intVal);
    static bool dateTimeParserH(const string &str, int &intVal);
    static bool nanoTimeParserH(const string &str, long long &longVal);
    static bool nanoTimestampParserH(const string &str, long long &longVal);
    static bool timestampParserH(const string &str, long long &longVal);
    int openScanner(const std::string &tableName, const std::vector<std::string> &columns, const HBaseScanOptions *scanOptions);
    void connectWithProtocol(const std::string &hostname, int port, HBaseTransportMode transportMode, int timeout, bool useCompactProtocol);
    void closeConnectionQuietly();
};


/* HELPERS */
void connectionOnCloseH(Heap *heap, argsT &args);

extern ddb::ResourceMap<HBaseConnect> HBASE_CONNECTION_MAP;
