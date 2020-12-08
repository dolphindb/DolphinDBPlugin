#include "opc_main.h"
#include <Windows.h>
#include <random>
#include <utility>
#include "ScalarImp.h"
#include "opcimp.h"

DictionarySP dict = Util::createDictionary(DT_STRING, 0, DT_LONG, 0);

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

ConstantSP toConstant(OPCItemData data, string itemName) {
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

class CMyCallback : public IAsynchDataCallback {
private:
    Heap *_heap;
    ConstantSP _handler;
    OPCClient *_conn;

public:
    CMyCallback(Heap *heap,OPCClient *conn, ConstantSP callback) : _heap(heap),_conn(conn), _handler(callback) {
    }
    void OnDataChange(COPCGroup &group, map<ItemID, OPCItemData *> &changes) {
        // printf("Group %s, item changes\n", group.getName().c_str());
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
                vector<ConstantSP> args = {v};
                ((FunctionDefSP)_handler)->call(_heap, args);
            } catch (exception &e) {
                cout << "Failed to append:" << e.what() << endl;
                _conn->setErrorMsg(e.what());
            }
        }
    }
};

// todo : use forward??
typedef std::function<Table *(Heap *, vector<ConstantSP> &)> FUNC;
struct Task {    // connect or sub
    enum ACTION { CONN, SUB, UNSUB, CALL };
    ACTION act;
    Heap *heap;
    OPCClient *conn;
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
    Table **retTable;
    std::shared_ptr<string> errMsg;
    Task() : Task(nullptr, nullptr, nullptr, nullptr, nullptr) {
    }
    explicit Task(Heap *heap, OPCClient *conn, const ConstantSP &tagName, const ConstantSP &callback,
                  CountDownLatchSP latch)
        : act(SUB),
          heap(heap),
          conn(conn),
          tagName(tagName),
          serverName(""),
          callback(callback),
          reqRt(0),
          latch(latch),
          errMsg(new string()) {
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
          latch(latch),
          errMsg(new string()) {
    }
    explicit Task(Heap *heap, FUNC &&func, vector<ConstantSP> &args, CountDownLatchSP latch)
        : act(CALL), heap(heap), func(func), args(args), latch(latch), errMsg(new string()) {
        retTable = reinterpret_cast<Table **>(new long long);
    }
    explicit Task(Heap *heap, OPCClient *conn, CountDownLatchSP latch)
        : act(UNSUB), conn(conn), latch(latch), errMsg(new string()) {
    }
};

class OPCWorker : public Runnable {
public:
    explicit OPCWorker(int id) : id_(id) {
    }
    void run() override {
        while (true) {
            Task t;
            if (queue_.pop(t)) {
                try {
                    switch (t.act) {
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
                            (*t.retTable) = t.func(t.heap, t.args);
                            t.latch->countDown();
                            break;
                        default:
                            break;
                    }
                } catch (exception &e) {
                    // cout <<"Failed to work:"<< e.what() << endl;
                    *t.errMsg = e.what();
                    //*t.retTable = 0;
                    t.latch->countDown();
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
    }
    void sub(Heap *heap, OPCClient *conn, ConstantSP tagName, ConstantSP callback) {
        CountDownLatchSP latch = new CountDownLatch(1);
        Task t(heap, conn, tagName, callback, latch);
        queue_.push(t);
        latch->wait();
        if (!t.errMsg->empty()) {
            throw RuntimeException(*t.errMsg);
        }

    }
    void unsub(OPCClient *conn) {
        CountDownLatchSP latch = new CountDownLatch(1);
        Task t(0, conn, latch);
        queue_.push(t);
        latch->wait();
        if (!t.errMsg->empty()) {
            throw RuntimeException(*t.errMsg);
        }

    }
    OPCClient *connect(Heap *heap, string host, string serverName, unsigned long reqUpdateRate_ms) {
        CountDownLatchSP latch = new CountDownLatch(1);
        Task t(heap, host, serverName, reqUpdateRate_ms, latch);
        t.ret = new Long(-1);
        queue_.push(t);
        latch->wait();

        if (!t.errMsg->empty()) {
            throw RuntimeException(*t.errMsg);
        }
        if (t.ret->getLong() == 0)
            throw RuntimeException("Failed to connect the server");
        return reinterpret_cast<OPCClient *>(t.ret->getLong());
    }
    Table *call(Heap *heap, FUNC &&func, vector<ConstantSP> &args) {
        CountDownLatchSP latch = new CountDownLatch(1);
        Task t(heap, move(func), args, latch);
        queue_.push(t);
        latch->wait();

        if (!t.errMsg->empty()) {
            throw RuntimeException(*t.errMsg);
        }
        return *t.retTable;
    }

private:
    static void subInternal(Task &task) {
        auto &conn = task.conn;
        auto &heap = task.heap;
        auto &callback = task.callback;
        auto &tagName = task.tagName;

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
            throw OPCException("Failed to add item");
        }

        for (unsigned i = 0; i < items.size(); i++) {
            group->mapTag.insert(pair<int, string>(itemsCreated[i]->getID(), items[i]));
        }

        unique_ptr<IAsynchDataCallback> ptr(new CMyCallback(heap, conn,callback));

        group->enableAsynch(move(ptr));

        conn->setSubFlag(false);

        task.latch->countDown();
    }
    static void unsubInternal(Task &task) {
        auto &conn = task.conn;
        auto group = conn->group;
        // group->removeItems();
        group->mapTag.clear();
        group->disableAsynch();
        task.latch->countDown();
    }
    void connectInternal(Task &task) {
        auto &host = task.host;
        auto &serverName = task.serverName;
        auto &reqUpdateRate_ms = task.reqRt;
        OPCClient *cup = new OPCClient(host, id_,serverName);

        cup->connectToOPCServer(serverName);

        unsigned long refresh_rate = 0;
        cup->makeGroup("Group", true, reqUpdateRate_ms, refresh_rate, 0.0);
        task.ret->setLong(reinterpret_cast<long long>(cup));
        task.latch->countDown();
    }
    SynchronizedQueue<Task> queue_;
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
    void sub(Heap *heap, OPCClient *conn, ConstantSP tagName, ConstantSP callback) {
        int id = conn->id();

        cout << "threadID: " << id << endl;
        worker_[id]->sub(heap, conn, tagName, callback);
    }
    void unsub(OPCClient *conn) {
        int id = conn->id();
        cout << "unsub from id" << id << endl;
        worker_[id]->unsub(conn);
    }
    OPCClient *connect(Heap *heap, string host, string serverName, unsigned long reqUpdateRate_ms) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<> dis(0, poolSize_);
        int id = dis(gen);
        cout << "threadID: " << id << endl;
        return worker_[id]->connect(heap, host, serverName, reqUpdateRate_ms);
    }
    ConstantSP call(Heap *heap, FUNC &&func, vector<ConstantSP> &args, int id) {
        return worker_[id]->call(heap, move(func), args);
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
    auto client = new OPCClient(host, 0,"");    // fixme: this function may not work any more
    vector<string> progIds;
    vector<CLSID> ids;
    client->getServerList(progIds, ids);

    vector<string> colNames = {"progID", "CLSID"};
    ConstantSP cIds = createSVCFromVector(CLSIDToStringVector(ids));
    ConstantSP cProgIds = createSVCFromVector(progIds);
    vector<ConstantSP> cols = {cProgIds, cIds};
    Table *retTable = Util::createTable(colNames, cols);
    delete client;
    return retTable;
}
static void opcConnectionOnClose(Heap *heap, vector<ConstantSP> &args) {
    OPCClient *cp = (OPCClient *)(args[0]->getLong());
    if (dict->getMember(std::to_string(args[0]->getLong()))->isNothing()) {
        if (cp != nullptr) {
            delete cp;
            args[0]->setLong(0);
        }
    } else {
        cp->sessionClosed = true;
    }
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

    std::unique_ptr<OPCClient> cup(pool.connect(heap, host, serverName, reqUpdateRate_ms));

    FunctionDefSP onClose(Util::createSystemProcedure("opc connection onClose()", opcConnectionOnClose, 1, 1));
    return Util::createResource((long long)cup.release(), "opc connection", onClose, heap->currentSession());
}

ConstantSP disconnect(const ConstantSP &handle, const ConstantSP &b) {
    std::string usage = "Usage: close(conn). ";
    OPCClient *conn;
    if (handle->getType() == DT_RESOURCE) {
        conn = (OPCClient *)(handle->getLong());

    } else {
        throw IllegalArgumentException(__FUNCTION__, usage + "Invalid connection object.");
    }

    if (conn != nullptr) {
        handle->setLong(0);
        if (!conn->getSubFlag())
            return new Bool(false);//throw IllegalArgumentException(__FUNCTION__, "There is a subscriber yet.Please unsubscriber firstly.");//pool.unsub(conn);
        delete conn;
    }

    return new Bool(true);
}

ConstantSP endSub(const ConstantSP &handle, const ConstantSP &b) {
    std::string usage = "Usage: unsubscribe(subscription). ";
    OPCClient *conn;
    if (handle->getType() == DT_RESOURCE) {
        conn = (OPCClient *)(handle->getLong());

    } else if (handle->getType() == DT_STRING) {
        ConstantSP c = dict->getMember(handle->getString());
        if (c->isNothing()) {
            throw IllegalArgumentException(__FUNCTION__, "Invalid connection string.");
        }
        conn = (OPCClient *)(c->getLong());
    } else if (handle->getType() == DT_LONG || handle->getType() == DT_INT ) {
        ConstantSP c = dict->getMember(handle->getString());
        if (c->isNothing()) {
            throw IllegalArgumentException(__FUNCTION__, "Invalid connection string.");
        }
        conn = (OPCClient *)(c->getLong());
    } else {
        throw IllegalArgumentException(__FUNCTION__, "Invalid connection object.");
    }

    if (conn != nullptr) {
        conn->setSubFlag(true);
        pool.unsub(conn);
        dict->remove(new String(std::to_string(conn->id())));
        if(conn->sessionClosed) delete conn;
    }
    return new Void();
}
ConstantSP getSubscriberStat(const ConstantSP &handle, const ConstantSP &b) {
    int size = dict->size();
    ConstantSP connectionIdVec = Util::createVector(DT_STRING, size);
    ConstantSP userVec = Util::createVector(DT_STRING, size);
    ConstantSP hostVec = Util::createVector(DT_STRING, size);
    ConstantSP serverNameVec = Util::createVector(DT_STRING, size);
    ConstantSP tagVec = Util::createVector(DT_STRING, size);
    ConstantSP timestampVec = Util::createVector(DT_TIMESTAMP, size);
    ConstantSP recvVec = Util::createVector(DT_LONG, size);
    ConstantSP errorMsgVec = Util::createVector(DT_STRING, size);
    VectorSP keys = dict->keys();
    for (int i = 0; i < keys->size(); i++) {
        string key = keys->getString(i);
        connectionIdVec->setString(i, key);
        OPCClient *conn = (OPCClient *)dict->getMember(key)->getLong();
        hostVec->setString(i,conn->getHost());
        serverNameVec->setString(i,conn->getServerName());
        tagVec->setString(i,conn->getTagName());
        recvVec->setLong(i,conn->getRecv());
        timestampVec->setLong(i,conn->getCreateTime());
        userVec->setString(i,conn->session->getUser()->getUserId());
        errorMsgVec->setString(i,conn->getErrorMsg());
    }

    vector<string> colNames = {"subscriptionId", "user", "host", "serverName", "tag", "createTimestamp", "receivedPackets","errorMsg"};
    vector<ConstantSP> cols = {connectionIdVec, userVec, hostVec, serverNameVec, tagVec, timestampVec, recvVec,errorMsgVec};
    return Util::createTable(colNames, cols);
}
Table *readTagInternal(Heap *heap, vector<ConstantSP> &arguments) {
    OPCClient *conn = (OPCClient *)(arguments[0]->getLong());
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
        throw OPCException("Failed to add item");
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

        OpcVal *val_first = new OpcVal(data_first);
        cout << val_first->vt << endl;

        switch (val_first->vt) {
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
            OpcVal *val = new OpcVal(data);

            valueVec->set(i, val->toConstant());
            ConstantSP quality = Util::createConstant(DT_INT);
            quality->setInt(data.wQuality);
            qualityVec->set(i, quality);
            ConstantSP name = Util::createConstant(DT_STRING);
            name->setString(item->getName());
            nameVec->set(i, name);
        }
        vector<string> colNames = {"name", "timestamp", "value", "quality"};
        vector<ConstantSP> cols = {nameVec, timestampVec, valueVec, qualityVec};
        Table *retTable = Util::createTable(colNames, cols);
        return retTable;
    } else {
        if (arguments[2]->isTable()) {
            Table *tp = (Table *)arguments[2].get();
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

                    OpcVal *val = new OpcVal(data);
                    colNames.push_back(item->getName());
                    cols.push_back(val->toConstant());

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
                    OpcVal *val = new OpcVal(data);

                    valueVec->set(i, val->toConstant());
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

                OpcVal *val = new OpcVal(data);
                colNames.push_back(item->getName());
                cols.push_back(val->toConstant());

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

    OPCClient *conn;
    if (arguments[0]->getType() == DT_RESOURCE) {
        conn = (OPCClient *)(arguments[0]->getLong());
        if (conn == nullptr || !conn->getConnected()) {
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
    return pool.call(heap, readTagInternal, arguments, conn->id());
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

Table *writeTagInternal(Heap *heap, vector<ConstantSP> &arguments) {
    OPCClient *conn = (OPCClient *)(arguments[0]->getLong());
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

    OPCClient *conn;
    if (arguments[0]->getType() == DT_RESOURCE) {
        conn = (OPCClient *)(arguments[0]->getLong());
        if (conn == nullptr || !conn->getConnected()) {
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

            ConstantSP ret = pool.call(heap, writeTagInternal, args1, conn->id());
            if (ret.isNull())
                return new Bool(false);
        }
        return new Bool(true);
    }
    // one tag
    ConstantSP ret = pool.call(heap, writeTagInternal, arguments, conn->id());
    if (ret.isNull())
        return new Bool(false);
    else
        return new Bool(true);
}

ConstantSP subscribeTag(Heap *heap, vector<ConstantSP> &arguments) {
    std::string usage = "Usage: subscribe(conn,Tag,handler). ";

    OPCClient *conn;
    if (arguments[0]->getType() == DT_RESOURCE) {
        conn = (OPCClient *)(arguments[0]->getLong());
        if (conn == nullptr || !conn->getConnected()) {
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

    pool.sub(conn->session->getHeap().get(), conn, arguments[1], handler);

    conn->setCreateTime(Util::getEpochTime());
    conn->setErrorMsg("");
    conn->resetRecv();
    conn->setTagName(arguments[1]->getString());
    dict->set(std::to_string(conn->id()), new Long((long long)conn));

    return new Void();
}
