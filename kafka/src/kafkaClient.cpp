#include "kafkaClient.h"
#include "Exceptions.h"
#include "ddbplugin/PluginLogger.h"

using std::map;

MetaTable mockMetaTable = {{"payload", "key", "topic", "timestamp"},
                           {DT_STRING, DT_STRING, DT_STRING, DT_TIMESTAMP}};

long long getMsgTimestamp(const rd_kafka_message_t *msg, long long localTimeGap) {
    if (msg == nullptr) {
        return LONG_LONG_MIN;
    }
    rd_kafka_timestamp_type_t timestampType = RD_KAFKA_TIMESTAMP_NOT_AVAILABLE;
    int64_t timestamp = rd_kafka_message_timestamp(msg, &timestampType);
    if (timestampType == RD_KAFKA_TIMESTAMP_NOT_AVAILABLE || timestamp < 0) {
        return LONG_LONG_MIN;
    }
    return timestamp + localTimeGap;
}

void commitMsg(const rawMessageWrapperSP &msg, const SmartPointer<Consumer> &consumer) {
    rd_kafka_resp_err_t error;
    error = rd_kafka_commit_message(consumer->get_handle(), msg->msgPtr_, 0);
    if (error != RD_KAFKA_RESP_ERR_NO_ERROR) {
        throw RuntimeException(rd_kafka_err2str(error));
    }
};

void subJobCallBack(vector<ConstantSP> &buffer, MessageWrapper &data, long long localTimeGap) {
    int colNum = 0;
    rd_kafka_message_t *msg = data.rawMessage_->msgPtr_;
    string payload;
    if (msg->payload != nullptr) {
        payload = string(static_cast<char *>(msg->payload), msg->len);
    }
    string key;
    if (msg->key != nullptr) {
        key = string(static_cast<char *>(msg->key), msg->key_len);
    }
    string topic(msg->rkt != nullptr ? rd_kafka_topic_name(msg->rkt) : "");
    long long timestamp = getMsgTimestamp(msg, localTimeGap/1000000);
    (VectorSP(buffer[colNum++]))->appendString(&payload, 1);
    (VectorSP(buffer[colNum++]))->appendString(&key, 1);
    (VectorSP(buffer[colNum++]))->appendString(&topic, 1);
    (VectorSP(buffer[colNum++]))->appendLong(&timestamp, 1);
}

void subJobFinalizer(vector<MessageWrapper> &msgs, const SmartPointer<Consumer> &consumer) {
    if (!msgs.empty()) {
        commitMsg(msgs.back().rawMessage_, consumer);
    }
}

void subJobTransform(Heap *heap, vector<ConstantSP> &args) {
    FunctionDefSP parser = args[0];
    FunctionDefSP handler = args[1];
    vector<ConstantSP> parserArgs{args[2]};
    ConstantSP parseResult = parser->call(heap, parserArgs);
    vector<ConstantSP> handleArgs{parseResult};
    handler->call(heap, handleArgs);
}

