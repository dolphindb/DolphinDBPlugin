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

namespace ddb {

using std::ostringstream;

class DataSource;
class Resource;
typedef ObjectPtr<DataSource> DataSourceSP;
typedef ObjectPtr<Resource> ResourceSP;

void initFormatters();

class SWORDFISH_API Void: public Constant{
public:
	Void(bool explicitNull = false, bool isDefault = false);
	inline bool isDefault() const { return isDefault_;}
	ConstantSP getInstance() const override;
	ConstantSP getValue() const override;
	DATA_TYPE getRawType() const override { return DT_VOID;}
	string getString() const override;
	string getScript() const override;
	string getString(Heap* heap) const override;
	string getScript(Heap* heap) const override;
	const DolphinString& getStringRef() const override {return Constant::DEMPTY;}
	char getBool() const override {return CHAR_MIN;}
	char getChar() const override {return CHAR_MIN;}
	short getShort() const override {return SHRT_MIN;}
	int getInt() const override {return INT_MIN;}
	INDEX getIndex() const override {return INDEX_MIN;}
	long long  getLong() const override {return LLONG_MIN;}
	float getFloat() const override {return FLT_NMIN;}
	double getDouble() const override {return DBL_NMIN;}
	const Guid getInt128() const override { return Guid::ZERO;}
	bool isNull() const override {return true;}
	void nullFill(const ConstantSP& val) override{}
	bool isNull(INDEX start, int len, char* buf) const override;
	bool isValid(INDEX start, int len, char* buf) const override;
	bool getBool(INDEX start, int len, char* buf) const override;
	const char* getBoolConst(INDEX start, int len, char* buf) const override;
	bool getChar(INDEX start, int len, char* buf) const override;
	const char* getCharConst(INDEX start, int len, char* buf) const override;
	bool getShort(INDEX start, int len, short* buf) const override;
	const short* getShortConst(INDEX start, int len, short* buf) const override;
	bool getInt(INDEX start, int len, int* buf) const override;
	const int* getIntConst(INDEX start, int len, int* buf) const override;
	bool getLong(INDEX start, int len, long long* buf) const override;
	const long long* getLongConst(INDEX start, int len, long long* buf) const override;
	bool getIndex(INDEX start, int len, INDEX* buf) const override;
	const INDEX* getIndexConst(INDEX start, int len, INDEX* buf) const override;
	bool getFloat(INDEX start, int len, float* buf) const override;
	const float* getFloatConst(INDEX start, int len, float* buf) const override;
	bool getDouble(INDEX start, int len, double* buf) const override;
	const double* getDoubleConst(INDEX start, int len, double* buf) const override;
	bool getSymbol(INDEX start, int len, int* buf, SymbolBase* symBase,bool insertIfNotThere) const override;
	const int* getSymbolConst(INDEX start, int len, int* buf, SymbolBase* symBase, bool insertIfNotThere) const override;
	DolphinString** getStringConst(INDEX start, int len, DolphinString** buf) const override;
	char** getStringConst(INDEX start, int len, char** buf) const override;
	bool getBinary(INDEX start, int len, int unitLength, unsigned char* buf) const override;
	const unsigned char* getBinaryConst(INDEX start, int len, int unitLength, unsigned char* buf) const override;
	bool isNull(INDEX* indices, int len, char* buf) const override;
	bool isValid(INDEX* indices, int len, char* buf) const override;
	bool getBool(INDEX* indices, int len, char* buf) const override;
	bool getChar(INDEX* indices, int len,char* buf) const override;
	bool getShort(INDEX* indices, int len, short* buf) const override;
	bool getInt(INDEX* indices, int len, int* buf) const override;
	bool getLong(INDEX* indices, int len, long long* buf) const override;
	bool getIndex(INDEX* indices, int len, INDEX* buf) const override;
	bool getFloat(INDEX* indices, int len, float* buf) const override;
	bool getDouble(INDEX* indices, int len, double* buf) const override;
	bool getSymbol(INDEX* indices, int len, int* buf, SymbolBase* symBase,bool insertIfNotThere) const override;
	bool getString(INDEX* indices, int len, DolphinString** buf) const override;
	bool getString(INDEX* indices, int len, char** buf) const override;
	bool getBinary(INDEX* indices, int len, int unitLength, unsigned char* buf) const override;
	long long getAllocatedMemory() const override;
	IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const override;
	int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int& numElement, int& partial) const override;
	IO_ERR deserialize(DataInputStream* in, INDEX indexStart, int offset, INDEX targetNumElement, INDEX& numElement, int& partial) override;
	int compare(INDEX index, const ConstantSP& target) const override {return target->getType() == DT_VOID ? 0 : -1;}
	uint64_t hash() const override;
	bool equal(const ConstantSP& other) const override;

public:  /// {get,set}Decimal{32,64}
	int getDecimal32(int scale) const override {
		return INT_MIN;
	}
	long long getDecimal64(int scale) const override {
		return LLONG_MIN;
	}
	int128 getDecimal128(int scale) const override {
		return int128MinValue();
	}

	int getDecimal32(INDEX index, int scale) const override {
		int result = 0;
		getDecimal32(index, /*len*/1, scale, &result);
		return result;
	}
	long long getDecimal64(INDEX index, int scale) const override {
		long long result = 0;
		getDecimal64(index, /*len*/1, scale, &result);
		return result;
	}
	int128 getDecimal128(INDEX index, int scale) const override {
		int128 result = 0;
		getDecimal128(index, /*len*/1, scale, &result);
		return result;
	}

	bool getDecimal32(INDEX start, int len, int scale, int *buf) const override {
		return getDecimal(start, len, scale, buf);
	}
	bool getDecimal64(INDEX start, int len, int scale, long long *buf) const override {
		return getDecimal(start, len, scale, buf);
	}
	bool getDecimal128(INDEX start, int len, int scale, int128 *buf) const override {
		return getDecimal(start, len, scale, buf);
	}

	const int* getDecimal32Const(INDEX start, int len, int scale, int *buf) const override {
		getDecimal(start, len, scale, buf);
		return buf;
	}
	const long long* getDecimal64Const(INDEX start, int len, int scale, long long *buf) const override {
		getDecimal(start, len, scale, buf);
		return buf;
	}
	const int128* getDecimal128Const(INDEX start, int len, int scale,
			int128 *buf) const override {
		getDecimal(start, len, scale, buf);
		return buf;
	}

	bool getDecimal32(INDEX *indices, int len, int scale, int *buf) const override {
		return getDecimal(0, len, scale, buf);
	}
	bool getDecimal64(INDEX *indices, int len, int scale, long long *buf) const override {
		return getDecimal(0, len, scale, buf);
	}
	bool getDecimal128(INDEX *indices, int len, int scale, int128 *buf) const override {
		return getDecimal(0, len, scale, buf);
	}

private:
	template <typename R>
	bool getDecimal(INDEX /*start*/, int len, int scale, R *buf) const;

private:
	bool isDefault_;
};

