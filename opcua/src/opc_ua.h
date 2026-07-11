// SPDX-License-Identifier: Apache-2.0
// Copyright © 2025-2025 DolphinDB, Inc.
#pragma once

#include "Open62541Everything.h"

#include "DolphinDBEverything.h"
#include "ddbplugin/PluginLogger.h"
#include "CoreConcept.h"
#include "ScalarImp.h"
#include "ddbplugin/ThreadedQueue.h"
#include "ddbplugin/CommonInterface.h"

using namespace ddb;

extern "C" ConstantSP getOpcUaServerList(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP getOpcUaEndPointList(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP connectOpcUaServer(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP disconnect(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP readNode(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP writeNode(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP subscribeNode(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP unsubscribeNode(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP browseNode(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP getSubscriberStat(Heap *heap, vector<ConstantSP> &arguments);

const string OPCUA_CLIENT_DESC = "opc ua connection";
const static string OPCUA_PREFIX = "[PLUGIN::OPCUA] ";

inline static void mockOnClose(Heap *heap, vector<ConstantSP> &args) { std::ignore = heap; std::ignore = args; }

static UA_INLINE UA_Int64 UA_DateTime_toUnixTimeStamp(UA_DateTime date) {
    return (date + UA_DateTime_localTimeUtcOffset() - UA_DATETIME_UNIX_EPOCH) / UA_DATETIME_MSEC;
}
static UA_INLINE UA_DateTime UnixTimeStamp_To_UA_DateTime(UA_Int64 date) {
    return date * UA_DATETIME_MSEC + UA_DATETIME_UNIX_EPOCH - UA_DateTime_localTimeUtcOffset();
}

class OPCUAClient;
using OPCUAClientSP = ObjectPtr<OPCUAClient>;
class OPCUASub {
  public:
    static Mutex OPCUA_SUB_MAP_LATCH;
    static std::unordered_map<long long, SmartPointer<OPCUASub>> OPCUA_SUB_MAP;

    OPCUASub(Heap *heap, string endPoint, string clientUri, OPCUAClientSP client, const vector<int> &nsIdx,
             const vector<string> &nodeIdString, ConstantSP &handle, bool reconnect, long long resubTimeout,
             const string &actionName);
    ~OPCUASub() {
        try {
            if (running_) {
                stopThread();
            }
        } catch (std::exception &e) {
            LOG_ERR(OPCUA_PREFIX, "destruction of OPCUASub failed due to ", e.what());
        }
    }
    void subs();
    void unSub();
    void startThread();
    void stopThread();
    void reconnect();

    long long getSubID() { return subId_; };
    long long getTimeGap() { return timeGap_; }
    long long getReconnectTime() { return reconnectTime_; }
    Heap *getHeap() { return heap_; }
    ConstantSP getHandle() { return handle_; }
    UA_Client *getClientPtr() const;

    string getActionName() { return actionName_; }
    string getClientUri() { return clientUri_; }
    string getUser() { return user_; }
    string getEndPoint() { return endPoint_; };
    bool isConnected() { return isConnected_; }
    string getNodeID() {
        string result;
        for (size_t i = 0; i < nsIdx_.size(); i++) {
            result += std::to_string(nsIdx_[i]) + ":" + nodeIdString_[i] + ";";
        }
        return result;
    }
    StreamStatus &getStatus() { return status_; }

    std::map<UA_UInt32, string> monMap_;

  private:
    bool running_ = false;
    bool reconnect_ = false;
    bool isConnected_ = true;
    UA_UInt32 subId_ = 0;
    long long resubTimeout_ = 0;
    long long reconnectTime_ = LONG_LONG_MIN;
    long long timeGap_ = 0;
    string actionName_;
    string user_;
    string endPoint_;
    string clientUri_;

    Mutex mutex_;
    OPCUAClientSP client_;
    ConstantSP handle_ = nullptr;
    ThreadSP thread_ = nullptr;
    Heap *heap_;  // always use the heap from client_
    StreamStatus status_;
    vector<int> nsIdx_;
    vector<string> nodeIdString_;
};
using OPCUASubSP = SmartPointer<OPCUASub>;

class OPCUAClient : public Resource {
  public:
    OPCUAClient(Heap *heap)
        : Resource(0, OPCUA_CLIENT_DESC, Util::createSystemProcedure("OPCUA client onClose()", mockOnClose, 1, 1),
                   heap->currentSession()) {
        clientPtr_ = UA_Client_new();
        session_ = heap->currentSession()->copy();
        session_->setUser(heap->currentSession()->getUser());
        session_->setOutput(new DummyOutput);
    }
    ~OPCUAClient() {
        if (clientPtr_) {
            UA_Client_delete(clientPtr_);
            clientPtr_ = nullptr;
        }
    }
    void disconnect() {
        LockGuard<Mutex> lock(&mutex_);
        UA_Client_disconnect(clientPtr_);
    }
    void setSubscribeFlag() {
        LockGuard<Mutex> lock(&mutex_);
        isSubscribed = true;
    }

    void connect(string endPointUrl, string clientUri, string username, string password,
                 UA_MessageSecurityMode securityMode, UA_String securityPolicy, UA_ByteString certificate,
                 UA_ByteString privateKey, bool reconnect = false);
    void reconnect();
    ConstantSP readNode(const vector<int> &nsIdx, const vector<string> &nodeIdString, ConstantSP &table);
    ConstantSP writeNode(const vector<int> &nsIdx, const vector<string> &nodeIdString, ConstantSP &value);
    void browseNode(UA_NodeId object, vector<int> &nameSpace, vector<string> &nodeid);
    ConstantSP browseNode();
    bool getConnected() {
        LockGuard<Mutex> lock(&mutex_);
        return getConnectedUnsafe();
    }

    UA_Client *getClientPtr() const { return clientPtr_; }
    void setSession(const SessionSP &sess) { session_ = sess; }
    SessionSP getSession() { return session_; }
    bool getSessionStat() { return sessionClosed_; }
    void setSessionStat(bool flag) { sessionClosed_ = flag; }
    string getClientUrl() { return clientUri_; }
    string getConnEndPointUrl() { return endPointUrl_; }

  private:
    bool getConnectedUnsafe() {
        UA_SecureChannelState channelState = UA_SECURECHANNELSTATE_CLOSED;
        UA_StatusCode connectStatus = UA_STATUSCODE_GOOD;
        UA_Client_getState(clientPtr_, &channelState, nullptr, &connectStatus);
        return connectStatus == UA_STATUSCODE_GOOD && channelState != UA_SECURECHANNELSTATE_CLOSED;
    }

    UA_Client *clientPtr_ = nullptr;
    ThreadSP thread_;
    bool isSubscribed = false;
    bool sessionClosed_ = false;
    Mutex mutex_;
    SessionSP session_;
    string clientUri_;
    string endPointUrl_;
    string username_;
    string password_;
    UA_MessageSecurityMode securityMode_;
    UA_String securityPolicy_;
    UA_ByteString certificate_;
    UA_ByteString privateKey_;
};
