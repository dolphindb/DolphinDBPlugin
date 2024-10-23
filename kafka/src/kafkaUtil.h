#ifndef KAFKA_PLUGIN_UTIL_H
#define KAFKA_PLUGIN_UTIL_H
#include <cppkafka/error.h>
#include <cppkafka/logging.h>
#include <cppkafka/producer.h>

#include "ConstantMarshal.h"
#include "CoreConcept.h"
#include "Logger.h"
#include "ScalarImp.h"
#include "Util.h"
#include "cppkafka/cppkafka.h"
#include "json.hpp"

using namespace cppkafka;

namespace KafkaUtil {

const static int TYPE_SIZE[17] = {0, 1, 1, 2, 4, 8, 4, 4, 4, 4, 4, 8, 8, 8, 8, 4, 8};
const static string KAFKA_PREFIX = "[PLUGIN::KAFKA] ";
const static string PRODUCER_DESC = "kafka producer connection";
const static string CONSUMER_DESC = "kafka consumer connection";
const static string SUB_JOB_DESC = "kafka subJob connection";

enum KafkaMarshalType {
    AUTO = -1,
    DOLPHINDB = 0,
    JSON = 1,
    PLAIN = 2,
};

enum SubJobAutoCommit {
    UNKNOWN,
    COMMIT,
    NOT_COMMIT,
};

static const unordered_map<string, KafkaMarshalType> marshalMap = {
    {"DOLPHINDB", KafkaUtil::DOLPHINDB},
    {"JSON", KafkaUtil::JSON},
    {"PLAIN", KafkaUtil::PLAIN},
};

void produceMsg(SmartPointer<Producer> producer, const string &topic, const string &key, ConstantSP value,
                KafkaMarshalType marshalType, int partition, bool force = false);
VectorSP getMsg(Message &msg, KafkaMarshalType marshalType);
Configuration createConf(ConstantSP &dict, const string &funcName, bool consumer = false, Heap *heap = nullptr,
                         FunctionDefSP func = nullptr);

string kafkaSerialize(const ConstantSP &data, KafkaMarshalType type);
ConstantSP kafkaDeserialize(const string &buffer, KafkaMarshalType type);

string jsonSerialize(const ConstantSP &data);
string stringSerialize(const ConstantSP &data, bool key = false);
string dolphinSerialize(const ConstantSP &data);
ConstantSP jsonDeserialize(const string &buffer, KafkaMarshalType marshalType = JSON);
ConstantSP dolphindbDeserialize(const string &buffer);

}  // namespace KafkaUtil
#endif  // KAFKA_PLUGIN_UTIL_H