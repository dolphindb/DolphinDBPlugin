//
// Created by htxu on 11/21/2023.
//

#ifndef PLUGINNSQ_NSQQUEUES_H
#define PLUGINNSQ_NSQQUEUES_H

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <unistd.h>
#endif // _WIN32


#include <CoreConcept.h>
#include "NsqUtil.h"

/**
 * Member variables and methods related to ThreadedQueue
 */
class NsqQueues {

public:
    NsqQueues() {
        long long time = Util::getNanoEpochTime();
        timeGap_ = Util::toLocalNanoTimestamp(time) - time;
    }
    ~NsqQueues() = default;

    /// Interfaces
    void initAndStart(Heap *heap, const string &dataType, nsqUtil::MarketType marketType, const TableSP &table, long long queueDepth);
    void initAndStartTradeEntrust(Heap *heap, nsqUtil::MarketType marketType, int channel, const TableSP &table, long long queueDepth);
    void setOptionFlag(int option);
    void addSnapshotExtra(const string &dataVersion);
    ConstantSP getSchema(const string &dataType);
    void stop(const string& dataType, nsqUtil::MarketType marketType);
    ConstantSP getStatus();
    // push data to ThreadedQueues
    void pushData(const nsqUtil::SnapshotDataStruct& data, nsqUtil::MarketType marketType) const;
    void pushData(CHSNsqSecuTransactionTradeDataField *data, nsqUtil::MarketType marketType) const;
    void pushData(CHSNsqSecuTransactionEntrustDataField *data, nsqUtil::MarketType marketType) const;

    std::pair<vector<string>, vector<string>> getTypesToCancel(const string &dataType, nsqUtil::MarketType marketType);

private:
    /// ThreadedQueues
    long long timeGap_ = 0;
    unordered_map<nsqUtil::MarketType, SmartPointer<ThreadedQueue<nsqUtil::TradeDataStruct>>, nsqUtil::enum_hash> threadedQueueMapT_;
    unordered_map<nsqUtil::MarketType, SmartPointer<ThreadedQueue<nsqUtil::EntrustDataStruct>>, nsqUtil::enum_hash> threadedQueueMapE_;
    unordered_map<nsqUtil::MarketType, SmartPointer<ThreadedQueue<nsqUtil::SnapshotDataStruct>>, nsqUtil::enum_hash> threadedQueueMapS_;

    unordered_map<nsqUtil::MarketType, unordered_map<int, SmartPointer<ThreadedQueue<nsqUtil::TradeEntrustDataStruct>>>, nsqUtil::enum_hash> threadedQueueMapTE_;

    int optionFlag_ = 0;

    MarketTypeContainer<string> marketTypes_{
        NSQ_PREFIX,
        {nsqUtil::TRADE, nsqUtil::ENTRUST, nsqUtil::SNAPSHOT, nsqUtil::TRADE_ENTRUST, nsqUtil::ENTRUST_220105},
        {nsqUtil::tradeMeta, nsqUtil::entrustMeta, nsqUtil::snapshotMetaConcise, nsqUtil::tradeEntrustMeta, nsqUtil::entrustMeta_220105}
    };

    std::function<void (vector<ConstantSP> &, nsqUtil::SnapshotDataStruct &)>
    snapshotReader_ = [](vector<ConstantSP> &buffer, nsqUtil::SnapshotDataStruct &data) {

        auto col = buffer.begin();
        snapshotReaderConcise(col, data);
    };

    /// Helpers
    bool isSubscribed(const string& dataType, nsqUtil::MarketType marketType);
    template <typename DataStruct>
    void addThreadedQueue(Heap *heap, const string &dataType, nsqUtil::MarketType marketType, const TableSP &table,
                                 MetaTable meta, unordered_map<nsqUtil::MarketType, SmartPointer<ThreadedQueue<DataStruct>>, nsqUtil::enum_hash> &map,
                                 std::function<void(vector<ConstantSP> &, DataStruct &)> reader, long long queueDepth);
};


#endif //PLUGINNSQ_NSQQUEUES_H
