cmake_minimum_required(VERSION 3.22)
set(PluginName PluginHdf5)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

target_compile_options(${PluginName} PRIVATE "-fno-strict-aliasing" "-Wno-error")
target_include_directories(${PluginName} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include/hdf5
    ${CMAKE_SOURCE_DIR}/include/c-blosc
    ${CMAKE_SOURCE_DIR}/include/hdf5-blosc
)
target_link_directories(${PluginName} PRIVATE ${CMAKE_SOURCE_DIR}/lib)
target_link_libraries(${PluginName} PRIVATE hdf5_hl hdf5_cpp hdf5)

if(UNIX)
    target_link_libraries(${PluginName} PRIVATE dl)
    include(../template.cmake)
    AddDependency(blosc blosc)
else()
    target_link_libraries(${PluginName} PRIVATE blosc)
endif()
