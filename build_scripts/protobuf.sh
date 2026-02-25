#!/bin/bash

zlib_dir=$(ls -d $ARTIFACT_DIR/zlib-*)
absl_dir=$(ls -d $ARTIFACT_DIR/absl-*)/lib64/cmake/absl

rm -rf build && mkdir build && cd build
cmake_all -DZLIB_ROOT=$zlib_dir -Dabsl_DIR=$absl_dir
