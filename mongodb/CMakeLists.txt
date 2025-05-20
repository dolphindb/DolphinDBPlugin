cmake_minimum_required(VERSION 3.13)
set(PluginName PluginMongodb)
project(${PluginName})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 11)

if (WIN32)
    set(platform_macro "-DWINDOWS")
    set(platform_macro "-DWINDOWS")
    link_directories(${CMAKE_CURRENT_SOURCE_DIR}/bin/win64)
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginMongodb_win32.txt")
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/bin/win64/libbson-1.0.dll DESTINATION ${CMAKE_BINARY_DIR}/)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/bin/win64/libmongoc-1.0.dll DESTINATION ${CMAKE_BINARY_DIR}/)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/bin/win64/libsasl2-3.dll DESTINATION ${CMAKE_BINARY_DIR}/)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/bin/win64/libzlib.dll DESTINATION ${CMAKE_BINARY_DIR}/)
elseif(UNIX)
    set(platform_macro "-DLINUX")
    link_directories(${CMAKE_CURRENT_SOURCE_DIR}/bin/linux64)
    link_libraries(${CMAKE_CURRENT_SOURCE_DIR}/bin/linux64/libbson-1.0.so.0)
    link_libraries(${CMAKE_CURRENT_SOURCE_DIR}/bin/linux64/libmongoc-1.0.so.0)
    link_libraries(${CMAKE_CURRENT_SOURCE_DIR}/bin/linux64/libicudata.so.52)
    link_libraries(${CMAKE_CURRENT_SOURCE_DIR}/bin/linux64/libicuuc.so.52)
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginMongodb.txt")
endif()

if (${DDB_USE_ASAN})
    message("enable asan")
    add_compile_options(
        "-fsanitize=address"                # Enable ASAN.
        "-fno-omit-frame-pointer"           # Nicer stack traces in error messages.
        "-fno-optimize-sibling-calls"       # Disable tail call elimination (perfect stack traces if inlining off).
        )
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "8")
            add_compile_options("-fsanitize-recover=address")
            message("enable asan recover")
        endif()
    endif()
    set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-fsanitize=address")
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${platform_macro} -fPIC -O3 ")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/libbson-1.0)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/libmongoc-1.0)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

link_directories("${CMAKE_BINARY_DIR}")

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src DIR_SRCS)

add_library(PluginMongodb SHARED ${DIR_SRCS})

if (WIN32)
target_link_libraries(PluginMongodb DolphinDB mongoc-1.0 bson-1.0)
elseif(UNIX)
target_link_libraries(PluginMongodb)
endif()

set_target_properties(PluginMongodb PROPERTIES LINK_FLAGS "-Wl,-rpath,$ORIGIN")

configure_file(${plugin_file} ${CMAKE_BINARY_DIR}/PluginMongodb.txt COPYONLY)

cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME INSTALL_DIR)
install(TARGETS ${PluginName} DESTINATION ${INSTALL_DIR})
install(FILES ${CMAKE_BINARY_DIR}/${PluginName}.txt DESTINATION ${INSTALL_DIR})
