#ifndef __AMD_SPI_IMP_H
#define __AMD_SPI_IMP_H

#include <mutex>
#include <ostream>
#include <string>
#include <unordered_map>
#include <iostream>

#include "Exceptions.h"
#include "FlatHashmap.h"
#include "Types.h"
#include "ama.h"
#include "ama_tools.h"

#include "CoreConcept.h"
#include "amdQuoteImp.h"
#include "amdQuoteType.h"
#include "Logger.h"
#include "Util.h"
#include "ScalarImp.h"
#include "Plugin.h"

using dolphindb::Executor;
const static int CAPACITY = 100000;
static std::atomic<bool> ERROR_LOG(false);

class AMDSpiImp : public amd::ama::IAMDSpi {
public:
    AMDSpiImp(SessionSP session) :
    snapshotFlag_(false),
    executionFlag_(false),
    orderFlag_(false),
    indexFlag_(false),
    orderQueueFlag_(false),
    fundSnapshotFlag_(false),
    fundExecutionFlag_(false),
    fundOrderFlag_(false),
    bondSnapshotFlag_(false),
    bondExecutionFlag_(false),
    bondOrderFlag_(false),
    snapshotBoundQueue_(new SnapshotQueue(CAPACITY, ObjectSizer<timeMDSnapshot>(), ObjectUrgency<timeMDSnapshot>())),
    orderBoundQueue_(new OrderQueue(CAPACITY, ObjectSizer<timeMDTickOrder>(), ObjectUrgency<timeMDTickOrder>())),
    executionBoundQueue_(new ExecutionQueue(CAPACITY, ObjectSizer<timeMDTickExecution>(), ObjectUrgency<timeMDTickExecution>())),
    fundSnapshotBoundQueue_(new SnapshotQueue(CAPACITY, ObjectSizer<timeMDSnapshot>(), ObjectUrgency<timeMDSnapshot>())),
    fundOrderBoundQueue_(new OrderQueue(CAPACITY, ObjectSizer<timeMDTickOrder>(), ObjectUrgency<timeMDTickOrder>())),
    fundExecutionBoundQueue_(new ExecutionQueue(CAPACITY, ObjectSizer<timeMDTickExecution>(), ObjectUrgency<timeMDTickExecution>())),
    bondSnapshotBoundQueue_(new BondSnapshotQueue(CAPACITY, ObjectSizer<timeMDBondSnapshot>(), ObjectUrgency<timeMDBondSnapshot>())),
    bondOrderBoundQueue_(new BondOrderQueue(CAPACITY, ObjectSizer<timeMDBondTickOrder>(), ObjectUrgency<timeMDBondTickOrder>())),
    bondExecutionBoundQueue_(new BondExecutionQueue(CAPACITY, ObjectSizer<timeMDBondTickExecution>(), ObjectUrgency<timeMDBondTickExecution>())),
    indexBoundQueue_(new IndexQueue(CAPACITY, ObjectSizer<timeMDIndexSnapshot>(), ObjectUrgency<timeMDIndexSnapshot>())),
    orderQueueBoundQueue_(new OrderQueueQueue(CAPACITY, ObjectSizer<timeMDOrderQueue>(), ObjectUrgency<timeMDOrderQueue>())),

    orderExecutionStopFlag_(new bool(true)),
    fundOrderExecutionStopFlag_(new bool(true)),
    bondOrderExecutionStopFlag_(new bool(true)),

    orderExecutionFlag_(false),
    fundOrderExecutionFlag_(false),
    bondOrderExecutionFlag_(false),

    orderExecutionData_({}),
    fundOrderExecutionData_({}),
    bondOrderExecutionData_({}),

    orderExecutionBoundQueue_({}),
    fundOrderExecutionBoundQueue_({}),
    bondOrderExecutionBoundQueue_({}),

    orderExecutionThread_({}),
    fundOrderExecutionThread_({}),
    bondOrderExecutionThread_({}),

    stopFlag_(new bool()),

    session_(session),

