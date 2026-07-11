#include "MQTTConnect.h"

namespace MQTTPlugin {

static long long getCurrentTimestamp() {
    return Util::toLocalTimestamp(Util::getEpochTime());
}

static void setLastError(AsyncContext *context, const string &msg) {
    context->lastErrMsg_ = msg;
    context->lastErrTime_ = getCurrentTimestamp();
}

static void setFailMsg(AsyncContext *context, const string &msg) {
    context->failMsg_ = PLUGIN_NAME + " " + msg;
    setLastError(context, msg);
    LOG_INFO(context->failMsg_);
}

void onConnectLost(void *context, char *) {
    AsyncContext *p = (AsyncContext *)context;
    LockGuard<Mutex> lock(&p->lock_);
    p->connectCompleted_ = true;
    p->isConnected_ = false;
    setFailMsg(p, "Connection lost.");
}

int onMessageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message) {
    AsyncContext *p = (AsyncContext *)context;
    LockGuard<Mutex> lock(&p->lock_);
    try {
        string topic(topicName, (topicLen > 0) ? topicLen : strlen(topicName));
        string msg((char *)message->payload, message->payloadlen);
        string errMsg;
        if (!reinterpret_cast<MQTTConnection *>(p->connection_)->handleMsg(topic, msg, errMsg)) {
            setLastError(p, errMsg);
        }
        p->receivedPacketNum_++;
    } catch (const std::exception &e) {
        LOG_INFO(PLUGIN_NAME, " Exception occurred while processing message: %s", e.what());
        setLastError(p, e.what());
    }
    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    return 1;
}

void onConnectSuccess(void *context, MQTTAsync_successData *) {
    AsyncContext *p = (AsyncContext *)context;
    LockGuard<Mutex> lock(&p->lock_);
    p->connectCompleted_ = true;
    LOG_INFO(PLUGIN_NAME, " Successful connection.");
}

void onConnectSuccess5(void *context, MQTTAsync_successData5 *) {
    AsyncContext *p = (AsyncContext *)context;
    LockGuard<Mutex> lock(&p->lock_);
    p->connectCompleted_ = true;
    LOG_INFO(PLUGIN_NAME, " Successful connection.");
}

void onConnectFailure(void *context, MQTTAsync_failureData *response) {
    AsyncContext *p = (AsyncContext *)context;
    LockGuard<Mutex> lock(&p->lock_);
    p->connectCompleted_ = true;
    setFailMsg(p, "Connect failed, return code is " + std::to_string(response ? response->code : -1));
}

void onConnectFailure5(void *context, MQTTAsync_failureData5 *response) {
    AsyncContext *p = (AsyncContext *)context;
    LockGuard<Mutex> lock(&p->lock_);
    p->connectCompleted_ = true;
    setFailMsg(p, "Connect failed, return code is " + std::to_string(response ? response->code : -1));
}

void onSendFailure(void *context, MQTTAsync_failureData *response) {
    AsyncContext *p = (AsyncContext *)context;
    LockGuard<Mutex> lock(&p->lock_);
    setFailMsg(p, "Publish msg failed, return code is " + std::to_string(response ? response->code : -1));
}

void onSendFailure5(void *context, MQTTAsync_failureData5 *response) {
    AsyncContext *p = (AsyncContext *)context;
    LockGuard<Mutex> lock(&p->lock_);
    setFailMsg(p, "Publish msg failed, return code is " + std::to_string(response ? response->code : -1));
}

void onSubscribe(void *context, MQTTAsync_successData *) {
    AsyncContext *p = (AsyncContext *)context;
    LockGuard<Mutex> lock(&p->lock_);
    p->subscribeCompleted_ = true;
    LOG_INFO(PLUGIN_NAME, " Successful connection.");
}

