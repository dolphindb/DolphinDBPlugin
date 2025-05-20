#!/bin/bash
set -e

if [[ "$GCC_VERSION" == "8.4.0" ]]; then
    export CC=/opt/gcc/840/bin/gcc
    export CXX=/opt/gcc840/bin/g++
fi

amd_396_dir="/hdd/plugins/AMD_ama_hidden_gcc-4.8.5_RedHat-7.6_V3.9.6.220128-rc7.0_20220216-125729/c++/"
amd_401_dir="/hdd/plugins/AMD_ama_gcc-4.8.5_RedHat-7.6_V4.0.1.220812-rc4.0_20220812-100704/c++/"
amd_420_dir="/hdd/plugins/AMD_ama_gcc-4.8.5_RedHat-7.6_V4.2.0.221223-rc5.1_20230201-184629/c++/"
amd_430_dir="/hdd/plugins/AMD_ama_gcc-4.8.5_RedHat-7.6_V4.3.0.230331-rc6.4_20230904-154106/c++/"
amd_398_ABI_dir="/hdd/plugins/AMD_ama_gcc-5.4.0_Ubuntu_16.04_V3.9.8.220429-rc5.6_20220527-214132/c++/"

build_amdQuote() {
    local workDir="$1/amdQuote/"
    local amdVersion="$2"
    local pluginName="$3"
    local pluginTxtName="$4"
    local amdDir="$5"
    local useAsan="$6"
    local cov="$7"

    local asan_param=""
    local build_type=""
    local currentOutputDir="$workDir/output/$pluginName"

    if [[ "$useAsan" == "asan" ]]; then
        asan_param="-DDDB_USE_ASAN=ON"
        build_type="-DCMAKE_BUILD_TYPE=DEBUG"
        # currentOutputDir="$currentOutputDir/asan/"
    fi

    # if [[ "$cov" == "cov" ]]; then
        # currentOutputDir="$currentOutputDir/cov/"
    # fi

    cd $workDir

    rm -rf "$currentOutputDir"
    mkdir -p "$currentOutputDir"
    rm -rf build
    mkdir -p build

    cd build
    cmake .. -DAMD_VERSION="$amdVersion" -DAMDAPIDIR="$amdDir" $asan_param $build_type
    make -j

    cp -f libPluginAmdQuote.so "$currentOutputDir"
    cp -f PluginAmdQuote.txt "$currentOutputDir"
    cp -f PluginAmdQuote.txt "$currentOutputDir/$pluginTxtName"
    cp -f $amdDir/lib/*.so "$currentOutputDir"
    cp -f $amdDir/lib/*.so.* "$currentOutputDir"

    cd $currentOutputDir
    objcopy --only-keep-debug libPluginAmdQuote.so libPluginAmdQuote.so.debug
    objcopy --strip-unneeded libPluginAmdQuote.so
    if [[ "$cov" == "cov" ]]; then
        cp $1/test.cov ./
    fi
    cd $workDir
}

build_all() {
    local asan=$1
    local cov=$2
    local workDir=$(pwd)/../

    build_amdQuote $workDir 3.9.6 amdQuote396 PluginAmdQuote396.txt $amd_396_dir $asan $cov
    build_amdQuote $workDir 3.9.8 amdQuote398 PluginAmdQuote398.txt $(pwd) $asan $cov
    build_amdQuote $workDir 4.0.1 amdQuote401 PluginAmdQuote401.txt $amd_401_dir $asan $cov
    build_amdQuote $workDir 4.2.0 amdQuote420 PluginAmdQuote420.txt $amd_420_dir $asan $cov
    build_amdQuote $workDir 4.3.0 amdQuote430 PluginAmdQuote430.txt $amd_430_dir $asan $cov
    # build_amdQuote $workDir 3.9.8 ABI/amdQuote398 PluginAmdQuote398.txt $amd_398_ABI_dir $asan $cov
}

if [[ "$1" == "Asan" ]]; then
    build_all "asan" 
elif [[ "$1" == "Gcov" ]]; then
    build_all "mock" "cov"
else
    build_all
fi
mkdir -p $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp -r output/* $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
