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
    add_executable(${PROJECT_NAME} ./book/test.cpp)
    target_link_libraries(
            ${PROJECT_NAME}
            PUBLIC
            gtest
            gtest_main
            pthread)
    gtest_discover_tests(${PROJECT_NAME})
else()
    add_definitions(-DLOGGING_LEVEL_1 -D_GLIBCXX_USE_CXX11_ABI=0)
    include_directories(${DOLPHINDB_INC_DIR})
    link_directories(${DOLPHINDB_LIB_DIR})
    link_libraries(DolphinDB)
    aux_source_directory(. SOURCE)
    add_library(${PROJECT_NAME} SHARED ${SOURCE})
    target_link_libraries(
            ${PROJECT_NAME}
            PUBLIC
            DolphinDB
            pthread
            dl)
endif()

install(
        TARGETS ${PROJECT_NAME}
        DESTINATION ${PROJECT_SOURCE_DIR}
        CONFIGURATIONS Debug
)