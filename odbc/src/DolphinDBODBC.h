/*
 * DolphinDBODBC.h
 *
 *  Created on: May 26, 2017
 *      Author: xinjing.zhou
 */

#ifndef DOLPHINDBODBC_H_
#define DOLPHINDBODBC_H_

#include "CoreConcept.h"
//#include "DolphinDB.h"
// using dolphindb::ConstantSP;

extern "C" ConstantSP odbcConnect(Heap* heap, vector<ConstantSP>& args);
extern "C" ConstantSP odbcClose(Heap* heap, vector<ConstantSP>& args);
extern "C" ConstantSP odbcQuery(Heap* heap, vector<ConstantSP>& args);
extern "C" ConstantSP odbcExecute(Heap* heap, vector<ConstantSP>& args);
extern "C" ConstantSP odbcAppend(Heap* heap, vector<ConstantSP>& args);
#endif /* DOLPHINDBODBC_H_ */
