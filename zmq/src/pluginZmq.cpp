#include"pluginZmq.h"

DictionarySP status_dict = Util::createDictionary(DT_STRING, nullptr, DT_ANY, nullptr);

template<class T>
static void connectionOnClose(Heap *heap, vector<ConstantSP> &args) {
    delete (T *) (args[0]->getLong());
}

ConstantSP zmqSocket(Heap *heap, vector<ConstantSP> &args) {
    if (args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR) {
        throw RuntimeException("socket type must be a string scalar");
    }
    if (args[1]->getType() != DT_FUNCTIONDEF) {
        throw RuntimeException("formatter must be a fuction");
    }
    int batchSize = -1;
    if (args.size() > 2) {
        if (args[2]->getType() != DT_INT || args[2]->getForm() != DF_SCALAR) {
            throw RuntimeException("batchSize must be a int scalar");
        }
        batchSize = args[2]->getInt();
        if(batchSize <= 0){
            throw RuntimeException("batchSize must be greater than 0");
        }
    }
    string prefix;
    if (args.size() > 3) {
        if (args[3]->getType() != DT_STRING || args[3]->getForm() != DF_SCALAR) {
            throw RuntimeException("prefix must be a fuction");
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
        throw IllegalArgumentException(__FUNCTION__, "handle must be a zmq connection");
    }
    ZmqPushSocket *socket = reinterpret_cast<ZmqPushSocket *> (args[0]->getLong());
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR) {
        throw RuntimeException("addr must be a string scalar");
    }
    string prefix;
    if(args.size() > 2) {
        if (args[2]->getType() != DT_STRING || args[2]->getForm() != DF_SCALAR) {
            throw RuntimeException("prefix must be a string scalar");
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
        throw RuntimeException("addr must be a string scalar");
    }
    string addr = args[1]->getString();
    string prefix;
    if(args.size() > 2) {
        if (args[2]->getType() != DT_STRING || args[2]->getForm() != DF_SCALAR) {
            throw RuntimeException("prefix must be a string scalar");
        }
        prefix = args[2]->getString();
    }
    socket->bind(addr, prefix);
    return new Bool(true);
}

ConstantSP zmqSend(Heap *heap, vector<ConstantSP> &args) {
    if (args[0]->getType() != DT_RESOURCE || args[0]->getLong() == 0 || args[0]->getString().find("zmq socket") == string::npos) {
        throw IllegalArgumentException(__FUNCTION__, "handle must be a zmq connection");
    }
    ZmqPushSocket *socket = reinterpret_cast<ZmqPushSocket *> (args[0]->getLong());
    vector<ConstantSP> fucCallArgs;
    fucCallArgs.push_back(args[0]);
    shared_ptr<zmq::socket_t> zmqSocket = socket->getSocket();
    string prefix = socket->getPrefix();
    ConstantSP data = args[1];
    int batchSize = socket->getBatchSize();
    if(data->getForm() == DF_TABLE && batchSize != -1){
        int index = 0;
        int size = data->size();
        while(index < size){
            if (args.size() > 2) {
                if (args[2]->getType() != DT_STRING || args[2]->getForm() != DF_SCALAR)
                    throw RuntimeException("prefix must be a string scalar");
                string originMessage = args[2]->getString();
                zmq::message_t message(originMessage.size());
                memcpy(message.data(), originMessage.data(), originMessage.size());
                auto rc = zmqSocket->send(message, zmq::send_flags::sndmore);
                if (!rc.has_value() || rc.value() != originMessage.size())
                    return new Bool(false);
            } else {
                if (prefix != "") {
                    zmq::message_t message(prefix.size());
                    memcpy(message.data(), prefix.data(), prefix.size());
                    auto rc = zmqSocket->send(message, zmq::send_flags::sndmore);
                    if (!rc.has_value() || rc.value() != prefix.size())
                        return new Bool(false);
                }
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
            if (!rc.has_value() || rc.value() != serialResult.size())
                return new Bool(false);
            index += batchSize;
        }
    }
    else {
        if (args.size() > 2) {
            if (args[2]->getType() != DT_STRING || args[2]->getForm() != DF_SCALAR)
                throw RuntimeException("prefix must be a string scalar");
            string originMessage = args[2]->getString();
            zmq::message_t message(originMessage.size());
            memcpy(message.data(), originMessage.data(), originMessage.size());
            auto rc = zmqSocket->send(message, zmq::send_flags::sndmore);
            if (!rc.has_value() || rc.value() != originMessage.size())
                return new Bool(false);
        } else {
            if (prefix != "") {
                zmq::message_t message(prefix.size());
                memcpy(message.data(), prefix.data(), prefix.size());
                auto rc = zmqSocket->send(message, zmq::send_flags::sndmore);
                if (!rc.has_value() || rc.value() != prefix.size())
                    return new Bool(false);
            }
        }
        vector<ConstantSP> arg;
        arg.push_back(data);
        string serialResult = socket->getFormatter()->call(heap, arg)->getString();
        zmq::message_t message(serialResult.size());
        memcpy(message.data(), serialResult.data(), serialResult.size());
        auto rc = zmqSocket->send(message, zmq::send_flags::none);
        if (!rc.has_value() || rc.value() != serialResult.size())
            return new Bool(false);
    }
    return new Bool(true);
}

ConstantSP zmqCreateSubJob(Heap *heap, vector<ConstantSP> &args) {
    if (args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR || args[0]->getString().find("zmq socket") == string::npos) {
        throw RuntimeException("addr must be a string scalar");
    }
    string addr = args[0]->getString();
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR) {
        throw RuntimeException("type must be a string scalar");
    }
    string socketType = args[1]->getString();
    if (args[2]->getType() != DT_BOOL || args[2]->getForm() != DF_SCALAR) {
        throw RuntimeException("isConnect must be a string scalar");
    }
    bool isConnect = args[2]->getBool();
    if (args[3]->getForm() != DF_TABLE && args[3]->getType() != DT_FUNCTIONDEF) {
        throw RuntimeException("handle must be a table or a fuction");
    }
    ConstantSP handle = args[3];
    if (args[4]->getType() != DT_FUNCTIONDEF) {
        throw RuntimeException("parser must be a fuction");
    }
    FunctionDefSP parser = args[4];
    string prefix;
    if (args.size() > 5) {
        if (args[5]->getType() != DT_STRING || args[5]->getForm() != DF_SCALAR) {
            throw RuntimeException("prefix must be a string scalar");
        }
        prefix = args[5]->getString();
    }
    std::unique_ptr<SubConnection> cup(
            new SubConnection(heap, make_shared<ZmqSubSocket>(addr, socketType, parser, prefix, isConnect), parser, handle));
    FunctionDefSP onClose(
            Util::createSystemProcedure("zmq sub connection onClose()", connectionOnClose<SubConnection>, 1, 1));
    ConstantSP conn = Util::createResource((long long) cup.release(), "zmq subscribe connection", onClose,
                                           heap->currentSession());
    status_dict->set(to_string(conn->getLong()), conn);
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
                        LOG_ERR("zmqPlugin: The columns of the table returned is smaller than the handler table.");
                    }
                    if (table_insert->columns() > length)
                        LOG_ERR("zmqPlugin: The columns of the table returned is larger than the handler table, and the information may be ignored.");
                    vector<ConstantSP> args = {handle_, table_insert};
                    session_->getFunctionDef("append!")->call(heap.get(), args);
                } else {
                    vector<ConstantSP> args = {parser_result};
                    ((FunctionDefSP) handle_)->call(heap.get(), args);
                }
            }
        }
        catch (exception& e){
            LOG_ERR(e.what());
        }
        first = false;
    }
    isStop_.release();
}

