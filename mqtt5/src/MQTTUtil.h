#pragma once

#include "DolphinDBEverything.h"
#include "ddbplugin/PluginLogger.h"

static const string PLUGIN_NAME = "[PLUGIN:MQTT5]";
static const string PUB_RESOUCE_NAME = "mqtt publish connection";
static const string SUB_RESOUCE_NAME = "mqtt subscribe connection";

inline string generateClientId() {
    return "ddb_mqtt_plugin_" + std::to_string(std::time(nullptr)) + "_" + std::to_string(rand() % 10000);
}
