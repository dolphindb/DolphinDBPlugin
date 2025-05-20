/*
 * sub_client.cpp
 *
 *  Created on: May 14, 2019
 *      Author: qianxj
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "ScalarImp.h"
#include "Util.h"
#include "client.h"
#include "ddbplugin/Plugin.h"
#include "mqtt.h"

using namespace std;
using mqtt::SubConnection;
/**
 * @brief The function that would be called whenever a PUBLISH is received.
 */
static void subCallback(void **socketHandle, struct mqtt_response_publish *published) {
    if (socketHandle == nullptr || *socketHandle == nullptr) {
        PLUGIN_LOG_INFO("[PluginMQTT]: Socket handle is null or invalid.");
        return;
    }
    if (published == nullptr) {
        PLUGIN_LOG_INFO("[PluginMQTT]: Published message is null.");
        return;
    }
    SubConnection *cp = (SubConnection *)(*socketHandle);
    if (cp == nullptr) {
        // throw RuntimeException("Connection is not found.");
        PLUGIN_LOG_INFO("[PluginMQTT]: Connection is not found.");
        return;
    }

    try {
        std::string topic((const char *)published->topic_name, published->topic_name_size);
        std::string msg((const char *)published->application_message, published->application_message_size);
        cp->handleMsg(topic, msg);
    } catch (const std::exception &e) {
        PLUGIN_LOG_INFO("[PluginMQTT]: Exception occurred while processing message: %s", e.what());
    }
}

/**
 * @brief Safelty closes the \p sockfd and cancels the \p clientDaemon before \c exit.
 */

static void mqttConnectionOnClose(Heap *heap, vector<ConstantSP> &args) {
    PLUGIN_LOG_INFO("[PluginMQTT]: mqttConnectionOnClose");
}

/**
 * @brief Safelty closes the \p sockfd and cancels the \p clientDaemon before \c exit.
 */

