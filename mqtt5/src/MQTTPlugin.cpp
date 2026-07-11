#include "MQTTPlugin.h"

#include "MQTTConnect.h"

ConstantSP mqttConnect(Heap *heap, const vector<ConstantSP> &args) {
    return MQTTPlugin::MQTTInstance::getInstance().mqttConnect(heap, args);
}
ConstantSP mqttClose(Heap *, const vector<ConstantSP> &args) {
    return MQTTPlugin::MQTTInstance::getInstance().mqttClose(args);
}
ConstantSP mqttPublish(Heap *, const vector<ConstantSP> &args) {
    return MQTTPlugin::MQTTInstance::getInstance().mqttPublish(args);
}
ConstantSP mqttSubscribe(Heap *heap, const vector<ConstantSP> &args) {
    return MQTTPlugin::MQTTInstance::getInstance().mqttSubscribe(heap, args);
}
ConstantSP mqttUnsubscribe(Heap *, const vector<ConstantSP> &args) {
    return MQTTPlugin::MQTTInstance::getInstance().mqttUnsubscribe(args);
}
ConstantSP getSubscriberStat(Heap *, const vector<ConstantSP> &) {
    return MQTTPlugin::MQTTInstance::getInstance().getSubscriberStat();
}

namespace MQTTPlugin {

static void publishOnClose(Heap *, vector<ConstantSP> &args) {
    MQTTConnection *conn = (MQTTConnection *)(args[0]->getLong());
    if (conn != nullptr) {
        delete conn;
        args[0]->setLong(0);
    }
}

static void subscribeOnClose(Heap *, vector<ConstantSP> &) {
    // do nothing
}

static void parseConfig(const DictionarySP &config, string &username, string &password, string &clientId,
                        int &mqttVersion, bool &useSSL, bool &enableServerCertAuth, string &trustStore, int &qos,
                        int &maxBufferedMessages) {
    if (config->getKeyType() != DT_STRING || config->getType() != DT_ANY) {
        throw RuntimeException(PLUGIN_NAME + " Argument config dictionary must be string -> any type");
    }
    ConstantSP usernameObj = config->getMember("username");
    ConstantSP passwordObj = config->getMember("password");
    ConstantSP clientIdObj = config->getMember("clientID");
    ConstantSP mqttVersionObj = config->getMember("mqttVersion");
    ConstantSP useSSLObj = config->getMember("useSSL");
    ConstantSP enableServerCertAuthObj = config->getMember("enableServerCertAuth");
    ConstantSP trustStoreObj = config->getMember("trustStore");
    ConstantSP qosObj = config->getMember("qos");
    ConstantSP maxBufferedMessagesObj = config->getMember("maxBufferedMessages");

    if (!usernameObj->isNull()) {
        if (usernameObj->getType() != DT_STRING || usernameObj->getForm() != DF_SCALAR) {
            throw RuntimeException(PLUGIN_NAME + " Argument config.username must be a string");
        }
        username = usernameObj->getString();
        if (passwordObj->isNull()) {
            throw RuntimeException(PLUGIN_NAME + " Argument config.password is required when username is not empty");
        }
        if (passwordObj->getType() != DT_STRING || passwordObj->getForm() != DF_SCALAR) {
            throw RuntimeException(PLUGIN_NAME + " Argument config.password must be a string");
        }
        password = passwordObj->getString();
    }
    if (!clientIdObj->isNull()) {
        if (clientIdObj->getType() != DT_STRING || clientIdObj->getForm() != DF_SCALAR) {
            throw RuntimeException(PLUGIN_NAME + " Argument config.clientID must be a string");
        }
        clientId = clientIdObj->getString();
    }
    if (!mqttVersionObj->isNull()) {
        if (mqttVersionObj->getType() != DT_INT || mqttVersionObj->getForm() != DF_SCALAR) {
            throw RuntimeException(PLUGIN_NAME + " Argument config.mqttVersion must be an int");
        }
        int version = mqttVersionObj->getInt();
        if (version == 0) {
            mqttVersion = MQTTVERSION_DEFAULT;
        } else if (version == 3) {
            mqttVersion = MQTTVERSION_3_1;
        } else if (version == 4) {
            mqttVersion = MQTTVERSION_3_1_1;
        } else if (version == 5) {
            mqttVersion = MQTTVERSION_5;
        } else {
            throw RuntimeException(PLUGIN_NAME + " Argument config.mqttVersion must be 0, 3, 4, or 5");
        }
    }
    if (!useSSLObj->isNull()) {
        if (useSSLObj->getType() != DT_BOOL || useSSLObj->getForm() != DF_SCALAR) {
            throw RuntimeException(PLUGIN_NAME + " Argument config.useSSL must be a bool");
        }
        useSSL = useSSLObj->getBool();
    }
    if (!enableServerCertAuthObj->isNull()) {
        if (enableServerCertAuthObj->getType() != DT_BOOL || enableServerCertAuthObj->getForm() != DF_SCALAR) {
            throw RuntimeException(PLUGIN_NAME + " Argument config.enableServerCertAuth must be a bool");
        }
        enableServerCertAuth = enableServerCertAuthObj->getBool();
    }
    if (!trustStoreObj->isNull()) {
        if (trustStoreObj->getType() != DT_STRING || trustStoreObj->getForm() != DF_SCALAR) {
            throw RuntimeException(PLUGIN_NAME + " Argument config.trustStore must be a string");
        }
        trustStore = trustStoreObj->getString();
    }
    if (!qosObj->isNull()) {
        if (qosObj->getType() != DT_INT || qosObj->getForm() != DF_SCALAR) {
            throw RuntimeException(PLUGIN_NAME + " Argument config.qos must be an int");
        }
        qos = qosObj->getInt();
        if (qos < 0 || qos > 2) {
            throw RuntimeException(PLUGIN_NAME + " Argument config.qos must be 0, 1, or 2");
        }
    }
    if (!maxBufferedMessagesObj->isNull()) {
        if (maxBufferedMessagesObj->getType() != DT_INT || maxBufferedMessagesObj->getForm() != DF_SCALAR) {
            throw RuntimeException(PLUGIN_NAME + " Argument config.maxBufferedMessages must be an int");
        }
        maxBufferedMessages = maxBufferedMessagesObj->getInt();
        if (maxBufferedMessages <= 0) {
            throw RuntimeException(PLUGIN_NAME + " Argument config.maxBufferedMessages must be greater than 0");
        }
    }
}

ConstantSP MQTTInstance::mqttConnect(Heap *heap, const vector<ConstantSP> &args) {
    if (args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR) {
        throw RuntimeException(PLUGIN_NAME + " Argument uri must be a string");
    }

    string username, password, clientId, trustStore;
    int mqttVersion = MQTTVERSION_DEFAULT, qos = 0, maxBufferedMessages = 100;
    bool useSSL = false, enableServerCertAuth = false;
    if (args.size() > 1 && !args[1]->isNull()) {
        if (args[1]->getForm() != DF_DICTIONARY) {
            throw RuntimeException(PLUGIN_NAME + " Argument config must be a dictionary");
        }
        parseConfig(args[1], username, password, clientId, mqttVersion, useSSL, enableServerCertAuth, trustStore, qos,
                    maxBufferedMessages);
    }

    std::unique_ptr<MQTTConnection> conn(new MQTTConnection(args[0]->getString(), username, password, clientId,
                                                            mqttVersion, useSSL, enableServerCertAuth, trustStore, qos,
                                                            maxBufferedMessages, "", nullptr));
    conn->connect();
    FunctionDefSP onClose = Util::createSystemProcedure(PUB_RESOUCE_NAME + " onClose()", publishOnClose, 1, 1);
    return Util::createResource((long long)conn.release(), PUB_RESOUCE_NAME, onClose, heap->currentSession());
}

ConstantSP MQTTInstance::mqttClose(const vector<ConstantSP> &args) {
    if (args[0]->getType() != DT_RESOURCE) {
        throw RuntimeException(PLUGIN_NAME + " Invalid conn argument, should be resource type.");
    }
    if (args[0]->getString() != PUB_RESOUCE_NAME) {
        throw RuntimeException(PLUGIN_NAME + " Argument conn must be a " + PUB_RESOUCE_NAME + " resouce.");
    }

    MQTTConnection *conn = (MQTTConnection *)(args[0]->getLong());
    if (conn == nullptr) {
        throw RuntimeException(PLUGIN_NAME + " Argument conn is invalid.");
    }
    LockGuard<Mutex> lock(conn->getLock());
    if (!conn->close()) throw RuntimeException(PLUGIN_NAME + " This connection is already closed.");
    return new Void();
}

ConstantSP MQTTInstance::mqttPublish(const vector<ConstantSP> &args) {
    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != PUB_RESOUCE_NAME) {
        throw RuntimeException(PLUGIN_NAME + " Argument conn must be a mqtt publish connection.");
    }
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR) {
        throw RuntimeException(PLUGIN_NAME + " Argument topic must be a string");
    }
    string topic = args[1]->getString();
    if (topic.length() == 0) {
        throw RuntimeException(PLUGIN_NAME + " The length of topic should greater than 0");
    }
    MQTTConnection *conn = (MQTTConnection *)(args[0]->getLong());
    if (conn == nullptr) {
        throw RuntimeException(PLUGIN_NAME + " Argument conn is invalid.");
    }
    LockGuard<Mutex> lock(conn->getLock());
    if (!conn->isConnected()) {
        throw RuntimeException(PLUGIN_NAME + " Argument conn is not connected.");
    }

    string message;
    if (args[2]->getType() == DT_STRING && args[2]->isArray()) {
        for (int i = 0; i < args[2]->size(); i++) {
            message = args[2]->get(i)->getString();
            conn->publish(topic, message);
        }
    } else if (args[2]->getType() == DT_STRING && args[2]->getForm() == DF_SCALAR) {
        message = args[2]->getString();
        conn->publish(topic, message);
    } else {
        throw RuntimeException(PLUGIN_NAME + " Argument message must be a string scalar or string vector.");
    }
    return new Void();
}

