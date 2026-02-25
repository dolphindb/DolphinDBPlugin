// SPDX-License-Identifier: Apache-2.0
// Copyright © 2025-2025 DolphinDB, Inc.
#pragma once

#include "DolphinDBEverything.h"
#include "Concurrent.h"
#include "ScalarImp.h"
#include "ddbplugin/ThreadedQueue.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#include "zmq.hpp"
#pragma GCC diagnostic pop
#include "json/json.hpp"
#include "ddbplugin/Plugin.h"
#include "ddbplugin/PluginLogger.h"

using argsT = std::vector<ddb::ConstantSP>;

extern "C" {

ddb::ConstantSP zmqSocket(ddb::Heap *heap, argsT &args);
ddb::ConstantSP zmqSend(ddb::Heap *heap, argsT &args);
ddb::ConstantSP zmqConnect(ddb::Heap *heap, argsT &args);
ddb::ConstantSP zmqBind(ddb::Heap *heap, argsT &args);
ddb::ConstantSP zmqClose(ddb::Heap *heap, argsT &args);
ddb::ConstantSP zmqCreateSubJob(ddb::Heap *heap, argsT &args);
ddb::ConstantSP zmqCancelSubJob(ddb::Heap *heap, argsT args);
ddb::ConstantSP zmqGetSubJobStat(ddb::Heap *heap, argsT &args);
ddb::ConstantSP zmqCreatePusher(ddb::Heap *heap, argsT &args);
ddb::ConstantSP zmqSetMonitor(ddb::Heap *heap, argsT &args);

}

using namespace std;
using json = nlohmann::json;
static const std::string PLUGIN_ZMQ_PREFIX = "[PLUGIN::ZMQ]: ";

namespace ddb {

static shared_ptr<zmq::socket_t> createZmqSocket(zmq::context_t &context, const string &socketType);

class ZMQMonitor : public zmq::monitor_t{
public:
    ZMQMonitor() {}
    virtual ~ZMQMonitor() {}
    virtual void on_event_connected(const zmq_event_t &event_, const char *addr_) override {
        std::ignore = event_;
        LOG_INFO(PLUGIN_ZMQ_PREFIX + "Connected to " + string(addr_));
    }
    virtual void on_event_connect_delayed(const zmq_event_t &event_, const char *addr_) override {
        std::ignore = event_;
        LOG_INFO(PLUGIN_ZMQ_PREFIX + "Connect delayed to " + string(addr_));
    }
    virtual void on_event_connect_retried(const zmq_event_t &event_, const char *addr_) override {
        std::ignore = event_;
        LOG_INFO(PLUGIN_ZMQ_PREFIX + "Connect retried to " + string(addr_));
    }
    virtual void on_event_listening(const zmq_event_t &event_, const char *addr_) override {
        std::ignore = event_;
        LOG_INFO(PLUGIN_ZMQ_PREFIX + "Listening on " + string(addr_));
    }
    virtual void on_event_bind_failed(const zmq_event_t &event_, const char *addr_) override {
        std::ignore = event_;
        LOG_INFO(PLUGIN_ZMQ_PREFIX + "Bind failed on " + string(addr_));
    }
    virtual void on_event_accepted(const zmq_event_t &event_, const char *addr_) override {
        std::ignore = event_;
        LOG_INFO(PLUGIN_ZMQ_PREFIX + "Accepted on " + string(addr_));
    }
    virtual void on_event_accept_failed(const zmq_event_t &event_, const char *addr_) override {
        std::ignore = event_;
        LOG_INFO(PLUGIN_ZMQ_PREFIX + "Accept failed on " + string(addr_));
    }
    virtual void on_event_closed(const zmq_event_t &event_, const char *addr_) override {
        std::ignore = event_;
        LOG_INFO(PLUGIN_ZMQ_PREFIX + "Closed on " + string(addr_));
    }
    virtual void on_event_close_failed(const zmq_event_t &event_, const char *addr_) override {
        std::ignore = event_;
        LOG_INFO(PLUGIN_ZMQ_PREFIX + "Close failed on " + string(addr_));
    }
    virtual void on_event_disconnected(const zmq_event_t &event_, const char *addr_) override {
        std::ignore = event_;
        LOG_INFO(PLUGIN_ZMQ_PREFIX + "Disconnected on " + string(addr_));
    }
};

struct HashSocket{
    size_t operator()(const shared_ptr<zmq::socket_t>& socket) const{
        return std::hash<void*>()(socket.get());
    }
};

class ZMQExecutor;

class ZmqStatus{
public:
    static DictionarySP STATUS_DICT;
    static Mutex GLOBAL_LOCK;
    static std::unordered_map<shared_ptr<zmq::socket_t>, shared_ptr<ZMQMonitor>, HashSocket> ZMQ_MONITOR_MAP;
    static Mutex ZMQ_MONITOR_MAP_LOCK;
    static bool SET_MONITOR;
    static SmartPointer<ZMQExecutor> MONITOR_EXECUTOR;
    static SmartPointer<Thread> MONITOR_THREAD;
};

class ZMQExecutor : public Runnable {
public:
    ZMQExecutor(): isStop_(false) {};
    void run() override {
        try{
            while(true){
                {
                    LockGuard<Mutex> _(&ZmqStatus::ZMQ_MONITOR_MAP_LOCK);
                    if(ZmqStatus::SET_MONITOR){
                        for(auto iter : ZmqStatus::ZMQ_MONITOR_MAP){
                            if(iter.second.get() == nullptr){
                                ZmqStatus::ZMQ_MONITOR_MAP[iter.first] = shared_ptr<ZMQMonitor>(new ZMQMonitor());
                                ZmqStatus::ZMQ_MONITOR_MAP[iter.first]->init(*iter.first, "inproc://monitor.rep");
                            }
                            ZmqStatus::ZMQ_MONITOR_MAP[iter.first]->check_event();
                        }
                    }else{
                        for(auto iter : ZmqStatus::ZMQ_MONITOR_MAP){
                            iter.second.reset();
                        }
                    }
                    if(isStop_)
                        break;
                }
                Util::sleep(1000);
            }
        }
        catch(...){
            LOG_ERR(PLUGIN_ZMQ_PREFIX + "an uncaught exception was found");
        }
    };

