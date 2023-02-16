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

DictionarySP dict = Util::createDictionary(DT_STRING,0,DT_ANY,0);
/**
 * @brief The function that would be called whenever a PUBLISH is received.
 */
static void subCallback(void **socketHandle, struct mqtt_response_publish *published) {
    /* note that published->topic_name is NOT null-terminated (here we'll change it to a c-string) */
    try{
        SubConnection *cp = (SubConnection *)(*socketHandle);
        if (cp == NULL) {
            //throw RuntimeException("Connection is not found.");
            LOG_INFO("[PluginMQTT]: Connection is not found.");
            return;
        }
        cp->setReceived();
        cp->incRecv();

        ConstantSP handler = cp->getHandler();

        std::string topic((const char *)published->topic_name, 0, published->topic_name_size);
        std::string msg((const char *)published->application_message, 0, published->application_message_size);

        //std::cout<<*(int*)*socketHandle<<"Received publish:"<<topic<<",message:"<<(const char*)published->application_message;

        vector<ConstantSP> args;

        if (handler->isTable()) {
            TableSP tp = handler;
            ConstantSP m = Util::createConstant(DT_STRING);
            m->setString(msg);
            args.push_back(m);

            TableSP resultTable;
            try {
                FunctionDefSP parser = cp->getParser();
                resultTable= parser->call(cp->session->getHeap().get(), args);
            }
            catch(exception& e){
                LOG_INFO("[PluginMQTT]: parse exception:");
                LOG_INFO(e.what());
                return;
            }


            vector<ConstantSP> args1 = {handler, resultTable};
            Heap * heap=cp->session->getHeap().get();
            cp->session->getFunctionDef("append!")->call(heap, args1);
        } else {
            ConstantSP t = Util::createConstant(DT_STRING);
            t->setString(topic);
            args.push_back(t);

            ConstantSP m = Util::createConstant(DT_STRING);
            m->setString(msg);
            args.push_back(m);

            FunctionDefSP cb = (FunctionDefSP)handler;
            Heap * heap=cp->session->getHeap().get();
            try {
                cb->call(heap, args);
            }
            catch(exception& e){
                LOG_INFO("[PluginMQTT]: call function exception:");
                LOG_INFO(e.what());
                return;
            }
        }
    }
    catch(exception& e){
        LOG_INFO("[PluginMQTT]: subCallback exception:");
        LOG_INFO(e.what());
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

// static void *clientRefresher(void *client) {
//     while (1) {
// #ifdef WIN32
//         pthread_testcancel();
// #endif
//         mqtt_sync((struct mqtt_client *)client);
//         usleep(100000U);

//     }
//     return NULL;
// }

/**
 * @brief Safelty closes the \p sockfd and cancels the \p clientDaemon before \c exit.
 */

static void mqttConnectionOnClose(Heap *heap, vector<ConstantSP> &args) {
    LOG_INFO("[PluginMQTT]: mqttConnectionOnClose");
}

/**
 * @brief Safelty closes the \p sockfd and cancels the \p clientDaemon before \c exit.
 */

ConstantSP mqttClientSub(Heap *heap, vector<ConstantSP> &args) {
    std::string usage = "Usage: subscribe(host, port, topic, [parser], handler,[username],[password]).";
    std::string userName;
    std::string password;

    // parse args first
    FunctionDefSP parser;

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
            parser = args[3];
        }
    }

    if (!args[4]->isTable() && args[4]->getType() != DT_FUNCTIONDEF) {
        throw IllegalArgumentException(__FUNCTION__, usage + "handler must be a table or an unary function.");
    }
    if(args.size()>5){
        if (args[5]->getType() != DT_STRING || args[5]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, usage + "username must be a string");
        }
        if (args[6]->getType() != DT_STRING || args[6]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, usage + "password must be a string");
        }
        userName = args[5]->getString();
        password = args[6]->getString();
    }
    std::unique_ptr<SubConnection> cup(
        new SubConnection(args[0]->getString(), args[1]->getInt(), args[2]->getString(), parser, args[4],userName,password, heap));
    FunctionDefSP onClose(Util::createSystemProcedure("mqtt sub connection onClose()", mqttConnectionOnClose, 1, 1));
    ConstantSP conn = Util::createResource((long long)cup.release(), "mqtt subscribe connection", onClose, heap->currentSession());
    dict->set(std::to_string(conn->getLong()),conn);

    return conn;
}

