cmake_minimum_required (VERSION 3.13)
set(PluginName PluginZip)
project(${PluginName})

# for PluginZip and minizip
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/minizip
    ${CMAKE_CURRENT_SOURCE_DIR}/zipper
)

include(../template.cmake)
CreatePlugin(${PluginName})

target_compile_definitions(${PluginName} PRIVATE HAVE_AES)

aux_source_directory(minizip/aes DIR_LIB_SRCS)
aux_source_directory(minizip DIR_LIB_SRCS)
aux_source_directory(zipper DIR_LIB_SRCS)

if(WIN32)
    include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/minizip/win32
    )
    aux_source_directory(minizip/win32 DIR_LIB_SRCS)
endif()

BuildDependency(minizip DIR_LIB_SRCS "")
target_compile_definitions(minizip PRIVATE HAVE_AES)
if(WIN32)
    target_compile_options(minizip PRIVATE -DUSE_WINDOWS)
    target_link_directories(${PluginName} PRIVATE ${PROJECT_SOURCE_DIR}/lib/Win64)
    target_link_directories(${PluginName} PRIVATE ${CMAKE_BINARY_DIR})
    target_link_libraries(${PluginName} PRIVATE zlibstatic)
    set_target_properties(${PluginName} PROPERTIES LINK_FLAGS "-static-libstdc++")
else()
    AddZlib()
endif()
