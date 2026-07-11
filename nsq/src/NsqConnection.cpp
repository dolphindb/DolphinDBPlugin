#include "NsqConnection.h"

#include "NsqSpiImpl.h"
#include "ddb_nsq.h"
#include "NsqUtil.h"

#include <vector>

namespace {

auto market_id(nsq_market market) {
    switch (market) {
        case nsq_market::SSE: return HS_EI_SSE;
        case nsq_market::SZSE: return HS_EI_SZSE;
        case nsq_market::BJSE: return HS_EI_BJSE;
        case nsq_market::SWI: return HS_EI_SWI;
        case nsq_market::TZASE: return HS_EI_TZASE;
        case nsq_market::CZCE: return HS_EI_CZCE;
        case nsq_market::DCE: return HS_EI_DCE;
        case nsq_market::SHFE: return HS_EI_SHFE;
        case nsq_market::CFFEX: return HS_EI_CFFEX;
        case nsq_market::INE: return HS_EI_INE;
        case nsq_market::GFEX: return HS_EI_GFEX;
        case nsq_market::SSEHK: return HS_EI_SSEHK;
        case nsq_market::SZSEHK: return HS_EI_SZSEHK;
        default: throw RuntimeException(std::string(NSQ_PREFIX) + "invalid market type.");
    }
    return "";
}

} // namespace

Mutex NsqConnection::mutex;
const int NsqConnection::TIMEOUT_MS = 3000;

void NsqConnection::connect(const string &configFilePath, CHSNsqSpiImpl *spi) {
    loginErrMsg_ = "";
    /// already connected and logged in
    if (isConnected_ and isLoggedIn_) {
        throw RuntimeException(std::string(NSQ_PREFIX) + "there is already a connection.");
    }

    /// create and register
    api_ = NewNsqApiExt("", configFilePath.c_str());
    api_->RegisterSpi(spi);

    /// connect (using Init)
    LockGuard<Mutex> connectionL(&connectionM_);
    if (api_->Init("") != 0) {
        throw RuntimeException(std::string(NSQ_PREFIX) + "api init failed.");
    }
    // use cond var to wait
    if (!connectionCV_.wait(connectionM_, TIMEOUT_MS)) {
        throw RuntimeException(std::string(NSQ_PREFIX) + "connection timeout.");
    }
    // use cond var to wait
    if (!loginErrMsg_.empty()) {
        throw RuntimeException(std::string(NSQ_PREFIX) + "establish connection failed. " +  loginErrMsg_);
    }
    LockGuard<Mutex> loginL(&loginM_);
    if (!loginCV_.wait(loginM_, TIMEOUT_MS*20)) {
        loginErrMsg_ = "login timeout.";
        throw RuntimeException(std::string(NSQ_PREFIX) + "establish connection failed. " +  loginErrMsg_);
    }
    spi->isConnected_ = true;
}
void NsqConnection::login(const string &username, const string &password) {
    loginErrMsg_ = "";
    /// login
    // LockGuard<Mutex> loginL(&loginM_);
    CHSNsqReqUserLoginField reqLoginField;
    std::memset(&reqLoginField, 0, sizeof(reqLoginField));
    strncpy(reqLoginField.AccountID, username.c_str(), username.size());
    strncpy(reqLoginField.Password, password.c_str(), password.size());
    if (api_->ReqUserLogin(&reqLoginField, nRequestID_++) != 0) {
        loginErrMsg_ = "login failed.";
        return;
    }
}

void NsqConnection::connectionNotifyL() {

    LockGuard<Mutex> l(&connectionM_);

    isConnected_ = true;
    connectionCV_.notify();
}

void NsqConnection::loginNotifyL()
{
    LockGuard<Mutex> l(&loginM_);

    isLoggedIn_ = true;
    loginCV_.notify();
}

