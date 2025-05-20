cmake_minimum_required(VERSION 3.13)

set(PluginName PluginAWS)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PLUGIN_LINK_LIBRARIES
    aws-cpp-sdk-s3
    aws-cpp-sdk-core
)
set(PLUGIN_LINK_OPTIONS
    "-Wl,-rpath,$ORIGIN,-disable-new-dtags"
)
if(WIN32)
    list(APPEND PLUGIN_COMPILE_DEFINITIONS -DWINDOWS)
    list(APPEND PLUGIN_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/aws/windows/include)
    list(APPEND PLUGIN_LINK_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/aws/windows/lib)
elseif(UNIX)
    cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME INSTALL_DIR)
    if (CMAKE_CXX_COMPILER_VERSION STREQUAL "4.8.5")
        message(STATUS "linking 485 libs")
        list(APPEND PLUGIN_LINK_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/aws/x86_linux/lib/gcc485)
        list(APPEND PLUGIN_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/curl/x86_linux/include)
        install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/aws/x86_linux/lib/gcc485/ DESTINATION ${INSTALL_DIR})
        install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/curl/x86_linux/lib/ DESTINATION  ${INSTALL_DIR})
    else()
        message(STATUS "linking 840 libs")
        list(APPEND PLUGIN_LINK_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/aws/x86_linux/lib/gcc840)
        install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/aws/x86_linux/lib/gcc840/ DESTINATION  ${INSTALL_DIR})
    endif()
    list(APPEND PLUGIN_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/aws/x86_linux/include)
endif()

target_link_libraries(${PluginName} ${PLUGIN_LINK_OPTIONS} ${PLUGIN_LINK_LIBRARIES})
target_link_directories(${PluginName} PRIVATE ${PLUGIN_LINK_DIRECTORIES})
target_compile_definitions(${PluginName} PRIVATE ${PLUGIN_COMPILE_DEFINITIONS})
target_include_directories(${PluginName} PRIVATE ${PLUGIN_INCLUDE_DIRECTORIES})

