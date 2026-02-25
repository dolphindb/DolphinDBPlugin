#!/bin/bash

function set_cross_compiler()
{
    # for gnu autotools
    export CC="aarch64-linux-gnu-gcc"
    export CXX="aarch64-linux-gnu-g++"
    export AR="aarch64-linux-gnu-gcc-ar"
    export CROSS_HOST="--host=aarch64-linux-gnu"

    # for cmake
    # set(CMAKE_SYSTEM_NAME Linux)
    # set(CMAKE_SYSTEM_PROCESSOR aarch64)
    # set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
    # set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
    export CROSS_TOOLCHAIN="--toolchain /home/api/toolchain/cross_toolchain.cmake"
}

function select_toolchain()
{
    compiler=$1
    export CC=gcc
    export CXX=g++
    export AR=gcc-ar
    export CROSS_COMPILER=""
    export CROSS_HOST=""
    if [[ "$compiler" == "gcc-4.8.5" ]]; then
        export PATH="/usr/bin:$PATH"
    elif [[ "$compiler" == "gcc-linaro-4.9.4-2017.01-x86_64_aarch64-linux-gnu" ]]; then
        set_cross_compiler $compiler
    elif [[ "$compiler" == "gcc-arm-8.3-2019.03-x86_64-aarch64-linux-gnu" ]]; then
        set_cross_compiler $compiler
    elif [[ "$compiler" == "clang-latest" ]]; then
        export CC=clang
        export CXX=clang++
    elif [[ "$compiler" == "BullseyeCoverage-g++-4.8.5" ]]; then
        export PATH=/opt/BullseyeCoverage/bin:$PATH
    elif [[ "$compiler" == "BullseyeCoverage-g++-8" ]]; then
        #ln -s /home/api/toolchain/gcc-8/bin/g++ /opt/BullseyeCoverage/link/g++-8
        #ln -s /home/api/toolchain/gcc-8/bin/gcc /opt/BullseyeCoverage/link/gcc-8
        export PATH=/opt/BullseyeCoverage/link:$PATH
        export PATH=/opt/BullseyeCoverage/bin:$PATH
        export CC=gcc-8
        export CXX=g++-8
    elif [[ "$compiler" == "gcc-8.3.1" ]]; then
        export CC="gcc-8"
        export CXX="g++-8"
    fi
    export TOOLCHAIN_DIR="/home/api/toolchain/$compiler"
    export PATH="$TOOLCHAIN_DIR/bin:$PATH"
    export LD_LIBRARY_PATH="$TOOLCHAIN_DIR/lib64"
    export ARTIFACT_DIR="/home/api/artifact/$compiler"
}

function set_abi()
{
    if [ $1 ]; then
        export ARTIFACT_DIR=$ARTIFACT_DIR/ABI
    else
        export CXXFLAGS="-D_GLIBCXX_USE_CXX11_ABI=0 $CXXFLAGS"
    fi
}

function cmake_all()
{
    rm -rf build && mkdir build && cd build
    gcc_version=$($CC -dumpversion | cut -d. -f1)
    if [ "$gcc_version" -lt 7 ]; then
        cstd="-DCMAKE_C_STANDARD=11 -DCMAKE_CXX_STANDARD=11"
    elif [ "$gcc_version" -lt 10 ]; then
        cstd="-DCMAKE_CXX_STANDARD=17"
    elif [ "$gcc_version" -lt 16 ]; then
        cstd="-DCMAKE_CXX_STANDARD=20"
    fi
    prefix=$(echo "$TAG" | sed 's/ddb-//g')
    cmake .. $CROSS_TOOLCHAIN $cstd \
        -DCMAKE_BUILD_TYPE=RelWithDebInfo \
        -DCMAKE_INSTALL_PREFIX=$ARTIFACT_DIR/$ver \
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
        $@
    cmake --build . -j$(nproc) --verbose
    cmake --install .
    cd ..
}

function set_gnu_env()
{
    export CFLAGS="-fPIC"
    export CXXFLAGS="-fPIC"
    gcc_version=$($CC -dumpversion | cut -d. -f1)
    if [ "$gcc_version" -lt 7 ]; then
        $CFLAGS="$CFLAGS -std=gnu11"
        $CXXFLAGS="$CXXFLAGS -std=gnu++11"
    elif [ "$gcc_version" -lt 10 ]; then
        $CXXFLAGS="$CXXFLAGS -std=gnu++17"
    elif [ "$gcc_version" -lt 16 ]; then
        $CXXFLAGS="$CXXFLAGS -std=gnu++20"
    fi
}

# typical build commands for cmake projects:
#
# select_toolchain $Compiler
# # if this is a c++ project
# # set_abi $DefaultABI
# cmake_all
# # if this is a c project
# # ln -s $ARTIFACT_DIR/$prefix $ARTIFACT_DIR/ABI/$prefix

# typical build commands for gnu projects:
#
# select_toolchain $Compiler
# set_abi $DefaultABI
# set_gnu_env
# make_all
