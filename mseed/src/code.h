#ifndef CODE_H
#define CODE_H
#include "CoreConcept.h"

ConstantSP mseedCode(string sid, long long startTime, double sampleRate, VectorSP value, int blockSize);
#endif //CODE_H