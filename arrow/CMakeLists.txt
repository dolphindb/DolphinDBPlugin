cmake_minimum_required(VERSION 3.22)
set(PluginName PluginArrow)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

AddArrow()
