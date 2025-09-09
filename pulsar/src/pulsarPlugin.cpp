//
// Created by htxu on 10/19/2023.
//

#include <ScalarImp.h>

#include "pulsarPlugin.h"
#include "pulsarSubJob.h"
#include "pulsarUtil.h"

using namespace pulsar;

/// Interfaces

ConstantSP pulsarClient(Heap *heap, vector<ConstantSP> &args) {
    string usage = "client(serviceUrl, [clientConfig]): ";

    /// parse args
    // serviceUrl
    if (args[0]->getForm() != DF_SCALAR || args[0]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, usage + "serviceUrl should be a string.");
    }
    auto serviceUrl = args[0]->getString();
    // clientConfig
    auto clientConfig = ClientConfiguration();
    if (args.size() == 2) {
        if (args[1]->getForm() != DF_DICTIONARY) {
            throw IllegalArgumentException(__FUNCTION__, usage + "clientConfig should be a dictionary.");
        }
        clientConfig = parseClientConfig(args[1]);
    }

    /// create client
    try {
        // set logger
        clientConfig.setLogger(new FileLoggerFactory(pulsar::Logger::Level::LEVEL_INFO, "pulsar-cpp-client.log"));

        SmartPointer<ClientWrapper> client = new ClientWrapper{
            Client(serviceUrl, clientConfig),
            clientConfig
        };

        FunctionDefSP onClose(Util::createSystemProcedure("pulsar client onClose()", clientOnClose, 1, 1));
        ConstantSP resource = Util::createResource(reinterpret_cast<long long>(client.get()), PULSAR_CLIENT_DESC, onClose, heap->currentSession());
        PULSAR_CLIENT_MAP.safeAdd(resource, client);

        return resource;

    } catch (exception &e) {
        throw RuntimeException(PULSAR_PREFIX + e.what());
    }
}

ConstantSP pulsarProducer(Heap *heap, vector<ConstantSP> &args) {
    string usage = "producer(client, topic, [producerConfig]): ";

    /// parse args
    // client
    if (args[0]->getForm() != DF_SCALAR || args[0]->getType()!= DT_RESOURCE || args[0]->getString() != PULSAR_CLIENT_DESC) {
        throw IllegalArgumentException(__FUNCTION__, usage + "client should be a pulsar client handle.");
    }
    auto client = PULSAR_CLIENT_MAP.safeGet(args[0]);
    // topic
    if (args[1]->getForm() != DF_SCALAR || args[1]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, usage + "topic should be a string.");
    }
    auto topic = args[1]->getString();
    if (topic.length() >= MAX_TOPIC_LENGTH) {
        throw IllegalArgumentException(__FUNCTION__, usage + "topic length should be less than 4096.");
    }
    // producerConfig
    auto producerConfig = ProducerConfiguration();
    if (args.size() == 3) {
        if (args[2]->getForm() != DF_DICTIONARY) {
            throw IllegalArgumentException(__FUNCTION__, usage + "producerConfig should be a dictionary.");
        }
        producerConfig = parseProducerConfig(args[2]);
    }

    /// create producer
    try {
        SmartPointer<ProducerWrapper> producer = new ProducerWrapper{
            Producer(),
            producerConfig
        };
        auto result = client->client_.createProducer(topic, producerConfig, producer->producer_);
        if (result != ResultOk) {
            throw RuntimeException(string("Error creating producer: ") + strResult(result));
        }
        FunctionDefSP onClose(Util::createSystemProcedure("pulsar producer onClose()", producerOnClose, 1, 1));
        ConstantSP resource = Util::createResource(reinterpret_cast<long long>(producer.get()), PULSAR_PRODUCER_DESC,
                                                   onClose, heap->currentSession());
        PULSAR_PRODUCER_MAP.safeAdd(resource, producer);

        return resource;

    } catch (exception &e) {
        throw RuntimeException(PULSAR_PREFIX + e.what());
    }
}

