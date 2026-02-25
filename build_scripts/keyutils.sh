#!/bin/bash

prefix=$ARTIFACT_DIR/$ver

make
make DESTDIR=$prefix install
find $prefix/ -name "libkeyutils.so*" | xargs rm
