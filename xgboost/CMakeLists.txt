cmake_minimum_required(VERSION 3.13)

set(PluginName PluginXgboost)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

# 允许多个 python 版本并存时使用特定版本
set(XGBOOST_VERSION "" CACHE STRING "python version")
# apt install libxgboost-dev
# no rpm package
if (XGBOOST_VERSION STREQUAL "")
    find_package(xgboost REQUIRED)
else()
    find_package(xgboost ${XGBOOST_VERSION} EXACT REQUIRED)
    target_compile_definitions(())
endif()

target_link_libraries(${PluginName} PRIVATE xgboost::xgboost)
