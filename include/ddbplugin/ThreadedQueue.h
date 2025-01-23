#ifndef THREADED_QUEUE_H
#define THREADED_QUEUE_H

#include <climits>
#include <string>
#include <unordered_map>

#include "CoreConcept.h"
#include "Exceptions.h"
#include "Logger.h"
#include "ScalarImp.h"
#include "Types.h"
#include "Util.h"
#include "ddbplugin/Plugin.h"

#ifdef USE_PLUGIN_LOGGER
#include "PluginLogger.h"
#define COMMON_LOG_DEBUG PLUGIN_LOG
#define COMMON_LOG_INFO PLUGIN_LOG_INFO
#define COMMON_LOG_WARN PLUGIN_LOG_WARN
#define COMMON_LOG_ERR PLUGIN_LOG_ERR
#else
#define COMMON_LOG_DEBUG LOG
#define COMMON_LOG_INFO LOG_INFO
#define COMMON_LOG_WARN LOG_WARN
#define COMMON_LOG_ERR LOG_ERR
#endif

#ifdef AMD_USE_THREADED_QUEUE
#include "amdQuoteType.h"
class DailyIndex;
#endif

// static const string STOP_EXCEPTION_ERRMSG = " data being processed is discarded due to unsubscription";

static const string START_TIME_STR = "startTime";
static const string END_TIME_STR = "endTime";
static const string FIRST_MSG_TIME_STR = "firstMsgTime";
static const string LAST_MSG_TIME_STR = "lastMsgTime";
static const string PROCESSED_MSG_COUNT_STR = "processedMsgCount";
static const string FAILED_MSG_COUNT_STR = "failedMsgCount";
static const string LAST_ERR_MSG_STR = "lastErrMsg";
static const string LAST_FAILED_TIMESTAMP_STR = "lastFailedTimestamp";

static const string RECEIVED_TIME_OPTION_STR = "receivedTime";
static const string OUTPUT_RECV_TIME_OPTION_STR = "outputRecvTime";
static const string OUTPUT_ELAPSED_OPTION_STR = "outputElapsed";

struct MetaTable {
    vector<string> colNames_;
    vector<DATA_TYPE> colTypes_;
};

typedef enum StreamOptionFlag {
    OPT_RECEIVED = 0b01,
    OPT_ELAPSED = 0b10,
#ifdef AMD_USE_THREADED_QUEUE
    OPT_DAILY_INDEX = 0b100,
#endif
} MarketOptionFlag;

typedef struct StreamStatus {
    string name_;
    long long startTime_ = LONG_LONG_MIN;
    long long endTime_ = LONG_LONG_MIN;
    long long firstMsgTime_ = LONG_LONG_MIN;
    long long lastMsgTime_ = LONG_LONG_MIN;
    long long processedMsgCount_ = 0;  // total msg count
    long long failedMsgCount_ = 0;     // only err msg count
    string lastErrMsg_;
    long long lastFailedTimestamp_ = LONG_LONG_MIN;
    long long queueDepthLimit_ = LONG_LONG_MIN;
    long long queueDepth_ = 0;

    DictionarySP getStatus() const {
        DictionarySP dict = Util::createDictionary(DT_STRING, NULL, DT_ANY, NULL);
        dict->set(new String("name"), new String(name_));
        dict->set(new String(START_TIME_STR), new NanoTimestamp(startTime_));
        dict->set(new String(END_TIME_STR), new NanoTimestamp(endTime_));
        dict->set(new String(FIRST_MSG_TIME_STR), new NanoTimestamp(firstMsgTime_));
        dict->set(new String(LAST_MSG_TIME_STR), new NanoTimestamp(lastMsgTime_));
        dict->set(new String(PROCESSED_MSG_COUNT_STR), new Long(processedMsgCount_ - failedMsgCount_));
        dict->set(new String(FAILED_MSG_COUNT_STR), new Long(failedMsgCount_));
        dict->set(new String(LAST_ERR_MSG_STR), new String(lastErrMsg_));
        dict->set(new String(LAST_FAILED_TIMESTAMP_STR), new NanoTimestamp(lastFailedTimestamp_));
        return dict;
    }
} MarketStatus;

