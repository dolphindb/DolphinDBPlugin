cmake_minimum_required(VERSION 3.22)

set(PluginName PluginPython)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName} "PLUGIN::PYTHON")

# 允许多个 python 版本并存时使用特定版本
set(PYTHON_VERSION "" CACHE STRING "python version")
# conda create -n <env_name> python=<3.x>

if (PYTHON_VERSION STREQUAL "")
    find_package(Python COMPONENTS Development.Embed REQUIRED)
else()
    find_package(Python ${PYTHON_VERSION} EXACT COMPONENTS Development.Embed REQUIRED)
endif()
set(PYBIND11_NOPYTHON ON)

if (${Python_VERSION} VERSION_LESS 3.7)
    add_subdirectory(pybind11-2.12.1)
elseif (${Python_VERSION} VERSION_LESS 3.8)
    add_subdirectory(pybind11-2.13.6)
else()
    add_subdirectory(pybind11)
endif()
add_subdirectory(converter)

# required by pybind11
target_compile_options(${PluginName} PRIVATE -fvisibility=hidden)

target_link_libraries(${PluginName} PRIVATE
    Python::Python
    pybind11::embed
)
