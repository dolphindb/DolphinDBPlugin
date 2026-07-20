#pragma once

#include "DolphinDBEverything.h"

extern "C" {

extern "C" ddb::ConstantSP getS3Object(ddb::Heap *heap, argsT &args);
extern "C" ddb::ConstantSP listS3Object(ddb::Heap *heap, argsT &args);
extern "C" ddb::ConstantSP readS3Object(ddb::Heap *heap, argsT &args);
extern "C" void deleteS3Object(ddb::Heap *heap, argsT &args);
extern "C" void uploadS3Object(ddb::Heap *heap, argsT &args);
extern "C" ddb::ConstantSP listS3Bucket(ddb::Heap *heap, argsT &args);
extern "C" ddb::ConstantSP getUserAgentFeature(ddb::Heap *heap, argsT &args);
extern "C" void deleteS3Bucket(ddb::Heap *heap, argsT &args);
extern "C" void createS3Bucket(ddb::Heap *heap, argsT &args);
extern "C" ddb::ConstantSP headS3Object(ddb::Heap *heap, argsT &args);
extern "C" void copyS3Object(ddb::Heap *heap, argsT &args);
extern "C" ddb::ConstantSP loadS3Object(ddb::Heap *heap, argsT &args);
extern "C" void setClientConfig(ddb::Heap *heap, argsT &args);
extern "C" ddb::ConstantSP getClientConfig(ddb::Heap *heap, argsT &args);
extern "C" ddb::ConstantSP initialize(ddb::Heap *heap, argsT &args);
}