ConstantSP pulsarSend(Heap *heap, vector<ConstantSP> &args) {
    string usage = "send(producer, message, [partitionKey]): ";

    /// parse args
    // producer
    if (args[0]->getForm() != DF_SCALAR || args[0]->getType()!= DT_RESOURCE || args[0]->getString() != PULSAR_PRODUCER_DESC) {
        throw IllegalArgumentException(__FUNCTION__, usage + "producer should be a pulsar producer handle.");
    }
    auto producer = PULSAR_PRODUCER_MAP.safeGet(args[0]);
    // message
    if (args[1]->getForm() != DF_SCALAR || args[1]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, usage + "message should be a string.");
    }
    auto message = args[1]->getString();
    // partitionKey
    string partitionKey;
    if (args.size() == 3) {
        if (args[2]->getForm() != DF_SCALAR || args[2]->getType() != DT_STRING) {
            throw IllegalArgumentException(__FUNCTION__, usage + "partitionKey should be a string.");
        }
        partitionKey = args[2]->getString();
    }

    /// build and send message
    try {
        auto msgBuilder = MessageBuilder().setContent(std::move(message));
        if (!partitionKey.empty()) {
            msgBuilder.setPartitionKey(partitionKey);
        }
        Message msg = msgBuilder.build();

        Result result = producer->producer_.send(msg);
        if (result != ResultOk) {
            throw RuntimeException(string("Error sending message: ") + strResult(result));
        }

        return new Void();
    } catch (exception &e) {
        throw RuntimeException(PULSAR_PREFIX + e.what());
    }
}

ConstantSP pulsarConsumer(Heap *heap, vector<ConstantSP> &args) {
    string usage = "consumer(client, topic, subscriptionName, [consumerConfig]): ";

    /// parse args
    // client
    if (args[0]->getForm() != DF_SCALAR || args[0]->getType()!= DT_RESOURCE || args[0]->getString() != PULSAR_CLIENT_DESC) {
        throw IllegalArgumentException(__FUNCTION__, usage + "client should be a pulsar client handle.");
    }
    auto client = PULSAR_CLIENT_MAP.safeGet(args[0]);
    // topic
    if (args[1]->getForm() != DF_SCALAR || args[1]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, usage + "topic should be a string.");
    }
    auto topic = args[1]->getString();
    if (topic.length() >= MAX_TOPIC_LENGTH) {
        throw IllegalArgumentException(__FUNCTION__, usage + "topic length should be less than 4096.");
    }
    // subscriptionName
    if (args[2]->getForm() != DF_SCALAR || args[2]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, usage + "subscriptionName should be a string.");
    }
    auto subscriptionName = args[2]->getString();
    // consumerConfig
    auto consumerConfig = ConsumerConfiguration();
    if (args.size() == 4) {
        if (args[3]->getForm() != DF_DICTIONARY) {
            throw IllegalArgumentException(__FUNCTION__, usage + "consumerConfig should be a dictionary.");
        }
        consumerConfig = parseConsumerConfig(args[3]);
    }

    /// create consumer and subscribe
    try {
        SmartPointer<ConsumerWrapper> consumer = new ConsumerWrapper{
            Consumer(),
            consumerConfig
        };
        auto result = client->client_.subscribe(topic, subscriptionName, consumerConfig, consumer->consumer_);
        if (result != ResultOk) {
            throw RuntimeException(string("Failed to subscribe: ") + strResult(result));
        }
        FunctionDefSP onClose(Util::createSystemProcedure("pulsar consumer onClose()", consumerOnClose, 1, 1));
        ConstantSP resource = Util::createResource(reinterpret_cast<long long>(consumer.get()), PULSAR_CONSUMER_DESC,
                                                   onClose, heap->currentSession());
        PULSAR_CONSUMER_MAP.safeAdd(resource, consumer);

        return resource;

    } catch (exception &e) {
        throw RuntimeException(PULSAR_PREFIX + e.what());
    }
}

ConstantSP pulsarReceive(Heap *heap, vector<ConstantSP> &args) {
    string usage = "receive(consumer, [timeoutMs=1000]): ";

    /// parse args
    // consumer
    if (args[0]->getForm() != DF_SCALAR || args[0]->getType()!= DT_RESOURCE || args[0]->getString() != PULSAR_CONSUMER_DESC) {
        throw IllegalArgumentException(__FUNCTION__, usage + "consumer should be a pulsar consumer handle.");
    }
    auto consumer = PULSAR_CONSUMER_MAP.safeGet(args[0]);
    // timeoutMs
    auto timeoutMs = 1000;
    if (args.size() == 2) {
        if (args[1]->getForm() != DF_SCALAR || args[1]->getType() != DT_INT) {
            throw IllegalArgumentException(__FUNCTION__, usage + "timeoutMs should be an int.");
        }
        timeoutMs = args[1]->getInt();
    }

    /// receive message
    try {
        Message msg;
        auto result = consumer->consumer_.receive(msg, timeoutMs);
        if (result != ResultOk) {
            throw RuntimeException(string("Failed to receive: ") + strResult(result));
        }
        auto data = msg.getDataAsString();
        consumer->consumer_.acknowledge(msg);

        return new String(data);

    } catch (exception &e) {
        throw RuntimeException(PULSAR_PREFIX + e.what());
    }
}

