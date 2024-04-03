#include "CoreConcept.h"

extern "C" {
ConstantSP createJSONParser(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP createJSONFormatter(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP createCSVParser(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP createCSVFormatter(Heap* heap, vector<ConstantSP>& arguments);
}

ConstantSP parseJSON(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP parseCSV(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP formatJSON(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP formatCSV(Heap* heap, vector<ConstantSP>& arguments);
static const std::string PLUGIN_ZMQ_PARSERS_PREFIX = "[PLUGIN ZMQ PARSERS]:";
