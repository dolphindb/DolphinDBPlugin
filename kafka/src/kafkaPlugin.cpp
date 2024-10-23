#include "kafkaPlugin.h"

#include "CoreConcept.h"
#include "Exceptions.h"
#include "ddbplugin/CommonInterface.h"
#include "ddbplugin/Plugin.h"
#include "ddbplugin/ThreadedQueue.h"
#include "kafkaWrapper.h"
#include "librdkafka/rdkafkacpp.h"

using namespace KafkaUtil;
using namespace cppkafka;

Mutex DICT_LATCH;
dolphindb::BackgroundResourceMap<SubConnection> SUB_CONN_HANDLE_MAP(KAFKA_PREFIX, SUB_JOB_DESC);
// use atomic timestamp to indicate the last assign time.
// ddb would crash if the internal between assign() & unassign() is too short
std::atomic<int64_t> ASSIGN_TIMEOUT;
// concurrently using assign related function would cause crash, use a mutex to avoid crash
// alert! It's a little inefficient, but assign operation would not be called frequently
Mutex ASSIGN_LATCH;

static void kafkaSubConnectionOnClose(Heap *heap, vector<ConstantSP> &args) {
    SUB_CONN_HANDLE_MAP.safeRemoveWithoutException(args[0]);
}

KafkaMarshalType extractMarshalType(const string &typeString, const string &funcName, const string &usage) {
    string marshalString = Util::upper(typeString);
    if (marshalMap.find(marshalString) != marshalMap.end()) {
        return marshalMap.at(marshalString);
    } else {
        throw IllegalArgumentException(funcName, usage + "marshalType must be one of 'PLAIN', 'JSON', 'DOLPHINDB'.");
    }
}

////////////////////////////////////////////////////////////////////////////////

ConstantSP kafkaProducer(Heap *heap, vector<ConstantSP> &args) {
    string usage = string("kafka::producer(dict, [errCallback]) ");
    auto &dict = args[0];
    if (dict->getForm() != DF_DICTIONARY) {
        throw IllegalArgumentException(__FUNCTION__, usage + "dict must be a dict config.");
    }
    Configuration conf;
    if (args.size() > 1 && !args[1].isNull()) {
        if (args[1]->getType() != DT_FUNCTIONDEF) {
            throw IllegalArgumentException(__FUNCTION__, usage + "errCallback must be a DolphinDB function");
        }
        FunctionDefSP func = args[1];
        if (func->getParamCount() != 3) {
            throw IllegalArgumentException(__FUNCTION__,
                                           usage + "errCallback must be a DolphinDB function with three string params");
        }
        conf = createConf(dict, "producer", false, heap, func);
    } else {
        conf = createConf(dict, "producer");
    }
    try {
        return new DdbKafkaProducer(heap, conf);
    } catch (std::exception &exception) {
        throw RuntimeException(KAFKA_PREFIX + exception.what());
    }
}

ConstantSP kafkaProducerFlush(Heap *heap, vector<ConstantSP> &args) {
    string usage = string("kafka::produceFlush(producer) ");
    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != PRODUCER_DESC)
        throw IllegalArgumentException(__FUNCTION__, usage + "producer should be a producer handle.");
    try {
        ((DdbKafkaProducerSP)args[0])->getProducer()->flush();
    } catch (std::exception &exception) {
        throw RuntimeException(KAFKA_PREFIX + exception.what());
    }
    return new Void();
}

ConstantSP kafkaProduce(Heap *heap, vector<ConstantSP> &args) {
    string usage{"kafka::produce(producer, topic, key, value, marshalType, [partition]);"};
    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != PRODUCER_DESC) {
        throw IllegalArgumentException(__FUNCTION__, usage + "producer should be a producer handle.");
    }
    SmartPointer<Producer> producer = ((DdbKafkaProducerSP)args[0])->getProducer();

    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "topic must be a string scalar.");
    }
    string topic = args[1]->getString();
    // if (args[2]->getForm() != DF_SCALAR) { // NOTE compatible with regression
    //     throw IllegalArgumentException(__FUNCTION__, usage + "key must be a string scalar.");
    // }
    string key = args[2]->getString();
    ConstantSP value = args[3];
    KafkaMarshalType marshalType = KafkaUtil::DOLPHINDB;
    if (args[4]->getType() != DT_STRING || args[4]->getForm() != DF_SCALAR) {
        if (args[4]->getType() != DT_BOOL || args[4]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, usage + "marshalType must be a string scalar.");
        }
        marshalType = KafkaMarshalType(args[4]->getBool());
    } else {
        marshalType = extractMarshalType(args[4]->getString(), __FUNCTION__, usage);
    }

    long long partition = -1;
    if (args.size() > 5 && !args[5].isNull()) {
        if (args[5]->getCategory() != INTEGRAL || args[5]->getForm() != DF_SCALAR || args[5]->getLong() < 0) {
            throw IllegalArgumentException(__FUNCTION__, usage + "partition must be a non-negative integer scalar.");
        }
        partition = args[5]->getLong();
    }
    produceMsg(producer, topic, key, value, marshalType, partition);
    return new Void();
}

////////////////////////////////////////////////////////////////////////////////

