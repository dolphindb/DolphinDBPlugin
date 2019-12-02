cmake_minimum_required(VERSION 3.10)
project(MatchingEngine)

set(DOLPHINDB_INC_DIR "../include" CACHE STRING "")
set(DOLPHINDB_LIB_DIR "/usr/local/lib" CACHE STRING "")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -ggdb -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -w")
set(CMAKE_INSTALL_RPATH "$ORIGIN")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_definitions(-DLOGGING_LEVEL_1)

if (WIN32)
    add_definitions("-DWINDOWS")
elseif (UNIX)
    add_definitions("-DLINUX")
endif ()

if (TEST)
    include(GoogleTest)
    link_libraries(gtest gtest_main)
    add_executable(MatchingEngineTest ./book/test.cpp)
    target_link_libraries(
            MatchingEngineTest
            PUBLIC
            gtest
            gtest_main
            pthread)
    gtest_discover_tests(MatchingEngineTest)
endif ()

add_definitions(-DLOGGING_LEVEL_1)
include_directories(${DOLPHINDB_INC_DIR})
link_directories(${DOLPHINDB_LIB_DIR})
aux_source_directory(. SOURCE)
add_library(MatchingEngine SHARED ${SOURCE})
target_link_libraries(
        MatchingEngine
        PUBLIC
        DolphinDB
        pthread
        dl)
target_compile_definitions(MatchingEngine PRIVATE -D_GLIBCXX_USE_CXX11_ABI=0)
install(
        TARGETS MatchingEngine
        DESTINATION ${PROJECT_SOURCE_DIR}
        CONFIGURATIONS Debug
)