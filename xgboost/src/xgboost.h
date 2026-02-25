// SPDX-License-Identifier: Apache-2.0
// Copyright © 2025-2025 DolphinDB, Inc.
#pragma once

#include "DolphinDBEverything.h"
#include "CoreConcept.h"
#include "ddbplugin/CommonInterface.h"

using ddb::ConstantSP;
using ddb::Heap;
using std::vector;

extern "C" {
    ConstantSP train(Heap *heap, vector<ConstantSP> &args);
    ConstantSP predict(Heap *heap, vector<ConstantSP> &args);
    ConstantSP saveModel(Heap *heap, vector<ConstantSP> &args);
    ConstantSP loadModel(Heap *heap, vector<ConstantSP> &args);
    ConstantSP dumpModel(Heap *heap, vector<ConstantSP> &args);
}
