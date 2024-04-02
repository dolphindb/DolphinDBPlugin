#include "amdQuoteImp.h"

#include "Concurrent.h"
#include "Exceptions.h"
#include "amdQuote.h"
#include "amdQuoteType.h"
#include "amdSpiImp.h"

bool STOP_TEST = true;
AmdQuote::AmdQuote(const string &username, const string &password, const vector<string>& ips, const vector<int>& ports,
                   SessionSP session, bool receivedTime, bool dailyIndex, bool outputElapsed, int dailyStartTime, bool securityCodeToInt)
    : receivedTime_(receivedTime),
      dailyIndex_(dailyIndex),
      outputElapsed_(outputElapsed),
      dailyStartTime_(dailyStartTime),
      securityCodeToInt_(securityCodeToInt),
      username_(username),
      password_(password),
      ips_(ips),
      ports_(ports) {
    if(securityCodeToInt) {
        initSecurityCodeToIntTypeContainer(amdTypeContainer_);
    } else {
        initTypeContainer(amdTypeContainer_);
    }

    // The configuration is detailed in the AMD header file
    cfg_.channel_mode = amd::ama::ChannelMode::kTCP;
    cfg_.tcp_compress_mode = 0;
    cfg_.ha_mode = amd::ama::HighAvailableMode::kMasterSlaveA;
    cfg_.min_log_level = amd::ama::LogLevel::kInfo;
    cfg_.is_output_mon_data = false;
    cfg_.keep_order = false;
    cfg_.keep_order_timeout_ms = 3000;
    cfg_.is_subscribe_full = false;
    if (username.size() >= amd::ama::ConstField::kUsernameLen) {
        throw RuntimeException(AMDQUOTE_PREFIX + "username length should be less than 32");
    }
    strcpy(cfg_.username, username.c_str());
    if (password.size() >= amd::ama::ConstField::kPasswordLen) {
        throw RuntimeException(AMDQUOTE_PREFIX + "password length should be less than 64");
    }
    strcpy(cfg_.password, password.c_str());
    cfg_.ums_server_cnt = ips.size();
    for (unsigned int i = 0; i < cfg_.ums_server_cnt; i++) {
        strcpy(cfg_.ums_servers[i].local_ip, "0.0.0.0");
        if (ips[i].size() >= amd::ama::ConstField::kIPMaxLen) {
            throw RuntimeException(AMDQUOTE_PREFIX + "ip address length should be less than 24");
        }
        strcpy(cfg_.ums_servers[i].server_ip, ips[i].c_str());
        cfg_.ums_servers[i].server_port = ports[i];
    }

    cfg_.is_thread_safe = false;

    amdSpi_ = new AMDSpiImp(session);  // special treatment
    try {
        if (amd::ama::IAMDApi::Init(amdSpi_, cfg_) != amd::ama::ErrorCode::kSuccess) {
            amd::ama::IAMDApi::Release();
            throw RuntimeException("Init AMA failed");
        }
    } catch (std::exception &exception) {
        throw RuntimeException(AMDQUOTE_PREFIX + exception.what());
    }

    STOP_TEST = false;
}

AmdQuote::~AmdQuote() {
    try {
        STOP_TEST = true;
        vector<string> codeList;
        unsubscribe("all", 0, codeList);
        amd::ama::IAMDApi::Release();
        delete amdSpi_;
    } catch (std::exception &ex) {
        LOG_ERR(AMDQUOTE_PREFIX + "destruction failed. " + string(ex.what()));
    } catch (...) {
        LOG_ERR(AMDQUOTE_PREFIX + "destruction failed with unknown exception.");
    }
}

TableSP AmdQuote::getStatus() {
    return amdSpi_->getStatus();
}

