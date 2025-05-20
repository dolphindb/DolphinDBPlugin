#ifndef INSIGHT_PLUGIN_H
#define INSIGHT_PLUGIN_H

#include <vector>

#include "CoreConcept.h"
#include "InsightType.h"
#include "Util.h"

using ddb::ConstantSP;
using ddb::Heap;
using std::vector;

extern "C" ConstantSP connectInsight(Heap *heap, std::vector<ConstantSP> &arguments);
extern "C" void subscribe(Heap *heap, std::vector<ConstantSP> &arguments);
extern "C" void unsubscribe(Heap *heap, std::vector<ConstantSP> &arguments);
extern "C" void closeInsight(Heap *heap, std::vector<ConstantSP> &arguments);
extern "C" ConstantSP getSchema(Heap *heap, std::vector<ConstantSP> &arguments);
extern "C" ConstantSP getStatus(Heap *heap, std::vector<ConstantSP> &arguments);
extern "C" ConstantSP getHandle(Heap *heap, std::vector<ConstantSP> &arguments);

#endif
