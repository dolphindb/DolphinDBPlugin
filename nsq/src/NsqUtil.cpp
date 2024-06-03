//
// Created by htxu on 11/22/2023.
//


#include "NsqUtil.h"

namespace nsqUtil {

    void tradeReader(vector<ConstantSP> &buffer, TradeDataStruct &data) {

        auto col = buffer.begin();

        appendString(col++, data.data.ExchangeID);
        appendString(col++, data.data.InstrumentID);
        appendInt(col++, data.data.TransFlag);
        appendLong(col++, data.data.SeqNo);
        appendInt(col++, data.data.ChannelNo);
        appendInt(col++, getDate(data.data.TradeDate));
        appendInt(col++, getTime(data.data.TransactTime));
        appendDouble(col++, data.data.TrdPrice);
        appendLong(col++, data.data.TrdVolume);
        appendDouble(col++, data.data.TrdMoney);
        appendLong(col++, data.data.TrdBuyNo);
        appendLong(col++, data.data.TrdSellNo);
        appendChar(col++, data.data.TrdBSFlag);
        appendLong(col++, data.data.BizIndex);
    }

    void entrustHelper(ConstantVecIterator &col, EntrustDataStruct &data) {
        appendString(col++, data.data.ExchangeID);
        appendString(col++, data.data.InstrumentID);
        appendInt(col++, data.data.TransFlag);
        appendLong(col++, data.data.SeqNo);
        appendInt(col++, data.data.ChannelNo);
        appendInt(col++, getDate(data.data.TradeDate));
        appendInt(col++, getTime(data.data.TransactTime));
        appendDouble(col++, data.data.OrdPrice);
        appendLong(col++, data.data.OrdVolume);
        appendChar(col++, data.data.OrdSide);
        appendChar(col++, data.data.OrdType);
        appendLong(col++, data.data.OrdNo);
        appendLong(col++, data.data.BizIndex);
    }

    void entrustReader(vector<ConstantSP> &buffer, EntrustDataStruct &data) {
        auto col = buffer.begin();
        entrustHelper(col, data);
    }

    void entrustReader_220105(vector<ConstantSP> &buffer, EntrustDataStruct &data) {
        auto col = buffer.begin();
        entrustHelper(col, data);
        appendChar(col++, data.data.TickStatus);
        appendLong(col++, data.data.TrdVolume);
    }

    void tradeEntrustReader(vector<ConstantSP> &buffer, TradeEntrustDataStruct &data) {

        auto col = buffer.begin();

        if (data.isTrade) {
            auto &tradeData = data.tradeData;

            auto Type = INT_MIN;
            auto BSFlag = INT_MIN;
            switch (tradeData.TrdBSFlag) {
                case 'B':
                    BSFlag = 1;
                    break;
                case 'S':
                    BSFlag = 2;
                    break;
                case 'F':
                    Type = 0;
                    break;
                case '4':
                    Type = 1;
                    break;
            }

            appendString(col++, tradeData.InstrumentID);
            appendInt(col++, getDate(tradeData.TradeDate));
            appendInt(col++, getTime(tradeData.TransactTime));
            appendString(col++, tradeData.ExchangeID);
            appendString(col++, "");
            appendLong(col++, tradeData.SeqNo);
            appendInt(col++, 1);
            appendInt(col++, Type);
            appendLong(col++, (long long)round(tradeData.TrdPrice * 10000));
            appendLong(col++, tradeData.TrdVolume);
            appendInt(col++, BSFlag);
            appendLong(col++, tradeData.TrdBuyNo);
            appendLong(col++, tradeData.TrdSellNo);
            appendLong(col++, tradeData.SeqNo);
            appendInt(col++, tradeData.ChannelNo);
        } else {

            auto entrustData = data.entrustData;

            auto Type = INT_MIN;
            int sourceType = 0;
            switch (entrustData.OrdType) {
                case '1':
                    Type = 1;
                    break;
                case '2':
                case 'A':
                    Type = 2;
                    break;
                case 'U':
                    Type = 3;
                    break;
                case 'D':
                    Type = 10;
                    break;
                case 'S':
                    Type = 11;
                    sourceType = -1; // special treat of status msg
            }

            auto BSFlag = INT_MIN;
            switch (entrustData.OrdSide) {
                case '1':
                case 'B':
                    BSFlag = 1;
                    break;
                case '2':
                case 'S':
                    BSFlag = 2;
                    break;
            }

            appendString(col++, entrustData.InstrumentID);
            appendInt(col++, getDate(entrustData.TradeDate));
            appendInt(col++, getTime(entrustData.TransactTime));
            appendString(col++, entrustData.ExchangeID);
            appendString(col++, "");
            appendLong(col++, entrustData.SeqNo);
            appendInt(col++, sourceType);
            appendInt(col++, Type);
            appendLong(col++, (long long)round(entrustData.OrdPrice * 10000));
            appendLong(col++, entrustData.OrdVolume);
            appendInt(col++, BSFlag);
            appendLong(col++, entrustData.OrdNo);
            appendLong(col++, entrustData.OrdNo);
            appendLong(col++, entrustData.SeqNo);
            appendInt(col++, entrustData.ChannelNo);
        }
    }

