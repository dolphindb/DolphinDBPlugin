# DolphinDB Kafka Plugin

DolphinDB kafka plugin can be used to publish or subscribe to Kafka streaming services.
For the latest documentation, visit Kafka.

**Required server version**: DolphinDB 2.00.10 or higher

**Supported OS**: Linux x86-64 and Linux ARM

## Build from Source

(1) Install CMake. For Ubuntu users (replace apt with yum if you use Centos):

```
sudo apt install cmake
```

(2) The project depends on 'cppkafka', which depends on 'boost' and 'librdkafka'. Download with the following script:

```
# The ubuntu which is a low version such as 14.04 will not
# find rdkafka, and you need to compile the librdkafka manully.
# The address is https://github.com/edenhill/librdkafka
# For ubuntu install
sudo apt install librdkafka-dev
sudo apt install libboost-dev
sudo apt install libssl-dev
# For Centos install
sudo yum install librdkafka-devel
sudo yum install boost-devel
sudo yum install openssl-devel
cd /path/to/the/main/project/
git submodule update --init --recursive
```

(3) If it is too slow to download submodule, you can download it with cppkafka.git from the hidden file .gitModules.

```
git clone https://github.com/mfontanini/cppkafka.git
```

(4) Copy libDolphinDB.so to bin/linux64 or /lib.

```
cp /path/to/dolphindb/server/libDolphinDB.so /path/to/kafka/bin/linux64
```

(5) Build the project with CMake.

```
cd /path/to/DolphinDBPlugin/kafka
cd cppkafka
mkdir build
cd build
cmake ..
make
sudo make install
cd ../..
mkdir build
# copy the libDolphinDB.so to ./build
cp /path/to/dolphindb/server/libDolphinDB.so ./build
cd build
cmake ..
make
```

**Note**: Make sure libDolphinDB.so is under the GCC search path before compilation. You can add the plugin path to the library search path LD_LIBRARY_PATH or copy it to the build directory.