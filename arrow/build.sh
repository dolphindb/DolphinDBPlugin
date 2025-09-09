#!/bin/bash

source ../build_util.sh

if [[ "$OSTYPE" != "linux-gnu"* ]]; then
    unset FTP_URL
    prepare_dir $@
    build_plugin
    install_plugin
else 
    prepare_dir $@

    if [ $CXXFLAGS = "-D_GLIBCXX_USE_CXX11_ABI=0" ]; then
        cp arrowThirdParty/ABI0/libarrow.so.900 build/libarrow.so
    else
        cp arrowThirdParty/ABI1/libarrow.so.900 build/libarrow.so
    fi
    build_plugin
    mkdir -p $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
    cp build/libPluginArrow.so $CMAKE_INSTALL_PREFIX/$(basename $(pwd))/
    cp build/PluginArrow.txt $CMAKE_INSTALL_PREFIX/$(basename $(pwd))/
    cp -f build/libarrow.so $CMAKE_INSTALL_PREFIX/$(basename $(pwd))/libarrow.so.900
fi