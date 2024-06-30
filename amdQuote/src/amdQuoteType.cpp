#include "amdQuoteType.h"
#include "Types.h"
#ifndef AMD_USE_THREADED_QUEUE
#define AMD_USE_THREADED_QUEUE
#include "ddbplugin/ThreadedQueue.h"
#endif

using namespace ThreadedQueueUtil;
using namespace MarketUtil;

MetaTable AmdBondSnapshotTableMeta_4_0_1 = {
    {
    "marketType", "securityCode", "origTime", "tradingPhaseCode", "preClosePrice",
    "openPrice", "highPrice", "lowPrice", "lastPrice", "closePrice",

    "bidPrice1", "bidPrice2", "bidPrice3", "bidPrice4", "bidPrice5",
    "bidPrice6", "bidPrice7", "bidPrice8", "bidPrice9", "bidPrice10",
    "bidVolume1", "bidVolume2", "bidVolume3", "bidVolume4", "bidVolume5",
    "bidVolume6", "bidVolume7", "bidVolume8", "bidVolume9", "bidVolume10",
    "offerPrice1", "offerPrice2", "offerPrice3", "offerPrice4", "offerPrice5",
    "offerPrice6", "offerPrice7", "offerPrice8", "offerPrice9", "offerPrice10",
    "offerVolume1", "offerVolume2", "offerVolume3", "offerVolume4", "offerVolume5",
    "offerVolume6", "offerVolume7", "offerVolume8", "offerVolume9", "offerVolume10",

    "numTrades", "totalVolumeTrade", "totalValueTrade", "totalBidVolume", "totalOfferVolume",
    "weightedAvgBidPrice", "weightedAvgOfferPrice", "highLimited", "lowLimited", "change1",
    "change2", "weightedAvgBp", "preCloseWeightedAvgPrice", "auctLastPrice", "lastPriceTradingType",
    "channelNo", "mdStreamID", "instrumentStatus", "withdrawBuyNumber", "withdrawBuyAmount",
    "withdrawBuyMoney", "withdrawSellNumber", "withdrawSellAmount", "withdrawSellMoney", "totalBidNumber",
    "totalOfferNumber", "bidTradeMaxDuration", "offerTradeMaxDuration", "numBidOrders", "numOfferOrders",
    "lastTradeTime", "weightedAvgPrice", "noSubTradingPhaseCode", "subTradingPhaseCode1", "subTradingPhaseTradingType1",
    "subTradingPhaseCode2", "subTradingPhaseTradingType2", "subTradingPhaseCode3", "subTradingPhaseTradingType3", "subTradingPhaseCode4",
    "subTradingPhaseTradingType4", "subTradingPhaseCode5", "subTradingPhaseTradingType5","subTradingPhaseCode6", "subTradingPhaseTradingType6",
    "subTradingPhaseCode7", "subTradingPhaseTradingType7", "subTradingPhaseCode8", "subTradingPhaseTradingType8", "auctVolumeTrade",
    "auctValueTrade", "varietyCategory",
    },
    {
    DT_INT, DT_SYMBOL, DT_TIMESTAMP, DT_STRING, DT_LONG,
    DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,

    DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
    DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
    DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
    DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
    DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
    DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
    DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
    DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,

    DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
    DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
    DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_CHAR,
    DT_INT,  DT_STRING, DT_STRING, DT_LONG, DT_LONG,
    DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
    DT_LONG, DT_INT, DT_INT, DT_INT, DT_INT,
    DT_TIMESTAMP, DT_LONG, DT_LONG, DT_STRING, DT_CHAR,
    DT_STRING, DT_CHAR, DT_STRING, DT_CHAR, DT_STRING,
    DT_CHAR, DT_STRING, DT_CHAR, DT_STRING, DT_CHAR,
    DT_STRING, DT_CHAR, DT_STRING, DT_CHAR, DT_LONG,
    DT_LONG, DT_CHAR,
    }
};

MetaTable AmdSnapshotTableMeta = {
    {
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
    },
    {
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
    }
};


MetaTable AmdExecutionTableMeta = {
    {
    //  市场类型       证券代码         时间        频道号        频道编号       成交价格      成交数量      成交金额
        "marketType", "securityCode", "execTime", "channelNo", "applSeqNum", "execPrice", "execVolume", "valueTrade", \
    //  买方委托索引      卖方委托索引        买卖方向  成交类型   行情类别       业务序号     品种类别
        "bidAppSeqNum", "offerApplSeqNum", "side", "execType", "mdStreamId", "bizIndex", "varietyCategory",
    },
    {
        DT_INT, DT_SYMBOL, DT_TIMESTAMP, DT_INT, DT_LONG, DT_LONG, DT_LONG, DT_LONG, \
        DT_LONG, DT_LONG, DT_CHAR, DT_CHAR, DT_STRING, DT_LONG, DT_CHAR,
    }
};

MetaTable AmdOrderTableMeta {
        {
        //  市场类型       证券代码         频道号       频道索引       时间          委托价格      委托数量
            "marketType", "securityCode", "channelNo", "applSeqNum", "orderTime", "orderPrice", "orderVolume", \
        //  买卖方向 订单类别      行情类别(仅深圳市场有效) 原始订单号  业务序号   品种类别
            "side", "orderType", "mdStreamId", "origOrderNo", "bizIndex", "varietyCategory",
        },
        {
            DT_INT, DT_SYMBOL, DT_INT, DT_LONG, DT_TIMESTAMP, DT_LONG, DT_LONG, \
            DT_CHAR, DT_CHAR, DT_STRING, DT_LONG, DT_LONG, DT_CHAR,
        }
};

MetaTable AmdOrderTableMeta_4_0_1 {
        {
            "marketType", "securityCode", "channelNo", "applSeqNum", "orderTime",
            "orderPrice", "orderVolume", "side", "orderType", "mdStreamId",
            "origOrderNo", "bizIndex", "varietyCategory", "tradedOrderVolume"
        },
        {
            DT_INT, DT_SYMBOL, DT_INT, DT_LONG, DT_TIMESTAMP,
            DT_LONG, DT_LONG, DT_CHAR, DT_CHAR, DT_STRING,
            DT_LONG, DT_LONG, DT_CHAR, DT_LONG
        }
};

MetaTable AmdBondOrderTableMeta_4_0_1 {
        {
            "marketType", "securityCode", "channelNo", "applSeqNum", "orderTime",
            "orderPrice", "orderVolume", "side", "orderType", "mdStreamId",
            "productStatus", "origOrderNo", "varietyCategory"
        },
        {
            DT_INT, DT_SYMBOL, DT_INT, DT_LONG, DT_TIMESTAMP,
            DT_LONG, DT_LONG, DT_CHAR, DT_CHAR, DT_STRING,
            DT_STRING, DT_LONG, DT_CHAR
        }
};

MetaTable AmdIndexTableMeta {
    {
    //  市场类型       证券代码         时间        交易阶段代码         前收盘指数         今开盘指数   最高指数
        "marketType", "securityCode", "origTime", "tradingPhaseCode", "preCloseIndex", "openIndex", "highIndex", \
    //  最低指数     最新指数      收盘指数      交易数量             成交总金额          频道代码      行情类别
        "lowIndex", "lastIndex", "closeIndex", "totalVolumeTrade", "totalValueTrade", "channelNo", "mdStreamId",
    //  品种类别
        "varietyCategory",
    },
    {
        DT_INT, DT_SYMBOL, DT_TIMESTAMP, DT_STRING, DT_LONG, DT_LONG, DT_LONG, \
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_INT, DT_STRING,
        DT_CHAR,
    }
};

MetaTable AmdOrderQueueTableMeta {
    {
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
    },
    {
        DT_INT, DT_SYMBOL, DT_TIMESTAMP, DT_CHAR, DT_LONG, DT_LONG, DT_INT, DT_INT,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_INT, DT_STRING, DT_CHAR,
    }
};

MetaTable AmdOrderExecutionTableMeta {
    {
//           证券代码           交易日期   交易时间  证券市场             证券类型         编号     来源种类      类型
        "SecurityID", "MDDate", "MDTime", "SecurityIDSource", "SecurityType", "Index", "SourceType", "Type", \
//   真实价格*100000   股数   买卖方向   冗余列    冗余列    逐笔数据序号   原始频道代码   数据接收时间戳
        "Price", "Qty", "BSFlag", "BuyNo", "SellNo", "ApplSeqNum", "ChannelNo", "ReceiveTime",
    },

    {
        DT_SYMBOL, DT_DATE, DT_TIME, DT_SYMBOL, DT_SYMBOL, DT_LONG, DT_INT, DT_INT, \
        DT_LONG, DT_LONG, DT_INT, DT_LONG, DT_LONG, DT_LONG, DT_INT, DT_TIMESTAMP,
    }
};

MetaTable AmdFutureTableMeta {
    {
        "marketType", "securityCode", "actionDay", "origTime", "exchangeInstId",
        "lastPrice", "preSettlePrice", "preClosePrice", "preOpenInterest", "openPrice",
        "highPrice", "lowPrice", "totalVolumeTrade", "totalValueTrade", "openInterest",
        "closePrice", "settlePrice", "highLimited", "lowLimited", "preDelta",
        "currDelta", "bidPrice1", "bidPrice2", "bidPrice3", "bidPrice4",
        "bidPrice5", "bidVolume1", "bidVolume2", "bidVolume3", "bidVolume4",
        "bidVolume5", "offeRPrice1", "offerPrice2", "offerPrice3", "offerPrice4",
        "offerPrice5", "offerVolume1", "offerVolume2", "offerVolume3", "offerVolume4",
        "offerVolume5", "averagePrice", "tradingDay", "varietyCategory", "exchangeInstGroupid",
        "hisHighPrice", "hisLowPrice", "latestVolumeTrade", "initVolumeTrade", "changeVolumeTrade",
        "bidImplyVolume", "offerImplyVolume", "arbiType", "instrumentId1", "instrumentId2",
        "instrumentName", "totalBidVolumeTrade", "totalAskVolumeTrade",
    },
    {
        DT_INT, DT_STRING, DT_INT, DT_TIMESTAMP, DT_STRING,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_INT, DT_CHAR, DT_STRING,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_CHAR, DT_STRING, DT_STRING,
        DT_STRING, DT_LONG, DT_LONG
    }
};

MetaTable AmdOptionTableMeta {
    {
        "marketType","securityCode", "origTime", "preSettlePrice", "preClosePrice",
        "openPrice", "auctionPrice", "auctionVolume", "highPrice", "lowPrice",
        "lastPrice", "closePrice", "highLimited", "lowLimited", "bidPrice1",
        "bidPrice2", "bidPrice3", "bidPrice4", "bidPrice5", "bidVolume1",
        "bidVolume2", "bidVolume3", "bidVolume4", "bidVolume5", "offerPrice1",
        "offerPrice2", "offerPrice3", "offerPrice4", "offerPrice5", "offerVolume1",
        "offerVolume2", "offerVolume3", "offerVolume4", "offerVolume5", "settlePrice",
        "totalLongPosition", "totalVolumeTrade", "totalValueTrade", "tradingPhaseCode", "channelNo",
        "mdStreamId", "lastTradeTime", "refPrice", "varietyCategory", "contractType",
        "expireDate", "underlyingSecurityCode", "exercisePrice"
    },

    {
        DT_INT, DT_SYMBOL, DT_TIMESTAMP, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_SYMBOL, DT_INT,
        DT_SYMBOL, DT_TIMESTAMP, DT_LONG, DT_CHAR, DT_CHAR,
        DT_INT, DT_SYMBOL, DT_LONG
    }
};

#ifndef AMD_3_9_6
MetaTable AmdIOPVTableMeta {
    {
        "marketType", "securityCode", "origTime", "lastIopv",

        "bidIopv1", "bidIopv2", "bidIopv3", "bidIopv4", "bidIopv5",
        "bidIopv6", "bidIopv7", "bidIopv8", "bidIopv9", "bidIopv10",
        "offerIopv1", "offerIopv2", "offerIopv3", "offerIopv4", "offerIopv5",
        "offerIopv6", "offerIopv7", "offerIopv8", "offerIopv9", "offerIopv10",
    },

    {
        DT_INT, DT_SYMBOL, DT_TIMESTAMP, DT_LONG,

        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
    }
};
#endif

