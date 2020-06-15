/*
 * CoreConcept.h
 *
 *  Created on: Apr 20, 2017
 *      Author: dzhou
 */

#ifndef CORECONCEPT_H_
#define CORECONCEPT_H_

#include <string>
#include <unordered_map>
#include <vector>
#include <deque>
#include <algorithm>

#include "Types.h"
#include "SmartPointer.h"
#include "Exceptions.h"
#include "Concurrent.h"
#include "IO.h"

using std::string;
using std::vector;
using std::unordered_map;
using std::deque;
using std::pair;

class User;
class BinaryOperator;
class ByteArrayBuffer;
class Heap;
class Object;
class Operator;
class Statement;
class Param;
class FunctionDef;
class Constant;
class Vector;
class Matrix;
class Table;
class Set;
class Dictionary;
class SQLTransaction;
class SQLContext;
class ColumnRef;
class SymbolBase;
class SymbolBaseManager;
class Output;
class Console;
class Session;
class ConstantMarshall;
class ConstantUnmarshall;
class DebugContext;
class DomainSite;
class DomainPartition;
class Domain;
struct TableUpdate;

typedef SmartPointer<User> UserSP;
typedef SmartPointer<ByteArrayBuffer> ByteArrayBufferSP;
typedef SmartPointer<Constant> ConstantSP;
typedef SmartPointer<Vector> VectorSP;
typedef SmartPointer<Matrix> MatrixSP;
typedef SmartPointer<Object> ObjectSP;
typedef SmartPointer<Operator> OperatorSP;
typedef SmartPointer<Statement> StatementSP;
typedef SmartPointer<Param> ParamSP;
typedef SmartPointer<FunctionDef> FunctionDefSP;
typedef SmartPointer<Heap> HeapSP;
typedef SmartPointer<Table> TableSP;
typedef SmartPointer<Set> SetSP;
typedef SmartPointer<Dictionary> DictionarySP;
typedef SmartPointer<SQLTransaction> SQLTransactionSP;
typedef SmartPointer<SQLContext> SQLContextSP;
typedef SmartPointer<ColumnRef> ColumnRefSP;
typedef SmartPointer<SymbolBase> SymbolBaseSP;
typedef SmartPointer<SymbolBaseManager> SymbolBaseManagerSP;
typedef SmartPointer<Output> OutputSP;
typedef SmartPointer<Console> ConsoleSP;
typedef SmartPointer<Session> SessionSP;
typedef SmartPointer<ConstantMarshall> ConstantMarshallSP;
typedef SmartPointer<ConstantUnmarshall> ConstantUnmarshallSP;
typedef SmartPointer<DebugContext> DebugContextSP;
typedef SmartPointer<DomainSite> DomainSiteSP;
typedef SmartPointer<DomainPartition> DomainPartitionSP;
typedef SmartPointer<Domain> DomainSP;
typedef SmartPointer<TableUpdate> TableUpdateSP;
typedef SmartPointer<SynchronizedQueue<TableUpdate> > TableUpdateQueueSP;

typedef ConstantSP(*OptrFunc)(const ConstantSP&,const ConstantSP&);
typedef ConstantSP(*SysFunc)(Heap* heap,vector<ConstantSP>& arguments);
typedef ConstantSP(*TemplateOptr)(const ConstantSP&,const ConstantSP&,const string&, OptrFunc);
typedef ConstantSP(*TemplateUserOptr)(Heap* heap, const ConstantSP&,const ConstantSP&, const FunctionDefSP&);
typedef void (*SysProc)(Heap* heap,vector<ConstantSP>& arguments);

class User {
public:
	User(const string& name, const string& pwd, bool isAdmin, bool tableRead, const string& readTables, bool tableWrite,
			const string& writeTables, bool readView, const string& views, bool execScript, bool unitTest);
	inline bool isAdmin() const {return isAdmin_;}
	inline const string& getName() const {return name_;}
	bool verify(const string& password) const;
	bool canReadTable(const string& name) const;
	bool canWriteTable(const string& name) const;
	bool canReadView(const string& name) const;
	bool canUnitTest() const {return unitTest_;}
	bool canExecScript() const {return execScript_;}
	static UserSP createAdminUser();
	static UserSP createGuestUser();

private:
	string name_;
	string pwd_;
	bool isAdmin_;
	bool tableRead_;
	string readTables_;
	bool tableWrite_;
	string writeTables_;
	bool viewRead_;
	string views_;
	bool execScript_;
	bool unitTest_;
};

class Guid {
public:
	Guid(bool newGuid);
	Guid(unsigned char* guid);
	Guid(const string& guid);
	Guid(const Guid& copy);
	bool operator==(const Guid &other) const;
	bool operator!=(const Guid &other) const;
	bool isZero() const;
    string getString() const;
    inline const unsigned char* bytes() const { return uuid_;}

private:
    unsigned char hexDigitToChar(char ch) const;
    inline unsigned char hexPairToChar(char a, char b) const { return hexDigitToChar(a) * 16 + hexDigitToChar(b);}
    void charToHexPair(unsigned char ch, char* buf) const;

private:
	unsigned char uuid_[16];
};

struct GuidHash {
	inline size_t operator()(const Guid& guid) const {
		const unsigned char* key = guid.bytes();
		const uint32_t m = 0x5bd1e995;
		const int r = 24;
		uint32_t h = 16;

		uint32_t k1 = *(uint32_t*)(key);
		uint32_t k2 = *(uint32_t*)(key + 4);
		uint32_t k3 = *(uint32_t*)(key + 8);
		uint32_t k4 = *(uint32_t*)(key + 12);

		k1 *= m;
		k1 ^= k1 >> r;
		k1 *= m;

		k2 *= m;
		k2 ^= k2 >> r;
		k2 *= m;

		k3 *= m;
		k3 ^= k3 >> r;
		k3 *= m;

		k4 *= m;
		k4 ^= k4 >> r;
		k4 *= m;

		// Mix 4 bytes at a time into the hash
		h *= m;
		h ^= k1;
		h *= m;
		h ^= k2;
		h *= m;
		h ^= k3;
		h *= m;
		h ^= k4;

		// Do a few final mixes of the hash to ensure the last few
		// bytes are well-incorporated.
		h ^= h >> 13;
		h *= m;
		h ^= h >> 15;

		return h;
	}
};

template<class T>
class Array{
public:
	Array(int capacity) : data_(new T[capacity]), size_(0), capacity_(capacity){}
	Array(T* data, int size, int capacity): data_(data), size_(size), capacity_(capacity){}
	~Array(){delete[] data_;}
	const T& at(int index) const { return data_[index];}
	const T* getDataReference() const { return data_;}
	bool append(const T& val){
		if(size_ >= capacity_)
			return false;
		data_[size_++] = val;
		return true;
	}
	int size() const {return size_;}
	int capacity() const {return capacity_;}
	void clear(){ size_ = 0;}

private:
	T* data_;
	int size_;
	int capacity_;
};

template<class T>
class DynamicArray{
public:
	DynamicArray(int segmentSizeInBit, int segmentCapacity): dataSegment_(new T*[segmentCapacity]), segmentSize_(1<<segmentSizeInBit),
		segmentSizeInBit_(segmentSizeInBit), segmentMask_((1<<segmentSizeInBit) - 1), segmentCapacity_(segmentCapacity),
		size_(0), sizeInSegment_(0){}
	~DynamicArray(){
		for(int i=0; i<sizeInSegment_; ++i)
			delete[] dataSegment_[i];
		delete[] dataSegment_;
	}
	const T& at(int index) const { return dataSegment_[index >> segmentSizeInBit_][index & segmentMask_];}
	const T& operator [](int index){ return dataSegment_[index >> segmentSizeInBit_][index & segmentMask_];}
	bool push_back(const T& val){
		if((size_ >> segmentSizeInBit_) + 1 > sizeInSegment_){
			if(sizeInSegment_ >= segmentCapacity_)
				return false;
			else{
				dataSegment_[sizeInSegment_++] = new T[segmentSize_];
			}
		}
		dataSegment_[size_ >> segmentSizeInBit_][size_ & segmentMask_] = val;
		++size_;
		return true;
	}
	int size() const {return size_;}
	int capacity() const {return segmentSize_ * segmentCapacity_;}
	void clear(){
		size_ = 0;
		sizeInSegment_ = 0;
	}

private:
	T** dataSegment_;
	int segmentSize_; // the number of elements in one segment. it must be the power of 2, e.g. 2, 4, 8, 16...
	int segmentSizeInBit_;
	int segmentMask_; // 1<<segmentSizeInBit - 1
	int segmentCapacity_; // total number of segments available
	int size_; // the number of elements
	int sizeInSegment_;
};

