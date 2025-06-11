#!/bin/bash

plugin_version=3.00.2.0

function download_thirdparty() {
    # 下载闭源依赖库
    if [ ! -z "$FTP_URL" ]; then
        # export FTP_URL=<ftp地址>
        # export FTP_USER=<用户名:密码>
        file_name=$1.tar.xz
        curl -O $FTP_URL/PluginThirdParty/$file_name --user $FTP_USER
        if [ -f $file_name ]; then
            tar -xf $file_name
        fi
    fi
}

function prepare_dir() {
    set +e
    git submodule update --init .
    if [ ! -n "$1" ] && [ -d build ]; then
        return 0;
    fi
    plugin_name=$(basename $(pwd))
    echo "Configure project $plugin_name."
    download_thirdparty $plugin_name
    # 配置 cmake
    rm -rf build
    mkdir build
    cd build
    build_type="Debug"
    if [ -n "$1" ]; then
        build_type=$1
    fi
    if [ -n "$2" ]; then
        toolchain_dir=$2
        toolchain_arg="-DCMAKE_PREFIX_PATH=$toolchain_dir"
        toolchain_arg="-DZLIB_ROOT=$(ls -d $toolchain_dir/zlib-*) $toolchain_arg"
        toolchain_arg="-DOPENSSL_ROOT_DIR=$(ls -d $toolchain_dir/openssl-*) $toolchain_arg"
        toolchain_arg="-DCURL_ROOT=$(ls -d $toolchain_dir/curl-*) $toolchain_arg"
        library_path=""
        for lib in $(ls $toolchain_dir); do
            library_path="$toolchain_dir/$lib/lib;$library_path"
        done
    fi
    if [ -n "$3" ]; then
        sdk_version=$3
    fi
    set -e
    cmake .. -DCMAKE_BUILD_TYPE=$build_type $toolchain_arg -DCMAKE_LIBRARY_PATH="$library_path" $sdk_version
    cmake .. -DCMAKE_BUILD_TYPE=$build_type
    cd ..
}

function build_plugin() {
    cd build
    set -e
    cmake --build . -j --verbose
    cp compile_commands.json ../..
    cd ..
}

version_greater_equal()
{
    printf '%s\n%s\n' "$2" "$1" | sort --check=quiet --version-sort
}

function install_plugin() {
    cd build
    ver=$(cmake --version | head -n 1 | cut -d ' ' -f3)
    echo $ver
    if version_greater_equal $ver "3.29.0" ; then
        cmake --install .
    else
        cmake --install . --prefix $CMAKE_INSTALL_PREFIX
    fi
    cd ..
}
