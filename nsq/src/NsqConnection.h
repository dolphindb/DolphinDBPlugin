//
// Created by htxu on 11/20/2023.
//

#ifndef PLUGINNSQ_NSQCONNECTION_H
#define PLUGINNSQ_NSQCONNECTION_H

#include "DolphinDBEverything.h"
#include <CoreConcept.h>

#include "HSNsqApi.h"

#include "NsqSpiImpl.h"

/**
 * Includes all variables related to the state of the NSQ API, and all methods that cause state changes.
 */

using namespace ddb;

class NsqConnection {

    /// Singleton
public:
    // delete copy constructor and assignment operator
    NsqConnection(const NsqConnection&) = delete;
    NsqConnection& operator=(const NsqConnection&) = delete;
    // instance operations
    static void initInstance(const string &configFilePath, const DictionarySP& options, const string &username, const string &password, const string &version);
    static SmartPointer<NsqConnection> getInstance();
    static void destroyInstance();
private:
    // private constructor and singleton
    NsqConnection() = default;
    static SmartPointer<NsqConnection> instancePtr;

    /// Interfaces
public:
    ConstantSP getSchema(const string &dataType);
    void subscribe(Heap *heap, const string &dataType, const string &marketType, const TableSP &table);
    void subscribeTradeEntrust(Heap *heap, const string &dataType, const string &marketType, const DictionarySP &tableDict);
    void unsubscribe(const string &dataType, const string &marketType);
    ConstantSP getStatus();

    static Mutex* getMutex();

    // For NsqSpiImpl
    // conditional variables notify
    static void connectionNotifyL();
    static void loginNotifyL();

    void connect(const string &configFilePath);
    void login(const string &username, const string &password);

private:
    /// Helper Methods
    void parseOptions(const DictionarySP& options);
    void subscribeOrCancel(const string &dataType, const string &marketType, bool cancel = false);

    static Mutex mutex;
    static const int TIMEOUT_MS;

    /// Member Vars
    CHSNsqApi* api_ = nullptr;
    SmartPointer<CHSNsqSpiImpl> spi_ = new CHSNsqSpiImpl();

    string configFilePath_;
    string username_;
    string password_;
    string dataVersion_;
    string loginErrMsg_;

    int nRequestID_ = 0;

    bool isConnected_ = false;
    bool isLoggedIn_ = false;

    Mutex connectionM_;
    Mutex loginM_;
    ConditionalVariable connectionCV_;
    ConditionalVariable loginCV_;
};


#endif //PLUGINNSQ_NSQCONNECTION_H