void NsqConnection::init(const string &configFilePath, const DictionarySP& options, const string &username, const string &password,
                         nsq_version version, CHSNsqSpiImpl *spi)
{
    configFilePath_ = configFilePath;
    username_ = username;
    password_ = password;
    dataVersion_ = version;

    try {
        if (!options.isNull()) {
            parseOptions(options, spi);
        }
        connect(configFilePath, spi);
    } catch (std::exception &e) {
        destroyInstance(spi);
        throw RuntimeException(e.what());
    } catch (...) {
        destroyInstance(spi);
        throw RuntimeException(std::string(NSQ_PREFIX) + "init failed.");
    }
}

void NsqConnection::destroyInstance(CHSNsqSpiImpl *spi)
{
    if (spi == nullptr) {
        spi = g_spi.get();
    }
    if (spi != nullptr) {
        spi->isConnected_ = false;
    }
    if(api_) { api_->ReleaseApi(); }
}

void NsqConnection::subscribeOrCancel(nsq_data data, nsq_market market, bool cancel, const vector<string> &codes)
{
    if (data == nsq_data::orderbook) {
        return;
    }
    /// set request field for subscription
    std::vector<CHSNsqReqSecuDepthMarketDataField> reqFieldSub(1);
    std::memset(reqFieldSub.data(), 0, sizeof(CHSNsqReqSecuDepthMarketDataField) * reqFieldSub.size(    ));

    int nCount = 0;
    strcpy(&reqFieldSub[0].ExchangeID[0], market_id(market));

    /// request subscription
    int ret = 0;

    try {
        if (data == nsq_data::DepthMarket) {
            if (cancel) {
                ret = api_->ReqSecuDepthMarketDataCancel(reqFieldSub.data(), nCount, nRequestID_++);
            } else {
                if (!codes.empty()) {
                    reqFieldSub.resize(codes.size());
                    std::memset(reqFieldSub.data(), 0, sizeof(CHSNsqReqSecuDepthMarketDataField) * reqFieldSub.size());
                    nCount = static_cast<int>(codes.size());
                    for (int i = 0; i < nCount; i++) {
                        strcpy(&reqFieldSub[i].ExchangeID[0], market_id(market));
                        strncpy(reqFieldSub[i].InstrumentID, codes[i].c_str(), codes[i].size());
                    }
                }
                ret = api_->ReqSecuDepthMarketDataSubscribe(reqFieldSub.data(), nCount, nRequestID_++);
            } 
        } else {

            char cTransType = '\0';
            if (data == nsq_data::TransactionTrade) { cTransType = HS_TRANS_Trade; }
            else if (data == nsq_data::TransactionEntrust) { cTransType = HS_TRANS_Entrust; }

            if (cancel) {
                ret = api_->ReqSecuTransactionCancel(cTransType, reqFieldSub.data(), nCount, nRequestID_++);
            } else {
                if (!codes.empty()) {
                    reqFieldSub.resize(codes.size());
                    std::memset(reqFieldSub.data(), 0, sizeof(CHSNsqReqSecuDepthMarketDataField) * reqFieldSub.size());
                    nCount = static_cast<int>(codes.size());
                    for (int i = 0; i < nCount; i++) {
                        strcpy(&reqFieldSub[i].ExchangeID[0], market_id(market));
                        strncpy(reqFieldSub[i].InstrumentID, codes[i].c_str(), codes[i].size());
                    }
                }
                ret = api_->ReqSecuTransactionSubscribe(cTransType, reqFieldSub.data(), nCount, nRequestID_++);
            }
        }
    } catch (exception &e) {
        throw RuntimeException(std::string(NSQ_PREFIX) + e.what());
    }

    if (ret != 0) {
        const char *apiErrMsg = api_ == nullptr ? nullptr : api_->GetApiErrorMsg(ret);
        string detail = apiErrMsg == nullptr ? "unknown error" : apiErrMsg;
        if (cancel) {
            throw RuntimeException(std::string(NSQ_PREFIX) + "unsubscribe " + to_string(market) + " " + to_string(data) +
                                   " failed, ret[" + std::to_string(ret) + "], error[" + detail + "]");
        }
        throw RuntimeException(std::string(NSQ_PREFIX) + "subscribe " + to_string(market) + " " + to_string(data) +
                               " failed, ret[" + std::to_string(ret) + "], error[" + detail + "]");
    }
}