ConstantSP mqttClientSub(Heap *heap, vector<ConstantSP> &args) {
    std::string usage =
        "Usage: subscribe(host, port, topic, [parser], handler, [username], [password], [recvbufSize=20480], "
        "[config]).";
    std::string userName;
    std::string password;

    // parse args first
    FunctionDefSP parser;

    if (args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "host must be a string");
    }

    if (args[1]->getType() != DT_INT || args[1]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "port must be an integer");
    }
    if (args[1]->getInt() < 1 || args[1]->getInt() > 65535) {
        throw IllegalArgumentException(__FUNCTION__, usage + "port must be an integer(1-65535)");
    }

    if (args[2]->getType() != DT_STRING || args[2]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "topic must be a string");
    }
    string topic = args[2]->getString();
    if (topic.length() == 0) {
        throw IllegalArgumentException(__FUNCTION__, usage + " the length of topic shoule more than 0");
    }

    if (args[4]->isTable()) {
        if (args[3]->getType() != DT_FUNCTIONDEF) {
            throw IllegalArgumentException(__FUNCTION__, usage + "parser must be an function.");
        }
        parser = args[3];
        // string functionName = parser->getName();
        // if (functionName != "parseJson" && functionName != "parseCsv") {
        //     throw IllegalArgumentException(__FUNCTION__, usage + "parser should be parseJson or parseCsv.");
        // }
    }

    if (!args[4]->isTable() && args[4]->getType() != DT_FUNCTIONDEF) {
        throw IllegalArgumentException(__FUNCTION__, usage + "handler must be a table or an unary function.");
    }
    if (args.size() > 5 && !args[5]->isNull()) {
        if (args[5]->getType() != DT_STRING || args[5]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, usage + "username must be a string");
        }
        userName = args[5]->getString();
    }
    if (args.size() > 6 && !args[6]->isNull()) {
        if (args[6]->getType() != DT_STRING || args[6]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, usage + "password must be a string");
        }
        password = args[6]->getString();
    }
    if (!userName.empty() && (args.size() <= 6 || (args[6]->isNull() && args[6]->getType() == DT_VOID))) {
        throw IllegalArgumentException(__FUNCTION__, usage + "password can't be ignored when username is set");
    }

    int recvbufSize = 20480;
    if (args.size() > 7 && !args[7]->isNull()) {
        if (!args[7]->isScalar() || args[7]->getType() != DT_INT) {
            throw IllegalArgumentException(__FUNCTION__, usage + "the type of recvbufSize must be int");
        }
        recvbufSize = args[7]->getInt();

        if (recvbufSize <= 0) {
            throw IllegalArgumentException(__FUNCTION__, usage + "recvbufSize must be a positive number");
        }
    }

    // config
    int sendBufSize = 20480;
    std::string subscribeID;
    bool isAsync = false;
    if (args.size() > 8 && !args[8]->isNothing()) {
        if (args[8]->getForm() != DF_DICTIONARY) {
            throw IllegalArgumentException(__FUNCTION__, usage + "config must be a dictionary");
        }
        DictionarySP dic = args[8];
        if (dic->getKeyType() != DT_STRING) {
            throw IllegalArgumentException(__FUNCTION__,
                                           usage + "config must be a dictionary whose key type is string");
        }

        ConstantSP sendBufSizeConstant = dic->getMember("sendBufSize");
        if (!sendBufSizeConstant->isNull()) {
            if (sendBufSizeConstant->getType() != DT_INT || sendBufSizeConstant->getForm() != DF_SCALAR) {
                throw IllegalArgumentException(__FUNCTION__, usage + "config sendBufSize must be a INT scalar");
            }
            sendBufSize = sendBufSizeConstant->getInt();
        }

        ConstantSP ID = dic->getMember("subscribeID");
        if (!ID->isNull()) {
            if (ID->getType() != DT_STRING || ID->getForm() != DF_SCALAR) {
                throw IllegalArgumentException(__FUNCTION__, usage + "config subscribeID must be a STRING scalar");
            }
            subscribeID = ID->getString();
        }

        ConstantSP asyncFlag = dic->getMember("asyncFlag");
        if (!asyncFlag->isNull()) {
            if (asyncFlag->getType() != DT_BOOL || asyncFlag->getForm() != DF_SCALAR) {
                throw IllegalArgumentException(__FUNCTION__, usage + "config asyncFlag must be a bool scalar");
            }
            isAsync = asyncFlag->getBool();
        }
    }

    if (sendBufSize <= 0 || recvbufSize <= 0) {
        throw IllegalArgumentException(__FUNCTION__, usage + "config sendBufSize and recvbufSize must be positive.");
    }
    std::unique_ptr<SubConnection> cup(new SubConnection(args[0]->getString(), args[1]->getInt(), args[2]->getString(),
                                                         parser, args[4], userName, password, heap, sendBufSize,
                                                         recvbufSize, subscribeID, isAsync));
    if (isAsync) {
        cup->startAsyncHandleMsgThread();
    }
    FunctionDefSP onClose(Util::createSystemProcedure("mqtt sub connection onClose()", mqttConnectionOnClose, 1, 1));
    ConstantSP conn =
        Util::createResource((long long)cup.release(), "mqtt subscribe connection", onClose, heap->currentSession());
    LockGuard<Mutex> guard(&mqttConn::CONN_MUTEX_LOCK);
    mqttConn::CONN_DICT->set(std::to_string(conn->getLong()), conn);
    return conn;
}

