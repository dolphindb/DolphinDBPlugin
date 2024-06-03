//
// Created by htxu on 11/22/2023.
//

#ifndef PLUGINNSQ_NSQUTIL_H
#define PLUGINNSQ_NSQUTIL_H


#include <ddbplugin/ThreadedQueue.h>
#include "HSNsqStruct.h"

#include "PluginUtil.h"

using namespace pluginUtil;

const string NSQ_PREFIX = "[PLUGIN::NSQ] ";

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
    constexpr char SNAPSHOT[] = "snapshot";
    constexpr char TRADE[] = "trade";
    constexpr char ENTRUST[] = "orders";
    constexpr char ENTRUST_220105[] = "orders_220105";
    constexpr char TRADE_ENTRUST[] = "orderTrade";

    // Market Types
    constexpr char SH[] = "sh";
    constexpr char SZ[] = "sz";

    // Options
    constexpr char RECEIVED_TIME[] = "receivedTime";
    constexpr char OUTPUT_ELAPSED[] = "outputElapsed";
    constexpr char GET_ALL_FIELD_NAMES[] = "getAllFieldNames";

    // Table Metas
    extern const MetaTable tradeMeta;
    extern const MetaTable entrustMeta;
    extern const MetaTable entrustMeta_220105;
    extern const MetaTable snapshotMetaConcise;
    extern const MetaTable snapshotMetaExtra_220105;
    extern const MetaTable snapshotMetaExtra;
    extern const MetaTable tradeEntrustMeta;

    // Type-related Functions
    void checkTypes(const string& dataType, const string& marketType);

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

} // namespace nsqType


#endif //PLUGINNSQ_NSQUTIL_H
