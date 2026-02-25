#!/bin/bash

mbedtls_dir=$(ls -d $ARTIFACT_DIR/mbedtls-*)
cmake_all -Wno-dev \
    -DUA_ENABLE_ENCRYPTION=ON \
    -DMbedTLS_ROOT=$mbedtls_dir
