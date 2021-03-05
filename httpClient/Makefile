CC = g++
CFLAGS = -std=c++11 -fPIC -DLINUX -o3 -Wall  -D_GLIBCXX_USE_CXX11_ABI=0
INCLUDES =-I$(CURL_DIR)/include -I$(SSL_DIR)/include -I/$(Z_DIR)/include -I../include 

CURL_LIBS := lib/linux64/libcurl.a
SSL_LIBS := lib/linux64/libssl.a lib/linux64/libcrypto.a
Z_LIBS=lib/linux64/libz.a
VERSION_SCRIPT = -Wl,--version-script=./httpClient.version

libPluginHttpClient.so: httpClient.o email.o session.o
	$(CC) $(CFLAGS) -shared -o libPluginHttpClient.so httpClient.o email.o session.o  $(CURL_LIBS) $(SSL_LIBS) $(Z_LIBS) $(VERSION_SCRIPT) -lDolphinDB

httpClient.o: src/httpClient.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c src/httpClient.cpp -o httpClient.o

email.o: src/email.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c src/email.cpp -o email.o 
session.o: src/session.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c src/session.cpp -o session.o 

clean:
	rm -f *.o
	rm -f *.so
