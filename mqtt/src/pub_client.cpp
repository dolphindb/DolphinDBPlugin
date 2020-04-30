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

static void *clientRefresher(void *client) {
    while (1) {
        mqtt_sync((struct mqtt_client *)client);
        usleep(100000U);
    }
    return NULL;
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
    std::string usage = "Usage: connect(host, port,[qos=0],[formatter],[batchsize=0).";

    uint8_t publishFlags = MQTT_PUBLISH_QOS_0;
    FunctionDef *formatter = NULL;
    int batchSize = 1;

    // parse args first
    if (args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "host must be a string");
    }
    if (args[1]->getType() != DT_INT || args[1]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "port must be an integer");
    }
    if (!args[2]->isNull()) {
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
    if (!args[3]->isNull()) {
        if (args[3]->getType() != DT_FUNCTIONDEF) {
            throw IllegalArgumentException(__FUNCTION__, usage + "formatter must be an function.");
        } else {
            formatter = (FunctionDef *)args[3].get();    // FunctionDefSP(args[3]);
        }
    }
    if (!args[4]->isNull()) {
        if (args[4]->getType() != DT_INT || args[4]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, usage + "batchSize must be a integer");
        } else {
            batchSize = args[4]->getInt();
            if (batchSize < 1) {
                throw IllegalArgumentException(__FUNCTION__, usage + "batchSize must be a unsigned integer");
            }
        }
    }
    std::unique_ptr<Connection> cup(
        new Connection(args[0]->getString(), args[1]->getInt(), publishFlags, formatter, batchSize));

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
        conn = (Connection *)(args[0]->getLong());
        if (!conn->getConnected()) {
            throw IllegalArgumentException(__FUNCTION__, "Please connect firstly.");
        }

    } else {
        throw IllegalArgumentException(__FUNCTION__, "Invalid connection object.");
    }

    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "topic must be a string");
    } else {
        topicStr = args[1]->getString();
    }
    // std::cout<<topic;
    if (args[2]->isTable()) {
        if (conn->getFormatter() == NULL) {
            throw IllegalArgumentException(__FUNCTION__,
                                           "Please reconnect,the formatter must be provided while publishing table.");
        }

        TableSP t = args[2];
        int batchSize = conn->getBatchSize();
        if (batchSize > 0) {
            for (int r = 0; r < t->rows(); r += batchSize) {
                FunctionDefSP seq = heap->currentSession()->getFunctionDef("seq");
                int end = min(t->rows() - 1, r + batchSize);
                // cout<<r<<"->"<<end;
                vector<ConstantSP> args0 = {new Int(r), new Int(end)};
                ConstantSP result = seq->call(heap, args0);

                TableSP subTable = t->get(result);

                vector<ConstantSP> args1 = {subTable};
                ConstantSP s = conn->getFormatter()->call(heap, args1);

                messageStr = s->getString();
                msgLen = messageStr.length();

                // cout << "subtable data length is " << msgLen << endl;
                MQTTErrors err = conn->publishMsg(topicStr.c_str(), (void *)messageStr.c_str(), msgLen);
                // check for errors
                if (err != MQTT_OK) {
                    std::cout << "Error " << err << mqtt_error_str(err) << std::endl;
                    throw RuntimeException(mqtt_error_str(err));
                }
                Util::sleep(100);
            }

            return new Int(MQTT_OK);
        } else {
            vector<ConstantSP> args1 = {t};    // args1.push_back(table);

            FunctionDef *formatter = conn->getFormatter();
            ConstantSP s = formatter->call(heap, args1);

            messageStr = s->getString();
            msgLen = messageStr.length();
            // cout << "table data length is " << msgLen << endl;
        }

    } else if (args[2]->getType() == DT_STRING && args[2]->isArray()) {
        for (int i = 0; i < args[2]->size(); i++) {
            //cout << args[2]->get(i)->getString() << endl;
            messageStr = args[2]->get(i)->getString();
            msgLen = args[2]->get(i)->getString().length();

            MQTTErrors err = conn->publishMsg(topicStr.c_str(), (void *)messageStr.c_str(), msgLen);
            // check for errors
            if (err != MQTT_OK) {
                std::cout << "Error " << err << mqtt_error_str(err) << std::endl;
                throw RuntimeException(mqtt_error_str(err));
            }

        }
        return new Int(MQTT_OK);

    } else if (args[2]->getType() == DT_STRING && args[2]->getForm() == DF_SCALAR) {
        // cout<<"is message "<< args[2]->getForm() <<endl;

        messageStr = args[2]->getString();
        msgLen = messageStr.length();
    } else {
        cout << "not message not table " << args[2]->getForm() << endl;
        throw IllegalArgumentException(__FUNCTION__, usage + "obj must be a string or table ");
    }

    MQTTErrors err = conn->publishMsg(topicStr.c_str(), (void *)messageStr.c_str(), msgLen);
    // check for errors
    if (err != MQTT_OK) {
        std::cout << "Error " << err << mqtt_error_str(err) << std::endl;
        throw RuntimeException(mqtt_error_str(err));
    }

    return new Int(MQTT_OK);
}

