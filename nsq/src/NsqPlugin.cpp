//
// Created by htxu on 11/20/2023.
//

#include "NsqPlugin.h"
#include "NsqConnection.h"
#include "ini.h"

#include "PluginUtil.h"

using namespace ddb;
using namespace pluginUtil;

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
    string usage = "connect(fileName, [option], [username], [password], [dataVersion='ORIGIN']) ";

    LockGuard<Mutex> l(NsqConnection::getMutex());

    /// parse args
    // configFilePath
    auto configFilePath = getStringScalar(args[0], "fileName", __FUNCTION__, usage);
    if (!Util::exists(configFilePath)) {
        throw IllegalArgumentException(__FUNCTION__, usage + "configFile '" + configFilePath + "' does not exist.");
    }
    if (!isIniValid(configFilePath)) {
        throw IllegalArgumentException(__FUNCTION__, usage + "invalid configFile, could not find 'service_addr' or 'service_port' in [" + configFilePath + "]");
    }
    // options
    DictionarySP options;
    if (args.size() > 1 && !args[1]->isNull()) {
        options = getDictionary(args[1], "option", __FUNCTION__, usage);
    }
    string username, password;
    if (args.size() == 3) {
        throw IllegalArgumentException(__FUNCTION__, usage + "password must be assigned.");
    }
    if (args.size() == 4) {
        username = getStringScalar(args[2], "username", "nsq::connect", usage);
        password = getStringScalar(args[3], "password", "nsq::connect", usage);
    }
    string dataVersion = "ORIGIN";
    if (args.size() > 4 && !args[4]->isNull()) {
        dataVersion = getStringScalar(args[4], "dataVersion", __FUNCTION__, usage);
        if (dataVersion != "ORIGIN" && dataVersion != "v220105") {
            throw IllegalArgumentException(__FUNCTION__, usage + "dataVersion must be 'ORIGIN' or 'v220105'.");
        }
    }
    /// connect and login
    NsqConnection::initInstance(configFilePath, options, username, password, dataVersion);
    return new Void();
}

ConstantSP nsqClose(Heap *heap, vector<ConstantSP> &args) {

    LockGuard<Mutex> l(NsqConnection::getMutex());

    /// close
    NsqConnection::destroyInstance();

    return new Void();
}

ConstantSP nsqGetSchema(Heap *heap, vector<ConstantSP> &args) {
    string usage = "getSchema(dataType): ";

    LockGuard<Mutex> l(NsqConnection::getMutex());

    /// parse args
    auto dataType = getStringScalar(args[0], "dataType", __FUNCTION__, usage);

    /// get schema
    return NsqConnection::getInstance()->getSchema(dataType);
}

ConstantSP nsqSubscribe(Heap *heap, vector<ConstantSP> &args) {
    string usage = "subscribe(dataType, market, outputTable): ";

    LockGuard<Mutex> l(NsqConnection::getMutex());

    /// parse args
    auto dataType = getStringScalar(args[0], "dataType", __FUNCTION__, usage);
    auto marketType = getStringScalar(args[1], "market", __FUNCTION__, usage);

    /// check dataType and marketType
    nsqUtil::checkTypes(dataType, marketType);

    if (dataType == nsqUtil::TRADE_ENTRUST) {
        /// subscribe tradeOrders
        DictionarySP tableDict = getDictWithIntKeyAndSharedRealtimeTableValue(args[2], "outputTable", __FUNCTION__, usage);
        NsqConnection::getInstance()->subscribeTradeEntrust(heap, dataType, marketType, tableDict);
        return new Void();
    } else {
        /// subscribe
        auto table = getSharedRealtimeTable(args[2], "outputTable", __FUNCTION__, usage);
        NsqConnection::getInstance()->subscribe(heap, dataType, marketType, table);
        return new Void();
    }
}

ConstantSP nsqUnsubscribe(Heap *heap, vector<ConstantSP> &args) {
    string usage = "unsubscribe(dataType, market): ";

    LockGuard<Mutex> l(NsqConnection::getMutex());

    /// parse args
    auto dataType = getStringScalar(args[0], "dataType", __FUNCTION__, usage);
    auto marketType = getStringScalar(args[1], "market", __FUNCTION__, usage);

    /// check dataType and marketType
    nsqUtil::checkTypes(dataType, marketType);

    /// unsubscribe
    NsqConnection::getInstance()->unsubscribe(dataType, marketType);
    return new Void();
}

ConstantSP nsqGetStatus(Heap *heap, vector<ConstantSP> &args) {
    string usage = "getStatus(): ";

    LockGuard<Mutex> l(NsqConnection::getMutex());
    return NsqConnection::getInstance()->getStatus();
}

