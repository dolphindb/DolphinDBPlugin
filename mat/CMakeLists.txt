cmake_minimum_required(VERSION 3.13)
set(PluginName PluginMat)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

find_package(Matlab REQUIRED COMPONENTS MAT_LIBRARY)

target_include_directories(PluginMat PRIVATE ${Matlab_INCLUDE_DIRS})
# Since matlab libraries are compiled without SONAME (readelf -d libmat.so),
# linking matlab targets (e.g. Matlab::mat) will generate a full path dependency in libPluginMat.so
target_link_libraries(PluginMat PRIVATE ${Matlab_MAT_LIBRARY} ${Matlab_MX_LIBRARY})