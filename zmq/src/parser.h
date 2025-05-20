#include "DolphinDBEverything.h"
#include "CoreConcept.h"

using argsT = std::vector<ddb::ConstantSP>;

extern "C" {
ddb::ConstantSP createJSONParser(ddb::Heap* heap, argsT &arguments);
ddb::ConstantSP createJSONFormatter(ddb::Heap* heap, argsT &arguments);
ddb::ConstantSP createCSVParser(ddb::Heap* heap, argsT &arguments);
ddb::ConstantSP createCSVFormatter(ddb::Heap* heap, argsT &arguments);
}

namespace ddb {

ConstantSP parseJSON(Heap* heap, argsT &arguments);
ConstantSP parseCSV(Heap* heap, argsT &arguments);
ConstantSP formatJSON(Heap* heap, argsT &arguments);
ConstantSP formatCSV(Heap* heap, argsT &arguments);
static const std::string PLUGIN_ZMQ_PARSERS_PREFIX = "[PLUGIN ZMQ PARSERS]:";

}