class ByteArrayBuffer{
public:
	ByteArrayBuffer(size_t capacity) : buf_(new char[capacity]), capacity_(capacity), size_(0), constants_(0){}
	ByteArrayBuffer() : buf_(new char[2048]), capacity_(2048), size_(0), constants_(0){}
	~ByteArrayBuffer() {delete constants_;}
	IO_ERR write(const char* buffer, int length, int& actualLength);
	IO_ERR write(const char* buffer, int length);
	IO_ERR write(const ConstantSP& obj);
	inline IO_ERR write(const string& buffer){ return write(buffer.c_str(), buffer.length() + 1);}
	inline IO_ERR write(bool val){ return write((const char*)&val, 1);}
	inline IO_ERR write(char val){ return write(&val, 1);}
	inline IO_ERR write(short val){ return write((const char*)&val, 2);}
	inline IO_ERR write(unsigned short val){ return write((const char*)&val, 2);}
	inline IO_ERR write(int val){ return write((const char*)&val, 4);}
	inline IO_ERR write(long long val){ return write((const char*)&val, 8);}
	inline IO_ERR write(float val){ return write((const char*)&val, 4);}
	inline IO_ERR write(double val){ return write((const char*)&val, 8);}
	size_t size() const { return size_;}
	size_t capacity() const { return capacity_;}
	const char * getBuffer() const { return buf_;}
	void clear();
	int getConstantCount() const {return constants_ == 0 ? 0 :constants_->size();}
	const ConstantSP& getConstant(int index) const { return constants_->at(index);}

private:
	char* buf_;
	size_t capacity_;
	size_t size_;
	vector<ConstantSP>* constants_;
};

class SymbolBase{
public:
	SymbolBase();
	SymbolBase(const string& symbolFile);
	bool reload(const string& symbolFile);
	bool saveSymbolBase(const string& symbolFile, string& errMsg);
	int find(const string& symbol);
	void find(const char** symbols, int size, int* indices);
	int findAndInsert(const string& symbol);
	void getOrdinalCandidate(const string& symbol, int& ordinal, SmartPointer<Array<int> >& ordinalBase);
	void getOrdinalCandidate(const string& symbol1, const string& symbol2, int& ordinal1, int& ordinal2, SmartPointer<Array<int> >& ordinalBase);
	bool getSortedIndices(int* indices, int length, bool asc);
	SmartPointer<Array<int> > getOrdinalBase() const {return ordinal_;}
	inline const string& getSymbol(int index) { return key_[index];}
	int size() const {return  key_.size();}
	const Guid& getID(){return baseID_;}
	bool isModified(){return modified_;}
	int getVersion(){return version_;}
    const string& getDBPath() const {return dbPath_;}
    void setDBPath(const string& dbPath) { dbPath_ = dbPath;}
	static int readSymbolBaseVersion(const string symbolFile);

private:
	int getOrdinalCandidate(const string& symbol);
	void reAssignOrdinal(int capacity);

private:
	Guid baseID_;
	int interval_;
	bool modified_;
	int version_;
	string dbPath_;
	DynamicArray<string> key_;
	SmartPointer<Array<int> > ordinal_;
	/* TODO: Change the keyMap_ to lock free version. This will improve the performance of find and findAndInsert.
	 * After introducing lock-free hash table, we can remove the lock on find method. The findAndInsert needs a lock
	 * only in the case of insert.
	 */
	unordered_map<string,int> keyMap_;
	deque<int> sortedIndices_;
	Mutex writeMutex_;
};

class SymbolBaseManager{
public:
	SymbolBaseManager(const string& symbolBaseDir);
	bool contains(const SymbolBaseSP& symbase);
	int getSymbolBaseID(const SymbolBaseSP& symbase);
	int findAndInsert(const SymbolBaseSP& symbase);
	SymbolBaseSP findAndLoad(int symBaseID);
	SymbolBaseSP reloadForModification(int symBaseID);
	bool saveSymbolBase(const SymbolBaseSP& symbase, string& errMsg);
	string getSymbolFile(int baseID);
	void getSymbolBases(vector<int> symBaseIDs);

private:
	int checkSymbolBaseVersion(int baseID);

private:
	string dir_;
	unordered_map<Guid, int, GuidHash> symbases_;
	unordered_map<int, SymbolBaseSP> baseIDs_;
	Mutex mutex_;
};

class Object {
public:
	bool isConstant() const {return getObjectType()==CONSTOBJ;}
	bool isVariable() const {return getObjectType()==VAR;}
	virtual ConstantSP getValue(Heap* pHeap) = 0;
	virtual ConstantSP getReference(Heap* pHeap) = 0;
	virtual OBJECT_TYPE getObjectType() const = 0;
	virtual ~Object(){}
	virtual string getScript() const = 0;
	virtual IO_ERR serialize(const ByteArrayBufferSP& buffer) const = 0;
	virtual void collectUserDefinedFunctions(unordered_map<string,FunctionDef*>& functionDefs) const {}
	virtual void collectVariables(vector<int>& vars, int minIndex, int maxIndex) const {}
	virtual bool isLargeConstant() const {return false;}
};

class Constant: public Object{
public:
	static string EMPTY;
	static string NULL_STR;

	Constant() : flag_(3){}
	Constant(unsigned short flag) :  flag_(flag){}
	virtual ~Constant(){}

	inline bool isTemporary() const {return flag_ & 1;}
	inline void setTemporary(bool val){ if(val) flag_ |= 1; else flag_ &= ~1;}
	inline bool isIndependent() const {return flag_ & 2;}
	inline void setIndependent(bool val){ if(val) flag_ |= 2; else flag_ &= ~2;}
	inline bool isReadOnly() const {return flag_ & 4;}
	inline void setReadOnly(bool val){ if(val) flag_ |= 4; else flag_ &= ~4;}
	inline bool isReadOnlyArgument() const {return flag_ & 8;}
	inline void setReadOnlyArgument(bool val){ if(val) flag_ |= 8; else flag_ &= ~8;}
	inline bool isNothing() const {return flag_ & 16;}
	inline void setNothing(bool val){ if(val) flag_ |= 16; else flag_ &= ~16;}
	inline bool isStatic() const {return flag_ & 32;}
	inline void setStatic(bool val){ if(val) flag_ |= 32; else flag_ &= ~32;}
	inline DATA_FORM getForm() const {return DATA_FORM(flag_ >> 8);}
	inline void setForm(DATA_FORM df){ flag_ = (flag_ & 127) + (df << 8);}
	inline bool isScalar() const { return getForm()==DF_SCALAR;}
	inline bool isArray() const { return getForm()==DF_VECTOR;}
	inline bool isPair() const { return getForm()==DF_PAIR;}
	inline bool isMatrix() const {return getForm()==DF_MATRIX;}
	//a vector could be array, pair or matrix.
	inline bool isVector() const { return getForm()>=DF_VECTOR && getForm()<=DF_MATRIX;}
	inline bool isTable() const { return getForm()==DF_TABLE;}
	inline bool isSet() const {return getForm()==DF_SET;}
	inline bool isDictionary() const {return getForm()==DF_DICTIONARY;}
	bool isTuple() const {return getForm()==DF_VECTOR && getType()==DT_ANY;}
	bool isNumber() const { DATA_CATEGORY cat = getCategory(); return cat == INTEGRAL || cat == FLOATING;}

