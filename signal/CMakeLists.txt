cmake_minimum_required(VERSION 3.22)
set(CMAKE_SKIP_RPATH TRUE)

set(PluginName PluginSignal)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

if(DEFINED BUILD_ARM OR CMAKE_C_COMPILER MATCHES "(arm|aarch64)")
    message("build arm")
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/linux_arm/include ${INC_DIR})
    link_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/linux_arm/lib)
    target_link_libraries(${PluginName} 
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/linux_arm/lib/libfftw3_omp.so
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/linux_arm/lib/libfftw3.so
        m pthread)
    target_compile_options(${PluginName} PRIVATE -Wno-unused-parameter -Wno-vla -Wno-unknown-pragmas -Wno-pedantic)
    set_target_properties(${PluginName} PROPERTIES LINK_FLAGS "-Wl,-rpath,$ORIGIN")
    cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME INSTALL_DIR)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/linux_arm/lib/libfftw3_omp.so DESTINATION ${INSTALL_DIR})
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/linux_arm/lib/libfftw3.so DESTINATION ${INSTALL_DIR})
elseif(MINGW)
    target_include_directories(${PluginName} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/windows/include)
    target_link_directories(${PluginName} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/windows/lib)
    target_link_libraries(${PluginName} PRIVATE ${PLUGIN_LINK_OPTIONS} 
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/windows/lib/libfftw3-3.dll
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/windows/lib/libgomp-1.dll
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/windows/lib/libwavelib.a
        m)
    target_compile_options(${PluginName} PRIVATE -Wno-unused-parameter -Wno-vla -Wno-unknown-pragmas -Wno-pedantic)
    cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME INSTALL_DIR)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/windows/lib/libfftw3-3.dll DESTINATION ${INSTALL_DIR})
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/windows/lib/libgomp-1.dll DESTINATION ${INSTALL_DIR})
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/windows/lib/libwinpthread-1.dll DESTINATION ${INSTALL_DIR})
else()
    target_include_directories(${PluginName} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/linux_x86/include)
    target_link_directories(${PluginName} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/linux_x86/lib)
    target_link_libraries(${PluginName} PRIVATE ${PLUGIN_LINK_OPTIONS} 
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/linux_x86/lib/libfftw3_omp.so.3
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/linux_x86/lib/libfftw3.so.3
        wavelib
        m
        pthread)
    target_compile_options(${PluginName} PRIVATE -Wno-unused-parameter -Wno-vla -Wno-unknown-pragmas -Wno-pedantic)
    set_target_properties(${PluginName} PROPERTIES LINK_FLAGS "-Wl,-rpath,$ORIGIN")
    cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME INSTALL_DIR)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/linux_x86/lib/libfftw3_omp.so.3 DESTINATION ${INSTALL_DIR})
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/linux_x86/lib/libfftw3.so.3 DESTINATION ${INSTALL_DIR})
endif()