class SWORDFISH_API ObjectPool: public Void {
public:
	void cacheObject(long long sessionId, long long id, const ConstantSP& obj);
	ConstantSP getCache(long long sessionId, long long id);
	void clearCache();
	void clearCache(long long sessionId);
	void clearCache(long long sessionId, long long id);
	long long requestCacheId(int count);
	int getObjectCount() const;
	long long releaseMemory(long long target, bool& satisfied) override;
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

class SWORDFISH_API Int128: public Constant{
public:
	Int128();
	Int128(const unsigned char* data);
	~Int128() override{}
	inline const unsigned char* bytes() const { return uuid_;}
	string getString() const override { return toString(uuid_);}
	const Guid getInt128() const override { return uuid_;}
	const unsigned char* getBinary() const override {return uuid_;}
	bool isNull() const override;
	void setNull() override;
	void nullFill(const ConstantSP& val) override{
		if(isNull())
			memcpy(uuid_, val->getInt128().bytes(), 16);
	}
	bool isNull(INDEX start, int len, char* buf) const override {
		char null=isNull();
		for(int i=0;i<len;++i)
			buf[i]=null;
		return true;
	}
	bool isNull(INDEX* indices, int len, char* buf) const override {
		char null=isNull();
		for(int i=0;i<len;++i)
			buf[i]=null;
		return true;
	}
	bool isValid(INDEX start, int len, char* buf) const override {
		char valid=!isNull();
		for(int i=0;i<len;++i)
			buf[i]=valid;
		return true;
	}
	bool isValid(INDEX* indices, int len, char* buf) const override {
		char valid=!isNull();
		for(int i=0;i<len;++i)
			buf[i]=valid;
		return true;
	}
	int compare(INDEX index, const ConstantSP& target) const override;
	void setBinary(const unsigned char* val, int unitLength) override;
	bool getBinary(INDEX start, int len, int unitLenght, unsigned char* buf) const override;
	bool getBinary(INDEX* indices, int len, int unitLength, unsigned char* buf) const override;
	const unsigned char* getBinaryConst(INDEX start, int len, int unitLength, unsigned char* buf) const override;
	ConstantSP getInstance() const override {return new Int128();}
	ConstantSP getValue() const override {return new Int128(uuid_);}
	DATA_TYPE getRawType() const override { return DT_INT128;}
	long long getAllocatedMemory() const override {return sizeof(Int128);}
	IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const override {
		short flag = (DF_SCALAR <<8) + getType();
		buffer->write((char)OBJECT_TYPE::CONSTOBJ);
		buffer->write(flag);
		return buffer->write((const char*)uuid_, 16);
	}
	int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int& numElement, int& partial) const override;
	IO_ERR deserialize(DataInputStream* in, INDEX indexStart, int offset, INDEX targetNumElement, INDEX& numElement, int& partial) override;
	bool assign(const ConstantSP& value) override;
	static string toString(const unsigned char* data);
	static Int128* parseInt128(const char* str, int len);
	static bool parseInt128(const char* str, size_t len, unsigned char* buf);
	uint64_t hash() const override;
	bool equal(const ConstantSP& other) const override;
	bool set(INDEX index, const ConstantSP& value, INDEX valueIndex) override{
		memcpy(uuid_, value->getInt128(valueIndex).bytes(), 16);
		return true;
	}
	const unsigned char* getRawData() const { return uuid_;}
	void setRawData(const unsigned char* value){
		memcpy(uuid_, value, 16);
	}

protected:
	mutable unsigned char uuid_[16];
};

class SWORDFISH_API Uuid : public Int128 {
public:
	Uuid(bool newUuid = false);
	Uuid(const unsigned char* uuid);
	Uuid(const char* uuid, int len);
	Uuid(const Uuid& copy);
	~Uuid() override{}
	ConstantSP getInstance() const override {return new Uuid(false);}
	ConstantSP getValue() const override {return new Uuid(uuid_);}
	DATA_TYPE getRawType() const override { return DT_INT128;}
	string getString() const override { return Guid::getString(uuid_);}
	static Uuid* parseUuid(const char* str, int len);
	static bool parseUuid(const char* str, size_t len, unsigned char* buf);
};

class SWORDFISH_API IPAddr : public Int128 {
public:
	IPAddr();
	IPAddr(const char* ip, int len);
	IPAddr(const unsigned char* data);
	~IPAddr() override{}
	ConstantSP getInstance() const override {return new IPAddr();}
	ConstantSP getValue() const override {return new IPAddr(uuid_);}
	DATA_TYPE getRawType() const override { return DT_INT128;}
	string getString() const override { return toString(uuid_);}
	static string toString(const unsigned char* data);
	static IPAddr* parseIPAddr(const char* str, int len);
	static bool parseIPAddr(const char* str, size_t len, unsigned char* buf);

private:
	static bool parseIP4(const char* str, size_t len, unsigned char* buf);
	static bool parseIP6(const char* str, size_t len, unsigned char* buf);
};

class SWORDFISH_API DoublePair {
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

class SWORDFISH_API Double2 : public Int128 {
public:
	bool isNull() const override;
	void setNull() override;

protected:
	Double2();
	Double2(double x, double y);
	Double2(const unsigned char* data);
};

class SWORDFISH_API Complex : public Double2 {
public:
	Complex() : Double2(){setType(DT_COMPLEX);}
	Complex(double real, double image) : Double2(real, image){setType(DT_COMPLEX);}
	Complex(const unsigned char* data) : Double2(data){setType(DT_COMPLEX);}
	~Complex() override{}
	ConstantSP getInstance() const override {return new Complex();}
	ConstantSP getValue() const override {return new Complex(uuid_);}
	DATA_TYPE getRawType() const override { return DT_INT128;}
	string getString() const override { return toString(uuid_);}
	double getReal() const;
	double getImage() const;
	static string toString(const unsigned char* data);
};

class SWORDFISH_API Point : public Double2 {
public:
	Point() : Double2(){setType(DT_POINT);}
	Point(double x, double y) : Double2(x, y){setType(DT_POINT);}
	Point(const unsigned char* data) : Double2(data){setType(DT_POINT);}
	~Point() override{}
	ConstantSP getInstance() const override {return new Point();}
	ConstantSP getValue() const override {return new Point(uuid_);}
	DATA_TYPE getRawType() const override { return DT_INT128;}
	string getString() const override { return toString(uuid_);}
	double getX() const;
	double getY() const;
	static string toString(const unsigned char* data);
};

class SWORDFISH_API String: public Constant{
public:
	String(DolphinString val=""): Constant(DF_SCALAR, DT_STRING, LITERAL), blob_(false), val_(std::move(val)){}
	String(DolphinString val, bool blob): Constant(DF_SCALAR, blob ? DT_BLOB : DT_STRING, LITERAL), blob_(blob), val_(std::move(val)){}
	virtual ~String(){}
    virtual bool isLargeConstant() const { return val_.size()>1024;}
	virtual char getBool() const {throw IncompatibleTypeException(DT_BOOL, internalType());}
	virtual char getChar() const {throw IncompatibleTypeException(DT_CHAR, internalType());}
	virtual short getShort() const {throw IncompatibleTypeException(DT_SHORT, internalType());}
	virtual int getInt() const;
	virtual long long getLong() const;
	virtual INDEX getIndex() const {throw IncompatibleTypeException(DT_INDEX, internalType());}
	virtual float getFloat() const;
	virtual double getDouble() const;

	virtual int getDecimal32(int scale) const override;
	virtual long long getDecimal64(int scale) const override;
	virtual int128 getDecimal128(int scale) const override;
	virtual int getDecimal32(INDEX index, int scale) const override;
	virtual long long getDecimal64(INDEX index, int scale) const override;
	virtual int128 getDecimal128(INDEX index, int scale) const override;
	virtual bool getDecimal32(INDEX start, int len, int scale, int *buf) const override;
	virtual bool getDecimal64(INDEX start, int len, int scale, long long *buf) const override;
	virtual bool getDecimal128(INDEX start, int len, int scale, int128 *buf) const override;
	virtual bool getDecimal32(INDEX *indices, int len, int scale, int *buf) const override;
	virtual bool getDecimal64(INDEX *indices, int len, int scale, long long *buf) const override;
	virtual bool getDecimal128(INDEX *indices, int len, int scale, int128 *buf) const override;
	virtual const int* getDecimal32Const(INDEX start, int len, int scale, int *buf) const override;
	virtual const long long* getDecimal64Const(INDEX start, int len, int scale, long long *buf) const override;
	virtual const int128* getDecimal128Const(INDEX start, int len, int scale,
			int128 *buf) const override;

