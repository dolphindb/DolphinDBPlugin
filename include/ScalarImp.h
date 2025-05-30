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
	Void(bool explicitNull = false, bool isDefault = false);
	inline bool isDefault() const { return isDefault_;}
	virtual ConstantSP getInstance() const;
	virtual ConstantSP getValue() const;
	virtual DATA_TYPE getRawType() const { return DT_VOID;}
	virtual string getString() const {return Constant::EMPTY;}
	virtual string getScript() const {return isNothing() ? Constant::EMPTY : Constant::NULL_STR;}
	virtual const DolphinString& getStringRef() const {return Constant::DEMPTY;}
	virtual char getBool() const {return CHAR_MIN;}
	virtual char getChar() const {return CHAR_MIN;}
	virtual short getShort() const {return SHRT_MIN;}
	virtual int getInt() const {return INT_MIN;}
	virtual INDEX getIndex() const {return INDEX_MIN;}
	virtual long long  getLong() const {return LLONG_MIN;}
	virtual float getFloat() const {return FLT_NMIN;}
	virtual double getDouble() const {return DBL_NMIN;}
	virtual const Guid getInt128() const { return Guid::ZERO;}
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
	virtual DolphinString** getStringConst(INDEX start, int len, DolphinString** buf) const;
	virtual char** getStringConst(INDEX start, int len, char** buf) const;
	virtual bool getBinary(INDEX start, int len, int unitLength, unsigned char* buf) const;
	virtual const unsigned char* getBinaryConst(INDEX start, int len, int unitLength, unsigned char* buf) const;
	virtual bool isNull(INDEX* indices, int len, char* buf) const;
	virtual bool isValid(INDEX* indices, int len, char* buf) const;
	virtual bool getBool(INDEX* indices, int len, char* buf) const;
	virtual bool getChar(INDEX* indices, int len,char* buf) const;
	virtual bool getShort(INDEX* indices, int len, short* buf) const;
	virtual bool getInt(INDEX* indices, int len, int* buf) const;
	virtual bool getLong(INDEX* indices, int len, long long* buf) const;
	virtual bool getIndex(INDEX* indices, int len, INDEX* buf) const;
	virtual bool getFloat(INDEX* indices, int len, float* buf) const;
	virtual bool getDouble(INDEX* indices, int len, double* buf) const;
	virtual bool getSymbol(INDEX* indices, int len, int* buf, SymbolBase* symBase,bool insertIfNotThere) const;
	virtual bool getString(INDEX* indices, int len, DolphinString** buf) const;
	virtual bool getString(INDEX* indices, int len, char** buf) const;
	virtual bool getBinary(INDEX* indices, int len, int unitLength, unsigned char* buf) const;
	virtual long long getAllocatedMemory() const;
	virtual IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const;
	virtual int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int& numElement, int& partial) const;
	virtual IO_ERR deserialize(DataInputStream* in, INDEX indexStart, int offset, INDEX targetNumElement, INDEX& numElement, int& partial);
	virtual int compare(INDEX index, const ConstantSP& target) const {return target->getType() == DT_VOID ? 0 : -1;}

public:  /// {get,set}Decimal{32,64}
	virtual int getDecimal32(int scale) const override {
		return INT_MIN;
	}
	virtual long long getDecimal64(int scale) const override {
		return LLONG_MIN;
	}
	virtual int128 getDecimal128(int scale) const override {
		return int128MinValue();
	}

	virtual int getDecimal32(INDEX index, int scale) const override {
		int result = 0;
		getDecimal32(index, /*len*/1, scale, &result);
		return result;
	}
	virtual long long getDecimal64(INDEX index, int scale) const override {
		long long result = 0;
		getDecimal64(index, /*len*/1, scale, &result);
		return result;
	}
	virtual int128 getDecimal128(INDEX index, int scale) const override {
		int128 result = 0;
		getDecimal128(index, /*len*/1, scale, &result);
		return result;
	}

	virtual bool getDecimal32(INDEX start, int len, int scale, int *buf) const override {
		return getDecimal(start, len, scale, buf);
	}
	virtual bool getDecimal64(INDEX start, int len, int scale, long long *buf) const override {
		return getDecimal(start, len, scale, buf);
	}
	virtual bool getDecimal128(INDEX start, int len, int scale, int128 *buf) const override {
		return getDecimal(start, len, scale, buf);
	}

	virtual const int* getDecimal32Const(INDEX start, int len, int scale, int *buf) const override {
		getDecimal(start, len, scale, buf);
		return buf;
	}
	virtual const long long* getDecimal64Const(INDEX start, int len, int scale, long long *buf) const override {
		getDecimal(start, len, scale, buf);
		return buf;
	}
	virtual const int128* getDecimal128Const(INDEX start, int len, int scale,
			int128 *buf) const override {
		getDecimal(start, len, scale, buf);
		return buf;
	}

	virtual bool getDecimal32(INDEX *indices, int len, int scale, int *buf) const override {
		return getDecimal(0, len, scale, buf);
	}
	virtual bool getDecimal64(INDEX *indices, int len, int scale, long long *buf) const override {
		return getDecimal(0, len, scale, buf);
	}
	virtual bool getDecimal128(INDEX *indices, int len, int scale, int128 *buf) const override {
		return getDecimal(0, len, scale, buf);
	}

private:
	template <typename R>
	bool getDecimal(INDEX /*start*/, int len, int scale, R *buf) const;

private:
	bool isDefault_;
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

class Int128: public Constant{
public:
	Int128();
	Int128(const unsigned char* data);
	virtual ~Int128(){}
	inline const unsigned char* bytes() const { return uuid_;}
	virtual string getString() const { return toString(uuid_);}
	virtual const Guid getInt128() const { return uuid_;}
	virtual const unsigned char* getBinary() const {return uuid_;}
	virtual bool isNull() const;
	virtual void setNull();
	virtual void nullFill(const ConstantSP& val){
		if(isNull())
			memcpy(uuid_, val->getInt128().bytes(), 16);
	}
	virtual bool isNull(INDEX start, int len, char* buf) const {
		char null=isNull();
		for(int i=0;i<len;++i)
			buf[i]=null;
		return true;
	}
	virtual bool isNull(INDEX* indices, int len, char* buf) const {
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
	virtual bool isValid(INDEX* indices, int len, char* buf) const {
		char valid=!isNull();
		for(int i=0;i<len;++i)
			buf[i]=valid;
		return true;
	}
	virtual int compare(INDEX index, const ConstantSP& target) const;
	virtual void setBinary(const unsigned char* val, int unitLength);
	virtual bool getBinary(INDEX start, int len, int unitLenght, unsigned char* buf) const;
	virtual bool getBinary(INDEX* indices, int len, int unitLength, unsigned char* buf) const;
	virtual const unsigned char* getBinaryConst(INDEX start, int len, int unitLength, unsigned char* buf) const;
	virtual ConstantSP getInstance() const {return new Int128();}
	virtual ConstantSP getValue() const {return new Int128(uuid_);}
	virtual DATA_TYPE getRawType() const { return DT_INT128;}
	virtual long long getAllocatedMemory() const {return sizeof(Int128);}
	virtual IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const {
		short flag = (DF_SCALAR <<8) + getType();
		buffer->write((char)CONSTOBJ);
		buffer->write(flag);
		return buffer->write((const char*)uuid_, 16);
	}
	virtual int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int& numElement, int& partial) const;
	virtual IO_ERR deserialize(DataInputStream* in, INDEX indexStart, int offset, INDEX targetNumElement, INDEX& numElement, int& partial);
	virtual bool assign(const ConstantSP& value);
	static string toString(const unsigned char* data);
	static Int128* parseInt128(const char* str, int len);
	static bool parseInt128(const char* str, size_t len, unsigned char* buf);
	virtual bool set(INDEX index, const ConstantSP& value, INDEX valueIndex){
		memcpy(uuid_, value->getInt128(valueIndex).bytes(), 16);
		return true;
	}

protected:
	mutable unsigned char uuid_[16];
};

