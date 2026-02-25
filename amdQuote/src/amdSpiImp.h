#ifndef AMD_SPI_IMP_H
#define AMD_SPI_IMP_H

#include "DolphinDBEverything.h"
#include "CoreConcept.h"
#include "Plugin.h"
#include "Util.h"
#include "amdQuoteImp.h"
#include "amdQuoteType.h"
#include "ddbplugin/PluginLogger.h"

#ifndef AMD_USE_THREADED_QUEUE
#define AMD_USE_THREADED_QUEUE
#include "ddbplugin/ThreadedQueue.h"
#endif

using namespace ddb;

const static int TIMEOUT = 100;
static std::atomic<bool> ERROR_LOG(false);

#if defined(AMD_457) || defined(AMD_455)

inline const char *AmdQueryErrorToString(int code) {
    switch (code) {
        case amd::ama::ErrorCode::kAllocateMemoryFailed:
            return "Failed to allocate message memory";
        case amd::ama::ErrorCode::kCreateTcpClientFailed:
            return "Failed to create TCP client";
        case amd::ama::ErrorCode::kQueryTimeout:
            return "Query timed out";
        case amd::ama::ErrorCode::kQuerySendFailed:
            return "Failed to send query request";
        case amd::ama::ErrorCode::kNoQueryPermission:
            return "No permission to perform query";
        case amd::ama::ErrorCode::kQueryEmptyData:
            return "No data";
        case amd::ama::ErrorCode::kQueryPartData:
            return "Partial data returned";
        case amd::ama::ErrorCode::kOverReqestNumLimit:
            return "Exceeded query request limit";
        case amd::ama::ErrorCode::kOverTickNumLimit:
            return "Exceeded tick range limit";
        case amd::ama::ErrorCode::kQueryLogonFailed:
            return "Query login failed";
        case amd::ama::ErrorCode::kQueryConnectFailed:
            return "Query connection failed";
        case amd::ama::ErrorCode::kNoTickQueryServer:
            return "Upstream tick-query server not deployed; contact administrator";
        case amd::ama::ErrorCode::kIllegalMarketType:
            return "Invalid market type; only SZSE/SSE are supported";
        case amd::ama::ErrorCode::kIllegalChannelNo:
            return "Invalid channel number; must be > 0";
        case amd::ama::ErrorCode::kIllegalBeginSeqNum:
            return "Invalid begin_appl_seq_num parameter";
        case amd::ama::ErrorCode::kNullQuerySpi:
            return "Query SPI is null";
        case amd::ama::ErrorCode::kQueryEngineUnInited:
            return "Query engine not initialized (startup/switchover/cross-day)";
        default:
            return "Unknown AMD query error code";
    }
}

class QueryTask{
    public:
    QueryTask() :isFinish_(false) {}
    QueryTask(const QueryTask&) = delete;
    QueryTask& operator=(const QueryTask&) = delete;
    void setError(const string& error){
        LockGuard<Mutex> mutex(&dataLock_);
        errMsg_ = error;
        notify();
    }

    void OnMDTickOrder(const amd::ama::TickQueryItem& item, amd::ama::MDTickOrder* ticks, uint32_t cnt, bool is_last){
        std::ignore = item;
        PluginDefer defer([=]() { amd::ama::IAMDApi::FreeMemory(ticks); });
        LockGuard<Mutex> lock(&dataLock_);
        try{
            MDOrderExecution data{true, LONG_LONG_MIN};
            for(int i = 0; i < cnt; ++i){
                data.uni.tickOrder = ticks[i];
                orderExecutionReader(buffer_, data, MarketUtil::SeqCheckMode::IGNORE, szLastSeqNum_, shLastSeqNum_);
            }
            if(is_last) notify();
        }catch(exception& e){
            std::string errorMsg = AMDQUOTE_PREFIX + "Failed to call OnMDTickOrder for query: " + e.what();
            setError(errorMsg);
        }
    }

    void OnMDTickExecution(const amd::ama::TickQueryItem& item, amd::ama::MDTickExecution* ticks, uint32_t cnt, bool is_last){
        std::ignore = item;
        PluginDefer defer([=]() { amd::ama::IAMDApi::FreeMemory(ticks); });
        LockGuard<Mutex> lock(&dataLock_);
        try{
            MDOrderExecution data{false, LONG_LONG_MIN};
            for(int i = 0; i < cnt; ++i){
                data.uni.tickExecution = ticks[i];
                orderExecutionReader(buffer_, data, MarketUtil::SeqCheckMode::IGNORE, szLastSeqNum_, shLastSeqNum_);
            }
            if(is_last) notify();
        }catch(exception& e){
            std::string errorMsg = AMDQUOTE_PREFIX + "Failed to call OnMDTickExecution for query: " + e.what();
            setError(errorMsg);
        }
    }

    TableSP query(QueryArgs& queryArgs, uint64_t id, amd::ama::IQuerySpi* spi) {
        LockGuard<Mutex> lock(&dataLock_);
        int columns = queryArgs.colTypes.size();
        buffer_.resize(columns);
        int rows = queryArgs.endSeqNum - queryArgs.beginSeqNum + 1;
        for(int i = 0; i < columns; ++i){
            buffer_[i] = Util::createVector(queryArgs.colTypes[i], 0, rows);
        }

        amd::ama::TickQueryItem item;
        item.market = queryArgs.market;
        item.channel_no = queryArgs.channelNo;
        item.begin_appl_seq_num = queryArgs.beginSeqNum;
        item.end_appl_seq_num = queryArgs.endSeqNum;
        item.user_ctx = id;

        int ret = amd::ama::IAMDApi::QueryMDTick(spi, item);


        if(ret != amd::ama::ErrorCode::kSuccess){
            throw RuntimeException(AMDQUOTE_PREFIX + "Failed to query: received AMD error code (" + std::to_string(ret) + ") " + AmdQueryErrorToString(ret) + ".");
        }
        condition_.wait(dataLock_, queryArgs.timeoutMinute * 60000 + 2000);
        if(!errMsg_.empty()){
            throw RuntimeException(errMsg_);
        }
        if(!isFinish_){
            throw RuntimeException(AMDQUOTE_PREFIX + "Failed to query: timeout.");
        }

        int realRows = buffer_[0]->rows();
        VectorSP indexVector = Util::createIndexVector(0, realRows);
        for(int i = 1; i < columns; ++i){
            if(buffer_[i]->rows() != realRows){
                buffer_[i].getAs<Vector>()->resize(realRows);
                buffer_[i].getAs<Vector>()->set(indexVector, new Void());
            }
        }
        return Util::createTable(queryArgs.colNames, buffer_);
    }

private:
    void notify(){
        LockGuard<Mutex> lock(&dataLock_);
        isFinish_ = true;
        condition_.notify();
    }

