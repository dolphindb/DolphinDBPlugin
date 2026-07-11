#include "DolphinDBEverything.h"

#include "NsqQueues.h"

#include <chrono>
#include <cstring>
#include <limits>
#include <thread>
#include <unordered_map>

using namespace ddb;

namespace {

long long getPositiveLongArg(const vector<ConstantSP> &arguments, size_t index, long long defaultValue,
                             const string &name, const string &usage)
{
    if (arguments.size() <= index || arguments[index]->isNull()) {
        return defaultValue;
    }
    if (arguments[index]->getForm() != DF_SCALAR || arguments[index]->getCategory() != INTEGRAL) {
        throw IllegalArgumentException(__FUNCTION__, usage + name + " must be a positive integral scalar.");
    }
    long long value = arguments[index]->getLong();
    if (value <= 0) {
        throw IllegalArgumentException(__FUNCTION__, usage + name + " must be positive.");
    }
    return value;
}

string getStringArg(const vector<ConstantSP> &arguments, size_t index, const string &defaultValue, const string &name,
                    const string &usage)
{
    if (arguments.size() <= index || arguments[index]->isNull()) {
        return defaultValue;
    }
    if (arguments[index]->getForm() != DF_SCALAR || arguments[index]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, usage + name + " must be a string scalar.");
    }
    return arguments[index]->getString();
}

TableSP createOutputTable(const MetaTable &meta, long long capacity)
{
    INDEX tableCapacity =
        capacity > std::numeric_limits<INDEX>::max() ? std::numeric_limits<INDEX>::max() : static_cast<INDEX>(capacity);
    return Util::createTable(meta.colNames_, meta.colTypes_, 0, tableCapacity);
}

MetaTable tradeEntrustOutputMeta()
{
    MetaTable meta = nsqUtil::tradeEntrustMeta;
    meta.colNames_.push_back(nsqUtil::RECEIVED_TIME);
    meta.colTypes_.push_back(DT_NANOTIMESTAMP);
    return meta;
}

std::vector<nsq_market> getWorkerMarkets(long long threadCount)
{
    long long marketCount = static_cast<long long>(nsq_market::unknown);
    if (threadCount > marketCount) {
        throw IllegalArgumentException(__FUNCTION__, string(NSQ_PREFIX) + "threadCount must be no more than " +
                                                       std::to_string(marketCount) + ".");
    }
    // Non-orderTrade benchmarks use a distinct market per worker, so each worker
    // pushes into a distinct ThreadedQueue.
    std::vector<nsq_market> markets;
    markets.reserve(static_cast<size_t>(threadCount));
    for (long long i = 0; i < threadCount; ++i) {
        markets.push_back(static_cast<nsq_market>(i));
    }
    return markets;
}

std::vector<int> getWorkerChannels(long long threadCount)
{
    constexpr int firstChannel = 801;
    std::vector<int> channels;
    channels.reserve(static_cast<size_t>(threadCount));
    for (long long i = 0; i < threadCount; ++i) {
        channels.push_back(firstChannel + static_cast<int>(i));
    }
    return channels;
}

CHSNsqSecuTransactionTradeDataField makeTradeSample(int channel)
{
    CHSNsqSecuTransactionTradeDataField data;
    std::memset(&data, 0, sizeof(data));
    std::strncpy(data.ExchangeID, HS_EI_SSE, sizeof(data.ExchangeID) - 1);
    std::strncpy(data.InstrumentID, "600000", sizeof(data.InstrumentID) - 1);
    data.TransFlag = 0;
    data.SeqNo = 1;
    data.ChannelNo = channel;
    data.TradeDate = 20260527;
    data.TransactTime = 93000000;
    data.TrdPrice = 10.5;
    data.TrdVolume = 100;
    data.TrdMoney = 1050;
    data.TrdBuyNo = 10001;
    data.TrdSellNo = 10002;
    data.TrdBSFlag = 'B';
    data.BizIndex = 1;
    return data;
}

CHSNsqSecuTransactionEntrustDataField makeEntrustSample(int channel)
{
    CHSNsqSecuTransactionEntrustDataField data;
    std::memset(&data, 0, sizeof(data));
    std::strncpy(data.ExchangeID, HS_EI_SSE, sizeof(data.ExchangeID) - 1);
    std::strncpy(data.InstrumentID, "600000", sizeof(data.InstrumentID) - 1);
    data.TransFlag = 0;
    data.SeqNo = 1;
    data.ChannelNo = channel;
    data.TradeDate = 20260527;
    data.TransactTime = 93000000;
    data.OrdPrice = 10.5;
    data.OrdVolume = 100;
    data.OrdSide = 'B';
    data.OrdType = 'A';
    data.OrdNo = 10001;
    data.BizIndex = 1;
    return data;
}

nsqUtil::SnapshotDataStruct makeSnapshotSample()
{
    nsqUtil::SnapshotDataStruct data;
    std::memset(&data.data, 0, sizeof(data.data));
    data.reachTime = Util::getNanoEpochTime();
    std::strncpy(data.data.ExchangeID, HS_EI_SSE, sizeof(data.data.ExchangeID) - 1);
    std::strncpy(data.data.InstrumentID, "600000", sizeof(data.data.InstrumentID) - 1);
    data.data.LastPrice = 10.5;
    data.data.PreClosePrice = 10.0;
    data.data.OpenPrice = 10.1;
    data.data.HighPrice = 10.8;
    data.data.LowPrice = 10.0;
    data.data.ClosePrice = 10.5;
    data.data.UpperLimitPrice = 11.0;
    data.data.LowerLimitPrice = 9.0;
    data.data.TradeDate = 20260527;
    data.data.UpdateTime = 93000000;
    data.data.TradeVolume = 100000;
    data.data.TradeBalance = 1050000;
    data.data.AveragePrice = 10.5;
    for (int i = 0; i < 10; ++i) {
        data.data.BidPrice[i] = 10.5 - i * 0.01;
        data.data.AskPrice[i] = 10.5 + i * 0.01;
        data.data.BidVolume[i] = 1000 + i;
        data.data.AskVolume[i] = 1000 + i;
    }
    data.data.TradesNum = 100;
    data.data.InstrumentTradeStatus = 'T';
    data.data.TotalBidVolume = 10000;
    data.data.TotalAskVolume = 10000;
    data.data.MaBidPrice = 10.4;
    data.data.MaAskPrice = 10.6;
    data.Bid1Count = 0;
    data.MaxBid1Count = 0;
    data.Ask1Count = 0;
    data.MaxAsk1Count = 0;
    return data;
}