    ConstantVecIterator &snapshotReaderConcise(ConstantVecIterator &col, SnapshotDataStruct &data) {

        appendString(col++, data.data.ExchangeID);
        appendString(col++, data.data.InstrumentID);
        appendDouble(col++, data.data.LastPrice);
        appendDouble(col++, data.data.PreClosePrice);
        appendDouble(col++, data.data.OpenPrice);
        appendDouble(col++, data.data.HighPrice);
        appendDouble(col++, data.data.LowPrice);
        appendDouble(col++, data.data.ClosePrice);
        appendDouble(col++, data.data.UpperLimitPrice);
        appendDouble(col++, data.data.LowerLimitPrice);
        appendInt(col++, getDate(data.data.TradeDate));
        appendInt(col++, getTime(data.data.UpdateTime));
        appendLong(col++, data.data.TradeVolume);
        appendDouble(col++, data.data.TradeBalance);
        appendDouble(col++, data.data.AveragePrice);
        for (auto bidPrice : data.data.BidPrice) appendDouble(col++, bidPrice);
        for (auto askPrice : data.data.AskPrice) appendDouble(col++, askPrice);
        for (auto bidVolume : data.data.BidVolume) appendLong(col++, bidVolume);
        for (auto askVolume : data.data.AskVolume) appendLong(col++, askVolume);
        appendLong(col++, data.data.TradesNum);
        appendChar(col++, data.data.InstrumentTradeStatus);
        appendLong(col++, data.data.TotalBidVolume);
        appendLong(col++, data.data.TotalAskVolume);
        appendDouble(col++, data.data.MaBidPrice);
        appendDouble(col++, data.data.MaAskPrice);
        appendDouble(col++, data.data.MaBondBidPrice);
        appendDouble(col++, data.data.MaBondAskPrice);
        appendDouble(col++, data.data.YieldToMaturity);
        appendDouble(col++, data.data.IOPV);
        appendInt(col++, data.data.EtfBuycount);
        appendInt(col++, data.data.EtfSellCount);
        appendLong(col++, data.data.EtfBuyVolume);
        appendDouble(col++, data.data.EtfBuyBalance);
        appendLong(col++, data.data.EtfSellVolume);
        appendDouble(col++, data.data.EtfSellBalance);
        appendLong(col++, data.data.TotalWarrantExecVolume);
        appendDouble(col++, data.data.WarrantLowerPrice);
        appendDouble(col++, data.data.WarrantUpperPrice);
        appendInt(col++, data.data.CancelBuyNum);
        appendInt(col++, data.data.CancelSellNum);
        appendLong(col++, data.data.CancelBuyVolume);
        appendLong(col++, data.data.CancelSellVolume);
        appendDouble(col++, data.data.CancelBuyValue);
        appendDouble(col++, data.data.CancelSellValue);
        appendInt(col++, data.data.TotalBuyNum);
        appendInt(col++, data.data.TotalSellNum);
        appendInt(col++, data.data.DurationAfterBuy);
        appendInt(col++, data.data.DurationAfterSell);
        appendInt(col++, data.data.BidOrdersNum);
        appendInt(col++, data.data.AskOrdersNum);
        appendDouble(col++, data.data.PreIOPV);

        return col;
    }