    vector<ConstantSP> buffer_;
    std::unordered_map<int, long long> szLastSeqNum_;
    std::unordered_map<int, long long> shLastSeqNum_;
    ConditionalVariable condition_;
    Mutex dataLock_;
    bool isFinish_;
    std::string errMsg_;
};

typedef SmartPointer<QueryTask> QueryTaskSP;

class AMDQuerySpi : public amd::ama::IQuerySpi
{
public:
    AMDQuerySpi(): requestId_(0){}
    AMDQuerySpi(const AMDQuerySpi&) = delete;
    AMDQuerySpi& operator=(const AMDQuerySpi&) = delete;
    virtual void OnMDTickOrder(const amd::ama::TickQueryItem& item, amd::ama::MDTickOrder* ticks, uint32_t cnt, bool is_last) override
    {
        try {
            QueryTaskSP queryTask = nullptr;
            if(getQueryTask(item.user_ctx, queryTask)){
                queryTask->OnMDTickOrder(item, ticks, cnt, is_last);
            }
        } catch (...) {
            LOG_ERR(AMDQUOTE_PREFIX, "Caught exception in OnMDTickOrder");
        }
    }

    virtual void OnMDTickExecution(const amd::ama::TickQueryItem& item, amd::ama::MDTickExecution* ticks, uint32_t cnt, bool is_last) override
    {
        try {
            QueryTaskSP queryTask = nullptr;
            if(getQueryTask(item.user_ctx, queryTask)){
                queryTask->OnMDTickExecution(item, ticks, cnt, is_last);
            }
        } catch (...) {
            LOG_ERR(AMDQUOTE_PREFIX, "Caught exception in OnMDTickExecution");
        }
    }

    virtual void OnError(const amd::ama::TickQueryItem& item, int32_t error_code)
    {
        try {
            QueryTaskSP queryTask = nullptr;
            if(getQueryTask(item.user_ctx, queryTask)){
                std::string errMsg = AMDQUOTE_PREFIX + "Failed to query: received AMD error code (" + std::to_string(error_code) + ") " + AmdQueryErrorToString(error_code) + ".";
                queryTask->setError(errMsg);
            }
        } catch (...) {
            LOG_ERR(AMDQUOTE_PREFIX, "Caught exception in OnError");
        }
    }

    TableSP query(QueryArgs& queryArgs) {
        QueryTaskSP queryTask = nullptr;
        uint64_t id;
        {
            LockGuard<Mutex> lock(&lock_);
            id = requestId_++;
            queryTask = new QueryTask();
            queryMap_[id] = queryTask;
        }
        PluginDefer defer([this, id](){
            queryMap_.erase(id);
        });
        return queryTask->query(queryArgs, id, this);
    }

private:
    bool getQueryTask(uint64_t id, QueryTaskSP& queryTask){
        {
            LockGuard<Mutex> lock(&lock_);
            if(queryMap_.find(id) == queryMap_.end()){
                LOG_INFO(AMDQUOTE_PREFIX + "the query " + std::to_string(id) + " does not exist.");
                return false;
            }
            queryTask = queryMap_[id];
        }
        return true;
    }

    std::unordered_map<uint64_t, SmartPointer<QueryTask>> queryMap_;
    uint64_t requestId_;
    Mutex lock_;
};
#endif

