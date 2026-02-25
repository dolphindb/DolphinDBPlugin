//
// Created by htxu on 11/21/2023.
//

#include "NsqQueues.h"

using namespace nsqUtil;

void NsqQueues::initAndStart(Heap *heap, const string &dataType, nsqUtil::MarketType marketType, const TableSP &table, long long queueDepth) {

    if (isSubscribed(dataType, marketType)) {
        throw RuntimeException(NSQ_PREFIX + "subscription already exists"); // optimization: tradeAndOrder can be subscribed multiple times (to add channels)
    }

    if (dataType == TRADE) {
        addThreadedQueue<nsqUtil::TradeDataStruct>(heap, dataType, marketType, table, tradeMeta, threadedQueueMapT_, tradeReader, queueDepth);
    } else if (dataType == ENTRUST) {
        addThreadedQueue<nsqUtil::EntrustDataStruct>(heap, dataType, marketType, table, entrustMeta, threadedQueueMapE_, entrustReader, queueDepth);
    } else if (dataType == SNAPSHOT) {
        addThreadedQueue<nsqUtil::SnapshotDataStruct>(heap, dataType, marketType, table, marketTypes_.get(SNAPSHOT), threadedQueueMapS_, snapshotReader_, queueDepth);
    } else if (dataType == ENTRUST_220105) {
        addThreadedQueue<nsqUtil::EntrustDataStruct>(heap, dataType, marketType, table, entrustMeta_220105, threadedQueueMapE_, entrustReader_220105, queueDepth);
    }
}

void NsqQueues::initAndStartTradeEntrust(Heap *heap, nsqUtil::MarketType marketType, int channel, const TableSP &table, long long queueDepth) {
    if (threadedQueueMapTE_.count(marketType) == 0) {
        threadedQueueMapTE_[marketType] = {};
    }
    if (threadedQueueMapTE_[marketType].count(channel) == 0) {
        auto flag = optionFlag_ | OPT_RECEIVED; // tradeOrder must have receivedTime
        threadedQueueMapTE_[marketType][channel] = new ThreadedQueue<TradeEntrustDataStruct>(
                heap, 100, queueDepth, tradeEntrustMeta, nullptr, flag,
                "tradeOrder", NSQ_PREFIX, Util::BUF_SIZE, tradeEntrustReader
        );
    }
    threadedQueueMapTE_[marketType][channel]->setTable(table);
    threadedQueueMapTE_[marketType][channel]->start();
}

bool NsqQueues::isSubscribed(const string &dataType, nsqUtil::MarketType marketType) {

    if (dataType == TRADE && threadedQueueMapT_.count(marketType) && threadedQueueMapT_[marketType]->isStarted()) {
        return true;
    } else if (dataType == ENTRUST && threadedQueueMapE_.count(marketType) && threadedQueueMapE_[marketType]->isStarted()) {
        return true;
    } else if (dataType == SNAPSHOT && threadedQueueMapS_.count(marketType) && threadedQueueMapS_[marketType]->isStarted()) {
        return true;
    } else if (dataType == TRADE_ENTRUST && threadedQueueMapTE_.count(marketType)) {
        for (const auto& item : threadedQueueMapTE_[marketType]) {
            if (item.second->isStarted()) {
                return true;
            }
        }
    }

    return false;
}

template<typename DataStruct>
void NsqQueues::addThreadedQueue(Heap *heap, const string &dataType, nsqUtil::MarketType marketType, const TableSP &table,
                                 MetaTable meta, unordered_map<nsqUtil::MarketType, SmartPointer<ThreadedQueue<DataStruct>>> &map,
                                 std::function<void(vector<ConstantSP> &, DataStruct &)> reader, long long queueDepth) {

    if (map.count(marketType) == 0) {
        map[marketType] = new ThreadedQueue<DataStruct>(heap, 100, queueDepth, meta, nullptr, optionFlag_, dataType, NSQ_PREFIX, Util::BUF_SIZE, reader);
    }
    map[marketType]->setTable(table);
    map[marketType]->start();
}

void NsqQueues::pushData(CHSNsqSecuTransactionTradeDataField *data, const nsqUtil::MarketType marketType) const {

    // trade
    long long reachTime = Util::getNanoEpochTime() + timeGap_;
    auto iter = threadedQueueMapT_.find(marketType);
    if (LIKELY(iter != threadedQueueMapT_.end())) {
        iter->second->push(
                {
                        reachTime,
                        *data
                }
        );
    }

    // tradeEntrust
    int channel = data->ChannelNo;
    auto iterTE = threadedQueueMapTE_.find(marketType);
    if (LIKELY(iterTE != threadedQueueMapTE_.end())) {
        auto channelIter = iterTE->second.find(channel);
        if (LIKELY(channelIter != iterTE->second.end())) {
            channelIter->second->push(
                    {
                            reachTime,
                            true,
                            *data,
                            {}
                    }
            );
        }
    }
}