	virtual char getBool() const {throw RuntimeException("The object can't be converted to boolean scalar.");}
	virtual char getChar() const {throw RuntimeException("The object can't be converted to char scalar.");}
	virtual short getShort() const {throw RuntimeException("The object can't be converted to short scalar.");}
	virtual int getInt() const {throw RuntimeException("The object can't be converted to int scalar.");}
	virtual long long  getLong() const {throw RuntimeException("The object can't be converted to long scalar.");}
	virtual INDEX getIndex() const {throw RuntimeException("The object can't be converted to index scalar.");}
	virtual float getFloat() const {throw RuntimeException("The object can't be converted to float scalar.");}
	virtual double getDouble() const {throw RuntimeException("The object can't be converted to double scalar.");}
	virtual string getString() const {return "";}
	virtual string getScript() const { return getString();}
	virtual const string& getStringRef() const {return EMPTY;}
	virtual bool isNull() const {return false;}

	virtual void setBool(char val){}
	virtual void setChar(char val){}
	virtual void setShort(short val){}
	virtual void setInt(int val){}
	virtual void setLong(long long val){}
	virtual void setIndex(INDEX val){}
	virtual void setFloat(float val){}
	virtual void setDouble(double val){}
	virtual void setString(const string& val){}
	virtual void setNull(){}

	virtual char getBool(INDEX index) const {return getBool();}
	virtual char getChar(INDEX index) const { return getChar();}
	virtual short getShort(INDEX index) const { return getShort();}
	virtual int getInt(INDEX index) const {return getInt();}
	virtual long long getLong(INDEX index) const {return getLong();}
	virtual INDEX getIndex(INDEX index) const {return getIndex();}
	virtual float getFloat(INDEX index) const {return getFloat();}
	virtual double getDouble(INDEX index) const {return getDouble();}
	virtual string getString(INDEX index) const {return getString();}
	virtual const string& getStringRef(INDEX index) const {return EMPTY;}
	virtual bool isNull(INDEX index) const {return isNull();}

	virtual ConstantSP get(INDEX index) const {return getValue();}
	virtual ConstantSP get(INDEX column, INDEX row) const {return get(row);}
	virtual ConstantSP get(const ConstantSP& index) const {return getValue();}
	virtual ConstantSP getColumn(INDEX index) const {return getValue();}
	virtual ConstantSP getRow(INDEX index) const {return get(index);}
	virtual ConstantSP getItem(INDEX index) const {return get(index);}
	virtual ConstantSP getWindow(INDEX colStart, int colLength, INDEX rowStart, int rowLength) const {return getValue();}
	virtual ConstantSP getRowLabel() const;
	virtual ConstantSP getColumnLabel() const;

	virtual bool isNull(INDEX start, int len, char* buf) const {return false;}
	virtual bool isValid(INDEX start, int len, char* buf) const {return false;}
	virtual bool getBool(INDEX start, int len, char* buf) const {return false;}
	virtual bool getChar(INDEX start, int len,char* buf) const {return false;}
	virtual bool getShort(INDEX start, int len, short* buf) const {return false;}
	virtual bool getInt(INDEX start, int len, int* buf) const {return false;}
	virtual bool getLong(INDEX start, int len, long long* buf) const {return false;}
	virtual bool getIndex(INDEX start, int len, INDEX* buf) const {return false;}
	virtual bool getFloat(INDEX start, int len, float* buf) const {return false;}
	virtual bool getDouble(INDEX start, int len, double* buf) const {return false;}
	virtual bool getSymbol(INDEX start, int len, int* buf, SymbolBase* symBase,bool insertIfNotThere) const {return false;}
	virtual bool getString(INDEX start, int len, string** buf) const {return false;}
	virtual bool getString(INDEX start, int len, char** buf) const {return false;}

	virtual const char* getBoolConst(INDEX start, int len, char* buf) const {throw RuntimeException("getBoolConst method not supported");}
	virtual const char* getCharConst(INDEX start, int len,char* buf) const {throw RuntimeException("getCharConst method not supported");}
	virtual const short* getShortConst(INDEX start, int len, short* buf) const {throw RuntimeException("getShortConst method not supported");}
	virtual const int* getIntConst(INDEX start, int len, int* buf) const {throw RuntimeException("getIntConst method not supported");}
	virtual const long long* getLongConst(INDEX start, int len, long long* buf) const {throw RuntimeException("getLongConst method not supported");}
	virtual const INDEX* getIndexConst(INDEX start, int len, INDEX* buf) const {throw RuntimeException("getIndexConst method not supported");}
	virtual const float* getFloatConst(INDEX start, int len, float* buf) const {throw RuntimeException("getFloatConst method not supported");}
	virtual const double* getDoubleConst(INDEX start, int len, double* buf) const {throw RuntimeException("getDoubleConst method not supported");}
	virtual const int* getSymbolConst(INDEX start, int len, int* buf, SymbolBase* symBase, bool insertIfNotThere) const {throw RuntimeException("getSymbolConst method not supported");}
	virtual string** getStringConst(INDEX start, int len, string** buf) const {throw RuntimeException("getStringConst method not supported");}
	virtual char** getStringConst(INDEX start, int len, char** buf) const {throw RuntimeException("getStringConst method not supported");}

	virtual char* getBoolBuffer(INDEX start, int len, char* buf) const {return buf;}
	virtual char* getCharBuffer(INDEX start, int len,char* buf) const {return buf;}
	virtual short* getShortBuffer(INDEX start, int len, short* buf) const {return buf;}
	virtual int* getIntBuffer(INDEX start, int len, int* buf) const {return NULL;}
	virtual long long* getLongBuffer(INDEX start, int len, long long* buf) const {return buf;}
	virtual INDEX* getIndexBuffer(INDEX start, int len, INDEX* buf) const {return buf;}
	virtual float* getFloatBuffer(INDEX start, int len, float* buf) const {return buf;}
	virtual double* getDoubleBuffer(INDEX start, int len, double* buf) const {return buf;}
	virtual void* getDataBuffer(INDEX start, int len, void* buf) const {return buf;}

