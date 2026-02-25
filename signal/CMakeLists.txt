cmake_minimum_required(VERSION 3.22)
set(CMAKE_SKIP_RPATH TRUE)

set(PluginName PluginSignal)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

target_compile_options(${PluginName} PRIVATE -Wno-unused-parameter -Wno-vla -Wno-unknown-pragmas -Wno-pedantic)

AddOpenMP()
# Use PkgConfig to find dependencies, but it failed to find omp version of fftw3
find_package(PkgConfig REQUIRED)
pkg_check_modules(fftw3 IMPORTED_TARGET fftw3)
target_link_libraries(${PluginName} PRIVATE PkgConfig::fftw3)
AddDependency(fftw fftw3_omp)

find_package(wavelib REQUIRED)
target_link_libraries(${PluginName} PRIVATE wavelib::wavelib wavelib::wauxlib)

if(MINGW)
    target_include_directories(${PluginName} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/windows/include)
    target_link_directories(${PluginName} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/windows/lib)
    target_link_libraries(${PluginName} PRIVATE ${PLUGIN_LINK_OPTIONS} 
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/windows/lib/libfftw3-3.dll
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/windows/lib/libgomp-1.dll
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/windows/lib/libwavelib.a
        m)
    cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME INSTALL_DIR)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/windows/lib/libfftw3-3.dll DESTINATION ${INSTALL_DIR})
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/windows/lib/libgomp-1.dll DESTINATION ${INSTALL_DIR})
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/windows/lib/libwinpthread-1.dll DESTINATION ${INSTALL_DIR})
endif()
