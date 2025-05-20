//
// Created by ypfan on 2020/12/7.
//

#ifndef HDFS_PLUGINHDFS_H
#define HDFS_PLUGINHDFS_H

#include "CoreConcept.h"

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

#endif //HDFS_PLUGINHDFS_H