class AMDSpiImp : public amd::ama::IAMDSpi {
  public:
    AMDSpiImp(SessionSP session, string dataVersion) : dataVersion_(dataVersion), session_(session) {}
    ~AMDSpiImp() {
        LOG_INFO(AMDQUOTE_PREFIX + "release AMDSpiImp");
        removeAll();
    }
    TableSP getStatus() {
        vector<string> colNames{"topicType",
                                "market",
                                "channel",
                                START_TIME_STR,
                                END_TIME_STR,
                                FIRST_MSG_TIME_STR,
                                LAST_MSG_TIME_STR,
                                PROCESSED_MSG_COUNT_STR,
                                FAILED_MSG_COUNT_STR,
                                LAST_ERR_MSG_STR,
                                LAST_FAILED_TIMESTAMP_STR,
                                "queueDepthLimit",
                                "queueDepth"};
        vector<string> topicTypeVec;
        vector<int> marketVec;
        vector<int> channelVec;
        vector<long long> startTimeVec;
        vector<long long> endTimeVec;
        vector<long long> firstMsgTimeVec;
        vector<long long> lastMsgTimeVec;
        vector<long long> processedMsgCountVec;
        vector<string> lastErrMsgVec;
        vector<long long> failedMsgCountVec;
        vector<long long> lastFailedTimestampVec;
        vector<long long> queueDepthLimitVec;
        vector<long long> queueDepthVec;

#define GET_STATISTICS(map, name)                                                              \
    for (auto it = map.begin(); it != map.end(); ++it) {                                       \
        int market = it->first;                                                                \
        auto status = it->second->getStatusConst();                                            \
        topicTypeVec.emplace_back(name);                                                       \
        marketVec.emplace_back(market);                                                        \
        channelVec.emplace_back(INT_MIN);                                                      \
        startTimeVec.emplace_back(status.startTime_);                                          \
        endTimeVec.emplace_back(status.endTime_);                                              \
        firstMsgTimeVec.emplace_back(status.firstMsgTime_);                                    \
        lastMsgTimeVec.emplace_back(status.lastMsgTime_);                                      \
        processedMsgCountVec.emplace_back(status.processedMsgCount_ - status.failedMsgCount_); \
        failedMsgCountVec.emplace_back(status.failedMsgCount_);                                \
        lastErrMsgVec.emplace_back(status.lastErrMsg_);                                        \
        lastFailedTimestampVec.emplace_back(status.lastFailedTimestamp_);                      \
        queueDepthLimitVec.emplace_back(status.queueDepthLimit_);                              \
        queueDepthVec.emplace_back(status.queueDepth_);                                        \
    }
        GET_STATISTICS(orderQueueQueueMap_, "orderQueue")
        GET_STATISTICS(indexQueueMap_, "index")
        GET_STATISTICS(optionQueueMap_, "option")
        GET_STATISTICS(futureQueueMap_, "future")
#ifndef AMD_396
        GET_STATISTICS(IOPVQueueMap_, "IOPV")
#endif
#ifdef AMD_457
        GET_STATISTICS(HKExMergeSnapshotQueueMap_, "HKExMergeSnapshot")
        GET_STATISTICS(HKExIndexSnapshotQueueMap_, "HKExIndexSnapshot")
#endif
        GET_STATISTICS(neeqSnapshotQueueMap_, "NEEQSnapshot")
        GET_STATISTICS(hktSnapshotQueueMap_, "HKTSnapshot")
        GET_STATISTICS(snapshotQueueMap_, "snapshot")
        GET_STATISTICS(fundSnapshotQueueMap_, "fundSnapshot")
        GET_STATISTICS(bondSnapshotQueueMap_, "bondSnapshot")
        GET_STATISTICS(orderQueueMap_, "order")
        GET_STATISTICS(fundOrderQueueMap_, "fundOrder")
        GET_STATISTICS(bondOrderQueueMap_, "bondOrder")
        GET_STATISTICS(executionQueueMap_, "execution")
        GET_STATISTICS(fundExecutionQueueMap_, "fundExecution")
        GET_STATISTICS(bondExecutionQueueMap_, "bondExecution")
#undef GET_STATISTICS

#define GET_ORDER_EXECUTION_STATISTICS(map, name)                                              \
    for (auto it = map.begin(); it != map.end(); ++it) {                                       \
        int market = it->first;                                                                \
        int channel = market / ORDER_EXECUTION_OFFSET;                                         \
        market %= ORDER_EXECUTION_OFFSET;                                                      \
        auto status = it->second->getStatusConst();                                            \
        topicTypeVec.emplace_back(name);                                                       \
        marketVec.emplace_back(market);                                                        \
        channelVec.emplace_back(channel);                                                      \
        startTimeVec.emplace_back(status.startTime_);                                          \
        endTimeVec.emplace_back(status.endTime_);                                              \
        firstMsgTimeVec.emplace_back(status.firstMsgTime_);                                    \
        lastMsgTimeVec.emplace_back(status.lastMsgTime_);                                      \
        processedMsgCountVec.emplace_back(status.processedMsgCount_ - status.failedMsgCount_); \
        failedMsgCountVec.emplace_back(status.failedMsgCount_);                                \
        lastErrMsgVec.emplace_back(status.lastErrMsg_);                                        \
        lastFailedTimestampVec.emplace_back(status.lastFailedTimestamp_);                      \
        queueDepthLimitVec.emplace_back(status.queueDepthLimit_);                              \
        queueDepthVec.emplace_back(status.queueDepth_);                                        \
    }
        GET_ORDER_EXECUTION_STATISTICS(orderExecutionQueueMap_, "orderExecution");
        GET_ORDER_EXECUTION_STATISTICS(bondOrderExecutionQueueMap_, "bondOrderExecution");
#undef GET_ORDER_EXECUTION_STATISTICS

        INDEX size = topicTypeVec.size();
        VectorSP topicTypeDdbVec = Util::createVector(DT_STRING, 0, size);
        VectorSP marketDdbVec = Util::createVector(DT_INT, 0, size);
        VectorSP channelDdbVec = Util::createVector(DT_INT, 0, size);
        VectorSP startTimeDdbVec = Util::createVector(DT_NANOTIMESTAMP, 0, size);
        VectorSP endTimeDdbVec = Util::createVector(DT_NANOTIMESTAMP, 0, size);
        VectorSP firstMsgTimeDdbVec = Util::createVector(DT_NANOTIMESTAMP, 0, size);
        VectorSP lastMsgTimeDdbVec = Util::createVector(DT_NANOTIMESTAMP, 0, size);
        VectorSP processedMsgCountDdbVec = Util::createVector(DT_LONG, 0, size);
        VectorSP lastErrMsgDdbVec = Util::createVector(DT_STRING, 0, size);
        VectorSP failedMsgCountDdbVec = Util::createVector(DT_LONG, 0, size);
        VectorSP lastFailedTimestampDdbVec = Util::createVector(DT_NANOTIMESTAMP, 0, size);
        VectorSP queueDepthLimitDdbVec = Util::createVector(DT_LONG, 0, size);
        VectorSP queueDepthDdbVec = Util::createVector(DT_LONG, 0, size);

        topicTypeDdbVec->appendString(topicTypeVec.data(), size);
        topicTypeDdbVec->setNullFlag(topicTypeDdbVec->hasNull());
        marketDdbVec->appendInt(marketVec.data(), size);
        marketDdbVec->setNullFlag(marketDdbVec->hasNull());
        channelDdbVec->appendInt(channelVec.data(), size);
        channelDdbVec->setNullFlag(channelDdbVec->hasNull());
        startTimeDdbVec->appendLong(startTimeVec.data(), size);
        startTimeDdbVec->setNullFlag(startTimeDdbVec->hasNull());
        firstMsgTimeDdbVec->appendLong(firstMsgTimeVec.data(), size);
        firstMsgTimeDdbVec->setNullFlag(firstMsgTimeDdbVec->hasNull());
        lastMsgTimeDdbVec->appendLong(lastMsgTimeVec.data(), size);
        lastMsgTimeDdbVec->setNullFlag(lastMsgTimeDdbVec->hasNull());
        endTimeDdbVec->appendLong(endTimeVec.data(), size);
        endTimeDdbVec->setNullFlag(endTimeDdbVec->hasNull());
        processedMsgCountDdbVec->appendLong(processedMsgCountVec.data(), size);
        processedMsgCountDdbVec->setNullFlag(processedMsgCountDdbVec->hasNull());
        lastErrMsgDdbVec->appendString(lastErrMsgVec.data(), size);
        lastErrMsgDdbVec->setNullFlag(lastErrMsgDdbVec->hasNull());
        failedMsgCountDdbVec->appendLong(failedMsgCountVec.data(), size);
        failedMsgCountDdbVec->setNullFlag(failedMsgCountDdbVec->hasNull());
        lastFailedTimestampDdbVec->appendLong(lastFailedTimestampVec.data(), size);
        lastFailedTimestampDdbVec->setNullFlag(lastFailedTimestampDdbVec->hasNull());
        queueDepthLimitDdbVec->appendLong(queueDepthLimitVec.data(), size);
        queueDepthLimitDdbVec->setNullFlag(queueDepthLimitDdbVec->hasNull());
        queueDepthDdbVec->appendLong(queueDepthVec.data(), size);
        queueDepthDdbVec->setNullFlag(queueDepthDdbVec->hasNull());

        vector<ConstantSP> cols{topicTypeDdbVec,
                                marketDdbVec,
                                channelDdbVec,
                                startTimeDdbVec,
                                endTimeDdbVec,
                                firstMsgTimeDdbVec,
                                lastMsgTimeDdbVec,
                                processedMsgCountDdbVec,
                                failedMsgCountDdbVec,
                                lastErrMsgDdbVec,
                                lastFailedTimestampDdbVec,
                                queueDepthLimitDdbVec,
                                queueDepthDdbVec
                                };
        return Util::createTable(colNames, cols);
    }
    template <typename T>
    void findAndPush(unordered_map<int, SmartPointer<ThreadedQueue<T>>> &queueMap, int key, T &data) {
        if (queueMap.find(key) != queueMap.end()) {
            queueMap[key]->push(data);
        }
    }

