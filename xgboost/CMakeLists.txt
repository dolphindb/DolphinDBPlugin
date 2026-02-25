cmake_minimum_required(VERSION 3.22)

set(PluginName PluginXgboost)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

if (NOT XGBOOST_VERSION)
    set(XGBOOST_VERSION "3.1")
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set_target_properties(${PluginName} PROPERTIES LINK_OPTIONS "-Wl,-z,now,-z,relro") # DPLG-4961
    # only xgboost 1.2 has rabit dependency
    set(THIRD_PARTY_TYPE  "${XGBOOST_VERSION}_linux")
    if("${XGBOOST_VERSION}" STREQUAL "1.2") 
        list(APPEND PLUGIN_LINK_LIBRARIES librabit.a)
    endif()

    # stdc++fs is for centos 7 compile
    list(APPEND PLUGIN_LINK_LIBRARIES libxgboost.a libdmlc.a stdc++fs)
endif()
if (CMAKE_SYSTEM_NAME STREQUAL "MSYS")
    set(THIRD_PARTY_TYPE "${XGBOOST_VERSION}_win")
    if("${XGBOOST_VERSION}" STREQUAL "1.2") # only xgboost 1.2 has rabit dependency
    else()
        message(FATAL_ERROR "Xgboost version ${XGBOOST_VERSION} doesn't support Windows")
    endif()

    list(APPEND PLUGIN_LINK_LIBRARIES xgboost.dll)
    list(APPEND INSTALL_FILES ${CMAKE_CURRENT_LIST_DIR}/thirdParty/${THIRD_PARTY_TYPE}/lib/xgboost.dll)
endif ()

set(PLUGIN_TXT_FILE ${CMAKE_CURRENT_BINARY_DIR}/PluginXgboost.txt.tmp)
configure_file(${CMAKE_CURRENT_LIST_DIR}/PluginXgboost.txt ${PLUGIN_TXT_FILE})
if("${XGBOOST_VERSION}" STREQUAL "1.2")
    add_custom_command(TARGET ${PluginName} PRE_BUILD COMMAND sed -i "\'4d\'" ${PLUGIN_TXT_FILE})
else()
    set_property(TARGET ${PluginName} PROPERTY CXX_STANDARD 14)
    add_custom_command(TARGET ${PluginName} PRE_BUILD COMMAND sed -i "\'3d\'" ${PLUGIN_TXT_FILE})
endif()

set(PLUGIN_LINK_FLAGS "-L ./ -Wl,-rpath,$ORIGIN" "-fopenmp -static-libstdc++")

# set more compile options and link directories
target_compile_options(${PluginName} PRIVATE "-Wno-error")
target_include_directories(${PluginName} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/thirdParty/${THIRD_PARTY_TYPE}/include)
message(STATUS "Include directories: ${CMAKE_CURRENT_LIST_DIR}/thirdParty/${THIRD_PARTY_TYPE}/include")
target_link_directories(${PluginName} PRIVATE 
                        ${CMAKE_CURRENT_LIST_DIR}/thirdParty/${THIRD_PARTY_TYPE}/lib
                        ${CMAKE_CURRENT_LIST_DIR}/thirdParty/${THIRD_PARTY_TYPE}/lib64)
target_compile_definitions(${PluginName} PRIVATE ${PLUGIN_COMPILE_DEFINITIONS})
target_link_libraries(${PluginName} PRIVATE ${PLUGIN_LINK_FLAGS} ${PLUGIN_LINK_LIBRARIES})

# install
cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME INSTALL_DIR)
install(FILES ${INSTALL_FILES} DESTINATION ${INSTALL_DIR}/${XGBOOST_VERSION})
install(FILES ${PLUGIN_TXT_FILE} DESTINATION ${INSTALL_DIR}/${XGBOOST_VERSION} RENAME PluginXgboost.txt)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${PluginName}${CMAKE_SHARED_LIBRARY_SUFFIX} DESTINATION ${INSTALL_DIR}/${XGBOOST_VERSION} RENAME lib${PluginName}${CMAKE_SHARED_LIBRARY_SUFFIX})
