#ifndef JSON_UTIL_H
#define JSON_UTIL_H

#include <CoreConcept.h>
#include <Exceptions.h>
#include <ScalarImp.h>
#include <SysIO.h>
#include <Types.h>
#include <Util.h>


using ddb::ConstantSP;
using ddb::Heap;

using std::string;
using std::vector;

ConstantSP parseJson(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP parseJsonWrapper(Heap* heap, vector<ConstantSP>& arguments);

ConstantSP parseNestedJson(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP parseNestedJsonWrapper(Heap* heap, vector<ConstantSP>& arguments);

#endif