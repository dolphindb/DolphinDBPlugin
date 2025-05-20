#include "kafkaAbandoned.h"

#include "kafkaWrapper.h"

using namespace cppkafka;
using namespace ddb;

const string QUEUE_DESC = "kafka queue";
const string EVENT_DESC = "kafka event";

long long BUFFER_SIZE;  // HACK fake param
extern long long MESSAGE_SIZE;

class DdbKafkaQueue : public Resource {
  public:
    DdbKafkaQueue(Heap *heap, Queue &&queue)
        : Resource(0, QUEUE_DESC, Util::createSystemProcedure("kafka queue onClose()", mockOnClose, 1, 1),
                   heap->currentSession()),
          queue_(new Queue(std::move(queue))) {}
    ~DdbKafkaQueue() {}
    SmartPointer<Queue> getQueue() { return queue_; }

  private:
    SmartPointer<cppkafka::Queue> queue_;
};
class DdbKafkaEvent : public Resource {
  public:
    DdbKafkaEvent(Heap *heap, Event &&event)
        : Resource(0, EVENT_DESC, Util::createSystemProcedure("kafka event onClose()", mockOnClose, 1, 1),
                   heap->currentSession()),
          Event_(new Event(std::move(event))) {}
    ~DdbKafkaEvent() {}
    SmartPointer<Event> getEvent() { return Event_; }

  private:
    SmartPointer<cppkafka::Event> Event_;
};
typedef SmartPointer<DdbKafkaQueue> DdbKafkaQueueSP;
typedef SmartPointer<DdbKafkaEvent> DdbKafkaEventSP;

SmartPointer<Queue> extractQueue(const ConstantSP &handle, const string &funcName, const string &usage) {
    if (handle->getType() != DT_RESOURCE || handle->getString() != QUEUE_DESC)
        throw IllegalArgumentException(funcName, usage + "queue should be a queue handle.");
    return (DdbKafkaQueueSP(handle))->getQueue();
}

SmartPointer<Event> extractEvent(const ConstantSP &handle, const string &funcName, const string &usage) {
    if (handle->getType() != DT_RESOURCE || handle->getString() != EVENT_DESC)
        throw IllegalArgumentException(funcName, usage + "event should be a event handle.");
    return (DdbKafkaEventSP(handle))->getEvent();
}

