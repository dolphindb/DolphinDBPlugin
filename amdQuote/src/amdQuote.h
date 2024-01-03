#ifndef AMD_QUOTE_H
#define AMD_QUOTE_H

#include "CoreConcept.h"

extern "C" ConstantSP amdConnect(Heap *heap, vector<ConstantSP> &arguments);

extern "C" ConstantSP subscribe(Heap *heap, vector<ConstantSP> &arguments);

extern "C" ConstantSP unsubscribe(Heap *heap, vector<ConstantSP> &arguments);

extern "C" ConstantSP amdClose(Heap *heap, vector<ConstantSP> &arguments);

extern "C" ConstantSP getSchema(Heap *heap, vector<ConstantSP> &arguments);

extern "C" ConstantSP getStatus(Heap *heap, vector<ConstantSP> &arguments);

extern "C" ConstantSP getHandle(Heap *heap, vector<ConstantSP> &arguments);

#ifndef AMD_3_9_6
extern "C" ConstantSP getCodeList(Heap *heap, vector<ConstantSP> &arguments);

extern "C" ConstantSP getETFCodeList(Heap *heap, vector<ConstantSP> &arguments);
#endif

extern "C" ConstantSP setErrorLog(Heap *heap, vector<ConstantSP> &arguments);

#endif