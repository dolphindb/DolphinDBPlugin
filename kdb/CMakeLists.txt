cmake_minimum_required(VERSION 3.1)

#========================================
if("${CMAKE_BUILD_TYPE}" MATCHES "")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "")
endif ()
message(STATUS "CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE})

#=== vcpkg
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

if(NOT DEFINED VCPKG_TARGET_TRIPLET)
    set(VCPKG_TARGET_TRIPLET x64-windows)   # Use x64-windows by default
endif()
message(STATUS "VCPKG_TARGET_TRIPLET: " ${VCPKG_TARGET_TRIPLET})

#========================================
project(PluginKDB LANGUAGES CXX)

#=== Source Code
file(GLOB PluginKDB_HEADER CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.h  )
file(GLOB PluginKDB_SOURCE CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cpp)

#=== Build Target
add_library(PluginKDB SHARED)
set_property(TARGET PluginKDB PROPERTY CXX_STANDARD 11)
target_sources(PluginKDB PUBLIC ${PluginKDB_HEADER} PRIVATE ${PluginKDB_SOURCE})
target_compile_options(PluginKDB PRIVATE -Wall)

#=== Build Types
if    ("${CMAKE_BUILD_TYPE}" MATCHES DEBUG)
    set(optimize_opts -g -O0)
    set(dep_debug_tag "d")
elseif("${CMAKE_BUILD_TYPE}" MATCHES Release)
    set(optimize_opts -O3   )
    set(dep_debug_tag "")
elseif("${CMAKE_BUILD_TYPE}" MATCHES RelWithDebInfo)
    set(optimize_opts -g -O2)
    set(dep_debug_tag "")
else()
    message(FATAL_ERROR "Unknown build type: ${CMAKE_BUILD_TYPE}")
endif()

target_compile_options(PluginKDB PRIVATE ${optimize_opts})
if(WIN32)
    set(platform_macro WINDOWS)
    set(install_type   win)
    set(kdb_lib "${PROJECT_SOURCE_DIR}/lib/w64/c.lib")
    set(z_lib "${PROJECT_SOURCE_DIR}/lib/win/zlib1.dll")
    configure_file(
        "${PROJECT_SOURCE_DIR}/PluginKDB_WIN32.txt"
        "${PROJECT_BINARY_DIR}/PluginKDB.txt" COPYONLY)
    target_link_directories (PluginKDB PUBLIC "${PROJECT_SOURCE_DIR}/lib/win")
    target_compile_definitions(PluginKDB PUBLIC ${platform_macro})
elseif(UNIX)
    set(platform_macro LINUX)
    set(install_type   linux)
    set(kdb_lib "${PROJECT_SOURCE_DIR}/lib/c.o")
    set(z_lib "${PROJECT_SOURCE_DIR}/lib/linux/libz.a")
    configure_file(
        "${PROJECT_SOURCE_DIR}/PluginKDB.txt"
        "${PROJECT_BINARY_DIR}/PluginKDB.txt" COPYONLY)
    set(extra_bin )
    target_link_directories (PluginKDB PUBLIC "${PROJECT_SOURCE_DIR}/lib/linux")
    target_compile_definitions(PluginKDB PUBLIC ${platform_macro} "-D_GLIBCXX_USE_CXX11_ABI=0")
else()
    message(FATAL_ERROR "FIXME: Add build support for this platform!")
endif()

#=== ASAN or not?
if (${DDB_USE_ASAN})
    target_compile_options(PluginKDB PRIVATE
        -fsanitize=address              # Enable ASAN.
        -fno-omit-frame-pointer         # Nicer stack traces in error messages.
        -fno-optimize-sibling-calls     # Disable tail call elimination (perfect stack traces if inlining off).
    )
    # target_link_options(PluginKDB PRIVATE
    #     -fsanitize=address
    # )
endif()

target_compile_definitions(PluginKDB PUBLIC  LOCKFREE_SYMBASE)
target_include_directories(PluginKDB PUBLIC  "${PROJECT_SOURCE_DIR}/../include")
target_include_directories(PluginKDB PUBLIC  "${PROJECT_SOURCE_DIR}/include")
target_link_libraries     (PluginKDB PRIVATE DolphinDB)

# #== zlib
# find_package(ZLIB REQUIRED)
# target_link_libraries(PluginKDB PRIVATE ZLIB::ZLIB)

#== kdb+
target_compile_definitions(PluginKDB PUBLIC  KXVER=3)
target_link_directories   (PluginKDB PUBLIC  "${CMAKE_BINARY_DIR}")
target_link_libraries     (PluginKDB PRIVATE ${kdb_lib} ${z_lib} lz4 snappy)
