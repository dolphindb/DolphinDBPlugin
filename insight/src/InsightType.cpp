#include "InsightType.h"

#include <string>

#include "Exceptions.h"
#include "ddbplugin/ThreadedQueue.h"

using namespace ThreadedQueueUtil;

DATA_TYPE ARRAY_LONG_TYPE = DATA_TYPE(DT_LONG + ARRAY_TYPE_BASE);
DATA_TYPE ARRAY_INT_TYPE = DATA_TYPE(DT_INT + ARRAY_TYPE_BASE);

static int countDays(int day) {
    int year = day / 10000;
    day %= 10000;
    int month = day / 100;
    day %= 100;
    return Util::countDays(year, month, day);
}

static int countTime(int fake) {
    int real = 0;
    real += 3600000 * (fake / 10000000);
    fake %= 10000000;
    real += 60000 * (fake / 100000);
    fake %= 100000;
    real += fake;
    return real;
}
namespace OperatorImp {
ConstantSP temporalParse(const ConstantSP &a, const ConstantSP &b);
}

static int parseDays(const string &day) {
    try {
        return OperatorImp::temporalParse(new String(day), new String("yyyyMMdd"))->getInt();
    } catch (...) {
        LOG_WARN(PLUGIN_INSIGHT_PREFIX, "parse day '" + day + "' failed.");
        return INT_MIN;
    }
}
static int parseTime(const string &time) {
    try {
        return OperatorImp::temporalParse(new String(time), new String("HHmmssSSS"))->getInt();
    } catch (...) {
        LOG_WARN(PLUGIN_INSIGHT_PREFIX, "parse day '" + time + "' failed.");
        return INT_MIN;
    }
}

static long long countTimestamp(long long timestamp) {
    if (timestamp == 0) {
        return LONG_LONG_MIN;
    }
    int days = countDays(timestamp / 1000000000);
    int time = countTime(timestamp % 1000000000);
    return days * 24 * 3600 * 1000 + time;
}

MetaTable InsightIndexTickMeta {
    {
        "HTSCSecurityID", "MDDate", "MDTime", "DataTimestamp", "TradingPhaseCode",
        "securityIDSource", "securityType", "MaxPx", "MinPx", "PreClosePx",
        "NumTrades", "TotalVolumeTrade", "TotalValueTrade", "LastPx", "OpenPx",
        "ClosePx", "HighPx", "LowPx", "ChannelNo", "ExchangeDate",
        "ExchangeTime", "TotalBuyVolumeTrade", "TotalBuyValueTrade", "TotalBuyNumber", "TotalSellVolumeTrade",
        "TotalSellValueTrade", "TotalSellNumber", "DataMultiplePowerOf10"
    },
    {
        DT_SYMBOL, DT_DATE, DT_TIME, DT_TIMESTAMP, DT_SYMBOL,
        DT_SYMBOL, DT_SYMBOL, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_INT, DT_DATE,
        DT_TIME, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_INT
    }
};

MetaTable InsightStockTickMeta {
    {
        "HTSCSecurityID", "MDDate", "MDTime", "DataTimestamp", "TradingPhaseCode",
        "securityIDSource", "securityType", "MaxPx", "MinPx", "PreClosePx",
        "NumTrades", "TotalVolumeTrade", "TotalValueTrade", "LastPx", "OpenPx",
        "ClosePx", "HighPx", "LowPx", "DiffPx1", "DiffPx2",
        "TotalBuyQty", "TotalSellQty", "WeightedAvgBuyPx", "WeightedAvgSellPx", "WithdrawBuyNumber",
        "WithdrawBuyAmount", "WithdrawBuyMoney", "WithdrawSellNumber", "WithdrawSellAmount", "WithdrawSellMoney",
        "TotalBuyNumber", "TotalSellNumber", "BuyTradeMaxDuration", "SellTradeMaxDuration", "NumBuyOrders",
        "NumSellOrders", "NorminalPx", "ShortSellSharesTraded", "ShortSellTurnover", "ReferencePx",
        "ComplexEventStartTime", "ComplexEventEndTime", "ExchangeDate", "ExchangeTime", "AfterHoursNumTrades",
        "AfterHoursTotalVolumeTrade", "AfterHoursTotalValueTrade", "ChannelNo", "BuyPriceQueue", "BuyOrderQtyQueue",
        "SellPriceQueue", "SellOrderQtyQueue", "BuyOrderQueue", "SellOrderQueue", "BuyNumOrdersQueue",
        "SellNumOrdersQueue", "MaxBuyPrice", "MinBuyPrice", "MaxSellPrice", "MinSellPrice",
        "PreMarketLastPx", "PreMarketTotalVolumeTrade", "PreMarketTotalValueTrade", "PreMarketHighPx", "PreMarketLowPx",
        "AfterHoursLastPx", "AfterHoursHighPx", "AfterHoursLowPx", "MarketPhaseCode", "USConsolidateVolume",
        "USCompositeClosePx", "TradingHaltReason", "OtcTotalVolumeTrade", "OtcTotalValueTrade", "OtcNumTrades",
        "DataMultiplePowerOf10"
    },

    {
        DT_SYMBOL, DT_DATE, DT_TIME, DT_TIMESTAMP, DT_SYMBOL,
        DT_SYMBOL, DT_SYMBOL, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_INT,
        DT_INT, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_TIMESTAMP, DT_TIMESTAMP, DT_DATE, DT_TIME, DT_LONG,
        DT_LONG, DT_LONG, DT_INT, ARRAY_LONG_TYPE,ARRAY_LONG_TYPE,
        ARRAY_LONG_TYPE, ARRAY_LONG_TYPE, ARRAY_LONG_TYPE, ARRAY_LONG_TYPE, ARRAY_LONG_TYPE,
        ARRAY_LONG_TYPE, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_SYMBOL, DT_LONG,
        DT_LONG, DT_SYMBOL, DT_LONG, DT_LONG, DT_LONG,
        DT_INT
    }
};
MetaTable InsightFundTickMeta {
    {
        "HTSCSecurityID", "MDDate", "MDTime", "DataTimestamp", "TradingPhaseCode",
        "securityIDSource", "securityType", "MaxPx", "MinPx", "PreClosePx",
        "NumTrades", "TotalVolumeTrade", "TotalValueTrade", "LastPx", "OpenPx",
        "ClosePx", "HighPx", "LowPx", "DiffPx1", "DiffPx2",
        "TotalBuyQty", "TotalSellQty", "WeightedAvgBuyPx", "WeightedAvgSellPx", "WithdrawBuyNumber",
        "WithdrawBuyAmount", "WithdrawBuyMoney", "WithdrawSellNumber", "WithdrawSellAmount", "WithdrawSellMoney",
        "TotalBuyNumber", "TotalSellNumber", "BuyTradeMaxDuration", "SellTradeMaxDuration", "NumBuyOrders",
        "NumSellOrders",

        "IOPV", "PreIOPV", "PurchaseNumber", "PurchaseAmount", "PurchaseMoney",
        "RedemptionNumber", "RedemptionAmount", "RedemptionMoney", "ExchangeDate", "ExchangeTime",
        "ChannelNo",

        "BuyPriceQueue", "BuyOrderQtyQueue", "SellPriceQueue", "SellOrderQtyQueue", "BuyOrderQueue",
        "SellOrderQueue", "BuyNumOrdersQueue", "SellNumOrdersQueue",

        "NorminalPx", "ShortSellSharesTraded", "ShortSellTurnover",
        "PreMarketLastPx", "PreMarketTotalVolumeTrade", "PreMarketTotalValueTrade", "PreMarketHighPx", "PreMarketLowPx",
        "AfterHoursLastPx","AfterHoursTotalVolumeTrade", "AfterHoursTotalValueTrade","AfterHoursHighPx", "AfterHoursLowPx",
        "MarketPhaseCode", "USConsolidateVolume",
        "USCompositeClosePx", "TradingHaltReason", "OtcTotalVolumeTrade", "OtcTotalValueTrade", "OtcNumTrades",
        "DataMultiplePowerOf10",

        "WeightedAvgPx", "PreCloseWeightedAvgPx", "BestBuyPrice", "QtyAtBestBuyPrice", "BestSellPrice",
        "QtyAtBestSellPrice", "HighAccuracyIOPV", "HighAccuracyPreIOPV",
    },

    {
        DT_SYMBOL, DT_DATE, DT_TIME, DT_TIMESTAMP, DT_SYMBOL,
        DT_SYMBOL, DT_SYMBOL, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_INT,
        DT_LONG,

        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_INT, DT_INT,
        DT_INT,

        ARRAY_LONG_TYPE, ARRAY_LONG_TYPE, ARRAY_LONG_TYPE, ARRAY_LONG_TYPE, ARRAY_LONG_TYPE,
        ARRAY_LONG_TYPE, ARRAY_LONG_TYPE, ARRAY_LONG_TYPE,

        DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_SYMBOL, DT_LONG,
        DT_LONG, DT_SYMBOL, DT_LONG, DT_LONG, DT_LONG,
        DT_INT,

        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG,
    }
};

