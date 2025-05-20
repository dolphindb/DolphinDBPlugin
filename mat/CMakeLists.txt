cmake_minimum_required(VERSION 3.13)

set(PluginName PluginMat)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})
target_include_directories(PluginMat PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Matlab901/v901/extern/include)
target_link_libraries(PluginMat 
    ${CMAKE_CURRENT_SOURCE_DIR}/Matlab901/v901/bin/glnxa64/libmat.so
    ${CMAKE_CURRENT_SOURCE_DIR}/Matlab901/v901/bin/glnxa64/libmx.so)