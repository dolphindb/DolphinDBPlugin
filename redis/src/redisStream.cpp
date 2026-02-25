#include "redisStream.h"

#include <chrono>

#include "ddbplugin/Plugin.h"
#include "ddbplugin/PluginLogger.h"
#include "redisConnection.h"

static DictionarySP convertStreamReadReply(redisReply *reply) {
    DictionarySP result = Util::createDictionary(DT_STRING, nullptr, DT_ANY, nullptr);
    if (reply == nullptr || (reply->type != REDIS_REPLY_NIL && reply->type != REDIS_REPLY_ARRAY)) {
        throw RuntimeException("[Plugin::Redis] Failed to read message reply.");
    }
    if (reply->type == REDIS_REPLY_NIL || reply->elements == 0) return result;

    // reply->element[0] contains stream data
    redisReply *streamData = reply->element[0];
    if (streamData->type != REDIS_REPLY_ARRAY || streamData->elements < 2) {
        throw RuntimeException("[Plugin::Redis] invalid redis stream message reply.");
    }

    redisReply *messages = streamData->element[1];
    if (messages->type != REDIS_REPLY_ARRAY) {
        throw RuntimeException("[Plugin::Redis] invalid redis stream message reply.");
    }
    for (size_t i = 0; i < messages->elements; ++i) {
        redisReply *entry = messages->element[i];
        if (entry->type != REDIS_REPLY_ARRAY || entry->elements < 2) {
            throw RuntimeException("[Plugin::Redis] invalid redis stream message reply.");
        }

        string msgId(entry->element[0]->str, entry->element[0]->len);
        redisReply *fields = entry->element[1];
        if (fields->type != REDIS_REPLY_ARRAY) {
            throw RuntimeException("[Plugin::Redis] invalid redis stream message reply.");
        }

        DictionarySP fieldDict = Util::createDictionary(DT_STRING, nullptr, DT_STRING, nullptr);
        for (size_t j = 0; j + 1 < fields->elements; j += 2) {
            string field(fields->element[j]->str, fields->element[j]->len);
            string value(fields->element[j + 1]->str, fields->element[j + 1]->len);
            fieldDict->set(new String(field), new String(value));
        }
        result->set(new String(msgId), fieldDict);
    }
    return result;
}

ConstantSP RedisConnection::streamCreateGroup(const string &key, const string &group, const string &msgId,
                                              bool mkstream) {
    LockGuard<Mutex> guard(&redisMutex_);
    checkIsSub("createStreamGroup");

    // Build command: XGROUP CREATE key group msgId [MKSTREAM]
    vector<string> cmdStrs = {"XGROUP", "CREATE", key, group, msgId};
    if (mkstream) {
        cmdStrs.push_back("MKSTREAM");
    }
    RedisCommandArgs cmdArgs(cmdStrs);
    redisReply *reply =
        static_cast<redisReply *>(redisCommandArgv(redisConnect_, cmdArgs.size(), cmdArgs.argv(), cmdArgs.argvlen()));

    RedisReplyGuard replyGuard(reply);
    checkReply(reply, "XGROUP CREATE");
    return convertRedisReply(reply);
}

ConstantSP RedisConnection::streamReadGroup(const string &key, const string &group, const string &consumer, int count,
                                            bool pending, int block, bool writePending) {
    LockGuard<Mutex> guard(&redisMutex_);
    checkIsSub("readStreamGroup");

    // Build command: XREADGROUP GROUP group consumer [COUNT n] [BLOCK ms] [NOACK] STREAMS key msgId
    vector<string> cmdStrs = {"XREADGROUP", "GROUP", group, consumer};
    if (count > 0) {
        cmdStrs.push_back("COUNT");
        cmdStrs.push_back(std::to_string(count));
    }
    if (block > 0) {
        cmdStrs.push_back("BLOCK");
        cmdStrs.push_back(std::to_string(block));
    }
    if (!writePending) {
        cmdStrs.push_back("NOACK");
    }
    cmdStrs.push_back("STREAMS");
    cmdStrs.push_back(key);
    cmdStrs.push_back(pending ? "0" : ">");

    RedisCommandArgs cmdArgs(cmdStrs);
    redisReply *reply =
        static_cast<redisReply *>(redisCommandArgv(redisConnect_, cmdArgs.size(), cmdArgs.argv(), cmdArgs.argvlen()));
    RedisReplyGuard replyGuard(reply);
    checkReply(reply, "XREADGROUP");
    return convertStreamReadReply(reply);
}