ConstantSP mqttClientStopSub(const ConstantSP &handle, const ConstantSP &b) {
    // parse args first
    std::string usage = "Usage: close(connection or connection ID). ";
    SubConnection *sc = NULL;
    string key;
    ConstantSP conn;
    LockGuard<Mutex> guard(&mqttConn::CONN_MUTEX_LOCK);
    switch (handle->getType()) {
        case DT_RESOURCE:
            sc = (SubConnection *)(handle->getLong());
            key = std::to_string(handle->getLong());
            conn = mqttConn::CONN_DICT->getMember(key);
            if (conn->isNothing()) throw IllegalArgumentException(__FUNCTION__, "Invalid connection object.");
            break;
        case DT_STRING:
            key = handle->getString();
            conn = mqttConn::CONN_DICT->getMember(key);
            if (conn->isNothing())
                throw IllegalArgumentException(__FUNCTION__, "Invalid connection string.");
            else
                sc = (SubConnection *)(conn->getLong());
            break;
        case DT_LONG:
            key = std::to_string(handle->getLong());
            conn = mqttConn::CONN_DICT->getMember(key);
            if (conn->isNothing())
                throw IllegalArgumentException(__FUNCTION__, "Invalid connection integer.");
            else
                sc = (SubConnection *)(conn->getLong());
            break;
        default:
            throw IllegalArgumentException(__FUNCTION__, "Invalid connection object.");
    }

    bool bRemoved = mqttConn::CONN_DICT->remove(new String(key));
    if (bRemoved && sc != nullptr) {
        delete sc;
    }
    return new Int(MQTT_OK);
}
ConstantSP getSubscriberStat(const ConstantSP &handle, const ConstantSP &b) {
    LockGuard<Mutex> guard(&mqttConn::CONN_MUTEX_LOCK);
    int size = mqttConn::CONN_DICT->size();
    ConstantSP connectionIdVec = Util::createVector(DT_STRING, size);
    ConstantSP userVec = Util::createVector(DT_STRING, size);
    ConstantSP hostVec = Util::createVector(DT_STRING, size);
    ConstantSP portVec = Util::createVector(DT_INT, size);
    ConstantSP topicVec = Util::createVector(DT_STRING, size);
    ConstantSP timestampVec = Util::createVector(DT_TIMESTAMP, size);
    ConstantSP recvVec = Util::createVector(DT_LONG, size);
    VectorSP keys = mqttConn::CONN_DICT->keys();
    for (int i = 0; i < keys->size(); i++) {
        string key = keys->getString(i);
        connectionIdVec->setString(i, key);
        ConstantSP conn = mqttConn::CONN_DICT->getMember(key);
        SubConnection *sc = (SubConnection *)(conn->getLong());
        hostVec->setString(i, sc->getHost());
        portVec->setInt(i, sc->getPort());
        topicVec->setString(i, sc->getTopic());
        recvVec->setLong(i, sc->getRecv());
        timestampVec->setLong(i, sc->getCreateTime());
        userVec->setString(i, sc->session->getUser()->getUserId());
    }

    vector<string> colNames = {"subscriptionId", "user", "host", "port", "topic", "createTimestamp", "receivedPackets"};
    vector<ConstantSP> cols = {connectionIdVec, userVec, hostVec, portVec, topicVec, timestampVec, recvVec};
    return Util::createTable(colNames, cols);
}
/// Connection
namespace mqtt {
class SyncData : public Runnable {
  public:
    SyncData(SubConnection *connection, const SmartPointer<Mutex> &lockClient,
             SmartPointer<ConditionalNotifier> freeNotifier)
        : connection_(connection), lockClient_(lockClient), freeNotifier_(freeNotifier){};
    ~SyncData() override = default;
    void run() override;

