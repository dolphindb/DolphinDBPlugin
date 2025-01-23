#include "opc_main.h"
#include "ScalarImp.h"
#include <Windows.h>
#include <random>
#include <utility>
#include "opcimp.h"
#include "ddbplugin/Plugin.h"

typedef SmartPointer<OPCClient> OPCClientSP;
static void opcConnectionOnClose(Heap *heap, vector<ConstantSP> &args) {}

namespace OPCPluginImp{
    static unordered_map<long long, OPCClientSP> SUB_MAP;
    static Mutex LOCK;
}
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

class OpcVal : public ValWithType {
public:
    OpcVal(OPCItemData &d) : ValWithType(d) {
    }

    ConstantSP toConstant() {
        switch (vt) {
            case OPC_DOUBLE:
                return new Double(data_double);
            case OPC_STRING:
                return new String(data_string);
            case OPC_TIME:
                return new DateTime(data_time);
            case OPC_LONG:
                return new Long(data_long_long);
            case OPC_ARRAY_STRING: {
                ConstantSP ret = Util::createConstant(DT_STRING);
                string s = "";
                for (int i = 0, size = data_array_string.size(); i < size; i++) {
                    s += data_array_string[i];
                    s += ",";
                }
                ret->setString(s);
                return ret;
            }
            case OPC_ARRAY_DOUBLE: {
                ConstantSP ret = Util::createConstant(DT_STRING);
                string s = "";
                for (int i = 0, size = data_array_double.size(); i < size; i++) {
                    s += to_string(data_array_double[i]);
                    s += ",";
                }
                ret->setString(s);
                return ret;
            }
            default:
                throw OPCException("type error");
        }
    }
};

ConstantSP toConstant(OPCItemData& data, string itemName) {
    unsigned long long ms;
    vector<ConstantSP> cols;
    ConstantSP name = Util::createConstant(DT_STRING);
    name->setString(itemName);
    cols.push_back(name);

    FileTimeToMs(data.ftTimeStamp, ms);
    ConstantSP timestamp = Util::createConstant(DT_TIMESTAMP);
    timestamp->setLong(ms);
    cols.push_back(timestamp);

    OpcVal val(data);
    ConstantSP value = val.toConstant();
    // TODO: add more type supporting accounding to data.vDataValue.vt;
    // cout<<value->getDouble()<<"  int " <<value->getInt()<<endl;
    // value->setDouble(data.vDataValue.intVal);
    cols.push_back(value);

    ConstantSP quality = Util::createConstant(DT_INT);
    quality->setInt(data.wQuality);
    cols.push_back(quality);

    vector<string> colNames = {"tag", "time", "value", "quality"};
    TableSP table = Util::createTable(colNames, cols);
    return table;
}

struct Sizer {
    int operator()(const ConstantSP &obj) { return 1; }
};
struct Mandatory {
    bool operator()(const ConstantSP &obj) { return false; }
};

class CMyCallback : public IAsynchDataCallback {
private:
    SessionSP _session;
    ConstantSP _handler;
    OPCClient* _conn;
    ThreadSP _thread;
    RunnableSP _runnable;

public:
    CMyCallback(SessionSP session, OPCClient* conn, ConstantSP callback) : _session(session), _handler(callback), _conn(conn), _queue(1000, Sizer(), Mandatory()), _stopFlag(false) {

        _runnable = new dolphindb::Executor([this](){
            while (!this->_stopFlag) {
                ConstantSP v;
                if (this->_queue.blockingPop(v, 1000)) {
                    vector<ConstantSP> args = {v};
                    try{
                        ((FunctionDefSP)_handler)->call(_session->getHeap().get(), args);
                    } catch (exception &e) {
                        LOG_ERR("Plugin OPC: Failed to append data: " + string(e.what()));
                        LockGuard<Mutex> guard(_conn->getLock());
                        this->_conn->setErrorMsg(e.what());
                    }
                }
            }
        }
        );
        _thread = new Thread(_runnable);
        _thread->start();
    }
    GenericBoundedQueue<ConstantSP, Sizer, Mandatory> _queue;
    bool _stopFlag;

    void OnDataChange(COPCGroup &group, map<ItemID, OPCItemData *> &changes) {
        // printf("Group %s, item changes\n", group.getName().c_str());
        LockGuard<Mutex> guard(_conn->getLock());
        _conn->incRecv();
        for (auto id : changes) {
            OPCItemData *itemData = id.second;

            map<int, string>::iterator l_it;
            l_it = group.mapTag.find(id.first);
            if (l_it == group.mapTag.end()) {
                _conn->setErrorMsg("tag is not found!");
                continue;
            }
            string name = group.mapTag.find(id.first)->second;
            try {
                // log_item(*itemData);
                ConstantSP v = toConstant(*itemData, name);
                _queue.push(v);
            } catch (exception &e) {
                LOG_ERR("Plugin OPC: Failed to convert data: " + string(e.what()));
                _conn->setErrorMsg(e.what());
            }
        }
    }
    virtual ~CMyCallback() {
        _stopFlag = true;
        _thread->join();
    }
};