ConstantSP kafkaPollByteStream(Heap *heap, vector<ConstantSP> &args) {
    std::ignore = heap;
    string usage{"Usage: pollByteStream(consumer, [timeout])"};
    SmartPointer<Consumer> consumer = extractConsumer(args[0], __FUNCTION__, usage);
    try {
        KafkaMarshalType marshalType = KafkaMarshalType::PLAIN;

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
ConstantSP kafkaPollDict(Heap *heap, vector<ConstantSP> &args) {
    std::ignore = heap;
    string usage{"pollDict(consumer, batchSize, [timeout]) "};
    SmartPointer<Consumer> consumer = extractConsumer(args[0], __FUNCTION__, usage);

    if (args[1]->getType() < DT_SHORT || args[1]->getType() > DT_LONG || args[1]->getInt() < 0) {
        throw IllegalArgumentException(__FUNCTION__, usage + "batch size need positive integer");
    }
    auto batch_size = args[1]->getInt();
    auto result = Util::createDictionary(DT_STRING, nullptr, DT_ANY, nullptr);

    vector<Message> msgs;
    try {
        if (args.size() == 3) {
            if (args[2]->getType() < DT_SHORT || args[2]->getType() > DT_LONG || args[2]->getInt() < 0) {
                throw IllegalArgumentException(__FUNCTION__, +"time need positive integer");
            }
            auto time = args[2]->getInt();
            msgs = consumer->poll_batch(batch_size, std::chrono::milliseconds(time));

            int length = 0;
            while (length < batch_size) {
                if (msgs.size() == 0) {
                    break;
                }
                for (auto &msg : msgs) {
                    if (msg && msg.get_error() && msg.is_eof()) {
                        continue;
                    }
                    auto message = getMsg(msg, KafkaUtil::AUTO);
                    if (message->get(1)->get(1)->getType() != DT_STRING) {
                        throw RuntimeException("can only get string as key");
                    }
                    result->set(message->get(1)->get(1), message->get(1)->get(2));
                }
                length = result->size();
                msgs = consumer->poll_batch(batch_size - length, std::chrono::milliseconds(time));
            }
        } else {
            msgs = consumer->poll_batch(batch_size);
            int length = 0;
            while (length < batch_size) {
                if (msgs.size() == 0) {
                    break;
                }
                for (auto &msg : msgs) {
                    if (msg && msg.get_error() && msg.is_eof()) {
                        continue;
                    }
                    auto message = getMsg(msg, KafkaUtil::AUTO);
                    if (message->get(1)->get(1)->getType() != DT_STRING) {
                        throw RuntimeException("can only get string as key");
                    }
                    result->set(message->get(1)->get(1), message->get(1)->get(2));
                }
                length = result->size();
                msgs = consumer->poll_batch(batch_size - length);
            }
        }
    } catch (IllegalArgumentException &e) {
        throw e;
    } catch (std::exception &exception) {
        throw RuntimeException(KAFKA_PREFIX + exception.what());
    }
    if (result->size() == 0) {
        return new Void();
    }
    return result;
}

ConstantSP kafkaSetConsumerTimeout(Heap *heap, vector<ConstantSP> &args) {
    std::ignore = heap;
    string usage{"setConsumerTime(consumer, timeout) "};
    SmartPointer<Consumer> consumer = extractConsumer(args[0], __FUNCTION__, usage);
    if (args[1]->getType() < DT_SHORT || args[1]->getType() > DT_LONG || args[1]->getInt() < 0) {
        throw IllegalArgumentException(__FUNCTION__, usage + "time must be a positive integer");
    }
    auto time = args[1]->getInt();
    consumer->set_timeout(std::chrono::milliseconds(time));
    return new Void();
}
ConstantSP kafkaSetProducerTimeout(Heap *heap, vector<ConstantSP> &args) {
    std::ignore = heap;
    string usage{"setProducerTime(producer, timeout) "};
    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != PRODUCER_DESC) {
        throw IllegalArgumentException(__FUNCTION__, usage + "producer should be a producer handle.");
    }
    SmartPointer<Producer> producer = ((DdbKafkaProducerSP)args[0])->getProducer();
    if (args[1]->getType() < DT_SHORT || args[1]->getType() > DT_LONG || args[1]->getInt() < 0) {
        throw IllegalArgumentException(__FUNCTION__, usage + "time need positive integer");
    }
    auto time = args[1]->getInt();
    producer->set_timeout(std::chrono::milliseconds(time));
    return new Void();
}
ConstantSP kafkaGetConsumerTimeout(Heap *heap, vector<ConstantSP> &args) {
    std::ignore = heap;
    string usage{"getConsumerTime(consumer) "};
    SmartPointer<Consumer> consumer = extractConsumer(args[0], __FUNCTION__, usage);
    auto res = Util::createConstant(DT_INT);
    res->setInt(static_cast<int>(consumer->get_timeout().count()));
    return res;
}
ConstantSP kafkaGetProducerTimeout(Heap *heap, vector<ConstantSP> &args) {
    std::ignore = heap;
    string usage{"getProducerTime(producer) "};
    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != PRODUCER_DESC) {
        throw IllegalArgumentException(__FUNCTION__, usage + "producer should be a producer handle.");
    }
    SmartPointer<Producer> producer = ((DdbKafkaProducerSP)args[0])->getProducer();
    auto res = Util::createConstant(DT_INT);
    res->setInt(static_cast<int>(producer->get_timeout().count()));
    return res;
}
ConstantSP kafkaGetBufferSize(Heap *heap, vector<ConstantSP> &args) {
    std::ignore = heap;
    std::ignore = args;
    auto result = Util::createConstant(DT_LONG);
    result->setLong(BUFFER_SIZE);
    return result;
}
ConstantSP kafkaSetBufferSize(Heap *heap, vector<ConstantSP> &args) {
    std::ignore = heap;
    long long size = args[0]->getLong();
    if (size >= MESSAGE_SIZE)
        BUFFER_SIZE = size;
    else {
        BUFFER_SIZE = size;
        MESSAGE_SIZE = size;
        throw RuntimeException(
            KAFKA_PREFIX +
            "The buffer_size is smaller than message_size. The message_size is set the same as buffer_size.");
    }
    return new String(
        "The buffer size has been successfully set, please make sure the buffer size is no larger than broker size.");
}

