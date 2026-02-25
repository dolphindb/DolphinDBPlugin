// SPDX-License-Identifier: Apache-2.0
// Copyright © 2025-2025 DolphinDB, Inc.
#pragma once

#include "DolphinDBEverything.h"
#include "CoreConcept.h"
#include "Util.h"
#ifdef _WIN32
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

#include "cvt.h"
#include "nanodbc/nanodbc.h"
using ddb::ConstantSP;
using ddb::Heap;
using std::vector;

extern "C" {
ConstantSP odbcConnect(Heap* heap, vector<ConstantSP>& args);
ConstantSP odbcClose(Heap* heap, vector<ConstantSP>& args);
ConstantSP odbcQuery(Heap* heap, vector<ConstantSP>& args);
ConstantSP odbcExecute(Heap* heap, vector<ConstantSP>& args);
ConstantSP odbcAppend(Heap* heap, vector<ConstantSP>& args);
}
