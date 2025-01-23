
#pragma once

#include "Logger.h"

extern Logger log_inst;

enum class PluginSeverityType { DEBUG, INFO, WARNING, ERR };

extern PluginSeverityType PLUGIN_LOG_LEVEL;  // defined in PluginLoggerImp.h

#define PLUGIN_LOG(...)                                        \
    do {                                                       \
        if (PLUGIN_LOG_LEVEL <= PluginSeverityType::DEBUG) {   \
            log_inst.print<severity_type::DEBUG>(__VA_ARGS__); \
        }                                                      \
    } while (0)

#define PLUGIN_LOG_INFO(...)                                  \
    do {                                                      \
        if (PLUGIN_LOG_LEVEL <= PluginSeverityType::INFO) {   \
            log_inst.print<severity_type::INFO>(__VA_ARGS__); \
        }                                                     \
    } while (0)

#define PLUGIN_LOG_WARN(...)                                     \
    do {                                                         \
        if (PLUGIN_LOG_LEVEL <= PluginSeverityType::WARNING) {   \
            log_inst.print<severity_type::WARNING>(__VA_ARGS__); \
        }                                                        \
    } while (0)

#define PLUGIN_LOG_ERR(...)                              \
    do {                                                 \
        log_inst.print<severity_type::ERR>(__VA_ARGS__); \
    } while (0)
