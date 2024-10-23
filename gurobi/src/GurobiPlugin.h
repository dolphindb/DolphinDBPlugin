//
// Created by htxu on 12/1/2023.
//

#ifndef GUROBI_GUROBIPLUGIN_H
#define GUROBI_GUROBIPLUGIN_H

#include <CoreConcept.h>

#include "ddbplugin/CommonInterface.h"

extern "C" {
ConstantSP gurobiModel(Heap *heap, vector<ConstantSP> &args);
ConstantSP gurobiAddVars(Heap *heap, vector<ConstantSP> &args);
ConstantSP gurobiLinExpr(Heap *heap, vector<ConstantSP> &args);
ConstantSP gurobiQuadExpr(Heap *heap, vector<ConstantSP> &args);
ConstantSP gurobiAddConstr(Heap *heap, vector<ConstantSP> &args);
ConstantSP gurobiSetObjective(Heap *heap, vector<ConstantSP> &args);
ConstantSP gurobiOptimize(Heap *heap, vector<ConstantSP> &args);
ConstantSP gurobiGetResult(Heap *heap, vector<ConstantSP> &args);
ConstantSP gurobiGetObjective(Heap *heap, vector<ConstantSP> &args);
}

#endif  // GUROBI_GUROBIPLUGIN_H