// todo : use forward??
typedef std::function<TableSP (Heap *, vector<ConstantSP> &)> FUNC;
struct Task {    // connect or sub
    enum ACTION { CONN, SUB, UNSUB, CALL };
    ACTION act;
    Heap *heap;
    OPCClientSP conn;
    // string tagName;
    ConstantSP tagName;
    string host;
    string serverName;
    ConstantSP callback;
    unsigned long reqRt;
    CountDownLatchSP latch;
    FUNC func;
    vector<ConstantSP> args;
    ConstantSP ret;    // connect result, pass from worker thread out main thread
    TableSP retTable;
    std::string errMsg;
    Task() : Task(nullptr, nullptr, nullptr, nullptr, nullptr) {
    }
    explicit Task(Heap *heap, OPCClientSP conn, const ConstantSP &tagName, const ConstantSP &callback,
                  CountDownLatchSP latch)
        : act(SUB),
          heap(heap),
          conn(conn),
          tagName(tagName),
          serverName(""),
          callback(callback),
          reqRt(0),
          latch(latch) {
    }
    explicit Task(Heap *heap, string host, string serverName, unsigned long reqRt, CountDownLatchSP latch)
        : act(CONN),
          heap(heap),
          conn(0),
          tagName(0),
          host(move(host)),
          serverName(move(serverName)),
          callback(0),
          reqRt(reqRt),
          latch(latch) {
    }
    explicit Task(Heap *heap, FUNC &&func, vector<ConstantSP> &args, CountDownLatchSP latch, OPCClientSP connParam)
        : act(CALL), heap(heap), conn(connParam), latch(latch), func(func), args(args) {
    }
    explicit Task(Heap *heap, OPCClientSP conn, CountDownLatchSP latch)
        : act(UNSUB), conn(conn), latch(latch){
    }
};

class OPCWorker : public Runnable {
public:
    explicit OPCWorker(int id) : id_(id) {
    }
    void run() override {
        while (true) {
            try{
                SmartPointer<Task> t;
                if (queue_.pop(t)) {
                    try {
                        LockGuard<Mutex> guard(t->act != Task::CONN ? t->conn->getLock() : nullptr);
                        switch (t->act) {
                            case Task::SUB:
                                subInternal(t);
                                break;
                            case Task::CONN:
                                connectInternal(t);
                                break;
                            case Task::UNSUB:
                                unsubInternal(t);
                                break;
                            case Task::CALL:
                                t->retTable = t->func(t->heap, t->args);
                                t->latch->countDown();
                                break;
                            default:
                                break;
                        }
                    } catch (exception &e) {
                        // cout <<"Failed to work:"<< e.what() << endl;
                        LOG_ERR("Plugin OPC: " + string(e.what()));
                        t->errMsg = e.what();
                        //*t.retTable = 0;
                        t->latch->countDown();
                    }
                }

                MSG msg;
                for (int i = 0; i < 100; ++i) {
                    if (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
                        TranslateMessage(&msg);
                        DispatchMessageA(&msg);
                    }
                }
                Sleep(1);    // todo: is this needed?
            }
            catch(bad_alloc &e){
                LOG_ERR("Plugin OPC: Out of Memory. ");
            }
            catch (exception &e){
                LOG_ERR("Plugin OPC: Fail to work: ", e.what());
            }
            catch(...){
                LOG_ERR("Plugin OPC: Fail to work. ");
            }
        }
    }
    void sub(Heap *heap, OPCClientSP conn, ConstantSP tagName, ConstantSP callback) {
        CountDownLatchSP latch = new CountDownLatch(1);
        SmartPointer<Task> t = new Task(heap, conn, tagName, callback, latch);
        queue_.push(t);
        latch->wait();
        if (!t->errMsg.empty()) {
            throw RuntimeException(t->errMsg);
        }

    }
    void unsub(OPCClientSP conn) {
        CountDownLatchSP latch = new CountDownLatch(1);
        SmartPointer<Task> t = new Task(0, conn, latch);
        queue_.push(t);
        latch->wait();
        if (!t->errMsg.empty()) {
            throw RuntimeException(t->errMsg);
        }

    }
    OPCClientSP connect(Heap *heap, string host, string serverName, unsigned long reqUpdateRate_ms) {
        CountDownLatchSP latch = new CountDownLatch(1);
        SmartPointer<Task> t = new Task(heap, host, serverName, reqUpdateRate_ms, latch);
        queue_.push(t);
        latch->wait();

        if (!t->errMsg.empty()) {
            throw RuntimeException(t->errMsg);
        }
        if (t->ret.isNull())
            throw RuntimeException("Failed to connect the server");
        return t->ret;
    }
    TableSP call(Heap *heap, FUNC &&func, vector<ConstantSP> &args, OPCClientSP conn) {
        CountDownLatchSP latch = new CountDownLatch(1);
        SmartPointer<Task> t = new Task(heap, move(func), args, latch, conn);
        queue_.push(t);
        latch->wait();

        if (!t->errMsg.empty()) {
            throw RuntimeException(t->errMsg);
        }
        return t->retTable;
    }

private:
    static void subInternal(SmartPointer<Task> &task) {
        auto &conn = task->conn;
        if(!conn->getSubFlag())
            throw OPCException("client is subscribed");
        if(!conn->getConnected())
            throw OPCException("client is not connected");
        auto &heap = task->heap;
        auto &callback = task->callback;
        auto &tagName = task->tagName;

        auto group = conn->group;
        // group->addItem(tagName->getString(), true);
        std::vector<COPCItem *> itemsCreated;
        std::vector<HRESULT> errors;
        vector<string> items;
        if (tagName->isScalar()) {
            auto tag = tagName->getString();
            items.push_back(tag);
        } else {    // if (tagName->isArray()) {
            for (int i = 0; i < tagName->size(); i++) {
                auto tag = tagName->getString(i);
                items.push_back(tag);
            }
        }
        if (group->addItems(items, itemsCreated, errors, true) != 0) {
            for (size_t i = 0; i < errors.size(); i++) {
                if (FAILED(errors[i])) {
                    throw OPCException("Failed to add item: " + std::to_string((unsigned long long)errors[i]));
                }
            }
        }

        for (unsigned i = 0; i < items.size(); i++) {
            group->mapTag.insert(pair<int, string>(itemsCreated[i]->getID(), items[i]));
        }


        SessionSP session = heap->currentSession()->copy();
        session->setUser(heap->currentSession()->getUser());
        session->setOutput(new DummyOutput);

        unique_ptr<IAsynchDataCallback> ptr(new CMyCallback(session, conn.get(),callback));

        group->enableAsynch(move(ptr));

        conn->setSubFlag(false);

        task->latch->countDown();
    }
    static void unsubInternal(const SmartPointer<Task>& task) {
        auto &conn = task->conn;
        if(conn->getSubFlag())
            throw OPCException("client is not subscribed");
        if(!conn->getConnected())
            throw OPCException("client is not connected");
        auto group = conn->group;
        conn->setSubFlag(true);
        // group->removeItems();
        group->mapTag.clear();
        group->disableAsynch();
        task->latch->countDown();
    }
    void connectInternal(const SmartPointer<Task>& task) {
        auto &host = task->host;
        auto &serverName = task->serverName;
        auto &reqUpdateRate_ms = task->reqRt;
        FunctionDefSP onClose(Util::createSystemProcedure("opc connection onClose()", opcConnectionOnClose, 1, 1));
        OPCClientSP cup = new OPCClient(host, id_,serverName, onClose, task->heap->currentSession());

        cup->connectToOPCServer(serverName);

        unsigned long refresh_rate = 0;
        cup->makeGroup("Group", true, reqUpdateRate_ms, refresh_rate, 0.0);
        task->ret = cup;
        task->latch->countDown();
    }
    SynchronizedQueue<SmartPointer<Task>> queue_;
    int id_;
};