ConstantSP RedisConnection::streamAck(const string &key, const string &group, const VectorSP &msgIds) {
    LockGuard<Mutex> guard(&redisMutex_);
    checkIsSub("ackMessages");
    if (msgIds->size() == 0) {
        return new Long(0);
    }

    // Build command: XACK key group msgId1 msgId2 ...
    vector<string> cmdStrs = {"XACK", key, group};
    for (int i = 0; i < msgIds->size(); i++) {
        cmdStrs.push_back(msgIds->getString(i));
    }
    RedisCommandArgs cmdArgs(cmdStrs);
    redisReply *reply =
        static_cast<redisReply *>(redisCommandArgv(redisConnect_, cmdArgs.size(), cmdArgs.argv(), cmdArgs.argvlen()));

    RedisReplyGuard replyGuard(reply);
    checkReply(reply, "XACK");
    return convertRedisReply(reply);
}

RedisCommandArgs::RedisCommandArgs(const vector<string> &strs) {
    for (const auto &s : strs) {
        strs_.push_back(s);  // copy
    }
    for (const auto &s : strs_) {
        argv_.push_back(s.c_str());
        argvlen_.push_back(s.size());
    }
}

StreamSubscription::StreamSubscription(Heap *heap, const string &host, int port, const string &key, const string &group,
                                       const string &consumer, const TableSP &output, const FunctionDefSP &handler,
                                       const string &actionName, bool pending, int batchSize, bool autoAck)
    : host_(host),
      port_(port),
      streamKey_(key),
      groupName_(group),
      consumerName_(consumer),
      output_(output),
      handler_(handler),
      actionName_(actionName),
      pending_(pending),
      batchSize_(batchSize),
      autoAck_(autoAck),
      createTime_(Util::toLocalTimestamp(Util::getEpochTime())) {
    // copy session
    session_ = heap->currentSession()->copy();
    session_->setUser(heap->currentSession()->getUser());
    session_->setOutput(new DummyOutput);
}

StreamSubscription::~StreamSubscription() {
    LOG_INFO("[Plugin::Redis] Stream subscription destructed for " + actionName_ + ".");
}

void StreamSubscription::stop() {
    isEnd_ = true;
    if (!readerThread_.isNull()) {
        readerThread_->join();
    }
    if (redisConn_) {
        redisFree(redisConn_);
        redisConn_ = nullptr;
    }
}

void StreamSubscription::connect(const string& password) {
    redisConn_ = redisConnect(host_.c_str(), port_);
    if (!redisConn_) {
        throw RuntimeException("[Plugin::Redis] Redis connection error: can't allocate redis context.");
    }
    if (redisConn_->err) {
        string errMsg = string(redisConn_->errstr);
        redisFree(redisConn_);
        redisConn_ = nullptr;
        throw RuntimeException("[Plugin::Redis] Redis connection error: " + errMsg);
    }
    if (!password.empty()) {
        string cmd = "AUTH " + password;
        redisReply *reply = static_cast<redisReply *>(redisCommand(redisConn_, cmd.c_str()));
        RedisReplyGuard replyGuard(reply);
        if (redisConn_->err) {
            throw RuntimeException("[Plugin::Redis] AUTH in subscribeStream failed: " + string(redisConn_->errstr) +
                                   CONN_ERR);
        }
        if (reply == nullptr) {
            throw RuntimeException("[Plugin::Redis] AUTH in subscribeStream failed: invalid redis reply.");
        }
        if (reply->type == REDIS_REPLY_ERROR) {
            throw RuntimeException("[Plugin::Redis] AUTH in subscribeStream failed: " + string(reply->str));
        }
    }
}

void StreamSubscription::startRead(const ThreadSP &readThread) {
    readerThread_ = readThread;
    readerThread_->start();
}

// StreamJobManager implementation
StreamJobManager *StreamJobManager::getInstance() {
    static StreamJobManager instance;
    return &instance;
}

void StreamJobManager::addSubscription(const StreamSubscriptionSP &sub) {
    if (subscriptions_.find(sub->actionName_) != subscriptions_.end()) {
        throw RuntimeException("[Plugin::Redis] Stream subscription '" + sub->actionName_ + "' already exists");
    }
    subscriptions_[sub->actionName_] = sub;
}

void StreamJobManager::removeSubscription(const string &actionName) {
    LockGuard<Mutex> lock(&mutex_);
    auto it = subscriptions_.find(actionName);
    if (it == subscriptions_.end()) {
        throw RuntimeException("[Plugin::Redis] Stream subscription '" + actionName + "' not found");
    }

    it->second->stop();
    subscriptions_.erase(it);
}

