cmake_minimum_required(VERSION 3.22)
set(PluginName PluginGp)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

target_compile_options(${PluginName} PRIVATE -DHAVE_CONFIG_H)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include/ddbplugin)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dolphindbGnuplot)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dolphindbGnuplot/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dolphindbGnuplot/term)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/libgd/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/libjpeg/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/libpng/include)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} DIR_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/dolphindbGnuplot/src DIR_SRCS)

add_library(gnuPlot STATIC ${DIR_SRCS})
target_compile_options(gnuPlot PRIVATE -fPIC -DHAVE_CONFIG_H)
target_compile_features(gnuPlot PRIVATE cxx_std_11 c_std_11)

set_target_properties(${PluginName} PROPERTIES LINK_FLAGS "-Wl,-rpath,$ORIGIN")
target_link_directories(PluginGp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/gpThirdParty)
target_link_libraries(PluginGp gnuPlot readline tinfo gd)