class OPCWorkerPool {
public:
    OPCWorkerPool() {
        for (int i = 0; i < poolSize_; ++i) {
            worker_.push_back(new OPCWorker(i));
            pool_.emplace_back(new Thread(worker_.back()));
            pool_.back()->start();
        }
    }
    // fixme: heap may be invalid
    void sub(Heap *heap, OPCClientSP conn, ConstantSP tagName, ConstantSP callback) {
        int id = conn->id();

        cout << "threadID: " << id << endl;
        worker_[id]->sub(heap, conn, tagName, callback);
    }
    void unsub(OPCClientSP conn) {
        int id = conn->id();
        cout << "unsub from id" << id << endl;
        worker_[id]->unsub(conn);
    }
    OPCClientSP connect(Heap *heap, string host, string serverName, unsigned long reqUpdateRate_ms) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<> dis(0, poolSize_);
        int id = dis(gen);
        cout << "threadID: " << id << endl;
        return worker_[id]->connect(heap, host, serverName, reqUpdateRate_ms);
    }
    ConstantSP call(Heap *heap, FUNC &&func, vector<ConstantSP> &args, int id, OPCClientSP conn) {
        return worker_[id]->call(heap, move(func), args, conn);
    }

private:
    const int poolSize_ = 10;
    vector<ThreadSP> pool_;
    vector<OPCWorker *> worker_;
};

static OPCWorkerPool pool;


ConstantSP createSVCFromVector(vector<string> v) {
    int totallySize = v.size();
    auto ret = Util::createVector(DT_STRING, totallySize);
    for (int i = 0; i < totallySize; i++) {
        ret->setString(i, v[i]);
    }
    return ret;
}

vector<string> CLSIDToStringVector(vector<CLSID> v) {
    int totallySize = v.size();
    auto ret = vector<string>(totallySize);
    char buffer[40] = {0};
    for (int i = 0; i < totallySize; i++) {
        auto id = v[i];
        sprintf(buffer, "{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}", id.Data1, id.Data2,
                id.Data3, id.Data4[0], id.Data4[1], id.Data4[2], id.Data4[3], id.Data4[4], id.Data4[5], id.Data4[6],
                id.Data4[7]);
        ret[i] = std::string(buffer);    // copy
    }
    return ret;
}

