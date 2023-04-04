//
// Created by lin on 2021/2/23.
//

#ifndef PLUGINHBASE_PLUGINHBASE_H
#define PLUGINHBASE_PLUGINHBASE_H

#include "CoreConcept.h"
#include "Hbase.h"
#include <transport/TSocket.h>


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
    std::shared_ptr<apache::thrift::transport::TSocket> socket_;
    std::shared_ptr<apache::thrift::transport::TTransport> transport_;
    std::shared_ptr<apache::thrift::protocol::TProtocol> protocol_;
    std::shared_ptr<apache::hadoop::hbase::thrift::HbaseClient> client_;

    Mutex mtx_;
};
#endif //PLUGINHBASE_PLUGINHBASE_H
