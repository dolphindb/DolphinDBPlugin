#pragma once

#include "Python.h"
#include "DolphinDBEverything.h"

#define VISIBILITY __attribute__((visibility("default")))

extern "C" VISIBILITY ConstantSP initialize(Heap* heap, std::vector<ConstantSP>& arguments);
extern "C" VISIBILITY ConstantSP importModule(Heap* heap, std::vector<ConstantSP>& arguments);
extern "C" VISIBILITY ConstantSP call(Heap* heap, std::vector<ConstantSP>& arguments);
extern "C" VISIBILITY ConstantSP attr(Heap* heap, std::vector<ConstantSP>& arguments);
extern "C" VISIBILITY ConstantSP repr(Heap* heap, std::vector<ConstantSP>& arguments);

extern "C" VISIBILITY ConstantSP pyNone(Heap* heap, std::vector<ConstantSP>& arguments);
extern "C" VISIBILITY ConstantSP pyInt(Heap* heap, std::vector<ConstantSP>& arguments);
extern "C" VISIBILITY ConstantSP pyFloat(Heap* heap, std::vector<ConstantSP>& arguments);
extern "C" VISIBILITY ConstantSP pyBool(Heap* heap, std::vector<ConstantSP>& arguments);
extern "C" VISIBILITY ConstantSP pyStr(Heap* heap, std::vector<ConstantSP>& arguments);
extern "C" VISIBILITY ConstantSP pyBytes(Heap* heap, std::vector<ConstantSP>& arguments);

extern "C" VISIBILITY ConstantSP toPy(Heap* heap, std::vector<ConstantSP>& arguments);
extern "C" VISIBILITY ConstantSP fromPy(Heap* heap, std::vector<ConstantSP>& arguments);
extern "C" VISIBILITY ConstantSP toNumpy(Heap* heap, std::vector<ConstantSP>& arguments);
extern "C" VISIBILITY ConstantSP fromNumpy(Heap* heap, std::vector<ConstantSP>& arguments);
extern "C" VISIBILITY ConstantSP toPandas(Heap* heap, std::vector<ConstantSP>& arguments);
extern "C" VISIBILITY ConstantSP fromPandas(Heap* heap, std::vector<ConstantSP>& arguments);

extern "C" VISIBILITY ConstantSP pythonType(Heap* heap, std::vector<ConstantSP>& arguments);
extern "C" VISIBILITY ConstantSP pythonDir(Heap* heap, std::vector<ConstantSP>& arguments);
extern "C" VISIBILITY ConstantSP pyTuple(Heap* heap, std::vector<ConstantSP>& arguments);
extern "C" VISIBILITY ConstantSP pyList(Heap* heap, std::vector<ConstantSP>& arguments);
extern "C" VISIBILITY ConstantSP pySet(Heap* heap, std::vector<ConstantSP>& arguments);
extern "C" VISIBILITY ConstantSP pyDict(Heap* heap, std::vector<ConstantSP>& arguments);
extern "C" VISIBILITY ConstantSP setGlobalConfig(Heap* heap, std::vector<ConstantSP>& arguments);
