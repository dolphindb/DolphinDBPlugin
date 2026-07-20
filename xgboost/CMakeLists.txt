cmake_minimum_required(VERSION 3.22)

set(PluginName PluginXgboost)
project(${PluginName})

if("${XGBOOST_VERSION}" STREQUAL "1.2")
    set(XGBOOST_PREDICT_DEF "2,7,0,0,(model, X, [outputMargin=false], [ntreeLimit=0], [predLeaf=false], [predContribs=false], [training=false])")
else()
    set(XGBOOST_PREDICT_DEF "2,6,0,0,(model, X, [type=0], [iterationRange], [strictShape=false], [training=false])")
endif()

include(../template.cmake)
CreatePlugin(${PluginName})

if (UNIX)
    MESSAGE(STATUS "Platform: UNIX")
    set_target_properties(${PluginName} PROPERTIES LINK_OPTIONS "-Wl,-z,now,-z,relro") # DPLG-4961

    # stdc++fs is for centos 7 compile
    list(APPEND PLUGIN_LINK_LIBRARIES libxgboost.a libdmlc.a stdc++fs)

    # only xgboost 1.2 has rabit dependency
    set(THIRD_PARTY_TYPE  "${XGBOOST_VERSION}_linux")
    if("${XGBOOST_VERSION}" STREQUAL "1.2") 
        list(APPEND PLUGIN_LINK_LIBRARIES librabit.a)
    endif()

elseif (WIN32)
    MESSAGE(STATUS "Platform: WINDOWS")
    set(THIRD_PARTY_TYPE "${XGBOOST_VERSION}_win")
    if("${XGBOOST_VERSION}" STREQUAL "1.2") # only xgboost 1.2 has rabit dependency
    else()
        message(FATAL_ERROR "Xgboost version ${XGBOOST_VERSION} doesn't support Windows")
    endif()

    list(APPEND PLUGIN_LINK_LIBRARIES xgboost.dll)
    list(APPEND INSTALL_FILES ${CMAKE_CURRENT_LIST_DIR}/thirdParty/${THIRD_PARTY_TYPE}/lib/xgboost.dll)
else ()
    message(FATAL_ERROR "Unsupported platform.")
endif ()

set(PLUGIN_LINK_FLAGS "-L ./ -Wl,-rpath,$ORIGIN" "-fopenmp -static-libstdc++")

# set more compile options and link directories
target_compile_options(${PluginName} PRIVATE "-Wno-error")
target_link_options(${PluginName} PRIVATE "-Wl,--verbose")
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
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${PluginName}${CMAKE_SHARED_LIBRARY_SUFFIX} DESTINATION ${INSTALL_DIR}/${XGBOOST_VERSION})
