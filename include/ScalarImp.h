/*
 * ScalarImp.h
 *
 *  Created on: May 10, 2017
 *      Author: dzhou
 */

#ifndef SCALARIMP_H_
#define SCALARIMP_H_

#include <climits>

#include "CoreConcept.h"
#include "Util.h"

using std::ostringstream;

class SystemHandle;
class DataSource;
class Resource;
typedef SmartPointer<SystemHandle> SystemHandleSP;
typedef SmartPointer<DataSource> DataSourceSP;
typedef SmartPointer<Resource> ResourceSP;

void initFormatters();

class Void: public Constant{
public:
	Void(bool explicitNull = false){setNothing(!explicitNull);}
	virtual ConstantSP getInstance() const {return ConstantSP(new Void(!isNothing()));}
	virtual ConstantSP getValue() const {return ConstantSP(new Void(!isNothing()));}
	virtual DATA_TYPE getType() const {return DT_VOID;}
	virtual DATA_TYPE getRawType() const { return DT_VOID;}
	virtual DATA_CATEGORY getCategory() const {return NOTHING;}
	virtual string getString() const {return Constant::EMPTY;}
	virtual string getScript() const {return isNothing() ? Constant::EMPTY : Constant::NULL_STR;}
	virtual const string& getStringRef() const {return Constant::EMPTY;}
	virtual char getBool() const {return CHAR_MIN;}
	virtual char getChar() const {return CHAR_MIN;}
	virtual short getShort() const {return SHRT_MIN;}
	virtual int getInt() const {return INT_MIN;}
	virtual long long  getLong() const {return LLONG_MIN;}
	virtual float getFloat() const {return FLT_NMIN;}
	virtual double getDouble() const {return DBL_NMIN;}
	virtual bool isNull() const {return true;}
	virtual void nullFill(const ConstantSP& val){}
	virtual bool isNull(INDEX start, int len, char* buf) const;
	virtual bool isValid(INDEX start, int len, char* buf) const;
	virtual bool getBool(INDEX start, int len, char* buf) const;
	virtual const char* getBoolConst(INDEX start, int len, char* buf) const;
	virtual bool getChar(INDEX start, int len, char* buf) const;
	virtual const char* getCharConst(INDEX start, int len, char* buf) const;
	virtual bool getShort(INDEX start, int len, short* buf) const;
	virtual const short* getShortConst(INDEX start, int len, short* buf) const;
	virtual bool getInt(INDEX start, int len, int* buf) const;
	virtual const int* getIntConst(INDEX start, int len, int* buf) const;
	virtual bool getLong(INDEX start, int len, long long* buf) const;
	virtual const long long* getLongConst(INDEX start, int len, long long* buf) const;
	virtual bool getIndex(INDEX start, int len, INDEX* buf) const;
	virtual const INDEX* getIndexConst(INDEX start, int len, INDEX* buf) const;
	virtual bool getFloat(INDEX start, int len, float* buf) const;
	virtual const float* getFloatConst(INDEX start, int len, float* buf) const;
	virtual bool getDouble(INDEX start, int len, double* buf) const;
	virtual const double* getDoubleConst(INDEX start, int len, double* buf) const;
	virtual bool getSymbol(INDEX start, int len, int* buf, SymbolBase* symBase,bool insertIfNotThere) const;
	virtual const int* getSymbolConst(INDEX start, int len, int* buf, SymbolBase* symBase, bool insertIfNotThere) const;
	virtual bool getString(INDEX start, int len, string** buf) const;
	virtual string** getStringConst(INDEX start, int len, string** buf) const;
	virtual long long getAllocatedMemory() const;
	virtual IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const;
	virtual int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int& numElement, int& partial) const;
	virtual IO_ERR deserialize(DataInputStream* in, INDEX indexStart, INDEX targetNumElement, INDEX& numElement);
	virtual int compare(INDEX index, const ConstantSP& target) const {return target->getType() == DT_VOID ? 0 : -1;}
};

class ObjectPool: public Void {
public:
	void cacheObject(long long sessionId, long long id, const ConstantSP& obj);
	ConstantSP getCache(long long sessionId, long long id);
	void clearCache();
	void clearCache(long long sessionId);
	void clearCache(long long sessionId, long long id);
	long long requestCacheId(int count);
	int getObjectCount() const;
	virtual long long releaseMemory(long long target, bool& satisfied);
	static ObjectPool inst_;

private:
	ObjectPool() : cacheSeed_ (0){}
	struct pairHash {
		size_t operator()(const pair<long long, long long>& x) const{
			//TODO: improve the combined hash function
			return std::hash<long long>()(x.first) ^ std::hash<long long>()(x.second);
		}
	};

	unordered_map<pair<long long,long long>, ConstantSP, pairHash> caches_;
	mutable Mutex mutex_;
	long long cacheSeed_;
};

