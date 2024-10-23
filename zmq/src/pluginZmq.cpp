#include"pluginZmq.h"


template<class T>
static void connectionOnClose(Heap *heap, vector<ConstantSP> &args) {
    delete (T *) (args[0]->getLong());
}

ConstantSP zmqSocket(Heap *heap, vector<ConstantSP> &args) {
    if (args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR) {
        throw RuntimeException(PLUGIN_ZMQ_PREFIX+"socket type must be a string scalar");
    }
    if (args[1]->getType() != DT_FUNCTIONDEF) {
        throw RuntimeException(PLUGIN_ZMQ_PREFIX+"formatter must be a fuction");
    }
    int batchSize = -1;
    if (args.size() > 2) {
        if (args[2]->getType() != DT_INT || args[2]->getForm() != DF_SCALAR) {
            throw RuntimeException(PLUGIN_ZMQ_PREFIX+"batchSize must be a int scalar");
        }
        batchSize = args[2]->getInt();
        if(batchSize <= 0){
            throw RuntimeException(PLUGIN_ZMQ_PREFIX+"batchSize must be greater than 0");
        }
    }
    string prefix;
    if (args.size() > 3) {
        if (args[3]->getType() != DT_STRING || args[3]->getForm() != DF_SCALAR) {
            throw RuntimeException(PLUGIN_ZMQ_PREFIX+"prefix must be a string");
        }
        prefix = args[3]->getString();
    }
    string socketType = args[0]->getString();
    FunctionDefSP formatter = args[1];
    std::unique_ptr<ZmqPushSocket> cup(new ZmqPushSocket(socketType, formatter, batchSize));
    FunctionDefSP onClose(
            Util::createSystemProcedure("zmq connection onClose()", connectionOnClose<ZmqPushSocket>, 1, 1));
    return Util::createResource((long long) cup.release(), "zmq socket", onClose, heap->currentSession());
}

ConstantSP zmqConnect(Heap *heap, vector<ConstantSP> &args) {
    if (args[0]->getType() != DT_RESOURCE || args[0]->getLong() == 0 || args[0]->getString().find("zmq socket") == string::npos) {
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_ZMQ_PREFIX+"handle must be a zmq connection");
    }
    ZmqPushSocket *socket = reinterpret_cast<ZmqPushSocket *> (args[0]->getLong());
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR) {
        throw RuntimeException(PLUGIN_ZMQ_PREFIX+"addr must be a string scalar");
    }
    string prefix;
    if(args.size() > 2) {
        if (args[2]->getType() != DT_STRING || args[2]->getForm() != DF_SCALAR) {
            throw RuntimeException(PLUGIN_ZMQ_PREFIX+"prefix must be a string scalar");
        }
        prefix = args[2]->getString();
    }
    string addr = args[1]->getString();
    socket->connect(addr, prefix);
    return new Bool(true);
}

ConstantSP zmqBind(Heap *heap, vector<ConstantSP> &args) {
    if (args[0]->getType() != DT_RESOURCE || args[0]->getLong() == 0 || args[0]->getString().find("zmq socket") == string::npos) {
        throw IllegalArgumentException(__FUNCTION__, "handle must be a zmq connection");
    }
    ZmqPushSocket *socket = reinterpret_cast<ZmqPushSocket *> (args[0]->getLong());
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR) {
        throw RuntimeException(PLUGIN_ZMQ_PREFIX+"addr must be a string scalar");
    }
    string addr = args[1]->getString();
    string prefix;
    if(args.size() > 2) {
        if (args[2]->getType() != DT_STRING || args[2]->getForm() != DF_SCALAR) {
            throw RuntimeException(PLUGIN_ZMQ_PREFIX+"prefix must be a string scalar");
        }
        prefix = args[2]->getString();
    }
    socket->bind(addr, prefix);
    return new Bool(true);
}

bool sendSuccess(zmq::send_result_t& rc, size_t msgCount){
    if (!rc.has_value() || rc.value() != msgCount){
        return false;
    }
    return true;
}

