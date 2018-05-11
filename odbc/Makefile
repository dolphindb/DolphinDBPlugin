BITS=64 # machine word size
CFLAGS = -g -Wall -std=c++11 -fPIC -DLINUX -DLOGGING_LEVEL_2 -DLINUX -O0 -DNANODBC_ODBC_VERSION=SQL_OV_ODBC3 
INCLUDES= -I../include -Isrc/
OBJS=DolphinDBODBC.o nanodbc.o 

all: $(OBJS)
	g++ -DBIT$(BITS) $(CFLAGS) -shared -o libODBC.so DolphinDBODBC.o nanodbc.o  -lDolphinDB -lodbc
DolphinDBODBC.o: src/DolphinDBODBC.cpp
	g++ $(CFLAGS) $(INCLUDES) -c src/DolphinDBODBC.cpp -o DolphinDBODBC.o

nanodbc.o: src/nanodbc.cpp
	g++ $(CFLAGS) $(INCLUDES) -c src/nanodbc.cpp -o nanodbc.o
	
clean:
	rm *.o
	rm libPluginODBC.so