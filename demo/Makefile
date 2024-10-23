all: src/Demo.cpp src/Demo.h
	g++ -DLINUX -fPIC -std=c++11 -D_GLIBCXX_USE_CXX11_ABI=0 -DLOCKFREE_SYMBASE -c src/Demo.cpp -I../include -o Demo.o
	g++ -fPIC -shared -o libPluginDemo.so Demo.o

clean:
	rm Demo.o
	rm libPluginDemo.so