MetaTable InsightBondTickMeta {
    {
        "HTSCSecurityID", "MDDate", "MDTime", "DataTimestamp", "TradingPhaseCode",
        "securityIDSource", "securityType", "MaxPx", "MinPx", "PreClosePx",
        "NumTrades", "TotalVolumeTrade", "TotalValueTrade", "LastPx", "OpenPx",
        "ClosePx", "HighPx", "LowPx", "DiffPx1", "DiffPx2",
        "TotalBuyQty", "TotalSellQty", "WeightedAvgBuyPx", "WeightedAvgSellPx", "WithdrawBuyNumber",
        "WithdrawBuyAmount", "WithdrawBuyMoney", "WithdrawSellNumber", "WithdrawSellAmount", "WithdrawSellMoney",
        "TotalBuyNumber", "TotalSellNumber", "BuyTradeMaxDuration", "SellTradeMaxDuration", "NumBuyOrders",
        "NumSellOrders",

        "YieldToMaturity", "WeightedAvgPx", "WeightedAvgPxBP", "PreCloseWeightedAvgPx", "ExchangeDate",
        "ExchangeTime", "PreCloseYield", "PreWeightedAvgYield", "OpenYield", "HighYield",
        "LowYield", "LastYield", "WeightedAvgYield", "ChannelNo",

        "BuyPriceQueue", "BuyOrderQtyQueue", "SellPriceQueue", "SellOrderQtyQueue", "BuyOrderQueue",
        "SellOrderQueue", "BuyNumOrdersQueue", "SellNumOrdersQueue",

        "NorminalPx", "ShortSellSharesTraded", "ShortSellTurnover",

        "BuySettlTypeQueue", "SellSettlTypeQueue", "BuyYieldQueue", "SellYieldQueue",

        "PreMarketLastPx", "PreMarketTotalVolumeTrade", "PreMarketTotalValueTrade", "PreMarketHighPx", "PreMarketLowPx",
        "AfterHoursLastPx", "AfterHoursTotalVolumeTrade", "AfterHoursTotalValueTrade", "AfterHoursHighPx", "AfterHoursLowPx",
        "MarketPhaseCode", "SubTradingPhaseCode1", "SubTradingPhaseCode2", "SubTradingPhaseCode3", "SubTradingPhaseCode4",
        "SubTradingPhaseCode5", "LastPxType", "AuctionLastPx", "AuctionVolumeTrade", "AuctionValueTrade",
        "USConsolidateVolume", "USCompositeClosePx", "TradingHaltReason", "OtcTotalVolumeTrade", "OtcTotalValueTrade",
        "OtcNumTrades", "DataMultiplePowerOf10",
    },

    {
        DT_SYMBOL, DT_DATE, DT_TIME, DT_TIMESTAMP, DT_SYMBOL,
        DT_SYMBOL, DT_SYMBOL, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_INT,
        DT_LONG,

        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_DATE,
        DT_TIME, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_INT,

        ARRAY_LONG_TYPE, ARRAY_LONG_TYPE, ARRAY_LONG_TYPE, ARRAY_LONG_TYPE, ARRAY_LONG_TYPE,
        ARRAY_LONG_TYPE, ARRAY_LONG_TYPE, ARRAY_LONG_TYPE,

        DT_LONG, DT_LONG, DT_LONG,

        ARRAY_INT_TYPE, ARRAY_INT_TYPE, ARRAY_LONG_TYPE, ARRAY_LONG_TYPE,

        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_SYMBOL, DT_SYMBOL, DT_SYMBOL, DT_SYMBOL, DT_SYMBOL,
        DT_SYMBOL, DT_INT, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_SYMBOL, DT_LONG, DT_LONG,
        DT_LONG, DT_INT,
    }
};

MetaTable InsightOptionTickMeta {
    {
        "HTSCSecurityID", "MDDate", "MDTime", "DataTimestamp", "TradingPhaseCode",
        "securityIDSource", "securityType", "MaxPx", "MinPx", "PreClosePx",
        "NumTrades", "TotalVolumeTrade", "TotalValueTrade", "LastPx", "OpenPx",
        "ClosePx", "HighPx", "LowPx", "DiffPx1", "DiffPx2",
        "TotalBuyQty", "TotalSellQty", "WeightedAvgBuyPx", "WeightedAvgSellPx", "WithdrawBuyNumber",
        "WithdrawBuyAmount", "WithdrawBuyMoney", "WithdrawSellNumber", "WithdrawSellAmount", "WithdrawSellMoney",
        "TotalBuyNumber", "TotalSellNumber", "BuyTradeMaxDuration", "SellTradeMaxDuration", "NumBuyOrders",
        "NumSellOrders",

        "TradingDate", "PreOpenInterest", "PreSettlePrice", "OpenInterest", "SettlePrice",
        "PreDelta", "CurrDelta", "ExchangeDate", "ExchangeTime", "ReferencePrice",
        "ChannelNo",

        "BuyPriceQueue", "BuyOrderQtyQueue", "SellPriceQueue", "SellOrderQtyQueue", "BuyOrderQueue",
        "SellOrderQueue", "BuyNumOrdersQueue", "SellNumOrdersQueue",

        "DataMultiplePowerOf10",

    },

    {
        DT_SYMBOL, DT_DATE, DT_TIME, DT_TIMESTAMP, DT_SYMBOL,
        DT_SYMBOL, DT_SYMBOL, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_INT,
        DT_INT,
        DT_DATE, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_DATE, DT_TIME, DT_LONG,
        DT_INT,
        ARRAY_LONG_TYPE, ARRAY_LONG_TYPE, ARRAY_LONG_TYPE, ARRAY_LONG_TYPE, ARRAY_LONG_TYPE,
        ARRAY_LONG_TYPE, ARRAY_LONG_TYPE, ARRAY_LONG_TYPE,
        DT_INT,
    }
};

