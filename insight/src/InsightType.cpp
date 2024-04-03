#include "InsightType.h"

#include <string>

#include "Exceptions.h"
#include "ddbplugin/ThreadedQueue.h"

static int countDays(int day) {
    int year = day / 10000;
    day %= 10000;
    int month = day / 100;
    day %= 100;
    return Util::countDays(year, month, day);
}

static int realTime(int fake) {
    int real = 0;
    real += 3600000 * (fake / 10000000);
    fake %= 10000000;
    real += 60000 * (fake / 100000);
    fake %= 100000;
    real += fake;
    return real;
}

MetaTable InsightIndexTickMeta {
    {
        "MDDate", "MDTime", "HTSCSecurityID", "LastPx", "HighPx",
        "LowPx", "TotalVolumeTrade", "TotalValueTrade", "TradingPhaseCode",
    },
    {
        DT_DATE, DT_TIME, DT_SYMBOL, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_SYMBOL
    }
};

MetaTable InsightStockTickMeta {
    {
        "HTSCSecurityID", "MDDate", "MDTime","securityIDSource", "PreClosePx", "TotalVolumeTrade", "TotalValueTrade", "LastPx", "OpenPx", "HighPx",

        "LowPx", "DiffPx1", "TotalBuyQty", "TotalSellQty", "WeightedAvgBuyPx", "WeightedAvgSellPx", "BuyPrice1", "BuyPrice2", "BuyPrice3", "BuyPrice4",

        "BuyPrice5", "BuyPrice6", "BuyPrice7", "BuyPrice8", "BuyPrice9", "BuyPrice10", "BuyOrderQty1", "BuyOrderQty2", "BuyOrderQty3", "BuyOrderQty4",

        "BuyOrderQty5", "BuyOrderQty6", "BuyOrderQty7", "BuyOrderQty8", "BuyOrderQty9", "BuyOrderQty10", "SellPrice1", "SellPrice2", "SellPrice3", "SellPrice4",

        "SellPrice5", "SellPrice6", "SellPrice7", "SellPrice8", "SellPrice9", "SellPrice10", "SellOrderQty1", "SellOrderQty2", "SellOrderQty3", "SellOrderQty4",

        "SellOrderQty5", "SellOrderQty6", "SellOrderQty7", "SellOrderQty8", "SellOrderQty9", "SellOrderQty10", "BuyOrder1", "BuyOrder2", "BuyOrder3", "BuyOrder4",

        "BuyOrder5", "BuyOrder6", "BuyOrder7", "BuyOrder8", "BuyOrder9", "BuyOrder10", "SellOrder1", "SellOrder2", "SellOrder3", "SellOrder4",

        "SellOrder5", "SellOrder6", "SellOrder7", "SellOrder8", "SellOrder9", "SellOrder10", "BuyNumOrders1", "BuyNumOrders2", "BuyNumOrders3", "BuyNumOrders4",

        "BuyNumOrders5", "BuyNumOrders6", "BuyNumOrders7", "BuyNumOrders8", "BuyNumOrders9", "BuyNumOrders10", "SellNumOrders1", "SellNumOrders2", "SellNumOrders3", "SellNumOrders4",

        "SellNumOrders5", "SellNumOrders6", "SellNumOrders7", "SellNumOrders8", "SellNumOrders9", "SellNumOrders10", "TradingPhaseCode",
    },

    {
        DT_SYMBOL, DT_DATE, DT_TIME, DT_SYMBOL, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_SYMBOL
    }
};
MetaTable InsightFutureTickMeta {
    {
        "HTSCSecurityID", "MDDate", "MDTime", "securityIDSource", "PreClosePx", "TotalVolumeTrade", "TotalValueTrade", "LastPx", "OpenPx", "HighPx",
        "LowPx", "PreOpenInterestPx", "PreSettlePrice", "OpenInterest", "BuyPrice1", "BuyPrice2", "BuyPrice3", "BuyPrice4", "BuyPrice5", "BuyOrderQty1",
        "BuyOrderQty2", "BuyOrderQty3", "BuyOrderQty4", "BuyOrderQty5", "SellPrice1", "SellPrice2", "SellPrice3", "SellPrice4", "SellPrice5", "SellOrderQty1",
        "SellOrderQty2", "SellOrderQty3", "SellOrderQty4", "SellOrderQty5", "TradingPhaseCode",
    },

    {
        DT_SYMBOL, DT_DATE, DT_TIME, DT_SYMBOL, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG,   DT_LONG, DT_LONG, DT_LONG,   DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG,   DT_LONG, DT_LONG, DT_LONG,   DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG,   DT_LONG, DT_LONG, DT_LONG,   DT_SYMBOL,
    }
};
MetaTable InsightTransactionMeta {
    {
        "HTSCSecurityID", "MDDate",      "MDTime",      "securityIDSource", "TradeIndex",
        "TradeBuyNo",     "TradeSellNo", "TradeBSFlag", "TradePrice",       "TradeQty",
        "TradeMoney",     "ApplSeqNum",
    },

    {
        DT_SYMBOL, DT_DATE, DT_TIME, DT_SYMBOL, DT_LONG,
        DT_LONG,   DT_LONG, DT_INT,  DT_LONG,   DT_LONG,
        DT_LONG,   DT_LONG,
    }
};
MetaTable InsightOrderMeta {
    {
        "HTSCSecurityID", "MDDate",     "MDTime",    "securityIDSource", "securityType",
        "OrderIndex",     "SourceType", "OrderType", "OrderPrice",       "OrderQty",
        "OrderBSFlag",    "BuyNo",      "SellNo",    "ApplSeqNum",       "ChannelNo",

    },

    {
        DT_SYMBOL, DT_DATE, DT_TIME, DT_SYMBOL, DT_SYMBOL,
        DT_LONG,   DT_INT,  DT_INT,  DT_LONG,   DT_LONG,
        DT_INT,    DT_LONG, DT_LONG, DT_LONG,   DT_INT,

    }
};
MetaTable InsightOrderTransactionMeta {
    {
        "SecurityID", "MDDate", "MDTime", "SecurityIDSource", "SecurityType", "Index", "SourceType", "Type", \
        "Price", "Qty", "BSFlag", "BuyNo", "SellNo", "ApplSeqNum", "ChannelNo"
    },

    {
        DT_SYMBOL, DT_DATE, DT_TIME, DT_SYMBOL, DT_SYMBOL, DT_LONG, DT_INT, DT_INT, \
        DT_LONG, DT_LONG, DT_INT, DT_LONG, DT_LONG, DT_LONG, DT_INT,
    }
};

