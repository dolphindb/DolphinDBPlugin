#include "tcpSocket.h"
#include "ddbplugin/PluginLoggerImp.h"

namespace ddb {

struct TcpData
{
    char data_[TCP_BATCH_SIZE];
    size_t size_;
    bool isHeadData_ = false;
    long long reachTime;
};

void structReader(vector<ConstantSP> &dst, TcpData& data){
    string dataStr(data.data_, data.size_);
    static_cast<Vector*>(dst[0].get())->appendString(&dataStr, 1);
    static_cast<Vector*>(dst[1].get())->appendBool(reinterpret_cast<char*>(&data.isHeadData_), 1);
}

ConstantSP parseTcpData(Heap* heap, vector<ConstantSP>& args){
    vector<ConstantSP> newArgs;
    newArgs.insert(newArgs.begin(), args.begin() + 1, args.end());
    FunctionDefSP handler = args[0];
    handler->call(heap, newArgs);   
    TableSP table = Util::createTable({"col1"}, {DT_BOOL}, 0, 0);
    return table;
}

class TCPSubJob{
public:
    TCPSubJob(string host, int port, Heap* heap, int timeout, int batchSize, FunctionDefSP handler):
        stop_(false){
        MetaTable meta;
        meta.colNames_ = {"bytes", "isHead"};
        meta.colTypes_ = {DT_BLOB, DT_BOOL};
        queue_ = new ThreadedQueue<TcpData>(heap, timeout, 1024, meta, nullptr, 0, "tcp", "tcp", batchSize, structReader);
        FunctionDefSP newHandler(Util::createSystemFunction("tcp job handler", parseTcpData, 2, 2, false));
        handler = Util::createPartialFunction(newHandler, {handler});
        queue_->setTransform(handler);
        TableSP table = Util::createTable({"col1"}, {DT_BOOL}, 0, 0);
        queue_->setTable(table);
        queue_->start();

        std::function<void()> f = [this, host, port]() {
            while(!stop_){
                {
                    try{
                        bool isHeadData = true;
                        SocketSP socket = new Socket(host, port, true);
                        if(socket->connect() == OK){
                            TcpData data;
                            while(!stop_ && socket->read(data.data_, TCP_BATCH_SIZE, data.size_) == OK){
                                data.isHeadData_ = isHeadData;
                                queue_->push(data);
                                isHeadData = false;
                            }
                            string errMsg = PLUGIN_TCP_PREFIX + "tcp connection is disconnect. ";
                            PLUGIN_LOG_ERR(errMsg);
                            queue_->setError(errMsg);
                        }else{
                            string errMsg = PLUGIN_TCP_PREFIX + "failed to connect to " + host + ":" + std::to_string(port);
                            PLUGIN_LOG_ERR(errMsg);
                            queue_->setError(errMsg);
                        }
                    }catch(exception& e){
                        string errMsg = PLUGIN_TCP_PREFIX + "failed to receive data: " + e.what();
                        PLUGIN_LOG_ERR(errMsg);
                        queue_->setError(errMsg);
                    }
                    Util::sleep(1000);
                }
            }
        };

        SmartPointer<Executor> executor = new Executor(f);
        thread_ = new Thread(executor);
        thread_->start();
    }

    void stop(){
        LockGuard<Mutex> _(&lock_);
        if(stop_) return;
        queue_->stop();
        stop_ = true;
        thread_->join();
    }

    ~TCPSubJob(){
        stop();
    }

    MarketStatus getStatus(){
        return queue_->getStatusConst();
    }

    ThreadSP thread_;
    SmartPointer<ThreadedQueue<TcpData>> queue_;
    bool stop_;
    Mutex lock_;
    
};

static void tcpSubOnClose(Heap *heap, vector<ConstantSP> &args){
    std::ignore = heap;
    std::ignore = args;
}

BackgroundResourceMap<TCPSubJob> TCP_JOB_MAP(PLUGIN_TCP_PREFIX, "tcp job");

static int getOptionInt(const DictionarySP& options, const string& tag){
        int value = -1;
        ConstantSP data = options->getMember(tag.c_str());
        if(!data.isNull() && !data->isNull()){
            if(data->getType() != DT_INT || data->getForm() != DF_SCALAR){
                throw RuntimeException(PLUGIN_TCP_PREFIX + "the value of the dictionary option's key as the " + 
                    tag + " must be an int scalar");
            }
            value = data->getInt();
            PLUGIN_LOG_INFO(PLUGIN_TCP_PREFIX, "get options ", tag, ": ", value);
            if(value < 0){
                throw RuntimeException(PLUGIN_TCP_PREFIX + "the value of the dictionary option's key as the " + 
                    tag + " must not be less than 0");
            }
        }
        return value;
}

} // namespace ddb

