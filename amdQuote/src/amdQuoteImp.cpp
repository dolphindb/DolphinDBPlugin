#include "amdQuoteImp.h"
#include "Concurrent.h"
#include "amdSpiImp.h"

using std::to_string;

#ifdef ENUM_OR_STRING
#undef ENUM_OR_STRING
#endif
#define ENUM_OR_STRING(x) #x


AMDTableType getAmdTableType(AMDDataType dataType, int market);

AmdQuote::AmdQuote(const string& username, const string& password, vector<string> ips, vector<int> ports, SessionSP session) : username_(username),
    password_(password), ips_(ips), ports_(ports) {
    if (ips.size() != ports.size()) {
        throw RuntimeException("[PLUGIN::AMDQUOTE] connect to Amd Failed, ips num not equal to ports");
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
    if(username.size() >= amd::ama::ConstField::kUsernameLen) {
        throw RuntimeException("[PLUGIN::AMDQUOTE] username length should be less than 32");
    }
    strcpy(cfg_.username, username.c_str());
    if(password.size() >= amd::ama::ConstField::kPasswordLen) {
        throw RuntimeException("[PLUGIN::AMDQUOTE] password length should be less than 64");
    }
    strcpy(cfg_.password, password.c_str());
    cfg_.ums_server_cnt = ips.size();
    for (unsigned int i = 0; i < cfg_.ums_server_cnt; i++) {
        strcpy(cfg_.ums_servers[i].local_ip, "0.0.0.0");
        if(ips[i].size() >= amd::ama::ConstField::kIPMaxLen) {
            throw RuntimeException("[PLUGIN::AMDQUOTE] ip address length should be less than 24");
        }
        strcpy(cfg_.ums_servers[i].server_ip, ips[i].c_str());
        cfg_.ums_servers[i].server_port = ports[i];
    }

    cfg_.is_thread_safe = false;

    amdSpi_ = new AMDSpiImp(session);
    try {
        if (amd::ama::IAMDApi::Init(amdSpi_, cfg_) != amd::ama::ErrorCode::kSuccess) {
            amd::ama::IAMDApi::Release();
            throw RuntimeException("Init AMA failed");
        }
    } catch(std::exception& exception) {
        throw RuntimeException(string("[PLUGIN::AMDQUOTE] ") + exception.what());
    }

    STOP_TEST = false;
}

void AmdQuote::subscribeOrderExecution( string                     orderExecutionType,
                                        int                        market,
                                        DictionarySP               dict,
                                        vector<string>             codeList,
                                        FunctionDefSP              transform,
                                        bool                       receivedTimeFlag,
                                        long long                  dailyStartTimestamp)
{
    if(!receivedTimeFlag || !DAILY_INDEX_FLAG) {
        throw RuntimeException("[PLUGIN::AMDQUOTE] Subscribe type " + orderExecutionType + " must open ReceivedTime and DailyIndex option.");
    }
    uint64_t categoryType = 0;
    categoryType = amd::ama::SubscribeCategoryType::kStock;
    vector<string> nullCodeList;
    unsubscribe(orderExecutionType, 0, nullCodeList);               // temporal
    VectorSP keys = dict->keys();
    VectorSP values = dict->values();
    std::unordered_map<int, TableSP> tables = {};
    for(int index = 0; index < keys->size(); ++index) {
        int channelNo = keys->get(index)->getInt();
        TableSP table = values->get(index);
        tables[channelNo] = table;
    }
    if(orderExecutionType == "orderExecution") {
        categoryType = amd::ama::SubscribeCategoryType::kStock;
    } else if (orderExecutionType == "fundOrderExecution") {
        categoryType = amd::ama::SubscribeCategoryType::kFund;
    } else if (orderExecutionType == "bondOrderExecution") {
        categoryType = amd::ama::SubscribeCategoryType::kBond;
    }
    amdSpi_->setOrderExecutionData(orderExecutionType, tables, transform, receivedTimeFlag, dailyStartTimestamp, market);
    amdSpi_->startOrderExecution(orderExecutionType);

    // if codeList size == 0 subscribe all market
    unsigned int codeSize = 2;  // order & execution
    if (codeList.size() > 0) {
        codeSize = codeList.size()*2;
    }

    amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
    Defer df([=](){delete[] sub;});
    memset(sub, 0, sizeof(amd::ama::SubscribeCategoryItem) * codeSize);

    if (codeList.size() == 0) {
        sub[0].data_type = amd::ama::SubscribeSecuDataType::kTickExecution;
        sub[0].category_type = categoryType;
        sub[0].market = market;
        sub[0].security_code[0] = '\0';

        sub[1].data_type = amd::ama::SubscribeSecuDataType::kTickOrder;
        sub[1].category_type = categoryType;
        sub[1].market = market;
        sub[1].security_code[0] = '\0';
    } else {
        for (unsigned int codeIndex = 0; codeIndex*2 < codeSize; codeIndex++) {
            sub[2*codeIndex].data_type = amd::ama::SubscribeSecuDataType::kTickExecution;
            sub[2*codeIndex].category_type = categoryType;
            sub[2*codeIndex].market = market;
            memcpy(sub[2*codeIndex].security_code, codeList[codeIndex].c_str(), codeList[codeIndex].length());

            sub[2*codeIndex+1].data_type = amd::ama::SubscribeSecuDataType::kTickOrder;
            sub[2*codeIndex+1].category_type = categoryType;
            sub[2*codeIndex+1].market = market;
            memcpy(sub[2*codeIndex+1].security_code, codeList[codeIndex].c_str(), codeList[codeIndex].length());
        }
    }
    try {
        if (amd::ama::IAMDApi::SubscribeData(
                amd::ama::SubscribeType::kAdd, sub, codeSize)
        != amd::ama::ErrorCode::kSuccess)
        {
            throw RuntimeException("subscribe " + orderExecutionType + " err, market:" + to_string(market));
            return;
        }
    } catch(std::exception& exception) {
        throw RuntimeException(string("[PLUGIN::AMDQUOTE] ") + exception.what());
    }

    Info info(orderExecutionType, market);
    infoSet_.insert(info);
}

void AmdQuote::subscribeSnapshot(int market, TableSP table, vector<string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp) {
    vector<string> nullCodeList;
    unsubscribe("snapshot", market, nullCodeList);
    amdSpi_->setSnapshotData(table, transform, receivedTimeFlag, dailyStartTimestamp, market);

    unsigned int codeSize = 1;
    if (codeList.size() > 0) {
        codeSize = codeList.size();
    }

    amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
    Defer df([=](){delete[] sub;});
    memset(sub, 0, sizeof(amd::ama::SubscribeCategoryItem) * codeSize);

    if (codeList.size() == 0) {
        sub[0].data_type = amd::ama::SubscribeSecuDataType::kSnapshot;
        sub[0].category_type = amd::ama::SubscribeCategoryType::kStock;
        sub[0].market = market;
        sub[0].security_code[0] = '\0';
    } else {
        for (unsigned int codeIndex = 0; codeIndex < codeSize; codeIndex++) {
            sub[codeIndex].data_type = amd::ama::SubscribeSecuDataType::kSnapshot;
            sub[codeIndex].category_type = amd::ama::SubscribeCategoryType::kStock;
            sub[codeIndex].market = market;
            memcpy(sub[codeIndex].security_code, codeList[codeIndex].c_str(), codeList[codeIndex].length());
        }
    }
    try {
        if (amd::ama::IAMDApi::SubscribeData(
                amd::ama::SubscribeType::kAdd, sub, codeSize)
        != amd::ama::ErrorCode::kSuccess)
        {
            throw RuntimeException("subscribe Snapshot err, market:" + to_string(market));
            return;
        }
    } catch(std::exception& exception) {
        throw RuntimeException(string("[PLUGIN::AMDQUOTE] ") + exception.what());
    }

    Info info("snapshot", market);
    infoSet_.insert(info);
}

void AmdQuote::subscribeExecution(int market, TableSP table, vector<string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp) {
    vector<string> nullCodeList;
    unsubscribe("execution", market, nullCodeList);
    amdSpi_->setExecutionData(table, transform, receivedTimeFlag, dailyStartTimestamp, market);
    unsigned int codeSize = 1;
    if (codeList.size() > 0) {
        codeSize = codeList.size();
    }

    amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
    Defer df([=](){delete[] sub;});
    memset(sub, 0, sizeof(amd::ama::SubscribeCategoryItem) * codeSize);

    if (codeList.size() == 0) {
        sub[0].data_type = amd::ama::SubscribeSecuDataType::kTickExecution;
        sub[0].category_type = amd::ama::SubscribeCategoryType::kStock;
        sub[0].market = market;
        sub[0].security_code[0] = '\0';
    } else {
        for (unsigned int codeIndex = 0; codeIndex < codeSize; codeIndex++) {
            sub[codeIndex].data_type = amd::ama::SubscribeSecuDataType::kTickExecution;
            sub[codeIndex].category_type = amd::ama::SubscribeCategoryType::kStock;
            sub[codeIndex].market = market;
            memcpy(sub[codeIndex].security_code, codeList[codeIndex].c_str(), codeList[codeIndex].length());
        }
    }
    try {
        if (amd::ama::IAMDApi::SubscribeData(
                amd::ama::SubscribeType::kAdd, sub, codeSize)
        != amd::ama::ErrorCode::kSuccess)
        {
            throw RuntimeException("subscribe Execution err, market:" + to_string(market));
            return;
        }
    } catch(std::exception& exception) {
        throw RuntimeException(string("[PLUGIN::AMDQUOTE] ") + exception.what());
    }

    Info info("execution", market);
    infoSet_.insert(info);
}

void AmdQuote::subscribeOrder(int market, TableSP table, vector<string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp) {
    vector<string> nullCodeList;
    unsubscribe("order", market, nullCodeList);
    amdSpi_->setOrderData(table, transform, receivedTimeFlag, dailyStartTimestamp, market);
    unsigned int codeSize = 1;
    if (codeList.size() > 0) {
        codeSize = codeList.size();
    }

    amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
    Defer df([=](){delete[] sub;});
    memset(sub, 0, sizeof(amd::ama::SubscribeCategoryItem) * codeSize);

    if (codeList.size() == 0) {
        sub[0].data_type = amd::ama::SubscribeSecuDataType::kTickOrder;
        sub[0].category_type = amd::ama::SubscribeCategoryType::kStock;
        sub[0].market = market;
        sub[0].security_code[0] = '\0';
    } else {
        for (unsigned int codeIndex = 0; codeIndex < codeSize; codeIndex++) {
            sub[codeIndex].data_type = amd::ama::SubscribeSecuDataType::kTickOrder;
            sub[codeIndex].category_type = amd::ama::SubscribeCategoryType::kStock;
            sub[codeIndex].market = market;
            memcpy(sub[codeIndex].security_code, codeList[codeIndex].c_str(), codeList[codeIndex].length());
        }
    }
    try {
        if (amd::ama::IAMDApi::SubscribeData(
                amd::ama::SubscribeType::kAdd, sub, codeSize)
        != amd::ama::ErrorCode::kSuccess)
        {
            throw RuntimeException("subscribe Order err, market:" + to_string(market));
            return;
        }
    } catch(std::exception& exception) {
        throw RuntimeException(string("[PLUGIN::AMDQUOTE] ") + exception.what());
    }

    Info info("order", market);
    infoSet_.insert(info);
}

void AmdQuote::subscribeFundSnapshot(int market, TableSP table, vector<string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp) {
    vector<string> nullCodeList;
    unsubscribe("fundSnapshot", market, nullCodeList);
    amdSpi_->setFundSnapshotData(table, transform, receivedTimeFlag, dailyStartTimestamp, market);
    unsigned int codeSize = 1;
    if (codeList.size() > 0) {
        codeSize = codeList.size();
    }

    amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
    Defer df([=](){delete[] sub;});
    memset(sub, 0, sizeof(amd::ama::SubscribeCategoryItem) * codeSize);

    if (codeList.size() == 0) {
        sub[0].data_type = amd::ama::SubscribeSecuDataType::kSnapshot;
        sub[0].category_type = amd::ama::SubscribeCategoryType::kFund;
        sub[0].market = market;
        sub[0].security_code[0] = '\0';
    } else {
        for (unsigned int codeIndex = 0; codeIndex < codeSize; codeIndex++) {
            sub[codeIndex].data_type = amd::ama::SubscribeSecuDataType::kSnapshot;
            sub[codeIndex].category_type = amd::ama::SubscribeCategoryType::kFund;
            sub[codeIndex].market = market;
            memcpy(sub[codeIndex].security_code, codeList[codeIndex].c_str(), codeList[codeIndex].length());
        }
    }
    try {
        if (amd::ama::IAMDApi::SubscribeData(
                amd::ama::SubscribeType::kAdd, sub, codeSize)
        != amd::ama::ErrorCode::kSuccess)
        {
            throw RuntimeException("subscribe fundSnapshot err, market:"+ to_string(market));
            return;
        }
    } catch(std::exception& exception) {
        throw RuntimeException(string("[PLUGIN::AMDQUOTE] ") + exception.what());
    }

    Info info("fundSnapshot", market);
    infoSet_.insert(info);
}

void AmdQuote::subscribeFundExecution(int market, TableSP table, vector<string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp) {
    vector<string> nullCodeList;
    unsubscribe("fundExecution", market, nullCodeList);
    amdSpi_->setFundExecutionData(table, transform, receivedTimeFlag, dailyStartTimestamp, market);
    unsigned int codeSize = 1;
    if (codeList.size() > 0) {
        codeSize = codeList.size();
    }

    amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
    Defer df([=](){delete[] sub;});
    memset(sub, 0, sizeof(amd::ama::SubscribeCategoryItem) * codeSize);

    if (codeList.size() == 0) {
        sub[0].data_type = amd::ama::SubscribeSecuDataType::kTickExecution;
        sub[0].category_type = amd::ama::SubscribeCategoryType::kFund;
        sub[0].market = market;
        sub[0].security_code[0] = '\0';
    } else {
        for (unsigned int codeIndex = 0; codeIndex < codeSize; codeIndex++) {
            sub[codeIndex].data_type = amd::ama::SubscribeSecuDataType::kTickExecution;
            sub[codeIndex].category_type = amd::ama::SubscribeCategoryType::kFund;
            sub[codeIndex].market = market;
            memcpy(sub[codeIndex].security_code, codeList[codeIndex].c_str(), codeList[codeIndex].length());
        }
    }
    try {
        if (amd::ama::IAMDApi::SubscribeData(
                amd::ama::SubscribeType::kAdd, sub, codeSize)
        != amd::ama::ErrorCode::kSuccess)
        {
            throw RuntimeException("subscribe fundExecution err, market:"+ to_string(market));
            return;
        }
    } catch(std::exception& exception) {
        throw RuntimeException(string("[PLUGIN::AMDQUOTE] ") + exception.what());
    }

    Info info("fundExecution", market);
    infoSet_.insert(info);
}

AmdQuote::~AmdQuote() {
    STOP_TEST = true;
    vector<string> codeList;
    unsubscribe("all", 0, codeList);
    amd::ama::IAMDApi::Release();
    delete amdSpi_;
}

void AmdQuote::subscribeFundOrder(int market, TableSP table, vector<string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp) {
    vector<string> nullCodeList;
    unsubscribe("fundOrder", market, nullCodeList);
    amdSpi_->setFundOrderData(table, transform, receivedTimeFlag, dailyStartTimestamp, market);
    unsigned int codeSize = 1;
    if (codeList.size() > 0) {
        codeSize = codeList.size();
    }

    amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
    Defer df([=](){delete[] sub;});
    memset(sub, 0, sizeof(amd::ama::SubscribeCategoryItem) * codeSize);

    if (codeList.size() == 0) {
        sub[0].data_type = amd::ama::SubscribeSecuDataType::kTickOrder;
        sub[0].category_type = amd::ama::SubscribeCategoryType::kFund;
        sub[0].market = market;
        sub[0].security_code[0] = '\0';
    } else {
        for (unsigned int codeIndex = 0; codeIndex < codeSize; codeIndex++) {
            sub[codeIndex].data_type = amd::ama::SubscribeSecuDataType::kTickOrder;
            sub[codeIndex].category_type = amd::ama::SubscribeCategoryType::kFund;
            sub[codeIndex].market = market;
            memcpy(sub[codeIndex].security_code, codeList[codeIndex].c_str(), codeList[codeIndex].length());
        }
    }
    try {
        if (amd::ama::IAMDApi::SubscribeData(
                amd::ama::SubscribeType::kAdd, sub, codeSize)
        != amd::ama::ErrorCode::kSuccess)
        {
            throw RuntimeException("subscribe fundOrder err, market:" + to_string(market));
            return;
        }
    } catch(std::exception& exception) {
        throw RuntimeException(string("[PLUGIN::AMDQUOTE] ") + exception.what());
    }

    Info info("fundOrder", market);
    infoSet_.insert(info);
}

void AmdQuote::subscribeBondSnapshot(int market, TableSP table, vector<string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp) {
    vector<string> nullCodeList;
    unsubscribe("bondSnapshot", market, nullCodeList);
    amdSpi_->setBondSnapshotData(table, transform, receivedTimeFlag, dailyStartTimestamp, market);
    unsigned int codeSize = 1;
    if (codeList.size() > 0) {
        codeSize = codeList.size();
    }

    amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
    Defer df([=](){delete[] sub;});
    memset(sub, 0, sizeof(amd::ama::SubscribeCategoryItem) * codeSize);

    if (codeList.size() == 0) {
        sub[0].data_type = amd::ama::SubscribeSecuDataType::kSnapshot;
        sub[0].category_type = amd::ama::SubscribeCategoryType::kBond;
        sub[0].market = market;
        sub[0].security_code[0] = '\0';
    } else {
        for (unsigned int codeIndex = 0; codeIndex < codeSize; codeIndex++) {
            sub[codeIndex].data_type = amd::ama::SubscribeSecuDataType::kSnapshot;
            sub[codeIndex].category_type = amd::ama::SubscribeCategoryType::kBond;
            sub[codeIndex].market = market;
            memcpy(sub[codeIndex].security_code, codeList[codeIndex].c_str(), codeList[codeIndex].length());
        }
    }

    try {
        if (amd::ama::IAMDApi::SubscribeData(
                amd::ama::SubscribeType::kAdd, sub, codeSize)
        != amd::ama::ErrorCode::kSuccess)
        {
            throw RuntimeException("subscribe bondSnapshot err, market:" + to_string(market));
            return;
        }
    } catch(std::exception& exception) {
        throw RuntimeException(string("[PLUGIN::AMDQUOTE] ") + exception.what());
    }

    Info info("bondSnapshot", market);
    infoSet_.insert(info);
}

void AmdQuote::subscribeBondExecution(int market, TableSP table, vector<string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp) {
    vector<string> nullCodeList;
    unsubscribe("bondExecution", market, nullCodeList);
    amdSpi_->setBondExecutionData(table, transform, receivedTimeFlag, dailyStartTimestamp, market);
    unsigned int codeSize = 1;
    if (codeList.size() > 0) {
        codeSize = codeList.size();
    }

    amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
    Defer df([=](){delete[] sub;});
    memset(sub, 0, sizeof(amd::ama::SubscribeCategoryItem) * codeSize);

    if (codeList.size() == 0) {
        sub[0].data_type = amd::ama::SubscribeSecuDataType::kTickExecution;
        sub[0].category_type = amd::ama::SubscribeCategoryType::kBond;
        sub[0].market = market;
        sub[0].security_code[0] = '\0';
    } else {
        for (unsigned int codeIndex = 0; codeIndex < codeSize; codeIndex++) {
            sub[codeIndex].data_type = amd::ama::SubscribeSecuDataType::kTickExecution;
            sub[codeIndex].category_type = amd::ama::SubscribeCategoryType::kBond;
            sub[codeIndex].market = market;
            memcpy(sub[codeIndex].security_code, codeList[codeIndex].c_str(), codeList[codeIndex].length());
        }
    }
    try {
        if (amd::ama::IAMDApi::SubscribeData(
                amd::ama::SubscribeType::kAdd, sub, codeSize)
        != amd::ama::ErrorCode::kSuccess)
        {
            throw RuntimeException("subscribe bondExecution err, market:"+ to_string(market));
            return;
        }
    } catch(std::exception& exception) {
        throw RuntimeException(string("[PLUGIN::AMDQUOTE] ") + exception.what());
    }

    Info info("bondExecution", market);
    infoSet_.insert(info);
}

void AmdQuote::subscribeBondOrder(int market, TableSP table, vector<string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp) {
    vector<string> nullCodeList;
    unsubscribe("bondOrder", market, nullCodeList);
    amdSpi_->setBondOrderData(table, transform, receivedTimeFlag, dailyStartTimestamp, market);
    unsigned int codeSize = 1;
    if (codeList.size() > 0) {
        codeSize = codeList.size();
    }

    amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
    Defer df([=](){delete[] sub;});
    memset(sub, 0, sizeof(amd::ama::SubscribeCategoryItem) * codeSize);

    if (codeList.size() == 0) {
        sub[0].data_type = amd::ama::SubscribeSecuDataType::kTickOrder;
        sub[0].category_type = amd::ama::SubscribeCategoryType::kBond;
        sub[0].market = market;
        sub[0].security_code[0] = '\0';
    } else {
        for (unsigned int codeIndex = 0; codeIndex < codeSize; codeIndex++) {
            sub[codeIndex].data_type = amd::ama::SubscribeSecuDataType::kTickOrder;
            sub[codeIndex].category_type = amd::ama::SubscribeCategoryType::kBond;
            sub[codeIndex].market = market;
            memcpy(sub[codeIndex].security_code, codeList[codeIndex].c_str(), codeList[codeIndex].length());
        }
    }
    try {
        if (amd::ama::IAMDApi::SubscribeData(
                amd::ama::SubscribeType::kAdd, sub, codeSize)
        != amd::ama::ErrorCode::kSuccess)
        {
            throw RuntimeException("subscribe bondOrder err, market:" + to_string(market));
            return;
        }
    } catch(std::exception& exception) {
        throw RuntimeException(string("[PLUGIN::AMDQUOTE] ") + exception.what());
    }

    Info info("bondOrder", market);
    infoSet_.insert(info);
}

void AmdQuote::subscribeIndex(int market, TableSP table, vector<string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp) {
    vector<string> nullCodeList;
    unsubscribe("index", market, nullCodeList);
    amdSpi_->setIndexData(table, transform, receivedTimeFlag, dailyStartTimestamp, market);
    unsigned int codeSize = 1;
    if (codeList.size() > 0) {
        codeSize = codeList.size();
    }

    amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
    Defer df([=](){delete[] sub;});
    memset(sub, 0, sizeof(amd::ama::SubscribeCategoryItem) * codeSize);

    if (codeList.size() == 0) {
        sub[0].data_type = amd::ama::SubscribeSecuDataType::kSnapshot;
        sub[0].category_type = amd::ama::SubscribeCategoryType::kIndex;
        sub[0].market = market;
        sub[0].security_code[0] = '\0';
    } else {
        for (unsigned int codeIndex = 0; codeIndex < codeSize; codeIndex++) {
            sub[codeIndex].data_type = amd::ama::SubscribeSecuDataType::kSnapshot;
            sub[codeIndex].category_type = amd::ama::SubscribeCategoryType::kIndex;
            sub[codeIndex].market = market;
            memcpy(sub[codeIndex].security_code, codeList[codeIndex].c_str(), codeList[codeIndex].length());
        }
    }
    try {
        if (amd::ama::IAMDApi::SubscribeData(
                amd::ama::SubscribeType::kAdd, sub, codeSize)
        != amd::ama::ErrorCode::kSuccess)
        {
            throw RuntimeException("subscribe Index err, market:" + to_string(market));
            return;
        }
    } catch(std::exception& exception) {
        throw RuntimeException(string("[PLUGIN::AMDQUOTE] ") + exception.what());
    }

    Info info("index", market);
    infoSet_.insert(info);
}

void AmdQuote::enableLatencyStatistics(bool flag) {
    amdSpi_->setLatencyFlag(flag);
}

void AmdQuote::subscribeOrderQueue(int market, TableSP table, vector<string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp) {
    vector<string> nullCodeList;
    unsubscribe("orderQueue", market, nullCodeList);
    amdSpi_->setOrderQueueData(table, transform, receivedTimeFlag, dailyStartTimestamp, market);
    unsigned int codeSize = 1;
    if (codeList.size() > 0) {
        codeSize = codeList.size();
    }

    amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
    Defer df([=](){delete[] sub;});
    memset(sub, 0, sizeof(amd::ama::SubscribeCategoryItem) * codeSize);

    if (codeList.size() == 0) {
        sub[0].data_type = amd::ama::SubscribeSecuDataType::kOrderQueue;
        sub[0].category_type = amd::ama::SubscribeCategoryType::kNone;
        sub[0].market = market;
        sub[0].security_code[0] = '\0';
    } else {
        for (unsigned int codeIndex = 0; codeIndex < codeSize; codeIndex++) {
            sub[codeIndex].data_type = amd::ama::SubscribeSecuDataType::kOrderQueue;
            sub[codeIndex].category_type = amd::ama::SubscribeCategoryType::kNone;
            sub[codeIndex].market = market;
            memcpy(sub[codeIndex].security_code, codeList[codeIndex].c_str(), codeList[codeIndex].length());
        }
    }
    try {
        if (amd::ama::IAMDApi::SubscribeData(
                amd::ama::SubscribeType::kAdd, sub, codeSize)
        != amd::ama::ErrorCode::kSuccess)
        {
            throw RuntimeException("subscribe OrderQueue err, market:" + to_string(market));
            return;
        }
    } catch(std::exception& exception) {
        throw RuntimeException(string("[PLUGIN::AMDQUOTE] ") + exception.what());
    }

    Info info("orderQueue", market);
    infoSet_.insert(info);
}

void AmdQuote::unsubscribe(string dataType, int market, vector<string> codeList) {
    // unsubscribe
    // stop async threads
    if (dataType == "orderExecution") {
        amdSpi_->unsetFlag(dataType);
        unsigned int codeSize = 2;
        if (codeList.size() > 0) {
            codeSize = codeList.size()*2;
        }

        amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
        Defer df([=](){delete[] sub;});
        memset(sub, 0, sizeof(amd::ama::SubscribeCategoryItem) * codeSize);

        if (codeList.size() == 0) {
            sub[0].data_type = amd::ama::SubscribeSecuDataType::kTickOrder;
            sub[0].category_type = amd::ama::SubscribeCategoryType::kStock;
            sub[0].market = market;
            sub[0].security_code[0] = '\0';

            sub[1].data_type = amd::ama::SubscribeSecuDataType::kTickExecution;
            sub[1].category_type = amd::ama::SubscribeCategoryType::kStock;
            sub[1].market = market;
            sub[1].security_code[0] = '\0';
        } else {
            for (unsigned int codeIndex = 0; codeIndex*2 < codeSize; codeIndex++) {
                sub[2*codeIndex].data_type = amd::ama::SubscribeSecuDataType::kTickOrder;
                sub[2*codeIndex].category_type = amd::ama::SubscribeCategoryType::kStock;
                sub[2*codeIndex].market = market;
                memcpy(sub[2*codeIndex].security_code, codeList[codeIndex].c_str(), codeList[codeIndex].length());

                sub[2*codeIndex+1].data_type = amd::ama::SubscribeSecuDataType::kTickExecution;
                sub[2*codeIndex+1].category_type = amd::ama::SubscribeCategoryType::kStock;
                sub[2*codeIndex+1].market = market;
                memcpy(sub[2*codeIndex+1].security_code, codeList[codeIndex].c_str(), codeList[codeIndex].length());
            }
        }
        try {
            if (amd::ama::IAMDApi::SubscribeData(amd::ama::SubscribeType::kDel, sub, codeSize) != amd::ama::ErrorCode::kSuccess)
            {
                throw RuntimeException("unsubscribe orderExecution err, market:"+ to_string(market));
            }
        } catch(std::exception& exception) {
            throw RuntimeException(string("[PLUGIN::AMDQUOTE] ") + exception.what());
        }

        amdSpi_->clearOrderExecution(dataType);
    } else if (dataType == "fundOrderExecution") {
        amdSpi_->unsetFlag(dataType);
        unsigned int codeSize = 2;
        if (codeList.size() > 0) {
            codeSize = codeList.size()*2;
        }

        amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
        Defer df([=](){delete[] sub;});
        memset(sub, 0, sizeof(amd::ama::SubscribeCategoryItem) * codeSize);

        if (codeList.size() == 0) {
            sub[0].data_type = amd::ama::SubscribeSecuDataType::kTickOrder;
            sub[0].category_type = amd::ama::SubscribeCategoryType::kFund;
            sub[0].market = market;
            sub[0].security_code[0] = '\0';

            sub[1].data_type = amd::ama::SubscribeSecuDataType::kTickExecution;
            sub[1].category_type = amd::ama::SubscribeCategoryType::kFund;
            sub[1].market = market;
            sub[1].security_code[0] = '\0';
        } else {
            for (unsigned int codeIndex = 0; codeIndex*2 < codeSize; codeIndex++) {
                sub[2*codeIndex].data_type = amd::ama::SubscribeSecuDataType::kTickOrder;
                sub[2*codeIndex].category_type = amd::ama::SubscribeCategoryType::kFund;
                sub[2*codeIndex].market = market;
                memcpy(sub[2*codeIndex].security_code, codeList[codeIndex].c_str(), codeList[codeIndex].length());

                sub[2*codeIndex+1].data_type = amd::ama::SubscribeSecuDataType::kTickExecution;
                sub[2*codeIndex+1].category_type = amd::ama::SubscribeCategoryType::kFund;
                sub[2*codeIndex+1].market = market;
                memcpy(sub[2*codeIndex+1].security_code, codeList[codeIndex].c_str(), codeList[codeIndex].length());
            }
        }
        try {
            if (amd::ama::IAMDApi::SubscribeData(amd::ama::SubscribeType::kDel, sub, codeSize) != amd::ama::ErrorCode::kSuccess)
            {
                throw RuntimeException("unsubscribe fundOrderExecution err, market:"+ to_string(market));
            }
        } catch(std::exception& exception) {
            throw RuntimeException(string("[PLUGIN::AMDQUOTE] ") + exception.what());
        }

        amdSpi_->clearOrderExecution(dataType);
    } else if (dataType == "bondOrderExecution") {
        amdSpi_->unsetFlag(dataType);
        unsigned int codeSize = 2;
        if (codeList.size() > 0) {
            codeSize = codeList.size()*2;
        }

        amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
        Defer df([=](){delete[] sub;});
        memset(sub, 0, sizeof(amd::ama::SubscribeCategoryItem) * codeSize);

        if (codeList.size() == 0) {
            sub[0].data_type = amd::ama::SubscribeSecuDataType::kTickOrder;
            sub[0].category_type = amd::ama::SubscribeCategoryType::kBond;
            sub[0].market = market;
            sub[0].security_code[0] = '\0';

            sub[1].data_type = amd::ama::SubscribeSecuDataType::kTickExecution;
            sub[1].category_type = amd::ama::SubscribeCategoryType::kBond;
            sub[1].market = market;
            sub[1].security_code[0] = '\0';
        } else {
            for (unsigned int codeIndex = 0; codeIndex*2 < codeSize; codeIndex++) {
                sub[2*codeIndex].data_type = amd::ama::SubscribeSecuDataType::kTickOrder;
                sub[2*codeIndex].category_type = amd::ama::SubscribeCategoryType::kBond;
                sub[2*codeIndex].market = market;
                memcpy(sub[2*codeIndex].security_code, codeList[codeIndex].c_str(), codeList[codeIndex].length());

                sub[2*codeIndex+1].data_type = amd::ama::SubscribeSecuDataType::kTickExecution;
                sub[2*codeIndex+1].category_type = amd::ama::SubscribeCategoryType::kBond;
                sub[2*codeIndex+1].market = market;
                memcpy(sub[2*codeIndex+1].security_code, codeList[codeIndex].c_str(), codeList[codeIndex].length());
            }
        }
        try {
            if (amd::ama::IAMDApi::SubscribeData(amd::ama::SubscribeType::kDel, sub, codeSize) != amd::ama::ErrorCode::kSuccess)
            {
                throw RuntimeException("unsubscribe order bondOrderExecution err, market:" + to_string(market));
            }
        } catch(std::exception& exception) {
            throw RuntimeException(string("[PLUGIN::AMDQUOTE] ") + exception.what());
        }

        amdSpi_->clearOrderExecution(dataType);
    }

    else if (dataType == "snapshot") {
        unsigned int codeSize = 1;
        if (codeList.size() > 0) {
            codeSize = codeList.size();
        }

        amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
        Defer df([=](){delete[] sub;});
        memset(sub, 0, sizeof(amd::ama::SubscribeCategoryItem) * codeSize);

        if (codeList.size() == 0) {
            sub[0].data_type = amd::ama::SubscribeSecuDataType::kSnapshot;
            sub[0].category_type = amd::ama::SubscribeCategoryType::kStock;
            sub[0].market = market;
            sub[0].security_code[0] = '\0';
        } else {
            for (unsigned int codeIndex = 0; codeIndex < codeSize; codeIndex++) {
                sub[codeIndex].data_type = amd::ama::SubscribeSecuDataType::kSnapshot;
                sub[codeIndex].category_type = amd::ama::SubscribeCategoryType::kStock;
                sub[codeIndex].market = market;
                memcpy(sub[codeIndex].security_code, codeList[codeIndex].c_str(), codeList[codeIndex].length());
            }
        }

        try {
            if (amd::ama::IAMDApi::SubscribeData(
                amd::ama::SubscribeType::kDel, sub, codeSize)
            != amd::ama::ErrorCode::kSuccess)
            {
                throw RuntimeException("subscribe Snapshot err, market:"+to_string(market));
            }
        } catch(std::exception& exception) {
            throw RuntimeException(string("[PLUGIN::AMDQUOTE] ") + exception.what());
        }

    } else if (dataType == "execution") {
        unsigned int codeSize = 1;
        if (codeList.size() > 0) {
            codeSize = codeList.size();
        }

        amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
        Defer df([=](){delete[] sub;});
        memset(sub, 0, sizeof(amd::ama::SubscribeCategoryItem) * codeSize);

        /*
        if (market < (int)amd::ama::MarketType::kNone || market > (int)amd::ama::MarketType::kMax) {
            LOG_ERR("unsubscribe execution err, illegal market:", market);
            return ;
        }*/

        if (codeList.size() == 0) {
            sub[0].data_type = amd::ama::SubscribeSecuDataType::kTickExecution;
            sub[0].category_type = amd::ama::SubscribeCategoryType::kStock;
            sub[0].market = market;
            sub[0].security_code[0] = '\0';
        } else {
            for (unsigned int codeIndex = 0; codeIndex < codeSize; codeIndex++) {
                sub[codeIndex].data_type = amd::ama::SubscribeSecuDataType::kTickExecution;
                sub[codeIndex].category_type = amd::ama::SubscribeCategoryType::kStock;
                sub[codeIndex].market = market;
                memcpy(sub[codeIndex].security_code, codeList[codeIndex].c_str(), codeList[codeIndex].length());
            }
        }
        try {
            if (amd::ama::IAMDApi::SubscribeData(
                amd::ama::SubscribeType::kDel, sub, codeSize)
            != amd::ama::ErrorCode::kSuccess)
            {
                throw RuntimeException("subscribe ticker Execution err, market:"+ to_string(market));
            }
        } catch(std::exception& exception) {
            throw RuntimeException(string("[PLUGIN::AMDQUOTE] ") + exception.what());
        }

    } else if (dataType == "order") {
        unsigned int codeSize = 1;
        if (codeList.size() > 0) {
            codeSize = codeList.size();
        }

        amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
        Defer df([=](){delete[] sub;});
        memset(sub, 0, sizeof(amd::ama::SubscribeCategoryItem) * codeSize);

        /*
        if (market < (int)amd::ama::MarketType::kNone || market > (int)amd::ama::MarketType::kMax) {
            LOG_ERR("unsubscribe order err, illegal market:", market);
            return ;
        }*/

        if (codeList.size() == 0) {
            sub[0].data_type = amd::ama::SubscribeSecuDataType::kTickOrder;
            sub[0].category_type = amd::ama::SubscribeCategoryType::kStock;
            sub[0].market = market;
            sub[0].security_code[0] = '\0';
        } else {
            for (unsigned int codeIndex = 0; codeIndex < codeSize; codeIndex++) {
                sub[codeIndex].data_type = amd::ama::SubscribeSecuDataType::kTickOrder;
                sub[codeIndex].category_type = amd::ama::SubscribeCategoryType::kStock;
                sub[codeIndex].market = market;
                memcpy(sub[codeIndex].security_code, codeList[codeIndex].c_str(), codeList[codeIndex].length());
            }
        }
        try {
            if (amd::ama::IAMDApi::SubscribeData(
                amd::ama::SubscribeType::kDel, sub, codeSize)
            != amd::ama::ErrorCode::kSuccess)
            {
                throw RuntimeException("unsubscribe order err, market:" + to_string(market));
            }
        } catch(std::exception& exception) {
            throw RuntimeException(string("[PLUGIN::AMDQUOTE] ") + exception.what());
        }

    } else if (dataType == "index") {
        unsigned int codeSize = 1;
        if (codeList.size() > 0) {
            codeSize = codeList.size();
        }

        amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
        Defer df([=](){delete[] sub;});
        memset(sub, 0, sizeof(amd::ama::SubscribeCategoryItem) * codeSize);

        /*
        if (market < (int)amd::ama::MarketType::kNone || market > (int)amd::ama::MarketType::kMax) {
            LOG_ERR("unsubscribe order err, illegal market:", market);
            return ;
        }*/

        if (codeList.size() == 0) {
            sub[0].data_type = amd::ama::SubscribeSecuDataType::kSnapshot;
            sub[0].category_type = amd::ama::SubscribeCategoryType::kIndex;
            sub[0].market = market;
            sub[0].security_code[0] = '\0';
        } else {
            for (unsigned int codeIndex = 0; codeIndex < codeSize; codeIndex++) {
                sub[codeIndex].data_type = amd::ama::SubscribeSecuDataType::kSnapshot;
                sub[codeIndex].category_type = amd::ama::SubscribeCategoryType::kIndex;
                sub[codeIndex].market = market;
                memcpy(sub[codeIndex].security_code, codeList[codeIndex].c_str(), codeList[codeIndex].length());
            }
        }
        try {
            if (amd::ama::IAMDApi::SubscribeData(
                amd::ama::SubscribeType::kDel, sub, codeSize)
            != amd::ama::ErrorCode::kSuccess)
            {
                throw RuntimeException("unsubscribe order err, market:" + to_string(market));
            }
        } catch(std::exception& exception) {
            throw RuntimeException(string("[PLUGIN::AMDQUOTE] ") + exception.what());
        }

    } else if (dataType == "orderQueue") {
        unsigned int codeSize = 1;
        if (codeList.size() > 0) {
            codeSize = codeList.size();
        }

        amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
        Defer df([=](){delete[] sub;});
        memset(sub, 0, sizeof(amd::ama::SubscribeCategoryItem) * codeSize);

        /*
        if (market < (int)amd::ama::MarketType::kNone || market > (int)amd::ama::MarketType::kMax) {
            LOG_ERR("unsubscribe order err, illegal market:", market);
            return ;
        }*/

        if (codeList.size() == 0) {
            sub[0].data_type = amd::ama::SubscribeSecuDataType::kOrderQueue;
            sub[0].category_type = amd::ama::SubscribeCategoryType::kNone;
            sub[0].market = 0;
            sub[0].security_code[0] = '\0';
        } else {
            for (unsigned int codeIndex = 0; codeIndex < codeSize; codeIndex++) {
                sub[codeIndex].data_type = amd::ama::SubscribeSecuDataType::kOrderQueue;
                sub[codeIndex].category_type = amd::ama::SubscribeCategoryType::kNone;
                sub[codeIndex].market = market;
                memcpy(sub[codeIndex].security_code, codeList[codeIndex].c_str(), codeList[codeIndex].length());
            }
        }
        // if (market == 0) {
        //     amd::ama::IAMDApi::SubscribeData(
        //         amd::ama::SubscribeType::kCancelAll, sub, codeSize);
        // } else {
        //     amd::ama::IAMDApi::SubscribeData(
        //         amd::ama::SubscribeType::kDel, sub, codeSize);
        // }
        try {
            if (amd::ama::IAMDApi::SubscribeData(
                amd::ama::SubscribeType::kDel, sub, codeSize)
            != amd::ama::ErrorCode::kSuccess)
            {
                throw RuntimeException("unsubscribe order err, market:"+ to_string(market));
            }
        } catch(std::exception& exception) {
            throw RuntimeException(string("[PLUGIN::AMDQUOTE] ") + exception.what());
        }

    } else if (dataType == "fundSnapshot") {
        unsigned int codeSize = 1;
        if (codeList.size() > 0) {
            codeSize = codeList.size();
        }

        amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
        Defer df([=](){delete[] sub;});
        memset(sub, 0, sizeof(amd::ama::SubscribeCategoryItem) * codeSize);

        if (codeList.size() == 0) {
            sub[0].data_type = amd::ama::SubscribeSecuDataType::kSnapshot;
            sub[0].category_type = amd::ama::SubscribeCategoryType::kFund;
            sub[0].market = market;
            sub[0].security_code[0] = '\0';
        } else {
            for (unsigned int codeIndex = 0; codeIndex < codeSize; codeIndex++) {
                sub[codeIndex].data_type = amd::ama::SubscribeSecuDataType::kSnapshot;
                sub[codeIndex].category_type = amd::ama::SubscribeCategoryType::kFund;
                sub[codeIndex].market = market;
                memcpy(sub[codeIndex].security_code, codeList[codeIndex].c_str(), codeList[codeIndex].length());
            }
        }

        try {
            if (amd::ama::IAMDApi::SubscribeData(
                amd::ama::SubscribeType::kDel, sub, codeSize)
            != amd::ama::ErrorCode::kSuccess)
            {
                throw RuntimeException("unsubscribe fundSnapshot err, market: "+ to_string(market));
            }
        } catch(std::exception& exception) {
            throw RuntimeException(string("[PLUGIN::AMDQUOTE] ") + exception.what());
        }

    } else if (dataType == "fundExecution") {
        unsigned int codeSize = 1;
        if (codeList.size() > 0) {
            codeSize = codeList.size();
        }

        amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
        Defer df([=](){delete[] sub;});
        memset(sub, 0, sizeof(amd::ama::SubscribeCategoryItem) * codeSize);

        /*
        if (market < (int)amd::ama::MarketType::kNone || market > (int)amd::ama::MarketType::kMax) {
            LOG_ERR("unsubscribe execution err, illegal market:", market);
            return ;
        }*/

        if (codeList.size() == 0) {
            sub[0].data_type = amd::ama::SubscribeSecuDataType::kTickExecution;
            sub[0].category_type = amd::ama::SubscribeCategoryType::kFund;
            sub[0].market = market;
            sub[0].security_code[0] = '\0';
        } else {
            for (unsigned int codeIndex = 0; codeIndex < codeSize; codeIndex++) {
                sub[codeIndex].data_type = amd::ama::SubscribeSecuDataType::kTickExecution;
                sub[codeIndex].category_type = amd::ama::SubscribeCategoryType::kFund;
                sub[codeIndex].market = market;
                memcpy(sub[codeIndex].security_code, codeList[codeIndex].c_str(), codeList[codeIndex].length());
            }
        }
        try {
            if (amd::ama::IAMDApi::SubscribeData(
                amd::ama::SubscribeType::kDel, sub, codeSize)
            != amd::ama::ErrorCode::kSuccess)
            {
                throw RuntimeException("subscribe fundExecution err, market:" + to_string(market));
            }
        } catch(std::exception& exception) {
            throw RuntimeException(string("[PLUGIN::AMDQUOTE] ") + exception.what());
        }

    } else if (dataType == "fundOrder") {
        unsigned int codeSize = 1;
        if (codeList.size() > 0) {
            codeSize = codeList.size();
        }

        amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
        Defer df([=](){delete[] sub;});
        memset(sub, 0, sizeof(amd::ama::SubscribeCategoryItem) * codeSize);

        if (codeList.size() == 0) {
            sub[0].data_type = amd::ama::SubscribeSecuDataType::kTickOrder;
            sub[0].category_type = amd::ama::SubscribeCategoryType::kFund;
            sub[0].market = market;
            sub[0].security_code[0] = '\0';
        } else {
            for (unsigned int codeIndex = 0; codeIndex < codeSize; codeIndex++) {
                sub[codeIndex].data_type = amd::ama::SubscribeSecuDataType::kTickOrder;
                sub[codeIndex].category_type = amd::ama::SubscribeCategoryType::kFund;
                sub[codeIndex].market = market;
                memcpy(sub[codeIndex].security_code, codeList[codeIndex].c_str(), codeList[codeIndex].length());
            }
        }
        try {
            if (amd::ama::IAMDApi::SubscribeData(
                amd::ama::SubscribeType::kDel, sub, codeSize)
            != amd::ama::ErrorCode::kSuccess)
            {
                throw RuntimeException("unsubscribe fundOrder err, market:" + to_string(market));
            }
        } catch(std::exception& exception) {
            throw RuntimeException(string("[PLUGIN::AMDQUOTE] ") + exception.what());
        }

    } else if (dataType == "bondSnapshot") {
        unsigned int codeSize = 1;
        if (codeList.size() > 0) {
            codeSize = codeList.size();
        }

        amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
        Defer df([=](){delete[] sub;});
        memset(sub, 0, sizeof(amd::ama::SubscribeCategoryItem) * codeSize);

        if (codeList.size() == 0) {
            sub[0].data_type = amd::ama::SubscribeSecuDataType::kSnapshot;
            sub[0].category_type = amd::ama::SubscribeCategoryType::kBond;
            sub[0].market = market;
            sub[0].security_code[0] = '\0';
        } else {
            for (unsigned int codeIndex = 0; codeIndex < codeSize; codeIndex++) {
                sub[codeIndex].data_type = amd::ama::SubscribeSecuDataType::kSnapshot;
                sub[codeIndex].category_type = amd::ama::SubscribeCategoryType::kBond;
                sub[codeIndex].market = market;
                memcpy(sub[codeIndex].security_code, codeList[codeIndex].c_str(), codeList[codeIndex].length());
            }
        }

        try {
            if (amd::ama::IAMDApi::SubscribeData(
                amd::ama::SubscribeType::kDel, sub, codeSize)
            != amd::ama::ErrorCode::kSuccess)
            {
                throw RuntimeException("unsubscribe bondSnapshot err, market:"+ to_string(market));
            }
        } catch(std::exception& exception) {
            throw RuntimeException(string("[PLUGIN::AMDQUOTE] ") + exception.what());
        }

    } else if (dataType == "bondExecution") {
        unsigned int codeSize = 1;
        if (codeList.size() > 0) {
            codeSize = codeList.size();
        }

        amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
        Defer df([=](){delete[] sub;});
        memset(sub, 0, sizeof(amd::ama::SubscribeCategoryItem) * codeSize);

        /*
        if (market < (int)amd::ama::MarketType::kNone || market > (int)amd::ama::MarketType::kMax) {
            LOG_ERR("unsubscribe execution err, illegal market:", market);
            return ;
        }*/

        if (codeList.size() == 0) {
            sub[0].data_type = amd::ama::SubscribeSecuDataType::kTickExecution;
            sub[0].category_type = amd::ama::SubscribeCategoryType::kBond;
            sub[0].market = market;
            sub[0].security_code[0] = '\0';
        } else {
            for (unsigned int codeIndex = 0; codeIndex < codeSize; codeIndex++) {
                sub[codeIndex].data_type = amd::ama::SubscribeSecuDataType::kTickExecution;
                sub[codeIndex].category_type = amd::ama::SubscribeCategoryType::kBond;
                sub[codeIndex].market = market;
                memcpy(sub[codeIndex].security_code, codeList[codeIndex].c_str(), codeList[codeIndex].length());
            }
        }
        try {
            if (amd::ama::IAMDApi::SubscribeData(
                amd::ama::SubscribeType::kDel, sub, codeSize)
            != amd::ama::ErrorCode::kSuccess)
            {
                throw RuntimeException("subscribe bondExecution err, market:"+ to_string(market));
            }
        } catch(std::exception& exception) {
            throw RuntimeException(string("[PLUGIN::AMDQUOTE] ") + exception.what());
        }

    } else if (dataType == "bondOrder") {
        unsigned int codeSize = 1;
        if (codeList.size() > 0) {
            codeSize = codeList.size();
        }

        amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
        Defer df([=](){delete[] sub;});
        memset(sub, 0, sizeof(amd::ama::SubscribeCategoryItem) * codeSize);

        if (codeList.size() == 0) {
            sub[0].data_type = amd::ama::SubscribeSecuDataType::kTickOrder;
            sub[0].category_type = amd::ama::SubscribeCategoryType::kBond;
            sub[0].market = market;
            sub[0].security_code[0] = '\0';
        } else {
            for (unsigned int codeIndex = 0; codeIndex < codeSize; codeIndex++) {
                sub[codeIndex].data_type = amd::ama::SubscribeSecuDataType::kTickOrder;
                sub[codeIndex].category_type = amd::ama::SubscribeCategoryType::kBond;
                sub[codeIndex].market = market;
                memcpy(sub[codeIndex].security_code, codeList[codeIndex].c_str(), codeList[codeIndex].length());
            }
        }
        try {
            if (amd::ama::IAMDApi::SubscribeData(
                amd::ama::SubscribeType::kDel, sub, codeSize)
            != amd::ama::ErrorCode::kSuccess)
            {
                throw RuntimeException("unsubscribe bondOrder err, market:"+ to_string(market));
            }
        } catch(std::exception& exception) {
            throw RuntimeException(string("[PLUGIN::AMDQUOTE] ") + exception.what());
        }

    } else if (dataType == "all") {
        amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[1];
        Defer df([=](){delete[] sub;});
        memset(sub, 0, sizeof(amd::ama::SubscribeCategoryItem));
        try {
            if (amd::ama::IAMDApi::SubscribeData(amd::ama::SubscribeType::kCancelAll, sub, 1)
                != amd::ama::ErrorCode::kSuccess)
            {
                throw RuntimeException("unsubscribe all err");
            }
        } catch(std::exception& exception) {
            throw RuntimeException(string("[PLUGIN::AMDQUOTE] ") + exception.what());
        }

        amdSpi_->clearOrderExecution("orderExecution");
        amdSpi_->clearOrderExecution("fundOrderExecution");
        amdSpi_->clearOrderExecution("bondOrderExecution");
    } else {
        throw RuntimeException("[PLUGIN::AMDQUOTE] AMD Quote unsubscribe, illegal dataType");
    }

    if (dataType == "all") {
        infoSet_.clear();
        amdSpi_->unsetFlag("all");
    } else if((dataType == "orderExecution" || dataType == "fundOrderExecution" || dataType == "bondOrderExecution") && market == 0) {
        int marketNum = 0;
        for(const Info & info: infoSet_) {
            if(info.datatype_ == dataType) {
                marketNum = info.marketType_;
            }
        }
        Info info(dataType, marketNum);
        infoSet_.erase(info);
    } else {
        Info info(dataType, market);
        infoSet_.erase(info);
        for(const Info & info: infoSet_) {
            if(info.datatype_ == dataType) {
                return;
            }
        }
        amdSpi_->unsetFlag(dataType);
    }
}

AMDTableType getAmdTableType(AMDDataType dataType, int market){
    switch (market)
    {
    case 101:
        switch (dataType)
        {
        case AMD_SNAPSHOT:
            return AMD_SNAPSHOT_SH;
        case AMD_EXECUTION:
            return AMD_EXECUTION_SH;
        case AMD_ORDER:
            return AMD_ORDER_SH;
        case AMD_INDEX:
            return AMD_INDEX_SH;
        case AMD_ORDER_QUEUE:
            return AMD_ORDER_QUEUE_SH;
        case AMD_FUND_SNAPSHOT:
            return AMD_FUND_SNAPSHOT_SH;
        case AMD_FUND_EXECUTION:
            return AMD_FUND_EXECUTION_SH;
        case AMD_FUND_ORDER:
            return AMD_FUND_ORDER_SH;
        case AMD_BOND_SNAPSHOT:
            return AMD_BOND_SNAPSHOT_SH;
        case AMD_BOND_EXECUTION:
            return AMD_BOND_EXECUTION_SH;
        case AMD_BOND_ORDER:
            return AMD_BOND_ORDER_SH;

        case AMD_ORDER_EXECUTION:
            return AMD_ORDER_EXECUTION_SH;
        case AMD_FUND_ORDER_EXECUTION:
            return AMD_FUND_ORDER_EXECUTION_SH;
        case AMD_BOND_ORDER_EXECUTION:
            return AMD_BOND_ORDER_EXECUTION_SH;

        default:
            break;
        }
        break;
    case 102:
        switch (dataType)
        {
        case AMD_SNAPSHOT:
            return AMD_SNAPSHOT_SZ;
        case AMD_EXECUTION:
            return AMD_EXECUTION_SZ;
        case AMD_ORDER:
            return AMD_ORDER_SZ;
        case AMD_INDEX:
            return AMD_INDEX_SZ;
        case AMD_ORDER_QUEUE:
            return AMD_ORDER_QUEUE_SZ;
        case AMD_FUND_SNAPSHOT:
            return AMD_FUND_SNAPSHOT_SZ;
        case AMD_FUND_EXECUTION:
            return AMD_FUND_EXECUTION_SZ;
        case AMD_FUND_ORDER:
            return AMD_FUND_ORDER_SZ;
        case AMD_BOND_SNAPSHOT:
            return AMD_BOND_SNAPSHOT_SZ;
        case AMD_BOND_EXECUTION:
            return AMD_BOND_EXECUTION_SZ;
        case AMD_BOND_ORDER:
            return AMD_BOND_ORDER_SZ;
        case AMD_ORDER_EXECUTION:
            return AMD_ORDER_EXECUTION_SZ;
        case AMD_FUND_ORDER_EXECUTION:
            return AMD_FUND_ORDER_EXECUTION_SZ;
        case AMD_BOND_ORDER_EXECUTION:
            return AMD_BOND_ORDER_EXECUTION_SZ;
        default:
            break;
        }
        break;
    }
    throw RuntimeException("[PLUGIN::AMDQUOTE] If the market type is " + std::to_string(market) + ", and amdDataType is  " + std::to_string(dataType) + ", getAmdTableType cannot be executed.");
    return AMD_ERROR_TABLE_TYPE;
}