	virtual IO_ERR serialize(const ByteArrayBufferSP& buffer) const {throw RuntimeException("code serialize method not supported");}
    virtual int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int& numElement, int& partial) const {throw RuntimeException("serialize method not supported");}
    virtual IO_ERR deserialize(DataInputStream* in, INDEX indexStart, INDEX targetNumElement, INDEX& numElement) {throw RuntimeException("deserialize method not supported");}

	virtual void nullFill(const ConstantSP& val){}
	virtual void setBool(INDEX index,bool val){setBool(val);}
	virtual void setChar(INDEX index,char val){setChar(val);}
	virtual void setShort(INDEX index,short val){setShort(val);}
	virtual void setInt(INDEX index,int val){setInt(val);}
	virtual void setLong(INDEX index,long long val){setLong(val);}
	virtual void setIndex(INDEX index,INDEX val){setIndex(val);}
	virtual void setFloat(INDEX index,float val){setFloat(val);}
	virtual void setDouble(INDEX index, double val){setDouble(val);}
	virtual void setString(INDEX index, const string& val){setString(val);}
	virtual void setNull(INDEX index){setNull();}

	virtual bool set(INDEX index, const ConstantSP& value){return false;}
	virtual bool set(INDEX column, INDEX row, const ConstantSP& value){return false;}
	virtual bool set(const ConstantSP& index, const ConstantSP& value) {return false;}
	virtual bool setItem(INDEX index, const ConstantSP& value){return set(index,value);}
	virtual bool setColumn(INDEX index, const ConstantSP& value){return assign(value);}
	virtual void setRowLabel(const ConstantSP& label){}
	virtual void setColumnLabel(const ConstantSP& label){}
	virtual bool reshape(INDEX cols, INDEX rows) {return false;}
	virtual bool assign(const ConstantSP& value){return false;}

	virtual bool setBool(INDEX start, int len, const char* buf){return false;}
	virtual bool setChar(INDEX start, int len, const char* buf){return false;}
	virtual bool setShort(INDEX start, int len, const short* buf){return false;}
	virtual bool setInt(INDEX start, int len, const int* buf){return false;}
	virtual bool setLong(INDEX start, int len, const long long* buf){return false;}
	virtual bool setIndex(INDEX start, int len, const INDEX* buf){return false;}
	virtual bool setFloat(INDEX start, int len, const float* buf){return false;}
	virtual bool setDouble(INDEX start, int len, const double* buf){return false;}
	virtual bool setString(INDEX start, int len, const string* buf){return false;}
	virtual bool setString(INDEX start, int len, char** buf){return false;}
	virtual bool setData(INDEX start, int len, void* buf) {return false;}

	virtual bool add(INDEX start, INDEX length, long long inc) { return false;}
	virtual bool add(INDEX start, INDEX length, double inc) { return false;}

	virtual int compare(INDEX index, const ConstantSP& target) const {return 0;}

	virtual bool getNullFlag() const {return isNull();}
	virtual void setNullFlag(bool containNull){}
	virtual bool hasNull(){return  isNull();}
	virtual bool hasNull(INDEX start, INDEX length){return isNull();}
	virtual bool sizeable() const {return false;}
	virtual bool copyable() const {return true;}
	virtual INDEX size() const {return 1;}
	virtual INDEX itemCount() const {return getForm()==DF_MATRIX?columns():size();}
	virtual INDEX rows() const {return size();}
	virtual INDEX columns() const {return 1;};
	virtual ConstantSP getMember(const ConstantSP& key) const { throw RuntimeException("getMember method not supported");}
	virtual ConstantSP keys() const {throw RuntimeException("keys method not supported");}
	virtual ConstantSP values() const {throw RuntimeException("values method not supported");}

	virtual long long releaseMemory(long long target, bool& satisfied) { satisfied = false; return 0;}
	virtual long long getAllocatedMemory() const {return 0;}
	virtual DATA_TYPE getType() const =0;
	virtual DATA_TYPE getRawType() const =0;
	virtual int getExtraParamForType() const { return 0;}
	virtual DATA_CATEGORY getCategory() const =0;

	virtual ConstantSP getInstance() const =0;
	virtual ConstantSP getValue() const =0;
	virtual ConstantSP getValue (Heap* pHeap){return getValue();}
	virtual ConstantSP getReference(Heap* pHeap){return getValue();}
	virtual OBJECT_TYPE getObjectType() const {return CONSTOBJ;}
	virtual SymbolBaseSP getSymbolBase() const {return SymbolBaseSP();}
	virtual void contain(const ConstantSP& target, const ConstantSP& resultSP) const {throw RuntimeException("contain method not supported");}
	virtual bool isFastMode() const {return false;}
	virtual void* getDataArray() const {return 0;}
	virtual void** getDataSegment() const {return 0;}
	virtual bool isIndexArray() const { return false;}
	virtual INDEX* getIndexArray() const { return NULL;}
	virtual bool isHugeIndexArray() const { return false;}
	virtual INDEX** getHugeIndexArray() const { return NULL;}
	virtual int getSegmentSize() const { return 1;}
	virtual int getSegmentSizeInBit() const { return 0;}
	virtual bool containNotMarshallableObject() const {return false;}
private:
	unsigned short flag_;
};

class Vector:public Constant{
public:
	Vector(): Constant(259){}
	virtual ~Vector(){}
	virtual ConstantSP getColumnLabel() const;
	const string& getName() const {return name_;}
	void setName(const string& name){name_=name;}
	virtual bool isLargeConstant() const { return isMatrix() || size()>1024; }
	virtual bool isView() const {return false;}
	virtual void initialize(){}
	virtual int getCapacity() const = 0;
	virtual short getUnitLength() const = 0;
	virtual void clear()=0;
	virtual bool remove(INDEX count){return false;}
	virtual bool append(const ConstantSP& value){return append(value, value->size());}
	virtual bool append(const ConstantSP& value, INDEX count){return false;}
	virtual bool appendBool(char* buf, int len){return false;}
	virtual bool appendChar(char* buf, int len){return false;}
	virtual bool appendShort(short* buf, int len){return false;}
	virtual bool appendInt(int* buf, int len){return false;}
	virtual bool appendLong(long long* buf, int len){return false;}
	virtual bool appendIndex(INDEX* buf, int len){return false;}
	virtual bool appendFloat(float* buf, int len){return false;}
	virtual bool appendDouble(double* buf, int len){return false;}
	virtual bool appendString(string* buf, int len){return false;}
	virtual bool appendString(char** buf, int len){return false;}
	virtual string getString() const;
	virtual string getScript() const;
	virtual string getString(INDEX index) const = 0;
	virtual ConstantSP getInstance() const {return getInstance(size());}
	virtual ConstantSP getInstance(INDEX size) const = 0;
	virtual ConstantSP getValue(INDEX capacity) const {throw RuntimeException("Vector::getValue method not supported");}
	virtual ConstantSP get(INDEX column, INDEX rowStart,INDEX rowEnd) const {return getSubVector(column*rows()+rowStart,rowEnd-rowStart);}
	virtual ConstantSP get(INDEX index) const = 0;
	virtual ConstantSP getWindow(INDEX colStart, int colLength, INDEX rowStart, int rowLength) const {return getSubVector(rowStart,rowLength);}
	virtual ConstantSP getSubVector(INDEX start, INDEX length) const { throw RuntimeException("getSubVector method not supported");}
	virtual ConstantSP getSubVector(INDEX start, INDEX length, INDEX capacity) const { return getSubVector(start, length);}
	virtual void fill(INDEX start, INDEX length, const ConstantSP& value)=0;
	virtual void next(INDEX steps)=0;
	virtual void prev(INDEX steps)=0;
	virtual void reverse()=0;
	virtual void reverse(INDEX start, INDEX length)=0;
	virtual void shuffle(){}
	virtual void replace(const ConstantSP& oldVal, const ConstantSP& newVal){}
	virtual bool validIndex(INDEX uplimit){return false;}
	virtual bool validIndex(INDEX start, INDEX length, INDEX uplimit){return false;}
	virtual void neg()=0;
	virtual void find(const ConstantSP& target, const ConstantSP& resultSP){}
	virtual void binarySearch(const ConstantSP& target, const ConstantSP& resultSP){}
	virtual void asof(const ConstantSP& target, const ConstantSP& resultSP){ throw RuntimeException("asof method not supported.");}
	virtual void upper(){throw RuntimeException("upper method not supported");}
	virtual void lower(){throw RuntimeException("lower method not supported");}
	virtual void trim(){throw RuntimeException("trim method not supported");}
	virtual void strip(){throw RuntimeException("strip method not supported");}
	virtual long long count() const = 0;
	virtual long long count(INDEX start, INDEX length) const = 0;
	virtual ConstantSP minmax() const;
	virtual ConstantSP minmax(INDEX start, INDEX length) const;
	virtual ConstantSP max() const = 0;
	virtual ConstantSP max(INDEX start, INDEX length) const = 0;
	virtual INDEX imax() const = 0;
	virtual INDEX imax(INDEX start, INDEX length) const = 0;
	virtual ConstantSP min() const = 0;
	virtual ConstantSP min(INDEX start, INDEX length) const = 0;
	virtual INDEX imin() const = 0;
	virtual INDEX imin(INDEX start, INDEX length) const = 0;
	virtual ConstantSP avg() const = 0;
	virtual ConstantSP avg(INDEX start, INDEX length) const = 0;
	virtual ConstantSP sum() const = 0;
	virtual ConstantSP sum(INDEX start, INDEX length) const = 0;
	virtual ConstantSP sum2() const = 0;
	virtual ConstantSP sum2(INDEX start, INDEX length) const = 0;
	virtual ConstantSP prd() const = 0;
	virtual ConstantSP prd(INDEX start, INDEX length) const = 0;
	virtual ConstantSP var() const = 0;
	virtual ConstantSP var(INDEX start, INDEX length) const = 0;
	virtual ConstantSP std() const = 0;
	virtual ConstantSP std(INDEX start, INDEX length) const = 0;
	virtual ConstantSP median() const = 0;
	virtual ConstantSP median(INDEX start, INDEX length) const = 0;
	virtual ConstantSP searchK(INDEX k) const {throw RuntimeException("searchK method not supported");}
	virtual ConstantSP searchK(INDEX start, INDEX length, INDEX k) const {throw RuntimeException("searchK method not supported");}
	virtual ConstantSP mode() const = 0;
	virtual ConstantSP mode(INDEX start, INDEX length) const = 0;
	virtual ConstantSP stat() const;
	virtual ConstantSP stat(INDEX start, INDEX length) const;