ConstantSP getOpcServerList(Heap *heap, vector<ConstantSP> &arguments) {
    if ((arguments[0]->getType() != DT_STRING) || !arguments[0]->isScalar()) {
        throw IllegalArgumentException(__FUNCTION__, "the host must be string scalar");
    }
    // create server
    auto host = arguments[0]->getString();
    FunctionDefSP onClose(Util::createSystemProcedure("opc connection onClose()", opcConnectionOnClose, 1, 1));
    auto client = new OPCClient(host, 0,"", onClose, heap->currentSession());    // fixme: this function may not work any more
    vector<string> progIds;
    vector<CLSID> ids;
    client->getServerList(progIds, ids);

    vector<string> colNames = {"progID", "CLSID"};
    ConstantSP cIds = createSVCFromVector(CLSIDToStringVector(ids));
    ConstantSP cProgIds = createSVCFromVector(progIds);
    vector<ConstantSP> cols = {cProgIds, cIds};
    TableSP retTable = Util::createTable(colNames, cols);
    return retTable;
}

ConstantSP connectOpcServer(Heap *heap, vector<ConstantSP> &arguments) {
    std::string usage = "Usage: connect(host,server,[reqUpdateRate_ms=100]). ";

    if ((arguments[0]->getType() != DT_STRING) || !arguments[0]->isScalar()) {
        throw IllegalArgumentException(__FUNCTION__, "the hostname must be string scalar");
    }
    if ((arguments[0]->getType() != DT_STRING) || !arguments[0]->isScalar()) {
        throw IllegalArgumentException(__FUNCTION__, "the server name must be string scalar");
    }
    // create server
    auto host = arguments[0]->getString();
    auto serverName = arguments[1]->getString();
    unsigned long reqUpdateRate_ms = 100;

    if (arguments.size() > 2) {
        if (arguments[2]->getType() != DT_INT || arguments[2]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, usage + "reqUpdateRate_ms must be a integer");
        }
        if (arguments[2]->getInt() < 0 || arguments[2]->getInt() > 65536)
            throw IllegalArgumentException(__FUNCTION__, usage + "reqUpdateRate_ms must be a integer(0-65536)");
        reqUpdateRate_ms = arguments[2]->getInt();
        cout << "reqUpdateRate_ms is " << reqUpdateRate_ms << endl;
    }

    OPCClientSP cup(pool.connect(heap, host, serverName, reqUpdateRate_ms));

    return cup;
}

ConstantSP disconnect(const ConstantSP &handle, const ConstantSP &b) {
    std::string usage = "Usage: close(conn). ";
    OPCClientSP conn;
    if (handle->getType() == DT_RESOURCE && handle->getString() == "opc connection") {
        conn = handle;

    } else {
        throw IllegalArgumentException(__FUNCTION__, usage + "Invalid connection object.");
    }

    LockGuard<Mutex> lk(conn->getLock());
    if (!conn->getSubFlag())
        return new Bool(false);  // throw IllegalArgumentException(__FUNCTION__, "There is a subscriber yet.Please
                                 // unsubscriber firstly.");//pool.unsub(conn);
    if(!conn->getConnected()){
        throw IllegalArgumentException(__FUNCTION__, "The connection is not connected.");
    }
    conn->disconnect();
    return new Bool(true);
}

