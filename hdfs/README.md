# DolphinDB HDFS Plugin

DolphinDB hdfs plugin can read files in Parquet or ORC format from Hadoop HDFS and write them into DolphinDB in-memory tables. Additionally, it also supports saving DolphinDB in-memory tables to HDFS in specific formats.
For the latest documentation, visit HDFS.

**Required server version**: DolphinDB 2.00.10 or higher

**Supported OS**: Linux x86-64 and Linux JIT

## Build from Source

(1) Configure the environment.

```
# Download Hadoop
https://hadoop.apache.org
# for ubuntu users
sudo apt install cmake
# for Centos users
sudo yum install cmake
```

(2) Compile the plugin.

```
cd hdfs
mkdir build
cd build
cmake .. -DHADOOP_DIR=/path/to/your/hadoop/home
make
```

**Note**: Make sure libDolphinDB.so is under the GCC search path before compilation. You can add the plugin path to the library search path LD_LIBRARY_PATH or copy it to the build directory.