#!/bin/bash

unset FTP_URL
source ../build_util.sh

prepare_dir $@
build_plugin
install_plugin
