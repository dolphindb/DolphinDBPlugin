/*
 * DolphinDBODBC.h
 *
 *  Created on: May 26, 2017
 *      Author: xinjing.zhou
 */

#ifndef DOLPHINDBODBC_H_
#define DOLPHINDBODBC_H_

#include "CoreConcept.h"
#include "Util.h"
#ifndef LINUX
#include <windows.h>
#endif
#include <sql.h>
#include <sqlext.h>

#include <cassert>
#include <climits>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <locale>
#include <map>
#include <vector>

#include "Logger.h"
#include "cvt.h"
#include "nanodbc/nanodbc.h"

extern "C" {
ConstantSP odbcConnect(Heap* heap, vector<ConstantSP>& args);
ConstantSP odbcClose(Heap* heap, vector<ConstantSP>& args);
ConstantSP odbcQuery(Heap* heap, vector<ConstantSP>& args);
ConstantSP odbcExecute(Heap* heap, vector<ConstantSP>& args);
ConstantSP odbcAppend(Heap* heap, vector<ConstantSP>& args);
}
#endif /* DOLPHINDBODBC_H_ */
