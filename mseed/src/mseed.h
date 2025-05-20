#include "DolphinDBEverything.h"
#include <CoreConcept.h>

using ddb::ConstantSP;
using ddb::Heap;
using std::vector;

extern "C" ConstantSP mseedRead(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP mseedWrite(Heap *heap, vector<ConstantSP> &args);