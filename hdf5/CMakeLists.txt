cmake_minimum_required(VERSION 3.22)
set(PluginName PluginHdf5)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

AddZlib()
find_package(hdf5 REQUIRED)
target_link_libraries(${PluginName} PRIVATE hdf5-static hdf5_hl-static hdf5_cpp-static hdf5_hl_cpp-static)
AddDependency(blosc blosc)
