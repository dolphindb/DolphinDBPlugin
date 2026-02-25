cmake_minimum_required(VERSION 3.22)

set(PluginName PluginMySQL)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

UseSignedCharOnARM()
target_compile_options(${PluginName} PRIVATE -Wno-error=extra)
AddDependency(mariadb mariadbclient)
AddOpenSSL()
if (CMAKE_SYSTEM_NAME STREQUAL "MSYS")
    target_link_libraries(${PluginName} PRIVATE ws2_32 crypt32 shlwapi secur32 bcrypt)
endif()
AddZlib()
AddZstd()
