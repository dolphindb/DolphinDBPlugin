#!/bin/bash

cd cpp

# parquet requires arrow, boost, thrift, xsimd

boost_dir=$(ls -d $(ls -d $ARTIFACT_DIR/boost-*)/lib/cmake/Boost-*)
xsimd_dir=$(ls -d $ARTIFACT_DIR/xsimd-*)
zlib_dir=$(ls -d $ARTIFACT_DIR/zlib-*)
lz4_dir=$(ls -d $ARTIFACT_DIR/lz4-*)
zstd_dir=$(ls -d $ARTIFACT_DIR/zstd-*)
thrift_dir=$(ls -d $ARTIFACT_DIR/thrift-*)
snappy_dir=$(ls -d $ARTIFACT_DIR/snappy-*)

cmake_all -DARROW_BUILD_SHARED=OFF \
	-DARROW_PARQUET=ON \
    -DARROW_RUNTIME_SIMD_LEVEL=AVX2 \
    -DARROW_MIMALLOC=OFF -DARROW_JEMALLOC=OFF \
    -DARROW_DEPENDENCY_SOURCE=SYSTEM \
    -DARROW_WITH_ZSTD=ON -DARROW_ZSTD_USE_SHARED=OFF \
    -DARROW_WITH_LZ4=ON -DARROW_WITH_ZLIB=ON -DARROW_WITH_SNAPPY=ON \
    -DCMAKE_PREFIX_PATH="$zlib_dir;$lz4_dir;$zstd_dir;$boost_dir;$thrift_dir;$xsimd_dir;$snappy_dir"
