#!/bin/bash

function prepare_dir() {
    git submodule update --init .
    if [ ! -n "$1" ] && [ -d build ]; then
        return 0;
    fi
    echo "Configure project."
    rm -rf build
    mkdir build
    cd build
    build_type="Debug"
    if [ -n "$1" ]; then
        build_type=$1
    fi
    cmake .. -DCMAKE_BUILD_TYPE=$build_type
    cd ..
}

function build_plugin() {
    cd build
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