namespace std {
inline static std::string to_string(const std::string &str) { return str; }
}  // namespace std

namespace ThreadedQueueUtil {
template <typename T>
class StringConvertor {
  public:
    string operator()(const T &element) { return std::to_string(element); }
};
}  // namespace ThreadedQueueUtil

template <typename T = string, typename F = ThreadedQueueUtil::StringConvertor<T>>
class MarketTypeContainer {
  public:
    MarketTypeContainer() = default;
    explicit MarketTypeContainer(const string &prefix) : prefix_(prefix), toStringFunc_(F()) {}
    explicit MarketTypeContainer(const string &prefix, const vector<T> &name, const vector<MetaTable> &meta)
        : prefix_(prefix), toStringFunc_(F()) {
        if (name.size() != meta.size()) {
            throw RuntimeException(prefix_ + "name & meta is not the same length.");
        }
        for (unsigned int i = 0; i < name.size(); ++i) {
            metaMap_[name[i]] = meta[i];
        }
    }

    void add(const T &type, const MetaTable &meta) { metaMap_[type] = meta; }

    MetaTable get(const T &type) {
        if (metaMap_.find(type) == metaMap_.end()) {
            throw RuntimeException(prefix_ + "unknown Type: " + toStringFunc_(type));
        }
        return metaMap_[type];
    }

    bool contain(const T &type) {
        if (metaMap_.find(type) == metaMap_.end()) {
            return false;
        }
        return true;
    }

    ConstantSP getSchema(const T &type, int flag) {
        if (metaMap_.find(type) == metaMap_.end()) {
            throw RuntimeException(prefix_ + "unknown type: " + toStringFunc_(type));
        }

        string receiveTimeColName = "receivedTime";
        DATA_TYPE receiveTimeType = DT_NANOTIMESTAMP;
        string receivedTimeTypeStr = Util::getDataTypeString(receiveTimeType);
        string elapsedColName = "perPenetrationTime";
        DATA_TYPE elapsedType = DT_LONG;
        string elapsedTypeStr = Util::getDataTypeString(elapsedType);
#ifdef AMD_USE_THREADED_QUEUE
        string dailyIndexColName = "dailyIndex";
        DATA_TYPE dailyIndexType = DT_INT;
        string dailyIndexTypeStr = Util::getDataTypeString(dailyIndexType);
#endif

        VectorSP name = Util::createVector(DT_STRING, 0);
        VectorSP typeStr = Util::createVector(DT_STRING, 0);
        VectorSP typeInt = Util::createVector(DT_INT, 0);
        vector<string> retNames{"name", "typeString", "typeInt"};
        vector<string> colName;
        vector<DATA_TYPE> colType;

        MetaTable &target = metaMap_[type];
        colName = target.colNames_;
        colType = target.colTypes_;

        name->reserve(colName.size() + 2);
        typeStr->reserve(colName.size() + 2);
        typeInt->reserve(colName.size() + 2);

        vector<string> colStr;
        for (DATA_TYPE &t : colType) {
            colStr.push_back(Util::getDataTypeString(t));
        }
        name->appendString(colName.data(), colName.size());
        typeStr->appendString(colStr.data(), colName.size());
        typeInt->appendInt((int *)colType.data(), colName.size());

        if (flag & MarketOptionFlag::OPT_RECEIVED) {
            name->appendString(&receiveTimeColName, 1);
            typeStr->appendString(&receivedTimeTypeStr, 1);
            typeInt->appendInt((int *)&receiveTimeType, 1);
        }
#ifdef AMD_USE_THREADED_QUEUE
        // this order matters, dailyIndex must between receivedTime & outputElapsed
        if (flag & MarketOptionFlag::OPT_DAILY_INDEX) {
            name->appendString(&dailyIndexColName, 1);
            typeStr->appendString(&dailyIndexTypeStr, 1);
            typeInt->appendInt((int *)&dailyIndexType, 1);
        }
#endif
        if (flag & MarketOptionFlag::OPT_ELAPSED) {
            name->appendString(&elapsedColName, 1);
            typeStr->appendString(&elapsedTypeStr, 1);
            typeInt->appendInt((int *)&elapsedType, 1);
        }

        vector<ConstantSP> retCols{name, typeStr, typeInt};
        return Util::createTable(retNames, retCols);
    }

