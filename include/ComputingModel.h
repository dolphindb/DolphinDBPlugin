/*
 * ComputingModel.h
 *
 *  Created on: Oct 2, 2017
 *      Author: dzhou
 */

#ifndef COMPUTINGMODEL_H_
#define COMPUTINGMODEL_H_

#include "CoreConcept.h"

namespace ddb {
void checkTaskErrorMsg(const vector<DistributedCallSP>& calls);
namespace OperatorImp{

ConstantSP localReducer(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API now(const ConstantSP& a, const ConstantSP& b);
}

class QueryMonitor{
public:
    QueryMonitor(){enable_.store(true);}
    long long  addQuery(const string& script, const vector<DistributedCallSP>& tasks, long long sessionId, const string& usrId){
        if(tasks.size() > 0 && !tasks.back()->getRootJobId().isNull())
            return -1;
        if(!enable_.load())
            return index_;
        LockGuard<Mutex> _(&mutex_);
        queries_.insert(std::make_pair(index_++, Info(script, tasks, sessionId, usrId)));
        return index_ - 1;
    }

    void removeQuery(long long id){
        if(!enable_.load())
            return;
        LockGuard<Mutex> _(&mutex_);
        queries_.erase(id);
    }

    TableSP getQueryProcess() ;
    void enable() { enable_.store(true); }
    void disable() { enable_.store(false); }
    void clear() {
        LockGuard<Mutex> _(&mutex_);
        queries_.clear();
    }
private:
    struct Info {
        Info(const string& s, const vector<DistributedCallSP>& tasks, long long sessionId, const string& usrId);
        string script;
        long long startTime;
        vector<DistributedCallSP> tasks;
        long long sessionId;
        string usrId;
    };

    unordered_map<long long, Info> queries_;
    long long index_ = 0;
    std::atomic_bool enable_;
    Mutex mutex_;
};

class HardLimitSlotGuard {
public:
	explicit HardLimitSlotGuard(bool active = true);
	~HardLimitSlotGuard();
	bool released() const { return released_; }
private:
	bool released_ = false;
};

class StageExecutor {
public:
	virtual ~StageExecutor(){}
	virtual vector<DistributedCallSP> execute(Heap* heap, const vector<DistributedCallSP>& tasks, bool forceParallel = false) = 0;
	virtual vector<DistributedCallSP> execute(Heap* heap, const vector<DistributedCallSP>& tasks, const JobProperty& jobProp, bool forceParallel = false) = 0;
};

class StaticStageExecutor : public StageExecutor{
public:
	StaticStageExecutor(bool parallel, bool reExecuteOnOOM, bool trackJobs, bool resumeOnError = false, bool scheduleRemoteSite = true) :  parallel_(parallel),
		reExecuteOnOOM_(reExecuteOnOOM), trackJobs_(trackJobs),	resumeOnError_(resumeOnError), scheduleRemoteSite_(scheduleRemoteSite){}
	virtual ~StaticStageExecutor(){}
	vector<DistributedCallSP> execute(Heap* heap, const vector<DistributedCallSP>& tasks, const JobProperty& jobProp, bool forceParallel = false);
	vector<DistributedCallSP> execute(Heap* heap, const vector<DistributedCallSP>& tasks, bool forceParallel = false);
	void execute(Heap* heap, const vector<DistributedCallSP>& tasks, const std::function<void(const vector<DistributedCallSP>&, int)>& callback,
        long long batchTaskSize = -1);
    void setForbidProbingGroupSize(bool flag){forbidProbingGroupSize_ = flag;}
    void setWaitRunningTaskFinishedOnError(bool flag){waitRunningTaskFinishedOnError_ = flag;}
    bool getForbidProbingGroupSize() const {return forbidProbingGroupSize_;}
    void setMonitorProcessAndMemory(bool flag, const string& script){monitorProcessAndMemory_ = flag; script_ = script;}
    bool getMonitorProcessAndMemory() const {return monitorProcessAndMemory_;}

private:
    void groupRemoteCalls(const unordered_map<int, vector<DistributedCallSP>>& tasks,
                          vector<DistributedCallSP>& groupedCalls, const ClusterNodesSP& clusterNodes, int groupSize);

    bool probingGroupSize(bool& groupCall, const vector<DistributedCallSP>& tasks, const ClusterNodesSP& clusterNodes,
                          unordered_map<int, vector<DistributedCallSP>>& siteCalls,
                          vector<std::pair<int,DistributedCallSP>>& needCheckTasks);

    static void cancelJob(const CountDownLatchSP& latch, int remainingCount, int sessionDepth, const Guid& rootJobId);

    struct ClassifyResult {
        int localCallCount = 0;
        bool potentialLocalCall = false;
        ClusterNodesSP clusterNodes;
    };
    ClassifyResult classifyAndScheduleTasks(const vector<DistributedCallSP>& tasks);

private:
    const int MIN_TASK_COUNT_FOR_PROBING_GROUP_SIZE = 4; // if all site's task count is less or equal than it, will group call directly and won't prob group size
    const int TASK_LIMIT_OF_A_GROUP = 1024; // the max task size of a group
	bool parallel_;
	bool reExecuteOnOOM_;
	bool trackJobs_;
	bool resumeOnError_;
	bool scheduleRemoteSite_;
    bool forbidProbingGroupSize_ = true;
    bool monitorProcessAndMemory_ = false;
    bool waitRunningTaskFinishedOnError_ = false;
    long long monitorId_ = 0;
    string script_;
};

class PipelineStageExecutor : public StageExecutor {
public:
	PipelineStageExecutor(vector<FunctionDefSP>& followingFunctors, bool trackJobs, int queueDepth = 2, int parallel = 1) : followingFunctors_(followingFunctors), trackJobs_(trackJobs),
		queueDepth_(queueDepth), parallel_(parallel){}
	virtual ~PipelineStageExecutor(){}
	virtual vector<DistributedCallSP> execute(Heap* heap, const vector<DistributedCallSP>& tasks, bool forceParallel = false);
	virtual vector<DistributedCallSP> execute(Heap* heap, const vector<DistributedCallSP>& tasks, const JobProperty& jobProp, bool forceParallel = false);

private:
	void parallelExecute(Heap* heap, vector<DistributedCallSP>& tasks);

private:
	vector<FunctionDefSP> followingFunctors_;
	bool trackJobs_;
	int queueDepth_;
	int parallel_;
};

class ImprovedPipelineStageExecutor : public StageExecutor {
public:
    ImprovedPipelineStageExecutor(vector<FunctionDefSP>& followingFunctors, bool trackJobs, int queueDepth = 2, int parallel = 1) : followingFunctors_(followingFunctors), trackJobs_(trackJobs),
                                                                                                                                    queueDepth_(queueDepth), parallel_(parallel){}
    virtual ~ImprovedPipelineStageExecutor(){}
    virtual vector<DistributedCallSP> execute(Heap* heap, const vector<DistributedCallSP>& tasks, bool forceParallel = false);
    virtual vector<DistributedCallSP> execute(Heap* heap, const vector<DistributedCallSP>& tasks, const JobProperty& jobProp, bool forceParallel = false);

private:
    vector<FunctionDefSP> followingFunctors_;
    bool trackJobs_;
    int queueDepth_;
    int parallel_;
};
}
#endif /* COMPUTINGMODEL_H_ */