    void snapshotReaderExtra_220105(ConstantVecIterator &col, SnapshotDataStruct &data) {
        appendInt(col++, data.data.ChannelNo);
        appendDouble(col++, data.data.BondLastAuctionPrice);
        appendLong(col++, data.data.BondAuctionVolume);
        appendDouble(col++, data.data.BondAuctionBalance);
        appendChar(col++, data.data.BondLastTradeType);
        appendString(col++, data.data.BondTradeStatus);
        for(int i = 0; i < 10; ++i) {
            appendInt(col++, data.data.BidNumOrders[i]);
        }
        for(int i = 0; i < 10; ++i) {
            appendInt(col++, data.data.AskNumOrders[i]);
        }
    }

    void snapshotReaderExtra(ConstantVecIterator &col, SnapshotDataStruct &data) {

        appendInt(col++, data.Bid1Count);
        appendInt(col++, data.MaxBid1Count);
        appendInt(col++, data.Ask1Count);
        appendInt(col++, data.MaxAsk1Count);
        for (auto i = 0; i < data.Bid1Count; i++) {
            appendLong(col++, data.Bid1Volume[i]);
        }
        for (auto i = data.Bid1Count; i < 50; i++) {
            appendLong(col++, LONG_LONG_MIN);
        }
        for (auto i = 0; i < data.Ask1Count; i++) {
            appendLong(col++, data.Ask1Volume[i]);
        }
        for (auto i = data.Ask1Count; i < 50; i++) {
            appendLong(col++, LONG_LONG_MIN);
        }
    }

    void checkTypes(const string& dataType, const string& marketType) {

        if (dataType != ENTRUST_220105 and dataType != SNAPSHOT and dataType != TRADE and dataType != ENTRUST and dataType != TRADE_ENTRUST) {
            throw RuntimeException(NSQ_PREFIX + "dataType should be snapshot, trade, orders, or tradeAndOrder");
        }
        if (marketType != SH and marketType != SZ) {
            throw RuntimeException(NSQ_PREFIX + "marketType should be sh or sz.");
        }
    }

    int getDate(HSDate date) {
        int year, month, day;
        year = date / 10000;
        month = date % 10000 / 100;
        day = date % 100;
        return Date(year, month, day).getInt();
    }

    int getTime(HSTime time) {
        int hour, minute, second, ms;
        hour = time / 10000000;
        minute = time % 10000000 / 100000;
        second = time % 100000 / 1000;
        ms = time % 1000;
        return Time(hour, minute, second, ms).getInt();
    }

    /** Table Metas **/

    const MetaTable tradeMeta = {
            {
                    "ExchangeID", "InstrumentID", "TransFlag", "SeqNo", "ChannelNo",
                    "TradeDate", "TransactTime", "TrdPrice", "TrdVolume", "TrdMoney",
                    "TrdBuyNo", "TrdSellNo", "TrdBSFlag", "BizIndex"
            },
            {
                    DT_SYMBOL, DT_SYMBOL, DT_INT, DT_LONG, DT_INT,
                    DT_DATE, DT_TIME, DT_DOUBLE, DT_LONG, DT_DOUBLE,
                    DT_LONG, DT_LONG, DT_CHAR, DT_LONG
            }
    };

    const MetaTable entrustMeta = {
            {
                    "ExchangeID", "InstrumentID", "TransFlag", "SeqNo", "ChannelNo",
                    "TradeDate", "TransactTime", "OrdPrice", "OrdVolume", "OrdSide",
                    "OrdType", "OrdNo", "BizIndex"
            },
            {
                    DT_SYMBOL, DT_SYMBOL, DT_INT, DT_LONG, DT_INT,
                    DT_DATE, DT_TIME, DT_DOUBLE, DT_LONG, DT_CHAR,
                    DT_CHAR, DT_LONG, DT_LONG
            }
    };