class Uuid: public Constant{
public:
	Uuid(bool newUuid = false);
	Uuid(const unsigned char* uuid);
	Uuid(const string& uuid);
	Uuid(const Uuid& copy);
	virtual ~Uuid(){}
	bool operator==(const Uuid &other) const;
	bool operator!=(const Uuid &other) const;
	inline const unsigned char* bytes() const { return uuid_;}
	virtual string getString() const;
	virtual const Guid getUuid() const { return uuid_;}
	virtual bool isNull() const;
	virtual void setNull();
	virtual void nullFill(const ConstantSP& val){
		if(isNull())
			memcpy(uuid_, val->getUuid().bytes(), 16);
	}
	virtual bool isNull(INDEX start, int len, char* buf) const {
		char null=isNull();
		for(int i=0;i<len;++i)
			buf[i]=null;
		return true;
	}
	virtual bool isValid(INDEX start, int len, char* buf) const {
		char valid=!isNull();
		for(int i=0;i<len;++i)
			buf[i]=valid;
		return true;
	}
	virtual ConstantSP getInstance() const {return new Uuid(false);}
	virtual ConstantSP getValue() const {return new Uuid(uuid_);}
	virtual DATA_TYPE getType() const {return DT_UUID;}
	virtual DATA_TYPE getRawType() const { return DT_UUID;}
	virtual DATA_CATEGORY getCategory() const {return LITERAL;}
	virtual long long getAllocatedMemory() const {return sizeof(Uuid);}
	virtual IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const {
		short flag = (DF_SCALAR <<8) + DT_UUID;
		buffer->write((char)CONSTOBJ);
		buffer->write(flag);
		return buffer->write((const char*)uuid_, 16);
	}
	virtual int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int& numElement, int& partial) const;
	virtual IO_ERR deserialize(DataInputStream* in, INDEX indexStart, INDEX targetNumElement, INDEX& numElement);
	virtual int compare(INDEX index, const ConstantSP& target) const;

private:
    unsigned char hexDigitToChar(char ch) const;
    inline unsigned char hexPairToChar(char a, char b) const { return hexDigitToChar(a) * 16 + hexDigitToChar(b);}
    void charToHexPair(unsigned char ch, char* buf) const;

private:
	mutable unsigned char uuid_[16];
};

class String: public Constant{
public:
	String(string val=""):val_(val){}
	virtual ~String(){}
	virtual char getBool() const {throw IncompatibleTypeException(DT_BOOL,DT_STRING);}
	virtual char getChar() const {throw IncompatibleTypeException(DT_CHAR,DT_STRING);}
	virtual short getShort() const {throw IncompatibleTypeException(DT_SHORT,DT_STRING);}
	virtual int getInt() const {throw IncompatibleTypeException(DT_INT,DT_STRING);}
	virtual long long getLong() const {throw IncompatibleTypeException(DT_LONG,DT_STRING);}
	virtual INDEX getIndex() const {throw IncompatibleTypeException(DT_INDEX,DT_STRING);}
	virtual float getFloat() const {throw IncompatibleTypeException(DT_FLOAT,DT_STRING);}
	virtual double getDouble() const {throw IncompatibleTypeException(DT_DOUBLE,DT_STRING);}
	virtual string getString() const {return val_;}
	virtual string getScript() const {return Util::literalConstant(val_);}
	virtual const string& getStringRef() const {return val_;}
	virtual const string& getStringRef(INDEX index) const {return val_;}
	virtual bool isNull() const {return val_.empty();}
	virtual void setString(const string& val) {val_=val;}
	virtual void setNull(){val_="";}
	virtual void nullFill(const ConstantSP& val){
		if(isNull())
			val_=val->getStringRef();
	}
	virtual bool isNull(INDEX start, int len, char* buf) const {
		char null=isNull();
		for(int i=0;i<len;++i)
			buf[i]=null;
		return true;
	}
	virtual bool isValid(INDEX start, int len, char* buf) const {
		char valid=!isNull();
		for(int i=0;i<len;++i)
			buf[i]=valid;
		return true;
	}
	virtual const int* getSymbolConst(INDEX start, int len, int* buf, SymbolBase* symBase, bool insertIfNotThere) const {
		int tmp=insertIfNotThere?symBase->findAndInsert(val_):symBase->find(val_);
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return buf;
	}
	virtual bool getString(INDEX start, int len, string** buf) const {
		for(int i=0;i<len;++i)
			buf[i]=&val_;
		return true;
	}
	virtual string** getStringConst(INDEX start, int len, string** buf) const {
		for(int i=0;i<len;++i)
			buf[i]=&val_;
		return buf;
	}
	virtual char** getStringConst(INDEX start, int len, char** buf) const {
		char* val = (char*)val_.c_str();
		for(int i=0;i<len;++i)
			buf[i]=val;
		return buf;
	}
	virtual ConstantSP getInstance() const {return ConstantSP(new String());}
	virtual ConstantSP getValue() const {return ConstantSP(new String(val_));}
	virtual DATA_TYPE getType() const {return DT_STRING;}
	virtual DATA_TYPE getRawType() const { return DT_STRING;}
	virtual DATA_CATEGORY getCategory() const {return LITERAL;}
	virtual long long getAllocatedMemory() const {return sizeof(string)+val_.size();}
	virtual IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const {
		short flag = (DF_SCALAR <<8) + getType();
		buffer->write((char)CONSTOBJ);
		buffer->write(flag);
		return buffer->write(val_);
	}
	virtual int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int& numElement, int& partial) const;
	virtual IO_ERR deserialize(DataInputStream* in, INDEX indexStart, INDEX targetNumElement, INDEX& numElement);
	virtual int compare(INDEX index, const ConstantSP& target) const {
		return val_.compare(target->getString());
	}