void initSecurityCodeToIntTypeContainer(MarketTypeContainer& container){
    auto indexMeta = AmdIndexTableMeta;
    indexMeta.colTypes_[1] = DT_INT;
    auto snapshotMeta = AmdSnapshotTableMeta;
    snapshotMeta.colTypes_[1] = DT_INT;
    auto orderMeta = AmdOrderTableMeta;
    orderMeta.colTypes_[1] = DT_INT;
    auto executionMeta = AmdExecutionTableMeta;
    executionMeta.colTypes_[1] = DT_INT;
    auto orderQueueMeta = AmdOrderQueueTableMeta;
    orderQueueMeta.colTypes_[1] = DT_INT;
    auto optionMeta = AmdOptionTableMeta;
    optionMeta.colTypes_[1] = DT_INT;
    auto futureMeta = AmdFutureTableMeta;
    futureMeta.colTypes_[1] = DT_INT;
    container.add("index", indexMeta);
    container.add("snapshot", snapshotMeta);
    container.add("order", orderMeta);
    container.add("execution", executionMeta);
    container.add("orderQueue", orderQueueMeta);
    container.add("orderExecution", AmdOrderExecutionTableMeta);
    container.add("option", optionMeta);
    container.add("future", futureMeta);
#ifndef AMD_3_9_6
    auto IOPVMeta = AmdIOPVTableMeta;
    IOPVMeta.colTypes_[1] = DT_INT;
    container.add("IOPV", IOPVMeta);

    auto bondSnashotMeta = AmdBondSnapshotTableMeta_4_0_1;
    bondSnashotMeta.colTypes_[1] = DT_INT;
    container.add("bondSnapshot_4.0.1", bondSnashotMeta);
    orderMeta = AmdOrderTableMeta_4_0_1;
    orderMeta.colTypes_[1] = DT_INT;
    container.add("order_4.0.1", orderMeta);
    auto bondOrder = AmdBondOrderTableMeta_4_0_1;
    bondOrder.colTypes_[1] = DT_INT;
    container.add("bondOrder_4.0.1", bondOrder);
#endif
}

void initTypeContainer(MarketTypeContainer& container){
    container.add("index", AmdIndexTableMeta);
    container.add("snapshot", AmdSnapshotTableMeta);
    container.add("order", AmdOrderTableMeta);
    container.add("execution", AmdExecutionTableMeta);
    container.add("orderQueue", AmdOrderQueueTableMeta);
    container.add("orderExecution", AmdOrderExecutionTableMeta);
    container.add("option", AmdOptionTableMeta);
    container.add("future", AmdFutureTableMeta);
#ifndef AMD_3_9_6
    container.add("IOPV", AmdIOPVTableMeta);
#endif
    container.add("bondSnapshot_4.0.1", AmdBondSnapshotTableMeta_4_0_1);
    container.add("order_4.0.1", AmdOrderTableMeta_4_0_1);
    container.add("bondOrder_4.0.1", AmdBondOrderTableMeta_4_0_1);
}

#ifndef AMD_3_9_6
void IOPVReader(vector<ConstantSP> &buffer, timeMDIOPV &data, bool securityCodeToInt) {
    int colNum = 0;
    ((VectorSP)(buffer[colNum++]))->appendInt(&(data.IOPV.market_type), 1);
    string securityCode = data.IOPV.security_code;
    if(securityCodeToInt) {
        int num = std::atoi(securityCode.c_str());
        ((VectorSP)(buffer[colNum++]))->appendInt(&num, 1);
    } else {
        ((VectorSP)(buffer[colNum++]))->appendString(&securityCode, 1);
    }
    long long origTime = convertTime(data.IOPV.orig_time);
    ((VectorSP)(buffer[colNum++]))->appendLong(&origTime, 1);
    long long last_iopv = data.IOPV.last_iopv;
    ((VectorSP)(buffer[colNum++]))->appendLong(&last_iopv, 1);

    for(int i = 0; i < 10; ++i) {
        long long bid_iopv = data.IOPV.bid_iopv[i];
        ((VectorSP)(buffer[colNum++]))->appendLong(&bid_iopv , 1);
    }
    for(int i = 0; i < 10; ++i) {
        long long offer_iopv = data.IOPV.offer_iopv[i];
        ((VectorSP)(buffer[colNum++]))->appendLong(&offer_iopv , 1);
    }
}
#endif

void futureReader(vector<ConstantSP> &buffer, timeMDFuture &data, bool securityCodeToInt) {
    int colNum = 0;
    int32_t   market_type = data.future.market_type;
    ((VectorSP)(buffer[colNum++]))->appendInt(&market_type, 1);

    string      security_code = data.future.security_code;
    if(securityCodeToInt) {
        int num = std::atoi(security_code.c_str());
        ((VectorSP)(buffer[colNum++]))->appendInt(&num, 1);
    } else {
        ((VectorSP)(buffer[colNum++]))->appendString(&security_code, 1);
    }

    int32_t   action_day = data.future.action_day;
    ((VectorSP)(buffer[colNum++]))->appendInt(&action_day, 1);
    long long   orig_time = convertTime(data.future.orig_time);
    ((VectorSP)(buffer[colNum++]))->appendLong(&orig_time, 1);
    string      exchange_inst_id = data.future.exchange_inst_id;
    ((VectorSP)(buffer[colNum++]))->appendString(&exchange_inst_id, 1);
    long long   last_price = data.future.last_price;
    ((VectorSP)(buffer[colNum++]))->appendLong(&last_price, 1);
    long long   pre_settle_price = data.future.pre_settle_price;
    ((VectorSP)(buffer[colNum++]))->appendLong(&pre_settle_price, 1);
    long long   pre_close_price = data.future.pre_close_price;
    ((VectorSP)(buffer[colNum++]))->appendLong(&pre_close_price, 1);
    long long   pre_open_interest = data.future.pre_open_interest;
    ((VectorSP)(buffer[colNum++]))->appendLong(&pre_open_interest, 1);
    long long   open_price = data.future.open_price;
    ((VectorSP)(buffer[colNum++]))->appendLong(&open_price, 1);
    long long   high_price = data.future.high_price;
    ((VectorSP)(buffer[colNum++]))->appendLong(&high_price, 1);
    long long   low_price = data.future.low_price;
    ((VectorSP)(buffer[colNum++]))->appendLong(&low_price, 1);
    long long   total_volume_trade = data.future.total_volume_trade;
    ((VectorSP)(buffer[colNum++]))->appendLong(&total_volume_trade, 1);
    long long   total_value_trade = data.future.total_value_trade;
    ((VectorSP)(buffer[colNum++]))->appendLong(&total_value_trade, 1);
    long long   open_interest = data.future.open_interest;
    ((VectorSP)(buffer[colNum++]))->appendLong(&open_interest, 1);
    long long   close_price = data.future.close_price;
    ((VectorSP)(buffer[colNum++]))->appendLong(&close_price, 1);
    long long   settle_price = data.future.settle_price;
    ((VectorSP)(buffer[colNum++]))->appendLong(&settle_price, 1);
    long long   high_limited = data.future.high_limited;
    ((VectorSP)(buffer[colNum++]))->appendLong(&high_limited, 1);
    long long   low_limited = data.future.low_limited;
    ((VectorSP)(buffer[colNum++]))->appendLong(&low_limited, 1);
    long long   pre_delta = data.future.pre_delta;
    ((VectorSP)(buffer[colNum++]))->appendLong(&pre_delta, 1);
    long long   curr_delta = data.future.curr_delta;
    ((VectorSP)(buffer[colNum++]))->appendLong(&curr_delta , 1);
    long long   bid_price1 = data.future.bid_price[0];
    ((VectorSP)(buffer[colNum++]))->appendLong(&bid_price1 , 1);
    long long   bid_price2 = data.future.bid_price[1];
    ((VectorSP)(buffer[colNum++]))->appendLong(&bid_price2 , 1);
    long long   bid_price3 = data.future.bid_price[2];
    ((VectorSP)(buffer[colNum++]))->appendLong(&bid_price3 , 1);
    long long   bid_price4 = data.future.bid_price[3];
    ((VectorSP)(buffer[colNum++]))->appendLong(&bid_price4 , 1);
    long long   bid_price5 = data.future.bid_price[4];
    ((VectorSP)(buffer[colNum++]))->appendLong(&bid_price5 , 1);
    long long   bid_volume1 = data.future.bid_volume[0];
    ((VectorSP)(buffer[colNum++]))->appendLong(&bid_volume1 , 1);
    long long   bid_volume2 = data.future.bid_volume[1];
    ((VectorSP)(buffer[colNum++]))->appendLong(&bid_volume2 , 1);
    long long   bid_volume3 = data.future.bid_volume[2];
    ((VectorSP)(buffer[colNum++]))->appendLong(&bid_volume3 , 1);
    long long   bid_volume4 = data.future.bid_volume[3];
    ((VectorSP)(buffer[colNum++]))->appendLong(&bid_volume4 , 1);
    long long   bid_volume5 = data.future.bid_volume[4];
    ((VectorSP)(buffer[colNum++]))->appendLong(&bid_volume5 , 1);
    long long   offer_price1 = data.future.offer_price[0];
    ((VectorSP)(buffer[colNum++]))->appendLong(&offer_price1 , 1);
    long long   offer_price2 = data.future.offer_price[1];
    ((VectorSP)(buffer[colNum++]))->appendLong(&offer_price2 , 1);
    long long   offer_price3 = data.future.offer_price[2];
    ((VectorSP)(buffer[colNum++]))->appendLong(&offer_price3, 1);
    long long   offer_price4 = data.future.offer_price[3];
    ((VectorSP)(buffer[colNum++]))->appendLong(&offer_price4 , 1);
    long long   offer_price5 = data.future.offer_price[4];
    ((VectorSP)(buffer[colNum++]))->appendLong(&offer_price5 , 1);
    long long   offer_volume1 = data.future.offer_volume[0];
    ((VectorSP)(buffer[colNum++]))->appendLong(&offer_volume1 , 1);
    long long   offer_volume2 = data.future.offer_volume[1];
    ((VectorSP)(buffer[colNum++]))->appendLong(&offer_volume2 , 1);
    long long   offer_volume3 = data.future.offer_volume[2];
    ((VectorSP)(buffer[colNum++]))->appendLong(&offer_volume3 , 1);
    long long   offer_volume4 = data.future.offer_volume[3];
    ((VectorSP)(buffer[colNum++]))->appendLong(&offer_volume4 , 1);
    long long   offer_volume5 = data.future.offer_volume[4];
    ((VectorSP)(buffer[colNum++]))->appendLong(&offer_volume5 , 1);
    long long   average_price = data.future.average_price;
    ((VectorSP)(buffer[colNum++]))->appendLong(&average_price , 1);
    int32_t   trading_day = data.future.trading_day;
    ((VectorSP)(buffer[colNum++]))->appendInt(&trading_day , 1);
    char   variety_category = data.future.variety_category;
    ((VectorSP)(buffer[colNum++]))->appendChar(&variety_category, 1);
    string      exchange_inst_groupid = data.future.exchange_inst_groupid;
    ((VectorSP)(buffer[colNum++]))->appendString(&exchange_inst_groupid , 1);
    long long   his_high_price = data.future.his_high_price;
    ((VectorSP)(buffer[colNum++]))->appendLong(&his_high_price , 1);
    long long   his_low_price = data.future.his_low_price;
    ((VectorSP)(buffer[colNum++]))->appendLong(&his_low_price , 1);
    long long   latest_volume_trade = data.future.latest_volume_trade;
    ((VectorSP)(buffer[colNum++]))->appendLong(&latest_volume_trade , 1);
    long long   init_volume_trade = data.future.init_volume_trade;
    ((VectorSP)(buffer[colNum++]))->appendLong(&init_volume_trade , 1);
    long long   change_volume_trade = data.future.change_volume_trade;
    ((VectorSP)(buffer[colNum++]))->appendLong(&change_volume_trade , 1);
    long long   bid_imply_volume = data.future.bid_imply_volume;
    ((VectorSP)(buffer[colNum++]))->appendLong(&bid_imply_volume , 1);
    long long   offer_imply_volume = data.future.offer_imply_volume;
    ((VectorSP)(buffer[colNum++]))->appendLong(&offer_imply_volume , 1);
    char      arbi_type = data.future.arbi_type;
    ((VectorSP)(buffer[colNum++]))->appendChar(&arbi_type, 1);
    string      instrument_id_1 = data.future.instrument_id_1;
    ((VectorSP)(buffer[colNum++]))->appendString(&instrument_id_1 , 1);
    string      instrument_id_2 = data.future.instrument_id_2;
    ((VectorSP)(buffer[colNum++]))->appendString(&instrument_id_2 , 1);
    string      instrument_name = data.future.instrument_name;
    ((VectorSP)(buffer[colNum++]))->appendString(&instrument_name , 1);
    long long   total_bid_volume_trade = data.future.total_bid_volume_trade;
    ((VectorSP)(buffer[colNum++]))->appendLong(&total_bid_volume_trade , 1);
    long long   total_ask_volume_trad = data.future.total_ask_volume_trade;
    ((VectorSP)(buffer[colNum++]))->appendLong(&total_ask_volume_trad , 1);
}