class Uuid : public Int128 {
public:
	Uuid(bool newUuid = false);
	Uuid(const unsigned char* uuid);
	Uuid(const char* uuid, int len);
	Uuid(const Uuid& copy);
	virtual ~Uuid(){}
	virtual ConstantSP getInstance() const {return new Uuid(false);}
	virtual ConstantSP getValue() const {return new Uuid(uuid_);}
	virtual DATA_TYPE getRawType() const { return DT_INT128;}
	virtual string getString() const { return Guid::getString(uuid_);}
	static Uuid* parseUuid(const char* str, int len);
	static bool parseUuid(const char* str, size_t len, unsigned char* buf);
};

# ifndef OPCUA
class IPAddr : public Int128 {
public:
	IPAddr();
	IPAddr(const char* ip, int len);
	IPAddr(const unsigned char* data);
	virtual ~IPAddr(){}
	virtual ConstantSP getInstance() const {return new IPAddr();}
	virtual ConstantSP getValue() const {return new IPAddr(uuid_);}
	virtual DATA_TYPE getRawType() const { return DT_INT128;}
	virtual string getString() const { return toString(uuid_);}
	static string toString(const unsigned char* data);
	static IPAddr* parseIPAddr(const char* str, int len);
	static bool parseIPAddr(const char* str, size_t len, unsigned char* buf);

private:
	static bool parseIP4(const char* str, size_t len, unsigned char* buf);
	static bool parseIP6(const char* str, size_t len, unsigned char* buf);
};
# endif
class DoublePair {
public:
	DoublePair(bool null = true);
	DoublePair(double x, double y);
	DoublePair(const unsigned char* data);

	inline bool operator==(const DoublePair &other) const {
		return	data_.doubleVal[0] == other.data_.doubleVal[0] && data_.doubleVal[1] == other.data_.doubleVal[1];
	}

	inline bool operator!=(const DoublePair &other) const {
		return	data_.doubleVal[0] != other.data_.doubleVal[0] || data_.doubleVal[1] != other.data_.doubleVal[1];
	}

	inline bool isZero() const {
		return data_.longVal[0] == 0 && data_.longVal[1] == 0;
	}

	inline bool isNull() const {
		return data_.doubleVal[0] == DBL_NMIN || data_.doubleVal[1] == DBL_NMIN;
	}

	inline bool isValid() const {
		return data_.doubleVal[0] != DBL_NMIN && data_.doubleVal[1] != DBL_NMIN;
	}

	inline const unsigned char* bytes() const {
		return data_.uuid;
	}

	inline double x() const {
		return data_.doubleVal[0];
	}

	inline double y() const {
		return data_.doubleVal[1];
	}

private:
	typedef union {
		unsigned char uuid[16];
		double doubleVal[2];
		float floatVal[4];
		long long longVal[2];
	} U16;

	U16 data_;
};

class Double2 : public Int128 {
public:
	virtual bool isNull() const;
	virtual void setNull();

protected:
	Double2();
	Double2(double x, double y);
	Double2(const unsigned char* data);
};

class Complex : public Double2 {
public:
	Complex() : Double2(){setType(DT_COMPLEX);}
	Complex(double real, double image) : Double2(real, image){setType(DT_COMPLEX);}
	Complex(const unsigned char* data) : Double2(data){setType(DT_COMPLEX);}
	virtual ~Complex(){}
	virtual ConstantSP getInstance() const {return new Complex();}
	virtual ConstantSP getValue() const {return new Complex(uuid_);}
	virtual DATA_TYPE getRawType() const { return DT_INT128;}
	virtual string getString() const { return toString(uuid_);}
	double getReal() const;
	double getImage() const;
	static string toString(const unsigned char* data);
};

class Point : public Double2 {
public:
	Point() : Double2(){setType(DT_POINT);}
	Point(double x, double y) : Double2(x, y){setType(DT_POINT);}
	Point(const unsigned char* data) : Double2(data){setType(DT_POINT);}
	virtual ~Point(){}
	virtual ConstantSP getInstance() const {return new Point();}
	virtual ConstantSP getValue() const {return new Point(uuid_);}
	virtual DATA_TYPE getRawType() const { return DT_INT128;}
	virtual string getString() const { return toString(uuid_);}
	double getX() const;
	double getY() const;
	static string toString(const unsigned char* data);
};

class String: public Constant{
public:
	String(DolphinString val=""): Constant(DF_SCALAR, DT_STRING, LITERAL), blob_(false), val_(val){}
	String(DolphinString val, bool blob): Constant(DF_SCALAR, blob ? DT_BLOB : DT_STRING, LITERAL), blob_(blob), val_(val){}
	virtual ~String(){}
    virtual bool isLargeConstant() const { return val_.size()>1024;}
	virtual char getBool() const {throw IncompatibleTypeException(DT_BOOL, internalType());}
	virtual char getChar() const {throw IncompatibleTypeException(DT_CHAR, internalType());}
	virtual short getShort() const {throw IncompatibleTypeException(DT_SHORT, internalType());}
	virtual int getInt() const {throw IncompatibleTypeException(DT_INT, internalType());}
	virtual long long getLong() const {throw IncompatibleTypeException(DT_LONG, internalType());}
	virtual INDEX getIndex() const {throw IncompatibleTypeException(DT_INDEX, internalType());}
	virtual float getFloat() const {throw IncompatibleTypeException(DT_FLOAT, internalType());}
	virtual double getDouble() const {throw IncompatibleTypeException(DT_DOUBLE, internalType());}

	virtual int getDecimal32(int scale) const override {
		throw IncompatibleTypeException(DT_DECIMAL32, internalType());
	}
	virtual long long getDecimal64(int scale) const override {
		throw IncompatibleTypeException(DT_DECIMAL64, internalType());
	}
	virtual int128 getDecimal128(int scale) const override {
		throw IncompatibleTypeException(DT_DECIMAL128, internalType());
	}
	virtual int getDecimal32(INDEX index, int scale) const override {
		throw IncompatibleTypeException(DT_DECIMAL32, internalType());
	}
	virtual long long getDecimal64(INDEX index, int scale) const override {
		throw IncompatibleTypeException(DT_DECIMAL64, internalType());
	}
	virtual int128 getDecimal128(INDEX index, int scale) const override {
		throw IncompatibleTypeException(DT_DECIMAL128, internalType());
	}
	virtual bool getDecimal32(INDEX start, int len, int scale, int *buf) const override {
		throw IncompatibleTypeException(DT_DECIMAL32, internalType());
	}
	virtual bool getDecimal64(INDEX start, int len, int scale, long long *buf) const override {
		throw IncompatibleTypeException(DT_DECIMAL64, internalType());
	}
	virtual bool getDecimal128(INDEX start, int len, int scale, int128 *buf) const override {
		throw IncompatibleTypeException(DT_DECIMAL128, internalType());
	}
	virtual bool getDecimal32(INDEX *indices, int len, int scale, int *buf) const override {
		throw IncompatibleTypeException(DT_DECIMAL32, internalType());
	}
	virtual bool getDecimal64(INDEX *indices, int len, int scale, long long *buf) const override {
		throw IncompatibleTypeException(DT_DECIMAL64, internalType());
	}
	virtual bool getDecimal128(INDEX *indices, int len, int scale, int128 *buf) const override {
		throw IncompatibleTypeException(DT_DECIMAL128, internalType());
	}
	virtual const int* getDecimal32Const(INDEX start, int len, int scale, int *buf) const override {
		throw IncompatibleTypeException(DT_DECIMAL32, internalType());
	}
	virtual const long long* getDecimal64Const(INDEX start, int len, int scale, long long *buf) const override {
		throw IncompatibleTypeException(DT_DECIMAL64, internalType());
	}
	virtual const int128* getDecimal128Const(INDEX start, int len, int scale,
			int128 *buf) const override {
		throw IncompatibleTypeException(DT_DECIMAL128, internalType());
	}

