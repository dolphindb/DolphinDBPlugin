#!/bin/bash

autoreconf -fi
rm -rf build && mkdir build && cd build
../configure $CROSS_HOST --enable-static --disable-shared --with-pic=yes \
    --with-included-ltdl=yes \
    --prefix=$ARTIFACT_DIR/$ver\
    --sysconfdir=/etc/
make -j
make install
