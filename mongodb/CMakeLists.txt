cmake_minimum_required(VERSION 3.0)
project(PluginMongodb)

set(CMAKE_CXX_STANDARD 11)

if (WIN32)
    set(platform_macro "-DWINDOWS")
    link_directories(${PROJECT_SOURCE_DIR}/bin/win64) 
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginMongodb_win32.txt")
    file(COPY ${PROJECT_SOURCE_DIR}/bin/win64/libbson-1.0.dll DESTINATION ${CMAKE_BINARY_DIR}/)
    file(COPY ${PROJECT_SOURCE_DIR}/bin/win64/libmongoc-1.0.dll DESTINATION ${CMAKE_BINARY_DIR}/)
    file(COPY ${PROJECT_SOURCE_DIR}/bin/win64/libsasl2-3.dll DESTINATION ${CMAKE_BINARY_DIR}/)
    file(COPY ${PROJECT_SOURCE_DIR}/bin/win64/libzlib.dll DESTINATION ${CMAKE_BINARY_DIR}/)
elseif(UNIX)
    set(platform_macro "-DLINUX")
    add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
    set(CMAKE_EXE_LINKER_FLAGS,"-Wl,-rpath,.")
    link_directories(${PROJECT_SOURCE_DIR}/bin/linux64) 
    link_libraries(${PROJECT_SOURCE_DIR}/bin/linux64/libbson-1.0.so.0)
    link_libraries(${PROJECT_SOURCE_DIR}/bin/linux64/libmongoc-1.0.so.0)
    link_libraries(${PROJECT_SOURCE_DIR}/bin/linux64/libicudata.so.52)
    link_libraries(${PROJECT_SOURCE_DIR}/bin/linux64/libicuuc.so.52)
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginMongodb.txt")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${platform_macro} -fPIC -O3 ")

include_directories(${PROJECT_SOURCE_DIR}/include/libbson-1.0)
include_directories(${PROJECT_SOURCE_DIR}/include/libmongoc-1.0)
include_directories(${PROJECT_SOURCE_DIR}/../include)

link_directories("${CMAKE_BINARY_DIR}")

aux_source_directory(${PROJECT_SOURCE_DIR}/src DIR_SRCS)

add_library(PluginMongodb SHARED ${DIR_SRCS})

if (WIN32)
target_link_libraries(PluginMongodb DolphinDB mongoc-1.0 bson-1.0)
elseif(UNIX)
target_link_libraries(PluginMongodb DolphinDB)
endif()

configure_file(${plugin_file} ${CMAKE_BINARY_DIR}/PluginMongodb.txt COPYONLY)
