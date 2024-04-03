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
    NsqQueues() = default;
    ~NsqQueues() = default;

    /// Interfaces
    void initAndStart(Heap *heap, const string &dataType, const string &marketType, const TableSP &table);
    void initAndStartTradeEntrust(Heap *heap, const string &marketType, int channel, const TableSP &table);
    void setOptionFlag(int option);
    void addSnapshotExtra();
    ConstantSP getSchema(const string &dataType);
    void stop(const string& dataType, const string& marketType);
    ConstantSP getStatus();
    // push data to ThreadedQueues
    void pushData(const nsqUtil::SnapshotDataStruct& data, const string& marketType);
    void pushData(CHSNsqSecuTransactionTradeDataField *data, const string& marketType);
    void pushData(CHSNsqSecuTransactionEntrustDataField *data, const string& marketType);

    vector<string> getTypesToCancel(const string &dataType, const string &marketType);

private:
    /// ThreadedQueues
    unordered_map<string, SmartPointer<ThreadedQueue<nsqUtil::TradeDataStruct>>> threadedQueueMapT_;
    unordered_map<string, SmartPointer<ThreadedQueue<nsqUtil::EntrustDataStruct>>> threadedQueueMapE_;
    unordered_map<string, SmartPointer<ThreadedQueue<nsqUtil::SnapshotDataStruct>>> threadedQueueMapS_;

    unordered_map<string, unordered_map<int, SmartPointer<ThreadedQueue<nsqUtil::TradeEntrustDataStruct>>>> threadedQueueMapTE_;

    int optionFlag_ = 0;

    MarketTypeContainer marketTypes_{
        NSQ_PREFIX,
        {nsqUtil::TRADE, nsqUtil::ENTRUST, nsqUtil::SNAPSHOT, nsqUtil::TRADE_ENTRUST},
        {nsqUtil::tradeMeta, nsqUtil::entrustMeta, nsqUtil::snapshotMetaConcise, nsqUtil::tradeEntrustMeta}
    };

    std::function<void (vector<ConstantSP> &, nsqUtil::SnapshotDataStruct &)>
    snapshotReader_ = [](vector<ConstantSP> &buffer, nsqUtil::SnapshotDataStruct &data) {

        auto col = buffer.begin();
        snapshotReaderConcise(col, data);
    };

    /// Helpers
    bool isSubscribed(const string& dataType, const string& marketType);
    template <typename DataStruct>
    void addThreadedQueue(Heap *heap, const string &dataType, const string &marketType, const TableSP &table,
                                 MetaTable meta, unordered_map<string, SmartPointer<ThreadedQueue<DataStruct>>> &map,
                                 std::function<void(vector<ConstantSP> &, DataStruct &)> reader);
};


#endif //PLUGINNSQ_NSQQUEUES_H