void onSubscribe5(void *context, MQTTAsync_successData5 *) {
    AsyncContext *p = (AsyncContext *)context;
    LockGuard<Mutex> lock(&p->lock_);
    p->subscribeCompleted_ = true;
    LOG_INFO(PLUGIN_NAME, " Successful connection.");
}

void onSubscribeFailure(void *context, MQTTAsync_failureData *response) {
    AsyncContext *p = (AsyncContext *)context;
    LockGuard<Mutex> lock(&p->lock_);
    p->subscribeCompleted_ = true;
    setFailMsg(p, "Subscribe failed, return code is " + std::to_string(response ? response->code : -1));
}

void onSubscribeFailure5(void *context, MQTTAsync_failureData5 *response) {
    AsyncContext *p = (AsyncContext *)context;
    LockGuard<Mutex> lock(&p->lock_);
    p->subscribeCompleted_ = true;
    setFailMsg(p, "Subscribe failed, return code is " + std::to_string(response ? response->code : -1));
}

void MQTTConnection::connect() {
    context_.connection_ = this;
    int rc = -1;
    // connect with options
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    if (mqttVersion_ == MQTTVERSION_5) {
        conn_opts = MQTTAsync_connectOptions_initializer5;
    }
    if (!username_.empty()) conn_opts.username = username_.c_str();
    if (!password_.empty()) conn_opts.password = password_.c_str();
    conn_opts.connectTimeout = 20;
    if (mqttVersion_ == MQTTVERSION_5) {
        conn_opts.onSuccess5 = onConnectSuccess5;
        conn_opts.onFailure5 = onConnectFailure5;
    } else {
        conn_opts.onSuccess = onConnectSuccess;
        conn_opts.onFailure = onConnectFailure;
    }

    conn_opts.context = &context_;
    conn_opts.MQTTVersion = mqttVersion_;

    MQTTAsync_SSLOptions ssl_opts = MQTTAsync_SSLOptions_initializer;
    ssl_opts.enableServerCertAuth = (enableServerCertAuth_) ? 1 : 0;
    if (!trustStore_.empty()) ssl_opts.trustStore = trustStore_.c_str();
    if (useSSL_) {
        conn_opts.ssl = &ssl_opts;
    }
    if ((rc = MQTTAsync_connect(handle_, &conn_opts)) != MQTTASYNC_SUCCESS) {
        throw RuntimeException(PLUGIN_NAME + " Failed to start connect, return code " + std::to_string(rc) + ".");
    }

    for (int i = 0; i < 300; ++i) {  // Wait for connection to complete, timeout is 30s
        {
            LockGuard<Mutex> lock(&context_.lock_);
            if (context_.connectCompleted_) {
                if (!context_.failMsg_.empty()) {
                    throw RuntimeException(context_.failMsg_);
                }
                break;
            }
        }
        Util::sleep(100L);
    }
    if (!context_.connectCompleted_) {
        close();
        throw RuntimeException(PLUGIN_NAME + " Connect timeout.");
    }
    LockGuard<Mutex> lock(&context_.lock_);
    context_.isConnected_ = true;
}

bool MQTTConnection::close() {
    if (!handle_) return false;

    MQTTAsync_disconnectOptions discOpts = MQTTAsync_disconnectOptions_initializer;
    discOpts.timeout = 1000;
    MQTTAsync_disconnect(handle_, &discOpts);
    MQTTAsync_destroy(&handle_);
    LOG_INFO(PLUGIN_NAME, " Connection closed: ", clientId_);

    LockGuard<Mutex> lock(&context_.lock_);
    context_.isConnected_ = false;
    handle_ = nullptr;
    return true;
}