    void pushOptionData(amd::ama::MDOptionSnapshot *snapshot, uint32_t cnt, long long time) {
        for (uint32_t i = 0; i < cnt; ++i) {
            timeMDOption data{time, snapshot[i]};
            int market = snapshot[i].market_type;
            findAndPush(optionQueueMap_, market, data);
        }
    }
    void pushFutureData(amd::ama::MDFutureSnapshot *snapshot, uint32_t cnt, long long time) {
        for (uint32_t i = 0; i < cnt; ++i) {
            timeMDFuture data{time, snapshot[i]};
            int market = snapshot[i].market_type;
            findAndPush(futureQueueMap_, market, data);
        }
    }
#ifndef AMD_396
    void pushIOPVData(amd::ama::MDIOPVSnapshot *snapshot, uint32_t cnt, long long time) {
        for (uint32_t i = 0; i < cnt; ++i) {
            timeMDIOPV data{time, snapshot[i]};
            int market = snapshot[i].market_type;
            findAndPush(IOPVQueueMap_, market, data);
        }
    }
#endif
#ifdef AMD_457
    void pushHKExMergeSnapshotData(amd::ama::MDHKExMergeSnapshot *snapshot, uint32_t cnt, long long time) {
        for (uint32_t i = 0; i < cnt; ++i) {
            timeHKExMergeSnapshot data{time, snapshot[i]};
            int market = snapshot[i].market_type;
            findAndPush(HKExMergeSnapshotQueueMap_, market, data);
        }
    }

    void pushHKExIndexSnapshotData(amd::ama::MDHKExIndexSnapshot *snapshot, uint32_t cnt, long long time) {
        for (uint32_t i = 0; i < cnt; ++i) {
            timeHKExIndexSnapshot data{time, snapshot[i]};
            int market = snapshot[i].market_type;
            findAndPush(HKExIndexSnapshotQueueMap_, market, data);
        }
    }
#endif

    void pushNEEQSnapshotData(amd::ama::MDNEEQSnapshot *snapshot, uint32_t cnt, long long time) {
        for (uint32_t i = 0; i < cnt; ++i) {
            timeMDNEEQSnapshot data{time, snapshot[i]};
            int market = snapshot[i].market_type;
            findAndPush(neeqSnapshotQueueMap_, market, data);
        }
    }
    void pushHKTSnapshotData(amd::ama::MDHKTSnapshot *snapshot, uint32_t cnt, long long time) {
        for (uint32_t i = 0; i < cnt; ++i) {
            timeMDHKTSnapshot data{time, snapshot[i]};
            int market = snapshot[i].market_type;
            findAndPush(hktSnapshotQueueMap_, market, data);
        }
    }

    void pushSnapshotData(amd::ama::MDSnapshot *snapshot, uint32_t cnt, long long time) {
        for (uint32_t i = 0; i < cnt; ++i) {
            timeMDSnapshot data{time, snapshot[i]};
            int market = snapshot[i].market_type;
            if (snapshot[i].variety_category == 1) {
                findAndPush(snapshotQueueMap_, market, data);
            } else {
                findAndPush(fundSnapshotQueueMap_, market, data);
            }
        }
    }
    void pushOrderData(amd::ama::MDTickOrder *ticks, uint32_t cnt, long long time) {
        for (uint32_t i = 0; i < cnt; ++i) {
            timeMDTickOrder data{time, ticks[i]};
            int market = ticks[i].market_type;
            if (ticks[i].variety_category == 1) {
                findAndPush(orderQueueMap_, market, data);
            } else {
                findAndPush(fundOrderQueueMap_, market, data);
            }
            int channel = ticks[i].channel_no;
            /**
             * orderExecution push data logic:
             * if the data's channel is unknown, ignore this data.
             *
             * the next pushExecutionData/pushBondOrderData/pushBondExecutionData
             * functions has same push logic
             */
            market += channel * ORDER_EXECUTION_OFFSET;
            if (orderExecutionQueueMap_.find(market) != orderExecutionQueueMap_.end()) {
                MDOrderExecution data{true, time, {}};
                data.uni.tickOrder = ticks[i];
                orderExecutionQueueMap_[market]->push(data);
            }
        }
    }
    void pushExecutionData(amd::ama::MDTickExecution *ticks, uint32_t cnt, long long time) {
        for (uint32_t i = 0; i < cnt; ++i) {
            timeMDTickExecution data{time, ticks[i]};
            int market = ticks[i].market_type;
            if (ticks[i].variety_category == 1) {
                findAndPush(executionQueueMap_, market, data);
            } else {
                findAndPush(fundExecutionQueueMap_, market, data);
            }
            int channel = ticks[i].channel_no;
            market += channel * ORDER_EXECUTION_OFFSET;
            if (orderExecutionQueueMap_.find(market) != orderExecutionQueueMap_.end()) {
                MDOrderExecution data{false, time, {}};
                data.uni.tickExecution = ticks[i];
                orderExecutionQueueMap_[market]->push(data);
            }
        }
    }
    void pushIndexData(amd::ama::MDIndexSnapshot *index, uint32_t cnt, long long time) {
        for (uint32_t i = 0; i < cnt; ++i) {
            timeMDIndexSnapshot data{time, index[i]};
            int market = index[i].market_type;
            findAndPush(indexQueueMap_, market, data);
        }
    }
    void pushOrderQueueData(amd::ama::MDOrderQueue *queue, uint32_t cnt, long long time) {
        for (uint32_t i = 0; i < cnt; ++i) {
            timeMDOrderQueue data{time, queue[i]};
            int market = queue[i].market_type;
            findAndPush(orderQueueQueueMap_, market, data);
        }
    }
    void pushBondSnapshotData(amd::ama::MDBondSnapshot *snapshots, uint32_t cnt, long long time) {
        for (uint32_t i = 0; i < cnt; ++i) {
            timeMDBondSnapshot data{time, snapshots[i]};
            int market = snapshots[i].market_type;
            findAndPush(bondSnapshotQueueMap_, market, data);
        }
    }
    void pushBondOrderData(amd::ama::MDBondTickOrder *ticks, uint32_t cnt, long long time) {
        for (uint32_t i = 0; i < cnt; ++i) {
            timeMDBondTickOrder data{time, ticks[i]};
            int market = ticks[i].market_type;
            findAndPush(bondOrderQueueMap_, market, data);
            int channel = ticks[i].channel_no;
            market += channel * ORDER_EXECUTION_OFFSET;
            if (bondOrderExecutionQueueMap_.find(market) != bondOrderExecutionQueueMap_.end()) {
                MDBondOrderExecution data{true, time, {}};
                data.uni.tickOrder = ticks[i];
                bondOrderExecutionQueueMap_[market]->push(data);
            }
        }
    }
    void pushBondExecutionData(amd::ama::MDBondTickExecution *ticks, uint32_t cnt, long long time) {
        for (uint32_t i = 0; i < cnt; ++i) {
            timeMDBondTickExecution data{time, ticks[i]};
            int market = ticks[i].market_type;
            findAndPush(bondExecutionQueueMap_, market, data);
            int channel = ticks[i].channel_no;
            market += channel * ORDER_EXECUTION_OFFSET;
            if (bondOrderExecutionQueueMap_.find(market) != bondOrderExecutionQueueMap_.end()) {
                MDBondOrderExecution data{false, time, {}};
                data.uni.tickExecution = ticks[i];
                bondOrderExecutionQueueMap_[market]->push(data);
            }
        }
    }