uint64_t getSubscribeDataType(AMDDataType dataType) {
    switch (dataType) {
        case AMD_SNAPSHOT:
            return amd::ama::SubscribeSecuDataType::kSnapshot;
        case AMD_EXECUTION:
            return amd::ama::SubscribeSecuDataType::kTickExecution;
        case AMD_ORDER:
            return amd::ama::SubscribeSecuDataType::kTickOrder;
        case AMD_FUND_SNAPSHOT:
            return amd::ama::SubscribeSecuDataType::kSnapshot;
        case AMD_FUND_EXECUTION:
            return amd::ama::SubscribeSecuDataType::kTickExecution;
        case AMD_FUND_ORDER:
            return amd::ama::SubscribeSecuDataType::kTickOrder;
        case AMD_BOND_SNAPSHOT:
            return amd::ama::SubscribeSecuDataType::kSnapshot;
        case AMD_BOND_EXECUTION:
            return amd::ama::SubscribeSecuDataType::kTickExecution;
        case AMD_BOND_ORDER:
            return amd::ama::SubscribeSecuDataType::kTickOrder;
        case AMD_INDEX:
            return amd::ama::SubscribeSecuDataType::kSnapshot;
        case AMD_ORDER_QUEUE:
            return amd::ama::SubscribeSecuDataType::kOrderQueue;
        case AMD_OPTION_SNAPSHOT:
            return amd::ama::SubscribeSecuDataType::kSnapshot;
        case AMD_FUTURE_SNAPSHOT:
            return amd::ama::SubscribeSecuDataType::kSnapshot;
#ifndef AMD_3_9_6
        case AMD_IOPV_SNAPSHOT:
            return amd::ama::SubscribeSecuDataType::kSnapshot;
#endif
        default:
            throw RuntimeException(AMDQUOTE_PREFIX + "Invalid dataType " + std::to_string(dataType) + ".");
    }
}

uint64_t getSubscribeCategoryType(AMDDataType dataType) {
    switch (dataType) {
        case AMD_SNAPSHOT:
            return amd::ama::SubscribeCategoryType::kStock;
        case AMD_EXECUTION:
            return amd::ama::SubscribeCategoryType::kStock;
        case AMD_ORDER:
            return amd::ama::SubscribeCategoryType::kStock;
        case AMD_FUND_SNAPSHOT:
            return amd::ama::SubscribeCategoryType::kFund;
        case AMD_FUND_EXECUTION:
            return amd::ama::SubscribeCategoryType::kFund;
        case AMD_FUND_ORDER:
            return amd::ama::SubscribeCategoryType::kFund;
        case AMD_BOND_SNAPSHOT:
            return amd::ama::SubscribeCategoryType::kBond;
        case AMD_BOND_EXECUTION:
            return amd::ama::SubscribeCategoryType::kBond;
        case AMD_BOND_ORDER:
            return amd::ama::SubscribeCategoryType::kBond;
        case AMD_ORDER_EXECUTION:
            return amd::ama::SubscribeCategoryType::kStock | amd::ama::SubscribeCategoryType::kFund;
        case AMD_BOND_ORDER_EXECUTION:
            return amd::ama::SubscribeCategoryType::kBond;
        case AMD_INDEX:
            return amd::ama::SubscribeCategoryType::kIndex;
        case AMD_ORDER_QUEUE:
            return amd::ama::SubscribeCategoryType::kNone;
        case AMD_OPTION_SNAPSHOT:
            return amd::ama::SubscribeCategoryType::kOption;
        case AMD_FUTURE_SNAPSHOT:
            return amd::ama::SubscribeCategoryType::kNone;
#ifndef AMD_3_9_6
        case AMD_IOPV_SNAPSHOT:
            return amd::ama::SubscribeDerivedDataType::kIOPVSnapshot;
#endif
        default:
            throw RuntimeException(AMDQUOTE_PREFIX + "Invalid dataType " + std::to_string(dataType) + ".");
    }
}

void doSubscribe(const int market, const vector<string> &codeList, const uint64_t dataType, const uint64_t categoryType,
                 const string &typeName) {
    unsigned int codeSize = 1;
    if (codeList.size() > 0) {
        codeSize = codeList.size();
    }
    amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
    PluginDefer df([=]() { delete[] sub; });
    memset(sub, 0, sizeof(amd::ama::SubscribeCategoryItem) * codeSize);

    if (codeList.size() == 0) {
        sub[0].data_type = dataType;
        sub[0].category_type = categoryType;
        sub[0].market = market;
        sub[0].security_code[0] = '\0';
    } else {
        for (unsigned int codeIndex = 0; codeIndex < codeSize; codeIndex++) {
            sub[codeIndex].data_type = dataType;
            sub[codeIndex].category_type = categoryType;
            sub[codeIndex].market = market;
            memcpy(sub[codeIndex].security_code, codeList[codeIndex].c_str(), codeList[codeIndex].length());
        }
    }
    try {
        if (amd::ama::IAMDApi::SubscribeData(amd::ama::SubscribeType::kAdd, sub, codeSize) !=
            amd::ama::ErrorCode::kSuccess) {
            throw RuntimeException("subscribe [" + typeName + "] err, market:" + std::to_string(market));
            return;
        }
    } catch (std::exception &exception) {
        throw RuntimeException(AMDQUOTE_PREFIX + exception.what());
    }
}

