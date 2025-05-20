cmake_minimum_required(VERSION 3.13)
set(PluginName PluginOrc)
project(${PluginName})

include (../template.cmake)
CreatePlugin(${PluginName})

if(${BUILD_ARM})
    list(APPEND PLUGIN_LINK_LIBRARIES orc protobuf snappy z zstd)
else ()
    list(APPEND PLUGIN_LINK_LIBRARIES orc protobuf snappy lz4 z zstd hdfspp_static pthread)
endif ()

target_link_directories(${PluginName} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/linux)
target_link_libraries(${PluginName} PRIVATE ${PLUGIN_LINK_LIBRARIES})

AddOpenSSL()
AddDependency(cyrus-sasl sasl2)