ConstantSP kafkaConsumer(Heap *heap, vector<ConstantSP> &args) {
    string usage = string("kafka::consumer(dict[string, string]) ");
    auto &dict = args[0];

    if (dict->getForm() != DF_DICTIONARY) {
        throw IllegalArgumentException(__FUNCTION__, usage + "Not a dict config.");
    }
    Configuration conf;
    if (args.size() > 1 && !args[1].isNull()) {
        if (args[1]->getType() != DT_FUNCTIONDEF) {
            throw IllegalArgumentException(__FUNCTION__, usage + "errCallback must be a DolphinDB function");
        }
        FunctionDefSP func = args[1];
        if (func->getParamCount() != 3) {
            throw IllegalArgumentException(__FUNCTION__,
                                           usage + "errCallback must be a DolphinDB function with three string params");
        }
        conf = createConf(dict, "consumer", true, heap, func);
    } else {
        conf = createConf(dict, "consumer", true);
    }

    try {
        DdbKafkaConsumerSP ret = new DdbKafkaConsumer(heap, conf);
        auto consumer = ret->getConsumer();
        consumer->set_assignment_callback([=](TopicPartitionList &list) {
            for (auto l : list) {
                LOG_INFO(KAFKA_PREFIX, consumer->get_member_id(), " assignment of topic:", l.get_topic(),
                         "; partition:", l.get_partition(), "; offset:", l.get_offset());
            }
        });
        consumer->set_revocation_callback([=](const TopicPartitionList &list) {
            for (auto l : list) {
                LOG_INFO(KAFKA_PREFIX, consumer->get_member_id(), " revocation of topic:", l.get_topic(),
                         "; partition:", l.get_partition(), "; offset:", l.get_offset());
            }
        });
        consumer->set_rebalance_error_callback(
            [](Error e) { LOG_ERR(KAFKA_PREFIX, " rebalance error:", e.to_string()); });
        return ret;
    } catch (std::exception &exception) {
        throw RuntimeException(KAFKA_PREFIX + exception.what());
    }
}

ConstantSP kafkaSubscribe(Heap *heap, vector<ConstantSP> &args) {
    string usage = string("kafka::subscribe(consumer, topics) ");
    SmartPointer<Consumer> conn = extractConsumer(args[0], __FUNCTION__, usage);
    if ((args[1]->getForm() != DF_VECTOR && args[1]->getForm() != DF_SCALAR) || args[1]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, usage + "Not a topic vector.");
    }
    vector<string> topics;
    if (args[1]->isScalar()) {
        topics.push_back(args[1]->getString());
    } else {
        auto &vec = args[1];
        for (auto i = 0; i < vec->size(); i++) {
            topics.push_back(vec->get(i)->getString());
        }
    }
    try {
        conn->subscribe(topics);
    } catch (std::exception &exception) {
        throw RuntimeException(KAFKA_PREFIX + exception.what());
    }
    return new Void();
}

ConstantSP kafkaConsumerPoll(Heap *heap, vector<ConstantSP> &args) {
    string usage{"kafka::consumerPoll(consumer, [timeout=1000], [marshalType])"};
    SmartPointer<Consumer> consumer = extractConsumer(args[0], __FUNCTION__, usage);
    try {
        KafkaMarshalType marshalType = KafkaMarshalType::AUTO;
        if (args.size() > 2 && !args[2]->isNull()) {
            if (args[2]->getForm() != DF_SCALAR || args[2]->getType() != DT_STRING) {
                throw IllegalArgumentException(__FUNCTION__, usage + "marshalType must be a string scalar.");
            }
            marshalType = extractMarshalType(args[2]->getString(), __FUNCTION__, usage);
        }

        // Try to consume a message first
        if (args.size() >= 2 && !args[1]->isNull()) {
            if (args[1]->getType() < DT_SHORT || args[1]->getType() > DT_LONG || args[1]->getInt() < 0) {
                throw IllegalArgumentException(__FUNCTION__, +"time should be a positive integer");
            }
            auto time = args[1]->getInt();
            auto msg = consumer->poll(std::chrono::milliseconds(time));

            // DPLG-275
            for (int i = 0; i < 10; ++i) {
                if (msg && msg.get_error() && msg.is_eof()) {
                    msg = consumer->poll(std::chrono::milliseconds(time));
                } else {
                    break;
                }
            }
            return getMsg(msg, marshalType);
        } else {
            auto msg = consumer->poll();
            if (msg) {
                // TODO error msg
            }
            // DPLG-275
            for (int i = 0; i < 10; ++i) {
                if (msg && msg.get_error() && msg.is_eof()) {
                    msg = consumer->poll();
                } else {
                    break;
                }
            }
            return getMsg(msg, marshalType);
        }
    } catch (std::exception &exception) {
        throw RuntimeException(KAFKA_PREFIX + exception.what());
    }
}

