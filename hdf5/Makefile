CFLAGS = -g -Wall -std=c++11 -fPIC -DLINUX -DLINUX -O2 -Wall -Werror -DLOCKFREE_SYMBASE
INCLUDES= -I../include -I./include

HDF5_INCLUDES = $(shell  h5c++ -shlib -show|xargs -n1|grep "^\-I")
HDF5_DEFINITIONS = $(shell h5c++ -shlib -show|xargs -n1|grep "^\-D")
HDF5_LIBRARIES_DIRS = $(shell h5c++ -shlib -show|xargs -n1|grep "^\-L")
HDF5_LIBRARIES = $(shell h5c++ -shlib -show|xargs -n1|grep "^\-l")

OBJS=hdf5_plugin.o 

libPluginHdf5.so: hdf5_plugin.o
	g++ $(CFLAGS) $(INCLUDES) $(HDF5_INCLUDES) $(HDF5_DEFINITIONS) $(HDF5_LIBRARIES_DIRS) -shared -o libPluginHdf5.so hdf5_plugin.o -lDolphinDB $(HDF5_LIBRARIES)
hdf5_plugin.o : src/hdf5_plugin.cpp
	g++ $(CFLAGS) $(INCLUDES) $(HDF5_INCLUDES) $(HDF5_DEFINITIONS) -c src/hdf5_plugin.cpp -o hdf5_plugin.o

clean:
	rm *.o
	rm libPluginHdf5.so
