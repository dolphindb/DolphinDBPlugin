#pragma once

#include "MQTTAsync.h"
#include "MQTTUtil.h"

namespace MQTTPlugin {
using namespace ddb;

struct AsyncContext {  // Async callback threads share the same context
    Mutex lock_;
    bool connectCompleted_{false};
    bool isConnected_{false};
    string failMsg_;
    long long receivedPacketNum_{0};
    string lastErrMsg_;
    long long lastErrTime_{0};
    bool subscribeCompleted_{false};
    void *connection_{nullptr};
};

class MQTTConnection {
  public:
    inline MQTTConnection(const string &uri, const string &username, const string &password, const string &clientId,
                          int mqttVersion, bool useSSL, bool enableServerCertAuth, const string &trustStore, int qos,
                          int maxBufferedMessages, const string &topic, const FunctionDefSP &handler)
        : uri_(uri),
          username_(username),
          password_(password),
          clientId_(clientId),
          mqttVersion_(mqttVersion),
          useSSL_(useSSL),
          enableServerCertAuth_(enableServerCertAuth),
          trustStore_(trustStore),
          qos_(qos),
          maxBufferedMessages_(maxBufferedMessages),
          topic_(topic),
          handler_(handler) {
        if (clientId_.empty()) {
            clientId_ = generateClientId();
        }
        initContext();
    }
    inline ~MQTTConnection() { close(); }

    void connect();
    bool close();
    void publish(const string &topic, const string &msg);
    void subscribe(Heap *heap);
    bool handleMsg(const string &topic, const string &msg, string &errMsg);

    inline Mutex *getLock() { return &mutex_; }
    inline bool isConnected() {
        LockGuard<Mutex> lock(&context_.lock_);
        return context_.isConnected_;
    }
    inline long long getReceivedPacketNum() {
        LockGuard<Mutex> lock(&context_.lock_);
        return context_.receivedPacketNum_;
    }
    inline void getLastError(string &lastErrMsg, long long &lastErrTime) {
        LockGuard<Mutex> lock(&context_.lock_);
        lastErrMsg = context_.lastErrMsg_;
        lastErrTime = context_.lastErrTime_;
    }

    inline long long getCreatedTime() const { return createdTime_; }
    inline string getSessionUserId() const { return (session_.isNull()) ? "" : session_->getUser()->getUserId(); }
    inline string getUri() const { return uri_; }
    inline string getTopic() const { return topic_; }

  private:
    void initContext();

  private:
    Mutex mutex_;
    MQTTAsync handle_;
    AsyncContext context_;
    SessionSP session_;
    long long createdTime_{0};

    // input parameters
    string uri_;
    string username_;
    string password_;
    string clientId_;
    int mqttVersion_{MQTTVERSION_DEFAULT};
    bool useSSL_{false};
    bool enableServerCertAuth_{false};
    string trustStore_;
    int qos_{0};
    int maxBufferedMessages_{100};
    string topic_;
    FunctionDefSP handler_;
};

}  // namespace MQTTPlugin
