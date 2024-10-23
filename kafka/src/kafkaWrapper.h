#ifndef KAFKA_PLUGIN_WRAPPER_H
#define KAFKA_PLUGIN_WRAPPER_H

#include "kafkaUtil.h"

using namespace KafkaUtil;
/* do nothing*/
class rawMessageWrapper {
  public:
    rawMessageWrapper(rd_kafka_message_t *ptr) : msgPtr_(ptr) {}
    ~rawMessageWrapper() {
        if (msgPtr_) {
            rd_kafka_message_destroy(msgPtr_);
        }
    }

  public:
    rd_kafka_message_t *msgPtr_;
};

class DdbKafkaProducer : public Resource {
  public:
    DdbKafkaProducer(Heap *heap, cppkafka::Configuration conf);
    ~DdbKafkaProducer();
    SmartPointer<Producer> getProducer() { return producer_; }

  private:
    SmartPointer<cppkafka::Producer> producer_;
};

class DdbKafkaConsumer : public Resource {
  public:
    DdbKafkaConsumer(Heap *heap, cppkafka::Configuration conf);
    ~DdbKafkaConsumer();
    SmartPointer<Consumer> getConsumer() { return consumer_; }

  private:
    SmartPointer<cppkafka::Consumer> consumer_;
};

class Conversion {
  public:
    TopicPartitionList topicPartitions;
    Conversion(string usage, vector<ConstantSP> &args);
};

typedef SmartPointer<rawMessageWrapper> rawMessageWrapperSP;
typedef SmartPointer<DdbKafkaProducer> DdbKafkaProducerSP;
typedef SmartPointer<DdbKafkaConsumer> DdbKafkaConsumerSP;

SmartPointer<Consumer> extractConsumer(const ConstantSP &handle, const string &funcName, const string &usage);

inline static void mockOnClose(Heap *heap, vector<ConstantSP> &args) {}

#endif  // KAFKA_PLUGIN_WRAPPER_H