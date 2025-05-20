#include "kafkaUtil.h"

#include "kafkaWrapper.h"
#include "ddbplugin/PluginLogger.h"
#include "OperatorImp.h"

using json = nlohmann::json;

long long MESSAGE_SIZE = 10240;

namespace KafkaUtil {
void kafkaErrorCallback(KafkaHandleBase &handle, int error, const std::string &reason, Heap *heap, FunctionDefSP func) {
    std::ignore = handle;
    if (heap && !func.isNull()) {
        vector<ConstantSP> args{new String("ERROR"), new String(Error((rd_kafka_resp_err_t)error).to_string()),
                                new String(reason)};
        func->call(heap, args);
    }
    PLUGIN_LOG_ERR(KAFKA_PREFIX, "error: ", Error((rd_kafka_resp_err_t)error).to_string(), ", reason: ", reason);
}

void kafkaLogCallback(KafkaHandleBase &handle, int level, const std::string &facility, const std::string &message,
                      Heap *heap, FunctionDefSP func) {
    std::ignore = handle;
    switch ((LogLevel)level) {
        case LogLevel::LogEmerg:
        case LogLevel::LogAlert:
        case LogLevel::LogCrit:
        case LogLevel::LogErr:
            if (heap && !func.isNull()) {
                vector<ConstantSP> args{new String("ERROR"), new String(facility), new String(message)};
                func->call(heap, args);
            }
            PLUGIN_LOG_ERR(KAFKA_PREFIX + "facility: ", facility, ", message: ", message);
            break;
        case LogLevel::LogWarning:
            if (heap && !func.isNull()) {
                vector<ConstantSP> args{new String("WARNING"), new String(facility), new String(message)};
                func->call(heap, args);
            }
            PLUGIN_LOG_WARN(KAFKA_PREFIX + "facility: ", facility, ", message: ", message);
            break;
        case LogLevel::LogNotice:
        case LogLevel::LogInfo:
            PLUGIN_LOG_INFO(KAFKA_PREFIX + "facility: ", facility, ", message: ", message);
            break;
        case LogLevel::LogDebug:
        default:
            PLUGIN_LOG(KAFKA_PREFIX + "facility: ", facility, ", message: ", message);
    }
}
void kafkaStatCallBack(KafkaHandleBase &handle, const std::string &json) {
    PLUGIN_LOG_INFO(KAFKA_PREFIX, handle.get_name(), " stat: ", json);
}

void kafkaDeliveryReportCallback(Producer& producer, const Message&) {
    std::ignore = producer;
    PLUGIN_LOG_INFO(KAFKA_PREFIX, __FUNCTION__);
}

void kafkaEventCallBack(KafkaHandleBase &handle, Event e) {
    PLUGIN_LOG_INFO(KAFKA_PREFIX, handle.get_name(), " event: ", e.get_name(), e.get_type(), e.get_stats());
}

Configuration createConf(ConstantSP &dict, const string &funcName, bool consumer, Heap *heap, FunctionDefSP func) {
    Configuration configuration;
    configuration.set_error_callback([=](KafkaHandleBase &handle, int error, const std::string &reason) {
        kafkaErrorCallback(handle, error, reason, heap, func);
    });
    configuration.set_log_callback(
        [=](KafkaHandleBase &handle, int level, const std::string &facility, const std::string &message) {
            kafkaLogCallback(handle, level, facility, message, heap, func);
        });
    configuration.set_stats_callback(kafkaStatCallBack);
#ifndef BUILD_ARM
    configuration.set_background_event_callback(kafkaEventCallBack);
#endif
    // if (!consumer) {
    //     configuration.set_delivery_report_callback(kafkaDeliveryReportCallback);
    // }

    bool hasGroupId = false;
    bool server = false;

    auto keys = dict->keys();
    for (auto i = 0; i < keys->size(); i++) {
        auto key = keys->get(i);
        auto value = dict->getMember(key);
        if (value->getType() == DT_STRING) {
            configuration.set(key->getString(), value->getString());
        } else if (value->getType() == DT_BOOL) {
            configuration.set(key->getString(), (bool)value->getBool() ? "true" : "false");
        } else {
            throw IllegalArgumentException(funcName, "some configurations are illegal");
        }
        if (key->getString() == "group.id") {
            hasGroupId = true;
        } else if (key->getString() == "metadata.broker.list" || key->getString() == "bootstrap.servers") {
            server = true;
        }
    }
    if (consumer && !hasGroupId) {
        throw IllegalArgumentException(funcName, "consumer need setting group.id");
    }
    if (!server) {
        throw IllegalArgumentException(funcName, "must setting metadata.broker.list or bootstrap.servers");
    }

    return configuration;
}

////////////////////////////////////////////////////////////////////////////////

void produceMsg(SmartPointer<Producer> producer, const string &topic, const string &key, ConstantSP value,
                KafkaMarshalType marshalType, int partition, bool force) {
    string usage = string("kafka::produce(producer, topic: string, key, value, json, [partition]) ");

    if (marshalType == PLAIN) {
        if (value->getCategory() != LITERAL || (value->getForm() != DF_VECTOR && value->getForm() != DF_SCALAR)) {
            throw RuntimeException(KAFKA_PREFIX +
                                   "if marshalType is 'PLAIN', value must be string scalar or string vector.");
        }
        MessageBuilder msg = MessageBuilder(topic);
        msg.key(key);
        if (partition != -1) {
            msg.partition(partition);
        }
        if (value->getForm() == DF_SCALAR) {
            string val = value->getString();
            msg.payload(val);
            producer->produce(msg);
        } else {  // only could be vector
            std::vector<DolphinString*> strBuf(Util::BUF_SIZE);
            INDEX len = value->size();
            INDEX start = 0;
            while (start < len) {
                int count = std::min(len - start, Util::BUF_SIZE);
                DolphinString **valueStrList = value->getStringConst(start, count, strBuf.data());
                for (int i = 0; i < count; ++i) {
                    string val = valueStrList[i]->getString();
                    msg.payload(val);
                    producer->produce(msg);
                }
                start += count;
            }
        }
        return;
    }

    long long valueSize = OperatorImp::memSize(value, nullptr)->getLong();
    if (force || valueSize * 2 < MESSAGE_SIZE || (value->getForm() != DF_TABLE && value->getForm() != DF_VECTOR)) {
        string valueStr = kafkaSerialize(value, marshalType);
        try {
            MessageBuilder msg = MessageBuilder(topic);
            msg.key(key).payload(valueStr);
            if (partition != -1) {
                msg.partition(partition);
            }
            producer->produce(std::move(msg));
        } catch (std::exception &e) {
            throw RuntimeException(KAFKA_PREFIX + e.what());
        }
    } else {
        if (value->getForm() == DF_TABLE) {
            int headLen = 0;
            int rowLen = 0;
            int columns = value->columns();
            int rows = value->rows();
            if (rows == 1) {  // HACK only with only one, force to produce
                produceMsg(producer, topic, key, value, marshalType, partition, true);
            }
            headLen = OperatorImp::memSize(value->keys(), nullptr)->getLong() * 2;
            rowLen = OperatorImp::memSize(value->values(), nullptr)->getLong() * 2;

            long long step = (MESSAGE_SIZE - headLen) / (rowLen / rows);
            step = step >= rows / 2 ? rows/2: step;
            step = step < 1 ? 1 : step;
            for (long long i = 0; i < rows; i += step) {
                auto pass = value->getWindow(0, columns, i, (i + step >= rows) ? (rows - i) : step);
                produceMsg(producer, topic, key, pass, marshalType, partition);
            }
        } else if (value->getForm() == DF_VECTOR) {
            VectorSP vector = value;
            int size = vector->size();
            long long totalSize = OperatorImp::memSize(value, nullptr)->getLong() * 2;
            if (size == 1) {  // HACK only with only one, force to produce
                produceMsg(producer, topic, key, value, marshalType, partition, true);
            }
            long long step = (MESSAGE_SIZE) / (totalSize / size);
            step = step >= size / 2 ? size/2: step;
            step = step < 1 ? 1 : step;
            for (long long i = 0; i < size; i += step) {
                auto pass = vector->getSubVector(i, (i + step >= size) ? (size - i) : step);
                produceMsg(producer, topic, key, pass, marshalType, partition);
            }
        }
    }
}

string stringSerialize(const ConstantSP &data, bool key) {
    if (data->getForm() == DF_SCALAR) {
        if (key && data->getType() != DT_STRING && data->getType() != DT_CHAR && data->getType() != DT_BOOL) {
            return "\"" + data->getString() + "\"";
        } else if (data->getType() == DT_BOOL)
            if ((int)data->getBool() == 1)
                return string("true");
            else
                return string("false");
        else if (data->isNull())
            return string("null");
        else if (data->getType() == DT_STRING || data->getType() == DT_CHAR)
            return "\"" + data->getString() + "\"";
        else if (data->getType() == DT_INT || data->getType() == DT_DOUBLE || data->getType() == DT_FLOAT ||
                 data->getType() == DT_SHORT || data->getType() == DT_LONG)
            return data->getString();
        else {
            return "\"" + data->getString() + "\"";
        }
    } else {
        return jsonSerialize(data);
    }
}

string jsonSerialize(const ConstantSP &data) {
    string result;
    if (data->getForm() == DF_VECTOR) {
        if (data->size() == 0) {
            return string("[]");
        }
        result += "[";
        result += stringSerialize(data->get(0));
        for (int i = 1; i < data->size(); i++) {
            result += ",";
            result += stringSerialize(data->get(i));
        }
        result += "]";
    } else if (data->getForm() == DF_DICTIONARY) {
        if (data->size() == 0) return string("{}");
        int length = data->size();

        ConstantSP dictKeys = data->keys();
        ConstantSP dictValues = data->values();

        result += "{";
        result += stringSerialize(dictKeys->get(length - 1), true);
        result += ":";
        result += stringSerialize(dictValues->get(length - 1));
        for (int i = length - 2; i >= 0; i--) {
            result += ",";
            result += stringSerialize(dictKeys->get(i), true);
            result += ":";
            result += stringSerialize(dictValues->get(i));
        }
        result += "}";
    } else if (data->getForm() == DF_TABLE) {
        if (data->size() == 0 || data->columns() == 0) return string("{}");
        ConstantSP dictKeys = data->keys();
        ConstantSP dictValues = data->values();
        result += "{";
        result += stringSerialize(dictKeys->get(0), true);
        result += ":";
        result += jsonSerialize(dictValues->get(0));
        for (int i = 1; i < data->columns(); i++) {
            result += ",";
            result += stringSerialize(dictKeys->get(i), true);
            result += ":";
            result += jsonSerialize(dictValues->get(i));
        }
        result += "}";
    } else if (data->getForm() == DF_SCALAR) {
        result += "[";
        result += stringSerialize(data);
        result += "]";
    } else
        throw IllegalArgumentException(__FUNCTION__,
                                       "Only scalar, vector, dictionary and table can be passed as json.");

    return result;
}

string kafkaSerialize(const ConstantSP &data, KafkaMarshalType marshalType) {
    if (data->getType() == DT_FUNCTIONDEF) {
        throw IllegalArgumentException(__FUNCTION__, "Can't pass function type.");
    }
    // plain marshalType would not use this function
    if (marshalType == KafkaMarshalType::JSON) {
        return jsonSerialize(data);
    } else {
        return dolphinSerialize(data);
    }
}

string dolphinSerialize(const ConstantSP &data) {
    string result;
    IO_ERR ret;
    DataOutputStreamSP outStream = new DataOutputStream();
    ConstantMarshalFactory marshallFactory(outStream);
    ConstantMarshal *marshall = marshallFactory.getConstantMarshal(data->getForm());
    marshall->start(data, true, ret);
    result += string(outStream->getBuffer(), outStream->size());

    ret = outStream->flush();
    if (ret != IO_ERR::OK) {
        throw IOException(ret);
    }
    return result;
}

////////////////////////////////////////////////////////////////////////////////

ConstantSP extractMsg(Message &msg, KafkaMarshalType marshalType) {
    auto topic = Util::createConstant(DT_STRING);
    auto partition = Util::createConstant(DT_INT);
    ConstantSP key, value;
    try {
        // key = kafkaDeserialize(msg.get_key());
        key = new String(string(msg.get_key()));
        if (marshalType == KafkaMarshalType::PLAIN) {
            value = new String(string(msg.get_payload()));
        } else {
            value = kafkaDeserialize(msg.get_payload(), marshalType);
        }
    } catch (std::exception &exception) {
        throw RuntimeException(KAFKA_PREFIX + exception.what());
    }
    auto ts = Util::createConstant(DT_TIMESTAMP);

    topic->setString(msg.get_topic());
    partition->setInt(msg.get_partition());
    if (msg.get_timestamp()) {
        ts->setLong(msg.get_timestamp()->get_timestamp().count());
    }

    auto res = Util::createVector(DT_ANY, 5);
    res->set(0, topic);
    res->set(1, key);
    res->set(2, value);
    res->set(3, partition);
    res->set(4, ts);
    return res;
}

VectorSP getMsg(Message &msg, KafkaMarshalType marshalType) {
    auto err_arg = Util::createConstant(DT_STRING);

    if (msg) {
        ConstantSP msg_arg;
        try {
            if (msg.get_error()) {
                if (msg.is_eof())
                    err_arg->setString("Broker: No more messages");
                else
                    err_arg->setString(msg.get_error().to_string());
                msg_arg = Util::createConstant(DT_VOID);
            } else {
                err_arg->setString("");
                msg_arg = extractMsg(msg, marshalType);
            }
        } catch (std::exception &ex) {
            msg_arg = Util::createConstant(DT_VOID);
            err_arg->setString("Parse message failed, " + string(ex.what()));
        } catch (...) {
            msg_arg = Util::createConstant(DT_VOID);
            err_arg->setString("Parse message failed.");
        }

        auto res = Util::createVector(DT_ANY, 2);
        res->set(0, err_arg);
        res->set(1, msg_arg);
        return res;
    } else {
        err_arg->setString("No more message");
        auto msg_arg = Util::createNullConstant(DT_ANY);
        auto res = Util::createVector(DT_ANY, 2);
        res->set(0, err_arg);
        res->set(1, msg_arg);
        return res;
    }
}

ConstantSP jsonDeserialize(const string &buffer, KafkaMarshalType marshalType) {
    const char *str = buffer.c_str();
    json data = json::parse(str);
    if (data.is_object()) {
        auto result = Util::createDictionary(DT_STRING, nullptr, DT_ANY, nullptr);
        for (json::iterator it = data.begin(); it != data.end(); ++it) {
            ConstantSP value;
            if (it.value().is_array() || it.value().is_object()) {
                value = kafkaDeserialize(it.value().dump(), marshalType);
            } else if (it.value().is_boolean()) {
                value = Util::createConstant(DT_BOOL);
                value->setBool(0, (bool)*it);
            } else if (it.value().is_null()) {
                value = Util::createNullConstant(DT_ANY);
            } else if (it.value().is_number_integer() || it.value().is_number_unsigned()) {
                if (*it > 0x7fffffffffffffffL) {
                    PLUGIN_LOG_INFO(KAFKA_PREFIX + "The integer is too large and it will be cast to string.");
                    value = Util::createConstant(DT_STRING);
                    string temp = it.value().dump();
                    value->setString(temp.substr(0, temp.length()));
                } else if (*it > 0x7fffffff) {
                    value = Util::createConstant(DT_LONG);
                    value->setLong(*it);
                } else {
                    value = Util::createConstant(DT_INT);
                    value->setInt(*it);
                }
            } else if (it.value().is_number_float()) {
                value = Util::createConstant(DT_DOUBLE);
                value->setDouble(*it);
            } else if (it.value().is_string()) {
                value = Util::createConstant(DT_STRING);
                string temp = it.value().dump();
                value->setString(temp.substr(1, temp.length() - 2));
            } else {
                PLUGIN_LOG_INFO(KAFKA_PREFIX + string(*it) + ":un defined data type.");
                value = Util::createNullConstant(DT_ANY);
            }
            auto key = Util::createConstant(DT_STRING);
            key->setString(it.key());
            result->set(key, value);
        }
        return result;
    } else if (data.is_array()) {
        auto result = Util::createVector(DT_ANY, 0);
        for (json::iterator it = data.begin(); it != data.end(); ++it) {
            ConstantSP value;
            if (it->is_number_integer() || it->is_number_unsigned()) {
                if (*it > 0x7fffffffffffffffL) {
                    PLUGIN_LOG_INFO(KAFKA_PREFIX + "The integer is too large and it will be cast to string.");
                    value = Util::createConstant(DT_STRING);
                    string temp = it.value().dump();
                    value->setString(temp.substr(0, temp.length()));
                } else if (*it > 0x7fffffff) {
                    value = Util::createConstant(DT_LONG);
                    value->setLong(*it);
                } else {
                    value = Util::createConstant(DT_INT);
                    value->setInt(*it);
                }
            } else if (it->is_number_float()) {
                value = Util::createConstant(DT_DOUBLE);
                value->setDouble(*it);
            } else if (it->is_object() || it->is_array()) {
                value = kafkaDeserialize(it.value().dump(), marshalType);
            } else if (it->is_null()) {
                value = Util::createNullConstant(DT_ANY);
            } else if (it->is_boolean()) {
                value = Util::createConstant(DT_BOOL);
                value->setBool(0, (bool)*it);
            } else if (it->is_string()) {
                value = Util::createConstant(DT_STRING);
                string temp = it.value().dump();
                value->setString(temp.substr(1, temp.length() - 2));
            } else {
                PLUGIN_LOG_INFO(KAFKA_PREFIX + string(*it) + ":un defined data type.");
                value = Util::createNullConstant(DT_ANY);
            }
            result->append(value);
        }
        return result;
    } else {
        throw RuntimeException(KAFKA_PREFIX + "invalid json input.");
    }
}

ConstantSP dolphindbDeserialize(const string &buffer) {
    short flag;
    IO_ERR ret;
    DataInputStreamSP in = new DataInputStream(buffer.c_str(), buffer.length());
    ret = in->readShort(flag);
    auto data_form = static_cast<DATA_FORM>(flag >> 8);
    ConstantUnmarshalFactory factory(in, nullptr);
    ConstantUnmarshal *unmarshall = factory.getConstantUnmarshal(data_form);
    if (unmarshall == nullptr) {
        throw RuntimeException(KAFKA_PREFIX + "Failed to parse the incoming object: " + buffer +
                               ". Please poll the stream by kafka::pollByteStream.");
    }
    if (!unmarshall->start(flag, true, ret)) {
        unmarshall->reset();
        throw IOException(KAFKA_PREFIX + "Failed to parse the incoming object with IO error type " +
                          std::to_string(ret));
    }

    ConstantSP result = unmarshall->getConstant();

    return result;
}

ConstantSP kafkaDeserialize(const string &buffer, KafkaMarshalType marshalType) {
    if (buffer == "") {
        auto empty = Util::createNullConstant(DT_ANY);
        return empty;
    }
    if (marshalType == KafkaMarshalType::AUTO) {
        if (buffer[0] == '{' || buffer[0] == '[') {
            marshalType = KafkaMarshalType::JSON;
        } else {
            marshalType = KafkaMarshalType::DOLPHINDB;
        }
    }
    try {
        if (marshalType == KafkaMarshalType::JSON) {
            return jsonDeserialize(buffer);
        } else {
            return dolphindbDeserialize(buffer);
        }
    } catch (std::exception &e) {
        string errMsg = e.what();
        if (errMsg.find(KAFKA_PREFIX) == errMsg.npos) {
            throw RuntimeException(KAFKA_PREFIX + e.what());
        } else {
            throw RuntimeException(e.what());
        }
    }
}
}  // namespace KafkaUtil