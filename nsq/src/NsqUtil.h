#pragma once

#include "NsqEverything.h"

#include <ddbplugin/ThreadedQueue.h>

#include "PluginUtil.h"

using namespace pluginUtil;

const static long long QUEUE_DEPTH = 1000000;

/**
 * Includes definitions and constants of data types, as well as functions related to data types and data processing.
 */
namespace nsqUtil {

    // Data Structs
    struct TradeDataStruct {
        long long reachTime;
        CHSNsqSecuTransactionTradeDataField data;
    };
    struct EntrustDataStruct {
        long long reachTime;
        CHSNsqSecuTransactionEntrustDataField data;
    };
    struct SnapshotDataStruct {
        long long reachTime;
        CHSNsqSecuDepthMarketDataField data;
        vector<HSIntVolume> Bid1Volume;
        HSNum Bid1Count;
        HSNum MaxBid1Count;
        vector<HSIntVolume> Ask1Volume;
        HSNum Ask1Count;
        HSNum MaxAsk1Count;
    };
    struct TradeEntrustDataStruct {
        long long reachTime;
        bool isTrade;
        CHSNsqSecuTransactionTradeDataField tradeData;
        CHSNsqSecuTransactionEntrustDataField entrustData;
    };

    // Data Types
    constexpr auto SNAPSHOT = "snapshot";
    constexpr auto TRADE = "trade";
    constexpr auto ENTRUST = "orders";
    constexpr auto ENTRUST_220105 = "orders_220105";
    constexpr auto TRADE_ENTRUST = "orderTrade";

    // Options
    constexpr auto RECEIVED_TIME = "receivedTime";
    constexpr auto OUTPUT_ELAPSED = "outputElapsed";
    constexpr auto GET_ALL_FIELD_NAMES = "getAllFieldNames";

    // Table Metas
    extern const MetaTable tradeMeta;
    extern const MetaTable entrustMeta;
    extern const MetaTable entrustMeta_220105;
    extern const MetaTable snapshotMetaConcise;
    extern const MetaTable snapshotMetaExtra_220105;
    extern const MetaTable snapshotMetaExtra;
    extern const MetaTable tradeEntrustMeta;

    // Struct Readers for ThreadedQueue
    void tradeReader(vector<ConstantSP> &buffer, TradeDataStruct &data);
    void entrustReader(vector<ConstantSP> &buffer, EntrustDataStruct &data);
    void entrustReader_220105(vector<ConstantSP> &buffer, EntrustDataStruct &data);
    void tradeEntrustReader(vector<ConstantSP> &buffer, TradeEntrustDataStruct &data);
    ConstantVecIterator &snapshotReaderConcise(ConstantVecIterator &col, SnapshotDataStruct &data);
    void snapshotReaderExtra_220105(ConstantVecIterator &col, SnapshotDataStruct &data);
    void snapshotReaderExtra(ConstantVecIterator &col, SnapshotDataStruct &data);

    // Helpers
    int getDate(HSDate date);
    int getTime(HSTime time);

} // namespace nsqUtil