private:
	mutable string val_;
};

class MetaCode : public String {
public:
	MetaCode(const ObjectSP& code) : String("< " + code->getScript() +" >"), code_(code){}
	virtual DATA_TYPE getType() const {return DT_CODE;}
	virtual DATA_TYPE getRawType() const { return DT_CODE;}
	virtual DATA_CATEGORY getCategory() const {return SYSTEM;}
	virtual string getScript() const;
	virtual bool copyable() const {return false;}
	virtual bool containNotMarshallableObject() const {return true;}
	virtual IO_ERR serialize(Heap* pHeap, const ByteArrayCodeBufferSP& buffer) const;
	ObjectSP getCode() const { return code_;}

private:
	ObjectSP code_;
};

class DataSource : public String {
public:
	DataSource(const ObjectSP& code, long long cacheId = -1, bool isTable = true, bool localMode = false) : String("DataSource< " + code->getScript() +" >"), code_(code),
		parentId_(-1), id_(cacheId), action_(-1), isTable_(isTable), localMode_(localMode){}
	DataSource(Session* session, const DataInputStreamSP& in);
	virtual DATA_TYPE getType() const {return DT_DATASOURCE;}
	virtual DATA_TYPE getRawType() const { return DT_DATASOURCE;}
	virtual DATA_CATEGORY getCategory() const {return SYSTEM;}
	virtual string getScript() const;
	virtual bool copyable() const {return false;}
	virtual bool containNotMarshallableObject() const {return true;}
	virtual void collectUserDefinedFunctions(unordered_map<string,FunctionDef*>& functionDefs) const;
	virtual IO_ERR serialize(Heap* pHeap, const ByteArrayCodeBufferSP& buffer) const;
	virtual ConstantSP getReference(Heap* pHeap);
	ObjectSP getCode() const { return code_;}
	bool addTransformer(const FunctionDefSP& transformer);
	bool isLocalData() const { return sites_.isNull();}
	void setCacheId(long long id) { id_ = id;}
	long long getCacheId() const { return id_;}
	void setParentId(long long id) { parentId_ = id;}
	long long getParentId() const { return parentId_;}
	void setCacheAction(char action) { action_ = action;}
	char getCacheAction() const { return action_;}
	void setSitePool(const DomainSitePoolSP& sites) { sites_ = sites;}
	DomainSitePoolSP getSitePool() const {return sites_;}
	bool isTable() const { return isTable_;}
	bool isLocalMode() const { return localMode_;}

private:
	ObjectSP code_;
	vector<FunctionDefSP> transformers_;
	DomainSitePoolSP sites_;
	long long parentId_;
	long long id_;
	char action_; // -1: do nothing, 1: enable cache, i.e. set cache id, 0:clear cache after use.
	bool isTable_; // check if it can be used in SQL statement as the source of a table.
	bool localMode_;
};


class SystemHandle : public String{
public:
	SystemHandle(SOCKET handle, bool isLittleEndian, const string& sessionID, const string& host, int port, const string& userId, const string& pwd) : String("Conn[" + host + ":" +Util::convert(port) + ":" +sessionID + "]"),
		type_(REMOTE_HANDLE), socket_(handle), flag_(isLittleEndian ? 1 : 0), sessionID_(sessionID), userId_(userId), pwd_(pwd), tables_(0){}
	SystemHandle(const DataStreamSP& handle, bool isLittleEndian) : String(handle->getDescription()),
		type_(handle->isFileStream() ? FILE_HANDLE : SOCKET_HANDLE), socket_(-1), flag_(isLittleEndian ? 1 : 0), stream_(handle), tables_(0){}
	SystemHandle(const string& dbDir, const DomainSP& domain): String("DB[" + dbDir + "]"),
		type_(DB_HANDLE), socket_(-1), flag_(Util::isLittleEndian() ? 1 : 0), dbDir_(dbDir), domain_(domain), symbaseManager_(new SymbolBaseManager(dbDir)), tables_(new unordered_map<string, TableSP>()){}
	SystemHandle(const string& dbDir, const DomainSP& domain, const SymbolBaseManagerSP& symManager): String("DB[" + dbDir + "]"),
		type_(DB_HANDLE), socket_(-1), flag_(Util::isLittleEndian() ? 1 : 0), dbDir_(dbDir), domain_(domain), symbaseManager_(symManager), tables_(new unordered_map<string, TableSP>()){}
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
	void setExpired(bool option);
	SOCKET getSocketHandle() const {return socket_;}
	HANDLE_TYPE getHandleType() const { return type_;}
	DataStreamSP getDataStream() const { return stream_;}
    const string& getDatabaseDir() const { return dbDir_;}
    DomainSP getDomain() const { return domain_;}
    SymbolBaseManagerSP getSymbolBaseManager() const { return symbaseManager_;}
	virtual DATA_TYPE getType() const {return DT_HANDLE;}
	virtual DATA_CATEGORY getCategory() const {return SYSTEM;}
	virtual bool copyable() const {return false;}
	virtual ConstantSP getValue() const { throw RuntimeException("System handle is not copyable.");}
	virtual IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const { throw RuntimeException("System handle is not able to serialize.");}
	virtual bool containNotMarshallableObject() const {return true;}
	void close();

private:
	HANDLE_TYPE type_;
	SOCKET socket_;
	char flag_; //bit0: littleEndian, bit1: closed, bit2: expired
	string sessionID_;
	string userId_;
	string pwd_;
	DataStreamSP stream_;
	string dbDir_;
	DomainSP domain_;
	SymbolBaseManagerSP symbaseManager_;
	unordered_map<string, TableSP>* tables_;
};

