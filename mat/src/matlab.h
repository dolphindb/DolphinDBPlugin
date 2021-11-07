#include <CoreConcept.h>

extern "C" ConstantSP extractMatSchema(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP loadMat(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP convertToDatetime(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP writeMat(Heap *heap, vector<ConstantSP> &args);