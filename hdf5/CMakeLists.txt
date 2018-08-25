cmake_minimum_required(VERSION 3.0)

project(dolphindb_hdf5_plugin)

find_package(HDF5 REQUIRED)

# set(CMAKE_CXX_COMPILER "/home/swang/gcc/gcc6.2-install/bin/g++")

LINK_DIRECTORIES(${HDF5_LIBRARIES_DIRS})
INCLUDE_DIRECTORIES("./include" "../include" ${HDF5_INCLUDE_DIRS})



aux_source_directory("./src" hdf5_plugin_src)

if(WIN32)
    set (platform_macro "-DWINDOWS")
    set (plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginHdf5_win.txt")
elseif(UNIX)
    set(platform_macro "-DLINUX")
    set (plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginHdf5.txt")
endif()

add_compile_options( "-std=c++11" "-fPIC" ${platform_macro} "-Wall" "-Werror" ${HDF5_DEFINITIONS} )

macro(replace_cxx_release_flag src_flag dest_flag)
    string(REPLACE "${src_flag}" "${dest_flag}" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
endmacro(replace_cxx_release_flag flag)

replace_cxx_release_flag("-O3" "-O2") 
#replace -O3 with -O2


add_library(PluginHdf5 SHARED ${hdf5_plugin_src})
target_link_libraries(PluginHdf5  DolphinDB ${HDF5_C_LIBRARIES})



add_custom_command( TARGET PluginHdf5 POST_BUILD 
                    COMMAND cp ${plugin_file} "${CMAKE_BINARY_DIR}/PluginHdf5.txt" 
                    COMMENT "copy PluginHdf5.txt")

# enable_testing()

# add_executable(h5test ./left/tests.cpp)
# target_link_libraries(h5test PluginHdf5)

# add_test(h5test ./h5test)