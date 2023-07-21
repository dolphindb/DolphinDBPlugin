#ifndef __AMD_QUOTE_H
#define __AMD_QUOTE_H

#include <exception>
#include <mutex>
#include <ostream>
#include <string>
#include <unordered_map>
#include <iostream>

#include "Exceptions.h"
#include "FlatHashmap.h"
#include "ama.h"
#include "ama_tools.h"

#include "CoreConcept.h"
#include "amdQuoteType.h"
#include "Logger.h"
#include "Util.h"
#include "ScalarImp.h"
#include "Plugin.h"

extern "C" ConstantSP amdConnect(Heap *heap, vector<ConstantSP> &arguments);

extern "C" ConstantSP subscribe(Heap *heap, vector<ConstantSP> &arguments);

extern "C" ConstantSP unsubscribe(Heap *heap, vector<ConstantSP> &arguments);

extern "C" ConstantSP amdClose(Heap *heap, vector<ConstantSP> &arguments);

extern "C" ConstantSP getSchema(Heap *heap, vector<ConstantSP> &arguments);

extern "C" ConstantSP getStatus(Heap *heap, vector<ConstantSP> &arguments);

extern "C" ConstantSP enableLatencyStatistics(Heap *heap, vector<ConstantSP> &arguments);

#ifndef AMD_3_9_6
extern "C" ConstantSP getCodeList(Heap *heap, vector<ConstantSP> &arguments);

extern "C" ConstantSP getETFCodeList(Heap *heap, vector<ConstantSP> &arguments);
#endif

extern "C" ConstantSP setErrorLog(Heap *heap, vector<ConstantSP> &arguments);


#endif