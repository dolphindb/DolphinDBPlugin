cmake_minimum_required(VERSION 3.22)
set(CMAKE_SKIP_RPATH TRUE)

set(PluginName PluginSignal)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

target_compile_options(${PluginName} PRIVATE -Wno-unused-parameter -Wno-vla -Wno-unknown-pragmas -Wno-pedantic)

# fftw use gnu configure, no cross-platform CMake target
AddDependency(fftw fftw3)
AddDependency(fftw fftw3_omp)
AddOpenMP()

find_package(wavelib REQUIRED)
target_link_libraries(${PluginName} PRIVATE wavelib::wavelib wavelib::wauxlib)
