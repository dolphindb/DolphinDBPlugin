/*
 * pub_client.cpp
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
#include "publisher.h"
using mqtt::Connection;
using namespace std;

/**
 * @brief The function that would be called whenever a PUBLISH is received.
 *
 * @note This function is not used in this example.
 */
static void publishCallback(void **unused, struct mqtt_response_publish *published) {
    /* not used in this example */
    std::cout << "publish callback" << std::endl;
}

/**
 * @brief Safelty closes the \p sockfd and cancels the \p clientDaemon before \c
 * exit.
 */

static void mqttConnectionOnClose(Heap *heap, vector<ConstantSP> &args) {
    Connection *cp = (Connection *)(args[0]->getLong());
    if (cp != nullptr) {
        delete cp;
        args[0]->setLong(0);
    }
}

/**
 * @brief connect a mqtt broke or server.
 */
ConstantSP mqttClientConnect(Heap *heap, vector<ConstantSP> &args) {
    std::string usage =
        "Usage: connect(host, port, [qos=0], [formatter], [batchsize=1], [username], [password], [sendbufSize=40960]).";

    uint8_t publishFlags = MQTT_PUBLISH_QOS_0;
    FunctionDefSP formatter;
    int batchSize = 1;
    std::string userName;
    std::string password;

    // parse args first
    if (args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "host must be a string");
    }
    if (args[1]->getType() != DT_INT || args[1]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "port must be an integer");
    }

    if (args.size() >= 3 && !args[2]->isNull()) {
        if (args[2]->getType() != DT_INT || args[2]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, usage + "QoS must be a integer");
        }
        if (args[2]->getInt() < 0 || args[2]->getInt() > 2)
            throw IllegalArgumentException(__FUNCTION__, usage + "QoS must be a integer(0-2)");
        switch (args[2]->getInt()) {
            case 1:
                publishFlags = MQTT_PUBLISH_QOS_1;
                break;
            case 2:
                publishFlags = MQTT_PUBLISH_QOS_2;
                break;
            default:
                publishFlags = MQTT_PUBLISH_QOS_0;
        }
    }
    if (args.size() >= 4 && !args[3]->isNull()) {
        if (args[3]->getType() != DT_FUNCTIONDEF) {
            throw IllegalArgumentException(__FUNCTION__, usage + "formatter must be an function.");
        }
        formatter = FunctionDefSP(args[3]);
        // string functionName = formatter->getName();
        // if (functionName != "formatCsv" && functionName != "formatJson" && functionName != "toStdJson") {
        //     throw IllegalArgumentException(__FUNCTION__, usage + "formatter must be formatCsv or formatJson or
        //     toStdJson, paramFunction:" + functionName);
        // }
    }
    if (args.size() >= 5 && !args[4]->isNull()) {
        if (args[4]->getType() != DT_INT || args[4]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, usage + "batchSize must be a integer");
        } else {
            batchSize = args[4]->getInt();
            if (batchSize < 1) {
                throw IllegalArgumentException(__FUNCTION__, usage + "batchSize must be a unsigned integer");
            }
        }
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

    int sendbufSize = 40960;
    if (args.size() > 7) {
        if (!args[7]->isScalar() || args[7]->getType() != DT_INT) {
            throw IllegalArgumentException(__FUNCTION__, usage + "the type of sendbufSize must be int");
        }
        sendbufSize = args[7]->getInt();
    }

    // config
    int recvBufSize = 20480;
    std::string clientID;
    if (args.size() > 8 && !args[8]->isNothing()) {
        if (args[8]->getForm() != DF_DICTIONARY) {
            throw IllegalArgumentException(__FUNCTION__, usage + "config must be a dictionary");
        }
        DictionarySP dic = args[8];
        if (dic->getKeyType() != DT_STRING) {
            throw IllegalArgumentException(__FUNCTION__,
                                           usage + "config must be a dictionary whose key type is string");
        }

        ConstantSP recvBufSizeConstant = dic->getMember("recvBufSize");
        if (!recvBufSizeConstant->isNull()) {
            if (recvBufSizeConstant->getType() != DT_INT || recvBufSizeConstant->getForm() != DF_SCALAR) {
                throw IllegalArgumentException(__FUNCTION__, usage + "config recvBufSize must be a INT scalar");
            }
            recvBufSize = recvBufSizeConstant->getInt();
        }

        ConstantSP clientIDConstant = dic->getMember("clientID");
        if (!clientIDConstant->isNull()) {
            if (clientIDConstant->getType() != DT_STRING || clientIDConstant->getForm() != DF_SCALAR) {
                throw IllegalArgumentException(__FUNCTION__, usage + "config clientID must be a STRING scalar");
            }
            clientID = clientIDConstant->getString();
        }
    }

    if (sendbufSize <= 0 || recvBufSize <= 0) {
        throw IllegalArgumentException(__FUNCTION__, usage + "config sendBufSize and recvbufSize must be positive.");
    }
    std::unique_ptr<Connection> cup(new Connection(args[0]->getString(), args[1]->getInt(), publishFlags, formatter,
                                                   batchSize, userName, password, sendbufSize, recvBufSize, clientID));

    FunctionDefSP onClose(Util::createSystemProcedure("mqtt connection onClose()", mqttConnectionOnClose, 1, 1));
    return Util::createResource((long long)cup.release(), "mqtt publish connection", onClose, heap->currentSession());
}

