#!/bin/bash

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
    git submodule update --init .
    set +e
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
        shift 1
    fi
    if [ -n "$1" ]; then
        toolchain_dir=$1
        shift 1
        cmake_path="-DCMAKE_PREFIX_PATH=$ARTIFACT_DIR"
        cmake_path="$cmake_path;$(ls -d $toolchain_dir/absl-*)"
        cmake_path="$cmake_path;$(ls -d $toolchain_dir/arrow-*)"
        cmake_path="$cmake_path;$(ls -d $toolchain_dir/aws-*)"
        cmake_path="$cmake_path;$(ls -d $toolchain_dir/blosc-*)"
        cmake_path="$cmake_path;$(ls -d $toolchain_dir/boost-*)"
        cmake_path="$cmake_path;$(ls -d $toolchain_dir/cppkafka-*)"
        cmake_path="$cmake_path;$(ls -d $toolchain_dir/curl-*)"
        cmake_path="$cmake_path;$(ls -d $toolchain_dir/fftw-*)"
        cmake_path="$cmake_path;$(ls -d $toolchain_dir/hiredis-*)"
        cmake_path="$cmake_path;$(ls -d $toolchain_dir/jsoncpp-*)"
        cmake_path="$cmake_path;$(ls -d $toolchain_dir/krb5-*)"
        cmake_path="$cmake_path;$(ls -d $toolchain_dir/lgbm-*)"
        cmake_path="$cmake_path;$(ls -d $toolchain_dir/libevent-*)"
        cmake_path="$cmake_path;$(ls -d $toolchain_dir/librdkafka-*)"
        cmake_path="$cmake_path;$(ls -d $toolchain_dir/libzmq-*)"
        cmake_path="$cmake_path;$(ls -d $toolchain_dir/lz4-*)"
        cmake_path="$cmake_path;$(ls -d $toolchain_dir/mariadb-*)"
        cmake_path="$cmake_path;$(ls -d $toolchain_dir/mbedtls-*)"
        cmake_path="$cmake_path;$(ls -d $toolchain_dir/open62541-*)"
        cmake_path="$cmake_path;$(ls -d $toolchain_dir/openldap-*)"
        cmake_path="$cmake_path;$(ls -d $toolchain_dir/protobuf-*)"
        cmake_path="$cmake_path;$(ls -d $toolchain_dir/pulsar-*)"
        cmake_path="$cmake_path;$(ls -d $toolchain_dir/quantlib-*)"
        cmake_path="$cmake_path;$(ls -d $toolchain_dir/rocketmq-*)"
        cmake_path="$cmake_path;$(ls -d $toolchain_dir/sasl2-*)"
        cmake_path="$cmake_path;$(ls -d $toolchain_dir/thrift-*)"
        cmake_path="$cmake_path;$(ls -d $toolchain_dir/unixODBC-*)"
        cmake_path="$cmake_path;$(ls -d $toolchain_dir/wavelib-*)"
        cmake_path="$cmake_path;$(ls -d $toolchain_dir/zstd-*)"
        cmake_path="-DOPENSSL_ROOT_DIR=$(ls -d $toolchain_dir/openssl-*) $cmake_path"
        cmake_path="-DZLIB_ROOT=$(ls -d $toolchain_dir/zlib-*) $cmake_path"
        cmake_path="-DOLD_BOOST=$(ls -d $toolchain_dir/old-boost-*) $cmake_path"
    fi
    set -e
    cmake .. -DCMAKE_BUILD_TYPE=$build_type $cmake_path $library_path $@
    cd ..
}

function build_plugin() {
    cd build
    set -e
    cmake --build . -j --verbose
    cp compile_commands.json ../..
    cd ..
}

version_greater_equal() {
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