    const MetaTable entrustMeta_220105 = {
            {
                    "ExchangeID", "InstrumentID", "TransFlag", "SeqNo", "ChannelNo",
                    "TradeDate", "TransactTime", "OrdPrice", "OrdVolume", "OrdSide",
                    "OrdType", "OrdNo", "BizIndex", "TickStatus", "TrdVolume"
            },
            {
                    DT_SYMBOL, DT_SYMBOL, DT_INT, DT_LONG, DT_INT,
                    DT_DATE, DT_TIME, DT_DOUBLE, DT_LONG, DT_CHAR,
                    DT_CHAR, DT_LONG, DT_LONG, DT_CHAR, DT_LONG
            }
    };


    const MetaTable snapshotMetaConcise = {
            {
                    "ExchangeID", "InstrumentID", "LastPrice", "PreClosePrice", "OpenPrice",
                    "HighPrice", "LowPrice", "ClosePrice", "UpperLimitPrice", "LowerLimitPrice",
                    "TradeDate", "UpdateTime", "TradeVolume", "TradeBalance", "AveragePrice",
                    "BidPrice0", "BidPrice1", "BidPrice2", "BidPrice3", "BidPrice4",
                    "BidPrice5", "BidPrice6", "BidPrice7", "BidPrice8", "BidPrice9",
                    "AskPrice0", "AskPrice1", "AskPrice2", "AskPrice3", "AskPrice4",
                    "AskPrice5", "AskPrice6", "AskPrice7", "AskPrice8", "AskPrice9",
                    "BidVolume0", "BidVolume1", "BidVolume2", "BidVolume3", "BidVolume4",
                    "BidVolume5", "BidVolume6", "BidVolume7", "BidVolume8", "BidVolume9",
                    "AskVolume0", "AskVolume1", "AskVolume2", "AskVolume3", "AskVolume4",
                    "AskVolume5", "AskVolume6", "AskVolume7", "AskVolume8", "AskVolume9",
                    "TradesNum", "InstrumentTradeStatus", "TotalBidVolume", "TotalAskVolume", "MaBidPrice",
                    "MaAskPrice", "MaBondBidPrice", "MaBondAskPrice", "YieldToMaturity", "IOPV",
                    "EtfBuycount", "EtfSellCount", "EtfBuyVolume", "EtfBuyBalance", "EtfSellVolume",
                    "EtfSellBalance", "TotalWarrantExecVolume", "WarrantLowerPrice", "WarrantUpperPrice", "CancelBuyNum",
                    "CancelSellNum", "CancelBuyVolume", "CancelSellVolume", "CancelBuyValue", "CancelSellValue",
                    "TotalBuyNum", "TotalSellNum", "DurationAfterBuy", "DurationAfterSell", "BidOrdersNum",
                    "AskOrdersNum", "PreIOPV"
            },
            {
                    DT_SYMBOL, DT_SYMBOL, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE,
                    DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE,
                    DT_DATE, DT_TIME, DT_LONG, DT_DOUBLE, DT_DOUBLE,
                    DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE,
                    DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE,
                    DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE,
                    DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE,
                    DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
                    DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
                    DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
                    DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
                    DT_LONG, DT_CHAR, DT_LONG, DT_LONG, DT_DOUBLE,
                    DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE,
                    DT_INT, DT_INT, DT_LONG, DT_DOUBLE, DT_LONG,
                    DT_DOUBLE, DT_LONG, DT_DOUBLE, DT_DOUBLE, DT_INT,
                    DT_INT, DT_LONG, DT_LONG, DT_DOUBLE, DT_DOUBLE,
                    DT_INT, DT_INT, DT_INT, DT_INT, DT_INT,
                    DT_INT, DT_DOUBLE
            }
    };