  private:
    string prefix_;
    unordered_map<T, MetaTable> metaMap_;
    F toStringFunc_;
};

namespace ThreadedQueueUtil {
template <class T>
struct Sizer {
    int operator()(const T &obj) { return 1; }
};

template <class T>
struct Mandatory {
    bool operator()(const T &obj) { return false; }
};
}  // namespace ThreadedQueueUtil

using ThreadedQueueUtil::Sizer;
using ThreadedQueueUtil::Mandatory;
/**
 * @brief  An Asynchronous Struct Conversion Framework
 * transform struct into dolphindb table asynchronously
 *
 * @tparam DataStruct Type of Struct to be Converted
 */
template <typename DataStruct,
          typename InnerQueue = GenericBoundedQueue<DataStruct, Sizer<DataStruct>, Mandatory<DataStruct>>>
class ThreadedQueue {
  public:
    ThreadedQueue() = delete;
    ThreadedQueue operator=(const ThreadedQueue &queue) = delete;
    /**
     * @brief Construct a new Threaded Queue object
     *
     * @param heap         heap of current session
     * @param timeout      queue pop timeout
     * @param capacity     capacity of async queue
     * @param meta         schema of origin output data
     * @param schema       schema of converted output data
     * @param flag         flags of MarketOptionFlag
     * @param info         info for this ThreadedQueue
     * @param prefix       prefix for log & exception
     * @param bufferSize   queue pop size
     * @param structReader function to convert struct to ddb vector
     */
    ThreadedQueue(Heap *heap, int timeout, long long capacity, const MetaTable &meta, TableSP schema, int flag,
                  const string &info, const string &prefix, long long bufferSize,
                  std::function<void(vector<ConstantSP> &, DataStruct &)> structReader)
        : stopFlag_(true),
          receivedTimeFlag_(flag & MarketOptionFlag::OPT_RECEIVED),
          outputElapsedFlag_(flag & MarketOptionFlag::OPT_ELAPSED),
#ifdef AMD_USE_THREADED_QUEUE
          dailyIndexFlag_(flag & MarketOptionFlag::OPT_DAILY_INDEX),
#endif
          timeout_(timeout),
          bufferSize_(bufferSize),
          info_(info),
          prefix_(prefix),
          meta_(meta),
          queue_(capacity, Sizer<DataStruct>(), Mandatory<DataStruct>()),
          structReader_(structReader) {
        initQueueBuffer();
        session_ = heap->currentSession()->copy();
        session_->setUser(heap->currentSession()->getUser());
        status_.name_ = info;
        status_.queueDepthLimit_ = capacity;

        long long currentTime = Util::getNanoEpochTime();
        localTimeGap_ = Util::toLocalNanoTimestamp(currentTime) - currentTime;

        if (!schema.isNull() && !schema->isNull()) {
            if (schema->getForm() != DF_TABLE) {
                throw RuntimeException(prefix_ + "schema must be a table");
            }
            if (schema->columns() != 4 || schema->getColumnName(0) != "type" || schema->getColumnName(1) != "name" ||
                schema->getColumnName(2) != "format" || schema->getColumnName(3) != "col") {
                throw RuntimeException(prefix_ +
                                       "schema must be a table composed of four columns: name, type, format, and col.");
            }
            if (schema->getColumnType(0) != DT_STRING || schema->getColumnType(1) != DT_STRING ||
                schema->getColumnType(2) != DT_STRING || schema->getColumnType(3) != DT_INT) {
                throw RuntimeException(prefix_ +
                                       "The types of the four columns in schema must be: STRING, STRING, STRING, INT");
            }
            INDEX size = schema->rows();
            VectorSP namesCol = schema->getColumn(0);
            char *buf[size];
            char **ret = namesCol->getStringConst(0, size, buf);
            for (INDEX i = 0; i < size; ++i) {
                resultColNames_.push_back(string(ret[i]));
            }
            VectorSP numsCol = schema->getColumn(3);
            int numBuf[size];
            const int *numRet = numsCol->getIntConst(0, size, numBuf);
            for (INDEX i = 0; i < size; ++i) {
                resultColNums_.push_back(numRet[i]);
            }
        }
    }

