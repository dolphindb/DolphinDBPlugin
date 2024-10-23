#include "kafkaWrapper.h"
#include "Types.h"

using namespace KafkaUtil;
/* do nothing*/

DdbKafkaProducer::DdbKafkaProducer(Heap *heap, cppkafka::Configuration conf)
    : Resource(0, PRODUCER_DESC, Util::createSystemProcedure("kafka producer onClose()", mockOnClose, 1, 1),
               heap->currentSession()),
      producer_(new Producer(conf)) {
    // producer_->set_payload_policy(Producer::PayloadPolicy::BLOCK_ON_FULL_QUEUE);  //TODO block if produce too fast
    producer_->get_metadata();  // HACK use this function to verify if really connected
}

DdbKafkaProducer::~DdbKafkaProducer() { 
    try {
        producer_->flush(); 
    } catch (std::exception &e) {
        LOG_WARN(KAFKA_PREFIX, "producer destruction failed: ", e.what());
    } catch (...) {
        LOG_WARN(KAFKA_PREFIX, "producer destruction failed.");
    }
}

inline void drain(SmartPointer<Consumer> consumer) {
    auto start_time = Util::getEpochTime();
    cppkafka::Error last_error(RD_KAFKA_RESP_ERR_NO_ERROR);

    while (true) {
        auto msg = consumer->poll(std::chrono::milliseconds(100));
        if (!msg) break;

        auto error = msg.get_error();

        if (error) {
            if (msg.is_eof() || error == last_error) {
                break;
            } else {
                LOG_ERR(KAFKA_PREFIX, "Timeout during draining.");
            }
        }

        // i don't stop draining on first error,
        // only if it repeats once again sequentially
        last_error = error;

        auto ts = Util::getEpochTime();
        if (ts - start_time > 5000) {
            LOG_ERR(KAFKA_PREFIX, "Timeout during draining.");
            break;
        }
    }
}

DdbKafkaConsumer::DdbKafkaConsumer(Heap *heap, cppkafka::Configuration conf)
    : Resource(0, CONSUMER_DESC, Util::createSystemProcedure("kafka consumer onClose()", mockOnClose, 1, 1),
               heap->currentSession()),
      consumer_(new Consumer(conf)) {
    consumer_->get_metadata();  // HACK use this function to verify if really connected
}
DdbKafkaConsumer::~DdbKafkaConsumer() {
    try {
        consumer_->pause();
        consumer_->unsubscribe();
        consumer_->unassign();
        drain(consumer_);
    } catch (std::exception &e) {
        LOG_WARN(KAFKA_PREFIX, "consumer destruction failed: ", e.what());
    } catch (...) {
        LOG_WARN(KAFKA_PREFIX, "consumer destruction failed.");
    }
}

SmartPointer<Consumer> extractConsumer(const ConstantSP &handle, const string &funcName, const string &usage) {
    if (handle->getType() != DT_RESOURCE || handle->getString() != CONSUMER_DESC)
        throw IllegalArgumentException(funcName, usage + "consumer should be a consumer handle.");
    return (DdbKafkaConsumerSP(handle))->getConsumer();
}

Conversion::Conversion(string usage, vector<ConstantSP> &args) {
    vector<string> topics;
    vector<int> parts;
    if (args[1]->getForm() == DF_VECTOR && args[1]->getType() == DT_STRING) {
        for (int i = 0; i < (int)args[1]->size(); ++i) {
            topics.push_back(args[1]->getString(i));
        }
    } else if (args[1]->getForm() == DF_SCALAR && args[1]->getType() == DT_STRING) {
        topics.push_back(args[1]->getString());
    } else {
        throw IllegalArgumentException(__FUNCTION__, usage + "topics must be a string scalar or string vector.");
    }
    if (args[2]->getForm() == DF_VECTOR && args[2]->getCategory() == INTEGRAL) {
        for (int i = 0; i < (int)args[2]->size(); ++i) {
            parts.push_back(args[2]->getInt(i));
        }
    } else if (args[2]->getForm() == DF_SCALAR && args[2]->getCategory() == INTEGRAL) {
        parts.push_back(args[2]->getInt());
    } else {
        throw IllegalArgumentException(__FUNCTION__, usage + "partitions must be an integer scalar or integer vector.");
    }
    if (topics.size() != parts.size()) {
        throw IllegalArgumentException(__FUNCTION__, usage + "the length of topics and partitions are not the same");
    }

    topicPartitions = TopicPartitionList(topics.begin(), topics.end());
    for (int i = 0; i < (int)parts.size(); i++) {
        topicPartitions[i].set_partition(parts[i]);
    }

    if (args.size() == 4) {
        vector<long long> offsets;
        if (args[3]->getForm() == DF_VECTOR && args[3]->getCategory() == INTEGRAL) {
            for (int i = 0; i < (int)args[3]->size(); ++i) {
                offsets.push_back(args[3]->getLong(i));
            }
        } else if (args[3]->getForm() == DF_SCALAR && args[3]->getCategory() == INTEGRAL) {
            offsets.push_back(args[3]->getLong());
        } else {
            throw IllegalArgumentException(__FUNCTION__, usage + "offsets must be an integer scalar or integer vector.");
        }

        if (topics.size() != offsets.size()) {
            throw IllegalArgumentException(__FUNCTION__, usage + "the length of topics and offsets are not the same");
        }
        for (int i = 0; i < (int)topicPartitions.size(); i++) {
            topicPartitions[i].set_offset(offsets[i]);
        }
    }
}