using namespace ddb;

ConstantSP tcpCreateSubJob(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR)
        throw RuntimeException(PLUGIN_TCP_PREFIX + "host must be a string scalar. ");
    if(args[1]->getType() != DT_INT || args[1]->getForm() != DF_SCALAR)
        throw RuntimeException(PLUGIN_TCP_PREFIX + "port must be a int scalar. ");
    if(args[2]->getType() != DT_FUNCTIONDEF || args[2]->getForm() != DF_SCALAR)
        throw RuntimeException(PLUGIN_TCP_PREFIX + "handler must be a function. ");
    int batchSize = -1;
    if(args.size() > 3){
        if(args[3]->getForm() != DF_DICTIONARY)
            throw RuntimeException(PLUGIN_TCP_PREFIX + "config must be a dictionary. ");
        DictionarySP config = args[3];
        if(config->getKeyType() != DT_STRING || config->getType() != DT_ANY)
            throw RuntimeException(PLUGIN_TCP_PREFIX + "config must be a dictionary with STRING-ANY key-value type. ");
        batchSize = getOptionInt(config, "maxRows");
    }
    if(batchSize == -1) batchSize = 128;
    string host = args[0]->getString();
    int port = args[1]->getInt();
    FunctionDefSP handle = args[2];
    SmartPointer<TCPSubJob> ptr = new TCPSubJob(host, port, heap, 10, 10, handle);
    FunctionDefSP onClose(Util::createSystemProcedure("tcp onClose()", tcpSubOnClose, 1, 1));
    ConstantSP job = Util::createResource(reinterpret_cast<long long>(ptr.get()), "tcp job connect to " + host + ":" + std::to_string(port) + 
       + " " + Uuid(true).getString(), onClose, heap->currentSession());
    TCP_JOB_MAP.safeAdd(job, ptr, job->getString());
    return new String(job->getString());
}

ConstantSP tcpGetSubJobStat(Heap *heap, vector<ConstantSP> &args){
    std::ignore = heap;
    std::ignore = args;
    vector<string> data = TCP_JOB_MAP.getHandleNames();
    int size = data.size();
    size = std::max(size, 1);
    DdbVector<string> tag(0, size);
    DdbVector<long long> startTime(0, size);
    DdbVector<long long> endTime(0, size);
    DdbVector<long long> firstMsgTime(0, size);
    DdbVector<long long> lastMsgTime(0, size);
    DdbVector<long long> processedMsgCount(0, size);
    DdbVector<long long> failedMsgCount(0, size);
    DdbVector<string> lastErrMsg(0, size);
    DdbVector<long long> lastFailedTimestamp(0, size);
    for(string& str : data){
        SmartPointer<TCPSubJob> subJob = TCP_JOB_MAP.safeGetByName(str);
        MarketStatus status = subJob->getStatus();
        tag.add(str);
        startTime.add(status.startTime_);
        endTime.add(status.endTime_);
        firstMsgTime.add(status.firstMsgTime_);
        lastMsgTime.add(status.lastMsgTime_);
        processedMsgCount.add(status.processedMsgCount_);
        failedMsgCount.add(status.failedMsgCount_);
        lastErrMsg.add(status.lastErrMsg_);
        lastFailedTimestamp.add(status.lastFailedTimestamp_);
    }
    vector<string> colNames = {"tag", "startTime", "endTime", "firstMsgTime", "lastMsgTime", "processedMsgCount", "failedMsgCount", "lastErrMsg", "lastFailedTimestamp"};
    vector<ConstantSP> cols = {tag.createVector(DT_STRING), startTime.createVector(DT_NANOTIMESTAMP), endTime.createVector(DT_NANOTIMESTAMP), firstMsgTime.createVector(DT_NANOTIMESTAMP), 
        lastMsgTime.createVector(DT_NANOTIMESTAMP),processedMsgCount.createVector(DT_LONG), failedMsgCount.createVector(DT_LONG), lastErrMsg.createVector(DT_STRING), 
        lastFailedTimestamp.createVector(DT_NANOTIMESTAMP)};
    return Util::createTable(colNames, cols);
}

ConstantSP tcpCancelSubJob(Heap *heap, vector<ConstantSP> &args){
    std::ignore = heap;
    if(args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR)
        throw RuntimeException(PLUGIN_TCP_PREFIX + "tag must be a string scalar. ");
    string name = args[0]->getString();
    ConstantSP handle = TCP_JOB_MAP.getHandleByName(name);
    SmartPointer<TCPSubJob> subJob = TCP_JOB_MAP.safeGet(handle); 
    subJob->stop();
    return new Bool(true);
}