void optionReader(vector<ConstantSP> &buffer, timeMDOption &data, bool securityCodeToInt) {
    int colNum = 0;
    ((VectorSP)(buffer[colNum++]))->appendInt(&(data.option.market_type), 1);
    string securityCode = data.option.security_code;
    if(securityCodeToInt) {
        int num = std::atoi(securityCode.c_str());
        ((VectorSP)(buffer[colNum++]))->appendInt(&num, 1);
    } else {
        ((VectorSP)(buffer[colNum++]))->appendString(&securityCode, 1);
    }
    long long origTime = convertTime(data.option.orig_time);
    ((VectorSP)(buffer[colNum++]))->appendLong(&origTime, 1);
    long long preSettlePrice = data.option.pre_settle_price;
    ((VectorSP)(buffer[colNum++]))->appendLong(&preSettlePrice, 1);
    long long preClosePrice = data.option.pre_close_price;
    ((VectorSP)(buffer[colNum++]))->appendLong(&preClosePrice, 1);

    long long open_price = data.option.open_price;
    ((VectorSP)(buffer[colNum++]))->appendLong(&open_price, 1);
    long long auction_price = data.option.auction_price;
    ((VectorSP)(buffer[colNum++]))->appendLong(&auction_price, 1);
    long long auction_volume = data.option.auction_volume;
    ((VectorSP)(buffer[colNum++]))->appendLong(&auction_volume, 1);
    long long high_price = data.option.high_price;
    ((VectorSP)(buffer[colNum++]))->appendLong(&high_price, 1);
    long long low_price = data.option.low_price;
    ((VectorSP)(buffer[colNum++]))->appendLong(&low_price, 1);

    long long last_price = data.option.last_price;
    ((VectorSP)(buffer[colNum++]))->appendLong(&last_price, 1);
    long long close_price = data.option.close_price;
    ((VectorSP)(buffer[colNum++]))->appendLong(&close_price, 1);
    long long high_limited = data.option.high_limited;
    ((VectorSP)(buffer[colNum++]))->appendLong(&high_limited, 1);
    long long low_limited = data.option.low_limited;
    ((VectorSP)(buffer[colNum++]))->appendLong(&low_limited, 1);
    long long bid_price1 = data.option.bid_price[0];
    ((VectorSP)(buffer[colNum++]))->appendLong(&bid_price1, 1);

    long long bid_price2 = data.option.bid_price[1];
    ((VectorSP)(buffer[colNum++]))->appendLong(&bid_price2, 1);
    long long bid_price3 = data.option.bid_price[2];
    ((VectorSP)(buffer[colNum++]))->appendLong(&bid_price3, 1);
    long long bid_price4 = data.option.bid_price[3];
    ((VectorSP)(buffer[colNum++]))->appendLong(&bid_price4, 1);
    long long bid_price5 = data.option.bid_price[4];
    ((VectorSP)(buffer[colNum++]))->appendLong(&bid_price5, 1);
    long long bid_volume1 = data.option.bid_volume[0];
    ((VectorSP)(buffer[colNum++]))->appendLong(&bid_volume1, 1);

    long long bid_volume2 = data.option.bid_volume[1];
    ((VectorSP)(buffer[colNum++]))->appendLong(&bid_volume2, 1);
    long long bid_volume3 = data.option.bid_volume[2];
    ((VectorSP)(buffer[colNum++]))->appendLong(&bid_volume3, 1);
    long long bid_volume4 = data.option.bid_volume[3];
    ((VectorSP)(buffer[colNum++]))->appendLong(&bid_volume4, 1);
    long long bid_volume5 = data.option.bid_volume[4];
    ((VectorSP)(buffer[colNum++]))->appendLong(&bid_volume5, 1);
    long long offer_price1 = data.option.offer_price[0];
    ((VectorSP)(buffer[colNum++]))->appendLong(&offer_price1, 1);

    long long offer_price2 = data.option.offer_price[1];
    ((VectorSP)(buffer[colNum++]))->appendLong(&offer_price2, 1);
    long long offer_price3 = data.option.offer_price[2];
    ((VectorSP)(buffer[colNum++]))->appendLong(&offer_price3, 1);
    long long offer_price4 = data.option.offer_price[3];
    ((VectorSP)(buffer[colNum++]))->appendLong(&offer_price4, 1);
    long long offer_price5 = data.option.offer_price[4];
    ((VectorSP)(buffer[colNum++]))->appendLong(&offer_price5, 1);
    long long offer_volume1 = data.option.offer_volume[0];
    ((VectorSP)(buffer[colNum++]))->appendLong(&offer_volume1, 1);

    long long offer_volume2 = data.option.offer_volume[1];
    ((VectorSP)(buffer[colNum++]))->appendLong(&offer_volume2, 1);
    long long offer_volume3 = data.option.offer_volume[2];
    ((VectorSP)(buffer[colNum++]))->appendLong(&offer_volume3, 1);
    long long offer_volume4 = data.option.offer_volume[3];
    ((VectorSP)(buffer[colNum++]))->appendLong(&offer_volume4, 1);
    long long offer_volume5 = data.option.offer_volume[4];
    ((VectorSP)(buffer[colNum++]))->appendLong(&offer_volume5, 1);
    long long settle_price = data.option.settle_price;
    ((VectorSP)(buffer[colNum++]))->appendLong(&settle_price, 1);

    long long total_long_position = data.option.total_long_position;
    ((VectorSP)(buffer[colNum++]))->appendLong(&total_long_position, 1);
    long long total_volume_trade = data.option.total_volume_trade;
    ((VectorSP)(buffer[colNum++]))->appendLong(&total_volume_trade, 1);
    long long total_value_trade = data.option.total_value_trade;
    ((VectorSP)(buffer[colNum++]))->appendLong(&total_value_trade, 1);
    string trading_phase_code = data.option.trading_phase_code;
    ((VectorSP)(buffer[colNum++]))->appendString(&trading_phase_code, 1);
    ((VectorSP)(buffer[colNum++]))->appendInt(&(data.option.channel_no), 1);

    string mdStreamID = data.option.md_stream_id;
    ((VectorSP)(buffer[colNum++]))->appendString(&mdStreamID, 1);
    long long lastTradeTime = convertTime(data.option.last_trade_time);
    ((VectorSP)(buffer[colNum++]))->appendLong(&lastTradeTime, 1);
    long long ref_price = data.option.ref_price;
    ((VectorSP)(buffer[colNum++]))->appendLong(&ref_price, 1);
    char variety_category = data.option.variety_category;
    ((VectorSP)(buffer[colNum++]))->appendChar(&variety_category, 1);
    char contract_type = data.option.contract_type;
    ((VectorSP)(buffer[colNum++]))->appendChar(&contract_type, 1);

    ((VectorSP)(buffer[colNum++]))->appendInt(&(data.option.expire_date), 1);
    string underlying_security_code = data.option.underlying_security_code;
    ((VectorSP)(buffer[colNum++]))->appendString(&underlying_security_code, 1);
    long long exercise_price = data.option.exercise_price;
    ((VectorSP)(buffer[colNum++]))->appendLong(&exercise_price, 1);
}

void indexReader(vector<ConstantSP> &buffer, timeMDIndexSnapshot &data, bool securityCodeToInt) {
    int colNum = 0;
    int marketType = data.indexSnapshot.market_type;
    ((VectorSP)(buffer[colNum++]))->appendInt(&marketType, 1);
    string securityCode = data.indexSnapshot.security_code;
    if(securityCodeToInt) {
        int num = std::atoi(securityCode.c_str());
        ((VectorSP)(buffer[colNum++]))->appendInt(&num, 1);
    } else {
        ((VectorSP)(buffer[colNum++]))->appendString(&securityCode, 1);
    }
    long long origTime = convertTime(data.indexSnapshot.orig_time);
    ((VectorSP)(buffer[colNum++]))->appendLong(&origTime, 1);
    string tradingPhaseCode = data.indexSnapshot.trading_phase_code;
    ((VectorSP)(buffer[colNum++]))->appendString(&tradingPhaseCode, 1);
    long long preCloseIndex = data.indexSnapshot.pre_close_index;
    ((VectorSP)(buffer[colNum++]))->appendLong(&preCloseIndex, 1);
    long long openIndex = data.indexSnapshot.open_index;
    ((VectorSP)(buffer[colNum++]))->appendLong(&openIndex, 1);
    long long highIndex = data.indexSnapshot.high_index;
    ((VectorSP)(buffer[colNum++]))->appendLong(&highIndex, 1);

    long long lowIndex = data.indexSnapshot.low_index;
    ((VectorSP)(buffer[colNum++]))->appendLong(&lowIndex, 1);
    long long lastIndex = data.indexSnapshot.last_index;
    ((VectorSP)(buffer[colNum++]))->appendLong(&lastIndex, 1);
    long long closeIndex = data.indexSnapshot.close_index;
    ((VectorSP)(buffer[colNum++]))->appendLong(&closeIndex, 1);
    long long totalVolumeTrade = data.indexSnapshot.total_volume_trade;
    ((VectorSP)(buffer[colNum++]))->appendLong(&totalVolumeTrade, 1);
    long long totalValueTrade = data.indexSnapshot.total_value_trade;
    ((VectorSP)(buffer[colNum++]))->appendLong(&totalValueTrade, 1);
    int channelNo = data.indexSnapshot.channel_no;
    ((VectorSP)(buffer[colNum++]))->appendInt(&channelNo, 1);
    string mdStreamId = data.indexSnapshot.md_stream_id;
    ((VectorSP)(buffer[colNum++]))->appendString(&mdStreamId, 1);

    char varietyCategory = data.indexSnapshot.variety_category;
    ((VectorSP)(buffer[colNum++]))->appendChar(&varietyCategory, 1);
}

void orderQueueReader(vector<ConstantSP> &buffer, timeMDOrderQueue &data, bool securityCodeToInt) {
    int colNum = 0;
    int marketType = data.orderQueue.market_type;
    ((VectorSP)(buffer[colNum++]))->appendInt(&marketType, 1);
    string securityCode = data.orderQueue.security_code;
    if(securityCodeToInt) {
        int num = std::atoi(securityCode.c_str());
        ((VectorSP)(buffer[colNum++]))->appendInt(&num, 1);
    } else {
        ((VectorSP)(buffer[colNum++]))->appendString(&securityCode, 1);
    }
    long long orderTime = convertTime(data.orderQueue.order_time);
    ((VectorSP)(buffer[colNum++]))->appendLong(&orderTime, 1);
    char side = data.orderQueue.side;
    ((VectorSP)(buffer[colNum++]))->appendChar(&side, 1);
    long long orderPrice = data.orderQueue.order_price;
    ((VectorSP)(buffer[colNum++]))->appendLong(&orderPrice, 1);
    long long orderVolume = data.orderQueue.order_volume;
    ((VectorSP)(buffer[colNum++]))->appendLong(&orderVolume, 1);
    int numOfOrders = data.orderQueue.num_of_orders;
    ((VectorSP)(buffer[colNum++]))->appendInt(&numOfOrders, 1);
    int items = data.orderQueue.items;
    ((VectorSP)(buffer[colNum++]))->appendInt(&items, 1);

    for(int i = 0; i < 50; ++i) {
        long long volume = data.orderQueue.volume[i];
        ((VectorSP)(buffer[colNum++]))->appendLong(&volume, 1);
    }

    int channelNo = data.orderQueue.channel_no;
    ((VectorSP)(buffer[colNum++]))->appendInt(&channelNo, 1);
    string mdStreamId = data.orderQueue.md_stream_id;
    ((VectorSP)(buffer[colNum++]))->appendString(&mdStreamId, 1);
    char varietyCategory = data.orderQueue.variety_category;
    ((VectorSP)(buffer[colNum++]))->appendChar(&varietyCategory, 1);
}