    ~ThreadedQueue() { stop(); }
    bool isStarted() const { return !stopFlag_; }
    // NOTE deprecate
    DictionarySP getStatus() const { return status_.getStatus(); }
    const MarketStatus &getStatusConst() const { return status_; }
    string getInfo() const { return info_; };

    // WARNING: The start() and stop() functions must not be called concurrently
    // start a ThreadedQueue
    void start() {
        status_.startTime_ = Util::getNanoEpochTime() + localTimeGap_;
        status_.endTime_ = LONG_LONG_MIN;
        status_.firstMsgTime_ = LONG_LONG_MIN;
        status_.lastMsgTime_ = LONG_LONG_MIN;
        status_.processedMsgCount_ = 0;
        status_.lastErrMsg_ = "";
        status_.failedMsgCount_ = 0;
        status_.lastFailedTimestamp_ = LONG_LONG_MIN;
        if (stopFlag_) {
            stopFlag_ = false;
            startThread();
        }
    }

    // stop a ThreadedQueue
    void stop() {
        if (!stopFlag_) {
            status_.endTime_ = Util::getNanoEpochTime() + localTimeGap_;
            stopFlag_ = true;
            thread_->join();
            DataStruct item;
            queue_.pop(item);
            queue_.clear();
        }
    }

#ifdef AMD_USE_THREADED_QUEUE
    void setDailyIndex(long long timestamp) { dailyIndex_ = DailyIndex(timestamp); }
#endif
    void setTable(TableSP table) { insertedTable_ = table; }
    void setTransform(FunctionDefSP func) { transform_ = func; }
    void setFinalizer(std::function<void(vector<DataStruct> &)> finalizer) { finalizer_ = finalizer; }

    // if set true, timeout would act like throttle
    void setTimeoutAsThrottle(bool flag) { timeoutAsThrottle_ = flag; }
    // use to ignore table insert
    void ignoreTableInsert() { ignoreTableInsert_ = true; }

    // use push function to input the struct data.
    void push(DataStruct &&data) {
        if (LIKELY(!stopFlag_)) {
            queue_.blockingPush(std::move(data));
        }
    }
    void push(const DataStruct &data) {
        if (LIKELY(!stopFlag_)) {
            queue_.blockingPush(data);
        }
    }

    void setError(const string &errMsg, uint32_t failedMsgCount = 0) {
        status_.lastErrMsg_ = errMsg;
        status_.lastFailedTimestamp_ = Util::getNanoEpochTime() + localTimeGap_;
        status_.failedMsgCount_ += failedMsgCount;
        COMMON_LOG_INFO(prefix_, info_, " Failed to process ", failedMsgCount, " lines of data due to ", errMsg);
    }

    // get outputTable
    TableSP getTable() { return insertedTable_; }