ConstantSP kafkaGetMessageSize(Heap *heap, vector<ConstantSP> &args) {
    std::ignore = heap;
    std::ignore = args;
    auto result = Util::createConstant(DT_LONG);
    result->setLong(MESSAGE_SIZE);
    return result;
}
ConstantSP kafkaSetMessageSize(Heap *heap, vector<ConstantSP> &args) {
    std::ignore = heap;
    long long size = args[0]->getLong();
    if (size <= BUFFER_SIZE)
        MESSAGE_SIZE = size;
    else {
        throw IllegalArgumentException(__FUNCTION__, +"message_size should not be larger than buffer_size");
    }
    return new Void();
}

ConstantSP kafkaAsyncCommit(Heap *heap, vector<ConstantSP> &args) {
    std::ignore = heap;
    string usage{"asyncCommit(consumer) "};
    SmartPointer<Consumer> consumer = extractConsumer(args[0], __FUNCTION__, usage);
    try {
        consumer->async_commit();
    } catch (std::exception &exception) {
        throw RuntimeException(KAFKA_PREFIX + exception.what());
    }
    return new Void();
}
ConstantSP kafkaAsyncCommitTopic(Heap *heap, vector<ConstantSP> &args) {
    std::ignore = heap;
    string usage{"asyncCommitTopic(consumer, topic, partition, offset)"};
    SmartPointer<Consumer> consumer = extractConsumer(args[0], __FUNCTION__, usage);
    Conversion convert(usage, args);
    try {
        consumer->async_commit(convert.topicPartitions);
    } catch (std::exception &exception) {
        throw RuntimeException(KAFKA_PREFIX + exception.what());
    }

    return new Void();
}
ConstantSP kafkaStoreConsumedOffsets(Heap *heap, vector<ConstantSP> &args) {
    std::ignore = heap;
    string usage{"storeOffset(consumer)"};
    SmartPointer<Consumer> consumer = extractConsumer(args[0], __FUNCTION__, usage);
    try {
        consumer->store_consumed_offsets();
    } catch (std::exception &e) {
        throw RuntimeException(KAFKA_PREFIX + e.what());
    }
    return new Void();
}
ConstantSP kafkaStoreOffsets(Heap *heap, vector<ConstantSP> &args) {
    std::ignore = heap;
    string usage{"storeOffset(consumer, topic, partition, offset) "};
    SmartPointer<Consumer> consumer = extractConsumer(args[0], __FUNCTION__, usage);
    Conversion convert(usage, args);
    try {
        consumer->store_offsets(convert.topicPartitions);
    } catch (std::exception &e) {
        throw RuntimeException(KAFKA_PREFIX + e.what());
    }

    return new Void();
}

