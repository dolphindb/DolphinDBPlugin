cmake_minimum_required(VERSION 3.13)

set(PluginName PluginDemo)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})
