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
#include "mqtt.h"
#include "templates/posix_sockets.h"

using namespace std;
using mqtt::SubConnection;

/**
 * @brief The function that would be called whenever a PUBLISH is received.
 */
static void subCallback(void **socketHandle, struct mqtt_response_publish *published) {
    /* note that published->topic_name is NOT null-terminated (here we'll change it to a c-string) */
    SubConnection *cp = (SubConnection *)(*socketHandle);
    if (cp == NULL) {
        throw RuntimeException("Connection is not found.");
    }
    cp->incRecv();

    ConstantSP handler = cp->getHandler();

    std::string topic((const char *)published->topic_name, 0, published->topic_name_size);
    std::string msg((const char *)published->application_message, 0, published->application_message_size);

    // std::cout<<*(int*)*socketHandle<<"Received publish:"<<topic<<",message:"<<(const char*)
    // published->application_message;

    vector<ConstantSP> args;

    if (handler->isTable()) {
        TableSP tp = handler;
        // std::cout<<topic<<",message len:"<<published->application_message_size<<endl;

        ConstantSP m = Util::createConstant(DT_STRING);
        m->setString(msg);
        args.push_back(m);

        TableSP resultTable = cp->getParser()->call(cp->getHeap(), args);
        vector<ConstantSP> args1 = {resultTable};
        INDEX insertedRows = 1;
        string errMsg;
        tp->append(args1, insertedRows, errMsg);
        if (insertedRows != resultTable->rows()) {
            //cout << "insert " << insertedRows << " err " << errMsg << endl;
            throw RuntimeException(errMsg);
        }

    } else {
        // args.push_back(Util::createConstant(DT_VOID));

        ConstantSP t = Util::createConstant(DT_STRING);
        t->setString(topic);
        args.push_back(t);

        ConstantSP m = Util::createConstant(DT_STRING);
        m->setString(msg);
        args.push_back(m);

        FunctionDef *cb = (FunctionDef *)handler.get();
        cb->call(cp->getHeap(), args);
    }
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
 * @brief Safelty closes the \p sockfd and cancels the \p clientDaemon before \c exit.
 */

static void mqttConnectionOnClose(Heap *heap, vector<ConstantSP> &args) {
    SubConnection *cp = (SubConnection *)(args[0]->getLong());
    if (cp != nullptr) {
        delete cp;
        args[0]->setLong(0);
    }
}

/**
 * @brief Safelty closes the \p sockfd and cancels the \p clientDaemon before \c exit.
 */

ConstantSP mqttClientSub(Heap *heap, vector<ConstantSP> &args) {
    std::string usage = "Usage: subscribe(host, port, topic, [parser], handler).";

    // parse args first
    FunctionDef *parser = NULL;

    if (args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "host must be a string");
    }

    if (args[1]->getType() != DT_INT || args[1]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "port must be an integer");
    } else {
        if (args[1]->getInt() < 1 || args[1]->getInt() > 65535)
            throw IllegalArgumentException(__FUNCTION__, usage + "port must be an integer(1-65535)");
    }

    if (args[2]->getType() != DT_STRING || args[2]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "topic must be a string");
    }
    if (args[4]->isTable()) {
        if (args[3]->getType() != DT_FUNCTIONDEF) {
            throw IllegalArgumentException(__FUNCTION__, usage + "parser must be an function.");
        } else {
            parser = (FunctionDef *)args[3].get();
        }
    }

    if (!args[4]->isTable() && args[4]->getType() != DT_FUNCTIONDEF) {
        throw IllegalArgumentException(__FUNCTION__, usage + "handler must be a table or an unary function.");
    } else {
        // handler =(FunctionDef*)args[4].get();// FunctionDefSP(args[4]);
        // if(handler->getMaxParamCount()<1 || handler->getMinParamCount()>1)
        // throw IllegalArgumentException(__FUNCTION__, usage + "handler must 1 param.");
    }
    std::unique_ptr<SubConnection> cup(
        new SubConnection(args[0]->getString(), args[1]->getInt(), args[2]->getString(), parser, args[4], heap));
    FunctionDefSP onClose(Util::createSystemProcedure("mqtt sub connection onClose()", mqttConnectionOnClose, 1, 1));
    return Util::createResource((long long)cup.release(), "mqtt subscribe connection", onClose, heap->currentSession());
}

ConstantSP mqttClientStopSub(const ConstantSP &handle, const ConstantSP &b) {
    // parse args first
    std::string usage = "Usage: close(conn). ";
    SubConnection *cp = NULL;
    // parse args first
    if (handle->getType() == DT_RESOURCE) {
        cp = (SubConnection *)(handle->getLong());
    } else {
        throw IllegalArgumentException(__FUNCTION__, "Invalid connection object.");
    }

    if (cp != nullptr) {
        delete cp;
        handle->setLong(0);
    }
    //cout << "stop sub !" << endl;
    return new Int(MQTT_OK);
}

/// Connection
namespace mqtt {
SubConnection::SubConnection() {
    connected_ = false;
    sockfd_ = -1;
}

SubConnection::~SubConnection() {
    if (sockfd_ != -1) {
        close(sockfd_);
        //std::cout << "close sub conn.sockfd is " << sockfd_ << std::endl;
    }
    if (connected_)
        pthread_cancel(clientDaemon_);
    //std::cout << "received packet is " << recv << std::endl;
}

SubConnection::SubConnection(std::string hostname, int port, std::string topic, FunctionDef *parser, ConstantSP handler,
                             Heap *pHeap)
    : host_(hostname), port_(port), topic_(topic), parser_(parser), handler_(handler), pHeap_(pHeap) {
    sockfd_ = open_nb_socket(host_.c_str(), std::to_string(port).c_str());
    if (sockfd_ == -1) {
        throw RuntimeException("Failed to open socket: ");
    }



    mqtt_init(&client_, sockfd_, sendbuf_, sizeof(sendbuf_), recvbuf_, sizeof(recvbuf_), subCallback);

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

    /* start a thread to refresh the client (handle egress and ingree client traffic) */
    if (pthread_create(&clientDaemon_, NULL, clientRefresher, &client_)) {
        std::cout << "Failed to start client daemon." << std::endl;
        throw RuntimeException("Failed to start client daemon.");
    }

    connected_ = true;
    recv = 0;
    client_.publish_response_callback_state = this;

    /* subscribe */
    mqtt_subscribe(&client_, topic.c_str(), 0);

    /* check for errors */
    if (client_.error != MQTT_OK) {
        std::cout << client_.error << std::endl;
        throw RuntimeException(mqtt_error_str(client_.error));
    }

    //std::cout << "Subscribe connected.sockfd is " << sockfd_ << std::endl;
}

}    // namespace mqtt
