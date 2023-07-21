cmake_minimum_required (VERSION 2.8)

project (dolphindb_insight_plugin)

set(CMAKE_CXX_COMPILER g++)
add_compile_options(-std=c++11 -DLINUX -D_GLIBCXX_USE_CXX11_ABI=0 -Wall -O3 -g -fPIC -Wl,--no-as-needed)

aux_source_directory(src1 DIR_LIB_SRCS)
include_directories(
    ${PROJECT_SOURCE_DIR}/../include
    ${PROJECT_SOURCE_DIR}/include/insight-tcp-client/mdc_gateway_client/include
    ${PROJECT_SOURCE_DIR}/include/insight-tcp-client/mdc_gateway_client/protobuf
    ${PROJECT_SOURCE_DIR}/include/insight-tcp-client/protobuf-3.1.0/include
)
link_directories(
    ${CMAKE_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}/lib
)

add_library(PluginInsight SHARED ${DIR_LIB_SRCS})
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)
target_link_libraries(PluginInsight
    #dl
    DolphinDB
    #pthread
    #ssl
    #crypto
    #uuid
    #rt
    #aeron_client_shared
    #aeron_driver
    ACE
    ACE_SSL
    protobuf
    mdc_gateway_client
    #grpc++
    #grpc
    #cares
    #address_sorting
    #z
    #gpr
)

set_target_properties(PluginInsight PROPERTIES LINK_FLAGS "-Wl,-rpath,$ORIGIN,-disable-new-dtags")

set(plugin_file "${PROJECT_SOURCE_DIR}/PluginInsight.txt")
add_custom_command(
    TARGET PluginInsight POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${plugin_file} ${PROJECT_SOURCE_DIR}/build/PluginInsight.txt
    COMMENT "copy PluginInsight.txt"
)
