cmake_minimum_required(VERSION 3.22)
set(PluginName PluginKDB)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/hdb)
cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME INSTALL_DIR)
target_compile_definitions(${PluginName} PUBLIC KXVER=3)

target_compile_options(${PluginName} PRIVATE -Wno-error -O2)  # DPLG-4596 prevent SIMD unaligned unaligned bug

if (WIN32)
    target_link_directories(${PluginName} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/win)
    set(kdb_lib "${CMAKE_CURRENT_SOURCE_DIR}/lib/win/c.lib")
    set(z_lib "${CMAKE_CURRENT_SOURCE_DIR}/lib/win/zlib1.dll")
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/lib/win/c.dll DESTINATION ${INSTALL_DIR})
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/lib/win/zlib1.dll DESTINATION ${INSTALL_DIR})
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/lib/win/libsnappy.dll DESTINATION ${INSTALL_DIR})
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/lib/win/lz4.dll DESTINATION ${INSTALL_DIR})
elseif(UNIX)
    target_link_directories(${PluginName} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/linux)
    set_target_properties(${PluginName} PROPERTIES LINK_FLAGS "-Wl,-rpath,$ORIGIN")
    set(kdb_lib "${CMAKE_CURRENT_SOURCE_DIR}/lib/linux/c.o")
    set(z_lib "${CMAKE_CURRENT_SOURCE_DIR}/lib/linux/libz.a")
endif()
target_link_libraries(${PluginName} PRIVATE ${kdb_lib} ${z_lib} snappy lz4)