    virtual void OnMDNEEQSnapshot(amd::ama::MDNEEQSnapshot* snapshots, uint32_t cnt) override {
        try {
            PluginDefer df([=]() { amd::ama::IAMDApi::FreeMemory(snapshots); });
            long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
            pushNEEQSnapshotData(snapshots, cnt, time);
        } catch (...) {
            LOG_ERR(AMDQUOTE_PREFIX, "Unknown exception in OnMDNEEQSnapshot");
        }
    }
    virtual void OnMDHKTSnapshot(amd::ama::MDHKTSnapshot* snapshots, uint32_t cnt) override {
        try {
            PluginDefer df([=]() { amd::ama::IAMDApi::FreeMemory(snapshots); });
            long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
            pushHKTSnapshotData(snapshots, cnt, time);
        } catch (...) {
            LOG_ERR(AMDQUOTE_PREFIX, "Unknown exception in OnMDHKTSnapshot");
        }
    }
    virtual void OnMDOptionSnapshot(amd::ama::MDOptionSnapshot *snapshots, uint32_t cnt) override {
        try {
            PluginDefer df([=]() { amd::ama::IAMDApi::FreeMemory(snapshots); });
            long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
            pushOptionData(snapshots, cnt, time);
        } catch (...) {
            LOG_ERR(AMDQUOTE_PREFIX, "Unknown exception in OnMDOptionSnapshot");
        }
    }
    virtual void OnMDFutureSnapshot(amd::ama::MDFutureSnapshot *snapshots, uint32_t cnt) override {
        try {
            PluginDefer df([=]() { amd::ama::IAMDApi::FreeMemory(snapshots); });
            long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
            pushFutureData(snapshots, cnt, time);
        } catch (...) {
            LOG_ERR(AMDQUOTE_PREFIX, "Unknown exception in OnMDFutureSnapshot");
        }
    }
#ifndef AMD_396
    virtual void OnMDIOPVSnapshot(amd::ama::MDIOPVSnapshot *snapshots, uint32_t cnt) override {
        try {
            PluginDefer df([=]() { amd::ama::IAMDApi::FreeMemory(snapshots); });
            long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
            pushIOPVData(snapshots, cnt, time);
        } catch (...) {
            LOG_ERR(AMDQUOTE_PREFIX, "Unknown exception in OnMDIOPVSnapshot");
        }
    }
#endif
#ifdef AMD_457
    virtual void OnMDHKExMergeSnapshot(amd::ama::MDHKExMergeSnapshot* snapshots, uint32_t cnt) override {
        try {
            PluginDefer df([=]() { amd::ama::IAMDApi::FreeMemory(snapshots); });
            long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
            pushHKExMergeSnapshotData(snapshots, cnt, time);
        } catch (...) {
            LOG_ERR(AMDQUOTE_PREFIX, "Unknown exception in OnMDHKExMergeSnapshot");
        }
    }

    virtual void OnMDHKExIndexSnapshot(amd::ama::MDHKExIndexSnapshot* snapshots, uint32_t cnt) override {
        try {
            PluginDefer df([=]() { amd::ama::IAMDApi::FreeMemory(snapshots); });
            long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
            pushHKExIndexSnapshotData(snapshots, cnt, time);
        } catch (...) {
            LOG_ERR(AMDQUOTE_PREFIX, "Unknown exception in OnMDHKExIndexSnapshot");
        }
    }
#endif