/**
 * @brief publish a message to broke/server.
 */
ConstantSP mqttClientPub(Heap *heap, vector<ConstantSP> &args) {
    std::string usage = "Usage: publish(conn,topic,obj). ";
    size_t msgLen;
    Connection *conn;
    string topicStr;
    string messageStr;

    // parse args first
    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != "mqtt publish connection") {
        throw IllegalArgumentException(__FUNCTION__, "connection must be a mqtt publish connection.");
    }
    conn = (Connection *)(args[0]->getLong());
    if (conn == nullptr || conn->isClosed()) {
        throw IllegalArgumentException(__FUNCTION__, "connection is closed.");
    }

    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "topic must be a string");
    }
    topicStr = args[1]->getString();
    if (topicStr.length() == 0) {
        throw RuntimeException(LOG_PRE_STR + " the length of topic should more than 0");
    }
    if (args[2]->isTable()) {
        if (conn->getFormatter().isNull() == true) {
            throw IllegalArgumentException(__FUNCTION__,
                                           "Please reconnect,the formatter must be provided while publishing table.");
        }

        TableSP t = args[2];
        int batchSize = conn->getBatchSize();
        if (batchSize > 0) {
            for (int r = 0; r < t->rows(); r += batchSize) {
                FunctionDefSP seq = heap->currentSession()->getFunctionDef("seq");
                int end = min(t->rows() - 1, r + batchSize - 1);
                vector<ConstantSP> args0 = {new Int(r), new Int(end)};
                ConstantSP result = seq->call(heap, args0);
                TableSP subTable = t->get(result);
                vector<ConstantSP> args1 = {subTable};
                ConstantSP s = conn->getFormatter()->call(heap, args1);

                messageStr = s->getString();
                msgLen = messageStr.length();
                MQTTErrors err = conn->publishMsg(topicStr.c_str(), (void *)messageStr.c_str(), msgLen);
                // check for errors
                if (err != MQTT_OK) {
                    throw RuntimeException(mqtt_error_str(err));
                }
                Util::sleep(100);
            }

            return new Int(MQTT_OK);
        } else {
            vector<ConstantSP> args1 = {t};  // args1.push_back(table);

            FunctionDefSP formatter = conn->getFormatter();
            ConstantSP s = formatter->call(heap, args1);
            messageStr = s->getString();
            msgLen = messageStr.length();
        }

    } else if (args[2]->getType() == DT_STRING && args[2]->isArray()) {
        for (int i = 0; i < args[2]->size(); i++) {
            messageStr = args[2]->get(i)->getString();
            msgLen = args[2]->get(i)->getString().length();

            MQTTErrors err = conn->publishMsg(topicStr.c_str(), (void *)messageStr.c_str(), msgLen);
            if (err != MQTT_OK) {
                throw RuntimeException(mqtt_error_str(err));
            }
        }
        return new Int(MQTT_OK);

    } else if (args[2]->getType() == DT_STRING && args[2]->getForm() == DF_SCALAR) {
        messageStr = args[2]->getString();
        msgLen = messageStr.length();
    } else {
        throw IllegalArgumentException(__FUNCTION__, usage + "obj must be a string or table ");
    }

    MQTTErrors err = conn->publishMsg(topicStr.c_str(), (void *)messageStr.c_str(), msgLen);
    if (err != MQTT_OK) {
        throw RuntimeException(mqtt_error_str(err));
    }
    return new Int(MQTT_OK);
}

