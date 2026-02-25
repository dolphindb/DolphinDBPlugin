cmake_minimum_required(VERSION 3.22)
set(PluginName PluginGp)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/libgd/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/libjpeg/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/libpng/include)

target_link_directories(PluginGp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/gnuplot/build/src)
target_link_libraries(PluginGp PRIVATE "gnuplot")
set(PLUGIN_LINK_OPTIONS "-Wl,-rpath,$ORIGIN,-disable-new-dtags")
target_link_directories(PluginGp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/gpThirdParty)
target_link_libraries(PluginGp PRIVATE ${PLUGIN_LINK_OPTIONS} gnuPlot readline tinfo gd)