    extern const MetaTable snapshotMetaExtra_220105 = {
        {
            "ChannelNo", "BondLastAuctionPrice", "BondAuctionVolume","BondAuctionBalance", "BondLastTradeType",
            "BondTradeStatus",
            "BidNumOrders1", "BidNumOrders2", "BidNumOrders3", "BidNumOrders4", "BidNumOrders5",
            "BidNumOrders6", "BidNumOrders7", "BidNumOrders8", "BidNumOrders9", "BidNumOrders10",
            "AskNumOrders1", "AskNumOrders2", "AskNumOrders3", "AskNumOrders4", "AskNumOrders5",
            "AskNumOrders6", "AskNumOrders7", "AskNumOrders8", "AskNumOrders9", "AskNumOrders10",
        },
        {
            DT_INT, DT_DOUBLE, DT_LONG, DT_DOUBLE, DT_CHAR,
            DT_SYMBOL,
            DT_INT, DT_INT, DT_INT, DT_INT, DT_INT,
            DT_INT, DT_INT, DT_INT, DT_INT, DT_INT,
            DT_INT, DT_INT, DT_INT, DT_INT, DT_INT,
            DT_INT, DT_INT, DT_INT, DT_INT, DT_INT,
        }
    };
    const MetaTable snapshotMetaExtra = {
            {
                    "Bid1Count", "MaxBid1Count", "Ask1Count", "MaxAsk1Count",
                    "Bid1Qty0", "Bid1Qty1", "Bid1Qty2", "Bid1Qty3", "Bid1Qty4",
                    "Bid1Qty5", "Bid1Qty6", "Bid1Qty7", "Bid1Qty8", "Bid1Qty9",
                    "Bid1Qty10", "Bid1Qty11", "Bid1Qty12", "Bid1Qty13", "Bid1Qty14",
                    "Bid1Qty15", "Bid1Qty16", "Bid1Qty17", "Bid1Qty18", "Bid1Qty19",
                    "Bid1Qty20", "Bid1Qty21", "Bid1Qty22", "Bid1Qty23", "Bid1Qty24",
                    "Bid1Qty25", "Bid1Qty26", "Bid1Qty27", "Bid1Qty28", "Bid1Qty29",
                    "Bid1Qty30", "Bid1Qty31", "Bid1Qty32", "Bid1Qty33", "Bid1Qty34",
                    "Bid1Qty35", "Bid1Qty36", "Bid1Qty37", "Bid1Qty38", "Bid1Qty39",
                    "Bid1Qty40", "Bid1Qty41", "Bid1Qty42", "Bid1Qty43", "Bid1Qty44",
                    "Bid1Qty45", "Bid1Qty46", "Bid1Qty47", "Bid1Qty48", "Bid1Qty49",
                    "Ask1Qty0", "Ask1Qty1", "Ask1Qty2", "Ask1Qty3", "Ask1Qty4",
                    "Ask1Qty5", "Ask1Qty6", "Ask1Qty7", "Ask1Qty8", "Ask1Qty9",
                    "Ask1Qty10", "Ask1Qty11", "Ask1Qty12", "Ask1Qty13", "Ask1Qty14",
                    "Ask1Qty15", "Ask1Qty16", "Ask1Qty17", "Ask1Qty18", "Ask1Qty19",
                    "Ask1Qty20", "Ask1Qty21", "Ask1Qty22", "Ask1Qty23", "Ask1Qty24",
                    "Ask1Qty25", "Ask1Qty26", "Ask1Qty27", "Ask1Qty28", "Ask1Qty29",
                    "Ask1Qty30", "Ask1Qty31", "Ask1Qty32", "Ask1Qty33", "Ask1Qty34",
                    "Ask1Qty35", "Ask1Qty36", "Ask1Qty37", "Ask1Qty38", "Ask1Qty39",
                    "Ask1Qty40", "Ask1Qty41", "Ask1Qty42", "Ask1Qty43", "Ask1Qty44",
                    "Ask1Qty45", "Ask1Qty46", "Ask1Qty47", "Ask1Qty48", "Ask1Qty49"
            },
            {
                    DT_INT, DT_INT, DT_INT, DT_INT,
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
                    DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
                    DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
                    DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
                    DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
                    DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
                    DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
                    DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
                    DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
                    DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG
            }
    };

    const MetaTable tradeEntrustMeta {
            {
                    "SecurityID", "MDDate", "MDTime", "SecurityIDSource", "SecurityType",
                    "Index", "SourceType", "Type", "Price", "Qty",
                    "BSFlag", "BuyNo", "SellNo", "ApplSeqNum", "ChannelNo"
            },
            {
                    DT_SYMBOL, DT_DATE, DT_TIME, DT_SYMBOL, DT_SYMBOL,
                    DT_LONG, DT_INT, DT_INT, DT_LONG, DT_LONG,
                    DT_INT, DT_LONG, DT_LONG, DT_LONG, DT_INT,
            }
    };

} // namespace nsqUtil