class Resource : public String{
public:
	Resource(long long handle, const string& desc, const FunctionDefSP& onClose, Session* session) : String(desc), handle_(handle), onClose_(onClose), session_(session){}
	virtual ~Resource();
	virtual DATA_TYPE getType() const {return DT_RESOURCE;}
	virtual DATA_CATEGORY getCategory() const {return SYSTEM;}
	virtual bool copyable() const {return false;}
	virtual ConstantSP getValue() const { throw RuntimeException("Resource is not copyable.");}
	virtual IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const { throw RuntimeException("Resource is not able to serialize.");}
	virtual bool containNotMarshallableObject() const {return true;}
	virtual long long  getLong() const {return handle_;}
	virtual void setLong(long long val){ handle_ = val;}
	void close();

private:
	long long handle_;
	FunctionDefSP onClose_;
	Session* session_;
};

template <class T>
class AbstractScalar: public Constant{
public:
	AbstractScalar(T val=0):val_(val){}
	virtual ~AbstractScalar(){}
	virtual char getBool() const {return isNull()?CHAR_MIN:(bool)val_;}
	virtual char getChar() const {return isNull()?CHAR_MIN:val_;}
	virtual short getShort() const {return isNull()?SHRT_MIN:val_;}
	virtual int getInt() const {return isNull()?INT_MIN:val_;}
	virtual long long getLong() const {return isNull()?LLONG_MIN:val_;}
	virtual INDEX getIndex() const {return isNull()?INDEX_MIN:val_;}
	virtual float getFloat() const {return isNull()?FLT_NMIN:val_;}
	virtual double getDouble() const {return isNull()?DBL_NMIN:val_;}

	virtual void setBool(char val){if(val != CHAR_MIN) val_=(T)val; else setNull();}
	virtual void setChar(char val){if(val != CHAR_MIN) val_=(T)val; else setNull();}
	virtual void setShort(short val){if(val != SHRT_MIN) val_=(T)val; else setNull();}
	virtual void setInt(int val){if(val != INT_MIN) val_=(T)val; else setNull();}
	virtual void setLong(long long val){if(val != LLONG_MIN) val_=(T)val; else setNull();}
	virtual void setIndex(INDEX val){if(val != INDEX_MIN) val_=(T)val; else setNull();}
	virtual void setFloat(float val){if(val != FLT_NMIN) val_=(T)val; else setNull();}
	virtual void setDouble(double val){if(val != DBL_NMIN) val_=(T)val; else setNull();}
	virtual void setString(const string& val){}
	virtual bool isNull() const = 0;
	virtual string getScript() const {
		if(isNull()){
			string str("00");
			return str.append(1, Util::getDataTypeSymbol(getType()));
		}
		else
			return getString();
	}

