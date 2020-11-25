#ifndef KAFKA_PLUGIN_KAFKA_H
#define KAFKA_PLUGIN_KAFKA_H

#include "CoreConcept.h"
#include "Exceptions.h"
#include "Util.h"
#include "Types.h"
#include "Concurrent.h"
#include "cppkafka/cppkafka.h"
#include "ScalarImp.h"
#include "ConstantMarshall.h"
#include "json.hpp"
#include "client.h"

#include <iostream>
#include <string>

using namespace cppkafka;
using namespace std;
using json = nlohmann::json;

string producer_desc = "kafka producer connection";
string consumer_desc = "kafka consumer connection";
string queue_desc = "kafka queue connection";
string event_desc = "kafka event connection";


DictionarySP status_dict = Util::createDictionary(DT_STRING,nullptr,DT_ANY,nullptr);

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

static void produceMessage(ConstantSP &produce, ConstantSP &pTopic, ConstantSP &key, ConstantSP &value, ConstantSP &json, ConstantSP &pPartition);

static Vector *getMsg(Message &msg);

static string kafkaGetString(const ConstantSP &data, bool key = false);

static string kafkaJsonSerialize(const ConstantSP &data);

static string kafkaSerialize(ConstantSP &data, ConstantSP &json);

static ConstantSP kafkaDeserialize(const string &data_init);

static Configuration createConf(ConstantSP &dict, bool consumer = false);

long long int buffer_size = 900000;

long long int message_size = 10000;

double factor = 0.95;

int type_size[17] = {0,1,1,2,4,8,4,4,4,4,4,8,8,8,8,4,8};

class SerializationException : public exception {
public:
    SerializationException(size_t expected, size_t actual) : expected_(expected), actual_(actual) {};

    virtual ~SerializationException() {}

    virtual const char *what() const throw() {
        return ("Serialization Exception: expected " + to_string(expected_) + " bytes," + " actual " +
                to_string(actual_) + " bytes.").c_str();
    }

private:
    size_t expected_;
    size_t actual_;
};

class Convertion{
public:
    TopicPartitionList topic_partitions;
    Convertion(string usage, vector<ConstantSP> &args){
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

template<typename T>
static void kafkaOnClose(Heap *heap, vector<ConstantSP> &args) {
    T* pObject = (T*)(args[0]->getLong());
    if(pObject!= nullptr) {
        delete (T *) (args[0]->getLong());
        args[0]->setLong(0);
    }
}

template<typename T>
static T *getConnection(ConstantSP &handler) {
    if (handler->getType() != DT_RESOURCE) {
        throw IllegalArgumentException(__FUNCTION__, "Invalid connection object.");
    } else {
        return (T *) (handler->getLong());
    }
}

static void kafkaConsumerOnClose(Heap *heap, vector<ConstantSP> &args) {
    auto consumer = (Consumer *) args[0]->getLong();
    consumer->unsubscribe();
    delete consumer;
}

template<typename T, size_t N>
static T check_decode(const string &data, DATA_TYPE dataType) {
    if (data.length() != N) {
        throw SerializationException(N, data.length());
    }
    T val = 0;
    for (auto c: data) {
        val <<= 8;
        val |= c & 0xFF;
    }
    return val;
}

#endif //KAFKA_PLUGIN_KAFKA_H