ConstantSP endSub(const ConstantSP &handle, const ConstantSP &b) {
    std::string usage = "Usage: unsubscribe(subscription). ";
    OPCClientSP conn;
    if (handle->getType() == DT_RESOURCE && handle->getString() == "opc connection") {
        conn = handle;

    } else if (handle->getType() == DT_STRING && handle->getForm() == DF_SCALAR) {
        std::string stringData = handle->getString();
        long long ptr;
        try{
            ptr = std::stoll(stringData);
        }catch(exception& e){
            throw RuntimeException("Invalid connection string.");
        }
        LockGuard<Mutex> lk(&OPCPluginImp::LOCK);
        if(OPCPluginImp::SUB_MAP.find(ptr) == OPCPluginImp::SUB_MAP.end()){
            throw IllegalArgumentException(__FUNCTION__, "Invalid connection string.");
        }
        conn = OPCPluginImp::SUB_MAP[ptr];
    } else if (handle->getType() == DT_LONG && handle->getForm() == DF_SCALAR) {
        LockGuard<Mutex> lk(&OPCPluginImp::LOCK);
        long long ptr = handle->getLong();
        if(OPCPluginImp::SUB_MAP.find(ptr) == OPCPluginImp::SUB_MAP.end()){
            throw IllegalArgumentException(__FUNCTION__, "Invalid connection id.");
        }
        conn = OPCPluginImp::SUB_MAP[ptr];
    } else {
        throw IllegalArgumentException(__FUNCTION__, "subscription must be an opc connection or a connection id.");
    }
    pool.unsub(conn);
    LockGuard<Mutex> lk(&OPCPluginImp::LOCK);
    OPCPluginImp::SUB_MAP.erase(reinterpret_cast<long long>(conn.get()));
    return new Void();
}
ConstantSP getSubscriberStat(const ConstantSP &handle, const ConstantSP &b) {
    LockGuard<Mutex> lk(&OPCPluginImp::LOCK);
    int size = OPCPluginImp::SUB_MAP.size();
    ConstantSP connectionIdVec = Util::createVector(DT_STRING, size);
    ConstantSP userVec = Util::createVector(DT_STRING, size);
    ConstantSP hostVec = Util::createVector(DT_STRING, size);
    ConstantSP serverNameVec = Util::createVector(DT_STRING, size);
    ConstantSP tagVec = Util::createVector(DT_STRING, size);
    ConstantSP timestampVec = Util::createVector(DT_TIMESTAMP, size);
    ConstantSP recvVec = Util::createVector(DT_LONG, size);
    ConstantSP errorMsgVec = Util::createVector(DT_STRING, size);

    int index = 0;
    for(auto it = OPCPluginImp::SUB_MAP.begin(); it != OPCPluginImp::SUB_MAP.end(); ++it){
        OPCClientSP conn = it->second;
        LockGuard<Mutex> guard(conn->getLock());
        connectionIdVec->setString(index, to_string(reinterpret_cast<long long>(conn.get())));
        hostVec->setString(index,conn->getHost());
        serverNameVec->setString(index,conn->getServerName());
        tagVec->setString(index,conn->getTagName());
        recvVec->setLong(index,conn->getRecv());
        timestampVec->setLong(index,conn->getCreateTime());
        userVec->setString(index,conn->session->getUser()->getUserId());
        errorMsgVec->setString(index,conn->getErrorMsg());
        ++index;
    }

    vector<string> colNames = {"subscriptionId", "user", "host", "serverName", "tag", "createTimestamp", "receivedPackets","errorMsg"};
    vector<ConstantSP> cols = {connectionIdVec, userVec, hostVec, serverNameVec, tagVec, timestampVec, recvVec,errorMsgVec};
    return Util::createTable(colNames, cols);
}
TableSP readTagInternal(Heap *heap, vector<ConstantSP> &arguments) {
    OPCClientSP conn = arguments[0];
    std::vector<COPCItem *> itemsCreated;
    std::vector<HRESULT> errors;
    vector<string> items;
    if (arguments[1]->isScalar()) {
        auto tagName = arguments[1]->getString();
        items.push_back(tagName);
    }
    if (arguments[1]->isArray()) {
        for (int i = 0; i < arguments[1]->size(); i++) {
            auto tagName = arguments[1]->getString(i);
            items.push_back(tagName);
        }
    }
    if (conn->group->addItems(items, itemsCreated, errors, 0) != 0) {
		for(size_t i = 0; i < errors.size(); i++){
			if(FAILED(errors[i])){
				throw OPCException("Failed to add item: " + std::to_string((unsigned long long)errors[i]));
			}
		}
    }

    int toltallySize = items.size();
    if (toltallySize == 0) {
        throw OPCException("item is empty");
    }
    if (arguments.size() == 2) {
        ConstantSP timestampVec = Util::createVector(DT_TIMESTAMP, items.size());
        ConstantSP valueVec;
        // read one
        auto item_first = itemsCreated[0];
        OPCItemData data_first;
        item_first->readSync(data_first, OPC_DS_DEVICE);

        OpcVal val_first(data_first);
        //cout << val_first->vt << endl;

        switch (val_first.vt) {
            case OPC_LONG:
                valueVec = Util::createVector(DT_LONG, items.size());
                break;
            case OPC_TIME:
                valueVec = Util::createVector(DT_DATETIME, items.size());
                break;
            case OPC_STRING:
                valueVec = Util::createVector(DT_STRING, items.size());
                break;
            case OPC_DOUBLE:
                valueVec = Util::createVector(DT_DOUBLE, items.size());
                break;
            case OPC_ARRAY_DOUBLE:
                valueVec = Util::createVector(DT_STRING, items.size());
                break;
            case OPC_ARRAY_STRING:
                valueVec = Util::createVector(DT_STRING, items.size());
                break;
            default:
                throw OPCException("array unsupported now");
        }
        ConstantSP qualityVec = Util::createVector(DT_INT, items.size());

        ConstantSP nameVec = Util::createVector(DT_STRING, items.size());

        for (int i = 0; i < toltallySize; i++) {
            auto item = itemsCreated[i];
            OPCItemData data;
            item->readSync(data, OPC_DS_DEVICE);
            unsigned long long ms;
            FileTimeToMs(data.ftTimeStamp, ms);
            ConstantSP timestamp = Util::createConstant(DT_TIMESTAMP);
            timestamp->setLong(ms);
            timestampVec->set(i, timestamp);
            OpcVal val = OpcVal(data);

            valueVec->set(i, val.toConstant());
            ConstantSP quality = Util::createConstant(DT_INT);
            quality->setInt(data.wQuality);
            qualityVec->set(i, quality);
            ConstantSP name = Util::createConstant(DT_STRING);
            name->setString(item->getName());
            nameVec->set(i, name);
        }
        vector<string> colNames = {"name", "timestamp", "value", "quality"};
        vector<ConstantSP> cols = {nameVec, timestampVec, valueVec, qualityVec};
        TableSP retTable = Util::createTable(colNames, cols);
        return retTable;
    } else {
        if (arguments[2]->isTable()) {
            TableSP tp = arguments[2];
            cout << "columns" << tp->columns() << endl;

            if (tp->columns() > 4) {    // multiple value mode
                vector<string> colNames;
                vector<ConstantSP> cols;

                for (int i = 0; i < toltallySize; i++) {
                    auto item = itemsCreated[i];
                    OPCItemData data;
                    item->readSync(data, OPC_DS_DEVICE);
                    if (i == 0) {
                        unsigned long long ms;
                        FileTimeToMs(data.ftTimeStamp, ms);
                        ConstantSP timestamp = Util::createConstant(DT_TIMESTAMP);
                        timestamp->setLong(ms);
                        colNames.push_back("timestamp");
                        cols.push_back(timestamp);
                    }

                    OpcVal val(data);
                    colNames.push_back(item->getName());
                    cols.push_back(val.toConstant());

                    ConstantSP quality = Util::createConstant(DT_INT);
                    quality->setInt(data.wQuality);

                    cols.push_back(quality);
                    colNames.push_back("quality" + std::to_string(i));
                }

                TableSP resultTable = Util::createTable(colNames, cols);
                vector<ConstantSP> args1 = {resultTable};

                INDEX insertedRows = 1;
                string errMsg;

                tp->append(args1, insertedRows, errMsg);
                cout << "insert rows:" << insertedRows << endl;
                if (insertedRows != resultTable->rows()|| errMsg !="") {
                    cout << "insert " << insertedRows << " err " << errMsg << endl;
                    throw RuntimeException(errMsg);
                }
                return Util::createTable(colNames, cols);

            } else {    // single value mode
                ConstantSP timestampVec = Util::createVector(DT_TIMESTAMP, items.size());
                ConstantSP valueVec;
                // read one
                auto item_first = itemsCreated[0];
                OPCItemData data_first;
                item_first->readSync(data_first, OPC_DS_DEVICE);

                valueVec = Util::createVector(tp->getColumn(2)->getType(), items.size());
                // valueVec = Util::createVector(DT_DOUBLE, items.size());
                ConstantSP qualityVec = Util::createVector(DT_INT, items.size());

                ConstantSP nameVec = Util::createVector(DT_STRING, items.size());

                for (int i = 0; i < toltallySize; i++) {
                    auto item = itemsCreated[i];
                    OPCItemData data;
                    item->readSync(data, OPC_DS_DEVICE);
                    unsigned long long ms;
                    FileTimeToMs(data.ftTimeStamp, ms);
                    ConstantSP timestamp = Util::createConstant(DT_TIMESTAMP);
                    timestamp->setLong(ms);
                    timestampVec->set(i, timestamp);
                    OpcVal val(data);

                    valueVec->set(i, val.toConstant());
                    ConstantSP quality = Util::createConstant(DT_INT);
                    quality->setInt(data.wQuality);
                    qualityVec->set(i, quality);
                    ConstantSP name = Util::createConstant(DT_STRING);
                    name->setString(item->getName());
                    nameVec->set(i, name);
                }
                vector<string> colNames = {"name", "timestamp", "value", "quality"};
                vector<ConstantSP> cols = {nameVec, timestampVec, valueVec, qualityVec};
                TableSP resultTable = Util::createTable(colNames, cols);

                vector<ConstantSP> args1 = {resultTable};
                INDEX insertedRows = 1;
                string errMsg;

                tp->append(args1, insertedRows, errMsg);
                cout << "insert rows:" << insertedRows << endl;
                if (insertedRows != resultTable->rows() || errMsg !="") {
                    cout << "insert " << insertedRows << " err " << errMsg << endl;
                    throw RuntimeException(errMsg);
                }
                return Util::createTable(colNames, cols);
                ;
            }

        } else {
            for (int i = 0; i < toltallySize; i++) {
                vector<string> colNames;
                vector<ConstantSP> cols;
                auto item = itemsCreated[i];
                OPCItemData data;
                item->readSync(data, OPC_DS_DEVICE);

                ConstantSP name = Util::createConstant(DT_STRING);
                colNames.push_back("tag");
                name->setString(item->getName());
                cols.push_back(name);

                unsigned long long ms;
                FileTimeToMs(data.ftTimeStamp, ms);
                ConstantSP timestamp = Util::createConstant(DT_TIMESTAMP);
                timestamp->setLong(ms);
                colNames.push_back("timestamp");
                cols.push_back(timestamp);

                OpcVal val(data);
                colNames.push_back(item->getName());
                cols.push_back(val.toConstant());

                ConstantSP quality = Util::createConstant(DT_INT);
                quality->setInt(data.wQuality);

                cols.push_back(quality);
                colNames.push_back("quality" + std::to_string(i));

                TableSP resultTable = Util::createTable(colNames, cols);
                vector<ConstantSP> args1 = {resultTable};

                INDEX insertedRows = 1;
                string errMsg;
                TableSP tp = arguments[2]->get(i);
                tp->append(args1, insertedRows, errMsg);

                if (insertedRows != resultTable->rows()|| errMsg !="") {
                    cout << "insert " << insertedRows << " err " << errMsg << endl;
                    throw RuntimeException(errMsg);
                }
            }
            vector<string> colNames{"col"};
            vector<ConstantSP> cols{Util::createVector(DT_INT, 0, 0)};
            return Util::createTable(colNames, cols);
        }
    }
}

