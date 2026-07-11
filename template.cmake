# 插件 CMakeLists.txt 模板

set(PluginVersion 3.00.6.0)

function(CreatePlugin PluginName)
    message(STATUS "Detecting build system: ${CMAKE_SYSTEM_NAME}")
    # 用户可使用的构建选项
    set(LINK_DIRECTORIES "" CACHE STRING "link directories")

    # 项目必需的构建配置
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    set(CMAKE_CXX_FLAGS_ASAN "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -Og -fno-optimize-sibling-calls -fno-ipa-icf -fno-omit-frame-pointer" PARENT_SCOPE)
    set(CMAKE_CXX_FLAGS_GCOV "${CMAKE_CXX_FLAGS_DEBUG} --coverage" PARENT_SCOPE)
    # Fortification level 3 has significant performance impact on gcc-8.4.0
    # Reference: https://developers.redhat.com/articles/2022/09/17/gccs-new-fortification-level
    if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "12.0.0")
            set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -D_FORTIFY_SOURCE=3" PARENT_SCOPE)
        else()
            set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -D_FORTIFY_SOURCE=2" PARENT_SCOPE)
        endif()
    endif()

    add_library(${PluginName} SHARED)
    target_sources(${PluginName} PRIVATE ../src/PluginLogger.cpp)
    add_subdirectory(src)
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "11.0.0")
            target_compile_options(${PluginName} PRIVATE -march=x86-64-v3)
        else()
            target_compile_options(${PluginName} PRIVATE -mavx2 -mfma)
        endif()
    endif()

    # 编译
    if (${ARGC} EQUAL 2)
        target_compile_definitions(${PluginName} PRIVATE LOG_NAME=${ARGV1})
    endif()
    target_compile_definitions(${PluginName} PRIVATE PLUGIN_NAME=${PluginName})
    target_compile_features(${PluginName} PRIVATE cxx_std_17)
    if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_compile_options(${PluginName} PRIVATE -frecord-gcc-switches)
    endif()
    target_compile_options(${PluginName} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "15.0.0")
        target_compile_options(${PluginName} PRIVATE -Wno-c++20-extensions)
    endif()
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        # We can ignore this warning as long as this plugin is built with the same compiler as DolphinDB
        target_compile_options(${PluginName} PRIVATE -Wno-return-type-c-linkage)
    endif()
    target_include_directories(${PluginName} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
        ${CMAKE_CURRENT_SOURCE_DIR}/../include/ddbplugin
        ${CMAKE_CURRENT_SOURCE_DIR}/../third_party
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/include
    )

    # 链接
    target_link_directories(${PluginName} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/lib)
    if(LINK_DIRECTORIES)
        target_link_directories(${PluginName} PRIVATE ${LINK_DIRECTORIES})
    endif()
    if (CMAKE_SYSTEM_NAME STREQUAL "MSYS" OR CMAKE_SYSTEM_NAME STREQUAL "Windows")
        target_link_libraries(${PluginName} PRIVATE DolphinDB)
    endif()
    if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_link_options(${PluginName} PRIVATE -Wl,-z,now,-z,relro)
        # OpenSSL has weak symbols
        target_link_options(${PluginName} PRIVATE -Wl,-Bsymbolic)
        # for third_party sdks
        target_link_options(${PluginName} PRIVATE "-Wl,-rpath,$ORIGIN")
    endif()

    # 安装
    cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME INSTALL_DIR)

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${PluginName}${CMAKE_SHARED_LIBRARY_SUFFIX}
        DESTINATION ${INSTALL_DIR}
        RENAME lib${PluginName}${CMAKE_SHARED_LIBRARY_SUFFIX}
    )

    set(PluginConfig ${CMAKE_CURRENT_BINARY_DIR}/${PluginName}.txt)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${PluginName}.txt ${PluginConfig})
    install(FILES ${PluginConfig} DESTINATION ${INSTALL_DIR})

endfunction()

# https://github.com/madler/zlib
function(AddZlib)
    find_package(ZLIB CONFIG REQUIRED)
    target_link_libraries(${PluginName} PRIVATE ZLIB::ZLIBSTATIC)

    # for compatibility with module mode
    if(TARGET ZLIB::ZLIBSTATIC AND NOT TARGET ZLIB::ZLIB)
        add_library(ZLIB::ZLIB INTERFACE IMPORTED)
        target_link_libraries(ZLIB::ZLIB INTERFACE ZLIB::ZLIBSTATIC)
    endif()

    # some libs use -lz
    get_target_property(ZLIB_CONFIGS ZLIB::ZLIBSTATIC IMPORTED_CONFIGURATIONS)
    foreach(ZLIB_CONFIG IN LISTS ZLIB_CONFIGS)
        get_target_property(ZLIB_LIBRARY ZLIB::ZLIBSTATIC "IMPORTED_LOCATION_${ZLIB_CONFIG}")
        if(ZLIB_LIBRARY)
            break()
        endif()
    endforeach()
    get_filename_component(ZLIB_LINK_DIR "${ZLIB_LIBRARY}" DIRECTORY)
    target_link_directories(${PluginName} PRIVATE "${ZLIB_LINK_DIR}")
endfunction()

