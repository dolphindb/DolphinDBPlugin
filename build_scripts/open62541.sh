#!/bin/bash

openssl_dir=$(ls -d $ARTIFACT_DIR/openssl-*)
cmake_all -Wno-dev -DUA_MULTITHREADING=99 -DUA_ENABLE_ENCRYPTION="OPENSSL" -DOPENSSL_ROOT_DIR=$openssl_dir