	virtual void nullFill(const ConstantSP& val){
		if(isNull()){
			if(val->getCategory()==FLOATING)
				val_=val->getDouble();
			else
				val_=val->getLong();
		}
	}
	virtual bool isNull(INDEX start, int len, char* buf) const {
		char null=isNull();
		for(int i=0;i<len;++i)
			buf[i]=null;
		return true;
	}
	virtual bool isValid(INDEX start, int len, char* buf) const {
		char valid=!isNull();
		for(int i=0;i<len;++i)
			buf[i]=valid;
		return true;
	}
	virtual bool getBool(INDEX start, int len, char* buf) const {
		char tmp=isNull()?CHAR_MIN:(bool)val_;
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return true;
	}
	virtual const char* getBoolConst(INDEX start, int len, char* buf) const {
		char tmp=isNull()?CHAR_MIN:(bool)val_;
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return buf;
	}
	virtual bool getChar(INDEX start, int len, char* buf) const {
		char tmp=isNull()?CHAR_MIN:val_;
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return true;
	}
	virtual const char* getCharConst(INDEX start, int len, char* buf) const {
		char tmp=isNull()?CHAR_MIN:val_;
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return buf;
	}
	virtual bool getShort(INDEX start, int len, short* buf) const {
		short tmp=isNull()?SHRT_MIN:val_;
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return true;
	}
	virtual const short* getShortConst(INDEX start, int len, short* buf) const {
		short tmp=isNull()?SHRT_MIN:val_;
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return buf;
	}
	virtual bool getInt(INDEX start, int len, int* buf) const {
		int tmp=isNull()?INT_MIN:val_;
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return true;
	}
	virtual const int* getIntConst(INDEX start, int len, int* buf) const {
		int tmp=isNull()?INT_MIN:val_;
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return buf;
	}
	virtual bool getLong(INDEX start, int len, long long* buf) const {
		long long tmp=isNull()?LLONG_MIN:val_;
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return true;
	}
	virtual const long long* getLongConst(INDEX start, int len, long long* buf) const {
		long long tmp=isNull()?LLONG_MIN:val_;
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return buf;
	}
	virtual bool getIndex(INDEX start, int len, INDEX* buf) const {
		INDEX tmp=isNull()?INDEX_MIN:val_;
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return true;
	}
	virtual const INDEX* getIndexConst(INDEX start, int len, INDEX* buf) const {
		INDEX tmp=isNull()?INDEX_MIN:val_;
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return buf;
	}
	virtual bool getFloat(INDEX start, int len, float* buf) const {
		float tmp=isNull()?FLT_NMIN:val_;
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return true;
    }
	virtual const float* getFloatConst(INDEX start, int len, float* buf) const {
		float tmp=isNull()?FLT_NMIN:val_;
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return buf;
    }
	virtual bool getDouble(INDEX start, int len, double* buf) const {
		double tmp=isNull()?DBL_NMIN:val_;
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return true;
	}
	virtual const double* getDoubleConst(INDEX start, int len, double* buf) const {
		double tmp=isNull()?DBL_NMIN:val_;
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return buf;
	}
	virtual long long getAllocatedMemory() const {return sizeof(AbstractScalar);}
	virtual int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int& numElement, int& partial) const {
		int len = sizeof(T)-offset;
		if(len < 0)
			return -1;
		else if(bufSize >= len){
			numElement = 1;
			partial = 0;
			memcpy(buf,((char*)&val_)+offset, len);
			return len;
		}
		else{
			len = bufSize;
			numElement = 0;
			partial = offset+bufSize;
			memcpy(buf,((char*)&val_)+offset, len);
			return len;
		}
	}

	virtual bool add(INDEX start, INDEX length, long long inc) {
		if(isNull())
			return false;
		val_ += inc;
		return true;
	}

	virtual bool add(INDEX start, INDEX length, double inc) {
		if(isNull())
			return false;
		val_ += inc;
		return true;
	}

	virtual IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const {
		short flag = (DF_SCALAR <<8) + getType();
		buffer->write((char)CONSTOBJ);
		buffer->write(flag);
		char buf[8];
		int numElement, partial;
		int length = serialize(buf, 8, 0, 0, numElement, partial);
		return buffer->write(buf, length);
	}
	virtual int compare(INDEX index, const ConstantSP& target) const {
		if(getCategory() == FLOATING){
			T val= (T)target->getDouble();
			return val_==val?0:(val_<val?-1:1);
		}
		else{
			T val= (T)target->getLong();
			return val_==val?0:(val_<val?-1:1);
		}
	}

protected:
	T val_;
};

class Bool: public AbstractScalar<char>{
public:
	Bool(char val=0):AbstractScalar(val){}
	virtual ~Bool(){}
	virtual bool isNull() const {return val_==CHAR_MIN;}
	virtual void setNull(){val_= CHAR_MIN;}
	virtual void setBool(char val){ val_ = val;}
	virtual DATA_TYPE getType() const {return DT_BOOL;}
	virtual DATA_TYPE getRawType() const { return DT_BOOL;}
	virtual ConstantSP getInstance() const {return ConstantSP(new Bool());}
	virtual ConstantSP getValue() const {return ConstantSP(new Bool(val_));}
	virtual DATA_CATEGORY getCategory() const {return LOGICAL;}
	virtual string getString() const { return toString(val_);}
	virtual bool add(INDEX start, INDEX length, long long inc) { return false;}
	virtual bool add(INDEX start, INDEX length, double inc) { return false;}
	virtual IO_ERR deserialize(DataInputStream* in, INDEX indexStart, INDEX targetNumElement, INDEX& numElement);
	static Bool* parseBool(const string& str);
	static string toString(char val){
		if(val == CHAR_MIN)
			return "";
		else if(val)
			return "1";
		else
			return "0";
	}
};

