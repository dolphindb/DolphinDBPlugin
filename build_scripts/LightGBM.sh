#!/bin/bash

rm -rf $ARTIFACT_DIR/lgbm-*

ver=$(echo "$TAG" | sed 's/ddb/lgbm/g')

mkdir build && cd build
cmake_all -DBUILD_STATIC_LIB=OFF
