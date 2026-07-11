#!/bin/bash

set -e

function apply_thirdparty_patches() {
    for patch_file in $(ls third_party/*.patch 2>/dev/null); do
        patch -d third_party -p1 < "$patch_file"
    done
}

function download_thirdparty() {
    # 下载闭源依赖库
    if [ -z "$FTP_URL" ]; then
        return
    fi
    # export FTP_URL=<ftp地址>
    # export FTP_USER=<用户名:密码>
    file_name=$1.tar.xz
    curl -O $FTP_URL/PluginThirdParty/$file_name --user $FTP_USER
    if [ ! -f $file_name ]; then
        return
    fi
    tar -xf $file_name
    apply_thirdparty_patches
}

function detect_cmake_path() {
    toolchain_dir=$1
    set +e
    cmake_path="-DCMAKE_PREFIX_PATH="
    libs=(
        absl arrow aws blosc boost cppkafka curl fftw hdf5 hiredis jsoncpp krb5 lgbm libarchive libevent librdkafka
        libtorch libzmq lz4 mariadb open62541 openldap orc paho protobuf pulsar sasl2 snappy thrift
        unixODBC wavelib xgboost zlib zstd
    )
    for lib in "${libs[@]}"; do
        cmake_path="$cmake_path;$(ls -d $toolchain_dir/$lib-*)"
    done
    cmake_path="-DOPENSSL_ROOT_DIR=$(ls -d $toolchain_dir/openssl-*) $cmake_path"
    cmake_path="-DMatlab_ROOT_DIR=$(ls -d $toolchain_dir/matlab-*) $cmake_path"
    set -e
}

function prepare_dir() {
    git submodule update --init .
    if [ ! -n "$1" ] && [ -d build ]; then
        return 0;
    fi
    plugin_name=$(basename $(pwd))
    download_thirdparty $plugin_name
    rm -rf build
    build_type=${1:-Debug}
    toolchain_dir=${2:-$HOME/install}
    detect_cmake_path $toolchain_dir
    cmake -B build -DCMAKE_BUILD_TYPE=$build_type $cmake_path ${@:3}
}

function build_plugin() {
    cmake --build build -j --verbose
    cp build/compile_commands.json ..
}

function install_plugin() {
    cmake --install build
    if [ -d third_party/lib ]; then
        cp third_party/lib/* $CMAKE_INSTALL_PREFIX/
    fi
}