    virtual void OnMDSnapshot(amd::ama::MDSnapshot *snapshot, uint32_t cnt) override {
        try {
            PluginDefer df([=]() { amd::ama::IAMDApi::FreeMemory(snapshot); });
            long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
            pushSnapshotData(snapshot, cnt, time);
        } catch (...) {
            LOG_ERR(AMDQUOTE_PREFIX, "Unknown exception in OnMDSnapshot");
        }
    }
    virtual void OnMDTickOrder(amd::ama::MDTickOrder *ticks, uint32_t cnt) override {
        try {
            PluginDefer df([=]() { amd::ama::IAMDApi::FreeMemory(ticks); });
            long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
            if (UNLIKELY(PLUGIN_VAR(PLUGIN_LOG_LEVEL) == severity_type::DEBUG)) {
                for (auto i = 0U; i < cnt; ++i) {
                    LOG(AMDQUOTE_PREFIX, __FUNCTION__, " securityCode: ", ticks[i].security_code, "; bizIndex: ", ticks[i].appl_seq_num);
                }
            }
            pushOrderData(ticks, cnt, time);
        } catch (...) {
            LOG_ERR(AMDQUOTE_PREFIX, "Unknown exception in OnMDTickOrder");
        }
    }
    virtual void OnMDTickExecution(amd::ama::MDTickExecution *ticks, uint32_t cnt) override {
        try {
            PluginDefer df([=]() { amd::ama::IAMDApi::FreeMemory(ticks); });
            long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
            if (UNLIKELY(PLUGIN_VAR(PLUGIN_LOG_LEVEL) == severity_type::DEBUG)) {
                for (auto i = 0U; i < cnt; ++i) {
                    LOG(AMDQUOTE_PREFIX, __FUNCTION__, " securityCode: ", ticks[i].security_code, "; bizIndex: ", ticks[i].appl_seq_num);
                }
            }
            pushExecutionData(ticks, cnt, time);
        } catch (...) {
            LOG_ERR(AMDQUOTE_PREFIX, "Unknown exception in OnMDTickExecution");
        }
    }
    virtual void OnMDIndexSnapshot(amd::ama::MDIndexSnapshot *index, uint32_t cnt) override {
        try {
            PluginDefer df([=]() { amd::ama::IAMDApi::FreeMemory(index); });
            long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
            pushIndexData(index, cnt, time);
        } catch (...) {
            LOG_ERR(AMDQUOTE_PREFIX, "Unknown exception in OnMDIndexSnapshot");
        }
    }
    virtual void OnMDOrderQueue(amd::ama::MDOrderQueue *queue, uint32_t cnt) override {
        try {
            PluginDefer df([=]() { amd::ama::IAMDApi::FreeMemory(queue); });
            long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
            pushOrderQueueData(queue, cnt, time);
        } catch (...) {
            LOG_ERR(AMDQUOTE_PREFIX, "Unknown exception in OnMDOrderQueue");
        }
    }
    virtual void OnMDBondSnapshot(amd::ama::MDBondSnapshot *snapshots, uint32_t cnt) override {
        try {
            PluginDefer df([=]() { amd::ama::IAMDApi::FreeMemory(snapshots); });
            long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
            pushBondSnapshotData(snapshots, cnt, time);
        } catch (...) {
            LOG_ERR(AMDQUOTE_PREFIX, "Unknown exception in OnMDBondSnapshot");
        }
    }
    virtual void OnMDBondTickOrder(amd::ama::MDBondTickOrder *ticks, uint32_t cnt) override {
        try {
            PluginDefer df([=]() { amd::ama::IAMDApi::FreeMemory(ticks); });
            long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
            pushBondOrderData(ticks, cnt, time);
        } catch (...) {
            LOG_ERR(AMDQUOTE_PREFIX, "Unknown exception in OnMDBondTickOrder");
        }
    }
    virtual void OnMDBondTickExecution(amd::ama::MDBondTickExecution *ticks, uint32_t cnt) override {
        try {
            PluginDefer df([=]() { amd::ama::IAMDApi::FreeMemory(ticks); });
            long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
            pushBondExecutionData(ticks, cnt, time);
        } catch (...) {
            LOG_ERR(AMDQUOTE_PREFIX, "Unknown exception in OnMDBondTickExecution");
        }
    }

    virtual void OnEvent(uint32_t level, uint32_t code, const char *event_msg, uint32_t len) override;

    virtual void OnLog(const int32_t &level, const char *log, uint32_t len) override;

    virtual void OnIndicator(const char *indicator, uint32_t len) override;