	virtual string getString() const {return val_.getString();}
	virtual string getScript() const {return Util::literalConstant(val_.getString());}
	virtual const DolphinString& getStringRef() const {return val_;}
	virtual const DolphinString& getStringRef(INDEX index) const {return val_;}
	virtual bool isNull() const {return val_.empty();}
	virtual void setString(const DolphinString& val) {val_=val;}
	virtual ConstantSP get(const ConstantSP& index) const;
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
	virtual bool getSymbol(INDEX* indices, int len, int* buf, SymbolBase* symBase,bool insertIfNotThere) const {
		int tmp=insertIfNotThere?symBase->findAndInsert(val_):symBase->find(val_);
		for(int i=0;i<len;++i)
			buf[i]= indices[i] >= 0 ? tmp : 0;
		return true;
	}
	virtual bool getSymbol(INDEX start, int len, int* buf, SymbolBase* symBase,bool insertIfNotThere) const {
		int tmp=insertIfNotThere?symBase->findAndInsert(val_):symBase->find(val_);
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return true;
	}
	virtual bool getString(INDEX start, int len, DolphinString** buf) const {
		for(int i=0;i<len;++i)
			buf[i]=&val_;
		return true;
	}
	virtual bool getString(INDEX start, int len, char** buf) const {
		char* tmp = (char*)val_.c_str();
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return true;
	}
	virtual DolphinString** getStringConst(INDEX start, int len, DolphinString** buf) const {
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
	virtual ConstantSP getInstance() const {return ConstantSP(new String("", blob_));}
	virtual ConstantSP getValue() const {return ConstantSP(new String(val_, blob_));}
	virtual DATA_TYPE getRawType() const { return internalType();}
	virtual long long getAllocatedMemory() const {return sizeof(DolphinString);}
	virtual IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const;
	virtual int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int& numElement, int& partial) const;
	virtual IO_ERR deserialize(DataInputStream* in, INDEX indexStart, int offset, INDEX targetNumElement, INDEX& numElement, int& partial);
	virtual int compare(INDEX index, const ConstantSP& target) const {
		return val_.compare(target->getString());
	}
	virtual bool assign(const ConstantSP& value);
	virtual bool set(INDEX index, const ConstantSP& value, INDEX valueIndex){ val_ = value->getStringRef(valueIndex);return true;}

protected:
	inline DATA_TYPE internalType() const { return blob_ ? DT_BLOB : DT_STRING;}

private:
	bool blob_;
	mutable DolphinString val_;
};

class MetaCode : public String {
public:
	MetaCode(const ObjectSP& code) : String("< " + code->getScript() +" >"), code_(code){setTypeAndCategory(DT_CODE, SYSTEM);}
	virtual DATA_TYPE getRawType() const { return DT_CODE;}
	virtual string getScript() const;
	virtual bool copyable() const {return false;}
	virtual bool containNotMarshallableObject() const {return true;}
	virtual IO_ERR serialize(Heap* pHeap, const ByteArrayCodeBufferSP& buffer) const;
	ObjectSP getCode() const { return code_;}
	virtual void collectUserDefinedFunctions(unordered_map<string,FunctionDef*>& functionDefs) const;
	virtual bool isLargeConstant() const { return false;}
	IO_ERR deserialize(DataInputStream* in, INDEX indexStart, int offset, INDEX targetNumElement, INDEX& numElement, int& partial) override;

private:
	ObjectSP code_;
};

class DataSource : public String {
public:
	DataSource(const ObjectSP& code, long long cacheId = -1, bool isTable = true, bool localMode = false) : String("DataSource< " + code->getScript() +" >"), code_(1, code),
		parentId_(-1), id_(cacheId), action_(-1), isTable_(isTable), localMode_(localMode){setTypeAndCategory(DT_DATASOURCE, SYSTEM);}
	DataSource(const vector<ObjectSP>& code, long long cacheId = -1, bool isTable = true, bool localMode = false);
	DataSource(Session* session, const DataInputStreamSP& in);
	virtual DATA_TYPE getRawType() const { return DT_DATASOURCE;}
	virtual string getScript() const;
	virtual bool copyable() const {return false;}
	virtual bool containNotMarshallableObject() const {return true;}
	virtual void collectUserDefinedFunctions(unordered_map<string,FunctionDef*>& functionDefs) const;
	virtual IO_ERR serialize(Heap* pHeap, const ByteArrayCodeBufferSP& buffer) const;
	virtual ConstantSP getReference(Heap* pHeap);
	ObjectSP getCode() const { return code_[0];}
	ObjectSP getCode(int index) const { return code_[index];}
	int getObjectCount() const { return code_.size();}
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
	virtual bool isLargeConstant() const { return false;}
    void setTaskFirstLevel(bool flag) {isTaskFirstLevel_ = flag;}
    bool isTaskFirstLevel() const {return isTaskFirstLevel_;}
private:
	vector<ObjectSP> code_;
	vector<FunctionDefSP> transformers_;
	DomainSitePoolSP sites_;
	long long parentId_;
	long long id_;
	char action_; // -1: do nothing, 1: enable cache, i.e. set cache id, 0:clear cache after use.
	bool isTable_; // check if it can be used in SQL statement as the source of a table.
	bool localMode_;
    bool isTaskFirstLevel_ = true;
};


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

class Resource : public String{
public:
	Resource(long long handle, const string& desc, const FunctionDefSP& onClose, Session* session) : String(desc), handle_(handle), onClose_(onClose), session_(session){setTypeAndCategory(DT_RESOURCE, SYSTEM);}
	virtual ~Resource();
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
	AbstractScalar(DATA_TYPE dt, DATA_CATEGORY dc, T val=0): Constant(DF_SCALAR, dt, dc), val_(val){}
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
	virtual void setString(const DolphinString& val){}
	virtual bool isNull() const = 0;

