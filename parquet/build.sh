#!/bin/bash

source ../build_util.sh

unset FTP_URL
prepare_dir $@
build_plugin
install_plugin
