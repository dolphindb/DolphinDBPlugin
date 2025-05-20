#include "DolphinDBEverything.h"

using ddb::ConstantSP;
using ddb::Heap;
using std::vector;

extern "C" ConstantSP extractMatSchema(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP loadMat(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP convertToDatetime(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP writeMat(Heap *heap, vector<ConstantSP> &args);