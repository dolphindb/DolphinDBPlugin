cmake_minimum_required(VERSION 3.10)
project(signal)
add_definitions(-DLOCKFREE_SYMBASE)

ADD_DEFINITIONS(-DLINUX)
ADD_DEFINITIONS(-D_GLIBCXX_USE_CXX11_ABI=0)
add_compile_options(-fPIC)
FIND_PACKAGE(OpenMP REQUIRED)
if(OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

if("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE "RELEASE")
endif ()

string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
message("CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE})

if("${CMAKE_BUILD_TYPE}" STREQUAL "DEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -fPIC -std=c++11 -Wall")
elseif("${CMAKE_BUILD_TYPE}" STREQUAL "RELEASE")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -fPIC -std=c++11 -Wall")
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "RELWITHDEBINFO")  # release with debug info
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -fPIC -O2 -Wall")
else ()
    message(FATAL_ERROR "Unknown build type: ${CMAKE_BUILD_TYPE}")
endif ()

if (${DDB_USE_ASAN})
    add_compile_options(
        "-fsanitize=address"                # Enable ASAN.
        "-fno-omit-frame-pointer"           # Nicer stack traces in error messages.
        "-fno-optimize-sibling-calls"       # Disable tail call elimination (perfect stack traces if inlining off).
        )
    set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-fsanitize=address")
endif ()


set(INC_DIR ${PROJECT_SOURCE_DIR}/../include)

include_directories(${PROJECT_SOURCE_DIR}/include ${INC_DIR})
link_directories(${LIBDOLPHINDB})
if(DEFINED BUILD_ARM)
    message("build arm")
    link_directories(${PROJECT_SOURCE_DIR}/lib_arm)
else()
    link_directories(${PROJECT_SOURCE_DIR}/lib)
endif()
link_directories(${PROJECT_BINARY_DIR})
aux_source_directory(${PROJECT_SOURCE_DIR}/src DIR_SRCS)
add_library(PluginSignal SHARED ${DIR_SRCS})
set_target_properties(PluginSignal PROPERTIES LINK_FLAGS "-Wl,-rpath,$ORIGIN")
target_link_libraries(PluginSignal DolphinDB fftw3_omp fftw3 m pthread)