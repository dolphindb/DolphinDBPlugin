#include "amdQuoteImp.h"
#include "Concurrent.h"
#include "amdSpiImp.h"

using namespace dolphindb;

extern bool receivedTimeFlag;
extern bool dailyIndexFlag;
extern bool stopTest;
extern unordered_map<string, AMDDataType> nameType;


#ifdef ENUM_OR_STRING
#undef ENUM_OR_STRING
#endif
#define ENUM_OR_STRING(x) #x


AMDTableType getAmdTableType(AMDDataType dataType, int market);

AmdQuote::AmdQuote(std::string username, std::string password, std::vector<std::string> ips, std::vector<int> ports, SessionSP session) : username_(username), 
    password_(password), ips_(ips), ports_(ports) {
    if (ips.size() != ports.size()) {
        throw RuntimeException("connect to Amd Failed, ips num not equal to ports");
    }

    // 选择TCP传输数据方式
    cfg_.channel_mode = amd::ama::ChannelMode::kTCP;
    // 不使用压缩
    cfg_.tcp_compress_mode = 0;
    // 选择HA模式, 相关参数参考华锐相关文档
    cfg_.ha_mode = amd::ama::HighAvailableMode::kMasterSlaveA;
    // 配置日志级别
    cfg_.min_log_level = amd::ama::LogLevel::kInfo;
    // 是否输出监控数据
    cfg_.is_output_mon_data = false;
    // 逐笔保序开关，先设置为关，后续根据客户反馈可以动态配置
    cfg_.keep_order = false;
    cfg_.keep_order_timeout_ms = 3000; 
    // 设置默认订阅， 需要用户手动订阅
    cfg_.is_subscribe_full = false;
    // 配置行情服务用户名密码及服务器地址
    strcpy(cfg_.username, username.c_str());
    strcpy(cfg_.password, password.c_str());
    cfg_.ums_server_cnt = ips.size();
    for (unsigned int i = 0; i < cfg_.ums_server_cnt; i++) {
        strcpy(cfg_.ums_servers[i].local_ip, "0.0.0.0");
        strcpy(cfg_.ums_servers[i].server_ip, ips[i].c_str());
        cfg_.ums_servers[i].server_port = ports[i]; 
    }

    // 参数含义参考华锐文档
    cfg_.is_thread_safe = false;

    amdSpi_ = new AMDSpiImp(session);
    if (amd::ama::IAMDApi::Init(amdSpi_, cfg_) != amd::ama::ErrorCode::kSuccess) {
        amd::ama::IAMDApi::Release();
        throw RuntimeException("Init AMA failed");
    }
    stopTest = false;
}

/*
按品种类型订阅信息设置:
1. 订阅信息分三个维度 market:市场, data_type:证券数据类型, category_type:品种类型, security_code:证券代码
2. 订阅操作有三种:
    kSet 设置订阅, 以市场为单位覆盖订阅信息
    kAdd 增加订阅, 在前一个基础上增加订阅信息
    kDel 删除订阅, 在前一个基础上删除订阅信息
    kCancelAll 取消所有订阅信息
*/


// subscribe
void AmdQuote::subscribeOrderExecution(string                     orderExecutionType,
                                int                        market,
                                DictionarySP               dict,
                                std::vector<string>        codeList,
                                FunctionDefSP              transform,
                                bool                       receivedTimeFlag,
                                long long                  dailyStartTimestamp) 
{
    if(!receivedTimeFlag || !dailyIndexFlag) {
        throw RuntimeException("Subscribe type " + orderExecutionType + " must open ReceivedTime and DailyIndex option.");
    }
    uint64_t categoryType = 0;
    categoryType = amd::ama::SubscribeCategoryType::kStock;
    std::vector<std::string> nullCodeList;
    // unsubscribe(orderExecutionType, market, nullCodeList);
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

    if (amd::ama::IAMDApi::SubscribeData(
            amd::ama::SubscribeType::kAdd, sub, codeSize)
    != amd::ama::ErrorCode::kSuccess)
    {
        LOG_ERR("subscribe " + orderExecutionType + " err, market:", market);
    }
    Info info(orderExecutionType, market);
    infoSet_.insert(info);
}

