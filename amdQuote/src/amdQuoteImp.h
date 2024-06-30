#ifndef AMD_QUOTE_IMP_H
#define AMD_QUOTE_IMP_H

#include "CoreConcept.h"
#include "Plugin.h"
#include "ScalarImp.h"
#include "Util.h"
#include "ama.h"
#include "ama_tools.h"
#include "amdQuoteType.h"

#ifndef AMD_USE_THREADED_QUEUE
#define AMD_USE_THREADED_QUEUE
#include "ddbplugin/ThreadedQueue.h"
#endif

using namespace dolphindb;

class AMDSpiImp;
extern unordered_map<string, AMDDataType> NAME_TYPE;

class AmdQuote {
  public:
    AmdQuote(const string &username, const string &password, const vector<string>& ips, const vector<int>& ports,
             SessionSP session, bool receivedTime, bool dailyIndex, bool outputElapsed, int dailyStartTime, bool securityCodeToInt, string dataVersion);
    void enableLatencyStatistics(bool flag);
    void subscribe(Heap *heap, const string &dataType, int market, vector<string> codeList, ConstantSP table,
                   FunctionDefSP transform, long long timestamp, int seqCheckMode);
    void addSubscribe(Heap *heap, const string &typeName, AMDDataType type, int key, TableSP table,
                      FunctionDefSP transform, long long dailyStartTime, int seqCheckMode);
    void removeSubscribe(AMDDataType type, int key);
    void unsubscribe(const string &dataType, int market, vector<string> codeList);
    TableSP getStatus();
    ~AmdQuote();

    bool checkIfSame(const vector<string>& ips, const vector<int> &ports, bool receivedTime, bool dailyIndex,
                     bool outputElapsed, int dailyStartTime) {
        if(ips.size() != ips_.size() || ports.size() != ips.size()) { return false; }
        set<string> ipSet;
        set<int> portSet;
        for(auto& ip: ips) { ipSet.insert(ip); }
        for(auto& port: ports) { portSet.insert(port); }
        for(auto& ip: ips_) { if(ipSet.find(ip) == ipSet.end()) return false; }
        for(auto& port: ports_) { if(portSet.find(port) == portSet.end()) return false; }
        return (receivedTime == receivedTime_) && (dailyIndex == dailyIndex_) && (outputElapsed == outputElapsed_) &&
                (dailyStartTime == dailyStartTime_);
    }
    MetaTable getMetaByAMDType(AMDDataType type) {
        switch (type) {
            case AMD_SNAPSHOT:
            case AMD_FUND_SNAPSHOT:
                return amdTypeContainer_.get("snapshot");
            case AMD_BOND_SNAPSHOT:
                if(dataVersion_ == "4.0.1") {
                    return amdTypeContainer_.get("bondSnapshot_4.0.1");
                } else {
                    return amdTypeContainer_.get("snapshot");
                }
            case AMD_EXECUTION:
            case AMD_FUND_EXECUTION:
            case AMD_BOND_EXECUTION:
                return amdTypeContainer_.get("execution");
            case AMD_ORDER:
            case AMD_FUND_ORDER:
                if(dataVersion_ == "4.0.1") {
                    return amdTypeContainer_.get("order_4.0.1");
                } else {
                    return amdTypeContainer_.get("order");
                }
            case AMD_BOND_ORDER:
                if(dataVersion_ == "4.0.1") {
                    return amdTypeContainer_.get("bondOrder_4.0.1");
                } else {
                    return amdTypeContainer_.get("order");
                }
            case AMD_ORDER_EXECUTION:
            case AMD_BOND_ORDER_EXECUTION:
                return amdTypeContainer_.get("orderExecution");
            case AMD_INDEX:
                return amdTypeContainer_.get("index");
            case AMD_ORDER_QUEUE:
                return amdTypeContainer_.get("orderQueue");
            case AMD_OPTION_SNAPSHOT:
                return amdTypeContainer_.get("option");
            case AMD_FUTURE_SNAPSHOT:
                return amdTypeContainer_.get("future");
#ifndef AMD_3_9_6
            case AMD_IOPV_SNAPSHOT:
                return amdTypeContainer_.get("IOPV");
#endif
            default:
                throw RuntimeException(AMDQUOTE_PREFIX + "Invalid dataType " + std::to_string(type) + ".");
        }
    }

    TableSP getSchemaByAMDType(AMDDataType type, int optionFlag) {
        switch (type) {
            case AMD_SNAPSHOT:
            case AMD_FUND_SNAPSHOT:
                return amdTypeContainer_.getSchema("snapshot", optionFlag);
            case AMD_BOND_SNAPSHOT:
                if(dataVersion_ == "4.0.1") {
                    return amdTypeContainer_.getSchema("bondSnapshot_4.0.1", optionFlag);
                } else {
                    return amdTypeContainer_.getSchema("snapshot", optionFlag);
                }
            case AMD_EXECUTION:
            case AMD_FUND_EXECUTION:
            case AMD_BOND_EXECUTION:
                return amdTypeContainer_.getSchema("execution", optionFlag);
            case AMD_ORDER:
            case AMD_FUND_ORDER:
                if(dataVersion_ == "4.0.1") {
                    return amdTypeContainer_.getSchema("order_4.0.1", optionFlag);
                } else {
                    return amdTypeContainer_.getSchema("order", optionFlag);
                }
            case AMD_BOND_ORDER:
                if(dataVersion_ == "4.0.1") {
                    return amdTypeContainer_.getSchema("bondOrder_4.0.1", optionFlag);
                } else {
                    return amdTypeContainer_.getSchema("order", optionFlag);
                }
            case AMD_ORDER_EXECUTION:
            case AMD_BOND_ORDER_EXECUTION:
                // special treatment, no flag passed in.
                return amdTypeContainer_.getSchema("orderExecution", optionFlag & OPT_ELAPSED);
            case AMD_INDEX:
                return amdTypeContainer_.getSchema("index", optionFlag);
            case AMD_ORDER_QUEUE:
                return amdTypeContainer_.getSchema("orderQueue", optionFlag);
            case AMD_OPTION_SNAPSHOT:
                return amdTypeContainer_.getSchema("option", optionFlag);
            case AMD_FUTURE_SNAPSHOT:
                return amdTypeContainer_.getSchema("future", optionFlag);
#ifndef AMD_3_9_6
            case AMD_IOPV_SNAPSHOT:
                return amdTypeContainer_.getSchema("IOPV", optionFlag);
#endif
            default:
                throw RuntimeException(AMDQUOTE_PREFIX + "Invalid dataType " + std::to_string(type) + ".");
        }
    }

    TableSP getSchema(const string &dataType) {
        int flag = 0;
        if (receivedTime_) flag |= OPT_RECEIVED;
        if (outputElapsed_) flag |= OPT_ELAPSED;
        if (dailyIndex_) flag |= OPT_DAILY_INDEX;
        return getSchemaByAMDType(getAmdDataType(dataType), flag);
    }

  public:
    AMDSpiImp *getAMDSpi() { return amdSpi_; }

  private:
    bool receivedTime_;
    bool dailyIndex_;
    bool outputElapsed_;
    int dailyStartTime_;
    bool securityCodeToInt_;
    string dataVersion_;
    amd::ama::Cfg cfg_;
    AMDSpiImp *amdSpi_;
    string username_;
    string password_;
    vector<string> ips_;
    vector<int> ports_;
    MarketTypeContainer amdTypeContainer_;
};
#endif