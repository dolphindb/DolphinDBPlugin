// SPDX-License-Identifier: Apache-2.0
// Copyright © 2025-2025 DolphinDB, Inc.
#pragma once

#include "DolphinDBEverything.h"
#include "ddbplugin/CommonInterface.h"
#include "ddbplugin/PluginLogger.h"

using ddb::ConstantSP;
using ddb::Heap;
using std::vector;

extern "C" ConstantSP redisPluginConnect(ddb::Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisPluginRun(ddb::Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisPluginBatchSet(ddb::Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisPluginBatchHashSet(ddb::Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisPluginRelease(ddb::Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisPluginReleaseAll(ddb::Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisGetHandle(ddb::Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisGetHandleStaus(ddb::Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisBatchPush(ddb::Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisBatchGet(ddb::Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisSubscribe(ddb::Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisUnsubscribe(ddb::Heap *heap, const vector<ConstantSP> &args);
// reids stream
extern "C" ConstantSP redisStreamCreateGroup(ddb::Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisStreamReadGroup(ddb::Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisStreamAck(ddb::Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisStreamSubscribe(ddb::Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisStreamGetSubscribeStat(ddb::Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP redisStreamUnsubscribe(ddb::Heap *heap, const vector<ConstantSP> &args);