  private:
    void extractHelper(vector<DataStruct> &data) {
        int cnt = data.size();
        if (UNLIKELY(cnt == 0)) return;
        vector<long long> reachTimeVec;
        if (receivedTimeFlag_ || outputElapsedFlag_) {
            reachTimeVec.reserve(cnt);
        }
#ifdef AMD_USE_THREADED_QUEUE
        vector<long long> dailyIndexVec;
        if (dailyIndexFlag_) {
            dailyIndexVec.reserve(cnt);
        }
#endif
        clearQueueBuffer();
        for (uint32_t i = 0; i < cnt; ++i) {
            structReader_(buffer_, data[i]);

#ifdef AMD_USE_THREADED_QUEUE
            if (dailyIndexFlag_) {
                // use reachTime, not convertTime(ticks[i].uni.tickExecution.exec_time)))
                int dailyIndex = getDailyIndex<DataStruct>(dailyIndex_, data[i], data[i].reachTime);
                dailyIndexVec.push_back(dailyIndex);
            }
#endif
            // If the receivedTimeFlag_ or outputElapsedFlag_ are enabled
            // you need to ensure that the passed struct has the 'reachTime' field
            if (receivedTimeFlag_ || outputElapsedFlag_) {
                reachTimeVec.push_back(data[i].reachTime);
                status_.lastMsgTime_ = data[i].reachTime;
                if (UNLIKELY(status_.firstMsgTime_ == LONG_LONG_MIN)) status_.firstMsgTime_ = status_.lastMsgTime_;
            } else {
                status_.lastMsgTime_ = Util::getNanoEpochTime() + localTimeGap_;
                if (UNLIKELY(status_.firstMsgTime_ == LONG_LONG_MIN)) status_.firstMsgTime_ = status_.lastMsgTime_;
            }
        }
        vector<ConstantSP> cols;
        for (unsigned int i = 0; i < meta_.colNames_.size(); ++i) {
            cols.push_back(buffer_[i]);
        }
        vector<string> colNames = meta_.colNames_;

        // add some additional field to output
        if (receivedTimeFlag_) {
            colNames.push_back("receivedTime");
            receivedTimeVec_->appendLong(reachTimeVec.data(), reachTimeVec.size());
            cols.push_back(receivedTimeVec_);
        }
#ifdef AMD_USE_THREADED_QUEUE
        // this order matters, dailyIndex must between receivedTime & outputElapsed
        if (dailyIndexFlag_) {
            colNames.push_back("dailyIndex");
            dailyIndexVec_->appendLong(dailyIndexVec.data(), dailyIndexVec.size());
            cols.push_back(dailyIndexVec_);
        }
#endif
        if (outputElapsedFlag_) {
            colNames.push_back("perPenetrationTime");
            long long time = Util::getNanoEpochTime() + localTimeGap_;
            for (int i = 0; i < cols[0]->size(); ++i) {
                long long gap = time - reachTimeVec[i];
                outputElapsedVec_->appendLong(&gap, 1);
            }
            cols.push_back(outputElapsedVec_);
        }

        // convert output according to schema
        if (!resultColNums_.empty()) {
            vector<ConstantSP> convertCols;
            for (unsigned int i = 0; i < resultColNums_.size(); ++i) {
                if (UNLIKELY(i >= cols.size())) {
                    throw RuntimeException("schema col number (" + std::to_string(i) + ") exceed origin result size (" +
                                           std::to_string(cols.size()) + ")");
                }
                convertCols.push_back(cols[resultColNums_[i]]);
            }
            colNames = resultColNames_;
            cols = convertCols;
        }

        for (ConstantSP &col : cols) {
            col->setNullFlag(true);
        }

        TableSP originData = Util::createTable(colNames, cols);
        vector<ConstantSP> args = {originData};

        try {
            if (LIKELY(!transform_.isNull())) originData = transform_->call(session_->getHeap().get(), args);
        } catch (exception &e) {
            throw RuntimeException("call transform error: " + string(e.what()));
        }
        if (originData->getForm() != DF_TABLE) {
            throw RuntimeException("transform result must be a TABLE");
        }

        if (UNLIKELY(ignoreTableInsert_)) {
            return;
        }
        if (UNLIKELY(insertedTable_.isNull())) throw RuntimeException("insertedTable is null");
        if (UNLIKELY(insertedTable_->columns() != originData->columns()))
            throw RuntimeException("data append failed: The number of columns (" +
                                   std::to_string(originData->columns()) +
                                   ") being inserted must match the column number (" +
                                   std::to_string(insertedTable_->columns()) + ") of the insertedTable");

        INDEX rows;
        string errMsg;
        // TODO column verification
        // It may not be necessary, as sometimes the intention is to utilize the forced type conversion
        // in the 'append' operation to achieve certain goals.
        LockGuard<Mutex> _(insertedTable_->getLock());
        vector<ConstantSP> appendCols;
        INDEX appendSize = originData->columns();
        for (INDEX i = 0; i < appendSize; ++i) {
            appendCols.push_back(originData->getColumn(i));
        }
        if (LIKELY(insertedTable_->getTableType() == REALTIMETBL || insertedTable_->getTableType() == STREAMENGINE)) {
            for (auto &col : appendCols) {
                col = col->getValue();
            }
        }
        insertedTable_->append(appendCols, rows, errMsg);
        if (UNLIKELY(errMsg != "")) {
            throw RuntimeException("data append failed, " + errMsg);
        }
    }

