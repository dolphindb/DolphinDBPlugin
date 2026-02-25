#ifndef PLUGIN_REDIS_H
#define PLUGIN_REDIS_H

#include "CoreConcept.h"
#include "ddbplugin/CommonInterface.h"


extern "C" ConstantSP redisPluginConnect(Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisPluginRun(Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisPluginBatchSet(Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisPluginBatchHashSet(Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisPluginRelease(Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisPluginReleaseAll(Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisGetHandle(Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisGetHandleStaus(Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisBatchPush(Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisBatchGet(Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisSubscribe(Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisUnsubscribe(Heap *heap, const vector<ConstantSP> &args);
// reids stream
extern "C" ConstantSP redisStreamCreateGroup(Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisStreamReadGroup(Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisStreamAck(Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisStreamSubscribe(Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisStreamGetSubscribeStat(Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisStreamUnsubscribe(Heap *heap, const vector<ConstantSP> &args);

#endif
