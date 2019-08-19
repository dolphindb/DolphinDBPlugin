cmake_minimum_required(VERSION 2.8)
project(DolphinDBPlugin)
set(CMAKE_CXX_STANDARD 11)
 #set(MINGW32_LOCATION C://MinGW/MinGW/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_VERBOSE_MAKEFILE ON)
if (WIN32 OR MINGW)
    set (platform_macro "-DWINDOWS")
elseif (UNIX)
    set (platform_macro "-DLINUX")
    add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
endif ()
message("platform: ${platform_macro}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fPIC -DLOGGING_LEVEL_2 ${platform_macro} ")
set(OPC_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${OPC_ROOT}/src)
include_directories(${OPC_ROOT}/OPCClientToolKit)
include_directories(${OPC_ROOT}/../include)
include_directories(${CMAKE_BINARY_DIR}/curl/include)
include_directories(${OPC_ROOT}/comet/include/)

include_directories(${OPC_ROOT}/IPC/Inc)
link_directories(${CMAKE_BINARY_DIR}/curl/lib)
link_directories(${CMAKE_BINARY_DIR})
#link_directories(${MINGW32_LOCATION}/lib)
#link_directories(${MINGW32_LOCATION}/lib/gcc/mingw32/8.2.0)
set(CMAKE_EXE_LINKER_FLAGS "-Wl,-rpath,.")
add_library(PluginOPC SHARED
                ${OPC_ROOT}/src/opc_main.cpp ${OPC_ROOT}/src/opcimp.cpp)
target_link_libraries(PluginOPC
        DolphinDB
        Ole32
        oleaut32
        uuid
        )
