cmake_minimum_required(VERSION 3.13)
set(PluginName PluginPy)
project(${PluginName})
include(../template.cmake)
CreatePlugin(${PluginName})

# 允许多个 python 版本并存时使用特定版本
set(PYTHON_VERSION "" CACHE STRING "python version")
# conda create -n <env_name> python=<3.x>
if (PYTHON_VERSION STREQUAL "")
    find_package(Python COMPONENTS Development.Embed REQUIRED)
else()
    find_package(Python ${PYTHON_VERSION} EXACT COMPONENTS Development.Embed REQUIRED)
endif()
set(PYBIND11_NOPYTHON ON)
target_link_directories(PluginPy PRIVATE ${CMAKE_INSTALL_PREFIX}/py)
add_subdirectory(pybind11-2.12.1)
target_link_libraries(PluginPy Python::Python libopenblas.so.0 pybind11::embed)
set_target_properties(PluginPy PROPERTIES LINK_FLAGS "-Wl,-rpath,$ORIGIN,-disable-new-dtags -static-libstdc++")