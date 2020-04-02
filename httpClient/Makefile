CC = g++
CFLAGS = -g -std=c++11 -fPIC -DLINUX -O2 -Wall -Werror
INCLUDES = -I./include -I../include -I$(CURL_DIR)/include

CURL_LIBS := $(CURL_DIR)/lib/libcurl.a
SSL_LIBS := $(SSL_DIR)/lib/libssl.a $(SSL_DIR)/lib/libcrypto.a
Z_LIBS := $(Z_DIR)/lib/libz.a
VERSION_SCRIPT = -Wl,--version-script=./httpClient.version

libPluginHttpClient.so: httpClient.o
	$(CC) $(CFLAGS) -shared -o libPluginHttpClient.so httpClient.o $(CURL_LIBS) $(SSL_LIBS) $(Z_LIBS) $(VERSION_SCRIPT) -lDolphinDB

httpClient.o: src/httpClient.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c src/httpClient.cpp -o httpClient.o

clean:
	rm *.o
	rm *.so
