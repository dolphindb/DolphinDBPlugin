#ifndef __AMD_QUOTE_TYPE_H
#define __AMD_QUOTE_TYPE_H

#include "Logger.h"
#include "Types.h"
#include "ama.h"

#include "CoreConcept.h"
#include "Util.h"

extern bool receivedTimeFlag;
extern bool dailyIndexFlag;
extern bool outputElapsedFlag;

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
    AMD_ERROR_DATA_TYPE,

    AMD_ORDER_EXECUTION,
    AMD_FUND_ORDER_EXECUTION,
    AMD_BOND_ORDER_EXECUTION,
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
    AMD_ORDER_EXECUTION_SH,
    AMD_ORDER_EXECUTION_SZ,
    AMD_FUND_ORDER_EXECUTION_SH,
    AMD_FUND_ORDER_EXECUTION_SZ,
    AMD_BOND_ORDER_EXECUTION_SH,
    AMD_BOND_ORDER_EXECUTION_SZ,
    AMD_ERROR_TABLE_TYPE,
};

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
    inline int getIndex(int32_t param, long long timestamp){
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
// union structure
struct MDOrderExecution {
    bool orderOrExecution;
    long long reachTime;
    union {
        amd::ama::MDTickOrder tickOrder;
        amd::ama::MDTickExecution tickExecution;
    } uni;
};
struct MDBondOrderExecution {
    bool orderOrExecution;
    long long reachTime;
    union {
        amd::ama::MDBondTickOrder tickOrder;
        amd::ama::MDBondTickExecution tickExecution;
    } uni;
};

typedef GenericBoundedQueue<MDOrderExecution, ObjectSizer<MDOrderExecution>, ObjectUrgency<MDOrderExecution>> OrderExecutionQueue;
typedef GenericBoundedQueue<MDBondOrderExecution, ObjectSizer<MDBondOrderExecution>, ObjectUrgency<MDBondOrderExecution>> BondOrderExecutionQueue;

struct timeMDSnapshot {
    long long reachTime;
    amd::ama::MDSnapshot snapshot;
};
struct timeMDTickOrder {
    long long reachTime;
    amd::ama::MDTickOrder order;
};
struct timeMDTickExecution {
    long long reachTime;
    amd::ama::MDTickExecution execution;
};

struct timeMDBondSnapshot {
    long long reachTime;
    amd::ama::MDBondSnapshot bondSnapshot;
};

struct timeMDBondTickOrder {
    long long reachTime;
    amd::ama::MDBondTickOrder bondOrder;
};

struct timeMDBondTickExecution {
    long long reachTime;
    amd::ama::MDBondTickExecution bondExecution;
};

struct timeMDIndexSnapshot {
    long long reachTime;
    amd::ama::MDIndexSnapshot indexSnapshot;
};
struct timeMDOrderQueue {
    long long reachTime;
    amd::ama::MDOrderQueue orderQueue;
};

typedef GenericBoundedQueue<timeMDSnapshot, ObjectSizer<timeMDSnapshot>, ObjectUrgency<timeMDSnapshot>> SnapshotQueue;
typedef GenericBoundedQueue<timeMDTickOrder, ObjectSizer<timeMDTickOrder>, ObjectUrgency<timeMDTickOrder>> OrderQueue;
typedef GenericBoundedQueue<timeMDTickExecution, ObjectSizer<timeMDTickExecution>, ObjectUrgency<timeMDTickExecution>> ExecutionQueue;
typedef GenericBoundedQueue<timeMDBondSnapshot, ObjectSizer<timeMDBondSnapshot>, ObjectUrgency<timeMDBondSnapshot>> BondSnapshotQueue;
typedef GenericBoundedQueue<timeMDBondTickOrder, ObjectSizer<timeMDBondTickOrder>, ObjectUrgency<timeMDBondTickOrder>> BondOrderQueue;
typedef GenericBoundedQueue<timeMDBondTickExecution, ObjectSizer<timeMDBondTickExecution>, ObjectUrgency<timeMDBondTickExecution>> BondExecutionQueue;
typedef GenericBoundedQueue<timeMDIndexSnapshot, ObjectSizer<timeMDIndexSnapshot>, ObjectUrgency<timeMDIndexSnapshot>> IndexQueue;
typedef GenericBoundedQueue<timeMDOrderQueue, ObjectSizer<timeMDOrderQueue>, ObjectUrgency<timeMDOrderQueue>> OrderQueueQueue;


// typedef GenericBoundedQueue<amd::ama::MDSnapshot, ObjectSizer<amd::ama::MDSnapshot>, ObjectUrgency<amd::ama::MDSnapshot>> SnapshotQueue;
// typedef GenericBoundedQueue<amd::ama::MDTickOrder, ObjectSizer<amd::ama::MDTickOrder>, ObjectUrgency<amd::ama::MDTickOrder>> OrderQueue;
// typedef GenericBoundedQueue<amd::ama::MDTickExecution, ObjectSizer<amd::ama::MDTickExecution>, ObjectUrgency<amd::ama::MDTickExecution>> ExecutionQueue;
// typedef GenericBoundedQueue<amd::ama::MDBondSnapshot, ObjectSizer<amd::ama::MDBondSnapshot>, ObjectUrgency<amd::ama::MDBondSnapshot>> BondSnapshotQueue;
// typedef GenericBoundedQueue<amd::ama::MDBondTickOrder, ObjectSizer<amd::ama::MDBondTickOrder>, ObjectUrgency<amd::ama::MDBondTickOrder>> BondOrderQueue;
// typedef GenericBoundedQueue<amd::ama::MDBondTickExecution, ObjectSizer<amd::ama::MDBondTickExecution>, ObjectUrgency<amd::ama::MDBondTickExecution>> BondExecutionQueue;
// typedef GenericBoundedQueue<amd::ama::MDIndexSnapshot, ObjectSizer<amd::ama::MDIndexSnapshot>, ObjectUrgency<amd::ama::MDIndexSnapshot>> IndexQueue;
// typedef GenericBoundedQueue<amd::ama::MDOrderQueue, ObjectSizer<amd::ama::MDOrderQueue>, ObjectUrgency<amd::ama::MDOrderQueue>> OrderQueueQueue;

template <class ITEMTYPE>
void blockHandling(SmartPointer<GenericBoundedQueue<ITEMTYPE, ObjectSizer<ITEMTYPE>, ObjectUrgency<ITEMTYPE>>> queue, std::function<void(vector<ITEMTYPE> &)> dealFunc, SmartPointer<bool> stopFlag, string msgPrefix, string threadInfo)
{
    LOG_INFO(msgPrefix, " ", threadInfo, " bg thread start ");
    bool ret;
    while (!*stopFlag)
    {
        long long size = 0;
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

            size = std::min(queue->size(), (long long)2048);
            vector<ITEMTYPE> items;
            items.reserve(size + 1);
            items.push_back(std::move(item));
            if (size > 0)
                queue->pop(items, size);

            size = items.size();
            LOG(msgPrefix, " ", threadInfo, " bg thread pop size ", size);

            dealFunc(items);
        }
        catch (exception &e)
        {
            LOG_ERR(msgPrefix, " ", threadInfo, " Failed to process data of size ", size, " because ", e.what());
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
class AmdSnapshotTableMeta {
public:
    AmdSnapshotTableMeta() {
        colNames_ = {
        // 市场类型        证券代码         时间        交易阶段代码         昨收价            开盘价       最高价        最低价       最新价       收盘价
        "marketType", "securityCode", "origTime", "tradingPhaseCode", "preClosePrice", "openPrice", "highPrice", "lowPrice", "lastPrice", "closePrice", \
        //  十档申买价                              
        "bidPrice1", "bidPrice2", "bidPrice3", "bidPrice4", "bidPrice5", "bidPrice6", "bidPrice7", "bidPrice8", "bidPrice9", "bidPrice10", \
        //  十档申买量   
        "bidVolume1", "bidVolume2", "bidVolume3", "bidVolume4", "bidVolume5", "bidVolume6", "bidVolume7", "bidVolume8", "bidVolume9", "bidVolume10", \
        //  十档申卖价   
        "offerPrice1", "offerPrice2", "offerPrice3", "offerPrice4", "offerPrice5", "offerPrice6", "offerPrice7", "offerPrice8", "offerPrice9", "offerPrice10", \
        //  十档申卖量
        "offerVolume1", "offerVolume2", "offerVolume3", "offerVolume4", "offerVolume5", "offerVolume6", "offerVolume7", "offerVolume8", "offerVolume9", "offerVolume10", \
        //  成交笔数      成交总量             成交总金额         委托买入总量       委托卖出总量         加权平均为委买价格       加权平均为委卖价格    IOPV净值估产   到期收益率        涨停价
        "numTrades", "totalVolumeTrade", "totalValueTrade", "totalBidVolume", "totalOfferVolume", "weightedAvgBidPrice", "weightedAvgOfferPrice", "ioPV", "yieldToMaturity", "highLimited", \
        //  跌停价        市盈率1               市盈率2                升跌1      升跌2       频道代码      行情类别       当前品种交易状态  基金T-1日收盘时刻IOPV  债券加权平均委买价格
        "lowLimited", "priceEarningRatio1", "priceEarningRatio2", "change1", "change2", "channelNo", "mdStreamID", "instrumentStatus", "preCloseIOPV", "altWeightedAvgBidPrice", \
        // 债券加权平均委卖价格          ETF 申购笔数    ETF 申购数量     ETF 申购金额    ETF 赎回笔数     ETF 赎回数量      ETF 赎回金额    权证执行的总数量           债券质押式回购品种加权平均价       
        "altWeightedAvgOfferPrice", "etfBuyNumber", "etfBuyAmount", "etfBuyMoney", "etfSellNumber", "etfSellAmount", "etfSellMoney", "totalWarrantExecVolume", "warLowerPrice", \
        // 权证涨停价格       买入撤单笔数          买入撤单数量         买入撤单金额          卖出撤单笔数            卖出撤单数量           卖出撤单金额          买入总笔数        卖出总笔数           买入委托成交最大等待时间 
        "warUpperPrice", "withdrawBuyNumber", "withdrawBuyAmount", "withdrawBuyMoney", "withdrawSellNumber", "withdrawSellAmount", "withdrawSellMoney", "totalBidNumber", "totalOfferNumber", "bidTradeMaxDuration", \
        //  卖出委托成交最大等待时间   买方委托价位数   卖方委托价位数      最近成交时间      品种类别  
        "offerTradeMaxDuration", "numBidOrders", "numOfferOrders", "lastTradeTime", "varietyCategory",
        };

        colTypes_ = {
        DT_INT, DT_SYMBOL, DT_TIMESTAMP, DT_STRING, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, \
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, \
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, \
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, \
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, \
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, \
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_INT,  DT_STRING, DT_STRING, DT_LONG, DT_LONG, \
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, \
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_INT, DT_INT,  \
        DT_INT,  DT_INT,  DT_LONG, DT_CHAR,
        };
    }

public:
    std::vector<string> colNames_;
    std::vector<DATA_TYPE> colTypes_;
};


class AmdExecutionTableMeta {
public:
    AmdExecutionTableMeta() {
        colNames_ = {
        //  市场类型       证券代码         时间        频道号        频道编号       成交价格      成交数量      成交金额
            "marketType", "securityCode", "execTime", "channelNo", "applSeqNum", "execPrice", "execVolume", "valueTrade", \
        //  买方委托索引      卖方委托索引        买卖方向  成交类型   行情类别       业务序号     品种类别
            "bidAppSeqNum", "offerApplSeqNum", "side", "execType", "mdStreamId", "bizIndex", "varietyCategory",
        };

        colTypes_ = {
            DT_INT, DT_SYMBOL, DT_TIMESTAMP, DT_INT, DT_LONG, DT_LONG, DT_LONG, DT_LONG, \
            DT_LONG, DT_LONG, DT_CHAR, DT_CHAR, DT_STRING, DT_LONG, DT_CHAR,
        };
    }

    ~AmdExecutionTableMeta() {}

public:
    std::vector<string> colNames_;
    std::vector<DATA_TYPE> colTypes_;
};

class AmdOrderTableMeta {
public:
    AmdOrderTableMeta() {
        colNames_ = {
        //  市场类型       证券代码         频道号       频道索引       时间          委托价格      委托数量       
            "marketType", "securityCode", "channelNo", "applSeqNum", "orderTime", "orderPrice", "orderVolume", \
        //  买卖方向 订单类别      行情类别(仅深圳市场有效) 原始订单号  业务序号   品种类别
            "side", "orderType", "mdStreamId", "origOrderNo", "bizIndex", "varietyCategory", 
        };

        colTypes_ = {
            DT_INT, DT_SYMBOL, DT_INT, DT_LONG, DT_TIMESTAMP, DT_LONG, DT_LONG, \
            DT_CHAR, DT_CHAR, DT_STRING, DT_LONG, DT_LONG, DT_CHAR,
        };
    }
    ~AmdOrderTableMeta(){}

public:
    std::vector<string> colNames_;
    std::vector<DATA_TYPE> colTypes_; 
};

class AmdIndexTableMeta {
public:
    AmdIndexTableMeta() {
        colNames_ = {
        //  市场类型       证券代码         时间        交易阶段代码         前收盘指数         今开盘指数   最高指数       
            "marketType", "securityCode", "origTime", "tradingPhaseCode", "preCloseIndex", "openIndex", "highIndex", \
        //  最低指数     最新指数      收盘指数      交易数量             成交总金额          频道代码      行情类别
            "lowIndex", "lastIndex", "closeIndex", "totalVolumeTrade", "totalValueTrade", "channelNo", "mdStreamId",
        //  品种类别
            "varietyCategory",
        };

        colTypes_ = {
            DT_INT, DT_SYMBOL, DT_TIMESTAMP, DT_STRING, DT_LONG, DT_LONG, DT_LONG, \
            DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_INT, DT_STRING,
            DT_CHAR,
        };
    }
    ~AmdIndexTableMeta(){}

public:
    std::vector<string> colNames_;
    std::vector<DATA_TYPE> colTypes_; 
};

class AmdOrderQueueTableMeta {
public:
    AmdOrderQueueTableMeta() {
        colNames_ = {
        //  市场类型       证券代码         委托时间     买卖方向 委托价格       订单数目       总委托笔数      明细个数      
            "marketType", "securityCode", "orderTime", "side", "orderPrice", "orderVolume", "numOfOrders", "items", 
        //  订单明细1-10 
            "volume1", "volume2", "volume3", "volume4", "volume5", "volume6", "volume7", "volume8", "volume9", "volume10",
        //  订单明细11-20
            "volume11", "volume12", "volume13", "volume14", "volume15", "volume16", "volume17", "volume18", "volume19", "volume20",
        //  订单明细21-30
            "volume21", "volume22", "volume23", "volume24", "volume25", "volume26", "volume27", "volume28", "volume29", "volume30",
        //  订单明细31-40 
            "volume31", "volume32", "volume33", "volume34", "volume35", "volume36", "volume37", "volume38", "volume39", "volume40",
        //  订单明细41-50 
            "volume41", "volume42", "volume43", "volume44", "volume45", "volume46", "volume47", "volume48", "volume49", "volume50",
        //  品种类别      行情类别       品种类别
            "channelNo", "mdStreamId", "varietyCategory",
        };

        colTypes_ = {
            DT_INT, DT_SYMBOL, DT_TIMESTAMP, DT_CHAR, DT_LONG, DT_LONG, DT_INT, DT_INT,
            DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
            DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
            DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
            DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
            DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
            DT_INT, DT_STRING, DT_CHAR,
        };
    }
    ~AmdOrderQueueTableMeta(){}

public:
    std::vector<string> colNames_;
    std::vector<DATA_TYPE> colTypes_; 
};

class AmdOrderExecutionTableMeta {
public:
    AmdOrderExecutionTableMeta() {
        colNames_ = {
//           证券代码           交易日期   交易时间  证券市场             证券类型         编号     来源种类      类型
            "HTSCSecurityID", "MDDate", "MDTime", "SecurityIDSource", "SecurityType", "Index", "SourceType", "Type", \
//   真实价格*10000   股数   买卖方向   冗余列    冗余列    逐笔数据序号   原始频道代码   数据接收时间戳
            "Price", "Qty", "BSFlag", "BuyNo", "SellNo", "ApplSeqNum", "ChannelNo", "ReceiveTime",
        };

        colTypes_ = {
            DT_SYMBOL, DT_DATE, DT_TIME, DT_SYMBOL, DT_SYMBOL, DT_LONG, DT_INT, DT_INT, \
            DT_LONG, DT_LONG, DT_INT, DT_LONG, DT_LONG, DT_LONG, DT_INT, DT_TIMESTAMP,
        };
    }

    ~AmdOrderExecutionTableMeta() {}

public:
    std::vector<string> colNames_;
    std::vector<DATA_TYPE> colTypes_;
};


TableSP getSnapshotSchema(bool receivedTimeFlag, bool dailyIndexFlag, bool outputElapsedFlag);
TableSP getExecutionSchema(bool receivedTimeFlag, bool dailyIndexFlag, bool outputElapsedFlag);
TableSP getOrderSchema(bool receivedTimeFlag, bool dailyIndexFlag, bool outputElapsedFlag);
TableSP getIndexSchema(bool receivedTimeFlag, bool dailyIndexFlag, bool outputElapsedFlag);
TableSP getOrderQueueSchema(bool receivedTimeFlag, bool dailyIndexFlag, bool outputElapsedFlag);
TableSP getOrderExecutionSchema(bool receivedTimeFlag, bool dailyIndexFlag, bool outputElapsedFlag);

// TODO(ruibinhuang@dolphindb.com): check the real attributes of the table
bool checkSchema(const string& type, TableSP table);
#endif // __AMD_QUOTE_TYPE_H