// 订阅快照数据
void AmdQuote::subscribeSnapshot(int market, TableSP table, std::vector<std::string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp) {
    std::vector<std::string> nullCodeList;
    unsubscribe("snapshot", market, nullCodeList);
    amdSpi_->setSnapshotData(table, transform, receivedTimeFlag, dailyStartTimestamp, market);

    // 选择某个市场，如果codeList size为0 代表订阅全市场， 多个市场需要调用多次
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
    if (amd::ama::IAMDApi::SubscribeData(
            amd::ama::SubscribeType::kAdd, sub, codeSize)
    != amd::ama::ErrorCode::kSuccess)
    {
        LOG_ERR("subscribe Snapshot err, market:", market);
    }
    Info info("snapshot", market);
    infoSet_.insert(info);
}
// 订阅逐笔成交
void AmdQuote::subscribeExecution(int market, TableSP table, std::vector<std::string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp) {
    std::vector<std::string> nullCodeList;
    unsubscribe("execution", market, nullCodeList);
    amdSpi_->setExecutionData(table, transform, receivedTimeFlag, dailyStartTimestamp, market);
    // 选择某个市场，如果codeList size为0 代表订阅全市场，多个市场需要调用多次
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

    if (amd::ama::IAMDApi::SubscribeData(
            amd::ama::SubscribeType::kAdd, sub, codeSize)
    != amd::ama::ErrorCode::kSuccess)
    {
        LOG_ERR("subscribe Execution err, market:", market);
    }
    Info info("execution", market);
    infoSet_.insert(info);
}

// 订阅逐笔委托
void AmdQuote::subscribeOrder(int market, TableSP table, std::vector<std::string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp) {
    std::vector<std::string> nullCodeList;
    unsubscribe("order", market, nullCodeList);
    amdSpi_->setOrderData(table, transform, receivedTimeFlag, dailyStartTimestamp, market);
    // 选择某个市场，如果codeList size为0 代表订阅全市场， 多个市场需要调用多次
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

    if (amd::ama::IAMDApi::SubscribeData(
            amd::ama::SubscribeType::kAdd, sub, codeSize)
    != amd::ama::ErrorCode::kSuccess)
    {
        LOG_ERR("subscribe Order err, market:", market);
    }
    Info info("order", market);
    infoSet_.insert(info);
}

// 订阅基金快照数据
void AmdQuote::subscribeFundSnapshot(int market, TableSP table, std::vector<std::string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp) {
    std::vector<std::string> nullCodeList;
    unsubscribe("fundSnapshot", market, nullCodeList);
    amdSpi_->setFundSnapshotData(table, transform, receivedTimeFlag, dailyStartTimestamp, market);
    // 选择某个市场，如果codeList size为0 代表订阅全市场， 多个市场需要调用多次
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
    if (amd::ama::IAMDApi::SubscribeData(
            amd::ama::SubscribeType::kAdd, sub, codeSize)
    != amd::ama::ErrorCode::kSuccess)
    {
        LOG_ERR("subscribe fundSnapshot err, market:", market);
    }
    Info info("fundSnapshot", market);
    infoSet_.insert(info);
}

// 订阅基金逐笔成交
void AmdQuote::subscribeFundExecution(int market, TableSP table, std::vector<std::string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp) {
    std::vector<std::string> nullCodeList;
    unsubscribe("fundExecution", market, nullCodeList);
    amdSpi_->setFundExecutionData(table, transform, receivedTimeFlag, dailyStartTimestamp, market);
    // 选择某个市场，如果codeList size为0 代表订阅全市场，多个市场需要调用多次
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

    if (amd::ama::IAMDApi::SubscribeData(
            amd::ama::SubscribeType::kAdd, sub, codeSize)
    != amd::ama::ErrorCode::kSuccess)
    {
        LOG_ERR("subscribe fundExecution err, market:", market);
    }
    Info info("fundExecution", market);
    infoSet_.insert(info);
}

AmdQuote::~AmdQuote() {
    stopTest = true;
    std::vector<std::string> codeList;
    unsubscribe("all", 0, codeList);
    amd::ama::IAMDApi::Release();
    delete amdSpi_;
}

// 订阅基金逐笔委托
void AmdQuote::subscribeFundOrder(int market, TableSP table, std::vector<std::string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp) {
    std::vector<std::string> nullCodeList;
    unsubscribe("fundOrder", market, nullCodeList);
    amdSpi_->setFundOrderData(table, transform, receivedTimeFlag, dailyStartTimestamp, market);
    // 选择某个市场，如果codeList size为0 代表订阅全市场， 多个市场需要调用多次
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

    if (amd::ama::IAMDApi::SubscribeData(
            amd::ama::SubscribeType::kAdd, sub, codeSize)
    != amd::ama::ErrorCode::kSuccess)
    {
        LOG_ERR("subscribe fundOrder err, market:", market);
    }
    Info info("fundOrder", market);
    infoSet_.insert(info);
}

// 订阅债券快照数据
void AmdQuote::subscribeBondSnapshot(int market, TableSP table, std::vector<std::string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp) {
    std::vector<std::string> nullCodeList;
    unsubscribe("bondSnapshot", market, nullCodeList);
    amdSpi_->setBondSnapshotData(table, transform, receivedTimeFlag, dailyStartTimestamp, market);
    // 选择某个市场，如果codeList size为0 代表订阅全市场， 多个市场需要调用多次
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
    if (amd::ama::IAMDApi::SubscribeData(
            amd::ama::SubscribeType::kAdd, sub, codeSize)
    != amd::ama::ErrorCode::kSuccess)
    {
        LOG_ERR("subscribe bondSnapshot err, market:", market);
    }
    Info info("bondSnapshot", market);
    infoSet_.insert(info);
}