  private:
    SubConnection *connection_;
    SmartPointer<Mutex> lockClient_;
    SmartPointer<ConditionalNotifier> freeNotifier_;
};

void SyncData::run() {
    while (!connection_->isClosed()) {
        try {
            connection_->setReceived();
            mqtt_client *client = connection_->getClient();
            while (connection_->received()) {
                connection_->resetReceived();
                MQTTErrors ret = mqtt_sync(client);
                int currentRetryCount = 0;
                while (ret == MQTT_ERROR_SOCKET_ERROR && currentRetryCount < MAX_RETRY_COUNT) {
                    PLUGIN_LOG_INFO("SyncDdata:: ret:", mqtt_error_str(ret));
                    connection_->reconnect();
                    Util::sleep(500);
                    ret = mqtt_sync(client);
                    currentRetryCount++;
                }
                if (currentRetryCount == MAX_RETRY_COUNT) {
                    PLUGIN_LOG_ERR(LOG_PRE_STR, " sub reconnect failed.");
                }
                freeNotifier_->notify();
            }
        } catch (exception &e) {
            std::string errMsg(e.what());
            PLUGIN_LOG_ERR(LOG_PRE_STR + " mqtt subscribe refresh connection failed, error message is <", errMsg, ">");
        } catch (...) {
            PLUGIN_LOG_ERR(LOG_PRE_STR + " mqtt subscribe refresh connection failed.");
        }
        usleep(100000U);
    }
    PLUGIN_LOG_INFO("[PluginMQTT]: SyncData:run thread exit");
}

SubConnection::~SubConnection() {
    isClosed_ = true;
    if (!asyncHandleMsgThread_.isNull()) {
        asyncHandleMsgThread_->join();
    }
    clientDaemon_->join();
    PLUGIN_LOG_INFO("[PluginMQTT]: SubConnection is freed");
    sockfd_->close();
}

SubConnection::SubConnection(const std::string &hostname, int port, const std::string &topic,
                             const FunctionDefSP &parser, const ConstantSP &handler, const std::string &userName,
                             const std::string &password, Heap *pHeap, int sendbufSize, int recvbufSize,
                             const std::string clientID, bool isAsync)
    : ConnctionBase(new ConditionalNotifier()),
      host_(hostname),
      port_(port),
      topic_(topic),
      isAsync_(isAsync),
      parser_(parser),
      handler_(handler),
      pHeap_(pHeap),
      userName_(userName),
      password_(password),
      clientID_(clientID),
      sendbufSize_(sendbufSize),
      recvbufSize_(recvbufSize),
      sendbuf_(new uint8_t[sendbufSize]),
      recvbuf_(new uint8_t[recvbufSize]) {
    sockfd_ = new Socket(hostname, port, false);
    IO_ERR ret = sockfd_->connect();
    if (ret != OK && ret != INPROGRESS) {
        throw RuntimeException("Failed to connect. ");
    }
    if (ret == INPROGRESS &&
        !checkConnectStatus(sockfd_->getHandle())) {  // handle EINPROGRESS on non-block TCP connection
        throw RuntimeException("Failed to connect.");
    }

    try {
        mqtt_init(&client_, sockfd_->getHandle(), sendbuf_.get(), sendbufSize_, recvbuf_.get(), recvbufSize_,
                  subCallback);
        std::string client_id =
            clientID_.empty() ? ("ddb_mqtt_plugin_sub" + std::to_string(sockfd_->getHandle())) : clientID_;
        uint8_t connect_flags = MQTT_CONNECT_CLEAN_SESSION;
        if (userName == "") {
            mqtt_connect(&client_, client_id.c_str(), NULL, NULL, 0, NULL, NULL, connect_flags, 400);
        } else {
            mqtt_connect(&client_, client_id.c_str(), NULL, NULL, 0, userName.c_str(), password.c_str(), connect_flags,
                         400);
        }
        if (client_.error != MQTT_OK) {
            throw RuntimeException(mqtt_error_str(client_.error));
        }
        checkConnack(&client_);

        lockClient_ = new Mutex();
        /* start a thread to refresh the client (handle egress and ingree clien traffic) */
        SmartPointer<SyncData> syncData = new SyncData(this, lockClient_, freeNotifier_);
        clientDaemon_ = new Thread(syncData);
        if (!clientDaemon_->isStarted()) {
            clientDaemon_->start();
        }

        connected_ = true;
        recv = 0;
        client_.publish_response_callback_state = this;
        createTime_ = Util::getEpochTime();

        /* subscribe */
        mqtt_subscribe(&client_, topic.c_str(), 0);

        /* check for errors */
        if (client_.error != MQTT_OK) {
            throw RuntimeException(mqtt_error_str(client_.error));
        }
        session = pHeap->currentSession()->copy();
        session->setUser(pHeap->currentSession()->getUser());
    } catch (exception &e) {
        string errMsg(e.what());
        PLUGIN_LOG_ERR(LOG_PRE_STR + " mqtt subscribe init failed, error message is <", errMsg, ">");
        throw RuntimeException(LOG_PRE_STR + " mqtt subscribe init failed, error message is <" + errMsg + ">");
    }
}

void SubConnection::asyncHandleMsg() {
    std::vector<MqttInfo> msgs;
    msgs.reserve(1024);
    while (!isClosed_) {
        msgs.clear();
        msgQueue_.blockingPop(msgs, 1024, 1000);
        for (auto &msg : msgs) {
            handleMsgInternal(msg.topic, msg.msg);
        }
    }
}

void SubConnection::startAsyncHandleMsgThread() {
    asyncHandleMsgThread_ = new Thread(new dolphindb::Executor([this]() { asyncHandleMsg(); }));
    asyncHandleMsgThread_->start();
}

void SubConnection::handleMsg(const std::string &topic, const std::string &msg) {
    if (isAsync_) {
        msgQueue_.push({topic, msg});
    } else {
        handleMsgInternal(topic, msg);
    }
}

void SubConnection::handleMsgInternal(const std::string &topic, const std::string &msg) {
    try {
        setReceived();
        incRecv();
        vector<ConstantSP> args;

        if (handler_->isTable()) {
            TableSP tp = handler_;
            ConstantSP m = Util::createConstant(DT_STRING);
            m->setString(msg);
            args.push_back(m);

            TableSP resultTable;
            try {
                resultTable = parser_->call(session->getHeap().get(), args);
            } catch (exception &e) {
                PLUGIN_LOG_INFO("[PluginMQTT]: parse exception:");
                PLUGIN_LOG_INFO(e.what());
                return;
            }
            vector<ConstantSP> args1 = {handler_, resultTable};
            Heap *heap = session->getHeap().get();
            session->getFunctionDef("append!")->call(heap, args1);
        } else {
            ConstantSP t = Util::createConstant(DT_STRING);
            t->setString(topic);
            args.push_back(t);

            ConstantSP m = Util::createConstant(DT_STRING);
            m->setString(msg);
            args.push_back(m);

            FunctionDefSP cb = (FunctionDefSP)handler_;
            Heap *heap = session->getHeap().get();
            try {
                cb->call(heap, args);
            } catch (exception &e) {
                PLUGIN_LOG_INFO("[PluginMQTT]: call function exception:");
                PLUGIN_LOG_INFO(e.what());
                return;
            }
        }
    } catch (exception &e) {
        PLUGIN_LOG_INFO("[PluginMQTT]: subCallback exception:", e.what());
    }
}

void SubConnection::reconnect() {
    try {
        sockfd_ = new Socket(host_, port_, false);
        IO_ERR ret = sockfd_->connect();
        if (ret != OK && ret != INPROGRESS) {
            PLUGIN_LOG_ERR("[PluginMQTT]: Failed to connect. ");
            return;
        }
        mqtt_reinit(&client_, sockfd_->getHandle(), sendbuf_.get(), sendbufSize_, recvbuf_.get(), recvbufSize_);
        uint8_t connect_flags = MQTT_CONNECT_CLEAN_SESSION;
        std::string client_id = clientID_.empty() ? "ddb_mqtt_plugin_sub" : clientID_;
        if (userName_ == "") {
            mqtt_connect(&client_, client_id.c_str(), NULL, NULL, 0, NULL, NULL, connect_flags, 400);
        } else {
            mqtt_connect(&client_, client_id.c_str(), NULL, NULL, 0, userName_.c_str(), password_.c_str(),
                         connect_flags, 400);
        }
        if (client_.error != MQTT_OK) {
            throw RuntimeException(mqtt_error_str(client_.error));
        }
        mqtt_subscribe(&client_, topic_.c_str(), 2);
        if (client_.error != MQTT_OK) {
            throw RuntimeException(mqtt_error_str(client_.error));
        }
    } catch (exception &e) {
        std::string errMsg(e.what());
        PLUGIN_LOG_ERR(LOG_PRE_STR + " mqtt subscribe connection reconnect failed, error message is <", errMsg, ">");
        throw RuntimeException(LOG_PRE_STR + " mqtt subscribe connection reconnect failed, error message is <" +
                               errMsg + ">");
    }
}

}  // namespace mqtt
