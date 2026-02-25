// SPDX-License-Identifier: Apache-2.0
// Copyright © 2025-2025 DolphinDB, Inc.
#pragma once

#include "CoreConcept.h"
#include "ddbplugin/CommonInterface.h"

using ddb::ConstantSP;
using ddb::Heap;
using std::vector;
extern "C" ConstantSP getOpcServerList(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP connectOpcServer(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP disconnect(Heap* heap, vector<ConstantSP>& arguments );
extern "C" ConstantSP readTag(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP writeTag(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP subscribeTag(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP endSub(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP getSubscriberStat(Heap* heap, vector<ConstantSP>& arguments);
