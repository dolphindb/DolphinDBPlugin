#!/bin/bash

openssl_dir=$(ls -d $ARTIFACT_DIR/openssl-*)
krb5_dir=$(ls -d $ARTIFACT_DIR/krb5-*)
keyutils_dir=$(ls -d $ARTIFACT_DIR/keyutils-*)

keyutils_lib_dir=$(dirname $(find $keyutils_dir -name "libkeyutils.a"))
export LIBS="-ldl"
export LDFLAGS="-L$keyutils_lib_dir"
autoreconf -fi
gnu_all --with-openssl=$openssl_dir \
    --enable-gssapi=$krb5_dir \
    --enable-sample=no \
    --with-saslauthd=no
