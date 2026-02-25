#!/bin/bash

openssl_dir=$(ls -d $ARTIFACT_DIR/openssl-*)

sed -i 's/cmake_minimum_required(VERSION 3.1 FATAL_ERROR)/cmake_minimum_required(VERSION 3.10 FATAL_ERROR)/g' CMakeLists.txt
cmake_all -DOPENSSL_ROOT_DIR=$openssl_dir \
    -DEVENT__LIBRARY_TYPE=STATIC \
    -DEVENT__DISABLE_SAMPLES=ON \
    -DEVENT__DISABLE_TESTS=ON
