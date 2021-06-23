//
// Created by lin on 2021/2/23.
//

#ifndef PLUGINHBASE_PLUGINHBASE_H
#define PLUGINHBASE_PLUGINHBASE_H

#include "CoreConcept.h"
#include "Hbase.h"
#include <transport/TSocket.h>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace apache::hadoop::hbase::thrift;

extern "C" ConstantSP hbaseConnect(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP showTables(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP load(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP deleteTable(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP getRow(Heap *heap, vector<ConstantSP> &args);

class HbaseConnect{
public:
    HbaseConnect(const string &hostname, const int port, bool isFramed, int timeout);
    ~HbaseConnect(){}
    void close();
    ConstantSP showTables();
    ConstantSP load(const std::string &tableName);
    ConstantSP load(const std::string &tableName, TableSP schema);
    void deleteTable(const std::string &tableName);
    ConstantSP getRow(const std::string &tableName, const std::string &rowKey);
    ConstantSP getRow(const std::string &tableName, const std::string &rowKey, const std::vector<std::string> &columnNames);

private:
    std::string host_;
    int port_;
    //std::shared_ptr<TTransport> socket_;
    std::shared_ptr<TSocket> socket_;
    std::shared_ptr<TTransport> transport_;
    std::shared_ptr<TProtocol> protocol_;
    std::shared_ptr<HbaseClient> client_;

};
#endif //PLUGINHBASE_PLUGINHBASE_H
