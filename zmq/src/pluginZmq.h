#ifndef PLUGINZMQ_PLUGINZMQ_H
#define PLUGINZMQ_PLUGINZMQ_H

#include "Concurrent.h"
#include "ScalarImp.h"
#include "zmq.h"
#include "zmq.hpp"
#include "json.hpp"
#include "Logger.h"
#include "ddbplugin/Plugin.h"

using namespace std;
using json = nlohmann::json;
static const std::string PLUGIN_ZMQ_PREFIX = "[PLUGIN::ZMQ]: ";
extern "C" ConstantSP zmqSocket(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP zmqSend(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP zmqConnect(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP zmqBind(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP zmqClose(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP zmqCreateSubJob(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP zmqCancelSubJob(Heap *heap, vector<ConstantSP> args);

extern "C" ConstantSP zmqGetSubJobStat(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP zmqCreatepusher(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP zmqSetMonitor(Heap *heap, vector<ConstantSP> &args);

static shared_ptr<zmq::socket_t> createZmqSocket(zmq::context_t &context, const string &socketType);

class ZMQMonitor : public zmq::monitor_t{
public:
    ZMQMonitor() {}
    virtual ~ZMQMonitor() {}
    virtual void on_event_connected(const zmq_event_t &event_, const char *addr_) override {
        LOG_INFO(PLUGIN_ZMQ_PREFIX + "Connected to " + string(addr_));
    }
    virtual void on_event_connect_delayed(const zmq_event_t &event_, const char *addr_) override {
        LOG_INFO(PLUGIN_ZMQ_PREFIX + "Connect delayed to " + string(addr_));
    }
    virtual void on_event_connect_retried(const zmq_event_t &event_, const char *addr_) override {
        LOG_INFO(PLUGIN_ZMQ_PREFIX + "Connect retried to " + string(addr_));
    }
    virtual void on_event_listening(const zmq_event_t &event_, const char *addr_) override {
        LOG_INFO(PLUGIN_ZMQ_PREFIX + "Listening on " + string(addr_));
    }
    virtual void on_event_bind_failed(const zmq_event_t &event_, const char *addr_) override {
        LOG_INFO(PLUGIN_ZMQ_PREFIX + "Bind failed on " + string(addr_));
    }
    virtual void on_event_accepted(const zmq_event_t &event_, const char *addr_) override {
        LOG_INFO(PLUGIN_ZMQ_PREFIX + "Accepted on " + string(addr_));
    }
    virtual void on_event_accept_failed(const zmq_event_t &event_, const char *addr_) override {
        LOG_INFO(PLUGIN_ZMQ_PREFIX + "Accept failed on " + string(addr_));
    }
    virtual void on_event_closed(const zmq_event_t &event_, const char *addr_) override {
        LOG_INFO(PLUGIN_ZMQ_PREFIX + "Closed on " + string(addr_));
    }
    virtual void on_event_close_failed(const zmq_event_t &event_, const char *addr_) override {
        LOG_INFO(PLUGIN_ZMQ_PREFIX + "Close failed on " + string(addr_));
    }
    virtual void on_event_disconnected(const zmq_event_t &event_, const char *addr_) override {
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

class DummyOutput : public Output {
public:
    virtual bool timeElapsed(long long nanoSeconds) { return true; }

    virtual bool write(const ConstantSP &obj) { return true; }

    virtual bool message(const string &msg) { return true; }

    virtual void enableIntermediateMessage(bool enabled) {}

    virtual IO_ERR done() { return OK; }

    virtual IO_ERR done(const string &errMsg) { return OK; }

    virtual bool start() { return true; }

    virtual bool start(const string &message) { return true; }

    virtual IO_ERR writeReady() { return OK; }

    virtual ~DummyOutput() {}

    virtual OUTPUT_TYPE getOutputType() const { return STDOUT; }

    virtual void close() {}

    virtual void setWindow(INDEX index, INDEX size) {};

    virtual IO_ERR flush() { return OK; }
};



class AppendTable : public Runnable {
public:
    AppendTable(Heap *heap, shared_ptr<ZmqSubSocket> socket, const FunctionDefSP &parser, ConstantSP handle)
            : socket_(socket), parser_(parser), handle_(handle), recv(0), needStop_(false), isStop_(1){
        session_ = heap->currentSession()->copy();
        session_->setUser(heap->currentSession()->getUser());
        session_->setOutput(new DummyOutput);
        isStop_.acquire();
    }

    void run() override;

    shared_ptr<ZmqSubSocket> getZmqSocket() {
        return socket_;
    }

    void increaseRecv() {
        recv.fetch_add(1);
    }

    long long getRecv() {
        return recv;
    }

    void setNeedStop(){
        needStop_.store(true);
    }

    void cancelThread(){
        needStop_.store(true);
        isStop_.acquire();
        socket_->close();
    }

private:
    shared_ptr<ZmqSubSocket> socket_;
    FunctionDefSP parser_;
    ConstantSP handle_;
    SessionSP session_;
    atomic<long long> recv;
    atomic<bool> needStop_;
    Semaphore isStop_;
};

class SubConnection {
private:
    long long createTime_;
    Heap *heap_;
    ThreadSP thread_;
    SessionSP session_;
    SmartPointer<AppendTable> appendTable_;
public:

    SubConnection(Heap *heap, shared_ptr<ZmqSubSocket> socket, const FunctionDefSP &parser, ConstantSP handle);

    long long getCreateTime() const {
        return createTime_;
    }

    SessionSP getSession() {
        return session_;
    }

    SmartPointer<AppendTable> getAppendTable() {
        return appendTable_;
    }

    Heap *getHeap() {
        return heap_;
    }

    void cancelThread() {
        appendTable_->cancelThread();
    }

    ~SubConnection(){
        appendTable_->setNeedStop();
        thread_->join();
    }
};

class ZmqPusher : public Table{

private:
    ConstantSP zmqSocket_;
    TableSP dummytable_;
    SessionSP session_;
    vector<string> colNames_;

public: 
    ZmqPusher(ConstantSP zmqSocket, TableSP dummytable, Heap* heap){
        zmqSocket_ = zmqSocket;
        vector<DATA_TYPE> colTypes;
        int cols = dummytable->columns();
        for(int i = 0; i < cols; ++i){
            colNames_.push_back(dummytable->getColumnName(i));
            colTypes.push_back(dummytable->getColumnType(i));
        }
        dummytable_ = Util::createTable(colNames_, colTypes, 0, 0);
        session_ = heap->currentSession()->copy();
    }

    virtual ~ZmqPusher(){}

    virtual bool append(vector<ConstantSP>& values, INDEX& insertedRows, string& errMsg){
        if(values.size() < 0){
            errMsg = "The size of the values is 0";
            return false;
        }
        try{
            TableSP inputTable;
            if(values.size() == 1 && values[0]->isTuple()){
                vector<ConstantSP> cols;
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
                    vector<ConstantSP>(values.begin(), values.begin() + values.size()));
            }
            vector<ConstantSP> args{zmqSocket_, inputTable};
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
    virtual string getString() const {return dummytable_->getString();};
    virtual string getString(INDEX index) const {return dummytable_->getString(index);};
    virtual ConstantSP get(const ConstantSP& index) const {return dummytable_->get(index);};
    virtual ConstantSP getColumn(INDEX index) const {return dummytable_->getColumn(index);};
    virtual ConstantSP getColumn(const string& name) const {return dummytable_->getColumn(name);};
    virtual ConstantSP getColumn(const string& qualifier, const string& name) const {return dummytable_->getColumn(qualifier, name);};
    virtual ConstantSP getColumn(INDEX index, const ConstantSP& rowFilter) const {return dummytable_->getColumn(index, rowFilter);};
    virtual ConstantSP getColumn(const string& name, const ConstantSP& rowFilter) const {return dummytable_->getColumn(name, rowFilter);};
    virtual ConstantSP getColumn(const string& qualifier, const string& name, const ConstantSP& rowFilter) const {return dummytable_->getColumn(qualifier, name, rowFilter);};
    virtual ConstantSP getWindow(INDEX colStart, int colLength, INDEX rowStart, int rowLength) const {return dummytable_->getWindow(colStart, colLength, rowStart, rowLength);};
    virtual bool sizeable() const {return dummytable_->sizeable();};
    virtual INDEX size() const {return dummytable_->size();};
    virtual INDEX columns() const {return dummytable_->columns();};
    virtual ConstantSP getMember(const ConstantSP& key) const {return dummytable_->getMember(key);};
    virtual ConstantSP keys() const {return dummytable_->keys();};
    virtual ConstantSP values() const {return dummytable_->values();};
    virtual long long getAllocatedMemory() const {return dummytable_->getAllocatedMemory();};
    virtual ConstantSP getValue() const {return dummytable_->getValue();};
    virtual ConstantSP getValue(INDEX capacity) const {return dummytable_->getValue(capacity);};
    virtual const string& getColumnName(int index) const {return dummytable_->getColumnName(index);};
    virtual const string& getColumnQualifier(int index) const {return dummytable_->getColumnQualifier(index);};
    virtual void setColumnName(int index, const string& name) {return dummytable_->setColumnName(index, name);};
    virtual int getColumnIndex(const string& name) const {return dummytable_->getColumnIndex(name);};
    virtual bool contain(const string& qualifier, const string& name) const {return dummytable_->contain(qualifier, name);};
    virtual bool contain(const ColumnRef* col) const {return dummytable_->contain(col);};
    virtual bool contain(const ColumnRefSP& col) const {return dummytable_->contain(col);};
    virtual bool contain(const string& name) const {return dummytable_->contain(name);};
    virtual bool containAll(const vector<ColumnRefSP>& cols) const {return dummytable_->containAll(cols);};
    virtual void setName(const string& name) {return dummytable_->setName(name);};
    virtual bool remove(const ConstantSP& indexSP, string& errMsg) {return dummytable_->remove(indexSP, errMsg);};
    virtual DATA_TYPE getColumnType(int index) const {return dummytable_->getColumnType(index);};
	virtual TABLE_TYPE getTableType() const {return dummytable_->getTableType();};
    virtual ConstantSP getInstance(INDEX size) const {return dummytable_->getInstance(size);};
    virtual ConstantSP getColumn(const string& name, const ConstantSP& rowFilter) {return dummytable_->getColumn(name, rowFilter);};
    virtual const string& getName() const {return dummytable_->getName();};
    virtual bool update(vector<ConstantSP>& values, const ConstantSP& indexSP, vector<string>& colNames, string& errMsg) {return dummytable_->update(values, indexSP, colNames, errMsg);};
    virtual ConstantSP getInstance() const {return ((ConstantSP)dummytable_)->getInstance();};
    int getColumnExtraParam(int index) const override { return dummytable_->getColumnExtraParam(index);}
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

#endif //PLUGINZMQ_PLUGINZMQ_H
