//
// Created by htxu on 11/20/2023.
//

#include "ddb_nsq.h"
#include "NsqPlugin.h"
#include <memory>
#include "NsqConnection.h"
#include "NsqSpiImpl.h"
#include "NsqUtil.h"
#include "ini.h"

#include "PluginUtil.h"

using namespace pluginUtil;

namespace {

std::shared_ptr<NsqConnection> g_nsq; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

void check_nsq() {
    if (g_nsq == nullptr) {
        throw RuntimeException(std::string(NSQ_PREFIX) + "there is no connection.");
    }
}

std::pair<nsq_data, nsq_market> get_types(const string &dataType, const string &marketType) {
    auto market = nsq_market::unknown;
    if (marketType == "sh" || marketType == "SSE") {
        market = nsq_market::SSE;
    } else if (marketType == "sz" || marketType == "SZSE") {
        market = nsq_market::SZSE;
    } else if (marketType == "bj" || marketType == "BJSE") {
        market = nsq_market::BJSE;
    } else if (marketType == "swi" || marketType == "SWI") {
        market = nsq_market::SWI;
    } else if (marketType == "neeq" || marketType == "TZASE") {
        market = nsq_market::TZASE;
    } else if (marketType == "czce" || marketType == "CZCE") {
        market = nsq_market::CZCE;
    } else if (marketType == "dce" || marketType == "DCE") {
        market = nsq_market::DCE;
    } else if (marketType == "shfe" || marketType == "SHFE") {
        market = nsq_market::SHFE;
    } else if (marketType == "cffex" || marketType == "CFFEX") {
        market = nsq_market::CFFEX;
    } else if (marketType == "ine" || marketType == "INE") {
        market = nsq_market::INE;
    } else if (marketType == "gfex" || marketType == "GFEX") {
        market = nsq_market::GFEX;
    } else if (marketType == "SSEHK") {
        market = nsq_market::SSEHK;
    } else if (marketType == "SZSEHK") {
        market = nsq_market::SZSEHK;
    } else {
        throw RuntimeException(std::string(NSQ_PREFIX) + R"(Unknown market id".)");
    }
    if (dataType == "snapshot") {
        return {nsq_data::DepthMarket, market};
    }
    if (dataType == "trade") {
        return {nsq_data::TransactionTrade, market};
    }
    if (dataType == "orders") {
        return {nsq_data::TransactionEntrust, market};
    }
    if (dataType == "orderTrade") {
        return {nsq_data::orderbook, market};
    }
    throw RuntimeException(std::string(NSQ_PREFIX) + R"(dataType should be "snapshot", "trade", "orders", or "orderTrade".)");
}

}  // namespace

bool isIniValid(const string &configPath) {
    mINI::INIFile file(configPath);
    mINI::INIStructure ini;
    file.read(ini);
    if (ini["sailfish"]["service_addr"].empty() || ini["sailfish"]["service_port"].empty()) {
        if (ini["sailfish"]["service_addr1"].empty() || ini["sailfish"]["service_port1"].empty()) {
            return false;
        }
    }
    return true;
}

ConstantSP nsqConnect(Heap *heap, vector<ConstantSP> &args) {
    std::ignore = heap;
    string usage = "connect(fileName, [option], [username], [password], [dataVersion='ORIGIN']) ";

    LockGuard<Mutex> l(NsqConnection::getMutex());

    /// parse args
    // configFilePath
    auto configFilePath = getStringScalar(args[0], "fileName", __FUNCTION__, usage);
    if (!Util::exists(configFilePath)) {
        throw IllegalArgumentException(__FUNCTION__, usage + "configFile '" + configFilePath + "' does not exist.");
    }
    // if (!isIniValid(configFilePath)) {
    //     throw IllegalArgumentException(__FUNCTION__, usage + "invalid configFile, could not find 'service_addr' or 'service_port' in [" + configFilePath + "]");
    // }
    // options
    DictionarySP options;
    if (args.size() > 1 && !args[1]->isNull()) {
        options = getDictionary(args[1], "option", __FUNCTION__, usage);
    }
    string username, password;
    if (args.size() == 3) {
        throw IllegalArgumentException(__FUNCTION__, usage + "password must be assigned.");
    }
    if(args.size() > 2 && !args[2]->isNothing()){
        username = getStringScalar(args[2], "username", "nsq::connect", usage);
        if (username.size() >= sizeof(HSAccountID)) {
            throw IllegalArgumentException(__FUNCTION__, usage + "username length must be less than " + std::to_string(sizeof(HSAccountID)));
        }
    }
    if (args.size() > 3 && !args[3]->isNothing()) {
        password = getStringScalar(args[3], "password", "nsq::connect", usage);
        if (password.size() >= sizeof(HSPassword)) {
            throw IllegalArgumentException(__FUNCTION__, usage + "password length must be less than " + std::to_string(sizeof(HSPassword)));
        }
    }
    string dataVersion = "ORIGIN";
    if (args.size() > 4 && !args[4]->isNull()) {
        dataVersion = getStringScalar(args[4], "dataVersion", __FUNCTION__, usage);
        if (dataVersion != "ORIGIN" && dataVersion != "v220105") {
            throw IllegalArgumentException(__FUNCTION__, usage + "dataVersion must be 'ORIGIN' or 'v220105'.");
        }
    }
    /// connect and login
    nsq_version version = (dataVersion == "v220105") ? nsq_version::v220105 : nsq_version::origin;

    if (g_nsq != nullptr) {
        throw RuntimeException(std::string(NSQ_PREFIX) + "there is already a connection.");
    }
    auto nsq = std::make_shared<NsqConnection>();
    auto spi = std::make_shared<CHSNsqSpiImpl>(nsq);
    spi->username_ = username;
    spi->password_ = password;
    nsq->init(configFilePath, options, username, password, version, spi.get());
    g_spi = std::move(spi);
    g_nsq = std::move(nsq);
    return new Void();
}

