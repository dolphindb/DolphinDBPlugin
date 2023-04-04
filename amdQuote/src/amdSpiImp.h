#ifndef __AMD_SPI_IMP_H
#define __AMD_SPI_IMP_H

#include <mutex>
#include <ostream>
#include <string>
#include <unordered_map>
#include <iostream>

#include "Exceptions.h"
#include "FlatHashmap.h"
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
const int CAPACITY = 100000;

// 该类主要实现回调函数，回调函数用于接受并处理数据
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
    // init three map
    // avoid crashing after use uninitialized data
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

    dailyIndexFlag_(dailyIndexFlag),
    receivedTimeFlag_(receivedTimeFlag),
    outputElapsedFlag_(outputElapsedFlag)
        {
        *stopFlag_ = false;
        SmartPointer<Executor> snapshotExecutor = new Executor(std::bind(&blockHandling<timeMDSnapshot>, snapshotBoundQueue_, 
        [this](vector<timeMDSnapshot>& data){
            OnMDSnapshotHelper(data.data(), data.size());
        }, stopFlag_, "[PluginAmdQuote]:", "snapshot"
        ));
        snapshotThread_ = new Thread(snapshotExecutor);
        snapshotThread_->start();

        SmartPointer<Executor> orderExecutor = new Executor(std::bind(&blockHandling<timeMDTickOrder>, orderBoundQueue_, 
        [this](vector<timeMDTickOrder>& data){
            OnMDTickOrderHelper(data.data(), data.size());
        }, stopFlag_, "[PluginAmdQuote]:", "order"
        ));
        orderThread_ = new Thread(orderExecutor);
        orderThread_->start();

        SmartPointer<Executor> executionExecutor = new Executor(std::bind(&blockHandling<timeMDTickExecution>, executionBoundQueue_, 
        [this](vector<timeMDTickExecution>& data){
            OnMDTickExecutionHelper(data.data(), data.size());
        }, stopFlag_, "[PluginAmdQuote]:", "execution"
        ));
        executionThread_ = new Thread(executionExecutor);
        executionThread_->start();

        SmartPointer<Executor> bondSnapshotExecutor = new Executor(std::bind(&blockHandling<timeMDBondSnapshot>, bondSnapshotBoundQueue_, 
        [this](vector<timeMDBondSnapshot>& data){
            OnMDBondSnapshotHelper(data.data(), data.size());
        }, stopFlag_, "[PluginAmdQuote]:", "bondSnapshot"
        ));
        bondSnapshotThread_ = new Thread(bondSnapshotExecutor);
        bondSnapshotThread_->start();

        SmartPointer<Executor> bondOrderExecutor = new Executor(std::bind(&blockHandling<timeMDBondTickOrder>, bondOrderBoundQueue_, 
        [this](vector<timeMDBondTickOrder>& data){
            OnMDBondTickOrderHelper(data.data(), data.size());
        }, stopFlag_, "[PluginAmdQuote]:", "bondOrder"
        ));
        bondOrderThread_ = new Thread(bondOrderExecutor);
        bondOrderThread_->start();

        SmartPointer<Executor> bondExecutionExecutor = new Executor(std::bind(&blockHandling<timeMDBondTickExecution>, bondExecutionBoundQueue_, 
        [this](vector<timeMDBondTickExecution>& data){
            OnMDBondTickExecutionHelper(data.data(), data.size());
        }, stopFlag_, "[PluginAmdQuote]:", "bondExecution"
        ));
        bondExecutionThread_ = new Thread(bondExecutionExecutor);
        bondExecutionThread_->start();

        SmartPointer<Executor> indexExecutor = new Executor(std::bind(&blockHandling<timeMDIndexSnapshot>, indexBoundQueue_, 
        [this](vector<timeMDIndexSnapshot>& data){
            OnMDIndexSnapshotHelper(data.data(), data.size());
        }, stopFlag_, "[PluginAmdQuote]:", "index"
        ));
        indexThread_ = new Thread(indexExecutor);
        indexThread_->start();

        SmartPointer<Executor> orderQueueExecutor = new Executor(std::bind(&blockHandling<timeMDOrderQueue>, orderQueueBoundQueue_, 
        [this](vector<timeMDOrderQueue>& data){
            OnMDOrderQueueHelper(data.data(), data.size());
        }, stopFlag_, "[PluginAmdQuote]:", "orderQueue"
        ));
        orderQueueThread_ = new Thread(orderQueueExecutor);
        orderQueueThread_->start();
    }

    ~AMDSpiImp(){
        LOG_INFO("[pluginAmdQuote]: release AMDSpiImp");
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

    // use stockOrFund to distinguish stock or fund 
    void OnMDorderExecutionHelper(int channel, MDOrderExecution* ticks, uint32_t cnt, bool stockOrFund);

    void OnMDBondOrderExecutionHelper(int channel, MDBondOrderExecution* ticks, uint32_t cnt);

    // 接受并处理快照数据（包含基金和股票数据）
    void OnMDSnapshotHelper(timeMDSnapshot* snapshot, uint32_t cnt);
    
    // 接受并处理快照数据(债券）
    void OnMDBondSnapshotHelper(timeMDBondSnapshot* snapshot, uint32_t cnt);

    // 接受并处理逐笔委托数据（包含基金和股票数据）
    void OnMDTickOrderHelper(timeMDTickOrder* ticks, uint32_t cnt);

    // 接受并处理逐笔委托数据（债券）
    void OnMDBondTickOrderHelper(timeMDBondTickOrder* ticks, uint32_t cnt);

    // 接受并处理逐笔成交数据（包含基金和股票数据）
    void OnMDTickExecutionHelper(timeMDTickExecution* tick, uint32_t cnt);

    // 接受并处理逐笔成交数据（债券）
    void OnMDBondTickExecutionHelper(timeMDBondTickExecution* tick, uint32_t cnt);

    // 接受并处理指数快照数据
    void OnMDIndexSnapshotHelper(timeMDIndexSnapshot* index, uint32_t cnt);
    
    // 接受并处理委托队列数据
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

    void latencyLog(int type, long long startTime, uint32_t cnt, long long latency) { // 0: snapshot, 1: execution, 2: order
        Statistic& stat = statistics_[type];
        if (stat.startTime != 0 && startTime > stat.endTime) { // 大于30s，打日志
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

    // TableSP getData(int type) {
    //     if (type == 0) {
    //         return snapshotData_;
    //     } else if (type == 1) {
    //         return executionData_;
    //     } else {
    //         return orderData_;
    //     }
    // }

    void setLatencyFlag(bool flag) {
        latencyFlag_ = flag;
    }

    long long getDailyIndexStarTime(const string& dataType, int market){
        AMDDataType amdDataType;
        if(nameType.count(dataType) != 0){
            amdDataType = nameType[dataType];
        }
        else
            throw IllegalArgumentException(__FUNCTION__, "error dataType: " + dataType);
        AMDTableType tableType = getAmdTableType(amdDataType, market);
        if((int)tableType == (int)AMD_ERROR_TABLE_TYPE){
            throw RuntimeException("getAmdTableType failed");
        }
        if(tableType >= sizeof(dailyIndex_)){
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

    TableSP snapshotData_; // 保存股票快照数据, 建议定义成流表
    TableSP executionData_; // 保存股票逐笔委托, 建议定义成流表
    TableSP orderData_; // 保存股票逐笔成交, 建议定义成流表
    TableSP indexData_; // 保存指数快照数据, 建议定义成流表
    TableSP orderQueueData_; // 保存委托队列数据, 建议定义成流表
    TableSP fundSnapshotData_; // 保存基金快照数据, 建议定义成流表
    TableSP fundExecutionData_; // 保存基金逐笔委托, 建议定义成流表
    TableSP fundOrderData_; // 保存基金逐笔成交, 建议定义成流表
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
    ThreadSP orderThread_;          // FIXME add comment of fund
    ThreadSP indexThread_;
    ThreadSP orderQueueThread_;
    ThreadSP bondSnapshotThread_;
    ThreadSP bondOrderThread_;
    ThreadSP bondExecutionThread_;

    SmartPointer<SnapshotQueue> snapshotBoundQueue_;
    SmartPointer<OrderQueue> orderBoundQueue_;
    SmartPointer<ExecutionQueue> executionBoundQueue_;
    SmartPointer<BondSnapshotQueue> bondSnapshotBoundQueue_;
    SmartPointer<BondOrderQueue> bondOrderBoundQueue_;
    SmartPointer<BondExecutionQueue> bondExecutionBoundQueue_;
    SmartPointer<IndexQueue> indexBoundQueue_;
    SmartPointer<OrderQueueQueue> orderQueueBoundQueue_;
    
    // order execution data structure
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
    
    SmartPointer<bool> stopFlag_;
    DailyIndex dailyIndex_[28];

    struct Statistic {
        long long startTime = 0; // 起始时间（读到第一条数据的时间）
        long long endTime = 0; // 统计结束时间
        long long totalLatency = 0; // 总时间。单位纳秒
        long long maxLatency = 0; // 单条最大延迟
        long long minLatency = 0; // 单条最大延迟
        long totalMessageCount = 0; // 总消息量
        long totalHandleCount = 0; // 处理数据次数，即调用了多少次回调函数
    };
    Statistic statistics_[9]; // 分别对应snapshot, execution, order, 的股票基金/债券 和 orderExecution的股票/基金/债券 共9个
    const long long duration_ = 30 * (long long)1000000000;
    SessionSP session_;

    bool latencyFlag_ = false; // 分别对应snapshot, execution, order, orderExecution 的股票基金/债券
    bool dailyIndexFlag_ = false;
    bool receivedTimeFlag_ = false;
    bool outputElapsedFlag_ = false;
};

#endif