	/**
	 * Find the first element that is no less than the target value in the sorted vector.
	 * start: the starting point of the search.
	 */
	virtual INDEX lowerBound(INDEX start, const ConstantSP& target)=0;

	virtual bool equalToPrior(INDEX start, INDEX length, bool* result){ return false;}
	virtual bool equalToPrior(INDEX prior, const INDEX* indices, INDEX length, bool* result){ return false;}

	/**
	 * Sort the whole vector with given order
	 * asc: indicating if it is ascending order
	 */
	virtual bool sort(bool asc) = 0;

	/**
	 * Sort the vector and the corresponding indices with given order
	 * asc: indicating if it is ascending order
	 * indices: the corresponding indices of the data to sort.The length of the data should be equal to
	 * the length of the indices. The indices will be rearranged accordingly during sorting
	 */
	virtual bool sort(bool asc, Vector* indices) = 0;

	/**
	 * Sort the selected indices based on the corresponding data with given order.
	 * indices: the selected indices to sort
	 * start: the start position in indices vector
	 * length: the number of indices to sort
	 * asc: indicating if it is ascending order
	 */
	virtual bool sortSelectedIndices(Vector* indices, INDEX start, INDEX length, bool asc) = 0;

    /**
     * Find duplicated elements in an ascending-sorted array
     * indices: indices of data to search. The underlying data was not sorted. However the indices
     *     are in the ascending order
     * start: the start position in indices vector
     * length: the number of indices to process
     * duplicates: output vector of the duplicated elements. pair.first stores the starting position
     *     of the duplicated elements in the indices array and pair.second has the number of duplicated elements.
     */
	virtual bool findDuplicatedElements(Vector* indices, INDEX start, INDEX length, vector<pair<INDEX,INDEX> >& duplicates) = 0;

	/**
	 * Find duplicated elements in the segment of an ascending-sorted array
	 * start: the starting position of the segment
	 * length: the length of the segment.
     * duplicates: output vector of the duplicated elements. pair.first stores the starting position
     *     of the duplicated elements in the array and pair.second has the number of duplicated elements.
	 */
	virtual bool findDuplicatedElements(INDEX start, INDEX length, vector<pair<INDEX,INDEX> >& duplicates)=0;

	virtual bool findUniqueElements(INDEX start, INDEX length, vector<pair<INDEX,INDEX> >& uniques)=0;

	virtual bool findRange(INDEX* ascIndices,const ConstantSP& target,INDEX* targetIndices,vector<pair<INDEX,INDEX> >& ranges)=0;
	virtual bool findRange(const ConstantSP& target,INDEX* targetIndices,vector<pair<INDEX,INDEX> >& ranges)=0;
private:
	string name_;
};

class Matrix{
public:
	Matrix(int cols, int rows);
	virtual ~Matrix(){}
	void setRowLabel(const ConstantSP& label);
	void setColumnLabel(const ConstantSP& label);
	bool reshape(INDEX cols, INDEX rows);
	string getString() const;
	string getString(INDEX index) const ;
	ConstantSP get(const ConstantSP& index) const ;
	bool set(const ConstantSP index, const ConstantSP& value);
	virtual string getString(int column, int row) const = 0;
	virtual ConstantSP getInstance(INDEX size) const = 0;
	virtual ConstantSP getColumn(INDEX index) const = 0;
	virtual bool setColumn(INDEX index, const ConstantSP& value)=0;
protected:
	int cols_;
	int rows_;
	ConstantSP rowLabel_;
	ConstantSP colLabel_;
};

class Set: public Constant {
public:
	Set() : Constant(1027){}
	virtual ~Set() {}
	virtual void clear()=0;
	virtual bool remove(const ConstantSP& value) = 0;
	virtual bool append(const ConstantSP& value) = 0;
	virtual bool inverse(const ConstantSP& value) = 0;
	virtual void contain(const ConstantSP& target, const ConstantSP& resultSP) const = 0;
	virtual bool isSuperset(const ConstantSP& target) const = 0;
	virtual ConstantSP interaction(const ConstantSP& target) const = 0;
	virtual ConstantSP getSubVector(INDEX start, INDEX length) const = 0;
	virtual string getScript() const {return "set()";}
	virtual bool isLargeConstant() const {return true;}
};

class Dictionary:public Constant{
public:
	Dictionary() : Constant(1283){}
	virtual ~Dictionary() {}
	virtual INDEX size() const = 0;
	virtual INDEX count() const = 0;
	virtual void clear()=0;
	virtual ConstantSP getMember(const ConstantSP& key) const =0;
	virtual ConstantSP getMember(const string& key) const {throw RuntimeException("String key not supported");}
	virtual ConstantSP get(INDEX column, INDEX row){throw RuntimeException("Dictionary does not support cell function");}
	virtual DATA_TYPE getKeyType() const = 0;
	virtual DATA_CATEGORY getKeyCategory() const = 0;
	virtual DATA_TYPE getType() const = 0;
	virtual DATA_CATEGORY getCategory() const = 0;
	virtual ConstantSP keys() const = 0;
	virtual ConstantSP values() const = 0;
	virtual string getString() const = 0;
	virtual string getScript() const {return "dict()";}
	virtual string getString(int index) const {throw RuntimeException("Dictionary::getString(int index) not supported");}
	virtual bool remove(const ConstantSP& key) = 0;
	virtual bool set(const ConstantSP& key, const ConstantSP& value)=0;
	virtual bool set(const string& key, const ConstantSP& value){throw RuntimeException("String key not supported");}
	virtual bool reduce(BinaryOperator& optr, const ConstantSP& key, const ConstantSP& value)=0;
	virtual ConstantSP get(const ConstantSP& key) const {return getMember(key);}
	virtual void contain(const ConstantSP& target, const ConstantSP& resultSP) const = 0;
	virtual bool isLargeConstant() const {return true;}

};

