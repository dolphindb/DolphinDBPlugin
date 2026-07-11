cmake_minimum_required(VERSION 3.22)
set(PluginName PluginOrc)
project(${PluginName})

include (../template.cmake)
CreatePlugin(${PluginName})

find_package(orc REQUIRED)
target_link_libraries(${PluginName} PRIVATE orc::orc)
AddProtobuf()
