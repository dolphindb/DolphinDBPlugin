cmake_minimum_required(VERSION 3.22)

set(PluginName PluginTCPSocket)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})