class Table: public Constant{
public:
	Table() : Constant(1539), flag_(0), lock_(0){}
	virtual ~Table();
	virtual string getScript() const {return getName();}
	virtual ConstantSP getColumn(const string& name) const = 0;
	virtual ConstantSP getColumn(const string& qualifier, const string& name) const = 0;
	virtual ConstantSP getColumn(INDEX index) const = 0;
	virtual ConstantSP getColumn(const string& name, const ConstantSP& rowFilter) const = 0;
	virtual ConstantSP getColumn(const string& qualifier, const string& name, const ConstantSP& rowFilter) const = 0;
	virtual ConstantSP getColumn(INDEX index, const ConstantSP& rowFilter) const = 0;
	virtual INDEX columns() const = 0;
	virtual const string& getColumnName(int index) const = 0;
	virtual const string& getColumnQualifier(int index) const = 0;
	virtual void setColumnName(int index, const string& name)=0;
	virtual int getColumnIndex(const string& name) const = 0;
	virtual DATA_TYPE getColumnType(int index) const = 0;
	virtual bool contain(const string& name) const = 0;
	virtual bool contain(const string& qualifier, const string& name) const = 0;
	virtual bool contain(ColumnRef* col) const = 0;
	virtual bool contain(const ColumnRefSP& col) const = 0;
	virtual bool containAll(const vector<ColumnRefSP>& cols) const = 0;
	virtual void setName(const string& name)=0;
	virtual const string& getName() const = 0;
	virtual ConstantSP get(INDEX index) const {return getColumn(index);}
	virtual ConstantSP get(const ConstantSP& index) const = 0;
	virtual ConstantSP getValue(INDEX capacity) const = 0;
	virtual TABLE_TYPE getTableType() const = 0;
	virtual bool append(vector<ConstantSP>& values, string& errMsg)=0;
	virtual bool update(vector<ConstantSP>& values, const ConstantSP& indexSP, vector<string>& colNames, string& errMsg)=0;
	virtual DATA_TYPE getType() const {return DT_DICTIONARY;}
	virtual DATA_TYPE getRawType() const {return DT_DICTIONARY;}
	virtual DATA_CATEGORY getCategory() const {return MIXED;}
	virtual bool isDistributedTable() const {return false;}
	virtual bool isSegmentedTable() const {return false;}
	virtual DomainSP getGlobalDomain() const {return DomainSP();}
	virtual DomainSP getLocalDomain() const {return DomainSP();}
	virtual int getGlobalPartitionColumnIndex() const {return -1;}
	virtual int getLocalPartitionColumnIndex() const {return -1;}
	virtual void setGlobalPartition(const DomainSP& domain, const string& partitionColumn){throw RuntimeException("Table::setGlobalPartition() not supported");}
	virtual bool isLargeConstant() const {return true;}
	virtual bool addSubscriber(const TableUpdateQueueSP& queue, INDEX offset = -1) { return false;}
	virtual bool removeSubscriber(const TableUpdateQueueSP& queue) { return false;}
	inline bool isSharedTable() const {return flag_ & 1;}
	inline bool isRealtimeTable() const {return flag_ & 2;}
	void setSharedTable(bool option);
	void setRealtimeTable(bool option);
	RWLock* getRWLock() const { return lock_;}
private:
	char flag_; //BIT0: shared table, BIT1: realtime table
	RWLock* lock_;
};

class Param{
public:
	Param(const string& name, bool readOnly):
		name_(name),readOnly_(readOnly),type_(DT_VOID){}
	Param(const DataInputStreamSP& in);
	string getName(){return name_;}
	bool isReadOnly(){return readOnly_;}
	DATA_TYPE  getType(){return type_;}
	IO_ERR serialize(const ByteArrayBufferSP& buffer) const;
	string getScript(int indention) const;
private:
	string name_;
	bool readOnly_;
	DATA_TYPE type_;
};

class FunctionDef : public Constant{
public:
	FunctionDef(FUNCTIONDEF_TYPE defType, const string& name, const vector<ParamSP>& params, bool hasReturnValue=true, bool aggregation=false);
	FunctionDef(FUNCTIONDEF_TYPE defType, const string& name, int minParamNum, int maxParamNum, bool hasReturnValue, bool aggregation=false);
	inline const string& getName() const { return name_;}
	inline const string& getModule() const { return module_;}
	inline string getFullName() const { return module_.empty() ? name_ : module_ + "::" + name_;}
	inline void setModule(const string& module) { module_ = module;}
	inline bool hasReturnValue() const {return flag_ & 1;}
	inline bool isAggregatedFunction() const {	return flag_ & 2;}
	inline bool allConstantParameters() const { return flag_ & 4;}
	inline bool bySystemUser() const { return flag_ & 8;}
	inline void bySystemUser(bool option) { if(option) flag_ |= 8; else flag_ &= ~8;}
	inline bool isView() const { return flag_ & 16;}
	inline void setView(bool option) {if(option) flag_ |= 16; else flag_ &= ~16;}
	inline bool variableParamNum() const {	return minParamNum_<maxParamNum_;}
	inline int getMaxParamCount() const { return maxParamNum_;}
	inline int getMinParamCount() const {	return minParamNum_;}
	inline bool acceptParamCount(int count) const { return minParamNum_ <= count && maxParamNum_ >= count;}
	inline int getParamCount() const {return minParamNum_;}
	const ParamSP& getParam(int index) const;
	inline bool isUserDefined() const {return defType_ == USERDEFFUNC;}
	inline bool isSystemFunction() const {return defType_ == SYSFUNC;}
	inline FUNCTIONDEF_TYPE getFunctionDefType() const {return defType_;}
	inline unsigned char getFlag() const { return flag_;}
	virtual bool copyable() const {return false;}
	virtual DATA_TYPE getType() const {return DT_FUNCTIONDEF;}
	virtual DATA_TYPE getRawType() const { return DT_STRING;}
	virtual DATA_CATEGORY getCategory() const {return SYSTEM;}
	virtual string getScript() const {return name_;}
	virtual string getString() const {return name_;}
	virtual IO_ERR serialize(const ByteArrayBufferSP& buffer) const = 0;
	virtual FunctionDefSP materializeFunctionDef(Heap* pHeap) { return FunctionDefSP();}
	virtual ConstantSP call(Heap* pHeap, vector<ConstantSP>& arguments) = 0;
	virtual ConstantSP call(Heap* pHeap, const ConstantSP& a, const ConstantSP& b) = 0;
	virtual ConstantSP call(Heap* pHeap,vector<ObjectSP>& arguments) = 0;
	virtual bool containNotMarshallableObject() const {return defType_ >= PARTIALFUNC ;}

protected:
	void setConstantParameterFlag();
	void setReturnValueFlag(bool val);
	void setAggregationFlag(bool val);

protected:
	static ParamSP constParam_;

	FUNCTIONDEF_TYPE defType_;
	string name_;
	string module_;
	vector<ParamSP> params_;
	int minParamNum_;
	int maxParamNum_;
	unsigned char flag_;
};

class SQLTransaction {
public:
	INDEX size(Table* table) const;
	bool addTable(Table* table, INDEX size);

private:
	vector<pair<Table*, INDEX>> tableSize_;
};

class SQLContext{
public:
	SQLContext();
	SQLContext(const SQLTransactionSP& trans);
	void setTable(const TableSP& table);
	void setFilter(const ConstantSP& filter);
	SQLTransactionSP getTransaction() const { return transSP_;}
	TableSP getTable() const{return tableSP_;}
	ConstantSP getFilter() const{return filterSP_;}
	ConstantSP getColumn(const string& qualifier, const string& name);
	ConstantSP getColumn(const string& name);
	void cacheColumn(const string& name,const ConstantSP& col);
	void enableCache();
	void disableCache(){cache_=false;}
	void clear();
	inline bool isInitialized() const { return !tableSP_.isNull();}
	inline bool isNotInitialized() const { return tableSP_.isNull();}

private:
	TableSP tableSP_;
	ConstantSP filterSP_;
	bool cache_;
	DictionarySP cachedCols_;
	SQLTransactionSP transSP_;
};

