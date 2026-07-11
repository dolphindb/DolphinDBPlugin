#!/bin/bash

source ../build_util.sh

unset FTP_URL
thrift --gen cpp:no_skeleton Hbase.thrift
prepare_dir $@
build_plugin
install_plugin
