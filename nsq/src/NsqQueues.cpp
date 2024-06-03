//
// Created by htxu on 11/21/2023.
//

#include "NsqQueues.h"

using namespace nsqUtil;

void NsqQueues::initAndStart(Heap *heap, const string &dataType, const string &marketType, const TableSP &table) {

    if (isSubscribed(dataType, marketType)) {
        throw RuntimeException(NSQ_PREFIX + "subscription already exists"); // optimization: tradeAndOrder can be subscribed multiple times (to add channels)
    }

    if (dataType == TRADE) {
        addThreadedQueue<nsqUtil::TradeDataStruct>(heap, dataType, marketType, table, tradeMeta, threadedQueueMapT_, tradeReader);
    } else if (dataType == ENTRUST) {
        addThreadedQueue<nsqUtil::EntrustDataStruct>(heap, dataType, marketType, table, entrustMeta, threadedQueueMapE_, entrustReader);
    } else if (dataType == SNAPSHOT) {
        addThreadedQueue<nsqUtil::SnapshotDataStruct>(heap, dataType, marketType, table, marketTypes_.get(SNAPSHOT), threadedQueueMapS_, snapshotReader_);
    } else if (dataType == ENTRUST_220105) {
        addThreadedQueue<nsqUtil::EntrustDataStruct>(heap, dataType, marketType, table, entrustMeta_220105, threadedQueueMapE_, entrustReader_220105);
    }
}

void NsqQueues::initAndStartTradeEntrust(Heap *heap, const string &marketType, int channel, const TableSP &table) {
    if (threadedQueueMapTE_.count(marketType) == 0) {
        threadedQueueMapTE_[marketType] = {};
    }
    if (threadedQueueMapTE_[marketType].count(channel) == 0) {
        auto flag = optionFlag_ & OPT_RECEIVED; // tradeOrder must have receivedTime
        threadedQueueMapTE_[marketType][channel] = new ThreadedQueue<TradeEntrustDataStruct>(
                heap, 100, 100000, tradeEntrustMeta, nullptr, flag,
                "tradeOrder", NSQ_PREFIX, Util::BUF_SIZE, tradeEntrustReader
        );
    }
    threadedQueueMapTE_[marketType][channel]->setTable(table);
    threadedQueueMapTE_[marketType][channel]->start();
}

