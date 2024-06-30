cmake_minimum_required(VERSION 3.0)
set(CMAKE_SKIP_RPATH TRUE)
project(PluginAWSS3)
add_definitions(-DLOCKFREE_SYMBASE)

set(CMAKE_CXX_STANDARD 11)

include_directories(${PROJECT_SOURCE_DIR}/../include)
include_directories(${PROJECT_SOURCE_DIR}/thirdParty/aws/windows/include)
include_directories(src)
link_directories(${PROJECT_SOURCE_DIR}/thirdParty/aws/windows/lib)
link_directories(${PROJECT_BINARY_DIR})
aux_source_directory(src PLUGINAWS_SRC)


if(WIN32)
    set(platform_macro "-DWINDOWS")
elseif(UNIX)
    set(platform_macro "-DLINUX")
endif()

add_compile_options(${platform_macro} -O0 -Wall -g -ggdb)

# if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
#     add_compile_options(${platform_macro} -O0 -Wall -g -ggdb)
#     message(STATUS "Build: Debug with -O0 -Wall -g -ggdb")
# else()
#     set(CMAKE_BUILD_TYPE Release)
#     add_compile_options(${platform_macro} -O3 -std=c++11)
#     message(STATUS "Build: Release with -O3")
# endif()

add_library(
	${PROJECT_NAME} SHARED
        ${PLUGINAWS_SRC})

target_link_libraries(
	${PROJECT_NAME}
        DolphinDB
        aws-cpp-sdk-s3
        aws-cpp-sdk-core
        )
