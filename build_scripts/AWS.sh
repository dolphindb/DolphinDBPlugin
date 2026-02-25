#!/bin/bash

zlib_dir=$(ls -d $ARTIFACT_DIR/zlib-*)
curl_dir=$(ls -d $ARTIFACT_DIR/curl-*)
openssl_dir=$(ls -d $ARTIFACT_DIR/openssl-*)

cmake_all -DBUILD_ONLY="S3" \
    -DENABLE_TESTING=OFF \
    -DBUILD_SHARED_LIBS=OFF \
    -DFORCE_SHARED_CRT=OFF \
    -DBUILD_ONLY="s3" \
    -DZLIB_ROOT=$zlib_dir -DCURL_ROOT=$curl_dir -DOPENSSL_ROOT_DIR=$openssl_dir