class Char: public AbstractScalar<char>{
public:
	Char(char val=0):AbstractScalar(val){}
	virtual ~Char(){}
	virtual bool isNull() const {return val_==CHAR_MIN;}
	virtual void setNull(){val_=CHAR_MIN;}
	virtual void setChar(char val){ val_ = val;}
	virtual DATA_TYPE getType() const {return DT_CHAR;}
	virtual DATA_TYPE getRawType() const { return DT_CHAR;}
	virtual ConstantSP getInstance() const {return ConstantSP(new Char());}
	virtual ConstantSP getValue() const {return ConstantSP(new Char(val_));}
	virtual DATA_CATEGORY getCategory() const {return INTEGRAL;}
	virtual string getString() const { return toString(val_);}
	virtual string getScript() const;
	virtual IO_ERR deserialize(DataInputStream* in, INDEX indexStart, INDEX targetNumElement, INDEX& numElement);
	static Char* parseChar(const string& str);
	static string toString(char val);
};

class Short: public AbstractScalar<short>{
public:
	Short(short val=0):AbstractScalar(val){}
	virtual ~Short(){}
	virtual bool isNull() const {return val_==SHRT_MIN;}
	virtual void setNull(){val_=SHRT_MIN;}
	virtual void setShort(short val){ val_ = val;}
	virtual DATA_TYPE getType() const {return DT_SHORT;}
	virtual DATA_TYPE getRawType() const { return DT_SHORT;}
	virtual ConstantSP getInstance() const {return ConstantSP(new Short());}
	virtual ConstantSP getValue() const {return ConstantSP(new Short(val_));}
	virtual DATA_CATEGORY getCategory() const {return INTEGRAL;}
	virtual string getString() const { return toString(val_);}
	virtual IO_ERR deserialize(DataInputStream* in, INDEX indexStart, INDEX targetNumElement, INDEX& numElement);
	static Short* parseShort(const string& str);
	static string toString(short val);
};

class Int: public AbstractScalar<int>{
public:
	Int(int val=0):AbstractScalar(val){}
	virtual ~Int(){}
	virtual bool isNull() const {return val_==INT_MIN;}
	virtual void setNull(){val_=INT_MIN;}
	virtual void setInt(int val){ val_ = val;}
	virtual DATA_TYPE getType() const {return DT_INT;}
	virtual DATA_TYPE getRawType() const { return DT_INT;}
	virtual ConstantSP getInstance() const {return ConstantSP(new Int());}
	virtual ConstantSP getValue() const {return ConstantSP(new Int(val_));}
	virtual DATA_CATEGORY getCategory() const {return INTEGRAL;}
	virtual string getString() const { return toString(val_);}
	virtual IO_ERR deserialize(DataInputStream* in, INDEX indexStart, INDEX targetNumElement, INDEX& numElement);
	static Int* parseInt(const string& str);
	static string toString(int val);
};

class EnumInt : public Int {
public:
	EnumInt(const string& desc, int val):Int(val), desc_(desc){}
	virtual ~EnumInt(){}
	virtual string getScript() const {return desc_;}
	virtual ConstantSP getValue() const {return ConstantSP(new EnumInt(desc_, val_));}
	virtual ConstantSP getInstance() const {return ConstantSP(new EnumInt(desc_, val_));}
	virtual string getString() const {return desc_;}

private:
	string desc_;
};

class Long: public AbstractScalar<long long>{
public:
	Long(long long val=0):AbstractScalar(val){}
	virtual ~Long(){}
	virtual bool isNull() const {return val_==LLONG_MIN;}
	virtual void setNull(){val_=LLONG_MIN;}
	virtual void setLong(long long val){ val_ = val;}
	virtual DATA_TYPE getType() const {return DT_LONG;}
	virtual DATA_TYPE getRawType() const { return DT_LONG;}
	virtual ConstantSP getInstance() const {return ConstantSP(new Long());}
	virtual ConstantSP getValue() const {return ConstantSP(new Long(val_));}
	virtual string getString() const { return toString(val_);}
	virtual DATA_CATEGORY getCategory() const {return INTEGRAL;}
	virtual IO_ERR deserialize(DataInputStream* in, INDEX indexStart, INDEX targetNumElement, INDEX& numElement);
	static Long* parseLong(const string& str);
	static string toString(long long val);
};

