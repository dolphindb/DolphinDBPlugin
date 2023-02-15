#ifndef __AMD_QUOTE_TYPE_H
#define __AMD_QUOTE_TYPE_H

#include "ama.h"

#include "CoreConcept.h"
#include "Util.h"
extern bool receivedTimeFlag;
extern bool dailyIndexFlag;

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

TableSP getSnapshotSchema(bool receivedTimeFlag, bool dailyIndexFlag);
TableSP getExecutionSchema(bool receivedTimeFlag, bool dailyIndexFlag);
TableSP getOrderSchema(bool receivedTimeFlag, bool dailyIndexFlag );
TableSP getIndexSchema(bool receivedTimeFlag, bool dailyIndexFlag);
TableSP getOrderQueueSchema(bool receivedTimeFlag, bool dailyIndexFlag);
// TODO(ruibinhuang@dolphindb.com): check the real attributes of the table
bool checkSchema(const string& type, TableSP table);
#endif // __AMD_QUOTE_TYPE_H
