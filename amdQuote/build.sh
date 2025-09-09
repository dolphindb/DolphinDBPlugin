#!/bin/bash
set -e

amd_396_dir="/hdd/plugins/AMD_ama_hidden_gcc-4.8.5_RedHat-7.6_V3.9.6.220128-rc7.0_20220216-125729/c++/"
amd_398_dir="/hdd/plugins/AMD_ama_gcc-4.8.5_RedHat-7.6_V3.9.8.220429-rc5.0_20220429-182103/c++/"
amd_401_dir="/hdd/plugins/AMD_ama_gcc-4.8.5_RedHat-7.6_V4.0.1.220812-rc4.0_20220812-100704/c++/"
amd_420_dir="/hdd/plugins/AMD_ama_gcc-4.8.5_RedHat-7.6_V4.2.0.221223-rc5.1_20230201-184629/c++/"
amd_430_dir="/hdd/plugins/AMD_ama_gcc-4.8.5_RedHat-7.6_V4.3.0.230331-rc6.4_20230904-154106/c++/"
amd_398_ABI_dir="/hdd/plugins/AMD_ama_gcc-5.4.0_Ubuntu_16.04_V3.9.8.220429-rc5.6_20220527-214132/c++/"
amd_457_dir="/hdd/plugins/AMD_ama-with-query_gcc-4.8.5_RedHat-7.6_V4.5.7.240930-rc3.7_20241226-235039/c++/"


function build_amd() {
    set -e
    amd_sdk_dir=$1
    amd_version=$2

    plugin_name=$(basename $(pwd))
    echo "Configure project $plugin_name."

    # cmake
    rm -rf build
    mkdir build
    cd build
    build_type="Debug"
    if [ -n "$3" ]; then
        build_type=$3
    fi
    cmake .. -DCMAKE_BUILD_TYPE=$build_type $toolchain_arg -DCMAKE_LIBRARY_PATH="$library_path" $sdk_version -DAMD_SDK_DIR=$amd_sdk_dir -DAMD_VERSION=$amd_version

    # build
    cmake --build . -j --verbose

    # install
    ver=$(cmake --version | head -n 1 | cut -d ' ' -f3)
    echo $ver
    if version_greater_equal $ver "3.29.0" ; then
        cmake --install .
    else
        cmake --install . --prefix $CMAKE_INSTALL_PREFIX
    fi
    cd ..
    mv $CMAKE_INSTALL_PREFIX/amdQuote $CMAKE_INSTALL_PREFIX/amdQuote$amd_version
}

if [[ "$CXXFLAGS" == "-D_GLIBCXX_USE_CXX11_ABI=0" ]];then
    build_amd $amd_396_dir "396"
    build_amd $amd_398_dir "398"
    build_amd $amd_401_dir "401"
    build_amd $amd_420_dir "420"
    build_amd $amd_430_dir "430"
    build_amd $amd_457_dir "457"
else
    build_amd $amd_398_ABI_dir "398"
fi