void snapshotReader(vector<ConstantSP> &buffer, timeMDSnapshot &data, bool securityCodeToInt) {
    int colNum = 0;
    int marketType          = data.snapshot.market_type;
    ((Vector*)buffer[colNum++].get())->appendInt(&marketType, 1);
    string securityCode     = data.snapshot.security_code;
    if(securityCodeToInt) {
        int num = std::atoi(securityCode.c_str());
        ((VectorSP)(buffer[colNum++]))->appendInt(&num, 1);
    } else {
        ((VectorSP)(buffer[colNum++]))->appendString(&securityCode, 1);
    }
    long long origTime      = convertTime(data.snapshot.orig_time);
    ((Vector*)buffer[colNum++].get())->appendLong(&origTime, 1);
    string tradingPhaseCode = data.snapshot.trading_phase_code;
    ((Vector*)buffer[colNum++].get())->appendString(&tradingPhaseCode, 1);
    long long preClosePrice = data.snapshot.pre_close_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&preClosePrice, 1);
    long long openPrice     = data.snapshot.open_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&openPrice, 1);
    long long highPrice     = data.snapshot.high_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&highPrice, 1);
    long long lowPrice      = data.snapshot.low_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&lowPrice, 1);
    long long lastPrice     = data.snapshot.last_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&lastPrice, 1);
    long long closePrice    = data.snapshot.close_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&closePrice, 1);

    for(int i = 0; i < 10; ++i) {
        long long bidPrice = data.snapshot.bid_price[i];
        ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice, 1);
    }
    for(int i = 0; i < 10; ++i) {
        long long bidVolume = data.snapshot.bid_volume[i];
        ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume, 1);
    }
    for(int i = 0; i < 10; ++i) {
        long long offerPrice = data.snapshot.offer_price[i];
        ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice, 1);
    }
    for(int i = 0; i < 10; ++i) {
        long long offerVolume  = data.snapshot.offer_volume[i];
        ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume, 1);
    }

    long long numTrades             = data.snapshot.num_trades;
    ((Vector*)buffer[colNum++].get())->appendLong(&numTrades, 1);
    long long totalVolumeTrade      = data.snapshot.total_volume_trade;
    ((Vector*)buffer[colNum++].get())->appendLong(&totalVolumeTrade, 1);
    long long totalValueTrade       = data.snapshot.total_value_trade;
    ((Vector*)buffer[colNum++].get())->appendLong(&totalValueTrade, 1);
    long long totalBidVolume        = data.snapshot.total_bid_volume;
    ((Vector*)buffer[colNum++].get())->appendLong(&totalBidVolume, 1);
    long long totalOfferVolume      = data.snapshot.total_offer_volume;
    ((Vector*)buffer[colNum++].get())->appendLong(&totalOfferVolume, 1);
    long long weightedAvgBidPrice   = data.snapshot.weighted_avg_bid_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&weightedAvgBidPrice, 1);
    long long weightedAvgOfferPrice = data.snapshot.weighted_avg_offer_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&weightedAvgOfferPrice, 1);
    long long ioPV                  = data.snapshot.IOPV;
    ((Vector*)buffer[colNum++].get())->appendLong(&ioPV, 1);
    long long yieldToMaturity       = data.snapshot.yield_to_maturity;
    ((Vector*)buffer[colNum++].get())->appendLong(&yieldToMaturity, 1);
    long long highLimited           = data.snapshot.high_limited;
    ((Vector*)buffer[colNum++].get())->appendLong(&highLimited, 1);

    long long lowLimited             = data.snapshot.low_limited;
    ((Vector*)buffer[colNum++].get())->appendLong(&lowLimited, 1);
    long long priceEarningRatio1     = data.snapshot.price_earning_ratio1;
    ((Vector*)buffer[colNum++].get())->appendLong(&priceEarningRatio1, 1);
    long long priceEarningRatio2     = data.snapshot.price_earning_ratio2;
    ((Vector*)buffer[colNum++].get())->appendLong(&priceEarningRatio2, 1);
    long long change1                = data.snapshot.change1;
    ((Vector*)buffer[colNum++].get())->appendLong(&change1, 1);
    long long change2                = data.snapshot.change2;
    ((Vector*)buffer[colNum++].get())->appendLong(&change2, 1);
    int channelNo                    = data.snapshot.channel_no;
    ((Vector*)buffer[colNum++].get())->appendInt(&channelNo, 1);
    string mdStreamID                = data.snapshot.md_stream_id;
    ((Vector*)buffer[colNum++].get())->appendString(&mdStreamID, 1);
    string instrumentStatus          = data.snapshot.instrument_status;
    ((Vector*)buffer[colNum++].get())->appendString(&instrumentStatus, 1);
    long long preCloseIOPV           = data.snapshot.pre_close_iopv;
    ((Vector*)buffer[colNum++].get())->appendLong(&preCloseIOPV, 1);
    long long altWeightedAvgBidPrice = data.snapshot.alt_weighted_avg_bid_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&altWeightedAvgBidPrice, 1);

    long long altWeightedAvgOfferPrice = data.snapshot.alt_weighted_avg_offer_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&altWeightedAvgOfferPrice, 1);
    long long etfBuyNumber             = data.snapshot.etf_buy_number;
    ((Vector*)buffer[colNum++].get())->appendLong(&etfBuyNumber, 1);
    long long etfBuyAmount             = data.snapshot.etf_buy_amount;
    ((Vector*)buffer[colNum++].get())->appendLong(&etfBuyAmount, 1);
    long long etfBuyMoney              = data.snapshot.etf_buy_money;
    ((Vector*)buffer[colNum++].get())->appendLong(&etfBuyMoney, 1);
    long long etfSellNumber            = data.snapshot.etf_sell_number;
    ((Vector*)buffer[colNum++].get())->appendLong(&etfSellNumber, 1);
    long long etfSellAmount            = data.snapshot.etf_sell_amount;
    ((Vector*)buffer[colNum++].get())->appendLong(&etfSellAmount, 1);
    long long etfSellMoney             = data.snapshot.etf_sell_money;
    ((Vector*)buffer[colNum++].get())->appendLong(&etfSellMoney, 1);
    long long totalWarrantExecVolume   = data.snapshot.total_warrant_exec_volume;
    ((Vector*)buffer[colNum++].get())->appendLong(&totalWarrantExecVolume, 1);
    long long warLowerPrice            = data.snapshot.war_lower_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&warLowerPrice, 1);

    long long warUpperPrice       = data.snapshot.war_upper_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&warUpperPrice, 1);
    long long withdrawBuyNumber   = data.snapshot.withdraw_buy_number;
    ((Vector*)buffer[colNum++].get())->appendLong(&withdrawBuyNumber, 1);
    long long withdrawBuyAmount   = data.snapshot.withdraw_buy_amount;
    ((Vector*)buffer[colNum++].get())->appendLong(&withdrawBuyAmount, 1);
    long long withdrawBuyMoney    = data.snapshot.withdraw_buy_money;
    ((Vector*)buffer[colNum++].get())->appendLong(&withdrawBuyMoney, 1);
    long long withdrawSellNumber  = data.snapshot.withdraw_sell_number;
    ((Vector*)buffer[colNum++].get())->appendLong(&withdrawSellNumber, 1);
    long long withdrawSellAmount  = data.snapshot.withdraw_sell_amount;
    ((Vector*)buffer[colNum++].get())->appendLong(&withdrawSellAmount, 1);
    long long withdrawSellMoney   = data.snapshot.withdraw_sell_money;
    ((Vector*)buffer[colNum++].get())->appendLong(&withdrawSellMoney, 1);
    long long totalBidNumber      = data.snapshot.total_bid_number;
    ((Vector*)buffer[colNum++].get())->appendLong(&totalBidNumber, 1);
    long long totalOfferNumber    = data.snapshot.total_offer_number;
    ((Vector*)buffer[colNum++].get())->appendLong(&totalOfferNumber, 1);
    int bidTradeMaxDuration       = data.snapshot.bid_trade_max_duration;
    ((Vector*)buffer[colNum++].get())->appendInt(&bidTradeMaxDuration, 1);

    int offerTradeMaxDuration       = data.snapshot.offer_trade_max_duration;
    ((Vector*)buffer[colNum++].get())->appendInt(&offerTradeMaxDuration, 1);
    int numBidOrders                = data.snapshot.num_bid_orders;
    ((Vector*)buffer[colNum++].get())->appendInt(&numBidOrders, 1);
    long long numOfferOrders        = data.snapshot.num_offer_orders;
    ((Vector*)buffer[colNum++].get())->appendLong(&numOfferOrders, 1);
    long long lastTradeTime         = data.snapshot.last_trade_time;
    ((Vector*)buffer[colNum++].get())->appendLong(&lastTradeTime, 1);
    char varietyCategory            = data.snapshot.variety_category;
    ((Vector*)buffer[colNum++].get())->appendChar(&varietyCategory, 1);
}
void orderReader(vector<ConstantSP> &buffer, timeMDTickOrder &data, bool securityCodeToInt) {
    int colNum = 0;
    int marketType = data.order.market_type;
    ((VectorSP)(buffer[colNum++]))->appendInt(&marketType, 1);
    string securityCode(data.order.security_code);
    if(securityCodeToInt) {
        int num = std::atoi(securityCode.c_str());
        ((VectorSP)(buffer[colNum++]))->appendInt(&num, 1);
    } else {
        ((VectorSP)(buffer[colNum++]))->appendString(&securityCode, 1);
    }
    int channelNo = data.order.channel_no;
    ((VectorSP)(buffer[colNum++]))->appendInt(&channelNo, 1);
    long long applSeqNum = data.order.appl_seq_num;
    ((VectorSP)(buffer[colNum++]))->appendLong(&applSeqNum, 1);
    long long orderTime = convertTime(data.order.order_time);
    ((VectorSP)(buffer[colNum++]))->appendLong(&orderTime, 1);
    long long orderPrice = data.order.order_price;
    ((VectorSP)(buffer[colNum++]))->appendLong(&orderPrice, 1);
    long long orderVolume = data.order.order_volume;
    ((VectorSP)(buffer[colNum++]))->appendLong(&orderVolume, 1);
    int side = data.order.side;
    ((VectorSP)(buffer[colNum++]))->appendInt(&side, 1);
    int orderType = data.order.order_type;
    ((VectorSP)(buffer[colNum++]))->appendInt(&orderType, 1);
    string mdStreamId = data.order.md_stream_id;
    ((VectorSP)(buffer[colNum++]))->appendString(&mdStreamId, 1);
    long long origOrderNo = data.order.orig_order_no;
    ((VectorSP)(buffer[colNum++]))->appendLong(&origOrderNo, 1);
    long long bizIndex = data.order.biz_index;
    ((VectorSP)(buffer[colNum++]))->appendLong(&bizIndex, 1);
    int varietyCategory = data.order.variety_category;
    ((VectorSP)(buffer[colNum++]))->appendInt(&varietyCategory, 1);
}
void executionReader(vector<ConstantSP> &buffer, timeMDTickExecution &data, bool securityCodeToInt) {
    int colNum = 0;
    int marketType = data.execution.market_type;
    ((VectorSP)(buffer[colNum++]))->appendInt(&marketType, 1);
    string securityCode = data.execution.security_code;
    if(securityCodeToInt) {
        int num = std::atoi(securityCode.c_str());
        ((VectorSP)(buffer[colNum++]))->appendInt(&num, 1);
    } else {
        ((VectorSP)(buffer[colNum++]))->appendString(&securityCode, 1);
    }
    long long execTime = convertTime(data.execution.exec_time);
    ((VectorSP)(buffer[colNum++]))->appendLong(&execTime, 1);
    int channelNo = data.execution.channel_no;
    ((VectorSP)(buffer[colNum++]))->appendInt(&channelNo, 1);
    long long applSeqNum = data.execution.appl_seq_num;
    ((VectorSP)(buffer[colNum++]))->appendLong(&applSeqNum, 1);
    long long execPrice = data.execution.exec_price;
    ((VectorSP)(buffer[colNum++]))->appendLong(&execPrice, 1);
    long long execVolume = data.execution.exec_volume;
    ((VectorSP)(buffer[colNum++]))->appendLong(&execVolume, 1);
    long long valueTrade = data.execution.value_trade;
    ((VectorSP)(buffer[colNum++]))->appendLong(&valueTrade, 1);

    long long bidAppSeqNum = data.execution.bid_appl_seq_num;
    ((VectorSP)(buffer[colNum++]))->appendLong(&bidAppSeqNum, 1);
    long long offerApplSeqNum = data.execution.offer_appl_seq_num;
    ((VectorSP)(buffer[colNum++]))->appendLong(&offerApplSeqNum, 1);
    char side = data.execution.side;
    ((VectorSP)(buffer[colNum++]))->appendChar(&side, 1);
    char execType = data.execution.exec_type;
    ((VectorSP)(buffer[colNum++]))->appendChar(&execType, 1);
    string mdStreamId = data.execution.md_stream_id;
    ((VectorSP)(buffer[colNum++]))->appendString(&mdStreamId, 1);
    long long bizIndex = data.execution.biz_index;
    ((VectorSP)(buffer[colNum++]))->appendLong(&bizIndex, 1);
    char varietyCategory = data.execution.variety_category;
    ((VectorSP)(buffer[colNum++]))->appendChar(&varietyCategory, 1);
}
void orderExecutionReader(vector<ConstantSP> &buffer, MDOrderExecution &data, int seqCheckMode,
                            std::unordered_map<int, long long> &szLastSeqNum,
                            std::unordered_map<int, long long> &shLastSeqNum) {
    ConstantVecIterator iter = buffer.begin();
    if(data.orderOrExecution) {
        int marketType = data.uni.tickOrder.market_type;
        uint8_t varietyCategory = data.uni.tickOrder.variety_category;
        string securityCode;
        if(marketType == amd::ama::MarketType::kSSE) {
            securityCode = data.uni.tickOrder.security_code + string(".SH");
            MarketUtil::checkSeqNum("", "XSHG Order", shLastSeqNum, data.uni.tickOrder.appl_seq_num,
                                    data.uni.tickOrder.channel_no, seqCheckMode);
        } else if (marketType == amd::ama::MarketType::kSZSE) {
            securityCode = data.uni.tickOrder.security_code + string(".SZ");
            MarketUtil::checkSeqNum("", "XSHE Order", szLastSeqNum, data.uni.tickOrder.appl_seq_num,
                                    data.uni.tickOrder.channel_no, seqCheckMode);
        }
        int orderDate = convertToDate(data.uni.tickOrder.order_time);
        int orderTime = convertToTime(data.uni.tickOrder.order_time);
        string securityIDSource = transMarket(data.uni.tickOrder.market_type);
        string securityType = varietyCategory == amd::ama::VarietyCategory::kStock ? "StockType": "FundType";
        int sourceType = OrderBookMsgType::ORDER;
        if (data.uni.tickOrder.order_type == 'S') {
            sourceType = OrderBookMsgType::PRODUCT_STATUS;
        }
        int type = convertType(data.uni.tickOrder.order_type);
        int BSFlag = convertBSFlag(data.uni.tickOrder.side);

        appendString(iter++, securityCode);
        appendInt(iter++, orderDate);
        appendInt(iter++, orderTime);
        appendString(iter++, securityIDSource);
        appendString(iter++, securityType);
        appendInt(iter++, data.uni.tickOrder.appl_seq_num); // use appl_seq_num as index
        appendInt(iter++, sourceType);
        appendInt(iter++, type);
        appendLong(iter++, data.uni.tickOrder.order_price);
        appendLong(iter++, data.uni.tickOrder.order_volume);
        appendInt(iter++, BSFlag);
        appendLong(iter++, data.uni.tickOrder.orig_order_no);
        appendLong(iter++, data.uni.tickOrder.orig_order_no); // same as the previous one
        appendLong(iter++, data.uni.tickOrder.appl_seq_num);
        appendInt(iter++, data.uni.tickOrder.channel_no);
        appendLong(iter++, data.reachTime / 1000000);
    } else {
        int marketType = data.uni.tickExecution.market_type;
        uint8_t varietyCategory = data.uni.tickExecution.variety_category;
        string securityCode;
        if(marketType == amd::ama::MarketType::kSSE) {
            securityCode = data.uni.tickExecution.security_code + string(".SH");
            MarketUtil::checkSeqNum("", "XSHG Execution", shLastSeqNum, data.uni.tickExecution.appl_seq_num,
                                    data.uni.tickExecution.channel_no, seqCheckMode);
        } else if (marketType == amd::ama::MarketType::kSZSE) {
            securityCode = data.uni.tickExecution.security_code + string(".SZ");
            MarketUtil::checkSeqNum("", "XSHG Execution", szLastSeqNum, data.uni.tickExecution.appl_seq_num,
                                    data.uni.tickExecution.channel_no, seqCheckMode);
        }
        int orderDate = convertToDate(data.uni.tickExecution.exec_time);
        int orderTime = convertToTime(data.uni.tickExecution.exec_time);
        string securityIDSource = transMarket(data.uni.tickExecution.market_type);
        string securityType = varietyCategory == amd::ama::VarietyCategory::kStock ? "StockType": "FundType";
        int sourceType = OrderBookMsgType::TRADE;
        int type = convertType(data.uni.tickExecution.exec_type);
        int BSFlag = convertBSFlag(data.uni.tickExecution.side, data.uni.tickExecution.bid_appl_seq_num,
                                   data.uni.tickExecution.offer_appl_seq_num);

        appendString(iter++, securityCode);
        appendInt(iter++, orderDate);
        appendInt(iter++, orderTime);
        appendString(iter++, securityIDSource);
        appendString(iter++, securityType);
        appendInt(iter++, data.uni.tickExecution.appl_seq_num); // use appl_seq_num as index
        appendInt(iter++, sourceType);
        appendInt(iter++, type);
        appendLong(iter++, data.uni.tickExecution.exec_price);
        appendLong(iter++, data.uni.tickExecution.exec_volume);
        appendInt(iter++, BSFlag);
        appendLong(iter++, data.uni.tickExecution.bid_appl_seq_num);
        appendLong(iter++, data.uni.tickExecution.offer_appl_seq_num);
        appendLong(iter++, data.uni.tickExecution.appl_seq_num);
        appendInt(iter++, data.uni.tickExecution.channel_no);
        appendLong(iter++, data.reachTime / 1000000);
    }
}
void bondSnapshotReader(vector<ConstantSP> &buffer, timeMDBondSnapshot &data, bool securityCodeToInt) {
    int colNum = 0;
    int marketType          = data.bondSnapshot.market_type;
    ((Vector*)buffer[colNum++].get())->appendInt(&marketType, 1);
    string securityCode     = data.bondSnapshot.security_code;
    if (securityCodeToInt) {
        int num = std::atoi(securityCode.c_str());
        ((VectorSP)(buffer[colNum++]))->appendInt(&num, 1);
    } else {
        ((VectorSP)(buffer[colNum++]))->appendString(&securityCode, 1);
    }
    long long origTime      = convertTime(data.bondSnapshot.orig_time);
    ((Vector*)buffer[colNum++].get())->appendLong(&origTime, 1);
    string tradingPhaseCode = data.bondSnapshot.trading_phase_code;
    ((Vector*)buffer[colNum++].get())->appendString(&tradingPhaseCode, 1);
    long long preClosePrice = data.bondSnapshot.pre_close_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&preClosePrice, 1);
    long long openPrice     = data.bondSnapshot.open_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&openPrice, 1);
    long long highPrice     = data.bondSnapshot.high_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&highPrice, 1);
    long long lowPrice      = data.bondSnapshot.low_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&lowPrice, 1);
    long long lastPrice     = data.bondSnapshot.last_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&lastPrice, 1);
    long long closePrice    = data.bondSnapshot.close_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&closePrice, 1);

    for(int i = 0; i < 10; ++i) {
        long long bidPrice = data.bondSnapshot.bid_price[i];
        ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice, 1);
    }
    for(int i = 0; i < 10; ++i) {
        long long bidVolume = data.bondSnapshot.bid_volume[i];
        ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume, 1);
    }
    for(int i = 0; i < 10; ++i) {
        long long offerPrice = data.bondSnapshot.offer_price[i];
        ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice, 1);
    }
    for(int i = 0; i < 10; ++i) {
        long long offerVolume  = data.bondSnapshot.offer_volume[i];
        ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume, 1);
    }

    long long numTrades             = data.bondSnapshot.num_trades;
    ((Vector*)buffer[colNum++].get())->appendLong(&numTrades, 1);
    long long totalVolumeTrade      = data.bondSnapshot.total_volume_trade;
    ((Vector*)buffer[colNum++].get())->appendLong(&totalVolumeTrade, 1);
    long long totalValueTrade       = data.bondSnapshot.total_value_trade;
    ((Vector*)buffer[colNum++].get())->appendLong(&totalValueTrade, 1);
    long long totalBidVolume        = data.bondSnapshot.total_bid_volume;
    ((Vector*)buffer[colNum++].get())->appendLong(&totalBidVolume, 1);
    long long totalOfferVolume      = data.bondSnapshot.total_offer_volume;
    ((Vector*)buffer[colNum++].get())->appendLong(&totalOfferVolume, 1);
    long long weightedAvgBidPrice   = data.bondSnapshot.weighted_avg_bid_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&weightedAvgBidPrice, 1);
    long long weightedAvgOfferPrice = data.bondSnapshot.weighted_avg_offer_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&weightedAvgOfferPrice, 1);
    long long ioPV                  = 0;
    ((Vector*)buffer[colNum++].get())->appendLong(&ioPV, 1);
    long long yieldToMaturity       = LONG_LONG_MIN;
    ((Vector*)buffer[colNum++].get())->appendLong(&yieldToMaturity, 1);
    long long highLimited           = data.bondSnapshot.high_limited;
    ((Vector*)buffer[colNum++].get())->appendLong(&highLimited, 1);

    long long lowLimited             = data.bondSnapshot.low_limited;
    ((Vector*)buffer[colNum++].get())->appendLong(&lowLimited, 1);
    long long priceEarningRatio1     = LONG_LONG_MIN;
    ((Vector*)buffer[colNum++].get())->appendLong(&priceEarningRatio1, 1);
    long long priceEarningRatio2     = LONG_LONG_MIN;
    ((Vector*)buffer[colNum++].get())->appendLong(&priceEarningRatio2, 1);
    long long change1                = data.bondSnapshot.change1;
    ((Vector*)buffer[colNum++].get())->appendLong(&change1, 1);
    long long change2                = data.bondSnapshot.change2;
    ((Vector*)buffer[colNum++].get())->appendLong(&change2, 1);
    int channelNo                    = data.bondSnapshot.channel_no;
    ((Vector*)buffer[colNum++].get())->appendInt(&channelNo, 1);
    string mdStreamID                = data.bondSnapshot.md_stream_id;
    ((Vector*)buffer[colNum++].get())->appendString(&mdStreamID, 1);
    string instrumentStatus          = data.bondSnapshot.instrument_status;
    ((Vector*)buffer[colNum++].get())->appendString(&instrumentStatus, 1);
    long long preCloseIOPV           = LONG_LONG_MIN;
    ((Vector*)buffer[colNum++].get())->appendLong(&preCloseIOPV, 1);
    long long altWeightedAvgBidPrice = LONG_LONG_MIN;
    ((Vector*)buffer[colNum++].get())->appendLong(&altWeightedAvgBidPrice, 1);

    long long altWeightedAvgOfferPrice = LONG_LONG_MIN;
    ((Vector*)buffer[colNum++].get())->appendLong(&altWeightedAvgOfferPrice, 1);
    long long etfBuyNumber             = LONG_LONG_MIN;
    ((Vector*)buffer[colNum++].get())->appendLong(&etfBuyNumber, 1);
    long long etfBuyAmount             = LONG_LONG_MIN;
    ((Vector*)buffer[colNum++].get())->appendLong(&etfBuyAmount, 1);
    long long etfBuyMoney              = LONG_LONG_MIN;
    ((Vector*)buffer[colNum++].get())->appendLong(&etfBuyMoney, 1);
    long long etfSellNumber            = LONG_LONG_MIN;
    ((Vector*)buffer[colNum++].get())->appendLong(&etfSellNumber, 1);
    long long etfSellAmount            = LONG_LONG_MIN;
    ((Vector*)buffer[colNum++].get())->appendLong(&etfSellAmount, 1);
    long long etfSellMoney             = LONG_LONG_MIN;
    ((Vector*)buffer[colNum++].get())->appendLong(&etfSellMoney, 1);
    long long totalWarrantExecVolume   = LONG_LONG_MIN;
    ((Vector*)buffer[colNum++].get())->appendLong(&totalWarrantExecVolume, 1);
    long long warLowerPrice            = LONG_LONG_MIN;
    ((Vector*)buffer[colNum++].get())->appendLong(&warLowerPrice, 1);

    long long warUpperPrice       = LONG_LONG_MIN;
    ((Vector*)buffer[colNum++].get())->appendLong(&warUpperPrice, 1);
    long long withdrawBuyNumber   = data.bondSnapshot.withdraw_buy_number;
    ((Vector*)buffer[colNum++].get())->appendLong(&withdrawBuyNumber, 1);
    long long withdrawBuyAmount   = data.bondSnapshot.withdraw_buy_amount;
    ((Vector*)buffer[colNum++].get())->appendLong(&withdrawBuyAmount, 1);
    long long withdrawBuyMoney    = data.bondSnapshot.withdraw_buy_money;
    ((Vector*)buffer[colNum++].get())->appendLong(&withdrawBuyMoney, 1);
    long long withdrawSellNumber  = data.bondSnapshot.withdraw_sell_number;
    ((Vector*)buffer[colNum++].get())->appendLong(&withdrawSellNumber, 1);
    long long withdrawSellAmount  = data.bondSnapshot.withdraw_sell_amount;
    ((Vector*)buffer[colNum++].get())->appendLong(&withdrawSellAmount, 1);
    long long withdrawSellMoney   = data.bondSnapshot.withdraw_sell_money;
    ((Vector*)buffer[colNum++].get())->appendLong(&withdrawSellMoney, 1);
    long long totalBidNumber      = data.bondSnapshot.total_bid_number;
    ((Vector*)buffer[colNum++].get())->appendLong(&totalBidNumber, 1);
    long long totalOfferNumber    = data.bondSnapshot.total_offer_number;
    ((Vector*)buffer[colNum++].get())->appendLong(&totalOfferNumber, 1);
    int bidTradeMaxDuration       = data.bondSnapshot.bid_trade_max_duration;
    ((Vector*)buffer[colNum++].get())->appendInt(&bidTradeMaxDuration, 1);

    int offerTradeMaxDuration       = data.bondSnapshot.offer_trade_max_duration;
    ((Vector*)buffer[colNum++].get())->appendInt(&offerTradeMaxDuration, 1);
    int numBidOrders                = data.bondSnapshot.num_bid_orders;
    ((Vector*)buffer[colNum++].get())->appendInt(&numBidOrders, 1);
    long long numOfferOrders        = data.bondSnapshot.num_offer_orders;
    ((Vector*)buffer[colNum++].get())->appendLong(&numOfferOrders, 1);
    long long lastTradeTime         = data.bondSnapshot.last_trade_time;
    ((Vector*)buffer[colNum++].get())->appendLong(&lastTradeTime, 1);
    char varietyCategory            = data.bondSnapshot.variety_category;
    ((Vector*)buffer[colNum++].get())->appendChar(&varietyCategory, 1);
}
void bondOrderReader(vector<ConstantSP> &buffer, timeMDBondTickOrder &data, bool securityCodeToInt) {
    int colNum = 0;
    int marketType = data.bondOrder.market_type;
    ((VectorSP)(buffer[colNum++]))->appendInt(&marketType, 1);
    string securityCode(data.bondOrder.security_code);
    if(securityCodeToInt) {
        int num = std::atoi(securityCode.c_str());
        ((VectorSP)(buffer[colNum++]))->appendInt(&num, 1);
    } else {
        ((VectorSP)(buffer[colNum++]))->appendString(&securityCode, 1);
    }
    int channelNo = data.bondOrder.channel_no;
    ((VectorSP)(buffer[colNum++]))->appendInt(&channelNo, 1);
    long long applSeqNum = data.bondOrder.appl_seq_num;
    ((VectorSP)(buffer[colNum++]))->appendLong(&applSeqNum, 1);
    long long orderTime = convertTime(data.bondOrder.order_time);
    ((VectorSP)(buffer[colNum++]))->appendLong(&orderTime, 1);
    long long orderPrice = data.bondOrder.order_price;
    ((VectorSP)(buffer[colNum++]))->appendLong(&orderPrice, 1);
    long long orderVolume = data.bondOrder.order_volume;
    ((VectorSP)(buffer[colNum++]))->appendLong(&orderVolume, 1);
    int side = data.bondOrder.side;
    ((VectorSP)(buffer[colNum++]))->appendInt(&side, 1);
    int orderType = data.bondOrder.order_type;
    ((VectorSP)(buffer[colNum++]))->appendInt(&orderType, 1);
    string mdStreamId = data.bondOrder.md_stream_id;
    ((VectorSP)(buffer[colNum++]))->appendString(&mdStreamId, 1);
    long long origOrderNo = data.bondOrder.orig_order_no;
    ((VectorSP)(buffer[colNum++]))->appendLong(&origOrderNo, 1);
    long long bizIndex = LONG_LONG_MIN;
    ((VectorSP)(buffer[colNum++]))->appendLong(&bizIndex, 1);
    int varietyCategory = data.bondOrder.variety_category;
    ((VectorSP)(buffer[colNum++]))->appendInt(&varietyCategory, 1);
}
void bondExecutionReader(vector<ConstantSP> &buffer, timeMDBondTickExecution &data, bool securityCodeToInt) {
    int colNum = 0;
    int marketType = data.bondExecution.market_type;
    ((VectorSP)(buffer[colNum++]))->appendInt(&marketType, 1);
    string securityCode = data.bondExecution.security_code;
    if (securityCodeToInt) {
        int num = std::atoi(securityCode.c_str());
        ((VectorSP)(buffer[colNum++]))->appendInt(&num, 1);
    } else {
        ((VectorSP)(buffer[colNum++]))->appendString(&securityCode, 1);
    }
    long long execTime = convertTime(data.bondExecution.exec_time);
    ((VectorSP)(buffer[colNum++]))->appendLong(&execTime, 1);
    int channelNo = data.bondExecution.channel_no;
    ((VectorSP)(buffer[colNum++]))->appendInt(&channelNo, 1);
    long long applSeqNum = data.bondExecution.appl_seq_num;
    ((VectorSP)(buffer[colNum++]))->appendLong(&applSeqNum, 1);
    long long execPrice = data.bondExecution.exec_price;
    ((VectorSP)(buffer[colNum++]))->appendLong(&execPrice, 1);
    long long execVolume = data.bondExecution.exec_volume;
    ((VectorSP)(buffer[colNum++]))->appendLong(&execVolume, 1);
    long long valueTrade = data.bondExecution.value_trade;
    ((VectorSP)(buffer[colNum++]))->appendLong(&valueTrade, 1);

    long long bidAppSeqNum = data.bondExecution.bid_appl_seq_num;
    ((VectorSP)(buffer[colNum++]))->appendLong(&bidAppSeqNum, 1);
    long long offerApplSeqNum = data.bondExecution.offer_appl_seq_num;
    ((VectorSP)(buffer[colNum++]))->appendLong(&offerApplSeqNum, 1);
    char side = data.bondExecution.side;
    ((VectorSP)(buffer[colNum++]))->appendChar(&side, 1);
    char execType = data.bondExecution.exec_type;
    ((VectorSP)(buffer[colNum++]))->appendChar(&execType, 1);
    string mdStreamId = data.bondExecution.md_stream_id;
    ((VectorSP)(buffer[colNum++]))->appendString(&mdStreamId, 1);
    long long bizIndex = LONG_LONG_MIN;
    ((VectorSP)(buffer[colNum++]))->appendLong(&bizIndex, 1);
    char varietyCategory = data.bondExecution.variety_category;
    ((VectorSP)(buffer[colNum++]))->appendChar(&varietyCategory, 1);
}

