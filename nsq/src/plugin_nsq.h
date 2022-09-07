#pragma once

#ifndef PLUGIN_NSQ_H_
#define PLUGIN_NSQ_H_

#include <mutex>
#include <vector>

#include "CoreConcept.h"
#include "Util.h"
#include "ScalarImp.h"
#include "HSNsqSpiImpl.h"
#include "Logger.h"

// 一个全局的表结构
// extern TableSP tsp;
extern std::vector<TableSP> tables;
extern std::vector<bool> subscribe_status;

struct Status {
    long processedMsgCount = 0;
    std::string lastErrMsg;
    long failedMsgCount = 0;
    Timestamp lastFailedTimestamp = LLONG_MIN;
};
extern vector<Status> status;
extern SmartPointer<Session> session;
extern vector<string> tablenames;

// extern CHSNsqApi* lpNsqApi;
// extern CHSNsqSpiImpl* lpNsqSpi;

extern vector<string> snapshotColumnNames;
extern vector<string> tradeColumnNames;
extern vector<string> ordersColumnNames;

extern vector<DATA_TYPE> snapshotTypes;
extern vector<DATA_TYPE> tradeTypes;
extern vector<DATA_TYPE> ordersTypes;

extern "C" ConstantSP nsqConnect(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP subscribe(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP unsubscribe(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP getSchema(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP nsqClose(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP getSubscriptionStatus(Heap *heap, vector<Constant> &arguments);

#endif /* PLUGIN_NSQ_H_ */