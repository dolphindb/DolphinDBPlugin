cmake_minimum_required(VERSION 3.0)

project(dolphindb_hdf5_plugin)
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
    include_directories("../include" "./include_win" )
elseif(UNIX)
     include_directories("../include" "./include")
endif()

link_directories(${CMAKE_BINARY_DIR} ./lib)

add_library(PluginHdf5 SHARED ./src/hdf5_plugin.cpp)
if(WIN32)
    target_link_libraries(PluginHdf5 DolphinDB hdf5_hl hdf5_cpp hdf5 z)
elseif(UNIX)
    target_link_libraries(PluginHdf5 DolphinDB hdf5_hl hdf5_cpp hdf5 dl z)
endif()

add_custom_command(TARGET PluginHdf5 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${plugin_file} ${CMAKE_BINARY_DIR}/PluginHdf5.txt
    COMMENT "copy PluginHdf5.txt"
)
# enable_testing()

# add_executable(h5test ./left/tests.cpp)
# target_link_libraries(h5test PluginHdf5)

# add_test(h5test ./h5test)