void NsqQueues::pushData(CHSNsqSecuTransactionEntrustDataField *data, nsqUtil::MarketType marketType) const {

    long long reachTime = Util::getNanoEpochTime() + timeGap_;
    auto iter = threadedQueueMapE_.find(marketType);
    if (LIKELY(iter != threadedQueueMapE_.end())) {
        iter->second->push(
                {
                        reachTime,
                        *data
                }
        );
    }
    // entrust

    // tradeEntrust
    int channel = data->ChannelNo;
    auto iterTE = threadedQueueMapTE_.find(marketType);
    if (LIKELY(iterTE != threadedQueueMapTE_.end())) {
        auto channelIter = iterTE->second.find(channel);
        if (LIKELY(channelIter != iterTE->second.end())) {
            channelIter->second->push(
                    {
                            reachTime,
                            false,
                            {},
                            *data
                    }
            );
        }
    }
}

void NsqQueues::pushData(const nsqUtil::SnapshotDataStruct& data, nsqUtil::MarketType marketType) const {
    auto iter = threadedQueueMapS_.find(marketType);
    if (LIKELY(iter != threadedQueueMapS_.end())) {
        iter->second->push(data);
    } 
}

ConstantSP NsqQueues::getStatus() {
    vector<string> colNames{"topicType",        START_TIME_STR,       END_TIME_STR,
                            FIRST_MSG_TIME_STR, LAST_MSG_TIME_STR,    PROCESSED_MSG_COUNT_STR,
                            LAST_ERR_MSG_STR,   FAILED_MSG_COUNT_STR, LAST_FAILED_TIMESTAMP_STR,
                            QUEUE_DEPTH_LIMIT,  QUEUE_DEPTH_STR};

    vector<ConstantSP> cols(colNames.size());
    vector<DATA_TYPE> dataTypes{DT_STRING,        DT_NANOTIMESTAMP, DT_NANOTIMESTAMP, DT_NANOTIMESTAMP,
                                DT_NANOTIMESTAMP, DT_LONG,          DT_STRING,        DT_LONG,
                                DT_NANOTIMESTAMP, DT_LONG,          DT_LONG};
    for (auto i = 0; i < (int)colNames.size(); i++) {
        cols[i] = Util::createVector(dataTypes[i], 0, 0);
    }

    for (auto &dataType : {TRADE, ENTRUST, SNAPSHOT}) {
        for (auto &marketType : {MarketType::SH, MarketType::SZ}) {
            StreamStatus status;
            if (dataType == TRADE && threadedQueueMapT_.count(marketType)) {
                status = threadedQueueMapT_[marketType]->getStatusConst();
            } else if (dataType == ENTRUST && threadedQueueMapE_.count(marketType)) {
                status = threadedQueueMapE_[marketType]->getStatusConst();
            } else if (dataType == SNAPSHOT && threadedQueueMapS_.count(marketType)) {
                status = threadedQueueMapS_[marketType]->getStatusConst();
            } else {
                continue;
            }

            auto col = cols.begin();

            appendString(col++, string("(") + dataType + ", " + getMarketTypeStr(marketType) + ")");
            appendLong(col++, status.startTime_);
            appendLong(col++, status.endTime_);
            appendLong(col++, status.firstMsgTime_);
            appendLong(col++, status.lastMsgTime_);
            appendLong(col++, status.processedMsgCount_);
            appendString(col++, status.lastErrMsg_);
            appendLong(col++, status.failedMsgCount_);
            appendLong(col++, status.lastFailedTimestamp_);
            appendLong(col++, status.queueDepthLimit_);
            appendLong(col++, status.queueDepth_);
        }
    }

    // orderTrade type status
    for (auto &marketType : {nsqUtil::MarketType::SH, nsqUtil::MarketType::SZ}) {
        StreamStatus status;
        if (threadedQueueMapTE_.count(marketType)) {
            for (auto it = threadedQueueMapTE_[marketType].begin(); it != threadedQueueMapTE_[marketType].end(); ++it) {
                int channelNo = it->first;
                status = it->second->getStatusConst();

                auto col = cols.begin();

                appendString(col++, string("(orderTrade") + ", " + getMarketTypeStr(marketType) + ", channel " + std::to_string(channelNo) +")");
                appendLong(col++, status.startTime_);
                appendLong(col++, status.endTime_);
                appendLong(col++, status.firstMsgTime_);
                appendLong(col++, status.lastMsgTime_);
                appendLong(col++, status.processedMsgCount_);
                appendString(col++, status.lastErrMsg_);
                appendLong(col++, status.failedMsgCount_);
                appendLong(col++, status.lastFailedTimestamp_);
                appendLong(col++, status.queueDepthLimit_);
                appendLong(col++, status.queueDepth_);
            }
        }
    }

    return Util::createTable(colNames, cols);
}

