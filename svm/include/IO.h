/*
 * Socket.h
 *
 *  Created on: Mar 14, 2015
 *      Author: dzhou
 */

#ifndef SOCKET_H_
#define SOCKET_H_

#include <string>
#include <iostream>
#include "SmartPointer.h"
#include "Types.h"

#define MAX_CAPACITY 65536
#define MAX_PACKET_SIZE 1024

#ifdef LINUX
	#include <netinet/in.h>
    #include <sys/socket.h>
	typedef int SOCKET;
	#define INVALID_SOCKET -1
	#define SOCKET_ERROR   -1
#else
	#include <winsock2.h>
	#include<windows.h>
#endif

using std::string;

class Constant;
class Socket;
class UdpSocket;
class DataInputStream;
class DataOutputStream;
class DataStream;
typedef SmartPointer<Socket> SocketSP;
typedef SmartPointer<UdpSocket> UdpSocketSP;
typedef SmartPointer<DataInputStream> DataInputStreamSP;
typedef SmartPointer<DataOutputStream> DataOutputStreamSP;
typedef SmartPointer<DataStream> DataStreamSP;

class Socket{
public:
	Socket();
	Socket(const string& host, int port, bool blocking);
	Socket(SOCKET handle, bool blocking);
	~Socket();
	const string& getHost() const {return host_;}
	int getPort() const {return port_;}
	IO_ERR read(char* buffer, size_t length, size_t& actualLength);
	IO_ERR write(const char* buffer, size_t length, size_t& actualLength);
	IO_ERR bind();
	IO_ERR listen();
	IO_ERR connect(const string& host, int port, bool blocking);
	IO_ERR connect();
	IO_ERR close();
	Socket* accept();
	SOCKET getHandle();
	bool isBlockingMode() const {return blocking_;}
	bool isValid();
	void setAutoClose(bool option) { autoClose_ = option;}

private:
	bool setNonBlocking();
	int getErrorCode();

private:
	string host_;
	int port_;
	SOCKET handle_;
	bool blocking_;
	bool autoClose_;
};

class UdpSocket{
public:
	UdpSocket(int port);
	UdpSocket(const string& remoteHost, int remotePort);
	~UdpSocket();
	int getPort() const {return port_;}
	IO_ERR send(const char* buffer, size_t length);
	IO_ERR recv(char* buffer, size_t length, size_t& actualLength);
	void setRemotePort(int remotePort){ remotePort_ = remotePort;}
	IO_ERR bind();

private:
	int getErrorCode();

private:
	int port_;
	string remoteHost_;
	int remotePort_;
	SOCKET handle_;
	struct sockaddr_in addrRemote_;
};

class DataInputStream{
public:
	DataInputStream(const char* data, int size, bool copy = true);
	DataInputStream(const SocketSP& socket, int bufSize = 2048);
	DataInputStream(FILE* file, int bufSize = 2048);
	virtual ~DataInputStream();
	bool reset(int size);
	IO_ERR close();
	void enableReverseIntegerByteOrder() { reverseOrder_ = true;}
	void disableReverseIntegerByteOrder() { reverseOrder_ = false;}
	IO_ERR read(char* buf, size_t length) { return readBytes(buf, length, false);}
	IO_ERR readBytes(char* buf, size_t length, bool reverseOrder);
	IO_ERR readBytes(char* buf, size_t length, size_t& actualLength);
	IO_ERR readBytes(char* buf, size_t unitLength, size_t length, size_t& actualLength);
	IO_ERR readBool(bool& value);
	IO_ERR readBool(char& value);
	IO_ERR readChar(char& value);
	IO_ERR readUnsignedChar(unsigned char& value);
	IO_ERR readShort(short& value);
	IO_ERR readUnsignedShort(unsigned short& value);
	IO_ERR readInt(int& value);
	IO_ERR readUnsignedInt(unsigned int& value);
	IO_ERR readLong(long long& value);
	IO_ERR readFloat(float& value);
	IO_ERR readDouble(double& value);
	IO_ERR readString(string& value);
	IO_ERR readString(string& value, size_t length);
	IO_ERR readLine(string& value);
	/**
	 * Preview the given size of stream data from the current position. The internal current position will not change
	 * after this operation. If the available data in the internal buffer from the current position is less than the
	 * requested size, the method will read data from the socket and be blocked or immediately return an error if there
	 * is not data available unfortunately depending on the socket mode, blocking or non-blocking.
	 */
	IO_ERR peekBuffer(char* buf, size_t size);
	IO_ERR peekLine(string& value);
	bool isFileStream() const { return source_ == 1;}
	bool isSocketStream() const {return source_ == 0;}
	bool isArrayStream() const {return source_ == 2;}
	SocketSP getSocket() const { return socket_;}
	FILE* getFileHandle() const { return file_;}
	const char* getArrayCursor() const { return buf_ + cursor_;}
	INDEX getDataSizeInArray() const { return size_;}
	bool isIntegerReversed() const {return reverseOrder_;}

private:
	IO_ERR prepareBytes(size_t length);
	IO_ERR prepareBytesEndWith(char endChar, size_t& endPos);

protected:
	SocketSP socket_;
	FILE* file_;
	char* buf_;
	char source_;
	bool reverseOrder_;
	size_t capacity_;
	size_t size_;
	size_t cursor_;
	bool externalBuf_;
};