    dailyIndexFlag_(DAILY_INDEX_FLAG),
    receivedTimeFlag_(RECEIVED_TIME_FLAG),
    outputElapsedFlag_(OUTPUT_ELAPSED_FLAG)
        {
        *stopFlag_ = false;

        initQueueBuffer<AmdOrderTableMeta>(orderBuffer_, orderTableMeta_);
        initQueueBuffer<AmdOrderTableMeta>(fundOrderBuffer_, orderTableMeta_);
        initQueueBuffer<AmdOrderTableMeta>(bondOrderBuffer_, orderTableMeta_);

        initQueueBuffer<AmdIndexTableMeta>(indexBuffer_, indexTableMeta_);
        initQueueBuffer<AmdOrderQueueTableMeta>(orderQueueBuffer_, orderQueueMeta_);

        initQueueBuffer<AmdExecutionTableMeta>(executionBuffer_, executionTableMeta_);
        initQueueBuffer<AmdExecutionTableMeta>(fundExecutionBuffer_, executionTableMeta_);
        initQueueBuffer<AmdExecutionTableMeta>(bondExecutionBuffer_, executionTableMeta_);

        initQueueBuffer<AmdSnapshotTableMeta>(snapshotBuffer_, snapshotDataTableMeta_);
        initQueueBuffer<AmdSnapshotTableMeta>(fundSnapshotBuffer_, snapshotDataTableMeta_);
        initQueueBuffer<AmdSnapshotTableMeta>(bondSnapshotBuffer_, snapshotDataTableMeta_);

        startThread<timeMDSnapshot, SnapshotQueue>(snapshotBoundQueue_,
            &AMDSpiImp::OnMDSnapshotHelper, "snapshot", snapshotThread_);
        startThread<timeMDTickOrder, OrderQueue>(orderBoundQueue_,
            &AMDSpiImp::OnMDTickOrderHelper, "order", orderThread_);
        startThread<timeMDTickExecution, ExecutionQueue>(executionBoundQueue_,
            &AMDSpiImp::OnMDTickExecutionHelper, "execution", executionThread_);

        startThread<timeMDSnapshot, SnapshotQueue>(fundSnapshotBoundQueue_,
            &AMDSpiImp::OnMDFundSnapshotHelper, "fundSnapshot", fundSnapshotThread_);
        startThread<timeMDTickOrder, OrderQueue>(fundOrderBoundQueue_,
            &AMDSpiImp::OnMDTickFundOrderHelper, "fundOrder", fundOrderThread_);
        startThread<timeMDTickExecution, ExecutionQueue>(fundExecutionBoundQueue_,
            &AMDSpiImp::OnMDTickFundExecutionHelper, "fundExecution", fundExecutionThread_);

        startThread<timeMDBondSnapshot, BondSnapshotQueue>(bondSnapshotBoundQueue_,
            &AMDSpiImp::OnMDBondSnapshotHelper, "bondSnapshot", bondSnapshotThread_);
        startThread<timeMDBondTickOrder, BondOrderQueue>(bondOrderBoundQueue_,
            &AMDSpiImp::OnMDBondTickOrderHelper, "bondOrder", bondOrderThread_);
        startThread<timeMDBondTickExecution, BondExecutionQueue>(bondExecutionBoundQueue_,
            &AMDSpiImp::OnMDBondTickExecutionHelper, "bondExecution", bondExecutionThread_);

        startThread<timeMDIndexSnapshot, IndexQueue>(indexBoundQueue_,
            &AMDSpiImp::OnMDIndexSnapshotHelper, "index", indexThread_);
        startThread<timeMDOrderQueue, OrderQueueQueue>(orderQueueBoundQueue_,
            &AMDSpiImp::OnMDOrderQueueHelper, "orderQueue", orderQueueThread_);
    }

    template <typename DataStruct, typename Queue>
    Executor* initExecutor(
        SmartPointer<Queue>& queue,
        void (AMDSpiImp::* helperFunc)(DataStruct*, uint32_t),
        const string& typeStr)
    {
        return new Executor(std::bind(&blockHandling<DataStruct>, queue,
            [this, helperFunc](vector<DataStruct>& data){
                (this->*helperFunc)(data.data(), data.size());
            }, stopFlag_, "[PLUGIN::AMDQUOTE]:", typeStr
        ));
    }

    template <typename DataStruct, typename Queue>
    void startThread(
        SmartPointer<Queue>& queue,
        void (AMDSpiImp::* helperFunc)(DataStruct*, uint32_t),
        const string& typeStr, ThreadSP& thread)
    {
        SmartPointer<Executor> executor = initExecutor<DataStruct, Queue>(queue, helperFunc, typeStr);
        thread = new Thread(executor);
        thread->start();
    }


