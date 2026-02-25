cmake_minimum_required(VERSION 3.22)

set(PluginName PluginODBC)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

include(CheckCXXSourceCompiles)
check_cxx_source_compiles("
    #ifdef __aarch64__
        int main() { return 0; }
    #endif
" AARCH64_MACRO_DEFINED)

if(AARCH64_MACRO_DEFINED)
    target_compile_options(${PluginName} PRIVATE -fsigned-char)
endif()

target_compile_options(${PluginName} PRIVATE -Wno-deprecated-declarations)

set(NANODBC_SRC nanodbc/nanodbc.cpp nanodbcw/nanodbcw.cpp)
set(NANODBC_INC . src ../include ../include/ddbplugin)
BuildDependency(nanodbc NANODBC_SRC NANODBC_INC)
target_compile_definitions(nanodbc PRIVATE NANODBC_ODBC_VERSION=SQL_OV_ODBC3)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_library(odbc_LIBRARY odbc REQUIRED)
    cmake_path(GET odbc_LIBRARY PARENT_PATH LIBRARY_ROOT)
    cmake_path(GET LIBRARY_ROOT PARENT_PATH LIBRARY_ROOT)
    set(INCLUDE_DIR ${LIBRARY_ROOT}/include)
    set(PLUGIN_LINK_OPTIONS "-Wl,-rpath,$ORIGIN,-disable-new-dtags")
    target_link_libraries(${PluginName} PRIVATE ${PLUGIN_LINK_OPTIONS} odbc odbcinst)
    target_link_directories(${PluginName} PRIVATE ${LIBRARY_ROOT}/lib)
else()
    target_link_directories(${PluginName} PRIVATE ${CMAKE_BINARY_DIR})
    target_link_libraries(${PluginName} PRIVATE  odbc32 odbccp32)
endif()
target_include_directories(nanodbc PRIVATE ${INCLUDE_DIR})
target_include_directories(${PluginName} PRIVATE ${INCLUDE_DIR})
target_compile_options(nanodbc PRIVATE -w)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME INSTALL_DIR)
    get_filename_component(REAL_ODBC ${LIBRARY_ROOT}/lib/libodbc.so.2 REALPATH)
    get_filename_component(REAL_ODBCINST ${LIBRARY_ROOT}/lib/libodbcinst.so.2 REALPATH)
    install(FILES 
        ${REAL_ODBC}
        RENAME libodbc.so.2
        DESTINATION ${INSTALL_DIR})
    install(FILES 
        ${REAL_ODBCINST}
        RENAME libodbcinst.so.2
        DESTINATION ${INSTALL_DIR})
endif()

