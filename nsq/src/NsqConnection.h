#pragma once

#include "ddb_nsq.h"
#include "NsqEverything.h"

#include "DolphinDBEverything.h"
#include <CoreConcept.h>

/**
 * Includes all variables related to the state of the NSQ API, and all methods that cause state changes.
 */

using namespace ddb;

class CHSNsqSpiImpl;

class NsqConnection {
  public:
    NsqConnection() = default;
    NsqConnection(const NsqConnection&) = delete;
    NsqConnection& operator=(const NsqConnection&) = delete;
    void init(const string &configFilePath, const DictionarySP& options, const string &username, const string &password,
              nsq_version version, CHSNsqSpiImpl *spi);
    void destroyInstance(CHSNsqSpiImpl *spi = nullptr);
    ConstantSP getSchema(nsq_data dataType);
    void subscribe(Heap *heap, nsq_data data, nsq_market market, const TableSP &table, long long queueDepth, const vector<string> &codes = {});
    void subscribeTradeEntrust(Heap *heap, nsq_market market, const DictionarySP &tableDict, long long queueDepth, const vector<string> &codes = {});
    void unsubscribe(nsq_data data, nsq_market market);
    ConstantSP getStatus();

    static Mutex* getMutex();

    // For NsqSpiImpl
    // conditional variables notify
    void connectionNotifyL();
    void loginNotifyL();

    void connect(const string &configFilePath, CHSNsqSpiImpl *spi);
    void login(const string &username, const string &password);

private:
    /// Helper Methods
    void parseOptions(const DictionarySP& options, CHSNsqSpiImpl *spi);
    void subscribeOrCancel(nsq_data data, nsq_market market, bool cancel = false, const vector<string> &codes = {});

    static Mutex mutex;
    static const int TIMEOUT_MS;

    /// Member Vars
    CHSNsqApi* api_ = nullptr;

    string configFilePath_;
    string username_;
    string password_;
    string loginErrMsg_;

    int nRequestID_ = 0;

    bool isConnected_ = false;
    bool isLoggedIn_ = false;

    Mutex connectionM_;
    Mutex loginM_;
    ConditionalVariable connectionCV_;
    ConditionalVariable loginCV_;
    
    nsq_version dataVersion_;
};
