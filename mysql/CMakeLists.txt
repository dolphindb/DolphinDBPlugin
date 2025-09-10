cmake_minimum_required(VERSION 3.22)

set(PluginName PluginMySQL)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

add_subdirectory(contrib/mariadb-connector-c-cmake)

target_compile_options(${PluginName} PRIVATE -Wno-error=extra)
target_link_libraries(${PluginName} PRIVATE mysqlclient)