    template <typename T>
    void initQueueBuffer(vector<ConstantSP>& buffer, const T& meta) {
        buffer = vector<ConstantSP>(meta.colNames_.size());
        for(unsigned int i = 0; i < meta.colNames_.size(); ++i) {
            buffer[i] = Util::createVector(meta.colTypes_[i], 0, BUFFER_SIZE);
            ((VectorSP)buffer[i])->initialize();
        }
        buffer.push_back(Util::createVector(DT_NANOTIMESTAMP, 0, BUFFER_SIZE));
        buffer.push_back(Util::createVector(DT_INT, 0, BUFFER_SIZE));
        buffer.push_back(Util::createVector(DT_NANOTIME, 0, BUFFER_SIZE));
    }

    ~AMDSpiImp(){
        LOG_INFO("[PLUGIN::AMDQUOTE]: release AMDSpiImp");
        *stopFlag_ = true;
        snapshotThread_->join();
        orderThread_->join();
        executionThread_->join();
        bondSnapshotThread_->join();
        bondOrderThread_->join();
        bondExecutionThread_->join();
        indexThread_->join();
        orderQueueThread_->join();
    }

    void startOrderExecution(string type);

    void clearOrderExecution(string type);

    string transMarket(int type);

    template <typename DataStruct, typename Meta>
    void genericAMDHelper(DataStruct* data, uint32_t cnt, TableSP& insertedTable, FunctionDefSP transform, bool& flag,
        vector<ConstantSP>& buffer, const Meta& meta, const string& typeStr, AMDDataType datatype) noexcept;

    void OnMDorderExecutionHelper(int channel, MDOrderExecution* ticks, uint32_t cnt, bool stockOrFund);
    void OnMDBondOrderExecutionHelper(int channel, MDBondOrderExecution* ticks, uint32_t cnt);

    void OnMDSnapshotHelper(timeMDSnapshot* snapshot, uint32_t cnt);
    void OnMDFundSnapshotHelper(timeMDSnapshot* snapshot, uint32_t cnt);
    void OnMDBondSnapshotHelper(timeMDBondSnapshot* snapshot, uint32_t cnt);

    void OnMDTickOrderHelper(timeMDTickOrder* ticks, uint32_t cnt);
    void OnMDTickFundOrderHelper(timeMDTickOrder* ticks, uint32_t cnt);
    void OnMDBondTickOrderHelper(timeMDBondTickOrder* ticks, uint32_t cnt);

    void OnMDTickExecutionHelper(timeMDTickExecution* tick, uint32_t cnt);
    void OnMDTickFundExecutionHelper(timeMDTickExecution* tick, uint32_t cnt);
    void OnMDBondTickExecutionHelper(timeMDBondTickExecution* tick, uint32_t cnt);

    void OnMDIndexSnapshotHelper(timeMDIndexSnapshot* index, uint32_t cnt);
    void OnMDOrderQueueHelper(timeMDOrderQueue* queue, uint32_t cnt);

    void pushSnashotData(amd::ama::MDSnapshot* snapshot, uint32_t cnt, long long time);
    void pushOrderData(amd::ama::MDTickOrder* ticks, uint32_t cnt, long long time);
    void pushExecutionData(amd::ama::MDTickExecution* ticks, uint32_t cnt, long long time);
    void pushIndexData(amd::ama::MDIndexSnapshot* index, uint32_t cnt, long long time);
    void pushOrderQueueData(amd::ama::MDOrderQueue* queue, uint32_t cnt, long long time);
    void pushBondSnapshotData(amd::ama::MDBondSnapshot* snapshots, uint32_t cnt, long long time);
    void pushBondOrderData(amd::ama::MDBondTickOrder* ticks, uint32_t cnt, long long time);
    void pushBondExecutionData(amd::ama::MDBondTickExecution* ticks, uint32_t cnt, long long time);