ConstantSP readTag(Heap *heap, vector<ConstantSP> &arguments) {
    std::string usage = "Usage: readTag(conn,tag).";

    OPCClientSP conn;
    if (arguments[0]->getType() == DT_RESOURCE && arguments[0]->getString() == "opc connection") {
        conn = arguments[0];
        if (!conn->getConnected()) {
            throw IllegalArgumentException(__FUNCTION__, usage + "client is not connected.");
        }
    } else {
        throw IllegalArgumentException(__FUNCTION__, usage + "Invalid connection object.");
    }

    if ((arguments[1]->getType() != DT_STRING)) {
        throw IllegalArgumentException(__FUNCTION__, "the tag must be string scalar or string array");
    }

    if (arguments.size() > 2) {
        if (!arguments[2]->isTable()) {
            if (arguments[2]->isArray()) {
                if (!arguments[1]->isArray()) {
                    throw IllegalArgumentException(__FUNCTION__, "the table is a array but the tag  not");
                } else if (arguments[2]->size() != arguments[1]->size()) {
                    throw IllegalArgumentException(__FUNCTION__, "the tag size and the table size must be same");
                }
                for (int i = 0; i < arguments[2]->size(); i++) {
                    if (!arguments[2]->get(i)->isTable())
                        throw IllegalArgumentException(__FUNCTION__, "the table array must be all table");
                }
            } else {
                throw IllegalArgumentException(__FUNCTION__, "the table must be table scalar or table array");
            }
        } else {
            // cout<<"table.cols"<<arguments[2]->columns()<<endl;
        }
    }
    return pool.call(heap, readTagInternal, arguments, conn->id(), conn);
}

