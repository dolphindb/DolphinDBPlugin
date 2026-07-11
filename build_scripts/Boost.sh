#!/bin/bash

prefix="boost-1.91.0"
filename=$(echo "$prefix" | sed 's/-/_/g; s/\./_/g')
cp $SOURCE_DIR/$filename.tar.gz .
rm -rf $filename && tar -xf $filename.tar.gz && cd $filename
./bootstrap.sh
if [ -n "$CROSS_TOOLCHAIN" ];then
	echo "using gcc : arm_64 : $(which $CXX) ;" >> user-config.jam
    config="--user-config=user-config.jam"
fi
libs="--with-headers"
./b2 $libs cxxflags="$CXXFLAGS" -d+2 $config
./b2 install --prefix=$ARTIFACT_DIR/$prefix $libs