	virtual ConstantSP get(const ConstantSP& index) const override {
		if (index->isScalar()) {
			return getValue();
		}
		else if (!index->isVector()) {
			throw RuntimeException("Scalar get only support index scalar and index vector yet.");
		}
		ConstantSP vec = Util::createVector(getType(), index->size());
		((Vector*)vec.get())->fill(0, vec->size(), getValue());

		if (((Vector*)index.get())->min()->getInt() >= 0) {
			return vec;
		}
		INDEX len = index->size();
		if(index->isIndexArray()){
			UINDEX* bufIndex=(UINDEX*)index->getIndexArray();
			for(INDEX i=0;i<len;++i)
				if (bufIndex[i]<0) {
					vec->setNull(i);
				}
		}
		else{
			UINDEX bufIndex[Util::BUF_SIZE];
			const UINDEX* pbufIndex;
			INDEX start=0;
			int count=0;
			int i;
			while(start<len){
				count=std::min(len-start,Util::BUF_SIZE);
				pbufIndex = (const UINDEX*)index->getIndexConst(start,count,(INDEX*)bufIndex);
				for(i=0;i<count;++i) {
					if (pbufIndex[i]<0) {
						vec->setNull(i);
					}
				}
				start+=count;
			}
		}
		return vec;
	}

public:  /// {get,set}Decimal{32,64,128}
	virtual int getDecimal32(int scale) const override {
		return getDecimal32(/*index*/0, scale);
	}
	virtual long long getDecimal64(int scale) const override {
		return getDecimal64(/*index*/0, scale);
	}
	virtual int128 getDecimal128(int scale) const override {
		return getDecimal128(/*index*/0, scale);
	}

	virtual int getDecimal32(INDEX index, int scale) const override {
		int result = 0;
		getDecimal32(index, /*len*/1, scale, &result);
		return result;
	}
	virtual long long getDecimal64(INDEX index, int scale) const override {
		long long result = 0;
		getDecimal64(index, /*len*/1, scale, &result);
		return result;
	}
	virtual int128 getDecimal128(INDEX index, int scale) const override {
		int128 result = 0;
		getDecimal128(index, /*len*/1, scale, &result);
		return result;
	}

	virtual bool getDecimal32(INDEX start, int len, int scale, int *buf) const override {
		return getDecimal(start, len, scale, buf);
	}
	virtual bool getDecimal64(INDEX start, int len, int scale, long long *buf) const override {
		return getDecimal(start, len, scale, buf);
	}
	virtual bool getDecimal128(INDEX start, int len, int scale, int128 *buf) const override {
		return getDecimal(start, len, scale, buf);
	}

	virtual bool getDecimal32(INDEX *indices, int len, int scale, int *buf) const override {
		return getDecimal(/*start*/0, len, scale, buf);
	}
	virtual bool getDecimal64(INDEX *indices, int len, int scale, long long *buf) const override {
		return getDecimal(/*start*/0, len, scale, buf);
	}
	virtual bool getDecimal128(INDEX *indices, int len, int scale, int128 *buf) const override {
		return getDecimal(/*start*/0, len, scale, buf);
	}

	virtual const int* getDecimal32Const(INDEX start, int len, int scale, int *buf) const override {
		return getDecimal32Buffer(start, len, scale, buf);
	}
	virtual const long long* getDecimal64Const(INDEX start, int len, int scale, long long *buf) const override {
		return getDecimal64Buffer(start, len, scale, buf);
	}
	virtual const int128* getDecimal128Const(INDEX start, int len, int scale,
			int128 *buf) const override {
		return getDecimal128Buffer(start, len, scale, buf);
	}

	virtual int* getDecimal32Buffer(INDEX start, int len, int scale, int *buf) const override {
		getDecimal(start, len, scale, buf);
		return buf;
	}
	virtual long long* getDecimal64Buffer(INDEX start, int len, int scale, long long *buf) const override {
		getDecimal(start, len, scale, buf);
		return buf;
	}
	virtual int128* getDecimal128Buffer(INDEX start, int len, int scale,
			int128 *buf) const override {
		getDecimal(start, len, scale, buf);
		return buf;
	}

	virtual void setDecimal32(INDEX index, int scale, int val) override {
		setDecimal32(index, /*len*/1, scale, &val);
	}
	virtual void setDecimal64(INDEX index, int scale, long long val) override {
		setDecimal64(index, /*len*/1, scale, &val);
	}
	virtual void setDecimal128(INDEX index, int scale, int128 val) override {
		setDecimal128(index, /*len*/1, scale, &val);
	}

	virtual bool setDecimal32(INDEX start, int len, int scale, const int *buf) override {
		return setDecimal(start, len, scale, buf);
	}
	virtual bool setDecimal64(INDEX start, int len, int scale, const long long *buf) override {
		return setDecimal(start, len, scale, buf);
	}
	virtual bool setDecimal128(INDEX start, int len, int scale, const int128 *buf) override {
		return setDecimal(start, len, scale, buf);
	}

private:
	template <typename R>
	bool getDecimal(INDEX /*start*/, int len, int scale, R *buf) const;