ConstantSP kafkaQueueLength(Heap *heap, vector<ConstantSP> &args) {
    std::ignore = heap;
    string usage{"queueLength(queue) "};
    SmartPointer<Queue> queue = extractQueue(args[0], __FUNCTION__, usage);

    auto result = Util::createConstant(DT_INT);
    try {
        result->setInt(queue->get_length());
        return result;
    } catch (std::exception &e) {
        throw RuntimeException(KAFKA_PREFIX + e.what());
    }
}
ConstantSP kafkaForToQueue(Heap *heap, vector<ConstantSP> &args) {
    std::ignore = heap;
    string usage{"forToQueue(queue, forwardQueue) "};
    SmartPointer<Queue> queue = extractQueue(args[0], __FUNCTION__, usage);
    SmartPointer<Queue> forward_queue = extractQueue(args[1], __FUNCTION__, usage);
    if (args[0]->getLong() == args[1]->getLong()) {
        throw RuntimeException(KAFKA_PREFIX + "Messages cannot be forwarded to the same queue.");
    }
    try {
        queue->forward_to_queue(*forward_queue);
    } catch (std::exception &e) {
        throw RuntimeException(KAFKA_PREFIX + e.what());
    }
    return new Void();
}
ConstantSP kafkaDisForToQueue(Heap *heap, vector<ConstantSP> &args) {
    std::ignore = heap;
    string usage{"disforToQueue(queue) "};
    SmartPointer<Queue> queue = extractQueue(args[0], __FUNCTION__, usage);
    try {
        queue->disable_queue_forwarding();
    } catch (std::exception &e) {
        throw RuntimeException(KAFKA_PREFIX + e.what());
    }
    return new Void();
}
ConstantSP kafkaSetQueueTime(Heap *heap, vector<ConstantSP> &args) {
    std::ignore = heap;
    string usage{"setQueueTime(queue, time) "};
    SmartPointer<Queue> queue = extractQueue(args[0], __FUNCTION__, usage);
    if (args[1]->getType() < DT_SHORT || args[1]->getType() > DT_LONG || args[1]->getInt() < 0) {
        throw IllegalArgumentException(__FUNCTION__, usage + "time need positive integer");
    }
    auto time = args[1]->getInt();
    queue->set_timeout(std::chrono::milliseconds(time));
    return new Void();
}
ConstantSP kafkaGetQueueTime(Heap *heap, vector<ConstantSP> &args) {
    std::ignore = heap;
    string usage{"getQueueTime(queue) "};
    SmartPointer<Queue> queue = extractQueue(args[0], __FUNCTION__, usage);

    auto result = Util::createConstant(DT_INT);
    result->setInt(static_cast<int>(queue->get_timeout().count()));
    return result;
}
ConstantSP kafkaQueueConsume(Heap *heap, vector<ConstantSP> &args) {
    std::ignore = heap;
    string usage{"queuePoll(queue, [timeout]) "};
    SmartPointer<Queue> queue = extractQueue(args[0], __FUNCTION__, usage);

    try {
        if (args.size() == 2) {
            if (args[1]->getType() < DT_SHORT || args[1]->getType() > DT_LONG || args[1]->getInt() < 0) {
                throw IllegalArgumentException(__FUNCTION__, +"time need positive integer");
            }
            auto time = args[1]->getInt();
            auto msg = queue->consume(std::chrono::milliseconds(time));
            if (msg && msg.get_error() && msg.is_eof()) {
                msg = queue->consume(std::chrono::milliseconds(time));
            }
            return getMsg(msg, KafkaUtil::AUTO);
        } else {
            auto msg = queue->consume(std::chrono::milliseconds(1000));
            if (msg && msg.get_error() && msg.is_eof()) {
                msg = queue->consume(std::chrono::milliseconds(1000));
            }
            return getMsg(msg, KafkaUtil::AUTO);
        }
    } catch (std::exception &e) {
        throw RuntimeException(KAFKA_PREFIX + e.what());
    }
}
ConstantSP kafkaQueueConsumeBatch(Heap *heap, vector<ConstantSP> &args) {
    std::ignore = heap;
    string usage{"queuePollBatch(queue, batch_size, [timeout]) "};
    SmartPointer<Queue> queue = extractQueue(args[0], __FUNCTION__, usage);

    if (args[1]->getType() < DT_SHORT || args[1]->getType() > DT_LONG || args[1]->getInt() < 0) {
        throw IllegalArgumentException(__FUNCTION__, usage + "batch size need positive integer");
    }
    auto batch_size = args[1]->getInt();
    auto result = Util::createVector(DT_ANY, 0);

    vector<Message> msgs;
    try {
        if (args.size() == 3) {
            if (args[2]->getType() < DT_SHORT || args[2]->getType() > DT_LONG || args[2]->getInt() < 0) {
                throw IllegalArgumentException(__FUNCTION__, +"time need positive integer");
            }
            auto time = args[2]->getInt();
            msgs = queue->consume_batch(batch_size, std::chrono::milliseconds(time));

            int length = 0;
            while (length < batch_size) {
                if (msgs.size() == 0) {
                    break;
                }
                for (auto &msg : msgs) {
                    if (msg && msg.get_error() && msg.is_eof()) {
                        continue;
                    }
                    result->append(getMsg(msg, KafkaUtil::AUTO));
                }
                length = result->size();
                msgs = queue->consume_batch(batch_size - length, std::chrono::milliseconds(time));
            }
        } else {
            msgs = queue->consume_batch(batch_size, std::chrono::milliseconds(1000));
            int length = 0;
            while (length < batch_size) {
                if (msgs.size() == 0) {
                    break;
                }
                for (auto &msg : msgs) {
                    if (msg && msg.get_error() && msg.is_eof()) {
                        continue;
                    }
                    result->append(getMsg(msg, KafkaUtil::AUTO));
                }
                length = result->size();
                msgs = queue->consume_batch(batch_size - length, std::chrono::milliseconds(1000));
            }
        }
    } catch (std::exception &e) {
        throw RuntimeException(KAFKA_PREFIX + e.what());
    }

    if (result->size() == 0) {
        auto err_arg = Util::createConstant(DT_STRING);
        auto msg_arg = Util::createNullConstant(DT_ANY);
        err_arg->setString("No more message");
        auto res = Util::createVector(DT_ANY, 2);
        res->set(0, err_arg);
        res->set(1, msg_arg);
        free(result);
        return res;
    }

    return result;
}
ConstantSP kafkaGetMainQueue(Heap *heap, vector<ConstantSP> &args) {
    string usage{"getMainQueue(consumer) "};
    SmartPointer<Consumer> consumer = extractConsumer(args[0], __FUNCTION__, usage);

    try {
        return new DdbKafkaQueue(heap, consumer->get_main_queue());
    } catch (std::exception &e) {
        throw RuntimeException(KAFKA_PREFIX + e.what());
    }
}
ConstantSP kafkaGetConsumerQueue(Heap *heap, vector<ConstantSP> &args) {
    string usage{"getConsumerQueue(consumer) "};
    SmartPointer<Consumer> consumer = extractConsumer(args[0], __FUNCTION__, usage);

    try {
        return new DdbKafkaQueue(heap, consumer->get_consumer_queue());
    } catch (std::exception &e) {
        throw RuntimeException(KAFKA_PREFIX + e.what());
    }
}
ConstantSP kafkaGetPartitionQueue(Heap *heap, vector<ConstantSP> &args) {
    string usage{"getPartitionQueue(consumer, topic, partition) "};
    SmartPointer<Consumer> consumer = extractConsumer(args[0], __FUNCTION__, usage);

    Conversion convert(usage, args);
    try {
        return new DdbKafkaQueue(heap, consumer->get_partition_queue(convert.topicPartitions[0]));
    } catch (std::exception &e) {
        throw RuntimeException(KAFKA_PREFIX + e.what());
    }
}
ConstantSP kafkaQueueEvent(Heap *heap, vector<ConstantSP> &args) {
    string usage{"queuePollBatch(queue, batch_size, [timeout]) "};
    SmartPointer<Queue> queue = extractQueue(args[0], __FUNCTION__, usage);
    try {
        return new DdbKafkaEvent(heap, queue->next_event());
    } catch (std::exception &e) {
        throw RuntimeException(KAFKA_PREFIX + e.what());
    }
}
ConstantSP kafkaGetEventName(Heap *heap, vector<ConstantSP> &args) {
    std::ignore = heap;
    string usage{"getEventName(event) "};
    SmartPointer<Event> event = extractEvent(args[0], __FUNCTION__, usage);

    auto name = Util::createConstant(DT_STRING);
    name->setString(event->get_name());
    return name;
}
ConstantSP kafkaEventGetMessages(Heap *heap, vector<ConstantSP> &args) {
    std::ignore = heap;
    string usage{"eventGetMessage(event) "};
    SmartPointer<Event> event = extractEvent(args[0], __FUNCTION__, usage);

    if (!event->operator bool()) {
        throw RuntimeException(KAFKA_PREFIX + "The event is empty!");
    }
    auto result = Util::createVector(DT_ANY, 0);
    vector<Message> msgs;
    try {
        msgs = event->get_messages();
    } catch (std::exception &e) {
        throw RuntimeException(KAFKA_PREFIX + e.what());
    }
    for (auto &msg : msgs) {
        result->append(getMsg(msg, KafkaUtil::AUTO));
    }
    if (msgs.size() == 0) {
        auto err_arg = Util::createConstant(DT_STRING);
        auto msg_arg = Util::createNullConstant(DT_ANY);
        err_arg->setString("No more message");
        auto res = Util::createVector(DT_ANY, 2);
        res->set(0, err_arg);
        res->set(1, msg_arg);
        free(result);
        return res;
    }
    return result;
}
ConstantSP kafkaGetEventMessageCount(Heap *heap, vector<ConstantSP> &args) {
    std::ignore = heap;
    string usage{"getEventMessageCount(event) "};
    SmartPointer<Event> event = extractEvent(args[0], __FUNCTION__, usage);

    if (!event->operator bool()) {
        throw RuntimeException(KAFKA_PREFIX + "The event is empty!");
    }
    auto count = Util::createConstant(DT_INT);
    count->setInt(event->get_message_count());
    return count;
}
ConstantSP kafkaEventGetError(Heap *heap, vector<ConstantSP> &args) {
    std::ignore = heap;
    string usage{"eventGetError(event) "};
    SmartPointer<Event> event = extractEvent(args[0], __FUNCTION__, usage);

    if (!event->operator bool()) {
        throw RuntimeException(KAFKA_PREFIX + "The event is empty!");
    }
    auto error = event->get_error();
    auto string = Util::createConstant(DT_STRING);
    string->setString(error.to_string());
    return string;
}
ConstantSP kafkaEventGetPartition(Heap *heap, vector<ConstantSP> &args) {
    std::ignore = heap;
    string usage{"eventGetPart(event) "};
    SmartPointer<Event> event = extractEvent(args[0], __FUNCTION__, usage);

    if (!event->operator bool()) {
        throw RuntimeException(KAFKA_PREFIX + "The event is empty!");
    }

    std::stringstream ss;
    try {
        ss << event->get_topic_partition() << std::endl;
    } catch (std::exception &e) {
        throw RuntimeException(KAFKA_PREFIX + e.what());
    }
    auto result = Util::createConstant(DT_STRING);
    result->setString(ss.str());

    return result;
}
ConstantSP kafkaEventGetPartitionList(Heap *heap, vector<ConstantSP> &args) {
    std::ignore = heap;
    string usage{"eventGetParts(event) "};
    SmartPointer<Event> event = extractEvent(args[0], __FUNCTION__, usage);

    if (!event->operator bool()) {
        throw RuntimeException(KAFKA_PREFIX + "The event is empty!");
    }

    if (event->get_type() != RD_KAFKA_EVENT_OFFSET_COMMIT && event->get_type() != RD_KAFKA_EVENT_REBALANCE) {
        throw RuntimeException(KAFKA_PREFIX + "wrong event type.");
    }
    vector<TopicPartition> result;
    try {
        result = event->get_topic_partition_list();
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
ConstantSP kafkaEventBool(Heap *heap, vector<ConstantSP> &args) {
    std::ignore = heap;
    string usage{"eventBool(event) "};
    SmartPointer<Event> event = extractEvent(args[0], __FUNCTION__, usage);
    auto result = Util::createConstant(DT_BOOL);
    result->setBool(1, event->operator bool());
    return result;
}