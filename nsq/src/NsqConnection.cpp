//
// Created by htxu on 11/20/2023.
//

#include "NsqConnection.h"

Mutex NsqConnection::mutex;
const int NsqConnection::TIMEOUT_MS = 3000;

SmartPointer<NsqConnection> NsqConnection::instancePtr;

SmartPointer<NsqConnection> NsqConnection::getInstance() {

    if (instancePtr == nullptr) {
        throw RuntimeException(NSQ_PREFIX + "there is no connection.");
    }
    return instancePtr;
}

void NsqConnection::connectAndLogin(const string &configFilePath, const string &username, const string &password) {

    /// already connected and logged in
    if (isConnected_ and isLoggedIn_) {
        throw RuntimeException(NSQ_PREFIX + "there is already a connection.");
    }

    /// create and register
    api_ = NewNsqApiExt("", configFilePath.c_str());
    api_->RegisterSpi(spi_.get());

    /// connect (using Init)
    LockGuard<Mutex> connectionL(&connectionM_);
    if (api_->Init("") != 0) {
        throw RuntimeException(NSQ_PREFIX + "api init failed.");
    }
    // use cond var to wait
    if (!connectionCV_.wait(connectionM_, TIMEOUT_MS)) {
        throw RuntimeException(NSQ_PREFIX + "connection timeout");
    }

    /// login
    LockGuard<Mutex> loginL(&loginM_);
    CHSNsqReqUserLoginField reqLoginField;
    strncpy(reqLoginField.AccountID, username.c_str(), sizeof(reqLoginField.AccountID)-1);
    strncpy(reqLoginField.Password, password.c_str(), sizeof(reqLoginField.Password)-1);
    if (api_->ReqUserLogin(&reqLoginField, nRequestID_++) != 0) {
        throw RuntimeException(NSQ_PREFIX + "login failed");
    }
    // use cond var to wait
    if (!loginCV_.wait(loginM_, TIMEOUT_MS)) {
        throw RuntimeException(NSQ_PREFIX + "login timeout");
    }
}

void NsqConnection::connectionNotifyL() {

    if (instancePtr != nullptr) {
        LockGuard<Mutex> l(&instancePtr->connectionM_);

        instancePtr->isConnected_ = true;
        instancePtr->connectionCV_.notify();
    } else {
        // optimization: log
    }
}

void NsqConnection::loginNotifyL() {

    if (instancePtr != nullptr) {
        LockGuard<Mutex> l(&instancePtr->loginM_);

        instancePtr->isLoggedIn_ = true;
        instancePtr->loginCV_.notify();
    } else {
        // optimization: log
    }
}

void NsqConnection::initInstance(const string &configFilePath, const DictionarySP& options, const string &username, const string &password, const string &version) {

    // init singleton
    if (instancePtr == nullptr) {
        instancePtr = new NsqConnection();
        instancePtr->configFilePath_ = configFilePath;
        instancePtr->username_ = username;
        instancePtr->password_ = password;
        instancePtr->dataVersion_ = version;
    } else {
        throw RuntimeException(NSQ_PREFIX + "there is already a connection.");
    }

    try {
        if (!options.isNull()) {
            instancePtr->parseOptions(options);
        }
        instancePtr->connectAndLogin(configFilePath, username, password);
    } catch (std::exception &e) {
        NsqConnection::destroyInstance();
        throw RuntimeException(e.what());
    } catch (...) {
        NsqConnection::destroyInstance();
        throw RuntimeException(NSQ_PREFIX + "init failed.");
    }
    instancePtr->spi_->isConnected_ = true;
}

void NsqConnection::destroyInstance() {

    if (instancePtr != nullptr) {
        instancePtr->spi_->isConnected_ = false;
        if(instancePtr->api_) { instancePtr->api_->ReleaseApi(); }
        instancePtr.clear();
    } else {
        throw RuntimeException(NSQ_PREFIX + "there is no connection to close.");
    }
}

