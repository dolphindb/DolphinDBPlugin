cmake_minimum_required(VERSION 3.22)
set(PluginName PluginMongodb)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

target_include_directories(${PluginName} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/libbson-1.0)
target_include_directories(${PluginName} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/libmongoc-1.0)

cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME INSTALL_DIR)
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_options(${PluginName} PRIVATE -Wno-error=type-limits)
    target_link_directories(${PluginName} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/bin/win64)
    target_link_libraries(${PluginName} PRIVATE mongoc-1.0 bson-1.0)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin/win64/ DESTINATION ${INSTALL_DIR})
elseif(UNIX)
    target_link_directories(${PluginName} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/bin/linux64)
    target_link_libraries(${PluginName} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/bin/linux64/libbson-1.0.so.0
        ${CMAKE_CURRENT_SOURCE_DIR}/bin/linux64/libmongoc-1.0.so.0
        ${CMAKE_CURRENT_SOURCE_DIR}/bin/linux64/libicudata.so.52
        ${CMAKE_CURRENT_SOURCE_DIR}/bin/linux64/libicuuc.so.52
    )
    set_target_properties(${PluginName} PROPERTIES LINK_FLAGS "-Wl,-rpath,$ORIGIN")
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin/linux64/ DESTINATION ${INSTALL_DIR})
endif()
