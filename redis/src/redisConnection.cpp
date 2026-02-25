#include "redisConnection.h"

#include <ctime>

#include "ddbplugin/Plugin.h"
#include "ddbplugin/PluginLogger.h"

static const int BUFFER_SIZE = 1024;

class DeferClose {
  private:
    bool &ref_;

  public:
    DeferClose(bool &flag) : ref_(flag) {}
    ~DeferClose() { ref_ = true; }
};

ConstantSP convertRedisReply(const redisReply *const reply) {
    if (reply == nullptr) {
        return new Void();
    }

    switch (reply->type) {
        case REDIS_REPLY_ERROR:
            throw RuntimeException("[Plugin::Redis] Redis reply error: " + string(reply->str));

        case REDIS_REPLY_STRING:
        case REDIS_REPLY_STATUS:
        case REDIS_REPLY_BIGNUM:
        case REDIS_REPLY_VERB:
            return new String(DolphinString(reply->str, reply->len));

        case REDIS_REPLY_NIL:
            return new Void();

        case REDIS_REPLY_INTEGER:
            return new Long(reply->integer);

        case REDIS_REPLY_DOUBLE:
            return new Double(reply->dval);

        case REDIS_REPLY_BOOL:
            return new Bool(reply->integer);

        case REDIS_REPLY_ARRAY: {
            size_t size = reply->elements;
            VectorSP vec = Util::createVector(DT_ANY, size);
            for (size_t i = 0; i < size; i++) {
                vec->set(i, convertRedisReply(reply->element[i]));
            }
            return vec;
        }

        default:
            throw RuntimeException("[Plugin::Redis] Not support this redis reply type: " + std::to_string(reply->type) +
                                   ".");
    }
}

