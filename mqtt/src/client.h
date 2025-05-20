/*
 * client.h
 *
 *  Created on: Apr 19, 2019
 *      Author: xjqian
 */

#ifndef MQTT_H_
#define MQTT_H_

#include "CoreConcept.h"
#include "ddbplugin/PluginLogger.h"
#include "Util.h"
#include "mqtt.h"

extern "C" ConstantSP mqttClientSub(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP mqttClientStopSub(const ConstantSP &handle, const ConstantSP &b);

extern "C" ConstantSP mqttClientConnect(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP mqttClientPub(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP mqttClientCreatePublisher(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP mqttClientClose(const ConstantSP &handle, const ConstantSP &b);
extern "C" ConstantSP getSubscriberStat(const ConstantSP &handle, const ConstantSP &b);

static const string LOG_PRE_STR = "[PLUGIN:MQTT]";
static const int MAX_RETRY_COUNT = 10;
namespace mqttConn {
static DictionarySP CONN_DICT = Util::createDictionary(DT_STRING, 0, DT_ANY, 0);
static Mutex CONN_MUTEX_LOCK;
}  // namespace mqttConn
namespace mqtt {

struct MqttInfo {
    std::string topic;
    std::string msg;
};

class ConnctionBase {
  public:
    ConnctionBase(SmartPointer<ConditionalNotifier> freeNotifier) : freeNotifier_(freeNotifier) {}
    virtual struct mqtt_client *getClient() = 0;
    virtual void reconnect() = 0;
    virtual bool isClosed() = 0;
    virtual void handleMsg(const std::string &topic, const std::string &msg) {}
    bool received() { return received_; }
    void resetReceived() { received_ = false; }
    void setReceived() { received_ = true; }

  protected:
    bool received_ = false;
    SmartPointer<ConditionalNotifier> freeNotifier_;
};

class Connection;
class Connection : public ConnctionBase {
  public:
    Connection(const std::string &hostname, int port, uint8_t qos, const FunctionDefSP &formatter, int batchSize,
               const std::string &userName, const std::string &password, int sendbufSize, int recvbufSize,
               const string &clientID);
    virtual ~Connection();
    MQTTErrors publishMsg(const char *topic_name, void *application_message, size_t application_message_size);
    FunctionDefSP getFormatter() { return formatter_; }
    int getBatchSize() { return batchSize_; }
    void reconnect() override {
        sockfd_ = new Socket(host_, port_, false);
        IO_ERR ret = sockfd_->connect();
        if (ret != OK && ret != INPROGRESS) {
            PLUGIN_LOG_ERR("[PluginMQTT]: Failed to connect. ");
            return;
        }
        mqtt_reinit(&client_, sockfd_->getHandle(), sendbuf_.get(), sendbufSize_, recvbuf_.get(), recvbufSize_);
        uint8_t connect_flags = MQTT_CONNECT_CLEAN_SESSION;
        string client_id = clientID_.empty() ? "ddb_mqtt_plugin_pub" : clientID_;
        if (userName_ == "") {
            // mqtt_connect(&client_, client_id, NULL, NULL, 0, NULL, NULL, connect_flags, 400);
            mqtt_connect(&client_, client_id.c_str(), NULL, NULL, 0, NULL, NULL, connect_flags, 30);
        } else {
            mqtt_connect(&client_, client_id.c_str(), NULL, NULL, 0, userName_.c_str(), password_.c_str(),
                         connect_flags, 400);
        }
    }

    bool isClosed() override { return isClosed_; }

    void close() { isClosed_ = true; }

    struct mqtt_client *getClient() override { return &client_; }

  private:
    std::string host_;
    int port_ = 1883;
    uint8_t publishFlags_;
    FunctionDefSP formatter_;
    int batchSize_;

    SocketSP sockfd_;
    bool isClosed_ = false;
    struct mqtt_client client_;
    SmartPointer<Mutex> lockClient_;
    ThreadSP clientDaemon_;
    int sendtimes_;
    int failed_;
    string userName_;
    string password_;
    string clientID_;

    int sendbufSize_;
    int recvbufSize_;
    std::unique_ptr<uint8_t[]> sendbuf_ = nullptr; /* sendbuf should be large enough to hold multiple
                                whole mqtt messages */
    std::unique_ptr<uint8_t[]> recvbuf_ = nullptr; /* recvbuf should be large enough any whole mqtt
                                message expected to be received */
};

class SubConnection : public ConnctionBase {
  private:
    std::string host_;
    int port_ = 1883;
    std::string topic_;

    SocketSP sockfd_;
    struct mqtt_client client_;
    ThreadSP clientDaemon_;
    ThreadSP asyncHandleMsgThread_;
    SmartPointer<Mutex> lockClient_;

    bool connected_;
    bool isAsync_;
    long long recv;  // received packet number
    long long createTime_;

    FunctionDefSP parser_;
    ConstantSP handler_;
    Heap *pHeap_;
    string userName_;
    string password_;
    string clientID_;

    int sendbufSize_;
    int recvbufSize_;
    std::unique_ptr<uint8_t[]> sendbuf_ = nullptr; /* sendbuf should be large enough to hold multiple
                                whole mqtt messages */
    std::unique_ptr<uint8_t[]> recvbuf_ = nullptr; /* recvbuf should be large enough any whole mqtt
                                message expected to be received */

    bool isClosed_ = false;
    SynchronizedQueue<MqttInfo> msgQueue_;

  public:
    SessionSP session;

  public:
    SubConnection(const std::string &hostname, int port, const std::string &topic, const FunctionDefSP &parser,
                  const ConstantSP &handler, const std::string &userName, const std::string &password, Heap *pHeap,
                  int sendbufSize, int recvbufSize, const std::string clientID, bool isAsync);

    virtual ~SubConnection();
    ConstantSP getHandler() { return handler_; }
    Heap *getHeap() { return pHeap_; }
    FunctionDefSP getParser() { return parser_; }
    void incRecv() { recv++; }
    long long getRecv() { return recv; }
    string getTopic() { return topic_; }
    long long getCreateTime() { return createTime_; }
    string getHost() { return host_; }
    int getPort() { return port_; }

    struct mqtt_client *getClient() override { return &client_; }

    void reconnect() override;
    void handleMsg(const std::string &topic, const std::string &msg) override;
    bool isClosed() override { return isClosed_; }
    void startAsyncHandleMsgThread();

  private:
    void asyncHandleMsg();
    void handleMsgInternal(const std::string &topic, const std::string &msg);
};

bool checkConnectStatus(int sockfd);
void checkConnack(mqtt_client *client);

}  // namespace mqtt

#endif /* MQTT_H_ */
