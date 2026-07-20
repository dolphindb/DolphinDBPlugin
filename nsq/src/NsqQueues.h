#pragma once

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <unistd.h>
#endif // _WIN32

#include "ddb_nsq.h"
#include "DolphinDBEverything.h"
#include <CoreConcept.h>
#include <LocklessContainer.h>
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
    void initAndStart(Heap *heap, nsq_data data, nsq_market market, nsq_version version,const TableSP &table, long long queueDepth);
    void initAndStartTradeEntrust(Heap *heap, nsq_market market, int channel, const TableSP &table, long long queueDepth);
    void setOptionFlag(int option);
    void addSnapshotExtra(nsq_version version);
    ConstantSP getSchema(nsq_data data, nsq_version version);
    void stop(nsq_data data, nsq_market market);
    ConstantSP getStatus();
    // push data to ThreadedQueues
    void pushData(const nsqUtil::SnapshotDataStruct& data, nsq_market market) const;
    void pushData(CHSNsqSecuTransactionTradeDataField *data, nsq_market market) const;
    void pushData(CHSNsqSecuTransactionEntrustDataField *data, nsq_market market) const;

    std::pair<vector<nsq_data>, vector<nsq_data>> getTypesToCancel(nsq_data data, nsq_market market);

private:
    using TradeQueue = ThreadedQueue<nsqUtil::TradeDataStruct>;
    using EntrustQueue = ThreadedQueue<nsqUtil::EntrustDataStruct>;
    using SnapshotQueue = ThreadedQueue<nsqUtil::SnapshotDataStruct>;
    using TradeEntrustQueue = ThreadedQueue<nsqUtil::TradeEntrustDataStruct>;

    using TradeQueueSP = SmartPointer<ThreadedQueue<nsqUtil::TradeDataStruct>>;
    using EntrustQueueSP = SmartPointer<ThreadedQueue<nsqUtil::EntrustDataStruct>>;
    using SnapshotQueueSP = SmartPointer<ThreadedQueue<nsqUtil::SnapshotDataStruct>>;
    using TradeEntrustQueueSP = SmartPointer<ThreadedQueue<nsqUtil::TradeEntrustDataStruct>>;

    /// ThreadedQueues
    long long timeGap_ = 0;
    unordered_map<nsq_market, TradeQueueSP, enum_hash> threadedQueueMapT_;
    unordered_map<nsq_market, EntrustQueueSP, enum_hash> threadedQueueMapE_;
    unordered_map<nsq_market, SnapshotQueueSP, enum_hash> threadedQueueMapS_;

    unordered_map<nsq_market, unordered_map<int, TradeEntrustQueueSP>, enum_hash> threadedQueueMapTE_;
    mutable IrremovableLocklessFlatHashmap<int, TradeQueue *> tradeRoutes_;
    mutable IrremovableLocklessFlatHashmap<int, EntrustQueue *> entrustRoutes_;
    mutable IrremovableLocklessFlatHashmap<int, SnapshotQueue *> snapshotRoutes_;
    mutable IrremovableLocklessFlatHashmap<long long, TradeEntrustQueue *> tradeEntrustRoutes_;

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
    bool isSubscribed(nsq_data data, nsq_market market);
    template <typename DataStruct>
    void addThreadedQueue(Heap *heap, nsq_data data, nsq_market market, const TableSP &table,
                                 MetaTable meta, unordered_map<nsq_market, SmartPointer<ThreadedQueue<DataStruct>>, enum_hash> &map,
                                 std::function<void(vector<ConstantSP> &, DataStruct &)> reader, long long queueDepth);
};
