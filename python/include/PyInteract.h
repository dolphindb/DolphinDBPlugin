// SPDX-License-Identifier: Apache-2.0
// Copyright © 2025-2025 DolphinDB, Inc.
#pragma once

#include "Python.h"
#include "DolphinDBEverything.h"

#define VISIBILITY __attribute__((visibility("default")))

extern "C" VISIBILITY ddb::ConstantSP initialize(ddb::Heap* heap, std::vector<ddb::ConstantSP>& arguments);
extern "C" VISIBILITY ddb::ConstantSP importModule(ddb::Heap* heap, std::vector<ddb::ConstantSP>& arguments);
extern "C" VISIBILITY ddb::ConstantSP call(ddb::Heap* heap, std::vector<ddb::ConstantSP>& arguments);

extern "C" VISIBILITY ddb::ConstantSP pyNone(ddb::Heap* heap, std::vector<ddb::ConstantSP>& arguments);
extern "C" VISIBILITY ddb::ConstantSP pyInt(ddb::Heap* heap, std::vector<ddb::ConstantSP>& arguments);
extern "C" VISIBILITY ddb::ConstantSP pyFloat(ddb::Heap* heap, std::vector<ddb::ConstantSP>& arguments);
extern "C" VISIBILITY ddb::ConstantSP pyBool(ddb::Heap* heap, std::vector<ddb::ConstantSP>& arguments);
extern "C" VISIBILITY ddb::ConstantSP pyStr(ddb::Heap* heap, std::vector<ddb::ConstantSP>& arguments);
extern "C" VISIBILITY ddb::ConstantSP pyBytes(ddb::Heap* heap, std::vector<ddb::ConstantSP>& arguments);

extern "C" VISIBILITY ddb::ConstantSP toPy(ddb::Heap* heap, std::vector<ddb::ConstantSP>& arguments);
extern "C" VISIBILITY ddb::ConstantSP fromPy(ddb::Heap* heap, std::vector<ddb::ConstantSP>& arguments);
extern "C" VISIBILITY ddb::ConstantSP toNumpy(ddb::Heap* heap, std::vector<ddb::ConstantSP>& arguments);
extern "C" VISIBILITY ddb::ConstantSP fromNumpy(ddb::Heap* heap, std::vector<ddb::ConstantSP>& arguments);
extern "C" VISIBILITY ddb::ConstantSP toPandas(ddb::Heap* heap, std::vector<ddb::ConstantSP>& arguments);
extern "C" VISIBILITY ddb::ConstantSP fromPandas(ddb::Heap* heap, std::vector<ddb::ConstantSP>& arguments);

extern "C" VISIBILITY ddb::ConstantSP pythonType(ddb::Heap* heap, std::vector<ddb::ConstantSP>& arguments);
extern "C" VISIBILITY ddb::ConstantSP pythonDir(ddb::Heap* heap, std::vector<ddb::ConstantSP>& arguments);
extern "C" VISIBILITY ddb::ConstantSP pyTuple(ddb::Heap* heap, std::vector<ddb::ConstantSP>& arguments);
extern "C" VISIBILITY ddb::ConstantSP pyList(ddb::Heap* heap, std::vector<ddb::ConstantSP>& arguments);
extern "C" VISIBILITY ddb::ConstantSP pySet(ddb::Heap* heap, std::vector<ddb::ConstantSP>& arguments);
extern "C" VISIBILITY ddb::ConstantSP pyDict(ddb::Heap* heap, std::vector<ddb::ConstantSP>& arguments);
extern "C" VISIBILITY ddb::ConstantSP setGlobalConfig(ddb::Heap* heap, std::vector<ddb::ConstantSP>& arguments);
