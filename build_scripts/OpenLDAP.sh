#!/bin/bash

openssl_dir=$(ls -d $ARTIFACT_DIR/openssl-*)
cyrus_sasl_dir=$(ls -d $ARTIFACT_DIR/cyrus-sasl-*)

export CPPFLAGS="-I$openssl_dir/include -I$cyrus_sasl_dir/include"
export LDFLAGS="-L$openssl_dir/lib64 -L$cyrus_sasl_dir/lib"
export LIBS="-lpthread -ldl"

autoreconf -fi
rm -rf objs && mkdir objs && cd objs
../configure --enable-shared=no --prefix=$ARTIFACT_DIR/$ver \
    --enable-slapd=no \
    --with-pic=yes \
    --with-tls=openssl

make -j
make install