/**
 * @brief create a publisher to ease publishing
 */
ConstantSP mqttClientCreatePublisher(Heap *heap, vector<ConstantSP> &args) {
    std::string usage = "Usage: createPublisher(conn,topic,colNames,colTypes).";
    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != "mqtt publish connection") {
        throw IllegalArgumentException(__FUNCTION__, usage + " connection must be a mqtt publish connection.");
    }
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + " topic must be a string.");
    }
    if (args[2]->getType() != DT_STRING || args[2]->getForm() != DF_VECTOR) {
        throw IllegalArgumentException(__FUNCTION__, usage + " colNames must be a string vector.");
    }
    if (args[3]->getType() != DT_INT || args[3]->getForm() != DF_VECTOR) {
        throw IllegalArgumentException(__FUNCTION__, usage + " colTypes must be a DATA_TYPE vector.");
    }

    ConstantSP tmpVec = args[2];
    int num = tmpVec->size();
    if (num != args[3]->size()) {
        throw IllegalArgumentException(__FUNCTION__, usage + " the num of colNames and colTypes must be the same.");
    }

    vector<string> colNames(num);
    for (int i = 0; i < num; ++i) {
        colNames[i] = tmpVec->getString(i);
    }

    tmpVec = args[3];
    vector<ConstantSP> cols(num);
    for (int i = 0; i < num; ++i) {
        cols[i] = Util::createVector(static_cast<DATA_TYPE>(tmpVec->getInt(i)), 0, 0);
    }

    return new PublishTable(cols, colNames, args[0], args[1]->getString(), heap);
}

/**
 * @brief close a connection.
 */
ConstantSP mqttClientClose(const ConstantSP &handle, const ConstantSP &b) {
    std::string usage = "Usage: close(conn). ";
    Connection *cp = nullptr;
    // parse args first
    if (handle->getType() == DT_RESOURCE) {
        cp = (Connection *)(handle->getLong());
        if (handle->getString() != "mqtt publish connection") {
            throw IllegalArgumentException(__FUNCTION__, "connection must be a mqtt publish connection.");
        }
    } else {
        throw IllegalArgumentException(__FUNCTION__, "Invalid connection object.");
    }

    if (cp != nullptr) {
        cp->close();
    }

    return new Int(MQTT_OK);
}

/// Connection
namespace mqtt {
class RefeshPubConn : public Runnable {
  public:
    RefeshPubConn(Connection *connection) : connection_(connection){};
    ~RefeshPubConn() override = default;
    void run() override;