ConstantSP pulsarCreateSubJob(Heap *heap, vector<ConstantSP> &args) {
    string usage = "createSubJob(jobName, client, topic, subscriptionName, table, parser, [consumerConfig]): ";

    /// parse args
    // jobName
    if (args[0]->getForm() != DF_SCALAR || args[0]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, usage + "jobName should be a string.");
    }
    auto jobName = args[0]->getString();
    // client
    if (args[1]->getForm() != DF_SCALAR || args[1]->getType()!= DT_RESOURCE || args[1]->getString() != PULSAR_CLIENT_DESC) {
        throw IllegalArgumentException(__FUNCTION__, usage + "client should be a pulsar client handle.");
    }
    auto client = PULSAR_CLIENT_MAP.safeGet(args[1]);
    // topic
    if (args[2]->getForm() != DF_SCALAR || args[2]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, usage + "topic should be a string.");
    }
    auto topic = args[2]->getString();
    if (topic.length() >= MAX_TOPIC_LENGTH) {
        throw IllegalArgumentException(__FUNCTION__, usage + "topic length should be less than 4096.");
    }
    // subscriptionName
    if (args[3]->getForm() != DF_SCALAR || args[3]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, usage + "subscriptionName should be a string.");
    }
    auto subscriptionName = args[3]->getString();
    // table
    if (args[4]->getForm() != DF_TABLE) {
        throw IllegalArgumentException(__FUNCTION__, usage + "table should be a table.");
    }
    TableSP table = args[4];
    if (table->getTableType() != REALTIMETBL or !table->isSharedTable()) {
        throw IllegalArgumentException(__FUNCTION__, usage + "table should be a shared stream table.");
    }
    // parser
    if (args[5]->getForm() != DF_SCALAR || args[5]->getType() != DT_FUNCTIONDEF) {
        throw IllegalArgumentException(__FUNCTION__, usage + "parser should be an function.");
    }
    auto parser = args[5];
    // consumerConfig
    auto consumerConfig = ConsumerConfiguration();
    if (args.size() == 7) {
        if (args[6]->getForm() != DF_DICTIONARY) {
            throw IllegalArgumentException(__FUNCTION__, usage + "consumerConfig should be a dictionary.");
        }
        consumerConfig = parseConsumerConfig(args[6]);
    }

    try {
        /// create consumer
        SmartPointer<Consumer> consumer = new Consumer();
        // set listener
        auto session = heap->currentSession()->copy();
        consumerConfig.setMessageListener([session, table, parser](pulsar::Consumer &consumer_, const pulsar::Message &msg) {
            PulsarSubJob::listener(session, table, parser, consumer_, msg);
        });
        // subscribe
        auto result = client->client_.subscribe(topic, subscriptionName, consumerConfig, *consumer);
        if (result != ResultOk) {
            throw RuntimeException(string("Failed to subscribe: ") + strResult(result));
        }

        /// create subscription job
        SmartPointer<PulsarSubJob> subJob(new PulsarSubJob(heap, args[1], consumer, consumerConfig));
        FunctionDefSP onClose(Util::createSystemProcedure("pulsar subscription job onClose()", subJobOnClose, 1, 1));
        ConstantSP resource = Util::createResource(reinterpret_cast<long long>(subJob.get()), PULSAR_SUB_JOB_DESC,
                                                   onClose, heap->currentSession());
        PULSAR_SUB_JOB_MAP.safeAdd(resource, subJob, jobName);

        return resource;

    } catch (exception &e) {
        throw RuntimeException(PULSAR_PREFIX + e.what());
    }
}

ConstantSP pulsarCancelSubJob(Heap *heap, vector<ConstantSP> &args) {
    string usage = "cancelSubJob(subJob): ";

    /// parse args
    // subJob
    SmartPointer<PulsarSubJob> subJob;
    switch (args[0]->getType()) {
        case DT_RESOURCE:
            subJob = PULSAR_SUB_JOB_MAP.safeGet(args[0]);
            break;
        case DT_STRING:
            subJob = PULSAR_SUB_JOB_MAP.safeGetByName(args[0]->getString());
            break;
        default:
            throw IllegalArgumentException(__FUNCTION__, usage + "subJob should be a job handle or description string.");
    }
    
    /// stop the sub job
    subJob->stop();

    return new Void();
}

