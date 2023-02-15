#ifndef __AMD_QUOTE_H
#define __AMD_QUOTE_H

#include <mutex>
#include <iostream>

#include "ama.h"
#include "ama_tools.h"

#include "CoreConcept.h"
#include "amdQuoteType.h"
#include "Logger.h"
#include "Util.h"
#include "ScalarImp.h"
#include "Plugin.h"

using namespace dolphindb;

extern bool receivedTimeFlag;
extern bool dailyIndexFlag;

enum AMDDataType{
    AMD_SNAPSHOT, 
    AMD_EXECUTION,
    AMD_ORDER,
    AMD_INDEX, 
    AMD_ORDER_QUEUE,
    AMD_FUND_SNAPSHOT,
    AMD_FUND_EXECUTION,
    AMD_FUND_ORDER,
    AMD_BOND_SNAPSHOT,
    AMD_BOND_EXECUTION,
    AMD_BOND_ORDER,
    AMD_ERROR_DATA_TYPE
};

enum AMDTableType{
    AMD_SNAPSHOT_SH, 
    AMD_EXECUTION_SH,
    AMD_ORDER_SH,
    AMD_INDEX_SH, 
    AMD_ORDER_QUEUE_SH,
    AMD_FUND_SNAPSHOT_SH,
    AMD_FUND_EXECUTION_SH,
    AMD_FUND_ORDER_SH,
    AMD_BOND_SNAPSHOT_SH,
    AMD_BOND_EXECUTION_SH,
    AMD_BOND_ORDER_SH,

    AMD_SNAPSHOT_SZ, 
    AMD_EXECUTION_SZ,
    AMD_ORDER_SZ,
    AMD_INDEX_SZ, 
    AMD_ORDER_QUEUE_SZ,
    AMD_FUND_SNAPSHOT_SZ,
    AMD_FUND_EXECUTION_SZ,
    AMD_FUND_ORDER_SZ,
    AMD_BOND_SNAPSHOT_SZ,
    AMD_BOND_EXECUTION_SZ,
    AMD_BOND_ORDER_SZ,
    AMD_ERROR_TABLE_TYPE
};
extern unordered_map<string, AMDDataType> nameType;


template<class T>
struct ObjectSizer {
    inline int operator()(const T& obj){
        return 1;
    }
};

template<class T>
struct ObjectUrgency {
    inline bool operator()(const T& obj){
        return false;
    }
};

typedef GenericBoundedQueue<amd::ama::MDSnapshot, ObjectSizer<amd::ama::MDSnapshot>, ObjectUrgency<amd::ama::MDSnapshot>> SnapshotQueue;
typedef GenericBoundedQueue<amd::ama::MDTickOrder, ObjectSizer<amd::ama::MDTickOrder>, ObjectUrgency<amd::ama::MDTickOrder>> OrderQueue;
typedef GenericBoundedQueue<amd::ama::MDTickExecution, ObjectSizer<amd::ama::MDTickExecution>, ObjectUrgency<amd::ama::MDTickExecution>> ExecutionQueue;
typedef GenericBoundedQueue<amd::ama::MDBondSnapshot, ObjectSizer<amd::ama::MDBondSnapshot>, ObjectUrgency<amd::ama::MDBondSnapshot>> BondSnapshotQueue;
typedef GenericBoundedQueue<amd::ama::MDBondTickOrder, ObjectSizer<amd::ama::MDBondTickOrder>, ObjectUrgency<amd::ama::MDBondTickOrder>> BondOrderQueue;
typedef GenericBoundedQueue<amd::ama::MDBondTickExecution, ObjectSizer<amd::ama::MDBondTickExecution>, ObjectUrgency<amd::ama::MDBondTickExecution>> BondExecutionQueue;
typedef GenericBoundedQueue<amd::ama::MDIndexSnapshot, ObjectSizer<amd::ama::MDIndexSnapshot>, ObjectUrgency<amd::ama::MDIndexSnapshot>> IndexQueue;
typedef GenericBoundedQueue<amd::ama::MDOrderQueue, ObjectSizer<amd::ama::MDOrderQueue>, ObjectUrgency<amd::ama::MDOrderQueue>> OrderQueueQueue;

template <class ITEMTYPE>
void blockHandling(SmartPointer<GenericBoundedQueue<ITEMTYPE, ObjectSizer<ITEMTYPE>, ObjectUrgency<ITEMTYPE>>> queue, std::function<void(vector<ITEMTYPE> &)> dealFunc, SmartPointer<bool> stopFlag, string msgPrefix, string threadInfo)
{
    LOG_INFO(msgPrefix, " ", threadInfo, " bg thread start ");
    bool ret;
    while (!*stopFlag)
    {
        try
        {
            ITEMTYPE item;
            ret = queue->blockingPop(item, 100);
            if (!ret)
            {
                if (*stopFlag)
                    break;
                else
                {
                    LOG(msgPrefix, " ", threadInfo, " bg thread pop size 0");
                    continue;
                }
            }

            auto size = std::max(queue->size(), (long long)65535);
            vector<ITEMTYPE> items;
            items.reserve(size + 1);
            items.push_back(std::move(item));
            if (size > 0)
                queue->pop(items, size);

            size = items.size();
            LOG(msgPrefix, " ", threadInfo, " bg thread pop size ", size);
            if (queue->size() < 100)
                Util::sleep(3);

            dealFunc(items);
        }
        catch (exception &e)
        {
            LOG_ERR(msgPrefix, " ", threadInfo, " Failed to process data of size 5000 because ", e.what());
        }
    }
    LOG_INFO(msgPrefix, " ", threadInfo, " bg thread end ");
};

class Defer {
public:
    Defer(std::function<void()> code): code(code) {}
    ~Defer() { code(); }
private:
    std::function<void()> code;
};

inline long long countTemporalUnit(int days, long long multiplier, long long remainder){
	return days == INT_MIN ? LLONG_MIN : days * multiplier + remainder;
}

long long convertTime(long long time);

int countdays(int amdDays);

class Info {
public:
    Info(string datatype, int marketType) : datatype_(datatype), marketType_(marketType) {}
    string datatype_;
    int marketType_;
    // vector<string> codeList;
};

class InfoHash {
public:
    size_t operator() (const Info& info) const {
        return std::hash<string>{}(info.datatype_) ^
               std::hash<int>{}(info.marketType_);
    }
};

class InfoEqual {
public:
    bool operator() (const Info& info1, const Info& info2) const {
        return info1.marketType_ == info2.marketType_ &&
               info1.datatype_ == info2.datatype_;
    }
};

class DailyIndex{
public:
    DailyIndex(): startTimestamp_(LONG_LONG_MIN), flag_(false){}
    DailyIndex(long long startTimestamp){
        if(startTimestamp != LONG_LONG_MIN){
            startTimestamp_ = startTimestamp;
            flag_ = true;
        }else{
            flag_ = false;
        }
    }
    int getIndex(int32_t param, long long timestamp){
        if(startTimestamp_ == LONG_LONG_MIN)
            throw RuntimeException("getIndex failed because DailyIndex was not set");
        const long long dateTimestamp = 24 * 60 * 60 * 1000;
        long long originBase = startTimestamp_ / dateTimestamp;
        long long newBase = timestamp / dateTimestamp;
        if(originBase < newBase){
            startTimestamp_ = newBase * dateTimestamp;
            LOG_INFO("[PluginAmdQuote]: The new DailyIndex with channel_no as " + std::to_string(param) + " will start at " + std::to_string(startTimestamp_));
            indexMap_.clear();
        }
        if(timestamp < startTimestamp_){
            return INT_MIN;
        }
        if(indexMap_.count(param) != 0){
            return ++indexMap_[param];
        }else{
            indexMap_[param] = 0;
            return 0;
        }   
    }
    long long getStartTimestamp(){
        return startTimestamp_;
    }

public:
    bool getFlag(){
        return flag_;
    }
private:
    unordered_map<int32_t, int> indexMap_;
    long long startTimestamp_;
    bool flag_;
};

#ifdef ENUM_OR_STRING
#undef ENUM_OR_STRING
#endif
#define ENUM_OR_STRING(x) #x


AMDTableType getAmdTableType(AMDDataType dataType, int market);

bool getDailyIndex(int& ret, DailyIndex* dailyIndexArray, int dailyIndexArraySize, int market, AMDDataType datatype, int32_t channel_no, long long timestamp);

class AmdQuote {
private: 
    AmdQuote(std::string username, std::string password, std::vector<std::string> ips, std::vector<int> ports, SessionSP session) : username_(username), 
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
            std::lock_guard<std::mutex> amdLock_(amdMutex_);
            amd::ama::IAMDApi::Release();
            throw RuntimeException("Init AMA failed");
        }
    }
public:
    static AmdQuote* getInstance(std::string username, std::string password, std::vector<std::string> ips, std::vector<int> ports, SessionSP session) {
        if (instance_ == nullptr) {
            instance_ = new AmdQuote(username, password, ips, ports, session);
        }

        return instance_;
    }

    static AmdQuote* getInstance() {
        return instance_;
    }

    static void deleteInstance() {
        if (instance_ != nullptr) {
            delete instance_;
            instance_ = nullptr;
        }
    }

    void enableLatencyStatistics(bool flag) {
        amdSpi_->setLatencyFlag(flag);
    }

    TableSP getStatus() {
        INDEX size = (INDEX)infoSet_.size();

        vector<DATA_TYPE> types{DT_STRING, DT_INT};
        vector<string> names{"dataType", "marketType"};
        std::vector<ConstantSP> cols;
        for (size_t i = 0; i < types.size(); i++) {
            cols.push_back(Util::createVector(types[i], size, size));
        }
        
        int i = 0;
        for (auto& info : infoSet_) {
            cols[0]->set(i, new String(info.datatype_));
            cols[1]->set(i, new Int(info.marketType_));
            i++;
        }
        // if(dailyIndexFlag){
        //     cols.push_back(Util::createVector(DT_TIMESTAMP, size, size));
        //     int i = 0;
        //     for (auto& info : infoSet_) {
        //         if(info.marketType_ == 101 || info.marketType_ == 102)
        //             cols.back()->set(i, new Timestamp(amdSpi_->getDailyIndexStarTime(info.datatype_, info.marketType_)));
        //         else
        //             cols.back()->set(i, new Timestamp(LONG_LONG_MIN));
        //         ++i;
        //     }
        //     names.push_back("dailyIndex");
        // }
        return Util::createTable(names, cols);
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

    // 订阅快照数据
    void subscribeSnapshot(int market, TableSP table, std::vector<std::string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp) {
        std::vector<std::string> nullCodeList;
        unsubscribe("snapshot", market, nullCodeList);
        amdSpi_->setSnapshotData(table, transform, receivedTimeFlag, dailyStartTimestamp, market);

        // 选择某个市场，如果codeList size为0 代表订阅全市场， 多个市场需要调用多次
        unsigned int codeSize = 1;
        if (codeList.size() > 0) {
            codeSize = codeList.size();
        }
        
        amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
        Defer df([=](){delete sub;});
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
            std::lock_guard<std::mutex> amdLock_(amdMutex_);
            LOG_ERR("subscribe Snapshot err, market:", market);
        }
        Info info("snapshot", market);
        infoSet_.insert(info);
    }

    // 订阅逐笔成交
    void subscribeExecution(int market, TableSP table, std::vector<std::string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp) {
        std::vector<std::string> nullCodeList;
        unsubscribe("execution", market, nullCodeList);
        amdSpi_->setExecutionData(table, transform, receivedTimeFlag, dailyStartTimestamp, market);
        // 选择某个市场，如果codeList size为0 代表订阅全市场，多个市场需要调用多次
        unsigned int codeSize = 1;
        if (codeList.size() > 0) {
            codeSize = codeList.size();
        }

        amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
        Defer df([=](){delete sub;});
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
            std::lock_guard<std::mutex> amdLock_(amdMutex_);
            LOG_ERR("subscribe Execution err, market:", market);
        }
        Info info("execution", market);
        infoSet_.insert(info);
    }

    // 订阅逐笔委托
    void subscribeOrder(int market, TableSP table, std::vector<std::string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp) {
        std::vector<std::string> nullCodeList;
        unsubscribe("order", market, nullCodeList);
        amdSpi_->setOrderData(table, transform, receivedTimeFlag, dailyStartTimestamp, market);
        // 选择某个市场，如果codeList size为0 代表订阅全市场， 多个市场需要调用多次
        unsigned int codeSize = 1;
        if (codeList.size() > 0) {
            codeSize = codeList.size();
        }

        amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
        Defer df([=](){delete sub;});
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
            std::lock_guard<std::mutex> amdLock_(amdMutex_);
            LOG_ERR("subscribe Order err, market:", market);
        }
        Info info("order", market);
        infoSet_.insert(info);
    }

    // 订阅基金快照数据
    void subscribeFundSnapshot(int market, TableSP table, std::vector<std::string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp) {
        std::vector<std::string> nullCodeList;
        unsubscribe("fundSnapshot", market, nullCodeList);
        amdSpi_->setFundSnapshotData(table, transform, receivedTimeFlag, dailyStartTimestamp, market);
        // 选择某个市场，如果codeList size为0 代表订阅全市场， 多个市场需要调用多次
        unsigned int codeSize = 1;
        if (codeList.size() > 0) {
            codeSize = codeList.size();
        }

        amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
        Defer df([=](){delete sub;});
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
            std::lock_guard<std::mutex> amdLock_(amdMutex_);
            LOG_ERR("subscribe fundSnapshot err, market:", market);
        }
        Info info("fundSnapshot", market);
        infoSet_.insert(info);
    }

    // 订阅基金逐笔成交
    void subscribeFundExecution(int market, TableSP table, std::vector<std::string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp) {
        std::vector<std::string> nullCodeList;
        unsubscribe("fundExecution", market, nullCodeList);
        amdSpi_->setFundExecutionData(table, transform, receivedTimeFlag, dailyStartTimestamp, market);
        // 选择某个市场，如果codeList size为0 代表订阅全市场，多个市场需要调用多次
        unsigned int codeSize = 1;
        if (codeList.size() > 0) {
            codeSize = codeList.size();
        }

        amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
        Defer df([=](){delete sub;});
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
            std::lock_guard<std::mutex> amdLock_(amdMutex_);
            LOG_ERR("subscribe fundExecution err, market:", market);
        }
        Info info("fundExecution", market);
        infoSet_.insert(info);
    }

    // 订阅基金逐笔委托
    void subscribeFundOrder(int market, TableSP table, std::vector<std::string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp) {
        std::vector<std::string> nullCodeList;
        unsubscribe("fundOrder", market, nullCodeList);
        amdSpi_->setFundOrderData(table, transform, receivedTimeFlag, dailyStartTimestamp, market);
        // 选择某个市场，如果codeList size为0 代表订阅全市场， 多个市场需要调用多次
        unsigned int codeSize = 1;
        if (codeList.size() > 0) {
            codeSize = codeList.size();
        }

        amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
        Defer df([=](){delete sub;});
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
            std::lock_guard<std::mutex> amdLock_(amdMutex_);
            LOG_ERR("subscribe fundOrder err, market:", market);
        }
        Info info("fundOrder", market);
        infoSet_.insert(info);
    }

    // 订阅债券快照数据
    void subscribeBondSnapshot(int market, TableSP table, std::vector<std::string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp) {
        std::vector<std::string> nullCodeList;
        unsubscribe("bondSnapshot", market, nullCodeList);
        amdSpi_->setBondSnapshotData(table, transform, receivedTimeFlag, dailyStartTimestamp, market);
        // 选择某个市场，如果codeList size为0 代表订阅全市场， 多个市场需要调用多次
        unsigned int codeSize = 1;
        if (codeList.size() > 0) {
            codeSize = codeList.size();
        }

        amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
        Defer df([=](){delete sub;});
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
            std::lock_guard<std::mutex> amdLock_(amdMutex_);
            LOG_ERR("subscribe bondSnapshot err, market:", market);
        }
        Info info("bondSnapshot", market);
        infoSet_.insert(info);
    }

    // 订阅债券逐笔成交
    void subscribeBondExecution(int market, TableSP table, std::vector<std::string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp) {
        std::vector<std::string> nullCodeList;
        unsubscribe("bondExecution", market, nullCodeList);
        amdSpi_->setBondExecutionData(table, transform, receivedTimeFlag, dailyStartTimestamp, market);
        // 选择某个市场，如果codeList size为0 代表订阅全市场，多个市场需要调用多次
        unsigned int codeSize = 1;
        if (codeList.size() > 0) {
            codeSize = codeList.size();
        }

        amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
        Defer df([=](){delete sub;});
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
            std::lock_guard<std::mutex> amdLock_(amdMutex_);
            LOG_ERR("subscribe bondExecution err, market:", market);
        }
        Info info("bondExecution", market);
        infoSet_.insert(info);
    }

    // 订阅债券逐笔委托
    void subscribeBondOrder(int market, TableSP table, std::vector<std::string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp) {
        std::vector<std::string> nullCodeList;
        unsubscribe("bondOrder", market, nullCodeList);
        amdSpi_->setBondOrderData(table, transform, receivedTimeFlag, dailyStartTimestamp, market);
        // 选择某个市场，如果codeList size为0 代表订阅全市场， 多个市场需要调用多次
        unsigned int codeSize = 1;
        if (codeList.size() > 0) {
            codeSize = codeList.size();
        }

        amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
        Defer df([=](){delete sub;});
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
            std::lock_guard<std::mutex> amdLock_(amdMutex_);
            LOG_ERR("subscribe bondOrder err, market:", market);
        }
        Info info("bondOrder", market);
        infoSet_.insert(info);
    }

    // 订阅指数快照数据
    void subscribeIndex(int market, TableSP table, std::vector<std::string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp) {
        std::vector<std::string> nullCodeList;
        unsubscribe("index", market, nullCodeList);
        amdSpi_->setIndexData(table, transform, receivedTimeFlag, dailyStartTimestamp, market);
        // 选择某个市场，如果codeList size为0 代表订阅全市场， 多个市场需要调用多次
        unsigned int codeSize = 1;
        if (codeList.size() > 0) {
            codeSize = codeList.size();
        }

        amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
        Defer df([=](){delete sub;});
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
            std::lock_guard<std::mutex> amdLock_(amdMutex_);
            LOG_ERR("subscribe Index err, market:", market);
        }
        Info info("index", market);
        infoSet_.insert(info);
    }

    // 订阅委托队列所有类型数据
    void subscribeOrderQueue(int market, TableSP table, std::vector<std::string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp) {
        std::vector<std::string> nullCodeList;
        unsubscribe("orderQueue", market, nullCodeList);
        amdSpi_->setOrderQueueData(table, transform, receivedTimeFlag, dailyStartTimestamp, market);
        // 选择某个市场，如果codeList size为0 代表订阅全市场， 多个市场需要调用多次
        unsigned int codeSize = 1;
        if (codeList.size() > 0) {
            codeSize = codeList.size();
        }

        amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
        Defer df([=](){delete sub;});
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
            std::lock_guard<std::mutex> amdLock_(amdMutex_);
            LOG_ERR("subscribe OrderQueue err, market:", market);
        }
        Info info("orderQueue", market);
        infoSet_.insert(info);
    }

    void unsubscribe(std::string dataType, int market, std::vector<std::string> codeList) {
        // 取消订阅
        if (dataType == "snapshot") {
            // 取消快照订阅
            unsigned int codeSize = 1;
            if (codeList.size() > 0) {
                codeSize = codeList.size();
            }

            amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
            Defer df([=](){delete sub;});
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
                std::lock_guard<std::mutex> amdLock_(amdMutex_);
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
            Defer df([=](){delete sub;});
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
                std::lock_guard<std::mutex> amdLock_(amdMutex_);
                LOG_ERR("subscribe ticker Execution err, market:", market);
            }
        } else if (dataType == "order") {
            // 取消逐笔委托订阅
            unsigned int codeSize = 1;
            if (codeList.size() > 0) {
                codeSize = codeList.size();
            }

            amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
            Defer df([=](){delete sub;});
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
                std::lock_guard<std::mutex> amdLock_(amdMutex_);
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
            Defer df([=](){delete sub;});
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
                std::lock_guard<std::mutex> amdLock_(amdMutex_);
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
            Defer df([=](){delete sub;});
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
                std::lock_guard<std::mutex> amdLock_(amdMutex_);
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
            Defer df([=](){delete sub;});
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
                std::lock_guard<std::mutex> amdLock_(amdMutex_);
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
            Defer df([=](){delete sub;});
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
                std::lock_guard<std::mutex> amdLock_(amdMutex_);
                LOG_ERR("subscribe fundExecution err, market:", market);
            }
        } else if (dataType == "fundOrder") {
            // 取消逐笔委托订阅
            unsigned int codeSize = 1;
            if (codeList.size() > 0) {
                codeSize = codeList.size();
            }

            amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
            Defer df([=](){delete sub;});
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
                std::lock_guard<std::mutex> amdLock_(amdMutex_);
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
            Defer df([=](){delete sub;});
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
                std::lock_guard<std::mutex> amdLock_(amdMutex_);
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
            Defer df([=](){delete sub;});
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
                std::lock_guard<std::mutex> amdLock_(amdMutex_);
                LOG_ERR("subscribe bondExecution err, market:", market);
            }
        } else if (dataType == "bondOrder") {
            // 取消逐笔委托订阅
            unsigned int codeSize = 1;
            if (codeList.size() > 0) {
                codeSize = codeList.size();
            }

            amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[codeSize];
            Defer df([=](){delete sub;});
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
                std::lock_guard<std::mutex> amdLock_(amdMutex_);
                LOG_ERR("unsubscribe bondOrder err, market:", market);
                return;
            }
        } else if (dataType == "all") {
            amd::ama::SubscribeCategoryItem *sub = new amd::ama::SubscribeCategoryItem[1];
            Defer df([=](){delete sub;});
            memset(sub, 0, sizeof(amd::ama::SubscribeCategoryItem));
            if (amd::ama::IAMDApi::SubscribeData(
                amd::ama::SubscribeType::kCancelAll, sub, 1)
            != amd::ama::ErrorCode::kSuccess)
            {
                std::lock_guard<std::mutex> amdLock_(amdMutex_);
                LOG_ERR("unsubscribe all err");
                return;
            }
        } else {
            LOG_ERR("AMD Quote unsubscribe, illegal dataType:", dataType);
            throw RuntimeException("AMD Quote unsubscribe, illegal dataType");
            return;
        }

        if (dataType == "all") {
            infoSet_.clear();
        } else {
            Info info(dataType, market);
            infoSet_.erase(info);
        }
    }


    ~AmdQuote() {
        std::vector<std::string> codeList;
        unsubscribe("all", 0, codeList);
        amd::ama::IAMDApi::Release();
        delete amdSpi_;
    }

