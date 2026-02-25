// SPDX-License-Identifier: Apache-2.0
// Copyright © 2025-2025 DolphinDB, Inc.
#pragma once

#include "DolphinDBEverything.h"
#include <CoreConcept.h>

#include "ddbplugin/CommonInterface.h"
#include "ddbplugin/PluginLogger.h"

using ddb::ConstantSP;
using ddb::Heap;
using std::vector;

extern "C" {
ConstantSP gurobiModel(Heap *heap, vector<ConstantSP> &args);
ConstantSP gurobiAddVars(Heap *heap, vector<ConstantSP> &args);
ConstantSP gurobiLinExpr(Heap *heap, vector<ConstantSP> &args);
ConstantSP gurobiQuadExpr(Heap *heap, vector<ConstantSP> &args);
ConstantSP gurobiAddConstr(Heap *heap, vector<ConstantSP> &args);
ConstantSP gurobiSetObjective(Heap *heap, vector<ConstantSP> &args);
ConstantSP gurobiOptimize(Heap *heap, vector<ConstantSP> &args);
ConstantSP gurobiGetResult(Heap *heap, vector<ConstantSP> &args);
ConstantSP gurobiGetObjective(Heap *heap, vector<ConstantSP> &args);
}
