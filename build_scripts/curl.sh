#!/bin/bash

openssl_dir=$(ls -d $ARTIFACT_DIR/openssl-*)
cmake_all -DOPENSSL_ROOT_DIR=$openssl_dir