// 订阅债券逐笔成交
void AmdQuote::subscribeBondExecution(int market, TableSP table, std::vector<std::string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp) {
    std::vector<std::string> nullCodeList;
    unsubscribe("bondExecution", market, nullCodeList);
    amdSpi_->setBondExecutionData(table, transform, receivedTimeFlag, dailyStartTimestamp, market);
    // 选择某个市场，如果codeList size为0 代表订阅全市场，多个市场需要调用多次
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

    if (amd::ama::IAMDApi::SubscribeData(
            amd::ama::SubscribeType::kAdd, sub, codeSize)
    != amd::ama::ErrorCode::kSuccess)
    {
        LOG_ERR("subscribe bondExecution err, market:", market);
    }
    Info info("bondExecution", market);
    infoSet_.insert(info);
}

// 订阅债券逐笔委托
void AmdQuote::subscribeBondOrder(int market, TableSP table, std::vector<std::string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp) {
    std::vector<std::string> nullCodeList;
    unsubscribe("bondOrder", market, nullCodeList);
    amdSpi_->setBondOrderData(table, transform, receivedTimeFlag, dailyStartTimestamp, market);
    // 选择某个市场，如果codeList size为0 代表订阅全市场， 多个市场需要调用多次
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

    if (amd::ama::IAMDApi::SubscribeData(
            amd::ama::SubscribeType::kAdd, sub, codeSize)
    != amd::ama::ErrorCode::kSuccess)
    {
        LOG_ERR("subscribe bondOrder err, market:", market);
    }
    Info info("bondOrder", market);
    infoSet_.insert(info);
}

// 订阅指数快照数据
void AmdQuote::subscribeIndex(int market, TableSP table, std::vector<std::string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp) {
    std::vector<std::string> nullCodeList;
    unsubscribe("index", market, nullCodeList);
    amdSpi_->setIndexData(table, transform, receivedTimeFlag, dailyStartTimestamp, market);
    // 选择某个市场，如果codeList size为0 代表订阅全市场， 多个市场需要调用多次
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

    if (amd::ama::IAMDApi::SubscribeData(
            amd::ama::SubscribeType::kAdd, sub, codeSize)
    != amd::ama::ErrorCode::kSuccess)
    {
        LOG_ERR("subscribe Index err, market:", market);
    }
    Info info("index", market);
    infoSet_.insert(info);
}

void AmdQuote::enableLatencyStatistics(bool flag) {
    amdSpi_->setLatencyFlag(flag);
}

// 订阅委托队列所有类型数据
void AmdQuote::subscribeOrderQueue(int market, TableSP table, std::vector<std::string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp) {
    std::vector<std::string> nullCodeList;
    unsubscribe("orderQueue", market, nullCodeList);
    amdSpi_->setOrderQueueData(table, transform, receivedTimeFlag, dailyStartTimestamp, market);
    // 选择某个市场，如果codeList size为0 代表订阅全市场， 多个市场需要调用多次
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

    if (amd::ama::IAMDApi::SubscribeData(
            amd::ama::SubscribeType::kAdd, sub, codeSize)
    != amd::ama::ErrorCode::kSuccess)
    {
        LOG_ERR("subscribe OrderQueue err, market:", market);
    }
    Info info("orderQueue", market);
    infoSet_.insert(info);
}