ConstantSP zmqSend(Heap *heap, vector<ConstantSP> &args) {
    if (args[0]->getType() != DT_RESOURCE || args[0]->getLong() == 0 || args[0]->getString().find("zmq socket") == string::npos) {
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_ZMQ_PREFIX+"handle must be a zmq connection");
    }
    ZmqPushSocket *socket = reinterpret_cast<ZmqPushSocket *> (args[0]->getLong());
    vector<ConstantSP> fucCallArgs;
    fucCallArgs.push_back(args[0]);
    shared_ptr<zmq::socket_t> zmqSocket = socket->getSocket();
    string prefix = socket->getPrefix();
    ConstantSP data = args[1];
    int batchSize = socket->getBatchSize();
    if (args.size() > 2) {
        if (args[2]->getType() != DT_STRING || args[2]->getForm() != DF_SCALAR)
            throw RuntimeException(PLUGIN_ZMQ_PREFIX+"prefix must be a string scalar");
        prefix = args[2]->getString();
    }
    if(batchSize != -1 && args[1]->getForm() == DF_TABLE){
        int index = 0;
        int size = data->size();
        while(index < size){
            if (prefix != "") {
                zmq::message_t message(prefix.size());
                memcpy(message.data(), prefix.data(), prefix.size());
                auto rc = zmqSocket->send(message, zmq::send_flags::sndmore);
                if (!sendSuccess(rc, prefix.size()))
                    return new Bool(false);
            }
            int end = min(index + batchSize, size);
            static FunctionDefSP seq = heap->currentSession()->getFunctionDef("seq");
            vector<ConstantSP> args0 = {new Int(index), new Int(end - 1)};
            ConstantSP result = seq->call(heap, args0);
            TableSP subTable = data->get(result);
            vector<ConstantSP> arg;
            arg.push_back(subTable);
            string serialResult = socket->getFormatter()->call(heap, arg)->getString();
            zmq::message_t message(serialResult.size());
            memcpy(message.data(), serialResult.data(), serialResult.size());
            auto rc = zmqSocket->send(message, zmq::send_flags::none);
            if (!sendSuccess(rc, serialResult.size()))
                return new Bool(false);
            index += batchSize;
        }
    }
    else {
        if (prefix != "") {
            zmq::message_t message(prefix.size());
            memcpy(message.data(), prefix.data(), prefix.size());
            auto rc = zmqSocket->send(message, zmq::send_flags::sndmore);
            if (!sendSuccess(rc, prefix.size()))
                return new Bool(false);
        }
        vector<ConstantSP> arg;
        arg.push_back(data);
        string serialResult = socket->getFormatter()->call(heap, arg)->getString();
        zmq::message_t message(serialResult.size());
        memcpy(message.data(), serialResult.data(), serialResult.size());
        auto rc = zmqSocket->send(message, zmq::send_flags::none);
        if (!sendSuccess(rc, serialResult.size()))
            return new Bool(false);
    }
    return new Bool(true);
}

ConstantSP zmqCreateSubJob(Heap *heap, vector<ConstantSP> &args) {
    if (args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR) {
        throw RuntimeException(PLUGIN_ZMQ_PREFIX+"addr must be a string scalar");
    }
    string addr = args[0]->getString();
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR) {
        throw RuntimeException(PLUGIN_ZMQ_PREFIX+"type must be a string scalar");
    }
    string socketType = args[1]->getString();
    if (args[2]->getType() != DT_BOOL || args[2]->getForm() != DF_SCALAR) {
        throw RuntimeException(PLUGIN_ZMQ_PREFIX+"isConnect must be a bool scalar");
    }
    bool isConnect = args[2]->getBool();
    if (args[3]->getForm() != DF_TABLE && args[3]->getType() != DT_FUNCTIONDEF) {
        throw RuntimeException(PLUGIN_ZMQ_PREFIX+"handle must be a table or a fuction");
    }
    ConstantSP handle = args[3];
    if (args[4]->getType() != DT_FUNCTIONDEF) {
        throw RuntimeException(PLUGIN_ZMQ_PREFIX+"parser must be a fuction");
    }
    FunctionDefSP parser = args[4];
    string prefix;
    if (args.size() > 5) {
        if (args[5]->getType() != DT_STRING || args[5]->getForm() != DF_SCALAR) {
            throw RuntimeException(PLUGIN_ZMQ_PREFIX+"prefix must be a string scalar");
        }
        prefix = args[5]->getString();
    }
    std::unique_ptr<SubConnection> cup(
            new SubConnection(heap, make_shared<ZmqSubSocket>(addr, socketType, parser, prefix, isConnect), parser, handle));
    FunctionDefSP onClose(
            Util::createSystemProcedure("zmq sub connection onClose()", connectionOnClose<SubConnection>, 1, 1));
    ConstantSP conn = Util::createResource((long long) cup.release(), "zmq subscribe connection", onClose,
                                           heap->currentSession());
    LockGuard<Mutex> lock(&ZmqStatus::GLOBAL_LOCK);
    ZmqStatus::STATUS_DICT->set(to_string(conn->getLong()), conn);
    return conn;
}

