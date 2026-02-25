#!/bin/bash

openssl_dir=$(ls -d $ARTIFACT_DIR/openssl-*)
sasl_dir=$(ls -d $ARTIFACT_DIR/cyrus-sasl-*)
mkdir objs && cd objs
cmake .. -DCMAKE_INSTALL_PREFIX=$ARTIFACT_DIR/$prefix \
	-DOPENSSL_ROOT_DIR=$openssl_dir \
	-DSASL2_PATHS=$sasl_dir \
    -DBUILD_TESTING=OFF \
    -DENABLE_SHARED=OFF
cmake --build . -j$(nproc) --verbose
cmake --install .
