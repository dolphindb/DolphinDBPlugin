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
extern "C" ConstantSP getSubscriberStat(const ConstantSP& handle, const ConstantSP& b);
namespace mqtt {

class Connection {
private:
    std::string host_;
    int port_ = 1883;
    uint8_t publishFlags_;
    FunctionDefSP formatter_;
    int batchSize_;

    int sockfd_;
    struct mqtt_client client_;
    uint8_t sendbuf_[40960]; /* sendbuf should be large enough to hold multiple
                                whole mqtt messages */
    uint8_t recvbuf_[20480]; /* recvbuf should be large enough any whole mqtt
                                message expected to be received */
    pthread_t clientDaemon_;

    bool connected_;
    int sendtimes;
    int failed;

public:
    Connection();
    Connection(std::string host, int port, uint8_t qos, FunctionDefSP formatter, int batchSize,std::string userName,std::string password);
    ~Connection();

    MQTTErrors publishMsg(const char* topic_name, void* application_message, size_t application_message_size);
    bool getConnected() {
        return connected_;
    }
    FunctionDefSP getFormatter() {
        return formatter_;
    }
    int getBatchSize() {
        return batchSize_;
    }
};

class SubConnection {
private:
    std::string host_;
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
    long long recv;    // received packet number
    long long createTime_;

    FunctionDefSP parser_;
    ConstantSP handler_;
    Heap* pHeap_;
public:
    SessionSP session;

public:
    SubConnection();
    SubConnection(std::string hostname, int port, std::string topic, FunctionDefSP parser, ConstantSP handler,
                  std::string userName,std::string password,Heap* pHeap);
    ~SubConnection();
    ConstantSP getHandler() {
        return handler_;
    }
    Heap* getHeap() {
        return pHeap_;
    }
    FunctionDefSP getParser() {
        return parser_;
    }
    void incRecv() {
        recv++;
    }
    long long getRecv(){
        return recv;
    }
    string getTopic(){
        return topic_;
    }
    long long getCreateTime(){
        return createTime_;
    }
    string getHost(){
        return host_;
    }
    int getPort(){
        return port_;
    }
};

}    // namespace mqtt

#endif /* MQTT_H_ */