TableSP StreamJobManager::getStatusTable() {
    static const vector<string> columnNames = {
        "actionName",        "conn",   "stream",     "group",          "consumer", "createTime",
        "processedMsgCount", "status", "lastErrMsg", "lastFailedTime", "autoAck"};
    static const vector<DATA_TYPE> columnTypes = {DT_STRING, DT_STRING,    DT_STRING, DT_STRING,
                                                  DT_STRING, DT_TIMESTAMP, DT_LONG,   DT_STRING,
                                                  DT_STRING, DT_TIMESTAMP, DT_BOOL};

    vector<StreamSubscriptionSP> subs;
    int num = 0;
    {  // release global lock as soon as possible
        LockGuard<Mutex> lock(&mutex_);
        num = subscriptions_.size();
        subs.reserve(num);
        for (const auto &pair : subscriptions_) {
            subs.push_back(pair.second);
        }
    }

    TableSP table = Util::createTable(columnNames, columnTypes, num, num);
    vector<ConstantSP> columns(columnNames.size());
    for (size_t i = 0; i < columnNames.size(); ++i) {
        columns[i] = table->getColumn(i);
    }
    for (int i = 0; i < num; ++i) {
        const auto &sub = subs[i];
        columns[0]->setString(i, sub->actionName_);                               // actionName
        columns[1]->setString(i, sub->host_ + ":" + std::to_string(sub->port_));  // conn
        columns[2]->setString(i, sub->streamKey_);                                // stream
        columns[3]->setString(i, sub->groupName_);                                // group
        columns[4]->setString(i, sub->consumerName_);                             // consumer
        columns[5]->setLong(i, sub->createTime_);                                 // createTime
        columns[10]->setBool(i, sub->autoAck_);                                   // autoAck

        LockGuard<Mutex> lock(&sub->statusLock_);         // lock non const status fields
        columns[6]->setLong(i, sub->processedMsgCount_);  // processedMsgCount
        columns[7]->setString(i, sub->status_);           // status
        columns[8]->setString(i, sub->lastErrMsg_);       // lastErrMsg
        columns[9]->setLong(i, sub->lastFailedTime_);     // lastFailedTime
    }
    return table;
}

static RedisCommandArgs buildSubscribeCommand(const StreamSubscription *sub, bool readPending) {
    vector<string> cmdStrs = {"XREADGROUP", "GROUP", sub->groupName_, sub->consumerName_};
    if (!readPending) {
        cmdStrs.push_back("COUNT");
        cmdStrs.push_back(std::to_string(sub->batchSize_));
        cmdStrs.push_back("BLOCK");
        cmdStrs.push_back("100");
    }
    if (sub->autoAck_) {
        cmdStrs.push_back("NOACK");
    }
    cmdStrs.push_back("STREAMS");
    cmdStrs.push_back(sub->streamKey_);
    cmdStrs.push_back(readPending ? "0" : ">");
    return RedisCommandArgs(cmdStrs);
}

static void errorLog(StreamSubscription *sub, const string &msg) {
    LockGuard<Mutex> lock(&sub->statusLock_);
    sub->status_ = "FATAL";
    sub->lastErrMsg_ = msg;
    sub->lastFailedTime_ = Util::toLocalTimestamp(Util::getEpochTime());
    LOG_ERR(msg);
}

static void appendParserTable(const TableSP &tb, const ConstantSP &append) {
    if (append.isNull() || !append->isTable()) {
        throw RuntimeException("[Plugin::Redis] The result of parser callback function should be a table.");
    }
    vector<ConstantSP> cols = {append};

    INDEX insertedRows;
    string errMsg;
    LockGuard<Mutex> _(tb->getLock());
    tb->append(cols, insertedRows, errMsg);
    if (!errMsg.empty()) {
        throw RuntimeException("[Plugin::Redis] Append table failed: " + errMsg);
    }
}

static void ackMessages(redisContext *conn, const string &streamKey, const string &groupName, const VectorSP &msgIds) {
    int num = msgIds->size();
    if (num == 0) return;
    vector<string> cmdStrs = {"XACK", streamKey, groupName};
    for (int i = 0; i < num; ++i) {
        cmdStrs.push_back(msgIds->getString(i));
    }

    RedisCommandArgs cmdArgs(cmdStrs);
    redisReply *reply =
        static_cast<redisReply *>(redisCommandArgv(conn, cmdArgs.size(), cmdArgs.argv(), cmdArgs.argvlen()));
    if (reply == nullptr) throw RuntimeException("[Plugin::Redis] Failed to read XACK messages.");
    RedisReplyGuard guard(reply);
    if (reply->type == REDIS_REPLY_ERROR) {
        throw RuntimeException("[Plugin::Redis] XACK error: " + string(reply->str, reply->len));
    }
}