MetaTable InsightFutureTickMeta {
    {
        "HTSCSecurityID", "MDDate", "MDTime", "DataTimestamp", "TradingPhaseCode",
        "securityIDSource", "securityType", "MaxPx", "MinPx", "PreClosePx",
        "NumTrades", "TotalVolumeTrade", "TotalValueTrade", "LastPx", "OpenPx",
        "ClosePx", "HighPx", "LowPx",
        "TradingDate", "PreOpenInterest", "PreSettlePrice", "OpenInterest", "SettlePrice",
        "PreDelta", "CurrDelta", "MiddlePx", "ImpliedBuyPx", "ImpliedBuyQty",
        "ImpliedSellPx", "ImpliedSellQty", "PositionTrend", "ChangeSpeed", "ChangeRate",
        "ChangeValue", "Swing", "CommodityContractNumber", "ExchangeDate", "ExchangeTime",
        "ChannelNo",
        "BuyPriceQueue", "BuyOrderQtyQueue", "SellPriceQueue", "SellOrderQtyQueue", "BuyOrderQueue",
        "SellOrderQueue", "BuyNumOrdersQueue", "SellNumOrdersQueue",
        "DataMultiplePowerOf10",
    },

    {
        DT_SYMBOL, DT_DATE, DT_TIME, DT_TIMESTAMP, DT_SYMBOL,
        DT_SYMBOL, DT_SYMBOL, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG,
        DT_DATE, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_STRING, DT_DATE, DT_TIME,
        DT_INT,
        ARRAY_LONG_TYPE, ARRAY_LONG_TYPE, ARRAY_LONG_TYPE, ARRAY_LONG_TYPE, ARRAY_LONG_TYPE,
        ARRAY_LONG_TYPE, ARRAY_LONG_TYPE, ARRAY_LONG_TYPE,
        DT_INT,
    }
};
MetaTable InsightTransactionMeta {
    {
        "HTSCSecurityID", "MDDate", "MDTime", "DataTimestamp", "securityIDSource",
        "securityType", "TradeIndex", "TradeBuyNo", "TradeSellNo", "TradeType",
        "TradeBSFlag", "TradePrice", "TradeQty", "TradeMoney", "ApplSeqNum",
        "ChannelNo", "ExchangeDate", "ExchangeTime", "TradeCleanPrice", "AccruedInterestAmt",
        "TradeDirtyPrice", "MaturityYield", "FITradingMethod", "SettlPeriod", "SettlType",
        "SecondaryOrderID", "BidExecInstType", "MarginPrice", "HKTradeType", "DataMultiplePowerOf10",
        "DealDate", "DealTime", "DealNumber", "MarketIndicator", "RepoTerm",
        "LegSettlementAmount1st", "LegSettlementAmount2nd", "BondCode", "BondName", "TotalFacevalue",
        "LegCleanPrice1st", "LegCleanPrice2nd", "LegYield1st", "LegYield2nd",
    },

    {
        DT_SYMBOL, DT_DATE, DT_TIME, DT_TIMESTAMP, DT_SYMBOL,
        DT_SYMBOL, DT_LONG, DT_LONG, DT_LONG, DT_INT,
        DT_INT, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_INT, DT_DATE, DT_TIME, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_SYMBOL, DT_INT, DT_INT,
        DT_STRING, DT_INT, DT_LONG, DT_INT, DT_INT,
        DT_DATE, DT_TIME, DT_STRING, DT_INT, DT_INT,
        DT_LONG, DT_LONG, DT_SYMBOL, DT_SYMBOL, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG,

    }
};
MetaTable InsightOrderMeta {
    {
        "HTSCSecurityID", "MDDate", "MDTime", "DataTimestamp", "securityIDSource",
        "securityType", "OrderIndex", "OrderType", "OrderPrice", "OrderQty",
        "OrderBSFlag", "ChannelNo", "ExchangeDate", "ExchangeTime", "OrderNO",
        "ApplSeqNum", "SecurityStatus", "QuoteID", "MemberID", "InvestorType",
        "InvestorID", "InvestorName", "TraderCode", "SettlPeriod", "SettlType",
        "Memo", "SecondaryOrderID", "BidTransType", "BidExecInstType", "LowLimitPrice",
        "HighLimitPrice", "MinQty", "TradeDate", "DataMultiplePowerOf10"
    },

    {
        DT_SYMBOL, DT_DATE, DT_TIME, DT_TIMESTAMP, DT_SYMBOL,
        DT_SYMBOL, DT_LONG, DT_INT, DT_LONG, DT_LONG,
        DT_INT, DT_INT, DT_INT, DT_INT, DT_LONG,
        DT_LONG, DT_SYMBOL, DT_STRING, DT_SYMBOL, DT_SYMBOL,
        DT_SYMBOL, DT_SYMBOL, DT_STRING, DT_INT, DT_INT,
        DT_STRING, DT_STRING, DT_INT, DT_INT, DT_LONG,
        DT_LONG, DT_LONG, DT_STRING, DT_INT
    }
};