void bondOrderExecutionReader(vector<ConstantSP> &buffer, MDBondOrderExecution &data, int seqCheckMode,
                            std::unordered_map<int, long long> &szLastSeqNum,
                            std::unordered_map<int, long long> &shLastSeqNum) {
    ConstantVecIterator iter = buffer.begin();
    if(data.orderOrExecution) {
        int marketType = data.uni.tickOrder.market_type;
        string securityCode;
        if(marketType == amd::ama::MarketType::kSSE) {
            securityCode = data.uni.tickOrder.security_code + string(".SH");
            MarketUtil::checkSeqNum("", "XSHG BondOrder", shLastSeqNum, data.uni.tickOrder.appl_seq_num,
                                    data.uni.tickOrder.channel_no, seqCheckMode);
        } else if (marketType == amd::ama::MarketType::kSZSE) {
            securityCode = data.uni.tickOrder.security_code + string(".SZ");
            MarketUtil::checkSeqNum("", "XSHE BondOrder", szLastSeqNum, data.uni.tickOrder.appl_seq_num,
                                    data.uni.tickOrder.channel_no, seqCheckMode);
        }
        int orderDate = convertToDate(data.uni.tickOrder.order_time);
        int orderTime = convertToTime(data.uni.tickOrder.order_time);
        string securityIDSource = transMarket(data.uni.tickOrder.market_type);
        string securityType = "BondType";
        int sourceType = OrderBookMsgType::ORDER;
        if (data.uni.tickOrder.order_type == 'S') {
            sourceType = OrderBookMsgType::PRODUCT_STATUS;
        }
        int type = convertType(data.uni.tickOrder.order_type);
        int BSFlag = convertBSFlag(data.uni.tickOrder.side);

        appendString(iter++, securityCode);
        appendInt(iter++, orderDate);
        appendInt(iter++, orderTime);
        appendString(iter++, securityIDSource);
        appendString(iter++, securityType);
        appendInt(iter++, data.uni.tickOrder.appl_seq_num); // use appl_seq_num as index
        appendInt(iter++, sourceType);
        appendInt(iter++, type);
        appendLong(iter++, data.uni.tickOrder.order_price);
        appendLong(iter++, data.uni.tickOrder.order_volume);
        appendInt(iter++, BSFlag);
        appendLong(iter++, data.uni.tickOrder.orig_order_no);
        appendLong(iter++, data.uni.tickOrder.orig_order_no); // same as the previous one
        appendLong(iter++, data.uni.tickOrder.appl_seq_num);
        appendInt(iter++, data.uni.tickOrder.channel_no);
        appendLong(iter++, data.reachTime / 1000000);
    } else {
        int marketType = data.uni.tickExecution.market_type;
        string securityCode;
        if(marketType == amd::ama::MarketType::kSSE) {
            securityCode = data.uni.tickExecution.security_code + string(".SH");
            MarketUtil::checkSeqNum("", "XSHG BondExecution", shLastSeqNum, data.uni.tickExecution.appl_seq_num,
                                    data.uni.tickExecution.channel_no, seqCheckMode);
        } else if (marketType == amd::ama::MarketType::kSZSE) {
            securityCode = data.uni.tickExecution.security_code + string(".SZ");
            MarketUtil::checkSeqNum("", "XSHG BondExecution", szLastSeqNum, data.uni.tickExecution.appl_seq_num,
                                    data.uni.tickExecution.channel_no, seqCheckMode);
        }
        int orderDate = convertToDate(data.uni.tickExecution.exec_time);
        int orderTime = convertToTime(data.uni.tickExecution.exec_time);
        string securityIDSource = transMarket(data.uni.tickExecution.market_type);
        string securityType = "BondType";
        int sourceType = OrderBookMsgType::TRADE;
        int type = convertType(data.uni.tickExecution.exec_type);
        int BSFlag = convertBSFlag(data.uni.tickExecution.side, data.uni.tickExecution.bid_appl_seq_num,
                                   data.uni.tickExecution.offer_appl_seq_num);

        appendString(iter++, securityCode);
        appendInt(iter++, orderDate);
        appendInt(iter++, orderTime);
        appendString(iter++, securityIDSource);
        appendString(iter++, securityType);
        appendInt(iter++, data.uni.tickExecution.appl_seq_num); // use appl_seq_num as index
        appendInt(iter++, sourceType);
        appendInt(iter++, type);
        appendLong(iter++, data.uni.tickExecution.exec_price);
        appendLong(iter++, data.uni.tickExecution.exec_volume);
        appendInt(iter++, BSFlag);
        appendLong(iter++, data.uni.tickExecution.bid_appl_seq_num);
        appendLong(iter++, data.uni.tickExecution.offer_appl_seq_num);
        appendLong(iter++, data.uni.tickExecution.appl_seq_num);
        appendInt(iter++, data.uni.tickExecution.channel_no);
        appendLong(iter++, data.reachTime / 1000000);
    }
}

