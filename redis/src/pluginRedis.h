#ifndef PLUGIN_REDIS_H
#define PLUGIN_REDIS_H

#include "CoreConcept.h"
#include "ddbplugin/CommonInterface.h"

extern "C" ConstantSP redisPluginConnect(Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisPluginRun(Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisPluginBatchSet(Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisPluginBatchHashSet(Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisPluginRelease(const ConstantSP &handle);
extern "C" ConstantSP redisPluginReleaseAll();
extern "C" ConstantSP redisGetHandle(Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisGetHandleStaus();
extern "C" ConstantSP redisBatchPush(Heap *heap, const vector<ConstantSP> &args);

#endif