	string getString() const override {return val_.getString();}
	string getScript() const override {return Util::literalConstant(val_.getString());}
	const DolphinString& getStringRef() const override {return val_;}
	const DolphinString& getStringRef(INDEX index) const override {return val_;}
	bool isNull() const override {return val_.empty();}
	void setString(const DolphinString& val) override {val_=val;}
	ConstantSP get(const ConstantSP& index) const override;
	void setNull() override{val_="";}
	void nullFill(const ConstantSP& val) override{
		if(isNull())
			val_=val->getStringRef();
	}
	bool isNull(INDEX start, int len, char* buf) const override {
		char null=isNull();
		for(int i=0;i<len;++i)
			buf[i]=null;
		return true;
	}
	bool isValid(INDEX start, int len, char* buf) const override {
		char valid=!isNull();
		for(int i=0;i<len;++i)
			buf[i]=valid;
		return true;
	}
	const int* getSymbolConst(INDEX start, int len, int* buf, SymbolBase* symBase, bool insertIfNotThere) const override {
		int tmp=insertIfNotThere?symBase->findAndInsert(val_):symBase->find(val_);
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return buf;
	}
	bool getSymbol(INDEX* indices, int len, int* buf, SymbolBase* symBase,bool insertIfNotThere) const override {
		int tmp=insertIfNotThere?symBase->findAndInsert(val_):symBase->find(val_);
		for(int i=0;i<len;++i)
			buf[i]= indices[i] >= 0 ? tmp : 0;
		return true;
	}
	bool getSymbol(INDEX start, int len, int* buf, SymbolBase* symBase,bool insertIfNotThere) const override {
		int tmp=insertIfNotThere?symBase->findAndInsert(val_):symBase->find(val_);
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return true;
	}
	bool getString(INDEX start, int len, DolphinString** buf) const override {
		for(int i=0;i<len;++i)
			buf[i]=&val_;
		return true;
	}
	bool getString(INDEX start, int len, char** buf) const override {
		char* tmp = (char*)val_.c_str();
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return true;
	}
	DolphinString** getStringConst(INDEX start, int len, DolphinString** buf) const override {
		for(int i=0;i<len;++i)
			buf[i]=&val_;
		return buf;
	}
	char** getStringConst(INDEX start, int len, char** buf) const override {
		char* val = (char*)val_.c_str();
		for(int i=0;i<len;++i)
			buf[i]=val;
		return buf;
	}
	ConstantSP getInstance() const override {return ConstantSP(new String("", blob_));}
	ConstantSP getValue() const override {return ConstantSP(new String(val_, blob_));}
	DATA_TYPE getRawType() const override { return internalType();}
	long long getAllocatedMemory() const override {return sizeof(DolphinString);}
	IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const override;
	int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int& numElement, int& partial) const override;
	IO_ERR deserialize(DataInputStream* in, INDEX indexStart, int offset, INDEX targetNumElement, INDEX& numElement, int& partial) override;
	int compare(INDEX index, const ConstantSP& target) const override {
		return val_.compare(target->getString());
	}
	bool assign(const ConstantSP& value) override;
	uint64_t hash() const override;
	bool equal(const ConstantSP& other) const override;
	bool set(INDEX index, const ConstantSP& value, INDEX valueIndex) override{ val_ = value->getStringRef(valueIndex);return true;}
	const DolphinString& getRawData() const { return val_;}
	void setRawData(const DolphinString& value) { val_ = value;}

	ConstantSP getIterator(const ConstantSP &self) const override;

protected:
	inline DATA_TYPE internalType() const { return blob_ ? DT_BLOB : DT_STRING;}

private:
	bool blob_;
	mutable DolphinString val_;
};

class SWORDFISH_API MetaCode : public String {
public:
	MetaCode(const ObjectSP& code) : String("< " + code->getScript() +" >"), code_(code) {
		setTypeAndCategory(DT_CODE, SYSTEM);
	}
	DATA_TYPE getRawType() const override { return DT_CODE;}
	string getScript() const override;
	bool copyable() const override {return false;}
	bool containNotMarshallableObject() const override {return true;}
	using String::serialize;
	IO_ERR serialize(Heap* pHeap, const ByteArrayCodeBufferSP& buffer) const override;
	ObjectSP getCode() const { return code_;}
	void collectUserDefinedFunctions(unordered_map<string,FunctionDef*>& functionDefs) const override;
	ObjectSP copy(Heap* pHeap, const SQLContextSP& context, bool localize) const override;
	ObjectSP copyAndMaterialize(Heap* pHeap, const SQLContextSP& context, const TableSP& table) const override;
	bool mayContainColumnRefOrVariable() const override { return false;}
	bool isLargeConstant() const override { return false;}
	IO_ERR deserialize(DataInputStream* in, INDEX indexStart, int offset, INDEX targetNumElement, INDEX& numElement, int& partial) override;

private:
	ObjectSP code_;
};

class SWORDFISH_API DataSource : public String {
public:
	DataSource(const ObjectSP& code, long long cacheId = -1, bool isTable = true, bool localMode = false) : String("DataSource< " + code->getScript() +" >"), code_(1, code),
		parentId_(-1), id_(cacheId), action_(-1), isTable_(isTable), localMode_(localMode){setTypeAndCategory(DT_DATASOURCE, SYSTEM);}
	DataSource(const vector<ObjectSP>& code, long long cacheId = -1, bool isTable = true, bool localMode = false);
	DataSource(Session* session, const DataInputStreamSP& in);
	DATA_TYPE getRawType() const override { return DT_DATASOURCE;}
	string getScript() const override;
	bool copyable() const override {return false;}
	bool containNotMarshallableObject() const override {return true;}
	void collectUserDefinedFunctions(unordered_map<string,FunctionDef*>& functionDefs) const override;
	using String::serialize;
	IO_ERR serialize(Heap* pHeap, const ByteArrayCodeBufferSP& buffer) const override;
	ConstantSP getReference(Heap* pHeap) override;
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
	bool isLargeConstant() const override { return false;}
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

class SWORDFISH_API Resource : public String{
public:
	Resource(long long handle, const string& desc, const FunctionDefSP& onClose, Session* session) : String(desc), handle_(handle), onClose_(onClose), session_(session){setTypeAndCategory(DT_RESOURCE, SYSTEM);}
	~Resource() override;
	bool copyable() const override {return false;}
	ConstantSP getValue() const override { throw RuntimeException("Resource is not copyable.");}
	IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const override { throw RuntimeException("Resource is not able to serialize.");}
	bool containNotMarshallableObject() const override {return true;}
	long long  getLong() const override {return handle_;}
	void setLong(long long val) override{ handle_ = val;}
	void close();
	uint64_t hash() const override;
	bool equal(const ConstantSP& other) const override;

private:
	long long handle_;
	FunctionDefSP onClose_;
	Session* session_;
};

template <class T>
class SWORDFISH_API AbstractScalar: public Constant{
public:
	AbstractScalar(DATA_TYPE dt, DATA_CATEGORY dc, T val=0): Constant(DF_SCALAR, dt, dc), val_(val){}
	~AbstractScalar() override{}
	char getBool() const override {return isNull()?CHAR_MIN:(bool)val_;}
	char getChar() const override {return isNull()?CHAR_MIN:val_;}
	short getShort() const override {return isNull()?SHRT_MIN:val_;}
	int getInt() const override {return isNull()?INT_MIN:val_;}
	long long getLong() const override {return isNull()?LLONG_MIN:val_;}
	INDEX getIndex() const override {return isNull()?INDEX_MIN:val_;}
	float getFloat() const override {return isNull()?FLT_NMIN:val_;}
	double getDouble() const override {return isNull()?DBL_NMIN:val_;}

