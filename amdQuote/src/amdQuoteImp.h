#ifndef __AMD_QUOTE_IMP_H
#define __AMD_QUOTE_IMP_H

#include <exception>
#include <mutex>
#include <ostream>
#include <string>
#include <unordered_map>
#include <iostream>

#include "Exceptions.h"
#include "FlatHashmap.h"
#include "ama.h"
#include "ama_tools.h"

#include "CoreConcept.h"
#include "amdQuoteType.h"
#include "Logger.h"
#include "Util.h"
#include "ScalarImp.h"
#include "Plugin.h"

using namespace dolphindb;

class AMDSpiImp;
extern bool receivedTimeFlag;
extern bool dailyIndexFlag;
extern bool stopTest;
extern unordered_map<string, AMDDataType> nameType;

inline long long countTemporalUnit(int days, long long multiplier, long long remainder){
	return days == INT_MIN ? LLONG_MIN : days * multiplier + remainder;
}


inline int countDays(int amdDays){
    return Util::countDays(amdDays / 10000, (amdDays / 100) % 100, amdDays % 100);
}
inline int convertToDate(long long time) {
    long long  year, month, day;
    day = time / 1000 / 100 / 100 / 100 % 100;
    month = time / 1000 / 100 / 100 / 100 / 100 % 100;
    year = time / 1000 / 100 / 100 / 100 / 100 / 100;
    return Util::countDays(year,month,day);
}
inline int convertToTime(long long time) {
    long long  hour, minute, second, milliSecond;
    milliSecond = time % 1000;
    second = time / 1000 % 100;
    minute = time / 1000 / 100 % 100;
    hour = time / 1000 / 100 / 100 % 100;
    return ((hour * 60 + minute)*60+second)*1000ll+milliSecond;
}
inline long long convertTime(long long time) {
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


#ifdef ENUM_OR_STRING
#undef ENUM_OR_STRING
#endif
#define ENUM_OR_STRING(x) #x


AMDTableType getAmdTableType(AMDDataType dataType, int market);


inline bool getDailyIndex(int& ret, DailyIndex* dailyIndexArray, int dailyIndexArraySize, int market, AMDDataType datatype, int32_t channel_no, long long timestamp){
    AMDTableType dataType = getAmdTableType(datatype, market);
    if((int)datatype == (int)AMD_ERROR_TABLE_TYPE){
        LOG_ERR("[PluginAmdQuote]: getAmdTableType failed");
        return false;
    }
    if(datatype >= dailyIndexArraySize){
        LOG_ERR("[PluginAmdQuote]: tableType exceeds the size of DailyIndex_");
        return false;
    }

    if(dailyIndexArray[dataType].getFlag())
        ret = dailyIndexArray[dataType].getIndex(channel_no, timestamp);
    else
        ret = INT_MIN;
    return true;
}
class AmdQuote {
private: 
    AmdQuote(std::string username, std::string password, std::vector<std::string> ips, std::vector<int> ports, SessionSP session);
public:
    static AmdQuote* getInstance(std::string username, std::string password, std::vector<std::string> ips, std::vector<int> ports, SessionSP session) {
        if (instance_ == nullptr) {
            instance_ = new AmdQuote(username, password, ips, ports, session);
        }

        return instance_;
    }

    static bool instanceValid() {
        if(!instance_) {
            return false;
        } else {
            return true;
        }
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

    void enableLatencyStatistics(bool flag);

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


    // subscribe
    void subscribeOrderExecution(string                     orderExecutionType,
                                 int                        market,
                                 DictionarySP               dict,
                                 std::vector<string>        codeList,
                                 FunctionDefSP              transform,
                                 bool                       receivedTimeFlag,
                                 long long                  dailyStartTimestamp);

    // 订阅快照数据
    void subscribeSnapshot(int market, TableSP table, std::vector<std::string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp);

    // 订阅逐笔成交
    void subscribeExecution(int market, TableSP table, std::vector<std::string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp);

    // 订阅逐笔委托
    void subscribeOrder(int market, TableSP table, std::vector<std::string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp);

    // 订阅基金快照数据
    void subscribeFundSnapshot(int market, TableSP table, std::vector<std::string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp);

    // 订阅基金逐笔成交
    void subscribeFundExecution(int market, TableSP table, std::vector<std::string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp);

    // 订阅基金逐笔委托
    void subscribeFundOrder(int market, TableSP table, std::vector<std::string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp);

    // 订阅债券快照数据
    void subscribeBondSnapshot(int market, TableSP table, std::vector<std::string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp);

    // 订阅债券逐笔成交
    void subscribeBondExecution(int market, TableSP table, std::vector<std::string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp);

    // 订阅债券逐笔委托
    void subscribeBondOrder(int market, TableSP table, std::vector<std::string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp);

    // 订阅指数快照数据
    void subscribeIndex(int market, TableSP table, std::vector<std::string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp);

    // 订阅委托队列所有类型数据
    void subscribeOrderQueue(int market, TableSP table, std::vector<std::string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp);

    void unsubscribe(std::string dataType, int market, std::vector<std::string> codeList);

    ~AmdQuote();

public:
    AMDSpiImp* getAMDSpi(){
        return amdSpi_;
    }
    static Mutex amdMutex_;
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

#endif