void AmdQuote::addSubscribe(Heap *heap, const string &typeName, AMDDataType amdType, int key, TableSP table,
                            FunctionDefSP transform, long long dailyStartTime) {
    int flag = 0;
    if (receivedTime_) flag |= OPT_RECEIVED;
    if (outputElapsed_) flag |= OPT_ELAPSED;
    if (dailyIndex_) flag |= OPT_DAILY_INDEX;
    if (amdType == AMD_ORDER_EXECUTION || amdType == AMD_BOND_ORDER_EXECUTION) {
        flag &= OPT_ELAPSED;
    }
    // FUTURE add schema checking logic
    TableSP schema = nullptr;
    amdSpi_->addThreadedQueue(heap, typeName, amdType, key, table, transform, getMetaByAMDType(amdType), schema, flag,
                              dailyStartTime, securityCodeToInt_);
}

void AmdQuote::subscribe(Heap *heap, const string &typeName, int market, vector<string> codeList, ConstantSP table,
                         FunctionDefSP transform, long long timestamp) {
    vector<string> nullCodeList;
    // FUTURE unsubscribe maybe not required
    unsubscribe(typeName, market, nullCodeList);
    AMDDataType amdType = getAmdDataType(typeName);
    uint64_t categoryType = getSubscribeCategoryType(amdType);

    long long dailyStartTime = LONG_LONG_MIN;
    int current = timestamp % (3600 * 24 * 1000);
    if (current < dailyStartTime_) {
        dailyStartTime = timestamp;
    }
    if (!(amdType == AMD_ORDER || amdType == AMD_FUND_ORDER || amdType == AMD_BOND_ORDER || amdType == AMD_EXECUTION ||
          amdType == AMD_FUND_EXECUTION || amdType == AMD_BOND_EXECUTION)) {
        dailyStartTime = LONG_LONG_MIN;
    }

    if (amdType == AMD_ORDER_EXECUTION) {
        doSubscribe(market, codeList, amd::ama::SubscribeSecuDataType::kTickOrder, amd::ama::SubscribeCategoryType::kStock , typeName);
        doSubscribe(market, codeList, amd::ama::SubscribeSecuDataType::kTickOrder, amd::ama::SubscribeCategoryType::kFund, typeName);
        doSubscribe(market, codeList, amd::ama::SubscribeSecuDataType::kTickExecution, amd::ama::SubscribeCategoryType::kStock , typeName);
        doSubscribe(market, codeList, amd::ama::SubscribeSecuDataType::kTickExecution, amd::ama::SubscribeCategoryType::kFund, typeName);
    } else if (amdType == AMD_BOND_ORDER_EXECUTION) {
        doSubscribe(market, codeList, amd::ama::SubscribeSecuDataType::kTickOrder, categoryType, typeName);
        doSubscribe(market, codeList, amd::ama::SubscribeSecuDataType::kTickExecution, categoryType, typeName);
    } else {
        uint64_t dataType = getSubscribeDataType(amdType);
        doSubscribe(market, codeList, dataType, categoryType, typeName);
    }
    if (table->getForm() == DF_DICTIONARY) {
        DictionarySP dict = table;
        VectorSP keys = dict->keys();
        VectorSP values = dict->values();
        dailyStartTime = 0;
        for (int index = 0; index < keys->size(); ++index) {
            int channelNo = keys->get(index)->getInt();
            TableSP table = values->get(index);
            // the key for threadedQueue is market + channelNo * ORDER_EXECUTION_OFFSET(1000 currently)
            // the callback registered in amdSpi also follow this logic to distribute data
            int marketKey = market + channelNo * ORDER_EXECUTION_OFFSET;
            addSubscribe(heap, typeName, amdType, marketKey, table, transform, dailyStartTime);
        }
    } else {
        addSubscribe(heap, typeName, amdType, market, table, transform, dailyStartTime);
    }
}

