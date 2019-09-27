/*
 * client.h
 *
 *  Created on: Apr 19, 2019
 *      Author: xjqian
 */

#ifndef MQTT_H_
#define MQTT_H_

#include "CoreConcept.h"
#include "mqtt.h"

extern "C" ConstantSP mqttClientSub(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP mqttClientStopSub(const ConstantSP& handle, const ConstantSP& b);

extern "C" ConstantSP mqttClientConnect(Heap* heap, vector<ConstantSP>& args);
extern "C" ConstantSP mqttClientPub(Heap* heap, vector<ConstantSP>& args);
extern "C" ConstantSP mqttClientClose(const ConstantSP& handle, const ConstantSP& b);

namespace mqtt {

class Connection {
private:
    std::string host_;
    // std::string user_;
    // std::string password_;
    int port_ = 1883;
    uint8_t publishFlags_;
    FunctionDef* formatter_;
    int batchSize_;

    int sockfd_;
    struct mqtt_client client_;
    uint8_t sendbuf_[20480]; /* sendbuf should be large enough to hold multiple
                                whole mqtt messages */
    uint8_t recvbuf_[10240]; /* recvbuf should be large enough any whole mqtt
                                message expected to be received */
    pthread_t clientDaemon_;

    bool connected_;
    int sendtimes;
    int failed;

public:
    Connection();
    Connection(std::string host, int port, uint8_t qos, FunctionDef* formatter, int batchSize);
    ~Connection();

    MQTTErrors publishMsg(const char* topic_name, void* application_message, size_t application_message_size);
    bool getConnected() {
        return connected_;
    }
    FunctionDef* getFormatter() {
        return formatter_;
    }
    int getBatchSize() {
        return batchSize_;
    }
};

class SubConnection {
private:
    std::string host_;
    // std::string user_;
    // std::string password_;
    int port_ = 1883;
    std::string topic_;

    int sockfd_;
    struct mqtt_client client_;
    uint8_t sendbuf_[20480]; /* sendbuf should be large enough to hold multiple
                                whole mqtt messages */
    uint8_t recvbuf_[20480]; /* recvbuf should be large enough any whole mqtt
                                message expected to be received */
    pthread_t clientDaemon_;

    bool connected_;
    int recv;    // received packet number

    FunctionDef* parser_;
    ConstantSP handler_;
    Heap* pHeap_;

public:
    SubConnection();
    SubConnection(std::string hostname, int port, std::string topic, FunctionDef* parser, ConstantSP handler,
                  Heap* pHeap);
    ~SubConnection();
    ConstantSP getHandler() {
        return handler_;
    }
    Heap* getHeap() {
        return pHeap_;
    }
    FunctionDef* getParser() {
        return parser_;
    }
    void incRecv() {
        recv++;
    }
};

}    // namespace mqtt

#endif /* MQTT_H_ */