ConstantSP MQTTInstance::mqttSubscribe(Heap *heap, const vector<ConstantSP> &args) {
    if (args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR) {
        throw RuntimeException(PLUGIN_NAME + " Argument uri must be a string");
    }
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR) {
        throw RuntimeException(PLUGIN_NAME + " Argument topic must be a string");
    }
    if (args[2]->getType() != DT_FUNCTIONDEF || args[2]->getForm() != DF_SCALAR) {
        throw RuntimeException(PLUGIN_NAME + " Argument handler must be a function");
    }
    FunctionDefSP handler = args[2];
    if (handler->getMinParamCount() != 2 || handler->getMaxParamCount() != 2) {
        throw RuntimeException(PLUGIN_NAME + " Argument handler must have 2 parameters");
    }

    string username, password, clientId, trustStore;
    int mqttVersion = MQTTVERSION_DEFAULT, qos = 0, maxBufferedMessages = 100;
    bool useSSL = false, enableServerCertAuth = false;
    if (args.size() > 3 && !args[3]->isNull()) {
        if (args[3]->getForm() != DF_DICTIONARY) {
            throw RuntimeException(PLUGIN_NAME + " Argument config must be a dictionary");
        }
        parseConfig(args[3], username, password, clientId, mqttVersion, useSSL, enableServerCertAuth, trustStore, qos,
                    maxBufferedMessages);
    }

    std::unique_ptr<MQTTConnection> cup(new MQTTConnection(args[0]->getString(), username, password, clientId,
                                                           mqttVersion, useSSL, enableServerCertAuth, trustStore, qos,
                                                           maxBufferedMessages, args[1]->getString(), handler));
    cup->subscribe(heap);
    FunctionDefSP onClose(Util::createSystemProcedure(SUB_RESOUCE_NAME + " onClose()", subscribeOnClose, 1, 1));
    ConstantSP conn = Util::createResource((long long)cup.release(), SUB_RESOUCE_NAME, onClose, heap->currentSession());
    LockGuard<Mutex> guard(&subscribersLock_);
    subscribers_->set(std::to_string(conn->getLong()), conn);
    return conn;
}