void initTypeContainer(MarketTypeContainer &container) {
    container.add("OrderTransaction", InsightOrderTransactionMeta);
    container.add("StockTick", InsightStockTickMeta);
    container.add("IndexTick", InsightIndexTickMeta);
    container.add("FuturesTick", InsightFutureTickMeta);
    container.add("Transaction", InsightTransactionMeta);
    container.add("Order", InsightOrderMeta);
}

void orderReader(vector<ConstantSP> &buffer, InsightOrder &data) {
    string htscsecurityid = data.order.htscsecurityid();
    ((VectorSP)buffer[(0)])->appendString(&htscsecurityid, 1);
    int mddate = countDays(data.order.mddate());
    ((VectorSP)buffer[(1)])->appendInt(&mddate, 1);
    int mdtime = realTime(data.order.mdtime());
    ((VectorSP)buffer[(2)])->appendInt(&mdtime, 1);
    string securityidsource = com::htsc::mdc::model::ESecurityIDSource_Name(data.order.securityidsource());
    ((VectorSP)buffer[(3)])->appendString(&securityidsource, 1);
    string securitytype = com::htsc::mdc::model::ESecurityType_Name(data.order.securitytype());
    ((VectorSP)buffer[(4)])->appendString(&securitytype, 1);

    long long orderindex = data.order.orderindex();
    ((VectorSP)buffer[(5)])->appendLong(&orderindex, 1);
    int tmp = 0;
    ((VectorSP)buffer[(6)])->appendInt(&tmp, 1);

    int ordertype = data.order.ordertype();
    ((VectorSP)buffer[(7)])->appendInt(&ordertype, 1);
    long long orderprice = data.order.orderprice();
    ((VectorSP)buffer[(8)])->appendLong(&orderprice, 1);
    long long orderqty = data.order.orderqty();
    ((VectorSP)buffer[(9)])->appendLong(&orderqty, 1);

    int orderbsflag = data.order.orderbsflag();
    ((VectorSP)buffer[(10)])->appendInt(&orderbsflag, 1);
    long long orderno = data.order.orderno();
    ((VectorSP)buffer[(11)])->appendLong(&orderno, 1);
    ((VectorSP)buffer[(12)])->appendLong(&orderno, 1);
    long long applseqnum = data.order.applseqnum();
    ((VectorSP)buffer[(13)])->appendLong(&applseqnum, 1);
    int channelno = data.order.channelno();
    ((VectorSP)buffer[(14)])->appendInt(&channelno, 1);
}