	void setBool(char val) override{if(val != CHAR_MIN) val_=(T)val; else setNull();}
	void setChar(char val) override{if(val != CHAR_MIN) val_=(T)val; else setNull();}
	void setShort(short val) override{if(val != SHRT_MIN) val_=(T)val; else setNull();}
	void setInt(int val) override{if(val != INT_MIN) val_=(T)val; else setNull();}
	void setLong(long long val) override{if(val != LLONG_MIN) val_=(T)val; else setNull();}
	void setIndex(INDEX val) override{if(val != INDEX_MIN) val_=(T)val; else setNull();}
	void setFloat(float val) override{if(val != FLT_NMIN) val_=(T)val; else setNull();}
	void setDouble(double val) override{if(val != DBL_NMIN) val_=(T)val; else setNull();}
	using Constant::setString;
	virtual void setString(const string& val){}
	void setString(const DolphinString& val) override{}
	bool isNull() const override = 0;

	bool setBool(INDEX start, int len, const char* buf) override {
		if(UNLIKELY(len != 1))
			return false;
		if(buf[0] != CHAR_MIN)
			val_ = (T)buf[0];
		else
			setNull();
		return true;
	}
	bool setChar(INDEX start, int len, const char* buf) override {
		if(UNLIKELY(len != 1))
			return false;
		if(buf[0] != CHAR_MIN)
			val_ = (T)buf[0];
		else
			setNull();
		return true;
	}
	bool setShort(INDEX start, int len, const short* buf) override {
		if(UNLIKELY(len != 1))
			return false;
		if(buf[0] != SHRT_MIN)
			val_ = (T)buf[0];
		else
			setNull();
		return true;
	}
	bool setInt(INDEX start, int len, const int* buf) override {
		if(UNLIKELY(len != 1))
			return false;
		if(buf[0] != INT_MIN)
			val_ = (T)buf[0];
		else
			setNull();
		return true;
	}
	bool setLong(INDEX start, int len, const long long* buf) override {
		if(UNLIKELY(len != 1))
			return false;
		if(buf[0] != LLONG_MIN)
			val_ = (T)buf[0];
		else
			setNull();
		return true;
	}
	bool setFloat(INDEX start, int len, const float* buf) override {
		if(UNLIKELY(len != 1))
			return false;
		if(buf[0] != FLT_NMIN)
			val_ = (T)buf[0];
		else
			setNull();
		return true;
	}
	bool setDouble(INDEX start, int len, const double* buf) override {
		if(UNLIKELY(len != 1))
			return false;
		if(buf[0] != DBL_NMIN)
			val_ = (T)buf[0];
		else
			setNull();
		return true;
	}

