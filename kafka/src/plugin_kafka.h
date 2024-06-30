#ifndef KAFKA_PLUGIN_KAFKA_H
#define KAFKA_PLUGIN_KAFKA_H

#include "CoreConcept.h"
#include "Exceptions.h"
#include "LocklessContainer.h"
#include "Util.h"
#include "Types.h"
#include "Concurrent.h"
#include "cppkafka/cppkafka.h"
#include "ScalarImp.h"
#include "ConstantMarshal.h"
#include "json.hpp"
#include "client.h"

#include <cstring>
#include <iostream>
#include <string>
#include <sstream>
#include <unordered_set>

using namespace cppkafka;
using namespace std;
using json = nlohmann::json;

/// See Config https://github.com/edenhill/librdkafka/blob/master/CONFIGURATION.md
extern "C" ConstantSP kafkaProducer(Heap *heap, vector<ConstantSP> &dicts);

extern "C" ConstantSP kafkaProduce(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaProducerFlush(Heap *heap, vector<ConstantSP> &args);

/// See Config https://github.com/edenhill/librdkafka/blob/master/CONFIGURATION.md
extern "C" ConstantSP kafkaConsumer(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaSubscribe(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaUnsubscribe(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaConsumerPoll(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaPollByteStream(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaConsumerPollBatch(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaCreateSubJob(Heap *heap, vector<ConstantSP> args);

extern "C" ConstantSP kafkaGetJobStat(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaCancelSubJob(Heap *heap, vector<ConstantSP> args);

extern "C" ConstantSP kafkaPollDict(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaCommit(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaCommitTopic(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaAsyncCommit(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaAsyncCommitTopic(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaSetConsumerTimeout(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaSetProducerTimeout(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaGetConsumerTimeout(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaGetProducerTimeout(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaConsumerAssign(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaConsumerUnassign(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaConsumerGetAssignment(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaConsumerPause(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaConsumerResume(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaGetOffset(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaGetOffsetsCommitted(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaGetOffsetPosition(Heap *heap, vector<ConstantSP> &args);

#if (RD_KAFKA_VERSION >= RD_KAFKA_STORE_OFFSETS_SUPPORT_VERSION)
extern "C" ConstantSP kafkaStoreConsumedOffsets(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaStoreOffsets(Heap *heap, vector<ConstantSP> &args);
#endif

extern "C" ConstantSP kafkaGetMemberId(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaQueueLength(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaForToQueue(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaDisForToQueue(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaSetQueueTime(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaGetQueueTime(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaQueueConsume(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaQueueConsumeBatch(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaGetMainQueue(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaGetConsumerQueue(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaGetPartitionQueue(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaQueueEvent(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaGetEventName(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaEventGetMessages(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaGetEventMessageCount(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaEventGetError(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaEventGetPartition(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaEventGetPartitionList(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaEventBool(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaGetBufferSize(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaSetBufferSize(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaGetMessageSize(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaSetMessageSize(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP kafkaGetSubJobConsumer(Heap *heap, vector<ConstantSP> &args);

void produceMessage(ConstantSP &produce, ConstantSP &pTopic, ConstantSP &key, ConstantSP &value, ConstantSP &json, ConstantSP &pPartition);

Vector *getMsg(Message &msg);

string kafkaGetString(const ConstantSP &data, bool key = false);

string kafkaJsonSerialize(const ConstantSP &data);

string kafkaSerialize(ConstantSP &data, ConstantSP &json);

ConstantSP kafkaDeserialize(const string &data_init);

Configuration createConf(ConstantSP &dict, bool consumer = false);

class Conversion{
public:
    TopicPartitionList topic_partitions;
    Conversion(string usage, vector<ConstantSP> &args){
        if (args[1]->getForm() != DF_VECTOR || args[1]->getType() != DT_STRING) {
            throw IllegalArgumentException(__FUNCTION__, usage + "Not a topic vector.");
        }
        if (args[2]->getForm() != DF_VECTOR || args[2]->getType() != DT_INT) {
            throw IllegalArgumentException(__FUNCTION__, usage + "Not a partition vector.");
        }
        auto &topic = args[1];
        auto &part = args[2];
        if(topic->size()!=part->size()){
            throw IllegalArgumentException(__FUNCTION__, usage + "the length of two vectors are not the same");
        }
        auto topics = vector<string>{};
        for (int i = 0; i < (int)topic->size(); i++) {
            topics.push_back(topic->get(i)->getString());
        }
        topic_partitions = TopicPartitionList(topics.begin(), topics.end());
        for(int j = 0;j<(int)topic_partitions.size();j++){
            topic_partitions[j].set_partition(part->get(j)->getInt());
        }
        if(args.size() == 4){
            if (args[3]->getForm() != DF_VECTOR || args[3]->getType() != DT_INT) {
                throw IllegalArgumentException(__FUNCTION__, usage + "Not a integer vector");
            }
            if(topic->size()!=args[3]->size()){
                throw IllegalArgumentException(__FUNCTION__, usage + "the length of three vectors are not the same");
            }
            for(int j = 0;j<(int)topic_partitions.size();j++){
                topic_partitions[j].set_offset(args[3]->get(j)->getInt());
            }
        }
    }
};

class Defer {
public:
    Defer(std::function<void()> code): code(code) {}
    ~Defer() {code(); }
private:
    std::function<void()> code;
};

extern Mutex HANDLE_MUTEX;
extern set<long long> HANDLE_SET;

template<typename T>
class KafkaWrapper{
public:
    explicit KafkaWrapper(T * ptr): dataPtr(ptr){
    }
    ~KafkaWrapper() {
        auto start = Util::getEpochTime();
        while(!rwLock.tryAcquireWrite()) {
            auto current = Util::getEpochTime();
            if(current - start > 5000) {
                LOG_ERR(string("[PLUGIN::KAFKA] ") + typeid(T).name() + string("destruction failed"));
                return;
            }
        }
        Defer([this]{
            rwLock.releaseWrite();
        });
        if(dataPtr != nullptr) {
            delete dataPtr;
            dataPtr = NULL;
        }
    }
    RWLock* getLock() {
        return &rwLock;
    }
    T* getDataPtr() {
        return dataPtr;
    }
private:
    T * dataPtr;
    RWLock rwLock;
};

template<typename T>
static void kafkaOnClose(Heap *heap, vector<ConstantSP> &args) {
    try {
        LockGuard<Mutex> _(&HANDLE_MUTEX);
        if(HANDLE_SET.find(args[0]->getLong()) == HANDLE_SET.end()) {
            throw IllegalArgumentException(__FUNCTION__, "Invalid connection object.");
        }
        KafkaWrapper<T> * wrapper = (KafkaWrapper<T>*)(args[0]->getLong());
        if(wrapper!= nullptr) {
            long long handleLong = args[0]->getLong();
            args[0]->setLong(0);
            delete wrapper;
            HANDLE_SET.erase(handleLong);
        }
    } catch(std::exception& exception) {
        LOG_ERR(string("[PLUGIN:KAFKA] Destruction failed: ") + exception.what());
    }
}

// must be used before getConnection !
template<typename T>
static void getWrapperLockGuard(ConstantSP &handler, RWLockGuard<RWLock>& lock) {
    LockGuard<Mutex> _(&HANDLE_MUTEX);
    if(HANDLE_SET.find(handler->getLong()) == HANDLE_SET.end()) {
        throw IllegalArgumentException(__FUNCTION__, "Invalid connection object.");
    }
    if (handler->getType() != DT_RESOURCE || handler->getLong() == 0) {
        throw IllegalArgumentException(__FUNCTION__, "Invalid connection object.");
    } else {
        KafkaWrapper<T> * wrapper = (KafkaWrapper<T>*)(handler->getLong());
        if(wrapper!= nullptr) {
            lock = RWLockGuard<RWLock>(wrapper->getLock(), false);
        } else {
            throw IllegalArgumentException(__FUNCTION__, "Invalid connection object.");
        }
    }
}

// must be used after getWrapperLock !
template<typename T>
static T* getConnection(ConstantSP &handler) {
    LockGuard<Mutex> _(&HANDLE_MUTEX);
    if(HANDLE_SET.find(handler->getLong()) == HANDLE_SET.end()) {
        throw IllegalArgumentException(__FUNCTION__, "Invalid connection object.");
    }
    if (handler->getType() != DT_RESOURCE || handler->getLong() == 0) {
        throw IllegalArgumentException(__FUNCTION__, "Invalid connection object.");
    } else {
        KafkaWrapper<T> * wrapper = (KafkaWrapper<T>*)(handler->getLong());
        if(wrapper!= nullptr) {
            auto conn =  wrapper->getDataPtr();
            if(!conn) {
                throw IllegalArgumentException(__FUNCTION__, "Invalid connection object.");
            }
            return conn;
        } else {
            throw IllegalArgumentException(__FUNCTION__, "Invalid connection object.");
        }
    }
}


#endif //KAFKA_PLUGIN_KAFKA_H