public:
    // 该类主要实现回调函数，回调函数用于接受并处理数据
    class AMDSpiImp : public amd::ama::IAMDSpi {
    public:
        AMDSpiImp(SessionSP session) :
        snapshotBoundQueue_(new SnapshotQueue(100000, ObjectSizer<amd::ama::MDSnapshot>(), ObjectUrgency<amd::ama::MDSnapshot>())),
        orderBoundQueue_(new OrderQueue(100000, ObjectSizer<amd::ama::MDTickOrder>(), ObjectUrgency<amd::ama::MDTickOrder>())),
        executionBoundQueue_(new ExecutionQueue(100000, ObjectSizer<amd::ama::MDTickExecution>(), ObjectUrgency<amd::ama::MDTickExecution>())),
        bondSnapshotBoundQueue_(new BondSnapshotQueue(100000, ObjectSizer<amd::ama::MDBondSnapshot>(), ObjectUrgency<amd::ama::MDBondSnapshot>())),
        bondOrderBoundQueue_(new BondOrderQueue(100000, ObjectSizer<amd::ama::MDBondTickOrder>(), ObjectUrgency<amd::ama::MDBondTickOrder>())),
        bondExecutionBoundQueue_(new BondExecutionQueue(100000, ObjectSizer<amd::ama::MDBondTickExecution>(), ObjectUrgency<amd::ama::MDBondTickExecution>())),
        indexBoundQueue_(new IndexQueue(100000, ObjectSizer<amd::ama::MDIndexSnapshot>(), ObjectUrgency<amd::ama::MDIndexSnapshot>())),
        orderQueueBoundQueue_(new OrderQueueQueue(100000, ObjectSizer<amd::ama::MDOrderQueue>(), ObjectUrgency<amd::ama::MDOrderQueue>())),
        stopFlag_(new bool()),
        session_(session)
         {
            *stopFlag_ = false;
            SmartPointer<Executor> snapshotExecutor = new Executor(std::bind(&blockHandling<amd::ama::MDSnapshot>, snapshotBoundQueue_, 
            [this](vector<amd::ama::MDSnapshot>& data){
                OnMDSnapshotHelper(data.data(), data.size());
            }, stopFlag_, "[PluginAmdQuote]:", "snapshot"
            ));
            snapshotThread_ = new Thread(snapshotExecutor);
            snapshotThread_->start();

            SmartPointer<Executor> orderExecutor = new Executor(std::bind(&blockHandling<amd::ama::MDTickOrder>, orderBoundQueue_, 
            [this](vector<amd::ama::MDTickOrder>& data){
                OnMDTickOrderHelper(data.data(), data.size());
            }, stopFlag_, "[PluginAmdQuote]:", "order"
            ));
            orderThread_ = new Thread(orderExecutor);
            orderThread_->start();

            SmartPointer<Executor> executionExecutor = new Executor(std::bind(&blockHandling<amd::ama::MDTickExecution>, executionBoundQueue_, 
            [this](vector<amd::ama::MDTickExecution>& data){
                OnMDTickExecutionHelper(data.data(), data.size());
            }, stopFlag_, "[PluginAmdQuote]:", "execution"
            ));
            executionThread_ = new Thread(executionExecutor);
            executionThread_->start();

            SmartPointer<Executor> bondSnapshotExecutor = new Executor(std::bind(&blockHandling<amd::ama::MDBondSnapshot>, bondSnapshotBoundQueue_, 
            [this](vector<amd::ama::MDBondSnapshot>& data){
                OnMDBondSnapshotHelper(data.data(), data.size());
            }, stopFlag_, "[PluginAmdQuote]:", "bondSnapshot"
            ));
            bondSnapshotThread_ = new Thread(bondSnapshotExecutor);
            bondSnapshotThread_->start();

            SmartPointer<Executor> bondOrderExecutor = new Executor(std::bind(&blockHandling<amd::ama::MDBondTickOrder>, bondOrderBoundQueue_, 
            [this](vector<amd::ama::MDBondTickOrder>& data){
                OnMDBondTickOrderHelper(data.data(), data.size());
            }, stopFlag_, "[PluginAmdQuote]:", "bondOrder"
            ));
            bondOrderThread_ = new Thread(bondOrderExecutor);
            bondOrderThread_->start();

            SmartPointer<Executor> bondExecutionExecutor = new Executor(std::bind(&blockHandling<amd::ama::MDBondTickExecution>, bondExecutionBoundQueue_, 
            [this](vector<amd::ama::MDBondTickExecution>& data){
                OnMDBondTickExecutionHelper(data.data(), data.size());
            }, stopFlag_, "[PluginAmdQuote]:", "bondExecution"
            ));
            bondExecutionThread_ = new Thread(bondExecutionExecutor);
            bondExecutionThread_->start();

            SmartPointer<Executor> indexExecutor = new Executor(std::bind(&blockHandling<amd::ama::MDIndexSnapshot>, indexBoundQueue_, 
            [this](vector<amd::ama::MDIndexSnapshot>& data){
                OnMDIndexSnapshotHelper(data.data(), data.size());
            }, stopFlag_, "[PluginAmdQuote]:", "index"
            ));
            indexThread_ = new Thread(indexExecutor);
            indexThread_->start();

            SmartPointer<Executor> orderQueueExecutor = new Executor(std::bind(&blockHandling<amd::ama::MDOrderQueue>, orderQueueBoundQueue_, 
            [this](vector<amd::ama::MDOrderQueue>& data){
                OnMDOrderQueueHelper(data.data(), data.size());
            }, stopFlag_, "[PluginAmdQuote]:", "orderQueue"
            ));
            orderQueueThread_ = new Thread(orderQueueExecutor);
            orderQueueThread_->start();
        }

        ~AMDSpiImp(){
            LOG_INFO("[pluginAmdQuote]: release AMDSpiImp");
            *stopFlag_ = true;
            snapshotThread_->join();
            orderThread_->join();
            executionThread_->join();
            bondSnapshotThread_->join();
            bondOrderThread_->join();
            bondExecutionThread_->join();
            indexThread_->join();
            orderQueueThread_->join();
        }

        // 接受并处理快照数据（包含基金和股票数据）
        void OnMDSnapshotHelper(amd::ama::MDSnapshot* snapshot, uint32_t cnt) {
            try{
                if(cnt == 0)return;
                long long startTime = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
                // std::lock_guard<std::mutex> amdLock_(amdMutex_);

                // 基金和股票数据插入到不同的表
                TableSP insertedTable;
                uint8_t varietyCategory = snapshot[0].variety_category;
                FunctionDefSP transform;

                AMDDataType datatype;
                if (varietyCategory == 1) { // 股票快照
                    insertedTable = snapshotData_;
                    transform = snapshotTransform_;
                    datatype = AMD_SNAPSHOT;
                } else if (varietyCategory == 2) { // 基金快照
                    insertedTable = fundSnapshotData_;
                    transform = fundSnapshotTransform_;
                    datatype = AMD_FUND_SNAPSHOT;
                } else {
                    return ;
                }

                int market_type = snapshot[0].market_type;
                AMDTableType tableTyeTotal = getAmdTableType(datatype, market_type);
                if(tableTyeTotal == AMD_ERROR_TABLE_TYPE){
                    LOG_ERR("[PluginAmdQuote]: error amd table type AMD_ERROR_TABLE_TYPE");
                    return;
                }

                // 检查是否需要时间戳列
                bool receivedTimeFlag = (varietyCategory == 1 && snapshotReceivedTimeFlag_) || (varietyCategory ==2 && fundSnapshotReceivedTimeFlag_);

                bool dailyIndexFlagTotal = dailyIndex_[tableTyeTotal].getFlag();

                DdbVector<int> col0(0, cnt);
                DdbVector<string> col1(0, cnt);
                DdbVector<long long> col2(0, cnt);
                DdbVector<string> col3(0, cnt);
                DdbVector<long long> col4(0, cnt);
                DdbVector<long long> col5(0, cnt);
                DdbVector<long long> col6(0, cnt);
                DdbVector<long long> col7(0, cnt);
                DdbVector<long long> col8(0, cnt);
                DdbVector<long long> col9(0, cnt);

                DdbVector<long long> col10(0, cnt);
                DdbVector<long long> col11(0, cnt);
                DdbVector<long long> col12(0, cnt);
                DdbVector<long long> col13(0, cnt);
                DdbVector<long long> col14(0, cnt);
                DdbVector<long long> col15(0, cnt);
                DdbVector<long long> col16(0, cnt);
                DdbVector<long long> col17(0, cnt);
                DdbVector<long long> col18(0, cnt);
                DdbVector<long long> col19(0, cnt);

                DdbVector<long long> col20(0, cnt);
                DdbVector<long long> col21(0, cnt);
                DdbVector<long long> col22(0, cnt);
                DdbVector<long long> col23(0, cnt);
                DdbVector<long long> col24(0, cnt);
                DdbVector<long long> col25(0, cnt);
                DdbVector<long long> col26(0, cnt);
                DdbVector<long long> col27(0, cnt);
                DdbVector<long long> col28(0, cnt);
                DdbVector<long long> col29(0, cnt);

                DdbVector<long long> col30(0, cnt);
                DdbVector<long long> col31(0, cnt);
                DdbVector<long long> col32(0, cnt);
                DdbVector<long long> col33(0, cnt);
                DdbVector<long long> col34(0, cnt);
                DdbVector<long long> col35(0, cnt);
                DdbVector<long long> col36(0, cnt);
                DdbVector<long long> col37(0, cnt);
                DdbVector<long long> col38(0, cnt);
                DdbVector<long long> col39(0, cnt);
                
                DdbVector<long long> col40(0, cnt);
                DdbVector<long long> col41(0, cnt);
                DdbVector<long long> col42(0, cnt);
                DdbVector<long long> col43(0, cnt);
                DdbVector<long long> col44(0, cnt);
                DdbVector<long long> col45(0, cnt);
                DdbVector<long long> col46(0, cnt);
                DdbVector<long long> col47(0, cnt);
                DdbVector<long long> col48(0, cnt);
                DdbVector<long long> col49(0, cnt);

                DdbVector<long long> col50(0, cnt);
                DdbVector<long long> col51(0, cnt);
                DdbVector<long long> col52(0, cnt);
                DdbVector<long long> col53(0, cnt);
                DdbVector<long long> col54(0, cnt);
                DdbVector<long long> col55(0, cnt);
                DdbVector<long long> col56(0, cnt);
                DdbVector<long long> col57(0, cnt);
                DdbVector<long long> col58(0, cnt);
                DdbVector<long long> col59(0, cnt);

                DdbVector<long long> col60(0, cnt);
                DdbVector<long long> col61(0, cnt);
                DdbVector<long long> col62(0, cnt);
                DdbVector<long long> col63(0, cnt);
                DdbVector<long long> col64(0, cnt);
                DdbVector<int> col65(0, cnt);
                DdbVector<string> col66(0, cnt);
                DdbVector<string> col67(0, cnt);
                DdbVector<long long> col68(0, cnt);
                DdbVector<long long> col69(0, cnt);

                DdbVector<long long> col70(0, cnt);
                DdbVector<long long> col71(0, cnt);
                DdbVector<long long> col72(0, cnt);
                DdbVector<long long> col73(0, cnt);
                DdbVector<long long> col74(0, cnt);
                DdbVector<long long> col75(0, cnt);
                DdbVector<long long> col76(0, cnt);
                DdbVector<long long> col77(0, cnt);
                DdbVector<long long> col78(0, cnt);
                DdbVector<long long> col79(0, cnt);

                DdbVector<long long> col80(0, cnt);
                DdbVector<long long> col81(0, cnt);
                DdbVector<long long> col82(0, cnt);
                DdbVector<long long> col83(0, cnt);
                DdbVector<long long> col84(0, cnt);
                DdbVector<long long> col85(0, cnt);
                DdbVector<long long> col86(0, cnt);
                DdbVector<long long> col87(0, cnt);
                DdbVector<int> col88(0, cnt);
                DdbVector<int> col89(0, cnt);

                DdbVector<int> col90(0, cnt);
                DdbVector<int> col91(0, cnt);
                DdbVector<long long> col92(0, cnt);
                DdbVector<char> col93(0, cnt);
                DdbVector<long long> col94(0, cnt);
                DdbVector<int> col95(0, cnt);


                for (uint32_t i = 0; i < cnt; ++i) {
                    col0.add(snapshot[i].market_type);
                    col1.add(snapshot[i].security_code);
                    col2.add(convertTime(snapshot[i].orig_time));
                    col3.add(snapshot[i].trading_phase_code);
                    col4.add(snapshot[i].pre_close_price);
                    col5.add(snapshot[i].open_price);
                    col6.add(snapshot[i].high_price);
                    col7.add(snapshot[i].low_price);
                    col8.add(snapshot[i].last_price);
                    col9.add(snapshot[i].close_price);

                    int bidPriceIndex = 0;
                    col10.add(snapshot[i].bid_price[bidPriceIndex++]);
                    col11.add(snapshot[i].bid_price[bidPriceIndex++]);
                    col12.add(snapshot[i].bid_price[bidPriceIndex++]);
                    col13.add(snapshot[i].bid_price[bidPriceIndex++]);
                    col14.add(snapshot[i].bid_price[bidPriceIndex++]);
                    col15.add(snapshot[i].bid_price[bidPriceIndex++]);
                    col16.add(snapshot[i].bid_price[bidPriceIndex++]);
                    col17.add(snapshot[i].bid_price[bidPriceIndex++]);
                    col18.add(snapshot[i].bid_price[bidPriceIndex++]);
                    col19.add(snapshot[i].bid_price[bidPriceIndex++]);

                    int bidVolumeIndex = 0;
                    col20.add(snapshot[i].bid_volume[bidVolumeIndex++]);
                    col21.add(snapshot[i].bid_volume[bidVolumeIndex++]);
                    col22.add(snapshot[i].bid_volume[bidVolumeIndex++]);
                    col23.add(snapshot[i].bid_volume[bidVolumeIndex++]);
                    col24.add(snapshot[i].bid_volume[bidVolumeIndex++]);
                    col25.add(snapshot[i].bid_volume[bidVolumeIndex++]);
                    col26.add(snapshot[i].bid_volume[bidVolumeIndex++]);
                    col27.add(snapshot[i].bid_volume[bidVolumeIndex++]);
                    col28.add(snapshot[i].bid_volume[bidVolumeIndex++]);
                    col29.add(snapshot[i].bid_volume[bidVolumeIndex++]);

                    int offerPriceIndex = 0;
                    col30.add(snapshot[i].offer_price[offerPriceIndex++]);
                    col31.add(snapshot[i].offer_price[offerPriceIndex++]);
                    col32.add(snapshot[i].offer_price[offerPriceIndex++]);
                    col33.add(snapshot[i].offer_price[offerPriceIndex++]);
                    col34.add(snapshot[i].offer_price[offerPriceIndex++]);
                    col35.add(snapshot[i].offer_price[offerPriceIndex++]);
                    col36.add(snapshot[i].offer_price[offerPriceIndex++]);
                    col37.add(snapshot[i].offer_price[offerPriceIndex++]);
                    col38.add(snapshot[i].offer_price[offerPriceIndex++]);
                    col39.add(snapshot[i].offer_price[offerPriceIndex++]);

                    int offerVolumeIndex = 0;
                    col40.add(snapshot[i].offer_volume[offerVolumeIndex++]);
                    col41.add(snapshot[i].offer_volume[offerVolumeIndex++]);
                    col42.add(snapshot[i].offer_volume[offerVolumeIndex++]);
                    col43.add(snapshot[i].offer_volume[offerVolumeIndex++]);
                    col44.add(snapshot[i].offer_volume[offerVolumeIndex++]);
                    col45.add(snapshot[i].offer_volume[offerVolumeIndex++]);
                    col46.add(snapshot[i].offer_volume[offerVolumeIndex++]);
                    col47.add(snapshot[i].offer_volume[offerVolumeIndex++]);
                    col48.add(snapshot[i].offer_volume[offerVolumeIndex++]);
                    col49.add(snapshot[i].offer_volume[offerVolumeIndex++]);

                    col50.add(snapshot[i].num_trades);
                    col51.add(snapshot[i].total_volume_trade);
                    col52.add(snapshot[i].total_value_trade);
                    col53.add(snapshot[i].total_bid_volume);
                    col54.add(snapshot[i].total_offer_volume);
                    col55.add(snapshot[i].weighted_avg_bid_price);
                    col56.add(snapshot[i].weighted_avg_offer_price);
                    col57.add(snapshot[i].IOPV);
                    col58.add(snapshot[i].yield_to_maturity);
                    col59.add(snapshot[i].high_limited);
                    col60.add(snapshot[i].low_limited);
                    col61.add(snapshot[i].price_earning_ratio1);
                    col62.add(snapshot[i].price_earning_ratio2);
                    col63.add(snapshot[i].change1);
                    col64.add(snapshot[i].change2);
                    col65.add(snapshot[i].channel_no);
                    col66.add(snapshot[i].md_stream_id);
                    col67.add(snapshot[i].instrument_status);
                    col68.add(snapshot[i].pre_close_iopv);
                    col69.add(snapshot[i].alt_weighted_avg_bid_price);
                    col70.add(snapshot[i].alt_weighted_avg_offer_price);
                    col71.add(snapshot[i].etf_buy_number);
                    col72.add(snapshot[i].etf_buy_amount);
                    col73.add(snapshot[i].etf_buy_money);
                    col74.add(snapshot[i].etf_sell_number);
                    col75.add(snapshot[i].etf_sell_amount);
                    col76.add(snapshot[i].etf_sell_money);
                    col77.add(snapshot[i].total_warrant_exec_volume);
                    col78.add(snapshot[i].war_lower_price);
                    col79.add(snapshot[i].war_upper_price);
                    col80.add(snapshot[i].withdraw_buy_number);
                    col81.add(snapshot[i].withdraw_buy_amount);
                    col82.add(snapshot[i].withdraw_buy_money);
                    col83.add(snapshot[i].withdraw_sell_number);
                    col84.add(snapshot[i].withdraw_sell_amount);
                    col85.add(snapshot[i].withdraw_sell_money);
                    col86.add(snapshot[i].total_bid_number);
                    col87.add(snapshot[i].total_offer_number);
                    col88.add(snapshot[i].bid_trade_max_duration);
                    col89.add(snapshot[i].offer_trade_max_duration);
                    col90.add(snapshot[i].num_bid_orders);
                    col91.add(snapshot[i].num_offer_orders);
                    col92.add(snapshot[i].last_trade_time);
                    col93.add(snapshot[i].variety_category);

                    if (receivedTimeFlag) {
                        col94.add(startTime);
                    }
                    if(dailyIndexFlagTotal){
                        int dailyIndex = INT_MIN;
                        if(!getDailyIndex(dailyIndex, dailyIndex_, sizeof(dailyIndex_), snapshot[i].market_type, datatype, snapshot[i].channel_no, convertTime(snapshot[i].orig_time))){
                            LOG_ERR("[PluginAmdQute]: getDailyIndex failed. ");
                            return;
                        }
                        col95.add(dailyIndex);
                    }
                }

                vector<ConstantSP> cols;
                cols.push_back(col0.createVector(DT_INT));
                cols.push_back(col1.createVector(DT_SYMBOL));
                cols.push_back(col2.createVector(DT_TIMESTAMP));
                cols.push_back(col3.createVector(DT_STRING));
                cols.push_back(col4.createVector(DT_LONG));
                cols.push_back(col5.createVector(DT_LONG));
                cols.push_back(col6.createVector(DT_LONG));
                cols.push_back(col7.createVector(DT_LONG));
                cols.push_back(col8.createVector(DT_LONG));
                cols.push_back(col9.createVector(DT_LONG));

                cols.push_back(col10.createVector(DT_LONG));
                cols.push_back(col11.createVector(DT_LONG));
                cols.push_back(col12.createVector(DT_LONG));
                cols.push_back(col13.createVector(DT_LONG));
                cols.push_back(col14.createVector(DT_LONG));
                cols.push_back(col15.createVector(DT_LONG));
                cols.push_back(col16.createVector(DT_LONG));
                cols.push_back(col17.createVector(DT_LONG));
                cols.push_back(col18.createVector(DT_LONG));
                cols.push_back(col19.createVector(DT_LONG));

                cols.push_back(col20.createVector(DT_LONG));
                cols.push_back(col21.createVector(DT_LONG));
                cols.push_back(col22.createVector(DT_LONG));
                cols.push_back(col23.createVector(DT_LONG));
                cols.push_back(col24.createVector(DT_LONG));
                cols.push_back(col25.createVector(DT_LONG));
                cols.push_back(col26.createVector(DT_LONG));
                cols.push_back(col27.createVector(DT_LONG));
                cols.push_back(col28.createVector(DT_LONG));
                cols.push_back(col29.createVector(DT_LONG));

                cols.push_back(col30.createVector(DT_LONG));
                cols.push_back(col31.createVector(DT_LONG));
                cols.push_back(col32.createVector(DT_LONG));
                cols.push_back(col33.createVector(DT_LONG));
                cols.push_back(col34.createVector(DT_LONG));
                cols.push_back(col35.createVector(DT_LONG));
                cols.push_back(col36.createVector(DT_LONG));
                cols.push_back(col37.createVector(DT_LONG));
                cols.push_back(col38.createVector(DT_LONG));
                cols.push_back(col39.createVector(DT_LONG));

                cols.push_back(col40.createVector(DT_LONG));
                cols.push_back(col41.createVector(DT_LONG));
                cols.push_back(col42.createVector(DT_LONG));
                cols.push_back(col43.createVector(DT_LONG));
                cols.push_back(col44.createVector(DT_LONG));
                cols.push_back(col45.createVector(DT_LONG));
                cols.push_back(col46.createVector(DT_LONG));
                cols.push_back(col47.createVector(DT_LONG));
                cols.push_back(col48.createVector(DT_LONG));
                cols.push_back(col49.createVector(DT_LONG));

                cols.push_back(col50.createVector(DT_LONG));
                cols.push_back(col51.createVector(DT_LONG));
                cols.push_back(col52.createVector(DT_LONG));
                cols.push_back(col53.createVector(DT_LONG));
                cols.push_back(col54.createVector(DT_LONG));
                cols.push_back(col55.createVector(DT_LONG));
                cols.push_back(col56.createVector(DT_LONG));
                cols.push_back(col57.createVector(DT_LONG));
                cols.push_back(col58.createVector(DT_LONG));
                cols.push_back(col59.createVector(DT_LONG));

                cols.push_back(col60.createVector(DT_LONG));
                cols.push_back(col61.createVector(DT_LONG));
                cols.push_back(col62.createVector(DT_LONG));
                cols.push_back(col63.createVector(DT_LONG));
                cols.push_back(col64.createVector(DT_LONG));
                cols.push_back(col65.createVector(DT_INT));
                cols.push_back(col66.createVector(DT_STRING));
                cols.push_back(col67.createVector(DT_STRING));
                cols.push_back(col68.createVector(DT_LONG));
                cols.push_back(col69.createVector(DT_LONG));

                cols.push_back(col70.createVector(DT_LONG));
                cols.push_back(col71.createVector(DT_LONG));
                cols.push_back(col72.createVector(DT_LONG));
                cols.push_back(col73.createVector(DT_LONG));
                cols.push_back(col74.createVector(DT_LONG));
                cols.push_back(col75.createVector(DT_LONG));
                cols.push_back(col76.createVector(DT_LONG));
                cols.push_back(col77.createVector(DT_LONG));
                cols.push_back(col78.createVector(DT_LONG));
                cols.push_back(col79.createVector(DT_LONG));

                cols.push_back(col80.createVector(DT_LONG));
                cols.push_back(col81.createVector(DT_LONG));
                cols.push_back(col82.createVector(DT_LONG));
                cols.push_back(col83.createVector(DT_LONG));
                cols.push_back(col84.createVector(DT_LONG));
                cols.push_back(col85.createVector(DT_LONG));
                cols.push_back(col86.createVector(DT_LONG));
                cols.push_back(col87.createVector(DT_LONG));
                cols.push_back(col88.createVector(DT_INT));
                cols.push_back(col89.createVector(DT_INT));

                cols.push_back(col90.createVector(DT_INT));
                cols.push_back(col91.createVector(DT_INT));
                cols.push_back(col92.createVector(DT_LONG));
                cols.push_back(col93.createVector(DT_CHAR));
                
                if(receivedTimeFlag)
                    cols.push_back(col94.createVector(DT_NANOTIMESTAMP));
                if(dailyIndexFlagTotal)
                    cols.push_back(col95.createVector(DT_INT));     

                vector<string> colNames = snapshotDataTableMeta_.colNames_;
                if(receivedTimeFlag)
                    colNames.push_back("receivedTime");
                if(dailyIndexFlagTotal)
                    colNames.push_back("dailyIndex");
                
                TableSP data = Util::createTable(colNames, cols);
                vector<ConstantSP> args = {data};
                try{
                    if(!transform.isNull())
                        data = transform->call(session_->getHeap().get(), args);
                }catch(exception &e){
                    throw RuntimeException("call transform error " + string(e.what()));
                }

                if(insertedTable.isNull())
                    throw RuntimeException("insertedTable is null");
                if(insertedTable ->columns() != data->columns())
                    throw RuntimeException("The number of columns of the table to insert must be the same as that of the original table");
                args = {insertedTable, data};   
                session_->getFunctionDef("append!")->call(session_->getHeap().get(), args);

                if (latencyFlag_) {
                    long long diff = Util::toLocalNanoTimestamp(Util::getNanoEpochTime() - startTime);
                    latencyLog(0, startTime, cnt, diff);
                }
            }
            catch(exception &e){
                LOG_ERR("[PluginAmdQuote]: OnMDsnapshot failed, ", e.what());
            }
        }
        
        // 接受并处理快照数据(债券）
        void OnMDBondSnapshotHelper(amd::ama::MDBondSnapshot* snapshot, uint32_t cnt) {
            try{
                if(cnt == 0)return;
                long long startTime = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
                // std::lock_guard<std::mutex> amdLock_(amdMutex_);

                // 基金和股票数据插入到不同的表
                TableSP insertedTable;
                uint8_t varietyCategory = snapshot[0].variety_category;
                FunctionDefSP transform;

                AMDDataType datatype;
                if (varietyCategory == 3) { // 债券快照
                    insertedTable = bondSnapshotData_;
                    transform = bondSnapshotTransform_;
                    datatype = AMD_BOND_SNAPSHOT;
                }else {
                    return ;
                }

                int market_type = snapshot[0].market_type;
                AMDTableType tableTyeTotal = getAmdTableType(datatype, market_type);
                if(tableTyeTotal == AMD_ERROR_TABLE_TYPE){
                    LOG_ERR("[PluginAmdQuote]: error amd table type AMD_ERROR_TABLE_TYPE");
                    return;
                }

                // 检查是否需要时间戳列
                bool receivedTimeFlag = (varietyCategory ==3 && bondSnapshotReceivedTimeFlag_);

                bool dailyIndexFlagTotal = dailyIndex_[tableTyeTotal].getFlag();

                DdbVector<int> col0(0, cnt);
                DdbVector<string> col1(0, cnt);
                DdbVector<long long> col2(0, cnt);
                DdbVector<string> col3(0, cnt);
                DdbVector<long long> col4(0, cnt);
                DdbVector<long long> col5(0, cnt);
                DdbVector<long long> col6(0, cnt);
                DdbVector<long long> col7(0, cnt);
                DdbVector<long long> col8(0, cnt);
                DdbVector<long long> col9(0, cnt);

                DdbVector<long long> col10(0, cnt);
                DdbVector<long long> col11(0, cnt);
                DdbVector<long long> col12(0, cnt);
                DdbVector<long long> col13(0, cnt);
                DdbVector<long long> col14(0, cnt);
                DdbVector<long long> col15(0, cnt);
                DdbVector<long long> col16(0, cnt);
                DdbVector<long long> col17(0, cnt);
                DdbVector<long long> col18(0, cnt);
                DdbVector<long long> col19(0, cnt);

                DdbVector<long long> col20(0, cnt);
                DdbVector<long long> col21(0, cnt);
                DdbVector<long long> col22(0, cnt);
                DdbVector<long long> col23(0, cnt);
                DdbVector<long long> col24(0, cnt);
                DdbVector<long long> col25(0, cnt);
                DdbVector<long long> col26(0, cnt);
                DdbVector<long long> col27(0, cnt);
                DdbVector<long long> col28(0, cnt);
                DdbVector<long long> col29(0, cnt);

                DdbVector<long long> col30(0, cnt);
                DdbVector<long long> col31(0, cnt);
                DdbVector<long long> col32(0, cnt);
                DdbVector<long long> col33(0, cnt);
                DdbVector<long long> col34(0, cnt);
                DdbVector<long long> col35(0, cnt);
                DdbVector<long long> col36(0, cnt);
                DdbVector<long long> col37(0, cnt);
                DdbVector<long long> col38(0, cnt);
                DdbVector<long long> col39(0, cnt);
                
                DdbVector<long long> col40(0, cnt);
                DdbVector<long long> col41(0, cnt);
                DdbVector<long long> col42(0, cnt);
                DdbVector<long long> col43(0, cnt);
                DdbVector<long long> col44(0, cnt);
                DdbVector<long long> col45(0, cnt);
                DdbVector<long long> col46(0, cnt);
                DdbVector<long long> col47(0, cnt);
                DdbVector<long long> col48(0, cnt);
                DdbVector<long long> col49(0, cnt);

                DdbVector<long long> col50(0, cnt);
                DdbVector<long long> col51(0, cnt);
                DdbVector<long long> col52(0, cnt);
                DdbVector<long long> col53(0, cnt);
                DdbVector<long long> col54(0, cnt);
                DdbVector<long long> col55(0, cnt);
                DdbVector<long long> col56(0, cnt);
                DdbVector<long long> col57(0, cnt);
                DdbVector<long long> col58(0, cnt);
                DdbVector<long long> col59(0, cnt);

                DdbVector<long long> col60(0, cnt);
                DdbVector<long long> col61(0, cnt);
                DdbVector<long long> col62(0, cnt);
                DdbVector<long long> col63(0, cnt);
                DdbVector<long long> col64(0, cnt);
                DdbVector<int> col65(0, cnt);
                DdbVector<string> col66(0, cnt);
                DdbVector<string> col67(0, cnt);
                DdbVector<long long> col68(0, cnt);
                DdbVector<long long> col69(0, cnt);

                DdbVector<long long> col70(0, cnt);
                DdbVector<long long> col71(0, cnt);
                DdbVector<long long> col72(0, cnt);
                DdbVector<long long> col73(0, cnt);
                DdbVector<long long> col74(0, cnt);
                DdbVector<long long> col75(0, cnt);
                DdbVector<long long> col76(0, cnt);
                DdbVector<long long> col77(0, cnt);
                DdbVector<long long> col78(0, cnt);
                DdbVector<long long> col79(0, cnt);

                DdbVector<long long> col80(0, cnt);
                DdbVector<long long> col81(0, cnt);
                DdbVector<long long> col82(0, cnt);
                DdbVector<long long> col83(0, cnt);
                DdbVector<long long> col84(0, cnt);
                DdbVector<long long> col85(0, cnt);
                DdbVector<long long> col86(0, cnt);
                DdbVector<long long> col87(0, cnt);
                DdbVector<int> col88(0, cnt);
                DdbVector<int> col89(0, cnt);

                DdbVector<int> col90(0, cnt);
                DdbVector<int> col91(0, cnt);
                DdbVector<long long> col92(0, cnt);
                DdbVector<char> col93(0, cnt);
                DdbVector<long long> col94(0, cnt);
                DdbVector<int> col95(0, cnt);


                for (uint32_t i = 0; i < cnt; ++i) {
                    col0.add(snapshot[i].market_type);
                    col1.add(snapshot[i].security_code);
                    col2.add(convertTime(snapshot[i].orig_time));
                    col3.add(snapshot[i].trading_phase_code);
                    col4.add(snapshot[i].pre_close_price);
                    col5.add(snapshot[i].open_price);
                    col6.add(snapshot[i].high_price);
                    col7.add(snapshot[i].low_price);
                    col8.add(snapshot[i].last_price);
                    col9.add(snapshot[i].close_price);

                    int bidPriceIndex = 0;
                    col10.add(snapshot[i].bid_price[bidPriceIndex++]);
                    col11.add(snapshot[i].bid_price[bidPriceIndex++]);
                    col12.add(snapshot[i].bid_price[bidPriceIndex++]);
                    col13.add(snapshot[i].bid_price[bidPriceIndex++]);
                    col14.add(snapshot[i].bid_price[bidPriceIndex++]);
                    col15.add(snapshot[i].bid_price[bidPriceIndex++]);
                    col16.add(snapshot[i].bid_price[bidPriceIndex++]);
                    col17.add(snapshot[i].bid_price[bidPriceIndex++]);
                    col18.add(snapshot[i].bid_price[bidPriceIndex++]);
                    col19.add(snapshot[i].bid_price[bidPriceIndex++]);

                    int bidVolumeIndex = 0;
                    col20.add(snapshot[i].bid_volume[bidVolumeIndex++]);
                    col21.add(snapshot[i].bid_volume[bidVolumeIndex++]);
                    col22.add(snapshot[i].bid_volume[bidVolumeIndex++]);
                    col23.add(snapshot[i].bid_volume[bidVolumeIndex++]);
                    col24.add(snapshot[i].bid_volume[bidVolumeIndex++]);
                    col25.add(snapshot[i].bid_volume[bidVolumeIndex++]);
                    col26.add(snapshot[i].bid_volume[bidVolumeIndex++]);
                    col27.add(snapshot[i].bid_volume[bidVolumeIndex++]);
                    col28.add(snapshot[i].bid_volume[bidVolumeIndex++]);
                    col29.add(snapshot[i].bid_volume[bidVolumeIndex++]);

                    int offerPriceIndex = 0;
                    col30.add(snapshot[i].offer_price[offerPriceIndex++]);
                    col31.add(snapshot[i].offer_price[offerPriceIndex++]);
                    col32.add(snapshot[i].offer_price[offerPriceIndex++]);
                    col33.add(snapshot[i].offer_price[offerPriceIndex++]);
                    col34.add(snapshot[i].offer_price[offerPriceIndex++]);
                    col35.add(snapshot[i].offer_price[offerPriceIndex++]);
                    col36.add(snapshot[i].offer_price[offerPriceIndex++]);
                    col37.add(snapshot[i].offer_price[offerPriceIndex++]);
                    col38.add(snapshot[i].offer_price[offerPriceIndex++]);
                    col39.add(snapshot[i].offer_price[offerPriceIndex++]);

                    int offerVolumeIndex = 0;
                    col40.add(snapshot[i].offer_volume[offerVolumeIndex++]);
                    col41.add(snapshot[i].offer_volume[offerVolumeIndex++]);
                    col42.add(snapshot[i].offer_volume[offerVolumeIndex++]);
                    col43.add(snapshot[i].offer_volume[offerVolumeIndex++]);
                    col44.add(snapshot[i].offer_volume[offerVolumeIndex++]);
                    col45.add(snapshot[i].offer_volume[offerVolumeIndex++]);
                    col46.add(snapshot[i].offer_volume[offerVolumeIndex++]);
                    col47.add(snapshot[i].offer_volume[offerVolumeIndex++]);
                    col48.add(snapshot[i].offer_volume[offerVolumeIndex++]);
                    col49.add(snapshot[i].offer_volume[offerVolumeIndex++]);

                    col50.add(snapshot[i].num_trades);
                    col51.add(snapshot[i].total_volume_trade);
                    col52.add(snapshot[i].total_value_trade);
                    col53.add(snapshot[i].total_bid_volume);
                    col54.add(snapshot[i].total_offer_volume);
                    col55.add(snapshot[i].weighted_avg_bid_price);
                    col56.add(snapshot[i].weighted_avg_offer_price);
                    // col57.add(snapshot[i].IOPV);
                    col57.add(0);
                    // col58.add(snapshot[i].yield_to_maturity);
                    col58.add(LONG_LONG_MIN);
                    col59.add(snapshot[i].high_limited);
                    col60.add(snapshot[i].low_limited);
                    // col61.add(snapshot[i].price_earning_ratio1);
                    col61.add(LONG_LONG_MIN);
                    // col62.add(snapshot[i].price_earning_ratio2);
                    col62.add(LONG_LONG_MIN);
                    col63.add(snapshot[i].change1);
                    col64.add(snapshot[i].change2);
                    col65.add(snapshot[i].channel_no);
                    col66.add(snapshot[i].md_stream_id);
                    col67.add(snapshot[i].instrument_status);
                    // col68.add(snapshot[i].pre_close_iopv);
                    col68.add(LONG_LONG_MIN);
                    // col69.add(snapshot[i].alt_weighted_avg_bid_price);
                    col69.add(LONG_LONG_MIN);
                    // col70.add(snapshot[i].alt_weighted_avg_offer_price);
                    col70.add(LONG_LONG_MIN);
                    // col71.add(snapshot[i].etf_buy_number);
                    // col72.add(snapshot[i].etf_buy_amount);
                    // col73.add(snapshot[i].etf_buy_money);
                    // col74.add(snapshot[i].etf_sell_number);
                    // col75.add(snapshot[i].etf_sell_amount);
                    // col76.add(snapshot[i].etf_sell_money);
                    col71.add(LONG_LONG_MIN);
                    col72.add(LONG_LONG_MIN);
                    col73.add(LONG_LONG_MIN);
                    col74.add(LONG_LONG_MIN);
                    col75.add(LONG_LONG_MIN);
                    col76.add(LONG_LONG_MIN);
                    // col77.add(snapshot[i].total_warrant_exec_volume);
                    // col78.add(snapshot[i].war_lower_price);
                    // col79.add(snapshot[i].war_upper_price);
                    col77.add(LONG_LONG_MIN);
                    col78.add(LONG_LONG_MIN);
                    col79.add(LONG_LONG_MIN);
                    col80.add(snapshot[i].withdraw_buy_number);
                    col81.add(snapshot[i].withdraw_buy_amount);
                    col82.add(snapshot[i].withdraw_buy_money);
                    col83.add(snapshot[i].withdraw_sell_number);
                    col84.add(snapshot[i].withdraw_sell_amount);
                    col85.add(snapshot[i].withdraw_sell_money);
                    col86.add(snapshot[i].total_bid_number);
                    col87.add(snapshot[i].total_offer_number);
                    col88.add(snapshot[i].bid_trade_max_duration);
                    col89.add(snapshot[i].offer_trade_max_duration);
                    col90.add(snapshot[i].num_bid_orders);
                    col91.add(snapshot[i].num_offer_orders);
                    col92.add(snapshot[i].last_trade_time);
                    col93.add(snapshot[i].variety_category);

                    if (receivedTimeFlag) {
                        col94.add(startTime);
                    }
                    if(dailyIndexFlagTotal){
                        int dailyIndex = INT_MIN;
                        if(!getDailyIndex(dailyIndex, dailyIndex_, sizeof(dailyIndex_), snapshot[i].market_type, datatype, snapshot[i].channel_no, convertTime(snapshot[i].orig_time))){
                            LOG_ERR("[PluginAmdQute]: getDailyIndex failed. ");
                            return;
                        }
                        col95.add(dailyIndex);
                    }
                }

                vector<ConstantSP> cols;
                cols.push_back(col0.createVector(DT_INT));
                cols.push_back(col1.createVector(DT_SYMBOL));
                cols.push_back(col2.createVector(DT_TIMESTAMP));
                cols.push_back(col3.createVector(DT_STRING));
                cols.push_back(col4.createVector(DT_LONG));
                cols.push_back(col5.createVector(DT_LONG));
                cols.push_back(col6.createVector(DT_LONG));
                cols.push_back(col7.createVector(DT_LONG));
                cols.push_back(col8.createVector(DT_LONG));
                cols.push_back(col9.createVector(DT_LONG));

                cols.push_back(col10.createVector(DT_LONG));
                cols.push_back(col11.createVector(DT_LONG));
                cols.push_back(col12.createVector(DT_LONG));
                cols.push_back(col13.createVector(DT_LONG));
                cols.push_back(col14.createVector(DT_LONG));
                cols.push_back(col15.createVector(DT_LONG));
                cols.push_back(col16.createVector(DT_LONG));
                cols.push_back(col17.createVector(DT_LONG));
                cols.push_back(col18.createVector(DT_LONG));
                cols.push_back(col19.createVector(DT_LONG));

                cols.push_back(col20.createVector(DT_LONG));
                cols.push_back(col21.createVector(DT_LONG));
                cols.push_back(col22.createVector(DT_LONG));
                cols.push_back(col23.createVector(DT_LONG));
                cols.push_back(col24.createVector(DT_LONG));
                cols.push_back(col25.createVector(DT_LONG));
                cols.push_back(col26.createVector(DT_LONG));
                cols.push_back(col27.createVector(DT_LONG));
                cols.push_back(col28.createVector(DT_LONG));
                cols.push_back(col29.createVector(DT_LONG));

                cols.push_back(col30.createVector(DT_LONG));
                cols.push_back(col31.createVector(DT_LONG));
                cols.push_back(col32.createVector(DT_LONG));
                cols.push_back(col33.createVector(DT_LONG));
                cols.push_back(col34.createVector(DT_LONG));
                cols.push_back(col35.createVector(DT_LONG));
                cols.push_back(col36.createVector(DT_LONG));
                cols.push_back(col37.createVector(DT_LONG));
                cols.push_back(col38.createVector(DT_LONG));
                cols.push_back(col39.createVector(DT_LONG));

                cols.push_back(col40.createVector(DT_LONG));
                cols.push_back(col41.createVector(DT_LONG));
                cols.push_back(col42.createVector(DT_LONG));
                cols.push_back(col43.createVector(DT_LONG));
                cols.push_back(col44.createVector(DT_LONG));
                cols.push_back(col45.createVector(DT_LONG));
                cols.push_back(col46.createVector(DT_LONG));
                cols.push_back(col47.createVector(DT_LONG));
                cols.push_back(col48.createVector(DT_LONG));
                cols.push_back(col49.createVector(DT_LONG));

                cols.push_back(col50.createVector(DT_LONG));
                cols.push_back(col51.createVector(DT_LONG));
                cols.push_back(col52.createVector(DT_LONG));
                cols.push_back(col53.createVector(DT_LONG));
                cols.push_back(col54.createVector(DT_LONG));
                cols.push_back(col55.createVector(DT_LONG));
                cols.push_back(col56.createVector(DT_LONG));
                cols.push_back(col57.createVector(DT_LONG));
                cols.push_back(col58.createVector(DT_LONG));
                cols.push_back(col59.createVector(DT_LONG));

                cols.push_back(col60.createVector(DT_LONG));
                cols.push_back(col61.createVector(DT_LONG));
                cols.push_back(col62.createVector(DT_LONG));
                cols.push_back(col63.createVector(DT_LONG));
                cols.push_back(col64.createVector(DT_LONG));
                cols.push_back(col65.createVector(DT_INT));
                cols.push_back(col66.createVector(DT_STRING));
                cols.push_back(col67.createVector(DT_STRING));
                cols.push_back(col68.createVector(DT_LONG));
                cols.push_back(col69.createVector(DT_LONG));

                cols.push_back(col70.createVector(DT_LONG));
                cols.push_back(col71.createVector(DT_LONG));
                cols.push_back(col72.createVector(DT_LONG));
                cols.push_back(col73.createVector(DT_LONG));
                cols.push_back(col74.createVector(DT_LONG));
                cols.push_back(col75.createVector(DT_LONG));
                cols.push_back(col76.createVector(DT_LONG));
                cols.push_back(col77.createVector(DT_LONG));
                cols.push_back(col78.createVector(DT_LONG));
                cols.push_back(col79.createVector(DT_LONG));

                cols.push_back(col80.createVector(DT_LONG));
                cols.push_back(col81.createVector(DT_LONG));
                cols.push_back(col82.createVector(DT_LONG));
                cols.push_back(col83.createVector(DT_LONG));
                cols.push_back(col84.createVector(DT_LONG));
                cols.push_back(col85.createVector(DT_LONG));
                cols.push_back(col86.createVector(DT_LONG));
                cols.push_back(col87.createVector(DT_LONG));
                cols.push_back(col88.createVector(DT_INT));
                cols.push_back(col89.createVector(DT_INT));

                cols.push_back(col90.createVector(DT_INT));
                cols.push_back(col91.createVector(DT_INT));
                cols.push_back(col92.createVector(DT_LONG));
                cols.push_back(col93.createVector(DT_CHAR));
                
                if(receivedTimeFlag)
                    cols.push_back(col94.createVector(DT_NANOTIMESTAMP));
                if(dailyIndexFlagTotal)
                    cols.push_back(col95.createVector(DT_INT));     

                vector<string> colNames = snapshotDataTableMeta_.colNames_;
                if(receivedTimeFlag)
                    colNames.push_back("receivedTime");
                if(dailyIndexFlagTotal)
                    colNames.push_back("dailyIndex");
                
                TableSP data = Util::createTable(colNames, cols);
                vector<ConstantSP> args = {data};
                try{
                    if(!transform.isNull())
                        data = transform->call(session_->getHeap().get(), args);
                }catch(exception &e){
                    throw RuntimeException("call transform error " + string(e.what()));
                }

                if(insertedTable.isNull())
                    throw RuntimeException("insertedTable is null");
                if(insertedTable ->columns() != data->columns())
                    throw RuntimeException("The number of columns of the table to insert must be the same as that of the original table");
                args = {insertedTable, data};   
                session_->getFunctionDef("append!")->call(session_->getHeap().get(), args);

                if (latencyFlag_) {
                    long long diff = Util::toLocalNanoTimestamp(Util::getNanoEpochTime() - startTime);
                    latencyLog(3, startTime, cnt, diff);
                }
            }
            catch(exception &e){
                LOG_ERR("[PluginAmdQuote]: OnMDBondSnapshot failed, ", e.what());
            }
        }

        // 接受并处理逐笔委托数据（包含基金和股票数据）
        void OnMDTickOrderHelper(amd::ama::MDTickOrder* ticks, uint32_t cnt) {
            try{
                if(cnt == 0)return;
                long long startTime = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
                // std::lock_guard<std::mutex> amdLock_(amdMutex_);

                // 基金和股票数据插入到不同的表
                TableSP insertedTable;
                FunctionDefSP transform;
                uint8_t varietyCategory = ticks[0].variety_category;
                AMDDataType datatype;
                if (varietyCategory == 1) { // 股票逐笔委托
                    insertedTable = orderData_;
                    transform = orderTransform_;
                    datatype = AMD_ORDER;
                } else if (varietyCategory == 2) { // 基金逐笔委托
                    insertedTable = fundOrderData_;
                    transform = fundOrderTransform_;
                    datatype = AMD_FUND_ORDER;
                } else {
                    return ;
                }

                int market_type = ticks[0].market_type;
                AMDTableType tableTyeTotal = getAmdTableType(datatype, market_type);
                if(tableTyeTotal == AMD_ERROR_TABLE_TYPE){
                    LOG_ERR("[PluginAmdQuote]: error amd table type AMD_ERROR_TABLE_TYPE");
                    return;
                }

                bool receivedTimeFlag = (varietyCategory == 1 && orderReceivedTimeFlag_) || (varietyCategory ==2 && fundOrderReceivedTimeFlag_);

                bool dailyIndexFlagTotal = dailyIndex_[tableTyeTotal].getFlag();

                DdbVector<int> col0(0, cnt);
                DdbVector<string> col1(0, cnt);
                DdbVector<int> col2(0, cnt);
                DdbVector<long long> col3(0, cnt);
                DdbVector<long long> col4(0, cnt);
                DdbVector<long long> col5(0, cnt);
                DdbVector<long long> col6(0, cnt);
                DdbVector<char> col7(0, cnt);
                DdbVector<char> col8(0, cnt);
                DdbVector<string> col9(0, cnt);
                DdbVector<long long> col10(0, cnt);
                DdbVector<long long> col11(0, cnt);
                DdbVector<char> col12(0, cnt);
                DdbVector<long long> col13(0, cnt);
                DdbVector<int> col14(0, cnt);

                for (uint32_t i = 0; i < cnt; ++i) {
                    col0.add(ticks[i].market_type);
                    col1.add(ticks[i].security_code);
                    col2.add(ticks[i].channel_no);
                    col3.add(ticks[i].appl_seq_num);
                    col4.add(convertTime(ticks[i].order_time));
                    col5.add(ticks[i].order_price);
                    col6.add(ticks[i].order_volume);
                    col7.add(ticks[i].side);
                    col8.add(ticks[i].order_type);
                    col9.add(ticks[i].md_stream_id);
                    col10.add(ticks[i].orig_order_no);
                    col11.add(ticks[i].biz_index);
                    col12.add(ticks[i].variety_category);
                    if (receivedTimeFlag) {
                        col13.add(startTime);
                    }
                    if(dailyIndexFlagTotal){
                        int dailyIndex = INT_MIN;
                        if(!getDailyIndex(dailyIndex, dailyIndex_, sizeof(dailyIndex_), ticks[i].market_type, datatype, ticks[i].channel_no, convertTime(ticks[i].order_time))){
                            LOG_ERR("[PluginAmdQute]: getDailyIndex failed. ");
                            return;
                        }
                        col14.add(dailyIndex);
                    }
                }

                vector<ConstantSP> cols;
                cols.push_back(col0.createVector(DT_INT));
                cols.push_back(col1.createVector(DT_STRING));
                cols.push_back(col2.createVector(DT_INT));
                cols.push_back(col3.createVector(DT_LONG));
                cols.push_back(col4.createVector(DT_TIMESTAMP));
                cols.push_back(col5.createVector(DT_LONG));
                cols.push_back(col6.createVector(DT_LONG));
                cols.push_back(col7.createVector(DT_CHAR));
                cols.push_back(col8.createVector(DT_CHAR));
                cols.push_back(col9.createVector(DT_STRING));
                cols.push_back(col10.createVector(DT_LONG));
                cols.push_back(col11.createVector(DT_LONG));
                cols.push_back(col12.createVector(DT_CHAR));
                if (receivedTimeFlag) {
                    cols.push_back(col13.createVector(DT_NANOTIMESTAMP));
                }
                if(dailyIndexFlagTotal)
                    cols.push_back(col14.createVector(DT_INT));    

                vector<string> colNames = orderTableMeta_.colNames_;
                if(receivedTimeFlag)
                    colNames.push_back("receivedTime");
                if(dailyIndexFlagTotal)
                    colNames.push_back("dailyIndex");
                TableSP data = Util::createTable(colNames, cols);

                vector<ConstantSP> args = {data};
                try{
                    if(!transform.isNull())
                        data = transform->call(session_->getHeap().get(), args);
                }catch(exception &e){
                    throw RuntimeException("call transform error " + string(e.what()));
                }

                if(insertedTable.isNull())
                    throw RuntimeException("insertedTable is null");
                if(insertedTable ->columns() != data->columns())
                    throw RuntimeException("The number of columns of the table to insert must be the same as that of the original table");
                args = {insertedTable, data};
                session_->getFunctionDef("append!")->call(session_->getHeap().get(), args);

                if (latencyFlag_) { 
                    long long diff = Util::toLocalNanoTimestamp(Util::getNanoEpochTime() - startTime);
                    latencyLog(2, startTime, cnt, diff);
                }
            }
            catch(exception &e){
                LOG_ERR("[PluginAmdQuote]: OnMDTickOrder failed, ", e.what());
            }
        }

        // 接受并处理逐笔委托数据（债券）
        void OnMDBondTickOrderHelper(amd::ama::MDBondTickOrder* ticks, uint32_t cnt) {
            try{
                if(cnt == 0)return;
                long long startTime = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
                // std::lock_guard<std::mutex> amdLock_(amdMutex_);

                // 基金和股票数据插入到不同的表
                TableSP insertedTable;
                FunctionDefSP transform;
                uint8_t varietyCategory = ticks[0].variety_category;
                AMDDataType datatype;
                if (varietyCategory == 3) { // 债券
                    insertedTable = bondOrderData_;
                    transform = bondOrderTransform_;
                    datatype = AMD_BOND_ORDER;
                } else {
                    return ;
                }

                int market_type = ticks[0].market_type;
                AMDTableType tableTyeTotal = getAmdTableType(datatype, market_type);
                if(tableTyeTotal == AMD_ERROR_TABLE_TYPE){
                    LOG_ERR("[PluginAmdQuote]: error amd table type AMD_ERROR_TABLE_TYPE");
                    return;
                }

                bool receivedTimeFlag = (varietyCategory ==3 && bondOrderReceivedTimeFlag_);

                bool dailyIndexFlagTotal = dailyIndex_[tableTyeTotal].getFlag();

                DdbVector<int> col0(0, cnt);
                DdbVector<string> col1(0, cnt);
                DdbVector<int> col2(0, cnt);
                DdbVector<long long> col3(0, cnt);
                DdbVector<long long> col4(0, cnt);
                DdbVector<long long> col5(0, cnt);
                DdbVector<long long> col6(0, cnt);
                DdbVector<char> col7(0, cnt);
                DdbVector<char> col8(0, cnt);
                DdbVector<string> col9(0, cnt);
                DdbVector<long long> col10(0, cnt);
                DdbVector<long long> col11(0, cnt);
                DdbVector<char> col12(0, cnt);
                DdbVector<long long> col13(0, cnt);
                DdbVector<int> col14(0, cnt);

                for (uint32_t i = 0; i < cnt; ++i) {
                    col0.add(ticks[i].market_type);
                    col1.add(ticks[i].security_code);
                    col2.add(ticks[i].channel_no);
                    col3.add(ticks[i].appl_seq_num);
                    col4.add(convertTime(ticks[i].order_time));
                    col5.add(ticks[i].order_price);
                    col6.add(ticks[i].order_volume);
                    col7.add(ticks[i].side);
                    col8.add(ticks[i].order_type);
                    col9.add(ticks[i].md_stream_id);
                    col10.add(ticks[i].orig_order_no);
                    col11.add(LONG_LONG_MIN);
                    col12.add(ticks[i].variety_category);
                    if (receivedTimeFlag) {
                        col13.add(startTime);
                    }
                    if(dailyIndexFlagTotal){
                        int dailyIndex = INT_MIN;
                        if(!getDailyIndex(dailyIndex, dailyIndex_, sizeof(dailyIndex_), ticks[i].market_type, datatype, ticks[i].channel_no, convertTime(ticks[i].order_time))){
                            LOG_ERR("[PluginAmdQute]: getDailyIndex failed. ");
                            return;
                        }
                        col14.add(dailyIndex);
                    }
                }

                vector<ConstantSP> cols;
                cols.push_back(col0.createVector(DT_INT));
                cols.push_back(col1.createVector(DT_STRING));
                cols.push_back(col2.createVector(DT_INT));
                cols.push_back(col3.createVector(DT_LONG));
                cols.push_back(col4.createVector(DT_TIMESTAMP));
                cols.push_back(col5.createVector(DT_LONG));
                cols.push_back(col6.createVector(DT_LONG));
                cols.push_back(col7.createVector(DT_CHAR));
                cols.push_back(col8.createVector(DT_CHAR));
                cols.push_back(col9.createVector(DT_STRING));
                cols.push_back(col10.createVector(DT_LONG));
                cols.push_back(col11.createVector(DT_LONG));
                cols.push_back(col12.createVector(DT_CHAR));
                if (receivedTimeFlag) {
                    cols.push_back(col13.createVector(DT_NANOTIMESTAMP));
                }
                if(dailyIndexFlagTotal)
                    cols.push_back(col14.createVector(DT_INT));    

                vector<string> colNames = orderTableMeta_.colNames_;
                if(receivedTimeFlag)
                    colNames.push_back("receivedTime");
                if(dailyIndexFlagTotal)
                    colNames.push_back("dailyIndex");
                TableSP data = Util::createTable(colNames, cols);

                vector<ConstantSP> args = {data};
                try{
                    if(!transform.isNull())
                        data = transform->call(session_->getHeap().get(), args);
                }catch(exception &e){
                    throw RuntimeException("call transform error " + string(e.what()));
                }

                if(insertedTable.isNull())
                    throw RuntimeException("insertedTable is null");
                if(insertedTable ->columns() != data->columns())
                    throw RuntimeException("The number of columns of the table to insert must be the same as that of the original table");
                args = {insertedTable, data};
                session_->getFunctionDef("append!")->call(session_->getHeap().get(), args);

                if (latencyFlag_) { 
                    long long diff = Util::toLocalNanoTimestamp(Util::getNanoEpochTime() - startTime);
                    latencyLog(5, startTime, cnt, diff);
                }
            }
            catch(exception &e){
                LOG_ERR("[PluginAmdQuote]: OnMDTickOrder failed, ", e.what());
            }
        }

        // 接受并处理逐笔成交数据（包含基金和股票数据）
        void OnMDTickExecutionHelper(amd::ama::MDTickExecution* tick, uint32_t cnt)  {
            try{
                if(cnt == 0)return;
                long long startTime = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
                // std::lock_guard<std::mutex> amdLock_(amdMutex_);

                TableSP insertedTable;
                FunctionDefSP transform;
                char varietyCategory = tick[0].variety_category;
                AMDDataType datatype;
                if (varietyCategory == 1) { // 股票逐笔成交
                    insertedTable = executionData_;
                    transform = executionTransform_;
                    datatype = AMD_EXECUTION;
                } else if (varietyCategory == 2) { // 基金逐笔成交
                    insertedTable = fundExecutionData_;
                    transform = fundExecutionTransform_;
                    datatype = AMD_FUND_EXECUTION;
                } else {
                    return ;
                }

                int market_type = tick[0].market_type;
                AMDTableType tableTyeTotal = getAmdTableType(datatype, market_type);
                if(tableTyeTotal == AMD_ERROR_TABLE_TYPE){
                    LOG_ERR("[PluginAmdQuote]: error amd table type AMD_ERROR_TABLE_TYPE");
                    return;
                }

                bool receivedTimeFlag = (varietyCategory == 1 && executioReceivedTimeFlag_) || (varietyCategory ==2 && fundExecutionReceivedTimeFlag_);

                bool dailyIndexFlagTotal = dailyIndex_[tableTyeTotal].getFlag();

                DdbVector<int> col0(0, cnt);
                DdbVector<string> col1(0, cnt);
                DdbVector<long long> col2(0, cnt);
                DdbVector<int> col3(0, cnt);
                DdbVector<long long> col4(0, cnt);
                DdbVector<long long> col5(0, cnt);
                DdbVector<long long> col6(0, cnt);
                DdbVector<long long> col7(0, cnt);
                DdbVector<long long> col8(0, cnt);
                DdbVector<long long> col9(0, cnt);
                DdbVector<char> col10(0, cnt);
                DdbVector<char> col11(0, cnt);
                DdbVector<string> col12(0, cnt);
                DdbVector<long long> col13(0, cnt);
                DdbVector<char> col14(0, cnt);
                DdbVector<long long> col15(0, cnt);
                DdbVector<int> col16(0, cnt);

                for (uint32_t i = 0; i < cnt; ++i) {
                    col0.add(tick[i].market_type);
                    col1.add(tick[i].security_code);
                    col2.add(convertTime(tick[i].exec_time));
                    col3.add(tick[i].channel_no);
                    col4.add(tick[i].appl_seq_num);
                    col5.add(tick[i].exec_price);
                    col6.add(tick[i].exec_volume);
                    col7.add(tick[i].value_trade);
                    col8.add(tick[i].bid_appl_seq_num);
                    col9.add(tick[i].offer_appl_seq_num);
                    col10.add(tick[i].side);
                    col11.add(tick[i].exec_type);
                    col12.add(tick[i].md_stream_id);
                    col13.add(tick[i].biz_index);
                    col14.add(tick[i].variety_category);

                    if (receivedTimeFlag) {
                        col15.add(startTime);
                    }
                    if(dailyIndexFlagTotal){
                        int dailyIndex = INT_MIN;
                        if(!getDailyIndex(dailyIndex, dailyIndex_, sizeof(dailyIndex_), tick[i].market_type, datatype, tick[i].channel_no, convertTime(tick[i].exec_time))){
                            LOG_ERR("[PluginAmdQute]: getDailyIndex failed. ");
                            return;
                        }
                        col16.add(dailyIndex);
                    }
                }

                vector<ConstantSP> cols;
                cols.push_back(col0.createVector(DT_INT));
                cols.push_back(col1.createVector(DT_SYMBOL));
                cols.push_back(col2.createVector(DT_TIMESTAMP));
                cols.push_back(col3.createVector(DT_INT));
                cols.push_back(col4.createVector(DT_LONG));
                cols.push_back(col5.createVector(DT_LONG));
                cols.push_back(col6.createVector(DT_LONG));
                cols.push_back(col7.createVector(DT_LONG));
                cols.push_back(col8.createVector(DT_LONG));
                cols.push_back(col9.createVector(DT_LONG));
                cols.push_back(col10.createVector(DT_CHAR));
                cols.push_back(col11.createVector(DT_CHAR));
                cols.push_back(col12.createVector(DT_STRING));
                cols.push_back(col13.createVector(DT_LONG));
                cols.push_back(col14.createVector(DT_CHAR));
                if (receivedTimeFlag) {
                    cols.push_back(col15.createVector(DT_NANOTIMESTAMP));
                }
                if(dailyIndexFlagTotal)
                    cols.push_back(col16.createVector(DT_INT)); 
            

                vector<string> colNames = executionTableMeta_.colNames_;
                if(receivedTimeFlag)
                    colNames.push_back("receivedTime");
                if(dailyIndexFlagTotal)
                    colNames.push_back("dailyIndex");
                TableSP data = Util::createTable(colNames, cols);

                vector<ConstantSP> args = {data};
                try{
                    if(!transform.isNull())
                        data = transform->call(session_->getHeap().get(), args);
                }catch(exception &e){
                    throw RuntimeException("call transform error " + string(e.what()));
                }

                if(insertedTable.isNull())
                    throw RuntimeException("insertedTable is null");
                if(insertedTable ->columns() != data->columns())
                    throw RuntimeException("The number of columns of the table to insert must be the same as that of the original table");
                args = {insertedTable, data};
                try{
                session_->getFunctionDef("append!")->call(session_->getHeap().get(), args);
                }
                catch(exception& e){
                    LOG_ERR("[Pluginamd] : append failed: " + string(e.what()));
                }
                
                if (latencyFlag_) { 
                    long long latency = Util::toLocalNanoTimestamp(Util::getNanoEpochTime() - startTime);
                    latencyLog(1, startTime, cnt, latency);
                }
            }
            catch(exception& e){
                LOG_ERR("[PluginAmdQuote]: OnMDTickExecution failed, ", e.what());
            }
        }

       // 接受并处理逐笔成交数据（债券）
        void OnMDBondTickExecutionHelper(amd::ama::MDBondTickExecution* tick, uint32_t cnt)  {
            try{
                if(cnt == 0)return;
                long long startTime = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
                // std::lock_guard<std::mutex> amdLock_(amdMutex_);

                TableSP insertedTable;
                FunctionDefSP transform;
                char varietyCategory = tick[0].variety_category;
                AMDDataType datatype;
                if (varietyCategory == 3) { // 债券逐笔成交
                    insertedTable = bondExecutionData_;
                    transform = bondExecutionTransform_;
                    datatype = AMD_BOND_EXECUTION;
                } else {
                    return ;
                }

                int market_type = tick[0].market_type;
                AMDTableType tableTyeTotal = getAmdTableType(datatype, market_type);
                if(tableTyeTotal == AMD_ERROR_TABLE_TYPE){
                    LOG_ERR("[PluginAmdQuote]: error amd table type AMD_ERROR_TABLE_TYPE");
                    return;
                }

                bool receivedTimeFlag = (varietyCategory ==3 && bondExecutionReceivedTimeFlag_);

                bool dailyIndexFlagTotal = dailyIndex_[tableTyeTotal].getFlag();

                DdbVector<int> col0(0, cnt);
                DdbVector<string> col1(0, cnt);
                DdbVector<long long> col2(0, cnt);
                DdbVector<int> col3(0, cnt);
                DdbVector<long long> col4(0, cnt);
                DdbVector<long long> col5(0, cnt);
                DdbVector<long long> col6(0, cnt);
                DdbVector<long long> col7(0, cnt);
                DdbVector<long long> col8(0, cnt);
                DdbVector<long long> col9(0, cnt);
                DdbVector<char> col10(0, cnt);
                DdbVector<char> col11(0, cnt);
                DdbVector<string> col12(0, cnt);
                DdbVector<long long> col13(0, cnt);
                DdbVector<char> col14(0, cnt);
                DdbVector<long long> col15(0, cnt);
                DdbVector<int> col16(0, cnt);

                for (uint32_t i = 0; i < cnt; ++i) {
                    col0.add(tick[i].market_type);
                    col1.add(tick[i].security_code);
                    col2.add(convertTime(tick[i].exec_time));
                    col3.add(tick[i].channel_no);
                    col4.add(tick[i].appl_seq_num);
                    col5.add(tick[i].exec_price);
                    col6.add(tick[i].exec_volume);
                    col7.add(tick[i].value_trade);
                    col8.add(tick[i].bid_appl_seq_num);
                    col9.add(tick[i].offer_appl_seq_num);
                    col10.add(tick[i].side);
                    col11.add(tick[i].exec_type);
                    col12.add(tick[i].md_stream_id);
                    // col13.add(tick[i].biz_index);
                    col13.add(LONG_LONG_MIN);
                    col14.add(tick[i].variety_category);

                    if (receivedTimeFlag) {
                        col15.add(startTime);
                    }
                    if(dailyIndexFlagTotal){
                        int dailyIndex = INT_MIN;
                        if(!getDailyIndex(dailyIndex, dailyIndex_, sizeof(dailyIndex_), tick[i].market_type, datatype, tick[i].channel_no, convertTime(tick[i].exec_time))){
                            LOG_ERR("[PluginAmdQute]: getDailyIndex failed. ");
                            return;
                        }
                        col16.add(dailyIndex);
                    }
                }

                vector<ConstantSP> cols;
                cols.push_back(col0.createVector(DT_INT));
                cols.push_back(col1.createVector(DT_SYMBOL));
                cols.push_back(col2.createVector(DT_TIMESTAMP));
                cols.push_back(col3.createVector(DT_INT));
                cols.push_back(col4.createVector(DT_LONG));
                cols.push_back(col5.createVector(DT_LONG));
                cols.push_back(col6.createVector(DT_LONG));
                cols.push_back(col7.createVector(DT_LONG));
                cols.push_back(col8.createVector(DT_LONG));
                cols.push_back(col9.createVector(DT_LONG));
                cols.push_back(col10.createVector(DT_CHAR));
                cols.push_back(col11.createVector(DT_CHAR));
                cols.push_back(col12.createVector(DT_STRING));
                cols.push_back(col13.createVector(DT_LONG));
                cols.push_back(col14.createVector(DT_CHAR));
                if (receivedTimeFlag) {
                    cols.push_back(col15.createVector(DT_NANOTIMESTAMP));
                }
                if(dailyIndexFlagTotal)
                    cols.push_back(col16.createVector(DT_INT)); 
            

                vector<string> colNames = executionTableMeta_.colNames_;
                if(receivedTimeFlag)
                    colNames.push_back("receivedTime");
                if(dailyIndexFlagTotal)
                    colNames.push_back("dailyIndex");
                TableSP data = Util::createTable(colNames, cols);

                vector<ConstantSP> args = {data};
                try{
                    if(!transform.isNull())
                        data = transform->call(session_->getHeap().get(), args);
                }catch(exception &e){
                    throw RuntimeException("call transform error " + string(e.what()));
                }

                if(insertedTable.isNull())
                    throw RuntimeException("insertedTable is null");
                if(insertedTable ->columns() != data->columns())
                    throw RuntimeException("The number of columns of the table to insert must be the same as that of the original table");
                args = {insertedTable, data};
                try{
                session_->getFunctionDef("append!")->call(session_->getHeap().get(), args);
                }
                catch(exception& e){
                    LOG_ERR("[Pluginamd] : append failed: " + string(e.what()));
                }
                
                if (latencyFlag_) { 
                    long long latency = Util::toLocalNanoTimestamp(Util::getNanoEpochTime() - startTime);
                    latencyLog(4, startTime, cnt, latency);
                }
            }
            catch(exception& e){
                LOG_ERR("[PluginAmdQuote]: OnMDTickExecution failed, ", e.what());
            }
        }

        // 接受并处理指数快照数据
        void OnMDIndexSnapshotHelper(amd::ama::MDIndexSnapshot* index, uint32_t cnt)  {
            try{
                if(cnt == 0)return;
                long long startTime = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
                // std::lock_guard<std::mutex> amdLock_(amdMutex_);

                AMDDataType datatype = AMD_INDEX;
                int market_type = index[0].market_type;
                AMDTableType tableTyeTotal = getAmdTableType(datatype, market_type);
                if(tableTyeTotal == AMD_ERROR_TABLE_TYPE){
                    LOG_ERR("[PluginAmdQuote]: error amd table type AMD_ERROR_TABLE_TYPE");
                    return;
                }
                bool receivedTimeFlag = indexReceivedTimeFlag_;

                bool dailyIndexFlagTotal = dailyIndex_[tableTyeTotal].getFlag();

                DdbVector<int> col0(0, cnt);
                DdbVector<string> col1(0, cnt);
                DdbVector<long long> col2(0, cnt);
                DdbVector<string> col3(0, cnt);
                DdbVector<long long> col4(0, cnt);
                DdbVector<long long> col5(0, cnt);
                DdbVector<long long> col6(0, cnt);
                DdbVector<long long> col7(0, cnt);
                DdbVector<long long> col8(0, cnt);
                DdbVector<long long> col9(0, cnt);
                DdbVector<long long> col10(0, cnt);
                DdbVector<long long> col11(0, cnt);
                DdbVector<int> col12(0, cnt);
                DdbVector<string> col13(0, cnt);
                DdbVector<char> col14(0, cnt);
                DdbVector<long long> col15(0, cnt);
                DdbVector<int> col16(0, cnt);
                
                for (uint32_t i = 0; i < cnt; i++) {
                    col0.add(index[i].market_type);
                    col1.add(index[i].security_code);
                    col2.add(convertTime(index[i].orig_time));
                    col3.add(index[i].trading_phase_code);
                    col4.add(index[i].pre_close_index);
                    col5.add(index[i].open_index);
                    col6.add(index[i].high_index);
                    col7.add(index[i].low_index);
                    col8.add(index[i].last_index);
                    col9.add(index[i].close_index);
                    col10.add(index[i].total_volume_trade);
                    col11.add(index[i].total_value_trade);
                    col12.add(index[i].channel_no);
                    col13.add(index[i].md_stream_id);
                    col14.add(index[i].variety_category);
                    if (receivedTimeFlag) {
                        col15.add(startTime);
                    }
                    if(dailyIndexFlagTotal){
                        int dailyIndex = INT_MIN;
                        if(!getDailyIndex(dailyIndex, dailyIndex_, sizeof(dailyIndex_), index[i].market_type, datatype, index[i].channel_no, convertTime(index[i].orig_time))){
                            LOG_ERR("[PluginAmdQute]: getDailyIndex failed. ");
                            return;
                        }
                        col16.add(dailyIndex);
                    }
                }

                vector<ConstantSP> cols;
                cols.push_back(col0.createVector(DT_INT));
                cols.push_back(col1.createVector(DT_SYMBOL));
                cols.push_back(col2.createVector(DT_TIMESTAMP));
                cols.push_back(col3.createVector(DT_STRING));
                cols.push_back(col4.createVector(DT_LONG));
                cols.push_back(col5.createVector(DT_LONG));
                cols.push_back(col6.createVector(DT_LONG));
                cols.push_back(col7.createVector(DT_LONG));
                cols.push_back(col8.createVector(DT_LONG));
                cols.push_back(col9.createVector(DT_LONG));
                cols.push_back(col10.createVector(DT_LONG));
                cols.push_back(col11.createVector(DT_LONG));
                cols.push_back(col12.createVector(DT_INT));
                cols.push_back(col13.createVector(DT_STRING));
                cols.push_back(col14.createVector(DT_CHAR));
                if (receivedTimeFlag) {
                    cols.push_back(col15.createVector(DT_NANOTIMESTAMP));
                }
                if(dailyIndexFlagTotal)
                    cols.push_back(col16.createVector(DT_INT)); 
            
                vector<string> colNames = indexTableMeta_.colNames_;
                if(receivedTimeFlag)
                    colNames.push_back("receivedTime");
                if(dailyIndexFlagTotal)
                    colNames.push_back("dailyIndex");
                TableSP data = Util::createTable(colNames, cols);

                vector<ConstantSP> args = {data};
                try{
                    if(!indexTransform_.isNull())
                        data = indexTransform_->call(session_->getHeap().get(), args);
                }catch(exception &e){
                    throw RuntimeException("call transform error " + string(e.what()));
                }

                if(indexData_.isNull())
                    throw RuntimeException("insertedTable is null");
                if(indexData_ ->columns() != data->columns())
                    throw RuntimeException("The number of columns of the table to insert must be the same as that of the original table");
                args = {indexData_, data};
                session_->getFunctionDef("append!")->call(session_->getHeap().get(), args);
            }
            catch(exception& e){
                LOG_ERR("[PlguinAmdQuote]: OnMDIndexSnapshot failed, ", e.what());
            }
        }
        
        // 接受并处理委托队列数据
        void OnMDOrderQueueHelper(amd::ama::MDOrderQueue* queue, uint32_t cnt) {
            try{
                if(cnt == 0)return;
                long long startTime = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
                // std::lock_guard<std::mutex> amdLock_(amdMutex_);
                AMDDataType datatype = AMD_ORDER_QUEUE;
                int market_type = queue[0].market_type;
                AMDTableType tableTyeTotal = getAmdTableType(datatype, market_type);
                if(tableTyeTotal == AMD_ERROR_TABLE_TYPE){
                    LOG_ERR("[PluginAmdQuote]: error amd table type AMD_ERROR_TABLE_TYPE");
                    return;
                }
                bool receivedTimeFlag = orderQueueReceivedTimeFlag_;
                bool dailyIndexFlagTotal = dailyIndex_[tableTyeTotal].getFlag();

                DdbVector<int> col0(0, cnt);
                DdbVector<string> col1(0, cnt);
                DdbVector<long long> col2(0, cnt);
                DdbVector<char> col3(0, cnt);
                DdbVector<long long> col4(0, cnt);
                DdbVector<long long> col5(0, cnt);
                DdbVector<int> col6(0, cnt);
                DdbVector<int> col7(0, cnt);

                DdbVector<long long> col8(0, cnt);
                DdbVector<long long> col9(0, cnt);
                DdbVector<long long> col10(0, cnt);
                DdbVector<long long> col11(0, cnt);
                DdbVector<long long> col12(0, cnt);
                DdbVector<long long> col13(0, cnt);
                DdbVector<long long> col14(0, cnt);
                DdbVector<long long> col15(0, cnt);
                DdbVector<long long> col16(0, cnt);
                DdbVector<long long> col17(0, cnt);

                DdbVector<long long> col18(0, cnt);
                DdbVector<long long> col19(0, cnt);
                DdbVector<long long> col20(0, cnt);
                DdbVector<long long> col21(0, cnt);
                DdbVector<long long> col22(0, cnt);
                DdbVector<long long> col23(0, cnt);
                DdbVector<long long> col24(0, cnt);
                DdbVector<long long> col25(0, cnt);
                DdbVector<long long> col26(0, cnt);
                DdbVector<long long> col27(0, cnt);

                DdbVector<long long> col28(0, cnt);
                DdbVector<long long> col29(0, cnt);
                DdbVector<long long> col30(0, cnt);
                DdbVector<long long> col31(0, cnt);
                DdbVector<long long> col32(0, cnt);
                DdbVector<long long> col33(0, cnt);
                DdbVector<long long> col34(0, cnt);
                DdbVector<long long> col35(0, cnt);
                DdbVector<long long> col36(0, cnt);
                DdbVector<long long> col37(0, cnt);

                DdbVector<long long> col38(0, cnt);
                DdbVector<long long> col39(0, cnt);
                DdbVector<long long> col40(0, cnt);
                DdbVector<long long> col41(0, cnt);
                DdbVector<long long> col42(0, cnt);
                DdbVector<long long> col43(0, cnt);
                DdbVector<long long> col44(0, cnt);
                DdbVector<long long> col45(0, cnt);
                DdbVector<long long> col46(0, cnt);
                DdbVector<long long> col47(0, cnt);

                DdbVector<long long> col48(0, cnt);
                DdbVector<long long> col49(0, cnt);
                DdbVector<long long> col50(0, cnt);
                DdbVector<long long> col51(0, cnt);
                DdbVector<long long> col52(0, cnt);
                DdbVector<long long> col53(0, cnt);
                DdbVector<long long> col54(0, cnt);
                DdbVector<long long> col55(0, cnt);
                DdbVector<long long> col56(0, cnt);
                DdbVector<long long> col57(0, cnt);

                DdbVector<int> col58(0, cnt);
                DdbVector<string> col59(0, cnt);
                DdbVector<char> col60(0, cnt);
                DdbVector<long long> col61(0, cnt);
                DdbVector<int> col62(0, cnt);

                for (uint32_t i = 0; i < cnt; ++i) {
                    col0.add(queue[i].market_type);
                    col1.add(queue[i].security_code);
                    col2.add(convertTime(queue[i].order_time));
                    col3.add(queue[i].side);
                    col4.add(queue[i].order_price);
                    col5.add(queue[i].order_volume);
                    col6.add(queue[i].num_of_orders);
                    col7.add(queue[i].items);

                    col8.add(queue[i].volume[0]);
                    col9.add(queue[i].volume[1]);
                    col10.add(queue[i].volume[2]);
                    col11.add(queue[i].volume[3]);
                    col12.add(queue[i].volume[4]);
                    col13.add(queue[i].volume[5]);
                    col14.add(queue[i].volume[6]);
                    col15.add(queue[i].volume[7]);
                    col16.add(queue[i].volume[8]);
                    col17.add(queue[i].volume[9]);

                    col18.add(queue[i].volume[10]);
                    col19.add(queue[i].volume[11]);
                    col20.add(queue[i].volume[12]);
                    col21.add(queue[i].volume[13]);
                    col22.add(queue[i].volume[14]);
                    col23.add(queue[i].volume[15]);
                    col24.add(queue[i].volume[16]);
                    col25.add(queue[i].volume[17]);
                    col26.add(queue[i].volume[18]);
                    col27.add(queue[i].volume[19]);

                    col28.add(queue[i].volume[20]);
                    col29.add(queue[i].volume[21]);
                    col30.add(queue[i].volume[22]);
                    col31.add(queue[i].volume[23]);
                    col32.add(queue[i].volume[24]);
                    col33.add(queue[i].volume[25]);
                    col34.add(queue[i].volume[26]);
                    col35.add(queue[i].volume[27]);
                    col36.add(queue[i].volume[28]);
                    col37.add(queue[i].volume[29]);

                    col38.add(queue[i].volume[30]);
                    col39.add(queue[i].volume[31]);
                    col40.add(queue[i].volume[32]);
                    col41.add(queue[i].volume[33]);
                    col42.add(queue[i].volume[34]);
                    col43.add(queue[i].volume[35]);
                    col44.add(queue[i].volume[36]);
                    col45.add(queue[i].volume[37]);
                    col46.add(queue[i].volume[38]);
                    col47.add(queue[i].volume[39]);

                    col48.add(queue[i].volume[40]);
                    col49.add(queue[i].volume[41]);
                    col50.add(queue[i].volume[42]);
                    col51.add(queue[i].volume[43]);
                    col52.add(queue[i].volume[44]);
                    col53.add(queue[i].volume[45]);
                    col54.add(queue[i].volume[46]);
                    col55.add(queue[i].volume[47]);
                    col56.add(queue[i].volume[48]);
                    col57.add(queue[i].volume[49]);

                    col58.add(queue[i].channel_no);
                    col59.add(queue[i].md_stream_id);
                    col60.add(queue[i].variety_category);
                    if (receivedTimeFlag) {
                        col61.add(startTime);
                    }
                    if(dailyIndexFlagTotal){
                        int dailyIndex = INT_MIN;
                        if(!getDailyIndex(dailyIndex, dailyIndex_, sizeof(dailyIndex_), queue[i].market_type, datatype, queue[i].channel_no, convertTime(queue[i].order_time))){
                            LOG_ERR("[PluginAmdQute]: getDailyIndex failed. ");
                            return;
                        }
                        col62.add(dailyIndex);
                    }
                }

                vector<ConstantSP> cols;
                cols.push_back(col0.createVector(DT_INT));
                cols.push_back(col1.createVector(DT_SYMBOL));
                cols.push_back(col2.createVector(DT_TIMESTAMP));
                cols.push_back(col3.createVector(DT_CHAR));
                cols.push_back(col4.createVector(DT_LONG));
                cols.push_back(col5.createVector(DT_LONG));
                cols.push_back(col6.createVector(DT_INT));
                cols.push_back(col7.createVector(DT_INT));

                cols.push_back(col8.createVector(DT_LONG));
                cols.push_back(col9.createVector(DT_LONG));
                cols.push_back(col10.createVector(DT_LONG));
                cols.push_back(col11.createVector(DT_LONG));
                cols.push_back(col12.createVector(DT_LONG));
                cols.push_back(col13.createVector(DT_LONG));
                cols.push_back(col14.createVector(DT_LONG));
                cols.push_back(col15.createVector(DT_LONG));
                cols.push_back(col16.createVector(DT_LONG));
                cols.push_back(col17.createVector(DT_LONG));

                cols.push_back(col18.createVector(DT_LONG));
                cols.push_back(col19.createVector(DT_LONG));
                cols.push_back(col20.createVector(DT_LONG));
                cols.push_back(col21.createVector(DT_LONG));
                cols.push_back(col22.createVector(DT_LONG));
                cols.push_back(col23.createVector(DT_LONG));
                cols.push_back(col24.createVector(DT_LONG));
                cols.push_back(col25.createVector(DT_LONG));
                cols.push_back(col26.createVector(DT_LONG));
                cols.push_back(col27.createVector(DT_LONG));

                cols.push_back(col28.createVector(DT_LONG));
                cols.push_back(col29.createVector(DT_LONG));
                cols.push_back(col30.createVector(DT_LONG));
                cols.push_back(col31.createVector(DT_LONG));
                cols.push_back(col32.createVector(DT_LONG));
                cols.push_back(col33.createVector(DT_LONG));
                cols.push_back(col34.createVector(DT_LONG));
                cols.push_back(col35.createVector(DT_LONG));
                cols.push_back(col36.createVector(DT_LONG));
                cols.push_back(col37.createVector(DT_LONG));

                cols.push_back(col38.createVector(DT_LONG));
                cols.push_back(col39.createVector(DT_LONG));
                cols.push_back(col40.createVector(DT_LONG));
                cols.push_back(col41.createVector(DT_LONG));
                cols.push_back(col42.createVector(DT_LONG));
                cols.push_back(col43.createVector(DT_LONG));
                cols.push_back(col44.createVector(DT_LONG));
                cols.push_back(col45.createVector(DT_LONG));
                cols.push_back(col46.createVector(DT_LONG));
                cols.push_back(col47.createVector(DT_LONG));

                cols.push_back(col48.createVector(DT_LONG));
                cols.push_back(col49.createVector(DT_LONG));
                cols.push_back(col50.createVector(DT_LONG));
                cols.push_back(col51.createVector(DT_LONG));
                cols.push_back(col52.createVector(DT_LONG));
                cols.push_back(col53.createVector(DT_LONG));
                cols.push_back(col54.createVector(DT_LONG));
                cols.push_back(col55.createVector(DT_LONG));
                cols.push_back(col56.createVector(DT_LONG));
                cols.push_back(col57.createVector(DT_LONG));

                cols.push_back(col58.createVector(DT_INT));
                cols.push_back(col59.createVector(DT_STRING));
                cols.push_back(col60.createVector(DT_CHAR));
                if (receivedTimeFlag) {
                    cols.push_back(col61.createVector(DT_NANOTIMESTAMP));
                }
                if(dailyIndexFlagTotal)
                    cols.push_back(col62.createVector(DT_INT)); 
                
                vector<string> colNames = orderQueueMeta_.colNames_;
                if(receivedTimeFlag)
                    colNames.push_back("receivedTime");
                if(dailyIndexFlagTotal)
                    colNames.push_back("dailyIndex");
                TableSP data = Util::createTable(colNames, cols);

                vector<ConstantSP> args = {data};
                try{
                    if(!orderQueueTransform_.isNull())
                        data = orderQueueTransform_->call(session_->getHeap().get(), args);
                }catch(exception &e){
                    throw RuntimeException("call transform error " + string(e.what()));
                }

                if(orderQueueData_.isNull())
                    throw RuntimeException("insertedTable is null");
                if(orderQueueData_ ->columns() != data->columns())
                    throw RuntimeException("The number of columns of the table to insert must be the same as that of the original table");
                args = {orderQueueData_, data};
                session_->getFunctionDef("append!")->call(session_->getHeap().get(), args);
            }
            catch(exception &e){
                LOG_ERR("[PluginAmdQuote]: OnMDOrderQueue failed, ", e.what());
            }
        }

        void pushSnashotDadata(amd::ama::MDSnapshot* snapshot, uint32_t cnt){
            for(uint32_t i = 0; i < cnt; ++i){
                snapshotBoundQueue_->blockingPush(snapshot[i]);
            }
        }

        void pushOrderDadata(amd::ama::MDTickOrder* ticks, uint32_t cnt){
            for(uint32_t i = 0; i < cnt; ++i){
                orderBoundQueue_->blockingPush(ticks[i]);
            }
        }

        void pushExecutionData(amd::ama::MDTickExecution* tick, uint32_t cnt){
            for(uint32_t i = 0; i < cnt; ++i){
                executionBoundQueue_->blockingPush(tick[i]);
            }
        }

        void pushIndexData(amd::ama::MDIndexSnapshot* index, uint32_t cnt){
            for(uint32_t i = 0; i < cnt; ++i){
                indexBoundQueue_->blockingPush(index[i]);
            }
        }

        void pushOrderQueueData(amd::ama::MDOrderQueue* queue, uint32_t cnt){
            for(uint32_t i = 0; i < cnt; ++i){
                orderQueueBoundQueue_->blockingPush(queue[i]);
            }
        }

        void pushBondSnapshotData(amd::ama::MDBondSnapshot* snapshots, uint32_t cnt){
            for(uint32_t i = 0; i < cnt; ++i){
                bondSnapshotBoundQueue_->blockingPush(snapshots[i]);
            }
        }

        void pushBondOrderData(amd::ama::MDBondTickOrder* ticks, uint32_t cnt){
            for(uint32_t i = 0; i < cnt; ++i){
                bondOrderBoundQueue_->blockingPush(ticks[i]);
            }
        }

        void pushBondExecutionData(amd::ama::MDBondTickExecution* ticks, uint32_t cnt){
            for(uint32_t i = 0; i < cnt; ++i){
                bondExecutionBoundQueue_->blockingPush(ticks[i]);
            }
        }

        // 接受并处理快照数据（包含基金和股票数据）
        virtual void OnMDSnapshot(amd::ama::MDSnapshot* snapshot, uint32_t cnt) override{
            Defer df([=](){amd::ama::IAMDApi::FreeMemory(snapshot);});
            pushSnashotDadata(snapshot, cnt);
        }
        // 接受并处理逐笔委托数据（包含基金和股票数据）
        virtual void OnMDTickOrder(amd::ama::MDTickOrder* ticks, uint32_t cnt) override{
            Defer df([=](){amd::ama::IAMDApi::FreeMemory(ticks);});
            pushOrderDadata(ticks, cnt);
        }
        // 接受并处理逐笔成交数据（包含基金和股票数据）
        virtual void OnMDTickExecution(amd::ama::MDTickExecution* tick, uint32_t cnt) override{
            Defer df([=](){amd::ama::IAMDApi::FreeMemory(tick);});
            pushExecutionData(tick, cnt);
        }
        // 接受并处理指数快照数据
        virtual void OnMDIndexSnapshot(amd::ama::MDIndexSnapshot* index, uint32_t cnt) override{
            Defer df([=](){amd::ama::IAMDApi::FreeMemory(index);});
            pushIndexData(index, cnt);
        }
        // 接受并处理委托队列数据
        virtual void OnMDOrderQueue(amd::ama::MDOrderQueue* queue, uint32_t cnt) override{
            Defer df([=](){amd::ama::IAMDApi::FreeMemory(queue);});
            pushOrderQueueData(queue, cnt);
        }

        virtual void OnMDBondSnapshot(amd::ama::MDBondSnapshot* snapshots, uint32_t cnt) {
            Defer df([=](){amd::ama::IAMDApi::FreeMemory(snapshots);});
            pushBondSnapshotData(snapshots, cnt);
        } 
        virtual void OnMDBondTickOrder(amd::ama::MDBondTickOrder* ticks, uint32_t cnt) {
            Defer df([=](){amd::ama::IAMDApi::FreeMemory(ticks);});
            pushBondOrderData(ticks, cnt);
        } 
        virtual void OnMDBondTickExecution(amd::ama::MDBondTickExecution* ticks, uint32_t cnt) {
            Defer df([=](){amd::ama::IAMDApi::FreeMemory(ticks);});
            pushBondExecutionData(ticks, cnt);
        } 

        virtual void OnEvent(uint32_t level, uint32_t code, const char* event_msg, uint32_t len)
        {
            static unordered_set<string> errorSet;
            string codeString = amd::ama::Tools::GetEventCodeString(code);
            std::lock_guard<std::mutex> amdLock_(amdMutex_);
            if(codeString.find("Failed") != string::npos){
                if(errorSet.count(event_msg) == 0){
                    errorSet.insert(event_msg);
                    LOG_ERR("[PluginAmdQuote]: AMA event: " + codeString);
                    LOG_INFO("[PluginAmdQuote] AMA event: ", std::string(event_msg));
                }
            }else{
                codeString.clear();
                LOG_INFO("[PluginAmdQuote] AMA event: ", std::string(event_msg));    
            }

        }

        void latencyLog(int type, long long startTime, uint32_t cnt, long long latency) { // 0: snapshot, 1: execution, 2: order
            Statistic& stat = statistics_[type];
            if (stat.startTime != 0 && startTime > stat.endTime) { // 大于30s，打日志
                if(stat.totalHandleCount > 0){
                        LOG_INFO(       "type ", type,
                                        ", statistics: total message count ", stat.totalMessageCount,
                                        ", total latency(ns) ", stat.totalLatency,
                                        ", min latency(ns) ", stat.minLatency,
                                        ", max latency(ns) ", stat.maxLatency,
                                        ", average latency(ns) ", stat.totalLatency / stat.totalHandleCount,
                                        ", average message latency(ns) ", stat.totalLatency / stat.totalMessageCount,
                                        ", total handle count", stat.totalHandleCount);
                }
                goto setValue;
            } else if (stat.startTime == 0) {
            setValue:
                stat.startTime = startTime;
                stat.endTime = stat.startTime  + duration_;
                stat.totalMessageCount = cnt;
                stat.totalHandleCount = 1;
                stat.maxLatency = stat.minLatency = stat.totalLatency = latency;
            } else  {
                if(stat.maxLatency < latency)
                    stat.maxLatency = latency;
                if(stat.minLatency > latency)
                    stat.minLatency = latency;
                stat.totalLatency += latency;
                stat.totalHandleCount += 1;
                stat.totalMessageCount += cnt;
            }
        }

        void setSnapshotData(TableSP snapshotData, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp, int market) {
            snapshotData_ = snapshotData;
            snapshotTransform_ = transform;
            snapshotReceivedTimeFlag_ = receivedTimeFlag;
            if(market == 101)
                dailyIndex_[AMD_SNAPSHOT_SH] = dailyStartTimestamp;
            else if(market == 102)
                dailyIndex_[AMD_SNAPSHOT_SZ] = dailyStartTimestamp;
        }

        void setExecutionData(TableSP executionData, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp, int market) {
            executionData_ = executionData;
            executionTransform_ = transform;
            executioReceivedTimeFlag_ = receivedTimeFlag;
            if(market == 101)
                dailyIndex_[AMD_EXECUTION_SH] = dailyStartTimestamp;
            else if(market == 102)
                dailyIndex_[AMD_EXECUTION_SZ] = dailyStartTimestamp;
        }

        void setOrderData(TableSP orderData, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp, int market) {
            orderData_ = orderData;
            orderTransform_ = transform;
            orderReceivedTimeFlag_ = receivedTimeFlag;
            if(market == 101)
                dailyIndex_[AMD_ORDER_SH] = dailyStartTimestamp;
            else if(market == 102)
                dailyIndex_[AMD_ORDER_SZ] = dailyStartTimestamp;
        }

        void setIndexData(TableSP indexData, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp, int market) {
            indexData_ = indexData;
            indexTransform_ = transform;
            indexReceivedTimeFlag_ = receivedTimeFlag;
            if(market == 101)
                dailyIndex_[AMD_INDEX_SH] = dailyStartTimestamp;
            else if(market == 102)
                dailyIndex_[AMD_INDEX_SZ] = dailyStartTimestamp;
        }

        void setOrderQueueData(TableSP orderQueueData, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp, int market) {
            orderQueueData_ = orderQueueData;
            orderQueueTransform_ = transform;
            orderQueueReceivedTimeFlag_ = receivedTimeFlag;
            if(market == 101)
                dailyIndex_[AMD_ORDER_QUEUE_SH] = dailyStartTimestamp;
            else if(market == 102)
                dailyIndex_[AMD_ORDER_QUEUE_SZ] = dailyStartTimestamp;
        }

        void setFundSnapshotData(TableSP snapshotData, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp, int market) {
            fundSnapshotData_ = snapshotData;
            fundSnapshotTransform_ = transform;
            fundSnapshotReceivedTimeFlag_ = receivedTimeFlag;
            if(market == 101)
                dailyIndex_[AMD_FUND_SNAPSHOT_SH] = dailyStartTimestamp;
            else if(market == 102)
                dailyIndex_[AMD_FUND_SNAPSHOT_SZ] = dailyStartTimestamp;
        }

        void setFundExecutionData(TableSP executionData, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp, int market) {
            fundExecutionData_ = executionData;
            fundExecutionTransform_ = transform;
            fundExecutionReceivedTimeFlag_ = receivedTimeFlag;
            if(market == 101)
                dailyIndex_[AMD_FUND_EXECUTION_SH] = dailyStartTimestamp;
            else if(market == 102)
                dailyIndex_[AMD_FUND_EXECUTION_SZ] = dailyStartTimestamp;
        }

        void setFundOrderData(TableSP orderData, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp, int market) {
            fundOrderData_ = orderData;
            fundOrderTransform_ = transform;
            fundOrderReceivedTimeFlag_ = receivedTimeFlag;
            if(market == 101)
                dailyIndex_[AMD_FUND_ORDER_SH] = dailyStartTimestamp;
            else if(market == 102)
                dailyIndex_[AMD_FUND_ORDER_SZ] = dailyStartTimestamp;
        }

        void setBondSnapshotData(TableSP snapshotData, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp, int market) {
            bondSnapshotData_ = snapshotData;
            bondSnapshotTransform_ = transform;
            bondSnapshotReceivedTimeFlag_ = receivedTimeFlag;
            if(market == 101)
                dailyIndex_[AMD_BOND_SNAPSHOT_SH] = dailyStartTimestamp;
            else if(market == 102)
                dailyIndex_[AMD_BOND_SNAPSHOT_SZ] = dailyStartTimestamp;
        }

        void setBondExecutionData(TableSP executionData, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp, int market) {
            bondExecutionData_ = executionData;
            bondExecutionTransform_ = transform;
            bondExecutionReceivedTimeFlag_ = receivedTimeFlag;
            if(market == 101)
                dailyIndex_[AMD_BOND_EXECUTION_SH] = dailyStartTimestamp;
            else if(market == 102)
                dailyIndex_[AMD_BOND_EXECUTION_SZ] = dailyStartTimestamp;
        }

        void setBondOrderData(TableSP orderData, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp, int market) {
            bondOrderData_ = orderData;
            bondOrderTransform_ = transform;
            bondOrderReceivedTimeFlag_ = receivedTimeFlag;
            if(market == 101)
                dailyIndex_[AMD_BOND_ORDER_SH] = dailyStartTimestamp;
            else if(market == 102)
                dailyIndex_[AMD_BOND_ORDER_SZ] = dailyStartTimestamp;
        }

        TableSP getData(int type) {
            if (type == 0) {
                return snapshotData_;
            } else if (type == 1) {
                return executionData_;
            } else {
                return orderData_;
            }
        }

        void setLatencyFlag(bool flag) {
            latencyFlag_ = flag;
        }

        long long getDailyIndexStarTime(const string& dataType, int market){
            AMDDataType amdDataType;
            if(nameType.count(dataType) != 0){
                amdDataType = nameType[dataType];
            }
            else
                throw IllegalArgumentException(__FUNCTION__, "error dataType: " + dataType);
            AMDTableType tableType = getAmdTableType(amdDataType, market);
            if((int)tableType == (int)AMD_ERROR_TABLE_TYPE){
                throw RuntimeException("getAmdTableType failed");
            }
            if(tableType >= sizeof(dailyIndex_)){
                throw RuntimeException("tableType exceeds the size of DailyIndex_");
            }
            return dailyIndex_[tableType].getStartTimestamp();
        }

    private:
        AmdSnapshotTableMeta  snapshotDataTableMeta_;
        AmdExecutionTableMeta executionTableMeta_;
        AmdOrderTableMeta  orderTableMeta_;
        AmdIndexTableMeta indexTableMeta_;
        AmdOrderQueueTableMeta orderQueueMeta_;

        TableSP snapshotData_; // 保存股票快照数据, 建议定义成流表
        TableSP executionData_; // 保存股票逐笔委托, 建议定义成流表
        TableSP orderData_; // 保存股票逐笔成交, 建议定义成流表
        TableSP indexData_; // 保存指数快照数据, 建议定义成流表
        TableSP orderQueueData_; // 保存委托队列数据, 建议定义成流表
        TableSP fundSnapshotData_; // 保存基金快照数据, 建议定义成流表
        TableSP fundExecutionData_; // 保存基金逐笔委托, 建议定义成流表
        TableSP fundOrderData_; // 保存基金逐笔成交, 建议定义成流表
        TableSP bondSnapshotData_;
        TableSP bondExecutionData_;
        TableSP bondOrderData_;

        bool snapshotReceivedTimeFlag_; 
        bool executioReceivedTimeFlag_; 
        bool orderReceivedTimeFlag_; 
        bool indexReceivedTimeFlag_; 
        bool orderQueueReceivedTimeFlag_; 
        bool fundSnapshotReceivedTimeFlag_; 
        bool fundExecutionReceivedTimeFlag_; 
        bool fundOrderReceivedTimeFlag_; 
        bool bondSnapshotReceivedTimeFlag_;
        bool bondExecutionReceivedTimeFlag_;
        bool bondOrderReceivedTimeFlag_;

        FunctionDefSP snapshotTransform_; 
        FunctionDefSP executionTransform_; 
        FunctionDefSP orderTransform_; 
        FunctionDefSP indexTransform_; 
        FunctionDefSP orderQueueTransform_; 
        FunctionDefSP fundSnapshotTransform_; 
        FunctionDefSP fundExecutionTransform_;
        FunctionDefSP fundOrderTransform_; 
        FunctionDefSP bondSnapshotTransform_;
        FunctionDefSP bondExecutionTransform_;
        FunctionDefSP bondOrderTransform_;

        ThreadSP snapshotThread_;
        ThreadSP executionThread_;
        ThreadSP orderThread_;
        ThreadSP indexThread_;
        ThreadSP orderQueueThread_;
        ThreadSP bondSnapshotThread_;
        ThreadSP bondOrderThread_;
        ThreadSP bondExecutionThread_;

        SmartPointer<SnapshotQueue> snapshotBoundQueue_;
        SmartPointer<OrderQueue> orderBoundQueue_;
        SmartPointer<ExecutionQueue> executionBoundQueue_;
        SmartPointer<BondSnapshotQueue> bondSnapshotBoundQueue_;
        SmartPointer<BondOrderQueue> bondOrderBoundQueue_;
        SmartPointer<BondExecutionQueue> bondExecutionBoundQueue_;
        SmartPointer<IndexQueue> indexBoundQueue_;
        SmartPointer<OrderQueueQueue> orderQueueBoundQueue_;
        
        SmartPointer<bool> stopFlag_;
        
        DailyIndex dailyIndex_[22];

        struct Statistic {
            long long startTime = 0; // 起始时间（读到第一条数据的时间）
            long long endTime = 0; // 统计结束时间
            long long totalLatency = 0; // 总时间。单位纳秒
            long long maxLatency = 0; // 单条最大延迟
            long long minLatency = 0; // 单条最大延迟
            long totalMessageCount = 0; // 总消息量
            long totalHandleCount = 0; // 处理数据次数，即调用了多少次回调函数
        };
        Statistic statistics_[6]; // 分别对应snapshot, execution, order
        const long long duration_ = 30 * (long long)1000000000;
        SessionSP session_;

        bool latencyFlag_ = false; // 分别对应snapshot, execution, order
    };

