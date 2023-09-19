cmake_minimum_required(VERSION 3.1)

#========================================
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Debug|Release|RelWithDebInfo|MinSizeRel")
endif ()
message(STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

#=== vcpkg
if(ENV{VCPKG_ROOT})
    set(VCPKG_ROOT "$ENV{VCPKG_ROOT}" CACHE PATH "vcpkg installation directory")
endif()
if(VCPKG_ROOT AND NOT CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE FILEPATH "vcpkg toolchain for CMake")
endif()
message(STATUS "VCPKG_ROOT = ${VCPKG_ROOT}")

if(NOT VCPKG_TARGET_TRIPLET)
    # We support only x64 builds on Windows for now.
    set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "vcpkg target triplet" FORCE)
endif()
message(STATUS "VCPKG_TARGET_TRIPLET = ${VCPKG_TARGET_TRIPLET}")

#========================================
project(PluginKDB LANGUAGES CXX)

option(KDB_READ_PARALLEL "Read local kdb+ data files in parallel?" ON)
message(STATUS "KDB_READ_PARALLEL = ${KDB_READ_PARALLEL}")

#=== Source Code
file(GLOB PluginKDB_HEADER CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.h  )
file(GLOB PluginKDB_SOURCE CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cpp)

#=== Build Target
add_library(PluginKDB SHARED)
set_property(TARGET PluginKDB PROPERTY CXX_STANDARD 11)
target_sources(PluginKDB PUBLIC ${PluginKDB_HEADER} PRIVATE ${PluginKDB_SOURCE})
target_compile_options(PluginKDB PRIVATE -Wall)

#=== Build Types
if    ("${CMAKE_BUILD_TYPE}" MATCHES Debug)
    set(dep_debug_tag "d")
elseif("${CMAKE_BUILD_TYPE}" MATCHES Release)
    set(dep_debug_tag "")
elseif("${CMAKE_BUILD_TYPE}" MATCHES RelWithDebInfo)
    set(dep_debug_tag "")
elseif("${CMAKE_BUILD_TYPE}" MATCHES MinSizeRel)
    set(dep_debug_tag "")
else()
    message(FATAL_ERROR "Unknown build type: ${CMAKE_BUILD_TYPE}")
endif()

target_compile_options(PluginKDB PRIVATE ${optimize_opts})
if(WIN32)
    set(platform_macro WINDOWS)
    set(install_type   win)
    set(kdb_lib "${PROJECT_SOURCE_DIR}/lib/w64/c.lib")
    configure_file(
        "${PROJECT_SOURCE_DIR}/PluginKDB_WIN32.txt"
        "${PROJECT_BINARY_DIR}/PluginKDB.txt" COPYONLY)
    #FIXME: CMake 3.21 offers better way to do this: install(IMPORTED_RUNTIME_ARTIFACTS ...)
    configure_file(
        "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin/zlib1.dll"
        "${PROJECT_BINARY_DIR}/zlib${dep_debug_tag}1.dll" COPYONLY)
    set(extra_bin
        "${PROJECT_SOURCE_DIR}/lib/w64/c.dll"
        "${PROJECT_BINARY_DIR}/zlib${dep_debug_tag}1.dll"
    )
elseif(UNIX)
    set(platform_macro LINUX)
    set(install_type   linux)
    set(kdb_lib "${PROJECT_SOURCE_DIR}/lib/c.o")
    configure_file(
        "${PROJECT_SOURCE_DIR}/PluginKDB.txt"
        "${PROJECT_BINARY_DIR}/PluginKDB.txt" COPYONLY)
    set(extra_bin )
else()
    message(FATAL_ERROR "FIXME: Add build support for this platform!")
endif()
target_compile_definitions(PluginKDB PUBLIC ${platform_macro})
message(STATUS "Extra binaries to install: " ${extra_bin})

#=== ASAN or not?
if (${DDB_USE_ASAN})
    target_compile_options(PluginKDB PRIVATE
        -fsanitize=address              # Enable ASAN.
        -fno-omit-frame-pointer         # Nicer stack traces in error messages.
        -fno-optimize-sibling-calls     # Disable tail call elimination (perfect stack traces if inlining off).
    )
    target_link_options(PluginKDB PRIVATE
        -fsanitize=address
    )
endif()

#=== Dependencies
if(NOT DOLPHINDB_ROOT)
    if(NOT ENV{DOLPHINDB_ROOT})
        message(FATAL_ERROR "DOLPHINDB_ROOT should be defined to provide path to DolphinDB installation!")
    else()
        set(DOLPHINDB_ROOT "$ENV{DOLPHINDB_ROOT}")
    endif()
endif()
message(STATUS "DOLPIHNDB_ROOT = ${DOLPHINDB_ROOT}")
target_compile_definitions(PluginKDB PUBLIC  LOCKFREE_SYMBASE)
target_include_directories(PluginKDB PUBLIC  "${PROJECT_SOURCE_DIR}/../include")
target_link_directories   (PluginKDB PRIVATE "${DOLPHINDB_ROOT}")
target_link_libraries     (PluginKDB PRIVATE DolphinDB)

#== zlib
find_package(ZLIB REQUIRED)
target_link_libraries(PluginKDB PRIVATE ZLIB::ZLIB)

#== kdb+
target_compile_definitions(PluginKDB PUBLIC  KXVER=3)
target_link_directories   (PluginKDB PUBLIC  "${CMAKE_BINARY_DIR}")
target_link_libraries     (PluginKDB PRIVATE ${kdb_lib})
if(KDB_READ_PARALLEL)
    target_compile_definitions(PluginKDB PRIVATE KDB_READ_PARALLEL=1)
endif()

#=== Post-build Actions
install(TARGETS PluginKDB
    RUNTIME DESTINATION "${PROJECT_SOURCE_DIR}/bin/${install_type}"
)
install(FILES
    "${PROJECT_BINARY_DIR}/PluginKDB.txt" ${extra_bin}
    DESTINATION "${PROJECT_SOURCE_DIR}/bin/${install_type}"
)