void orderReader_4_0_1(vector<ConstantSP> &buffer, timeMDTickOrder &data, bool securityCodeToInt){
    int colNum = 0;
    int marketType = data.order.market_type;
    ((VectorSP)(buffer[colNum++]))->appendInt(&marketType, 1);
    string securityCode(data.order.security_code);
    if(securityCodeToInt) {
        int num = std::atoi(securityCode.c_str());
        ((VectorSP)(buffer[colNum++]))->appendInt(&num, 1);
    } else {
        ((VectorSP)(buffer[colNum++]))->appendString(&securityCode, 1);
    }
    int channelNo = data.order.channel_no;
    ((VectorSP)(buffer[colNum++]))->appendInt(&channelNo, 1);
    long long applSeqNum = data.order.appl_seq_num;
    ((VectorSP)(buffer[colNum++]))->appendLong(&applSeqNum, 1);
    long long orderTime = convertTime(data.order.order_time);
    ((VectorSP)(buffer[colNum++]))->appendLong(&orderTime, 1);
    long long orderPrice = data.order.order_price;
    ((VectorSP)(buffer[colNum++]))->appendLong(&orderPrice, 1);
    long long orderVolume = data.order.order_volume;
    ((VectorSP)(buffer[colNum++]))->appendLong(&orderVolume, 1);
    int side = data.order.side;
    ((VectorSP)(buffer[colNum++]))->appendInt(&side, 1);
    int orderType = data.order.order_type;
    ((VectorSP)(buffer[colNum++]))->appendInt(&orderType, 1);
    string mdStreamId = data.order.md_stream_id;
    ((VectorSP)(buffer[colNum++]))->appendString(&mdStreamId, 1);
    long long origOrderNo = data.order.orig_order_no;
    ((VectorSP)(buffer[colNum++]))->appendLong(&origOrderNo, 1);
    long long bizIndex = data.order.biz_index;
    ((VectorSP)(buffer[colNum++]))->appendLong(&bizIndex, 1);
    int varietyCategory = data.order.variety_category;
    ((VectorSP)(buffer[colNum++]))->appendInt(&varietyCategory, 1);
#ifdef AMD_TRADE_ORDER_VOLUME
    long long tradedOrderVolume = data.order.traded_order_volume;
    ((VectorSP)(buffer[colNum++]))->appendLong(&tradedOrderVolume, 1);
# else
    long long tradedOrderVolume = LONG_LONG_MIN;
    ((VectorSP)(buffer[colNum++]))->appendLong(&tradedOrderVolume, 1);
# endif
}