    void stop(){
        isStop_ = true;
    }
private:
    bool isStop_;
};

DictionarySP ZmqStatus::STATUS_DICT = Util::createDictionary(DT_STRING, nullptr, DT_ANY, nullptr);
Mutex ZmqStatus::GLOBAL_LOCK;
std::unordered_map<shared_ptr<zmq::socket_t>, shared_ptr<ZMQMonitor>, HashSocket> ZmqStatus::ZMQ_MONITOR_MAP;
Mutex ZmqStatus::ZMQ_MONITOR_MAP_LOCK;
bool ZmqStatus::SET_MONITOR = false;
SmartPointer<ZMQExecutor> ZmqStatus::MONITOR_EXECUTOR = nullptr;
SmartPointer<Thread> ZmqStatus::MONITOR_THREAD = nullptr;

class ZmqSocket {
public:
    ZmqSocket(const string &type)
        : context(), zmq_Socket_(createZmqSocket(context, type)) {
        LockGuard<Mutex> _(&ZmqStatus::ZMQ_MONITOR_MAP_LOCK);
        ZmqStatus::ZMQ_MONITOR_MAP[zmq_Socket_] = shared_ptr<ZMQMonitor>(new ZMQMonitor());
        ZmqStatus::ZMQ_MONITOR_MAP[zmq_Socket_] ->init(*zmq_Socket_, "inproc://monitor.rep");
        if(ZmqStatus::MONITOR_THREAD.isNull()){
            ZmqStatus::MONITOR_EXECUTOR = new ZMQExecutor();
            ZmqStatus::MONITOR_THREAD = new Thread(ZmqStatus::MONITOR_EXECUTOR);
            ZmqStatus::MONITOR_THREAD->start();
        }
    }

    void connect(const string &addr, const string &prefix) {
        ZmqSocket::checkFileNum();
        zmq_Socket_->set(zmq::sockopt::tcp_keepalive, 1);
        zmq_Socket_->set(zmq::sockopt::tcp_keepalive_idle, 30);
        zmq_Socket_->set(zmq::sockopt::tcp_keepalive_cnt, 5);
        zmq_Socket_->set(zmq::sockopt::tcp_keepalive_intvl, 1);

        zmq_Socket_->connect(addr);
        addr_ = addr;
        prefix_ = prefix;
    }

