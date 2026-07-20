cmake_minimum_required(VERSION 3.22)
set(PluginName PluginParquet)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

UseSignedCharOnARM()
AddArrow()