void NsqConnection::subscribe(Heap *heap, nsq_data data, nsq_market market, const TableSP &table, long long queueDepth, const vector<string> &codes)
{
    subscribeOrCancel(data, market, false, codes);
    g_spi->queues_->initAndStart(heap, data, market, dataVersion_, table, queueDepth);
}

void
NsqConnection::subscribeTradeEntrust(Heap *heap, nsq_market market, const DictionarySP &tableDict, long long queueDepth, const vector<string> &codes) {

    /// subscribe
    if (tableDict->size() != 0) {
        subscribeOrCancel(nsq_data::TransactionTrade, market, false, codes);
        subscribeOrCancel(nsq_data::TransactionEntrust, market, false, codes);
    }

    /// start ThreadedQueues
    VectorSP keys = tableDict->keys();
    for (auto i = 0; i < keys->size(); i++) {
        auto key = keys->get(i);
        g_spi->queues_->initAndStartTradeEntrust(heap, market, key->getInt(), tableDict->get(key), queueDepth);
    }
}

void NsqConnection::unsubscribe(nsq_data data, nsq_market market)
{
    auto [typesForCancel, typesForStop] = g_spi->queues_->getTypesToCancel(data, market);
    for (const auto &type : typesForCancel) {
        subscribeOrCancel(type, market, true);
    }
    for (const auto &type: typesForStop) {
        g_spi->queues_->stop(type, market);
    }
}

ConstantSP NsqConnection::getStatus() {
    return g_spi->queues_->getStatus();
}

Mutex *NsqConnection::getMutex() {
    return &mutex;
}

void NsqConnection::parseOptions(const DictionarySP &options, CHSNsqSpiImpl *spi) {
    VectorSP keys = options->keys();
    for (auto i = 0; i < keys->size(); i++) {
        auto key = keys->get(i);
        auto value = options->get(key);
        if (key->getString() == nsqUtil::RECEIVED_TIME) {
            if (value->getType() != DT_BOOL) {
                throw RuntimeException(std::string(NSQ_PREFIX) + nsqUtil::RECEIVED_TIME + " value should be bool");
            }
            if (value->getBool()) {
                spi->queues_->setOptionFlag(OPT_RECEIVED);
            }
        } else if (key->getString() == nsqUtil::OUTPUT_ELAPSED) {
            if (value->getType() != DT_BOOL) {
                throw RuntimeException(std::string(NSQ_PREFIX) + nsqUtil::OUTPUT_ELAPSED + " value should be bool");
            }
            if (value->getBool()) {
                spi->queues_->setOptionFlag(OPT_ELAPSED);
            }
        } else if (key->getString() == nsqUtil::GET_ALL_FIELD_NAMES) {
            if (value->getType() != DT_BOOL) {
                throw RuntimeException(std::string(NSQ_PREFIX) + nsqUtil::GET_ALL_FIELD_NAMES + " value should be bool");
            }
            if (value->getBool()) {
                spi->queues_->addSnapshotExtra(dataVersion_);
            }
        } else {
            throw RuntimeException(std::string(NSQ_PREFIX) + "options only support " + nsqUtil::RECEIVED_TIME + ", " + nsqUtil::OUTPUT_ELAPSED + ", and " + nsqUtil::GET_ALL_FIELD_NAMES);
        }
    }
}

ConstantSP NsqConnection::getSchema(nsq_data data) {
    return g_spi->queues_->getSchema(data, dataVersion_);
}