ConstantSP MQTTInstance::mqttUnsubscribe(const vector<ConstantSP> &args) {
    string key;
    switch (args[0]->getType()) {
        case DT_RESOURCE:
            if (args[0]->getString() != PUB_RESOUCE_NAME && args[0]->getString() != SUB_RESOUCE_NAME) {
                throw RuntimeException(PLUGIN_NAME + " Argument conn must be a " + PUB_RESOUCE_NAME + " or " +
                                       SUB_RESOUCE_NAME + " resouce.");
            }
            key = std::to_string(args[0]->getLong());
            break;
        case DT_STRING:
            key = args[0]->getString();
            break;
        default:
            throw RuntimeException(PLUGIN_NAME + " Invalid conn type, should be string or resource.");
    }

    LockGuard<Mutex> guard(&subscribersLock_);
    ConstantSP conn = subscribers_->getMember(key);
    if (conn->isNull()) {
        throw RuntimeException(PLUGIN_NAME + " Can't find connection with key " + key + ".");
    }

    MQTTConnection *ptr = reinterpret_cast<MQTTConnection *>(conn->getLong());
    if (ptr == nullptr) {
        throw RuntimeException(PLUGIN_NAME + " Invalid connection with key " + key + ".");
    }
    LockGuard<Mutex> lock(ptr->getLock());
    if (subscribers_->remove(new String(key)) && ptr != nullptr) {
        delete ptr;
    }
    return new Void();
}

