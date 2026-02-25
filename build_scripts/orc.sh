#!/bin/bash

# https://downloads.apache.org/orc/orc-format-1.1.1/orc-format-1.1.1.tar.gz
export ORC_FORMAT_URL=$SOURCE_DIR/orc-format-1.1.1.tar.gz
zlib_dir=$(ls -d $ARTIFACT_DIR/zlib-*)
zstd_dir=$(ls -d $ARTIFACT_DIR/zstd-*)
lz4_dir=$(ls -d $ARTIFACT_DIR/lz4-*)
snappy_dir=$(ls -d $ARTIFACT_DIR/snappy-*)
protobuf_dir=$(ls -d $ARTIFACT_DIR/protobuf-*)

cmake_all -DBUILD_CPP_TESTS=OFF \
    -DBUILD_JAVA=OFF \
    -DBUILD_TOOLS=OFF \
    -DZLIB_HOME=$zlib_dir \
    -DZSTD_HOME=$zstd_dir \
    -DLZ4_HOME=$lz4_dir \
    -DSNAPPY_HOME=$snappy_dir \
    -DPROTOBUF_HOME=$protobuf_dir \
    -DCMAKE_PREFIX_PATH=$protobuf_dir