class DataOutputStream {
public:
	DataOutputStream(const SocketSP& socket) : socket_(socket), file_(0){}
	DataOutputStream(FILE* file) : file_(file){}
	IO_ERR write(const char* buffer, size_t length, size_t& actualLength);
	inline IO_ERR write(const char* buffer, size_t length){
		size_t actualLength;
		return write(buffer, length, actualLength);
	}

protected:
	SocketSP socket_;
	FILE* file_;
};

template<class T>
class BufferWriter{
public:
	BufferWriter(const T& out) : out_(out), buffer_(0), size_(0){}

	IO_ERR start(const char* buffer, size_t length){
		IO_ERR ret;
		size_t actualRead = 0;

		do{
			buffer = buffer + actualRead;
			length -= actualRead;
			ret = out_->write(buffer, length, actualRead);
		} while(ret == OK && actualRead < length);

		if(ret == NOSPACE){
			buffer_ = (char *)buffer;
			size_ = length;
		}
		else
			size_ = 0;
		return ret;
	}

	IO_ERR resume(){
		IO_ERR ret = OK;
		size_t actualRead = 0;

		while(ret==OK  && actualRead < size_){
			buffer_ = buffer_ + actualRead;
			size_ -= actualRead;
			ret = out_->write(buffer_, size_, actualRead);
		}
		return ret;
	}

	inline size_t size() const { return size_;}
	inline T getDataOutputStream() const { return out_;}

private:
	T out_;
	char* buffer_;
	size_t size_;
};

class DataStream : public DataInputStream{
public:
	DataStream(const char* data, int size) : DataInputStream(data, size), flag_(1), outBuf_(0), outSize_(0){}
	DataStream(const SocketSP& socket) : DataInputStream(socket), flag_(3), outBuf_(new char[2048]), outSize_(2048){}
	DataStream(FILE* file, bool readable, bool writable);
	virtual ~DataStream();
	bool isReadable() const;
	void isReadable(bool option);
	bool isWritable() const;
	void isWritable(bool option);
	IO_ERR clearReadBuffer();
	IO_ERR write(const char* buf, int length, int& sent);
	IO_ERR write(Constant* obj, INDEX offset, INDEX length, INDEX& sent);
	IO_ERR writeLine(const char* obj, const char* newline);
	IO_ERR writeLine(Constant* obj, INDEX offset, INDEX length, const char* newline, INDEX& sent);
	IO_ERR seek(long long offset, int mode, long long& newPosition);
	string getDescription() const;

private:
	char flag_; // bit0: readable bit1: writable
	char* outBuf_;
	size_t outSize_;
};

struct FileAttributes{
	string name;
	bool isDir;
	long long size;
	long long lastModified; //epoch time in milliseconds
	long long lastAccessed; //epoch time in milliseconds
};


#endif /* SOCKET_H_ */
