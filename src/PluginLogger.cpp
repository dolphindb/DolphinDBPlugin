#include "DolphinDBEverything.h"
#include "CoreConcept.h"
#include "ScalarImp.h"
#include "ddbplugin/PluginLogger.h"

severity_type PLUGIN_VAR(LOG_LEVEL) = log_inst.getLogLevel();

extern "C" {

__attribute__((visibility("default")))
void setLogLevel(Heap *heap, std::vector<ConstantSP> &args) {
    std::ignore = heap;
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
        PLUGIN_VAR(LOG_LEVEL) = severity_type(level);
        return;
    }
    throw IllegalArgumentException("setLogLevel", usage + errMsg);
}

__attribute__((visibility("default")))
ConstantSP getLogLevel(Heap *heap, std::vector<ConstantSP> &args) {
    std::ignore = heap;
    string usage{"getLogLevel() "};
    if (args.size() != 0) {
        throw IllegalArgumentException(
            "getLogLevel", usage +
                               "function [getLogLevel] expects 0 argument(s), but the actual number of arguments is: " +
                               std::to_string(args.size()));
    }
    switch (PLUGIN_VAR(LOG_LEVEL)) {
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

} // extern