SubConnection::SubConnection(Heap *heap, shared_ptr<ZmqSubSocket> socket, const FunctionDefSP &parser,
                             ConstantSP handle)
        : heap_(heap) {
    session_ = heap->currentSession()->copy();
    createTime_ = Util::getEpochTime();
    session_->setUser(heap->currentSession()->getUser());
    appendTable_ = new AppendTable(heap, socket, parser, handle);
    thread_ = new Thread(appendTable_);
    if (!thread_->isStarted()) {
        thread_->detach();
        thread_->start();
    }
}

void AppendTable::run() {
    try{
        HeapSP heap = session_->getHeap();
        shared_ptr<zmq::socket_t> zmqSocket = socket_->getSocket();
        string prefix = socket_->getPrefix();
        bool first = true;
        while (!needStop_) {
            if(!first)
                increaseRecv();
            try {
                vector<ConstantSP> args;
                if (prefix != "") {
                    zmq::message_t message;
                    while(!needStop_){
                        zmq::recv_result_t ret = zmqSocket->recv(message);
                        if(ret.has_value())
                            break;
                    }
                }
                if(needStop_)
                    break;
                zmq::message_t message;
                while(!needStop_){
                    zmq::recv_result_t ret = zmqSocket->recv(message);
                    if(ret.has_value())
                        break;
                }
                if(needStop_)
                    break;
                string contents((char *) message.data(), message.size());
                ConstantSP m = Util::createConstant(DT_STRING);
                m->setString(contents);
                args.push_back(m);
                ConstantSP parser_result;
                parser_result = parser_->call(heap.get(), args);
                if (!parser_result.isNull() && parser_result->isTable()) {
                    if (handle_->isTable()) {
                        TableSP table_insert = (TableSP) parser_result;
                        int length = handle_->columns();
                        if (table_insert->columns() < length) {
                            LOG_ERR(PLUGIN_ZMQ_PREFIX+"The columns of the table returned is smaller than the handler table.");
                        }
                        if (table_insert->columns() > length)
                            LOG_ERR(PLUGIN_ZMQ_PREFIX+"The columns of the table returned is larger than the handler table, and the information may be ignored.");
                        vector<ConstantSP> args = {handle_, table_insert};
                        session_->getFunctionDef("append!")->call(heap.get(), args);
                    } else {
                        vector<ConstantSP> args = {parser_result};
                        ((FunctionDefSP) handle_)->call(heap.get(), args);
                    }
                }else{
                    LOG_ERR(PLUGIN_ZMQ_PREFIX+"parser result must be a table.");
                }
            }
            catch (exception& e){
                LOG_ERR(PLUGIN_ZMQ_PREFIX + " SubConnection throws an exception: "+e.what());
            }
            first = false;
        }
        isStop_.release();
    }catch(exception& e){
        LOG_ERR(PLUGIN_ZMQ_PREFIX + "The subscribed thread ends because of the exception: " + e.what());
    }catch(...){
        LOG_ERR(PLUGIN_ZMQ_PREFIX + "The subscribed thread ends because of the exception.");
    }
}

ConstantSP zmqClose(Heap *heap, vector<ConstantSP> &args) {
    if (args[0]->getType() != DT_RESOURCE || args[0]->getLong() == 0 ||
        args[0]->getString().find("zmq socket") != 0) {
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_ZMQ_PREFIX+"channel must be a zmq Socket handle");
    }
    ZmqSocket *socket = reinterpret_cast<ZmqSocket *> (args[0]->getLong());
    shared_ptr<zmq::socket_t> zmqSocket = socket->getSocket();
    zmqSocket->close();
    return new Bool(true);
}