class ColumnRef: public Object{
public:
	ColumnRef(const SQLContextSP& contextSP, const string& name):contextSP_(contextSP),name_(name),index_(-1),acceptFunctionDef_(true){}
	ColumnRef(const SQLContextSP& contextSP, const string& qualifier, const string& name):contextSP_(contextSP),
			qualifier_(qualifier),name_(name),index_(-1),acceptFunctionDef_(true){}
	ColumnRef(const SQLContextSP& contextSP, const string& name, int index):contextSP_(contextSP),name_(name),index_(index),acceptFunctionDef_(true){}
	ColumnRef(const SQLContextSP& contextSP, const string& qualifier, const string& name, int index):contextSP_(contextSP),
				qualifier_(qualifier),name_(name),index_(index),acceptFunctionDef_(true){}
	ColumnRef(const SQLContextSP& context, const DataInputStreamSP& in);
	virtual ~ColumnRef(){}
	virtual ConstantSP getValue(Heap* pHeap);
	virtual ConstantSP getReference(Heap* pHeap);
	virtual OBJECT_TYPE getObjectType() const {return COLUMN;}
	const string& getQualifier() const { return qualifier_;}
	const string& getName() const { return name_;}
	string getFullname() const { return qualifier_.empty()? name_: qualifier_+"."+name_;}
	int getIndex() const { return index_;}
	DATA_TYPE getType() const;
	void setType(DATA_TYPE type);
	void setAcceptFunctionDef(bool option) { acceptFunctionDef_ = option;}
	virtual string getScript() const;
	virtual IO_ERR serialize(const ByteArrayBufferSP& buffer) const;
	ColumnRef* copy(const SQLContextSP& contextSP) const{ return new ColumnRef(contextSP, qualifier_, name_, index_);}
	ColumnRef* localize(const SQLContextSP& contextSP) const{ return new ColumnRef(contextSP, qualifier_, name_);}
	bool operator ==(const ColumnRef& target){ return  name_ == target.name_ && (qualifier_.empty() || target.qualifier_.empty() ||qualifier_ == target.qualifier_);}
	virtual void collectVariables(vector<int>& vars, int minIndex, int maxIndex) const { if(index_<=maxIndex && index_>=minIndex) vars.push_back(index_);}

private:
	SQLContextSP contextSP_;
	string qualifier_;
	string name_;
	int index_;
	bool acceptFunctionDef_;
};

class Operator{
public:
	Operator(int priority, bool unary): priority_(priority), unary_(unary){}
	virtual ~Operator(){}
	int getPriority() const {return priority_;}
	bool isUnary() const {return unary_;}
	virtual ConstantSP evaluate(Heap* heap, const ConstantSP& a, const ConstantSP& b) = 0;
	virtual const string& getName() const = 0;
	virtual string getOperatorSymbol() const  = 0;
	virtual string getTemplateSymbol() const = 0;
	virtual bool isPrimitiveOperator() const = 0;
	virtual IO_ERR serialize(const ByteArrayBufferSP& buffer) const = 0;
	virtual void collectUserDefinedFunctions(unordered_map<string,FunctionDef*>& functionDefs) const = 0;

private:
	int priority_;
	bool unary_;
};

class Output {
public:
	virtual bool timeElapsed(long long nanoSeconds) = 0;
	virtual bool write(const ConstantSP& obj) = 0;
	virtual bool message(const string& msg) = 0;
	virtual void enableIntermediateMessage(bool enabled) = 0;
	virtual bool start() = 0;
	virtual bool start(const string& message) = 0;
	virtual IO_ERR done()=0;
	virtual IO_ERR done(const string& errMsg) = 0;
	virtual IO_ERR writeReady() = 0;
	virtual bool needWrite() = 0;
	virtual ~Output(){};
	virtual void close() = 0;
	virtual OUTPUT_TYPE getOutputType() const = 0;
	virtual void setWindow(INDEX offset, INDEX size) = 0;
};

class Session {
public:
	Session(const HeapSP& heap);
	Session(const HeapSP& heap, bool systemSession);
	virtual ~Session(){}
	virtual bool run(const vector<string>& source)=0;
	virtual bool run(const string& scriptFile)=0;
	virtual bool run(const ObjectSP& script)=0;
	virtual bool run(const string& function, vector<ConstantSP>& params)=0;
	virtual bool run(const FunctionDefSP& function, vector<ConstantSP>& params)=0;
	virtual bool run(const vector<string>& variables, vector<ConstantSP>& params)=0;
	virtual bool test(const string& scriptFile, const string& resultFile, bool testMemoryLeaking)=0;
	virtual FunctionDefSP parseFunctionDef(const string& script) = 0;
	virtual bool contain(const string& key) const =0;
	virtual ConstantSP get(const string& key) const=0;
	virtual vector<pair<string,ConstantSP>> getAll() const=0;
	virtual void set(const string& key, const ConstantSP& value, bool copyIfDifferent = false)=0;
	virtual long long getLastActiveTime()=0;
	Output* getOutput() const { return out_.get();}
	void setOutput(const OutputSP& output) {out_ = output;}
	long long getSessionID() const {return sessionID_;}
	void setSessionID(long long sessionID){sessionID_=sessionID;}
	UserSP getUser() const { return user_;}
	bool setUser(const UserSP& user);
	HeapSP getHeap() const {return heap_;}
	bool isSystemSession() const {return flag_ & 1;}
	void setSystemSession(bool option) { if(option) flag_ |= 1; else flag_ &= ~1;}
	bool isWithinThreadCall() const { return flag_ & 2;}
	void setWithinThreadCall(bool option){ if(option) flag_ |= 2; else flag_ &= ~2;}
	virtual SysFunc getTableJoiner(const string& name) const = 0;
	virtual FunctionDefSP getFunctionDef(const string& name) const = 0;
	virtual TemplateOptr getTemplateOperator(const string& name) const = 0;
	virtual TemplateUserOptr getTemplateUserOperator(const string& name) const = 0;
	virtual OptrFunc getOperator(const string& optrSymbol, bool unary, const string& templateName) const = 0;
	virtual OperatorSP getOperator(const FunctionDefSP& func, bool unary) const = 0;
	virtual bool addPluginFunction(const FunctionDefSP& funcDef) = 0;
	virtual void addFunctionDeclaration(FunctionDefSP& func) = 0;
	virtual void completePendingFunctions(bool commit) = 0;
	virtual void getFunctions(vector<pair<string,FunctionDefSP> >& functions) = 0;
	virtual void undefine(const string& objectName, OBJECT_TYPE objectType) = 0;
	virtual bool isEnum(const string& word, int& value) const = 0;
	virtual bool isDebugMode() const  = 0;
	virtual void setDebugContext(const DebugContextSP& context) = 0;
	virtual DebugContext* getDebugContext() const  = 0;
	virtual string getCurrentModule() const = 0;
	virtual void pushTemporaryObject(const ConstantSP& obj) = 0;
	virtual ConstantSP getTemoraryObject(int index) = 0;
	virtual void clearTemporaryObject() = 0;

protected:
	char flag_;
	long long sessionID_;
	HeapSP heap_;
	OutputSP out_;
	UserSP user_;
};

class Console: public Runnable{
public:
	Console(const SessionSP& session):session_(session){
		out_=session->getOutput();
	}
	virtual ~Console(){}
	Output* getOutput(){return out_;}
	Session* getSession(){return session_.get();}
	virtual IO_ERR readReady()=0;
	virtual void execute()=0;

protected:
	SessionSP session_;
	Output* out_;
};

class ConstantMarshall {
public:
	virtual ~ConstantMarshall(){}
	virtual bool start(const ConstantSP& target, bool blocking, IO_ERR& ret)=0;
	virtual bool resume(IO_ERR& ret)=0;
	virtual void reset() = 0;
};

class ConstantUnmarshall{
public:
	virtual ~ConstantUnmarshall(){}
	virtual bool start(short flag, bool blocking, IO_ERR& ret)=0;
	virtual bool resume(IO_ERR& ret)=0;
	virtual void reset() = 0;
	ConstantSP getConstant(){return obj_;}

protected:
	ConstantSP obj_;
};

