#include "amdQuoteType.h"
#include "Types.h"
#ifndef AMD_USE_THREADED_QUEUE
#define AMD_USE_THREADED_QUEUE
#include "ddbplugin/ThreadedQueue.h"
#endif

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
//   真实价格*10000   股数   买卖方向   冗余列    冗余列    逐笔数据序号   原始频道代码   数据接收时间戳
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

    long long bid_iopv1 = data.IOPV.bid_iopv[0];
    ((VectorSP)(buffer[colNum++]))->appendLong(&bid_iopv1 , 1);
    long long bid_iopv2 = data.IOPV.bid_iopv[1];
    ((VectorSP)(buffer[colNum++]))->appendLong(&bid_iopv2 , 1);
    long long bid_iopv3 = data.IOPV.bid_iopv[2];
    ((VectorSP)(buffer[colNum++]))->appendLong(&bid_iopv3 , 1);
    long long bid_iopv4 = data.IOPV.bid_iopv[3];
    ((VectorSP)(buffer[colNum++]))->appendLong(&bid_iopv4 , 1);
    long long bid_iopv5 = data.IOPV.bid_iopv[4];
    ((VectorSP)(buffer[colNum++]))->appendLong(&bid_iopv5 , 1);
    long long bid_iopv6 = data.IOPV.bid_iopv[5];
    ((VectorSP)(buffer[colNum++]))->appendLong(&bid_iopv6 , 1);
    long long bid_iopv7 = data.IOPV.bid_iopv[6];
    ((VectorSP)(buffer[colNum++]))->appendLong(&bid_iopv7 , 1);
    long long bid_iopv8 = data.IOPV.bid_iopv[7];
    ((VectorSP)(buffer[colNum++]))->appendLong(&bid_iopv8 , 1);
    long long bid_iopv9 = data.IOPV.bid_iopv[8];
    ((VectorSP)(buffer[colNum++]))->appendLong(&bid_iopv9 , 1);
    long long bid_iopv10 = data.IOPV.bid_iopv[9];
    ((VectorSP)(buffer[colNum++]))->appendLong(&bid_iopv10 , 1);
    long long offer_iopv1 = data.IOPV.offer_iopv[0];
    ((VectorSP)(buffer[colNum++]))->appendLong(&offer_iopv1, 1);
    long long offer_iopv2 = data.IOPV.offer_iopv[1];
    ((VectorSP)(buffer[colNum++]))->appendLong(&offer_iopv2, 1);
    long long offer_iopv3 = data.IOPV.offer_iopv[2];
    ((VectorSP)(buffer[colNum++]))->appendLong(&offer_iopv3, 1);
    long long offer_iopv4 = data.IOPV.offer_iopv[3];
    ((VectorSP)(buffer[colNum++]))->appendLong(&offer_iopv4, 1);
    long long offer_iopv5 = data.IOPV.offer_iopv[4];
    ((VectorSP)(buffer[colNum++]))->appendLong(&offer_iopv5, 1);
    long long offer_iopv6 = data.IOPV.offer_iopv[5];
    ((VectorSP)(buffer[colNum++]))->appendLong(&offer_iopv6, 1);
    long long offer_iopv7 = data.IOPV.offer_iopv[6];
    ((VectorSP)(buffer[colNum++]))->appendLong(&offer_iopv7, 1);
    long long offer_iopv8 = data.IOPV.offer_iopv[7];
    ((VectorSP)(buffer[colNum++]))->appendLong(&offer_iopv8, 1);
    long long offer_iopv9 = data.IOPV.offer_iopv[8];
    ((VectorSP)(buffer[colNum++]))->appendLong(&offer_iopv9, 1);
    long long offer_iopv10 = data.IOPV.offer_iopv[9];
    ((VectorSP)(buffer[colNum++]))->appendLong(&offer_iopv10, 1);
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

    long long volume0 = data.orderQueue.volume[0];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume0, 1);
    long long volume1 = data.orderQueue.volume[1];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume1, 1);
    long long volume2 = data.orderQueue.volume[2];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume2, 1);
    long long volume3 = data.orderQueue.volume[3];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume3, 1);
    long long volume4 = data.orderQueue.volume[4];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume4, 1);
    long long volume5 = data.orderQueue.volume[5];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume5, 1);
    long long volume6 = data.orderQueue.volume[6];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume6, 1);
    long long volume7 = data.orderQueue.volume[7];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume7, 1);
    long long volume8 = data.orderQueue.volume[8];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume8, 1);
    long long volume9 = data.orderQueue.volume[9];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume9, 1);
    long long volume10 = data.orderQueue.volume[10];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume10, 1);

    long long volume11 = data.orderQueue.volume[11];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume11, 1);
    long long volume12 = data.orderQueue.volume[12];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume12, 1);
    long long volume13 = data.orderQueue.volume[13];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume13, 1);
    long long volume14 = data.orderQueue.volume[14];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume14, 1);
    long long volume15 = data.orderQueue.volume[15];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume15, 1);
    long long volume16 = data.orderQueue.volume[16];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume16, 1);
    long long volume17 = data.orderQueue.volume[17];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume17, 1);
    long long volume18 = data.orderQueue.volume[18];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume18, 1);
    long long volume19 = data.orderQueue.volume[19];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume19, 1);
    long long volume20 = data.orderQueue.volume[20];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume20, 1);

    long long volume21 = data.orderQueue.volume[21];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume21, 1);
    long long volume22 = data.orderQueue.volume[22];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume22, 1);
    long long volume23 = data.orderQueue.volume[23];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume23, 1);
    long long volume24 = data.orderQueue.volume[24];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume24, 1);
    long long volume25 = data.orderQueue.volume[25];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume25, 1);
    long long volume26 = data.orderQueue.volume[26];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume26, 1);
    long long volume27 = data.orderQueue.volume[27];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume27, 1);
    long long volume28 = data.orderQueue.volume[28];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume28, 1);
    long long volume29 = data.orderQueue.volume[29];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume29, 1);
    long long volume30 = data.orderQueue.volume[30];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume30, 1);

    long long volume31 = data.orderQueue.volume[31];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume31, 1);
    long long volume32 = data.orderQueue.volume[32];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume32, 1);
    long long volume33 = data.orderQueue.volume[33];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume33, 1);
    long long volume34 = data.orderQueue.volume[34];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume34, 1);
    long long volume35 = data.orderQueue.volume[35];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume35, 1);
    long long volume36 = data.orderQueue.volume[36];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume36, 1);
    long long volume37 = data.orderQueue.volume[37];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume37, 1);
    long long volume38 = data.orderQueue.volume[38];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume38, 1);
    long long volume39 = data.orderQueue.volume[39];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume39, 1);
    long long volume40 = data.orderQueue.volume[40];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume40, 1);

    long long volume41 = data.orderQueue.volume[41];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume41, 1);
    long long volume42 = data.orderQueue.volume[42];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume42, 1);
    long long volume43 = data.orderQueue.volume[43];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume43, 1);
    long long volume44 = data.orderQueue.volume[44];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume44, 1);
    long long volume45 = data.orderQueue.volume[45];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume45, 1);
    long long volume46 = data.orderQueue.volume[46];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume46, 1);
    long long volume47 = data.orderQueue.volume[47];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume47, 1);
    long long volume48 = data.orderQueue.volume[48];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume48, 1);
    long long volume49 = data.orderQueue.volume[49];
    ((VectorSP)(buffer[colNum++]))->appendLong(&volume49, 1);

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

    int bidPriceIndex = 0;
    long long bidPrice1 = data.snapshot.bid_price[bidPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice1, 1);
    long long bidPrice2 = data.snapshot.bid_price[bidPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice2, 1);
    long long bidPrice3 = data.snapshot.bid_price[bidPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice3, 1);
    long long bidPrice4 = data.snapshot.bid_price[bidPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice4, 1);
    long long bidPrice5 = data.snapshot.bid_price[bidPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice5, 1);
    long long bidPrice6 = data.snapshot.bid_price[bidPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice6, 1);
    long long bidPrice7 = data.snapshot.bid_price[bidPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice7, 1);
    long long bidPrice8 = data.snapshot.bid_price[bidPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice8, 1);
    long long bidPrice9 = data.snapshot.bid_price[bidPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice9, 1);
    long long bidPrice10= data.snapshot.bid_price[bidPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice10, 1);

    int bidVolumeIndex = 0;
    long long bidVolume1 = data.snapshot.bid_volume[bidVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume1, 1);
    long long bidVolume2 = data.snapshot.bid_volume[bidVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume2, 1);
    long long bidVolume3 = data.snapshot.bid_volume[bidVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume3, 1);
    long long bidVolume4 = data.snapshot.bid_volume[bidVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume4, 1);
    long long bidVolume5 = data.snapshot.bid_volume[bidVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume5, 1);
    long long bidVolume6 = data.snapshot.bid_volume[bidVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume6, 1);
    long long bidVolume7 = data.snapshot.bid_volume[bidVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume7, 1);
    long long bidVolume8 = data.snapshot.bid_volume[bidVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume8, 1);
    long long bidVolume9 = data.snapshot.bid_volume[bidVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume9, 1);
    long long bidVolume10= data.snapshot.bid_volume[bidVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume10, 1);

    int offerPriceIndex = 0;
    long long offerPrice1 = data.snapshot.offer_price[offerPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice1, 1);
    long long offerPrice2 = data.snapshot.offer_price[offerPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice2, 1);
    long long offerPrice3 = data.snapshot.offer_price[offerPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice3, 1);
    long long offerPrice4 = data.snapshot.offer_price[offerPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice4, 1);
    long long offerPrice5 = data.snapshot.offer_price[offerPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice5, 1);
    long long offerPrice6 = data.snapshot.offer_price[offerPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice6, 1);
    long long offerPrice7 = data.snapshot.offer_price[offerPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice7, 1);
    long long offerPrice8 = data.snapshot.offer_price[offerPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice8, 1);
    long long offerPrice9 = data.snapshot.offer_price[offerPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice9, 1);
    long long offerPrice10= data.snapshot.offer_price[offerPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice10, 1);

    int offerVolumeIndex = 0;
    long long offerVolume1  = data.snapshot.offer_volume[offerVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume1, 1);
    long long offerVolume2  = data.snapshot.offer_volume[offerVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume2, 1);
    long long offerVolume3  = data.snapshot.offer_volume[offerVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume3, 1);
    long long offerVolume4  = data.snapshot.offer_volume[offerVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume4, 1);
    long long offerVolume5  = data.snapshot.offer_volume[offerVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume5, 1);
    long long offerVolume6  = data.snapshot.offer_volume[offerVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume6, 1);
    long long offerVolume7  = data.snapshot.offer_volume[offerVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume7, 1);
    long long offerVolume8  = data.snapshot.offer_volume[offerVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume8, 1);
    long long offerVolume9  = data.snapshot.offer_volume[offerVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume9, 1);
    long long offerVolume10 = data.snapshot.offer_volume[offerVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume10, 1);

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
void orderExecutionReader(vector<ConstantSP> &buffer, MDOrderExecution &data) {
    int marketType;
    uint8_t varietyCategory;
    if(data.orderOrExecution) {
        marketType = data.uni.tickOrder.market_type;
    } else {
        marketType = data.uni.tickExecution.market_type;
    }
    if (data.orderOrExecution) {
        varietyCategory = data.uni.tickOrder.variety_category;
    } else {
        varietyCategory = data.uni.tickExecution.variety_category;
    }

    if(varietyCategory != 1 && varietyCategory != 2) {
        return;
    }
    int colNum = 0;
    if(data.orderOrExecution) {
        if(marketType == 101) {
            string securityCode = data.uni.tickOrder.security_code + string(".SH");
            ((VectorSP)(buffer[colNum++]))->appendString(&securityCode, 1);
        } else if (marketType == 102) {
            string securityCode = data.uni.tickOrder.security_code + string(".SZ");
            ((VectorSP)(buffer[colNum++]))->appendString(&securityCode, 1);
        }

        int orderDate = convertToDate(data.uni.tickOrder.order_time);
        ((VectorSP)(buffer[colNum++]))->appendInt(&orderDate, 1);
        int orderTime = convertToTime(data.uni.tickOrder.order_time);
        ((VectorSP)(buffer[colNum++]))->appendInt(&orderTime, 1);
        string securityIDSource = transMarket(data.uni.tickOrder.market_type);
        ((VectorSP)(buffer[colNum++]))->appendString(&securityIDSource, 1);
        string securityType("StockType");
        ((VectorSP)(buffer[colNum++]))->appendString(&securityType, 1);

        //  make sure DailyIndexFlagTotal always true
        int DailyIndex = data.uni.tickOrder.appl_seq_num;
        ((VectorSP)(buffer[colNum++]))->appendInt(&DailyIndex, 1);
        int sourceType = 0;
        ((VectorSP)(buffer[colNum++]))->appendInt(&sourceType, 1);
        int type = convertType(data.uni.tickOrder.order_type);
        ((VectorSP)(buffer[colNum++]))->appendInt(&type, 1);
        long long orderPrice = data.uni.tickOrder.order_price;
        ((VectorSP)(buffer[colNum++]))->appendLong(&orderPrice, 1);
        long long orderVolume = data.uni.tickOrder.order_volume;
        ((VectorSP)(buffer[colNum++]))->appendLong(&orderVolume, 1);
        int BSFlag = convertBSFlag(data.uni.tickOrder.side);
        ((VectorSP)(buffer[colNum++]))->appendInt(&BSFlag, 1);
        long long origOrderNo = data.uni.tickOrder.orig_order_no;
        ((VectorSP)(buffer[colNum++]))->appendLong(&origOrderNo, 1);
        // same as the previous one
        ((VectorSP)(buffer[colNum++]))->appendLong(&origOrderNo, 1);
        long long applSeqNum = data.uni.tickOrder.appl_seq_num;
        ((VectorSP)(buffer[colNum++]))->appendLong(&applSeqNum, 1);
        int channelNo = data.uni.tickOrder.channel_no;
        ((VectorSP)(buffer[colNum++]))->appendInt(&channelNo, 1);
        int timestamp = data.reachTime / 1000000;
        ((VectorSP)(buffer[colNum++]))->appendInt(&timestamp, 1);
    } else {
        if(marketType == 101) {
            string securityCode = data.uni.tickExecution.security_code + string(".SH");
            ((VectorSP)(buffer[colNum++]))->appendString(&securityCode, 1);
        } else if (marketType == 102) {
            string securityCode = data.uni.tickExecution.security_code + string(".SZ");
            ((VectorSP)(buffer[colNum++]))->appendString(&securityCode, 1);
        }
        int orderDate = convertToDate(data.uni.tickExecution.exec_time);
        ((VectorSP)(buffer[colNum++]))->appendInt(&orderDate, 1);
        int orderTime = convertToTime(data.uni.tickExecution.exec_time);
        ((VectorSP)(buffer[colNum++]))->appendInt(&orderTime, 1);
        string securityIDSource = transMarket(data.uni.tickExecution.market_type);
        ((VectorSP)(buffer[colNum++]))->appendString(&securityIDSource, 1);
        string securityType("StockType");
        ((VectorSP)(buffer[colNum++]))->appendString(&securityType, 1);
        int DailyIndex = data.uni.tickExecution.appl_seq_num;
        ((VectorSP)(buffer[colNum++]))->appendInt(&DailyIndex, 1);
        int sourceType = 1;
        ((VectorSP)(buffer[colNum++]))->appendInt(&sourceType, 1);
        int type = convertType(data.uni.tickExecution.exec_type);
        ((VectorSP)(buffer[colNum++]))->appendInt(&type, 1);
        long long orderPrice = data.uni.tickExecution.exec_price;
        ((VectorSP)(buffer[colNum++]))->appendLong(&orderPrice, 1);
        long long orderVolume = data.uni.tickExecution.exec_volume;
        ((VectorSP)(buffer[colNum++]))->appendLong(&orderVolume, 1);
        int BSFlag = convertBSFlag(data.uni.tickExecution.side);
        ((VectorSP)(buffer[colNum++]))->appendInt(&BSFlag, 1);
        long long origOrderNo = data.uni.tickExecution.bid_appl_seq_num;
        ((VectorSP)(buffer[colNum++]))->appendLong(&origOrderNo, 1);
        long long offerApplSeqNum = data.uni.tickExecution.offer_appl_seq_num;
        ((VectorSP)(buffer[colNum++]))->appendLong(&offerApplSeqNum, 1);
        long long applSeqNum = data.uni.tickExecution.appl_seq_num;
        ((VectorSP)(buffer[colNum++]))->appendLong(&applSeqNum, 1);
        int channelNo = data.uni.tickExecution.channel_no;
        ((VectorSP)(buffer[colNum++]))->appendInt(&channelNo, 1);
        int timestamp = data.reachTime / 1000000;
        ((VectorSP)(buffer[colNum++]))->appendInt(&timestamp, 1);
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

    int bidPriceIndex = 0;
    long long bidPrice1 = data.bondSnapshot.bid_price[bidPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice1, 1);
    long long bidPrice2 = data.bondSnapshot.bid_price[bidPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice2, 1);
    long long bidPrice3 = data.bondSnapshot.bid_price[bidPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice3, 1);
    long long bidPrice4 = data.bondSnapshot.bid_price[bidPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice4, 1);
    long long bidPrice5 = data.bondSnapshot.bid_price[bidPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice5, 1);
    long long bidPrice6 = data.bondSnapshot.bid_price[bidPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice6, 1);
    long long bidPrice7 = data.bondSnapshot.bid_price[bidPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice7, 1);
    long long bidPrice8 = data.bondSnapshot.bid_price[bidPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice8, 1);
    long long bidPrice9 = data.bondSnapshot.bid_price[bidPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice9, 1);
    long long bidPrice10= data.bondSnapshot.bid_price[bidPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice10, 1);

    int bidVolumeIndex = 0;
    long long bidVolume1 = data.bondSnapshot.bid_volume[bidVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume1, 1);
    long long bidVolume2 = data.bondSnapshot.bid_volume[bidVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume2, 1);
    long long bidVolume3 = data.bondSnapshot.bid_volume[bidVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume3, 1);
    long long bidVolume4 = data.bondSnapshot.bid_volume[bidVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume4, 1);
    long long bidVolume5 = data.bondSnapshot.bid_volume[bidVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume5, 1);
    long long bidVolume6 = data.bondSnapshot.bid_volume[bidVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume6, 1);
    long long bidVolume7 = data.bondSnapshot.bid_volume[bidVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume7, 1);
    long long bidVolume8 = data.bondSnapshot.bid_volume[bidVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume8, 1);
    long long bidVolume9 = data.bondSnapshot.bid_volume[bidVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume9, 1);
    long long bidVolume10= data.bondSnapshot.bid_volume[bidVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume10, 1);

    int offerPriceIndex = 0;
    long long offerPrice1 = data.bondSnapshot.offer_price[offerPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice1, 1);
    long long offerPrice2 = data.bondSnapshot.offer_price[offerPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice2, 1);
    long long offerPrice3 = data.bondSnapshot.offer_price[offerPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice3, 1);
    long long offerPrice4 = data.bondSnapshot.offer_price[offerPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice4, 1);
    long long offerPrice5 = data.bondSnapshot.offer_price[offerPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice5, 1);
    long long offerPrice6 = data.bondSnapshot.offer_price[offerPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice6, 1);
    long long offerPrice7 = data.bondSnapshot.offer_price[offerPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice7, 1);
    long long offerPrice8 = data.bondSnapshot.offer_price[offerPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice8, 1);
    long long offerPrice9 = data.bondSnapshot.offer_price[offerPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice9, 1);
    long long offerPrice10= data.bondSnapshot.offer_price[offerPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice10, 1);

    int offerVolumeIndex = 0;
    long long offerVolume1  = data.bondSnapshot.offer_volume[offerVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume1, 1);
    long long offerVolume2  = data.bondSnapshot.offer_volume[offerVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume2, 1);
    long long offerVolume3  = data.bondSnapshot.offer_volume[offerVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume3, 1);
    long long offerVolume4  = data.bondSnapshot.offer_volume[offerVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume4, 1);
    long long offerVolume5  = data.bondSnapshot.offer_volume[offerVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume5, 1);
    long long offerVolume6  = data.bondSnapshot.offer_volume[offerVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume6, 1);
    long long offerVolume7  = data.bondSnapshot.offer_volume[offerVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume7, 1);
    long long offerVolume8  = data.bondSnapshot.offer_volume[offerVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume8, 1);
    long long offerVolume9  = data.bondSnapshot.offer_volume[offerVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume9, 1);
    long long offerVolume10 = data.bondSnapshot.offer_volume[offerVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume10, 1);

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
void bondOrderExecutionReader(vector<ConstantSP> &buffer, MDBondOrderExecution &data) {
    TableSP insertedTable;
    FunctionDefSP transform;
    uint8_t varietyCategory;

    int marketType;
    if(data.orderOrExecution) {
        marketType = data.uni.tickOrder.market_type;
        varietyCategory = data.uni.tickOrder.variety_category;
    } else {
        marketType = data.uni.tickExecution.market_type;
        varietyCategory = data.uni.tickExecution.variety_category;
    }

    if (varietyCategory != 3) { // 债券
        return;
    }
    int colNum = 0;
    if(data.orderOrExecution) {
        if(marketType == 101) {
            string securityCode = data.uni.tickOrder.security_code + string(".SH");
            ((VectorSP)(buffer[colNum++]))->appendString(&securityCode, 1);
        } else if (marketType == 102) {
            string securityCode = data.uni.tickOrder.security_code + string(".SZ");
            ((VectorSP)(buffer[colNum++]))->appendString(&securityCode, 1);
        }
        int orderDate = convertToDate(data.uni.tickOrder.order_time);
        ((VectorSP)(buffer[colNum++]))->appendInt(&orderDate, 1);
        int orderTime = convertToTime(data.uni.tickOrder.order_time);
        ((VectorSP)(buffer[colNum++]))->appendInt(&orderTime, 1);
        string securityIDSource = transMarket(data.uni.tickOrder.market_type);
        ((VectorSP)(buffer[colNum++]))->appendString(&securityIDSource, 1);
        string securityType("BondType");
        ((VectorSP)(buffer[colNum++]))->appendString(&securityType, 1);
        int DailyIndex = data.uni.tickOrder.appl_seq_num;
        ((VectorSP)(buffer[colNum++]))->appendInt(&DailyIndex, 1);
        int sourceType = 0;
        ((VectorSP)(buffer[colNum++]))->appendInt(&sourceType, 1);
        int type = convertType(data.uni.tickOrder.order_type);
        ((VectorSP)(buffer[colNum++]))->appendInt(&type, 1);
        long long orderPrice = data.uni.tickOrder.order_price;
        ((VectorSP)(buffer[colNum++]))->appendLong(&orderPrice, 1);
        long long orderVolume = data.uni.tickOrder.order_volume;
        ((VectorSP)(buffer[colNum++]))->appendLong(&orderVolume, 1);
        int BSFlag = convertBSFlag(data.uni.tickOrder.side);
        ((VectorSP)(buffer[colNum++]))->appendInt(&BSFlag, 1);
        long long origOrderNo = data.uni.tickOrder.orig_order_no;
        ((VectorSP)(buffer[colNum++]))->appendLong(&origOrderNo, 1);
        // same as the previous one
        ((VectorSP)(buffer[colNum++]))->appendLong(&origOrderNo, 1);
        long long applSeqNum = data.uni.tickOrder.appl_seq_num;
        ((VectorSP)(buffer[colNum++]))->appendLong(&applSeqNum, 1);
        int channelNo = data.uni.tickOrder.channel_no;
        ((VectorSP)(buffer[colNum++]))->appendInt(&channelNo, 1);
        int fakeDay = INT_MIN;
        ((VectorSP)(buffer[colNum++]))->appendInt(&fakeDay, 1);
    } else {
        if(marketType == 101) {
            string securityCode = data.uni.tickExecution.security_code + string(".SH");
            ((VectorSP)(buffer[colNum++]))->appendString(&securityCode, 1);
        } else if (marketType == 102) {
            string securityCode = data.uni.tickExecution.security_code + string(".SZ");
            ((VectorSP)(buffer[colNum++]))->appendString(&securityCode, 1);
        }
        int orderDate = convertToDate(data.uni.tickExecution.exec_time);
        ((VectorSP)(buffer[colNum++]))->appendInt(&orderDate, 1);
        int orderTime = convertToTime(data.uni.tickExecution.exec_time);
        ((VectorSP)(buffer[colNum++]))->appendInt(&orderTime, 1);
        string securityIDSource = transMarket(data.uni.tickExecution.market_type);
        ((VectorSP)(buffer[colNum++]))->appendString(&securityIDSource, 1);
        string securityType("BondType");
        ((VectorSP)(buffer[colNum++]))->appendString(&securityType, 1);
        int DailyIndex = data.uni.tickExecution.appl_seq_num;
        ((VectorSP)(buffer[colNum++]))->appendInt(&DailyIndex, 1);
        int sourceType = 1;
        ((VectorSP)(buffer[colNum++]))->appendInt(&sourceType, 1);
        int type = convertType(data.uni.tickExecution.exec_type);
        ((VectorSP)(buffer[colNum++]))->appendInt(&type, 1);
        long long orderPrice = data.uni.tickExecution.exec_price;
        ((VectorSP)(buffer[colNum++]))->appendLong(&orderPrice, 1);
        long long orderVolume = data.uni.tickExecution.exec_volume;
        ((VectorSP)(buffer[colNum++]))->appendLong(&orderVolume, 1);
        int BSFlag = convertBSFlag(data.uni.tickExecution.side);
        ((VectorSP)(buffer[colNum++]))->appendInt(&BSFlag, 1);
        long long origOrderNo = data.uni.tickExecution.bid_appl_seq_num;
        ((VectorSP)(buffer[colNum++]))->appendLong(&origOrderNo, 1);
        long long offerApplSeqNum = data.uni.tickExecution.offer_appl_seq_num;
        ((VectorSP)(buffer[colNum++]))->appendLong(&offerApplSeqNum, 1);
        long long applSeqNum = data.uni.tickExecution.appl_seq_num;
        ((VectorSP)(buffer[colNum++]))->appendLong(&applSeqNum, 1);
        int channelNo = data.uni.tickExecution.channel_no;
        ((VectorSP)(buffer[colNum++]))->appendInt(&channelNo, 1);
        int fakeDay = INT_MIN;
        ((VectorSP)(buffer[colNum++]))->appendInt(&fakeDay, 1);
    }
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

int convertType(int type) {
    switch (type) {
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
    if (type == 101) {
        return "XSHG";
    } else if (type == 102) {
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
    } else if (typeStr == "fundOrderExecution") {
        return AMD_FUND_ORDER_EXECUTION;
    } else if (typeStr == "bondOrderExecution") {
        return AMD_BOND_ORDER_EXECUTION;
    } else {
        throw RuntimeException(
            "DataType should be `snapshot, `execution, `order, `index, `future, `option, `IOPV(Except for amd version 3.9.6), `orderQueue, `fundSnapshot, "
            "`fundExecution`, `fundOrder, 'orderExecution', 'fundOrderExecution' or 'bondOrderExecution'");
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
    int channelNo = 0;
    if(data.orderOrExecution) {
        channelNo = data.uni.tickOrder.channel_no;
    } else {
        channelNo = data.uni.tickExecution.channel_no;
    }
    return index.getIndex(channelNo, timestamp);
}

template <>
int getDailyIndex(DailyIndex &index, MDBondOrderExecution& data, long long timestamp) {
    int channelNo = 0;
    if(data.orderOrExecution) {
        channelNo = data.uni.tickOrder.channel_no;
    } else {
        channelNo = data.uni.tickExecution.channel_no;
    }
    return index.getIndex(channelNo, timestamp);
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
