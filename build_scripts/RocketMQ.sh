#!/bin/bash

openssl_dir=$(ls -d $ARTIFACT_DIR/openssl-*)
boost_dir=$(ls -d $ARTIFACT_DIR/old-boost-*)
libevent_dir=$(ls -d $ARTIFACT_DIR/libevent-*)
jsoncpp_dir=$(ls -d $ARTIFACT_DIR/jsoncpp-*)
mkdir build && cd build
cmake_all -DBUILD_ROCKETMQ_SHARED=OFF \
    -DOPENSSL_ROOT_DIR=$openssl_dir \
	-DCMAKE_PREFIX_PATH="$boost_dir;$libevent_dir;$jsoncpp_dir"
