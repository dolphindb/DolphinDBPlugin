#!/bin/bash

make CFLAGS=-fPIC -j$(nproc) -O V=1
make DESTDIR=$ARTIFACT_DIR/$prefix install

find $ARTIFACT_DIR/$prefix -name "libkeyutils.so*" | xargs rm
