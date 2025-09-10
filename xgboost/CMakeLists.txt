cmake_minimum_required(VERSION 3.22)

set(PluginName PluginXgboost)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})


# # 允许多个 python 版本并存时使用特定版本
# set(XGBOOST_VERSION "" CACHE STRING "python version")
# # apt install libxgboost-dev
# # no rpm package
# if (XGBOOST_VERSION STREQUAL "")
#     find_package(xgboost REQUIRED)
# else()
#     find_package(xgboost ${XGBOOST_VERSION} EXACT REQUIRED)
#     target_compile_definitions(())
# endif()

# target_link_libraries(${PluginName} PRIVATE xgboost::xgboost)

set(PLUGIN_LINK_LIBRARIES
)
set(INSTALL_FILES
)
set(PLUGIN_LINK_FLAGS
    "-L ./ -Wl,-rpath,$ORIGIN"
)
set(THIRD_PARTY_PATH  ${CMAKE_CURRENT_LIST_DIR}/thirdParty/)

if("${XGBOOST_VERSION}" STREQUAL "2.0")
    list(APPEND PLUGIN_COMPILE_DEFINITIONS "-DXGBOOST_2_0_0")
elseif("${XGBOOST_VERSION}" STREQUAL "1.2")
    list(APPEND PLUGIN_COMPILE_DEFINITIONS "-DXGBOOST_1_2_0")
else()
    message(STATUS "Set XGBOOST_VERSION to 1.2 as default")
    set(XGBOOST_VERSION "1.2") # use 1.2 as default
endif()

set(destPluginTxt "PluginXgboost.txt")
if (UNIX)
    MESSAGE(STATUS "Platform: UNIX")
    list(APPEND PLUGIN_COMPILE_DEFINITIONS "-DLINUX")
    list(APPEND PLUGIN_LINK_LIBRARIES
        ${THIRD_PARTY_PATH}/lib/${XGBOOST_VERSION}/libxgboost.a
        ${THIRD_PARTY_PATH}/lib/${XGBOOST_VERSION}/libdmlc.a
        stdc++fs # for centos 7 compile
    )
    if("${XGBOOST_VERSION}" STREQUAL "1.2") # only xgboost 1.2 has rabit dependency
        list(APPEND PLUGIN_LINK_LIBRARIES
            ${THIRD_PARTY_PATH}/lib/${XGBOOST_VERSION}/librabit.a
        )
        set(pluginFileTxt "${CMAKE_CURRENT_LIST_DIR}/PluginXgboost1.2.txt")
    else()
        set(pluginFileTxt "${CMAKE_CURRENT_LIST_DIR}/PluginXgboost2.0.txt")
    endif()
elseif (WIN32)
    MESSAGE(STATUS "Platform: WINDOWS")
    if("${XGBOOST_VERSION}" STREQUAL "2.0")
        message(FATAL_ERROR "Xgboost 2.0 version plugin doesn't support windows")
    endif()
    list(APPEND PLUGIN_LINK_LIBRARIES DolphinDB
        ${THIRD_PARTY_PATH}/lib/${XGBOOST_VERSION}/xgboost.dll
    )
    list(APPEND INSTALL_FILES
        ${THIRD_PARTY_PATH}/lib/${XGBOOST_VERSION}/xgboost.dll
    )
    list(APPEND PLUGIN_COMPILE_DEFINITIONS "-DWINDOWS")
    set(pluginFileTxt "${CMAKE_CURRENT_LIST_DIR}/PluginXgboost_win.txt")
else ()
    message(FATAL_ERROR "Unsupported platform.")
endif ()

if("${XGBOOST_VERSION}" STREQUAL "2.0")
    set_property(TARGET ${PluginName} PROPERTY CXX_STANDARD 14)
elseif("${XGBOOST_VERSION}" STREQUAL "1.2")
endif()

if (UNIX)
    set_target_properties(${PluginName} PROPERTIES LINK_OPTIONS "-Wl,-z,now,-z,relro") # DPLG-4961
endif()

list(APPEND PLUGIN_LINK_FLAGS "-fopenmp -static-libstdc++")

target_compile_options(${PluginName} PRIVATE "-Wno-error")
target_include_directories(${PluginName} PRIVATE ${THIRD_PARTY_PATH}/include/${XGBOOST_VERSION}/)
target_compile_definitions(${PluginName} PRIVATE ${PLUGIN_COMPILE_DEFINITIONS})

target_link_libraries(${PluginName} PRIVATE ${PLUGIN_LINK_FLAGS} ${PLUGIN_LINK_LIBRARIES})

cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME INSTALL_DIR)
install(FILES ${INSTALL_FILES} DESTINATION ${INSTALL_DIR})
install(FILES ${pluginFileTxt} DESTINATION ${INSTALL_DIR} RENAME PluginXgboost.txt )
