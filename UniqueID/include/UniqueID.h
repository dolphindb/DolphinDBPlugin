#pragma once

#include "DolphinDBEverything.h"
#include "CoreConcept.h"

namespace ddb{

extern "C" ConstantSP createGenerator(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP getGenerator(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP newUid(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP listGenerator(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP destroyGenerator(Heap *heap, vector<ConstantSP> &arguments);

} // namespace ddb;