MetaTable InsightOrderMeta_3_2_11 {
    {
        "HTSCSecurityID", "MDDate", "MDTime", "DataTimestamp", "securityIDSource",
        "securityType", "OrderIndex", "OrderType", "OrderPrice", "OrderQty",
        "OrderBSFlag", "ChannelNo", "ExchangeDate", "ExchangeTime", "OrderNO",
        "ApplSeqNum", "SecurityStatus", "QuoteID", "MemberID", "InvestorType",
        "InvestorID", "InvestorName", "TraderCode", "SettlPeriod", "SettlType",
        "Memo", "SecondaryOrderID", "BidTransType", "BidExecInstType", "LowLimitPrice",
        "HighLimitPrice", "MinQty", "TradeDate", "DataMultiplePowerOf10", "TradedQty"
    },

    {
        DT_SYMBOL, DT_DATE, DT_TIME, DT_TIMESTAMP, DT_SYMBOL,
        DT_SYMBOL, DT_LONG, DT_INT, DT_LONG, DT_LONG,
        DT_INT, DT_INT, DT_INT, DT_INT, DT_LONG,
        DT_LONG, DT_SYMBOL, DT_STRING, DT_SYMBOL, DT_SYMBOL,
        DT_SYMBOL, DT_SYMBOL, DT_STRING, DT_INT, DT_INT,
        DT_STRING, DT_STRING, DT_INT, DT_INT, DT_LONG,
        DT_LONG, DT_LONG, DT_STRING, DT_INT, DT_LONG
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

DATA_TYPE INT_ARRAY_TYPE = DATA_TYPE(DT_INT + ARRAY_TYPE_BASE);
DATA_TYPE LONG_ARRAY_TYPE = DATA_TYPE(DT_LONG + ARRAY_TYPE_BASE);
DATA_TYPE BOOL_ARRAY_TYPE = DATA_TYPE(DT_BOOL + ARRAY_TYPE_BASE);

MetaTable InsightMDSecurityLendingMeta {
    // TODO
    {
        "HTSCSecurityID", "MDDate", "MDTime", "DataTimestamp", "TradingPhaseCode",
        "securityIDSource", "securityType", "PreWeightedRate", "PreHighRate", "PreLowRate",
        "PreHtscVolume", "PreMarketVolume", "WeightedRate", "HighRate", "LowRate",
        "HtscVolume", "MarketVolume", "BestBorrowRate", "BestLendRate",
        "ValidBorrows_Level", "ValidBorrows_Rate", "ValidBorrows_Term", "ValidBorrows_Amount", "ValidBorrows_HtscProvided",
        "ValidALends_Level", "ValidALends_Rate", "ValidALends_Term", "ValidALends_Amount", "ValidALends_HtscProvided",
        "ValidBLends_Level", "ValidBLends_Rate", "ValidBLends_Term", "ValidBLends_Amount", "ValidBLends_HtscProvided",
        "ValidCLends_Level", "ValidCLends_Rate", "ValidCLends_Term", "ValidCLends_Amount", "ValidCLends_HtscProvided",
        "ALends_Level", "ALends_Rate", "ALends_Term", "ALends_TotalAmount", "ALends_MatchedAmount",
        "BLends_Level", "BLends_Rate", "BLends_Term", "BLends_TotalAmount", "BLends_MatchedAmount",
        "CLends_Level", "CLends_Rate", "CLends_Term", "CLends_TotalAmount", "CLends_MatchedAmount",
        "ValidReservationBorrows_Level", "ValidReservationBorrows_Rate", "ValidReservationBorrows_Term", "ValidReservationBorrows_Amount", "ValidReservationBorrows_HtscProvided",
        "ValidReservationLends_Level", "ValidReservationLends_Rate", "ValidReservationLends_Term", "ValidReservationLends_Amount", "ValidReservationLends_HtscProvided",
        "ReservationBorrows_Level", "ReservationBorrows_Rate", "ReservationBorrows_Term", "ReservationBorrows_TotalAmount", "ReservationBorrows_MatchedAmount",
        "ReservationLends_Level", "ReservationLends_Rate", "ReservationLends_Term", "ReservationLends_TotalAmount", "ReservationLends_MatchedAmount",
        "ValidOtcLends_Level", "ValidOtcLends_Rate", "ValidOtcLends_Term", "ValidOtcLends_Amount", "ValidOtcLends_HtscProvided",
        "BestReservationBorrowRate", "BestReservationLendRate", "ValidLendAmount", "ValidALendAmount", "ValidBLendAmount",
        "HtscBorrowAmount", "HtscBorrowRate", "BestLoanRate", "HtscBorrowTradeVolume", "HtscBorrowWeightedRate",
        "PreHtscBorrowTradeVolume", "PreHtscBorrowWeightedRate",
        "HtscBorrows_Level", "HtscBorrows_Rate", "HtscBorrows_Term", "HtscBorrows_Amount", "HtscBorrows_HtscProvided",
        "Loans_Level", "Loans_Rate", "Loans_Term", "Loans_Amount", "Loans_HtscProvided",
        "DataMultiplePowerOf10",
        "ExternalLends_Level", "ExternalLends_Rate", "ExternalLends_Term", "ExternalLends_Amount", "ExternalLends_PostponeProbability",
        "HtscBorrowTerm", "LoanAmount",

        "MarketBorrows_Level", "MarketBorrows_Rate", "MarketBorrows_Term", "MarketBorrows_Amount", "MarketBorrows_HtscProvided",
        "ValidLendTerm", "ValidBorrowAmount",

        "MarketLends_Level", "MarketLends_Rate", "MarketLends_Term", "MarketLends_Amount", "MarketLends_HtscProvided",

    },
    {
        DT_SYMBOL, DT_DATE, DT_TIME, DT_TIMESTAMP, DT_SYMBOL,
        DT_SYMBOL, DT_SYMBOL, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        INT_ARRAY_TYPE, LONG_ARRAY_TYPE, INT_ARRAY_TYPE, LONG_ARRAY_TYPE, BOOL_ARRAY_TYPE,
        INT_ARRAY_TYPE, LONG_ARRAY_TYPE, INT_ARRAY_TYPE, LONG_ARRAY_TYPE, BOOL_ARRAY_TYPE,
        INT_ARRAY_TYPE, LONG_ARRAY_TYPE, INT_ARRAY_TYPE, LONG_ARRAY_TYPE, BOOL_ARRAY_TYPE,
        INT_ARRAY_TYPE, LONG_ARRAY_TYPE, INT_ARRAY_TYPE, LONG_ARRAY_TYPE, BOOL_ARRAY_TYPE,

        INT_ARRAY_TYPE, LONG_ARRAY_TYPE, INT_ARRAY_TYPE, LONG_ARRAY_TYPE, LONG_ARRAY_TYPE,
        INT_ARRAY_TYPE, LONG_ARRAY_TYPE, INT_ARRAY_TYPE, LONG_ARRAY_TYPE, LONG_ARRAY_TYPE,
        INT_ARRAY_TYPE, LONG_ARRAY_TYPE, INT_ARRAY_TYPE, LONG_ARRAY_TYPE, LONG_ARRAY_TYPE,

        INT_ARRAY_TYPE, LONG_ARRAY_TYPE, INT_ARRAY_TYPE, LONG_ARRAY_TYPE, BOOL_ARRAY_TYPE,
        INT_ARRAY_TYPE, LONG_ARRAY_TYPE, INT_ARRAY_TYPE, LONG_ARRAY_TYPE, BOOL_ARRAY_TYPE,
        INT_ARRAY_TYPE, LONG_ARRAY_TYPE, INT_ARRAY_TYPE, LONG_ARRAY_TYPE, LONG_ARRAY_TYPE,
        INT_ARRAY_TYPE, LONG_ARRAY_TYPE, INT_ARRAY_TYPE, LONG_ARRAY_TYPE, LONG_ARRAY_TYPE,
        INT_ARRAY_TYPE, LONG_ARRAY_TYPE, INT_ARRAY_TYPE, LONG_ARRAY_TYPE, BOOL_ARRAY_TYPE,

        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
        DT_LONG, DT_LONG,

        INT_ARRAY_TYPE, LONG_ARRAY_TYPE, INT_ARRAY_TYPE, LONG_ARRAY_TYPE, BOOL_ARRAY_TYPE,
        INT_ARRAY_TYPE, LONG_ARRAY_TYPE, INT_ARRAY_TYPE, LONG_ARRAY_TYPE, BOOL_ARRAY_TYPE,

        DT_INT,
        INT_ARRAY_TYPE, LONG_ARRAY_TYPE, DT_STRING, LONG_ARRAY_TYPE, INT_ARRAY_TYPE,

        DT_STRING, DT_LONG,
        INT_ARRAY_TYPE, LONG_ARRAY_TYPE, INT_ARRAY_TYPE, LONG_ARRAY_TYPE, BOOL_ARRAY_TYPE,
        DT_STRING, DT_LONG,
        INT_ARRAY_TYPE, LONG_ARRAY_TYPE, INT_ARRAY_TYPE, LONG_ARRAY_TYPE, BOOL_ARRAY_TYPE,
    }
};

void initTypeContainer(MarketTypeContainer &container, const string &dataVersion) {
    container.add("OrderTransaction", InsightOrderTransactionMeta);
    container.add("StockTick", InsightStockTickMeta);
    container.add("IndexTick", InsightIndexTickMeta);
    container.add("FuturesTick", InsightFutureTickMeta);
    container.add("Transaction", InsightTransactionMeta);
    if (dataVersion == "3.2.8") {
        container.add("Order", InsightOrderMeta);
    } else {  // only could be 3.2.11
        container.add("Order", InsightOrderMeta_3_2_11);
    }
    container.add("BondTick", InsightBondTickMeta);
    container.add("FundTick", InsightFundTickMeta);
    container.add("OptionTick", InsightOptionTickMeta);
    container.add("SecurityLending", InsightMDSecurityLendingMeta);
}

template <typename T>
void buySellQueueAppender(ConstantVecIterator &iter, TimedWrapper<T> &data) {
    VectorSP tuple = Util::createVector(DT_ANY, 1);
    VectorSP tupleBuffer = Util::createVector(DT_LONG, 0, 10);

#define ARRAY_VECTOR_APPEND(func1, func2)         \
    tupleBuffer->clear();                         \
    for (int i = 0; i < data.data.func1(); ++i) { \
        long long element = data.data.func2(i);   \
        tupleBuffer->appendLong(&element, 1);     \
    }                                             \
    tuple->set(0, tupleBuffer);                   \
    getVec(iter++)->append(tuple);

    ARRAY_VECTOR_APPEND(buypricequeue_size, buypricequeue)
    ARRAY_VECTOR_APPEND(buyorderqtyqueue_size, buyorderqtyqueue)
    ARRAY_VECTOR_APPEND(sellpricequeue_size, sellpricequeue)
    ARRAY_VECTOR_APPEND(sellorderqtyqueue_size, sellorderqtyqueue)
    ARRAY_VECTOR_APPEND(buyorderqueue_size, buyorderqueue)
    ARRAY_VECTOR_APPEND(sellorderqueue_size, sellorderqueue)
    ARRAY_VECTOR_APPEND(buynumordersqueue_size, buynumordersqueue)
    ARRAY_VECTOR_APPEND(sellnumordersqueue_size, sellnumordersqueue)

#undef ARRAY_VECTOR_APPEND
}

template <typename T>
void snapshotHeadAppender1(ConstantVecIterator &iter, TimedWrapper<T> &data) {
    appendString(iter++, data.data.htscsecurityid());
    appendInt(iter++, countDays(data.data.mddate()));
    appendInt(iter++, countTime(data.data.mdtime()));
    appendLong(iter++, countTimestamp(data.data.datatimestamp()));
    appendString(iter++, data.data.tradingphasecode());

    appendString(iter++, com::htsc::mdc::model::ESecurityIDSource_Name(data.data.securityidsource()));
    appendString(iter++, com::htsc::mdc::model::ESecurityType_Name(data.data.securitytype()));
    appendLong(iter++, data.data.maxpx());
    appendLong(iter++, data.data.minpx());
    appendLong(iter++, data.data.preclosepx());

    appendLong(iter++, data.data.numtrades());
    appendLong(iter++, data.data.totalvolumetrade());
    appendLong(iter++, data.data.totalvaluetrade());
    appendLong(iter++, data.data.lastpx());
    appendLong(iter++, data.data.openpx());

    appendLong(iter++, data.data.closepx());
    appendLong(iter++, data.data.highpx());
    appendLong(iter++, data.data.lowpx());
}
template <typename T>
void snapshotHeadAppender2(ConstantVecIterator &iter, TimedWrapper<T> &data) {
    snapshotHeadAppender1(iter, data);
    appendLong(iter++, data.data.diffpx1());
    appendLong(iter++, data.data.diffpx2());

    appendLong(iter++, data.data.totalbuyqty());
    appendLong(iter++, data.data.totalsellqty());
    appendLong(iter++, data.data.weightedavgbuypx());
    appendLong(iter++, data.data.weightedavgsellpx());
    appendLong(iter++, data.data.withdrawbuynumber());

    appendLong(iter++, data.data.withdrawbuyamount());
    appendLong(iter++, data.data.withdrawbuymoney());
    appendLong(iter++, data.data.withdrawsellnumber());
    appendLong(iter++, data.data.withdrawsellamount());
    appendLong(iter++, data.data.withdrawsellmoney());

    appendLong(iter++, data.data.totalbuynumber());
    appendLong(iter++, data.data.totalsellnumber());
    appendLong(iter++, data.data.buytrademaxduration());
    appendLong(iter++, data.data.selltrademaxduration());
    appendInt(iter++, data.data.numbuyorders());

    appendInt(iter++, data.data.numsellorders());
}

void iterOrder_3_2_8(ConstantVecIterator &iter, TimedWrapper<MDOrder> &data) {
    appendString(iter++, data.data.htscsecurityid());
    appendInt(iter++, countDays(data.data.mddate()));
    appendInt(iter++, countTime(data.data.mdtime()));
    appendLong(iter++, countTimestamp(data.data.datatimestamp()));
    appendString(iter++, com::htsc::mdc::model::ESecurityIDSource_Name(data.data.securityidsource()));

    appendString(iter++, com::htsc::mdc::model::ESecurityType_Name(data.data.securitytype()));
    appendLong(iter++, data.data.orderindex());
    appendInt(iter++, data.data.ordertype());
    appendLong(iter++, data.data.orderprice());
    appendLong(iter++, data.data.orderqty());

    appendInt(iter++, data.data.orderbsflag());
    appendInt(iter++, data.data.channelno());
    appendInt(iter++, countDays(data.data.exchangedate()));
    appendInt(iter++, countTime(data.data.exchangetime()));
    appendLong(iter++, data.data.orderno());

    appendLong(iter++, data.data.applseqnum());
    appendString(iter++, data.data.securitystatus());
    appendString(iter++, data.data.quoteid());
    appendString(iter++, data.data.memberid());
    appendString(iter++, data.data.investortype());

    appendString(iter++, data.data.investorid());
    appendString(iter++, data.data.investorname());
    appendString(iter++, data.data.tradercode());
    appendInt(iter++, data.data.settlperiod());
    appendInt(iter++, data.data.settltype());

    appendString(iter++, data.data.memo());
    appendString(iter++, data.data.secondaryorderid());
    appendInt(iter++, data.data.bidtranstype());
    appendInt(iter++, data.data.bidexecinsttype());
    appendLong(iter++, data.data.lowlimitprice());

    appendLong(iter++, data.data.highlimitprice());
    appendLong(iter++, data.data.minqty());
    appendString(iter++, data.data.tradedate());
    appendInt(iter++, data.data.datamultiplepowerof10());
}

void orderReader(vector<ConstantSP> &buffer, TimedWrapper<MDOrder> &data) {
    ConstantVecIterator iter = buffer.begin();
    iterOrder_3_2_8(iter, data);
}

void orderReader_3_2_11(vector<ConstantSP> &buffer, TimedWrapper<MDOrder> &data) {
    ConstantVecIterator iter = buffer.begin();
    iterOrder_3_2_8(iter, data);
    appendLong(iter, data.data.tradedqty()); // version 3.2.11 offer tradeQty
}

void transactionReader(vector<ConstantSP> &buffer, TimedWrapper<MDTransaction> &data) {
    ConstantVecIterator iter = buffer.begin();

    appendString(iter++, data.data.htscsecurityid());
    appendInt(iter++, countDays(data.data.mddate()));
    appendInt(iter++, countTime(data.data.mdtime()));
    appendLong(iter++, countTimestamp(data.data.datatimestamp()));
    appendString(iter++, com::htsc::mdc::model::ESecurityIDSource_Name(data.data.securityidsource()));

    appendString(iter++, com::htsc::mdc::model::ESecurityType_Name(data.data.securitytype()));
    appendLong(iter++, data.data.tradeindex());
    appendLong(iter++, data.data.tradebuyno());
    appendLong(iter++, data.data.tradesellno());
    appendInt(iter++, data.data.tradetype());

    appendInt(iter++, data.data.tradebsflag());
    appendLong(iter++, data.data.tradeprice());
    appendLong(iter++, data.data.tradeqty());
    appendLong(iter++, data.data.trademoney());
    appendLong(iter++, data.data.applseqnum());

    appendInt(iter++, data.data.channelno());
    appendInt(iter++, countDays(data.data.exchangedate()));
    appendInt(iter++, countTime(data.data.exchangetime()));
    appendLong(iter++, data.data.tradecleanprice());
    appendLong(iter++, data.data.accruedinterestamt());

    appendLong(iter++, data.data.tradedirtyprice());
    appendLong(iter++, data.data.maturityyield());
    appendString(iter++, data.data.fitradingmethod());
    appendInt(iter++, data.data.settlperiod());
    appendInt(iter++, data.data.settltype());

    appendString(iter++, data.data.secondaryorderid());
    appendInt(iter++, data.data.bidexecinsttype());
    appendLong(iter++, data.data.marginprice());
    appendInt(iter++, data.data.hktradetype());
    appendInt(iter++, data.data.datamultiplepowerof10());

    appendInt(iter++, parseDays(data.data.dealdate()));
    appendInt(iter++, parseTime(data.data.dealtime()));
    appendString(iter++, data.data.dealnumber());
    appendInt(iter++, data.data.marketindicator());
    appendInt(iter++, data.data.repoterm());

    appendLong(iter++, data.data.legsettlementamount1st());
    appendLong(iter++, data.data.legsettlementamount2nd());
    appendString(iter++, data.data.bondcode());
    appendString(iter++, data.data.bondname());
    appendLong(iter++, data.data.totalfacevalue());

    appendLong(iter++, data.data.legcleanprice1st());
    appendLong(iter++, data.data.legcleanprice2nd());
    appendLong(iter++, data.data.legyield1st());
    appendLong(iter++, data.data.legyield2nd());
}

void orderTransactionReader(vector<ConstantSP> &buffer, InsightOrderTransaction &data, int seqCheckMode,
                            std::unordered_map<int, long long> &szLastSeqNum,
                            std::unordered_map<int, long long> &shLastSeqNum) {
    if (!data.orderOrTransaction) {
        string securityidsource = com::htsc::mdc::model::ESecurityIDSource_Name(data.transaction.securityidsource());
        int channelno = data.transaction.channelno();
        long long applseqnum = data.transaction.applseqnum();
        long long tradeindex = data.transaction.tradeindex();
        if (securityidsource == "XSHE") {
            string tag = "applseqnum on " + securityidsource + " transaction";
            MarketUtil::checkSeqNum("", tag, szLastSeqNum, applseqnum, channelno, seqCheckMode);
        } else if (securityidsource == "XSHG") {
            string tag = "applseqnum on " + securityidsource + " transaction";
            MarketUtil::checkSeqNum("", tag, shLastSeqNum, applseqnum, channelno, seqCheckMode);
        } else {
            throw RuntimeException("wrong securityidsource: " + securityidsource);
        }
        string htscsecurityid = data.transaction.htscsecurityid();
        ((VectorSP)buffer[0])->appendString(&htscsecurityid, 1);
        int mddate = countDays(data.transaction.mddate());
        ((VectorSP)buffer[1])->appendInt(&mddate, 1);
        int mdtime = countTime(data.transaction.mdtime());
        ((VectorSP)buffer[2])->appendInt(&mdtime, 1);

        ((VectorSP)buffer[3])->appendString(&securityidsource, 1);
        string securitytype = com::htsc::mdc::model::ESecurityType_Name(data.transaction.securitytype());
        ((VectorSP)buffer[4])->appendString(&securitytype, 1);

        ((VectorSP)buffer[5])->appendLong(&tradeindex, 1);
        int type = MarketUtil::OrderBookMsgType::TRADE;
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
        int ordertype = data.order.ordertype();

        int type = MarketUtil::OrderBookMsgType::ORDER;
        if (ordertype == 11) {
            type = MarketUtil::OrderBookMsgType::PRODUCT_STATUS;
        }
        if (securityidsource == "XSHE") {
            string tag = "applseqnum on " + securityidsource + " order";
            MarketUtil::checkSeqNum("", tag, szLastSeqNum, applseqnum, channelno, seqCheckMode);
        } else if (securityidsource == "XSHG") {
            string tag = "applseqnum on " + securityidsource + " order";
            MarketUtil::checkSeqNum("", tag, shLastSeqNum, applseqnum, channelno, seqCheckMode);
        } else {
            throw RuntimeException("wrong securityidsource: " + securityidsource);
        }
        string htscsecurityid = data.order.htscsecurityid();
        ((VectorSP)buffer[0])->appendString(&htscsecurityid, 1);
        int mddate = countDays(data.order.mddate());
        ((VectorSP)buffer[1])->appendInt(&mddate, 1);
        int mdtime = countTime(data.order.mdtime());
        ((VectorSP)buffer[2])->appendInt(&mdtime, 1);
        ((VectorSP)buffer[3])->appendString(&securityidsource, 1);
        string securitytype = com::htsc::mdc::model::ESecurityType_Name(data.order.securitytype());
        ((VectorSP)buffer[4])->appendString(&securitytype, 1);
        ((VectorSP)buffer[5])->appendLong(&orderindex, 1);
        ((VectorSP)buffer[6])->appendInt(&type, 1);
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

void indexTickReader(vector<ConstantSP> &buffer, TimedWrapper<MDIndex> &data) {
    ConstantVecIterator iter = buffer.begin();
    snapshotHeadAppender1(iter, data);
    appendInt(iter++, data.data.channelno());
    appendInt(iter++, countDays(data.data.exchangedate()));

    appendInt(iter++, countTime(data.data.exchangetime()));
    appendLong(iter++, data.data.totalbuyvolumetrade());
    appendLong(iter++, data.data.totalbuyvaluetrade());
    appendLong(iter++, data.data.totalbuynumber());
    appendLong(iter++, data.data.totalsellvolumetrade());

    appendLong(iter++, data.data.totalsellvaluetrade());
    appendLong(iter++, data.data.totalsellnumber());
    appendLong(iter++, data.data.datamultiplepowerof10());
}

void futureTickReader(vector<ConstantSP> &buffer, TimedWrapper<MDFuture> &data) {
    ConstantVecIterator iter = buffer.begin();
    snapshotHeadAppender1(iter, data);

    appendInt(iter++, countDays(data.data.tradingdate()));
    appendLong(iter++, data.data.preopeninterest());
    appendLong(iter++, data.data.presettleprice());
    appendLong(iter++, data.data.openinterest());
    appendLong(iter++, data.data.settleprice());

    appendLong(iter++, data.data.predelta());
    appendLong(iter++, data.data.currdelta());
    appendLong(iter++, data.data.middlepx());
    appendLong(iter++, data.data.impliedbuypx());
    appendLong(iter++, data.data.impliedbuyqty());

    appendLong(iter++, data.data.impliedsellpx());
    appendLong(iter++, data.data.impliedsellqty());
    appendLong(iter++, data.data.positiontrend());
    appendLong(iter++, data.data.changespeed());
    appendLong(iter++, data.data.changerate());

    appendLong(iter++, data.data.changevalue());
    appendLong(iter++, data.data.swing());
    appendString(iter++, data.data.commoditycontractnumber());
    appendInt(iter++, countDays(data.data.exchangedate()));
    appendInt(iter++, countTime(data.data.exchangetime()));

    appendInt(iter++, data.data.channelno());

    buySellQueueAppender<MDFuture>(iter, data);

    appendInt(iter++, data.data.datamultiplepowerof10());
}

void stockTickReader(vector<ConstantSP> &buffer, TimedWrapper<MDStock> &data) {
    ConstantVecIterator iter = buffer.begin();
    snapshotHeadAppender2<MDStock>(iter, data);

    appendLong(iter++, data.data.norminalpx());
    appendLong(iter++, data.data.shortsellsharestraded());
    appendLong(iter++, data.data.shortsellturnover());
    appendLong(iter++, data.data.referencepx());

    appendLong(iter++, countTimestamp(data.data.complexeventstarttime()));
    appendLong(iter++, countTimestamp(data.data.complexeventendtime()));
    appendInt(iter++, countDays(data.data.exchangedate()));
    appendInt(iter++, countTime(data.data.exchangetime()));
    appendLong(iter++, data.data.afterhoursnumtrades());

    appendLong(iter++, data.data.afterhourstotalvolumetrade());
    appendLong(iter++, data.data.afterhourstotalvaluetrade());
    appendInt(iter++, data.data.channelno());

    buySellQueueAppender(iter, data);

    appendLong(iter++, data.data.maxbuyprice());
    appendLong(iter++, data.data.minbuyprice());
    appendLong(iter++, data.data.maxsellprice());
    appendLong(iter++, data.data.minsellprice());

    appendLong(iter++, data.data.premarketlastpx());
    appendLong(iter++, data.data.premarkettotalvolumetrade());
    appendLong(iter++, data.data.premarkettotalvaluetrade());
    appendLong(iter++, data.data.premarkethighpx());
    appendLong(iter++, data.data.premarketlowpx());

    appendLong(iter++, data.data.afterhourslastpx());
    appendLong(iter++, data.data.afterhourshighpx());
    appendLong(iter++, data.data.afterhourslowpx());
    appendString(iter++, data.data.marketphasecode());
    appendLong(iter++, data.data.usconsolidatevolume());

    appendLong(iter++, data.data.uscompositeclosepx());
    appendString(iter++, data.data.tradinghaltreason());
    appendLong(iter++, data.data.otctotalvolumetrade());
    appendLong(iter++, data.data.otctotalvaluetrade());
    appendLong(iter++, data.data.otcnumtrades());

    appendInt(iter++, data.data.datamultiplepowerof10());
}

void fundTickReader(vector<ConstantSP> &buffer, TimedWrapper<MDFund> &data) {
    ConstantVecIterator iter = buffer.begin();
    snapshotHeadAppender2<MDFund>(iter, data);

    appendLong(iter++, data.data.iopv());
    appendLong(iter++, data.data.preiopv());
    appendLong(iter++, data.data.purchasenumber());
    appendLong(iter++, data.data.purchaseamount());
    appendLong(iter++, data.data.purchasemoney());

    appendLong(iter++, data.data.redemptionnumber());
    appendLong(iter++, data.data.redemptionamount());
    appendLong(iter++, data.data.redemptionmoney());
    appendInt(iter++, countDays(data.data.exchangedate()));
    appendInt(iter++, countTime(data.data.exchangetime()));

    appendInt(iter++, data.data.channelno());

    buySellQueueAppender<MDFund>(iter, data);

    appendLong(iter++, data.data.norminalpx());
    appendLong(iter++, data.data.shortsellsharestraded());
    appendLong(iter++, data.data.shortsellturnover());

    appendLong(iter++, data.data.premarketlastpx());
    appendLong(iter++, data.data.premarkettotalvolumetrade());
    appendLong(iter++, data.data.premarkettotalvaluetrade());
    appendLong(iter++, data.data.premarkethighpx());
    appendLong(iter++, data.data.premarketlowpx());

    appendLong(iter++, data.data.afterhourslastpx());
    appendLong(iter++, data.data.afterhourstotalvolumetrade());
    appendLong(iter++, data.data.afterhourstotalvaluetrade());
    appendLong(iter++, data.data.afterhourshighpx());
    appendLong(iter++, data.data.afterhourslowpx());

    appendString(iter++, data.data.marketphasecode());
    appendLong(iter++, data.data.usconsolidatevolume());
    appendLong(iter++, data.data.uscompositeclosepx());
    appendString(iter++, data.data.tradinghaltreason());
    appendLong(iter++, data.data.otctotalvolumetrade());

    appendLong(iter++, data.data.otctotalvaluetrade());
    appendLong(iter++, data.data.otcnumtrades());
    appendInt(iter++, data.data.datamultiplepowerof10());

    appendLong(iter++, data.data.weightedavgbuypx());
    appendLong(iter++, data.data.precloseweightedavgpx());
    appendLong(iter++, data.data.bestbuyprice());
    appendLong(iter++, data.data.qtyatbestbuyprice());
    appendLong(iter++, data.data.bestsellprice());

    appendLong(iter++, data.data.qtyatbestsellprice());
    appendLong(iter++, data.data.highaccuracyiopv());
    appendLong(iter++, data.data.highaccuracypreiopv());
}

void bondTickReader(vector<ConstantSP> &buffer, TimedWrapper<MDBond> &data) {
    ConstantVecIterator iter = buffer.begin();
    snapshotHeadAppender2<MDBond>(iter, data);

    appendLong(iter++, data.data.yieldtomaturity());
    appendLong(iter++, data.data.weightedavgpx());
    appendLong(iter++, data.data.weightedavgpxbp());
    appendLong(iter++, data.data.precloseweightedavgpx());
    appendInt(iter++, countDays(data.data.exchangedate()));

    appendInt(iter++, countTime(data.data.exchangetime()));
    appendLong(iter++, data.data.precloseyield());
    appendLong(iter++, data.data.preweightedavgyield());
    appendLong(iter++, data.data.openyield());
    appendLong(iter++, data.data.highyield());

    appendLong(iter++, data.data.lowyield());
    appendLong(iter++, data.data.lastyield());
    appendLong(iter++, data.data.weightedavgyield());
    appendInt(iter++, data.data.channelno());

    buySellQueueAppender<MDBond>(iter, data);

    appendLong(iter++, data.data.norminalpx());
    appendLong(iter++, data.data.shortsellsharestraded());
    appendLong(iter++, data.data.shortsellturnover());

    VectorSP tuple = Util::createVector(DT_ANY, 1);
    VectorSP intTupleBuffer = Util::createVector(DT_LONG, 0, 10);
    VectorSP longTupleBuffer = Util::createVector(DT_LONG, 0, 10);

#define ARRAY_VECTOR_APPEND(type1, type2, func1, func2, buf) \
    buf->clear();                                            \
    for (int i = 0; i < data.data.func1(); ++i) {            \
        type1 element = data.data.func2(i);                  \
        buf->append##type2(&element, 1);                     \
    }                                                        \
    tuple->set(0, buf);                                      \
    getVec(iter++)->append(tuple);

    ARRAY_VECTOR_APPEND(int, Int, buysettltypequeue_size, buysettltypequeue, intTupleBuffer)
    ARRAY_VECTOR_APPEND(int, Int, sellsettltypequeue_size, sellsettltypequeue, intTupleBuffer)
    ARRAY_VECTOR_APPEND(long long, Long, buyyieldqueue_size, buyyieldqueue, longTupleBuffer)
    ARRAY_VECTOR_APPEND(long long, Long, sellyieldqueue_size, sellyieldqueue, longTupleBuffer)

#undef ARRAY_VECTOR_APPEND

    appendLong(iter++, data.data.premarketlastpx());
    appendLong(iter++, data.data.premarkettotalvolumetrade());
    appendLong(iter++, data.data.premarkettotalvaluetrade());
    appendLong(iter++, data.data.premarkethighpx());
    appendLong(iter++, data.data.premarketlowpx());

    appendLong(iter++, data.data.afterhourslastpx());
    appendLong(iter++, data.data.afterhourstotalvolumetrade());
    appendLong(iter++, data.data.afterhourstotalvaluetrade());
    appendLong(iter++, data.data.afterhourshighpx());
    appendLong(iter++, data.data.afterhourslowpx());

    appendString(iter++, data.data.marketphasecode());
    appendString(iter++, data.data.subtradingphasecode1());
    appendString(iter++, data.data.subtradingphasecode2());
    appendString(iter++, data.data.subtradingphasecode3());
    appendString(iter++, data.data.subtradingphasecode4());

    appendString(iter++, data.data.subtradingphasecode5());
    appendInt(iter++, data.data.lastpxtype());
    appendLong(iter++, data.data.auctionlastpx());
    appendLong(iter++, data.data.auctionvolumetrade());
    appendLong(iter++, data.data.auctionvaluetrade());

    appendLong(iter++, data.data.usconsolidatevolume());
    appendLong(iter++, data.data.uscompositeclosepx());
    appendString(iter++, data.data.tradinghaltreason());
    appendLong(iter++, data.data.otctotalvolumetrade());
    appendLong(iter++, data.data.otctotalvaluetrade());

    appendLong(iter++, data.data.otcnumtrades());
    appendInt(iter++, data.data.datamultiplepowerof10());
}

void optionTickReader(vector<ConstantSP> &buffer, TimedWrapper<MDOption> &data) {
    ConstantVecIterator iter = buffer.begin();
    snapshotHeadAppender2<MDOption>(iter, data);

    appendInt(iter++, countDays(data.data.tradingdate()));
    appendLong(iter++, data.data.preopeninterest());
    appendLong(iter++, data.data.presettleprice());
    appendLong(iter++, data.data.openinterest());
    appendLong(iter++, data.data.settleprice());

    appendLong(iter++, data.data.predelta());
    appendLong(iter++, data.data.currdelta());
    appendInt(iter++, countDays(data.data.exchangedate()));
    appendInt(iter++, countTime(data.data.exchangetime()));
    appendLong(iter++, data.data.referenceprice());

    appendInt(iter++, data.data.channelno());

    buySellQueueAppender<MDOption>(iter, data);
    appendInt(iter++, data.data.datamultiplepowerof10());
}

void ValidSecurityLendingEntryAppender(
    ConstantVecIterator &iter,
    ::google::protobuf::RepeatedPtrField<::com::htsc::mdc::insight::model::ADValidSecurityLendingEntry> data) {
    int size = data.size();
    VectorSP tuple = Util::createVector(DT_ANY, 1);
    VectorSP levelBuffer = Util::createVector(DT_INT, 0, size);
    VectorSP rateBuffer = Util::createVector(DT_LONG, 0, size);
    VectorSP termBuffer = Util::createVector(DT_INT, 0, size);
    VectorSP amountBuffer = Util::createVector(DT_LONG, 0, size);
    VectorSP HtscProvidedBuffer = Util::createVector(DT_BOOL, 0, size);

    for (auto it = data.begin(); it != data.end(); ++it) {
        int level = it->level();
        long long rate = it->rate();
        int term = it->term();
        long long amount = it->amount();
        char HtscProvided = it->htscprovided();

        levelBuffer->appendInt(&level, 1);
        rateBuffer->appendLong(&rate, 1);
        termBuffer->appendInt(&term, 1);
        amountBuffer->appendLong(&amount, 1);
        HtscProvidedBuffer->appendBool(&HtscProvided, 1);
    }
    tuple->set(0, levelBuffer);
    getVec(iter++)->append(tuple);
    tuple->set(0, rateBuffer);
    getVec(iter++)->append(tuple);
    tuple->set(0, termBuffer);
    getVec(iter++)->append(tuple);
    tuple->set(0, amountBuffer);
    getVec(iter++)->append(tuple);
    tuple->set(0, HtscProvidedBuffer);
    getVec(iter++)->append(tuple);
}

void SecurityLendingEntryAppender(
    ConstantVecIterator &iter,
    ::google::protobuf::RepeatedPtrField<::com::htsc::mdc::insight::model::ADSecurityLendingEntry> data) {
    int size = data.size();
    VectorSP tuple = Util::createVector(DT_ANY, 1);
    VectorSP levelBuffer = Util::createVector(DT_INT, 0, size);
    VectorSP rateBuffer = Util::createVector(DT_LONG, 0, size);
    VectorSP termBuffer = Util::createVector(DT_INT, 0, size);
    VectorSP amountBuffer = Util::createVector(DT_LONG, 0, size);
    VectorSP matchedAmountBuffer = Util::createVector(DT_LONG, 0, size);

    for (auto it = data.begin(); it != data.end(); ++it) {
        int level = it->level();
        long long rate = it->rate();
        int term = it->term();
        long long amount = it->totalamount();
        long long matchedAmount = it->matchedamount();

        levelBuffer->appendInt(&level, 1);
        rateBuffer->appendLong(&rate, 1);
        termBuffer->appendInt(&term, 1);
        amountBuffer->appendLong(&amount, 1);
        matchedAmountBuffer->appendLong(&matchedAmount, 1);
    }
    tuple->set(0, levelBuffer);
    getVec(iter++)->append(tuple);
    tuple->set(0, rateBuffer);
    getVec(iter++)->append(tuple);
    tuple->set(0, termBuffer);
    getVec(iter++)->append(tuple);
    tuple->set(0, amountBuffer);
    getVec(iter++)->append(tuple);
    tuple->set(0, matchedAmountBuffer);
    getVec(iter++)->append(tuple);
}

string mergeStringList(const vector<string> &vec) {
    if (vec.empty()) {
        return "[]";
    }
    string ret;
    for (auto i = 0u; i < vec.size(); ++i) {
        ret += "\"";
        ret += vec[i];
        ret += "\"";
        if (vec.size() - 1 != i) {
            ret += ", ";
        }
    }
    return ret;
}

void EstimatedSecurityLendingEntryAppender(
    ConstantVecIterator &iter,
    ::google::protobuf::RepeatedPtrField<::com::htsc::mdc::insight::model::ADEstimatedSecurityLendingEntry> data) {
    int size = data.size();
    VectorSP tuple = Util::createVector(DT_ANY, 1);
    VectorSP levelBuffer = Util::createVector(DT_INT, 0, size);
    VectorSP rateBuffer = Util::createVector(DT_LONG, 0, size);
    vector<string> termStringBuffer;
    VectorSP amountBuffer = Util::createVector(DT_LONG, 0, size);
    VectorSP PostponeProbabilityBuffer = Util::createVector(DT_LONG, 0, size);

    for (auto it = data.begin(); it != data.end(); ++it) {
        int level = it->level();
        long long rate = it->rate();
        termStringBuffer.push_back(it->term());
        long long amount = it->amount();
        long long PostponeProbability = it->postponeprobability();

        levelBuffer->appendInt(&level, 1);
        rateBuffer->appendLong(&rate, 1);
        amountBuffer->appendLong(&amount, 1);
        PostponeProbabilityBuffer->appendLong(&PostponeProbability, 1);
    }
    tuple->set(0, levelBuffer);
    getVec(iter++)->append(tuple);
    tuple->set(0, rateBuffer);
    getVec(iter++)->append(tuple);
    appendString(iter++, mergeStringList(termStringBuffer));
    tuple->set(0, amountBuffer);
    getVec(iter++)->append(tuple);
    tuple->set(0, PostponeProbabilityBuffer);
    getVec(iter++)->append(tuple);
}

void securityLendingTickReader(vector<ConstantSP> &buffer, TimedWrapper<MDSecurityLending> &data) {
    ConstantVecIterator iter = buffer.begin();
    appendString(iter++, data.data.htscsecurityid());
    appendInt(iter++, countDays(data.data.mddate()));
    appendInt(iter++, countTime(data.data.mdtime()));
    appendLong(iter++, countTimestamp(data.data.datatimestamp()));
    appendString(iter++, data.data.tradingphasecode());

    appendString(iter++, com::htsc::mdc::model::ESecurityIDSource_Name(data.data.securityidsource()));
    appendString(iter++, com::htsc::mdc::model::ESecurityType_Name(data.data.securitytype()));
    appendLong(iter++, data.data.preweightedrate());
    appendLong(iter++, data.data.prehighrate());
    appendLong(iter++, data.data.prelowrate());

    appendLong(iter++, data.data.prehtscvolume());
    appendLong(iter++, data.data.premarketvolume());
    appendLong(iter++, data.data.weightedrate());
    appendLong(iter++, data.data.highrate());
    appendLong(iter++, data.data.lowrate());

    appendLong(iter++, data.data.htscvolume());
    appendLong(iter++, data.data.marketvolume());
    appendLong(iter++, data.data.bestborrowrate());
    appendLong(iter++, data.data.bestlendrate());

    ValidSecurityLendingEntryAppender(iter, data.data.validborrows());
    ValidSecurityLendingEntryAppender(iter, data.data.validalends());
    ValidSecurityLendingEntryAppender(iter, data.data.validblends());
    ValidSecurityLendingEntryAppender(iter, data.data.validclends());

    SecurityLendingEntryAppender(iter, data.data.alends());
    SecurityLendingEntryAppender(iter, data.data.blends());
    SecurityLendingEntryAppender(iter, data.data.clends());

    ValidSecurityLendingEntryAppender(iter, data.data.validreservationborrows());
    ValidSecurityLendingEntryAppender(iter, data.data.validreservationlends());
    SecurityLendingEntryAppender(iter, data.data.reservationborrows());
    SecurityLendingEntryAppender(iter, data.data.reservationlends());
    ValidSecurityLendingEntryAppender(iter, data.data.validotclends());

    appendLong(iter++, data.data.bestreservationborrowrate());
    appendLong(iter++, data.data.bestreservationlendrate());
    appendLong(iter++, data.data.validlendamount());
    appendLong(iter++, data.data.validalendamount());
    appendLong(iter++, data.data.validblendamount());

    appendLong(iter++, data.data.htscborrowamount());
    appendLong(iter++, data.data.htscborrowrate());
    appendLong(iter++, data.data.bestloanrate());
    appendLong(iter++, data.data.htscborrowtradevolume());
    appendLong(iter++, data.data.htscborrowweightedrate());

    appendLong(iter++, data.data.prehtscborrowtradevolume());
    appendLong(iter++, data.data.prehtscborrowweightedrate());

    ValidSecurityLendingEntryAppender(iter, data.data.htscborrows());
    ValidSecurityLendingEntryAppender(iter, data.data.loans());

    appendInt(iter++, data.data.datamultiplepowerof10());

    EstimatedSecurityLendingEntryAppender(iter, data.data.externallends());

    appendString(iter++, data.data.htscborrowterm());
    appendLong(iter++, data.data.loanamount());
    ValidSecurityLendingEntryAppender(iter, data.data.marketborrows());
    appendString(iter++, data.data.validlendterm());
    appendLong(iter++, data.data.validborrowamount());
    ValidSecurityLendingEntryAppender(iter, data.data.marketlends());
}