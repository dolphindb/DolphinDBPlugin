#ifndef PLUGIN_REDIS_H
#define PLUGIN_REDIS_H

#include "CoreConcept.h"
#include "ddbplugin/CommonInterface.h"
#include "ddbplugin/PluginLoggerImp.h"

using ddb::ConstantSP;
using ddb::Heap;
using std::vector;

extern "C" ConstantSP redisPluginConnect(ddb::Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisPluginRun(ddb::Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisPluginBatchSet(ddb::Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisPluginBatchHashSet(ddb::Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisPluginRelease(ddb::Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisPluginReleaseAll();
extern "C" ConstantSP redisGetHandle(ddb::Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisGetHandleStaus();
extern "C" ConstantSP redisBatchPush(ddb::Heap *heap, const vector<ConstantSP> &args);

#endif
