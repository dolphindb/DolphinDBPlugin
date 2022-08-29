#ifndef INSIGHT_PLUGIN_H
#define INSIGHT_PLUGIN_H

#include "CoreConcept.h"
#include "Util.h"
#include <vector>

extern "C" ConstantSP connectInsight(Heap *heap, std::vector<ConstantSP> &arguments);
extern "C" ConstantSP subscribe(Heap *heap, std::vector<ConstantSP> &arguments);
extern "C" ConstantSP unsubscribe(Heap *heap, std::vector<ConstantSP> &arguments);
extern "C" ConstantSP closeInsight(Heap *heap, std::vector<ConstantSP> &arguments);
extern "C" ConstantSP getSchema(Heap *heap, std::vector<ConstantSP> &arguments);

class InsightStockTableMeta {
public:
    InsightStockTableMeta() {
        colNames_ = {
            "HTSCSecurityID", "MDDate", "MDTime","securityIDSource", "PreClosePx", "TotalVolumeTrade", "TotalValueTrade", "LastPx", "OpenPx", "HighPx",

            "LowPx", "DiffPx1", "TotalBuyQty", "TotalSellQty", "WeightedAvgBuyPx", "WeightedAvgSellPx", "BuyPrice1", "BuyPrice2", "BuyPrice3", "BuyPrice4",

            "BuyPrice5", "BuyPrice6", "BuyPrice7", "BuyPrice8", "BuyPrice9", "BuyPrice10", "BuyOrderQty1", "BuyOrderQty2", "BuyOrderQty3", "BuyOrderQty4",

            "BuyOrderQty5", "BuyOrderQty6", "BuyOrderQty7", "BuyOrderQty8", "BuyOrderQty9", "BuyOrderQty10", "SellPrice1", "SellPrice2", "SellPrice3", "SellPrice4",

            "SellPrice5", "SellPrice6", "SellPrice7", "SellPrice8", "SellPrice9", "SellPrice10", "SellOrderQty1", "SellOrderQty2", "SellOrderQty3", "SellOrderQty4",

            "SellOrderQty5", "SellOrderQty6", "SellOrderQty7", "SellOrderQty8", "SellOrderQty9", "SellOrderQty10", "BuyOrder1", "BuyOrder2", "BuyOrder3", "BuyOrder4",

            "BuyOrder5", "BuyOrder6", "BuyOrder7", "BuyOrder8", "BuyOrder9", "BuyOrder10", "SellOrder1", "SellOrder2", "SellOrder3", "SellOrder4",

            "SellOrder5", "SellOrder6", "SellOrder7", "SellOrder8", "SellOrder9", "SellOrder10", "BuyNumOrders1", "BuyNumOrders2", "BuyNumOrders3", "BuyNumOrders4",

            "BuyNumOrders5", "BuyNumOrders6", "BuyNumOrders7", "BuyNumOrders8", "BuyNumOrders9", "BuyNumOrders10", "SellNumOrders1", "SellNumOrders2", "SellNumOrders3", "SellNumOrders4",

            "SellNumOrders5", "SellNumOrders6", "SellNumOrders7", "SellNumOrders8", "SellNumOrders9", "SellNumOrders10", "TradingPhaseCode", "UpdateTime1",
        };

        colTypes_ = {
            DT_SYMBOL, DT_DATE, DT_TIME, DT_SYMBOL, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
            DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
            DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
            DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
            DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
            DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
            DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
            DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
            DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
            DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_SYMBOL, DT_TIMESTAMP,
        };
    }
public:
    std::vector<string> colNames_;
    std::vector<DATA_TYPE> colTypes_; 
};

TableSP getStockSchema() {
    InsightStockTableMeta insightStockTableMeta;
    vector<ConstantSP> cols(2);
    cols[0] = Util::createVector(DT_STRING, insightStockTableMeta.colNames_.size());
    cols[1] = Util::createVector(DT_STRING, insightStockTableMeta.colTypes_.size());
    for (unsigned int i = 0; i < insightStockTableMeta.colNames_.size(); i++) {
        cols[0]->setString(i, insightStockTableMeta.colNames_[i]);
        cols[1]->setString(i, Util::getDataTypeString(insightStockTableMeta.colTypes_[i]));
    }

    std::vector<string> colNames = {"name", "type"};
    TableSP table = Util::createTable(colNames, cols);

    return table;
}

class InsightIndexTableMeta {
public:
    InsightIndexTableMeta() {
        colNames_ = {
            "MDDate", "MDTime", "HTSCSecurityID", "LastPx", "HighPx",
            "LowPx", "TotalVolumeTrade", "TotalValueTrade", "TradingPhaseCode","UpdateTime1",
        };
        colTypes_ = {
            DT_DATE, DT_TIME, DT_SYMBOL, DT_LONG, DT_LONG,
            DT_LONG, DT_LONG, DT_LONG, DT_SYMBOL, DT_TIMESTAMP,
        };
    }
public:
    std::vector<string> colNames_;
    std::vector<DATA_TYPE> colTypes_;
};

TableSP getIndexSchema() {
    InsightIndexTableMeta insightIndexTableMeta;
    vector<ConstantSP> cols(2);
    cols[0] = Util::createVector(DT_STRING, insightIndexTableMeta.colNames_.size());
    cols[1] = Util::createVector(DT_STRING, insightIndexTableMeta.colTypes_.size());
    for (unsigned int i = 0; i < insightIndexTableMeta.colNames_.size(); i++) {
        cols[0]->setString(i, insightIndexTableMeta.colNames_[i]);
        cols[1]->setString(i, Util::getDataTypeString(insightIndexTableMeta.colTypes_[i]));
    }

    std::vector<string> colNames = {"name", "type"};
    TableSP table = Util::createTable(colNames, cols);

    return table;
}