void NsqConnection::subscribeOrCancel(const string &dataType, const string &marketType, bool cancel) {

    /// set request field for subscription
    CHSNsqReqSecuDepthMarketDataField reqFieldSub[1];
    int nCount = 0;
    if (marketType == nsqUtil::SH) {
        memcpy(reqFieldSub[0].ExchangeID, HS_EI_SSE, sizeof(HS_EI_SSE));
    } else {
        memcpy(reqFieldSub[0].ExchangeID, HS_EI_SZSE, sizeof(HS_EI_SZSE));
    }

    /// request subscription
    int ret;

    try {
        if (dataType == nsqUtil::SNAPSHOT) {

            if (cancel) ret = api_->ReqSecuDepthMarketDataCancel(reqFieldSub, nCount, nRequestID_++);
            else ret = api_->ReqSecuDepthMarketDataSubscribe(reqFieldSub, nCount, nRequestID_++);
        } else {

            char cTransType;
            if (dataType == nsqUtil::TRADE) { cTransType = HS_TRANS_Trade; }
            else { cTransType = HS_TRANS_Entrust; }

            if (cancel) ret = api_->ReqSecuTransactionCancel(cTransType, reqFieldSub, nCount, nRequestID_++);
            else ret = api_->ReqSecuTransactionSubscribe(cTransType, reqFieldSub, nCount, nRequestID_++);
        }
    } catch (exception &e) {

        throw RuntimeException(NSQ_PREFIX + e.what());
    }

    if (ret != 0) {
        if (cancel) {
            throw RuntimeException(NSQ_PREFIX + "unsubscribe failed");
        } else {
            throw RuntimeException(NSQ_PREFIX + "subscribe failed");
        }
    }
}

void NsqConnection::subscribe(Heap *heap, const string &dataType, const string &marketType, const TableSP &table) {

    string type = dataType;
    if (dataVersion_ == "v220105") {
        if (dataType == "orders") { type = nsqUtil::ENTRUST_220105;};
    }

    /// subscribe
    subscribeOrCancel(type, marketType);
    spi_->queues_->initAndStart(heap, type, marketType, table);
}

void
NsqConnection::subscribeTradeEntrust(Heap *heap, const string &dataType, const string &marketType, const DictionarySP &tableDict) {

    /// subscribe
    if (tableDict->size() != 0) {
        subscribeOrCancel(nsqUtil::TRADE, marketType);
        subscribeOrCancel(nsqUtil::ENTRUST, marketType);
    }

    /// start ThreadedQueues
    VectorSP keys = tableDict->keys();
    for (auto i = 0; i < keys->size(); i++) {
        auto key = keys->get(i);
        spi_->queues_->initAndStartTradeEntrust(heap, marketType, key->getInt(), tableDict->get(key));
    }
}

void NsqConnection::unsubscribe(const string &dataType, const string &marketType) {

    /// unsubscribe
    auto types = spi_->queues_->getTypesToCancel(dataType, marketType);
    for (const auto &type : types) {
        subscribeOrCancel(type, marketType, true);
        spi_->queues_->stop(type, marketType);
    }
}

ConstantSP NsqConnection::getStatus() {
    return spi_->queues_->getStatus();
}

Mutex *NsqConnection::getMutex() {
    return &mutex;
}

void NsqConnection::reconnectL() {

    LockGuard<Mutex> l(&mutex);

    if (instancePtr != nullptr) {

        instancePtr->isLoggedIn_ = false;
        instancePtr->isConnected_ = false;
        instancePtr->connectAndLogin(instancePtr->configFilePath_, instancePtr->username_, instancePtr->password_);
    }
}

void NsqConnection::parseOptions(const DictionarySP &options) {
    VectorSP keys = options->keys();
    for (auto i = 0; i < keys->size(); i++) {
        auto key = keys->get(i);
        auto value = options->get(key);
        if (key->getString() == nsqUtil::RECEIVED_TIME) {
            if (value->getType() != DT_BOOL) {
                throw RuntimeException(NSQ_PREFIX + nsqUtil::RECEIVED_TIME + " value should be bool");
            }
            if (value->getBool()) {
                spi_->queues_->setOptionFlag(OPT_RECEIVED);
            }
        } else if (key->getString() == nsqUtil::OUTPUT_ELAPSED) {
            if (value->getType() != DT_BOOL) {
                throw RuntimeException(NSQ_PREFIX + nsqUtil::OUTPUT_ELAPSED + " value should be bool");
            }
            if (value->getBool()) {
                spi_->queues_->setOptionFlag(OPT_ELAPSED);
            }
        } else if (key->getString() == nsqUtil::GET_ALL_FIELD_NAMES) {
            if (value->getType() != DT_BOOL) {
                throw RuntimeException(NSQ_PREFIX + nsqUtil::GET_ALL_FIELD_NAMES + " value should be bool");
            }
            if (value->getBool()) {
                spi_->queues_->addSnapshotExtra(dataVersion_);
            }
        } else {
            throw RuntimeException(NSQ_PREFIX + "options only support " + nsqUtil::RECEIVED_TIME + ", " + nsqUtil::OUTPUT_ELAPSED + ", and " + nsqUtil::GET_ALL_FIELD_NAMES);
        }
    }
}

ConstantSP NsqConnection::getSchema(const string &dataType) {
    string type = dataType;
    if (dataVersion_ == "v220105") {
        if (dataType == "orders") { type = nsqUtil::ENTRUST_220105;};
    }
    return spi_->queues_->getSchema(type);
}
