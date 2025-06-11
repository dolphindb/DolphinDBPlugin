# 插件 CMakeLists.txt 模板

set(PluginVersion 2.00.16)

function(CreatePlugin PluginName)
    # 用户可使用的构建选项
    set(LINK_DIRECTORIES "" CACHE STRING "link directories")

    # 项目必需的构建配置
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    set(CMAKE_CXX_FLAGS_ASAN "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -Og -fno-optimize-sibling-calls -fno-ipa-icf -fno-omit-frame-pointer")
    set(CMAKE_CXX_FLAGS_GCOV "${CMAKE_CXX_FLAGS_DEBUG} --coverage")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -D_FORTIFY_SOURCE=3")

    add_library(${PluginName} SHARED)
    add_subdirectory(src)

    # 编译
    target_compile_features(${PluginName} PRIVATE cxx_std_11)
    if(UNIX)
        target_compile_options(${PluginName} PRIVATE -frecord-gcc-switches)
    endif()
    target_compile_options(${PluginName} PRIVATE -Wall)
    #target_compile_options(${PluginName} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    if(WIN32)
        target_compile_options(${PluginName} PRIVATE -DWINDOWS)
    else()
        target_compile_options(${PluginName} PRIVATE -DLINUX)
    endif()
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        # We can ignore this warning as long as this plugin is built with the same compiler as DolphinDB
        target_compile_options(${PluginName} PRIVATE -Wno-return-type-c-linkage)
    endif()
    target_include_directories(${PluginName} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
        ${CMAKE_CURRENT_SOURCE_DIR}/../include/ddbplugin
    )

    # 链接
    if(LINK_DIRECTORIES)
        target_link_directories(${PluginName} PRIVATE ${LINK_DIRECTORIES})
    endif()
    if (WIN32)
        target_link_libraries(${PluginName} PRIVATE DolphinDB)
    endif()
    if (UNIX)
        target_link_options(${PluginName} PRIVATE -Wl,-z,now,-z,relro)
    endif()

    # 安装
    cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME INSTALL_DIR)

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${PluginName}${CMAKE_SHARED_LIBRARY_SUFFIX} DESTINATION ${INSTALL_DIR})

    set(PluginConfig ${CMAKE_CURRENT_BINARY_DIR}/${PluginName}.txt)
    set(PluginFile ${CMAKE_CURRENT_SOURCE_DIR}/${PluginName}.txt)
    add_custom_command(TARGET ${PluginName} POST_BUILD COMMAND cp ${PluginFile} ${PluginConfig})
    add_custom_command(TARGET ${PluginName} POST_BUILD COMMAND sed -i "\'s/<version>/${PluginVersion}/g\'" ${PluginConfig})
    if (WIN32)
        add_custom_command(TARGET ${PluginName} POST_BUILD COMMAND sed -i "\'s/.so,/.dll,/g\'" ${PluginConfig})
    endif()
    install(FILES ${PluginConfig} DESTINATION ${INSTALL_DIR} RENAME ${PluginName}.txt)

endfunction()

function(BuildDependency LibName LibSrc LibInc)
    add_library(${LibName} STATIC ${${LibSrc}})
    target_compile_features(${LibName} PRIVATE cxx_std_11)
    if(WIN32)
        target_compile_options(${LibName} PRIVATE -DWINDOWS)
    else()
        target_compile_options(${LibName} PRIVATE -DLINUX)
    endif()
    target_include_directories(${LibName} PUBLIC ${${LibInc}})
    target_compile_options(${LibName} PRIVATE -fPIC)
    target_link_libraries(${PluginName} PRIVATE ${LibName})
endfunction()

# apt install libz-dev
# yum install zlib-devel
function(AddZlib)
    set(ZLIB_USE_STATIC_LIBS ON)
    find_package(ZLIB REQUIRED)
    target_link_libraries(${PluginName} PRIVATE ZLIB::ZLIB)
endfunction()

# apt install libssl-dev
# yum install openssl-devel
function(AddOpenSSL)
    set(OPENSSL_USE_STATIC_LIBS TRUE)
    find_package(OpenSSL REQUIRED)
    target_link_libraries(${PluginName} PRIVATE OpenSSL::SSL OpenSSL::Crypto)
endfunction()

function(AddDependency PackageName LibName)
    find_library(${LibName}_LIBRARY ${LibName} REQUIRED)
    cmake_path(GET ${LibName}_LIBRARY PARENT_PATH LIBRARY_ROOT)
    cmake_path(GET LIBRARY_ROOT PARENT_PATH LIBRARY_ROOT)
    set(INCLUDE_DIR ${LIBRARY_ROOT}/include)
    target_include_directories(${PluginName} PRIVATE ${INCLUDE_DIR})
    target_link_libraries(${PluginName} PRIVATE ${${LibName}_LIBRARY})
endfunction()
