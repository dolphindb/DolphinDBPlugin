#ifndef PLUGIN_REDIS_H
#define PLUGIN_REDIS_H

#include "CoreConcept.h"
#include "ddbplugin/CommonInterface.h"
#include "ddbplugin/PluginLoggerImp.h"

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

#endif