void waitRows(const TableSP &table, long long expectedRows)
{
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(30);
    while (table->rows() < expectedRows) {
        if (std::chrono::steady_clock::now() > deadline) {
            throw RuntimeException(std::string(NSQ_PREFIX) + "pipeline batch test timed out, rows[" +
                                   std::to_string(table->rows()) + "], expected[" + std::to_string(expectedRows) + "]");
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

struct PipelineBatchResult {
    string test;
    string dataType;
    long long batchSize;
    long long batchCount;
    long long totalRows;
    long long queueDepth;
    long long threadCount;
    long long elapsedNs;
    double nsPerRow;
    double rowsPerSecond;
    long long elapsedNsNoWait;
    double nsPerRowNoWait;
    double rowsPerSecondNoWait;
    long long rows;
};

void finishPipelineBatchResult(PipelineBatchResult &result)
{
    result.nsPerRow =
        result.rows == 0 ? 0.0 : static_cast<double>(result.elapsedNs) / static_cast<double>(result.rows);
    result.rowsPerSecond =
        result.elapsedNs == 0 ? 0.0 : static_cast<double>(result.rows) * 1000000000.0 / static_cast<double>(result.elapsedNs);
    result.nsPerRowNoWait =
        result.rows == 0 ? 0.0 : static_cast<double>(result.elapsedNsNoWait) / static_cast<double>(result.rows);
    result.rowsPerSecondNoWait =
        result.elapsedNsNoWait == 0 ? 0.0 : static_cast<double>(result.rows) * 1000000000.0 / static_cast<double>(result.elapsedNsNoWait);
}

using Clock = std::chrono::steady_clock;

struct CaseTiming {
    long long rows;
    long long elapsedNs;
    long long elapsedNsNoWait;
};

long long elapsedNsSince(const Clock::time_point &start, const Clock::time_point &end)
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

int routeMarketKey(nsq_market market)
{
    return static_cast<int>(market);
}

long long routeTradeEntrustKey(nsq_market market, int channel)
{
    return (static_cast<long long>(routeMarketKey(market)) << 32) | static_cast<unsigned int>(channel);
}

class NoLockPushDataQueues {
public:
    NoLockPushDataQueues()
    {
        long long time = Util::getNanoEpochTime();
        timeGap_ = Util::toLocalNanoTimestamp(time) - time;
    }

    void initAndStart(Heap *heap, nsq_data data, nsq_market market, nsq_version version, const TableSP &table,
                      long long queueDepth)
    {
        if (data == nsq_data::TransactionTrade) {
            addThreadedQueue(heap, data, market, table, nsqUtil::tradeMeta, threadedQueueMapT_,
                             nsqUtil::tradeReader, queueDepth);
        } else if (data == nsq_data::TransactionEntrust) {
            if (version == nsq_version::v220105) {
                addThreadedQueue(heap, data, market, table, nsqUtil::entrustMeta_220105, threadedQueueMapE_,
                                 nsqUtil::entrustReader_220105, queueDepth);
            } else {
                addThreadedQueue(heap, data, market, table, nsqUtil::entrustMeta, threadedQueueMapE_,
                                 nsqUtil::entrustReader, queueDepth);
            }
        } else if (data == nsq_data::DepthMarket) {
            auto snapshotReader = [](vector<ConstantSP> &buffer, nsqUtil::SnapshotDataStruct &data) {
                auto col = buffer.begin();
                nsqUtil::snapshotReaderConcise(col, data);
            };
            addThreadedQueue(heap, data, market, table, nsqUtil::snapshotMetaConcise, threadedQueueMapS_,
                             snapshotReader, queueDepth);
        }
    }

    void initAndStartTradeEntrust(Heap *heap, nsq_market market, int channel, const TableSP &table, long long queueDepth)
    {
        if (threadedQueueMapTE_.count(market) == 0) {
            threadedQueueMapTE_[market] = {};
        }
        if (threadedQueueMapTE_[market].count(channel) == 0) {
            threadedQueueMapTE_[market][channel] = new ThreadedQueue<nsqUtil::TradeEntrustDataStruct>(
                heap, 100, queueDepth, nsqUtil::tradeEntrustMeta, nullptr, OPT_RECEIVED, "tradeOrder", NSQ_PREFIX,
                Util::BUF_SIZE, nsqUtil::tradeEntrustReader);
        }
        threadedQueueMapTE_[market][channel]->setTable(table);
        threadedQueueMapTE_[market][channel]->start();
    }

    void pushData(CHSNsqSecuTransactionTradeDataField *data, nsq_market market) const
    {
        long long reachTime = Util::getNanoEpochTime() + timeGap_;
        auto iter = threadedQueueMapT_.find(market);
        if (LIKELY(iter != threadedQueueMapT_.end())) {
            iter->second->push({reachTime, *data});
        }

        int channel = data->ChannelNo;
        auto iterTE = threadedQueueMapTE_.find(market);
        if (LIKELY(iterTE != threadedQueueMapTE_.end())) {
            auto channelIter = iterTE->second.find(channel);
            if (LIKELY(channelIter != iterTE->second.end())) {
                channelIter->second->push({reachTime, true, *data, {}});
            }
        }
    }

    void pushData(CHSNsqSecuTransactionEntrustDataField *data, nsq_market market) const
    {
        long long reachTime = Util::getNanoEpochTime() + timeGap_;
        auto iter = threadedQueueMapE_.find(market);
        if (LIKELY(iter != threadedQueueMapE_.end())) {
            iter->second->push({reachTime, *data});
        }

        int channel = data->ChannelNo;
        auto iterTE = threadedQueueMapTE_.find(market);
        if (LIKELY(iterTE != threadedQueueMapTE_.end())) {
            auto channelIter = iterTE->second.find(channel);
            if (LIKELY(channelIter != iterTE->second.end())) {
                channelIter->second->push({reachTime, false, {}, *data});
            }
        }
    }

    void pushData(const nsqUtil::SnapshotDataStruct &data, nsq_market market) const
    {
        auto iter = threadedQueueMapS_.find(market);
        if (LIKELY(iter != threadedQueueMapS_.end())) {
            iter->second->push(data);
        }
    }

    void stop(nsq_data data, nsq_market market)
    {
        if (data == nsq_data::TransactionTrade && threadedQueueMapT_.count(market)) {
            threadedQueueMapT_[market]->stop();
        } else if (data == nsq_data::TransactionEntrust && threadedQueueMapE_.count(market)) {
            threadedQueueMapE_[market]->stop();
        } else if (data == nsq_data::DepthMarket && threadedQueueMapS_.count(market)) {
            threadedQueueMapS_[market]->stop();
        } else if (data == nsq_data::orderbook && threadedQueueMapTE_.count(market)) {
            for (const auto &item : threadedQueueMapTE_[market]) {
                item.second->stop();
            }
        }
    }

private:
    template <typename DataStruct, typename Reader>
    void addThreadedQueue(Heap *heap, nsq_data data, nsq_market market, const TableSP &table, MetaTable meta,
                          unordered_map<nsq_market, SmartPointer<ThreadedQueue<DataStruct>>> &map,
                          Reader reader, long long queueDepth)
    {
        if (map.count(market) == 0) {
            map[market] = new ThreadedQueue<DataStruct>(heap, 100, queueDepth, meta, nullptr, 0, to_string(data),
                                                        NSQ_PREFIX, Util::BUF_SIZE, reader);
        }
        map[market]->setTable(table);
        map[market]->start();
    }

    long long timeGap_ = 0;
    unordered_map<nsq_market, SmartPointer<ThreadedQueue<nsqUtil::TradeDataStruct>>> threadedQueueMapT_;
    unordered_map<nsq_market, SmartPointer<ThreadedQueue<nsqUtil::EntrustDataStruct>>> threadedQueueMapE_;
    unordered_map<nsq_market, SmartPointer<ThreadedQueue<nsqUtil::SnapshotDataStruct>>> threadedQueueMapS_;
    unordered_map<nsq_market, unordered_map<int, SmartPointer<ThreadedQueue<nsqUtil::TradeEntrustDataStruct>>>>
        threadedQueueMapTE_;
};

template <typename Func>
void runWithWorkers(long long totalRows, long long workerCount, Func &&func)
{
    std::vector<std::thread> workers;
    workers.reserve(static_cast<size_t>(workerCount));
    for (long long i = 0; i < workerCount; ++i) {
        long long start = totalRows * i / workerCount;
        long long end = totalRows * (i + 1) / workerCount;
        workers.emplace_back([&, i, start, end]() {
            func(i, start, end);
        });
    }
    for (auto &worker : workers) {
        worker.join();
    }
}

template <typename Func>
PipelineBatchResult runBatchCase(const string &test, const string &dataType, long long batchSize, long long batchCount,
                                 long long queueDepth, long long threadCount, Func &&func)
{
    long long totalRows = batchSize * batchCount;
    PipelineBatchResult result{
        test, dataType, batchSize, batchCount, totalRows, queueDepth, threadCount, 0, 0.0, 0.0, 0, 0.0, 0.0, 0};
    auto start = Clock::now();
    CaseTiming timing = func(totalRows, start);
    result.rows = timing.rows;
    result.elapsedNs = timing.elapsedNs;
    result.elapsedNsNoWait = timing.elapsedNsNoWait;
    finishPipelineBatchResult(result);
    return result;
}

PipelineBatchResult runOldNoLockTradePushDataCase(const string &dataType, Heap *heap,
                                                  const std::vector<nsq_market> &markets, long long batchSize,
                                                  long long batchCount, long long queueDepth, long long threadCount)
{
    return runBatchCase("old_no_lock_pushData_dispatch", dataType, batchSize, batchCount, queueDepth, threadCount,
                        [&](long long expectedRows, const Clock::time_point &start) {
                            std::vector<TableSP> tables;
                            tables.reserve(static_cast<size_t>(threadCount));
                            NoLockPushDataQueues queues;
                            for (long long i = 0; i < threadCount; ++i) {
                                long long rows = expectedRows * (i + 1) / threadCount - expectedRows * i / threadCount;
                                tables.push_back(createOutputTable(nsqUtil::tradeMeta, rows));
                                queues.initAndStart(heap, nsq_data::TransactionTrade, markets[i], nsq_version::origin,
                                                    tables.back(), queueDepth);
                            }
                            runWithWorkers(expectedRows, threadCount,
                                           [&](long long workerIndex, long long start, long long end) {
                                               auto sample = makeTradeSample(801);
                                               for (long long i = start; i < end; ++i) {
                                                   sample.SeqNo = i + 1;
                                                   sample.BizIndex = i + 1;
                                                   queues.pushData(&sample, markets[workerIndex]);
                                               }
                                           });
                            auto noWaitEnd = Clock::now();
                            for (long long i = 0; i < threadCount; ++i) {
                                long long rows = expectedRows * (i + 1) / threadCount - expectedRows * i / threadCount;
                                waitRows(tables[i], rows);
                            }
                            auto waitEnd = Clock::now();
                            long long rows = 0;
                            for (long long i = 0; i < threadCount; ++i) {
                                rows += tables[i]->rows();
                                queues.stop(nsq_data::TransactionTrade, markets[i]);
                            }
                            return CaseTiming{rows, elapsedNsSince(start, waitEnd), elapsedNsSince(start, noWaitEnd)};
                        });
}

PipelineBatchResult runOldNoLockEntrustPushDataCase(const string &dataType, Heap *heap,
                                                    const std::vector<nsq_market> &markets, long long batchSize,
                                                    long long batchCount, long long queueDepth, long long threadCount)
{
    return runBatchCase("old_no_lock_pushData_dispatch", dataType, batchSize, batchCount, queueDepth, threadCount,
                        [&](long long expectedRows, const Clock::time_point &start) {
                            std::vector<TableSP> tables;
                            tables.reserve(static_cast<size_t>(threadCount));
                            NoLockPushDataQueues queues;
                            for (long long i = 0; i < threadCount; ++i) {
                                long long rows = expectedRows * (i + 1) / threadCount - expectedRows * i / threadCount;
                                tables.push_back(createOutputTable(nsqUtil::entrustMeta, rows));
                                queues.initAndStart(heap, nsq_data::TransactionEntrust, markets[i], nsq_version::origin,
                                                    tables.back(), queueDepth);
                            }
                            runWithWorkers(expectedRows, threadCount,
                                           [&](long long workerIndex, long long start, long long end) {
                                               auto sample = makeEntrustSample(801);
                                               for (long long i = start; i < end; ++i) {
                                                   sample.SeqNo = i + 1;
                                                   sample.BizIndex = i + 1;
                                                   queues.pushData(&sample, markets[workerIndex]);
                                               }
                                           });
                            auto noWaitEnd = Clock::now();
                            for (long long i = 0; i < threadCount; ++i) {
                                long long rows = expectedRows * (i + 1) / threadCount - expectedRows * i / threadCount;
                                waitRows(tables[i], rows);
                            }
                            auto waitEnd = Clock::now();
                            long long rows = 0;
                            for (long long i = 0; i < threadCount; ++i) {
                                rows += tables[i]->rows();
                                queues.stop(nsq_data::TransactionEntrust, markets[i]);
                            }
                            return CaseTiming{rows, elapsedNsSince(start, waitEnd), elapsedNsSince(start, noWaitEnd)};
                        });
}

PipelineBatchResult runOldNoLockSnapshotPushDataCase(const string &dataType, Heap *heap,
                                                     const std::vector<nsq_market> &markets, long long batchSize,
                                                     long long batchCount, long long queueDepth, long long threadCount)
{
    return runBatchCase("old_no_lock_pushData_dispatch", dataType, batchSize, batchCount, queueDepth, threadCount,
                        [&](long long expectedRows, const Clock::time_point &start) {
                            std::vector<TableSP> tables;
                            tables.reserve(static_cast<size_t>(threadCount));
                            NoLockPushDataQueues queues;
                            for (long long i = 0; i < threadCount; ++i) {
                                long long rows = expectedRows * (i + 1) / threadCount - expectedRows * i / threadCount;
                                tables.push_back(createOutputTable(nsqUtil::snapshotMetaConcise, rows));
                                queues.initAndStart(heap, nsq_data::DepthMarket, markets[i], nsq_version::origin,
                                                    tables.back(), queueDepth);
                            }
                            runWithWorkers(expectedRows, threadCount,
                                           [&](long long workerIndex, long long start, long long end) {
                                               auto sample = makeSnapshotSample();
                                               for (long long i = start; i < end; ++i) {
                                                   sample.reachTime = Util::getNanoEpochTime();
                                                   sample.data.TradesNum = i + 1;
                                                   queues.pushData(sample, markets[workerIndex]);
                                               }
                                           });
                            auto noWaitEnd = Clock::now();
                            for (long long i = 0; i < threadCount; ++i) {
                                long long rows = expectedRows * (i + 1) / threadCount - expectedRows * i / threadCount;
                                waitRows(tables[i], rows);
                            }
                            auto waitEnd = Clock::now();
                            long long rows = 0;
                            for (long long i = 0; i < threadCount; ++i) {
                                rows += tables[i]->rows();
                                queues.stop(nsq_data::DepthMarket, markets[i]);
                            }
                            return CaseTiming{rows, elapsedNsSince(start, waitEnd), elapsedNsSince(start, noWaitEnd)};
                        });
}

PipelineBatchResult runOldNoLockOrderTradePushDataCase(const string &dataType, Heap *heap, nsq_market market,
                                                       const std::vector<int> &channels, long long batchSize,
                                                       long long batchCount, long long queueDepth,
                                                       long long threadCount)
{
    return runBatchCase("old_no_lock_pushData_dispatch", dataType, batchSize, batchCount, queueDepth, threadCount,
                        [&](long long expectedRows, const Clock::time_point &start) {
                            MetaTable outputMeta = tradeEntrustOutputMeta();
                            std::vector<TableSP> tables;
                            tables.reserve(static_cast<size_t>(threadCount));
                            NoLockPushDataQueues queues;
                            for (long long i = 0; i < threadCount; ++i) {
                                long long rows = expectedRows * (i + 1) / threadCount - expectedRows * i / threadCount;
                                tables.push_back(createOutputTable(outputMeta, rows));
                                queues.initAndStartTradeEntrust(heap, market, channels[i], tables.back(), queueDepth);
                            }
                            runWithWorkers(expectedRows, threadCount,
                                           [&](long long workerIndex, long long start, long long end) {
                                               int workerChannel = channels[workerIndex];
                                               auto tradeSample = makeTradeSample(workerChannel);
                                               auto entrustSample = makeEntrustSample(workerChannel);
                                               for (long long i = start; i < end; ++i) {
                                                   if ((i & 1) == 0) {
                                                       tradeSample.SeqNo = i + 1;
                                                       tradeSample.BizIndex = i + 1;
                                                       queues.pushData(&tradeSample, market);
                                                   } else {
                                                       entrustSample.SeqNo = i + 1;
                                                       entrustSample.BizIndex = i + 1;
                                                       queues.pushData(&entrustSample, market);
                                                   }
                                               }
                                           });
                            auto noWaitEnd = Clock::now();
                            for (long long i = 0; i < threadCount; ++i) {
                                long long rows = expectedRows * (i + 1) / threadCount - expectedRows * i / threadCount;
                                waitRows(tables[i], rows);
                            }
                            auto waitEnd = Clock::now();
                            long long rows = 0;
                            for (long long i = 0; i < threadCount; ++i) {
                                rows += tables[i]->rows();
                            }
                            queues.stop(nsq_data::orderbook, market);
                            return CaseTiming{rows, elapsedNsSince(start, waitEnd), elapsedNsSince(start, noWaitEnd)};
                        });
}

template <typename RouteMap>
PipelineBatchResult runTradeRouteMapCase(const string &test, const string &dataType, Heap *heap,
                                         const std::vector<nsq_market> &markets, long long batchSize,
                                         long long batchCount, long long queueDepth, long long threadCount)
{
    return runBatchCase(test, dataType, batchSize, batchCount, queueDepth, threadCount,
                        [&](long long expectedRows, const Clock::time_point &start) {
                            std::vector<TableSP> tables;
                            tables.reserve(static_cast<size_t>(threadCount));
                            unordered_map<nsq_market, SmartPointer<ThreadedQueue<nsqUtil::TradeDataStruct>>> queues;
                            RouteMap routes;
                            for (long long i = 0; i < threadCount; ++i) {
                                long long rows = expectedRows * (i + 1) / threadCount - expectedRows * i / threadCount;
                                tables.push_back(createOutputTable(nsqUtil::tradeMeta, rows));
                                queues[markets[i]] = new ThreadedQueue<nsqUtil::TradeDataStruct>(
                                    heap, 100, queueDepth, nsqUtil::tradeMeta, nullptr, 0, "trade",
                                    NSQ_PREFIX, Util::BUF_SIZE, nsqUtil::tradeReader);
                                queues[markets[i]]->setTable(tables.back());
                                queues[markets[i]]->start();
                                routes.insert(routeMarketKey(markets[i]), queues[markets[i]].get());
                            }
                            runWithWorkers(expectedRows, threadCount, [&](long long workerIndex, long long start, long long end) {
                                auto sample = makeTradeSample(801);
                                for (long long i = start; i < end; ++i) {
                                    sample.SeqNo = i + 1;
                                    sample.BizIndex = i + 1;
                                    ThreadedQueue<nsqUtil::TradeDataStruct> *queue = nullptr;
                                    if (routes.find(routeMarketKey(markets[workerIndex]), queue) && queue != nullptr) {
                                        queue->push({Util::getNanoEpochTime(), sample});
                                    }
                                }
                            });
                            auto noWaitEnd = Clock::now();
                            for (long long i = 0; i < threadCount; ++i) {
                                long long rows = expectedRows * (i + 1) / threadCount - expectedRows * i / threadCount;
                                waitRows(tables[i], rows);
                            }
                            auto waitEnd = Clock::now();
                            long long rows = 0;
                            for (long long i = 0; i < threadCount; ++i) {
                                rows += tables[i]->rows();
                                queues[markets[i]]->stop();
                            }
                            return CaseTiming{rows, elapsedNsSince(start, waitEnd), elapsedNsSince(start, noWaitEnd)};
                        });
}

template <typename RouteMap>
PipelineBatchResult runEntrustRouteMapCase(const string &test, const string &dataType, Heap *heap,
                                           const std::vector<nsq_market> &markets, long long batchSize,
                                           long long batchCount, long long queueDepth, long long threadCount)
{
    return runBatchCase(test, dataType, batchSize, batchCount, queueDepth, threadCount,
                        [&](long long expectedRows, const Clock::time_point &start) {
                            std::vector<TableSP> tables;
                            tables.reserve(static_cast<size_t>(threadCount));
                            unordered_map<nsq_market, SmartPointer<ThreadedQueue<nsqUtil::EntrustDataStruct>>> queues;
                            RouteMap routes;
                            for (long long i = 0; i < threadCount; ++i) {
                                long long rows = expectedRows * (i + 1) / threadCount - expectedRows * i / threadCount;
                                tables.push_back(createOutputTable(nsqUtil::entrustMeta, rows));
                                queues[markets[i]] = new ThreadedQueue<nsqUtil::EntrustDataStruct>(
                                    heap, 100, queueDepth, nsqUtil::entrustMeta, nullptr, 0, "orders",
                                    NSQ_PREFIX, Util::BUF_SIZE, nsqUtil::entrustReader);
                                queues[markets[i]]->setTable(tables.back());
                                queues[markets[i]]->start();
                                routes.insert(routeMarketKey(markets[i]), queues[markets[i]].get());
                            }
                            runWithWorkers(expectedRows, threadCount, [&](long long workerIndex, long long start, long long end) {
                                auto sample = makeEntrustSample(801);
                                for (long long i = start; i < end; ++i) {
                                    sample.SeqNo = i + 1;
                                    sample.BizIndex = i + 1;
                                    ThreadedQueue<nsqUtil::EntrustDataStruct> *queue = nullptr;
                                    if (routes.find(routeMarketKey(markets[workerIndex]), queue) && queue != nullptr) {
                                        queue->push({Util::getNanoEpochTime(), sample});
                                    }
                                }
                            });
                            auto noWaitEnd = Clock::now();
                            for (long long i = 0; i < threadCount; ++i) {
                                long long rows = expectedRows * (i + 1) / threadCount - expectedRows * i / threadCount;
                                waitRows(tables[i], rows);
                            }
                            auto waitEnd = Clock::now();
                            long long rows = 0;
                            for (long long i = 0; i < threadCount; ++i) {
                                rows += tables[i]->rows();
                                queues[markets[i]]->stop();
                            }
                            return CaseTiming{rows, elapsedNsSince(start, waitEnd), elapsedNsSince(start, noWaitEnd)};
                        });
}

template <typename RouteMap>
PipelineBatchResult runSnapshotRouteMapCase(const string &test, const string &dataType, Heap *heap,
                                            const std::vector<nsq_market> &markets, long long batchSize,
                                            long long batchCount, long long queueDepth, long long threadCount)
{
    return runBatchCase(test, dataType, batchSize, batchCount, queueDepth, threadCount,
                        [&](long long expectedRows, const Clock::time_point &start) {
                            std::vector<TableSP> tables;
                            tables.reserve(static_cast<size_t>(threadCount));
                            auto snapshotReader = [](vector<ConstantSP> &buffer,
                                                     nsqUtil::SnapshotDataStruct &data) {
                                auto col = buffer.begin();
                                nsqUtil::snapshotReaderConcise(col, data);
                            };
                            unordered_map<nsq_market, SmartPointer<ThreadedQueue<nsqUtil::SnapshotDataStruct>>> queues;
                            RouteMap routes;
                            for (long long i = 0; i < threadCount; ++i) {
                                long long rows = expectedRows * (i + 1) / threadCount - expectedRows * i / threadCount;
                                tables.push_back(createOutputTable(nsqUtil::snapshotMetaConcise, rows));
                                queues[markets[i]] = new ThreadedQueue<nsqUtil::SnapshotDataStruct>(
                                    heap, 100, queueDepth, nsqUtil::snapshotMetaConcise, nullptr, 0,
                                    "snapshot", NSQ_PREFIX, Util::BUF_SIZE, snapshotReader);
                                queues[markets[i]]->setTable(tables.back());
                                queues[markets[i]]->start();
                                routes.insert(routeMarketKey(markets[i]), queues[markets[i]].get());
                            }
                            runWithWorkers(expectedRows, threadCount, [&](long long workerIndex, long long start, long long end) {
                                auto sample = makeSnapshotSample();
                                for (long long i = start; i < end; ++i) {
                                    sample.reachTime = Util::getNanoEpochTime();
                                    sample.data.TradesNum = i + 1;
                                    ThreadedQueue<nsqUtil::SnapshotDataStruct> *queue = nullptr;
                                    if (routes.find(routeMarketKey(markets[workerIndex]), queue) && queue != nullptr) {
                                        queue->push(sample);
                                    }
                                }
                            });
                            auto noWaitEnd = Clock::now();
                            for (long long i = 0; i < threadCount; ++i) {
                                long long rows = expectedRows * (i + 1) / threadCount - expectedRows * i / threadCount;
                                waitRows(tables[i], rows);
                            }
                            auto waitEnd = Clock::now();
                            long long rows = 0;
                            for (long long i = 0; i < threadCount; ++i) {
                                rows += tables[i]->rows();
                                queues[markets[i]]->stop();
                            }
                            return CaseTiming{rows, elapsedNsSince(start, waitEnd), elapsedNsSince(start, noWaitEnd)};
                        });
}

template <typename RouteMap>
PipelineBatchResult runOrderTradeRouteMapCase(const string &test, const string &dataType, Heap *heap,
                                              nsq_market market, const std::vector<int> &channels, long long batchSize,
                                              long long batchCount, long long queueDepth, long long threadCount)
{
    return runBatchCase(test, dataType, batchSize, batchCount, queueDepth, threadCount,
                        [&](long long expectedRows, const Clock::time_point &start) {
                            MetaTable outputMeta = tradeEntrustOutputMeta();
                            std::vector<TableSP> tables;
                            tables.reserve(static_cast<size_t>(threadCount));
                            std::vector<SmartPointer<ThreadedQueue<nsqUtil::TradeEntrustDataStruct>>> queues;
                            queues.reserve(static_cast<size_t>(threadCount));
                            RouteMap routes;
                            for (long long i = 0; i < threadCount; ++i) {
                                long long rows = expectedRows * (i + 1) / threadCount - expectedRows * i / threadCount;
                                tables.push_back(createOutputTable(outputMeta, rows));
                                queues.push_back(new ThreadedQueue<nsqUtil::TradeEntrustDataStruct>(
                                    heap, 100, queueDepth, nsqUtil::tradeEntrustMeta, nullptr, OPT_RECEIVED,
                                    "tradeOrder", NSQ_PREFIX, Util::BUF_SIZE, nsqUtil::tradeEntrustReader));
                                queues.back()->setTable(tables.back());
                                queues.back()->start();
                                routes.insert(routeTradeEntrustKey(market, channels[i]), queues.back().get());
                            }
                            runWithWorkers(expectedRows, threadCount, [&](long long workerIndex, long long start, long long end) {
                                int channel = channels[workerIndex];
                                auto tradeSample = makeTradeSample(channel);
                                auto entrustSample = makeEntrustSample(channel);
                                for (long long i = start; i < end; ++i) {
                                    ThreadedQueue<nsqUtil::TradeEntrustDataStruct> *queue = nullptr;
                                    if (!routes.find(routeTradeEntrustKey(market, channel), queue) || queue == nullptr) {
                                        continue;
                                    }
                                    if ((i & 1) == 0) {
                                        tradeSample.SeqNo = i + 1;
                                        tradeSample.BizIndex = i + 1;
                                        queue->push({Util::getNanoEpochTime(), true, tradeSample, {}});
                                    } else {
                                        entrustSample.SeqNo = i + 1;
                                        entrustSample.BizIndex = i + 1;
                                        queue->push({Util::getNanoEpochTime(), false, {}, entrustSample});
                                    }
                                }
                            });
                            auto noWaitEnd = Clock::now();
                            for (long long i = 0; i < threadCount; ++i) {
                                long long rows = expectedRows * (i + 1) / threadCount - expectedRows * i / threadCount;
                                waitRows(tables[i], rows);
                            }
                            auto waitEnd = Clock::now();
                            long long rows = 0;
                            for (long long i = 0; i < threadCount; ++i) {
                                rows += tables[i]->rows();
                                queues[i]->stop();
                            }
                            return CaseTiming{rows, elapsedNsSince(start, waitEnd), elapsedNsSince(start, noWaitEnd)};
                        });
}

} // namespace

extern "C" ConstantSP nsqTestPipelineBatchPerformance(Heap *heap, vector<ConstantSP> &arguments)
{
    string usage =
        "testPipelineBatchPerformance([batchSize=2], [batchCount=50], [dataType='trade'], [queueDepth=1000000], [threadCount=3], [mode='all']): ";

    long long batchSize = getPositiveLongArg(arguments, 0, 2, "batchSize", usage);
    long long batchCount = getPositiveLongArg(arguments, 1, 50, "batchCount", usage);
    string dataType = getStringArg(arguments, 2, "trade", "dataType", usage);
    long long queueDepth = getPositiveLongArg(arguments, 3, QUEUE_DEPTH, "queueDepth", usage);
    long long threadCount = getPositiveLongArg(arguments, 4, 3, "threadCount", usage);
    string mode = getStringArg(arguments, 5, "all", "mode", usage);
    if (mode != "all" && mode != "pushDataOnly") {
        throw IllegalArgumentException(__FUNCTION__, usage + "mode must be 'all' or 'pushDataOnly'.");
    }
    bool pushDataOnly = mode == "pushDataOnly";
    if (batchSize > std::numeric_limits<long long>::max() / batchCount) {
        throw IllegalArgumentException(__FUNCTION__, usage + "batchSize * batchCount is too large.");
    }
    constexpr int channel = 801;
    std::vector<nsq_market> markets = getWorkerMarkets(threadCount);
    std::vector<int> channels = getWorkerChannels(threadCount);

    vector<PipelineBatchResult> results;

    if (dataType == "trade") {
        results.push_back(runBatchCase("lockless_map_dispatch", dataType, batchSize, batchCount, queueDepth, threadCount,
                                       [&](long long expectedRows, const Clock::time_point &start) {
                                           std::vector<TableSP> tables;
                                           tables.reserve(static_cast<size_t>(threadCount));
                                           NsqQueues queues;
                                           for (long long i = 0; i < threadCount; ++i) {
                                               long long rows = expectedRows * (i + 1) / threadCount - expectedRows * i / threadCount;
                                               tables.push_back(createOutputTable(nsqUtil::tradeMeta, rows));
                                               queues.initAndStart(heap, nsq_data::TransactionTrade, markets[i],
                                                                   nsq_version::origin, tables.back(), queueDepth);
                                           }
                                           runWithWorkers(expectedRows, threadCount, [&](long long workerIndex, long long start, long long end) {
                                               auto sample = makeTradeSample(channel);
                                               for (long long i = start; i < end; ++i) {
                                                   sample.SeqNo = i + 1;
                                                   sample.BizIndex = i + 1;
                                                   queues.pushData(&sample, markets[workerIndex]);
                                               }
                                           });
                                           auto noWaitEnd = Clock::now();
                                           for (long long i = 0; i < threadCount; ++i) {
                                               long long rows = expectedRows * (i + 1) / threadCount - expectedRows * i / threadCount;
                                               waitRows(tables[i], rows);
                                           }
                                           auto waitEnd = Clock::now();
                                           long long rows = 0;
                                           for (long long i = 0; i < threadCount; ++i) {
                                               rows += tables[i]->rows();
                                               queues.stop(nsq_data::TransactionTrade, markets[i]);
                                           }
                                           return CaseTiming{rows, elapsedNsSince(start, waitEnd), elapsedNsSince(start, noWaitEnd)};
                                       }));
        if (pushDataOnly) {
            goto build_result_table;
        }
        results.push_back(runOldNoLockTradePushDataCase(dataType, heap, markets, batchSize, batchCount, queueDepth,
                                                        threadCount));

        results.push_back(runTradeRouteMapCase<LocklessFlatHashmap<int, ThreadedQueue<nsqUtil::TradeDataStruct> *>>(
            "direct_lockless_flat_hashmap", dataType, heap, markets, batchSize, batchCount, queueDepth, threadCount));
        results.push_back(runTradeRouteMapCase<IrremovableLocklessFlatHashmap<int, ThreadedQueue<nsqUtil::TradeDataStruct> *>>(
            "direct_irremovable_lockless_flat_hashmap", dataType, heap, markets, batchSize, batchCount, queueDepth, threadCount));
        results.push_back(runTradeRouteMapCase<LocklessHashmap<int, ThreadedQueue<nsqUtil::TradeDataStruct> *>>(
            "direct_lockless_hashmap", dataType, heap, markets, batchSize, batchCount, queueDepth, threadCount));
        results.push_back(runTradeRouteMapCase<IrremovableLocklessHashmap<int, ThreadedQueue<nsqUtil::TradeDataStruct> *>>(
            "direct_irremovable_lockless_hashmap", dataType, heap, markets, batchSize, batchCount, queueDepth, threadCount));

        results.push_back(runBatchCase("no_lock_map_dispatch", dataType, batchSize, batchCount, queueDepth, threadCount,
                                       [&](long long expectedRows, const Clock::time_point &start) {
                                           std::vector<TableSP> tables;
                                           tables.reserve(static_cast<size_t>(threadCount));
                                           unordered_map<nsq_market, SmartPointer<ThreadedQueue<nsqUtil::TradeDataStruct>>> queues;
                                           for (long long i = 0; i < threadCount; ++i) {
                                               long long rows = expectedRows * (i + 1) / threadCount - expectedRows * i / threadCount;
                                               tables.push_back(createOutputTable(nsqUtil::tradeMeta, rows));
                                               queues[markets[i]] = new ThreadedQueue<nsqUtil::TradeDataStruct>(
                                                   heap, 100, queueDepth, nsqUtil::tradeMeta, nullptr, 0, "trade",
                                                   NSQ_PREFIX, Util::BUF_SIZE, nsqUtil::tradeReader);
                                               queues[markets[i]]->setTable(tables.back());
                                               queues[markets[i]]->start();
                                           }
                                           const auto &queueMap = queues;
                                           runWithWorkers(expectedRows, threadCount, [&](long long workerIndex, long long start, long long end) {
                                               auto sample = makeTradeSample(channel);
                                               for (long long i = start; i < end; ++i) {
                                                   sample.SeqNo = i + 1;
                                                   sample.BizIndex = i + 1;
                                                   auto iter = queueMap.find(markets[workerIndex]);
                                                   if (iter != queueMap.end()) {
                                                       iter->second->push({Util::getNanoEpochTime(), sample});
                                                   }
                                               }
                                           });
                                           auto noWaitEnd = Clock::now();
                                           for (long long i = 0; i < threadCount; ++i) {
                                               long long rows = expectedRows * (i + 1) / threadCount - expectedRows * i / threadCount;
                                               waitRows(tables[i], rows);
                                           }
                                           auto waitEnd = Clock::now();
                                           long long rows = 0;
                                           for (long long i = 0; i < threadCount; ++i) {
                                               rows += tables[i]->rows();
                                               queues[markets[i]]->stop();
                                           }
                                           return CaseTiming{rows, elapsedNsSince(start, waitEnd), elapsedNsSince(start, noWaitEnd)};
                                       }));
    } else if (dataType == "orders") {
        results.push_back(runBatchCase("lockless_map_dispatch", dataType, batchSize, batchCount, queueDepth, threadCount,
                                       [&](long long expectedRows, const Clock::time_point &start) {
                                           std::vector<TableSP> tables;
                                           tables.reserve(static_cast<size_t>(threadCount));
                                           NsqQueues queues;
                                           for (long long i = 0; i < threadCount; ++i) {
                                               long long rows = expectedRows * (i + 1) / threadCount - expectedRows * i / threadCount;
                                               tables.push_back(createOutputTable(nsqUtil::entrustMeta, rows));
                                               queues.initAndStart(heap, nsq_data::TransactionEntrust, markets[i],
                                                                   nsq_version::origin, tables.back(), queueDepth);
                                           }
                                           runWithWorkers(expectedRows, threadCount, [&](long long workerIndex, long long start, long long end) {
                                               auto sample = makeEntrustSample(channel);
                                               for (long long i = start; i < end; ++i) {
                                                   sample.SeqNo = i + 1;
                                                   sample.BizIndex = i + 1;
                                                   queues.pushData(&sample, markets[workerIndex]);
                                               }
                                           });
                                           auto noWaitEnd = Clock::now();
                                           for (long long i = 0; i < threadCount; ++i) {
                                               long long rows = expectedRows * (i + 1) / threadCount - expectedRows * i / threadCount;
                                               waitRows(tables[i], rows);
                                           }
                                           auto waitEnd = Clock::now();
                                           long long rows = 0;
                                           for (long long i = 0; i < threadCount; ++i) {
                                               rows += tables[i]->rows();
                                               queues.stop(nsq_data::TransactionEntrust, markets[i]);
                                           }
                                           return CaseTiming{rows, elapsedNsSince(start, waitEnd), elapsedNsSince(start, noWaitEnd)};
                                       }));
        if (pushDataOnly) {
            goto build_result_table;
        }
        results.push_back(runOldNoLockEntrustPushDataCase(dataType, heap, markets, batchSize, batchCount, queueDepth,
                                                          threadCount));

        results.push_back(runEntrustRouteMapCase<LocklessFlatHashmap<int, ThreadedQueue<nsqUtil::EntrustDataStruct> *>>(
            "direct_lockless_flat_hashmap", dataType, heap, markets, batchSize, batchCount, queueDepth, threadCount));
        results.push_back(runEntrustRouteMapCase<IrremovableLocklessFlatHashmap<int, ThreadedQueue<nsqUtil::EntrustDataStruct> *>>(
            "direct_irremovable_lockless_flat_hashmap", dataType, heap, markets, batchSize, batchCount, queueDepth, threadCount));
        results.push_back(runEntrustRouteMapCase<LocklessHashmap<int, ThreadedQueue<nsqUtil::EntrustDataStruct> *>>(
            "direct_lockless_hashmap", dataType, heap, markets, batchSize, batchCount, queueDepth, threadCount));
        results.push_back(runEntrustRouteMapCase<IrremovableLocklessHashmap<int, ThreadedQueue<nsqUtil::EntrustDataStruct> *>>(
            "direct_irremovable_lockless_hashmap", dataType, heap, markets, batchSize, batchCount, queueDepth, threadCount));

        results.push_back(runBatchCase("no_lock_map_dispatch", dataType, batchSize, batchCount, queueDepth, threadCount,
                                       [&](long long expectedRows, const Clock::time_point &start) {
                                           std::vector<TableSP> tables;
                                           tables.reserve(static_cast<size_t>(threadCount));
                                           unordered_map<nsq_market, SmartPointer<ThreadedQueue<nsqUtil::EntrustDataStruct>>> queues;
                                           for (long long i = 0; i < threadCount; ++i) {
                                               long long rows = expectedRows * (i + 1) / threadCount - expectedRows * i / threadCount;
                                               tables.push_back(createOutputTable(nsqUtil::entrustMeta, rows));
                                               queues[markets[i]] = new ThreadedQueue<nsqUtil::EntrustDataStruct>(
                                                   heap, 100, queueDepth, nsqUtil::entrustMeta, nullptr, 0, "orders",
                                                   NSQ_PREFIX, Util::BUF_SIZE, nsqUtil::entrustReader);
                                               queues[markets[i]]->setTable(tables.back());
                                               queues[markets[i]]->start();
                                           }
                                           const auto &queueMap = queues;
                                           runWithWorkers(expectedRows, threadCount, [&](long long workerIndex, long long start, long long end) {
                                               auto sample = makeEntrustSample(channel);
                                               for (long long i = start; i < end; ++i) {
                                                   sample.SeqNo = i + 1;
                                                   sample.BizIndex = i + 1;
                                                   auto iter = queueMap.find(markets[workerIndex]);
                                                   if (iter != queueMap.end()) {
                                                       iter->second->push({Util::getNanoEpochTime(), sample});
                                                   }
                                               }
                                           });
                                           auto noWaitEnd = Clock::now();
                                           for (long long i = 0; i < threadCount; ++i) {
                                               long long rows = expectedRows * (i + 1) / threadCount - expectedRows * i / threadCount;
                                               waitRows(tables[i], rows);
                                           }
                                           auto waitEnd = Clock::now();
                                           long long rows = 0;
                                           for (long long i = 0; i < threadCount; ++i) {
                                               rows += tables[i]->rows();
                                               queues[markets[i]]->stop();
                                           }
                                           return CaseTiming{rows, elapsedNsSince(start, waitEnd), elapsedNsSince(start, noWaitEnd)};
                                       }));
    } else if (dataType == "snapshot") {
        results.push_back(runBatchCase("lockless_map_dispatch", dataType, batchSize, batchCount, queueDepth, threadCount,
                                       [&](long long expectedRows, const Clock::time_point &start) {
                                           std::vector<TableSP> tables;
                                           tables.reserve(static_cast<size_t>(threadCount));
                                           NsqQueues queues;
                                           for (long long i = 0; i < threadCount; ++i) {
                                               long long rows = expectedRows * (i + 1) / threadCount - expectedRows * i / threadCount;
                                               tables.push_back(createOutputTable(nsqUtil::snapshotMetaConcise, rows));
                                               queues.initAndStart(heap, nsq_data::DepthMarket, markets[i], nsq_version::origin,
                                                                   tables.back(), queueDepth);
                                           }
                                           runWithWorkers(expectedRows, threadCount, [&](long long workerIndex, long long start, long long end) {
                                               auto sample = makeSnapshotSample();
                                               for (long long i = start; i < end; ++i) {
                                                   sample.reachTime = Util::getNanoEpochTime();
                                                   sample.data.TradesNum = i + 1;
                                                   queues.pushData(sample, markets[workerIndex]);
                                               }
                                           });
                                           auto noWaitEnd = Clock::now();
                                           for (long long i = 0; i < threadCount; ++i) {
                                               long long rows = expectedRows * (i + 1) / threadCount - expectedRows * i / threadCount;
                                               waitRows(tables[i], rows);
                                           }
                                           auto waitEnd = Clock::now();
                                           long long rows = 0;
                                           for (long long i = 0; i < threadCount; ++i) {
                                               rows += tables[i]->rows();
                                               queues.stop(nsq_data::DepthMarket, markets[i]);
                                           }
                                           return CaseTiming{rows, elapsedNsSince(start, waitEnd), elapsedNsSince(start, noWaitEnd)};
                                       }));
        if (pushDataOnly) {
            goto build_result_table;
        }
        results.push_back(runOldNoLockSnapshotPushDataCase(dataType, heap, markets, batchSize, batchCount, queueDepth,
                                                           threadCount));

        results.push_back(runSnapshotRouteMapCase<LocklessFlatHashmap<int, ThreadedQueue<nsqUtil::SnapshotDataStruct> *>>(
            "direct_lockless_flat_hashmap", dataType, heap, markets, batchSize, batchCount, queueDepth, threadCount));
        results.push_back(runSnapshotRouteMapCase<IrremovableLocklessFlatHashmap<int, ThreadedQueue<nsqUtil::SnapshotDataStruct> *>>(
            "direct_irremovable_lockless_flat_hashmap", dataType, heap, markets, batchSize, batchCount, queueDepth, threadCount));
        results.push_back(runSnapshotRouteMapCase<LocklessHashmap<int, ThreadedQueue<nsqUtil::SnapshotDataStruct> *>>(
            "direct_lockless_hashmap", dataType, heap, markets, batchSize, batchCount, queueDepth, threadCount));
        results.push_back(runSnapshotRouteMapCase<IrremovableLocklessHashmap<int, ThreadedQueue<nsqUtil::SnapshotDataStruct> *>>(
            "direct_irremovable_lockless_hashmap", dataType, heap, markets, batchSize, batchCount, queueDepth, threadCount));

        results.push_back(runBatchCase("no_lock_map_dispatch", dataType, batchSize, batchCount, queueDepth, threadCount,
                                       [&](long long expectedRows, const Clock::time_point &start) {
                                           std::vector<TableSP> tables;
                                           tables.reserve(static_cast<size_t>(threadCount));
                                           auto snapshotReader = [](vector<ConstantSP> &buffer,
                                                                    nsqUtil::SnapshotDataStruct &data) {
                                               auto col = buffer.begin();
                                               nsqUtil::snapshotReaderConcise(col, data);
                                           };
                                           unordered_map<nsq_market, SmartPointer<ThreadedQueue<nsqUtil::SnapshotDataStruct>>> queues;
                                           for (long long i = 0; i < threadCount; ++i) {
                                               long long rows = expectedRows * (i + 1) / threadCount - expectedRows * i / threadCount;
                                               tables.push_back(createOutputTable(nsqUtil::snapshotMetaConcise, rows));
                                               queues[markets[i]] = new ThreadedQueue<nsqUtil::SnapshotDataStruct>(
                                                   heap, 100, queueDepth, nsqUtil::snapshotMetaConcise, nullptr, 0,
                                                   "snapshot", NSQ_PREFIX, Util::BUF_SIZE, snapshotReader);
                                               queues[markets[i]]->setTable(tables.back());
                                               queues[markets[i]]->start();
                                           }
                                           const auto &queueMap = queues;
                                           runWithWorkers(expectedRows, threadCount, [&](long long workerIndex, long long start, long long end) {
                                               auto sample = makeSnapshotSample();
                                               for (long long i = start; i < end; ++i) {
                                                   sample.reachTime = Util::getNanoEpochTime();
                                                   sample.data.TradesNum = i + 1;
                                                   auto iter = queueMap.find(markets[workerIndex]);
                                                   if (iter != queueMap.end()) {
                                                       iter->second->push(sample);
                                                   }
                                               }
                                           });
                                           auto noWaitEnd = Clock::now();
                                           for (long long i = 0; i < threadCount; ++i) {
                                               long long rows = expectedRows * (i + 1) / threadCount - expectedRows * i / threadCount;
                                               waitRows(tables[i], rows);
                                           }
                                           auto waitEnd = Clock::now();
                                           long long rows = 0;
                                           for (long long i = 0; i < threadCount; ++i) {
                                               rows += tables[i]->rows();
                                               queues[markets[i]]->stop();
                                           }
                                           return CaseTiming{rows, elapsedNsSince(start, waitEnd), elapsedNsSince(start, noWaitEnd)};
                                       }));
    } else if (dataType == "orderTrade") {
        nsq_market market = markets.front();
        results.push_back(runBatchCase("lockless_map_dispatch", dataType, batchSize, batchCount, queueDepth, threadCount,
                                       [&](long long expectedRows, const Clock::time_point &start) {
                                           MetaTable outputMeta = tradeEntrustOutputMeta();
                                           std::vector<TableSP> tables;
                                           tables.reserve(static_cast<size_t>(threadCount));
                                           NsqQueues queues;
                                           for (long long i = 0; i < threadCount; ++i) {
                                               long long rows = expectedRows * (i + 1) / threadCount - expectedRows * i / threadCount;
                                               tables.push_back(createOutputTable(outputMeta, rows));
                                               queues.initAndStartTradeEntrust(heap, market, channels[i], tables.back(), queueDepth);
                                           }
                                           runWithWorkers(expectedRows, threadCount, [&](long long workerIndex, long long start, long long end) {
                                               int workerChannel = channels[workerIndex];
                                               auto tradeSample = makeTradeSample(workerChannel);
                                               auto entrustSample = makeEntrustSample(workerChannel);
                                               for (long long i = start; i < end; ++i) {
                                                   if ((i & 1) == 0) {
                                                       tradeSample.SeqNo = i + 1;
                                                       tradeSample.BizIndex = i + 1;
                                                       queues.pushData(&tradeSample, market);
                                                   } else {
                                                       entrustSample.SeqNo = i + 1;
                                                       entrustSample.BizIndex = i + 1;
                                                       queues.pushData(&entrustSample, market);
                                                   }
                                               }
                                           });
                                           auto noWaitEnd = Clock::now();
                                           for (long long i = 0; i < threadCount; ++i) {
                                               long long rows = expectedRows * (i + 1) / threadCount - expectedRows * i / threadCount;
                                               waitRows(tables[i], rows);
                                           }
                                           auto waitEnd = Clock::now();
                                           long long rows = 0;
                                           for (long long i = 0; i < threadCount; ++i) {
                                               rows += tables[i]->rows();
                                           }
                                           queues.stop(nsq_data::orderbook, market);
                                           return CaseTiming{rows, elapsedNsSince(start, waitEnd), elapsedNsSince(start, noWaitEnd)};
                                       }));
        if (pushDataOnly) {
            goto build_result_table;
        }
        results.push_back(runOldNoLockOrderTradePushDataCase(dataType, heap, market, channels, batchSize, batchCount,
                                                             queueDepth, threadCount));

        results.push_back(runOrderTradeRouteMapCase<LocklessFlatHashmap<long long, ThreadedQueue<nsqUtil::TradeEntrustDataStruct> *>>(
            "direct_lockless_flat_hashmap", dataType, heap, market, channels, batchSize, batchCount, queueDepth, threadCount));
        results.push_back(runOrderTradeRouteMapCase<IrremovableLocklessFlatHashmap<long long, ThreadedQueue<nsqUtil::TradeEntrustDataStruct> *>>(
            "direct_irremovable_lockless_flat_hashmap", dataType, heap, market, channels, batchSize, batchCount, queueDepth, threadCount));
        results.push_back(runOrderTradeRouteMapCase<LocklessHashmap<long long, ThreadedQueue<nsqUtil::TradeEntrustDataStruct> *>>(
            "direct_lockless_hashmap", dataType, heap, market, channels, batchSize, batchCount, queueDepth, threadCount));
        results.push_back(runOrderTradeRouteMapCase<IrremovableLocklessHashmap<long long, ThreadedQueue<nsqUtil::TradeEntrustDataStruct> *>>(
            "direct_irremovable_lockless_hashmap", dataType, heap, market, channels, batchSize, batchCount, queueDepth, threadCount));

        results.push_back(runBatchCase("no_lock_map_dispatch", dataType, batchSize, batchCount, queueDepth, threadCount,
                                       [&](long long expectedRows, const Clock::time_point &start) {
                                           MetaTable outputMeta = tradeEntrustOutputMeta();
                                           std::vector<TableSP> tables;
                                           tables.reserve(static_cast<size_t>(threadCount));
                                           unordered_map<int, SmartPointer<ThreadedQueue<nsqUtil::TradeEntrustDataStruct>>> queues;
                                           for (long long i = 0; i < threadCount; ++i) {
                                               long long rows = expectedRows * (i + 1) / threadCount - expectedRows * i / threadCount;
                                               tables.push_back(createOutputTable(outputMeta, rows));
                                               queues[channels[i]] = new ThreadedQueue<nsqUtil::TradeEntrustDataStruct>(
                                                   heap, 100, queueDepth, nsqUtil::tradeEntrustMeta, nullptr, OPT_RECEIVED,
                                                   "tradeOrder", NSQ_PREFIX, Util::BUF_SIZE, nsqUtil::tradeEntrustReader);
                                               queues[channels[i]]->setTable(tables.back());
                                               queues[channels[i]]->start();
                                           }
                                           const auto &queueMap = queues;
                                           runWithWorkers(expectedRows, threadCount, [&](long long workerIndex, long long start, long long end) {
                                               int workerChannel = channels[workerIndex];
                                               auto tradeSample = makeTradeSample(workerChannel);
                                               auto entrustSample = makeEntrustSample(workerChannel);
                                               for (long long i = start; i < end; ++i) {
                                                   auto iter = queueMap.find(workerChannel);
                                                   if (iter == queueMap.end()) {
                                                       continue;
                                                   }
                                                   if ((i & 1) == 0) {
                                                       tradeSample.SeqNo = i + 1;
                                                       tradeSample.BizIndex = i + 1;
                                                       iter->second->push({Util::getNanoEpochTime(), true, tradeSample, {}});
                                                   } else {
                                                       entrustSample.SeqNo = i + 1;
                                                       entrustSample.BizIndex = i + 1;
                                                       iter->second->push({Util::getNanoEpochTime(), false, {}, entrustSample});
                                                   }
                                               }
                                           });
                                           auto noWaitEnd = Clock::now();
                                           for (long long i = 0; i < threadCount; ++i) {
                                               long long rows = expectedRows * (i + 1) / threadCount - expectedRows * i / threadCount;
                                               waitRows(tables[i], rows);
                                           }
                                           auto waitEnd = Clock::now();
                                           long long rows = 0;
                                           for (long long i = 0; i < threadCount; ++i) {
                                               rows += tables[i]->rows();
                                               queues[channels[i]]->stop();
                                           }
                                           return CaseTiming{rows, elapsedNsSince(start, waitEnd), elapsedNsSince(start, noWaitEnd)};
                                       }));
    } else {
        throw IllegalArgumentException(__FUNCTION__, usage + "dataType must be trade, orders, snapshot, or orderTrade.");
    }

build_result_table:
    int size = static_cast<int>(results.size());
    VectorSP testCol = Util::createVector(DT_STRING, 0, size);
    VectorSP dataTypeCol = Util::createVector(DT_STRING, 0, size);
    VectorSP batchSizeCol = Util::createVector(DT_LONG, 0, size);
    VectorSP batchCountCol = Util::createVector(DT_LONG, 0, size);
    VectorSP totalRowsCol = Util::createVector(DT_LONG, 0, size);
    VectorSP queueDepthCol = Util::createVector(DT_LONG, 0, size);
    VectorSP threadCountCol = Util::createVector(DT_LONG, 0, size);
    VectorSP elapsedNsCol = Util::createVector(DT_LONG, 0, size);
    VectorSP nsPerRowCol = Util::createVector(DT_DOUBLE, 0, size);
    VectorSP rowsPerSecondCol = Util::createVector(DT_DOUBLE, 0, size);
    VectorSP elapsedNsNoWaitCol = Util::createVector(DT_LONG, 0, size);
    VectorSP nsPerRowNoWaitCol = Util::createVector(DT_DOUBLE, 0, size);
    VectorSP rowsPerSecondNoWaitCol = Util::createVector(DT_DOUBLE, 0, size);
    VectorSP rowsCol = Util::createVector(DT_LONG, 0, size);

    for (auto &result : results) {
        testCol->appendString(&result.test, 1);
        dataTypeCol->appendString(&result.dataType, 1);
        batchSizeCol->appendLong(&result.batchSize, 1);
        batchCountCol->appendLong(&result.batchCount, 1);
        totalRowsCol->appendLong(&result.totalRows, 1);
        queueDepthCol->appendLong(&result.queueDepth, 1);
        threadCountCol->appendLong(&result.threadCount, 1);
        elapsedNsCol->appendLong(&result.elapsedNs, 1);
        nsPerRowCol->appendDouble(&result.nsPerRow, 1);
        rowsPerSecondCol->appendDouble(&result.rowsPerSecond, 1);
        elapsedNsNoWaitCol->appendLong(&result.elapsedNsNoWait, 1);
        nsPerRowNoWaitCol->appendDouble(&result.nsPerRowNoWait, 1);
        rowsPerSecondNoWaitCol->appendDouble(&result.rowsPerSecondNoWait, 1);
        rowsCol->appendLong(&result.rows, 1);
    }

    vector<string> colNames{"test",      "dataType", "batchSize", "batchCount", "totalRows",
                            "queueDepth", "threadCount", "elapsedNs", "nsPerRow",  "rowsPerSec",
                            "elapsedNsNoWait", "nsPerRowNoWait", "rowsPerSecNoWait", "rows"};
    vector<ConstantSP> cols{testCol,      dataTypeCol,      batchSizeCol,      batchCountCol, totalRowsCol,
                            queueDepthCol, threadCountCol, elapsedNsCol,    nsPerRowCol,      rowsPerSecondCol,
                            elapsedNsNoWaitCol, nsPerRowNoWaitCol, rowsPerSecondNoWaitCol, rowsCol};
    return Util::createTable(colNames, cols);
}
