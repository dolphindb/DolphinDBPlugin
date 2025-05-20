#pragma once
#include "ScalarImp.h"
#include "SysIO.h"
#include "SmartPointer.h"

namespace ddb {

class SystemHandle : public String{
public:
	SystemHandle(SocketSP& handle, bool isLittleEndian, const string& sessionID, const string& host, int port, const string& userId, const string& pwd) : String("Conn[" + host + ":" +Util::convert(port) + ":" +sessionID + "]"),
		type_(REMOTE_HANDLE), socket_(handle), flag_(isLittleEndian ? 1 : 0), sessionID_(sessionID), userId_(userId), pwd_(pwd), tables_(0){setTypeAndCategory(DT_HANDLE, SYSTEM);}
	SystemHandle(const DataStreamSP& handle, bool isLittleEndian) : String(handle->getDescription()),
		type_(handle->isFileStream() ? FILE_HANDLE : SOCKET_HANDLE), socket_(nullptr), flag_(isLittleEndian ? 1 : 0), stream_(handle), tables_(0){setTypeAndCategory(DT_HANDLE, SYSTEM);}
	SystemHandle(const string& dbDir, const DomainSP& domain): String("DB[" + dbDir + "]"),
		type_(DB_HANDLE), socket_(nullptr), flag_(Util::LITTLE_ENDIAN_ORDER ? 1 : 0), dbDir_(dbDir), domain_(domain), symbaseManager_(new SymbolBaseManager(dbDir)), tables_(new unordered_map<string, TableSP>()){setTypeAndCategory(DT_HANDLE, SYSTEM);}
	SystemHandle(const string& dbDir, const DomainSP& domain, const SymbolBaseManagerSP& symManager): String("DB[" + dbDir + "]"),
		type_(DB_HANDLE), socket_(nullptr), flag_(Util::LITTLE_ENDIAN_ORDER ? 1 : 0), dbDir_(dbDir), domain_(domain), symbaseManager_(symManager), tables_(new unordered_map<string, TableSP>()){setTypeAndCategory(DT_HANDLE, SYSTEM);}
	virtual ~SystemHandle();
	virtual bool isDatabase() const {return type_ == DB_HANDLE;}
	virtual ConstantSP getMember(const ConstantSP& key) const;
	void addMember(const ConstantSP& obj);
	void removeMember(const string& key);
	ConstantSP getMemberWithoutThrow(const ConstantSP& key) const;
	const string& getSessionID() const { return sessionID_;}
	inline const string& getUserId() const { return userId_;}
	inline const string& getPassword() const { return pwd_;}
	inline bool isLittleEndian() const { return flag_ & 1;}
	inline bool isClosed() const { return flag_ & 2;}
	inline bool isExpired() const { return flag_ & 4;}
	inline bool isDeleted() const { return flag_ & 8;}
	void setExpired(bool option);
	void setDeleted(bool option);
	SocketSP getSocketHandle() const {return socket_;}
	HANDLE_TYPE getHandleType() const { return type_;}
	DataStreamSP getDataStream() const { return stream_;}
    const string& getDatabaseDir() const { return dbDir_;}
    DomainSP getDomain() const;
    void setDomain(const DomainSP& domain);
    SymbolBaseManagerSP getSymbolBaseManager() const { return symbaseManager_;}
	virtual bool copyable() const {return false;}
	virtual ConstantSP getValue() const { throw RuntimeException("System handle is not copyable.");}
	virtual IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const { throw RuntimeException("System handle is not able to serialize.");}
	virtual bool containNotMarshallableObject() const {return true;}
	void close();

private:
	HANDLE_TYPE type_;
	SocketSP socket_;
	char flag_; //bit0: littleEndian, bit1: closed, bit2: expired, bit3: deleted
	string sessionID_;
	string userId_;
	string pwd_;
	DataStreamSP stream_;
	string dbDir_;
	DomainSP domain_;
	SymbolBaseManagerSP symbaseManager_;
	unordered_map<string, TableSP>* tables_;
	mutable Mutex mutex_;
};

typedef SmartPointer<SystemHandle> SystemHandleSP;

} // namespace ddb
