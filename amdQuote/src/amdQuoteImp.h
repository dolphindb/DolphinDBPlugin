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
extern bool RECEIVED_TIME_FLAG;
extern bool DAILY_INDEX_FLAG;
extern bool STOP_TEST;
extern unordered_map<string, AMDDataType> NAME_TYPE;

inline long long countTemporalUnit(int days, long long multiplier, long long remainder){
	return days == INT_MIN ? LLONG_MIN : days * multiplier + remainder;
}


inline int countDays(int amdDays){
    return Util::countDays(amdDays / 10000, (amdDays / 100) % 100, amdDays % 100);
}

// to cooperate with insight based orderbookSnapshot.
// use convertBSFlag() & convertType() to change amd flag & type to insight flag & type.
// if a flag & type not exist in insight, return origin value.
// details see https://dolphindb1.atlassian.net/browse/DPLG-837
inline int convertBSFlag(int flag) {
    switch(flag){
        case 49:
        case 66:
            return 1;
        case 50:
        case 83:
            return 2;
        default:
            return flag;
    }
}
inline int convertType(int type) {
    switch(type){
        case 50:
        case 65:
            return 2;
        case 68:
            return 10;
        case 83:
            return 11;
        case 85:
            return 3;
        case 70:
            return 0;
        case 4:
        case 49:
        case 52:
            return 1;
        default:
            return type;
    }
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
    AMDTableType dataTableType = getAmdTableType(datatype, market);
    if((int)datatype == (int)AMD_ERROR_TABLE_TYPE){
        LOG_ERR("[PLUGIN::AMDQUOTE]: getAmdTableType failed");
        return false;
    }
    if(dataTableType >= dailyIndexArraySize){
        LOG_ERR("[PLUGIN::AMDQUOTE]: tableType exceeds the size of DailyIndex_");
        return false;
    }

    if(dailyIndexArray[dataTableType].getFlag())
        ret = dailyIndexArray[dataTableType].getIndex(channel_no, timestamp);
    else
        ret = INT_MIN;
    return true;
}
class AmdQuote {
private:
    AmdQuote(const string& username, const string& password, vector<string> ips, vector<int> ports, SessionSP session);
public:
    static AmdQuote* getInstance(const string& username, const string& password, vector<string> ips, vector<int> ports, SessionSP session) {
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
        vector<ConstantSP> cols;
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

    // subscribe
    void subscribeOrderExecution(string                     orderExecutionType,
                                 int                        market,
                                 DictionarySP               dict,
                                 vector<string>             codeList,
                                 FunctionDefSP              transform,
                                 bool                       receivedTimeFlag,
                                 long long                  dailyStartTimestamp);

    void subscribeSnapshot(int market, TableSP table, vector<string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp);

    void subscribeExecution(int market, TableSP table, vector<string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp);

    void subscribeOrder(int market, TableSP table, vector<string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp);

    void subscribeFundSnapshot(int market, TableSP table, vector<string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp);

    void subscribeFundExecution(int market, TableSP table, vector<string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp);

    void subscribeFundOrder(int market, TableSP table, vector<string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp);

    void subscribeBondSnapshot(int market, TableSP table, vector<string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp);

    void subscribeBondExecution(int market, TableSP table, vector<string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp);

    void subscribeBondOrder(int market, TableSP table, vector<string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp);

    void subscribeIndex(int market, TableSP table, vector<string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp);

    void subscribeOrderQueue(int market, TableSP table, vector<string> codeList, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp);

    void unsubscribe(string dataType, int market, vector<string> codeList);

    ~AmdQuote();

public:
    AMDSpiImp* getAMDSpi(){
        return amdSpi_;
    }
    static Mutex amdMutex_;
    unordered_set<Info, InfoHash, InfoEqual> infoSet_;
private:
    amd::ama::Cfg cfg_;
    AMDSpiImp* amdSpi_;
    const string& username_;
    const string& password_;
    vector<string> ips_;
    vector<int> ports_;
    static AmdQuote* instance_;
};

#endif