#!/bin/bash

source ../build_util.sh


if [[ "$OSTYPE" != "linux-gnu"* ]]; then
    rm -rf lib
    rm -rf include
else 
    unset FTP_URL
fi
prepare_dir $@
build_plugin
install_plugin