	ConstantSP get(const ConstantSP& index) const override {
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
	int getDecimal32(int scale) const override {
		return getDecimal32(/*index*/0, scale);
	}
	long long getDecimal64(int scale) const override {
		return getDecimal64(/*index*/0, scale);
	}
	int128 getDecimal128(int scale) const override {
		return getDecimal128(/*index*/0, scale);
	}

	int getDecimal32(INDEX index, int scale) const override {
		int result = 0;
		getDecimal32(index, /*len*/1, scale, &result);
		return result;
	}
	long long getDecimal64(INDEX index, int scale) const override {
		long long result = 0;
		getDecimal64(index, /*len*/1, scale, &result);
		return result;
	}
	int128 getDecimal128(INDEX index, int scale) const override {
		int128 result = 0;
		getDecimal128(index, /*len*/1, scale, &result);
		return result;
	}

	bool getDecimal32(INDEX start, int len, int scale, int *buf) const override {
		return getDecimal(start, len, scale, buf);
	}
	bool getDecimal64(INDEX start, int len, int scale, long long *buf) const override {
		return getDecimal(start, len, scale, buf);
	}
	bool getDecimal128(INDEX start, int len, int scale, int128 *buf) const override {
		return getDecimal(start, len, scale, buf);
	}

	bool getDecimal32(INDEX *indices, int len, int scale, int *buf) const override {
		return getDecimal(/*start*/0, len, scale, buf);
	}
	bool getDecimal64(INDEX *indices, int len, int scale, long long *buf) const override {
		return getDecimal(/*start*/0, len, scale, buf);
	}
	bool getDecimal128(INDEX *indices, int len, int scale, int128 *buf) const override {
		return getDecimal(/*start*/0, len, scale, buf);
	}

	const int* getDecimal32Const(INDEX start, int len, int scale, int *buf) const override {
		return getDecimal32Buffer(start, len, scale, buf);
	}
	const long long* getDecimal64Const(INDEX start, int len, int scale, long long *buf) const override {
		return getDecimal64Buffer(start, len, scale, buf);
	}
	const int128* getDecimal128Const(INDEX start, int len, int scale,
			int128 *buf) const override {
		return getDecimal128Buffer(start, len, scale, buf);
	}

	int* getDecimal32Buffer(INDEX start, int len, int scale, int *buf) const override {
		getDecimal(start, len, scale, buf);
		return buf;
	}
	long long* getDecimal64Buffer(INDEX start, int len, int scale, long long *buf) const override {
		getDecimal(start, len, scale, buf);
		return buf;
	}
	int128* getDecimal128Buffer(INDEX start, int len, int scale,
			int128 *buf) const override {
		getDecimal(start, len, scale, buf);
		return buf;
	}

	void setDecimal32(INDEX index, int scale, int val) override {
		setDecimal32(index, /*len*/1, scale, &val);
	}
	void setDecimal64(INDEX index, int scale, long long val) override {
		setDecimal64(index, /*len*/1, scale, &val);
	}
	void setDecimal128(INDEX index, int scale, int128 val) override {
		setDecimal128(index, /*len*/1, scale, &val);
	}

	bool setDecimal32(INDEX start, int len, int scale, const int *buf) override {
		return setDecimal(start, len, scale, buf);
	}
	bool setDecimal64(INDEX start, int len, int scale, const long long *buf) override {
		return setDecimal(start, len, scale, buf);
	}
	bool setDecimal128(INDEX start, int len, int scale, const int128 *buf) override {
		return setDecimal(start, len, scale, buf);
	}

private:
	template <typename R>
	bool SWORDFISH_API getDecimal(INDEX /*start*/, int len, int scale, R *buf) const;

	template <typename R>
	bool SWORDFISH_API setDecimal(INDEX /*start*/, int len, int scale, const R *buf);

public:
	string getScript() const override {
		if(isNull()){
			string str("00");
			return str.append(1, Util::getDataTypeSymbol(getType()));
		}
		else
			return getString();
	}

	string getScript(Heap* heap) const override {
		if(isNull()){
			string str("00");
			return str.append(1, Util::getDataTypeSymbol(getType()));
		}
		else
			return getString(heap);
	}

	void nullFill(const ConstantSP& val) override{
		if(isNull()){
			if(val->getCategory()==FLOATING)
				val_=val->getDouble();
			else
				val_=val->getLong();
		}
	}
	bool isNull(INDEX start, int len, char* buf) const override {
		char null=isNull();
		for(int i=0;i<len;++i)
			buf[i]=null;
		return true;
	}
	bool isNull(INDEX* indices, int len, char* buf) const override {
		char null=isNull();
		for(int i=0;i<len;++i)
			buf[i]=null;
		return true;
	}
	bool isValid(INDEX start, int len, char* buf) const override {
		char valid=!isNull();
		for(int i=0;i<len;++i)
			buf[i]=valid;
		return true;
	}
	bool isValid(INDEX* indices, int len, char* buf) const override {
		char valid=!isNull();
		for(int i=0;i<len;++i)
			buf[i]=valid;
		return true;
	}
	bool getBool(INDEX start, int len, char* buf) const override {
		char tmp=isNull()?CHAR_MIN:(bool)val_;
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return true;
	}
	bool getBool(INDEX* indices, int len, char* buf) const override {
		char tmp=isNull()?CHAR_MIN:(bool)val_;
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return true;
	}
	const char* getBoolConst(INDEX start, int len, char* buf) const override {
		char tmp=isNull()?CHAR_MIN:(bool)val_;
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return buf;
	}
	bool getChar(INDEX start, int len, char* buf) const override {
		char tmp=isNull()?CHAR_MIN:val_;
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return true;
	}
	bool getChar(INDEX* indices, int len, char* buf) const override {
		char tmp=isNull()?CHAR_MIN:val_;
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return true;
	}
	const char* getCharConst(INDEX start, int len, char* buf) const override {
		char tmp=isNull()?CHAR_MIN:val_;
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return buf;
	}
	bool getShort(INDEX start, int len, short* buf) const override {
		short tmp=isNull()?SHRT_MIN:val_;
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return true;
	}
	bool getShort(INDEX* indices, int len, short* buf) const override {
		short tmp=isNull()?SHRT_MIN:val_;
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return true;
	}
	const short* getShortConst(INDEX start, int len, short* buf) const override {
		short tmp=isNull()?SHRT_MIN:val_;
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return buf;
	}
	bool getInt(INDEX start, int len, int* buf) const override {
		int tmp=isNull()?INT_MIN:val_;
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return true;
	}
	bool getInt(INDEX* indices, int len, int* buf) const override {
		int tmp=isNull()?INT_MIN:val_;
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return true;
	}
	const int* getIntConst(INDEX start, int len, int* buf) const override {
		int tmp=isNull()?INT_MIN:val_;
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return buf;
	}
	bool getLong(INDEX start, int len, long long* buf) const override {
		long long tmp=isNull()?LLONG_MIN:val_;
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return true;
	}
	bool getLong(INDEX* indices, int len, long long* buf) const override {
		long long tmp=isNull()?LLONG_MIN:val_;
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return true;
	}
	const long long* getLongConst(INDEX start, int len, long long* buf) const override {
		long long tmp=isNull()?LLONG_MIN:val_;
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return buf;
	}
	bool getIndex(INDEX start, int len, INDEX* buf) const override {
		INDEX tmp=isNull()?INDEX_MIN:val_;
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return true;
	}
	bool getIndex(INDEX* indices, int len, INDEX* buf) const override {
		INDEX tmp=isNull()?INDEX_MIN:val_;
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return true;
	}
	const INDEX* getIndexConst(INDEX start, int len, INDEX* buf) const override {
		INDEX tmp=isNull()?INDEX_MIN:val_;
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return buf;
	}
	bool getFloat(INDEX start, int len, float* buf) const override {
		float tmp=isNull()?FLT_NMIN:val_;
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return true;
    }
	bool getFloat(INDEX* indices, int len, float* buf) const override {
		float tmp=isNull()?FLT_NMIN:val_;
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return true;
    }
	const float* getFloatConst(INDEX start, int len, float* buf) const override {
		float tmp=isNull()?FLT_NMIN:val_;
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return buf;
    }
	bool getDouble(INDEX start, int len, double* buf) const override {
		double tmp=isNull()?DBL_NMIN:val_;
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return true;
	}
	bool getDouble(INDEX* indices, int len, double* buf) const override {
		double tmp=isNull()?DBL_NMIN:val_;
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return true;
	}
	const double* getDoubleConst(INDEX start, int len, double* buf) const override {
		double tmp=isNull()?DBL_NMIN:val_;
		for(int i=0;i<len;++i)
			buf[i]=tmp;
		return buf;
	}
	long long getAllocatedMemory() const override {return sizeof(AbstractScalar);}
	int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int& numElement, int& partial) const override {
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

	bool add(INDEX start, INDEX length, long long inc) override {
		if(isNull())
			return false;
		val_ += inc;
		return true;
	}

	bool add(INDEX start, INDEX length, double inc) override {
		if(isNull())
			return false;
		val_ += inc;
		return true;
	}

	IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const override {
		short flag = (DF_SCALAR <<8) + getType();
		buffer->write((char)OBJECT_TYPE::CONSTOBJ);
		buffer->write(flag);
		char buf[8];
		int numElement, partial;
		int length = serialize(buf, 8, 0, 0, numElement, partial);
		return buffer->write(buf, length);
	}
	int compare(INDEX index, const ConstantSP& target) const override {
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

	bool assign(const ConstantSP& value) override {
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
	T getRawData() const { return val_;}
	void setRawData(T value) { val_ = value;}

protected:
	T val_;
};

class SWORDFISH_API Bool: public AbstractScalar<char>{
public:
	Bool(char val=0):AbstractScalar(DT_BOOL, LOGICAL, val){}
	~Bool() override{}
	bool isNull() const override {return val_==CHAR_MIN;}
	void setNull() override{val_= CHAR_MIN;}
	void setBool(char val) override{ val_ = val;}
	DATA_TYPE getRawType() const override { return DT_BOOL;}
	char getBool() const override {
		if (val_ == CHAR_MIN) {
			return CHAR_MIN;
		}
		return static_cast<bool>(val_);
	}
	ConstantSP getInstance() const override {return ConstantSP(new Bool());}
	ConstantSP getValue() const override {return ConstantSP(new Bool(val_));}
	string getString() const override;
	string getString(Heap* heap) const override;
	bool add(INDEX start, INDEX length, long long inc) override { return false;}
	bool add(INDEX start, INDEX length, double inc) override { return false;}
	IO_ERR deserialize(DataInputStream* in, INDEX indexStart, int offset, INDEX targetNumElement, INDEX& numElement, int& partial) override;
	static Bool* parseBool(const string& str);
	static string toString(char val, bool isPython = false);
	bool assign(const ConstantSP& value) override;
	uint64_t hash() const override;
	bool equal(const ConstantSP& other) const override;
	bool set(INDEX index, const ConstantSP& value, INDEX valueIndex) override{ val_ = value->getBool(valueIndex);return true;}
};

class SWORDFISH_API Char: public AbstractScalar<char>{
public:
	Char(char val=0):AbstractScalar(DT_CHAR, INTEGRAL, val){}
	~Char() override{}
	bool isNull() const override {return val_==CHAR_MIN;}
	void setNull() override{val_=CHAR_MIN;}
	void setChar(char val) override{ val_ = val;}
	DATA_TYPE getRawType() const override { return DT_CHAR;}
	char getChar() const override { return val_; }
	ConstantSP getInstance() const override {return ConstantSP(new Char());}
	ConstantSP getValue() const override {return ConstantSP(new Char(val_));}
	string getString() const override { return toString(val_);}
	string getScript() const override;
	IO_ERR deserialize(DataInputStream* in, INDEX indexStart, int offset, INDEX targetNumElement, INDEX& numElement, int& partial) override;
	static Char* parseChar(const string& str);
	static string toString(char val);
	uint64_t hash() const override;
	bool equal(const ConstantSP& other) const override;
	bool set(INDEX index, const ConstantSP& value, INDEX valueIndex) override{ val_ = value->getChar(valueIndex);return true;}
};

class SWORDFISH_API Short: public AbstractScalar<short>{
public:
	Short(short val=0):AbstractScalar(DT_SHORT, INTEGRAL, val){}
	~Short() override{}
	bool isNull() const override {return val_==SHRT_MIN;}
	void setNull() override{val_=SHRT_MIN;}
	void setShort(short val) override{ val_ = val;}
	DATA_TYPE getRawType() const override { return DT_SHORT;}
	short getShort() const override { return val_; }
	ConstantSP getInstance() const override {return ConstantSP(new Short());}
	ConstantSP getValue() const override {return ConstantSP(new Short(val_));}
	string getString() const override { return toString(val_);}
	IO_ERR deserialize(DataInputStream* in, INDEX indexStart, int offset, INDEX targetNumElement, INDEX& numElement, int& partial) override;
	static Short* parseShort(const string& str);
	static string toString(short val);
	uint64_t hash() const override;
	bool equal(const ConstantSP& other) const override;
	bool set(INDEX index, const ConstantSP& value, INDEX valueIndex) override{ val_ = value->getShort(valueIndex);return true;}
};

class SWORDFISH_API Int: public AbstractScalar<int>{
public:
	Int(int val=0):AbstractScalar(DT_INT, INTEGRAL, val){}
	~Int() override{}
	bool isNull() const override {return val_==INT_MIN;}
	void setNull() override{val_=INT_MIN;}
	void setInt(int val) override{ val_ = val;}
	DATA_TYPE getRawType() const override { return DT_INT;}
	int getInt() const override { return val_; }
	ConstantSP getInstance() const override {return ConstantSP(new Int());}
	ConstantSP getValue() const override {return ConstantSP(new Int(val_));}
	string getString() const override { return toString(val_);}
	IO_ERR deserialize(DataInputStream* in, INDEX indexStart, int offset, INDEX targetNumElement, INDEX& numElement, int& partial) override;
	static Int* parseInt(const string& str);
	static string toString(int val);
	uint64_t hash() const override;
	bool equal(const ConstantSP& other) const override;
	bool set(INDEX index, const ConstantSP& value, INDEX valueIndex) override{ val_ = value->getInt(valueIndex);return true;}
};

class SWORDFISH_API EnumInt : public Int {
public:
	EnumInt(const string& desc, int val):Int(val), desc_(desc){}
	~EnumInt() override{}
	string getScript() const override {return desc_;}
	ConstantSP getValue() const override {return ConstantSP(new EnumInt(desc_, val_));}
	ConstantSP getInstance() const override {return ConstantSP(new EnumInt(desc_, val_));}
	string getString() const override {return desc_;}

private:
	string desc_;
};

class SWORDFISH_API Duration : public Int {
public:
	Duration() = default;
	Duration(DURATION unit, int val);
	Duration(int exchange, int val);
	Duration(const string& exchange, int val);
	Duration(FREQUENCY freq);
	~Duration() override{}
	DATA_TYPE getRawType() const override { return DT_DURATION;}
	long long getLong() const override;
	string getScript() const override {return getString();}
	ConstantSP getValue() const override;
	ConstantSP getInstance() const override;
	string getString() const override;
	int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int& numElement, int& partial) const override;
	IO_ERR deserialize(DataInputStream* in, INDEX indexStart, int offset, INDEX targetNumElement, INDEX& numElement, int& partial) override;
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
	uint64_t hash() const override;
	bool equal(const ConstantSP& other) const override;

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

class SWORDFISH_API Long: public AbstractScalar<long long>{
public:
	Long(long long val=0):AbstractScalar(DT_LONG, INTEGRAL, val){}
	~Long() override{}
	bool isNull() const override {return val_==LLONG_MIN;}
	void setNull() override{val_=LLONG_MIN;}
	void setLong(long long val) override{ val_ = val;}
	DATA_TYPE getRawType() const override { return DT_LONG;}
	long long getLong() const override { return val_; }
	ConstantSP getInstance() const override {return ConstantSP(new Long());}
	ConstantSP getValue() const override {return ConstantSP(new Long(val_));}
	string getString() const override { return toString(val_);}
	IO_ERR deserialize(DataInputStream* in, INDEX indexStart, int offset, INDEX targetNumElement, INDEX& numElement, int& partial) override;
	static Long* parseLong(const string& str);
	static string toString(long long val);
	uint64_t hash() const override;
	bool equal(const ConstantSP& other) const override;
	bool set(INDEX index, const ConstantSP& value, INDEX valueIndex) override{ val_ = value->getLong(valueIndex);return true;}
};

class SWORDFISH_API Float: public AbstractScalar<float>{
public:
	Float(float val=0):AbstractScalar(DT_FLOAT, FLOATING, val){}
	~Float() override{}
	bool isNull() const override {return val_==FLT_NMIN;}
	void setNull() override{val_=FLT_NMIN;}
	void setFloat(float val) override{ val_ = val;}
	DATA_TYPE getRawType() const override { return DT_FLOAT;}
	float getFloat() const override { return val_; }
	ConstantSP getInstance() const override {return ConstantSP(new Float());}
	ConstantSP getValue() const override {return ConstantSP(new Float(val_));}
	char getChar() const override {return isNull()?CHAR_MIN:(val_<0?val_-0.5:val_+0.5);}
	short getShort() const override {return isNull()?SHRT_MIN:(val_<0?val_-0.5:val_+0.5);}
	int getInt() const override {return isNull()?INT_MIN:(val_<0?val_-0.5:val_+0.5);}
	long long  getLong() const override {return isNull()?LLONG_MIN:(val_<0?val_-0.5:val_+0.5);}
	bool getChar(INDEX start, int len, char* buf) const override;
	const char* getCharConst(INDEX start, int len, char* buf) const override;
	bool getShort(INDEX start, int len, short* buf) const override;
	const short* getShortConst(INDEX start, int len, short* buf) const override;
	bool getInt(INDEX start, int len, int* buf) const override;
	const int* getIntConst(INDEX start, int len, int* buf) const override;
	bool getLong(INDEX start, int len, long long* buf) const override;
	const long long* getLongConst(INDEX start, int len, long long* buf) const override;
	string getString() const override { return toString(val_);}
	IO_ERR deserialize(DataInputStream* in, INDEX indexStart, int offset, INDEX targetNumElement, INDEX& numElement, int& partial) override;
	static Float* parseFloat(const string& str);
	static string toString(float val);
	uint64_t hash() const override;
	bool equal(const ConstantSP& other) const override;
	bool set(INDEX index, const ConstantSP& value, INDEX valueIndex) override{ val_ = value->getFloat(valueIndex);return true;}
};

class SWORDFISH_API Double: public AbstractScalar<double>{
public:
	Double(double val=0):AbstractScalar(DT_DOUBLE, FLOATING, (std::isnan(val)|| std::isinf(val)) ? DBL_NMIN : val){}
	~Double() override{}
	bool isNull() const override {return val_==DBL_NMIN;}
	void setNull() override{val_=DBL_NMIN;}
	void setDouble(double val) override{ val_ = (std::isnan(val)|| std::isinf(val)) ? DBL_NMIN : val; }
	DATA_TYPE getRawType() const override { return DT_DOUBLE;}
	double getDouble() const override { return val_; }
	ConstantSP getInstance() const override {return ConstantSP(new Double());}
	ConstantSP getValue() const override {return ConstantSP(new Double(val_));}
	char getChar() const override {return isNull()?CHAR_MIN:(val_<0?val_-0.5:val_+0.5);}
	short getShort() const override {return isNull()?SHRT_MIN:(val_<0?val_-0.5:val_+0.5);}
	int getInt() const override {return isNull()?INT_MIN:(val_<0?val_-0.5:val_+0.5);}
	long long  getLong() const override {return isNull()?LLONG_MIN:(val_<0?val_-0.5:val_+0.5);}
	bool getChar(INDEX start, int len, char* buf) const override;
	const char* getCharConst(INDEX start, int len, char* buf) const override;
	bool getShort(INDEX start, int len, short* buf) const override;
	const short* getShortConst(INDEX start, int len, short* buf) const override;
	bool getInt(INDEX start, int len, int* buf) const override;
	const int* getIntConst(INDEX start, int len, int* buf) const override;
	bool getLong(INDEX start, int len, long long* buf) const override;
	const long long* getLongConst(INDEX start, int len, long long* buf) const override;
	string getString() const override {return toString(val_);}
	IO_ERR deserialize(DataInputStream* in, INDEX indexStart, int offset, INDEX targetNumElement, INDEX& numElement, int& partial) override;
	static Double* parseDouble(const string& str);
	static string toString(double val);
	uint64_t hash() const override;
	bool equal(const ConstantSP& other) const override;
	bool set(INDEX index, const ConstantSP& value, INDEX valueIndex) override{ val_ = value->getDouble(valueIndex);return true;}
};

class SWORDFISH_API EnumDouble : public Double {
public:
	EnumDouble(const string& desc, double val):Double(val), desc_(desc){}
	~EnumDouble() override{}
	string getScript() const override {return desc_;}
	ConstantSP getValue() const override {return ConstantSP(new EnumDouble(desc_, val_));}
	ConstantSP getInstance() const override {return ConstantSP(new EnumDouble(desc_, val_));}
	string getString() const override { return desc_;}

private:
	string desc_;
};

class SWORDFISH_API TemporalScalar:public Int{
public:
	TemporalScalar(DATA_TYPE type, int val=0):Int(val){setTypeAndCategory(type, TEMPORAL);}
	~TemporalScalar() override{}
	bool equal(const ConstantSP& other) const override { return getType() == other->getType() && val_ == other->getInt();}
};

class SWORDFISH_API Date:public TemporalScalar{
public:
	Date(int val=0):TemporalScalar(DT_DATE, val){}
	Date(int year, int month, int day):TemporalScalar(DT_DATE, Util::countDays(year,month,day)){}
	~Date() override{}
	ConstantSP getInstance() const override {return ConstantSP(new Date());}
	ConstantSP getValue() const override {return ConstantSP(new Date(val_));}
	string getString() const override { return toString(val_);}
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


class SWORDFISH_API Month:public TemporalScalar{
public:
	Month():TemporalScalar(DT_MONTH, 1999*12+11){}
	Month(int val):TemporalScalar(DT_MONTH, val){}
	Month(int year, int month):TemporalScalar(DT_MONTH, year*12+month-1){}
	~Month() override{}
	ConstantSP getInstance() const override {return ConstantSP(new Month());}
	ConstantSP getValue() const override {return ConstantSP(new Month(val_));}
	string getString() const override { return toString(val_);}
	static Month* parseMonth(const string& str);
	static string toString(int val);
};

class SWORDFISH_API Time:public TemporalScalar{
public:
	Time(int val=0):TemporalScalar(DT_TIME, val){}
	Time(int hour, int minute, int second, int milliSecond):TemporalScalar(DT_TIME, ((hour*60+minute)*60+second)*1000+milliSecond){}
	~Time() override{}
	ConstantSP getInstance() const override {return ConstantSP(new Time());}
	ConstantSP getValue() const override {return ConstantSP(new Time(val_));}
	string getString() const override { return toString(val_);}
	void validate() override;
	static Time* parseTime(const string& str);
	static string toString(int val);
};

class SWORDFISH_API NanoTime:public Long{
public:
	NanoTime(long long val=0):Long(val){setTypeAndCategory(DT_NANOTIME, TEMPORAL);}
	NanoTime(int hour, int minute, int second, int nanoSecond):Long(((hour*60+minute)*60+second)*1000000000ll+ nanoSecond){setTypeAndCategory(DT_NANOTIME, TEMPORAL);}
	~NanoTime() override{}
	ConstantSP getInstance() const override {return ConstantSP(new NanoTime());}
	ConstantSP getValue() const override {return ConstantSP(new NanoTime(val_));}
	string getString() const override { return toString(val_);}
	void validate() override;
	static NanoTime* parseNanoTime(const string& str);
	static string toString(long long val);
	bool equal(const ConstantSP& other) const override { return DT_NANOTIME == other->getType() && val_ == other->getLong();}
};

class SWORDFISH_API Timestamp:public Long{
public:
	Timestamp(long long val=0):Long(val){setTypeAndCategory(DT_TIMESTAMP, TEMPORAL);}
	Timestamp(int year, int month, int day,int hour, int minute, int second, int milliSecond);
	~Timestamp() override{}
	ConstantSP getInstance() const override {return ConstantSP(new Timestamp());}
	ConstantSP getValue() const override {return ConstantSP(new Timestamp(val_));}
	string getString() const override { return toString(val_);}
	static Timestamp* parseTimestamp(const string& str);
	static string toString(long long val);
	bool equal(const ConstantSP& other) const override { return DT_TIMESTAMP == other->getType() && val_ == other->getLong();}
};

class SWORDFISH_API NanoTimestamp:public Long{
public:
	NanoTimestamp(long long val=0):Long(val){setTypeAndCategory(DT_NANOTIMESTAMP, TEMPORAL);}
	NanoTimestamp(int year, int month, int day,int hour, int minute, int second, int nanoSecond);
	~NanoTimestamp() override{}
	ConstantSP getInstance() const override {return ConstantSP(new NanoTimestamp());}
	ConstantSP getValue() const override {return ConstantSP(new NanoTimestamp(val_));}
	string getString() const override { return toString(val_);}
	static NanoTimestamp* parseNanoTimestamp(const string& str);
	static string toString(long long val);
	bool equal(const ConstantSP& other) const override { return DT_NANOTIMESTAMP == other->getType() && val_ == other->getLong();}
};

class SWORDFISH_API Minute:public TemporalScalar{
public:
	Minute(int val=0):TemporalScalar(DT_MINUTE, val){}
	Minute(int hour, int minute):TemporalScalar(DT_MINUTE, hour*60+minute){}
	~Minute() override{}
	ConstantSP getInstance() const override {return ConstantSP(new Minute());}
	ConstantSP getValue() const override {return ConstantSP(new Minute(val_));}
	string getString() const override { return toString(val_);}
	void validate() override;
	static Minute* parseMinute(const string& str);
	static string toString(int val);
};

class SWORDFISH_API Second:public TemporalScalar{
public:
	Second(int val=0):TemporalScalar(DT_SECOND, val){}
	Second(int hour, int minute,int second):TemporalScalar(DT_SECOND, (hour*60+minute)*60+second){}
	~Second() override{}
	ConstantSP getInstance() const override {return ConstantSP(new Second());}
	ConstantSP getValue() const override {return ConstantSP(new Second(val_));}
	string getString() const override { return toString(val_);}
	void validate() override;
	static Second* parseSecond(const string& str);
	static string toString(int val);
};

class SWORDFISH_API DateTime:public TemporalScalar{
public:
	DateTime(int val=0):TemporalScalar(DT_DATETIME, val){}
	DateTime(int year, int month, int day, int hour, int minute,int second);
	~DateTime() override{}
	ConstantSP getInstance() const override {return ConstantSP(new DateTime());}
	ConstantSP getValue() const override {return ConstantSP(new DateTime(val_));}
	string getString() const override { return toString(val_);}
	static DateTime* parseDateTime(const string& str);
	static string toString(int val);
};

class SWORDFISH_API DateHour:public TemporalScalar{
public:
	DateHour(int val=0):TemporalScalar(DT_DATEHOUR, val){}
	DateHour(int year, int month, int day, int hour);
	~DateHour() override{}
	ConstantSP getInstance() const override {return ConstantSP(new DateHour());}
	ConstantSP getValue() const override {return ConstantSP(new DateHour(val_));}
	string getString() const override { return toString(val_);}
	static DateHour* parseDateHour(const string& str);
	static string toString(int val);
};


template <typename T>
class SWORDFISH_API Decimal : public Constant {
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
    int getExtraParamForType() const override { return scale_; }

    bool isNull() const override;
    void setNull() override;

    DATA_TYPE getRawType() const override { return type(); }

    ConstantSP getInstance() const override;
    ConstantSP getValue() const override;

    std::string getString() const override {
        if (isNull()) {
            return "";
        }
        return toString();
    }

    bool isNull(INDEX start, int len, char *buf) const override;
    bool isValid(INDEX start, int len, char *buf) const override;

    bool set(INDEX index, const ConstantSP &value, INDEX valueIndex) override;

    void nullFill(const ConstantSP &val) override;

    uint64_t hash() const override;

public:  /// decimal to float
    float getFloat() const override;
    double getDouble() const override;
    bool getFloat(INDEX start, int len, float *buf) const override;
    bool getDouble(INDEX start, int len, double *buf) const override;
    const float *getFloatConst(INDEX start, int len, float *buf) const override;
    const double *getDoubleConst(INDEX start, int len, double *buf) const override;

public:  /// decimal to integer
    char getChar() const override { return toInteger<char>(CHAR_MIN); }
    short getShort() const override { return toInteger<short>(SHRT_MIN); }
    int getInt() const override { return toInteger<int>(INT_MIN); }
    long long getLong() const override { return toInteger<long long>(LLONG_MIN); }

    bool getChar(INDEX start, int len, char *buf) const override {
        return toInteger<char>(CHAR_MIN, start, len, buf);
    }
    bool getShort(INDEX start, int len, short *buf) const override {
        return toInteger<short>(SHRT_MIN, start, len, buf);
    }
    bool getInt(INDEX start, int len, int *buf) const override {
        return toInteger<int>(INT_MIN, start, len, buf);
    }
    bool getLong(INDEX start, int len, long long *buf) const override {
        return toInteger<long long>(LLONG_MIN, start, len, buf);
    }

    const char *getCharConst(INDEX start, int len, char *buf) const override {
        getChar(start, len, buf);
        return buf;
    }
    const short *getShortConst(INDEX start, int len, short *buf) const override {
        getShort(start, len, buf);
        return buf;
    }
    const int *getIntConst(INDEX start, int len, int *buf) const override {
        getInt(start, len, buf);
        return buf;
    }
    const long long *getLongConst(INDEX start, int len, long long *buf) const override {
        getLong(start, len, buf);
        return buf;
    }

public:  /// {get,set}Binary
    void setBinary(const unsigned char *val, int unitLength) override;
    bool getBinary(INDEX start, int len, int unitLength, unsigned char *buf) const override;
    const unsigned char *getBinaryConst(INDEX start, int len, int unitLength,
                                                unsigned char *buf) const override;

public:  /// {get,set}Decimal{32,64,128}
    int getDecimal32(int scale) const override {
        return getDecimal32(/*index*/0, scale);
    }
    long long getDecimal64(int scale) const override {
        return getDecimal64(/*index*/0, scale);
    }
    int128 getDecimal128(int scale) const override {
        return getDecimal128(/*index*/0, scale);
    }

    int getDecimal32(INDEX index, int scale) const override {
        int result = 0;
        getDecimal32(index, /*len*/1, scale, &result);
        return result;
    }
    long long getDecimal64(INDEX index, int scale) const override {
        long long result = 0;
        getDecimal64(index, /*len*/1, scale, &result);
        return result;
    }
    int128 getDecimal128(INDEX index, int scale) const override {
        int128 result = 0;
        getDecimal128(index, /*len*/1, scale, &result);
        return result;
    }

    bool getDecimal32(INDEX start, int len, int scale, int *buf) const override {
        return getDecimal(start, len, scale, buf);
    }
    bool getDecimal64(INDEX start, int len, int scale, long long *buf) const override {
        return getDecimal(start, len, scale, buf);
    }
    bool getDecimal128(INDEX start, int len, int scale, int128 *buf) const override {
        return getDecimal(start, len, scale, buf);
    }

    bool getDecimal32(INDEX *indices, int len, int scale, int *buf) const override {
        return getDecimal(/*start*/0, len, scale, buf);
    }
    bool getDecimal64(INDEX *indices, int len, int scale, long long *buf) const override {
        return getDecimal(/*start*/0, len, scale, buf);
    }
    bool getDecimal128(INDEX *indices, int len, int scale, int128 *buf) const override {
        return getDecimal(/*start*/0, len, scale, buf);
    }

    const int* getDecimal32Const(INDEX start, int len, int scale, int *buf) const override {
        return getDecimal32Buffer(start, len, scale, buf);
    }
    const long long* getDecimal64Const(INDEX start, int len, int scale, long long *buf) const override {
        return getDecimal64Buffer(start, len, scale, buf);
    }
    const int128* getDecimal128Const(INDEX start, int len, int scale,
                                                           int128 *buf) const override {
        return getDecimal128Buffer(start, len, scale, buf);
    }

    int* getDecimal32Buffer(INDEX start, int len, int scale, int *buf) const override {
        getDecimal(start, len, scale, buf);
        return buf;
    }
    long long* getDecimal64Buffer(INDEX start, int len, int scale, long long *buf) const override {
        getDecimal(start, len, scale, buf);
        return buf;
    }
    int128* getDecimal128Buffer(INDEX start, int len, int scale,
                                                      int128 *buf) const override {
        getDecimal(start, len, scale, buf);
        return buf;
    }

    void setDecimal32(INDEX index, int scale, int val) override {
        setDecimal32(index, /*len*/1, scale, &val);
    }
    void setDecimal64(INDEX index, int scale, long long val) override {
        setDecimal64(index, /*len*/1, scale, &val);
    }
    void setDecimal128(INDEX index, int scale, int128 val) override {
        setDecimal128(index, /*len*/1, scale, &val);
    }

    bool setDecimal32(INDEX start, int len, int scale, const int *buf) override {
        return setDecimal(start, len, scale, buf);
    }
    bool setDecimal64(INDEX start, int len, int scale, const long long *buf) override {
        return setDecimal(start, len, scale, buf);
    }
    bool setDecimal128(INDEX start, int len, int scale, const int128 *buf) override {
        return setDecimal(start, len, scale, buf);
    }

public:
    bool assign(const ConstantSP &value) override;

    int compare(INDEX /*index*/, const ConstantSP &target) const override;

    int serialize(char *buf, int bufSize, INDEX indexStart, int offset,
                          int &numElement, int &partial) const override;

    IO_ERR serialize(const ByteArrayCodeBufferSP &buffer) const override;

    IO_ERR deserialize(DataInputStream *in, INDEX indexStart, int offset, INDEX targetNumElement,
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
    bool SWORDFISH_API getDecimal(INDEX /*start*/, int len, int scale, R *buf) const;

    template <typename R>
    bool SWORDFISH_API setDecimal(INDEX /*start*/, int len, int scale, const R *buf);

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

} // namespace ddb

#endif /* SCALARIMP_H_ */
