// SPDX-License-Identifier: Apache-2.0
// Copyright © 2025-2025 DolphinDB, Inc.
#pragma once

#include "Python.h"
#include "DolphinDBEverything.h"
#include "CoreConcept.h"
#include "ddbplugin/CommonInterface.h"

using ddb::ConstantSP;
using ddb::Heap;
using std::vector;

extern "C" ConstantSP runCommand(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP importModule(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP reloadModule(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP getObject(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP getFunction(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP cvtPy2Dol(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP cvtDol2Py(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP callFunction(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP getInstance(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP callMethod(Heap *heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP getAttr(Heap *heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP getFunctionDol(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP createObject(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP getInstanceByName(Heap* heap, vector<ConstantSP>& arguments);