AppendTable::AppendTable(Heap *heap, ConstantSP parser, ConstantSP handle, ConstantSP consumer,
                         const string &actionName, KafkaUtil::SubJobAutoCommit autoCommit, bool msgAsTable,
                         long long batchSize, long long throttle, long long queueDepth)
    : autoCommit_(false),
      msgAsTable_(msgAsTable),
      batchSize_(batchSize),
      throttle_(throttle),
      actionName_(actionName),
      parser_(parser),
      handle_(handle),
      consumerWrapper_(consumer) {
    consumer_ = (DdbKafkaConsumerSP(consumerWrapper_))->getConsumer();
    timeout_ = consumer_->get_timeout().count();  // use consumer default timeout

    status_.lastErrMsg_.reserve(100); // in case of string oom

    session_ = heap->currentSession()->copy();
    session_->setUser(heap->currentSession()->getUser());
    session_->setOutput(heap->currentSession()->getOutput());

    long long nanoTimestamp = Util::getNanoEpochTime();
    localTimeGap_ = Util::toLocalNanoTimestamp(nanoTimestamp) - nanoTimestamp;

    map<string, string> configuration = consumer_->get_configuration().get_all();
    auto it = configuration.find("enable.auto.commit");
    if (it != configuration.end() && it->second == "false") {
        if (autoCommit != NOT_COMMIT) {  // if not deliberately set to false, auto commit after processing
            autoCommit_ = true;
        }
    } else if (autoCommit == KafkaUtil::COMMIT) {
        autoCommit_ = true;
    }

    if (msgAsTable_ && parser_->getType() == DT_FUNCTIONDEF) {
        queue_ = new ThreadedQueue<MessageWrapper>(
            session_->getHeap().get(), throttle, queueDepth, mockMetaTable, nullptr, 0, actionName, KAFKA_PREFIX,
            batchSize,
            [this](vector<ConstantSP> &buffer, MessageWrapper &data) { subJobCallBack(buffer, data, localTimeGap_); });
        queue_->setTimeoutAsThrottle(true);
        if (autoCommit_) {
            queue_->setFinalizer([&](vector<MessageWrapper> &msgs) { subJobFinalizer(msgs, consumer_); });
        }
        if (handle->getForm() == DF_TABLE) {
            queue_->setTransform(parser_);
            queue_->setTable(handle);
        } else {  // could only be FunctionDef
            FunctionDefSP trans(Util::createSystemProcedure("subJobTransform" + actionName, subJobTransform, 3, 3));
            vector<ConstantSP> args{parser, handle};
            FunctionDefSP partTrans = Util::createPartialFunction(trans, args);
            queue_->setTransform(partTrans);
            queue_->ignoreTableInsert();
        }
        queue_->start();
    } else {
        if (parser_->getType() == DT_FUNCTIONDEF) {
            int paramCount = ((FunctionDefSP)parser_)->getParamCount();
            for (int i = 0; i < paramCount; ++i) {
                if (i == 3) {
                    parserArgs_.emplace_back(Util::createConstant(DT_TIMESTAMP));
                } else {
                    parserArgs_.emplace_back(Util::createConstant(DT_STRING));
                }
            }
        } else {
            parserArgs_.emplace_back(Util::createConstant(DT_STRING));
        }
    }
}

const StreamStatus &AppendTable::getStatus() const {
    if (msgAsTable_ && parser_->getType() == DT_FUNCTIONDEF) {
        return queue_->getStatusConst();
    } else {
        return status_;
    }
}

TableSP AppendTable::doParse(const rawMessageWrapperSP &msg) {
    auto* message = msg->msgPtr_;
    char* payload = static_cast<char*>(message->payload);
    char* key = static_cast<char*>(message->key);

    parserArgs_[0]->setString(DolphinString(payload != nullptr ? payload : "", message->len));
    int paramCount = (FunctionDefSP(parser_))->getParamCount();
    if (paramCount >= 2) {
        parserArgs_[1]->setString(DolphinString(key != nullptr ? key : "", msg->msgPtr_->key_len));
    }
    if (paramCount >= 3) {
        parserArgs_[2]->setString(message->rkt != nullptr ? rd_kafka_topic_name(message->rkt) : "");
    }
    if (paramCount >= 4) {
        long long timestamp = getMsgTimestamp(message, localTimeGap_/1000000);
        if (timestamp == LONG_LONG_MIN) {
            parserArgs_[3]->setNull();
        } else {
            parserArgs_[3]->setLong(timestamp);
        }
    }
    ConstantSP parseResult = (FunctionDefSP(parser_))->call(session_->getHeap().get(), parserArgs_);
    if (UNLIKELY(!parseResult->isTable())) {
        throw RuntimeException("The parser should return a table.");
    }
    return parseResult;
}