ConstantSP kafkaConsumerPollBatch(Heap *heap, vector<ConstantSP> &args) {
    string usage{"consumerPollBatch(consumer, batchSize, [timeout], [marshalType]) "};
    SmartPointer<Consumer> consumer = extractConsumer(args[0], __FUNCTION__, usage);
    if (args[1]->getType() < DT_SHORT || args[1]->getType() > DT_LONG || args[1]->getInt() < 0) {
        throw IllegalArgumentException(__FUNCTION__, usage + "batchSize need positive integer");
    }
    auto batchSize = args[1]->getInt();
    VectorSP result = Util::createVector(DT_ANY, 0);

    KafkaMarshalType marshalType = KafkaMarshalType::AUTO;
    if (args.size() > 3 && !args[3]->isNull()) {
        if (args[3]->getForm() != DF_SCALAR || args[3]->getType() != DT_STRING) {
            throw IllegalArgumentException(__FUNCTION__, usage + "marshalType must be a string scalar.");
        }
        marshalType = extractMarshalType(args[3]->getString(), __FUNCTION__, usage);
    }
    vector<Message> msgs;
    try {
        if (args.size() >= 3) {
            if (args[2]->getType() < DT_SHORT || args[2]->getType() > DT_LONG || args[2]->getInt() < 0) {
                throw IllegalArgumentException(__FUNCTION__, +"time need positive integer");
            }
            auto time = args[2]->getInt();

            msgs = consumer->poll_batch(batchSize, std::chrono::milliseconds(time));

            // remove RD_KAFKA_RESP_ERR__PARTITION_EOF case manually
            int length = 0;
            while (length < batchSize) {
                if (msgs.size() == 0) {
                    break;
                }
                for (auto &msg : msgs) {
                    if (msg && msg.get_error() && msg.is_eof()) {
                        continue;
                    }
                    result->append(getMsg(msg, marshalType));
                }
                length = result->size();
                msgs = consumer->poll_batch(batchSize - length, std::chrono::milliseconds(time));
            }
        } else {
            // remove RD_KAFKA_RESP_ERR__PARTITION_EOF case manually
            msgs = consumer->poll_batch(batchSize);
            int length = 0;
            while (length < batchSize) {
                if (msgs.size() == 0) {
                    break;
                }
                for (auto &msg : msgs) {
                    if (msg && msg.get_error() && msg.is_eof()) {
                        continue;
                    }
                    result->append(getMsg(msg, marshalType));
                }
                length = result->size();
                msgs = consumer->poll_batch(batchSize - length);
            }
        }
    } catch (std::exception &exception) {
        throw RuntimeException(KAFKA_PREFIX + exception.what());
    }

    if (result->size() == 0) {
        auto err_arg = Util::createConstant(DT_STRING);
        auto msg_arg = Util::createNullConstant(DT_ANY);
        err_arg->setString("No more message");
        auto res = Util::createVector(DT_ANY, 2);
        res->set(0, err_arg);
        res->set(1, msg_arg);
        return res;
    }

    return result;
}
////////////////////////////////////////////////////////////////////////////////

ConstantSP kafkaCreateSubJob(Heap *heap, vector<ConstantSP> args) {
    string usage{
        "kafka::createSubJob(consumer, table, parser, actionName, [throttle=1.0], [autoCommit], "
        "[msgAsTable=false], [batchSize=0], [queueDepth=1000000] "};

    SmartPointer<Consumer> consumer = extractConsumer(args[0], __FUNCTION__, usage);

    if (!(args[1]->isTable() || args[1]->getType() == DT_FUNCTIONDEF || args[1]->isNothing())) {
        throw IllegalArgumentException(__FUNCTION__, usage +
                                                         "table should be a DolphinDB table. If the third "
                                                         "argument is a coder instance, handler must be null.");
    }
    if (args[1]->getType() == DT_FUNCTIONDEF) {
        FunctionDefSP handle = args[1];
        if (handle->getParamCount() != 1) {
            throw IllegalArgumentException(
                __FUNCTION__, usage + "if the second param is functiondef, it must accept only one param.");
        }
    }
    // TODO if table is null
    ConstantSP handler = args[1];
    if (args[2]->getType() == DT_FUNCTIONDEF) {
        if (args[1]->isNothing()) {
            throw IllegalArgumentException(__FUNCTION__,
                                           usage + "If parser is a function, the second argument must not be empty.");
        }
    } else if (args[2]->isOOInstance() && args[2]->getString() == "coder instance") {
        if (!args[1]->isNothing()) {
            throw IllegalArgumentException(__FUNCTION__,
                                           usage + "If parser is a decoder, the second argument must be empty.");
        }
    } else {
        throw IllegalArgumentException(__FUNCTION__, usage + "parser must be an function or a decoder.");
    }
    ConstantSP parser = args[2];

    if (parser->getType() == DT_FUNCTIONDEF &&
        (((FunctionDefSP)parser)->getParamCount() < 1 || ((FunctionDefSP)parser)->getParamCount() > 3)) {
        throw IllegalArgumentException(__FUNCTION__, usage + "parser function must accept only 1 to 3 param.");
    };

    if (args[3]->getType() != DT_STRING || args[3]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "actionName must be a string scalar.");
    }
    string actionName = args[3]->getString();

    // HACK get msgAsTable first
    bool msgAsTable = false;
    if (args.size() >= 7 && !args[6]->isNull()) {
        if (args[6]->getForm() != DF_SCALAR || args[6]->getType() != DT_BOOL) {
            throw IllegalArgumentException(__FUNCTION__, usage + "msgAsTable must be a boolean");
        }
        msgAsTable = args[6]->getBool();
    }

    long long throttle = 1000;
    if (args.size() >= 5 && !args[4]->isNull()) {
        if (args[4]->getForm() != DF_SCALAR || !args[4]->isNumber() || args[4]->getDouble() < 0) {
            throw IllegalArgumentException(__FUNCTION__, usage + "throttle must be a non-negative float");
        }
        if (!msgAsTable) {
            LOG_WARN(KAFKA_PREFIX, "if msgAsTable is false, throttle would be ignored.");
        }
        throttle = args[4]->getDouble() * 1000;
    }

    SubJobAutoCommit autoCommit = KafkaUtil::UNKNOWN;
    if (args.size() >= 6 && !args[5]->isNull()) {
        if (args[5]->getForm() != DF_SCALAR || args[5]->getType() != DT_BOOL) {
            throw IllegalArgumentException(__FUNCTION__, usage + "autoCommit must be a boolean");
        }
        if (args[5]->getBool()) {
            autoCommit = KafkaUtil::COMMIT;
        } else {
            autoCommit = KafkaUtil::NOT_COMMIT;
        }
    }
    long long batchSize = 0;
    if (args.size() >= 8 && !args[7]->isNull()) {
        if (args[7]->getForm() != DF_SCALAR || args[7]->getCategory() != INTEGRAL || args[7]->getLong() < 0) {
            throw IllegalArgumentException(__FUNCTION__, usage + "batchSize must be a non-negative integer");
        }
        if (!msgAsTable) {
            LOG_WARN(KAFKA_PREFIX, "if msgAsTable is false, batchSize would be ignored.");
        }
        batchSize = args[7]->getLong();
    }
    long long queueDepth = 1000000;
    if (args.size() >= 9 && !args[8]->isNull()) {
        if (args[8]->getForm() != DF_SCALAR || args[8]->getCategory() != INTEGRAL || args[8]->getLong() <= 0) {
            throw IllegalArgumentException(__FUNCTION__, usage + "queueDepth must be a positive integer");
        }
        if (!msgAsTable) {
            LOG_WARN(KAFKA_PREFIX, "if msgAsTable is false, queueDepth would be ignored.");
        }
        queueDepth = args[8]->getLong();
    }

    LockGuard<Mutex> _(&DICT_LATCH);
    auto nameMap = SUB_CONN_HANDLE_MAP.getHandleNames();
    for(auto &name: nameMap) {
        if (name == actionName) {
            throw IllegalArgumentException(__FUNCTION__, KAFKA_PREFIX + " actionName '" + actionName + "' has already been used by another kafka subJob connection.");
        }
    }
    SubConnectionSP cup = new SubConnection(heap, args[0], handler, parser, actionName, autoCommit, msgAsTable,
                                            batchSize, throttle, queueDepth);
    FunctionDefSP onClose(
        Util::createSystemProcedure("kafka subJob connection onClose()", kafkaSubConnectionOnClose, 1, 1));
    ConstantSP conn = Util::createResource((long long)(cup.get()), SUB_JOB_DESC, onClose, heap->currentSession());
    SUB_CONN_HANDLE_MAP.safeAdd(conn, cup, actionName);

    return conn;
}