ConstantSP zmqGetSubJobStat(Heap *heap, vector<ConstantSP> &args) {
    LockGuard<Mutex> lock(&ZmqStatus::GLOBAL_LOCK);
    int size = ZmqStatus::STATUS_DICT->size();
    VectorSP connetionVec = Util::createVector(DT_STRING, size);
    VectorSP subAddrVec = Util::createVector(DT_STRING, size);
    VectorSP prefixVec = Util::createVector(DT_STRING, size);
    VectorSP recv = Util::createVector(DT_LONG, size);
    VectorSP createTimestamp = Util::createVector(DT_TIMESTAMP, size);
    VectorSP keys = ZmqStatus::STATUS_DICT->keys();
    for (int i = 0; i < size; ++i) {
        string key = keys->getString(i);
        ConstantSP conn = ZmqStatus::STATUS_DICT->getMember(key);
        SubConnection *zmqSubCon = (SubConnection *) conn->getLong();
        SmartPointer<AppendTable> appendTable = zmqSubCon->getAppendTable();
        shared_ptr<ZmqSubSocket> subSocket = appendTable->getZmqSocket();
        connetionVec->setString(i, key);
        subAddrVec->setString(i, subSocket->getAddr());
        prefixVec->setString(i, subSocket->getPrefix());
        recv->setLong(i, appendTable->getRecv());
        createTimestamp->setLong(i, zmqSubCon->getCreateTime());
    }
    vector<ConstantSP> cols = {connetionVec, subAddrVec, prefixVec, recv, createTimestamp};
    vector<string> colName = {"subscriptionId", "addr", "prefix", "recvPackets", "createTimestamp"};
    return Util::createTable(colName, cols);
}

ConstantSP zmqCancelSubJob(Heap *heap, vector<ConstantSP> args) {
    LockGuard<Mutex> lock(&ZmqStatus::GLOBAL_LOCK);
    std::string usage = "Usage: cancelSubJob(connection or connection ID). ";
    SubConnection *sc = nullptr;
    string key;
    ConstantSP conn = nullptr;
    auto handle = args[0];
    if(handle->getForm() != DF_SCALAR)
                throw IllegalArgumentException(__FUNCTION__, PLUGIN_ZMQ_PREFIX+"handle must be a scalar.");
    switch (handle->getType()) {
        case DT_RESOURCE:
            sc = (SubConnection *) (handle->getLong());
            key = std::to_string(handle->getLong());
            conn = ZmqStatus::STATUS_DICT->getMember(key);
            if (conn->isNothing())
                throw IllegalArgumentException(__FUNCTION__, PLUGIN_ZMQ_PREFIX+"Invalid connection object.");
            break;
        case DT_STRING:
            key = handle->getString();
            conn = ZmqStatus::STATUS_DICT->getMember(key);
            if (conn->isNothing())
                throw IllegalArgumentException(__FUNCTION__, PLUGIN_ZMQ_PREFIX+"Invalid connection string.");
            else
                sc = (SubConnection *) (conn->getLong());
            break;
        case DT_LONG:
            key = std::to_string(handle->getLong());
            conn = ZmqStatus::STATUS_DICT->getMember(key);
            if (conn->isNothing())
                throw IllegalArgumentException(__FUNCTION__, PLUGIN_ZMQ_PREFIX+"Invalid connection integer.");
            else
                sc = (SubConnection *) (conn->getLong());
            break;
        case DT_INT:
            key = std::to_string(handle->getInt());
            conn = ZmqStatus::STATUS_DICT->getMember(key);
            if (conn->isNothing())
                throw IllegalArgumentException(__FUNCTION__, PLUGIN_ZMQ_PREFIX+"Invalid connection integer.");
            else
                sc = (SubConnection *) (conn->getLong());
            break;
        default:
            throw IllegalArgumentException(__FUNCTION__, PLUGIN_ZMQ_PREFIX+"Invalid connection object.");
    }
    ZmqStatus::STATUS_DICT->remove(new String(key));
    if (sc != nullptr) {
        sc->cancelThread();
        LOG_INFO(PLUGIN_ZMQ_PREFIX+"subscription: " + std::to_string(conn->getLong()) + " is stopped. ");
    }
    return new Void();
}

ConstantSP zmqCreatepusher(Heap *heap, vector<ConstantSP> &args){
    if (args[0]->getType() != DT_RESOURCE || args[0]->getLong() == 0 || args[0]->getString().find("zmq socket") == string::npos) {
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_ZMQ_PREFIX+"handle must be a zmq connection");
    }
    if(args[1]->getForm() != DF_TABLE)
        throw new RuntimeException(PLUGIN_ZMQ_PREFIX+"dummy table must be as table. ");
    return new  ZmqPusher(args[0], (TableSP)args[1], heap);
}

ConstantSP zmqSetMonitor(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType() != DT_BOOL || args[0]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_ZMQ_PREFIX + "enabled must be a bool scalar.");
    ZmqStatus::SET_MONITOR = args[0]->getBool();
    return new Bool(true);
}
