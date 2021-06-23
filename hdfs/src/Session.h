//
// Created by ypfan on 2020/12/9.
//

#ifndef HDFS_SESSION_H
#define HDFS_SESSION_H
#include <iostream>
#include <utility>
#include <queue>
#include "hdfs.h"
#include "CoreConcept.h"
#include "Exceptions.h"
#include "Util.h"
#include "Types.h"
#include "Concurrent.h"
#include "ScalarImp.h"
#include "Logger.h"

#ifndef MESSAGE_BODY
#define MESSAGE_BODY 'q'
#endif
#ifndef MESSAGE_END
#define MESSAGE_END 'e'
#endif

typedef SmartPointer<std::string> MyBuffer;

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

class GetData : public Runnable {
public:
    GetData(Heap *heap, hdfsFS file, std::queue<MyBuffer>* queue, SubConnection* connection);
    ~GetData() override = default;
    void run() override;

private:
    SessionSP session_;
    Heap* heap_;
    hdfsFS fs_{};
    std::queue<MyBuffer>* queue_{};
    SubConnection* connection_{};
};

class GetDataLine : public Runnable {
public:
    GetDataLine(Heap *heap, hdfsFS file, std::queue<MyBuffer>* queue, SubConnection* connection, int length);
    ~GetDataLine() override = default;
    void run() override;

private:
    SessionSP session_;
    Heap* heap_;
    hdfsFS fs_{};
    std::queue<MyBuffer>* queue_{};
    SubConnection* connection_{};
    int length_{};
};

class DealData : public Runnable {
public:
    DealData(Heap *heap, FunctionDefSP handle, std::queue<MyBuffer>* queue);
    ~DealData() override = default;
    void run() override;

private:
    SessionSP session_;
    Heap* heap_;
    FunctionDefSP handle_;
    std::queue<MyBuffer>* queue_{};
};

class SubConnection{
private:
    hdfsFS file_{};
    FunctionDefSP handler_;
    ConstantSP parser_;
    Heap* heap_{};
    SessionSP session_;
    long long createTime_{};
    ThreadSP getThread_;
    ThreadSP dealThread_;
    string path_;
    int bufferSize_{};
    int replication_{};
    long blockSize_{};
    std::queue<MyBuffer>* queue_{};
public:
    SubConnection() = default;
    SubConnection(Heap* heap, hdfsFS file, FunctionDefSP handler, string  path, int bufferSize, int replication, long blockSize, int length);
    ~SubConnection() = default;

    long long getCreateTime() const{return createTime_;}
    SessionSP getSession() const{return session_;}
    string getPath() const{return path_;}
    int getBufferSize() const{return bufferSize_;}
    int getReplication() const{return replication_;}
    long getBlockSize() const{return blockSize_;}
    void setBufferSize(int size);

    void cancelThread(){
        getThread_->cancel();
        dealThread_->cancel();
    }
};

#endif //HDFS_SESSION_H
