cmake_minimum_required(VERSION 3.22)
set(PluginName PluginKDB)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

AddLZ4()
AddZlib()
AddSnappy()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/hdb)
cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME INSTALL_DIR)
target_compile_definitions(${PluginName} PUBLIC KXVER=3)

target_compile_options(${PluginName} PRIVATE -Wno-error)

get_target_property(_opts ${PluginName} COMPILE_OPTIONS)
if(_opts)
    list(REMOVE_ITEM _opts "-march=x86-64-v3" "-mavx2" "-mfma")
    set_target_properties(${PluginName} PROPERTIES COMPILE_OPTIONS "${_opts}")
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(optimize_opts -O2) # prevent SIMD unaligned unaligned bug
    target_link_directories(${PluginName} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/win)
    set(kdb_lib "${CMAKE_CURRENT_SOURCE_DIR}/lib/win/c.lib")
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/lib/win/c.dll DESTINATION ${INSTALL_DIR})
endif()
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_directories(${PluginName} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/linux)
    set_target_properties(${PluginName} PROPERTIES LINK_FLAGS "-Wl,-rpath,$ORIGIN")
    set(kdb_lib "${CMAKE_CURRENT_SOURCE_DIR}/lib/linux/c.o")
endif()
target_link_libraries(${PluginName} PRIVATE ${kdb_lib})
