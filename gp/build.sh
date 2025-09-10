#!/bin/bash

source ../build_util.sh

prepare_dir $@
build_plugin
install_plugin
cp gpThirdParty/* $CMAKE_INSTALL_PREFIX/$(basename $(pwd))/