void doUnSubscribe(const int market, const vector<string> &codeList, const uint64_t dataType,
                   const uint64_t categoryType, const string &typeName) {
    unsigned int codeSize = 1;
    if (codeList.size() > 0) {
        codeSize = codeList.size();
    }
    amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
    PluginDefer df([=]() { delete[] sub; });
    memset(sub, 0, sizeof(amd::ama::SubscribeCategoryItem) * codeSize);

    if (codeList.size() == 0) {
        sub[0].data_type = dataType;
        sub[0].category_type = categoryType;
        sub[0].market = market;
        sub[0].security_code[0] = '\0';
    } else {
        for (unsigned int codeIndex = 0; codeIndex < codeSize; codeIndex++) {
            sub[codeIndex].data_type = dataType;
            sub[codeIndex].category_type = categoryType;
            sub[codeIndex].market = market;
            memcpy(sub[codeIndex].security_code, codeList[codeIndex].c_str(), codeList[codeIndex].length());
        }
    }

    try {
        if (amd::ama::IAMDApi::SubscribeData(amd::ama::SubscribeType::kDel, sub, codeSize) !=
            amd::ama::ErrorCode::kSuccess) {
            throw RuntimeException("unsubscribe [" + typeName + "] err, market:" + std::to_string(market));
        }
    } catch (std::exception &exception) {
        throw RuntimeException(AMDQUOTE_PREFIX + exception.what());
    }
}

void AmdQuote::removeSubscribe(AMDDataType type, int key) { amdSpi_->removeThreadedQueue(type, key); }

void AmdQuote::unsubscribe(const string &dataType, int market, vector<string> codeList) {
    if (dataType == "all") {
        amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[1];
        PluginDefer df([=]() { delete[] sub; });
        memset(sub, 0, sizeof(amd::ama::SubscribeCategoryItem));
        try {
            if (amd::ama::IAMDApi::SubscribeData(amd::ama::SubscribeType::kCancelAll, sub, 1) !=
                amd::ama::ErrorCode::kSuccess) {
                throw RuntimeException("unsubscribe all market failed");
            }
        } catch (std::exception &exception) {
            throw RuntimeException(AMDQUOTE_PREFIX + exception.what());
        }
        amdSpi_->removeAll();
        return;
    }
    AMDDataType amdType = getAmdDataType(dataType);
    uint64_t categoryType = getSubscribeCategoryType(amdType);
    // stop async threads
    if (amdType == AMD_ORDER_EXECUTION) {
        doUnSubscribe(market, codeList, amd::ama::SubscribeSecuDataType::kTickOrder, amd::ama::SubscribeCategoryType::kStock , dataType);
        doUnSubscribe(market, codeList, amd::ama::SubscribeSecuDataType::kTickOrder, amd::ama::SubscribeCategoryType::kFund, dataType);
        doUnSubscribe(market, codeList, amd::ama::SubscribeSecuDataType::kTickExecution, amd::ama::SubscribeCategoryType::kStock , dataType);
        doUnSubscribe(market, codeList, amd::ama::SubscribeSecuDataType::kTickExecution, amd::ama::SubscribeCategoryType::kFund, dataType);
    } else if (amdType == AMD_BOND_ORDER_EXECUTION) {
        doUnSubscribe(market, codeList, amd::ama::SubscribeSecuDataType::kTickOrder, categoryType, dataType);
        doUnSubscribe(market, codeList, amd::ama::SubscribeSecuDataType::kTickExecution, categoryType, dataType);
    } else {
        uint64_t subDataType = getSubscribeDataType(amdType);
        doUnSubscribe(market, codeList, subDataType, categoryType, dataType);
    }
    // TODO change key
    // only if unsubscribe a whole type of data, would stop a threadedQueue
    if(codeList.empty()) {
        removeSubscribe(amdType, market);
    }
}
