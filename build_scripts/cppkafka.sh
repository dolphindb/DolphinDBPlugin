#!/bin/bash

librdkafka_dir=$(ls -d $ARTIFACT_DIR/librdkafka-*)
openssl_dir=$(ls -d $ARTIFACT_DIR/openssl-*)
zstd_dir=$(ls -d $ARTIFACT_DIR/zstd-*)
lz4_dir=$(ls -d $ARTIFACT_DIR/lz4-*)
export BOOST=""
if ! echo "int main(){}" | g++ -x c++ -std=c++17 -o /dev/null - >/dev/null 2>&1; then
    export BOOST="-DBOOST_ROOT=$(ls -d /home/api/toolchain/gcc-4.8.5/boost-*)"
fi
cmake_all -DOPENSSL_ROOT_DIR=$openssl_dir -DCMAKE_PREFIX_PATH="$zstd_dir;$lz4_dir" -DRDKAFKA_ROOT=$librdkafka_dir $BOOST