    void initQueueBuffer() {
        buffer_ = vector<ConstantSP>(meta_.colNames_.size());
        for (unsigned int i = 0; i < meta_.colNames_.size(); ++i) {
            if (meta_.colTypes_[i] == DT_SYMBOL) {
                buffer_[i] = Util::createVector(DT_STRING, 0, bufferSize_);
                ((VectorSP)buffer_[i])->initialize();
            } else if (meta_.colTypes_[i] >= ARRAY_TYPE_BASE) {
                buffer_[i] = InternalUtil::createArrayVector(meta_.colTypes_[i], 0, 0, bufferSize_);
            } else {
                buffer_[i] = Util::createVector(meta_.colTypes_[i], 0, bufferSize_);
                ((VectorSP)buffer_[i])->initialize();
            }
        }
        receivedTimeVec_ = Util::createVector(DT_NANOTIMESTAMP, 0, bufferSize_);
        outputElapsedVec_ = Util::createVector(DT_LONG, 0, bufferSize_);
#ifdef AMD_USE_THREADED_QUEUE
        dailyIndexVec_ = Util::createVector(DT_LONG, 0, bufferSize_);
#endif
    }
    void clearQueueBuffer() {
        for (unsigned int i = 0; i < buffer_.size(); ++i) {
            ((VectorSP)(buffer_[i]))->clear();
        }
        receivedTimeVec_->clear();
        outputElapsedVec_->clear();
#ifdef AMD_USE_THREADED_QUEUE
        dailyIndexVec_->clear();
#endif
    }