ConstantSP pulsarGetJobStat(Heap *heap, vector<ConstantSP> &args) {
    string usage = "getJobStat(): ";

    /// init cols
    auto size = PULSAR_SUB_JOB_MAP.size();
    ConstantSP nameVec = Util::createVector(DT_STRING, size);
    ConstantSP userVec = Util::createVector(DT_STRING, size);
    ConstantSP createTimeVec = Util::createVector(DT_TIMESTAMP, size);
    ConstantSP endTimeVec = Util::createVector(DT_TIMESTAMP, size);

    /// set info
    auto names = PULSAR_SUB_JOB_MAP.getHandleNames();
    for (auto i = 0; i < size; i++) {
        auto name = names[i];
        SmartPointer<PulsarSubJob> subJob = PULSAR_SUB_JOB_MAP.safeGetByName(name);

        nameVec->setString(i, name);
        userVec->setString(i, subJob->getSession()->getUser()->getUserId());
        createTimeVec->setLong(i, subJob->getCreateTime());
        endTimeVec->setLong(i, subJob->getEndTime());
    }

    /// return status table
    vector<string> colNames = {"name", "user", "createTime", "endTime"};
    vector<ConstantSP> cols = {nameVec, userVec, createTimeVec, endTimeVec};
    return Util::createTable(colNames, cols);
}

ConstantSP pulsarGetConfig(Heap *heap, vector<ConstantSP> &args) {
    string usage = "getConfig(handle, configName): ";

    /// args
    // handle
    if (args[0]->getForm() != DF_SCALAR or args[0]->getType() != DT_RESOURCE) {
        throw IllegalArgumentException(__FUNCTION__, "handle should be a handle of client, producer, or consumer.");
    }
    auto type = args[0]->getString();
    auto handle = args[0];
    // config
    if (args[1]->getForm() != DF_SCALAR or args[1]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, "configName should be a string.");
    }
    auto configName = args[1]->getString();

    /// get config
    if (type == PULSAR_CLIENT_DESC) {

        auto &clientConfig = PULSAR_CLIENT_MAP.safeGet(handle)->clientConfig_;
        if (configName == "operationTimeoutSeconds") {
            return new Int(clientConfig.getOperationTimeoutSeconds());
        } else if (configName == "connectionTimeoutMs") {
            return new Int(clientConfig.getConnectionTimeout());
        }
    } else if (type == PULSAR_PRODUCER_DESC) {

        auto &producerConfig = PULSAR_PRODUCER_MAP.safeGet(handle)->producerConfig_;
        if (configName == "sendTimeoutMs") {
            return new Int(producerConfig.getSendTimeout());
        } else if (configName == "maxPendingMessages") {
            return new Int(producerConfig.getMaxPendingMessages());
        } else if (configName == "maxPendingMessagesAcrossPartitions") {
            return new Int(producerConfig.getMaxPendingMessagesAcrossPartitions());
        } else if (configName == "blockIfQueueFull") {
            return new Bool(producerConfig.getBlockIfQueueFull());
        } else if (configName == "batchingEnabled") {
            return new Bool(producerConfig.getBatchingEnabled());
        } else if (configName == "batchingMaxMessages") {
            return new Long(producerConfig.getBatchingMaxMessages());
        } else if (configName == "batchingMaxPublishDelayMs") {
            return new Long(producerConfig.getBatchingMaxPublishDelayMs());
        }
    } else if (type == PULSAR_CONSUMER_DESC or type == PULSAR_SUB_JOB_DESC) {

        auto &consumerConfig = type == PULSAR_CONSUMER_DESC
                ? PULSAR_CONSUMER_MAP.safeGet(handle)->consumerConfig_
                : PULSAR_SUB_JOB_MAP.safeGet(handle)->consumerConfig_;
        if (configName == "consumerType") {
            return new Int(consumerConfig.getConsumerType());
        } else if (configName == "receiverQueueSize") {
            return new Int(consumerConfig.getReceiverQueueSize());
        } else if (configName == "maxTotalReceiverQueueSizeAcrossPartitions") {
            return new Int(consumerConfig.getMaxTotalReceiverQueueSizeAcrossPartitions());
        } else if (configName == "unAckedMessagesTimeoutMs") {
            return new Long(consumerConfig.getUnAckedMessagesTimeoutMs());
        } else if (configName == "subscriptionInitialPosition") {
            return new Int(consumerConfig.getSubscriptionInitialPosition());
        }
    }

    throw RuntimeException("the configuration is not supported.");
}