    void addThreadedQueue(Heap *heap, const string &typeName, AMDDataType type, int key, TableSP table,
                          FunctionDefSP transform, MetaTable meta, TableSP schema, int optionFlag,
                          long long dailyStartTime, bool securityCodeToInt, int seqCheckMode, long long capacity) {
        std::function<void(vector<ConstantSP> &, timeMDTickOrder &)> readerParamOrder;
        std::function<void(vector<ConstantSP> &, timeMDBondTickOrder &)> readerParamBondOrder;
        std::function<void(vector<ConstantSP> &, timeMDBondSnapshot &)> readerParamBondSnapshot;
        if (dataVersion_ == "4.0.1") {
            readerParamOrder = [=](vector<ConstantSP> &buffer, timeMDTickOrder &data) {
                return orderReader_4_0_1(buffer, data, securityCodeToInt);
            };
            readerParamBondOrder = [=](vector<ConstantSP> &buffer, timeMDBondTickOrder &data) {
                return bondOrderReader_4_0_1(buffer, data, securityCodeToInt);
            };
            readerParamBondSnapshot = [=](vector<ConstantSP> &buffer, timeMDBondSnapshot &data) {
                return bondSnapshotReader_4_0_1(buffer, data, securityCodeToInt);
            };
        } else {
            readerParamOrder = [=](vector<ConstantSP> &buffer, timeMDTickOrder &data) {
                return orderReader(buffer, data, securityCodeToInt);
            };
            readerParamBondOrder = [=](vector<ConstantSP> &buffer, timeMDBondTickOrder &data) {
                return bondOrderReader(buffer, data, securityCodeToInt);
            };
            readerParamBondSnapshot = [=](vector<ConstantSP> &buffer, timeMDBondSnapshot &data) {
                return bondSnapshotReader(buffer, data, securityCodeToInt);
            };
        }
        switch (type) {
#define ADD_THREADED_QUEUE(type, map, struct, reader)                                                     \
    case type:                                                                                            \
        map[key] = new ThreadedQueue<struct>(heap, TIMEOUT, capacity, meta, schema, optionFlag, typeName, \
                                                AMDQUOTE_PREFIX, Util::BUF_SIZE, reader);                 \
        map[key]->setTable(table);                                                                        \
        map[key]->setTransform(transform);                                                                \
        map[key]->setDailyIndex(dailyStartTime);                                                          \
        map[key]->start();                                                                                \
        break;

        ADD_THREADED_QUEUE(AMD_HKT_SNAPSHOT, hktSnapshotQueueMap_, timeMDHKTSnapshot,
                            [=](vector<ConstantSP> &buffer, timeMDHKTSnapshot &data) {
                                return hktSnapshotReader(buffer, data, securityCodeToInt);
                            })
        ADD_THREADED_QUEUE(AMD_NEEQ_SNAPSHOT, neeqSnapshotQueueMap_, timeMDNEEQSnapshot,
                            [=](vector<ConstantSP> &buffer, timeMDNEEQSnapshot &data) {
                                return neeqSnapshotReader(buffer, data, securityCodeToInt);
                            })
        ADD_THREADED_QUEUE(AMD_SNAPSHOT, snapshotQueueMap_, timeMDSnapshot,
                            [=](vector<ConstantSP> &buffer, timeMDSnapshot &data) {
                                return snapshotReader(buffer, data, securityCodeToInt);
                            })
        ADD_THREADED_QUEUE(AMD_EXECUTION, executionQueueMap_, timeMDTickExecution,
                            [=](vector<ConstantSP> &buffer, timeMDTickExecution &data) {
                                return executionReader(buffer, data, securityCodeToInt);
                            })
        ADD_THREADED_QUEUE(AMD_ORDER, orderQueueMap_, timeMDTickOrder, readerParamOrder)
        ADD_THREADED_QUEUE(AMD_FUND_SNAPSHOT, fundSnapshotQueueMap_, timeMDSnapshot,
                            [=](vector<ConstantSP> &buffer, timeMDSnapshot &data) {
                                return snapshotReader(buffer, data, securityCodeToInt);
                            })
        ADD_THREADED_QUEUE(AMD_FUND_EXECUTION, fundExecutionQueueMap_, timeMDTickExecution,
                            [=](vector<ConstantSP> &buffer, timeMDTickExecution &data) {
                                return executionReader(buffer, data, securityCodeToInt);
                            })
        ADD_THREADED_QUEUE(AMD_FUND_ORDER, fundOrderQueueMap_, timeMDTickOrder, readerParamOrder)
        ADD_THREADED_QUEUE(AMD_BOND_SNAPSHOT, bondSnapshotQueueMap_, timeMDBondSnapshot, readerParamBondSnapshot)
        ADD_THREADED_QUEUE(AMD_BOND_EXECUTION, bondExecutionQueueMap_, timeMDBondTickExecution,
                            [=](vector<ConstantSP> &buffer, timeMDBondTickExecution &data) {
                                return bondExecutionReader(buffer, data, securityCodeToInt);
                            })
        ADD_THREADED_QUEUE(AMD_BOND_ORDER, bondOrderQueueMap_, timeMDBondTickOrder, readerParamBondOrder)
        ADD_THREADED_QUEUE(
            AMD_ORDER_EXECUTION, orderExecutionQueueMap_, MDOrderExecution,
            ThreadedQueueUtil::orderbookReaderWrapper<MDOrderExecution>(orderExecutionReader, seqCheckMode))
        ADD_THREADED_QUEUE(
            AMD_BOND_ORDER_EXECUTION, bondOrderExecutionQueueMap_, MDBondOrderExecution,
            ThreadedQueueUtil::orderbookReaderWrapper<MDBondOrderExecution>(bondOrderExecutionReader, seqCheckMode))
        ADD_THREADED_QUEUE(AMD_INDEX, indexQueueMap_, timeMDIndexSnapshot,
                            [=](vector<ConstantSP> &buffer, timeMDIndexSnapshot &data) {
                                return indexReader(buffer, data, securityCodeToInt);
                            })
        ADD_THREADED_QUEUE(AMD_ORDER_QUEUE, orderQueueQueueMap_, timeMDOrderQueue,
                            [=](vector<ConstantSP> &buffer, timeMDOrderQueue &data) {
                                return orderQueueReader(buffer, data, securityCodeToInt);
                            })
        ADD_THREADED_QUEUE(AMD_OPTION_SNAPSHOT, optionQueueMap_, timeMDOption,
                            [=](vector<ConstantSP> &buffer, timeMDOption &data) {
                                return optionReader(buffer, data, securityCodeToInt);
                            })
        ADD_THREADED_QUEUE(AMD_FUTURE_SNAPSHOT, futureQueueMap_, timeMDFuture,
                            [=](vector<ConstantSP> &buffer, timeMDFuture &data) {
                                return futureReader(buffer, data, securityCodeToInt);
                            })
#ifndef AMD_396
        ADD_THREADED_QUEUE(AMD_IOPV_SNAPSHOT, IOPVQueueMap_, timeMDIOPV,
                            [=](vector<ConstantSP> &buffer, timeMDIOPV &data) {
                                return IOPVReader(buffer, data, securityCodeToInt);
                            })
#endif
#ifdef AMD_457
        ADD_THREADED_QUEUE(AMD_HKEX_MERGE_SNAPSHOT, HKExMergeSnapshotQueueMap_, timeHKExMergeSnapshot,
                            [=](vector<ConstantSP> &buffer, timeHKExMergeSnapshot &data) {
                                return HKExMergeSnapshotReader(buffer, data, securityCodeToInt);
                            })
        ADD_THREADED_QUEUE(AMD_HKEX_INDEX_SNAPSHOT, HKExIndexSnapshotQueueMap_, timeHKExIndexSnapshot,
                            [=](vector<ConstantSP> &buffer, timeHKExIndexSnapshot &data) {
                                return HKExIndexSnapshotReader(buffer, data, securityCodeToInt);
                            })
#endif
        case AMD_ERROR_DATA_TYPE:
        default:
            throw RuntimeException(AMDQUOTE_PREFIX + "Invalid AMD DataType to add");
#undef ADD_THREADED_QUEUE
        }
    }

    void removeAll() {
#define ERASE_IF_EXIST(map)                              \
    for (auto it = map.begin(); it != map.end(); ++it) { \
        it->second->stop();                              \
    }                                                    \
    // map.clear();

        ERASE_IF_EXIST(neeqSnapshotQueueMap_)
        ERASE_IF_EXIST(hktSnapshotQueueMap_)
        ERASE_IF_EXIST(snapshotQueueMap_)
        ERASE_IF_EXIST(fundSnapshotQueueMap_)
        ERASE_IF_EXIST(bondSnapshotQueueMap_)
        ERASE_IF_EXIST(orderQueueMap_)
        ERASE_IF_EXIST(fundOrderQueueMap_)
        ERASE_IF_EXIST(bondOrderQueueMap_)
        ERASE_IF_EXIST(executionQueueMap_)
        ERASE_IF_EXIST(fundExecutionQueueMap_)
        ERASE_IF_EXIST(bondExecutionQueueMap_)
        ERASE_IF_EXIST(orderExecutionQueueMap_)
        ERASE_IF_EXIST(bondOrderExecutionQueueMap_)
        ERASE_IF_EXIST(indexQueueMap_)
        ERASE_IF_EXIST(orderQueueQueueMap_)
        ERASE_IF_EXIST(optionQueueMap_)
        ERASE_IF_EXIST(futureQueueMap_)
#ifndef AMD_396
        ERASE_IF_EXIST(IOPVQueueMap_)
#endif
#ifdef AMD_457
        ERASE_IF_EXIST(HKExMergeSnapshotQueueMap_)
        ERASE_IF_EXIST(HKExIndexSnapshotQueueMap_)
#endif

#undef ERASE_IF_EXIST
    }

