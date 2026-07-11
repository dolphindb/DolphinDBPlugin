#!/bin/bash

openssl_dir=$(ls -d $ARTIFACT_DIR/openssl-*)
cmake_all -DOPENSSL_ROOT_DIR=$openssl_dir \
    -DPAHO_BUILD_SHARED=OFF -DPAHO_BUILD_STATIC=ON \
    -DPAHO_ENABLE_TESTING=OFF \
    -DPAHO_WITH_SSL=ON
