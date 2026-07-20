#!/bin/bash

source ../build_util.sh

if [[ $OSTYPE == linux-gnu* ]]; then
    unset FTP_URL
fi
prepare_dir $@
build_plugin
install_plugin
