#!/bin/bash

openssl_dir=$(ls -d $ARTIFACT_DIR/openssl-*)
cyrus_sasl_dir=$(ls -d $ARTIFACT_DIR/cyrus-sasl-*)
krb5_dir=$(ls -d $ARTIFACT_DIR/krb5-*)
zstd_dir=$(ls -d $ARTIFACT_DIR/zstd-*)
lz4_dir=$(ls -d $ARTIFACT_DIR/lz4-*)
cmake_all -DRDKAFKA_BUILD_EXAMPLES=OFF \
    -DRDKAFKA_BUILD_TESTS=OFF \
    -DRDKAFKA_BUILD_STATIC=ON \
    -DOPENSSL_ROOT_DIR=$openssl_dir \
    -DCMAKE_PREFIX_PATH="$cyrus_sasl_dir;$zstd_dir;$lz4_dir" \
    -DWITH_SASL=ON -DWITH_SSL=ON -DWITH_ZSTD=ON
