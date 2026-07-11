// SPDX-License-Identifier: Apache-2.0
// Copyright © 2025-2025 DolphinDB, Inc.
#pragma once

#include "DolphinDBEverything.h"
#include "CoreConcept.h"
#include "ddbplugin/CommonInterface.h"

using ddb::ConstantSP;
using ddb::Heap;
using std::vector;

#if defined(_WIN32) || defined(__CYGWIN__)
  #define PLUGIN_API extern "C" __declspec(dllexport)
#else
  #define PLUGIN_API extern "C" __attribute__((visibility("default")))
#endif

PLUGIN_API ConstantSP train(Heap *heap, vector<ConstantSP> &args);
PLUGIN_API ConstantSP predict(Heap *heap, vector<ConstantSP> &args);
PLUGIN_API ConstantSP saveModel(Heap *heap, vector<ConstantSP> &args);
PLUGIN_API ConstantSP loadModel(Heap *heap, vector<ConstantSP> &args);
PLUGIN_API ConstantSP dumpModel(Heap *heap, vector<ConstantSP> &args);
