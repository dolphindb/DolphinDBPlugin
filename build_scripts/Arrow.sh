#!/bin/bash

cd cpp
SRC_DIR=/home/api/src

# arrow requires mimalloc
# parquet requires arrow, boost, rapidjson, thrift, xsimd

boost_dir=$(ls -d $(ls -d $ARTIFACT_DIR/boost-*)/lib/cmake/Boost-*)
zlib_dir=$(ls -d $(ls -d $ARTIFACT_DIR/zlib-*))
lz4_dir=$(ls -d $(ls -d $ARTIFACT_DIR/lz4-*))
zstd_dir=$(ls -d $(ls -d $ARTIFACT_DIR/zstd-*))

# https://github.com/microsoft/mimalloc/tags
export ARROW_MIMALLOC_URL=$SRC_DIR/mimalloc-3.1.5.tar.gz
# https://github.com/Tencent/rapidjson
export ARROW_RAPIDJSON_URL=$SRC_DIR/rapidjson-232389d4f1012dddec4ef84861face2d2ba85709.tar.gz
# https://thrift.apache.org/
export ARROW_THRIFT_URL=$SRC_DIR/thrift-0.22.0.tar.gz
# https://github.com/xtensor-stack/xsimd/tags
export ARROW_XSIMD_URL=$SRC_DIR/xsimd-13.0.0.tar.gz
# https://sourceware.org/pub/bzip2/
export ARROW_BZIP2_URL=$SRC_DIR/bzip2-1.0.8.tar.gz
# https://github.com/google/snappy/releases
export ARROW_SNAPPY_URL=$SRC_DIR/snappy-1.2.2.tar.gz
cmake_all -DARROW_BUILD_SHARED=OFF \
	-DARROW_PARQUET=ON \
    -DARROW_DEPENDENCY_SOURCE=SYSTEM \
    -DSnappy_SOURCE=BUNDLED \
    -DThrift_SOURCE=BUNDLED \
    -DRapidJSON_SOURCE=BUNDLED \
    -Dxsimd_SOURCE=BUNDLED \
    -DBZip2_SOURCE=BUNDLED \
    -DARROW_WITH_BZ2=ON \
    -DARROW_WITH_LZ4=ON \
    -DARROW_WITH_SNAPPY=ON \
    -DARROW_WITH_ZLIB=ON \
    -DARROW_WITH_ZSTD=ON \
    -DCMAKE_PREFIX_PATH="$zlib_dir;$lz4_dir;$zstd_dir;$boost_dir"
