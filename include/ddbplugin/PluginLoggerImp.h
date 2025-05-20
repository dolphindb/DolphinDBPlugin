// NOTE: This header should be included only once in each project
#include "CoreConcept.h"
#include "ScalarImp.h"
#include "ddbplugin/PluginLogger.h"

namespace ddb {
    extern ddb::Logger log_inst;
}

ddb::severity_type PLUGIN_LOG_LEVEL = ddb::log_inst.getLogLevel();

extern "C" {
void setLogLevel(ddb::Heap *heap, std::vector<ddb::ConstantSP> &args) {
    std::ignore = heap;
    string usage{"setLogLevel(logLevel) "};
    string errMsg{"logLevel must be DEBUG, INFO, WARNING or ERROR"};
    if (args.size() != 1) {
        throw ddb::IllegalArgumentException(
            "setLogLevel", usage +
                               "function [setLogLevel] expects 1 argument(s), but the actual number of arguments is: " +
                               std::to_string(args.size()));
    }
    if (!args[0]->isScalar() || args[0]->getType() != ddb::DT_INT) {
        throw ddb::IllegalArgumentException("setLogLevel", usage + errMsg);
    }
    int level = args[0]->getInt();
    if (level >= 0 && level <= 3) {
        PLUGIN_LOG_LEVEL = ddb::severity_type(level);
        return;
    }
    throw ddb::IllegalArgumentException("setLogLevel", usage + errMsg);
}

ddb::ConstantSP getLogLevel(ddb::Heap *heap, std::vector<ddb::ConstantSP> &args) {
    std::ignore = heap;
    string usage{"getLogLevel() "};
    if (args.size() != 0) {
        throw ddb::IllegalArgumentException(
            "getLogLevel", usage +
                               "function [getLogLevel] expects 0 argument(s), but the actual number of arguments is: " +
                               std::to_string(args.size()));
    }
    switch (PLUGIN_LOG_LEVEL) {
        case ddb::severity_type::DEBUG:
            return new ddb::String("DEBUG");
        case ddb::severity_type::INFO:
            return new ddb::String("INFO");
        case ddb::severity_type::WARNING:
            return new ddb::String("WARNING");
        case ddb::severity_type::ERR:
        default:
            return new ddb::String("ERROR");
    }
}
}