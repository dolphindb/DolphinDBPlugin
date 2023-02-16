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

using mqtt::Connection;

#include "templates/posix_sockets.h"

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
 * @brief The client's refresher. This function triggers back-end routines to
 *        handle ingress/egress traffic to the broker.
 *
 * @note All this function needs to do is call \ref __mqtt_recv and
 *       \ref __mqtt_send every so often. I've picked 100 ms meaning that
 *       client ingress/egress traffic will be handled every 100 ms.
 */

namespace mqtt{
void SyncData::run(){
    LOG_INFO("[PluginMQTT]: SyncData::run thread start");
    while (!connection_->isClosed()) {
    // #ifdef WIN32
    //         pthread_testcancel();
    // #endif
            {
                connection_->setReceived();
                mqtt_client* client = connection_->getClient();
                while(connection_->received()){
                    connection_->resetReceived();
                    try{
                        LockGuard<Mutex> lockGurad(lockClient_.get());
                        MQTTErrors ret = mqtt_sync(client);
                        
                        while (ret < 0){
                            LOG_INFO(string("[PluginMQTT]: ") + mqtt_error_str(client->error));
                            LOG_INFO("[PluginMQTT]: reconnect");
                            connection_->reconnect();
                            Util::sleep(1000);
                            ret = mqtt_sync(client);
                        }
                        
                        freeNotifier_->notify();
                    }catch(exception& e){
                        LOG_ERR("[PluginMQTT]: SyncData receive exception: ");
                        LOG_ERR(e.what());
                    }
                }
            }
            usleep(100000U);
        }
        
    LOG_INFO("[PluginMQTT]: SyncData:run thread exit");
}
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
    std::string usage = "Usage: connect(host, port,[qos=0],[formatter],[batchsize=0],[username],[password]).";

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
        } else {
            formatter =  FunctionDefSP(args[3]);
        }
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
    if(args.size()>6){
        if (args[5]->getType() != DT_STRING || args[5]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, usage + "username must be a string");
        }
        if (args[6]->getType() != DT_STRING || args[6]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, usage + "password must be a string");
        }
        userName = args[5]->getString();
        password = args[6]->getString();
    }

    std::unique_ptr<Connection> cup(
        new Connection(args[0]->getString(), args[1]->getInt(), publishFlags, formatter, batchSize,userName,password));

    FunctionDefSP onClose(Util::createSystemProcedure("mqtt connection onClose()", mqttConnectionOnClose, 1, 1));
    return Util::createResource((long long)cup.release(), "mqtt publish connection", onClose, heap->currentSession());
}

/**
 * @brief publish a message to broke/server.
 */

ConstantSP mqttClientPub(Heap *heap, vector<ConstantSP> &args) {
    std::string usage = "Usage: publish(conn,topic,obj). ";
    uint16_t msgLen;
    Connection *conn;
    string topicStr;
    string  messageStr;

    // parse args first
    if (args[0]->getType() == DT_RESOURCE) {
        if(args[0]->getString() != "mqtt publish connection"){
            throw IllegalArgumentException(__FUNCTION__, "connection must be a mqtt publish connection.");
        }
        conn = (Connection *)(args[0]->getLong());
        if (conn==nullptr || conn->isClosed()) {
            throw IllegalArgumentException(__FUNCTION__, "connection is closed.");
        }

    } else {
        throw IllegalArgumentException(__FUNCTION__, "Invalid connection object.");
    }

    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "topic must be a string");
    } else {
        topicStr = args[1]->getString();
    }
    if (args[2]->isTable()) {
        if (conn->getFormatter().isNull()== true) {
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
            vector<ConstantSP> args1 = {t};    // args1.push_back(table);

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
            // check for errors
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
    // check for errors
    if (err != MQTT_OK) {
        throw RuntimeException(mqtt_error_str(err));
    }

    return new Int(MQTT_OK);
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
        if(handle->getString() != "mqtt publish connection"){
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
Connection::~Connection(){
    isClosed_ = true;
    clientDaemon_->join();
    sockfd_->close();
    LOG_INFO("[PluginMQTT]: close publish connection");
}

Connection::Connection(const std::string& hostname, int port, uint8_t qos, const FunctionDefSP& formatter, int batchSize, 
		const std::string& userName, const std::string& password)
    : ConnctionBase(new ConditionalNotifier()),
    host_(hostname), port_(port), publishFlags_(qos), formatter_(formatter), batchSize_(batchSize),
    userName_(userName), password_(password)
     {
    sockfd_ = new Socket(hostname, port, false);
    IO_ERR ret = sockfd_->connect();
    if (ret != OK && ret != INPROGRESS) {
        throw RuntimeException("Failed to connect. ");
    }

    mqtt_init(&client_, sockfd_->getHandle(), sendbuf_, sizeof(sendbuf_), recvbuf_, sizeof(recvbuf_), publishCallback);

    /* Create an anonymous session */
    const char* client_id = NULL;
    /* Ensure we have a clean session */
    uint8_t connect_flags = MQTT_CONNECT_CLEAN_SESSION;
    /* Send connection request to the broker. */
    if(userName=="") {
        mqtt_connect(&client_, client_id, NULL, NULL, 0, NULL, NULL, connect_flags, 400);
    }
    else{
        mqtt_connect(&client_, client_id, NULL, NULL, 0, userName.c_str(), password.c_str(), connect_flags, 400);
    }

    /* check that we don't have any errors */
    if (client_.error != MQTT_OK) {
        throw RuntimeException(mqtt_error_str(client_.error));
    }
    lockClient_ = new Mutex();
    /* start a thread to refresh the client (handle egress and ingree clien traffic) */
    SmartPointer<SyncData> syncData= new SyncData(this, lockClient_, freeNotifier_);
    clientDaemon_ = new Thread(syncData);
    if (!clientDaemon_->isStarted()) {
        clientDaemon_->start();
    }
    sendtimes = 0;
    failed = 0;
}

MQTTErrors Connection::publishMsg(const char *topic, void *message, size_t size) {
    sendtimes++;
    MQTTErrors err = MQTT_OK;
    do {
        {
            LockGuard<Mutex> lockGurad(lockClient_.get());
            err = mqtt_publish(&client_, topic, message, size, publishFlags_);
            if(err == MQTT_ERROR_SEND_BUFFER_IS_FULL)
                client_.error = MQTT_OK;
            if (err != MQTT_OK) {
                LOG_INFO(string("[PluginMQTT]: publishMsg ") + mqtt_error_str(err));
                failed++;
            }
        }
        //Now only according to the repair methods of https://github.com/LiamBindle/MQTT-C/issues/124
        if (err == MQTT_ERROR_SEND_BUFFER_IS_FULL) {
            freeNotifier_->wait();
        }
        if(isClosed_)
            throw RuntimeException("connection is closed");
    } while (MQTT_OK != err);

    return err;
}

}    // namespace mqtt
