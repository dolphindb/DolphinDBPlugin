cmake_minimum_required(VERSION 3.0)
project(PluginMat)
add_definitions(-DLOCKFREE_SYMBASE)

set(CMAKE_CXX_STANDARD 11)

if (WIN32)
    set(platform_macro "-DWINDOWS")
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginMat_win32.txt")
elseif(UNIX)
    set(platform_macro "-DLINUX")
    add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
    set(CMAKE_EXE_LINKER_FLAGS,"-Wl,-rpath,${matlabRoot}/bin/glnxa64")
    link_directories(${matlabRoot}/bin/glnxa64)
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginMat.txt")
    add_compile_options("-D_GLIBCXX_USE_CXX11_ABI=0")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${platform_macro} -fPIC -O3 ")

include_directories(${PROJECT_SOURCE_DIR}/../include)
if(WIN32)
    include_directories(${PROJECT_SOURCE_DIR}/include/win/include)
elseif(UNIX)
    include_directories(${matlabRoot}/extern/include)
endif()
link_directories("${CMAKE_BINARY_DIR}")

aux_source_directory(${PROJECT_SOURCE_DIR}/src DIR_SRCS)

add_library(PluginMat SHARED ${DIR_SRCS})

target_link_libraries(PluginMat DolphinDB mat mx)

configure_file(${plugin_file} ${CMAKE_BINARY_DIR}/PluginMat.txt COPYONLY)
