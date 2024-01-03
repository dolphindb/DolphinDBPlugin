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

// extern TableSP tsp;
extern std::vector<TableSP> TABLES;
extern std::vector<bool> SUBSCRIBE_STATUS;

struct Status {
    long processedMsgCount = 0;
    std::string lastErrMsg;
    long failedMsgCount = 0;
    Timestamp lastFailedTimestamp = LLONG_MIN;
};
extern vector<Status> STATUS;
extern SmartPointer<Session> SESSION;
extern vector<string> TABLE_NAMES;

// extern CHSNsqApi* lpNsqApi;
// extern CHSNsqSpiImpl* lpNsqSpi;

extern bool RECEIVED_TIME_FLAG;
extern bool GET_ALL_FIELD_NAMES_FLAG;

extern vector<string> SNAPSHOT_COLUMN_NAMES;
extern vector<string> TRADE_COLUMN_NAMES;
extern vector<string> ORDER_COLUMN_NAMES;
extern vector<string> ADDED_SNAPSHOT_COLUMN_NAMES;
extern vector<string> TRADE_AND_ORDER_MERGED_COLUMN_NAMES;

extern vector<DATA_TYPE> SNAPSHOT_TYPES;
extern vector<DATA_TYPE> TRADE_TYPES;
extern vector<DATA_TYPE> ORDER_TYPES;
extern vector<DATA_TYPE> ADDED_SNAPSHOT_TYPES;
extern vector<DATA_TYPE> TRADE_AND_ORDER_MERGED_TYPES;

class Defer {
public:
    Defer(std::function<void()> code): code(code) {}
    ~Defer() { code(); }
private:
    std::function<void()> code;
};

static Mutex NSQ_MUTEX;

extern "C"
{
	ConstantSP nsqConnect(Heap* heap, vector<ConstantSP>& arguments);
	ConstantSP subscribe(Heap* heap, vector<ConstantSP>& arguments);
	ConstantSP unsubscribe(Heap* heap, vector<ConstantSP>& arguments);
	ConstantSP getSchema(Heap *heap, vector<ConstantSP>& arguments);
	ConstantSP nsqClose(Heap *heap, vector<ConstantSP>& arguments);
	ConstantSP getSubscriptionStatus(Heap *heap, vector<Constant>& arguments);
}

#endif /* PLUGIN_NSQ_H_ */