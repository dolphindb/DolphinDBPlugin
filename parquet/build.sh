source ../build_util.sh

unset FTP_URL
prepare_dir $@ -DArrow_DIR=/home/api/toolchain/gcc-8.4.0/arrow-2.0.0/lib64/cmake/arrow/ -DParquet_DIR=/home/api/toolchain/gcc-8.4.0/arrow-2.0.0/lib64/cmake/arrow/
build_plugin
install_plugin