// Stream reader thread implementation
static void streamReaderThread(StreamSubscription *sub) {
    try {
        LOG_INFO("[Plugin::Redis] Starting stream reader thread for: " + sub->actionName_);
        if (sub->pending_) {  // reading pending messages first
            Heap *heap = sub->session_->getHeap().get();
            RedisCommandArgs cmdArgs = buildSubscribeCommand(sub, true);

            redisReply *reply = static_cast<redisReply *>(
                redisCommandArgv(sub->redisConn_, cmdArgs.size(), cmdArgs.argv(), cmdArgs.argvlen()));
            if (!reply) {
                return errorLog(sub, "[Plugin::Redis] Failed to read pending messages");
            }
            RedisReplyGuard guard(reply);
            if (reply->type == REDIS_REPLY_ERROR) {
                return errorLog(sub, "[Plugin::Redis] XREADGROUP error: " + string(reply->str, reply->len));
            }

            DictionarySP msgs = convertStreamReadReply(reply);
            if (msgs->size() > 0) {
                vector<ConstantSP> handlerArgs = {msgs};
                ConstantSP result = sub->handler_->call(heap, handlerArgs);
                appendParserTable(sub->output_, result);
                {
                    LockGuard<Mutex> lock(&sub->statusLock_);
                    sub->processedMsgCount_ += msgs->size();
                }
                if (sub->autoAck_) {  // XACK pending messages for NOACK not supported
                    ackMessages(sub->redisConn_, sub->streamKey_, sub->groupName_, msgs->keys());
                }
            }
        }

        while (!sub->isEnd_) {  // reading new messages from stream
            Heap *heap = sub->session_->getHeap().get();
            RedisCommandArgs cmdArgs = buildSubscribeCommand(sub, false);

            redisReply *reply = static_cast<redisReply *>(
                redisCommandArgv(sub->redisConn_, cmdArgs.size(), cmdArgs.argv(), cmdArgs.argvlen()));
            if (!reply) {
                return errorLog(sub, "[Plugin::Redis] Failed to read from stream");
            }
            RedisReplyGuard guard(reply);
            if (reply->type == REDIS_REPLY_ERROR) {
                return errorLog(sub, "[Plugin::Redis] XREADGROUP error: " + string(reply->str, reply->len));
            }

            if (reply->type == REDIS_REPLY_NIL) continue;  // NIL reply means timeout (no new messages)
            DictionarySP msgs = convertStreamReadReply(reply);
            if (msgs->size() == 0) continue;  // no new messages

            vector<ConstantSP> handlerArgs = {msgs};
            ConstantSP result = sub->handler_->call(heap, handlerArgs);
            appendParserTable(sub->output_, result);
            {
                LockGuard<Mutex> lock(&sub->statusLock_);
                sub->processedMsgCount_ += msgs->size();
            }
        }
        LOG_INFO("[Plugin::Redis] Ending stream reader thread for: " + sub->actionName_);
    } catch (const exception &e) {
        return errorLog(sub, "[Plugin::Redis] Stream reader exception: " + string(e.what()));
    }
}

ConstantSP redisStreamSubscribeImpl(Heap *heap, const string &host, int port, const string &key, const string &group,
                                    const string &consumer, const TableSP &output, const FunctionDefSP &parser,
                                    const string &actionName, bool pending, int batchSize, bool autoAck,
                                    const string &password) {
    StreamJobManager *instance = StreamJobManager::getInstance();
    LockGuard<Mutex> lock(instance->getMutex());
    StreamSubscriptionSP sub = new StreamSubscription(heap, host, port, key, group, consumer, output, parser,
                                                      actionName, pending, batchSize, autoAck);
    sub->connect(password);  // connect to redis server
    instance->addSubscription(sub);

    StreamSubscription *rawPtr = sub.get();
    sub->startRead(new Thread(
        new dolphindb::Executor([rawPtr]() { streamReaderThread(rawPtr); })));  // connect and start subscribe thread
    return new Void();
}

ConstantSP redisStreamUnsubscribeImpl(const string &actionName) {
    StreamJobManager::getInstance()->removeSubscription(actionName);
    return new Void();
}

ConstantSP redisGetStreamJobStatImpl() { return StreamJobManager::getInstance()->getStatusTable(); }
