#pragma once

#include "Logger.h"

#include <cstdarg>

#define MACRO_CONTACT_(a, b)   a##b
#define MACRO_CONTACT(a, b)   MACRO_CONTACT_(a, b)
#define PLUGIN_VAR(name) MACRO_CONTACT(PLUGIN_NAME, name)

extern ddb::severity_type PLUGIN_VAR(PLUGIN_LOG_LEVEL);
namespace ddb {
    template<severity_type level, typename... Args>
    inline void PLUGIN_VAR(pluginLog)(Args... args) {
        if (PLUGIN_VAR(PLUGIN_LOG_LEVEL) <= level) {
            log_inst.print<level>(args...);
        }
    }
}

#undef LOG
#undef LOG_INFO
#undef LOG_WARN
#undef LOG_ERR

#ifndef LOG_NAME
#define LOG_NAME PLUGIN_NAME
#endif
#define LOG_STRING(x) #x
#define LOG_LABEL(x) " [" LOG_STRING(x) "] "

#define LOG(...) ddb::PLUGIN_VAR(pluginLog)<ddb::severity_type::DEBUG>(LOG_LABEL(LOG_NAME), __VA_ARGS__)
#define LOG_INFO(...) ddb::PLUGIN_VAR(pluginLog)<ddb::severity_type::INFO>(LOG_LABEL(LOG_NAME), __VA_ARGS__)
#define LOG_WARN(...) ddb::PLUGIN_VAR(pluginLog)<ddb::severity_type::WARNING>(LOG_LABEL(LOG_NAME), __VA_ARGS__)
#define LOG_ERR(...) ddb::PLUGIN_VAR(pluginLog)<ddb::severity_type::ERR>(LOG_LABEL(LOG_NAME), __VA_ARGS__)
