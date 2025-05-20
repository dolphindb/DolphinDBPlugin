#ifndef STREAM_H
#define STREAM_H
#include "DolphinDBEverything.h"
#include "CoreConcept.h"
#include "code.h"

using ddb::ConstantSP;
using ddb::Heap;
using std::vector;

extern "C" ConstantSP mseedStreamize(Heap *heap, vector<ConstantSP> &args);
#endif //STREAM_H
