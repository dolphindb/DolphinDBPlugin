if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RELEASE)
endif (NOT CMAKE_BUILD_TYPE)

string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
if("${CMAKE_BUILD_TYPE}" STREQUAL "DEBUG")
    list(APPEND PLUGIN_COMPILE_OPTIONS "-ggdb" "-O0" "-fno-omit-frame-pointer" "-fno-optimize-sibling-calls")
elseif("${CMAKE_BUILD_TYPE}" STREQUAL "RELEASE")
    list(APPEND PLUGIN_COMPILE_DEFINITIONS "-DNDEBUG")
    list(APPEND PLUGIN_COMPILE_OPTIONS "-O3")
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "RELWITHDEBINFO")
    list(APPEND PLUGIN_COMPILE_DEFINITIONS "-DNDEBUG")
    list(APPEND PLUGIN_COMPILE_OPTIONS "-ggdb" "-O2")
else ()
    message(FATAL_ERROR "Unknown build type: " ${CMAKE_BUILD_TYPE})
endif ()
message(STATUS "CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE})

message(STATUS "ASAN: ${DDB_USE_ASAN}")
if (${DDB_USE_ASAN})
    set(DDB_ASAN_FLAGS
        "-fsanitize=address"                # Enable ASAN.
        "-fno-omit-frame-pointer"           # Nicer stack traces in error messages.
        "-fno-optimize-sibling-calls"       # Disable tail call elimination (perfect stack traces if inlining off).
        )

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "8")
            list(APPEND DDB_ASAN_FLAGS "-fsanitize-recover=address")
            message(STATUS "ASAN: enable asan recover")
        endif()
    endif()

    list(APPEND PLUGIN_COMPILE_OPTIONS ${DDB_ASAN_FLAGS})
    # list(APPEND PLUGIN_LINK_OPTIONS "-fsanitize=address")

    unset(DDB_ASAN_FLAGS)
endif ()