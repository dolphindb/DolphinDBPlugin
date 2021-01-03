//
// Created by ypfan on 2020/11/20.
//
#include "CoreConcept.h"
#include "Exceptions.h"
#include "Util.h"
#include "Types.h"
#include "Concurrent.h"
#include "ScalarImp.h"
#include "cppkafka/cppkafka.h"

#define ENDLOOP 1897

#ifndef KAFKA_CLIENT_H
#define KAFKA_CLIENT_H

using namespace cppkafka;
using namespace std;

class SubConnection;

class DummyOutput: public Output{
public:
    virtual bool timeElapsed(long long nanoSeconds){return true;}
    virtual bool write(const ConstantSP& obj){return true;}
    virtual bool message(const string& msg){return true;}
    virtual void enableIntermediateMessage(bool enabled) {}
    virtual IO_ERR done(){return OK;}
    virtual IO_ERR done(const string& errMsg){return OK;}
    virtual bool start(){return true;}
    virtual bool start(const string& message){return true;}
    virtual IO_ERR writeReady(){return OK;}
    virtual ~DummyOutput(){}
    virtual OUTPUT_TYPE getOutputType() const {return STDOUT;}
    virtual void close() {}
    virtual void setWindow(INDEX index,INDEX size){};
    virtual IO_ERR flush() {return OK;}
};

class AppendTable : public Runnable {
public:
    AppendTable(Heap *heap, const FunctionDefSP& parser, SubConnection* client, ConstantSP handle, ConstantSP consumer, int timeout)
            : client_(client), parser_(parser), handle_(handle), consumer_(consumer), heap_(heap), timeout_(timeout){
        session_ = heap->currentSession()->copy();
        session_->setUser(heap->currentSession()->getUser());
        session_->setOutput(new DummyOutput);
    }
    ~AppendTable() override = default;
    void run() override;

private:
    SessionSP session_;
    SubConnection * client_;
    FunctionDefSP parser_;
    ConstantSP handle_;
    ConstantSP consumer_;
    Heap* heap_;
    bool flag_ = true;
    int timeout_;
};

class SubConnection {
private:
    std::string description_;
    bool connected_;
    long long createTime_{};
    Heap* heap_{};
    ThreadSP thread_;
    SessionSP session_;
public:
    SubConnection();
    SubConnection(Heap *heap, std::string description, const FunctionDefSP& parser, ConstantSP handle, ConstantSP consumer, int timeout);
    ~SubConnection();
    
    string getDescription(){
        return description_;
    }
    long long getCreateTime() const{
        return createTime_;
    }
    SessionSP getSession() {
        return session_;
    }
    Heap* getHeap(){
        return heap_;
    }
    void cancelThread(){
        thread_->cancel();
    }
};


#endif //KAFKA_CLIENT_H