ConstantSP mqttClientStopSub(const ConstantSP &handle, const ConstantSP &b) {
    // parse args first
    std::string usage = "Usage: close(connection or connection ID). ";
    SubConnection *sc = NULL;
    string key;
    ConstantSP conn;
    switch (handle->getType()){
        case DT_RESOURCE:
            sc = (SubConnection *)(handle->getLong());
            key = std::to_string(handle->getLong());
            conn = dict->getMember(key);
            if(conn->isNothing())
                throw IllegalArgumentException(__FUNCTION__, "Invalid connection object.");
            break;
        case DT_STRING:
            key = handle->getString();
            conn = dict->getMember(key);
            if(conn->isNothing())
                throw IllegalArgumentException(__FUNCTION__, "Invalid connection string.");
            else
                sc = (SubConnection *)(conn->getLong());
            break;
        case DT_LONG:
            key = std::to_string(handle->getLong());
            conn = dict->getMember(key);
            if(conn->isNothing())
              throw IllegalArgumentException(__FUNCTION__, "Invalid connection integer.");
            else
              sc = (SubConnection *)(conn->getLong());
            break;
        default:
            throw IllegalArgumentException(__FUNCTION__, "Invalid connection object.");
    }


    bool bRemoved=dict->remove(new String(key));
    if (bRemoved && sc != nullptr) {
        delete sc;
    }
    return new Int(MQTT_OK);
}
ConstantSP getSubscriberStat(const ConstantSP& handle, const ConstantSP& b){
    int size = dict->size();
    ConstantSP connectionIdVec = Util::createVector(DT_STRING, size);
    ConstantSP userVec = Util::createVector(DT_STRING, size);
    ConstantSP hostVec = Util::createVector(DT_STRING, size);
    ConstantSP portVec = Util::createVector(DT_INT, size);
    ConstantSP topicVec = Util::createVector(DT_STRING, size);
    ConstantSP timestampVec = Util::createVector(DT_TIMESTAMP, size);
    ConstantSP recvVec = Util::createVector(DT_LONG, size);
    VectorSP keys = dict->keys();
    for(int i = 0; i < keys->size();i++){
        string key = keys->getString(i);
        connectionIdVec->setString(i,key);
        ConstantSP conn = dict->getMember(key);
        SubConnection *sc = (SubConnection *)(conn->getLong());
        hostVec->setString(i,sc->getHost());
        portVec->setInt(i,sc->getPort());
        topicVec->setString(i,sc->getTopic());
        recvVec->setLong(i,sc->getRecv());
        timestampVec->setLong(i,sc->getCreateTime());
        userVec->setString(i,sc->session->getUser()->getUserId());
    }

    vector<string> colNames = {"subscriptionId","user","host","port","topic","createTimestamp","receivedPackets"};
    vector<ConstantSP> cols = {connectionIdVec,userVec,hostVec,portVec,topicVec,timestampVec,recvVec};
    return Util::createTable(colNames,cols);
}
/// Connection
namespace mqtt {

SubConnection::~SubConnection() {
    isClosed_ = true;
    clientDaemon_->join();
    LOG_INFO("[PluginMQTT]: SubConnection is freed");
    sockfd_->close();
}

SubConnection::SubConnection(const std::string& hostname, int port, const std::string& topic, const FunctionDefSP& parser,
		const ConstantSP& handler, const std::string& userName, const std::string& password, Heap *pHeap)
    : ConnctionBase(new ConditionalNotifier()),
    host_(hostname), port_(port), topic_(topic), parser_(parser), handler_(handler), pHeap_(pHeap), userName_(userName), password_(password) 
    {
    LOG_INFO("[PluginMQTT]: crete SubConnection");
    sockfd_ = new Socket(hostname, port, false);
    IO_ERR ret = sockfd_->connect();
    if (ret != OK && ret != INPROGRESS) {
        throw RuntimeException("Failed to connect. ");
    }



    mqtt_init(&client_, sockfd_->getHandle(), sendbuf_, sizeof(sendbuf_), recvbuf_, sizeof(recvbuf_), subCallback);
    // mqtt_init_reconnect(&client_, reconnect_client, this, subCallback);

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

    connected_ = true;
    recv = 0;
    client_.publish_response_callback_state = this;
    createTime_=Util::getEpochTime();

    /* subscribe */
    mqtt_subscribe(&client_, topic.c_str(), 2);

    /* check for errors */
    if (client_.error != MQTT_OK) {
        throw RuntimeException(mqtt_error_str(client_.error));
    }
    session  =  pHeap->currentSession()->copy();
    session->setUser(pHeap->currentSession()->getUser());
}

void SubConnection::reconnect(){
    sockfd_ = new Socket(host_, port_, false);
    IO_ERR ret = sockfd_->connect();
    if (ret != OK && ret != INPROGRESS){
        LOG_ERR("[PluginMQTT]: Failed to connect. ");
        return;
    }
    mqtt_reinit(&client_, sockfd_->getHandle(), sendbuf_, sizeof(sendbuf_), recvbuf_, sizeof(recvbuf_));
    uint8_t connect_flags = MQTT_CONNECT_CLEAN_SESSION;
    const char* client_id = NULL;
    if(userName_=="") {
        mqtt_connect(&client_, client_id, NULL, NULL, 0, NULL, NULL, connect_flags, 400);
    }
    else{
        mqtt_connect(&client_, client_id, NULL, NULL, 0, userName_.c_str(), password_.c_str(), connect_flags, 400);
    }
    if (client_.error != MQTT_OK) {
        throw RuntimeException(mqtt_error_str(client_.error));
    }
    mqtt_subscribe(&client_, topic_.c_str(), 2);
    if (client_.error != MQTT_OK) {
        
        throw RuntimeException(mqtt_error_str(client_.error));
    }
}

// void reconnect_client(struct mqtt_client* client, void **reconnect_state_vptr)
// {
//     LOG_INFO("reconnect_client");
//     SubConnection* connection = *((SubConnection**) reconnect_state_vptr);

//     /* Close the clients socket if this isn't the initial reconnect call */
//     if (client->error != MQTT_ERROR_INITIAL_RECONNECT) {
//         close(client->socketfd);
//     }

//     /* Perform error handling here. */
//     if (client->error != MQTT_ERROR_INITIAL_RECONNECT) {
//         printf("reconnect_client: called while client was in error state \"%s\"\n",
//                mqtt_error_str(client->error)
//         );
//     }
//     connection->reconnect();
// }

}    // namespace mqtt
