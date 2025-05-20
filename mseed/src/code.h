#ifndef CODE_H
#define CODE_H
#include "DolphinDBEverything.h"
#include "CoreConcept.h"

using ddb::ConstantSP;
using ddb::VectorSP;
using ddb::Heap;
using std::vector;

ConstantSP mseedCode(string sid, long long startTime, double sampleRate, VectorSP value, int blockSize);
#endif //CODE_H