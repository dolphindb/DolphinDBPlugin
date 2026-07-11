cmake_minimum_required(VERSION 3.22)

set(PluginName PluginMQTT5)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

find_package(eclipse-paho-mqtt-c REQUIRED)
target_link_libraries(${PluginName} PRIVATE eclipse-paho-mqtt-c::paho-mqtt3as-static)
target_compile_options(${PluginName} PRIVATE -Wno-error=pedantic)

AddOpenSSL()