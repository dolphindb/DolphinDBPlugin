BITS=64 # machine word size
CFLAGS = -Wall -std=c++11 -fPIC -DLINUX -DLOGGING_LEVEL_2 -DLINUX -O3 -DLOCKFREE_SYMBASE
INCLUDES= -I../include/
OBJS=ZlibImpl.o 
LINK_FLAGS="-Wl,-rpath,../../.,-rpath,."
libPluginZLib.so: $(OBJS)
	g++ -DBIT$(BITS) $(CFLAGS) $(LINK_FLAGS) -shared  -o libPluginZlib.so ZlibImpl.o -L. -lz #-Wl,-Bdynamic -lDolphinDB


ZlibImpl.o:./src/ZlibImpl.cpp
	g++ -DLINUX -std=c++11 -fPIC -c ./src/ZlibImpl.cpp $(INCLUDES)


clean:
	rm -f *.o
	rm -f *.so
