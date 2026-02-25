#!/bin/bash

cd src
autoreconf -fi
mkdir build && cd build

../configure $cross --prefix=$ARTIFACT_DIR/$ver \
	--enable-static --disable-shared

make -j$(nproc) -O V=1
make install

ln -s $ARTIFACT_DIR/$ver $ARTIFACT_DIR/ABI/$ver
cd $ARTIFACT_DIR/$ver/lib

gcc -r -fPIC -shared -o krb5_combined.o --whole-archive libgssapi_krb5.a libk5crypto.a libkrb5support.a libkrb5.a libcom_err.a
ar rcs libkrb5_combined.a krb5_combined.o
