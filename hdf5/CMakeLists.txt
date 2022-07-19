cmake_minimum_required(VERSION 3.0)

project(dolphindb_hdf5_plugin)
add_definitions(-DLOCKFREE_SYMBASE)
if(WIN32)
    set (platform_macro "-DWINDOWS")
    set (plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginHdf5_win.txt")
    
elseif(UNIX)
    set(platform_macro "-DLINUX")
    set (plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginHdf5.txt")
    add_compile_options("-D_GLIBCXX_USE_CXX11_ABI=0")
endif()

add_compile_options( "-std=c++11" "-fPIC" ${platform_macro} "-Wall" "-Werror")

macro(replace_cxx_release_flag src_flag dest_flag)
    string(REPLACE "${src_flag}" "${dest_flag}" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
endmacro(replace_cxx_release_flag flag)

replace_cxx_release_flag("-O3" "-O2") 
#replace -O3 with -O2

if(WIN32)
include_directories(${CMAKE_SOURCE_DIR}/../include ${CMAKE_SOURCE_DIR}/include_win/hdf5
${CMAKE_SOURCE_DIR}/include_win/c-blosc ${CMAKE_SOURCE_DIR}/include_win/hdf5-blosc)
elseif(UNIX)
     include_directories(${CMAKE_SOURCE_DIR}/../include ${CMAKE_SOURCE_DIR}/include/hdf5
     ${CMAKE_SOURCE_DIR}/include/c-blosc ${CMAKE_SOURCE_DIR}/include/hdf5-blosc)
endif()

link_directories(${CMAKE_BINARY_DIR} ./lib)
aux_source_directory(${PROJECT_SOURCE_DIR}/src DIR_SRCS)

add_library(PluginHdf5 SHARED ${DIR_SRCS})
if(WIN32)
    target_link_libraries(PluginHdf5 DolphinDB hdf5_hl hdf5_cpp hdf5 blosc z)
elseif(UNIX)
    target_link_libraries(PluginHdf5 DolphinDB hdf5_hl hdf5_cpp hdf5 blosc dl z)
endif()

add_custom_command(TARGET PluginHdf5 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${plugin_file} ${CMAKE_BINARY_DIR}/PluginHdf5.txt
    COMMENT "copy PluginHdf5.txt"
)
# enable_testing()

# add_executable(h5test ./left/tests.cpp)
# target_link_libraries(h5test PluginHdf5)

# add_test(h5test ./h5test)