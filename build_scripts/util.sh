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
    export CROSS_TOOLCHAIN="--toolchain $HOME/toolchain/cross_toolchain.cmake"
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
    elif [[ "$compiler" == "gcc-8.3.1" ]]; then
        export CC="gcc-8"
        export CXX="g++-8"
    fi
    export TOOLCHAIN_DIR="$HOME/toolchain/$compiler"
    export ARTIFACT_DIR="$HOME/artifact/$compiler"
    export PATH="$TOOLCHAIN_DIR/bin:$PATH"
    export LD_LIBRARY_PATH="$TOOLCHAIN_DIR/lib64"
}

function set_compat()
{
    export ARTIFACT_DIR=$ARTIFACT_DIR/compat
    export CXXFLAGS="-D_GLIBCXX_USE_CXX11_ABI=0 $CXXFLAGS"
}

function cmake_all()
{
    build=build_releas_static
    prefix=$(echo "$TAG" | sed 's/ddb-//g')
    gcc_version=$($CC -dumpversion | cut -d. -f1)
    if [ "$gcc_version" -lt 6 ]; then
        cstd="-DCMAKE_C_STANDARD=11 -DCMAKE_CXX_STANDARD=11"
    elif [ "$gcc_version" -lt 11 ]; then
        cstd="-DCMAKE_CXX_STANDARD=17"
    fi
    rm -rf $build && mkdir $build && cd $build
    cmake .. $CROSS_TOOLCHAIN $cstd \
        -DCMAKE_INSTALL_PREFIX=$ARTIFACT_DIR/$prefix \
        -DCMAKE_BUILD_TYPE=RelWithDebInfo \
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DBUILD_SHARED_LIBS=OFF \
        -DCMAKE_FIND_PACKAGE_PREFER_CONFIG=ON \
        $@
    cmake --build . -j$(nproc) --verbose
    cmake --install .
    cd ..
}

function gnu_all()
{
    build=build_releas_static
    prefix=$(echo "$TAG" | sed 's/ddb-//g')
    export CFLAGS="-fPIC -g -O2 $CFLAGS"
    export CXXFLAGS="-fPIC -g -O2 $CXXFLAGS"
    gcc_version=$($CC -dumpversion | cut -d. -f1)
    if [ "$gcc_version" -lt 6 ]; then
        export CFLAGS="$CFLAGS -std=gnu11"
        export CXXFLAGS="$CXXFLAGS -std=gnu++11"
    elif [ "$gcc_version" -lt 11 ]; then
        export CXXFLAGS="$CXXFLAGS -std=gnu++17"
    fi
    autoreconf -fi
    rm -rf $build && mkdir $build && cd $build
    ../configure $CROSS_HOST --prefix=$ARTIFACT_DIR/$prefix \
        --enable-static --disable-shared \
        $@
    make -j$(nproc) -O V=1
    make install
    cd ..
}

export SOURCE_DIR="$HOME/source"