bool checkArgs(int vt, int DolphinDBType, bool isScalar, int DF) {
    switch (vt) {
        case VT_I1:
        case VT_I2:
        case VT_I4:
        case VT_UI1:
        case VT_UI2:
        case VT_UI4:
            // must be char/short/int/long
            if (!isScalar || (DolphinDBType != DT_CHAR && DolphinDBType != DT_SHORT && DolphinDBType != DT_INT &&
                              DolphinDBType != DT_LONG)) {
                return false;
            }
            break;
        case VT_CY:
        case VT_R4:
        case VT_R8:
            // must be double/float
            if (!isScalar || (DolphinDBType != DT_DOUBLE && DolphinDBType != DT_FLOAT)) {
                return false;
            }
            break;
        case VT_BOOL:
            // must be bool
            if (!isScalar || (DolphinDBType != DT_BOOL)) {
                return false;
            }
            break;
        case VT_ARRAY | VT_R8:
            if (DF != DF_VECTOR || DolphinDBType != DT_DOUBLE) {
                return false;
            }
            break;
        case VT_ARRAY | VT_BSTR:
            if (DF != DF_VECTOR || DolphinDBType != DT_STRING) {
                return false;
            }
            break;
        case VT_BSTR:
            // must be string
            if (!isScalar || (DolphinDBType != DT_STRING)) {
                return false;
            }
            break;
        case VT_DATE:
            if (!isScalar || (DolphinDBType != DT_DATETIME && DolphinDBType != DT_SECOND)) {
                return false;
            }
            break;
        default:
            return false;
    }
    return true;
}

TableSP writeTagInternal(Heap *heap, vector<ConstantSP> &arguments) {
    OPCClientSP conn = arguments[0];
    string itemName = arguments[1]->getString();
    COPCItem *item;
    item = conn->group->addItem(itemName, 0);
    // get type
    OPCItemData data_first;
    item->readSync(data_first, OPC_DS_DEVICE);
    delete item;
    int vt = data_first.vDataValue.vt;
    int res = checkArgs(vt, arguments[2]->getType(), arguments[2]->isScalar(), arguments[2]->getForm());

    if (!res) {
        throw IllegalArgumentException(__FUNCTION__, "type check error");
    }
    VARIANT var;
    var.vt = vt;
    switch (vt) {
        case VT_I1:
            var.iVal = arguments[2]->getChar();
            break;
        case VT_I2:
            var.iVal = arguments[2]->getShort();
            break;
        case VT_I4:
            var.intVal = arguments[2]->getInt();
            break;
        case VT_UI1:
            var.uiVal = arguments[2]->getChar();
            break;
        case VT_UI2:
            var.uiVal = arguments[2]->getShort();
            break;
        case VT_UI4:
            var.uintVal = arguments[2]->getInt();
            break;
        case VT_CY:
            var.cyVal.int64 = arguments[2]->getDouble() * 10000;
            break;
        case VT_R4:
            var.fltVal = arguments[2]->getFloat();
            break;
        case VT_R8:
            var.dblVal = arguments[2]->getDouble();
            break;
        case VT_BOOL:
            var.boolVal = arguments[2]->getBool();
            break;
        case VT_BSTR:
            var.bstrVal = SysAllocString(T2OLE(arguments[2]->getString().c_str()));
            break;
        case VT_ARRAY | VT_BSTR: {
            auto size = arguments[2]->size();
            SAFEARRAYBOUND bound[1];
            bound[0].cElements = size;
            bound[0].lLbound = 0;
            var.parray = SafeArrayCreate(VT_BSTR, 1, bound);
            for (int i = 0; i < size; i++) {
                auto element = arguments[2]->get(i);
                BSTR s = SysAllocString(T2OLE(element->getString().c_str()));
                long int ix[1];
                ix[0] = i;
                SafeArrayPutElement(var.parray, ix, s);
            }
            break;
        }
        case VT_ARRAY | VT_R8: {
            auto size = arguments[2]->size();
            SAFEARRAYBOUND bound[1];
            bound[0].cElements = size;
            bound[0].lLbound = 0;
            var.parray = SafeArrayCreate(VT_R8, 1, bound);

            for (int i = 0; i < size; i++) {
                auto element = arguments[2]->get(i);
                auto dblVal = element->getDouble();
                long int ix[1];
                ix[0] = i;
                SafeArrayPutElement(var.parray, ix, &dblVal);
            }
            break;
        }
        case VT_DATE: {
            FILETIME t;
            SYSTEMTIME st;
            auto timestamp = arguments[2]->getLong();
            MsToFileTime(t, timestamp * 1000);
            FileTimeToSystemTime(&t, &st);
            SystemTimeToVariantTime(&st, &var.date);
            break;
        }
        default:
            throw OPCException("unknow type");
    }
    item->writeSync(var);

    vector<string> colNames{"col"};
    vector<ConstantSP> cols{Util::createVector(DT_INT, 0, 0)};
    return Util::createTable(colNames, cols);
}