ConstantSP kafkaGetJobStat(Heap *heap, vector<ConstantSP> &args) {
    LockGuard<Mutex> _(&DICT_LATCH);
    vector<string> names = SUB_CONN_HANDLE_MAP.getHandleNames();
    int size = names.size();
    ConstantSP connectionIdVec = Util::createVector(DT_STRING, size);
    ConstantSP userVec = Util::createVector(DT_STRING, size);
    ConstantSP desVec = Util::createVector(DT_STRING, size);
    ConstantSP timestampVec = Util::createVector(DT_TIMESTAMP, size);
    ConstantSP processedMsgCountVec = Util::createVector(DT_LONG, size);
    ConstantSP failedMsgCountVec = Util::createVector(DT_LONG, size);
    ConstantSP lastErrMsgVec = Util::createVector(DT_STRING, size);
    ConstantSP lastFailedTimestampVec = Util::createVector(DT_NANOTIMESTAMP, size);
    ConstantSP msgAsTableVec = Util::createVector(DT_BOOL, size);
    ConstantSP batchSizeVec = Util::createVector(DT_LONG, size);
    ConstantSP throttleVec = Util::createVector(DT_FLOAT, size);
    ConstantSP autoCommitVec = Util::createVector(DT_BOOL, size);
    for (int i = 0; i < size; ++i) {
        string name = names[i];
        SubConnectionSP conn = SUB_CONN_HANDLE_MAP.safeGetByName(name);
        StreamStatus status = conn->getStatus();
        long long connID = (long long)(conn.get());
        string idStr = std::to_string(connID);
        connectionIdVec->setString(i, idStr);
        userVec->setString(i, conn->getSession()->getUser()->getUserId());
        desVec->setString(i, conn->getDescription());
        timestampVec->setLong(i, conn->getCreateTime());
        processedMsgCountVec->setLong(i, status.processedMsgCount_ - status.failedMsgCount_);
        failedMsgCountVec->setLong(i, status.failedMsgCount_);
        lastErrMsgVec->setString(i, status.lastErrMsg_);
        lastFailedTimestampVec->setLong(i, status.lastFailedTimestamp_);
        msgAsTableVec->setBool(i, conn->getAppendTable()->ifMsgAsTable());
        batchSizeVec->setLong(i, conn->getAppendTable()->getBatchSize());
        throttleVec->setFloat(i, conn->getAppendTable()->getThrottle());
        autoCommitVec->setBool(i, conn->getAppendTable()->ifAutoCommit());
    }

    vector<string> colNames = {"subscriptionId",    "user",           "actionName", "createTimestamp",
                               "processedMsgCount", "failedMsgCount", "lastErrMsg",  "lastFailedTimestamp",
                               "msgAsTable",        "batchSize",      "throttle",    "autoCommit"};
    vector<ConstantSP> cols = {connectionIdVec,      userVec,           desVec,        timestampVec,
                               processedMsgCountVec, failedMsgCountVec, lastErrMsgVec, lastFailedTimestampVec,
                               msgAsTableVec,        batchSizeVec,      throttleVec,   autoCommitVec};
    return Util::createTable(colNames, cols);
}