void transactionReader(vector<ConstantSP> &buffer, InsightTransaction &data) {
    string htscsecurityid = data.transaction.htscsecurityid();
    ((VectorSP)buffer[(0)])->appendString(&htscsecurityid, 1);
    int mddate = countDays(data.transaction.mddate());
    ((VectorSP)buffer[(1)])->appendInt(&mddate, 1);
    int mdtime = realTime(data.transaction.mdtime());
    ((VectorSP)buffer[(2)])->appendInt(&mdtime, 1);
    string securityidsource = com::htsc::mdc::model::ESecurityIDSource_Name(data.transaction.securityidsource());
    ((VectorSP)buffer[(3)])->appendString(&securityidsource, 1);

    long long tradeindex = data.transaction.tradeindex();
    ((VectorSP)buffer[(4)])->appendLong(&tradeindex, 1);
    long long tradebuyno = data.transaction.tradebuyno();
    ((VectorSP)buffer[(5)])->appendLong(&tradebuyno, 1);
    long long tradesellno = data.transaction.tradesellno();
    ((VectorSP)buffer[(6)])->appendLong(&tradesellno, 1);
    int tradebsflag = data.transaction.tradebsflag();
    ((VectorSP)buffer[(7)])->appendInt(&tradebsflag, 1);
    long long tradeprice = data.transaction.tradeprice();
    ((VectorSP)buffer[(8)])->appendLong(&tradeprice, 1);
    long long tradeqty = data.transaction.tradeqty();
    ((VectorSP)buffer[(9)])->appendLong(&tradeqty, 1);
    long long trademoney = data.transaction.trademoney();
    ((VectorSP)buffer[(10)])->appendLong(&trademoney, 1);
    long long applseqnum = data.transaction.applseqnum();
    ((VectorSP)buffer[(11)])->appendLong(&applseqnum, 1);
}

void checkSeqNumComplete(std::string &tag, std::unordered_map<int, int64_t> &lastSeqNum, int64_t seqNum, int channelNo,
                         bool ignoreApplSeq) {
    if (lastSeqNum.find(channelNo) != lastSeqNum.end() && seqNum != lastSeqNum[channelNo] + 1) {
        string errMsg = "wrong " + tag + " channel " + std::to_string(channelNo) + ", expected " +
                        std::to_string(lastSeqNum[channelNo] + 1) + ", actually " + std::to_string(seqNum);
        if (!ignoreApplSeq) {
            if (lastSeqNum[channelNo] == -1)
                throw RuntimeException("channel " + std::to_string(channelNo) +
                                       "stops receiving data due to data discontinuity of " + tag);
            else {
                lastSeqNum[channelNo] = -1;
                throw RuntimeException(errMsg);
            }
        }
        LOG_WARN(PLUGIN_INSIGHT_PREFIX + errMsg);
    }
    lastSeqNum[channelNo] = seqNum;
}