class InsightFuturesTableMeta {
public:
    InsightFuturesTableMeta() {
        colNames_ = {
            "HTSCSecurityID", "MDDate", "MDTime", "securityIDSource", "PreClosePx", "TotalVolumeTrade", "TotalValueTrade", "LastPx", "OpenPx", "HighPx", 
            "LowPx", "PreOpenInterestPx", "PreSettlePrice", "OpenInterest", "BuyPrice1", "BuyPrice2", "BuyPrice3", "BuyPrice4", "BuyPrice5", "BuyOrderQty1", 
            "BuyOrderQty2", "BuyOrderQty3", "BuyOrderQty4", "BuyOrderQty5", "SellPrice1", "SellPrice2", "SellPrice3", "SellPrice4", "SellPrice5", "SellOrderQty1", 
            "SellOrderQty2", "SellOrderQty3", "SellOrderQty4", "SellOrderQty5", "TradingPhaseCode", "UpdateTime1",
        };
        colTypes_ = {
            DT_SYMBOL, DT_DATE, DT_TIME, DT_SYMBOL, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
            DT_LONG,   DT_LONG, DT_LONG, DT_LONG,   DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
            DT_LONG,   DT_LONG, DT_LONG, DT_LONG,   DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
            DT_LONG,   DT_LONG, DT_LONG, DT_LONG,   DT_SYMBOL, DT_TIMESTAMP,
        };
    }

public:
    std::vector<string> colNames_;
    std::vector<DATA_TYPE> colTypes_; 
};

TableSP getFuturesSchema() {
    InsightFuturesTableMeta insightFuturesTableMeta;
    vector<ConstantSP> cols(2);
    cols[0] = Util::createVector(DT_STRING, insightFuturesTableMeta.colNames_.size());
    cols[1] = Util::createVector(DT_STRING, insightFuturesTableMeta.colTypes_.size());
    for (unsigned int i = 0; i < insightFuturesTableMeta.colNames_.size(); i++) {
        cols[0]->setString(i, insightFuturesTableMeta.colNames_[i]);
        cols[1]->setString(i, Util::getDataTypeString(insightFuturesTableMeta.colTypes_[i]));
    }

    std::vector<string> colNames = {"name", "type"};
    TableSP table = Util::createTable(colNames, cols);

    return table;
}

class InsightTransactionTableMeta {
public:
    InsightTransactionTableMeta() {
        colNames_ = {
            "HTSCSecurityID", "MDDate",      "MDTime",      "securityIDSource", "TradeIndex",
            "TradeBuyNo",     "TradeSellNo", "TradeBSFlag", "TradePrice",       "TradeQty",
            "TradeMoney",     "ApplSeqNum",  "UpdateTime1",
        };
        colTypes_ = {
            DT_SYMBOL, DT_DATE, DT_TIME, DT_SYMBOL, DT_LONG, 
            DT_LONG,   DT_LONG, DT_INT,  DT_LONG,   DT_LONG,
            DT_LONG,   DT_LONG, DT_TIMESTAMP,
        };
    }
    
public:
    std::vector<string> colNames_;
    std::vector<DATA_TYPE> colTypes_; 
};

TableSP getTransactionSchema() {
    InsightTransactionTableMeta insightTransactionTableMeta;
    vector<ConstantSP> cols(2);
    cols[0] = Util::createVector(DT_STRING, insightTransactionTableMeta.colNames_.size());
    cols[1] = Util::createVector(DT_STRING, insightTransactionTableMeta.colTypes_.size());
    for (unsigned int i = 0; i < insightTransactionTableMeta.colNames_.size(); i++) {
        cols[0]->setString(i, insightTransactionTableMeta.colNames_[i]);
        cols[1]->setString(i, Util::getDataTypeString(insightTransactionTableMeta.colTypes_[i]));
    }

    std::vector<string> colNames = {"name", "type"};
    TableSP table = Util::createTable(colNames, cols);

    return table;
}

class InsightOrderTableMeta {
public:
    InsightOrderTableMeta() {
        colNames_ = {
            "HTSCSecurityID", "MDDate",     "MDTime",    "securityIDSource", "securityType",
            "OrderIndex",     "SourceType", "OrderType", "OrderPrice",       "OrderQty",
            "OrderBSFlag",    "BuyNo",      "SellNo",    "ApplSeqNum",       "ChannelNo",
            "UpdateTime1",
        };
        colTypes_ = {
            DT_SYMBOL, DT_DATE, DT_TIME, DT_SYMBOL, DT_SYMBOL,
            DT_LONG,   DT_INT,  DT_INT,  DT_LONG,   DT_LONG,
            DT_INT,    DT_LONG, DT_LONG, DT_LONG,   DT_INT,
            DT_TIMESTAMP,
        };
    }

public:
    std::vector<string> colNames_;
    std::vector<DATA_TYPE> colTypes_;
};

TableSP getOrderSchema() {
    InsightOrderTableMeta insightOrderTableMeta;
    vector<ConstantSP> cols(2);
    cols[0] = Util::createVector(DT_STRING, insightOrderTableMeta.colNames_.size());
    cols[1] = Util::createVector(DT_STRING, insightOrderTableMeta.colTypes_.size());
    for (unsigned int i = 0; i < insightOrderTableMeta.colNames_.size(); i++) {
        cols[0]->setString(i, insightOrderTableMeta.colNames_[i]);
        cols[1]->setString(i, Util::getDataTypeString(insightOrderTableMeta.colTypes_[i]));
    }

    std::vector<string> colNames = {"name", "type"};
    TableSP table = Util::createTable(colNames, cols);

    return table;
}
#endif
