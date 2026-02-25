if (CMAKE_SYSTEM_NAME STREQUAL "MSYS")
    target_sources(${PluginName} PRIVATE
        svm.h
        svmPlugin.cpp
    )
    target_link_directories(${PluginName} PRIVATE ${CMAKE_SOURCE_DIR}/lib/win/)
    target_link_libraries(${PluginName} PRIVATE svm)
    cmake_path(GET CMAKE_SOURCE_DIR FILENAME INSTALL_DIR)
    install(FILES ${CMAKE_SOURCE_DIR}/lib/win/libsvm.dll DESTINATION ${INSTALL_DIR})
endif()
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_sources(${PluginName} PRIVATE
        svmPlugin.cpp
        svm.cpp
    )
endif()

