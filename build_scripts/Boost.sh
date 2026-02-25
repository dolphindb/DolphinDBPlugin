#!/bin/bash

prefix="boost-1.90.0"
src=boost_1_90_0.tar.gz

cp /home/api/src/$src .
tar -xf $src
cd boost_1_90_0
./bootstrap.sh
if [ $CROSS_TOOLCHAIN ];then
	echo "using gcc : arm_64 : $(which $CXX) ;" >> user-config.jam
    config="--user-config=user-config.jam"
fi
libs="--with-headers"
./b2 $libs cxxflags="$CXXFLAGS" -d+2 $config
./b2 install --prefix=$ARTIFACT_DIR/$prefix $libs
