// SPDX-License-Identifier: Apache-2.0
// Copyright © 2025-2025 DolphinDB, Inc.
#pragma once

#include "DolphinDBEverything.h"
#include <CoreConcept.h>
#include "ddbplugin/CommonInterface.h"

using ddb::ConstantSP;
using ddb::Heap;
using std::vector;

extern "C" {

ConstantSP nsqConnect(Heap* heap, vector<ConstantSP>& args);
ConstantSP nsqClose(Heap *heap, vector<ConstantSP>& args);

ConstantSP nsqGetSchema(Heap *heap, vector<ConstantSP>& args);

ConstantSP nsqSubscribe(Heap* heap, vector<ConstantSP>& args);
ConstantSP nsqUnsubscribe(Heap* heap, vector<ConstantSP>& args);

ConstantSP nsqGetStatus(Heap *heap, vector<ConstantSP>& args);

}
