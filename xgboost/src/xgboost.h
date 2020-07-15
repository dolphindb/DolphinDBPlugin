#ifndef _PLUGIN_XGBOOST_
#define _PLUGIN_XGBOOST_

#include "CoreConcept.h"

#ifdef LINUX
    #define EXPORT_DLL
#else
    #define EXPORT_DLL __declspec(dllexport)
#endif

extern "C" {
    ConstantSP EXPORT_DLL train(Heap *heap, vector<ConstantSP> &args);
    ConstantSP EXPORT_DLL predict(Heap *heap, vector<ConstantSP> &args);
    ConstantSP EXPORT_DLL saveModel(const ConstantSP &model, const ConstantSP &fname);
    ConstantSP EXPORT_DLL loadModel(Heap *heap, vector<ConstantSP> &args);
    ConstantSP EXPORT_DLL dumpModel(Heap *heap, vector<ConstantSP> &args);
}

#endif