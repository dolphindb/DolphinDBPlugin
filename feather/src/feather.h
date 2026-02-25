// SPDX-License-Identifier: Apache-2.0
// Copyright © 2025-2025 DolphinDB, Inc.
#pragma once

#include "DolphinDBEverything.h"
#include <CoreConcept.h>
#include "ddbplugin/CommonInterface.h"
#include <Exceptions.h>
#include <FlatHashmap.h>
#include <ScalarImp.h>
#include <SysIO.h>
#include <Util.h>
#include <Types.h>

#include "arrow/type.h"
#include "arrow/type_fwd.h"

using ddb::ConstantSP;
using ddb::Heap;
using ddb::DATA_TYPE;
using std::vector;

extern "C" ConstantSP loadFeather(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP saveFeather(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP schemaFeather(Heap *heap, vector<ConstantSP> &arguments);
