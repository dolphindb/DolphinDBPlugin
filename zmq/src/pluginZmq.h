#ifndef PLUGINZMQ_PLUGINZMQ_H
#define PLUGINZMQ_PLUGINZMQ_H

#include "Concurrent.h"
#include "ScalarImp.h"
#include "zmq.h"
#include "zmq.hpp"
#include "ConstantMarshall.h"
#include "json.hpp"
#include "Logger.h"

using namespace std;
using json = nlohmann::json;

extern "C" ConstantSP zmqSocket(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP zmqSend(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP zmqConnect(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP zmqBind(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP zmqClose(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP zmqCreateSubJob(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP zmqCancelSubJob(Heap *heap, vector<ConstantSP> args);

extern "C" ConstantSP zmqGetSubJobStat(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP zmqCreatepusher(Heap *heap, vector<ConstantSP> &args);

static shared_ptr<zmq::socket_t> createZmqSocket(zmq::context_t &context, const string &socketType) {
    if (socketType == "ZMQ_PUB") {
        return shared_ptr<zmq::socket_t>(new zmq::socket_t(context, ZMQ_PUB));
    } else if (socketType == "ZMQ_SUB") {
        return shared_ptr<zmq::socket_t>(new zmq::socket_t(context, ZMQ_SUB));
    } else if (socketType == "ZMQ_PUSH") {
        return shared_ptr<zmq::socket_t>(new zmq::socket_t(context, ZMQ_PUSH));
    } else if (socketType == "ZMQ_PULL") {
        return shared_ptr<zmq::socket_t>(new zmq::socket_t(context, ZMQ_PULL));
    } else {
        throw RuntimeException("the zmq socket type is not supported");
    }
}

class ZmqSocket {
public:
    ZmqSocket(const string &type)
        : context(), zmq_Socket_(createZmqSocket(context, type)) {}

    void connect(const string &addr, const string &prefix) {
        zmq_Socket_->set(zmq::sockopt::tcp_keepalive, 1);
        zmq_Socket_->set(zmq::sockopt::tcp_keepalive_idle, 30);
        zmq_Socket_->set(zmq::sockopt::tcp_keepalive_cnt, 5);
        zmq_Socket_->set(zmq::sockopt::tcp_keepalive_intvl, 1);

        zmq_Socket_->connect(addr);
        addr_ = addr;
        prefix_ = prefix;
    }

    void bind(const string addr, const string &prefix) {
        addr_ = addr;
        zmq_Socket_->bind(addr);
        prefix_ = prefix;
    }

    // string str() {
    //     return addr_;
    // }

    virtual ~ZmqSocket() {
        LOG_INFO("PluginZmq: socket[" + addr_ + "] is closed. ");
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
            : socket_(socket), parser_(parser), handle_(handle), recv(0), needStop_(false), isStop_(1) {
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
            bool ret =zmqSend(session_->getHeap().get(), args)->getBool();
            if(!ret)
                return false;
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
    virtual bool contain(ColumnRef* col) const {return dummytable_->contain(col);};
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
};

#endif //PLUGINZMQ_PLUGINZMQ_H