class Float: public AbstractScalar<float>{
public:
	Float(float val=0):AbstractScalar(val){}
	virtual ~Float(){}
	virtual bool isNull() const {return val_==FLT_NMIN;}
	virtual void setNull(){val_=FLT_NMIN;}
	virtual void setFloat(float val){ val_ = val;}
	virtual DATA_TYPE getType() const {return DT_FLOAT;}
	virtual DATA_TYPE getRawType() const { return DT_FLOAT;}
	virtual ConstantSP getInstance() const {return ConstantSP(new Float());}
	virtual ConstantSP getValue() const {return ConstantSP(new Float(val_));}
	virtual DATA_CATEGORY getCategory() const {return FLOATING;}
	virtual char getChar() const {return isNull()?CHAR_MIN:(val_<0?val_-0.5:val_+0.5);}
	virtual short getShort() const {return isNull()?SHRT_MIN:(val_<0?val_-0.5:val_+0.5);}
	virtual int getInt() const {return isNull()?INT_MIN:(val_<0?val_-0.5:val_+0.5);}
	virtual long long  getLong() const {return isNull()?LLONG_MIN:(val_<0?val_-0.5:val_+0.5);}
	virtual bool getChar(INDEX start, int len, char* buf) const;
	virtual const char* getCharConst(INDEX start, int len, char* buf) const;
	virtual bool getShort(INDEX start, int len, short* buf) const;
	virtual const short* getShortConst(INDEX start, int len, short* buf) const;
	virtual bool getInt(INDEX start, int len, int* buf) const;
	virtual const int* getIntConst(INDEX start, int len, int* buf) const;
	virtual bool getLong(INDEX start, int len, long long* buf) const;
	virtual const long long* getLongConst(INDEX start, int len, long long* buf) const;
	virtual string getString() const { return toString(val_);}
	virtual IO_ERR deserialize(DataInputStream* in, INDEX indexStart, INDEX targetNumElement, INDEX& numElement);
	static Float* parseFloat(const string& str);
	static string toString(float val);
};

class Double: public AbstractScalar<double>{
public:
	Double(double val=0):AbstractScalar(val){}
	virtual ~Double(){}
	virtual bool isNull() const {return val_==DBL_NMIN;}
	virtual void setNull(){val_=DBL_NMIN;}
	virtual void setDouble(double val){ val_ = val;}
	virtual DATA_TYPE getType() const {return DT_DOUBLE;}
	virtual DATA_TYPE getRawType() const { return DT_DOUBLE;}
	virtual ConstantSP getInstance() const {return ConstantSP(new Double());}
	virtual ConstantSP getValue() const {return ConstantSP(new Double(val_));}
	virtual DATA_CATEGORY getCategory() const {return FLOATING;}
	virtual char getChar() const {return isNull()?CHAR_MIN:(val_<0?val_-0.5:val_+0.5);}
	virtual short getShort() const {return isNull()?SHRT_MIN:(val_<0?val_-0.5:val_+0.5);}
	virtual int getInt() const {return isNull()?INT_MIN:(val_<0?val_-0.5:val_+0.5);}
	virtual long long  getLong() const {return isNull()?LLONG_MIN:(val_<0?val_-0.5:val_+0.5);}
	virtual bool getChar(INDEX start, int len, char* buf) const;
	virtual const char* getCharConst(INDEX start, int len, char* buf) const;
	virtual bool getShort(INDEX start, int len, short* buf) const;
	virtual const short* getShortConst(INDEX start, int len, short* buf) const;
	virtual bool getInt(INDEX start, int len, int* buf) const;
	virtual const int* getIntConst(INDEX start, int len, int* buf) const;
	virtual bool getLong(INDEX start, int len, long long* buf) const;
	virtual const long long* getLongConst(INDEX start, int len, long long* buf) const;
	virtual string getString() const {return toString(val_);}
	virtual IO_ERR deserialize(DataInputStream* in, INDEX indexStart, INDEX targetNumElement, INDEX& numElement);
	static Double* parseDouble(const string& str);
	static string toString(double val);
};

class EnumDouble : public Double {
public:
	EnumDouble(const string& desc, double val):Double(val), desc_(desc){}
	virtual ~EnumDouble(){}
	virtual string getScript() const {return desc_;}
	virtual ConstantSP getValue() const {return ConstantSP(new EnumDouble(desc_, val_));}
	virtual ConstantSP getInstance() const {return ConstantSP(new EnumDouble(desc_, val_));}
	virtual string getString() const { return desc_;}

private:
	string desc_;
};

class TemporalScalar:public Int{
public:
	TemporalScalar(int val=0):Int(val){}
	virtual ~TemporalScalar(){}
	virtual DATA_CATEGORY getCategory() const {return TEMPORAL;}
};

class Date:public TemporalScalar{
public:
	Date(int val=0):TemporalScalar(val){}
	virtual ~Date(){}
	Date(int year, int month, int day):TemporalScalar(Util::countDays(year,month,day)){}
	virtual DATA_TYPE getType() const {return DT_DATE;}
	virtual ConstantSP getInstance() const {return ConstantSP(new Date());}
	virtual ConstantSP getValue() const {return ConstantSP(new Date(val_));}
	virtual string getString() const { return toString(val_);}
	static Date* parseDate(const string& str);
	static string toString(int val);
};