	template <typename R>
	bool setDecimal(INDEX /*start*/, int len, int scale, const R *buf);

public:
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
	virtual bool isNull(INDEX* indices, int len, char* buf) const {
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
	virtual bool isValid(INDEX* indices, int len, char* buf) const {
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
	virtual bool getBool(INDEX* indices, int len, char* buf) const {
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
	virtual bool getChar(INDEX* indices, int len, char* buf) const {
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
	virtual bool getShort(INDEX* indices, int len, short* buf) const {
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
	virtual bool getInt(INDEX* indices, int len, int* buf) const {
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
	virtual bool getLong(INDEX* indices, int len, long long* buf) const {
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
	virtual bool getIndex(INDEX* indices, int len, INDEX* buf) const {
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
	virtual bool getFloat(INDEX* indices, int len, float* buf) const {
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
	virtual bool getDouble(INDEX* indices, int len, double* buf) const {
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
		if(isNull()){
			return target->isNull() ? 0 : -1;
		}
		else if (target->isNull()) {
			return 1;
		}
		if(getCategory() == FLOATING){
			T val= (T)target->getDouble();
			return val_==val?0:(val_<val?-1:1);
		}
		else{
			T val= (T)target->getLong();
			return val_==val?0:(val_<val?-1:1);
		}
	}

	virtual bool assign(const ConstantSP& value) {
		if(value->isNull(0)){
			setNull();
			return true;
		}
		else if(getCategory() == FLOATING){
			val_ = (T)value->getDouble();
			return true;
		}
		else{
			val_ = (T)value->getLong();
			return true;
		}
	}

protected:
	T val_;
};

class Bool: public AbstractScalar<char>{
public:
	Bool(char val=0):AbstractScalar(DT_BOOL, LOGICAL, val){}
	virtual ~Bool(){}
	virtual bool isNull() const {return val_==CHAR_MIN;}
	virtual void setNull(){val_= CHAR_MIN;}
	virtual void setBool(char val){ val_ = val;}
	virtual DATA_TYPE getRawType() const { return DT_BOOL;}
	virtual char getBool() const override {
		if (val_ == CHAR_MIN) {
			return CHAR_MIN;
		}
		return static_cast<bool>(val_);
	}
	virtual ConstantSP getInstance() const {return ConstantSP(new Bool());}
	virtual ConstantSP getValue() const {return ConstantSP(new Bool(val_));}
	virtual string getString() const { return toString(val_);}
	virtual bool add(INDEX start, INDEX length, long long inc) { return false;}
	virtual bool add(INDEX start, INDEX length, double inc) { return false;}
	virtual IO_ERR deserialize(DataInputStream* in, INDEX indexStart, int offset, INDEX targetNumElement, INDEX& numElement, int& partial);
	static Bool* parseBool(const string& str);
	static string toString(char val){
		if(val == CHAR_MIN)
			return "";
		else if(val)
			return "1";
		else
			return "0";
	}
	virtual bool assign(const ConstantSP& value);
	virtual bool set(INDEX index, const ConstantSP& value, INDEX valueIndex){ val_ = value->getBool(valueIndex);return true;}
};

class Char: public AbstractScalar<char>{
public:
	Char(char val=0):AbstractScalar(DT_CHAR, INTEGRAL, val){}
	virtual ~Char(){}
	virtual bool isNull() const {return val_==CHAR_MIN;}
	virtual void setNull(){val_=CHAR_MIN;}
	virtual void setChar(char val){ val_ = val;}
	virtual DATA_TYPE getRawType() const { return DT_CHAR;}
	virtual char getChar() const override { return val_; }
	virtual ConstantSP getInstance() const {return ConstantSP(new Char());}
	virtual ConstantSP getValue() const {return ConstantSP(new Char(val_));}
	virtual string getString() const { return toString(val_);}
	virtual string getScript() const;
	virtual IO_ERR deserialize(DataInputStream* in, INDEX indexStart, int offset, INDEX targetNumElement, INDEX& numElement, int& partial);
	static Char* parseChar(const string& str);
	static string toString(char val);
	virtual bool set(INDEX index, const ConstantSP& value, INDEX valueIndex){ val_ = value->getChar(valueIndex);return true;}
};

class Short: public AbstractScalar<short>{
public:
	Short(short val=0):AbstractScalar(DT_SHORT, INTEGRAL, val){}
	virtual ~Short(){}
	virtual bool isNull() const {return val_==SHRT_MIN;}
	virtual void setNull(){val_=SHRT_MIN;}
	virtual void setShort(short val){ val_ = val;}
	virtual DATA_TYPE getRawType() const { return DT_SHORT;}
	virtual short getShort() const override { return val_; }
	virtual ConstantSP getInstance() const {return ConstantSP(new Short());}
	virtual ConstantSP getValue() const {return ConstantSP(new Short(val_));}
	virtual string getString() const { return toString(val_);}
	virtual IO_ERR deserialize(DataInputStream* in, INDEX indexStart, int offset, INDEX targetNumElement, INDEX& numElement, int& partial);
	static Short* parseShort(const string& str);
	static string toString(short val);
	virtual bool set(INDEX index, const ConstantSP& value, INDEX valueIndex){ val_ = value->getShort(valueIndex);return true;}
};

class Int: public AbstractScalar<int>{
public:
	Int(int val=0):AbstractScalar(DT_INT, INTEGRAL, val){}
	virtual ~Int(){}
	virtual bool isNull() const {return val_==INT_MIN;}
	virtual void setNull(){val_=INT_MIN;}
	virtual void setInt(int val){ val_ = val;}
	virtual DATA_TYPE getRawType() const { return DT_INT;}
	virtual int getInt() const override { return val_; }
	virtual ConstantSP getInstance() const {return ConstantSP(new Int());}
	virtual ConstantSP getValue() const {return ConstantSP(new Int(val_));}
	virtual string getString() const { return toString(val_);}
	virtual IO_ERR deserialize(DataInputStream* in, INDEX indexStart, int offset, INDEX targetNumElement, INDEX& numElement, int& partial);
	static Int* parseInt(const string& str);
	static string toString(int val);
	virtual bool set(INDEX index, const ConstantSP& value, INDEX valueIndex){ val_ = value->getInt(valueIndex);return true;}
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

class Duration : public Int {
public:
	Duration() = default;
	Duration(DURATION unit, int val);
	Duration(int exchange, int val);
	Duration(const string& exchange, int val);
	Duration(FREQUENCY freq);
	virtual ~Duration(){}
	virtual DATA_TYPE getRawType() const { return DT_DURATION;}
	virtual long long getLong() const;
	virtual string getScript() const {return getString();}
	virtual ConstantSP getValue() const;
	virtual ConstantSP getInstance() const;
	virtual string getString() const;
	virtual int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int& numElement, int& partial) const;
	IO_ERR deserialize(DataInputStream* in, INDEX indexStart, int offset, INDEX targetNumElement, INDEX& numElement, int& partial);
	inline DURATION getUnit() const { return unit_;}
	inline DURATION unit() const { return unit_; }
	string getExchangeName() const;
	inline int getExchangeInt() const { return exchange_;}
	inline int getDuration() const { return val_;}
    inline int length() const { return val_; }
    double years() const;
    double months() const;
    double weeks() const;
    double days() const;
    FREQUENCY frequency() const;
    Duration& operator+=(const Duration&);
    Duration& operator-=(const Duration& d);
    Duration& operator*=(int n);
    Duration& operator/=(int);
	long long toDuration(DURATION newDuration) const;
	bool convertibleTo(DURATION to) const { return convertible(unit_, to);}
	static bool convertible(DURATION from, DURATION to);
	static long long convertRatio(DURATION from, DURATION to);
	static Duration* parseDuration(const string& str);
	static string toString(long long val);
	static DURATION getDuration(DATA_TYPE type);
	static DURATION getDuration(const string& unit);
	static string getExchangeName(int exchange);

private:
	static const string durationSymbols_[11];
	static const long long durationRatios_[12][12];
	DURATION unit_;
	int exchange_;
};

template <typename T>
inline Duration operator*(T n, DURATION unit){
	return Duration(unit, n);
}

template <typename T>
inline Duration operator*(DURATION unit, T n){
	return Duration(unit, n);
}

inline Duration operator-(const Duration& d) { return Duration(d.unit(), -d.length());}

inline Duration operator*(int n, const Duration& d) { return Duration(d.unit(), d.length() * n);}

inline Duration operator*(const Duration& d, int n) { return Duration(d.unit(), d.length() * n);}

inline Duration operator/(const Duration& d, int n){
	Duration result = d;
	result /= n;
	return result;
}

inline Duration operator+(const Duration& d1, const Duration& d2){
	Duration result = d1;
	result += d2;
	return result;
}

inline Duration operator-(const Duration& d1, const Duration& d2) {
	Duration result = d1;
	result -= d2;
	return result;
}

bool operator<(const Duration&, const Duration&);

bool operator==(const Duration&, const Duration&);

inline bool operator!=(const Duration& p1, const Duration& p2){
	return !(p1 == p2);
}

inline bool operator>(const Duration& p1, const Duration& p2){
	return p2 < p1;
}

inline bool operator<=(const Duration& p1, const Duration& p2) {
	return !(p2 < p1);
}

inline bool operator>=(const Duration& p1, const Duration& p2) {
	return !(p1 < p2);
}

class Long: public AbstractScalar<long long>{
public:
	Long(long long val=0):AbstractScalar(DT_LONG, INTEGRAL, val){}
	virtual ~Long(){}
	virtual bool isNull() const {return val_==LLONG_MIN;}
	virtual void setNull(){val_=LLONG_MIN;}
	virtual void setLong(long long val){ val_ = val;}
	virtual DATA_TYPE getRawType() const { return DT_LONG;}
	virtual long long getLong() const override { return val_; }
	virtual ConstantSP getInstance() const {return ConstantSP(new Long());}
	virtual ConstantSP getValue() const {return ConstantSP(new Long(val_));}
	virtual string getString() const { return toString(val_);}
	virtual IO_ERR deserialize(DataInputStream* in, INDEX indexStart, int offset, INDEX targetNumElement, INDEX& numElement, int& partial);
	static Long* parseLong(const string& str);
	static string toString(long long val);
	virtual bool set(INDEX index, const ConstantSP& value, INDEX valueIndex){ val_ = value->getLong(valueIndex);return true;}
};

class Float: public AbstractScalar<float>{
public:
	Float(float val=0):AbstractScalar(DT_FLOAT, FLOATING, val){}
	virtual ~Float(){}
	virtual bool isNull() const {return val_==FLT_NMIN;}
	virtual void setNull(){val_=FLT_NMIN;}
	virtual void setFloat(float val){ val_ = val;}
	virtual DATA_TYPE getRawType() const { return DT_FLOAT;}
	virtual float getFloat() const override { return val_; }
	virtual ConstantSP getInstance() const {return ConstantSP(new Float());}
	virtual ConstantSP getValue() const {return ConstantSP(new Float(val_));}
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
	virtual IO_ERR deserialize(DataInputStream* in, INDEX indexStart, int offset, INDEX targetNumElement, INDEX& numElement, int& partial);
	static Float* parseFloat(const string& str);
	static string toString(float val);
	virtual bool set(INDEX index, const ConstantSP& value, INDEX valueIndex){ val_ = value->getFloat(valueIndex);return true;}
};

class Double: public AbstractScalar<double>{
public:
	Double(double val=0):AbstractScalar(DT_DOUBLE, FLOATING, (std::isnan(val)|| std::isinf(val)) ? DBL_NMIN : val){}
	virtual ~Double(){}
	virtual bool isNull() const {return val_==DBL_NMIN;}
	virtual void setNull(){val_=DBL_NMIN;}
	virtual void setDouble(double val){ val_ = (std::isnan(val)|| std::isinf(val)) ? DBL_NMIN : val; }
	virtual DATA_TYPE getRawType() const { return DT_DOUBLE;}
	virtual double getDouble() const override { return val_; }
	virtual ConstantSP getInstance() const {return ConstantSP(new Double());}
	virtual ConstantSP getValue() const {return ConstantSP(new Double(val_));}
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
	virtual IO_ERR deserialize(DataInputStream* in, INDEX indexStart, int offset, INDEX targetNumElement, INDEX& numElement, int& partial);
	static Double* parseDouble(const string& str);
	static string toString(double val);
	virtual bool set(INDEX index, const ConstantSP& value, INDEX valueIndex){ val_ = value->getDouble(valueIndex);return true;}
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
	TemporalScalar(DATA_TYPE type, int val=0):Int(val){setTypeAndCategory(type, TEMPORAL);}
	virtual ~TemporalScalar(){}
};

class Date:public TemporalScalar{
public:
	Date(int val=0):TemporalScalar(DT_DATE, val){}
	Date(int year, int month, int day):TemporalScalar(DT_DATE, Util::countDays(year,month,day)){}
	virtual ~Date(){}
	virtual ConstantSP getInstance() const {return ConstantSP(new Date());}
	virtual ConstantSP getValue() const {return ConstantSP(new Date(val_));}
	virtual string getString() const { return toString(val_);}
	static Date* parseDate(const string& str);
	static string toString(int val);

    inline WEEKDAY weekday() const {
    	return WEEKDAY(Util::mod<int>(val_+4,7));
    }

    inline int dayOfMonth() const {
    	int y, m, d;
    	Util::parseDate(val_, y, m, d);
    	return d;
    }

    inline int dayOfYear() const {
    	return Util::getDayOfYear(val_);
    }

    inline int month() const {
    	int y, m, d;
    	Util::parseDate(val_, y, m, d);
    	return m;
    }

    inline int year() const{
    	return Util::parseYear(val_);
    }

    inline int serialNumber() const { return val_;}

    inline Date& operator+=(int days) {
    	val_ += days;
    	return *this;
    }
	// increments date by the given period
	inline Date& operator+=(const Duration& duration){
		DURATION unit = duration.getUnit();
		val_ = advance(*this, duration.getDuration(),
				unit < DURATION::TDAY ? unit : ((DURATION)(duration.getExchangeInt()))).serialNumber();
		return *this;
	}

	// decrement date by the given number of days
	inline Date& operator-=(int days) {
		val_ -= days;
		return *this;
	}

	//! decrements date by the given period
	inline Date& operator-=(const Duration& duration){
		DURATION unit = duration.getUnit();
		val_ = advance(*this, -duration.getDuration(),
				unit < DURATION::TDAY ? unit : ((DURATION)(duration.getExchangeInt()))).serialNumber();
		return *this;
	}

	// 1-day pre-increment
	inline Date& operator++(){
		++val_;
		return *this;
	}

	// 1-day post-increment
	inline Date operator++(int ) {
		return Date(val_++);
	}

	// 1-day pre-decrement
	inline Date& operator--(){
		--val_;
		return *this;
	}

	// 1-day post-decrement
	inline Date operator--(int ){
		return Date(val_--);
	}

	// returns a new date incremented by the given number of days
	inline Date operator+(int days) const {
		return Date(serialNumber() + days);
	}

	// returns a new date incremented by the given period
	inline Date operator+(const Duration& duration) const {
		DURATION unit = duration.getUnit();
		return advance(*this, duration.getDuration(),
				unit < DURATION::TDAY ? unit : ((DURATION)(duration.getExchangeInt())));
	}

	// returns a new date decremented by the given number of days
	Date operator-(int days) const {
		return Date(serialNumber() - days);
	}

	// returns a new date decremented by the given period
	inline Date operator-(const Duration& duration) const {
		DURATION unit = duration.getUnit();
		return advance(*this, -duration.getDuration(),
				unit < DURATION::TDAY ? unit : ((DURATION)(duration.getExchangeInt())));
	}

    static Date todaysDate(){
    	struct tm lt;
    	Util::getLocalTime(lt);
    	return Date(1900+lt.tm_year, lt.tm_mon+1, lt.tm_mday);
    }

    // whether the given year is a leap one
    static bool isLeap(int year) {
    	return (year%4==0 && year%100!=0) || year%400==0;
    }

    // last day of the month to which the given date belongs
    static Date endOfMonth(const Date &d) {
    	return Date(Util::getMonthEnd(d.serialNumber()));
    }

    // whether a date is the last day of its month
    static bool isEndOfMonth(const Date &d) {
    	return Util::getMonthEnd(d.serialNumber()) == d.serialNumber();
    }

    // next given weekday following or equal to the given date
    static Date nextWeekday(const Date &d, WEEKDAY w) {
    	WEEKDAY wd = d.weekday();
    	return d + ((wd>w ? 7 : 0) - (int)wd + (int)w);
    }

    // n-th given weekday in the given month and year
    // E.g., the 4th Thursday of March, 1998 was March 26th, 1998.
    static Date nthWeekday(int n, WEEKDAY w, int m, int y);

private:
    static Date advance(const Date &d, int n, DURATION unit);
};

inline int operator-(const Date& d1, const Date& d2) {
	return d1.serialNumber() - d2.serialNumber();
}

inline int daysBetween(const Date& d1, const Date& d2) {
	return d2-d1;
}

inline bool operator==(const Date& d1, const Date& d2) {
	return (d1.serialNumber() == d2.serialNumber());
}

inline bool operator!=(const Date& d1, const Date& d2) {
	return (d1.serialNumber() != d2.serialNumber());
}

inline bool operator<(const Date& d1, const Date& d2) {
	return (d1.serialNumber() < d2.serialNumber());
}

inline bool operator<=(const Date& d1, const Date& d2) {
	return (d1.serialNumber() <= d2.serialNumber());
}

inline bool operator>(const Date& d1, const Date& d2) {
	return (d1.serialNumber() > d2.serialNumber());
}

inline bool operator>=(const Date& d1, const Date& d2) {
	return (d1.serialNumber() >= d2.serialNumber());
}


class Month:public TemporalScalar{
public:
	Month():TemporalScalar(DT_MONTH, 1999*12+11){}
	Month(int val):TemporalScalar(DT_MONTH, val){}
	Month(int year, int month):TemporalScalar(DT_MONTH, year*12+month-1){}
	virtual ~Month(){}
	virtual ConstantSP getInstance() const {return ConstantSP(new Month());}
	virtual ConstantSP getValue() const {return ConstantSP(new Month(val_));}
	virtual string getString() const { return toString(val_);}
	static Month* parseMonth(const string& str);
	static string toString(int val);
};

class Time:public TemporalScalar{
public:
	Time(int val=0):TemporalScalar(DT_TIME, val){}
	Time(int hour, int minute, int second, int milliSecond):TemporalScalar(DT_TIME, ((hour*60+minute)*60+second)*1000+milliSecond){}
	virtual ~Time(){}
	virtual ConstantSP getInstance() const {return ConstantSP(new Time());}
	virtual ConstantSP getValue() const {return ConstantSP(new Time(val_));}
	virtual string getString() const { return toString(val_);}
	virtual void validate();
	static Time* parseTime(const string& str);
	static string toString(int val);
};

class NanoTime:public Long{
public:
	NanoTime(long long val=0):Long(val){setTypeAndCategory(DT_NANOTIME, TEMPORAL);}
	NanoTime(int hour, int minute, int second, int nanoSecond):Long(((hour*60+minute)*60+second)*1000000000ll+ nanoSecond){setTypeAndCategory(DT_NANOTIME, TEMPORAL);}
	virtual ~NanoTime(){}
	virtual ConstantSP getInstance() const {return ConstantSP(new NanoTime());}
	virtual ConstantSP getValue() const {return ConstantSP(new NanoTime(val_));}
	virtual string getString() const { return toString(val_);}
	virtual void validate();
	static NanoTime* parseNanoTime(const string& str);
	static string toString(long long val);
};

class Timestamp:public Long{
public:
	Timestamp(long long val=0):Long(val){setTypeAndCategory(DT_TIMESTAMP, TEMPORAL);}
	Timestamp(int year, int month, int day,int hour, int minute, int second, int milliSecond);
	virtual ~Timestamp(){}
	virtual ConstantSP getInstance() const {return ConstantSP(new Timestamp());}
	virtual ConstantSP getValue() const {return ConstantSP(new Timestamp(val_));}
	virtual string getString() const { return toString(val_);}
	static Timestamp* parseTimestamp(const string& str);
	static string toString(long long val);
};

class NanoTimestamp:public Long{
public:
	NanoTimestamp(long long val=0):Long(val){setTypeAndCategory(DT_NANOTIMESTAMP, TEMPORAL);}
	NanoTimestamp(int year, int month, int day,int hour, int minute, int second, int nanoSecond);
	virtual ~NanoTimestamp(){}
	virtual ConstantSP getInstance() const {return ConstantSP(new NanoTimestamp());}
	virtual ConstantSP getValue() const {return ConstantSP(new NanoTimestamp(val_));}
	virtual string getString() const { return toString(val_);}
	static NanoTimestamp* parseNanoTimestamp(const string& str);
	static string toString(long long val);
};

class Minute:public TemporalScalar{
public:
	Minute(int val=0):TemporalScalar(DT_MINUTE, val){}
	Minute(int hour, int minute):TemporalScalar(DT_MINUTE, hour*60+minute){}
	virtual ~Minute(){}
	virtual ConstantSP getInstance() const {return ConstantSP(new Minute());}
	virtual ConstantSP getValue() const {return ConstantSP(new Minute(val_));}
	virtual string getString() const { return toString(val_);}
	virtual void validate();
	static Minute* parseMinute(const string& str);
	static string toString(int val);
};

class Second:public TemporalScalar{
public:
	Second(int val=0):TemporalScalar(DT_SECOND, val){}
	Second(int hour, int minute,int second):TemporalScalar(DT_SECOND, (hour*60+minute)*60+second){}
	virtual ~Second(){}
	virtual ConstantSP getInstance() const {return ConstantSP(new Second());}
	virtual ConstantSP getValue() const {return ConstantSP(new Second(val_));}
	virtual string getString() const { return toString(val_);}
	virtual void validate();
	static Second* parseSecond(const string& str);
	static string toString(int val);
};

class DateTime:public TemporalScalar{
public:
	DateTime(int val=0):TemporalScalar(DT_DATETIME, val){}
	DateTime(int year, int month, int day, int hour, int minute,int second);
	virtual ~DateTime(){}
	virtual ConstantSP getInstance() const {return ConstantSP(new DateTime());}
	virtual ConstantSP getValue() const {return ConstantSP(new DateTime(val_));}
	virtual string getString() const { return toString(val_);}
	static DateTime* parseDateTime(const string& str);
	static string toString(int val);
};

class DateHour:public TemporalScalar{
public:
	DateHour(int val=0):TemporalScalar(DT_DATEHOUR, val){}
	DateHour(int year, int month, int day, int hour);
	virtual ~DateHour(){}
	virtual ConstantSP getInstance() const {return ConstantSP(new DateHour());}
	virtual ConstantSP getValue() const {return ConstantSP(new DateHour(val_));}
	virtual string getString() const { return toString(val_);}
	static DateHour* parseDateHour(const string& str);
	static string toString(int val);
};


template <typename T>
class Decimal : public Constant {
    static_assert(std::is_same<T, int>::value || std::is_same<T, long long>::value ||
                  std::is_same<T, int128>::value,
                  "only allow to instantiate Decimal<int>, Decimal<long long> and Decimal<int128>");
public:
    using raw_data_t = T;

    Decimal() = delete;

    explicit Decimal(int scale);

    Decimal(int scale, raw_data_t rawData);

    Decimal(const Decimal &other);

    template <typename U>
    Decimal(const Decimal<U> &other);

public:
    static DATA_TYPE type();

    int getScale() const { return scale_; }
    raw_data_t getRawData() const { return rawData_; }
	raw_data_t* getRawDataPointer() { return &rawData_; }
    void setRawData(const raw_data_t data) { rawData_ = data; }

    std::string toString() const;

public:  /// Interface of Constant
    virtual int getExtraParamForType() const override { return scale_; }

    virtual bool isNull() const override;
    virtual void setNull() override;

    virtual DATA_TYPE getRawType() const override { return type(); }

    virtual ConstantSP getInstance() const override;
    virtual ConstantSP getValue() const override;

    virtual std::string getString() const override {
        if (isNull()) {
            return "";
        }
        return toString();
    }

    virtual bool isNull(INDEX start, int len, char *buf) const override;
    virtual bool isValid(INDEX start, int len, char *buf) const override;

    virtual bool set(INDEX index, const ConstantSP &value, INDEX valueIndex) override;

    virtual void nullFill(const ConstantSP &val) override;

public:  /// decimal to float
    virtual float getFloat() const override;
    virtual double getDouble() const override;
    virtual bool getFloat(INDEX start, int len, float *buf) const override;
    virtual bool getDouble(INDEX start, int len, double *buf) const override;
    virtual const float *getFloatConst(INDEX start, int len, float *buf) const override;
    virtual const double *getDoubleConst(INDEX start, int len, double *buf) const override;

public:  /// decimal to integer
    virtual char getChar() const override { return toInteger<char>(CHAR_MIN); }
    virtual short getShort() const override { return toInteger<short>(SHRT_MIN); }
    virtual int getInt() const override { return toInteger<int>(INT_MIN); }
    virtual long long getLong() const override { return toInteger<long long>(LLONG_MIN); }

    virtual bool getChar(INDEX start, int len, char *buf) const override {
        return toInteger<char>(CHAR_MIN, start, len, buf);
    }
    virtual bool getShort(INDEX start, int len, short *buf) const override {
        return toInteger<short>(SHRT_MIN, start, len, buf);
    }
    virtual bool getInt(INDEX start, int len, int *buf) const override {
        return toInteger<int>(INT_MIN, start, len, buf);
    }
    virtual bool getLong(INDEX start, int len, long long *buf) const override {
        return toInteger<long long>(LLONG_MIN, start, len, buf);
    }

    virtual const char *getCharConst(INDEX start, int len, char *buf) const override {
        getChar(start, len, buf);
        return buf;
    }
    virtual const short *getShortConst(INDEX start, int len, short *buf) const override {
        getShort(start, len, buf);
        return buf;
    }
    virtual const int *getIntConst(INDEX start, int len, int *buf) const override {
        getInt(start, len, buf);
        return buf;
    }
    virtual const long long *getLongConst(INDEX start, int len, long long *buf) const override {
        getLong(start, len, buf);
        return buf;
    }

public:  /// {get,set}Binary
    virtual void setBinary(const unsigned char *val, int unitLength) override;
    virtual bool getBinary(INDEX start, int len, int unitLength, unsigned char *buf) const override;
    virtual const unsigned char *getBinaryConst(INDEX start, int len, int unitLength,
                                                unsigned char *buf) const override;

public:  /// {get,set}Decimal{32,64,128}
    virtual int getDecimal32(int scale) const override {
        return getDecimal32(/*index*/0, scale);
    }
    virtual long long getDecimal64(int scale) const override {
        return getDecimal64(/*index*/0, scale);
    }
    virtual int128 getDecimal128(int scale) const override {
        return getDecimal128(/*index*/0, scale);
    }

    virtual int getDecimal32(INDEX index, int scale) const override {
        int result = 0;
        getDecimal32(index, /*len*/1, scale, &result);
        return result;
    }
    virtual long long getDecimal64(INDEX index, int scale) const override {
        long long result = 0;
        getDecimal64(index, /*len*/1, scale, &result);
        return result;
    }
    virtual int128 getDecimal128(INDEX index, int scale) const override {
        int128 result = 0;
        getDecimal128(index, /*len*/1, scale, &result);
        return result;
    }

    virtual bool getDecimal32(INDEX start, int len, int scale, int *buf) const override {
        return getDecimal(start, len, scale, buf);
    }
    virtual bool getDecimal64(INDEX start, int len, int scale, long long *buf) const override {
        return getDecimal(start, len, scale, buf);
    }
    virtual bool getDecimal128(INDEX start, int len, int scale, int128 *buf) const override {
        return getDecimal(start, len, scale, buf);
    }

    virtual bool getDecimal32(INDEX *indices, int len, int scale, int *buf) const override {
        return getDecimal(/*start*/0, len, scale, buf);
    }
    virtual bool getDecimal64(INDEX *indices, int len, int scale, long long *buf) const override {
        return getDecimal(/*start*/0, len, scale, buf);
    }
    virtual bool getDecimal128(INDEX *indices, int len, int scale, int128 *buf) const override {
        return getDecimal(/*start*/0, len, scale, buf);
    }

    virtual const int* getDecimal32Const(INDEX start, int len, int scale, int *buf) const override {
        return getDecimal32Buffer(start, len, scale, buf);
    }
    virtual const long long* getDecimal64Const(INDEX start, int len, int scale, long long *buf) const override {
        return getDecimal64Buffer(start, len, scale, buf);
    }
    virtual const int128* getDecimal128Const(INDEX start, int len, int scale,
                                                           int128 *buf) const override {
        return getDecimal128Buffer(start, len, scale, buf);
    }

    virtual int* getDecimal32Buffer(INDEX start, int len, int scale, int *buf) const override {
        getDecimal(start, len, scale, buf);
        return buf;
    }
    virtual long long* getDecimal64Buffer(INDEX start, int len, int scale, long long *buf) const override {
        getDecimal(start, len, scale, buf);
        return buf;
    }
    virtual int128* getDecimal128Buffer(INDEX start, int len, int scale,
                                                      int128 *buf) const override {
        getDecimal(start, len, scale, buf);
        return buf;
    }

    virtual void setDecimal32(INDEX index, int scale, int val) override {
        setDecimal32(index, /*len*/1, scale, &val);
    }
    virtual void setDecimal64(INDEX index, int scale, long long val) override {
        setDecimal64(index, /*len*/1, scale, &val);
    }
    virtual void setDecimal128(INDEX index, int scale, int128 val) override {
        setDecimal128(index, /*len*/1, scale, &val);
    }

    virtual bool setDecimal32(INDEX start, int len, int scale, const int *buf) override {
        return setDecimal(start, len, scale, buf);
    }
    virtual bool setDecimal64(INDEX start, int len, int scale, const long long *buf) override {
        return setDecimal(start, len, scale, buf);
    }
    virtual bool setDecimal128(INDEX start, int len, int scale, const int128 *buf) override {
        return setDecimal(start, len, scale, buf);
    }

public:
    virtual bool assign(const ConstantSP &value) override;

    virtual int compare(INDEX /*index*/, const ConstantSP &target) const override;

    virtual int serialize(char *buf, int bufSize, INDEX indexStart, int offset,
                          int &numElement, int &partial) const override;

    virtual IO_ERR serialize(const ByteArrayCodeBufferSP &buffer) const override;

    virtual IO_ERR deserialize(DataInputStream *in, INDEX indexStart, int offset, INDEX targetNumElement,
                       	       INDEX &numElement, int &partial) override;

public:
    template <typename U, typename R = typename std::conditional<sizeof(T) >= sizeof(U), T, U>::type>
    void assign(const Decimal<U> &other);

    /// convert integer to decimal
    void assign(short value) { assignInteger(value); }
    void assign(int value) { assignInteger(value); }
    void assign(long value) { assignInteger(value); }
    void assign(long long value) { assignInteger(value); }

    /// convert float to decimal
    void assign(float value) { assignFloat(value); }
    void assign(double value) { assignFloat(value); }

    template <typename U, typename R = typename std::conditional<sizeof(T) >= sizeof(U), T, U>::type>
    int compare(const Decimal<U> &other) const;

private:
    template <typename R>
    bool getDecimal(INDEX /*start*/, int len, int scale, R *buf) const;

    template <typename R>
    bool setDecimal(INDEX /*start*/, int len, int scale, const R *buf);

    template <typename U>
    U toInteger(U nullVal) const;

    template <typename U>
    bool toInteger(U nullVal, INDEX start, int len, U *buf) const;

    template <typename U>
    void assignInteger(U value);

    template <typename U>
    void assignFloat(U value);

private:
    template <typename U> friend class Decimal;

    /**
     * Determines how many decimal digits fraction can have.
     * Valid range: [ 0 : precision_ ].
     */
    int scale_;
    raw_data_t rawData_;
};

using Decimal32 = Decimal<int>;        // int32_t
using Decimal64 = Decimal<long long>;  // int64_t
using Decimal128 = Decimal<int128>;


#endif /* SCALARIMP_H_ */