void bondOrderReader_4_0_1(vector<ConstantSP> &buffer, timeMDBondTickOrder &data, bool securityCodeToInt) {
    int colNum = 0;
    int marketType = data.bondOrder.market_type;
    ((VectorSP)(buffer[colNum++]))->appendInt(&marketType, 1);
    string securityCode(data.bondOrder.security_code);
    if(securityCodeToInt) {
        int num = std::atoi(securityCode.c_str());
        ((VectorSP)(buffer[colNum++]))->appendInt(&num, 1);
    } else {
        ((VectorSP)(buffer[colNum++]))->appendString(&securityCode, 1);
    }
    int channelNo = data.bondOrder.channel_no;
    ((VectorSP)(buffer[colNum++]))->appendInt(&channelNo, 1);
    long long applSeqNum = data.bondOrder.appl_seq_num;
    ((VectorSP)(buffer[colNum++]))->appendLong(&applSeqNum, 1);
    long long orderTime = convertTime(data.bondOrder.order_time);
    ((VectorSP)(buffer[colNum++]))->appendLong(&orderTime, 1);
    long long orderPrice = data.bondOrder.order_price;
    ((VectorSP)(buffer[colNum++]))->appendLong(&orderPrice, 1);
    long long orderVolume = data.bondOrder.order_volume;
    ((VectorSP)(buffer[colNum++]))->appendLong(&orderVolume, 1);
    int side = data.bondOrder.side;
    ((VectorSP)(buffer[colNum++]))->appendInt(&side, 1);
    int orderType = data.bondOrder.order_type;
    ((VectorSP)(buffer[colNum++]))->appendInt(&orderType, 1);
    string mdStreamId = data.bondOrder.md_stream_id;
    ((VectorSP)(buffer[colNum++]))->appendString(&mdStreamId, 1);
    string productStatus = data.bondOrder.product_status;
    ((VectorSP)(buffer[colNum++]))->appendString(&productStatus, 1);
    long long origOrderNo = data.bondOrder.orig_order_no;
    ((VectorSP)(buffer[colNum++]))->appendLong(&origOrderNo, 1);
    int varietyCategory = data.bondOrder.variety_category;
    ((VectorSP)(buffer[colNum++]))->appendInt(&varietyCategory, 1);
}

void bondSnapshotReader_4_0_1(vector<ConstantSP> &buffer, timeMDBondSnapshot &data, bool securityCodeToInt) {
    int colNum = 0;
    int marketType          = data.bondSnapshot.market_type;
    ((Vector*)buffer[colNum++].get())->appendInt(&marketType, 1);
    string securityCode     = data.bondSnapshot.security_code;
    if (securityCodeToInt) {
        int num = std::atoi(securityCode.c_str());
        ((VectorSP)(buffer[colNum++]))->appendInt(&num, 1);
    } else {
        ((VectorSP)(buffer[colNum++]))->appendString(&securityCode, 1);
    }
    long long origTime      = convertTime(data.bondSnapshot.orig_time);
    ((Vector*)buffer[colNum++].get())->appendLong(&origTime, 1);
    string tradingPhaseCode = data.bondSnapshot.trading_phase_code;
    ((Vector*)buffer[colNum++].get())->appendString(&tradingPhaseCode, 1);
    long long preClosePrice = data.bondSnapshot.pre_close_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&preClosePrice, 1);
    long long openPrice     = data.bondSnapshot.open_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&openPrice, 1);
    long long highPrice     = data.bondSnapshot.high_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&highPrice, 1);
    long long lowPrice      = data.bondSnapshot.low_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&lowPrice, 1);
    long long lastPrice     = data.bondSnapshot.last_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&lastPrice, 1);
    long long closePrice    = data.bondSnapshot.close_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&closePrice, 1);

    for(int i = 0; i < 10; ++i) {
        long long bidPrice = data.bondSnapshot.bid_price[i];
        ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice, 1);
    }
    for(int i = 0; i < 10; ++i) {
        long long bidVolume = data.bondSnapshot.bid_volume[i];
        ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume, 1);
    }
    for(int i = 0; i < 10; ++i) {
        long long offerPrice = data.bondSnapshot.offer_price[i];
        ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice, 1);
    }
    for(int i = 0; i < 10; ++i) {
        long long offerVolume  = data.bondSnapshot.offer_volume[i];
        ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume, 1);
    }

    long long numTrades             = data.bondSnapshot.num_trades;
    ((Vector*)buffer[colNum++].get())->appendLong(&numTrades, 1);
    long long totalVolumeTrade      = data.bondSnapshot.total_volume_trade;
    ((Vector*)buffer[colNum++].get())->appendLong(&totalVolumeTrade, 1);
    long long totalValueTrade       = data.bondSnapshot.total_value_trade;
    ((Vector*)buffer[colNum++].get())->appendLong(&totalValueTrade, 1);
    long long totalBidVolume        = data.bondSnapshot.total_bid_volume;
    ((Vector*)buffer[colNum++].get())->appendLong(&totalBidVolume, 1);
    long long totalOfferVolume      = data.bondSnapshot.total_offer_volume;
    ((Vector*)buffer[colNum++].get())->appendLong(&totalOfferVolume, 1);
    long long weightedAvgBidPrice   = data.bondSnapshot.weighted_avg_bid_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&weightedAvgBidPrice, 1);
    long long weightedAvgOfferPrice = data.bondSnapshot.weighted_avg_offer_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&weightedAvgOfferPrice, 1);

    long long highLimited           = data.bondSnapshot.high_limited;
    ((Vector*)buffer[colNum++].get())->appendLong(&highLimited, 1);
    long long lowLimited             = data.bondSnapshot.low_limited;
    ((Vector*)buffer[colNum++].get())->appendLong(&lowLimited, 1);
    long long change1                = data.bondSnapshot.change1;
    ((Vector*)buffer[colNum++].get())->appendLong(&change1, 1);
    long long change2                = data.bondSnapshot.change2;
    ((Vector*)buffer[colNum++].get())->appendLong(&change2, 1);

    long long weighted_avg_bp = data.bondSnapshot.weighted_avg_bp;
    ((Vector*)buffer[colNum++].get())->appendLong(&weighted_avg_bp, 1);
    long long pre_close_weighted_avg_price = data.bondSnapshot.pre_close_weighted_avg_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&pre_close_weighted_avg_price, 1);
    long long auct_last_price = data.bondSnapshot.auct_last_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&auct_last_price, 1);
    long long last_price_trading_type = data.bondSnapshot.last_price_trading_type;
    ((Vector*)buffer[colNum++].get())->appendLong(&last_price_trading_type, 1);

    int channelNo                    = data.bondSnapshot.channel_no;
    ((Vector*)buffer[colNum++].get())->appendInt(&channelNo, 1);
    string mdStreamID                = data.bondSnapshot.md_stream_id;
    ((Vector*)buffer[colNum++].get())->appendString(&mdStreamID, 1);
    string instrumentStatus          = data.bondSnapshot.instrument_status;
    ((Vector*)buffer[colNum++].get())->appendString(&instrumentStatus, 1);
    long long withdrawBuyNumber   = data.bondSnapshot.withdraw_buy_number;
    ((Vector*)buffer[colNum++].get())->appendLong(&withdrawBuyNumber, 1);
    long long withdrawBuyAmount   = data.bondSnapshot.withdraw_buy_amount;
    ((Vector*)buffer[colNum++].get())->appendLong(&withdrawBuyAmount, 1);
    long long withdrawBuyMoney    = data.bondSnapshot.withdraw_buy_money;
    ((Vector*)buffer[colNum++].get())->appendLong(&withdrawBuyMoney, 1);
    long long withdrawSellNumber  = data.bondSnapshot.withdraw_sell_number;
    ((Vector*)buffer[colNum++].get())->appendLong(&withdrawSellNumber, 1);
    long long withdrawSellAmount  = data.bondSnapshot.withdraw_sell_amount;
    ((Vector*)buffer[colNum++].get())->appendLong(&withdrawSellAmount, 1);
    long long withdrawSellMoney   = data.bondSnapshot.withdraw_sell_money;
    ((Vector*)buffer[colNum++].get())->appendLong(&withdrawSellMoney, 1);
    long long totalBidNumber      = data.bondSnapshot.total_bid_number;
    ((Vector*)buffer[colNum++].get())->appendLong(&totalBidNumber, 1);
    long long totalOfferNumber    = data.bondSnapshot.total_offer_number;
    ((Vector*)buffer[colNum++].get())->appendLong(&totalOfferNumber, 1);
    int bidTradeMaxDuration       = data.bondSnapshot.bid_trade_max_duration;
    ((Vector*)buffer[colNum++].get())->appendInt(&bidTradeMaxDuration, 1);
    int offerTradeMaxDuration       = data.bondSnapshot.offer_trade_max_duration;
    ((Vector*)buffer[colNum++].get())->appendInt(&offerTradeMaxDuration, 1);
    int numBidOrders                = data.bondSnapshot.num_bid_orders;
    ((Vector*)buffer[colNum++].get())->appendInt(&numBidOrders, 1);
    long long numOfferOrders        = data.bondSnapshot.num_offer_orders;
    ((Vector*)buffer[colNum++].get())->appendLong(&numOfferOrders, 1);
    long long lastTradeTime         = data.bondSnapshot.last_trade_time;
    ((Vector*)buffer[colNum++].get())->appendLong(&lastTradeTime, 1);

    long long weighted_avg_price = data.bondSnapshot.weighted_avg_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&weighted_avg_price, 1);
    long long no_sub_trading_phase_code = data.bondSnapshot.no_sub_trading_phase_code;
    ((Vector*)buffer[colNum++].get())->appendLong(&no_sub_trading_phase_code, 1);

    for(int i = 0; i < 8; ++i) {
        string sub_trading_phase_code = data.bondSnapshot.sub_trading_phase[i].sub_trading_phase_code;
        ((Vector*)buffer[colNum++].get())->appendString(&sub_trading_phase_code, 1);
        char trading_type = data.bondSnapshot.sub_trading_phase[i].trading_type;
        ((Vector*)buffer[colNum++].get())->appendChar(&trading_type, 1);
    }

    long long auct_volume_trade = data.bondSnapshot.auct_volume_trade;
    ((Vector*)buffer[colNum++].get())->appendLong(&auct_volume_trade, 1);
    long long auct_value_trade = data.bondSnapshot.auct_value_trade;
    ((Vector*)buffer[colNum++].get())->appendLong(&auct_value_trade, 1);

    char varietyCategory            = data.bondSnapshot.variety_category;
    ((Vector*)buffer[colNum++].get())->appendChar(&varietyCategory, 1);
}


