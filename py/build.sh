#!/bin/bash
export PATH="/home/api/toolchain/gcc-8/bin/:$PATH"

source ../build_util.sh
source $(dirname $(which conda))/../etc/profile.d/conda.sh

function build_plugin_python() {
    rm -rf build
    prepare_dir "$1"
    mkdir -p $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
    curl $FTP_URL_TMP/origin/release200/Release/libopenblas.so.0 --user $FTP_USER_TMP -o "$CMAKE_INSTALL_PREFIX/$(basename $(pwd))/libopenblas.so"
    curl $FTP_URL_TMP/origin/release200/Release/libgfortran.so.3 --user $FTP_USER_TMP -o "$CMAKE_INSTALL_PREFIX/$(basename $(pwd))/libgfortran.so.3"
    build_plugin
    install_plugin
    mv "$CMAKE_INSTALL_PREFIX/$(basename $(pwd))/libopenblas.so" "$CMAKE_INSTALL_PREFIX/$(basename $(pwd))/libopenblas.so.0"
}

install_prefix="/usr/local"
if [ ! -z $CMAKE_INSTALL_PREFIX ]; then
    install_prefix=$CMAKE_INSTALL_PREFIX/$(basename $(pwd)) # xxxx/py
fi

# download dolphindb libopenblas.so.0 and libgfortran.so.3
export FTP_URL_TMP="$FTP_URL"
export FTP_USER_TMP="$FTP_USER"
unset FTP_URL
unset FTP_USER

for i in `seq 6 7`; do
    conda activate py3$i
    export CMAKE_INSTALL_PREFIX=$install_prefix/py3$i
    build_plugin_python $1
    cp -r $CMAKE_INSTALL_PREFIX/py/* $CMAKE_INSTALL_PREFIX/
    rm -rf $CMAKE_INSTALL_PREFIX/py
    mv $CMAKE_INSTALL_PREFIX/PluginPy.txt $CMAKE_INSTALL_PREFIX/PluginPy3$i.txt
done

rm -rf $install_prefix/py
cp -r $install_prefix/py36 $install_prefix/py
mv $install_prefix/py/PluginPy36.txt $install_prefix/py/PluginPy.txt
