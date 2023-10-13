CC=D:\x86_64-8.1.0-posix-seh-rt_v6-rev0\mingw64\bin\g++
CFLAGS=-DWINDOWS -O2 -std=c++11 -Wall -DLOCKFREE_SYMBASE
XGBOOST_DIR=xgboost_static/lib
INCLUDES=-I../include -Ixgboost_static/include -Iinclude

LIBS=-L../lib $(XGBOOST_DIR)/xgboost.a $(XGBOOST_DIR)/librabit.a $(XGBOOST_DIR)/libdmlc.a

libPluginXgboost.dll: xgboost.o
	$(CC) $^ -fPIC -shared -static-libstdc++ -o $@ $(LIBS) $(VERSION) -L. -lDolphinDB -fopenmp

xgboost.o: src/xgboost.cpp
	$(CC) $^ -c -o $@ $(CFLAGS) $(INCLUDES)

clean:
	del /f/q *.o 2>NUL
	del /f/q libPluginXgboost.dll 2>NUL