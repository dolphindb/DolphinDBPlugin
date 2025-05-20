cmake_minimum_required(VERSION 3.0)
set(CMAKE_SKIP_RPATH TRUE)
project(PluginAWSS3)
add_definitions(-DLOCKFREE_SYMBASE)

set(CMAKE_CXX_STANDARD 11)

include_directories(${PROJECT_SOURCE_DIR}/../include)
include_directories(src)
link_directories(${PROJECT_BINARY_DIR})
aux_source_directory(src PLUGINAWS_SRC)


if(WIN32)
    set(platform_macro "-DWINDOWS")
    include_directories(${PROJECT_SOURCE_DIR}/thirdParty/aws/windows/include)
    link_directories(${PROJECT_SOURCE_DIR}/thirdParty/aws/windows/lib)
elseif(UNIX)
    set(platform_macro "-DLINUX")
    add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
    include_directories(${PROJECT_SOURCE_DIR}/thirdParty/aws/x86_linux/include)
    link_directories(${PROJECT_SOURCE_DIR}/thirdParty/aws/x86_linux/lib)
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
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-Wl,-rpath,$ORIGIN")
target_link_libraries(
	${PROJECT_NAME}
        aws-cpp-sdk-s3
        aws-cpp-sdk-core
        )