ConstantSP writeTag(Heap *heap, vector<ConstantSP> &arguments) {
    std::string usage = "Usage: writeTag(conn,tag,value).";
    OPCClientSP conn;
    if (arguments[0]->getType() == DT_RESOURCE && arguments[0]->getString() == "opc connection") {
        conn = arguments[0];
        if (!conn->getConnected()) {
            throw IllegalArgumentException(__FUNCTION__, usage + "client is not connected.");
        }
    } else {
        throw IllegalArgumentException(__FUNCTION__, usage + "Invalid connection object.");
    }

    if (arguments[1]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, "the tag must be string scalar or string array");
    }
    // multiple tags
    if (arguments[1]->isArray()) {
        if (!arguments[2]->isArray()) {
            throw IllegalArgumentException(__FUNCTION__, "the tag is an array but the value not");
        } else if (arguments[2]->size() != arguments[1]->size()) {
            throw IllegalArgumentException(__FUNCTION__,
                                           "the tag array size and the value array size must be the same");
        }
        for (int i = 0; i < arguments[2]->size(); i++) {
            vector<ConstantSP> args1 = {arguments[0], arguments[1]->get(i), arguments[2]->get(i)};

            ConstantSP ret = pool.call(heap, writeTagInternal, args1, conn->id(), conn);
            if (ret.isNull())
                return new Bool(false);
        }
        return new Bool(true);
    }
    // one tag
    ConstantSP ret = pool.call(heap, writeTagInternal, arguments, conn->id(), conn);
    if (ret.isNull())
        return new Bool(false);
    else
        return new Bool(true);
}

ConstantSP subscribeTag(Heap *heap, vector<ConstantSP> &arguments) {
    std::string usage = "Usage: subscribe(conn,Tag,handler). ";
    OPCClientSP conn;
    if (arguments[0]->getType() == DT_RESOURCE && arguments[0]->getString() == "opc connection") {
        conn = arguments[0];
        if (!conn->getConnected()) {
            throw IllegalArgumentException(__FUNCTION__, usage + "client is not connected.");
        }
        if (!conn->getSubFlag()) {
            throw IllegalArgumentException(__FUNCTION__, usage + "client is subscribed.");
        }
    } else {
        throw IllegalArgumentException(__FUNCTION__, usage + "Invalid connection object.");
    }

    if ((arguments[1]->getType() != DT_STRING)) {
        throw IllegalArgumentException(__FUNCTION__, usage + "the tag must be string scalar or string array");
    }
    if (!arguments[2]->isTable() && (arguments[2]->getType() != DT_FUNCTIONDEF)) {
        throw IllegalArgumentException(__FUNCTION__, usage + "handler must be a  table or a unary function.");
    }else if (arguments[2]->getType() == DT_FUNCTIONDEF) {
        if (FunctionDefSP(arguments[2])->getMaxParamCount() < 1 || FunctionDefSP(arguments[2])->getMinParamCount() > 1)
            throw IllegalArgumentException(__FUNCTION__, usage + "handler must be a table or a unary function.");
    }

    FunctionDefSP handler;
    {
        LockGuard<Mutex> lk(conn->getLock());
        conn->session = heap->currentSession()->copy();
        conn->session->setUser(heap->currentSession()->getUser());
        conn->session->setOutput(new DummyOutput);

        if (arguments[2]->getType() == DT_FUNCTIONDEF) {
            handler = FunctionDefSP(arguments[2]);
        } else {
            TableSP table = arguments[2];
            FunctionDefSP func = conn->session->getFunctionDef("append!");
            vector<ConstantSP> params(1, table);
            handler = Util::createPartialFunction(func, params);
        }
        conn->setCreateTime(Util::getEpochTime());
        conn->setErrorMsg("");
        conn->resetRecv();
        conn->setTagName(arguments[1]->getString());
    }
    pool.sub(heap, conn, arguments[1], handler);
    LockGuard<Mutex> lk(&OPCPluginImp::LOCK);
    OPCPluginImp::SUB_MAP[reinterpret_cast<long long>(conn.get())] = conn;

    return new Void();
}
