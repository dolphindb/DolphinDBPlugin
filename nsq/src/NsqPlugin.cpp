//
// Created by htxu on 11/20/2023.
//

#include "NsqPlugin.h"
#include "NsqConnection.h"

#include "PluginUtil.h"

using namespace pluginUtil;

ConstantSP nsqConnect(Heap *heap, vector<ConstantSP> &args) {
    string usage = "connect(configFilePath, [options], [username], [password]) ";

    LockGuard<Mutex> l(NsqConnection::getMutex());

    /// parse args
    // configFilePath
    auto configFilePath = getStringScalar(args[0], "configFilePath", __FUNCTION__, usage);
    if (!Util::exists(configFilePath)) {
        throw IllegalArgumentException(__FUNCTION__, usage + "configFilePath does not exist.");
    }
    // options
    DictionarySP options;
    if (args.size() > 1 && !args[1]->isNull()) {
        options = getDictionary(args[1], "options", __FUNCTION__, usage);
    }
    string username, password;
    if (args.size() == 3) {
        throw IllegalArgumentException(__FUNCTION__, usage + "password must be assigned.");
    }
    if (args.size() == 4) {
        username = getStringScalar(args[2], "username", "nsq::connect", usage);
        password = getStringScalar(args[3], "password", "nsq::connect", usage);
    }
    /// connect and login
    NsqConnection::initInstance(configFilePath, options, username, password);
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
    string usage = "subscribe(type, location, streamTable): ";

    LockGuard<Mutex> l(NsqConnection::getMutex());

    /// parse args
    auto dataType = getStringScalar(args[0], "type", __FUNCTION__, usage);
    auto marketType = getStringScalar(args[1], "location", __FUNCTION__, usage);

    if (dataType == nsqUtil::TRADE_ENTRUST) {
        /// subscribe tradeOrders
        DictionarySP tableDict = getDictWithIntKeyAndSharedRealtimeTableValue(args[2], "streamTable", __FUNCTION__, usage);
        NsqConnection::getInstance()->subscribeTradeEntrust(heap, dataType, marketType, tableDict);
        return new Void();
    } else {
        /// subscribe
        auto table = getSharedRealtimeTable(args[2], "streamTable", __FUNCTION__, usage);
        NsqConnection::getInstance()->subscribe(heap, dataType, marketType, table);
        return new Void();
    }
}

ConstantSP nsqUnsubscribe(Heap *heap, vector<ConstantSP> &args) {
    string usage = "unsubscribe(type, location): ";

    LockGuard<Mutex> l(NsqConnection::getMutex());

    /// parse args
    auto dataType = getStringScalar(args[0], "type", __FUNCTION__, usage);
    auto marketType = getStringScalar(args[1], "location", __FUNCTION__, usage);

    /// unsubscribe
    NsqConnection::getInstance()->unsubscribe(dataType, marketType);
    return new Void();
}

ConstantSP nsqGetStatus(Heap *heap, vector<Constant> &args) {
    string usage = "getStatus(): ";

    LockGuard<Mutex> l(NsqConnection::getMutex());
    return NsqConnection::getInstance()->getStatus();
}

