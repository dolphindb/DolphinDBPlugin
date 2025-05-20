#ifndef _PLUGIN_XGBOOST_
#define _PLUGIN_XGBOOST_

#include "CoreConcept.h"
#include "ddbplugin/CommonInterface.h"

extern "C" {
    ConstantSP train(Heap *heap, vector<ConstantSP> &args);
    ConstantSP predict(Heap *heap, vector<ConstantSP> &args);
    ConstantSP saveModel(Heap *heap, vector<ConstantSP> &args);
    ConstantSP loadModel(Heap *heap, vector<ConstantSP> &args);
    ConstantSP dumpModel(Heap *heap, vector<ConstantSP> &args);
}

#endif