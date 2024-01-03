rm -rf build/*
cd build
cmake .. -DAMDAPIDIR=/hddtest/data/AMD_ama_gcc-5.4.0_Ubuntu_16.04_V3.9.8.220429-rc5.6_20220527-214132/c++
cp /hddtest/ddb/dolphindb2.00.10_abi/server/libDolphinDB.so ./
make -j
rm -rf ../bin
make install