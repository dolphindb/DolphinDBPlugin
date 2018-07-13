/*
 * ComputingModel.h
 *
 *  Created on: Oct 2, 2017
 *      Author: dzhou
 */

#ifndef COMPUTINGMODEL_H_
#define COMPUTINGMODEL_H_

#include "CoreConcept.h"

namespace OperatorImp{

ConstantSP localReducer(Heap* heap,vector<ConstantSP>& arguments);

}

class StageExecutor {
public:
	virtual ~StageExecutor(){}
	virtual vector<DistributedCallSP> execute(Heap* heap, const vector<DistributedCallSP>& tasks) = 0;
};

class StaticStageExecutor : public StageExecutor{
public:
	StaticStageExecutor(bool parallel, bool reExecuteOnOOM, bool trackJobs, bool resumeOnError = false) :  parallel_(parallel),	reExecuteOnOOM_(reExecuteOnOOM), trackJobs_(trackJobs),
		resumeOnError_(resumeOnError){}
	virtual ~StaticStageExecutor(){}
	virtual vector<DistributedCallSP> execute(Heap* heap, const vector<DistributedCallSP>& tasks);

private:
	bool parallel_;
	bool reExecuteOnOOM_;
	bool trackJobs_;
	bool resumeOnError_;
};

class PipelineStageExecutor : public StageExecutor {
public:
	PipelineStageExecutor(vector<FunctionDefSP>& followingFunctors, bool trackJobs, int queueDepth = 2, int parallel = 1) : followingFunctors_(followingFunctors), trackJobs_(trackJobs),
		queueDepth_(queueDepth), parallel_(parallel){}
	virtual ~PipelineStageExecutor(){}
	virtual vector<DistributedCallSP> execute(Heap* heap, const vector<DistributedCallSP>& tasks);

private:
	void parallelExecute(Heap* heap, vector<DistributedCallSP>& tasks);

private:
	vector<FunctionDefSP> followingFunctors_;
	bool trackJobs_;
	int queueDepth_;
	int parallel_;
};

#endif /* COMPUTINGMODEL_H_ */