void MQTTConnection::publish(const string &topic, const string &msg) {
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    pubmsg.payload = (void *)msg.c_str();
    pubmsg.payloadlen = static_cast<int>(msg.length());
    pubmsg.qos = qos_;
    pubmsg.retained = 0;

    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    if (mqttVersion_ == MQTTVERSION_5) {
        opts.onFailure5 = onSendFailure5;
    } else {
        opts.onFailure = onSendFailure;
    }
    opts.context = &context_;

    int rc = -1;
    if ((rc = MQTTAsync_sendMessage(handle_, topic.c_str(), &pubmsg, &opts)) != MQTTASYNC_SUCCESS) {
        throw RuntimeException(PLUGIN_NAME + " Failed to publish msg, return code is " + std::to_string(rc));
    }
}

void MQTTConnection::subscribe(Heap *heap) {
    try {
        session_ = heap->currentSession()->copy();
        session_->setUser(heap->currentSession()->getUser());
        session_->setOutput(new DummyOutput());

        connect();
        createdTime_ = Util::toLocalTimestamp(Util::getEpochTime());

        MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
        int rc;
        LOG_INFO(PLUGIN_NAME, " Subscribing to topic ", topic_, " for client ", uri_);
        if (mqttVersion_ == MQTTVERSION_5) {
            opts.onSuccess5 = onSubscribe5;
            opts.onFailure5 = onSubscribeFailure5;
        } else {
            opts.onSuccess = onSubscribe;
            opts.onFailure = onSubscribeFailure;
        }
        opts.context = &context_;
        if ((rc = MQTTAsync_subscribe(handle_, topic_.c_str(), qos_, &opts)) != MQTTASYNC_SUCCESS) {
            throw RuntimeException(PLUGIN_NAME + " Failed to start subscribe, return code " + std::to_string(rc));
        }

        for (int i = 0; i < 300; ++i) {  // Wait for subscribe to complete, timeout is 30s
            {
                LockGuard<Mutex> lock(&context_.lock_);
                if (context_.subscribeCompleted_) {
                    if (!context_.failMsg_.empty()) {
                        throw RuntimeException(context_.failMsg_);
                    }
                    break;
                }
            }
            Util::sleep(100L);
        }
        if (!context_.subscribeCompleted_) {
            throw RuntimeException(PLUGIN_NAME + " Subscribe timeout.");
        }
    } catch (exception &e) {
        string errMsg(e.what());
        LOG_ERR(PLUGIN_NAME + " mqtt subscribe init failed, error message is <", errMsg, ">");
        throw RuntimeException(PLUGIN_NAME + " mqtt subscribe init failed, error message is <" + errMsg + ">");
    }
}

bool MQTTConnection::handleMsg(const string &topic, const string &msg, string &errMsg) {
    try {
        Heap *heap = session_->getHeap().get();
        vector<ConstantSP> args = {new String(topic), new String(msg)};
        handler_->call(heap, args);
        return true;
    } catch (exception &e) {
        errMsg = e.what();
        LOG_INFO(PLUGIN_NAME, " Handle msg exception:", e.what());
        return false;
    }
}

void MQTTConnection::initContext() {
    LOG_INFO(PLUGIN_NAME, " MQTT connection uri is ", uri_);
    int rc = -1;
    // create client with options and set callbacks
    MQTTAsync_createOptions create_opts = MQTTAsync_createOptions_initializer;
    create_opts.MQTTVersion = mqttVersion_;
    create_opts.maxBufferedMessages = maxBufferedMessages_;
    if ((rc = MQTTAsync_createWithOptions(&handle_, uri_.c_str(), clientId_.c_str(), MQTTCLIENT_PERSISTENCE_NONE, NULL,
                                          &create_opts)) != MQTTASYNC_SUCCESS) {
        throw RuntimeException(PLUGIN_NAME + " Failed to create client, return code is " + std::to_string(rc) + ".");
    }
    if ((rc = MQTTAsync_setCallbacks(handle_, &context_, onConnectLost, onMessageArrived, NULL)) != MQTTASYNC_SUCCESS) {
        throw RuntimeException(PLUGIN_NAME + " Failed to set callback, return code is " + std::to_string(rc) + ".");
    }
}

}  // namespace MQTTPlugin