    void bind(const string addr, const string &prefix) {
        ZmqSocket::checkFileNum();
        addr_ = addr;
        zmq_Socket_->bind(addr);
        prefix_ = prefix;
    }

    // string str() {
    //     return addr_;
    // }

    virtual ~ZmqSocket() {
        LOG_INFO("PluginZmq: socket[" + addr_ + "] is closed. ");
        LockGuard<Mutex> _(&ZmqStatus::ZMQ_MONITOR_MAP_LOCK);
        if(ZmqStatus::ZMQ_MONITOR_MAP.erase(zmq_Socket_) == 0){
            LOG_ERR(PLUGIN_ZMQ_PREFIX + "Failed to erase the monitor map");
        }
        if(ZmqStatus::ZMQ_MONITOR_MAP.size() == 0 && !ZmqStatus::MONITOR_THREAD.isNull()){
            SmartPointer<Thread> thread = ZmqStatus::MONITOR_THREAD;
            SmartPointer<ZMQExecutor> executor = ZmqStatus::MONITOR_EXECUTOR;
            ZmqStatus::MONITOR_THREAD.clear();
            ZmqStatus::MONITOR_EXECUTOR.clear();
            if(!executor.isNull()) executor->stop();
            _.unlock();
            thread->join();
        }
    }

    shared_ptr<zmq::socket_t> getSocket() {
        return zmq_Socket_;
    }

    string getPrefix() {
        return prefix_;
    }

    string getAddr() {
        return addr_;
    }

    string getType() {
        return type_;
    }

    static void runCmd(const std::string& cmd, std::string& output) {
        FILE* fp = NULL;
        char data[1024] = {0};
        fp = popen(cmd.c_str(), "r");
        if (fp == nullptr) { throw RuntimeException(PLUGIN_ZMQ_PREFIX + "Failed to popen: " + std::strerror(errno)); }
        int ret = fread(data, 1, sizeof(data) - 1, fp);
        if (ret < 0) {
            pclose(fp);
            throw RuntimeException(PLUGIN_ZMQ_PREFIX + "Failed to fread: " + std::strerror(errno));
        }
        output.assign(data);
        if (0 != pclose(fp)) { throw RuntimeException(PLUGIN_ZMQ_PREFIX  + "Failed to pclose: " + std::strerror(errno)); }
    }

    static void checkFileNum() {
        std::string cmd("ls /proc/self/fd | wc -l");
        std::string currentOpenFileNumStr, maxOpenFileNumStr;
        runCmd(cmd, currentOpenFileNumStr);
        cmd = "ulimit -n";
        runCmd(cmd, maxOpenFileNumStr);
        long currentOpenFileNum;
        long maxOpenFileNum;
        try{
            currentOpenFileNum = std::strtol(currentOpenFileNumStr.c_str(), NULL, 10);
        }catch(exception &e){
            throw RuntimeException(PLUGIN_ZMQ_PREFIX + "Failed to parse long: " + e.what());
        }
        try{
            maxOpenFileNum = std::strtol(maxOpenFileNumStr.c_str(), NULL, 10);
        }catch(exception &e){
            throw RuntimeException(PLUGIN_ZMQ_PREFIX + "Failed to parse long: " + e.what());
        }
        if ((maxOpenFileNum - currentOpenFileNum) < 50) {
            throw RuntimeException(PLUGIN_ZMQ_PREFIX + "Can not connect, For already open too many files!");
        }
    }

protected:
    zmq::context_t context;
    shared_ptr<zmq::socket_t> zmq_Socket_;
    string prefix_;
    string addr_;
    string type_;
};

class ZmqPushSocket : public ZmqSocket {
public:
    ZmqPushSocket(const string &type, FunctionDefSP formatter, int batchSize) : ZmqSocket(type), formatter_(formatter), batchSize_(batchSize){}

    FunctionDefSP getFormatter() {
        return formatter_;
    }

