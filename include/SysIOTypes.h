#pragma once

#include "SmartPointer.h"

#ifdef __linux__

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

typedef int SOCKET;
#define INVALID_SOCKET -1
#define SOCKET_ERROR   -1

#else

#include <winsock2.h>
#include <windows.h>

#endif

namespace ddb {

class DataInputStream;
class DataOutputStream;
class DataStream;
class Buffer;

typedef SmartPointer<DataInputStream> DataInputStreamSP;
typedef SmartPointer<DataOutputStream> DataOutputStreamSP;
typedef SmartPointer<DataStream> DataStreamSP;
typedef SmartPointer<Buffer> BufferSP;

struct FileAttributes{
	string name;
	bool isDir;
	long long size;
	long long lastModified; //epoch time in milliseconds
	long long lastAccessed; //epoch time in milliseconds
	long long createTime;   //epoch time in milliseconds
};

} // namespace ddb
