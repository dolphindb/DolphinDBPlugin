// SPDX-License-Identifier: Apache-2.0
// Copyright © 2025-2025 DolphinDB, Inc.
#pragma once

#include "DolphinDBEverything.h"
#include "CoreConcept.h"

using ddb::ConstantSP;
using ddb::Heap;
using std::vector;

extern "C" ConstantSP hdfs_Connect(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP hdfs_Disconnect(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP hdfs_Exists(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP hdfs_Copy(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP hdfs_Move(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP hdfs_Delete(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP hdfs_Rename(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP hdfs_CreateDirectory(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP hdfs_Chmod(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP hdfs_getListDirectory(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP hdfs_listDirectory(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP hdfs_freeFileInfo(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP hdfs_readFile(Heap *heap, vector<ConstantSP> &args);

extern "C" ConstantSP hdfs_writeFile(Heap *heap, vector<ConstantSP> &args);
