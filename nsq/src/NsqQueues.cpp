#include "NsqQueues.h"
#include "NsqUtil.h"
#include "ddb_nsq.h"

namespace {

void append(vector<ConstantSP> &cols, const std::string &topic, const StreamStatus &status)
{
    auto col = cols.begin();
    appendString(col++, topic);
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

void for_each_market(const std::function<void(nsq_market)> &func)
{
    for (int i = 0; i < static_cast<int>(nsq_market::unknown); ++i) {
        auto market = static_cast<nsq_market>(i);
        func(market);
    }
}

int marketRouteKey(nsq_market market)
{
    return static_cast<int>(market);
}

long long tradeEntrustRouteKey(nsq_market market, int channel)
{
    return (static_cast<long long>(marketRouteKey(market)) << 32) | static_cast<unsigned int>(channel);
}

} // namespace

using namespace nsqUtil;

void NsqQueues::initAndStart(Heap *heap, nsq_data data, nsq_market market, nsq_version version, const TableSP &table, long long queueDepth) {

    if (isSubscribed(data, market)) {
        throw RuntimeException(std::string(NSQ_PREFIX) + "subscription already exists"); // optimization: tradeAndOrder can be subscribed multiple times (to add channels)
    }

    if (data == nsq_data::TransactionTrade) {
        addThreadedQueue<nsqUtil::TradeDataStruct>(heap, data, market, table, tradeMeta, threadedQueueMapT_, tradeReader, queueDepth);
        tradeRoutes_.upsert(marketRouteKey(market), threadedQueueMapT_[market].get());
    } else if (data == nsq_data::TransactionEntrust) {
        if (version == nsq_version::v220105) {
            addThreadedQueue<nsqUtil::EntrustDataStruct>(heap, data, market, table, entrustMeta_220105, threadedQueueMapE_, entrustReader_220105, queueDepth);
        } else {
            addThreadedQueue<nsqUtil::EntrustDataStruct>(heap, data, market, table, entrustMeta, threadedQueueMapE_, entrustReader, queueDepth);
        }
        entrustRoutes_.upsert(marketRouteKey(market), threadedQueueMapE_[market].get());
    } else if (data == nsq_data::DepthMarket) {
        addThreadedQueue<nsqUtil::SnapshotDataStruct>(heap, data, market, table, marketTypes_.get("snapshot"), threadedQueueMapS_, snapshotReader_, queueDepth);
        snapshotRoutes_.upsert(marketRouteKey(market), threadedQueueMapS_[market].get());
    }
}

void NsqQueues::initAndStartTradeEntrust(Heap *heap, nsq_market marketType, int channel, const TableSP &table, long long queueDepth) {
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
    tradeEntrustRoutes_.upsert(tradeEntrustRouteKey(marketType, channel), threadedQueueMapTE_[marketType][channel].get());
}

bool NsqQueues::isSubscribed(nsq_data data, nsq_market market) {

    if (data == nsq_data::TransactionTrade && threadedQueueMapT_.count(market) && threadedQueueMapT_[market]->isStarted()) {
        return true;
    }
    if (data == nsq_data::TransactionEntrust && threadedQueueMapE_.count(market) && threadedQueueMapE_[market]->isStarted()) {
        return true;
    }
    if (data == nsq_data::DepthMarket && threadedQueueMapS_.count(market) && threadedQueueMapS_[market]->isStarted()) {
        return true;
    }
    if (data == nsq_data::orderbook && threadedQueueMapTE_.count(market)) {
        for (const auto& item : threadedQueueMapTE_[market]) {
            if (item.second->isStarted()) {
                return true;
            }
        }
    }

    return false;
}

template<typename DataStruct>
void NsqQueues::addThreadedQueue(Heap *heap, nsq_data data, nsq_market market, const TableSP &table,
                                 MetaTable meta, unordered_map<nsq_market, SmartPointer<ThreadedQueue<DataStruct>>, enum_hash> &map,
                                 std::function<void(vector<ConstantSP> &, DataStruct &)> reader, long long queueDepth) {

    if (map.count(market) == 0) {
        map[market] = new ThreadedQueue<DataStruct>(heap, 100, queueDepth, meta, nullptr, optionFlag_, to_string(data), NSQ_PREFIX, Util::BUF_SIZE, reader);
    }
    map[market]->setTable(table);
    map[market]->start();
}

void NsqQueues::pushData(CHSNsqSecuTransactionTradeDataField *data, nsq_market market) const {
    // trade
    long long reachTime = Util::getNanoEpochTime() + timeGap_;
    TradeQueue *queue = nullptr;
    if (LIKELY(tradeRoutes_.find(marketRouteKey(market), queue) && queue != nullptr)) {
        queue->push(
                {
                        reachTime,
                        *data
                }
        );
    }

    // tradeEntrust
    int channel = data->ChannelNo;
    TradeEntrustQueue *tradeEntrustQueue = nullptr;
    if (LIKELY(tradeEntrustRoutes_.find(tradeEntrustRouteKey(market, channel), tradeEntrustQueue) && tradeEntrustQueue != nullptr)) {
        tradeEntrustQueue->push(
                {
                        reachTime,
                        true,
                        *data,
                        {}
                }
        );
    }
}

void NsqQueues::pushData(CHSNsqSecuTransactionEntrustDataField *data, nsq_market market) const {
    long long reachTime = Util::getNanoEpochTime() + timeGap_;
    EntrustQueue *queue = nullptr;
    if (LIKELY(entrustRoutes_.find(marketRouteKey(market), queue) && queue != nullptr)) {
        queue->push(
                {
                        reachTime,
                        *data
                }
        );
    }
    // entrust

    // tradeEntrust
    int channel = data->ChannelNo;
    TradeEntrustQueue *tradeEntrustQueue = nullptr;
    if (LIKELY(tradeEntrustRoutes_.find(tradeEntrustRouteKey(market, channel), tradeEntrustQueue) && tradeEntrustQueue != nullptr)) {
        tradeEntrustQueue->push(
                {
                        reachTime,
                        false,
                        {},
                        *data
                }
        );
    }
}

void NsqQueues::pushData(const nsqUtil::SnapshotDataStruct& data, nsq_market market) const {
    SnapshotQueue *queue = nullptr;
    if (LIKELY(snapshotRoutes_.find(marketRouteKey(market), queue) && queue != nullptr)) {
        queue->push(data);
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
    for (auto i = 0; i < static_cast<int>(colNames.size()); i++) {
        cols[i] = Util::createVector(dataTypes[i], 0, 0);
    }

    for_each_market([this, &cols](nsq_market market) {
        auto queue = threadedQueueMapT_.find(market);
        if (queue == threadedQueueMapT_.end()) {
            return;
        }
        auto topic = string("(") + to_string(nsq_data::TransactionTrade) + ", " + to_string(market) + ")";
        append(cols, topic, queue->second->getStatusConst());
    });

    for_each_market([this, &cols](nsq_market market) {
        auto queue = threadedQueueMapE_.find(market);
        if (queue == threadedQueueMapE_.end()) {
            return;
        }
        auto topic = string("(") + to_string(nsq_data::TransactionEntrust) + ", " + to_string(market) + ")";
        append(cols, topic, queue->second->getStatusConst());
    });

    for_each_market([this, &cols](nsq_market market) {
        auto queue = threadedQueueMapS_.find(market);
        if (queue == threadedQueueMapS_.end()) {
            return;
        }
        auto topic = string("(") + to_string(nsq_data::DepthMarket) + ", " + to_string(market) + ")";
        append(cols, topic, queue->second->getStatusConst());
    });

    for_each_market([this, &cols](nsq_market market) {
        auto queues = threadedQueueMapTE_.find(market);
        if (queues == threadedQueueMapTE_.end()) {
            return;
        }
        for (const auto &queue : queues->second) {
            auto topic = string("(orderTrade") + ", " + to_string(market) + ", channel " + std::to_string(queue.first) +")";
            append(cols, topic, queue.second->getStatusConst());
        }
    });

    return Util::createTable(colNames, cols);
}

void NsqQueues::stop(nsq_data data, nsq_market market) {

    if (!isSubscribed(data, market)) {
        throw RuntimeException(std::string(NSQ_PREFIX) + "there is no subscription to cancel.");
    }

    if (data == nsq_data::TransactionTrade) {
        tradeRoutes_.upsert(marketRouteKey(market), nullptr);
        threadedQueueMapT_[market]->stop();
    } else if (data == nsq_data::TransactionEntrust) {
        entrustRoutes_.upsert(marketRouteKey(market), nullptr);
        threadedQueueMapE_[market]->stop();
    } else if (data == nsq_data::DepthMarket) {
        snapshotRoutes_.upsert(marketRouteKey(market), nullptr);
        threadedQueueMapS_[market]->stop();
    } else if (data == nsq_data::orderbook) {
        for (const auto &item : threadedQueueMapTE_[market]) {
            tradeEntrustRoutes_.upsert(tradeEntrustRouteKey(market, item.first), nullptr);
            item.second->stop();
        }
    }
}

void NsqQueues::setOptionFlag(int option) {

    optionFlag_ |= option;
}

void NsqQueues::addSnapshotExtra(nsq_version version) {

    // update meta
    auto snapshotMeta = marketTypes_.get(SNAPSHOT);
    snapshotMeta.colNames_.insert(snapshotMeta.colNames_.end(),
            snapshotMetaExtra.colNames_.begin(), snapshotMetaExtra.colNames_.end());
    snapshotMeta.colTypes_.insert(snapshotMeta.colTypes_.end(),
            snapshotMetaExtra.colTypes_.begin(), snapshotMetaExtra.colTypes_.end());
    if (version == nsq_version::v220105) {
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

ConstantSP NsqQueues::getSchema(nsq_data data, nsq_version version) {

    auto desc = to_string(data);
    if (version == nsq_version::v220105 && data == nsq_data::TransactionEntrust) {
        desc = nsqUtil::ENTRUST_220105;
    }
    TableSP schema = marketTypes_.getSchema(desc, optionFlag_);
    schema->setColumnName(1, "type");
    return schema;
}

std::pair<vector<nsq_data>, vector<nsq_data>> NsqQueues::getTypesToCancel(nsq_data data, nsq_market market)
{
    vector<nsq_data> typesForCancel;
    vector<nsq_data> typesForStop;

    if (data == nsq_data::DepthMarket) {
        typesForCancel.emplace_back(data);
    // if canceling tradeOrder, only cancel trade and/or order that is not subscribed
    } else if (data == nsq_data::orderbook) {
        if (threadedQueueMapT_.count(market) == 0 or !threadedQueueMapT_[market]->isStarted()) {
            typesForCancel.emplace_back(nsq_data::TransactionTrade);
        }
        if (threadedQueueMapE_.count(market) == 0 or !threadedQueueMapE_[market]->isStarted()) {
            typesForCancel.emplace_back(nsq_data::TransactionEntrust);
        }
    // if canceling trade or order, only cancel it if tradeOrder is not subscribed
    } else {
        bool started = false;
        if (threadedQueueMapTE_.count(market)) {
            for (const auto &item : threadedQueueMapTE_[market]) {
                if (item.second->isStarted()) {
                    started = true;
                }
            }
        }
        if (!started) {
            typesForCancel.emplace_back(data);
        }
    }
    typesForStop.emplace_back(data);

    return std::make_pair(typesForCancel, typesForStop);
}