    void startThread() {
        std::function<void(vector<DataStruct> &)> dealFunc = [this](vector<DataStruct> &data) {
            (ThreadedQueue::extractHelper)(data);
        };

        std::function<void()> f = [this, dealFunc]() {
            COMMON_LOG_INFO(prefix_, info_, " async thread start ");
            bool ret;
            int popSize = 0;
            DataStruct item;
            vector<DataStruct> items;
            items.reserve(bufferSize_);
            while (LIKELY(!stopFlag_)) {
                items.clear();
                long long size = 0;
                status_.queueDepth_ = queue_.size();
                try {
                    if (UNLIKELY(timeoutAsThrottle_)) {
                        long long startTime = Util::getEpochTime();
                        long long currentTime = startTime;
                        popSize = 0;
                        do {
                            dolphindb::PluginDefer([&]() { currentTime = Util::getEpochTime(); });
                            int timeout = timeout_ - (currentTime - startTime);
                            ret = queue_.blockingPop(item, timeout);
                            if (UNLIKELY(!ret)) {
                                if (UNLIKELY(stopFlag_)) {
                                    break;
                                } else {
                                    continue;
                                }
                            }
                            items.emplace_back(std::move(item));
                            long long count = bufferSize_ - items.size();
                            size = std::min(queue_.size(), count);
                            if (LIKELY(size > 0)) queue_.pop(items, size);
                            popSize = items.size();
                        } while (currentTime - startTime < timeout_ && popSize < bufferSize_);

                        if (UNLIKELY(popSize == 0)) {
                            if (UNLIKELY(stopFlag_)) {
                                break;
                            }
                            COMMON_LOG_DEBUG(prefix_, info_, " async thread pop size (0)");
                        }
                    } else {
                        ret = queue_.blockingPop(item, timeout_);
                        if (UNLIKELY(!ret)) {
                            if (UNLIKELY(stopFlag_)) {
                                break;
                            } else {
                                COMMON_LOG_DEBUG(prefix_, info_, " async thread pop size (0)");
                                continue;
                            }
                        }
                        size = std::min(queue_.size(), (long long)bufferSize_ - 1);
                        items.emplace_back(std::move(item));
                        if (LIKELY(size > 0)) queue_.pop(items, size);
                        popSize = items.size();
                    }
                    status_.processedMsgCount_ += popSize;
                    COMMON_LOG_DEBUG(prefix_, info_, " async thread pop size (", items.size(), ")");
                    dealFunc(items);
                    if (finalizer_) {
                        finalizer_(items);
                    }
                    items.clear();
                } catch (exception &e) {
                    string errMsg = e.what();
                    status_.failedMsgCount_ += popSize;
                    status_.lastErrMsg_ = "topic=" + info_ + " length=" + std::to_string(popSize) + " exception=" + errMsg;
                    status_.lastFailedTimestamp_ = Util::getNanoEpochTime() + localTimeGap_;
                    COMMON_LOG_ERR(prefix_, info_, " Failed to process ", popSize, " lines of data due to ", errMsg);
                }
            }
            COMMON_LOG_INFO(prefix_, info_, " async thread end.");
        };

        SmartPointer<dolphindb::Executor> executor = new dolphindb::Executor(f);
        thread_ = new Thread(executor);
        thread_->start();
    }

  private:
    bool stopFlag_ = true;
    bool receivedTimeFlag_;
    bool outputElapsedFlag_;
    bool timeoutAsThrottle_ = false;
    bool ignoreTableInsert_ = false;
#ifdef AMD_USE_THREADED_QUEUE
    bool dailyIndexFlag_;
    DailyIndex dailyIndex_;
    VectorSP dailyIndexVec_;
#endif
    VectorSP receivedTimeVec_;
    VectorSP outputElapsedVec_;
    int timeout_;
    long long bufferSize_;
    long long localTimeGap_ = 0;
    string info_;
    string prefix_;
    SessionSP session_;
    Mutex mutex_;
    MetaTable meta_;
    TableSP insertedTable_ = nullptr;
    FunctionDefSP transform_ = nullptr;
    InnerQueue queue_;
    ThreadSP thread_;
    MarketStatus status_;
    vector<INDEX> resultColNums_;
    vector<string> resultColNames_;
    std::function<void(vector<ConstantSP> &, DataStruct &)> structReader_;
    vector<ConstantSP> buffer_;
    std::function<void(vector<DataStruct> &)> finalizer_;
};

namespace ThreadedQueueUtil {

template <typename T>
class TimedWrapper {
  public:
    T data;
    long long reachTime;
};

template <typename T>
struct TimedWrapperTrait {
    typedef SmartPointer<ThreadedQueue<TimedWrapper<T>>> Type;
};

template <typename T>
class orderbookReaderWrapper {
  public:
    typedef std::function<void(vector<ConstantSP> &, T &, int, std::unordered_map<int, long long> &,
                               std::unordered_map<int, long long> &)>
        FuncTrait;
    orderbookReaderWrapper(FuncTrait reader, int seqCheckMode) : seqCheckMode_(seqCheckMode), reader_(reader) {}
    void operator()(vector<ConstantSP> &buffer, T &data) {
        reader_(buffer, data, seqCheckMode_, shLastSeqNum_, szLastSeqNum_);
    }

