cmake_minimum_required(VERSION 3.22)
set(PluginName PluginFeather)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

target_compile_options(${PluginName} PRIVATE "-Wno-error")

AddArrow()
