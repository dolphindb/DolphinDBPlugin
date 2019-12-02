//
// Created by jccai on 19-5-24.
//

#ifndef MATCHINGENGINE_EXPORT_H
#define MATCHINGENGINE_EXPORT_H

#include <CoreConcept.h>

extern "C" ConstantSP setupGlobalConfig(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP createExchange(Heap *heap, vector<ConstantSP> &args);

#endif    // MATCHINGENGINE_EXPORT_H