void orderTransactionReader(vector<ConstantSP> &buffer, InsightOrderTransaction &data, bool ignoreApplSeq,
                            std::unordered_map<int, int64_t> &lastSeqNum, std::unordered_map<int, int64_t> &tradeSeqNum,
                            std::unordered_map<int, int64_t> &orderSeqNum) {
    if (!data.orderOrTransaction) {
        string securityidsource = com::htsc::mdc::model::ESecurityIDSource_Name(data.transaction.securityidsource());
        int channelno = data.transaction.channelno();
        long long applseqnum = data.transaction.applseqnum();
        long long tradeindex = data.transaction.tradeindex();
        if (securityidsource == "XSHE") {
            string tag = "applseqnum on " + securityidsource;
            checkSeqNumComplete(tag, lastSeqNum, applseqnum, channelno, ignoreApplSeq);
        } else if (securityidsource == "XSHG") {
            string tag = "tradeindex on " + securityidsource + " transaction";
            checkSeqNumComplete(tag, tradeSeqNum, tradeindex, channelno, ignoreApplSeq);
        } else {
            throw RuntimeException("wrong securityidsource: " + securityidsource);
        }
        string htscsecurityid = data.transaction.htscsecurityid();
        ((VectorSP)buffer[0])->appendString(&htscsecurityid, 1);
        int mddate = countDays(data.transaction.mddate());
        ((VectorSP)buffer[1])->appendInt(&mddate, 1);
        int mdtime = realTime(data.transaction.mdtime());
        ((VectorSP)buffer[2])->appendInt(&mdtime, 1);

        ((VectorSP)buffer[3])->appendString(&securityidsource, 1);
        string securitytype = com::htsc::mdc::model::ESecurityType_Name(data.transaction.securitytype());
        ((VectorSP)buffer[4])->appendString(&securitytype, 1);

        ((VectorSP)buffer[5])->appendLong(&tradeindex, 1);
        int type = 1;
        ((VectorSP)buffer[6])->appendInt(&type, 1);
        int tradetype = data.transaction.tradetype();
        ((VectorSP)buffer[7])->appendInt(&tradetype, 1);
        long long tradeprice = data.transaction.tradeprice();
        ((VectorSP)buffer[8])->appendLong(&tradeprice, 1);
        long long tradeqty = data.transaction.tradeqty();
        ((VectorSP)buffer[9])->appendLong(&tradeqty, 1);
        int tradebsflag = data.transaction.tradebsflag();
        ((VectorSP)buffer[10])->appendInt(&tradebsflag, 1);
        long long tradebuyno = data.transaction.tradebuyno();
        ((VectorSP)buffer[11])->appendLong(&tradebuyno, 1);
        long long tradesellno = data.transaction.tradesellno();
        ((VectorSP)buffer[12])->appendLong(&tradesellno, 1);

        ((VectorSP)buffer[13])->appendLong(&applseqnum, 1);

        ((VectorSP)buffer[14])->appendInt(&channelno, 1);
    } else {
        string securityidsource = com::htsc::mdc::model::ESecurityIDSource_Name(data.order.securityidsource());
        int channelno = data.order.channelno();
        long long applseqnum = data.order.applseqnum();
        long long orderindex = data.order.orderindex();
        if (securityidsource == "XSHE") {
            string tag = "applseqnum on " + securityidsource;
            checkSeqNumComplete(tag, lastSeqNum, applseqnum, channelno, ignoreApplSeq);
        } else if (securityidsource == "XSHG") {
            string tag = "orderindex on " + securityidsource + " order";
            checkSeqNumComplete(tag, orderSeqNum, orderindex, channelno, ignoreApplSeq);
        } else {
            throw RuntimeException("wrong securityidsource: " + securityidsource);
        }
        string htscsecurityid = data.order.htscsecurityid();
        ((VectorSP)buffer[0])->appendString(&htscsecurityid, 1);
        int mddate = countDays(data.order.mddate());
        ((VectorSP)buffer[1])->appendInt(&mddate, 1);
        int mdtime = realTime(data.order.mdtime());
        ((VectorSP)buffer[2])->appendInt(&mdtime, 1);
        ((VectorSP)buffer[3])->appendString(&securityidsource, 1);
        string securitytype = com::htsc::mdc::model::ESecurityType_Name(data.order.securitytype());
        ((VectorSP)buffer[4])->appendString(&securitytype, 1);
        ((VectorSP)buffer[5])->appendLong(&orderindex, 1);
        int type = 0;
        ((VectorSP)buffer[6])->appendInt(&type, 1);
        int ordertype = data.order.ordertype();
        ((VectorSP)buffer[7])->appendInt(&ordertype, 1);
        long long orderprice = data.order.orderprice();
        ((VectorSP)buffer[8])->appendLong(&orderprice, 1);
        long long orderqty = data.order.orderqty();
        ((VectorSP)buffer[9])->appendLong(&orderqty, 1);
        int orderbsflag = data.order.orderbsflag();
        ((VectorSP)buffer[10])->appendInt(&orderbsflag, 1);
        long long orderno = data.order.orderno();
        ((VectorSP)buffer[11])->appendLong(&orderno, 1);
        ((VectorSP)buffer[12])->appendLong(&orderno, 1);
        ((VectorSP)buffer[13])->appendLong(&applseqnum, 1);
        ((VectorSP)buffer[14])->appendInt(&channelno, 1);
    }
}

