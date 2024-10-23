rm -rf build
mkdir build                                                        # 新建build目录
cp /c/dolphindb2.00.8/server/libDolphinDB.dll build                 # 拷贝 libDolphinDB.dll 到build目录下
cp -r /d/curl build/                                                  # 拷贝 curl 头文件到build目录下
cd build
# cmake -DCMAKE_BUILD_TYPE=Release .. -G "MinGW Makefiles"
cmake .. -G "MinGW Makefiles"
mingw32-make -j4
# cp libPluginMySQL.dll /c/DDB1.20.25/server/plugins/mysql/
# cp libPluginMySQL.dll /c/DDB2.00.6/server/plugins/mysql/