  private:
    Connection *connection_;
    SmartPointer<Mutex> lockClient_;
    SmartPointer<ConditionalNotifier> freeNotifier_;
};

void RefeshPubConn::run() {
    struct mqtt_client *client = connection_->getClient();
    while (!connection_->isClosed()) {
        try {
            MQTTErrors ret = mqtt_sync(client);
            int currentRetryCount = 0;
            while (ret == MQTT_ERROR_SOCKET_ERROR && currentRetryCount < MAX_RETRY_COUNT) {
                connection_->reconnect();
                Util::sleep(100);
                ret = mqtt_sync(client);
                currentRetryCount++;
            }
            if (currentRetryCount == MAX_RETRY_COUNT) {
                PLUGIN_LOG_ERR(LOG_PRE_STR, " pub reconnect failed.");
            }
        } catch (exception &e) {
            PLUGIN_LOG_ERR(LOG_PRE_STR, " refresh publish connection failed, err message is <", string(e.what()), ">");
        } catch (...) {
            PLUGIN_LOG_ERR(LOG_PRE_STR, " refresh publish connection failed.");
        }
        usleep(100000U);
    }
}

Connection::~Connection() {
    isClosed_ = true;
    clientDaemon_->join();
    mqtt_disconnect(&client_);
    mqtt_sync(&client_);
    sockfd_->close();
    PLUGIN_LOG_INFO("[PluginMQTT]: close publish connection");
}

Connection::Connection(const std::string &hostname, int port, uint8_t qos, const FunctionDefSP &formatter,
                       int batchSize, const std::string &userName, const std::string &password, int sendbufSize,
                       int recvbufSize, const string &clientID)
    : ConnctionBase(new ConditionalNotifier()),
      host_(hostname),
      port_(port),
      publishFlags_(qos),
      formatter_(formatter),
      batchSize_(batchSize),
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
        throw RuntimeException(LOG_PRE_STR + " Failed to connect.");
    }
    if (ret == INPROGRESS &&
        !checkConnectStatus(sockfd_->getHandle())) {  // handle EINPROGRESS on non-block TCP connection
        throw RuntimeException("Failed to connect.");
    }

    try {
        mqtt_init(&client_, sockfd_->getHandle(), sendbuf_.get(), sendbufSize_, recvbuf_.get(), recvbufSize_,
                  publishCallback);
        std::string client_id =
            clientID_.empty() ? "ddb_mqtt_plugin_pub" + std::to_string(sockfd_->getHandle()) : clientID_;
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

        // check connack
        checkConnack(&client_);

        // create thread
        lockClient_ = new Mutex();
        SmartPointer<RefeshPubConn> PubConn = new RefeshPubConn(this);
        clientDaemon_ = new Thread(PubConn);
        if (!clientDaemon_->isStarted()) {
            clientDaemon_->start();
        }
        sendtimes_ = 0;
        failed_ = 0;
    } catch (exception &e) {
        std::string errMsg(e.what());
        PLUGIN_LOG_ERR(LOG_PRE_STR + " mqtt publish connection init failed, error message is <", errMsg, ">");
        throw RuntimeException(LOG_PRE_STR + " mqtt publish connection init failed, error message is <" + errMsg + ">");
    }
}

MQTTErrors Connection::publishMsg(const char *topic, void *message, size_t size) {
    sendtimes_++;
    MQTTErrors err = MQTT_OK;
    do {
        try {
            err = mqtt_publish(&client_, topic, message, size, publishFlags_);
            if (err == MQTT_ERROR_SEND_BUFFER_IS_FULL) client_.error = MQTT_OK;
            if (err != MQTT_OK) {
                PLUGIN_LOG_INFO(string("[PluginMQTT]: publishMsg error:") + mqtt_error_str(err));
                failed_++;
            }
            mqtt_sync(&client_);
        } catch (exception &e) {
            std::string errMsg(e.what());
            PLUGIN_LOG_ERR(LOG_PRE_STR + " mqtt publish msg failed, error message is <", errMsg, ">");
            throw RuntimeException(LOG_PRE_STR + " mqtt publish msg failed, error message is <" + errMsg + ">");
        }

        if (err == MQTT_ERROR_SEND_BUFFER_IS_FULL) {
            Util::sleep(500);
            PLUGIN_LOG_INFO("[PluginMQTT]: publishMsg get MQTT_ERROR_SEND_BUFFER_IS_FULL err.");
        }
        if (isClosed_) throw RuntimeException(LOG_PRE_STR + "connection is closed");
    } while (MQTT_OK != err && failed_ < MAX_RETRY_COUNT);
    failed_ = 0;
    return err;
}

}  // namespace mqtt