    // 接受并处理快照数据（包含基金和股票数据）
    virtual void OnMDSnapshot(amd::ama::MDSnapshot* snapshot, uint32_t cnt) override{
        Defer df([=](){amd::ama::IAMDApi::FreeMemory(snapshot);});
        long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
        pushSnashotData(snapshot, cnt, time);
    }
    // 接受并处理逐笔委托数据（包含基金和股票数据）
    virtual void OnMDTickOrder(amd::ama::MDTickOrder* ticks, uint32_t cnt) override{
        Defer df([=](){amd::ama::IAMDApi::FreeMemory(ticks);});
        long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
        pushOrderData(ticks, cnt, time);
    }
    // 接受并处理逐笔成交数据（包含基金和股票数据）
    virtual void OnMDTickExecution(amd::ama::MDTickExecution* tick, uint32_t cnt) override{
        Defer df([=](){amd::ama::IAMDApi::FreeMemory(tick);});
        long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
        pushExecutionData(tick, cnt, time);
    }
    // 接受并处理指数快照数据
    virtual void OnMDIndexSnapshot(amd::ama::MDIndexSnapshot* index, uint32_t cnt) override{
        Defer df([=](){amd::ama::IAMDApi::FreeMemory(index);});
        long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
        pushIndexData(index, cnt, time);
    }
    // 接受并处理委托队列数据
    virtual void OnMDOrderQueue(amd::ama::MDOrderQueue* queue, uint32_t cnt) override{
        Defer df([=](){amd::ama::IAMDApi::FreeMemory(queue);});
        long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
        pushOrderQueueData(queue, cnt, time);
    }

    virtual void OnMDBondSnapshot(amd::ama::MDBondSnapshot* snapshots, uint32_t cnt) override {
        Defer df([=](){amd::ama::IAMDApi::FreeMemory(snapshots);});
        long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
        pushBondSnapshotData(snapshots, cnt, time);
    }
    virtual void OnMDBondTickOrder(amd::ama::MDBondTickOrder* ticks, uint32_t cnt) override {
        Defer df([=](){amd::ama::IAMDApi::FreeMemory(ticks);});
        long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
        pushBondOrderData(ticks, cnt, time);
    }
    virtual void OnMDBondTickExecution(amd::ama::MDBondTickExecution* ticks, uint32_t cnt) override {
        Defer df([=](){amd::ama::IAMDApi::FreeMemory(ticks);});
        long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
        pushBondExecutionData(ticks, cnt, time);
    }

    virtual void OnEvent(uint32_t level, uint32_t code, const char* event_msg, uint32_t len) override;

    void latencyLog(AMDDataType type, long long startTime, uint32_t cnt, long long latency) {
        if(type < 0 || type >= 14) {
            return;
        }
        Statistic& stat = statistics_[type];
        if (stat.startTime != 0 && startTime > stat.endTime) { // more than 30s，print log
            if(stat.totalHandleCount > 0){
                    LOG_INFO(       "type ", type,
                                    ", statistics: total message count ", stat.totalMessageCount,
                                    ", total latency(ns) ", stat.totalLatency,
                                    ", min latency(ns) ", stat.minLatency,
                                    ", max latency(ns) ", stat.maxLatency,
                                    ", average latency(ns) ", stat.totalLatency / stat.totalHandleCount,
                                    ", average message latency(ns) ", stat.totalLatency / stat.totalMessageCount,
                                    ", total handle count", stat.totalHandleCount);
            }
            goto setValue;
        } else if (stat.startTime == 0) {
        setValue:
            stat.startTime = startTime;
            stat.endTime = stat.startTime  + duration_;
            stat.totalMessageCount = cnt;
            stat.totalHandleCount = 1;
            stat.maxLatency = stat.minLatency = stat.totalLatency = latency;
        } else  {
            if(stat.maxLatency < latency)
                stat.maxLatency = latency;
            if(stat.minLatency > latency)
                stat.minLatency = latency;
            stat.totalLatency += latency;
            stat.totalHandleCount += 1;
            stat.totalMessageCount += cnt;
        }
    }

    void unsetFlag(string type) {
        if (type == "snapshot") {
            snapshotFlag_ = false;
        } else if (type == "execution") {
            executionFlag_ = false;
        } else if (type == "order") {
            orderFlag_ = false;
        } else if (type == "index") {
            indexFlag_ = false;
        } else if (type == "orderQueue") {
            orderQueueFlag_ = false;
        } else if (type == "fundSnapshot") {
            fundSnapshotFlag_ = false;
        } else if (type == "fundExecution") {
            fundExecutionFlag_ = false;
        } else if (type == "fundOrder") {
            fundOrderFlag_ = false;
        } else if (type == "bondSnapshot") {
            bondSnapshotFlag_ = false;
        } else if (type == "bondExecution") {
            bondExecutionFlag_ = false;
        } else if (type == "bondOrder") {
            bondOrderFlag_ = false;
        } else if(type == "all") {
            snapshotFlag_ = false;
            executionFlag_ = false;
            orderFlag_ = false;
            indexFlag_ = false;
            orderQueueFlag_ = false;
            fundSnapshotFlag_ = false;
            fundExecutionFlag_ = false;
            fundOrderFlag_ = false;
            bondSnapshotFlag_ = false;
            bondExecutionFlag_ = false;
            bondOrderFlag_ = false;
        }
    }