ConstantSP nsqClose(Heap *heap, vector<ConstantSP> &args)
{
    std::ignore = heap;
    std::ignore = args;
    LockGuard<Mutex> l(NsqConnection::getMutex());
    check_nsq();

    /// close
    g_nsq->destroyInstance();
    g_spi.reset();
    g_nsq.reset();

    return new Void();
}

ConstantSP nsqGetSchema(Heap *heap, vector<ConstantSP> &args) {
    std::ignore = heap;
    string usage = "getSchema(dataType): ";

    LockGuard<Mutex> l(NsqConnection::getMutex());
    check_nsq();

    /// parse args
    auto dataType = getStringScalar(args[0], "dataType", __FUNCTION__, usage);

    // market type does not matter for schema, use "sh" as default
    return g_nsq->getSchema(get_types(dataType, "sh").first);
}

ConstantSP nsqSubscribe(Heap *heap, vector<ConstantSP> &args) {
    string usage = "subscribe(dataType, market, outputTable, [queueDepth=1000000], [codes]): ";

    LockGuard<Mutex> l(NsqConnection::getMutex());
    check_nsq();

    /// parse args
    auto data_str = getStringScalar(args[0], "dataType", __FUNCTION__, usage);
    auto market_str = getStringScalar(args[1], "market", __FUNCTION__, usage);

    auto types = get_types(data_str, market_str);
    auto data = types.first;
    auto market = types.second;

    long long queueDepth = QUEUE_DEPTH;
    if (args.size() > 3 && !args[3]->isNull()) {
        queueDepth = getLongScalar(args[3], "queueDepth", __FUNCTION__, usage);
        if (queueDepth <= 0) {
            throw IllegalArgumentException(__FUNCTION__, usage + "queueDepth must be positive.");
        }
    }
    vector<string> codes;
    if (args.size() > 4 && !args[4]->isNull()) {
        VectorSP codesVec = getStringVector(args[4], "codes", __FUNCTION__, usage);
        int end = codesVec->size();
        for (int i = 0; i < end; ++i) {
            codes.push_back(codesVec->getString(i));
            if (codes.back().size() >= sizeof(HSInstrumentID)) {
                throw IllegalArgumentException(__FUNCTION__, usage + "each code's length must be less than " + std::to_string(sizeof(HSInstrumentID)));
            }
        }
    }
    if (data == nsq_data::orderbook) {
        DictionarySP tableDict = getDictWithIntKeyAndSharedRealtimeTableValue(args[2], "outputTable", __FUNCTION__, usage);
        g_nsq->subscribeTradeEntrust(heap, market, tableDict, queueDepth, codes);
        return new Void();
    }
    /// subscribe
    auto table = getSharedRealtimeTable(args[2], "outputTable", __FUNCTION__, usage);
    g_nsq->subscribe(heap, data, market, table, queueDepth, codes);
    return new Void();
}

ConstantSP nsqUnsubscribe(Heap *heap, vector<ConstantSP> &args) {
    std::ignore = heap;
    string usage = "unsubscribe(dataType, market): ";

    LockGuard<Mutex> l(NsqConnection::getMutex());
    check_nsq();

    /// parse args
    auto dataType = getStringScalar(args[0], "dataType", __FUNCTION__, usage);
    auto marketType = getStringScalar(args[1], "market", __FUNCTION__, usage);
    /// unsubscribe
    g_nsq->unsubscribe(get_types(dataType, marketType));
    return new Void();
}

ConstantSP nsqGetStatus(Heap *heap, vector<ConstantSP> &args) {
    std::ignore = heap;
    std::ignore = args;
    string usage = "getStatus(): ";

    LockGuard<Mutex> l(NsqConnection::getMutex());
    check_nsq();
    return g_nsq->getStatus();
}
