#include "CoreConcept.h"

extern "C" {
ConstantSP createJsonParser(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP createJsonFormatter(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP createCsvParser(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP createCsvFormatter(Heap* heap, vector<ConstantSP>& arguments);
}

ConstantSP parseJson(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP parseCsv(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP formatJson(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP formatCsv(Heap* heap, vector<ConstantSP>& arguments);
