#ifndef PLUGIN_LOGGER_H
#define PLUGIN_LOGGER_H

#include <cstdarg>

#ifdef __linux__
#ifndef LINUX
#define LINUX
#endif
#elif defined(_WIN32)
#ifndef WINDOWS
#define WINDOWS
#endif
#endif

#include "Logger.h"

#undef LINUX
#undef WINDOWS

extern ddb::severity_type PLUGIN_LOG_LEVEL;  // defined in PluginLoggerImp.h
namespace ddb {
    template<severity_type level, typename... Args>
    inline void pluginLog(Args... args) {
        if (PLUGIN_LOG_LEVEL <= level) {
            log_inst.print<level>(args...);
        }
    }
}

#define PLUGIN_LOG(...) ddb::pluginLog<ddb::severity_type::DEBUG>(__VA_ARGS__)
#define PLUGIN_LOG_INFO(...) ddb::pluginLog<ddb::severity_type::INFO>(__VA_ARGS__)
#define PLUGIN_LOG_WARN(...) ddb::pluginLog<ddb::severity_type::WARNING>(__VA_ARGS__)
#define PLUGIN_LOG_ERR(...) ddb::pluginLog<ddb::severity_type::ERR>(__VA_ARGS__)

#endif