class DebugContext{
public:
	DebugContext();
	void waitForExecution(Heap* pHeap, Statement* pStatement);
	void waitForStop();
	void continueExecution(int steps);
	void decreaseSteps();
	int getSteps() const { return steps_;}
	bool continueFlag() const { return continueFlag_;}

private:
	int steps_;
	bool continueFlag_;
	bool stopped_;
	Mutex mutex_;
	ConditionalVariable execCondition_;
	ConditionalVariable stopCondition_;
	Heap* lastHeap_;
	Statement* lastStatement_;
};

class Heap{
public:
	Heap():session_(0), size_(0), status_(0){}
	Heap(Session* session):session_(session), size_(0), status_(0){}
	Heap(int size, Session* session):values_(size,ConstantSP()),flags_(size,0),session_(session), size_(size), status_(0){}

	ConstantSP getValue(int index) const;
	const ConstantSP& getReference(int index) const;
	inline int size() const {return size_;}
	inline bool isViewMode() const { return status_ & 1;}
	inline void setViewMode() { status_ |= 1; }
	int getIndex(const string& name) const;
	const string& getName(int index) const;
	bool contains(const string& name) const {
		return nameHash_.find(name)!=nameHash_.end();
	}
	int addItem(const string& name, const ConstantSP& value){ return addItem(name, value, false);}
	int addItem(const string& name, const ConstantSP& value, bool constant);
	void removeItem(const string& name);
	bool set(unsigned int index,const ConstantSP& value, bool constant);
	bool set(unsigned int index,const ConstantSP& value);
	void setConstant(int index, bool constant);
	inline bool isConstant(int index) const {return index>= MAX_SHARED_OBJ_INDEX ? flags_[index - MAX_SHARED_OBJ_INDEX] & 1 : true;}
	inline bool isInitialized(int index) const {return index>= MAX_SHARED_OBJ_INDEX ? flags_[index - MAX_SHARED_OBJ_INDEX] & 2 : true;}
	bool isSameObject(int index, Constant* obj) const;
	void rollback();
	void reset();
	void clearFlags();
	void currentSession(Session* session){session_=session;}
	Session* currentSession(){return session_;}

private:
	unordered_map<string,int> nameHash_;
	vector<string> names_;
	vector<ConstantSP> values_;
	vector<char> flags_;
	Session* session_;
	int size_;
	char status_;
};

class Statement{
public:
	Statement(STATEMENT_TYPE type):breakpoint_(false), type_(type){}
	virtual ~Statement(){}
	STATEMENT_TYPE getType() const {return type_;}
	virtual void execute(Heap* pHeap)=0;
	virtual void debugExecute(Heap* pHeap, DebugContext* pContext);
	virtual bool shouldReturn() const {return false;}
	virtual bool shouldBreak() const {return false;}
	virtual bool shouldContinue() const {return false;}
	virtual string getScript(int indention) const = 0;
	virtual IO_ERR serialize(const ByteArrayBufferSP& buffer) const = 0;
	virtual void collectUserDefinedFunctions(unordered_map<string,FunctionDef*>& functionDefs) const {}
	void disableBreakpoint();
	void enableBreakpoint();

protected:
	bool breakpoint_;

private:
	STATEMENT_TYPE type_;
};

class StatementFactory {
public:
	virtual ~StatementFactory(){}
	virtual Statement* readStatement(Session* session, const DataInputStreamSP& buffer) = 0;
	virtual Statement* createReturnStatement(const ObjectSP& obj) = 0;
};

class DomainSite{
public:
	DomainSite(const string& host, int port, int index);
	DomainSite(const string& host, int port, int index, const string& alias) ;
	const string& getHost() const {return host_;}
	int getPort() const {return port_;}
	int getIndex() const {return index_;}
	string getString() const;
	const string& getAlias() {return alias_;}
	inline bool operator ==(const DomainSite& target) const { return  host_ == target.host_ && port_ == target.port_;}
private:
	string host_;
	int port_;
	int index_;
	string alias_;
};

class DomainPartition{
public:
	DomainPartition(int key, const string& id, const string& desc) : key_(key), id_(id), desc_(desc){}
	virtual ~DomainPartition(){}
	const int getKey() const {return key_;}
	virtual bool addSite(int siteIndex) {return false;}
	virtual int getSiteCount() const { return 0;}
	virtual const DomainSite& getSite(int index) const { throw RuntimeException("DomainPartition::getSite method not supported.");}
	virtual bool isLocalPartition() const { return true;}
	string getString() const {return desc_;}
	string getId() const {return id_;}
	inline bool operator ==(const DomainPartition& target){ return  key_ == target.key_;}
	static string processPartitionId(const string& id);

private:
	int key_;
	string id_;
	string desc_;
};

class Domain{
public:
	Domain(PARTITION_TYPE partitionType, bool isLocalDomain) : partitionType_(partitionType), isLocalDomain_(isLocalDomain), key_(false){}
	Domain(PARTITION_TYPE partitionType, bool isLocalDomain, const Guid& key) : partitionType_(partitionType), isLocalDomain_(isLocalDomain), key_(key){}
	virtual ~Domain(){}
	int getPartitionCount() const { return partitions_.size();}
	DomainPartitionSP getPartition(int index) const { return partitions_[index];}
	PARTITION_TYPE getPartitionType() const { return partitionType_;}
	bool isLocalDomain() const {return isLocalDomain_;}
	const Guid& getKey() const { return key_;}
	virtual void retrievePartitionsInRange(const ConstantSP& start, bool startInclusive, const ConstantSP& end, bool endInclusive, vector<DomainPartitionSP>& partitions, bool localOnly) const = 0;
	virtual void retrievePartitionsIn(const ConstantSP& values, vector<DomainPartitionSP>& partitions, bool localOnly) const = 0;
	virtual void retrievePartitionAt(const ConstantSP& value, vector<DomainPartitionSP>& partitions, bool localOnly) const = 0;
	virtual void retrieveAllPartitions(vector<DomainPartitionSP>& partitions, bool localOnly) const;
	virtual IO_ERR saveDomain(const string& filename) const = 0;
	virtual ConstantSP getPartitionKey(const ConstantSP& partitionColumn) const = 0;
	virtual bool equals(const DomainSP& domain) const = 0;
	virtual DATA_TYPE getPartitionColumnType() const = 0;
	/*
	 * The input arguments set1 and set2 must be sorted by the key value of domain partitions. The ranking of key values must be the same as
	 * the ranking of partition column values when the partition is range-based or value-based.
	 */
	static void set_interaction(const vector<DomainPartitionSP>& set1, const vector<DomainPartitionSP>& set2, vector<DomainPartitionSP>& result);
	static void set_union(const vector<DomainPartitionSP>& set1, const vector<DomainPartitionSP>& set2, vector<DomainPartitionSP>& result);
	static DomainSP loadDomain(const string& domainFile);
	static DomainSP createDomain(PARTITION_TYPE  partitionType, const ConstantSP& scheme);
	static DomainSP createDomain(PARTITION_TYPE  partitionType, const ConstantSP& scheme, const ConstantSP& sites);

protected:
	void setKey(const Guid& key) { key_ = key;}
	static VectorSP parseSites(const ConstantSP& sites);
	static ConstantSP formatSites(const vector<DomainPartitionSP>& partitions);
	static bool addSiteToPartitions(const vector<DomainPartitionSP>& partitions, const VectorSP& sites);

protected:
	vector<DomainPartitionSP> partitions_;
	PARTITION_TYPE partitionType_;
	bool isLocalDomain_;
	Guid key_; //the unique identity for this domain
};

struct TableUpdate{
	TableUpdate(INDEX offset, INDEX length, Table* table) : offset_(offset), length_(length), table_(table){}
	TableUpdate() : offset_(0), length_(0), table_(0){}
	INDEX offset_;
	INDEX length_;
	Table* table_;
};

#endif /* CORECONCEPT_H_ */
