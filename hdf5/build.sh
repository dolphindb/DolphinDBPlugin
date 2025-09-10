#!/bin/bash
set -x

rm -rf lib
rm -rf include

source ../build_util.sh

prepare_dir $@

if [[ "$OSTYPE" != "linux-gnu"* ]]; then
    rm -rf lib
    rm -rf include
    mv lib_win lib
    mv include_win include
else 
    rm -rf lib_win
    rm -rf include_win
fi

build_plugin
install_plugin
