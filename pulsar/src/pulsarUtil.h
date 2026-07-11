#pragma once

#include "DolphinDBEverything.h"
#include "ddbplugin/Plugin.h"

#include "pulsarSubJob.h"

using namespace ddb;

/// Temporary wrapper, to be refactored
struct ClientWrapper {
    pulsar::Client client_;
    pulsar::ClientConfiguration clientConfig_;
};

struct ProducerWrapper {
    pulsar::Producer producer_;
    pulsar::ProducerConfiguration producerConfig_;
};

struct ConsumerWrapper {
    pulsar::Consumer consumer_;
    pulsar::ConsumerConfiguration consumerConfig_;
};

/// Constants
constexpr size_t MAX_TOPIC_LENGTH {4096};

/// Resource Descriptions
const string PULSAR_PREFIX = "[Plugin::Pulsar] ";
const string PULSAR_CLIENT_DESC = "pulsar client";
const string PULSAR_PRODUCER_DESC = "pulsar producer";
const string PULSAR_CONSUMER_DESC = "pulsar consumer";
const string PULSAR_SUB_JOB_DESC = "pulsar subscription job";

/// Resource Maps
extern ResourceMap<ClientWrapper> PULSAR_CLIENT_MAP;
extern ResourceMap<ProducerWrapper> PULSAR_PRODUCER_MAP;
extern ResourceMap<ConsumerWrapper> PULSAR_CONSUMER_MAP;
extern BackgroundResourceMap<PulsarSubJob> PULSAR_SUB_JOB_MAP;

/// Resource Close Functions
void clientOnClose(Heap *heap, vector<ConstantSP>& args);
void producerOnClose(Heap *heap, vector<ConstantSP>& args);
void consumerOnClose(Heap *heap, vector<ConstantSP>& args);
void subJobOnClose(Heap *heap, vector<ConstantSP>& args);

/// Helpers
pulsar::ClientConfiguration parseClientConfig(const DictionarySP& configDict);
pulsar::ProducerConfiguration parseProducerConfig(const DictionarySP& configDict);
pulsar::ConsumerConfiguration parseConsumerConfig(const DictionarySP& configDict);

int getPositiveIntConfig(const ConstantSP& value, const string& key);
bool getBoolConfig(const ConstantSP& value, const string& key);
std::string getStringConfig(const ConstantSP& value, const string& key);