    int getBatchSize(){
        return batchSize_;
    }

    virtual ~ZmqPushSocket(){};

protected:
    FunctionDefSP formatter_;
    int batchSize_;
};

struct AsyncConfig{
    bool isAsync_ = false;
    int batchSize_ = 0;
    double throttle_ = 1.0;
    int maxQueueDepth_ = 1000000;
};

struct HandleConfig{
    FunctionDefSP parser_;
    ConstantSP handle_;
    AsyncConfig asyncConfig_;
};

class MessageWrapper {
  public:
    MessageWrapper() {}
    MessageWrapper(std::string&& data) : data_(data) {}
    MessageWrapper(const MessageWrapper &) = default;
    MessageWrapper &operator=(const MessageWrapper &) = default;

  public:
    std::string data_;
    long long reachTime{};
};


static void subJobCallBack(vector<ConstantSP> &buffer, MessageWrapper &data) {
    ((VectorSP)buffer[0])->appendString(&data.data_, 1);
}

class ZmqSubSocket : public ZmqSocket {
public:
    ZmqSubSocket(const string &addr, const string &type, FunctionDefSP parser, const string &prefix, bool isConnect) :
            ZmqSocket(type), parser_(parser) {
        if(isConnect)
            connect(addr, prefix);
        else
            bind(addr, prefix);
        if(type == "ZMQ_SUB")
            zmq_Socket_->set(zmq::sockopt::subscribe, prefix.c_str());
        zmq_Socket_->set(zmq::sockopt::rcvtimeo, 100);
    }

    FunctionDefSP getParser() {
        return parser_;
    }

    void close(){
        zmq_Socket_->close();
    }

    virtual ~ZmqSubSocket(){};

protected:
    FunctionDefSP parser_;
};


MetaTable mockMetaTable = {{"data"}, {DT_STRING}};

void subJobTransform(Heap *heap, vector<ConstantSP> &args) {
    FunctionDefSP parser = args[0];
    FunctionDefSP handler = args[1];
    vector<ConstantSP> parserArgs{args[2]};
    ConstantSP parseResult = parser->call(heap, parserArgs);
    vector<ConstantSP> handleArgs{parseResult};
    handler->call(heap, handleArgs);
}


class AppendTable : public Runnable {
public:
    AppendTable(Heap *heap, shared_ptr<ZmqSubSocket> socket, const HandleConfig& handleConfig)
            : socket_(socket), handleConfig_(handleConfig), recv(0), isStop_(false){
        session_ = heap->currentSession()->copy();
        session_->setUser(heap->currentSession()->getUser());
        session_->setOutput(new DummyOutput);
        long long currentTime = Util::getNanoEpochTime();
        localTimeGap_ = Util::toLocalNanoTimestamp(currentTime) - currentTime;
        if(handleConfig_.asyncConfig_.isAsync_){
            queue_ = new ThreadedQueue<MessageWrapper>(session_->getHeap().get(), std::llround(handleConfig.asyncConfig_.throttle_ * 1000), 
                handleConfig.asyncConfig_.maxQueueDepth_, mockMetaTable,
                                                    nullptr, 0, socket->getAddr(), PLUGIN_ZMQ_PREFIX, handleConfig.asyncConfig_.batchSize_, subJobCallBack);
            queue_->setTimeoutAsThrottle(true);
            if (handleConfig.handle_->getForm() == DF_TABLE) {
                queue_->setTransform(handleConfig.parser_);
                queue_->setTable(handleConfig.handle_);
            } else {  // could only be FunctionDef
                FunctionDefSP trans(Util::createSystemProcedure( socket->getAddr() + " subJobTransform", subJobTransform, 3, 3));
                vector<ConstantSP> args{handleConfig.parser_, handleConfig.handle_};
                FunctionDefSP partTrans = Util::createPartialFunction(trans, args);
                queue_->setTransform(partTrans);
                queue_->ignoreTableInsert();
            }
            queue_->start();
        }
    }
    
    ~AppendTable(){
        stop();
    }

    void run() override;

    shared_ptr<ZmqSubSocket> getZmqSocket() {
        return socket_;
    }

    void increaseRecv() {
        recv++;
    }