ConstantSP zmqClose(Heap *heap, vector<ConstantSP> &args) {
    if (args[0]->getType() != DT_RESOURCE || args[0]->getLong() == 0 ||
        args[0]->getString().find("zmq socket bind") != 0) {
        throw IllegalArgumentException(__FUNCTION__, "channel must be a zmq Socket handle");
    }
    ZmqSocket *socket = reinterpret_cast<ZmqSocket *> (args[0]->getLong());
    shared_ptr<zmq::socket_t> zmqSocket = socket->getSocket();
    zmqSocket->close();
    return new Bool(true);
}

ConstantSP zmqGetSubJobStat(Heap *heap, vector<ConstantSP> &args) {
    int size = status_dict->size();
    VectorSP connetionVec = Util::createVector(DT_STRING, size);
    VectorSP subAddrVec = Util::createVector(DT_STRING, size);
    VectorSP prefixVec = Util::createVector(DT_STRING, size);
    VectorSP recv = Util::createVector(DT_LONG, size);
    VectorSP createTimestamp = Util::createVector(DT_TIMESTAMP, size);
    VectorSP keys = status_dict->keys();
    for (int i = 0; i < size; ++i) {
        string key = keys->getString(i);
        ConstantSP conn = status_dict->getMember(key);
        SubConnection *zmqSubCon = (SubConnection *) conn->getLong();
        SmartPointer<AppendTable> appendTable = zmqSubCon->getAppendTable();
        shared_ptr<ZmqSubSocket> subSocket = appendTable->getZmqSocket();
        connetionVec->setString(i, key);
        subAddrVec->setString(i, subSocket->getAddr());
        prefixVec->setString(i, subSocket->getAddr());
        recv->setLong(i, appendTable->getRecv());
        createTimestamp->setLong(i, zmqSubCon->getCreateTime());
    }
    vector<ConstantSP> cols = {connetionVec, subAddrVec, prefixVec, recv, createTimestamp};
    vector<string> colName = {"subscriptionId", "addr", "prefix", "recvPackets", "createTimestamp"};
    return Util::createTable(colName, cols);
}

