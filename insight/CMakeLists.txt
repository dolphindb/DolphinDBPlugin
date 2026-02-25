cmake_minimum_required(VERSION 3.22)

set(PluginName PluginInsight)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

target_compile_options(${PluginName} PRIVATE "-w")

target_include_directories(${PluginName} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/include/insight-tcp-client/mdc_gateway_client/include
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/include/insight-tcp-client/mdc_gateway_client/protobuf
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/include/insight-tcp-client/protobuf-3.1.0/include
)
target_link_directories(${PluginName} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/lib
)

target_link_libraries(${PluginName} PRIVATE ACE ACE_SSL protobuf mdc_gateway_client)

set_target_properties(${PluginName} PROPERTIES LINK_FLAGS "-Wl,-rpath,$ORIGIN,-disable-new-dtags")

cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME INSTALL_DIR)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/include/cert DESTINATION ${INSTALL_DIR})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/lib/ DESTINATION ${INSTALL_DIR})
