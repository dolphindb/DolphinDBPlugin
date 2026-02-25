// SPDX-License-Identifier: Apache-2.0
// Copyright © 2025-2025 DolphinDB, Inc.
#pragma once

#include "DolphinDBEverything.h"
#include <CoreConcept.h>
#include<Util.h>
#include <ScalarImp.h>
#include "ddbplugin/CommonInterface.h"

using ddb::ConstantSP;
using ddb::Heap;
using std::vector;
extern "C" ConstantSP gpPlot(Heap *heap, vector<ConstantSP> &args);