SubConnectionSP getSubJobConn(ConstantSP handle, const string &funcName, const string &usage, string &connName) {
    switch (handle->getType()) {
        case DT_RESOURCE:
            try {
                SubConnectionSP conn = SUB_CONN_HANDLE_MAP.safeGet(handle);
                connName = conn->getDescription();
                return conn;
            } catch (std::exception &e) {
                throw IllegalArgumentException(funcName, usage + "Invalid subJob handle resource.");
            }
            break;
        case DT_STRING: {
            try {
                SubConnectionSP conn = SUB_CONN_HANDLE_MAP.safeGetByName(handle->getString());
                connName = handle->getString();
                return conn;
            } catch (RuntimeException &e) {
                string errMsg(e.what());
                if (errMsg.find("Unknown handle name") != string::npos) {
                    vector<string> names = SUB_CONN_HANDLE_MAP.getHandleNames();
                    for (const string &name : names) {
                        SubConnectionSP conn = SUB_CONN_HANDLE_MAP.safeGetByName(name);
                        if (std::to_string((long long)(conn.get())) == handle->getString()) {
                            connName = name;
                            return conn;
                        }
                    }
                }
                throw e;
            } catch (exception& e) {
                throw e;
            }
        }
        case DT_LONG:
        case DT_INT: {
            vector<string> names = SUB_CONN_HANDLE_MAP.getHandleNames();
            for (const string &name : names) {
                SubConnectionSP conn = SUB_CONN_HANDLE_MAP.safeGetByName(name);
                if ((long long)(conn.get()) == handle->getLong()) {
                    connName = name;
                    return conn;
                }
            }
            throw IllegalArgumentException(funcName, usage + "Invalid subJob handle long.");
        }
        default:
            throw IllegalArgumentException(funcName, usage + "Invalid subJob handle.");
    }
}

ConstantSP kafkaCancelSubJob(Heap *heap, vector<ConstantSP> args) {
    LockGuard<Mutex> _(&DICT_LATCH);
    std::string usage = "kafka::cancelSubJob(subJobConnection) ";
    string name;
    SubConnectionSP conn = getSubJobConn(args[0], "cancelSubJob", usage, name);

    string ret;
    if (!conn.isNull()) {
        conn->cancelThread();
        ret =
            "subscription: " + conn->getDescription() + "(" + std::to_string(((long long)conn.get())) + ") is stopped.";
    }
    try {
        SUB_CONN_HANDLE_MAP.safeRemove(SUB_CONN_HANDLE_MAP.getHandleByName(name));
    } catch (std::exception &e) {
        throw RuntimeException(KAFKA_PREFIX + "remove subJob status failed due to " + e.what());
    }

    return new String(ret);
}

ConstantSP kafkaGetSubJobConsumer(Heap *heap, vector<ConstantSP> &args) {
    LockGuard<Mutex> _(&DICT_LATCH);
    string usage = "kafka::getSubJobConsumer(subJobConnection). ";
    string name;
    SubConnectionSP conn = getSubJobConn(args[0], "getSubJobConsumer", usage, name);
    return conn->getConsumerHandle();
}

////////////////////////////////////////////////////////////////////////////////

ConstantSP kafkaCommit(Heap *heap, vector<ConstantSP> &args) {
    string usage = string("kafka::commit(consumer, [topics], [partitions], [offsets]) ");
    SmartPointer<Consumer> consumer = extractConsumer(args[0], __FUNCTION__, usage);
    if (args.size() != 1 && args.size() != 4) {
        throw IllegalArgumentException("commit", usage + "the number of arguments is either 1 or 4");
    }
    try {
        if (args.size() == 1) {
            consumer->commit();
        } else {
            Conversion convert(usage, args);
            consumer->commit(convert.topicPartitions);
        }
    } catch (std::exception &exception) {
        throw RuntimeException(KAFKA_PREFIX + exception.what());
    }
    return new Void();
}

ConstantSP kafkaCommitTopic(Heap *heap, vector<ConstantSP> &args) {
    string usage = string("kafka::commitTopic(consumer, topic, partition, offset) ");
    SmartPointer<Consumer> consumer = extractConsumer(args[0], __FUNCTION__, usage);
    Conversion convert(usage, args);
    try {
        consumer->commit(convert.topicPartitions);
    } catch (std::exception &exception) {
        throw RuntimeException(KAFKA_PREFIX + exception.what());
    }
    return new Void();
}

ConstantSP kafkaUnsubscribe(Heap *heap, vector<ConstantSP> &args) {
    string usage{"kafka::unsubscribe(consumer) "};
    SmartPointer<Consumer> conn = extractConsumer(args[0], __FUNCTION__, usage);
    try {
        conn->unsubscribe();
    } catch (std::exception &exception) {
        throw RuntimeException(KAFKA_PREFIX + exception.what());
    }
    return new Void();
}

