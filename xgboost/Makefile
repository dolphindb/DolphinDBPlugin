CC=g++
CFLAGS=-DWINDOWS -O2 -std=c++11 -Wall
XGBOOST_DIR=./xgboost_static/lib
INCLUDES=-I../include -I./xgboost_static/include -I./include

LIBS=-L../lib $(XGBOOST_DIR)/xgboost.a $(XGBOOST_DIR)/librabit.a $(XGBOOST_DIR)/libdmlc.a

libPluginXgboost.dll: xgboost.o
	$(CC) $^ -fPIC -shared -static-libstdc++ -o $@ $(LIBS) $(VERSION) -lDolphinDB -fopenmp

xgboost.o: src/xgboost.cpp
	$(CC) $^ -c -o $@ $(CFLAGS) $(INCLUDES)

clean:
	rm *.o
	rm libPluginXgboost.dll