long long countTemporalUnit(int days, long long multiplier, long long remainder) {
    return days == INT_MIN ? LLONG_MIN : days * multiplier + remainder;
}

int countDays(int amdDays) { return Util::countDays(amdDays / 10000, (amdDays / 100) % 100, amdDays % 100); }

// to cooperate with insight based orderbookSnapshot.
// use convertBSFlag() & convertType() to change amd flag & type to insight flag & type.
// if a flag & type not exist in insight, return origin value.
// details see https://dolphindb1.atlassian.net/browse/DPLG-837
int convertBSFlag(int flag) {
    switch (flag) {
        case '1':
        case 'B':
            return OrderBookBSFlag::BUY;
        case '2':
        case 'S':
            return OrderBookBSFlag::SELL;
        case 'N':
            return OrderBookBSFlag::UNKNOWN;
        default:
            return flag;
    }
}

int convertBSFlag(int flag, int64_t buyNo, int64_t sellNo) {
    switch (flag) {
        case '1':
        case 'B':
            return OrderBookBSFlag::BUY;
        case '2':
        case 'S':
            return OrderBookBSFlag::SELL;
        case 'N':
            if (buyNo > sellNo) {
                return OrderBookBSFlag::BUY;
            } else {
                return OrderBookBSFlag::SELL;
            }
        default:
            return flag;
    }
}

int convertType(int type) {
    switch (type) {
        case '2':
        case 'A':
            return OrderBookOrderType::LIMIT_ORDER;
        case 'D':
            return OrderBookOrderType::CANCEL;
        case 'S':
            return OrderBookOrderType::STATUS;
        case 'U':
            return OrderBookOrderType::BEST;
        case 'F':
            return OrderBookTradeType::DEAL;
        case 4:
        case '1':
            return OrderBookOrderType::MARKET_ORDER;
        case '4':
            return OrderBookTradeType::CANCEL;
        default:
            return type;
    }
}

int convertToDate(long long time) {
    long long year, month, day;
    day = time / 1000 / 100 / 100 / 100 % 100;
    month = time / 1000 / 100 / 100 / 100 / 100 % 100;
    year = time / 1000 / 100 / 100 / 100 / 100 / 100;
    return Util::countDays(year, month, day);
}

int convertToTime(long long time) {
    long long hour, minute, second, milliSecond;
    milliSecond = time % 1000;
    second = time / 1000 % 100;
    minute = time / 1000 / 100 % 100;
    hour = time / 1000 / 100 / 100 % 100;
    return ((hour * 60 + minute) * 60 + second) * 1000ll + milliSecond;
}

long long convertTime(long long time) {
    long long year, month, day, hour, minute, second, milliSecond;
    milliSecond = time % 1000;
    second = time / 1000 % 100;
    minute = time / 1000 / 100 % 100;
    hour = time / 1000 / 100 / 100 % 100;
    day = time / 1000 / 100 / 100 / 100 % 100;
    month = time / 1000 / 100 / 100 / 100 / 100 % 100;
    year = time / 1000 / 100 / 100 / 100 / 100 / 100;
    return countTemporalUnit(Util::countDays(year, month, day), 86400000ll,
                             ((hour * 60 + minute) * 60 + second) * 1000ll + milliSecond);
}

string transMarket(int type) {
    if (type == amd::ama::MarketType::kSSE) {
        return "XSHG";
    } else if (type == amd::ama::MarketType::kSZSE) {
        return "XSHE";
    }
    return "";
}

AMDDataType getAmdDataType(const string &typeStr) {
    if (typeStr == "snapshot") {
        return AMD_SNAPSHOT;
    } else if (typeStr == "execution") {
        return AMD_EXECUTION;
    } else if (typeStr == "order") {
        return AMD_ORDER;
    } else if (typeStr == "fundSnapshot") {
        return AMD_FUND_SNAPSHOT;
    } else if (typeStr == "fundExecution") {
        return AMD_FUND_EXECUTION;
    } else if (typeStr == "fundOrder") {
        return AMD_FUND_ORDER;
    } else if (typeStr == "bondSnapshot") {
        return AMD_BOND_SNAPSHOT;
    } else if (typeStr == "bondExecution") {
        return AMD_BOND_EXECUTION;
    } else if (typeStr == "bondOrder") {
        return AMD_BOND_ORDER;
    } else if (typeStr == "index") {
        return AMD_INDEX;
    } else if (typeStr == "orderQueue") {
        return AMD_ORDER_QUEUE;
    } else if (typeStr == "option") {
        return AMD_OPTION_SNAPSHOT;
    } else if (typeStr == "future") {
        return AMD_FUTURE_SNAPSHOT;
#ifndef AMD_3_9_6
    } else if (typeStr == "IOPV") {
        return AMD_IOPV_SNAPSHOT;
#endif
    } else if (typeStr == "orderExecution") {
        return AMD_ORDER_EXECUTION;
    } else if (typeStr == "bondOrderExecution") {
        return AMD_BOND_ORDER_EXECUTION;
    } else {
        throw RuntimeException(
            "type should be `snapshot, `execution, `order, `index, `future, `option, `IOPV(Except for amd version 3.9.6), `orderQueue, `fundSnapshot, "
            "`fundExecution`, `fundOrder, 'orderExecution' or 'bondOrderExecution'");
    }
}

template <>
int getDailyIndex(DailyIndex &index,timeMDOption& data, long long timestamp) {
    return INT_MIN;
}

template <>
int getDailyIndex(DailyIndex &index,timeMDFuture& data, long long timestamp) {
    return INT_MIN;
}

#ifndef AMD_3_9_6
template <>
int getDailyIndex(DailyIndex &index,timeMDIOPV& data, long long timestamp) {
    return INT_MIN;
}
#endif

template <>
int getDailyIndex(DailyIndex &index,timeMDIndexSnapshot& data, long long timestamp) {
    return INT_MIN;
}
template <>
int getDailyIndex(DailyIndex &index,timeMDOrderQueue& data, long long timestamp) {
    return INT_MIN;
}
template <>
int getDailyIndex(DailyIndex &index,timeMDSnapshot& data, long long timestamp) {
    return INT_MIN;
}
template <>
int getDailyIndex(DailyIndex &index,timeMDBondSnapshot& data, long long timestamp) {
    return INT_MIN;
}

template <>
int getDailyIndex(DailyIndex &index,timeMDTickOrder& data, long long timestamp) {
    int channelNo = data.order.channel_no;
    return index.getIndex(channelNo, timestamp);
}

template <>
int getDailyIndex(DailyIndex &index, timeMDBondTickOrder& data, long long timestamp) {
    int channelNo = data.bondOrder.channel_no;
    return index.getIndex(channelNo, timestamp);
}

template <>
int getDailyIndex(DailyIndex &index, timeMDTickExecution& data, long long timestamp) {
    int channelNo = data.execution.channel_no;
    return index.getIndex(channelNo, timestamp);
}

template <>
int getDailyIndex(DailyIndex &index, timeMDBondTickExecution& data, long long timestamp) {
    int channelNo = data.bondExecution.channel_no;
    return index.getIndex(channelNo, timestamp);
}

template <>
int getDailyIndex(DailyIndex &index, MDOrderExecution& data, long long timestamp) {
    // int channelNo = 0;
    // if(data.orderOrExecution) {
    //     channelNo = data.uni.tickOrder.channel_no;
    // } else {
    //     channelNo = data.uni.tickExecution.channel_no;
    // }
    // return index.getIndex(channelNo, timestamp);
    return INT_MIN;
}

template <>
int getDailyIndex(DailyIndex &index, MDBondOrderExecution& data, long long timestamp) {
    // int channelNo = 0;
    // if(data.orderOrExecution) {
    //     channelNo = data.uni.tickOrder.channel_no;
    // } else {
    //     channelNo = data.uni.tickExecution.channel_no;
    // }
    // return index.getIndex(channelNo, timestamp);
    return INT_MIN;
}

// TODO(ruibinhuang@dolphindb.com): check the real attributes of the table
bool checkSchema(const string& type, TableSP table) {
    // INDEX tableColumns = table->columns();
    // if(RECEIVED_TIME_FLAG)
    //     tableColumns--;
    // if(DAILY_INDEX_FLAG)
    //     tableColumns--;
    // if(OUTPUT_ELAPSED_FLAG)
    //     tableColumns--;
    // if (type == "snapshot") {
    //     if (tableColumns!= 94) {
    //         return false;
    //     }
    // } else if (type == "execution") {
    //     if (tableColumns != 15) {
    //         return false;
    //     }
    // } else if (type == "index") {
    //     if (tableColumns != 15) {
    //         return false;
    //     }
    // } else if (type == "orderQueue") {
    //     if (tableColumns != 61) {
    //         return false;
    //     }
    // } else if (type == "fundSnapshot") {
    //     if (tableColumns != 94) {
    //         return false;
    //     }
    // } else if (type == "fundExecution") {
    //     if (tableColumns != 15) {
    //         return false;
    //     }
    // } else if (type == "fundOrder") {
    //     if (tableColumns != 13) {
    //         return false;
    //     }
    // } else if(type == "order") {
    //     if (tableColumns != 13) {
    //         return false;
    //     }
    // }else if (type == "bondSnapshot") {
    //     if (tableColumns != 94) {
    //         return false;
    //     }
    // } else if (type == "bondExecution") {
    //     if (tableColumns != 15) {
    //         return false;
    //     }
    // } else if (type == "bondOrder") {
    //     if (tableColumns != 13) {
    //         return false;
    //     }
    // } else if(type == "orderExecution" || type == "fundOrderExecution" || type == "bondOrderExecution") {
    //     tableColumns += 2;              //because two flags always on, so plus 2 to pass check
    //     if (tableColumns != 16) {       //FIXME columns number not depends on param
    //         return false;
    //     }
    // } else{
    //     throw IllegalArgumentException(__FUNCTION__, "error type " + type);
    // }

    return true;
}