void indexTickReader(vector<ConstantSP> &buffer, InsightIndexTick &data) {
    int mddate = countDays(data.indexTick.mddate());
    ((VectorSP)buffer[(0)])->appendInt(&mddate, 1);
    int mdtime = realTime(data.indexTick.mdtime());
    ((VectorSP)buffer[(1)])->appendInt(&mdtime, 1);
    string htscsecurityid = data.indexTick.htscsecurityid();
    ((VectorSP)buffer[(2)])->appendString(&htscsecurityid, 1);
    long long lastpx = data.indexTick.lastpx();
    ((VectorSP)buffer[(3)])->appendLong(&lastpx, 1);
    long long highpx = data.indexTick.highpx();
    ((VectorSP)buffer[(4)])->appendLong(&highpx, 1);
    long long lowpx = data.indexTick.lowpx();
    ((VectorSP)buffer[(5)])->appendLong(&lowpx, 1);
    long long totalvolumetrade = data.indexTick.totalvolumetrade();
    ((VectorSP)buffer[(6)])->appendLong(&totalvolumetrade, 1);
    long long totalvaluetrade = data.indexTick.totalvaluetrade();
    ((VectorSP)buffer[(7)])->appendLong(&totalvaluetrade, 1);
    string tradingphasecode = data.indexTick.tradingphasecode();
    ((VectorSP)buffer[(8)])->appendString(&tradingphasecode, 1);
}
long long LONG_MIN_VALUE = LONG_LONG_MIN;
void futureTickReader(vector<ConstantSP> &buffer, InsightFutureTick &data) {
    string htscsecurityid = data.futureTick.htscsecurityid();
    ((VectorSP)buffer[(0)])->appendString(&htscsecurityid, 1);
    int mddate = countDays(data.futureTick.mddate());
    ((VectorSP)buffer[(1)])->appendInt(&mddate, 1);
    int mdtime = realTime(data.futureTick.mdtime());
    ((VectorSP)buffer[(2)])->appendInt(&mdtime, 1);
    string securityidsource = com::htsc::mdc::model::ESecurityIDSource_Name(data.futureTick.securityidsource());
    ((VectorSP)buffer[(3)])->appendString(&securityidsource, 1);

    long long preclosepx = data.futureTick.preclosepx();
    ((VectorSP)buffer[(4)])->appendLong(&preclosepx, 1);
    long long totalvolumetrade = data.futureTick.totalvolumetrade();
    ((VectorSP)buffer[(5)])->appendLong(&totalvolumetrade, 1);
    long long totalvaluetrade = data.futureTick.totalvaluetrade();
    ((VectorSP)buffer[(6)])->appendLong(&totalvaluetrade, 1);
    long long lastpx = data.futureTick.lastpx();
    ((VectorSP)buffer[(7)])->appendLong(&lastpx, 1);
    long long openpx = data.futureTick.openpx();
    ((VectorSP)buffer[(8)])->appendLong(&openpx, 1);
    long long highpx = data.futureTick.highpx();
    ((VectorSP)buffer[(9)])->appendLong(&highpx, 1);
    long long lowpx = data.futureTick.lowpx();
    ((VectorSP)buffer[(10)])->appendLong(&lowpx, 1);
    long long preopeninterest = data.futureTick.preopeninterest();
    ((VectorSP)buffer[(11)])->appendLong(&preopeninterest, 1);
    long long presettleprice = data.futureTick.presettleprice();
    ((VectorSP)buffer[(12)])->appendLong(&presettleprice, 1);
    long long openinterest = data.futureTick.openinterest();
    ((VectorSP)buffer[(13)])->appendLong(&openinterest, 1);

    for (int i = 0; i < data.futureTick.buypricequeue_size() && i < 5; i++) {
        long long buypricequeue = data.futureTick.buypricequeue(i);
        ((VectorSP)buffer[14 + i])->appendLong(&buypricequeue, 1);
    }
    for (int i = data.futureTick.buypricequeue_size(); i < 5; i++) {
        ((VectorSP)buffer[14 + i])->appendLong(&LONG_MIN_VALUE, 1);
    }
    for (int i = 0; i < data.futureTick.buyorderqtyqueue_size() && i < 5; i++) {
        long long buyorderqtyqueue = data.futureTick.buyorderqtyqueue(i);
        ((VectorSP)buffer[19 + i])->appendLong(&buyorderqtyqueue, 1);
    }
    for (int i = data.futureTick.buyorderqtyqueue_size(); i < 5; i++) {
        ((VectorSP)buffer[19 + i])->appendLong(&LONG_MIN_VALUE, 1);
    }

    for (int i = 0; i < data.futureTick.sellpricequeue_size() && i < 5; i++) {
        long long sellpricequeue = data.futureTick.sellpricequeue(0 + i);
        ((VectorSP)buffer[24 + i])->appendLong(&sellpricequeue, 1);
    }
    for (int i = data.futureTick.sellpricequeue_size(); i < 5; i++) {
        ((VectorSP)buffer[24 + i])->appendLong(&LONG_MIN_VALUE, 1);
    }

    for (int i = 0; i < data.futureTick.sellorderqtyqueue_size() && i < 5; i++) {
        long long sellorderqtyqueue = data.futureTick.sellorderqtyqueue(0 + i);
        ((VectorSP)buffer[29 + i])->appendLong(&sellorderqtyqueue, 1);
    }
    for (int i = data.futureTick.sellorderqtyqueue_size(); i < 5; i++) {
        ((VectorSP)buffer[29 + i])->appendLong(&LONG_MIN_VALUE, 1);
    }

    string tradingphasecode = data.futureTick.tradingphasecode();
    ((VectorSP)buffer[(34)])->appendString(&tradingphasecode, 1);
}
void stockTickReader(vector<ConstantSP> &buffer, InsightStockTick &data) {
    string htscsecurityid = data.stockTick.htscsecurityid();
    ((VectorSP)((VectorSP)buffer[(0)]))->appendString(&htscsecurityid, 1);
    int mddate = countDays(data.stockTick.mddate());
    ((VectorSP)buffer[(1)])->appendInt(&mddate, 1);
    int mdtime = realTime(data.stockTick.mdtime());
    ((VectorSP)buffer[(2)])->appendInt(&mdtime, 1);
    string securityidsource = com::htsc::mdc::model::ESecurityIDSource_Name(data.stockTick.securityidsource());
    ((VectorSP)buffer[(3)])->appendString(&securityidsource, 1);

    long long preclosepx = data.stockTick.preclosepx();
    ((VectorSP)buffer[(4)])->appendLong(&preclosepx, 1);
    long long totalvolumetrade = data.stockTick.totalvolumetrade();
    ((VectorSP)buffer[(5)])->appendLong(&totalvolumetrade, 1);
    long long totalvaluetrade = data.stockTick.totalvaluetrade();
    ((VectorSP)buffer[(6)])->appendLong(&totalvaluetrade, 1);
    long long lastpx = data.stockTick.lastpx();
    ((VectorSP)buffer[(7)])->appendLong(&lastpx, 1);
    long long openpx = data.stockTick.openpx();
    ((VectorSP)buffer[(8)])->appendLong(&openpx, 1);
    long long highpx = data.stockTick.highpx();
    ((VectorSP)buffer[(9)])->appendLong(&highpx, 1);
    long long lowpx = data.stockTick.lowpx();
    ((VectorSP)buffer[(10)])->appendLong(&lowpx, 1);
    long long diffpx1 = data.stockTick.diffpx1();
    ((VectorSP)buffer[(11)])->appendLong(&diffpx1, 1);
    long long totalbuyqty = data.stockTick.totalbuyqty();
    ((VectorSP)buffer[(12)])->appendLong(&totalbuyqty, 1);
    long long totalsellqty = data.stockTick.totalsellqty();
    ((VectorSP)buffer[(13)])->appendLong(&totalsellqty, 1);
    long long weightedavgbuypx = data.stockTick.weightedavgbuypx();
    ((VectorSP)buffer[(14)])->appendLong(&weightedavgbuypx, 1);
    long long weightedavgsellpx = data.stockTick.weightedavgsellpx();
    ((VectorSP)buffer[(15)])->appendLong(&weightedavgsellpx, 1);

    for (int i = 0; i < data.stockTick.buypricequeue_size() && i < 10; i++) {
        long long buypricequeue = data.stockTick.buypricequeue(i);
        ((VectorSP)buffer[16 + i])->appendLong(&buypricequeue, 1);
    }
    for (int i = data.stockTick.buypricequeue_size(); i < 10; i++) {
        ((VectorSP)buffer[16 + i])->appendLong(&LONG_MIN_VALUE, 1);
    }
    for (int i = 0; i < data.stockTick.buyorderqtyqueue_size() && i < 10; i++) {
        long long buyorderqtyqueue = data.stockTick.buyorderqtyqueue(i);
        ((VectorSP)buffer[26 + i])->appendLong(&buyorderqtyqueue, 1);
    }
    for (int i = data.stockTick.buyorderqtyqueue_size(); i < 10; i++) {
        ((VectorSP)buffer[26 + i])->appendLong(&LONG_MIN_VALUE, 1);
    }
    for (int i = 0; i < data.stockTick.sellpricequeue_size() && i < 10; i++) {
        long long sellpricequeue = data.stockTick.sellpricequeue(i);
        ((VectorSP)buffer[36 + i])->appendLong(&sellpricequeue, 1);
    }
    for (int i = data.stockTick.sellpricequeue_size(); i < 10; i++) {
        ((VectorSP)buffer[36 + i])->appendLong(&LONG_MIN_VALUE, 1);
    }
    for (int i = 0; i < data.stockTick.sellorderqtyqueue_size() && i < 10; i++) {
        long long sellorderqtyqueue = data.stockTick.sellorderqtyqueue(i);
        ((VectorSP)buffer[46 + i])->appendLong(&sellorderqtyqueue, 1);
    }
    for (int i = data.stockTick.sellorderqtyqueue_size(); i < 10; i++) {
        ((VectorSP)buffer[46 + i])->appendLong(&LONG_MIN_VALUE, 1);
    }
    for (int i = 0; i < data.stockTick.buyorderqueue_size() && i < 10; i++) {
        long long buyorderqueue = data.stockTick.buyorderqueue(i);
        ((VectorSP)buffer[56 + i])->appendLong(&buyorderqueue, 1);
    }
    for (int i = data.stockTick.buyorderqueue_size(); i < 10; i++) {
        ((VectorSP)buffer[56 + i])->appendLong(&LONG_MIN_VALUE, 1);
    }
    for (int i = 0; i < data.stockTick.sellorderqueue_size() && i < 10; i++) {
        long long sellorderqueue = data.stockTick.sellorderqueue(i);
        ((VectorSP)buffer[66 + i])->appendLong(&sellorderqueue, 1);
    }
    for (int i = data.stockTick.sellorderqueue_size(); i < 10; i++) {
        ((VectorSP)buffer[66 + i])->appendLong(&LONG_MIN_VALUE, 1);
    }
    for (int i = 0; i < data.stockTick.buynumordersqueue_size() && i < 10; i++) {
        long long buynumordersqueue = data.stockTick.buynumordersqueue(i);
        ((VectorSP)buffer[76 + i])->appendLong(&buynumordersqueue, 1);
    }
    for (int i = data.stockTick.buynumordersqueue_size(); i < 10; i++) {
        ((VectorSP)buffer[76 + i])->appendLong(&LONG_MIN_VALUE, 1);
    }
    for (int i = 0; i < data.stockTick.sellnumordersqueue_size() && i < 10; i++) {
        long long sellnumordersqueue = data.stockTick.sellnumordersqueue(i);
        ((VectorSP)buffer[86 + i])->appendLong(&sellnumordersqueue, 1);
    }
    for (int i = data.stockTick.sellnumordersqueue_size(); i < 10; i++) {
        ((VectorSP)buffer[86 + i])->appendLong(&LONG_MIN_VALUE, 1);
    }
    string tradingphasecode = data.stockTick.tradingphasecode();
    ((VectorSP)buffer[(96)])->appendString(&tradingphasecode, 1);
}