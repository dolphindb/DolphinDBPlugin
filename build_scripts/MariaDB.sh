#!/bin/bash

openssl_dir=$(ls -d $ARTIFACT_DIR/openssl-*)
zlib_dir=$(ls -d $ARTIFACT_DIR/zlib-*)
zstd_dir=$(ls -d $ARTIFACT_DIR/zstd-*)

cmake_all -DCMAKE_PREFIX_PATH="$zlib_dir;$zstd_dir" \
    -DINSTALL_LIBDIR="lib" \
    -DOPENSSL_ROOT_DIR=$openssl_dir \
    -DWITH_UNIT_TESTS=OFF \
    -DWITH_EXTERNAL_ZLIB=ON
