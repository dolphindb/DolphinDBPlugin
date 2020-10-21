//
// Created by hj on 5/20/20.
//

#ifndef PYINTERACT_PYINTERACT_H
#define PYINTERACT_PYINTERACT_H

#include "Python.h"
#include "CoreConcept.h"

extern "C" ConstantSP runCommand(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP importModule(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP getObject(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP getFunction(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP cvtPy2Dol(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP cvtDol2Py(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP callFunction(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP getInstance(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP callMethod(Heap *heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP getAttr(Heap *heap, vector<ConstantSP>& arguments);
extern "C" FunctionDefSP getFunctionDol(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP createObject(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP getInstanceByName(Heap* heap, vector<ConstantSP>& arguments);

#endif //PLUGINDEMO_PLUGINDEMO_H