    long long getRecv() {
        return recv;
    }
    
    const MarketStatus& getStatus(){
        if(!queue_.isNull()){
            return queue_->getStatusConst();
        }
        return syncModeStatus_;
    }

    void stop(){
        LockGuard<Mutex> _(&lock_);
        if(!isStop_){
            isStop_ = true;
            if(!queue_.isNull()){
                queue_->stop();
            }
        }
    }

private:
    shared_ptr<ZmqSubSocket> socket_;
    HandleConfig handleConfig_;
    SessionSP session_;
    long long recv;
    bool isStop_;
    AsyncConfig  asyncConfig_;
    SmartPointer<ThreadedQueue<MessageWrapper>> queue_;
    Mutex lock_;
    MarketStatus syncModeStatus_;
    long long localTimeGap_;
};

class SubConnection {
private:
    long long createTime_;
    Heap *heap_;
    ThreadSP thread_;
    SessionSP session_;
    SmartPointer<AppendTable> appendTable_;
    HandleConfig handleConfig_;
    Mutex lock_;
    bool isStop_;
public:

    SubConnection(Heap *heap, shared_ptr<ZmqSubSocket> socket, const HandleConfig& handleConfig);

    long long getCreateTime() const {
        return createTime_;
    }

    SessionSP getSession() {
        return session_;
    }

    SmartPointer<AppendTable> getAppendTable() {
        return appendTable_;
    }

    ~SubConnection(){
        stop();
    }
    
    void stop(){
        LockGuard<Mutex> _(&lock_);
        if(!isStop_){
            isStop_ = true;
            appendTable_->stop();
            thread_->join();
            appendTable_->getZmqSocket()->close();
        }
    }
    
    const HandleConfig& getConfig(){
        return handleConfig_;
    }
};

class ZmqPusher : public Table{

private:
    ConstantSP zmqSocket_;
    TableSP dummyTable_;
    SessionSP session_;
    vector<string> colNames_;

public: 
    ZmqPusher(ConstantSP zmqSocket, TableSP dummyTable, Heap* heap){
        zmqSocket_ = zmqSocket;
        vector<DATA_TYPE> colTypes;
        int cols = dummyTable->columns();
        for(int i = 0; i < cols; ++i){
            colNames_.push_back(dummyTable->getColumnName(i));
            colTypes.push_back(dummyTable->getColumnType(i));
        }
        dummyTable_ = Util::createTable(colNames_, colTypes, 0, 0);
        session_ = heap->currentSession()->copy();
    }

    virtual ~ZmqPusher(){}

