CC = g++
CFLAGS = -std=c++11 -fPIC -DLINUX -O2 -Wall -Werror -D_GLIBCXX_USE_CXX11_ABI=0
INCLUDES = -I./include -I../include -I/usr/local/include/libbson-1.0 -I/usr/local/include/libmongoc-1.0
LDFLAGS="-Wl,-rpath,."
libPluginMongodb.so: mongodb.o
	$(CC) $(LDFLAGS) -shared -o libPluginMongodb.so mongodb.o -L.   -lDolphinDB  -lmongoc-1.0 -lbson-1.0

mongodb.o: src/mongodb.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c src/mongodb.cpp -o mongodb.o

clean:
	rm -rf *.o
	rm -rf libPluginMongodb.so
