cmake_minimum_required(VERSION 3.22)

set(PluginName PluginXgboost)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

find_package(xgboost REQUIRED)
target_link_libraries(${PluginName} PRIVATE xgboost::xgboost)
