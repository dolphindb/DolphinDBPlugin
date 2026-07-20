//
// Created by htxu on 10/23/2023.
//

#include "pulsarUtil.h"
#include <ddbplugin/PluginLogger.h>

ddb::ResourceMap<ClientWrapper> PULSAR_CLIENT_MAP(PULSAR_PREFIX, PULSAR_CLIENT_DESC);
ddb::ResourceMap<ProducerWrapper> PULSAR_PRODUCER_MAP(PULSAR_PREFIX, PULSAR_PRODUCER_DESC);
ddb::ResourceMap<ConsumerWrapper> PULSAR_CONSUMER_MAP(PULSAR_PREFIX, PULSAR_CONSUMER_DESC);
ddb::BackgroundResourceMap<PulsarSubJob> PULSAR_SUB_JOB_MAP(PULSAR_PREFIX, PULSAR_SUB_JOB_DESC);

void clientOnClose(Heap *heap, vector<ConstantSP>& args) {
    try {
        auto client = PULSAR_CLIENT_MAP.safeGet(args[0]);
        client->client_.close();
        PULSAR_CLIENT_MAP.safeRemove(args[0]);

    } catch (exception &e) {
        LOG_ERR(PULSAR_PREFIX + e.what());
    }
}

void producerOnClose(Heap *heap, vector<ConstantSP>& args) {
    try {
        auto producer = PULSAR_PRODUCER_MAP.safeGet(args[0]);
        producer->producer_.flush();
        producer->producer_.close();
        PULSAR_PRODUCER_MAP.safeRemove(args[0]);

    } catch (exception &e) {
        LOG_ERR(PULSAR_PREFIX + e.what());
    }
}

void consumerOnClose(Heap *heap, vector<ConstantSP>& args) {
    try {
        auto consumer = PULSAR_CONSUMER_MAP.safeGet(args[0]);
        consumer->consumer_.unsubscribe();
        PULSAR_CONSUMER_MAP.safeRemove(args[0]);

    } catch (exception &e) {
        LOG_ERR(PULSAR_PREFIX + e.what());
    }
}

void subJobOnClose(Heap *heap, vector<ConstantSP>& args) {
//    try {
//        auto subJob = PULSAR_SUB_JOB_MAP.safeGet(args[0]);
//        subJob->stop();
//        PULSAR_SUB_JOB_MAP.safeRemove(args[0]);
//
//    } catch (exception &e) {
//        LOG_ERR(PULSAR_PREFIX + e.what());
//    }
}

pulsar::ClientConfiguration parseClientConfig(const DictionarySP& configDict) {
    try {
        auto clientConfig = pulsar::ClientConfiguration();

        VectorSP keys = configDict->keys();
        for (auto i = 0; i < keys->size(); i++) {
            auto key = keys->getString(i);
            if (key == "operationTimeoutSeconds") {
                auto value = getPositiveIntConfig(configDict->getMember(key), key);
                clientConfig.setOperationTimeoutSeconds(value);
            } else if (key == "connectionTimeoutMs") {
                auto value = getPositiveIntConfig(configDict->getMember(key), key);
                clientConfig.setConnectionTimeout(value);
            } else if (key == "token") {
                std::string token = configDict->getMember(key)->getString();
                clientConfig.setAuth(pulsar::AuthToken::createWithToken(token));
            } else if (key == "memoryLimit") {
                ConstantSP limit = configDict->getMember(key);
                if (limit->getForm() != DF_SCALAR || limit->getCategory() != INTEGRAL || limit->getLong() <= 0) {
                    throw RuntimeException(key + " should be a positive integral");
                }
                uint64_t limitByMB = static_cast<uint64_t>(limit->getLong());
                if(limitByMB > 17592186044415ULL) {
                    throw RuntimeException(key + " cannot be larger than 17592186044415");
                }
                clientConfig.setMemoryLimit(limitByMB * 1024 * 1024);
            } else {
                throw IllegalArgumentException(__FUNCTION__, key + " is not supported");
            }
        }

        return clientConfig;

    } catch (exception &e) {
        throw RuntimeException(PULSAR_PREFIX + e.what());
    }
}