    // set data
    void setOrderExecutionData(string type, std::unordered_map<int, TableSP> orderExecutionData, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp, int market);

    void setSnapshotData(TableSP snapshotData, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp, int market);

    void setExecutionData(TableSP executionData, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp, int market);

    void setOrderData(TableSP orderData, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp, int market);

    void setIndexData(TableSP indexData, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp, int market);

    void setOrderQueueData(TableSP orderQueueData, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp, int market);

    void setFundSnapshotData(TableSP snapshotData, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp, int market);

    void setFundExecutionData(TableSP executionData, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp, int market);

    void setFundOrderData(TableSP orderData, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp, int market);

    void setBondSnapshotData(TableSP snapshotData, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp, int market);

    void setBondExecutionData(TableSP executionData, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp, int market);

    void setBondOrderData(TableSP orderData, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp, int market);


    void setLatencyFlag(bool flag) {
        latencyFlag_ = flag;
    }

    long long getDailyIndexStarTime(const string& dataType, int market){
        AMDDataType amdDataType;
        if(NAME_TYPE.count(dataType) != 0){
            amdDataType = NAME_TYPE[dataType];
        }
        else
            throw IllegalArgumentException(__FUNCTION__, "error dataType: " + dataType);
        AMDTableType tableType = getAmdTableType(amdDataType, market);
        if((int)tableType == (int)AMD_ERROR_TABLE_TYPE){
            throw RuntimeException("getAmdTableType failed");
        }
        if(tableType >= sizeof(dailyIndex_) / sizeof(DailyIndex)){
            throw RuntimeException("tableType exceeds the size of DailyIndex_");
        }
        return dailyIndex_[tableType].getStartTimestamp();
    }

private:
    /*
        * alert!!!
        * structure with name 'order' used to deal with stock order data
        * structure with name 'fund' order is with fund order data
        * these two types of order data sometime processed together in one structure
        * sometime separate into two structure to be processed
        */
    AmdSnapshotTableMeta  snapshotDataTableMeta_;
    AmdExecutionTableMeta executionTableMeta_;
    AmdOrderTableMeta  orderTableMeta_;
    AmdIndexTableMeta indexTableMeta_;
    AmdOrderQueueTableMeta orderQueueMeta_;

    TableSP snapshotData_;
    TableSP executionData_;
    TableSP orderData_;
    TableSP indexData_;
    TableSP orderQueueData_;
    TableSP fundSnapshotData_;
    TableSP fundExecutionData_;
    TableSP fundOrderData_;
    TableSP bondSnapshotData_;
    TableSP bondExecutionData_;
    TableSP bondOrderData_;

    bool snapshotFlag_;
    bool executionFlag_;
    bool orderFlag_;
    bool indexFlag_;
    bool orderQueueFlag_;
    bool fundSnapshotFlag_;
    bool fundExecutionFlag_;
    bool fundOrderFlag_;
    bool bondSnapshotFlag_;
    bool bondExecutionFlag_;
    bool bondOrderFlag_;

    FunctionDefSP snapshotTransform_;
    FunctionDefSP executionTransform_;
    FunctionDefSP orderTransform_;
    FunctionDefSP indexTransform_;
    FunctionDefSP orderQueueTransform_;
    FunctionDefSP fundSnapshotTransform_;
    FunctionDefSP fundExecutionTransform_;
    FunctionDefSP fundOrderTransform_;
    FunctionDefSP bondSnapshotTransform_;
    FunctionDefSP bondExecutionTransform_;
    FunctionDefSP bondOrderTransform_;