void NsqQueues::stop(const string &dataType, nsqUtil::MarketType marketType) {

    if (!isSubscribed(dataType, marketType)) {
        throw RuntimeException(NSQ_PREFIX + "there is no subscription to cancel.");
    }

    if (dataType == TRADE) {
        threadedQueueMapT_[marketType]->stop();
    } else if (dataType == ENTRUST) {
        threadedQueueMapE_[marketType]->stop();
    } else if (dataType == SNAPSHOT) {
        threadedQueueMapS_[marketType]->stop();
    } else if (dataType == TRADE_ENTRUST) {
        for (const auto &item : threadedQueueMapTE_[marketType]) {
            item.second->stop();
        }
    }
}

void NsqQueues::setOptionFlag(int option) {

    optionFlag_ |= option;
}

void NsqQueues::addSnapshotExtra(const string &dataVersion) {

    // update meta
    auto snapshotMeta = marketTypes_.get(SNAPSHOT);
    snapshotMeta.colNames_.insert(snapshotMeta.colNames_.end(),
            snapshotMetaExtra.colNames_.begin(), snapshotMetaExtra.colNames_.end());
    snapshotMeta.colTypes_.insert(snapshotMeta.colTypes_.end(),
            snapshotMetaExtra.colTypes_.begin(), snapshotMetaExtra.colTypes_.end());
    if (dataVersion == "v220105") {
        snapshotMeta.colNames_.insert(snapshotMeta.colNames_.end(),
                snapshotMetaExtra_220105.colNames_.begin(), snapshotMetaExtra_220105.colNames_.end());
        snapshotMeta.colTypes_.insert(snapshotMeta.colTypes_.end(),
                snapshotMetaExtra_220105.colTypes_.begin(), snapshotMetaExtra_220105.colTypes_.end());
        marketTypes_.add(SNAPSHOT, snapshotMeta);

        // update reader
        snapshotReader_ = [](vector<ConstantSP> &buffer, nsqUtil::SnapshotDataStruct &data) {

            auto col = buffer.begin();
            col = snapshotReaderConcise(col, data);
            snapshotReaderExtra(col, data);
            snapshotReaderExtra_220105(col, data);
        };

    } else {
        marketTypes_.add(SNAPSHOT, snapshotMeta);

        // update reader
        snapshotReader_ = [](vector<ConstantSP> &buffer, nsqUtil::SnapshotDataStruct &data) {

            auto col = buffer.begin();
            col = snapshotReaderConcise(col, data);
            snapshotReaderExtra(col, data);
        };

    }
}

ConstantSP NsqQueues::getSchema(const string &dataType) {

    TableSP schema = marketTypes_.getSchema(dataType, optionFlag_);
    schema->setColumnName(1, "type");
    return schema;
}

std::pair<vector<string>, vector<string>> NsqQueues::getTypesToCancel(const string &dataType, nsqUtil::MarketType marketType) {

    vector<string> typesForCancel;
    vector<string> typesForStop;

    if (dataType == SNAPSHOT) {
        typesForCancel.emplace_back(dataType);
    // if canceling tradeOrder, only cancel trade and/or order that is not subscribed
    } else if (dataType == nsqUtil::TRADE_ENTRUST) {
        if (threadedQueueMapT_.count(marketType) == 0 or !threadedQueueMapT_[marketType]->isStarted()) {
            typesForCancel.emplace_back(TRADE);
        }
        if (threadedQueueMapE_.count(marketType) == 0 or !threadedQueueMapE_[marketType]->isStarted()) {
            typesForCancel.emplace_back(ENTRUST);
        }
    // if canceling trade or order, only cancel it if tradeOrder is not subscribed
    } else {
        bool started = false;
        if (threadedQueueMapTE_.count(marketType)) {
            for (const auto &item : threadedQueueMapTE_[marketType]) {
                if (item.second->isStarted()) {
                    started = true;
                }
            }
        }
        if (!started) {
            typesForCancel.emplace_back(dataType);
        }
    }
    typesForStop.emplace_back(dataType);

    return std::make_pair(typesForCancel, typesForStop);
}
