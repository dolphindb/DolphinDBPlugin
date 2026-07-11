#pragma once

#include "SmartPointer.h"
#include "SysIO.h"
#include "Types.h"

namespace ddb {

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

typedef SmartPointer<UdpSocket> UdpSocketSP;

} // namespace ddb