bool NsqQueues::isSubscribed(const string &dataType, const string &marketType) {

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
void NsqQueues::addThreadedQueue(Heap *heap, const string &dataType, const string &marketType, const TableSP &table,
                                 MetaTable meta, unordered_map<string, SmartPointer<ThreadedQueue<DataStruct>>> &map,
                                 std::function<void(vector<ConstantSP> &, DataStruct &)> reader) {

    if (map.count(marketType) == 0) {
        map[marketType] = new ThreadedQueue<DataStruct>(heap, 100, 100000, meta, nullptr, optionFlag_, dataType, NSQ_PREFIX, Util::BUF_SIZE, reader);
    }
    map[marketType]->setTable(table);
    map[marketType]->start();
}

void NsqQueues::pushData(CHSNsqSecuTransactionTradeDataField *data, const string &marketType) {

    // trade
    if (threadedQueueMapT_.count(marketType) and threadedQueueMapT_[marketType]->isStarted()) {
        threadedQueueMapT_[marketType]->push(
                {
                        Util::toLocalNanoTimestamp(Util::getNanoEpochTime()),
                        *data
                }
        );
    }

    // tradeEntrust
    int channel = data->ChannelNo;
    if (threadedQueueMapTE_.count(marketType) and threadedQueueMapTE_[marketType].count(channel)
    and threadedQueueMapTE_[marketType][channel]->isStarted()) {

        threadedQueueMapTE_[marketType][channel]->push(
                {
                        Util::toLocalNanoTimestamp(Util::getNanoEpochTime()),
                        true,
                        *data,
                        {}
                }
        );
    }
}

void NsqQueues::pushData(CHSNsqSecuTransactionEntrustDataField *data, const string &marketType) {

    // entrust
    if (threadedQueueMapE_.count(marketType) and threadedQueueMapE_[marketType]->isStarted()) {
        threadedQueueMapE_[marketType]->push(
                {
                        Util::toLocalNanoTimestamp(Util::getNanoEpochTime()),
                        *data
                }
        );
    }

    // tradeEntrust
    int channel = data->ChannelNo;
    if (threadedQueueMapTE_.count(marketType) and threadedQueueMapTE_[marketType].count(channel)
    and threadedQueueMapTE_[marketType][channel]->isStarted()) {

        threadedQueueMapTE_[marketType][channel]->push(
                {
                        Util::toLocalNanoTimestamp(Util::getNanoEpochTime()),
                        false,
                        {},
                        *data
                }
        );
    }
}

void NsqQueues::pushData(const nsqUtil::SnapshotDataStruct& data, const string& marketType) {
    if (LIKELY(threadedQueueMapS_.count(marketType) and threadedQueueMapS_[marketType]->isStarted())) {
        threadedQueueMapS_[marketType]->push(data);
    } else {
        // optimization: log
    }
}

ConstantSP NsqQueues::getStatus() {
    vector<string> colNames {"topicType", START_TIME_STR, END_TIME_STR, FIRST_MSG_TIME_STR,
                             LAST_MSG_TIME_STR, PROCESSED_MSG_COUNT_STR, LAST_ERR_MSG_STR, FAILED_MSG_COUNT_STR,
                             LAST_FAILED_TIMESTAMP_STR};

    vector<ConstantSP> cols(colNames.size());
    vector<DATA_TYPE> dataTypes {DT_STRING, DT_NANOTIMESTAMP, DT_NANOTIMESTAMP, DT_NANOTIMESTAMP,
                                 DT_NANOTIMESTAMP, DT_LONG, DT_STRING, DT_LONG, DT_NANOTIMESTAMP};
    for (auto i = 0; i < (int)colNames.size(); i++) {
        cols[i] = Util::createVector(dataTypes[i], 0, 0);
    }

    for (auto &dataType : {TRADE, ENTRUST, SNAPSHOT}) {
        for (auto &marketType : {SH, SZ}) {
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

            appendString(col++, string("(") + dataType + ", " + marketType + ")");
            appendLong(col++, status.startTime_);
            appendLong(col++, status.endTime_);
            appendLong(col++, status.firstMsgTime_);
            appendLong(col++, status.lastMsgTime_);
            appendLong(col++, status.processedMsgCount_);
            appendString(col++, status.lastErrMsg_);
            appendLong(col++, status.failedMsgCount_);
            appendLong(col++, status.lastFailedTimestamp_);
        }
    }

    // orderTrade type status
    for (auto &marketType : {SH, SZ}) {
        StreamStatus status;
        if (threadedQueueMapTE_.count(marketType)) {
            for (auto it = threadedQueueMapTE_[marketType].begin(); it != threadedQueueMapTE_[marketType].end(); ++it) {
                int channelNo = it->first;
                status = it->second->getStatusConst();

                auto col = cols.begin();

                appendString(col++, string("(orderTrade") +  + ", " + marketType + ", channel " + std::to_string(channelNo) +")");
                appendLong(col++, status.startTime_);
                appendLong(col++, status.endTime_);
                appendLong(col++, status.firstMsgTime_);
                appendLong(col++, status.lastMsgTime_);
                appendLong(col++, status.processedMsgCount_);
                appendString(col++, status.lastErrMsg_);
                appendLong(col++, status.failedMsgCount_);
                appendLong(col++, status.lastFailedTimestamp_);
            }
        }
    }

    return Util::createTable(colNames, cols);
}

void NsqQueues::stop(const string &dataType, const string &marketType) {

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

vector<string> NsqQueues::getTypesToCancel(const string &dataType, const string &marketType) {

    vector<string> types;

    if (dataType == SNAPSHOT) {
        types.emplace_back(dataType);
    // if canceling tradeOrder, only cancel trade and/or order that is not subscribed
    } else if (dataType == nsqUtil::TRADE_ENTRUST) {
        if (threadedQueueMapT_.count(TRADE) == 0 or !threadedQueueMapT_[TRADE]->isStarted()) {
            types.emplace_back(TRADE);
        }
        if (threadedQueueMapE_.count(ENTRUST) == 0 or !threadedQueueMapE_[ENTRUST]->isStarted()) {
            types.emplace_back(ENTRUST);
        }
    // if canceling trade or order, only cancel it if tradeOrder is not subscribed
    } else {
        if (threadedQueueMapTE_.count(marketType)) {
            for (const auto &item : threadedQueueMapTE_[marketType]) {
                if (item.second->isStarted()) {
                    return {};
                }
            }
        }
        types.emplace_back(dataType);
    }

    return types;
}
