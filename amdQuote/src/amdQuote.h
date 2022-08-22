#ifndef __AMD_QUOTE_H
#define __AMD_QUOTE_H
#include "CoreConcept.h"
#include "amdQuoteType.h"
#include "Logger.h"
#include "Util.h"
#include "ama.h"
#include <mutex>
#include "ScalarImp.h"

inline long long countTemporalUnit(int days, long long multiplier, long long remainder){
	return days == INT_MIN ? LLONG_MIN : days * multiplier + remainder;
}

long long convertTime(long long time) {
    long long  year, month, day, hour, minute, second, milliSecond;
    milliSecond = time % 1000;
    second = time / 1000 % 100;
    minute = time / 1000 / 100 % 100;
    hour = time / 1000 / 100 / 100 % 100;
    day = time / 1000 / 100 / 100 / 100 % 100;
    month = time / 1000 / 100 / 100 / 100 / 100 % 100;
    year = time / 1000 / 100 / 100 / 100 / 100 / 100;
    return countTemporalUnit(Util::countDays(year,month,day), 86400000ll, ((hour*60+minute)*60+second)*1000ll+milliSecond);
}

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

    void enableLatencyStatistics() {
        amdSpi_->setLatencyFlag();
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
    void subscribeSnapshot(int market, TableSP table, std::vector<std::string> codeList) {
        std::vector<std::string> nullCodeList;
        unsubscribe("snapshot", market, nullCodeList);
        amdSpi_->setSnapshotData(table);
        // 选择某个市场，如果codeList size为0 代表订阅全市场， 多个市场需要调用多次
        unsigned int codeSize = 1;
        if (codeList.size() > 0) {
            codeSize = codeList.size();
        }

        amd::ama::SubscribeCategoryItem sub[codeSize];
        memset(sub, 0, sizeof(sub));

        /*
        if (market < (int)amd::ama::MarketType::kNone || market > (int)amd::ama::MarketType::kMax) {
            LOG_ERR("subscribe Snapshot err, illegal market:", market);
            return snapshotData_;
        }*/

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
            amd::ama::IAMDApi::Release();
            LOG_ERR("subscribe Snapshot err, market:", market);
        }
        Info info("snapshot", market);
        infoSet_.insert(info);
    }

    // 订阅逐笔成交
    void subscribeExecution(int market, TableSP table, std::vector<std::string> codeList) {
        std::vector<std::string> nullCodeList;
        unsubscribe("execution", market, nullCodeList);
        amdSpi_->setExecutionData(table);
        // 选择某个市场，如果codeList size为0 代表订阅全市场，多个市场需要调用多次
        unsigned int codeSize = 1;
        if (codeList.size() > 0) {
            codeSize = codeList.size();
        }

        amd::ama::SubscribeCategoryItem sub[codeSize];
        memset(sub, 0, sizeof(sub));

        /*
        if (market < (int)amd::ama::MarketType::kNone || market > (int)amd::ama::MarketType::kMax) {
            LOG_ERR("subscribe Execution err, illegal market:", market);
            return executionData_;
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
                amd::ama::SubscribeType::kAdd, sub, codeSize)
        != amd::ama::ErrorCode::kSuccess)
        {
            std::lock_guard<std::mutex> amdLock_(amdMutex_);
            amd::ama::IAMDApi::Release();
            LOG_ERR("subscribe Execution err, market:", market);
        }
        Info info("execution", market);
        infoSet_.insert(info);
    }

    // 订阅逐笔委托
    void subscribeOrder(int market, TableSP table, std::vector<std::string> codeList) {
        std::vector<std::string> nullCodeList;
        unsubscribe("order", market, nullCodeList);
        amdSpi_->setOrderData(table);
        // 选择某个市场，如果codeList size为0 代表订阅全市场， 多个市场需要调用多次
        unsigned int codeSize = 1;
        if (codeList.size() > 0) {
            codeSize = codeList.size();
        }

        amd::ama::SubscribeCategoryItem sub[codeSize];
        memset(sub, 0, sizeof(sub));
        /*
        if (market < (int)amd::ama::MarketType::kNone || market > (int)amd::ama::MarketType::kMax) {
            LOG_ERR("subscribe Order err, illegal market:", market);
            return orderData_;
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
                amd::ama::SubscribeType::kAdd, sub, codeSize)
        != amd::ama::ErrorCode::kSuccess)
        {
            std::lock_guard<std::mutex> amdLock_(amdMutex_);
            amd::ama::IAMDApi::Release();
            LOG_ERR("subscribe Order err, market:", market);
        }
        Info info("order", market);
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

            amd::ama::SubscribeCategoryItem sub[codeSize];
            memset(sub, 0, sizeof(sub));

            /*
            if (market < (int)amd::ama::MarketType::kNone || market > (int)amd::ama::MarketType::kMax) {
                LOG_ERR("subscribe Snapshot err, illegal market:", market);
                return;
            }*/

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

            amd::ama::SubscribeCategoryItem sub[codeSize];
            memset(sub, 0, sizeof(sub));

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

            amd::ama::SubscribeCategoryItem sub[codeSize];
            memset(sub, 0, sizeof(sub));

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
        } else if (dataType == "all") {
            amd::ama::SubscribeCategoryItem sub[1];
            memset(sub, 0, sizeof(sub));
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
        cancelAll();
        amd::ama::IAMDApi::Release();
    }


