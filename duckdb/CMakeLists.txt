cmake_minimum_required(VERSION 3.22)

set(PluginName PluginDuckDB)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

# Add DuckDB dependency
add_subdirectory(contrib/duckdb)

# Link DuckDB library
if(UNIX)
    target_link_libraries(${PluginName} PRIVATE duckdb)
elseif(WIN32)
    target_link_libraries(${PluginName} PRIVATE duckdb)
endif()

# Suppress warnings for DuckDB headers
target_compile_options(${PluginName} PRIVATE -Wno-noexcept-type)

# Add compatibility flags for newer compilers
target_compile_definitions(${PluginName} PRIVATE
    __STDC_FORMAT_MACROS
    __STDC_LIMIT_MACROS
    _GLIBCXX_USE_CXX11_ABI=0
)

# Add C++ standard flag
target_compile_features(${PluginName} PRIVATE cxx_std_14)

# Add include directory for compatibility headers
target_include_directories(${PluginName} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Force include compatibility header before all other headers

# Ignore pthread_self warning on macOS
if(APPLE)
    target_compile_options(${PluginName} PRIVATE -Wno-shorten-64-to-32)
    # Treat errors as warnings to continue compilation
    target_compile_options(${PluginName} PRIVATE -Wno-error)
    # Use gnu++14 instead of c++14 to allow extensions
    target_compile_options(${PluginName} PRIVATE -std=gnu++14)
endif()