void AppendTable::doHandle(TableSP tableInsert) {
    if (LIKELY(handle_->isTable())) {
        TableSP result = handle_;
        // NOTE columns verification, remove now
        if (UNLIKELY(result->isSegmentedTable())) {
            vector<ConstantSP> args = {result, tableInsert};
            session_->getFunctionDef("append!")->call(session_->getHeap().get(), args);
        } else {
            INDEX insertedRows = tableInsert->size();
            string errMsg;
            vector<ConstantSP> args = {tableInsert};
            LockGuard<Mutex> _(result->getLock());
            bool add = result->append(args, insertedRows, errMsg);
            if (!add) {
                throw RuntimeException(errMsg);
            }
        }
    } else {
        vector<ConstantSP> args = {tableInsert};
        ((FunctionDefSP)handle_)->call(session_->getHeap().get(), args);
    }
}

void AppendTable::handleErr(const string &errMsg) {
    status_.failedMsgCount_ += 1;
    status_.lastFailedTimestamp_ = Util::getNanoEpochTime() + localTimeGap_;
    try {
        status_.lastErrMsg_ = "topic=" + actionName_ + " length=1 exception=" + errMsg;
    } catch (...) {
        status_.lastErrMsg_ = "topic=unknown length=1 exception=unknown";
    }
    LOG_ERR(KAFKA_PREFIX, status_.lastErrMsg_);
}

void AppendTable::run() {
    while (LIKELY(flag_)) {
        try {
            rawMessageWrapperSP msg = new rawMessageWrapper(nullptr);
            auto *msgPtr = rd_kafka_consumer_poll(consumer_->get_handle(), timeout_);
            if (UNLIKELY(!msgPtr)) {
                continue;
            }
            msg->msgPtr_ = msgPtr;
            if (UNLIKELY(msg->msgPtr_->err)) {
                LOG(KAFKA_PREFIX, "topic=", actionName_, " polls msg failed: ", rd_kafka_err2str(msg->msgPtr_->err));
                continue;
            }
            status_.processedMsgCount_ += 1;
            if (LIKELY(parser_->getType() == DT_FUNCTIONDEF)) {
                if (!msgAsTable_) {
                    doHandle(doParse(msg));
                    if (autoCommit_) {
                        commitMsg(msg, consumer_);
                    }
                } else {
                    queue_->push(MessageWrapper(msg, LONG_LONG_MIN));
                }
            } else {  // only could be coder instance, cannot autoCommit
                parserArgs_[0]->setString(DolphinString((char *)msg->msgPtr_->payload, msg->msgPtr_->len));
                vector<ConstantSP> tableArgs = {parserArgs_[0]};
                ConstantSP content = Util::createTable({"string"}, tableArgs);
                string name = "parseAndHandle";
                vector<ConstantSP> args = {content};
                parser_->callMethod(name, session_->getHeap().get(), args);
            }
        } catch (std::exception &e) {
            handleErr(e.what());
        } catch (...) {
            handleErr("unknown");
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

SubConnection::SubConnection(Heap *heap, ConstantSP consumerWrapper, ConstantSP handler, ConstantSP parser,
                             const string &actionName, SubJobAutoCommit autoCommit, bool msgAsTable,
                             long long batchSize, long long throttle, long long queueDepth)
    : actionName_(actionName), heap_(heap), consumerWrapper_(consumerWrapper) {
    connected_ = true;
    createTime_ = Util::toLocalTimestamp(Util::getEpochTime());
    session_ = heap->currentSession()->copy();
    session_->setUser(heap->currentSession()->getUser());

    append_ = new AppendTable(heap, parser, handler, consumerWrapper, actionName, autoCommit, msgAsTable, batchSize,
                              throttle, queueDepth);
    thread_ = new Thread(append_);
    if (!thread_->isStarted()) {
        thread_->start();
    }
}

SubConnection::~SubConnection() {
    if (!thread_.isNull() && thread_->isRunning()) {
        cancelThread();
    }
    if (connected_) {
        connected_ = false;
    }
    LOG_INFO(KAFKA_PREFIX, "subJob: " + actionName_ + " is freed");
}