ConstantSP kafkaConsumerAssign(Heap *heap, vector<ConstantSP> &args) {
    LockGuard<Mutex> _(&ASSIGN_LATCH);
    string usage{"kafka::assign(consumer, topic, partition, offset) "};
    SmartPointer<Consumer> consumer = extractConsumer(args[0], __FUNCTION__, usage);
    Conversion convert(usage, args);
    try {
        consumer->assign(convert.topicPartitions);
    } catch (std::exception &e) {
        throw RuntimeException(KAFKA_PREFIX + e.what());
    }
    ASSIGN_TIMEOUT.store(Util::getEpochTime());
    return new Void();
}

ConstantSP kafkaConsumerUnassign(Heap *heap, vector<ConstantSP> &args) {
    LockGuard<Mutex> _(&ASSIGN_LATCH);
    string usage{"kafka::unassign(consumer) "};
    std::atomic<int64_t> current;
    current.store(Util::getEpochTime());
    if (current - ASSIGN_TIMEOUT < 2) {
        Util::sleep(1);
    }
    SmartPointer<Consumer> consumer = extractConsumer(args[0], __FUNCTION__, usage);
    try {
        consumer->unassign();
    } catch (std::exception &e) {
        throw RuntimeException(KAFKA_PREFIX + e.what());
    }
    return new Void();
}

ConstantSP kafkaConsumerGetAssignment(Heap *heap, vector<ConstantSP> &args) {
    LockGuard<Mutex> _(&ASSIGN_LATCH);
    string usage{"kafka::getAssignment(consumer) "};
    SmartPointer<Consumer> consumer = extractConsumer(args[0], __FUNCTION__, usage);
    auto result = consumer->get_assignment();

    int size = result.size();
    ConstantSP topicVec = Util::createVector(DT_STRING, size);
    ConstantSP partitionVec = Util::createVector(DT_INT, size);
    ConstantSP offsetVec = Util::createVector(DT_LONG, size);
    for (int i = 0; i < size; i++) {
        topicVec->setString(i, result[i].get_topic());
        partitionVec->setInt(i, result[i].get_partition());
        offsetVec->setLong(i, result[i].get_offset());
    }
    vector<string> colNames = {"topic", "partition", "offset"};
    vector<ConstantSP> cols = {topicVec, partitionVec, offsetVec};

    return Util::createTable(colNames, cols);
}

ConstantSP kafkaConsumerPause(Heap *heap, vector<ConstantSP> &args) {
    string usage{"kafka::pause(consumer) "};
    SmartPointer<Consumer> consumer = extractConsumer(args[0], __FUNCTION__, usage);
    try {
        consumer->pause();
    } catch (std::exception &e) {
        throw RuntimeException(KAFKA_PREFIX + e.what());
    }
    return new Void();
}

ConstantSP kafkaConsumerResume(Heap *heap, vector<ConstantSP> &args) {
    string usage{"kafka::resume(consumer) "};
    SmartPointer<Consumer> consumer = extractConsumer(args[0], __FUNCTION__, usage);
    try {
        consumer->resume();
    } catch (std::exception &e) {
        throw RuntimeException(KAFKA_PREFIX + e.what());
    }
    return new Void();
}

ConstantSP kafkaGetOffsetInfo(Heap *heap, vector<ConstantSP> &args) {
    string usage{"kafka::getOffsetInfo(consumer, topic, partition) "};
    SmartPointer<Consumer> consumer = extractConsumer(args[0], __FUNCTION__, usage);
    Conversion convert(usage, args);
    int size = convert.topicPartitions.size();

    VectorSP topic = Util::createVector(DT_STRING, size);
    if (args[1]->isScalar()) {
        topic->set(0, args[1]);
    } else {
        topic = args[1];
    }
    VectorSP partition = Util::createVector(DT_INT, size);
    if (args[2]->isScalar()) {
        partition->set(0, args[2]);
    } else {
        partition = args[2];
    }
    VectorSP min = Util::createVector(DT_LONG, size);
    VectorSP max = Util::createVector(DT_LONG, size);
    for (int i = 0; i < int(convert.topicPartitions.size()); ++i) {
        auto back = consumer->get_offsets(convert.topicPartitions[i]);
        min->setLong(i, std::get<0>(back));
        max->setLong(i, std::get<1>(back));
    }
    VectorSP position = Util::createVector(DT_LONG, size);
    auto result = consumer->get_offsets_position(convert.topicPartitions);
    for (int i = 0; i < int(convert.topicPartitions.size()); ++i) {
        position->setLong(i, result[i].get_offset());
    }
    VectorSP committed = Util::createVector(DT_LONG, size);
    result = consumer->get_offsets_committed(convert.topicPartitions);
    for (int i = 0; i < int(convert.topicPartitions.size()); ++i) {
        committed->setLong(i, result[i].get_offset());
    }

    vector<string> names{"topic", "partition", "minOffset", "maxOffset", "offsetPosition", "offsetCommitted"};
    vector<ConstantSP> cols{topic, partition, min, max, position, committed};
    return Util::createTable(names, cols);

}

ConstantSP kafkaGetOffset(Heap *heap, vector<ConstantSP> &args) {
    string usage{"kafka::getOffset(consumer, topic, partition) "};
    SmartPointer<Consumer> consumer = extractConsumer(args[0], __FUNCTION__, usage);
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "Topic must be string scalar.");
    }
    auto topic = args[1]->getString();
    if (args[2]->getType() != DT_INT || args[2]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "partition must be a integer scalar");
    }
    if (args[2]->getInt() < 0) {
        throw IllegalArgumentException(__FUNCTION__, usage + "partition must be a positive integer scalar");
    }
    auto partition = args[2]->getInt();
    auto back = consumer->get_offsets(TopicPartition(topic, partition));
    auto result = Util::createVector(DT_ANY, 2);
    auto low = Util::createConstant(DT_INT);
    auto high = Util::createConstant(DT_INT);
    low->setInt(std::get<0>(back));
    high->setInt(std::get<1>(back));
    result->set(0, low);
    result->set(1, high);

    return result;
}

