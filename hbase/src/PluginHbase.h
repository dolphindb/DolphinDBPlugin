//
// Created by lin on 2021/2/23.
//

#ifndef PLUGINHBASE_PLUGINHBASE_H
#define PLUGINHBASE_PLUGINHBASE_H

#include "DolphinDBEverything.h"
#include "CoreConcept.h"
#include "Hbase.h"
#include <transport/TSocket.h>
#include "ddbplugin/CommonInterface.h"
#include "ddbplugin/PluginLoggerImp.h"
/* INTERFACES */

extern "C" {
ddb::ConstantSP connectH(ddb::Heap *heap, argsT &args);
ddb::ConstantSP showTablesH(ddb::Heap *heap, argsT &args);
ddb::ConstantSP loadH(ddb::Heap *heap, argsT &args);
ddb::ConstantSP deleteTableH(ddb::Heap *heap, argsT &args);
ddb::ConstantSP getRowH(ddb::Heap *heap, argsT &args);
}

/* HBASECONNECT */

class HBaseConnect {
public:
    HBaseConnect(const string &hostname, int port, bool isFramed, int timeout);
    ~HBaseConnect() = default;
    void closeH();
    ddb::ConstantSP showTablesH();
    ddb::ConstantSP loadH(const std::string &tableName);
    ddb::ConstantSP loadH(const std::string &tableName, const ddb::TableSP &schema);
    void deleteTableH(const std::string &tableName);
    ddb::ConstantSP getRowH(const std::string &tableName, const std::string &rowKey, const std::vector<std::string> &columnNames);

private:
    std::shared_ptr<apache::thrift::transport::TSocket> socket_;
    std::shared_ptr<apache::thrift::transport::TTransport> transport_;
    std::shared_ptr<apache::hadoop::hbase::thrift::HbaseClient> client_;

    ddb::Mutex mtx_;

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

    static void customThriftLogFunction(const char *message);
};


/* HELPERS */
void connectionOnCloseH(ddb::Heap *heap, argsT &args);



#endif//PLUGINHBASE_PLUGINHBASE_H
