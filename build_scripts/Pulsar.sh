#!/bin/bash

if [ $Compiler = "gcc-4.8.5" ]; then
	export CXX_STD="11"
else
	export CXX_STD="17"
fi

rm -rf $ARTIFACT_DIR/pulsar-*
openssl_dir=$(ls -d $ARTIFACT_DIR/openssl-*)
curl_dir=$(ls -d $ARTIFACT_DIR/curl-*)
boost_dir=$(ls -d $ARTIFACT_DIR/boost-*)
zlib_dir=$(ls -d $ARTIFACT_DIR/zlib-*)
protobuf_dir=$(ls -d $ARTIFACT_DIR/protobuf-*)
roaring_dir=$(ls -d $ARTIFACT_DIR/roaring-*)
export PATH="$protobuf_dir/bin:$PATH"

ver=$(echo "$TAG" | sed 's/ddb/pulsar/g')
echo $ver

mkdir build && cd build
cmake_all -DCMAKE_CXX_STANDARD=$CXX_STD \
	-DOPENSSL_ROOT_DIR=$openssl_dir \
    -DCURL_ROOT=$curl_dir \
    -DBOOST_ROOT=$boost_dir \
    -DZLIB_ROOT=$zlib_dir \
	-DProtobuf_ROOT=$protobuf_dir \
    -DCMAKE_PREFIX_PATH=$roaring_dir \
	-DBUILD_TESTS=OFF -DBUILD_DYNAMIC_LIB=OFF

cmake --build . -j$(nproc) --verbose
cmake --install .