public:
    AMDSpiImp* getAMDSpi(){
        return amdSpi_;
    }
    static std::mutex amdMutex_;
    unordered_set<Info, InfoHash, InfoEqual> infoSet_;// 0: snapshot, 1: execution, 2: order
private:
    amd::ama::Cfg cfg_;
    AMDSpiImp* amdSpi_;
    std::string username_;
    std::string password_;
    std::vector<std::string> ips_;
    std::vector<int> ports_;

    static AmdQuote* instance_;
};

// 根据参数构造AmdQuote对象，并返回对象，后续接口第一个参数需要传这个对象
extern "C" ConstantSP amdConnect(Heap *heap, vector<ConstantSP> &arguments);
// 根据传入行情数据类型订阅行情数据，返回空
extern "C" ConstantSP subscribe(Heap *heap, vector<ConstantSP> &arguments);
// 取消订阅行情数据，返回空
extern "C" ConstantSP unsubscribe(Heap *heap, vector<ConstantSP> &arguments);
// 手动释放资源
extern "C" ConstantSP amdClose(Heap *heap, vector<ConstantSP> &arguments);
// 根据传入行情数据类型, 返回对应的表结构
extern "C" ConstantSP getSchema(Heap *heap, vector<ConstantSP> &arguments);
// 运维函数，获取当前连接状态
extern "C" ConstantSP getStatus(Heap *heap, vector<ConstantSP> &arguments);
// 打开延迟统计功能
extern "C" ConstantSP enableLatencyStatistics(Heap *heap, vector<ConstantSP> &arguments);

// 内部函数， 非插件接口处理断开连接 清理资源
extern "C" void closeAmd(Heap *heap, vector<ConstantSP> &arguments);

extern "C" ConstantSP getCodeList(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP getETFCodeList(Heap *heap, vector<ConstantSP> &arguments);


#endif