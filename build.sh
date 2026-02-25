source build_util.sh

git submodule update --init .

set +e
for i in $(ls -d */ | sed 's|/||'); do
    cd $i
    if [ ! -f $i.tar.xz ]; then
        download_thirdparty $i
    fi
    cd ..
done
set -e

unset FTP_URL

prepare_dir $@
cd build
cmake --build . -j$(nproc) --verbose
cd ..
install_plugin
