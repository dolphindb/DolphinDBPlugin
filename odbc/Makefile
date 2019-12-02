BITS=64 # machine word size
CFLAGS = -g -Wall -std=c++11 -fPIC -DLINUX -DLOGGING_LEVEL_2 -DLINUX -O0 -DNANODBC_ODBC_VERSION=SQL_OV_ODBC3 
INCLUDES= -I../include -Isrc/
OBJS=DolphinDBODBC.o nanodbc.o cvt.o

all: $(OBJS)
	g++ -DBIT$(BITS) $(CFLAGS) -shared -o libPluginODBC.so DolphinDBODBC.o nanodbc.o cvt.o -lDolphinDB -lodbc

DolphinDBODBC.o: src/DolphinDBODBC.cpp
	g++ $(CFLAGS) $(INCLUDES) -c src/DolphinDBODBC.cpp -o DolphinDBODBC.o

cvt.o: src/cvt.cpp
	g++ $(CFLAGS) $(INCLUDES) -c src/cvt.cpp -o cvt.o

nanodbc.o: src/nanodbc.cpp
	g++ $(CFLAGS) $(INCLUDES) -c src/nanodbc.cpp -o nanodbc.o

HashmapUtil.o: src/HashmapUtil.cpp
	g++ $(CFLAGS) $(INCLUDES) -c src/HashmapUtil.cpp -o HashmapUtil.o

clean:
	rm *.o
	rm libPluginODBC.so
