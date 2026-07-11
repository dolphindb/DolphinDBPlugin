#!/bin/bash

source ../build_util.sh
set +x
echo "command: source conda.sh"
source $(dirname $(which conda))/../etc/profile.d/conda.sh
echo "finish : source conda.sh"
set -x

function build_plugin_python() {
    rm -rf build
    prepare_dir $@
    build_plugin
    install_plugin
}

unset FTP_URL

for i in `seq 9 13`; do
    set +x
    echo "command: conda activate py3$i"
    conda activate py3$i
    echo "finish : conda activate py3$i"
    set -x
    build_plugin_python $1
    mv $CMAKE_INSTALL_PREFIX/python $CMAKE_INSTALL_PREFIX/py3$i
    mv $CMAKE_INSTALL_PREFIX/py3$i/PluginPython.txt $CMAKE_INSTALL_PREFIX/py3$i/PluginPy3$i.txt
done