  private:
    int seqCheckMode_;
    FuncTrait reader_;
    std::unordered_map<int, long long> shLastSeqNum_;
    std::unordered_map<int, long long> szLastSeqNum_;
};

typedef vector<ConstantSP>::iterator ConstantVecIterator;

inline VectorSP getVec(const ConstantVecIterator &colIter) { return *colIter; }
inline void appendString(const ConstantVecIterator &colIter, const string &data) {
    getVec(colIter)->appendString(&data, 1);
}
inline void appendInt(const ConstantVecIterator &colIter, int data) { getVec(colIter)->appendInt(&data, 1); }
inline void appendShort(const ConstantVecIterator &colIter, short data) { getVec(colIter)->appendShort(&data, 1); }
inline void appendLong(const ConstantVecIterator &colIter, long long data) { getVec(colIter)->appendLong(&data, 1); }
inline void appendFloat(const ConstantVecIterator &colIter, float data) { getVec(colIter)->appendFloat(&data, 1); }
inline void appendDouble(const ConstantVecIterator &colIter, double data) { getVec(colIter)->appendDouble(&data, 1); }
inline void appendChar(const ConstantVecIterator &colIter, char data) { getVec(colIter)->appendChar(&data, 1); }

}  // namespace ThreadedQueueUtil

namespace MarketUtil {
struct SeqCheckMode {
    enum {
        CHECK = 0,
        IGNORE_WITH_LOG,
#ifdef WIN32
        IGNORE_ALL,
#else
        IGNORE,
#endif
    };
};

struct OrderBookMsgType {
    enum {
        PRODUCT_STATUS = -1,
        ORDER = 0,
        TRADE = 1,
    };
};

struct OrderBookBSFlag {
    enum {
        UNKNOWN = 0,
        BUY = 1,
        SELL = 2,
    };
};

struct OrderBookOrderType {
    enum {
        MARKET_ORDER = 1,
        LIMIT_ORDER = 2,
        BEST = 3,
        CANCEL = 10,
        STATUS = 11,
    };
};

struct OrderBookTradeType {
    enum {
        DEAL = 0,
        CANCEL = 1,
    };
};

static inline void checkSeqNum(const string &prefix, const string &tag, std::unordered_map<int, long long> &lastSeqNum,
                               long long seqNum, int channelNo, int seqCheckMode) {
    if (lastSeqNum.find(channelNo) != lastSeqNum.end() && seqNum != lastSeqNum[channelNo] + 1) {
        std::stringstream errMsgStream;
        errMsgStream << "wrong " << tag << " channel " << channelNo << ", expected " << lastSeqNum[channelNo] + 1
                     << ", actually " << seqNum;
        std::string errMsg = errMsgStream.str();
        if (seqCheckMode == SeqCheckMode::CHECK) {
            if (lastSeqNum[channelNo] == -1)
                throw RuntimeException("channel " + std::to_string(channelNo) +
                                       " stops receiving data due to data discontinuity of " + tag);
            else {
                lastSeqNum[channelNo] = -1;
                throw RuntimeException(errMsg);
            }
        }
        if (seqCheckMode == SeqCheckMode::IGNORE_WITH_LOG) {
            COMMON_LOG_INFO(prefix, errMsg);
        }
    }
    lastSeqNum[channelNo] = seqNum;
}

}  // namespace MarketUtil

#undef COMMON_LOG_DEBUG
#undef COMMON_LOG_INFO
#undef COMMON_LOG_WARN
#undef COMMON_LOG_ERR

#endif
