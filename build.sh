source build_util.sh

for i in AMDHistory MDL XTP; do
    cd $i
    if [ ! -f $i.tar.xz ]; then
        download_thirdparty $i
    fi
    cd ..
done

unset FTP_URL

prepare_dir $@
cd build
cmake --build . -j$(nproc) --verbose
cd ..
install_plugin