RedisConnection::RedisConnection(redisContext *redisConnection, const string &ip, const int &port)
    : redisConnect_(redisConnection) {
    address_ = ip + ":" + std::to_string(port);

    std::time_t t = std::time(0);
    std::tm *now = std::localtime(&t);
    if (now) {
        datetime_ =
            DateTime(1900 + now->tm_year, 1 + now->tm_mon, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
    }
}

RedisConnection::~RedisConnection() {
    if (redisConnect_) {
        redisFree(redisConnect_);
    }
}

ConstantSP RedisConnection::redisRun(const vector<ConstantSP> &args, const string &command) {
    size_t sz = args.size();
    for (size_t i = 1; i < sz; i++) {
        if (args[i]->getForm() != DF_SCALAR || args[i]->getType() != DT_STRING) {
            throw IllegalArgumentException(
                __FUNCTION__, "[Plugin::Redis] argument " + std::to_string(i + 1) + " must be a string scalar.");
        }
    }
    LockGuard<Mutex> guard(&redisMutex_);
    checkIsSub("run");

    int argsLen = sz - 1;
    vector<string> strings;
    strings.reserve(argsLen);
    for (size_t i = 1; i < sz; i++) {
        strings.emplace_back(args[i]->getString());
    }

    vector<size_t> argvlen;
    argvlen.reserve(argsLen);
    vector<const char *> argv;
    argv.reserve(argsLen);
    for (size_t i = 0; i < strings.size(); i++) {
        argv.push_back(strings[i].c_str());
        argvlen.push_back(strings[i].size());
    }

    redisReply *reply = static_cast<redisReply *>(redisCommandArgv(
        redisConnect_, argsLen, static_cast<const char **>(argv.data()), static_cast<const size_t *>(argvlen.data())));
    RedisReplyGuard replyGuard(reply);
    checkReply(reply, command);
    return convertRedisReply(reply);
}

// TODO: use pipeline (redisAppendCommandArgv) to optimize it
ConstantSP RedisConnection::redisBatchSet(const vector<ConstantSP> &args) {
    if (args[1]->getForm() != DF_VECTOR || args[1]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument keys must be a string vector.");
    }
    if (args[2]->getForm() != DF_VECTOR || args[2]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument values must be a string vector.");
    }
    if (args[1]->size() != args[2]->size()) {
        throw IllegalArgumentException(__FUNCTION__,
                                       "[Plugin::Redis] Argument keys and values must have the same size.");
    }
    LockGuard<Mutex> guard(&redisMutex_);
    checkIsSub("batchSet");

    char *keysBuffer[BUFFER_SIZE];
    char *valuesBuffer[BUFFER_SIZE];
    int sz = args[1]->size();
    int rest = sz, nread = 0, read = 0;
    for (; rest > 0; nread += read, rest = sz - nread) {
        if (rest >= BUFFER_SIZE) {
            args[1]->getString(nread, BUFFER_SIZE, keysBuffer);
            args[2]->getString(nread, BUFFER_SIZE, valuesBuffer);
            read = BUFFER_SIZE;
        } else {
            args[1]->getString(nread, rest, keysBuffer);
            args[2]->getString(nread, rest, valuesBuffer);
            read = rest;
        }

        for (int i = 0; i < read; i++) {
            const char *setArgv[3] = {"SET", keysBuffer[i], valuesBuffer[i]};
            const size_t setArgvLen[3] = {3, strlen(keysBuffer[i]), strlen(valuesBuffer[i])};
            redisReply *reply = static_cast<redisReply *>(redisCommandArgv(redisConnect_, 3, setArgv, setArgvLen));
            RedisReplyGuard replyGuard(reply);
            checkReply(reply, "Set");
        }
    }
    return new String("batchSet finish.");
}

ConstantSP RedisConnection::redisBatchHashSet(const vector<ConstantSP> &args) {
    if (args[1]->getForm() != DF_VECTOR || args[1]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument idCol must be a string vector.");
    }
    if (!args[2]->isTable() || ((Table *)args[2].get())->getTableType() != BASICTBL) {
        throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument tb must be a basic table.");
    }
    if (args[1]->size() != args[2]->size()) {
        throw IllegalArgumentException(__FUNCTION__,
                                       "[Plugin::Redis] Arguments idCol and tb must have the same num of rows.");
    }
    LockGuard<Mutex> guard(&redisMutex_);
    checkIsSub("batchHashSet");

    DolphinString *idBuffer[BUFFER_SIZE];
    DolphinString *valueBuffer[BUFFER_SIZE];
    DolphinString *valueBuffer2[BUFFER_SIZE];
    DolphinString *valueBuffer3[BUFFER_SIZE];
    DolphinString *valueBuffer4[BUFFER_SIZE];
    int numCols = ((Table *)args[2].get())->columns();
    for (int colIndex = 0; colIndex < numCols;) {
        bool useBatch = numCols - colIndex >= 4;
        INDEX start = 0;
        INDEX len = args[1]->size();
        VectorSP idCol = args[1];
        while (start < len) {
            int count = std::min(len - start, BUFFER_SIZE);
            DolphinString **ids = idCol->getStringConst(start, count, idBuffer);

            if (useBatch) {
                VectorSP col = ((Table *)args[2].get())->getColumn(colIndex);
                VectorSP col2 = ((Table *)args[2].get())->getColumn(colIndex + 1);
                VectorSP col3 = ((Table *)args[2].get())->getColumn(colIndex + 2);
                VectorSP col4 = ((Table *)args[2].get())->getColumn(colIndex + 3);
                if (col->getType() != DT_STRING || col2->getType() != DT_STRING || col3->getType() != DT_STRING ||
                    col4->getType() != DT_STRING) {
                    throw RuntimeException("[Plugin::Redis] The type of field column need to be string.");
                }
                const char *fieldName = ((Table *)args[2].get())->getColumnName(colIndex).c_str();
                const char *fieldName2 = ((Table *)args[2].get())->getColumnName(colIndex + 1).c_str();
                const char *fieldName3 = ((Table *)args[2].get())->getColumnName(colIndex + 2).c_str();
                const char *fieldName4 = ((Table *)args[2].get())->getColumnName(colIndex + 3).c_str();
                DolphinString **values = col->getStringConst(start, count, valueBuffer);
                DolphinString **values2 = col2->getStringConst(start, count, valueBuffer2);
                DolphinString **values3 = col3->getStringConst(start, count, valueBuffer3);
                DolphinString **values4 = col4->getStringConst(start, count, valueBuffer4);
                for (int i = 0; i < count; ++i) {
                    vector<const char *> argv(10);
                    vector<size_t> argvlen(10);
                    argv[0] = "HSET";
                    argvlen[0] = 4;
                    argv[1] = ids[i]->c_str();
                    argvlen[1] = strlen(argv[1]);

                    argv[2] = fieldName;
                    argvlen[2] = strlen(argv[2]);
                    argv[3] = values[i]->c_str();
                    argvlen[3] = strlen(argv[3]);

                    argv[4] = fieldName2;
                    argvlen[4] = strlen(argv[4]);
                    argv[5] = values2[i]->c_str();
                    argvlen[5] = strlen(argv[5]);

                    argv[6] = fieldName3;
                    argvlen[6] = strlen(argv[6]);
                    argv[7] = values3[i]->c_str();
                    argvlen[7] = strlen(argv[7]);

                    argv[8] = fieldName4;
                    argvlen[8] = strlen(argv[8]);
                    argv[9] = values4[i]->c_str();
                    argvlen[9] = strlen(argv[9]);
                    if (redisAppendCommandArgv(redisConnect_, 10, &argv[0], &argvlen[0]) != REDIS_OK) {
                        throw RuntimeException("[Plugin::Redis] Failed to append redis command: " +
                                               string(redisConnect_->errstr) + CONN_ERR);
                    }
                }
            } else {
                VectorSP col = ((Table *)args[2].get())->getColumn(colIndex);
                if (col->getType() != DT_STRING) {
                    throw RuntimeException("[Plugin::Redis] The type of field column need to be string.");
                }
                const char *fieldName = ((Table *)args[2].get())->getColumnName(colIndex).c_str();
                DolphinString **values = col->getStringConst(start, count, valueBuffer);
                for (int i = 0; i < count; ++i) {
                    vector<const char *> argv(4);  // ["HSET", id, field, value]
                    vector<size_t> argvlen(4);
                    argv[0] = "HSET";
                    argvlen[0] = 4;
                    argv[1] = ids[i]->c_str();
                    argvlen[1] = strlen(argv[1]);

                    argv[2] = fieldName;
                    argvlen[2] = strlen(argv[2]);
                    argv[3] = values[i]->c_str();
                    argvlen[3] = strlen(argv[3]);
                    if (redisAppendCommandArgv(redisConnect_, 4, &argv[0], &argvlen[0]) != REDIS_OK) {
                        throw RuntimeException("[Plugin::Redis] Failed to append redis command: " +
                                               string(redisConnect_->errstr) + CONN_ERR);
                    }
                }
            }

            for (int i = 0; i < count; ++i) {
                redisReply *reply;
                if (redisGetReply(redisConnect_, (void **)&reply) != REDIS_OK) {
                    string connMsg = (redisConnect_->err) ? CONN_ERR : ".";
                    throw RuntimeException(
                        "[Plugin::Redis] Failed to execute HSET command: " + string(redisConnect_->errstr) + connMsg);
                }
                RedisReplyGuard replyGuard(reply);
                checkReply(reply, "HSET");
            }
            start += count;
        }
        colIndex = useBatch ? colIndex + 4 : colIndex + 1;
    }
    return new String("batchHashSet finish.");
}

static string checkArgsInBatchPush(const vector<ConstantSP> &args) {
    if (args[1]->getForm() != DF_VECTOR || args[1]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument keys must be a string vector.");
    }

    if (!args[2]->isVector()) {
        throw IllegalArgumentException(__FUNCTION__,
                                       "[Plugin::Redis] Argument values must be a two-level nested string vector.");
    }

    ConstantSP value;
    int size = args[2]->size();
    for (int i = 0; i < size; ++i) {
        value = args[2]->get(i);
        if (!value->isVector() || value->getType() != DT_STRING) {
            throw IllegalArgumentException(__FUNCTION__,
                                           "[Plugin::Redis] Argument values must be a two-level nested string vector.");
        }
        if (value->size() < 1) {
            throw IllegalArgumentException(__FUNCTION__,
                                           "[Plugin::Redis] The num of elements in values must greater than 0.");
        }
    }

    bool pushRight = true;
    if (args.size() > 3) {
        if (args[3]->getForm() != DF_SCALAR || args[3]->getType() != DT_BOOL) {
            throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument pushRight must be a bool scalar.");
        }
        pushRight = args[3]->getBool();
    }

    if (args[1]->size() != args[2]->size()) {
        throw IllegalArgumentException(__FUNCTION__,
                                       "[Plugin::Redis] Arguments keys and values must have the same num of elements.");
    }
    return pushRight ? "RPUSH" : "LPUSH";
}

ConstantSP RedisConnection::redisBatchPush(const vector<ConstantSP> &args) {
    string command = checkArgsInBatchPush(args);
    LockGuard<Mutex> guard(&redisMutex_);
    checkIsSub("batchPush");

    vector<const char *> argv(1);
    vector<size_t> argvlen(1);
    argv[0] = command.c_str();
    argvlen[0] = 5;

    DolphinString *keyBuffer[BUFFER_SIZE];
    DolphinString *valueBuffer[BUFFER_SIZE];

    INDEX len = args[1]->size();
    VectorSP keyVec = args[1];
    VectorSP outValueVec = args[2];
    INDEX start = 0;
    while (start < len) {
        int count = std::min(len - start, BUFFER_SIZE);
        DolphinString **keys = keyVec->getStringConst(start, count, keyBuffer);

        for (int i = 0; i < count; ++i) {
            VectorSP valueVec = outValueVec->get(start + i);
            int valueSize = valueVec->size();
            argv.resize(2 + valueSize);
            argvlen.resize(2 + valueSize);

            argv[1] = keys[i]->c_str();
            argvlen[1] = strlen(argv[1]);

            INDEX len2 = valueSize;
            INDEX start2 = 0;
            while (start2 < len2) {
                int count2 = std::min(len2 - start2, BUFFER_SIZE);
                DolphinString **values = valueVec->getStringConst(start2, count2, valueBuffer);

                for (int j = 0; j < count2; ++j) {
                    argv[2 + start2 + j] = values[j]->c_str();
                    argvlen[2 + start2 + j] = strlen(argv[2 + start2 + j]);
                }
                start2 += count2;
            }
            if (redisAppendCommandArgv(redisConnect_, argv.size(), &argv[0], &argvlen[0]) != REDIS_OK) {
                throw RuntimeException(
                    "[Plugin::Redis] Failed to append redis command: " + string(redisConnect_->errstr) + CONN_ERR);
            }
        }

        for (int i = 0; i < count; ++i) {
            redisReply *reply;
            if (redisGetReply(redisConnect_, (void **)&reply) != REDIS_OK) {
                string connMsg = (redisConnect_->err) ? CONN_ERR : ".";
                throw RuntimeException("[Plugin::Redis] Failed to execute " + command +
                                       " command: " + string(redisConnect_->errstr) + connMsg);
            }
            RedisReplyGuard replyGuard(reply);
            checkReply(reply, command);
        }
        start += count;
    }
    return new Void();
}

ConstantSP RedisConnection::redisBatchGet(const vector<ConstantSP> &args) {
    if (args[1]->getForm() != DF_VECTOR || args[1]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument keys must be a string vector.");
    }
    LockGuard<Mutex> guard(&redisMutex_);
    checkIsSub("batchGet");

    VectorSP keyVec = args[1];
    int len = keyVec->size();
    vector<string> valueVec(len);
    DolphinString *keyBuffer[BUFFER_SIZE];

    int start = 0;
    while (start < len) {
        int count = std::min(len - start, BUFFER_SIZE);
        vector<string> tmpStrs(count + 1);
        vector<const char *> argv(count + 1);
        vector<size_t> argvlen(count + 1);

        tmpStrs[0] = "MGET";
        argv[0] = tmpStrs[0].c_str();
        argvlen[0] = tmpStrs[0].length();
        DolphinString **keys = keyVec->getStringConst(start, count, keyBuffer);
        for (int i = 0; i < count; ++i) {
            tmpStrs[i + 1] = keys[i]->getString();
            argv[i + 1] = tmpStrs[i + 1].c_str();
            argvlen[i + 1] = tmpStrs[i + 1].length();
        }

        redisReply *reply =
            static_cast<redisReply *>(redisCommandArgv(redisConnect_, argv.size(), &argv[0], &argvlen[0]));
        RedisReplyGuard replyGuard(reply);
        checkReply(reply, "MGET");

        if (reply->type != REDIS_REPLY_ARRAY) {
            throw RuntimeException("[Plugin::Redis] The reply of mget is not array.");
        }
        if (reply->elements != static_cast<size_t>(count)) {
            throw RuntimeException("[Plugin::Redis] The num of mget reply values is not same with keys.");
        }
        for (size_t i = 0; i < reply->elements; i++) {
            redisReply *item = reply->element[i];
            if (item->type == REDIS_REPLY_STRING) {
                valueVec[start + i] = string(item->str, item->len);
            }
        }
        start += count;
    }

    VectorSP result = Util::createVector(DT_STRING, len, len);
    result->setString(0, len, valueVec.data());
    return result;
}

void RedisConnection::subscribe(Heap *heap, const vector<string> &channels, const FunctionDefSP &callback,
                                bool isPattern, const string &password) {
    LockGuard<Mutex> guard(&redisMutex_);
    if (!session_.isNull()) throw RuntimeException("[Plugin::Redis] Already invoke subscribe.");

    const timeval timeout = {2, 0};
    if (redisSetTimeout(redisConnect_, timeout) != REDIS_OK) {
        throw RuntimeException("[Plugin::Redis] Fail to set timeout: " + string(redisConnect_->errstr));
    }
    if (!password.empty()) {
        string cmd = "AUTH " + password;
        redisReply *reply = static_cast<redisReply *>(redisCommand(redisConnect_, cmd.c_str()));
        RedisReplyGuard replyGuard(reply);
        if (redisConnect_->err) {
            throw RuntimeException("[Plugin::Redis] AUTH in subscribeStream failed: " + string(redisConnect_->errstr) +
                                   CONN_ERR);
        }
        if (reply == nullptr) {
            throw RuntimeException("[Plugin::Redis] AUTH in subscribeStream failed: invalid redis reply.");
        }
        if (reply->type == REDIS_REPLY_ERROR) {
            throw RuntimeException("[Plugin::Redis] AUTH in subscribeStream failed: " + string(reply->str));
        }
    }

    isPattern_ = isPattern;
    password_ = password;
    buildSubCommand(channels, true);
    if (redisAppendCommand(redisConnect_, channelStr_.c_str()) != REDIS_OK) {
        throw RuntimeException("[Plugin::Redis] Failed to subscribe: " + string(redisConnect_->errstr) + CONN_ERR);
    }
    string errMsg = checkSubReply(numChannels_);
    if (!errMsg.empty()) throw RuntimeException(errMsg);

    callback_ = callback;
    initSubThreads(heap);
}

void RedisConnection::unsubscribe(const vector<string> &channels) {
    LockGuard<Mutex> guard(&redisMutex_);
    if (session_.isNull()) throw RuntimeException("[Plugin::Redis] Haven't subscribe any channel yet.");
    {
        LockGuard<Mutex> channelGuard(&channelMutex_);
        buildSubCommand(channels, false);
        isChanged_ = true;
    }

    for (int i = 0; i < 50; ++i) {
        Util::sleep(200);  // 200 ms
        LockGuard<Mutex> channelGuard(&channelMutex_);
        if (!isChanged_ || isClosed_) return;
    }
    throw RuntimeException("[Plugin::Redis] Unsubscribe timeout.");
    isClosed_ = true;
}

string RedisConnection::getSubscriptions() {
    LockGuard<Mutex> channelGuard(&channelMutex_);
    string subscriptions;
    for (const auto &chan : channels_) {
        subscriptions += chan + " ";
    }
    return subscriptions;
}

void RedisConnection::stopListen() {
    LockGuard<Mutex> guard(&redisMutex_);
    isClosed_ = true;
    if (!listener_.isNull()) {
        listener_->join();
    }
    if (!handler_.isNull()) {
        handler_->join();
    }
}

void RedisConnection::checkReply(const redisReply *reply, const string &command) {
    if (redisConnect_->err) {
        throw RuntimeException("[Plugin::Redis] Execute command failed: " + string(redisConnect_->errstr) + CONN_ERR);
    }
    if (reply == nullptr) {
        throw RuntimeException("[Plugin::Redis] Invalid redis reply.");
    }
    if (reply->type == REDIS_REPLY_ERROR) {
        throw RuntimeException("[Plugin::Redis] " + command + " failed: " + string(reply->str));
    }
}

void RedisConnection::checkIsSub(const string &funcName) const {
    if (!session_.isNull())
        throw RuntimeException("[Plugin::Redis] Can't execute " + funcName + " interface in subscribe mode.");
}

string RedisConnection::checkSubReplyHelper(const redisReply *reply) const {
    string errorMsg;
    if (redisConnect_->err) {
        errorMsg = string(redisConnect_->errstr) + CONN_ERR;
    } else if (reply == nullptr) {
        errorMsg = "invalid redis reply.";
    } else if (reply->type == REDIS_REPLY_ERROR) {
        errorMsg = string(reply->str);
    }

    if (!errorMsg.empty()) {
        errorMsg = "[Plugin::Redis] Subscribe failed. (subscribe fail: " + errorMsg + ")";
    } else if (reply->type != REDIS_REPLY_ARRAY || (reply->elements < 3 || reply->elements > 4)) {
        errorMsg = "[Plugin::Redis] Invalid subscribe message reply.";
    }
    return errorMsg;
}

string RedisConnection::checkSubReply(int numChannels) const {
    redisReply *reply;
    for (int i = 0; i < numChannels; ++i) {
        if (redisGetReply(redisConnect_, (void **)&reply) != REDIS_OK) {
            return "[Plugin::Redis] Failed to subscribe: " + string(redisConnect_->errstr) + CONN_ERR;
        }

        RedisReplyGuard replyGuard(reply);
        string errorMsg = checkSubReplyHelper(reply);
        if (!errorMsg.empty()) {
            return errorMsg;
        }

        string head = reply->element[0]->str;
        string expectHead = (isPattern_) ? "psubscribe" : "subscribe";
        if (head != expectHead) {
            return "[Plugin::Redis] The message head is not '" + expectHead + "', but " + head + ".";
        }
    }
    return "";
}

void RedisConnection::initSubThreads(Heap *heap) {
    // copy session
    session_ = heap->currentSession()->copy();
    session_->setUser(heap->currentSession()->getUser());
    session_->setOutput(new DummyOutput);

    // start listener thread
    listener_ = new Thread(new dolphindb::Executor([this]() {
        DeferClose _(isClosed_);
        try {
            const timeval timeout = {2, 0};
            const int chanIdx = (isPattern_) ? 2 : 1;
            const int msgIdx = (isPattern_) ? 3 : 2;
            const string expectHead = (isPattern_) ? "pmessage" : "message";
            RedisMsg msg;
            redisReply *reply;
            while (!isClosed_) {
                if (redisGetReply(redisConnect_, (void **)&reply) != REDIS_OK) {
#ifdef __linux__
                    if (redisConnect_->err != REDIS_ERR_IO || errno != EAGAIN) {
#else
                    if (redisConnect_->err != REDIS_ERR_TIMEOUT || errno != ETIMEDOUT) {
#endif
                        LOG_ERR("[Plugin::Redis] Failed to receive message: " + string(redisConnect_->errstr) +
                                       CONN_ERR + "(errno: " + std::to_string(errno) + ")");
                    } else if (reconnect(timeout)) {
                        continue;
                    }
                    break;
                }

                if (isClosed_) {
                    break;
                }
                RedisReplyGuard replyGuard(reply);
                string errMsg = checkSubReplyHelper(reply);
                if (!errMsg.empty()) {
                    LOG_ERR(errMsg);
                    break;
                }
                string head = reply->element[0]->str;
                if (head != expectHead) {
                    LOG_ERR("[Plugin::Redis] The message head is not '" + expectHead + "', but " + head + ".");
                    break;
                }
                msg.channel_ = reply->element[chanIdx]->str;
                msg.msg_ = reply->element[msgIdx]->str;

                LockGuard<Mutex> channelGuard(&channelMutex_);
                if (isChanged_) {
                    if (channels_.find(msg.channel_) != channels_.end()) {
                        msgQueue_.push(msg);
                    }
                    if (!reconnect(timeout)) {
                        break;
                    }
                } else {
                    msgQueue_.push(msg);
                }
            }
            LOG_INFO("[Plugin::Redis] Lisitener thread closed.");
        } catch (std::exception &ex) {
            LOG_ERR("[Plugin::Redis] catch exception in listener thread. " + string(ex.what()));
        } catch (...) {
            LOG_ERR("[Plugin::Redis] catch unknown exception in listener thread.");
        }
    }));
    listener_->start();

    // start handler thread
    handler_ = new Thread(new dolphindb::Executor([this]() {
        DeferClose _(isClosed_);
        try {
            const int QUEUE_MSG_BATCH_SIZE = 1024;
            Heap *heap = session_->getHeap().get();
            vector<ConstantSP> args = {new String(), new String()};

            vector<RedisMsg> msgVec;
            msgVec.reserve(QUEUE_MSG_BATCH_SIZE);
            while (!isClosed_) {
                // get msg from queue
                msgVec.clear();
                msgQueue_.blockingPop(msgVec, QUEUE_MSG_BATCH_SIZE, 100);  // timeout 100 ms

                // handle callback
                for (const auto &msg : msgVec) {
                    args[0]->setString(msg.channel_);
                    args[1]->setString(msg.msg_);
                    callback_->call(heap, args);
                }
            }
            LOG_INFO("[Plugin::Redis] Hanlder thread closed.");
        } catch (std::exception &ex) {
            LOG_ERR("[Plugin::Redis] catch exception in handler thread. " + string(ex.what()));
        } catch (...) {
            LOG_ERR("[Plugin::Redis] catch unknown exception in handler thread.");
        }
    }));
    handler_->start();
}

bool RedisConnection::reconnect(const timeval &timeout) {
    if (redisReconnect(redisConnect_) != REDIS_OK || redisSetTimeout(redisConnect_, timeout) != REDIS_OK) {
        LOG_ERR("[Plugin::Redis] Reconnect failed: " + string(redisConnect_->errstr));
        return false;
    }
    if (!password_.empty()) {
        string cmd = "AUTH " + password_;
        redisReply *reply = static_cast<redisReply *>(redisCommand(redisConnect_, cmd.c_str()));
        RedisReplyGuard replyGuard(reply);
        if (redisConnect_->err) {
            LOG_ERR("[Plugin::Redis] AUTH in subscribe failed: " + string(redisConnect_->errstr) + CONN_ERR);
            return false;
        }
        if (reply == nullptr) {
            LOG_ERR("[Plugin::Redis] AUTH in subscribe failed: invalid redis reply.");
            return false;
        }
        if (reply->type == REDIS_REPLY_ERROR) {
            LOG_ERR("[Plugin::Redis] AUTH in subscribe failed: " + string(reply->str));
            return false;
        }
    }

    LockGuard<Mutex> channelGuard(&channelMutex_);
    if (!numChannels_) {
        LOG_ERR("[Plugin::Redis] There's no channel to subscribe.");
        return false;
    }
    if (redisAppendCommand(redisConnect_, channelStr_.c_str()) != REDIS_OK) {
        LOG_ERR("[Plugin::Redis] Failed to subscribe: " + string(redisConnect_->errstr) + CONN_ERR);
        return false;
    }
    string errMsg = checkSubReply(numChannels_);
    if (!errMsg.empty()) {
        LOG_ERR(errMsg);
        return false;
    }

    isChanged_ = false;
    return true;
}

static bool isInvalidChannel(const string &channel) {
    for (const auto &ch : channel) {
        if (ch != ' ') return false;
    }
    return true;
}

void RedisConnection::buildSubCommand(const vector<string> &channels, bool isSub) {
    for (const auto &chan : channels) {
        if (isSub) {
            if (isInvalidChannel(chan)) {
                throw RuntimeException("[Plugin::Redis] Invalid chanel: " + chan + ".");
            }
            channels_.insert(chan);
        } else {
            if (!channels_.count(chan)) {
                throw RuntimeException("[Plugin::Redis] There's no such channel or pattern: " + chan + ".");
            }
            channels_.erase(chan);
        }
    }

    string command = (isPattern_) ? "PSUBSCRIBE" : "SUBSCRIBE";
    for (const auto &chan : channels_) {
        command += " " + chan;
    }
    numChannels_ = channels_.size();
    channelStr_ = command;
}
