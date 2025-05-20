// NOTE: This header should be included only once in each project
#include "CoreConcept.h"
#include "ScalarImp.h"
#include "ddbplugin/PluginLogger.h"

extern Logger log_inst;
severity_type PLUGIN_LOG_LEVEL = log_inst.getLogLevel();

extern "C" {
void setLogLevel(Heap *heap, vector<ConstantSP> &args) {
    string usage{"setLogLevel(logLevel) "};
    string errMsg{"logLevel must be DEBUG, INFO, WARNING or ERROR"};
    if (args.size() != 1) {
        throw IllegalArgumentException(
            "setLogLevel", usage +
                               "function [setLogLevel] expects 1 argument(s), but the actual number of arguments is: " +
                               std::to_string(args.size()));
    }
    if (!args[0]->isScalar() || args[0]->getType() != DT_INT) {
        throw IllegalArgumentException("setLogLevel", usage + errMsg);
    }
    int level = args[0]->getInt();
    if (level >= 0 && level <= 3) {
        PLUGIN_LOG_LEVEL = severity_type(level);
        return;
    }
    throw IllegalArgumentException("setLogLevel", usage + errMsg);
}

ConstantSP getLogLevel(Heap *heap, vector<ConstantSP> &args) {
    string usage{"getLogLevel() "};
    if (args.size() != 0) {
        throw IllegalArgumentException(
            "getLogLevel", usage +
                               "function [getLogLevel] expects 0 argument(s), but the actual number of arguments is: " +
                               std::to_string(args.size()));
    }
    switch (PLUGIN_LOG_LEVEL) {
        case severity_type::DEBUG:
            return new String("DEBUG");
        case severity_type::INFO:
            return new String("INFO");
        case severity_type::WARNING:
            return new String("WARNING");
        case severity_type::ERR:
        default:
            return new String("ERROR");
    }
}
}