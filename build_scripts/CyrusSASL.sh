#!/bin/bash

prefix=$(echo $TAG | sed 's/ddb-//g')
openssl_dir=$(ls -d $ARTIFACT_DIR/openssl-*)
krb5_dir=$(ls -d $ARTIFACT_DIR/krb5-*)
keyutils_dir=$(dirname $(find $(ls -d $ARTIFACT_DIR/keyutils-*) -name "libkeyutils.a"))

autoreconf -fi
mkdir build && cd build
set_gnu_env
export LIBS="-ldl"
export LDFLAGS="-L$keyutils_dir"
../configure $CROSS_HOST --prefix=$ARTIFACT_DIR/$prefix \
	--enable-static --disable-shared \
    --with-openssl=$openssl_dir \
    --enable-gssapi=$krb5_dir \
    --enable-sample=no \
    --with-saslauthd=no

make
make install

ln -s $ARTIFACT_DIR/$prefix $ARTIFACT_DIR/ABI/$prefix