void AmdQuote::unsubscribe(std::string dataType, int market, std::vector<std::string> codeList) {
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
        if (amd::ama::IAMDApi::SubscribeData(amd::ama::SubscribeType::kDel, sub, codeSize) != amd::ama::ErrorCode::kSuccess)
        {
            LOG_ERR("unsubscribe orderExecution err, market:", market);
            return;
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
        if (amd::ama::IAMDApi::SubscribeData(amd::ama::SubscribeType::kDel, sub, codeSize) != amd::ama::ErrorCode::kSuccess)
        {
            LOG_ERR("unsubscribe fundOrderExecution err, market:", market);
            return;
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
        if (amd::ama::IAMDApi::SubscribeData(amd::ama::SubscribeType::kDel, sub, codeSize) != amd::ama::ErrorCode::kSuccess)
        {
            LOG_ERR("unsubscribe order bondOrderExecution, market:", market);
            return;
        }
        amdSpi_->clearOrderExecution(dataType);
    }
    // 取消订阅
    else if (dataType == "snapshot") {
        // 取消快照订阅
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

        if (amd::ama::IAMDApi::SubscribeData(
            amd::ama::SubscribeType::kDel, sub, codeSize)
        != amd::ama::ErrorCode::kSuccess)
        {
            LOG_ERR("subscribe Snapshot err, market:", market);
            return;
        }
    } else if (dataType == "execution") {
        // 取消逐笔成交订阅
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
        if (amd::ama::IAMDApi::SubscribeData(
            amd::ama::SubscribeType::kDel, sub, codeSize)
        != amd::ama::ErrorCode::kSuccess)
        {
            LOG_ERR("subscribe ticker Execution err, market:", market);
        }
    } else if (dataType == "order") {
        // 取消逐笔委托订阅
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
        if (amd::ama::IAMDApi::SubscribeData(
            amd::ama::SubscribeType::kDel, sub, codeSize)
        != amd::ama::ErrorCode::kSuccess)
        {
            LOG_ERR("unsubscribe order err, market:", market);
            return;
        }
    } else if (dataType == "index") {
        // 取消指数快照订阅
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
        if (amd::ama::IAMDApi::SubscribeData(
            amd::ama::SubscribeType::kDel, sub, codeSize)
        != amd::ama::ErrorCode::kSuccess)
        {
            LOG_ERR("unsubscribe order err, market:", market);
            return;
        }
    } else if (dataType == "orderQueue") {
        // 取消委托队列订阅
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
        if (amd::ama::IAMDApi::SubscribeData(
            amd::ama::SubscribeType::kDel, sub, codeSize)
        != amd::ama::ErrorCode::kSuccess)
        {
            LOG_ERR("unsubscribe order err, market:", market);
            return;
        }
    } else if (dataType == "fundSnapshot") {
        // 取消快照订阅
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

        if (amd::ama::IAMDApi::SubscribeData(
            amd::ama::SubscribeType::kDel, sub, codeSize)
        != amd::ama::ErrorCode::kSuccess)
        {
            LOG_ERR("unsubscribe fundSnapshot err, market:", market);
            return;
        }
    } else if (dataType == "fundExecution") {
        // 取消逐笔成交订阅
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
        if (amd::ama::IAMDApi::SubscribeData(
            amd::ama::SubscribeType::kDel, sub, codeSize)
        != amd::ama::ErrorCode::kSuccess)
        {
            LOG_ERR("subscribe fundExecution err, market:", market);
        }
    } else if (dataType == "fundOrder") {
        // 取消逐笔委托订阅
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
        if (amd::ama::IAMDApi::SubscribeData(
            amd::ama::SubscribeType::kDel, sub, codeSize)
        != amd::ama::ErrorCode::kSuccess)
        {
            LOG_ERR("unsubscribe fundOrder err, market:", market);
            return;
        }
    } else if (dataType == "bondSnapshot") {
        // 取消快照订阅
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

        if (amd::ama::IAMDApi::SubscribeData(
            amd::ama::SubscribeType::kDel, sub, codeSize)
        != amd::ama::ErrorCode::kSuccess)
        {
            LOG_ERR("unsubscribe bondSnapshot err, market:", market);
            return;
        }
    } else if (dataType == "bondExecution") {
        // 取消逐笔成交订阅
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
        if (amd::ama::IAMDApi::SubscribeData(
            amd::ama::SubscribeType::kDel, sub, codeSize)
        != amd::ama::ErrorCode::kSuccess)
        {
            LOG_ERR("subscribe bondExecution err, market:", market);
        }
    } else if (dataType == "bondOrder") {
        // 取消逐笔委托订阅
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
        if (amd::ama::IAMDApi::SubscribeData(
            amd::ama::SubscribeType::kDel, sub, codeSize)
        != amd::ama::ErrorCode::kSuccess)
        {
            LOG_ERR("unsubscribe bondOrder err, market:", market);
            return;
        }
    } else if (dataType == "all") {
        amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[1];
        Defer df([=](){delete[] sub;});
        memset(sub, 0, sizeof(amd::ama::SubscribeCategoryItem));
        if (amd::ama::IAMDApi::SubscribeData(
            amd::ama::SubscribeType::kCancelAll, sub, 1)
        != amd::ama::ErrorCode::kSuccess)
        {
            LOG_ERR("unsubscribe all err");
            return;
        }
        amdSpi_->clearOrderExecution("orderExecution");
        amdSpi_->clearOrderExecution("fundOrderExecution");
        amdSpi_->clearOrderExecution("bondOrderExecution");
    } else {
        LOG_ERR("AMD Quote unsubscribe, illegal dataType:", dataType);
        throw RuntimeException("AMD Quote unsubscribe, illegal dataType");
        return;
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
    throw RuntimeException("If the market type is " + std::to_string(market) + ", and amdDataType is  " + std::to_string(dataType) + ", getAmdTableType cannot be executed.");
    return AMD_ERROR_TABLE_TYPE;
}

