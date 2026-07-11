source ../build_util.sh

prepare_dir $@
build_plugin
install_plugin

cp third_party/lib/win64/*.dll $CMAKE_INSTALL_PREFIX/nsq/
