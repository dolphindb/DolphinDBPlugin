// SPDX-License-Identifier: Apache-2.0
// Copyright © 2025-2025 DolphinDB, Inc.
#pragma once

#include "DolphinDBEverything.h"
#include <CoreConcept.h>

using ddb::ConstantSP;
using ddb::Heap;
using std::vector;

extern "C" {
ConstantSP h5ls(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP h5lsTable(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP extractHDF5Schema(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP loadHDF5(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP loadPandasHDF5(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP loadHDF5Ex(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP HDF5DS(Heap *heap, vector<ConstantSP>& arguments);
ConstantSP saveHDF5(Heap *heap, vector<ConstantSP> &arguments);
}