pulsar::ProducerConfiguration parseProducerConfig(const DictionarySP& configDict) {
    try {
        auto producerConfig = pulsar::ProducerConfiguration();

        VectorSP keys = configDict->keys();
        for (auto i = 0; i < keys->size(); i++) {
            auto key = keys->getString(i);
            if (key == "sendTimeoutMs") {
                auto value = getPositiveIntConfig(configDict->getMember(key), key);
                producerConfig.setSendTimeout(value);
            } else if (key == "maxPendingMessages") {
                auto value = getPositiveIntConfig(configDict->getMember(key), key);
                producerConfig.setMaxPendingMessages(value);
            } else if (key == "maxPendingMessagesAcrossPartitions") {
                auto value = getPositiveIntConfig(configDict->getMember(key), key);
                producerConfig.setMaxPendingMessagesAcrossPartitions(value);
            } else if (key == "blockIfQueueFull") {
                auto value = getBoolConfig(configDict->getMember(key), key);
                producerConfig.setBlockIfQueueFull(value);
            } else if (key == "batchingEnabled") {
                auto value = getBoolConfig(configDict->getMember(key), key);
                producerConfig.setBatchingEnabled(value);
            } else if (key == "batchingMaxMessages") {
                auto value = getPositiveIntConfig(configDict->getMember(key), key);
                producerConfig.setBatchingMaxMessages(value);
            } else if (key == "batchingMaxPublishDelayMs") {
                auto value = getPositiveIntConfig(configDict->getMember(key), key);
                producerConfig.setBatchingMaxPublishDelayMs(value);
            } else if (key == "compressionType") {
                auto value = getStringConfig(configDict->getMember(key), key);
                if (value == "lz4") {
                    producerConfig.setCompressionType(pulsar::CompressionLZ4);
                } else if (value == "zlib") {
                    producerConfig.setCompressionType(pulsar::CompressionZLib);
                } else if (value == "zstd") {
                    producerConfig.setCompressionType(pulsar::CompressionZSTD);
                } else if (value == "snappy") {
                    producerConfig.setCompressionType(pulsar::CompressionSNAPPY);
                } else if (value == "none") {
                    producerConfig.setCompressionType(pulsar::CompressionNone);
                } else {
                    throw RuntimeException("compressionType should be one of 'lz4', 'zlib', 'zstd', 'snappy' or 'none'");
                }
            } else {
                throw IllegalArgumentException(__FUNCTION__, key + " is not supported");
            }
        }

        return producerConfig;

    } catch (exception &e) {
        throw RuntimeException(PULSAR_PREFIX + e.what());
    }
}

pulsar::ConsumerConfiguration parseConsumerConfig(const DictionarySP& configDict) {
    try {
        auto consumerConfig = pulsar::ConsumerConfiguration();

        VectorSP keys = configDict->keys();
        for (auto i = 0; i < keys->size(); i++) {
            auto key = keys->getString(i);
            if (key == "consumerType") {
                auto consumerType = configDict->getMember(key)->getInt();
                if (pulsar::ConsumerExclusive <= consumerType and consumerType <= pulsar::ConsumerKeyShared) {
                    consumerConfig.setConsumerType(static_cast<pulsar::ConsumerType>(consumerType));
                } else {
                    throw RuntimeException("consumerType should be within 0 to 3");
                }
            } else if (key == "receiverQueueSize") {
                auto value = getPositiveIntConfig(configDict->getMember(key), key);
                consumerConfig.setReceiverQueueSize(value);
            } else if (key == "maxTotalReceiverQueueSizeAcrossPartitions") {
                auto value = getPositiveIntConfig(configDict->getMember(key), key);
                consumerConfig.setMaxTotalReceiverQueueSizeAcrossPartitions(value);
            } else if (key == "unAckedMessagesTimeoutMs") {
                auto value = getPositiveIntConfig(configDict->getMember(key), key);
                consumerConfig.setUnAckedMessagesTimeoutMs(value);
            } else if (key == "subscriptionInitialPosition") {
                auto position = configDict->getMember(key)->getInt();
                if (pulsar::InitialPositionLatest <= position and position <= pulsar::InitialPositionEarliest) {
                    consumerConfig.setSubscriptionInitialPosition(static_cast<pulsar::InitialPosition>(position));
                } else {
                    throw RuntimeException("subscriptionInitialPosition should be within 0 to 1");
                }
            } else {
                throw IllegalArgumentException(__FUNCTION__, key + " is not supported");
            }
        }

        return consumerConfig;

    } catch (exception &e) {
        throw RuntimeException(PULSAR_PREFIX + e.what());
    }
}

int getPositiveIntConfig(const ConstantSP& value, const string& key) {
    if (value->getForm() != DF_SCALAR or value->getType() != DT_INT or value->getInt() <= 0) {
        throw RuntimeException(key + " should be a positive int");
    }
    return value->getInt();
}

bool getBoolConfig(const ConstantSP& value, const string& key) {
    if (value->getForm() != DF_SCALAR or value->getType() != DT_BOOL) {
        throw RuntimeException(key + " should be bool");
    }
    return value->getBool();
}

std::string getStringConfig(const ConstantSP& value, const string& key) {
    if (value->getForm() != DF_SCALAR or value->getType() != DT_STRING) {
        throw RuntimeException(key + " should be string");
    }
    return value->getString();
}