# https://github.com/facebook/zstd
function(AddZstd)
    find_package(zstd REQUIRED)
    target_link_libraries(${PluginName} PRIVATE zstd::libzstd_static)
endfunction()

# https://github.com/lz4/lz4
function(AddLZ4)
    find_package(lz4 REQUIRED)
    target_link_libraries(${PluginName} PRIVATE LZ4::lz4)
endfunction()

function(AddSnappy)
    find_package(Snappy REQUIRED)
    target_link_libraries(${PluginName} PRIVATE Snappy::snappy)
endfunction()

# https://github.com/openssl/openssl
function(AddOpenSSL)
    set(OPENSSL_USE_STATIC_LIBS TRUE)
    find_package(OpenSSL REQUIRED)
    target_link_libraries(${PluginName} PRIVATE OpenSSL::SSL OpenSSL::Crypto)
endfunction()

# https://github.com/curl/curl
function(AddCurl)
    set(CURL_USE_STATIC_LIBS TRUE)
    find_package(CURL CONFIG REQUIRED)
    target_link_libraries(${PluginName} PRIVATE CURL::libcurl)
    AddOpenSSL()
endfunction()

# https://github.com/protocolbuffers/protobuf
function(AddProtobuf)
    find_package(absl CONFIG REQUIRED)
    find_package(ZLIB REQUIRED)
    # find_package(utf8_range CONFIG REQUIRED)
    set(Protobuf_USE_STATIC_LIBS ON)
    find_package(Protobuf CONFIG REQUIRED)
    target_link_libraries(${PluginName} PRIVATE protobuf::libprotobuf)
endfunction()

# https://github.com/apache/arrow
function(AddArrow)
    find_package(Arrow REQUIRED)
    find_package(Parquet REQUIRED)
    target_link_libraries(${PluginName} PRIVATE Arrow::arrow_static Parquet::parquet_static)
endfunction()

function(AddOpenMP)
    find_package(OpenMP COMPONENTS CXX REQUIRED)
    target_link_libraries(${PluginName} PRIVATE OpenMP::OpenMP_CXX)
endfunction()

function(AddDependency PackageName LibName)
    find_library(${LibName}_LIBRARY ${LibName} REQUIRED)
    cmake_path(GET ${LibName}_LIBRARY PARENT_PATH LIBRARY_ROOT)
    target_link_directories(${PluginName} PRIVATE ${LIBRARY_ROOT})
    target_include_directories(${PluginName} PRIVATE ${LIBRARY_ROOT}/../include)
    target_link_libraries(${PluginName} PRIVATE -Wl,--whole-archive ${${LibName}_LIBRARY} -Wl,--no-whole-archive)
endfunction()

function(BuildDependency LibName LibSrc LibInc)
    add_library(${LibName} STATIC ${${LibSrc}})
    target_include_directories(${LibName} PUBLIC ${${LibInc}})
    target_compile_features(${LibName} PRIVATE cxx_std_17)
    target_compile_options(${LibName} PRIVATE -fPIC)
    target_compile_definitions(${LibName} PRIVATE PLUGIN_NAME=${PluginName})
    target_link_libraries(${PluginName} PRIVATE ${LibName})
endfunction()

function(AddSASL2)
    AddOpenSSL()
    AddDependency(cyrus-sasl sasl2)
    AddDependency(krb5 krb5_combined)
    target_link_libraries(${PluginName} PRIVATE resolv)
endfunction()

function(CreateModule PluginName)
    add_library(${PluginName} STATIC)
    add_subdirectory(src ${PluginName})

    # === Compile ===
    if(UNIX)
        # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wall -fsigned-char -D_GLIBCXX_USE_CXX11_ABI=0")
        target_compile_options(${PluginName} PRIVATE
            -frecord-gcc-switches
            -fPIC
            -fsigned-char
        )
        target_compile_definitions(${PluginName} PRIVATE
            _GLIBCXX_USE_CXX11_ABI=0
        )
    endif()
    target_compile_definitions(${PluginName} PRIVATE IS_MODULE)
    target_compile_options(${PluginName} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(${PluginName} PRIVATE -Wno-error=pedantic)
    target_compile_options(${PluginName} PRIVATE -Wno-error=unused-parameter)

    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        # We can ignore this warning as long as this plugin is built with the same compiler as DolphinDB
        target_compile_options(${PluginName} PRIVATE -Wno-return-type-c-linkage)
    endif()
    target_include_directories(${PluginName} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
        ${CMAKE_CURRENT_SOURCE_DIR}/../include/ddbplugin
        ${CMAKE_CURRENT_SOURCE_DIR}/../../base/swordfish/include
    )

    # === Link ===
    if(LINK_DIRECTORIES)
        target_link_directories(${PluginName} PRIVATE ${LINK_DIRECTORIES})
    endif()
    if (WIN32)
        target_link_libraries(${PluginName} PRIVATE DolphinDB)
    endif()
    if (UNIX)
        target_link_options(${PluginName} PRIVATE -Wl,-z,now,-z,relro)
    endif()
endfunction()

function(UseSignedCharOnARM)
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
        target_compile_options(${PluginName} PRIVATE -fsigned-char)
    endif()
endfunction()