    virtual bool append(argsT& values, INDEX& insertedRows, string& errMsg){
        try{
            TableSP inputTable;
            if(values.size() == 1 && values[0]->isTuple()){
                argsT cols;
                for(int i = 0 ; i < values[0]->size(); i++){
                    cols.emplace_back(values[0]->get(i));
                }
                inputTable = Util::createTable(colNames_, cols);
            } else if(values.size() == 1 && values[0]->isTable()){
                inputTable = values[0];
            }
            else{
                inputTable = Util::createTable(
                    colNames_,
                    argsT(values.begin(), values.begin() + values.size()));
            }
            argsT args{zmqSocket_, inputTable};
            bool ret = zmqSend(session_->getHeap().get(), args)->getBool();
            return ret;
        }
        catch(exception &e){
            errMsg = e.what();
            return false;
        }
        insertedRows = values[0]->rows();
        return true;
    }
    virtual string getString() const {return dummyTable_->getString();};
    virtual string getString(INDEX index) const {return dummyTable_->getString(index);};
    virtual ConstantSP get(const ConstantSP& index) const {return dummyTable_->get(index);};
    virtual ConstantSP getColumn(INDEX index) const {return dummyTable_->getColumn(index);};
    virtual ConstantSP getColumn(const string& name) const {return dummyTable_->getColumn(name);};
    virtual ConstantSP getColumn(const string& qualifier, const string& name) const {return dummyTable_->getColumn(qualifier, name);};
    virtual ConstantSP getColumn(INDEX index, const ConstantSP& rowFilter) const {return dummyTable_->getColumn(index, rowFilter);};
    virtual ConstantSP getColumn(const string& name, const ConstantSP& rowFilter) const {return dummyTable_->getColumn(name, rowFilter);};
    virtual ConstantSP getColumn(const string& qualifier, const string& name, const ConstantSP& rowFilter) const {return dummyTable_->getColumn(qualifier, name, rowFilter);};
    virtual ConstantSP getWindow(INDEX colStart, int colLength, INDEX rowStart, int rowLength) const {return dummyTable_->getWindow(colStart, colLength, rowStart, rowLength);};
    virtual bool sizeable() const {return dummyTable_->sizeable();};
    virtual INDEX size() const {return dummyTable_->size();};
    virtual INDEX columns() const {return dummyTable_->columns();};
    virtual ConstantSP getMember(const ConstantSP& key) const {return dummyTable_->getMember(key);};
    virtual ConstantSP keys() const {return dummyTable_->keys();};
    virtual ConstantSP values() const {return dummyTable_->values();};
    virtual long long getAllocatedMemory() const {return dummyTable_->getAllocatedMemory();};
    virtual ConstantSP getValue() const {return dummyTable_->getValue();};
    virtual ConstantSP getValue(INDEX capacity) const {return dummyTable_->getValue(capacity);};
    virtual const string& getColumnName(int index) const {return dummyTable_->getColumnName(index);};
    virtual const string& getColumnQualifier(int index) const {return dummyTable_->getColumnQualifier(index);};
    virtual void setColumnName(int index, const string& name) {return dummyTable_->setColumnName(index, name);};
    virtual int getColumnIndex(const string& name) const {return dummyTable_->getColumnIndex(name);};
    virtual bool contain(const string& qualifier, const string& name) const {return dummyTable_->contain(qualifier, name);};
    virtual bool contain(const ColumnRef* col) const {return dummyTable_->contain(col);};
    virtual bool contain(const ColumnRefSP& col) const {return dummyTable_->contain(col);};
    virtual bool contain(const string& name) const {return dummyTable_->contain(name);};
    virtual bool containAll(const vector<ColumnRefSP>& cols) const {return dummyTable_->containAll(cols);};
    virtual void setName(const string& name) {return dummyTable_->setName(name);};
    virtual INDEX remove(const ConstantSP& indexSP, string& errMsg) {return dummyTable_->remove(indexSP, errMsg);};
    virtual DATA_TYPE getColumnType(int index) const {return dummyTable_->getColumnType(index);};
	virtual TABLE_TYPE getTableType() const {return dummyTable_->getTableType();};
    virtual ConstantSP getInstance(INDEX size) const {return dummyTable_->getInstance(size);};
    virtual ConstantSP getColumn(const string& name, const ConstantSP& rowFilter) {return dummyTable_->getColumn(name, rowFilter);};
    virtual const string& getName() const {return dummyTable_->getName();};
    virtual INDEX update(argsT& values, const ConstantSP& indexSP, vector<string>& colNames, string& errMsg) {return dummyTable_->update(values, indexSP, colNames, errMsg);};
    virtual ConstantSP getInstance() const {return ((ConstantSP)dummyTable_)->getInstance();};
    int getColumnExtraParam(int index) const override { return dummyTable_->getColumnExtraParam(index);}
};

static shared_ptr<zmq::socket_t> createZmqSocket(zmq::context_t &context, const string &socketType) {
    ZmqSocket::checkFileNum();
    if (socketType == "ZMQ_PUB") {
        return shared_ptr<zmq::socket_t>(new zmq::socket_t(context, ZMQ_PUB));
    } else if (socketType == "ZMQ_SUB") {
        return shared_ptr<zmq::socket_t>(new zmq::socket_t(context, ZMQ_SUB));
    } else if (socketType == "ZMQ_PUSH") {
        return shared_ptr<zmq::socket_t>(new zmq::socket_t(context, ZMQ_PUSH));
    } else if (socketType == "ZMQ_PULL") {
        return shared_ptr<zmq::socket_t>(new zmq::socket_t(context, ZMQ_PULL));
    } else {
        throw RuntimeException(PLUGIN_ZMQ_PREFIX+"the zmq socket type is not supported");
    }
}

} // namespace ddb