/**
 * @brief close a connection.
 */
ConstantSP mqttClientClose(const ConstantSP &handle, const ConstantSP &b) {
    std::string usage = "Usage: close(conn). ";
    Connection *cp = NULL;
    // parse args first
    if (handle->getType() == DT_RESOURCE) {
        cp = (Connection *)(handle->getLong());
    } else {
        throw IllegalArgumentException(__FUNCTION__, "Invalid connection object.");
    }

    if (cp != nullptr) {
        delete cp;
        handle->setLong(0);
    }

    return new Int(MQTT_OK);
}

/// Connection
namespace mqtt {
Connection::Connection() {
    connected_ = false;
    sockfd_ = -1;
}

Connection::~Connection() {
    if (sockfd_ != -1) {
        close(sockfd_);
        std::cout << "close sub conn. sockfd is " << sockfd_ << std::endl;
    }
    if (connected_)
        pthread_cancel(clientDaemon_);
    std::cout << "send times is " << sendtimes << " ,failed is " << failed << std::endl;
}

Connection::Connection(std::string hostname, int port, uint8_t qos, FunctionDef *formatter, int batchSize)
    : host_(hostname), port_(port), publishFlags_(qos), formatter_(formatter), batchSize_(batchSize) {
    sockfd_ = open_nb_socket(host_.c_str(), std::to_string(port).c_str());
    if (sockfd_ == -1) {
        throw RuntimeException("Failed to open socket: ");
    }

    mqtt_init(&client_, sockfd_, sendbuf_, sizeof(sendbuf_), recvbuf_, sizeof(recvbuf_), publishCallback);

    /* Create an anonymous session */
    const char* client_id = NULL;
    /* Ensure we have a clean session */
    uint8_t connect_flags = MQTT_CONNECT_CLEAN_SESSION;
    /* Send connection request to the broker. */
    mqtt_connect(&client_, client_id, NULL, NULL, 0, NULL, NULL, connect_flags, 400);
    /* check that we don't have any errors */
    if (client_.error != MQTT_OK) {
        std::cout << client_.error << std::endl;
        throw RuntimeException(mqtt_error_str(client_.error));
    }

    /* start a thread to refresh the client (handle egress and ingree client
     * traffic) */
    if (pthread_create(&clientDaemon_, NULL, clientRefresher, &client_)) {
        std::cout << "Failed to start client daemon." << std::endl;
        throw RuntimeException("Failed to start client daemon.");
    }

    connected_ = true;
    sendtimes = 0;
    failed = 0;
    //std::cout << "connected.sockfd is " << sockfd_ << std::endl;
}

MQTTErrors Connection::publishMsg(const char *topic, void *message, size_t size) {
    sendtimes++;

    MQTTErrors err = mqtt_publish(&client_, topic, message, size, publishFlags_);
    if (client_.error != MQTT_OK) {
        failed++;
    }

    return err;
}

}    // namespace mqtt