class Month:public TemporalScalar{
public:
	Month():TemporalScalar(1999*12+11){}
	Month(int val):TemporalScalar(val){}
	Month(int year, int month):TemporalScalar(year*12+month-1){}
	virtual ~Month(){}
	virtual DATA_TYPE getType() const {return DT_MONTH;}
	virtual ConstantSP getInstance() const {return ConstantSP(new Month());}
	virtual ConstantSP getValue() const {return ConstantSP(new Month(val_));}
	virtual string getString() const { return toString(val_);}
	static Month* parseMonth(const string& str);
	static string toString(int val);
};

class Time:public TemporalScalar{
public:
	Time(int val=0):TemporalScalar(val){}
	Time(int hour, int minute, int second, int milliSecond):TemporalScalar(((hour*60+minute)*60+second)*1000+milliSecond){}
	virtual ~Time(){}
	virtual DATA_TYPE getType() const {return DT_TIME;}
	virtual ConstantSP getInstance() const {return ConstantSP(new Time());}
	virtual ConstantSP getValue() const {return ConstantSP(new Time(val_));}
	virtual string getString() const { return toString(val_);}
	virtual void validate();
	static Time* parseTime(const string& str);
	static string toString(int val);
};

class NanoTime:public Long{
public:
	NanoTime(long long val=0):Long(val){}
	NanoTime(int hour, int minute, int second, int nanoSecond):Long(((hour*60+minute)*60+second)*1000000000ll+ nanoSecond){}
	virtual ~NanoTime(){}
	virtual DATA_TYPE getType() const {return DT_NANOTIME;}
	virtual DATA_CATEGORY getCategory() const {return TEMPORAL;}
	virtual ConstantSP getInstance() const {return ConstantSP(new NanoTime());}
	virtual ConstantSP getValue() const {return ConstantSP(new NanoTime(val_));}
	virtual string getString() const { return toString(val_);}
	virtual void validate();
	static NanoTime* parseNanoTime(const string& str);
	static string toString(long long val);
};

class Timestamp:public Long{
public:
	Timestamp(long long val=0):Long(val){}
	Timestamp(int year, int month, int day,int hour, int minute, int second, int milliSecond);
	virtual ~Timestamp(){}
	virtual DATA_TYPE getType() const {return DT_TIMESTAMP;}
	virtual DATA_CATEGORY getCategory() const {return TEMPORAL;}
	virtual ConstantSP getInstance() const {return ConstantSP(new Timestamp());}
	virtual ConstantSP getValue() const {return ConstantSP(new Timestamp(val_));}
	virtual string getString() const { return toString(val_);}
	static Timestamp* parseTimestamp(const string& str);
	static string toString(long long val);
};

class NanoTimestamp:public Long{
public:
	NanoTimestamp(long long val=0):Long(val){}
	NanoTimestamp(int year, int month, int day,int hour, int minute, int second, int nanoSecond);
	virtual ~NanoTimestamp(){}
	virtual DATA_TYPE getType() const {return DT_NANOTIMESTAMP;}
	virtual DATA_CATEGORY getCategory() const {return TEMPORAL;}
	virtual ConstantSP getInstance() const {return ConstantSP(new NanoTimestamp());}
	virtual ConstantSP getValue() const {return ConstantSP(new NanoTimestamp(val_));}
	virtual string getString() const { return toString(val_);}
	static NanoTimestamp* parseNanoTimestamp(const string& str);
	static string toString(long long val);
};

class Minute:public TemporalScalar{
public:
	Minute(int val=0):TemporalScalar(val){}
	Minute(int hour, int minute):TemporalScalar(hour*60+minute){}
	virtual ~Minute(){}
	virtual DATA_TYPE getType() const {return DT_MINUTE;}
	virtual ConstantSP getInstance() const {return ConstantSP(new Minute());}
	virtual ConstantSP getValue() const {return ConstantSP(new Minute(val_));}
	virtual string getString() const { return toString(val_);}
	virtual void validate();
	static Minute* parseMinute(const string& str);
	static string toString(int val);
};

class Second:public TemporalScalar{
public:
	Second(int val=0):TemporalScalar(val){}
	Second(int hour, int minute,int second):TemporalScalar((hour*60+minute)*60+second){}
	virtual ~Second(){}
	virtual DATA_TYPE getType() const {return DT_SECOND;}
	virtual ConstantSP getInstance() const {return ConstantSP(new Second());}
	virtual ConstantSP getValue() const {return ConstantSP(new Second(val_));}
	virtual string getString() const { return toString(val_);}
	virtual void validate();
	static Second* parseSecond(const string& str);
	static string toString(int val);
};

class DateTime:public TemporalScalar{
public:
	DateTime(int val=0):TemporalScalar(val){}
	DateTime(int year, int month, int day, int hour, int minute,int second);
	virtual ~DateTime(){}
	virtual DATA_TYPE getType() const {return DT_DATETIME;}
	virtual ConstantSP getInstance() const {return ConstantSP(new DateTime());}
	virtual ConstantSP getValue() const {return ConstantSP(new DateTime(val_));}
	virtual string getString() const { return toString(val_);}
	static DateTime* parseDateTime(const string& str);
	static string toString(int val);
};


#endif /* SCALARIMP_H_ */