ConstantSP zmqCancelSubJob(Heap *heap, vector<ConstantSP> args) {
    std::string usage = "Usage: cancelSubJob(connection or connection ID). ";
    SubConnection *sc = nullptr;
    string key;
    ConstantSP conn = nullptr;
    auto handle = args[0];
    switch (handle->getType()) {
        case DT_RESOURCE:
            sc = (SubConnection *) (handle->getLong());
            key = std::to_string(handle->getLong());
            conn = status_dict->getMember(key);
            if (conn->isNothing())
                throw IllegalArgumentException(__FUNCTION__, "Invalid connection object.");
            break;
        case DT_STRING:
            key = handle->getString();
            conn = status_dict->getMember(key);
            if (conn->isNothing())
                throw IllegalArgumentException(__FUNCTION__, "Invalid connection string.");
            else
                sc = (SubConnection *) (conn->getLong());
            break;
        case DT_LONG:
            key = std::to_string(handle->getLong());
            conn = status_dict->getMember(key);
            if (conn->isNothing())
                throw IllegalArgumentException(__FUNCTION__, "Invalid connection integer.");
            else
                sc = (SubConnection *) (conn->getLong());
            break;
        case DT_INT:
            key = std::to_string(handle->getInt());
            conn = status_dict->getMember(key);
            if (conn->isNothing())
                throw IllegalArgumentException(__FUNCTION__, "Invalid connection integer.");
            else
                sc = (SubConnection *) (conn->getLong());
            break;
        default:
            throw IllegalArgumentException(__FUNCTION__, "Invalid connection object.");
    }
    bool bRemoved = status_dict->remove(new String(key));
    if (bRemoved && sc != nullptr) {
        sc->cancelThread();
        LOG_INFO("subscription: " + std::to_string(conn->getLong()) + " is stopped. ");
    }
    return new Void();
}

ConstantSP zmqCreatepusher(Heap *heap, vector<ConstantSP> &args){
    if (args[0]->getType() != DT_RESOURCE || args[0]->getLong() == 0 || args[0]->getString().find("zmq socket") == string::npos) {
        throw IllegalArgumentException(__FUNCTION__, "handle must be a zmq connection");
    }
    if(args[1]->getForm() != DF_TABLE)
        throw new RuntimeException("dummy table must be as table. ");
    return new  ZmqPusher(args[0], (TableSP)args[1], heap);
}