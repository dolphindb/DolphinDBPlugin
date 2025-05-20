#ifndef PLUGIN_LOGGER_H
#define PLUGIN_LOGGER_H

#include "Logger.h"

extern Logger log_inst;

extern severity_type PLUGIN_LOG_LEVEL;  // defined in PluginLoggerImp.h

#define PLUGIN_LOG(...)                                        \
    do {                                                       \
        if (PLUGIN_LOG_LEVEL <= severity_type::DEBUG) {   \
            log_inst.print<severity_type::DEBUG>(__VA_ARGS__); \
        }                                                      \
    } while (0)

#define PLUGIN_LOG_INFO(...)                                  \
    do {                                                      \
        if (PLUGIN_LOG_LEVEL <= severity_type::INFO) {   \
            log_inst.print<severity_type::INFO>(__VA_ARGS__); \
        }                                                     \
    } while (0)

#define PLUGIN_LOG_WARN(...)                                     \
    do {                                                         \
        if (PLUGIN_LOG_LEVEL <= severity_type::WARNING) {   \
            log_inst.print<severity_type::WARNING>(__VA_ARGS__); \
        }                                                        \
    } while (0)

#define PLUGIN_LOG_ERR(...)                              \
    do {                                                 \
        log_inst.print<severity_type::ERR>(__VA_ARGS__); \
    } while (0)

#endif