private:
    void cancelAll() {
        std::vector<std::string> codeList;
        unsubscribe("all", 0, codeList);
    }
    class AMDSpiImp : public amd::ama::IAMDSpi {
    public:
        AMDSpiImp(SessionSP session) : session_(session) {}

        // 接受并处理快照数据
        virtual void OnMDSnapshot(amd::ama::MDSnapshot* snapshot, uint32_t cnt) override {
            // FIXME
            // if (enableLog_[0] == true) {
            //     latencyLog(0);
            // }
            
            long long startTime = Util::getNanoEpochTime();
            std::lock_guard<std::mutex> amdLock_(amdMutex_);

            std::vector<ConstantSP> cols;
            for (unsigned int snapshotIndex = 0; snapshotIndex < snapshotDataTableMeta_.colTypes_.size(); snapshotIndex++) {
                cols.push_back(Util::createVector(snapshotDataTableMeta_.colTypes_[snapshotIndex], 0, cnt));
            }

            std::vector<int> col0;
            std::vector<string> col1;
            std::vector<long long> col2;
            std::vector<string> col3;
            std::vector<long long> col4;
            std::vector<long long> col5;
            std::vector<long long> col6;
            std::vector<long long> col7;
            std::vector<long long> col8;
            std::vector<long long> col9;

            std::vector<long long> col10;
            std::vector<long long> col11;
            std::vector<long long> col12;
            std::vector<long long> col13;
            std::vector<long long> col14;
            std::vector<long long> col15;
            std::vector<long long> col16;
            std::vector<long long> col17;
            std::vector<long long> col18;
            std::vector<long long> col19;

            std::vector<long long> col20;
            std::vector<long long> col21;
            std::vector<long long> col22;
            std::vector<long long> col23;
            std::vector<long long> col24;
            std::vector<long long> col25;
            std::vector<long long> col26;
            std::vector<long long> col27;
            std::vector<long long> col28;
            std::vector<long long> col29;

            std::vector<long long> col30;
            std::vector<long long> col31;
            std::vector<long long> col32;
            std::vector<long long> col33;
            std::vector<long long> col34;
            std::vector<long long> col35;
            std::vector<long long> col36;
            std::vector<long long> col37;
            std::vector<long long> col38;
            std::vector<long long> col39;
            
            std::vector<long long> col40;
            std::vector<long long> col41;
            std::vector<long long> col42;
            std::vector<long long> col43;
            std::vector<long long> col44;
            std::vector<long long> col45;
            std::vector<long long> col46;
            std::vector<long long> col47;
            std::vector<long long> col48;
            std::vector<long long> col49;

            std::vector<long long> col50;
            std::vector<long long> col51;
            std::vector<long long> col52;
            std::vector<long long> col53;
            std::vector<long long> col54;
            std::vector<long long> col55;
            std::vector<long long> col56;
            std::vector<long long> col57;
            std::vector<long long> col58;
            std::vector<long long> col59;

            std::vector<long long> col60;
            std::vector<long long> col61;
            std::vector<long long> col62;
            std::vector<long long> col63;
            std::vector<long long> col64;
            std::vector<int> col65;
            std::vector<string> col66;
            std::vector<string> col67;
            std::vector<long long> col68;
            std::vector<long long> col69;

            std::vector<long long> col70;
            std::vector<long long> col71;
            std::vector<long long> col72;
            std::vector<long long> col73;
            std::vector<long long> col74;
            std::vector<long long> col75;
            std::vector<long long> col76;
            std::vector<long long> col77;
            std::vector<long long> col78;
            std::vector<long long> col79;

            std::vector<long long> col80;
            std::vector<long long> col81;
            std::vector<long long> col82;
            std::vector<long long> col83;
            std::vector<long long> col84;
            std::vector<long long> col85;
            std::vector<long long> col86;
            std::vector<long long> col87;
            std::vector<int> col88;
            std::vector<int> col89;

            std::vector<int> col90;
            std::vector<int> col91;
            std::vector<long long> col92;
            std::vector<char> col93;

            for (uint32_t i = 0; i < cnt; ++i) {
                col0.emplace_back(snapshot[i].market_type);
                
                std::string securityCode(snapshot[i].security_code);
                col1.emplace_back(securityCode);

                col2.emplace_back(convertTime(snapshot[i].orig_time));

                std::string tradingPhaseCode(snapshot[i].trading_phase_code);
                col3.emplace_back(tradingPhaseCode);

                // 设置preClosePrice
                col4.emplace_back(snapshot[i].pre_close_price);

                // 设置openPrice
                col5.emplace_back(snapshot[i].open_price);

                // 设置highPrice
                col6.emplace_back(snapshot[i].high_price);

                // 设置lowPrice
                col7.emplace_back(snapshot[i].low_price);

                // 设置lastPrice
                col8.emplace_back(snapshot[i].last_price);

                // 设置closePrice
                col9.emplace_back(snapshot[i].close_price);

                // 设置bidPrice
                int bidPriceIndex = 0;
                col10.emplace_back(snapshot[i].bid_price[bidPriceIndex]);

                bidPriceIndex++;
                col11.emplace_back(snapshot[i].bid_price[bidPriceIndex]);

                bidPriceIndex++;
                col12.emplace_back(snapshot[i].bid_price[bidPriceIndex]);

                bidPriceIndex++;
                col13.emplace_back(snapshot[i].bid_price[bidPriceIndex]);

                bidPriceIndex++;
                col14.emplace_back(snapshot[i].bid_price[bidPriceIndex]);

                bidPriceIndex++;
                col15.emplace_back(snapshot[i].bid_price[bidPriceIndex]);

                bidPriceIndex++;
                col16.emplace_back(snapshot[i].bid_price[bidPriceIndex]);

                bidPriceIndex++;
                col17.emplace_back(snapshot[i].bid_price[bidPriceIndex]);

                bidPriceIndex++;
                col18.emplace_back(snapshot[i].bid_price[bidPriceIndex]);

                bidPriceIndex++;
                col19.emplace_back(snapshot[i].bid_price[bidPriceIndex]);

                // 设置bidVolume
                int bidVolumeIndex = 0;
                col20.emplace_back(snapshot[i].bid_volume[bidVolumeIndex]);

                bidVolumeIndex++;
                col21.emplace_back(snapshot[i].bid_volume[bidVolumeIndex]);

                bidVolumeIndex++;
                col22.emplace_back(snapshot[i].bid_volume[bidVolumeIndex]);

                bidVolumeIndex++;
                col23.emplace_back(snapshot[i].bid_volume[bidVolumeIndex]);

                bidVolumeIndex++;
                col24.emplace_back(snapshot[i].bid_volume[bidVolumeIndex]);

                bidVolumeIndex++;
                col25.emplace_back(snapshot[i].bid_volume[bidVolumeIndex]);

                bidVolumeIndex++;
                col26.emplace_back(snapshot[i].bid_volume[bidVolumeIndex]);

                bidVolumeIndex++;
                col27.emplace_back(snapshot[i].bid_volume[bidVolumeIndex]);

                bidVolumeIndex++;
                col28.emplace_back(snapshot[i].bid_volume[bidVolumeIndex]);

                bidVolumeIndex++;
                col29.emplace_back(snapshot[i].bid_volume[bidVolumeIndex]);

                // 设置offerPrice
                int offerPriceIndex = 0;
                col30.emplace_back(snapshot[i].offer_price[offerPriceIndex]);

                offerPriceIndex++;
                col31.emplace_back(snapshot[i].offer_price[offerPriceIndex]);

                offerPriceIndex++;
                col32.emplace_back(snapshot[i].offer_price[offerPriceIndex]);

                offerPriceIndex++;
                col33.emplace_back(snapshot[i].offer_price[offerPriceIndex]);

                offerPriceIndex++;
                col34.emplace_back(snapshot[i].offer_price[offerPriceIndex]);

                offerPriceIndex++;
                col35.emplace_back(snapshot[i].offer_price[offerPriceIndex]);

                offerPriceIndex++;
                col36.emplace_back(snapshot[i].offer_price[offerPriceIndex]);

                offerPriceIndex++;
                col37.emplace_back(snapshot[i].offer_price[offerPriceIndex]);

                offerPriceIndex++;
                col38.emplace_back(snapshot[i].offer_price[offerPriceIndex]);

                offerPriceIndex++;
                col39.emplace_back(snapshot[i].offer_price[offerPriceIndex]);

                // 设置offerVolume
                int offerVolumeIndex = 0;
                col40.emplace_back(snapshot[i].offer_volume[offerVolumeIndex]);
                
                offerVolumeIndex++;
                col41.emplace_back(snapshot[i].offer_volume[offerVolumeIndex]);
                
                offerVolumeIndex++;
                col42.emplace_back(snapshot[i].offer_volume[offerVolumeIndex]);

                offerVolumeIndex++;
                col43.emplace_back(snapshot[i].offer_volume[offerVolumeIndex]);

                offerVolumeIndex++;
                col44.emplace_back(snapshot[i].offer_volume[offerVolumeIndex]);

                offerVolumeIndex++;
                col45.emplace_back(snapshot[i].offer_volume[offerVolumeIndex]);

                offerVolumeIndex++;
                col46.emplace_back(snapshot[i].offer_volume[offerVolumeIndex]);

                offerVolumeIndex++;
                col47.emplace_back(snapshot[i].offer_volume[offerVolumeIndex]);

                offerVolumeIndex++;
                col48.emplace_back(snapshot[i].offer_volume[offerVolumeIndex]);

                offerVolumeIndex++;
                col49.emplace_back(snapshot[i].offer_volume[offerVolumeIndex]);

                // 设置numTrade
                col50.emplace_back(snapshot[i].num_trades);
                // 设置totalVolumeTrade
                col51.emplace_back(snapshot[i].total_volume_trade);
                // 设置totalValueTrade
                col52.emplace_back(snapshot[i].total_value_trade);
                // 设置totalBidVolume
                col53.emplace_back(snapshot[i].total_bid_volume);
                // 设置totalOfferVolume
                col54.emplace_back(snapshot[i].total_offer_volume);
                // 设置weightedAvgBidPrice
                col55.emplace_back(snapshot[i].weighted_avg_bid_price);
                // 设置weightedAvgOfferPrice
                col56.emplace_back(snapshot[i].weighted_avg_offer_price);
                // 设置IOPV
                col57.emplace_back(snapshot[i].IOPV);
                // 设置yieldToMaturity
                col58.emplace_back(snapshot[i].yield_to_maturity);
                // 设置highLimited
                col59.emplace_back(snapshot[i].high_limited);
                // 设置lowLimited
                col60.emplace_back(snapshot[i].low_limited);
                // 设置priceEarningRatio1
                col61.emplace_back(snapshot[i].price_earning_ratio1);
                // 设置priceEarningRatio2
                col62.emplace_back(snapshot[i].price_earning_ratio2);
                // 设置change1
                col63.emplace_back(snapshot[i].change1);
                // 设置change2
                col64.emplace_back(snapshot[i].change2);
                // 设置channelNo
                col65.emplace_back(snapshot[i].channel_no);
                // 设置mdStreamId
                std::string mdStreamId(snapshot[i].md_stream_id);
                col66.emplace_back(mdStreamId);
                // 设置instrumentStatus
                std::string instrumentStatus(snapshot[i].instrument_status);
                col67.emplace_back(instrumentStatus);
                // 设置preCloseIopv
                col68.emplace_back(snapshot[i].pre_close_iopv);
                // 设置altWeightedAvgBidPrice
                col69.emplace_back(snapshot[i].alt_weighted_avg_bid_price);
                // 设置altWeightedAvgOfferPrice
                col70.emplace_back(snapshot[i].alt_weighted_avg_offer_price);
                // 设置etfBuyNumber
                col71.emplace_back(snapshot[i].etf_buy_number);
                // 设置etfBuyAmount
                col72.emplace_back(snapshot[i].etf_buy_amount);
                // 设置 etfBuyMoney
                col73.emplace_back(snapshot[i].etf_buy_money);
                // 设置 etfSellNumber
                col74.emplace_back(snapshot[i].etf_sell_number);
                // 设置 etfSellAmount
                col75.emplace_back(snapshot[i].etf_sell_amount);
                // 设置 etfSellMoney
                col76.emplace_back(snapshot[i].etf_sell_money);
                // 设置 totalWarrantExecVolume
                col77.emplace_back(snapshot[i].total_warrant_exec_volume);
                // 设置 warLowerPrice
                col78.emplace_back(snapshot[i].war_lower_price);
                // 设置 warUpperPrice
                col79.emplace_back(snapshot[i].war_upper_price);
                // 设置 withdrawBuyNumber
                col80.emplace_back(snapshot[i].withdraw_buy_number);
                // 设置 withdrawBuyAmount
                col81.emplace_back(snapshot[i].withdraw_buy_amount);
                // 设置 withdrawBuyMoney
                col82.emplace_back(snapshot[i].withdraw_buy_money);
                // 设置 withdrawSellNumber
                col83.emplace_back(snapshot[i].withdraw_sell_number);
                // 设置 withdrawSellAmount
                col84.emplace_back(snapshot[i].withdraw_sell_amount);
                // 设置 withdrawSellMoney
                col85.emplace_back(snapshot[i].withdraw_sell_money);
                // 设置 totalBidNumber
                col86.emplace_back(snapshot[i].total_bid_number);
                // 设置 totalOfferNumber
                col87.emplace_back(snapshot[i].total_offer_number);
                // 设置 bidTradeMaxDuration
                col88.emplace_back(snapshot[i].bid_trade_max_duration);
                // 设置 offerTradeMaxDuration
                col89.emplace_back(snapshot[i].offer_trade_max_duration);
                // 设置 numBidOrders
                col90.emplace_back(snapshot[i].num_bid_orders);
                // 设置 numOfferOrders
                col91.emplace_back(snapshot[i].num_offer_orders);
                // 设置 lastTradeTime
                col92.emplace_back(snapshot[i].last_trade_time);
                // 设置 varietyCategory
                col93.emplace_back(snapshot[i].variety_category);
            }

            ((VectorSP)cols[0])->appendInt(col0.data(), cnt);
            ((VectorSP)cols[1])->appendString(col1.data(), cnt);
            ((VectorSP)cols[2])->appendLong(col2.data(), cnt);
            ((VectorSP)cols[3])->appendString(col3.data(), cnt);
            ((VectorSP)cols[4])->appendLong(col4.data(), cnt);
            ((VectorSP)cols[5])->appendLong(col5.data(), cnt);
            ((VectorSP)cols[6])->appendLong(col6.data(), cnt);
            ((VectorSP)cols[7])->appendLong(col7.data(), cnt);
            ((VectorSP)cols[8])->appendLong(col8.data(), cnt);
            ((VectorSP)cols[9])->appendLong(col9.data(), cnt);

            ((VectorSP)cols[10])->appendLong(col10.data(), cnt);
            ((VectorSP)cols[11])->appendLong(col11.data(), cnt);
            ((VectorSP)cols[12])->appendLong(col12.data(), cnt);
            ((VectorSP)cols[13])->appendLong(col13.data(), cnt);
            ((VectorSP)cols[14])->appendLong(col14.data(), cnt);
            ((VectorSP)cols[15])->appendLong(col15.data(), cnt);
            ((VectorSP)cols[16])->appendLong(col16.data(), cnt);
            ((VectorSP)cols[17])->appendLong(col17.data(), cnt);
            ((VectorSP)cols[18])->appendLong(col18.data(), cnt);
            ((VectorSP)cols[19])->appendLong(col19.data(), cnt);

            ((VectorSP)cols[20])->appendLong(col20.data(), cnt);
            ((VectorSP)cols[21])->appendLong(col21.data(), cnt);
            ((VectorSP)cols[22])->appendLong(col22.data(), cnt);
            ((VectorSP)cols[23])->appendLong(col23.data(), cnt);
            ((VectorSP)cols[24])->appendLong(col24.data(), cnt);
            ((VectorSP)cols[25])->appendLong(col25.data(), cnt);
            ((VectorSP)cols[26])->appendLong(col26.data(), cnt);
            ((VectorSP)cols[27])->appendLong(col27.data(), cnt);
            ((VectorSP)cols[28])->appendLong(col28.data(), cnt);
            ((VectorSP)cols[29])->appendLong(col29.data(), cnt);

            ((VectorSP)cols[30])->appendLong(col30.data(), cnt);
            ((VectorSP)cols[31])->appendLong(col31.data(), cnt);
            ((VectorSP)cols[32])->appendLong(col32.data(), cnt);
            ((VectorSP)cols[33])->appendLong(col33.data(), cnt);
            ((VectorSP)cols[34])->appendLong(col34.data(), cnt);
            ((VectorSP)cols[35])->appendLong(col35.data(), cnt);
            ((VectorSP)cols[36])->appendLong(col36.data(), cnt);
            ((VectorSP)cols[37])->appendLong(col37.data(), cnt);
            ((VectorSP)cols[38])->appendLong(col38.data(), cnt);
            ((VectorSP)cols[39])->appendLong(col39.data(), cnt);

            ((VectorSP)cols[40])->appendLong(col40.data(), cnt);
            ((VectorSP)cols[41])->appendLong(col41.data(), cnt);
            ((VectorSP)cols[42])->appendLong(col42.data(), cnt);
            ((VectorSP)cols[43])->appendLong(col43.data(), cnt);
            ((VectorSP)cols[44])->appendLong(col44.data(), cnt);
            ((VectorSP)cols[45])->appendLong(col45.data(), cnt);
            ((VectorSP)cols[46])->appendLong(col46.data(), cnt);
            ((VectorSP)cols[47])->appendLong(col47.data(), cnt);
            ((VectorSP)cols[48])->appendLong(col48.data(), cnt);
            ((VectorSP)cols[49])->appendLong(col49.data(), cnt);

            ((VectorSP)cols[50])->appendLong(col50.data(), cnt);
            ((VectorSP)cols[51])->appendLong(col51.data(), cnt);
            ((VectorSP)cols[52])->appendLong(col52.data(), cnt);
            ((VectorSP)cols[53])->appendLong(col53.data(), cnt);
            ((VectorSP)cols[54])->appendLong(col54.data(), cnt);
            ((VectorSP)cols[55])->appendLong(col55.data(), cnt);
            ((VectorSP)cols[56])->appendLong(col56.data(), cnt);
            ((VectorSP)cols[57])->appendLong(col57.data(), cnt);
            ((VectorSP)cols[58])->appendLong(col58.data(), cnt);
            ((VectorSP)cols[59])->appendLong(col59.data(), cnt);

            ((VectorSP)cols[60])->appendLong(col60.data(), cnt);
            ((VectorSP)cols[61])->appendLong(col61.data(), cnt);
            ((VectorSP)cols[62])->appendLong(col62.data(), cnt);
            ((VectorSP)cols[63])->appendLong(col63.data(), cnt);
            ((VectorSP)cols[64])->appendLong(col64.data(), cnt);
            ((VectorSP)cols[65])->appendInt(col65.data(), cnt);
            ((VectorSP)cols[66])->appendString(col66.data(), cnt);
            ((VectorSP)cols[67])->appendString(col67.data(), cnt);
            ((VectorSP)cols[68])->appendLong(col68.data(), cnt);
            ((VectorSP)cols[69])->appendLong(col69.data(), cnt);

            ((VectorSP)cols[70])->appendLong(col70.data(), cnt);
            ((VectorSP)cols[71])->appendLong(col71.data(), cnt);
            ((VectorSP)cols[72])->appendLong(col72.data(), cnt);
            ((VectorSP)cols[73])->appendLong(col73.data(), cnt);
            ((VectorSP)cols[74])->appendLong(col74.data(), cnt);
            ((VectorSP)cols[75])->appendLong(col75.data(), cnt);
            ((VectorSP)cols[76])->appendLong(col76.data(), cnt);
            ((VectorSP)cols[77])->appendLong(col77.data(), cnt);
            ((VectorSP)cols[78])->appendLong(col78.data(), cnt);
            ((VectorSP)cols[79])->appendLong(col79.data(), cnt);

            ((VectorSP)cols[80])->appendLong(col80.data(), cnt);
            ((VectorSP)cols[81])->appendLong(col81.data(), cnt);
            ((VectorSP)cols[82])->appendLong(col82.data(), cnt);
            ((VectorSP)cols[83])->appendLong(col83.data(), cnt);
            ((VectorSP)cols[84])->appendLong(col84.data(), cnt);
            ((VectorSP)cols[85])->appendLong(col85.data(), cnt);
            ((VectorSP)cols[86])->appendLong(col86.data(), cnt);
            ((VectorSP)cols[87])->appendLong(col87.data(), cnt);
            ((VectorSP)cols[88])->appendInt(col88.data(), cnt);
            ((VectorSP)cols[89])->appendInt(col89.data(), cnt);

            ((VectorSP)cols[90])->appendInt(col90.data(), cnt);
            ((VectorSP)cols[91])->appendInt(col91.data(), cnt);
            ((VectorSP)cols[92])->appendLong(col92.data(), cnt);
            ((VectorSP)cols[93])->appendChar(col93.data(), cnt);

            TableSP data = Util::createTable(snapshotDataTableMeta_.colNames_, cols);
            vector<ConstantSP> args = {snapshotData_, data};
            session_->getFunctionDef("append!")->call(session_->getHeap().get(), args);

            if (latencyFlag_) {
                long long diff = Util::getNanoEpochTime() - startTime;
                latencyLog(0, startTime, cnt, diff);
            }

            /* 手动释放数据内存, 否则会造成内存泄露 */
            amd::ama::IAMDApi::FreeMemory(snapshot);
        }

        // 接受并处理逐笔委托数据
        virtual void OnMDTickOrder(amd::ama::MDTickOrder* ticks, uint32_t cnt) {
            long long startTime = Util::getNanoEpochTime();
            std::lock_guard<std::mutex> amdLock_(amdMutex_);
            std::vector<ConstantSP> cols;
            for (unsigned int orderIndex = 0; orderIndex < orderTableMeta_.colTypes_.size(); orderIndex++) {
                cols.push_back(Util::createVector(orderTableMeta_.colTypes_[orderIndex], 0, cnt));
            }
            
            vector<int> col0;
            vector<string> col1;
            vector<int> col2;
            vector<long long> col3;
            vector<long long> col4;
            vector<long long> col5;
            vector<long long> col6;
            vector<char> col7;
            vector<char> col8;
            vector<string> col9;
            vector<long long> col10;
            vector<long long> col11;
            vector<char> col12;

            for (uint32_t i = 0; i < cnt; ++i) {
                col0.emplace_back(ticks[i].market_type);
            
                std::string securityCode(ticks[i].security_code);
                col1.emplace_back(securityCode);

                col2.emplace_back(ticks[i].channel_no);

                col3.emplace_back(ticks[i].appl_seq_num);

                col4.emplace_back(convertTime(ticks[i].order_time));

                col5.emplace_back(ticks[i].order_price);

                col6.emplace_back(ticks[i].order_volume);

                col7.emplace_back(ticks[i].side);

                col8.emplace_back(ticks[i].order_type);

                std::string mdStreamId(ticks[i].md_stream_id);
                col9.emplace_back(mdStreamId);

                col10.emplace_back(ticks[i].orig_order_no);

                col11.emplace_back(ticks[i].biz_index);

                col12.emplace_back(ticks[i].variety_category);
            }

            ((VectorSP)cols[0])->appendInt(col0.data(), cnt);
            ((VectorSP)cols[1])->appendString(col1.data(), cnt);
            ((VectorSP)cols[2])->appendInt(col2.data(), cnt);
            ((VectorSP)cols[3])->appendLong(col3.data(), cnt);
            ((VectorSP)cols[4])->appendLong(col4.data(), cnt);
            ((VectorSP)cols[5])->appendLong(col5.data(), cnt);
            ((VectorSP)cols[6])->appendLong(col6.data(), cnt);
            ((VectorSP)cols[7])->appendChar(col7.data(), cnt);
            ((VectorSP)cols[8])->appendChar(col8.data(), cnt);
            ((VectorSP)cols[9])->appendString(col9.data(), cnt);
            ((VectorSP)cols[10])->appendLong(col10.data(), cnt);
            ((VectorSP)cols[11])->appendLong(col11.data(), cnt);
            ((VectorSP)cols[12])->appendChar(col12.data(), cnt);

            INDEX insertedRows;
            string errMsg;
            TableSP tmp_table = Util::createTable(orderTableMeta_.colNames_, orderTableMeta_.colTypes_, 0, 0);
            tmp_table->append(cols, insertedRows, errMsg);
            vector<ConstantSP> args = {orderData_, tmp_table};
            session_->getFunctionDef("append!")->call(session_->getHeap().get(), args);

            
            if (latencyFlag_) { 
                long long diff = Util::getNanoEpochTime() - startTime;
                latencyLog(2, startTime, cnt, diff);
            }
            
            /* 手动释放数据内存, 否则会造成内存泄露 */
            amd::ama::IAMDApi::FreeMemory(ticks);
        }

            // 接受并处理逐笔成交数据
        virtual void OnMDTickExecution(amd::ama::MDTickExecution* tick, uint32_t cnt) override {
            // FIXME
            // if (enableLog_[1] == true) {
            //     latencyLog(1);
            // }
            long long startTime = Util::getNanoEpochTime();
            std::lock_guard<std::mutex> amdLock_(amdMutex_);
            std::vector<ConstantSP> cols;
            for (unsigned int orderIndex = 0; orderIndex < executionTableMeta_.colTypes_.size(); orderIndex++) {
                cols.emplace_back(Util::createVector(executionTableMeta_.colTypes_[orderIndex], 0, cnt));
            }

            std::vector<int> col0;
            std::vector<string> col1;
            std::vector<long long> col2;
            std::vector<int> col3;
            std::vector<long long> col4;
            std::vector<long long> col5;
            std::vector<long long> col6;
            std::vector<long long> col7;
            std::vector<long long> col8;
            std::vector<long long> col9;
            std::vector<char> col10;
            std::vector<char> col11;
            std::vector<string> col12;
            std::vector<long long> col13;
            std::vector<char> col14;

            for (uint32_t i = 0; i < cnt; ++i) {
                col0.emplace_back(tick[i].market_type);

                std::string securityCode(tick[i].security_code);
                col1.emplace_back(securityCode);

                col2.emplace_back(convertTime(tick[i].exec_time));

                col3.emplace_back(tick[i].channel_no);

                col4.emplace_back(tick[i].appl_seq_num);

                col5.emplace_back(tick[i].exec_price);

                col6.emplace_back(tick[i].exec_volume);

                col7.emplace_back(tick[i].value_trade);

                col8.emplace_back(tick[i].bid_appl_seq_num);
                
                col9.emplace_back(tick[i].offer_appl_seq_num);

                col10.emplace_back(tick[i].side);

                col11.emplace_back(tick[i].exec_type);

                std::string mdStreamId(tick[i].md_stream_id);
                col12.emplace_back(mdStreamId);

                col13.emplace_back(tick[i].biz_index);

                col14.emplace_back(tick[i].variety_category);
            }

            ((VectorSP)cols[0])->appendInt(col0.data(), cnt);
            ((VectorSP)cols[1])->appendString(col1.data(), cnt);
            ((VectorSP)cols[2])->appendLong(col2.data(), cnt);
            ((VectorSP)cols[3])->appendInt(col3.data(), cnt);
            ((VectorSP)cols[4])->appendLong(col4.data(), cnt);
            ((VectorSP)cols[5])->appendLong(col5.data(), cnt);
            ((VectorSP)cols[6])->appendLong(col6.data(), cnt);
            ((VectorSP)cols[7])->appendLong(col7.data(), cnt);
            ((VectorSP)cols[8])->appendLong(col8.data(), cnt);
            ((VectorSP)cols[9])->appendLong(col9.data(), cnt);
            ((VectorSP)cols[10])->appendChar(col10.data(), cnt);
            ((VectorSP)cols[11])->appendChar(col11.data(), cnt);
            ((VectorSP)cols[12])->appendString(col12.data(), cnt);
            ((VectorSP)cols[13])->appendLong(col13.data(), cnt);
            ((VectorSP)cols[14])->appendChar(col14.data(), cnt);

            INDEX insertedRows;
            string errMsg;
            TableSP tmp_table = Util::createTable(executionTableMeta_.colNames_, executionTableMeta_.colTypes_, 0, 0);
            tmp_table->append(cols, insertedRows, errMsg);
            vector<ConstantSP> args = {executionData_, tmp_table};
            session_->getFunctionDef("append!")->call(session_->getHeap().get(), args);
            
            if (latencyFlag_) { 
                long long latency = Util::getNanoEpochTime() - startTime;
                latencyLog(1, startTime, cnt, latency);
            }

            /* 手动释放数据内存, 否则会造成内存泄露 */
            amd::ama::IAMDApi::FreeMemory(tick);
        }

        virtual void OnEvent(uint32_t level, uint32_t code, const char* event_msg, uint32_t len)
        {
            if(code == amd::ama::EventCode::kChannelTCPInitFailed || code == amd::ama::EventCode::kChannelTCPConnectFailed || \
            code == amd::ama::EventCode::kChannelTCPLogonFailed || code == amd::ama::EventCode::kChannelTCPSessionClosed)  //代码表模块初始化成功
            {
                LOG_ERR("amdQuote connect failed");
            }
            LOG_INFO("amdQuote AMA event:", std::string(event_msg));
        }

        void latencyLog(int type, long long startTime, uint32_t cnt, long long latency) { // 0: snapshot, 1: execution, 2: order
            Statistic& stat = statistics_[type];
            if (stat.startTime != 0 && startTime > stat.endTime) { // 大于30s，打日志
                if(stat.totalHandleCount > 0){
                        LOG_INFO(type, " statistics: total message count ", stat.totalMessageCount,
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

        void setSnapshotData(TableSP snapshotData) {
            snapshotData_ = snapshotData;
        }

        void setExecutionData(TableSP executionData) {
            executionData_ = executionData;
        }

        void setOrderData(TableSP orderData) {
            orderData_ = orderData;
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

        void setLatencyFlag() {
            latencyFlag_ = true;
        }

    private:
        AmdSnapshotTableMeta  snapshotDataTableMeta_;
        AmdExecutionTableMeta executionTableMeta_;
        AmdOrderTableMeta  orderTableMeta_;
        TableSP snapshotData_; // 保存快照数据, 建议定义成流表
        TableSP executionData_; // 保存逐笔委托, 建议定义成流表
        TableSP orderData_; // 保存逐笔成交, 建议定义成流表

        struct Statistic {
            long long startTime = 0; // 起始时间（读到第一条数据的时间）
            long long endTime = 0; // 统计结束时间
            long long totalLatency = 0; // 总时间。单位纳秒
            long long maxLatency = 0; // 单条最大延迟
            long long minLatency = 0; // 单条最大延迟
            long totalMessageCount = 0; // 总消息量
            long totalHandleCount = 0; // 处理数据次数，即调用了多少次回调函数
        };
        Statistic statistics_[3]; // 分别对应snapshot, execution, order
        const long long duration_ = 30 * (long long)1000000000;
        SessionSP session_;

        bool latencyFlag_ = false; // 分别对应snapshot, execution, order
    };

public:
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



#endif