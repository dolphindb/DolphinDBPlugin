cmake_minimum_required(VERSION 3.0)
project(kafka)
add_definitions(-DLOCKFREE_SYMBASE)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if (WIN32)
    set(platform_macro "-DWINDOWS")
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginKafka.txt")
elseif (UNIX)
    set(platform_macro "-DLINUX")
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginKafka.txt")
    add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
endif ()

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${platform_macro} -fPIC -std=c++11 -Wall -rdynamic ")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -fPIC -std=c++11 -Wall -rdynamic ")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -std=c++11 -Wall -rdynamic ")

if(DEFINED BUILD_ARM)
    message("build arm")
    link_directories(${CMAKE_BINARY_DIR})
    link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)

    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
    include_directories(/usr/local/boost/include)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/cppkafka/include)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/cppkafka/include/cppkafka)

    aux_source_directory("./src" plugin_kafka_src)
    aux_source_directory("cppkafka/src" cppkafka)
    #add_subdirectory(cppkafka)
    
    add_library(PluginKafka SHARED ${plugin_kafka_src} ${cppkafka})
    target_link_libraries(PluginKafka rdkafka rdkafka++ DolphinDB)
    set_target_properties(PluginKafka PROPERTIES LINK_FLAGS "-Wl,-rpath,$ORIGIN:.")
    #target_link_libraries(PluginKafka rdkafka++ cppkafka DolphinDB)

else ()
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
    link_directories(${CMAKE_BINARY_DIR})

    aux_source_directory("./src" plugin_kafka_src)
    aux_source_directory("cppkafka/src" cppkafka)

    add_subdirectory(cppkafka)

    add_library(PluginKafka SHARED ${plugin_kafka_src})
    target_link_libraries(PluginKafka librdkafka.a librdkafka++.a libcppkafka.a libz.a DolphinDB)
    # target_link_libraries(PluginKafka rdkafka++ cppkafka DolphinDB)
endif()

add_custom_command(TARGET PluginKafka POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${plugin_file} ${CMAKE_BINARY_DIR}/PluginKafka.txt
    COMMENT "copy PluginKafka.txt"
)