    ThreadSP snapshotThread_;
    ThreadSP executionThread_;
    ThreadSP orderThread_;
    ThreadSP fundSnapshotThread_;
    ThreadSP fundExecutionThread_;
    ThreadSP fundOrderThread_;
    ThreadSP indexThread_;
    ThreadSP orderQueueThread_;
    ThreadSP bondSnapshotThread_;
    ThreadSP bondOrderThread_;
    ThreadSP bondExecutionThread_;

    SmartPointer<SnapshotQueue> snapshotBoundQueue_;
    SmartPointer<OrderQueue> orderBoundQueue_;
    SmartPointer<ExecutionQueue> executionBoundQueue_;
    SmartPointer<SnapshotQueue> fundSnapshotBoundQueue_;
    SmartPointer<OrderQueue> fundOrderBoundQueue_;
    SmartPointer<ExecutionQueue> fundExecutionBoundQueue_;
    SmartPointer<BondSnapshotQueue> bondSnapshotBoundQueue_;
    SmartPointer<BondOrderQueue> bondOrderBoundQueue_;
    SmartPointer<BondExecutionQueue> bondExecutionBoundQueue_;
    SmartPointer<IndexQueue> indexBoundQueue_;
    SmartPointer<OrderQueueQueue> orderQueueBoundQueue_;


    AmdOrderExecutionTableMeta  orderExecutionTableMeta_;

    SmartPointer<bool> orderExecutionStopFlag_;
    SmartPointer<bool> fundOrderExecutionStopFlag_;
    SmartPointer<bool> bondOrderExecutionStopFlag_;

    bool orderExecutionFlag_;
    bool fundOrderExecutionFlag_;
    bool bondOrderExecutionFlag_;

    std::unordered_map<int, TableSP> orderExecutionData_;
    std::unordered_map<int, TableSP> fundOrderExecutionData_;
    std::unordered_map<int, TableSP> bondOrderExecutionData_;

    FunctionDefSP orderExecutionTransform_;
    FunctionDefSP fundOrderExecutionTransform_;
    FunctionDefSP bondOrderExecutionTransform_;

    std::unordered_map<int, SmartPointer<OrderExecutionQueue>> orderExecutionBoundQueue_;
    std::unordered_map<int, SmartPointer<OrderExecutionQueue>> fundOrderExecutionBoundQueue_;
    std::unordered_map<int, SmartPointer<BondOrderExecutionQueue>> bondOrderExecutionBoundQueue_;

    std::unordered_map<int, ThreadSP> orderExecutionThread_;
    std::unordered_map<int, ThreadSP> fundOrderExecutionThread_;
    std::unordered_map<int, ThreadSP> bondOrderExecutionThread_;

    vector<ConstantSP> snapshotBuffer_;
    vector<ConstantSP> executionBuffer_;
    vector<ConstantSP> orderBuffer_;
    vector<ConstantSP> indexBuffer_;
    vector<ConstantSP> orderQueueBuffer_;
    vector<ConstantSP> fundSnapshotBuffer_;
    vector<ConstantSP> fundExecutionBuffer_;
    vector<ConstantSP> fundOrderBuffer_;
    vector<ConstantSP> bondSnapshotBuffer_;
    vector<ConstantSP> bondExecutionBuffer_;
    vector<ConstantSP> bondOrderBuffer_;
    std::unordered_map<int, vector<ConstantSP>> orderExecutionBuffer_;
    std::unordered_map<int, vector<ConstantSP>> fundOrderExecutionBuffer_;
    std::unordered_map<int, vector<ConstantSP>> bondOrderExecutionBuffer_;

    SmartPointer<bool> stopFlag_;
    DailyIndex dailyIndex_[28];

    struct Statistic {
        long long startTime = 0;        // Time to read first data
        long long endTime = 0;          // Statistical end time
        long long totalLatency = 0;     // Total time(ns)
        long long maxLatency = 0;       // Maximum latency of a single piece of data
        long long minLatency = 0;       // Minimum latency of a single piece of data
        long totalMessageCount = 0;     // Total message count
        long totalHandleCount = 0;      // The number of times the data was processed, aka. the number of callback function calls
    };
    Statistic statistics_[14];
    const long long duration_ = 30 * (long long)1000000000;
    SessionSP session_;

    bool latencyFlag_ = false;
    bool dailyIndexFlag_ = false;
    bool receivedTimeFlag_ = false;
    bool outputElapsedFlag_ = false;
};

#endif