ConstantSP MQTTInstance::getSubscriberStat() {
    LockGuard<Mutex> guard(&subscribersLock_);
    int size = subscribers_->size();
    ConstantSP subscriptionIdVec = Util::createVector(DT_STRING, size);
    ConstantSP userVec = Util::createVector(DT_STRING, size);
    ConstantSP uriVec = Util::createVector(DT_STRING, size);
    ConstantSP topicVec = Util::createVector(DT_STRING, size);
    ConstantSP createTimestampVec = Util::createVector(DT_TIMESTAMP, size);
    ConstantSP receivedPacketsVec = Util::createVector(DT_LONG, size);
    ConstantSP lastErrMsgVec = Util::createVector(DT_STRING, size);
    ConstantSP lastErrTimeVec = Util::createVector(DT_TIMESTAMP, size);

    VectorSP keys = subscribers_->keys();
    for (int i = 0; i < keys->size(); i++) {
        string key = keys->getString(i);
        subscriptionIdVec->setString(i, key);
        ConstantSP conn = subscribers_->getMember(key);
        if (conn->isNull()) {
            throw RuntimeException(PLUGIN_NAME + " Can't find connection with key " + key + ".");
        }

        MQTTConnection *ptr = reinterpret_cast<MQTTConnection *>(conn->getLong());
        if (ptr == nullptr) {
            throw RuntimeException(PLUGIN_NAME + " Argument conn " + key + " is invalid.");
        }
        LockGuard<Mutex> lock(ptr->getLock());
        userVec->setString(i, ptr->getSessionUserId());
        uriVec->setString(i, ptr->getUri());
        topicVec->setString(i, ptr->getTopic());
        createTimestampVec->setLong(i, ptr->getCreatedTime());
        receivedPacketsVec->setLong(i, ptr->getReceivedPacketNum());
        string lastErrMsg;
        long long lastErrTime;
        ptr->getLastError(lastErrMsg, lastErrTime);
        lastErrMsgVec->setString(i, lastErrMsg);
        if (lastErrTime == 0) {
            lastErrTimeVec->setNull(i);
        } else {
            lastErrTimeVec->setLong(i, lastErrTime);
        }
    }

    static const vector<string> names = {"subscriptionId",  "user",           "uri", "topic",
                                         "createTimestamp", "receivedPackets", "lastErrMsg", "lastErrTime"};
    vector<ConstantSP> cols = {subscriptionIdVec,  userVec,            uriVec,        topicVec,
                               createTimestampVec, receivedPacketsVec, lastErrMsgVec, lastErrTimeVec};
    return Util::createTable(names, cols);
}

}  // namespace MQTTPlugin