ConstantSP kafkaGetOffsetsCommitted(Heap *heap, vector<ConstantSP> &args) {
    string usage{"kafka::getOffsetCommitted(consumer, topic, partition, offset, [timeout]) "};
    SmartPointer<Consumer> consumer = extractConsumer(args[0], __FUNCTION__, usage);
    Conversion convert(usage, args);
    vector<TopicPartition> result;
    try {
        if (args.size() == 5) {
            if (args[4]->getType() < DT_SHORT || args[4]->getType() > DT_LONG || args[4]->getInt() < 0) {
                throw IllegalArgumentException(__FUNCTION__, +"timeout should be a positive integer");
            }
            auto time = args[4]->getInt();
            result = consumer->get_offsets_committed(convert.topicPartitions, std::chrono::milliseconds(time));
        } else {
            result = consumer->get_offsets_committed(convert.topicPartitions);
        }
    } catch (std::exception &e) {
        throw RuntimeException(KAFKA_PREFIX + e.what());
    }

    vector<string> colNames{"topic", "partition", "offset"};
    vector<ConstantSP> cols;
    vector<string> topics;
    vector<int> partitions;
    vector<int> offsets;

    for (auto &r : result) {
        topics.emplace_back(r.get_topic());
        partitions.emplace_back(r.get_partition());
        offsets.emplace_back(r.get_offset());
    }
    VectorSP topic = Util::createVector(DT_STRING, 0, topics.size());
    VectorSP partition = Util::createVector(DT_INT, 0, partitions.size());
    VectorSP offset = Util::createVector(DT_INT, 0, offsets.size());

    topic->appendString(topics.data(), topics.size());
    partition->appendInt(partitions.data(), partitions.size());
    offset->appendInt(offsets.data(), offsets.size());
    cols.push_back(topic);
    cols.push_back(partition);
    cols.push_back(offset);

    ConstantSP ret = Util::createTable(colNames, cols);

    return ret;
}

ConstantSP kafkaGetOffsetPosition(Heap *heap, vector<ConstantSP> &args) {
    string usage{"kafka::getOffsetPosition(consumer, topic, partition) "};
    SmartPointer<Consumer> consumer = extractConsumer(args[0], __FUNCTION__, usage);
    Conversion convert(usage, args);

    vector<TopicPartition> result;
    try {
        result = consumer->get_offsets_position(convert.topicPartitions);
    } catch (std::exception &e) {
        throw RuntimeException(KAFKA_PREFIX + e.what());
    }
    vector<string> colNames{"topic", "partition", "offset"};
    vector<ConstantSP> cols;
    vector<string> topics;
    vector<int> partitions;
    vector<int> offsets;

    for (auto &r : result) {
        topics.emplace_back(r.get_topic());
        partitions.emplace_back(r.get_partition());
        offsets.emplace_back(r.get_offset());
    }
    VectorSP topic = Util::createVector(DT_STRING, 0, topics.size());
    VectorSP partition = Util::createVector(DT_INT, 0, partitions.size());
    VectorSP offset = Util::createVector(DT_INT, 0, offsets.size());

    topic->appendString(topics.data(), topics.size());
    partition->appendInt(partitions.data(), partitions.size());
    offset->appendInt(offsets.data(), offsets.size());
    cols.push_back(topic);
    cols.push_back(partition);
    cols.push_back(offset);

    ConstantSP ret = Util::createTable(colNames, cols);

    return ret;
}

ConstantSP kafkaGetMemberId(Heap *heap, vector<ConstantSP> &args) {
    string usage{"kafka::getMemId(consumer) "};
    SmartPointer<Consumer> consumer = extractConsumer(args[0], __FUNCTION__, usage);
    auto result = Util::createConstant(DT_STRING);
    try {
        auto str = consumer->get_member_id();
        result->setString(0, str);
        return result;
    } catch (std::exception &e) {
        throw RuntimeException(KAFKA_PREFIX + e.what());
    }
}