    void removeThreadedQueue(AMDDataType type, int key) {
        switch (type) {
#define ERASE_IF_EXIST(type, map)                        \
    case type:                                           \
        if (map.find(key) != map.end()) {                \
            if (map[key]->isStarted()) map[key]->stop(); \
        }                                                \
        break;
#define ERASE_MERGE_IF_EXIST(type, map)                            \
    {                                                              \
        case type:                                                 \
            vector<int> mergeList;                                 \
            for (auto it = map.begin(); it != map.end(); ++it) {   \
                if ((it->first % ORDER_EXECUTION_OFFSET) == key) { \
                    mergeList.push_back(it->first);                \
                }                                                  \
            }                                                      \
            for (int &num : mergeList) {                           \
                if (map[num]->isStarted()) map[num]->stop();       \
            }                                                      \
            break;                                                 \
    }

            ERASE_IF_EXIST(AMD_NEEQ_SNAPSHOT, neeqSnapshotQueueMap_)
            ERASE_IF_EXIST(AMD_HKT_SNAPSHOT, hktSnapshotQueueMap_)
            ERASE_IF_EXIST(AMD_SNAPSHOT, snapshotQueueMap_)
            ERASE_IF_EXIST(AMD_EXECUTION, executionQueueMap_)
            ERASE_IF_EXIST(AMD_ORDER, orderQueueMap_)
            ERASE_IF_EXIST(AMD_FUND_SNAPSHOT, fundSnapshotQueueMap_)
            ERASE_IF_EXIST(AMD_FUND_EXECUTION, fundExecutionQueueMap_)
            ERASE_IF_EXIST(AMD_FUND_ORDER, fundOrderQueueMap_)
            ERASE_IF_EXIST(AMD_BOND_SNAPSHOT, bondSnapshotQueueMap_)
            ERASE_IF_EXIST(AMD_BOND_EXECUTION, bondExecutionQueueMap_)
            ERASE_IF_EXIST(AMD_BOND_ORDER, bondOrderQueueMap_)
            ERASE_MERGE_IF_EXIST(AMD_ORDER_EXECUTION, orderExecutionQueueMap_)
            ERASE_MERGE_IF_EXIST(AMD_BOND_ORDER_EXECUTION, bondOrderExecutionQueueMap_)
            ERASE_IF_EXIST(AMD_INDEX, indexQueueMap_)
            ERASE_IF_EXIST(AMD_ORDER_QUEUE, orderQueueQueueMap_)
            ERASE_IF_EXIST(AMD_OPTION_SNAPSHOT, optionQueueMap_)
            ERASE_IF_EXIST(AMD_FUTURE_SNAPSHOT, futureQueueMap_)
#ifndef AMD_396
            ERASE_IF_EXIST(AMD_IOPV_SNAPSHOT, IOPVQueueMap_)
#endif
#ifdef AMD_457
        ERASE_IF_EXIST(AMD_HKEX_MERGE_SNAPSHOT, HKExMergeSnapshotQueueMap_)
        ERASE_IF_EXIST(AMD_HKEX_INDEX_SNAPSHOT, HKExIndexSnapshotQueueMap_)
#endif
            case AMD_ERROR_DATA_TYPE:
            default:
                throw RuntimeException(AMDQUOTE_PREFIX + "Invalid AMD DataType to remove");

#undef ERASE_IF_EXIST
#undef ERASE_MERGE_IF_EXIST
        }
    }
#if defined(AMD_457) || defined(AMD_455)
    TableSP query(QueryArgs& queryArgs){
        return querySpi_.query(queryArgs);
    }
#endif

  private:
    unordered_map<int, SmartPointer<ThreadedQueue<timeMDOrderQueue>>> orderQueueQueueMap_;
    unordered_map<int, SmartPointer<ThreadedQueue<timeMDIndexSnapshot>>> indexQueueMap_;
    unordered_map<int, SmartPointer<ThreadedQueue<timeMDOption>>> optionQueueMap_;
    unordered_map<int, SmartPointer<ThreadedQueue<timeMDFuture>>> futureQueueMap_;
#ifndef AMD_396
    unordered_map<int, SmartPointer<ThreadedQueue<timeMDIOPV>>> IOPVQueueMap_;
#endif
#ifdef AMD_457
    unordered_map<int, SmartPointer<ThreadedQueue<timeHKExMergeSnapshot>>> HKExMergeSnapshotQueueMap_;
    unordered_map<int, SmartPointer<ThreadedQueue<timeHKExIndexSnapshot>>> HKExIndexSnapshotQueueMap_;
#endif

    unordered_map<int, SmartPointer<ThreadedQueue<timeMDNEEQSnapshot>>> neeqSnapshotQueueMap_;
    unordered_map<int, SmartPointer<ThreadedQueue<timeMDHKTSnapshot>>> hktSnapshotQueueMap_;

    unordered_map<int, SmartPointer<ThreadedQueue<timeMDSnapshot>>> snapshotQueueMap_;
    unordered_map<int, SmartPointer<ThreadedQueue<timeMDSnapshot>>> fundSnapshotQueueMap_;
    unordered_map<int, SmartPointer<ThreadedQueue<timeMDBondSnapshot>>> bondSnapshotQueueMap_;

    unordered_map<int, SmartPointer<ThreadedQueue<timeMDTickOrder>>> orderQueueMap_;
    unordered_map<int, SmartPointer<ThreadedQueue<timeMDTickOrder>>> fundOrderQueueMap_;
    unordered_map<int, SmartPointer<ThreadedQueue<timeMDBondTickOrder>>> bondOrderQueueMap_;

    unordered_map<int, SmartPointer<ThreadedQueue<timeMDTickExecution>>> executionQueueMap_;
    unordered_map<int, SmartPointer<ThreadedQueue<timeMDTickExecution>>> fundExecutionQueueMap_;
    unordered_map<int, SmartPointer<ThreadedQueue<timeMDBondTickExecution>>> bondExecutionQueueMap_;

    std::unordered_map<int, SmartPointer<ThreadedQueue<MDOrderExecution>>> orderExecutionQueueMap_;
    std::unordered_map<int, SmartPointer<ThreadedQueue<MDBondOrderExecution>>> bondOrderExecutionQueueMap_;

    string dataVersion_;
    SessionSP session_;

#if defined(AMD_457) || defined(AMD_455)
    AMDQuerySpi querySpi_;
#endif
};
#endif