ConstantSP kafkaGetMetadata(Heap *heap, vector<ConstantSP> &args) {
    string usage{"kafka::getMetadata(broker) "};
    Metadata meta;
    GroupInformationList groups;
    DictionarySP result = Util::createDictionary(DT_STRING, nullptr, DT_ANY, nullptr);
    if (args[0]->getType() == DT_STRING && args[0]->getForm() == DF_SCALAR) {
        // HACK use producer to get metadata
        // TODO try catch, for more clear exception
        DictionarySP dict = Util::createDictionary(DT_STRING, nullptr, DT_STRING, nullptr);
        dict->set("metadata.broker.list", args[0]);
        vector<ConstantSP> producerArgs{dict};
        auto producer = ((DdbKafkaProducerSP)kafkaProducer(heap, producerArgs))->getProducer();
        meta = producer->get_metadata();
        groups = producer->get_consumer_groups();
    } else if (args[0]->getForm() == DF_DICTIONARY) {
        // TODO try catch, for more clear exception
        auto conf = createConf(args[0], "getMetadata");
        DdbKafkaProducerSP producerWrapper = new DdbKafkaProducer(heap, conf);
        auto producer = producerWrapper->getProducer();
        meta = producer->get_metadata();
        groups = producer->get_consumer_groups();
    } else if (args[0]->getType() == DT_RESOURCE && args[0]->getString() == CONSUMER_DESC) {
        SmartPointer<Consumer> consumer = (DdbKafkaConsumerSP(args[0]))->getConsumer();
        meta = consumer->get_metadata();
        groups = consumer->get_consumer_groups();
    } else if (args[0]->getType() == DT_RESOURCE && args[0]->getString() == PRODUCER_DESC) {
        SmartPointer<Producer> producer = (DdbKafkaProducerSP(args[0]))->getProducer();
        meta = producer->get_metadata();
        groups = producer->get_consumer_groups();
    } else {
        throw IllegalArgumentException("getMetadata", usage + "broker should be a string scalar.");
    }

    // broker table
    vector<string> colNames = {"id", "host", "port"};
    auto brokers = meta.get_brokers();
    vector<ConstantSP> cols(3);
    cols[0] = Util::createVector(DT_INT, brokers.size());
    cols[1] = Util::createVector(DT_STRING, brokers.size());
    cols[2] = Util::createVector(DT_INT, brokers.size());
    for (auto i = 0u; i < brokers.size(); ++i) {
        ((VectorSP)cols[0])->setInt(i, brokers[i].get_id());
        ((VectorSP)cols[1])->setString(i, brokers[i].get_host());
        ((VectorSP)cols[2])->setInt(i, brokers[i].get_port());
    }
    ConstantSP brokerTable = Util::createTable(colNames, cols);
    result->set("brokers", brokerTable);

    // group info
    DictionarySP groupDict = Util::createDictionary(DT_STRING, nullptr, DT_ANY, nullptr);
    for (auto &group : groups) {
        string name = group.get_name();
        DictionarySP groupData = Util::createDictionary(DT_STRING, nullptr, DT_ANY, nullptr);
        groupData->set("state", new String(group.get_state()));
        groupData->set("protocolType", new String(group.get_protocol_type()));
        groupData->set("protocol", new String(group.get_protocol()));
        groupData->set("error", new String(group.get_error().to_string()));
        DictionarySP memberDict = Util::createDictionary(DT_STRING, nullptr, DT_ANY, nullptr);
        for (auto &member : group.get_members()) {
            DictionarySP memberData = Util::createDictionary(DT_STRING, nullptr, DT_ANY, nullptr);
            string memName = member.get_member_id();
            memberData->set("clientID", new String(member.get_client_id()));
            memberData->set("clientHost", new String(member.get_client_host()));
            try {
                MemberAssignmentInformation info = member.get_member_assignment();
                memberData->set("memberAssignmentVersion", new Long(info.get_version()));
                vector<string> colNames = {"topic", "partition", "offset"};
                vector<ConstantSP> cols(3);
                auto partitions = info.get_topic_partitions();
                cols[0] = Util::createVector(DT_STRING, partitions.size());
                cols[1] = Util::createVector(DT_INT, partitions.size());
                cols[2] = Util::createVector(DT_LONG, partitions.size());
                for (auto i = 0u; i < partitions.size(); ++i) {
                    ((VectorSP)cols[0])->setString(i, partitions[i].get_topic());
                    ((VectorSP)cols[1])->setInt(i, partitions[i].get_partition());
                    ((VectorSP)cols[2])->setLong(i, partitions[i].get_offset());
                }
                ConstantSP partitionTable = Util::createTable(colNames, cols);
                memberData->set("partitions", partitionTable);
            } catch (std::exception &e) {
                LOG_WARN(KAFKA_PREFIX, "cannot parse member assignment info due to ", e.what());
            }
            memberDict->set(memName, memberData);
        }
        groupData->set("members", memberDict);
        groupDict->set(name, groupData);
    }
    result->set("consumerGroup", groupDict);

    // topic table
    DictionarySP topicDict = Util::createDictionary(DT_STRING, nullptr, DT_ANY, nullptr);
    for (auto &topic : meta.get_topics()) {
        string name = topic.get_name();
        vector<string> colNames = {"id", "error", "leader", "replicas"};
        auto partitions = topic.get_partitions();
        vector<ConstantSP> cols(4);
        cols[0] = Util::createVector(DT_LONG, partitions.size());
        cols[1] = Util::createVector(DT_STRING, partitions.size());
        cols[2] = Util::createVector(DT_INT, partitions.size());
        cols[3] = InternalUtil::createArrayVector(DATA_TYPE(DT_INT + ARRAY_TYPE_BASE), 0);
        for (auto i = 0u; i < partitions.size(); ++i) {
            ((VectorSP)cols[0])->setLong(i, partitions[i].get_id());
            ((VectorSP)cols[1])->setString(i, partitions[i].get_error().to_string());
            ((VectorSP)cols[2])->setInt(i, partitions[i].get_leader());
            auto replicas = partitions[i].get_replicas();
            ConstantSP tuple = Util::createVector(DT_ANY, 1);
            VectorSP vec = Util::createVector(DT_INT, 0, replicas.size());
            vec->appendInt(replicas.data(), replicas.size());
            tuple->set(0, vec);
            ((VectorSP)cols[3])->append(tuple);
        }
        ConstantSP topicTable = Util::createTable(colNames, cols);
        topicDict->set(name, topicTable);
    }
    result->set("topics", topicDict);

    return result;
}