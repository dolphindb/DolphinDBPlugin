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
#include <unordered_set>
#include <set>
#include <vector>
#include <deque>
#include <algorithm>
#include <chrono>
#include <functional>
#include <atomic>

#include "Types.h"
#include "SmartPointer.h"
#include "Exceptions.h"
#include "Concurrent.h"
#include "LocklessContainer.h"
#include "FlatHashmap.h"
#include "SysIO.h"
#include "DolphinString.h"

#define serverVersion "2.00.16"

#if defined(__GNUC__) && __GNUC__ >= 4
#define LIKELY(x) (__builtin_expect((x), 1))
#define UNLIKELY(x) (__builtin_expect((x), 0))
#else
#define LIKELY(x) (x)
#define UNLIKELY(x) (x)
#endif
#define TRANSIENT(x) x->isTransient() ? x->getValue() : x

using std::string;
using std::vector;
using std::unordered_map;
using std::unordered_set;
using std::set;
using std::deque;
using std::pair;

class AuthenticatedUser;
class BinaryOperator;
class ByteArrayCodeBuffer;
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
class DFSChunkMeta;
class SQLTransaction;
class SQLContext;
class ColumnRef;
class SymbolBase;
class SymbolBaseManager;
class Output;
class Console;
class Session;
class ConstantMarshal;
class ConstantUnmarshal;
class DebugContext;
class DomainSite;
class DomainSitePool;
struct ClusterNodes;
class DomainPartition;
class Domain;
class PartitionGuard;
class Function;
struct TableUpdate;
struct TableUpdateSizer;
struct TableUpdateUrgency;
struct LocalTableUpdate;
struct TopicSubscribe;
class SessionThreadCallGuard;
class ReducerContainer;
class DistributedCall;
struct JobProperty;
class IoTransaction;
class Decoder;
class VolumeMapper;
class SystemHandle;
typedef SmartPointer<VolumeMapper> VolumeMapperSP;
typedef SmartPointer<Decoder> DecoderSP;
struct JITCfgNode;
struct InferredType;
struct FunctionSignature;
class WindowJoinFunction;
class ColumnContext;
class Transaction;
class Parser;

typedef SmartPointer<AuthenticatedUser> AuthenticatedUserSP;
typedef SmartPointer<ByteArrayCodeBuffer> ByteArrayCodeBufferSP;
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
typedef SmartPointer<DFSChunkMeta> DFSChunkMetaSP;
typedef SmartPointer<SQLTransaction> SQLTransactionSP;
typedef SmartPointer<SQLContext> SQLContextSP;
typedef SmartPointer<ColumnRef> ColumnRefSP;
typedef SmartPointer<SymbolBase> SymbolBaseSP;
typedef SmartPointer<SymbolBaseManager> SymbolBaseManagerSP;
typedef SmartPointer<Output> OutputSP;
typedef SmartPointer<Console> ConsoleSP;
typedef SmartPointer<Session> SessionSP;
typedef SmartPointer<ConstantMarshal> ConstantMarshalSP;
typedef SmartPointer<ConstantUnmarshal> ConstantUnmarshalSP;
typedef SmartPointer<DebugContext> DebugContextSP;
typedef SmartPointer<DomainSite> DomainSiteSP;
typedef SmartPointer<DomainSitePool> DomainSitePoolSP;
typedef SmartPointer<ClusterNodes> ClusterNodesSP;
typedef SmartPointer<DomainPartition> DomainPartitionSP;
typedef SmartPointer<Domain> DomainSP;
typedef SmartPointer<PartitionGuard> PartitionGuardSP;
typedef SmartPointer<TableUpdate> TableUpdateSP;
typedef SmartPointer<GenericBoundedQueue<TableUpdate, TableUpdateSizer, TableUpdateUrgency> > TableUpdateQueueSP;
typedef SmartPointer<TopicSubscribe> TopicSubscribeSP;
typedef SmartPointer<SessionThreadCallGuard> SessionThreadCallGuardSP;
typedef SmartPointer<ReducerContainer> ReducerContainerSP;
typedef SmartPointer<DistributedCall> DistributedCallSP;
typedef SmartPointer<JobProperty> JobPropertySP;
typedef SmartPointer<JITCfgNode> JITCfgNodeSP;
typedef SmartPointer<InferredType> InferredTypeSP;
typedef SmartPointer<FunctionSignature> FunctionSignatureSP;
typedef SmartPointer<WindowJoinFunction> WindowJoinFunctionSP;
typedef SmartPointer<ColumnContext> ColumnContextSP;
typedef SmartPointer<Transaction> TransactionSP;
typedef SmartPointer<Parser> ParserSP;

typedef ConstantSP(*OptrFunc)(const ConstantSP&, const ConstantSP&);
typedef ConstantSP(*OptrFunc2)(Heap* heap, const ConstantSP&, const ConstantSP&);
typedef ConstantSP(*SysFunc)(Heap* heap, vector<ConstantSP>& arguments);
typedef INDEX(*FastFunc)(vector<ConstantSP>& arguments, const ConstantSP& result, INDEX outputStart, bool validate, INDEX inputStart, INDEX inputLen);
typedef ConstantSP(*TemplateOptr)(const ConstantSP&,const ConstantSP&,const string&, OptrFunc, FastFunc, int);
typedef ConstantSP(*TemplateUserOptr)(Heap* heap, const ConstantSP&,const ConstantSP&, const FunctionDefSP&, int);
typedef void (*SysProc)(Heap* heap,vector<ConstantSP>& arguments);
typedef std::function<void (StatementSP)> CFGTraversalFunc;
typedef bool (*JitOptimizedFunc)(ConstantSP &ret, Heap* heap, std::vector<ConstantSP> &arguments);


class AuthenticatedUser{
public:
    AuthenticatedUser(const string& userId, long long time, int priority, int parallelism, bool isAdmin, bool isGuest, bool execScript, bool unitTest,
    		bool globalRead, const set<string>& readTables, const set<string>& deniedReadTables,
			bool globalInsert, const set<string>& insertTables,const set<string>& deniedInsertTables,
			bool globalUpdate, const set<string>& updateTables,const set<string>& deniedUpdateTables,
			bool globalDelete, const set<string>& deleteTables,const set<string>& deniedDeleteTables,
			bool viewRead, const set<string>& views, const set<string>& deniedViews,
			bool dbobjCreate, const set<string>& createDBs, const set<string>& deniedCreateDBs, bool dbobjDelete,
			const set<string>& deleteDBs, const set<string>& deniedDeleteDBs, bool dbOwner,
			const set<string>& dbOwnerPattern, bool dbManage, const set<string>& allowDbManage, const set<string>& deniedDbManage,
			long long queryResultMemLimit, long long taskGroupMemLimit, bool isViewOwner,
			bool globalExecComputeGroup, const set<string>& execGroup, const set<string>& deniedExecGroup,
			bool globalSensitiveView, const set<string>& sensitiveCol, const set<string>& deniedSensitiveCol,
			long long maxPartitionPerQuery);
    AuthenticatedUser(const ConstantSP& userObj);
    ConstantSP toTuple() const ;
    void setLoginNanoTimeStamp(long long t){loginNanoTimestamp_ = t;}
    string getUserId() const {return userId_;}
    long long getLoginNanoTimeStamp() const {return loginNanoTimestamp_;}
    int getMaxJobPriority() const { return priority_;}
    int getMaxJobParallelism() const {return parallelism_;}
    inline bool isAdmin() const {return permissionFlag_ & 1;}
	inline bool isGuest() const { return permissionFlag_ & 2;}
	inline bool canExecScript() const {return permissionFlag_ & 4;}
	inline bool canUnitTest() const {return permissionFlag_ & 8;}
	inline bool canManageDatabase(const string& name) const { return accessObjectRule((permissionFlag_ & 16), "DM_", "DDM_", name); }
	inline bool canReadTable() const { return permissionFlag_ & 32;}
	inline bool canWriteTable() const { return canInsertTable() && canUpdateTable() && canDeleteTable(); }
	inline bool canInsertTable() const { return permissionFlag_ & (1 << 11);}
	inline bool canUpdateTable() const { return permissionFlag_ & (1 << 12);}
	inline bool canDeleteTable() const { return permissionFlag_ & (1 << 13);}
	inline bool canUseView() const { return permissionFlag_ & 128;}
	inline bool canCreateDBObject() const { return permissionFlag_ & 256;}
	inline bool canDeleteDBObject() const { return permissionFlag_ & 512;}
	inline bool isDBOwner() const { return (permissionFlag_ & 1024) || !dbOwnerPatterns_.empty(); }
	bool isViewOwner() const { return permissionFlag_ & (1 << 14); }
	bool matchViewOwner(const string& owner) const { return isViewOwner() && getUserId() == owner; }
	bool matchDBOwner(const string& obj) const { return matchPattern(permissionFlag_ & 1024, dbOwnerPatterns_, obj); }
	bool canReadTable(const string& name) const { return accessTableRule(canReadTable(), "RT_", "DRT_", name); }
	bool canWriteTable(const string& name) const { return canInsertTable(name) && canUpdateTable(name) && canDeleteTable(name); }
	bool canInsertTable(const string& name) const { return accessTableRule(canInsertTable(), "IT_", "DIT_", name); }
	bool canUpdateTable(const string& name) const { return accessTableRule(canUpdateTable(), "UT_", "DUT_", name); }
	bool canDeleteTable(const string& name) const { return accessTableRule(canDeleteTable(), "DT_", "DDT_", name); }
	bool canReadView(const string& viewName) const { return accessViewRule(canUseView(), viewName); }
	bool canCreateDBObject(const string& name) const { return accessObjectRule(canCreateDBObject(), "CD_", "DCD_", name, ""); }
	bool canDeleteDBObject(const string& name) const { return accessObjectRule(canDeleteDBObject(), "DD_", "DDD_", name, ""); }
	bool canAccessSensitiveCol(const string& tableUrl, const string& colName) const;
	bool canExecGroup(const string& group) const;
	// return 0 if no limit
	long long queryResultMemLimit() { return queryResultMemLimit_; }
	long long taskGroupMemLimit() { return taskGroupMemLimit_; }
	long long maxPartitionPerQuery() { return maxPartitionPerQuery_; }
	bool isExpired() const {return expired_;}
	void expire();

    static AuthenticatedUserSP createAdminUser();
    static AuthenticatedUserSP createGuestUser();

private:

	bool accessObjectRule(bool global, const char* prefix, const char* denyPrefix, const string& objName, const char* objPrefix = "$DB$") const;
	bool accessTableRule(bool global, const char* prefix, const char* denyPrefix, const string& tableName, const char* objPrefix = "$DB$") const;
	bool accessViewRule(bool global, const string& viewName) const;
	bool matchPattern(bool global, const unordered_set<string>& patterns, const string& name) const;

    string userId_;
    long long loginNanoTimestamp_;
    int priority_;
    int parallelism_;

    /**
     * bit0: isAdmin
     * bit1: isGuest
     * bit2: execute script
     * bit3: unit test
     * bit4: create or delete databases
     * bit5: global read
     * bit6: global write(deprecated)
     * bit7: use view functions
     * bit8: create objects in databases
     * bit9: delete objects in databases
     * bit10: dbOwner
     * bit11: global insert
     * bit12: global update
     * bit13: global delete
	 * bit14: view owner
	 * bit15: sensitive view
     */
    uint32_t permissionFlag_;

    std::atomic<bool> expired_;
    unordered_set<string> permissions_;
    unordered_set<string> dbOwnerPatterns_;
    long long queryResultMemLimit_;
    long long taskGroupMemLimit_;
	long long maxPartitionPerQuery_;
};

template<class T>
class Array{
public:
	Array(int capacity) : data_(new T[capacity]), size_(0), capacity_(capacity){}
	Array(T* data, int size, int capacity): data_(data), size_(size), capacity_(capacity){}
	Array(const Array<T>& copy) : size_(copy.size_), capacity_(copy.capacity_){
		data_ = new T[size_];
		memcpy(data_, copy.data_, sizeof(T) * size_);
	}
	~Array(){delete[] data_;}
	Array<T>* copy(){
		T* data = new T[size_];
		memcpy(data, data_, sizeof(T) * size_);
		return new Array(data, size_, capacity_);
	}
	Array<T>& operator =(const Array<T>& sp){
		size_ = sp.size_;
		if(capacity_ >= sp.capacity_){
			memcpy(data_, sp.data_, sizeof(T) * size_);
		}
		else{
			delete[] data_;
			capacity_ = sp.capacity_;
			data_ = new T[capacity_];
			memcpy(data_, sp.data_, sizeof(T) * size_);
		}
		return *this;
	}
	const T& at(int index) const { return data_[index];}
	const T& operator [](int index) const {return data_[index];}
	T& operator [](int index) {return data_[index];}
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
	const T& operator [](int index) const { return dataSegment_[index >> segmentSizeInBit_][index & segmentMask_];}
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
	void get(vector<T>& copy, int size){
		int start =0;
		int s = 0;
		while(start < size){
			int count = std::min(segmentSize_, size - start);
			T* data = dataSegment_[s++];
			for(int i=0; i<count; ++i)
				copy.push_back(data[i]);
			start += count;
		}
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

class ByteArrayCodeBuffer{
public:
	ByteArrayCodeBuffer(size_t capacity) : buf_(new char[capacity]), capacity_(capacity), size_(0), constantMap_(0), constants_(0){}
	ByteArrayCodeBuffer() : buf_(new char[2048]), capacity_(2048), size_(0), constantMap_(0), constants_(0){}
	~ByteArrayCodeBuffer();
	IO_ERR write(const char* buffer, int length, int& actualLength);
	IO_ERR write(const char* buffer, int length);
	IO_ERR write(const ConstantSP& obj);
	inline IO_ERR write(const string& buffer){ return write(buffer.c_str(), buffer.length() + 1);}
	inline IO_ERR write(const DolphinString& buffer){ return write(buffer.c_str(), buffer.length() + 1);}
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
	unordered_map<long long, int>* constantMap_;
	vector<ConstantSP>* constants_;
};

class SymbolBase{
public:
	SymbolBase(bool supportOrder = false);
	SymbolBase(const vector<DolphinString>& symbols, bool supportOrder = false);
	SymbolBase(const string& symbolFile, bool snapshot = false, bool supportOrder=false, bool readOnly = false);
	SymbolBase(const string& symbolFile, const DataInputStreamSP& in, bool snapshot = false, bool readOnly = false);
	SymbolBase* copy();
	bool saveSymbolBase(string& errMsg, bool sync = false);
	IO_ERR serialize(int offset, int length, Buffer& buf);
	inline bool lastSaveSynchronized() const { return lastSaveSynchronized_;}
	inline int find(const DolphinString& symbol){
		int index = -1;
		keyMap_.find(symbol, index);
		return index;
	}
	void find(const char** symbols, int size, int* indices);
	int findAndInsert(const DolphinString& symbol);
	/**
	 * This function will acquire the lock and compare the size with the input argument sizeBeforeInsert.
	 * returns 0 if the size doesn't equal to sizeBeforeInsert.
	 * returns -1 if some of the input symbols are already existed.
	 * returns new symbol size after if everything is OK.
	 */
	int atomicInsert(vector<DolphinString>& symbols, int sizeBeforeInsert);
	void getOrdinalCandidate(const DolphinString& symbol, int& ordinal, SmartPointer<Array<int> >& ordinalBase);
	void getOrdinalCandidate(const DolphinString& symbol1, const DolphinString& symbol2, int& ordinal1, int& ordinal2, SmartPointer<Array<int> >& ordinalBase);
	int* getSortedIndices(bool asc, char nullsOrder, int& length);
	inline bool supportOrder() const { return supportOrder_;}
	/**
	 * enableOrdinalBase and disableOrdinalBase can't be used in multi-threading environment.
	 */
	void enableOrdinalBase();
	void disableOrdinalBase();
	SmartPointer<Array<int> > getOrdinalBase();
	inline const DolphinString& getSymbol(int index) const { return key_[index];}
	void getSymbols(int offset, int length, vector<DolphinString>& symbols) const;
	int size() const {return  key_.size();}
	/**
	 * checkpoint is used for write transactions. When rolling back a transaction,
	 * one can roll back the in-memory symbol base to the checkpoint.
	 */
	void setCheckpoint(int n);
	inline int getCheckpoint() const { return checkpoint_;}
	const Guid& getID(){return baseID_;}
	bool isModified(){return modified_;}
	int getVersion(){return version_;}
    const string& getDBPath() const {return dbPath_;}
    void setDBPath(const string& dbPath) { dbPath_ = dbPath;}
    string getString() const;
    string getString(int start, int length) const;
	static int readSymbolBaseVersion(const string symbolFile);

private:
	SymbolBase(int size, const DynamicArray<DolphinString>& keys, bool supportOrder = false);
	int getOrdinalCandidate(const DolphinString& symbol);
	void reAssignOrdinal(int capacity);
	bool reload(const string& symbolFile, const DataInputStreamSP& in, bool snapshot = false, bool needTruncate = false);

private:
	Guid baseID_;
	int interval_;
	int version_;
	string dbPath_;
	int savingPoint_;
	int checkpoint_;
	bool modified_;
	bool lastSaveSynchronized_;
	bool supportOrder_;
	DynamicArray<DolphinString> key_;
	SmartPointer<Array<int> > ordinal_;
	IrremovableLocklessFlatHashmap<DolphinString, int> keyMap_;
	deque<int> sortedIndices_;
	mutable RWLock writeMutex_;
	mutable Mutex versionMutex_;
    bool readOnly_ = false;
};

class SymbolBaseManager{
public:
	SymbolBaseManager(const string& symbolBaseDir);
	SymbolBaseSP findAndLoad(int symBaseID);
	SymbolBaseSP findOrInsert(const string& key);
	SymbolBaseSP find(const string& key);
	void reloadSymbolBase(const string& key);
	string getSymbolFile(const string& key);

private:
	int checkSymbolBaseVersion(const string& key);

private:
	string dir_;
	int devId_;
	unordered_map<string, SymbolBaseSP> symbases_;
	Mutex mutex_;
};

struct ControlFlowEdge {
	void * edgeFrom;
	void * edgeTo;
	string varName;
	ControlFlowEdge(void * from, void * to, const string & name): edgeFrom(from), edgeTo(to), varName(name) {}
	bool operator==(const ControlFlowEdge & rhs) const {
		return edgeFrom == rhs.edgeFrom && edgeTo == rhs.edgeTo && varName == rhs.varName;
	}
};
namespace std {
	template<>
	struct hash<ControlFlowEdge> {
		std::size_t operator()(const  ControlFlowEdge & k) const {
			using std::size_t;
			using std::hash;
			using std::string;
			return hash<void*>{}(k.edgeFrom)
				   ^ (hash<void*>{}(k.edgeTo) >> 1)
				   ^ (hash<string>{}(k.varName) << 1);
		}
	};
}

struct FunctionSignature {
	InferredTypeSP returnTy;
	vector<InferredTypeSP> paramTys;
	vector<int> missingParams;
	FunctionDef* func;
	void* jitFunc;
	Function* funcObj;

	FunctionSignature() : func(0), jitFunc(0), funcObj(0){}
};

struct InferredType {
    DATA_FORM form;
    DATA_TYPE type;
    DATA_CATEGORY category;
    FunctionSignatureSP signature;

    InferredType(DATA_FORM form = DF_SCALAR, DATA_TYPE type = DT_VOID, DATA_CATEGORY category = NOTHING):
        form(form), type(type), category(category){}
    bool operator ==(const InferredType & rhs) const {
        return form == rhs.form && type == rhs.type && category == rhs.category;
    }
    bool operator !=(const InferredType & rhs) const {
        return !operator==(rhs);
    }
    string getString();
};

namespace std {
	template<>
	struct hash<InferredType> {
		std::size_t operator()(const InferredType & k) const {
			using std::size_t;
			using std::hash;
			using std::string;
			return hash<int8_t>{}(k.form) ^ hash<int8_t>{}(k.type);
		}
	};
}

class Object {
public:
	Object(OBJECT_TYPE type) : objType_(type){}
	inline OBJECT_TYPE getObjectType() const { return objType_;}
	bool isConstant() const {return objType_ == CONSTOBJ;}
	bool isVariable() const {return objType_ ==VAR;}
	virtual ConstantSP getValue(Heap* pHeap) = 0;
	virtual ConstantSP getReference(Heap* pHeap) = 0;
    virtual void getReference(Heap* pHeap, Constant*& ptr, ConstantSP& ref) {ptr = nullptr; ref = getReference(pHeap);}
	virtual ~Object(){}
	virtual string getScript() const = 0;
	virtual IO_ERR serialize(Heap* pHeap, const ByteArrayCodeBufferSP& buffer) const = 0;
	virtual bool isLargeConstant() const {return false;}
	virtual ObjectSP deepCopy() const { throw RuntimeException("Object::deepCopy not implemented yet.");}
	/**
	 * @brief Get the components of the object in the form of a dictionary.
	 *
	 * @return If the object doesn't support this method, return an empty dictionary.
	 */
	virtual ConstantSP getComponent() const;

	virtual void collectUserDefinedFunctions(unordered_map<string,FunctionDef*>& functionDefs) const {}
	void collectVariables(vector<int>& vars, int minIndex, int maxIndex) const;

	/**
	 * @biref Retrieve all ColumnRef objects contained in the currrent object.
	 *
	 * @param table: Use the given table to judge if a column reference is a variable in the case the name is ambiguous.
	 * @param columns: in & out parameter, pair<string, string> represents a column's quanlifier and name.
	 */
	void retrieveColumns(const TableSP& table, vector<pair<string,string>>& columns) const;
	/**
	 * @brief Whether the object or its sub components contains column reference.
	 */
	bool containColumnRef() const;
	/**
	 * @brief Whether the object or its sub components contains analytic function.
	 */
	bool containAnalyticFunction() const;
	/**
	 * @brief Check if the current object contains special functions.
	 *
	 * @param aggrOnly: if only check aggregate functions.
	 *
	 * @return an indicator -1, 0, 1 or 2
	 * -1: the object doesn't contain any column or variable
	 *  0: transform function only
	 *  1: aggregate function
	 *  2: order sensitive function)
	 *  3: other type function
	 */
	int checkSpecialFunction(bool aggrOnly, bool sqlMode=true) const;
	/**
	 * @brief Replace the SQLContext of any ColumnRef within the current object with the given new context.
	 *
	 * @param context: context can't be null.
	 * @param localize: replace the variable with the concrete object and detach the link of any column
	 * reference to a variable if this parameter is set to true.
	 *
	 * @return If the returned pointer is null, the object doesn't contain any column reference.
	 */
	virtual ObjectSP copy(Heap* pHeap, const SQLContextSP& context, bool localize) const { return nullptr;}

	/**
	 * @brief Replace the SQLContext of any ColumnRef within the current object with the given new context, and materialize
	 * all variables.
	 *
	 * @param context: if context is null, materialize the object only, otherwise copy and materialize the object.
	 * @param table: Use the given table to judge if a column reference is a variable in the case the name is ambiguous.
	 *
	 * @return If the returned pointer is null, the object doesn't contain any column reference or variable.
	 */
	virtual ObjectSP copyAndMaterialize(Heap* pHeap, const SQLContextSP& context, const TableSP& table) const { return nullptr;}

	/**
	 * @brief Whether the object or its sub components contains column reference or variable.
	 */
	virtual bool mayContainColumnRefOrVariable() const { return false;}

	/**
	 * @brief collect objects from the current object and its sub component.
	 */
	virtual void collectObjects(vector<const Object*>& vec) const {};

	/**
	 * @brief judge if the given object should be collected by collectObjects function.
	 *
	 * Now we only collect non-constant object and dynamic function
	 *
	 */
	static bool isCollectibleObject(const Object* obj);

	/**
	 * @brief Traverse all sub components of the given objects.
	 * @param objStack: objects to traverse. The container is also used as the buffer to collect sub
	 * 		components.
	 * @param func: func is an function with the signature bool func(Object*). The return value could
	 * 		be false (collecting sub components) or true (not collecting sub components).
	 * @param offset: Objects at or after the offset will be traversed.
	 *
	 */
	template<class T>
	static void traverse(vector<const Object*>& objStack, T& func, int offset = 0){
		while (static_cast<int>(objStack.size()) > offset) {
			const Object* cur = objStack.back();
			objStack.pop_back();
			if(!func(cur))
				cur->collectObjects(objStack);
		}
	}

	/**
	 * @brief Traverse all sub components of the given objects with short circuit evaluation.
	 * @param objStack: objects to traverse. The container is also used as the buffer to collect sub
	 * 		components.
	 * @param func: func is an function with the signature int func(Object*). The return value could
	 * 		be 0 (collecting sub components), 1 (not collecting sub components), or 2 (finish traversal).
	 * @param offset: Objects at or after the offset will be traversed.
	 *
	 */
	template<class T>
	static void traverseShortCircuit(vector<const Object*>& objStack, T& func, int offset = 0) {
		while (static_cast<int>(objStack.size()) > offset) {
			const Object* cur = objStack.back();
			objStack.pop_back();
			int ret = func(cur);
			if(ret == 2)
				return;
			else if(ret == 0)
				cur->collectObjects(objStack);
		}
	}

protected:
	OBJECT_TYPE objType_;
};

#define NOT_IMPLEMENT \
	throw RuntimeException("Data type [" + std::to_string(static_cast<int>(getType())) + "] form [" + \
			std::to_string(static_cast<int>(getForm())) + "] does not implement `" + __func__ + "`"); \
//======

class Constant: public Object{
public:
	static DolphinString DEMPTY;
	static string EMPTY;
	static string NULL_STR;

	Constant() : Object(CONSTOBJ), flag_(3){}
	Constant(unsigned short flag) : Object(CONSTOBJ), flag_(flag){}
	Constant(DATA_FORM df, DATA_TYPE dt, DATA_CATEGORY dc) : Object(CONSTOBJ), flag_(3 + (df<<8) + (dt<<16) + (dc<<24)){}
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
	inline bool transferAsString() const {return flag_ & 64;}
	inline void transferAsString(bool val){ if(val) flag_ |= 64; else flag_ &= ~64;}
	inline bool isSynchronized() const {return flag_ & 128;}
	inline void setSynchronized(bool val){ if(val) flag_ |= 128; else flag_ &= ~128;}
	inline bool isOOInstance() const {return flag_ & 4096;}
	inline void setOOInstance(bool val){ if(val) flag_ |= 4096; else flag_ &= ~4096;}
	inline bool isIndexed() const {return flag_ & 8192;}
	inline void setIndexed(bool val){ if(val) flag_ |= 8192; else flag_ &= ~8192;}
	inline bool isSeries() const {return flag_ & 16384;}
	inline void setSeries(bool val){ if(val) flag_ |= 16384; else flag_ &= ~16384;}
	inline bool isTransient() const {return flag_ & 32768;}
	inline void setTransient(bool val){ if(val) flag_ |= 32768; else flag_ &= ~32768;}
	inline DATA_FORM getForm() const {return DATA_FORM((flag_ >> 8) & 15);}
	inline void setForm(DATA_FORM df){ flag_ = (flag_ & 4294963455U) + (df << 8);}
	inline DATA_TYPE getType() const {return DATA_TYPE((flag_ >> 16) & 255);}
	inline DATA_CATEGORY getCategory() const {return DATA_CATEGORY((flag_ >> 24) & 15);}
	/**
	 * @brief set the COW(copy on write) flag (bit 28)
	 */
	inline void setCOW(bool val){ if(val) flag_ |= 268435456; else flag_ &= ~268435456;}
	/**
	 * @brief Return whether the flag of COW is on.
	 */
	inline bool isCOW() const { return flag_ & 268435456;}
	/**
	 * @brief Return whether this constant is scalar.
	 */
	inline bool isScalar() const { return getForm()==DF_SCALAR;}
	inline bool isArray() const { return getForm()==DF_VECTOR;}
	inline bool isPair() const { return getForm()==DF_PAIR;}
	inline bool isMatrix() const {return getForm()==DF_MATRIX;}
	//a vector could be array, pair or matrix.
	inline bool isVector() const { DATA_FORM df = getForm();return df>=DF_VECTOR && df<=DF_MATRIX;}
	inline bool isTable() const { return getForm()==DF_TABLE;}
	inline bool isSet() const {return getForm()==DF_SET;}
	inline bool isDictionary() const {return getForm()==DF_DICTIONARY;}
	inline bool isChunk() const {return getForm()==DF_CHUNK;}
	bool isTuple() const {return getForm()==DF_VECTOR && getType()==DT_ANY;}
	bool isNumber() const { DATA_CATEGORY cat = getCategory(); return cat == INTEGRAL || cat == FLOATING || cat == DENARY; }

	virtual bool isDatabase() const {return false;}
	virtual ObjectSP deepCopy() const { return getValue();}

	virtual char getBool() const {throw RuntimeException("The object can't be converted to boolean scalar.");}
	virtual char getChar() const {throw RuntimeException("The object can't be converted to char scalar.");}
	virtual short getShort() const {throw RuntimeException("The object can't be converted to short scalar.");}
	virtual int getInt() const {throw RuntimeException("The object can't be converted to int scalar.");}
	virtual long long  getLong() const {throw RuntimeException("The object can't be converted to long scalar.");}
	virtual INDEX getIndex() const {throw RuntimeException("The object can't be converted to index scalar.");}
	virtual float getFloat() const {throw RuntimeException("The object can't be converted to float scalar.");}
	virtual double getDouble() const {throw RuntimeException("The object can't be converted to double scalar.");}
	/**
	 * @brief Get the string version of this constant.
	 * 		  This function is very helpful for debugging.
	 */
	virtual string getString() const {return "";}
	virtual string getScript() const { return getString();}
	virtual const DolphinString& getStringRef() const {return DEMPTY;}
    virtual const Guid getInt128() const {throw RuntimeException("The object can't be converted to int128 scalar.");}
    virtual const unsigned char* getBinary() const {throw RuntimeException("The object can't be converted to int128 scalar.");}
	virtual bool isNull() const {return false;}

	virtual int getDecimal32(int scale) const { NOT_IMPLEMENT; }
	virtual long long getDecimal64(int scale) const { NOT_IMPLEMENT; }
	virtual int128 getDecimal128(int scale) const { NOT_IMPLEMENT; }

	virtual void setBool(char val){}
	virtual void setChar(char val){}
	virtual void setShort(short val){}
	virtual void setInt(int val){}
	virtual void setLong(long long val){}
	virtual void setIndex(INDEX val){}
	virtual void setFloat(float val){}
	virtual void setDouble(double val){}
	virtual void setString(const DolphinString& val){}
	virtual void setBinary(const unsigned char* val, int unitLength){}
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
	virtual const DolphinString& getStringRef(INDEX index) const {return DEMPTY;}
    virtual const Guid getInt128(INDEX index) const {return getInt128();}
    virtual const unsigned char* getBinary(INDEX index) const {return getBinary();}
	virtual bool isNull(INDEX index) const {return isNull();}

	virtual int getDecimal32(INDEX index, int scale) const { NOT_IMPLEMENT; }
	virtual long long getDecimal64(INDEX index, int scale) const { NOT_IMPLEMENT; }
	virtual int128 getDecimal128(INDEX index, int scale) const { NOT_IMPLEMENT; }

	virtual ConstantSP get(INDEX index) const {return getValue();}
	virtual ConstantSP get(INDEX column, INDEX row) const {return get(row);}
	/**
	 * @brief Get the data according to the index.
	 *
	 * @param index: the index gives the indices of data to get. If the index is out of range,
	 * 		i.e. negative or larger than the size, the null value is returned correspondingly.
	 * @return ConstantSP: the data
	 */
	virtual ConstantSP get(const ConstantSP& index) const {return getValue();}
	/**
	 * @brief Get the data according to the index.
	 *
	 * @param offset: the index is offseted by offset
	 * @param index: the index gives the indices of data to get
	 * @return ConstantSP: the data
	 */
	virtual ConstantSP get(INDEX offset, const ConstantSP& index) const {return getValue();}
	virtual ConstantSP getColumn(INDEX index) const {return getValue();}
	virtual ConstantSP getRow(INDEX index) const {return get(index);}
	virtual ConstantSP getItem(INDEX index) const {return get(index);}
	virtual ConstantSP getItems(const ConstantSP& index) const {return get(index);}
	virtual ConstantSP getWindow(INDEX colStart, int colLength, INDEX rowStart, int rowLength) const {return getValue();}
	virtual ConstantSP getSlice(const ConstantSP& rowIndex, const ConstantSP& colIndex) const {throw RuntimeException("getSlice method not supported");}
	virtual ConstantSP getRowLabel() const;
	virtual ConstantSP getColumnLabel() const;

	/**
	 * @brief Judge the data from start to (start + len - 1) is null or not
	 *
	 * @param start: the start index
	 * @param len: the length of data to be judged
	 * @param buf: the result is stored in buf
	 * @return true: the function call succeed
	 * @return false: the function call fail
	 */
	virtual bool isNull(INDEX start, int len, char* buf) const {return false;}
	/**
	 * @brief Judge the data from start to (start + len - 1) is valid or not.
	 * 		  All the param and return is the same as in isNull.
	 */
	virtual bool isValid(INDEX start, int len, char* buf) const {return false;}
	/**
	 * @brief Get the boolean data from start to (start + len - 1).
	 * 		  All the param and return is the same as in isNull.
	 */
	virtual bool getBool(INDEX start, int len, char* buf) const {return false;}
	virtual bool getChar(INDEX start, int len,char* buf) const {return false;}
	virtual bool getShort(INDEX start, int len, short* buf) const {return false;}
	virtual bool getInt(INDEX start, int len, int* buf) const {return false;}
	virtual bool getLong(INDEX start, int len, long long* buf) const {return false;}
	virtual bool getIndex(INDEX start, int len, INDEX* buf) const {return false;}
	virtual bool getFloat(INDEX start, int len, float* buf) const {return false;}
	virtual bool getDouble(INDEX start, int len, double* buf) const {return false;}
	virtual bool getSymbol(INDEX start, int len, int* buf, SymbolBase* symBase,bool insertIfNotThere) const {return false;}
	virtual bool getString(INDEX start, int len, DolphinString** buf) const {return false;}
	virtual bool getString(INDEX start, int len, char** buf) const {return false;}
	virtual bool getBinary(INDEX start, int len, int unitLength, unsigned char* buf) const {return false;}

	virtual bool getDecimal32(INDEX start, int len, int scale, int *buf) const { NOT_IMPLEMENT; }
	virtual bool getDecimal64(INDEX start, int len, int scale, long long *buf) const { NOT_IMPLEMENT; }
	virtual bool getDecimal128(INDEX start, int len, int scale, int128 *buf) const { NOT_IMPLEMENT; }

	/**
	 * @brief Judge the data according to indices is null or not
	 *
	 * @param indices: the maybe out-of-ordered indices to judge
	 * @param len: the length of data to be judged
	 * @param buf: the result is stored in buf
	 * @return true: the function call succeed
	 * @return false: the function call fail
	 */
	virtual bool isNull(INDEX* indices, int len, char* buf) const {return false;}
	virtual bool isValid(INDEX* indices, int len, char* buf) const {return false;}
	/**
	 * @brief Get the boolean data according to indices.
	 * 		  All the param and return is the same as in isNull.
	 */
	virtual bool getBool(INDEX* indices, int len, char* buf) const {return false;}
	virtual bool getChar(INDEX* indices, int len,char* buf) const {return false;}
	virtual bool getShort(INDEX* indices, int len, short* buf) const {return false;}
	virtual bool getInt(INDEX* indices, int len, int* buf) const {return false;}
	virtual bool getLong(INDEX* indices, int len, long long* buf) const {return false;}
	virtual bool getIndex(INDEX* indices, int len, INDEX* buf) const {return false;}
	virtual bool getFloat(INDEX* indices, int len, float* buf) const {return false;}
	virtual bool getDouble(INDEX* indices, int len, double* buf) const {return false;}
	virtual bool getSymbol(INDEX* indices, int len, int* buf, SymbolBase* symBase,bool insertIfNotThere) const {return false;}
	virtual bool getString(INDEX* indices, int len, DolphinString** buf) const {return false;}
	virtual bool getString(INDEX* indices, int len, char** buf) const {return false;}
	virtual bool getBinary(INDEX* indices, int len, int unitLength, unsigned char* buf) const {return false;}

	virtual bool getDecimal32(INDEX *indices, int len, int scale, int *buf) const { NOT_IMPLEMENT; }
	virtual bool getDecimal64(INDEX *indices, int len, int scale, long long *buf) const { NOT_IMPLEMENT; }
	virtual bool getDecimal128(INDEX *indices, int len, int scale, int128 *buf) const { NOT_IMPLEMENT; }

	/**
	 * @brief Get the boolean data from start to (start + len - 1).
	 * 		  This is the recommended method to view/iterate data in Constant.
	 * 		  Note that if the required underlying data is contiguous, then there is
	 *          no copy happened in this function and the underlying buffer is directly returned;
	 * 			otherwise, the data is copied into buf, and buf is returned.
	 *
	 * @param start: the start index
	 * @param len: the length of data to get
	 * @param buf: a buffer with at least length len
	 * @return a buffer with the data required.
	 */
	virtual const char* getBoolConst(INDEX start, int len, char* buf) const {throw RuntimeException("getBoolConst method not supported");}
	virtual const char* getCharConst(INDEX start, int len,char* buf) const {throw RuntimeException("getCharConst method not supported");}
	virtual const short* getShortConst(INDEX start, int len, short* buf) const {throw RuntimeException("getShortConst method not supported");}
	virtual const int* getIntConst(INDEX start, int len, int* buf) const {throw RuntimeException("getIntConst method not supported");}
	virtual const long long* getLongConst(INDEX start, int len, long long* buf) const {throw RuntimeException("getLongConst method not supported");}
	virtual const INDEX* getIndexConst(INDEX start, int len, INDEX* buf) const {throw RuntimeException("getIndexConst method not supported");}
	virtual const float* getFloatConst(INDEX start, int len, float* buf) const {throw RuntimeException("getFloatConst method not supported");}
	virtual const double* getDoubleConst(INDEX start, int len, double* buf) const {throw RuntimeException("getDoubleConst method not supported");}
	virtual const int* getSymbolConst(INDEX start, int len, int* buf, SymbolBase* symBase, bool insertIfNotThere) const {throw RuntimeException("getSymbolConst method not supported");}
	virtual DolphinString** getStringConst(INDEX start, int len, DolphinString** buf) const {throw RuntimeException("getStringConst method not supported");}
	virtual char** getStringConst(INDEX start, int len, char** buf) const {throw RuntimeException("getStringConst method not supported");}
	virtual const unsigned char* getBinaryConst(INDEX start, int len, int unitLength, unsigned char* buf) const {throw RuntimeException("getBinaryConst method not supported");}

	virtual const int* getDecimal32Const(INDEX start, int len, int scale, int *buf) const { NOT_IMPLEMENT; }
	virtual const long long* getDecimal64Const(INDEX start, int len, int scale, long long *buf) const { NOT_IMPLEMENT; }
	virtual const int128* getDecimal128Const(INDEX start, int len, int scale,
			int128 *buf) const {
		NOT_IMPLEMENT;
	}

	/**
	 * @brief Get a buffer for writing data from start to (start + len - 1).
	 * 		  This is the recommended method to write data in Constant.
	 * 		  This function is always used together with setBool(start, len, buf).
	 * 		  Note that if the required underlying data is contiguous,
	 * 			then the underlying buffer is directly returned;
	 * 			otherwise, the buf is returned.
	 *
	 * @param start: the start index
	 * @param len: the length of data to get
	 * @param buf: a buffer with at least length len
	 * @return a buffer to write data.
	 */
	virtual char* getBoolBuffer(INDEX start, int len, char* buf) const {return buf;}
	virtual char* getCharBuffer(INDEX start, int len,char* buf) const {return buf;}
	virtual short* getShortBuffer(INDEX start, int len, short* buf) const {return buf;}
	virtual int* getIntBuffer(INDEX start, int len, int* buf) const {return NULL;}
	virtual long long* getLongBuffer(INDEX start, int len, long long* buf) const {return buf;}
	virtual INDEX* getIndexBuffer(INDEX start, int len, INDEX* buf) const {return buf;}
	virtual float* getFloatBuffer(INDEX start, int len, float* buf) const {return buf;}
	virtual double* getDoubleBuffer(INDEX start, int len, double* buf) const {return buf;}
	virtual unsigned char* getBinaryBuffer(INDEX start, int len, int unitLength, unsigned char* buf) const {return buf;}
	virtual void* getDataBuffer(INDEX start, int len, void* buf) const {return buf;}

	virtual int* getDecimal32Buffer(INDEX start, int len, int scale, int *buf) const { NOT_IMPLEMENT; }
	virtual long long* getDecimal64Buffer(INDEX start, int len, int scale, long long *buf) const { NOT_IMPLEMENT; }
	virtual int128* getDecimal128Buffer(INDEX start, int len, int scale,
			int128 *buf) const {
		NOT_IMPLEMENT;
	}

	virtual IO_ERR serialize(Heap* pHeap, const ByteArrayCodeBufferSP& buffer) const {return serialize(buffer);}
	virtual IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const {throw RuntimeException("code serialize method not supported");}
    virtual int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int& numElement, int& partial) const {throw RuntimeException("serialize method not supported");}
    virtual int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int targetNumElement, int& numElement, int& partial) const;
    virtual IO_ERR deserialize(DataInputStream* in, INDEX indexStart, int offset, INDEX targetNumElement, INDEX& numElement, int& partial) {throw RuntimeException("deserialize method not supported");}
	/**
	 * @brief Fill the null value with val.
	 *
	 * @param val
	 */
	virtual void nullFill(const ConstantSP& val){}
	/**
	 * @brief Set the index-th element to be val.
	 *
	 * @param index: the index of element to be set.
	 * @param val: the value to be set.
	 */
	virtual void setBool(INDEX index,char val){setBool(val);}
	virtual void setChar(INDEX index,char val){setChar(val);}
	virtual void setShort(INDEX index,short val){setShort(val);}
	virtual void setInt(INDEX index,int val){setInt(val);}
	virtual void setLong(INDEX index,long long val){setLong(val);}
	virtual void setIndex(INDEX index,INDEX val){setIndex(val);}
	virtual void setFloat(INDEX index,float val){setFloat(val);}
	virtual void setDouble(INDEX index, double val){setDouble(val);}
	virtual void setString(INDEX index, const DolphinString& val){setString(val);}
	virtual void setBinary(INDEX index, int unitLength, const unsigned char* val){setBinary(val, unitLength);}
	virtual void setNull(INDEX index){setNull();}

	virtual void setDecimal32(INDEX index, int scale, int val) { NOT_IMPLEMENT; }
	virtual void setDecimal64(INDEX index, int scale, long long val) { NOT_IMPLEMENT; }
	virtual void setDecimal128(INDEX index, int scale, int128 val) { NOT_IMPLEMENT; }

	/**
	 * @brief Replace the cell value specified by the index with the new value specified by valueIndex.
	 *
	 * @param index: Make sure index is valid, i.e. no less than zero and less than the size of the object.
	 * @param value: the value to be set.
	 * @param valueIndex: Make sure valueIndex is valid, i.e. no less than zero and less than the size of the value.
	 * @return true if set succeed, false else.
	 */
	virtual bool set(INDEX index, const ConstantSP& value, INDEX valueIndex){return set(index, value->get(valueIndex));}
	virtual bool set(INDEX index, const ConstantSP& value){return false;}
	virtual bool set(INDEX column, INDEX row, const ConstantSP& value){return false;}
	/**
	 * @brief Replace the cell value specified by the index with value.
	 *
	 * @param index: scalar or vector. Make sure all indices in are valid,
	 * 		i.e. no less than zero and less than the size of the value.
	 * @param value: the value to be set.
	 * @return true if set succeed, false else.
	 */
	virtual bool set(const ConstantSP& index, const ConstantSP& value) {return false;}
	/**
	 * @brief Replace the cell value specified by the index with the new value specified by valueIndex.
	 *
	 * @param index: scalar or vector. Make sure all indices in are valid,
	 * 		i.e. no less than zero and less than the size of the value.
	 * @param value: vector.
	 * @param valueIndex: index vector to filter value. Make sure all indices in the vector are valid,
	 * 		i.e. no less than zero and less than the size of the value.
	 * @return true if set succeed, false else.
	 */
	virtual bool set(const ConstantSP& index, const ConstantSP& value, const ConstantSP& valueIndex) {return false;}
	/**
	 * @brief Replace the cell value specified by the index with the new value. Usually the current object
	 * is a tuple or an any dictionary.
	 *
	 * @param heap: the heap of the execution context.
	 * @param index: index must be a tuple.
	 * @param value: could be any ConstantSP object.
	 * @param dim: dim is a zero-based index. The index's dim-th element is the index of the current object to update.
	 * @return true if set succeed, false else.
	 */
	virtual bool set(Heap* heap, const ConstantSP& index, const ConstantSP& value, int dim) {return false;}
	/**
	 * @brief Replace the cell value specified by the index with value.
	 * 		  Note that setNonNull will ignore null value when replacing data.
	 *
	 * @param index: Scalar or vector. Make sure all indices in are valid,
	 * 		i.e. no less than zero and less than the size of the value.
	 * @param value: The value to be set.
	 * @return True if set succeed, false else.
	 */
	virtual bool setNonNull(const ConstantSP& index, const ConstantSP& value) {return false;}
	virtual bool setItem(INDEX index, const ConstantSP& value){return set(index,value);}
	virtual void setItemToHeap(Heap* pHeap, INDEX heapIndex, INDEX itemIndex, const string& name);
	virtual bool setColumn(INDEX index, const ConstantSP& value){return assign(value);}
	virtual void setRowLabel(const ConstantSP& label){}
	virtual void setColumnLabel(const ConstantSP& label){}
	virtual bool reshape(INDEX cols, INDEX rows) {return false;}
	/**
	 * @brief Replace the underlying data to be value.
	 *
	 * @param value: The data to be assigned. Note that the size of value must be the same as the
	 * 				 		the size of this constant.
	 * @return true if assignment succeed, else false.
	 */
	virtual bool assign(const ConstantSP& value){return false;}

	/**
	 * @brief Set the data from start to (start + len - 1) with buf.
	 * 		  This is the recommended method to write data in Constant.
	 * 		  This function is often used together with getBoolBuffer(start, len, buf).
	 * 		  Note that if buf is already among the underlying data, then actually
	 * 		  		nothing happens in this function; otherwise, the data of buf will be
	 * 				copied.
	 *
	 * @param start: the start index
	 * @param len: the length of data to set
	 * @param buf: a buffer with at least length len
	 * @return true if set succeed, else false
	 */
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
	virtual bool setString(INDEX start, int len, const DolphinString** buf){return false;}
	virtual bool setBinary(INDEX start, int len, int unitLength, const unsigned char* buf){return false;}
	virtual bool setData(INDEX start, int len, void* buf) {return false;}

	virtual bool setDecimal32(INDEX start, int len, int scale, const int *buf) { NOT_IMPLEMENT; }
	virtual bool setDecimal64(INDEX start, int len, int scale, const long long *buf) { NOT_IMPLEMENT; }
	virtual bool setDecimal128(INDEX start, int len, int scale, const int128 *buf) { NOT_IMPLEMENT; }

	/**
	 * @brief Add inc to the underlying data from start to (start + length - 1).
	 * @return true if succeed, else false
	 */
	virtual bool add(INDEX start, INDEX length, long long inc) { return false;}
	virtual bool add(INDEX start, INDEX length, double inc) { return false;}
	virtual void validate() {}
	/**
	 * @brief Compare the index-th cell with the constant target
	 *
	 * @param index
	 * @param target
	 * @return 0: if index-th cell is equal to target
	 * 		   1: if index-th cell is larger than target
	 * 		  -1: if index-th cell is smaller than target
	 */
	virtual int compare(INDEX index, const ConstantSP& target) const {return 0;}

	/**
	 * @brief Get the null flag.
	 *
	 * @return true: this constant **may** contain null value
	 * @return false: this constant must not contain null value
	 */
	virtual bool getNullFlag() const {return isNull();}
	/**
	 * @brief Set the null flag.
	 *
	 * @param containNull: true if this constant **may** contain null value,
	 * 					   false if this constant must not contain null value
	 */
	virtual void setNullFlag(bool containNull){}
	/**
	 * @brief Return whether this constant has null value.
	 */
	virtual bool hasNull(){return  isNull();}
	/**
	 * @brief Return whether this constant has null value from start to (start + length - 1).
	 */
	virtual bool hasNull(INDEX start, INDEX length){return isNull();}
	virtual bool sizeable() const {return false;}
	virtual bool copyable() const {return true;}
	virtual INDEX size() const {return 1;}
	virtual INDEX itemCount() const {return getForm()==DF_MATRIX?columns():size();}
	virtual INDEX rows() const {return size();}
	virtual INDEX uncompressedRows() const {return size();}
	virtual INDEX columns() const {return 1;};
	virtual ConstantSP getMember(const ConstantSP& key) const { throw RuntimeException("getMember method not supported");}
	virtual ConstantSP keys() const {throw RuntimeException("keys method not supported");}
	virtual ConstantSP values() const {throw RuntimeException("values method not supported");}
	virtual ConstantSP callMethod(const string& name, Heap* heap, vector<ConstantSP>& args) const {throw RuntimeException("callMethod method not supported");}

	virtual long long releaseMemory(long long target, bool& satisfied) { satisfied = false; return 0;}
	/**
	 * @brief Get the allocated memory of this constant in bytes.
	 */
	virtual long long getAllocatedMemory() const {return 0;}
	virtual DATA_TYPE getRawType() const =0;
	virtual int getExtraParamForType() const { return 0;}

	virtual ConstantSP getInstance() const =0;
	/**
	 * @brief Get a copy of this constant.
	 */
	virtual ConstantSP getValue() const =0;
	virtual ConstantSP getValue (Heap* pHeap){return getValue();}
	virtual ConstantSP getReference(Heap* pHeap){return getValue();}
	virtual OBJECT_TYPE getObjectType() const {return CONSTOBJ;}
	virtual SymbolBaseSP getSymbolBase() const {return SymbolBaseSP();}
	virtual void contain(const ConstantSP& target, const ConstantSP& resultSP) const {throw RuntimeException("contain method not supported");}
	/**
	 * @brief Return whether this vector is a regular vector, i.e. all the data is contiguous.
	 */
	virtual bool isFastMode() const {return false;}
	/**
	 * @brief Get the underlying data array of this constant. If the data is not contiguous, return 0.
	 */
	virtual void* getDataArray() const {return 0;}
	/**
	 * @brief Get the underlying data array segment of this constant. If the data is contiguous, return 0.
	 */
	virtual void** getDataSegment() const {return 0;}
	virtual bool isIndexArray() const { return false;}
	virtual INDEX* getIndexArray() const { return NULL;}
	virtual bool isHugeIndexArray() const { return false;}
	virtual INDEX** getHugeIndexArray() const { return NULL;}
	virtual int getSegmentSize() const { return 1;}
	virtual int getSegmentSizeInBit() const { return 0;}
	virtual bool containNotMarshallableObject() const {return false;}
	/**
	 * @brief Modify the member specified by the index using the given function and the parameters. Usually the current object
	 * is a tuple or an any dictionary.
	 *
	 * @param heap: the heap of the execution context.
	 * @param index: index must be a scalar, regular vector, or tuple.
	 * @param func: the mutable function that can manipulate the member object.
	 * @param parameters: the extra parameters for the function.
	 * @param dim: dim is a zero-based index. The index's dim-th element is the index of the current object to update.
	 * @return true if set succeed, false else.
	 */
	virtual bool modifyMember(Heap* heap, const FunctionDefSP& func, const ConstantSP& index, const ConstantSP& parameters, int dim){return false;}

protected:
	inline void setType(DATA_TYPE dt){ flag_ = (flag_ & 4278255615U) + (dt << 16);}
	inline void setCategory(DATA_CATEGORY dc){ flag_ = (flag_ & 4043309055U) + (dc << 24);}
	inline void setTypeAndCategory(DATA_TYPE dt, DATA_CATEGORY dc){ flag_ = (flag_ & 4026597375U) + (((dc<<8) + dt) << 16);}

private:
	unsigned int flag_;
};

#undef NOT_IMPLEMENT

class Vector : public Constant {
public:
	/// Once you overload a function (virtual function or normal function) from Base class
	/// in Derived class all functions with the same name in the Base class get hidden in
	/// Derived class.
	/// ref: https://stackoverflow.com/questions/8816794/overloading-a-virtual-function-in-a-child-class
	using Constant::get;

public:
	Vector(): Constant(259){}
	Vector(DATA_TYPE dt, DATA_CATEGORY dc): Constant(DF_VECTOR, dt, dc){}
	virtual ~Vector(){}
	virtual ConstantSP getColumnLabel() const;
	const string& getName() const {return name_;}
	void setName(const string& name){name_=name;}
	virtual bool isLargeConstant() const { return isMatrix() || size()>1024; }
	virtual bool isView() const {return false;}
	virtual bool isRepeatingVector() const {return false;}
	virtual VECTOR_TYPE getVectorType() const {return VECTOR_TYPE::OTHER;}
	virtual void initialize(){}
	virtual INDEX getCapacity() const = 0;
	virtual	INDEX reserve(INDEX capacity) {throw RuntimeException("Vector::reserve method not supported");}
	virtual	void resize(INDEX size) {throw RuntimeException("Vector::resize method not supported");}
	virtual short getUnitLength() const = 0;
	virtual void clear()=0;
	virtual bool isTableColumn() const {return false;};
	/**
	 * @brief Remove the last count elements from this vector.
	 */
	virtual bool remove(INDEX count){return false;}
	virtual bool remove(const ConstantSP& index){return false;}
	virtual bool append(const ConstantSP& value){return append(value, 0, value->size());}
	virtual bool append(const ConstantSP& value, INDEX count){return append(value, 0, count);}
	/**
	 * Append the value of the given vector at the specified range to the end of the current vector.
	 *
	 * start:
	 * count:
	 */
	virtual bool append(const ConstantSP& value, INDEX start, INDEX count){return false;}
	/**
	 * Append the value specified by the index to the end of the vector.
	 *
	 * value: vector.
	 * index: index vector. Make sure all indices in the vector are valid, i.e. no less than zero and less than the size of the value.
	 */
	virtual bool append(const ConstantSP& value, const ConstantSP& index){return append(value->get(index), 0, index->size());}
	virtual ConstantSP moveGet(const ConstantSP& index) { throw RuntimeException("Vector::moveGet method not supported"); }
	virtual bool moveAppend(ConstantSP& value, INDEX start, INDEX len) { throw RuntimeException("Vector::moveAppend method not supported"); }

	virtual bool appendBool(const char* buf, int len){return false;}
	virtual bool appendChar(const char* buf, int len){return false;}
	virtual bool appendShort(const short* buf, int len){return false;}
	virtual bool appendInt(const int* buf, int len){return false;}
	virtual bool appendLong(const long long* buf, int len){return false;}
	virtual bool appendIndex(const INDEX* buf, int len){return false;}
	virtual bool appendFloat(const float* buf, int len){return false;}
	virtual bool appendDouble(const double* buf, int len){return false;}
	virtual bool appendString(const DolphinString** buf, int len){return false;}
	virtual bool appendString(const string* buf, int len){return false;}
	virtual bool appendString(const char** buf, int len){return false;}
	virtual bool appendGuid(const Guid* buf, int len){return appendBinary((const unsigned char*)buf, len, 16);}
	virtual bool appendBinary(const unsigned char* buf, int len, int unitLength){return false;}
	virtual string getString() const;
	virtual string getScript() const;
	virtual string getString(INDEX index) const = 0;
	virtual ConstantSP getInstance() const {return getInstance(size());}
	virtual ConstantSP getInstance(INDEX size) const = 0;
	/**
	 * @brief  Copy this vector.
	 *
	 * @param capacity: The capacity of the new vector.
	 * @return ConstantSP: The new vector.
	 */
	virtual ConstantSP getValue(INDEX capacity) const {throw RuntimeException("Vector::getValue method not supported");}
	virtual ConstantSP get(INDEX column, INDEX rowStart,INDEX rowEnd) const {return getSubVector(column*rows()+rowStart,rowEnd-rowStart);}
	virtual ConstantSP get(INDEX index) const = 0;
	virtual ConstantSP getWindow(INDEX colStart, int colLength, INDEX rowStart, int rowLength) const {return getSubVector(rowStart,rowLength);}
	/**
	 * @brief Get the sub-vector of this vector.
	 *
	 * @param start
	 * @param length
	 * @return ConstantSP: the sub-vector
	 */
	virtual ConstantSP getSubVector(INDEX start, INDEX length) const { throw RuntimeException("getSubVector method not supported");}
	virtual ConstantSP getSubVector(INDEX start, INDEX length, INDEX capacity) const { return getSubVector(start, length);}
	/**
	 * Fill the value of the vector at the specified range by the value of the given vector.
	 *
	 * start: the starting position of the current vector to fill.
	 * length: the number of cells of the current vector to fill.
	 * value: vector or scalar
	 * valueOffset: a valid index
	 */
	virtual void fill(INDEX start, INDEX length, const ConstantSP& value, INDEX valueOffset = 0) = 0;
	/**
	 * Fill the value of the vector at the specified range by the value of the given vector at the specified index.
	 *
	 * start: the starting position of the current vector to fill.
	 * length: the number of cells of the current vector to fill.
	 * value: vector
	 * index: index vector. Make sure all indices in the vector are valid, i.e. no less than zero and less than the size of the value.
	 */
	virtual void fill(INDEX start, INDEX length, const ConstantSP& value, const ConstantSP& index) { fill(start, length, value->get(index));}
	virtual void next(INDEX steps)=0;
	virtual void prev(INDEX steps)=0;
	virtual void reverse()=0;
	virtual void reverse(INDEX start, INDEX length)=0;
	virtual void shuffle(){}
	virtual void replace(const ConstantSP& oldVal, const ConstantSP& newVal){}
	virtual bool validIndex(INDEX uplimit){return false;}
	virtual bool validIndex(INDEX start, INDEX length, INDEX uplimit){return false;}
	virtual void addIndex(INDEX start, INDEX length, INDEX offset){}
	virtual void neg()=0;
	virtual void find(INDEX start, INDEX length, const ConstantSP& target, const ConstantSP& resultSP){}
	virtual void find(const ConstantSP& target, const ConstantSP& resultSP){
		find(0, size(), target, resultSP);
	}
	virtual void binarySearch(INDEX start, INDEX length, const ConstantSP& target, const ConstantSP& resultSP){}
	virtual void binarySearch(const ConstantSP& target, const ConstantSP& resultSP){
		binarySearch(0, size(), target, resultSP);
	}
	virtual void asof(INDEX start, INDEX length, const ConstantSP& target, const ConstantSP& resultSP){ throw RuntimeException("asof method not supported.");}
	virtual void asof(const ConstantSP& target, const ConstantSP& resultSP){
		asof(0, size(), target, resultSP);
	}
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
	virtual void max(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart=0) const = 0;
	/**
	 * @param rightMost If there are multiple maximum/minimum values, choose the last one if `rightMost` is true.
	 */
	virtual INDEX imax(bool rightMost = false) const = 0;
	virtual INDEX imax(INDEX start, INDEX length, bool rightMost = false) const = 0;
	virtual ConstantSP min() const = 0;
	virtual ConstantSP min(INDEX start, INDEX length) const = 0;
	virtual void min(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart=0) const = 0;
	/**
	 * @param rightMost If there are multiple maximum/minimum values, choose the last one if `rightMost` is true.
	 */
	virtual INDEX imin(bool rightMost = false) const = 0;
	virtual INDEX imin(INDEX start, INDEX length, bool rightMost = false) const = 0;
	virtual ConstantSP avg() const = 0;
	virtual ConstantSP avg(INDEX start, INDEX length) const = 0;
	virtual void avg(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart=0) const = 0;
	virtual ConstantSP sum() const = 0;
	virtual ConstantSP sum(INDEX start, INDEX length) const = 0;
	virtual void sum(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart=0) const = 0;
	virtual ConstantSP sum2() const = 0;
	virtual ConstantSP sum2(INDEX start, INDEX length) const = 0;
	virtual void sum2(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart=0) const = 0;
	virtual ConstantSP prd() const = 0;
	virtual ConstantSP prd(INDEX start, INDEX length) const = 0;
	virtual void prd(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart=0) const = 0;
	virtual ConstantSP var() const = 0;
	virtual ConstantSP var(INDEX start, INDEX length) const = 0;
	virtual void var(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart=0) const = 0;
	virtual ConstantSP std() const = 0;
	virtual ConstantSP std(INDEX start, INDEX length) const = 0;
	virtual void std(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart=0) const = 0;
	virtual ConstantSP median() const = 0;
	virtual ConstantSP median(INDEX start, INDEX length) const = 0;
	virtual void median(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart=0) const = 0;
	virtual ConstantSP searchK(INDEX k) const = 0;
	virtual ConstantSP searchK(INDEX start, INDEX length, INDEX k) const = 0;
	virtual void searchK(INDEX start, INDEX length, INDEX k, const ConstantSP& out, INDEX outputStart=0) const = 0;
	virtual ConstantSP mode() const = 0;
	virtual ConstantSP mode(INDEX start, INDEX length) const = 0;
	virtual void mode(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart=0) const = 0;
	virtual ConstantSP stat() const;
	virtual ConstantSP stat(INDEX start, INDEX length) const;
	virtual ConstantSP firstNot(const ConstantSP& exclude) const = 0;
	virtual ConstantSP firstNot(INDEX start, INDEX length, const ConstantSP& exclude) const = 0;
	virtual void firstNot(INDEX start, INDEX length, const ConstantSP& exclude, const ConstantSP& out, INDEX outputStart=0) const = 0;
	virtual ConstantSP lastNot(const ConstantSP& exclude) const = 0;
	virtual ConstantSP lastNot(INDEX start, INDEX length, const ConstantSP& exclude) const = 0;
	virtual void lastNot(INDEX start, INDEX length, const ConstantSP& exclude, const ConstantSP& out, INDEX outputStart=0) const = 0;
	/**
	 * @brief Return whether this vector is sorted.
	 *
	 * @param asc: Indicating whether this vector is sorted in ascending order(true) or descending order(false).
	 * @param strict: Indicating whether this vector is strictly increasing(decreasing).
	 * @param nullsOrder: 0: NONE, 1: NULLS FIRST, 2: NULLS LAST.
	 * @return True if sorted, else false.
	*/
	virtual bool isSorted(bool asc, bool strict = false, char nullsOrder = 0) const { return isSorted(0, size(), asc, strict, nullsOrder);}
	/**
	 * @brief Return whether the specified range of this vector is sorted.
	 *
	 * @param start: The starting position of the specified range.
	 * @param length: The length of the specified range.
	 * @param asc: Indicating whether this vector is sorted in ascending order(true) or descending order(false).
	 * @param strict: Indicating whether this vector is strictly increasing(decreasing).
	 * @param nullsOrder: 0: NONE, 1: NULLS FIRST, 2: NULLS LAST.
	 * @return True if sorted, else false.
	*/
	virtual bool isSorted(INDEX start, INDEX length, bool asc, bool strict = false, char nullsOrder = 0) const = 0;

	/**
	 * Find the first element that is no less than the target value in the sorted vector. If all elements are
	 * less than the target value, return the size of the vector.
	 * start: the starting point of the search.
	 */
	virtual INDEX lowerBound(INDEX start, const ConstantSP& target)=0;
	/**
	 * Find the range of the specified value in a sorted vector.
	 *
	 * target: the target value. it must be a scalar.
	 * range: in/out parameter in the format of pair<offset, length>. When serving as a input parameter,
	 * it specifies the range to search. The search result is stored in this parameter too. If no element
	 * equals to the target value, the length of the output is set to 0.
	 */
	virtual void equalRange(const ConstantSP& target, pair<INDEX, INDEX>& range) const {throw RuntimeException("equalRange method not supported");}

	virtual bool equalToPrior(INDEX start, INDEX length, bool* result){ return false;}
	virtual bool equalToPrior(INDEX prior, const INDEX* indices, INDEX length, bool* result){ return false;}
	virtual ConstantSP topK(INDEX start, INDEX length, INDEX top, bool asc, bool extendEqualValue) {throw RuntimeException("topK method not supported");}
	virtual ConstantSP topK(INDEX top, bool asc, bool extendEqualValue) {
		return topK(0, size(), top, asc, extendEqualValue);
	}
	/**
	 * @brief Sort the whole vector with given order.
	 *
	 * @param asc: Indicating if it is ascending order.
	 * @param nullsOrder: 0: NONE, 1: NULLS FIRST, 2: NULLS LAST.
	 * @return True if sort succeed, else false.
	 */
	virtual bool sort(bool asc, char nullsOrder = 0) = 0;

	/**
	 * @brief Sort the vector and the corresponding indices with given order.
	 *
	 * @param asc: Indicating if it is ascending order.
	 * @param indices: The corresponding indices of the data to sort.The length of the data should be equal to
	 * 				   the length of the indices. The indices will be rearranged accordingly during sorting.
	 * @param nullsOrder: 0: NONE, 1: NULLS FIRST, 2: NULLS LAST.
	 * @return True if sort succeed, else false.
	 */
    virtual bool sort(bool asc, Vector* indices, char nullsOrder = 0) = 0;

	/**
	 * @brief Sort top-th elements of this vector and the corresponding indices with given order.
	 *
	 * @param asc: Indicating if it is ascending order.
	 * @param indices: The corresponding indices of the data to sort.
	 * @param nullsOrder: 0: NONE, 1: NULLS FIRST, 2: NULLS LAST.
	 * @return True if sort succeed, else false.
	 */
	virtual INDEX sortTop(bool asc, Vector* indices, INDEX top, char nullsOrder = 0) {throw RuntimeException("sortTop method not supported");}

	/**
	 * @brief Sort the selected indices based on the corresponding data with given order.
	 *
	 * @param indices: The selected indices to sort.
	 * @param start: The start position in indices vector.
	 * @param length: The number of indices to sort.
	 * @param asc: Indicating if it is ascending order.
	 * @param nullsOrder: 0: NONE, 1: NULLS FIRST, 2: NULLS LAST.
	 * @return True if sort succeed, else false.
	 */
	virtual bool sortSelectedIndices(Vector* indices, INDEX start, INDEX length, bool asc, char nullsOrder = 0) = 0;

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
	virtual long long getAllocatedMemory(INDEX size) const {return Constant::getAllocatedMemory();}
	virtual long long getAllocatedMemory() const {return getAllocatedMemory(size());}
	virtual int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int& numElement, int& partial) const {throw RuntimeException("serialize method not supported");}
	virtual int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int targetNumElement, int& numElement, int& partial) const;

	/**
	 * The following series of safe operators assumes:
	 * (1) indices is ascending sorted
	 * (2) offset + indices are guaranteed valid ( between 0 and size - 1)
	 */
	virtual bool isNullSafe(INDEX offset, INDEX* indices, int len, char* buf) const {return false;}
	virtual bool isValidSafe(INDEX offset, INDEX* indices, int len, char* buf) const {return false;}
	virtual bool getBoolSafe(INDEX offset, INDEX* indices, int len, char* buf) const {return false;}
	virtual bool getCharSafe(INDEX offset, INDEX* indices, int len,char* buf) const {return false;}
	virtual bool getShortSafe(INDEX offset, INDEX* indices, int len, short* buf) const {return false;}
	virtual bool getIntSafe(INDEX offset, INDEX* indices, int len, int* buf) const {return false;}
	virtual bool getLongSafe(INDEX offset, INDEX* indices, int len, long long* buf) const {return false;}
	virtual bool getIndexSafe(INDEX offset, INDEX* indices, int len, INDEX* buf) const {return false;}
	virtual bool getFloatSafe(INDEX offset, INDEX* indices, int len, float* buf) const {return false;}
	virtual bool getDoubleSafe(INDEX offset, INDEX* indices, int len, double* buf) const {return false;}
	virtual bool getSymbolSafe(INDEX offset, INDEX* indices, int len, int* buf, SymbolBase* symBase,bool insertIfNotThere) const {return false;}
	virtual bool getStringSafe(INDEX offset, INDEX* indices, int len, DolphinString** buf) const {return false;}
	virtual bool getStringSafe(INDEX offset, INDEX* indices, int len, char** buf) const {return false;}
	virtual bool getBinarySafe(INDEX offset, INDEX* indices, int len, int unitLength, unsigned char* buf) const {return false;}

	/**
	 * An array vector must implement following methods.
	 */
	virtual ConstantSP flatten(INDEX rowStart, INDEX count) const {throw RuntimeException("flatten method not supported");}
	virtual ConstantSP rowFirst(INDEX rowStart, INDEX count) const {throw RuntimeException("rowFirst method not supported");}
	virtual ConstantSP rowLast(INDEX rowStart, INDEX count) const {throw RuntimeException("rowLast method not supported");}
	virtual ConstantSP rowFirstNot(INDEX rowStart, INDEX count, const ConstantSP& exclude) const {throw RuntimeException("rowFirstNot method not supported");}
	virtual ConstantSP rowLastNot(INDEX rowStart, INDEX count, const ConstantSP& exclude) const {throw RuntimeException("rowLastNot method not supported");}
	virtual ConstantSP rowSum(INDEX rowStart, INDEX count) const {throw RuntimeException("rowSum method not supported");}
	virtual ConstantSP rowSum2(INDEX rowStart, INDEX count) const {throw RuntimeException("rowSum2 method not supported");}
	virtual ConstantSP rowCount(INDEX rowStart, INDEX count) const {throw RuntimeException("rowCount method not supported");}
	virtual ConstantSP rowSize(INDEX rowStart, INDEX count) const {throw RuntimeException("rowSize method not supported");}
	virtual ConstantSP rowAvg(INDEX rowStart, INDEX count) const {throw RuntimeException("rowAvg method not supported");}
	virtual ConstantSP rowStd(INDEX rowStart, INDEX count) const {throw RuntimeException("rowStd method not supported");}
	virtual ConstantSP rowStdp(INDEX rowStart, INDEX count) const {throw RuntimeException("rowStdp method not supported");}
	virtual ConstantSP rowVar(INDEX rowStart, INDEX count) const {throw RuntimeException("rowVar method not supported");}
	virtual ConstantSP rowVarp(INDEX rowStart, INDEX count) const {throw RuntimeException("rowVarp method not supported");}
	virtual ConstantSP rowMin(INDEX rowStart, INDEX count) const {throw RuntimeException("rowMin method not supported");}
	virtual ConstantSP rowMax(INDEX rowStart, INDEX count) const {throw RuntimeException("rowMax method not supported");}
	virtual ConstantSP rowProd(INDEX rowStart, INDEX count) const {throw RuntimeException("rowProd method not supported");}
	virtual ConstantSP rowAnd(INDEX rowStart, INDEX count) const {throw RuntimeException("rowAnd method not supported");}
	virtual ConstantSP rowOr(INDEX rowStart, INDEX count) const {throw RuntimeException("rowOr method not supported");}
	virtual ConstantSP rowXor(INDEX rowStart, INDEX count) const {throw RuntimeException("rowXor method not supported");}
	virtual ConstantSP rowMed(INDEX rowStart, INDEX count) const {throw RuntimeException("rowMed method not supported");}
	virtual ConstantSP rowKurtosis(INDEX rowStart, INDEX count, bool biased) const {throw RuntimeException("rowKurtosis method not supported");}
	virtual ConstantSP rowSkew(INDEX rowStart, INDEX count, bool biased) const {throw RuntimeException("rowSkew method not supported");}
	virtual ConstantSP rowPercentile(INDEX rowStart, INDEX count, double percentile) const {throw RuntimeException("rowPercentile method not supported");}
	virtual ConstantSP rowRank(INDEX rowStart, INDEX count, bool ascending, int groupNum, bool ignoreNA, int tiesMethod, bool percent) const {throw RuntimeException("rowRank method not supported");}
	virtual ConstantSP rowDenseRank(INDEX rowStart, INDEX count, bool ascending, bool ignoreNA, bool percent) const {throw RuntimeException("rowDenseRank method not supported");}

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
	Set(DATA_TYPE dt, DATA_CATEGORY dc) : Constant(DF_SET, dt, dc){}
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
	virtual void* getRawSet() const = 0;
};

class Dictionary:public Constant{
public:
	Dictionary(DATA_TYPE dt, DATA_CATEGORY dc) : Constant(DF_DICTIONARY, dt, dc), lock_(0){}
	virtual ~Dictionary();
	virtual INDEX size() const = 0;
	virtual INDEX count() const = 0;
	virtual void clear()=0;
	virtual ConstantSP getMember(const ConstantSP& key) const =0;
	virtual ConstantSP getMember(const string& key) const {throw RuntimeException("String key not supported");}
	virtual ConstantSP get(INDEX column, INDEX row){throw RuntimeException("Dictionary does not support cell function");}
	virtual SymbolBaseSP getKeySymbolBase() const { return nullptr;}
	virtual DATA_TYPE getKeyType() const = 0;
	virtual DATA_CATEGORY getKeyCategory() const = 0;
	virtual ConstantSP keys() const = 0;
	virtual ConstantSP values() const = 0;
	virtual string getString() const = 0;
	virtual string getScript() const {return "dict()";}
	virtual string getString(int index) const {throw RuntimeException("Dictionary::getString(int index) not supported");}
	virtual bool remove(const ConstantSP& key) = 0;
	virtual bool set(const ConstantSP& key, const ConstantSP& value)=0;
	// This set function avoids the overhead of smart pointers to improve JIT speed
	virtual bool set(Constant& key, Constant& value) { return false; }
	/**
	 * @brief Replace the cell value specified by the index with the new value. Usually the current object
	 * is a tuple or an any dictionary.
	 *
	 * @param heap: the heap of the execution context.
	 * @param index: index must be a tuple.
	 * @param value: could be any ConstantSP object.
	 * @param dim: dim is a zero-based index. The index's dim-th element is the index of the current object to update.
	 * @return true if set succeed, false else.
	 */
	virtual bool set(Heap* heap, const ConstantSP& index, const ConstantSP& value, int dim) {return false;}
	/**
	 * @brief Set the element values accoreding to key.
	 *
	 * @param key:Indicate the key of the elements to set.
	 * @param value:A scalar.
	 * @return True if set succeed, else false.
	 */
	virtual bool set(const string& key, const ConstantSP& value){throw RuntimeException("String key not supported");}
	virtual bool reduce(BinaryOperator& optr, const ConstantSP& key, const ConstantSP& value)=0;
	virtual bool reduce(Heap* heap, const FunctionDefSP& optr, const FunctionDefSP& initOptr, const ConstantSP& key, const ConstantSP& value)=0;
	virtual bool modifyMember(Heap* heap, const FunctionDefSP& func, const ConstantSP& index, const ConstantSP& parameters, int dim){return false;}
	virtual ConstantSP get(const ConstantSP& key) const {return getMember(key);}
	virtual void contain(const ConstantSP& target, const ConstantSP& resultSP) const = 0;
	virtual bool isLargeConstant() const {return true;}
	virtual void* getRawMap() const = 0;
	virtual bool isOrdered() const = 0;
	inline Mutex* getLock() const { return lock_;}
	inline void setLock(Mutex* lock) { lock_ = lock;}

private:
	Mutex* lock_;
};

class Table: public Constant{
public:
	/// Once you overload a function (virtual function or normal function) from Base class
	/// in Derived class all functions with the same name in the Base class get hidden in
	/// Derived class.
	/// ref: https://stackoverflow.com/questions/8816794/overloading-a-virtual-function-in-a-child-class
	using Constant::get;

public:
	Table() : Constant(DF_TABLE, DT_DICTIONARY, MIXED), flag_(0), engineType_((char)DBENGINE_TYPE::OLAP), lock_(0){}
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
	virtual int getColumnExtraParam(int index) const { throw RuntimeException("Table::getColumnExtraParam() not supported"); }
	virtual bool contain(const string& name) const = 0;
	virtual bool contain(const string& qualifier, const string& name) const = 0;
	virtual bool contain(const ColumnRef* col) const = 0;
	virtual bool contain(const ColumnRefSP& col) const = 0;
	virtual bool containAll(const vector<ColumnRefSP>& cols) const = 0;
	virtual void setName(const string& name)=0;
	virtual const string& getName() const = 0;
	virtual ConstantSP get(INDEX index) const {return getColumn(index);}
	virtual ConstantSP get(const ConstantSP& index) const = 0;
	virtual ConstantSP getValue(INDEX capacity) const = 0;
	virtual ConstantSP getValue() const = 0;
	virtual ConstantSP getInstance(INDEX size) const = 0;
	virtual INDEX size() const = 0;
	virtual bool sizeable() const = 0;
	virtual string getString(INDEX index) const = 0;
	virtual string getString() const = 0;
	virtual ConstantSP getWindow(INDEX colStart, int colLength, INDEX rowStart, int rowLength) const = 0;
	virtual ConstantSP getMember(const ConstantSP& key) const = 0;
	virtual ConstantSP values() const = 0;
	virtual ConstantSP keys() const = 0;
	virtual TABLE_TYPE getTableType() const = 0;
	virtual bool join(vector<ConstantSP>& columns) { return false;}
	virtual	bool clear() { return false;}
	/**
	 * @brief Reorder the columns of the table with the given new orders.
	 *
	 * @param newOrders: indices of the columns of the new orders.
	 * @return true if reorder succeed, else false.
	 */
	virtual bool reorderColumns(const vector<int>& newOrders) { return false;}
	virtual bool replaceColumn(int index, const ConstantSP& col) {return false;}
	/**
	 * @brief Drop specified columns.
	 *
	 * @param columns: indices of the columns to drop
	 * @return true if drop succeed, else false
	 */
	virtual bool drop(vector<int>& columns) {return false;}
	virtual void remove(Heap* heap, const SQLContextSP& context, const ConstantSP& filterExprs) {throw RuntimeException("Table::remove() not supported");}
	virtual void sortBy(Heap* heap, const ObjectSP& sortExpr, const ConstantSP& sortOrder) {throw RuntimeException("Table::sortBy() not supported");}
	virtual void update(Heap* heap, const SQLContextSP& context, const ConstantSP& updateColNames, const ObjectSP& updateExpr, const ConstantSP& filterExprs) {throw RuntimeException("Table::update() not supported");}
	virtual bool update(vector<ConstantSP>& values, const ConstantSP& indexSP, vector<string>& colNames, string& errMsg) = 0;
	/**
	 * @brief Append values to this table.
	 *
	 * @param values: the values to append
	 * @param insertedRows: out parameter, means how many rows are actually inserted
	 * @param errMsg: if the append fails, the error message is stored in errMsg.
	 * @return true if append succeed, else false
	 */
	virtual bool append(vector<ConstantSP>& values, INDEX& insertedRows, string& errMsg) = 0;
    virtual bool append(Heap* heap, vector<ConstantSP>& values, INDEX& insertedRows, string& errMsg){
        return append(values, insertedRows, errMsg);
    }
	virtual bool remove(const ConstantSP& indexSP, string& errMsg) = 0;
	virtual bool upsert(vector<ConstantSP>& values, bool ignoreNull, INDEX& insertedRows, string& errMsg) {throw RuntimeException("Table::upsert() not supported");}
	virtual bool upsert(vector<ConstantSP>& values, bool ignoreNull, INDEX& insertedRows, INDEX& updatedRows,
						string& errMsg) {
		throw RuntimeException("Table::upsert() not supported");
	}
	virtual DATA_TYPE getRawType() const {return DT_DICTIONARY;}
	virtual bool isDistributedTable() const {return false;}
	virtual bool isSegmentedTable() const {return false;}
	virtual bool isDimensionalTable() const {return false;}
	virtual bool isBasicTable() const {return false;}
	virtual bool isDFSTable() const {return false;}
	virtual bool isAppendable() const {return false;}
	virtual bool isEditable() const {return false;}
	virtual bool isSchemaEditable() const {return false;}
	virtual int getSortKeyCount() const { return 0;}
	virtual int getSortKeyColumnIndex(int index){return -1;}
	virtual int isAscendingKey(int index) { return true;}
	virtual DomainSP getGlobalDomain() const {return DomainSP();}
	virtual DomainSP getLocalDomain() const {return DomainSP();}
	virtual int getGlobalPartitionColumnIndex() const {return -1;}
	virtual int getLocalPartitionColumnIndex(int dim) const {return -1;}
	virtual void setGlobalPartition(const DomainSP& domain, const string& partitionColumn){throw RuntimeException("Table::setGlobalPartition() not supported");}
	virtual bool isLargeConstant() const {return true;}
	virtual bool addSubscriber(const TableUpdateQueueSP& queue, const string& topic, bool local, long long offset = -1, const ObjectSP& filter = 0) { return false;}
	virtual bool removeSubscriber(const TableUpdateQueueSP& queue, const string& topic) { return false;}
	virtual bool subscriberExists(const TableUpdateQueueSP& queue, const string& topic) const { return false;}
	virtual void release() const {}
	virtual void checkout() const {}
	virtual TableSP getSegment(Heap* heap, const DomainPartitionSP& partition, PartitionGuard* guard = 0) { throw RuntimeException("Table::getSegment() not supported");}
	/**
	 * @brief Retrieve the data of a partition with filters from this table.
	 *
	 * @param heap:
	 * @param partition : The partition information.
	 * @param filters: An in/out parameter. If all loaded rows satisfy one filter, the filter will be removed from filters.
	 * @param colNames: If colNames is empty, all columns of the tablet might be used. Otherwise, the specified columns would be used.
	 * @param guard: The memory of the returned table will not be released until the guard destructed.
	 * @param limit: The number of rows to read, positive means from the start, negative means from the end, zero means all.
	 * @param byKey: Whether the limit is by key.
	 * @param extraMap: A helper map containing index and query info
	 * @return An in-memory table containing the specified data.
	 */
	virtual TableSP getSegment(Heap *heap, const DomainPartitionSP &partition, vector<ObjectSP> &filters,
							const vector<string> &colNames, PartitionGuard *guard = 0, INDEX limit = 0,
							bool byKey = false, const unordered_map<int, void*> &extraMap = unordered_map<int, void*>{}) {
		throw RuntimeException("Table::getSegment() not supported");
	}
	/**
	 * @brief Return a empty table with the same schema as this table.
	 */
	virtual const TableSP& getEmptySegment() const { throw RuntimeException("Table::getEmptySegment() not supported");}
	virtual bool segmentExists(const DomainPartitionSP& partition) const { throw RuntimeException("Table::segmentExists() not supported");}
	virtual int getPartitionCount() const { throw RuntimeException("Table::getPartitionCount() not supported"); }
	virtual long long getAllocatedMemory() const = 0;
	virtual ConstantSP retrieveMessage(long long offset, int length, bool msgAsTable, const ObjectSP& filter, long long& messageId) { throw RuntimeException("Table::retrieveMessage() not supported"); }
	virtual INDEX getFilterColumnIndex() const { return -1; };
	virtual bool snapshotIsolate() const { return false;}
	virtual void getSnapshot(TableSP& copy) const {}
	virtual bool readPermitted(const AuthenticatedUserSP& user) const {return true;}
	virtual bool isExpired() const { return flag_ & 8;}
	virtual void transferAsString(bool option){throw RuntimeException("Table::transferAsString() not supported");}
	virtual int getKeyColumnCount() const { return 0;}
	virtual int getKeyColumnIndex(int index) const { throw RuntimeException("Table::getKeyColumnIndex() not supported");}
	virtual int getKeyTimeColumnIndex() const { throw RuntimeException("Table::getKeyTimeColumnIndex() not supported");}
	virtual string getChunkPath() const { return "";}
	virtual void share(){};

	/**
	 * Filter the table by a set of column and value relationships.
	 *
	 * filterExprs is an in/out parameter. If a filter is applied, it will be removed from filters.
	 *
	 * limit 0: no limit on each key, positive: first n rows, negative: last n rows
	 *
	 * byKey  true: limit by key, false: global limit
	 *
	 * Return an index vector. If no filter is applied or applied filters satisfy all rows of the table, return a null pointer.
	 */
	virtual ConstantSP filter(vector<ObjectSP>& filterExprs, INDEX limit = 0, bool byKey = true) const { return nullptr;}
	virtual bool supportBlockFilter() const {return false;}
	/**
	 * Prepare data in advance.
	 *
	 * rows: the row indices. If rows is a null pointer, load all rows. The indices are in ascending order.
	 * cols: the column indices. if empty, load all columns. The indices are in ascending order.
	 *
	 * return: true if this table supports the feature.
	 */
	virtual bool prepareData(const ConstantSP& rows, const vector<int>& cols) { return false;}

	/**
     * Group by the data according to sortkeys.
     *
     * groupBy: an in-out parameter. If there are sortKeys in groupBy columns, then we fill group the data by sortkeys, and remove
     * sortKey columns from groupBy.
     *
     * filter: an input parameter. The filter for the whole table. If the filter is specified, it must be in strictly ascending order.
     *
     * tablets: an outout parameter. If there are sortKeys in groupBy columns, then we will group the data and save them in tablets,
     * i.e. each tablet in tablets is a group. If there is no sortKey column in groupBy, then tablets will be empty.
     *
     * removedGroupBys: an output parameter. The groupBy objects for sortkeys. These objects are removed from the input groupBy.
     *
     * groupedFilters: an output parameter. the filter for each tablet
     */
    virtual void groupBySortKeys(vector<string>& groupBy, const ConstantSP& filter, vector<TableSP>& tablets, vector<string>& removedGroupBys, vector<ConstantSP>& groupedFilters){}

    virtual DUPLICATE_POLICY getRowDuplicatePolicy() const { return DUPLICATE_POLICY::KEEP_ALL;}

    inline bool isSharedTable() const {return flag_ & 1;}
	inline bool isRealtimeTable() const {return flag_ & 2;}
	inline bool isAliasTable() const {return flag_ & 4;}
	inline const string& getOwner() const {return owner_;}
	inline void setOwner(const string& owner){ owner_ = owner;}
	inline DBENGINE_TYPE getEngineType() const { return (DBENGINE_TYPE)engineType_;}
	inline void setEngineType(DBENGINE_TYPE engineType) { engineType_ = (char)engineType;}
	void setSharedTable(bool option);
	void setRealtimeTable(bool option);
	void setAliasTable(bool option);
	void setExpired(bool option);
	inline Mutex* getLock() const { return lock_;}
	virtual bool writePermitted(const AuthenticatedUserSP& user) const { return true; }
    virtual const string& getPhysicalName() const {return getName();}
	virtual void setAccessControl(bool option) {
		if (option) {
			flag_ |= 16;
		}
		else {
			flag_ &= ~16;
		}
	}
	virtual bool isAccessControl() const { return flag_ & 16; }
private:
	/*
	 * BIT0: shared table
	 * BIT1: stream table
	 * BIT2: alias table
	 * BIT3: expired
	 * BIT4: access control or not
	 */
	char flag_;
	char engineType_;
	string owner_;
	Mutex* lock_;
};

class DFSChunkMeta : public Constant{
public:
	DFSChunkMeta(const string& path, const Guid& id, int version, int size, CHUNK_TYPE chunkType, const vector<string>& sites, long long cid, long long term = -1, bool prefetchComputeNodeData = false);
	DFSChunkMeta(const string& path, const Guid& id, int version, int size, CHUNK_TYPE chunkType, const string* sites, int siteCount, long long cid, long long term = -1, bool prefetchComputeNodeData = false);
	DFSChunkMeta(const DataInputStreamSP& in);
	virtual ~DFSChunkMeta();
	virtual IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const;
	virtual int size() const {return size_;}
	virtual string getString() const;
	virtual long long getAllocatedMemory() const;
	virtual ConstantSP getMember(const ConstantSP& key) const;
	virtual ConstantSP get(const ConstantSP& index) const {return getMember(index);}
	virtual ConstantSP keys() const;
	virtual ConstantSP values() const;
	virtual DATA_TYPE getRawType() const {return DT_DICTIONARY;}
	virtual ConstantSP getInstance() const {return getValue();}
	virtual ConstantSP getValue() const {return new DFSChunkMeta(path_, id_, version_, size_, (CHUNK_TYPE)type_, sites_, replicaCount_, cid_, term_, prefetchComputeNodeData_);}
	inline const string& getPath() const {return path_;}
	inline const Guid& getId() const {return id_;}
	inline long long getCommitId() const {return cid_;}
	inline void setCommitId(long long cid) { cid_ = cid;}
	inline int getVersion() const {return version_;}
	inline void setVersion(int version){version_ = version;}
	inline void setSize(int size){size_ = size;}
	inline int getCopyCount() const {return replicaCount_;}
	bool addCopySite(const string& siteAlias);
	inline const string& getCopySite(int index) const {return sites_[index];}
	inline bool isTablet() const { return type_ == TABLET_CHUNK;}
	inline bool isFileBlock() const { return type_ == FILE_CHUNK;}
	inline bool isSplittable() const { return type_ == SPLIT_TABLET_CHUNK;}
	inline bool isSmallFileBlock() const {return type_ == SMALLFILE_CHUNK;}
	inline CHUNK_TYPE getChunkType() const {return (CHUNK_TYPE)type_;}
	inline long long getTerm() const { return term_; }
	inline void setPrefetchComputeNodeData(bool v) { prefetchComputeNodeData_ = v;}
	inline bool getPrefetchComputeNodeData() const { return prefetchComputeNodeData_;  }

protected:
	ConstantSP getAttribute(const string& attr) const;
	ConstantSP getSiteVector() const;

private:
	char type_;
	unsigned char replicaCount_;
	int version_;
	int size_;
	string* sites_;
	string path_;
	long long cid_;
	Guid id_;
	long long term_;
	bool prefetchComputeNodeData_ = false;
};

class Param{
public:
	Param(const string& name, bool readOnly, const ConstantSP& defaultValue = nullptr);
	Param(Session* session, const DataInputStreamSP& in);
	const string& getName() const {return name_;}
	bool isReadOnly() const{return readOnly_;}
	DATA_TYPE getType() const;
	DATA_TYPE getKeyType() const;
	DATA_FORM getForm() const;
	void setMeta(DATA_FORM form, DATA_TYPE type, DATA_TYPE keyType);
	ConstantSP getDefaultValue() const { return defaultValue_;}
	inline bool isDefaultSet() const { return !defaultValue_.isNull() && ! defaultValue_->isNothing();}
	IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const;
	string getScript(int indention) const;
private:
	string name_;
	bool readOnly_;
	int meta_;
	ConstantSP defaultValue_;
};

class FunctionDef : public Constant{
public:
	FunctionDef(FUNCTIONDEF_TYPE defType, const string& name, const vector<ParamSP>& params, bool hasReturnValue=true, bool aggregation=false, bool sequential=false, bool transform=false);
	FunctionDef(FUNCTIONDEF_TYPE defType, const string& name, int minParamNum, int maxParamNum, bool hasReturnValue, bool aggregation=false, bool sequential=false, bool transform=false);
	inline const string& getName() const { return name_;}
	inline const string& getModule() const { return module_;}
	inline string getFullName() const { return module_.empty() ? name_ : module_ + "::" + name_;}
	inline void setSyntax(const string& syntax){ syntax_ = syntax;}
	string getSyntax() const;
	inline void setModule(const string& module) { module_ = module;}
	inline void setName(const string& name) { name_ = name; }
	inline bool hasReturnValue() const {return flag_ & 1;}
	inline bool isAggregatedFunction() const {	return flag_ & 2;}
	inline bool isAggregatedFunction(int args) const {	return (flag_ & 2) && (argCountForAgg_ == 0 || args == (int)argCountForAgg_);}
	inline void setArgCountForAggregation(int args) { argCountForAgg_ = args;}
	inline bool supportReturnMeta() const {	return flag_ & 64;}
	inline bool allConstantParameters() const { return flag_ & 4;}
	inline bool bySystemUser() const { return flag_ & 8;}
	inline void bySystemUser(bool option) { if(option) flag_ |= 8; else flag_ &= ~8;}
	inline bool isView() const { return flag_ & 16;}
	inline void setView(bool option) {if(option) flag_ |= 16; else flag_ &= ~16;}
	inline bool isInternal() const { return flag_ & 32;}
	inline void setInternal(bool option) {if(option) flag_ |= 32; else flag_ &= ~32;}
	inline bool isPrivate() const { return extraFlag_ & 1;}
	inline void setPrivate(bool option) {if(option) extraFlag_ |= 1; else extraFlag_ &= ~1;}
	inline bool isProtected() const { return extraFlag_ & 2;}
	inline void setProtected(bool option) {if(option) extraFlag_ |= 2; else extraFlag_ &= ~2;}
	inline bool isJIT() const { return extraFlag_ & 4;}
	inline void setJIT(bool option) {if(option) extraFlag_ |= 4; else extraFlag_ &= ~4;}
	inline bool isSequentialFunction() const { return extraFlag_ & 8;}
	inline void setScalarFunction(bool option){ if(option) extraFlag_ |= 16; else extraFlag_ &= ~16;}
	inline bool isScalarFunction() const { return extraFlag_ & 16;}
	inline void setHigherOrderFunction(bool option){ if(option) extraFlag_ |= 32; else extraFlag_ &= ~32;}
	inline bool isHigherOrderFunction() const { return extraFlag_ & 32;}
	inline void setArrayAggFunction(bool option){ if(option) extraFlag_ |= 64; else extraFlag_ &= ~64;}
	inline bool isArrayAggFunction() const { return extraFlag_ & 64;}
	inline void setPrimitiveOperator(bool option){ if(option) extraFlag_ |= 2048; else extraFlag_ &= ~2048;}
	inline bool isPrimitiveOperator() const { return extraFlag_ & 2048;}
	inline bool isTransformFunction() const { return extraFlag_ & 4096;}
	inline void setTransformFunction(bool option){ if(option) extraFlag_ |= 4096; else extraFlag_ &= ~4096;}
	inline bool isDummyFunction() const { return extraFlag_ & 8192;}
	inline void setDummyFunction(bool option){ if(option) extraFlag_ |= 8192; else extraFlag_ &= ~8192;}
	inline bool variableParamNum() const {	return minParamNum_<maxParamNum_;}
	inline int getMaxParamCount() const { return maxParamNum_;}
	inline int getMinParamCount() const {	return minParamNum_;}
	inline bool acceptParamCount(int count) const { return minParamNum_ <= count && maxParamNum_ >= count;}
	inline int getParamCount() const {return maxParamNum_;}
	const ParamSP& getParam(int index) const;
	inline bool isUserDefined() const {return defType_ == USERDEFFUNC;}
	inline bool isSystemFunction() const {return defType_ == SYSFUNC;}
	inline FUNCTIONDEF_TYPE getFunctionDefType() const {return defType_;}
	inline unsigned char getFlag() const { return flag_;}
	inline unsigned short getExtraFlag() const { return extraFlag_;}
	DATA_TYPE getReturnType() const;
	DATA_TYPE getReturnKeyType() const;
	DATA_FORM getReturnForm() const;
	void setReturnMeta(DATA_FORM form, DATA_TYPE type, DATA_TYPE keyType);
	inline int getReturnMeta() const { return returnMeta_;}
	void checkArgumentSize(int actualArgCount);
	virtual bool copyable() const {return false;}
	virtual DATA_TYPE getRawType() const { return DT_STRING;}
	virtual string getScript() const {return getFullName();}
	virtual string getString() const {return name_;}
	virtual IO_ERR serialize(Heap* pHeap, const ByteArrayCodeBufferSP& buffer) const = 0;
	virtual FunctionDefSP materializeFunctionDef(Heap* pHeap) { return FunctionDefSP();}
	virtual ConstantSP call(Heap* pHeap, vector<ConstantSP>& arguments) = 0;
	virtual ConstantSP call(Heap* pHeap, const ConstantSP& a, const ConstantSP& b) = 0;
	virtual ConstantSP call(Heap* pHeap,vector<ObjectSP>& arguments) = 0;
	virtual bool containNotMarshallableObject() const {return defType_ >= USERDEFFUNC ;}
	virtual FastFunc getFastImplementation() const {return nullptr;}

protected:
	void setConstantParameterFlag();
	void setReturnValueFlag(bool val);
	void setAggregationFlag(bool val);
	void setSequentialFlag(bool val);
	void setTransformFlag(bool val);

protected:
	static ParamSP constParam_;

	FUNCTIONDEF_TYPE defType_;
	string name_;
	string module_;
	string syntax_;
	vector<ParamSP> params_;
	int minParamNum_;
	int maxParamNum_;
	/**
	 * Some built-in functions such as min and max have multiple signatures. However,
	 * only one of them is an aggregate function. The attribute argCountForAgg_
	 * records the number of argument when it is an aggregate function.
	 */
	unsigned char argCountForAgg_;
	unsigned char flag_;
	unsigned short extraFlag_;
	int returnMeta_;
};

class SQLTransaction {
public:
	TableSP snapshot(Table* table) const;
	bool addTable(Table* table, const TableSP& snapshot);
	void clear();

private:
	vector<pair<Table*, TableSP>> snapshots_;
	mutable Mutex mutex_;
};

class SQLContext{
public:
	SQLContext();
	SQLContext(const SQLTransactionSP& trans);
	void setTable(const TableSP& table);
	void setFilter(const ConstantSP& filter);
	void setGroup(vector<INDEX>* group);
	SQLTransactionSP getTransaction() const { return transSP_;}
	TableSP getTable() const{return tableSP_;}
	ConstantSP getFilter() const{return filterSP_;}
	vector<INDEX>* getGroup();
	ConstantSP getColumn(const string& qualifier, const string& name);
	ConstantSP getColumn(const string& name);
    ConstantSP getColumn(INDEX colIdx);
    INDEX getColumnIndex(const string& qualifier, const string& name);
	INDEX getColumnIndex(const string& name);
	void cacheColumn(const string& name,const ConstantSP& col);
	inline int getFlag() const { return flag_;}
	inline void setFlag(int flag) { flag_ = flag;}
	void enableCache();
	inline void disableCache(){flag_ &= ~1;}
	inline bool isCacheEnabled() const { return flag_ & 1;}
	void clear();
	inline bool isInitialized() const { return !tableSP_.isNull();}
	inline bool isNotInitialized() const { return tableSP_.isNull();}
	inline bool withFilter(INDEX& length) const {
		if(filterSP_.isNull())
			return false;
		else {
			length = filterSP_->size();
			return true;
		}
	}
	inline bool isWithinMetaCode() const { return flag_ & 2;}
	inline void setWithinMetaCode(bool option) { if(option) flag_ |= 2; else flag_ &= ~2;}
	inline bool isColumnRefDisabled() const { return flag_ & 4;}
	inline void setColumnRefDisabled(bool option) { if(option) flag_ |= 4; else flag_ &= ~4;}
	inline bool isCommaCrossJoinDisabled() const { return flag_ & 8;}
	inline void setCommaCrossJoinDisabled(bool option) { if(option) flag_ |= 8; else flag_ &= ~8;}
	inline bool isWithinFromClause() const { return flag_ & 16;}
	inline void setWithinFromClause(bool option) { if(option) flag_ |= 16; else flag_ &= ~16;}
	inline bool isWithinAnalyticFunction() const { return flag_ & 32;}
	inline void setWithinAnalyticFunction(bool option) { if(option) flag_ |= 32; else flag_ &= ~32;}
	inline bool isDefinedAnalyticFunction() const { return flag_ & 64;}
	inline void setDefinedAnalyticFunction(bool option) { if(option) flag_ |= 64; else flag_ &= ~64;}
	inline bool isMacroUsed() const {return flag_ & 128;}
	inline void setMacroUsed(bool option) { if(option) flag_ |= 128; else flag_ &= ~128;}
	inline bool isWithinWhereClause() const { return flag_ & 256;}
	inline void setWithinWhereClause(bool option) { if(option) flag_ |= 256; else flag_ &= ~256;}
	inline bool castIotAny() const { return !(flag_ & 512); }
	inline void setCastIotAny(bool option) { if (!option) flag_ |= 512; else flag_ &= ~512; }

private:
	TableSP tableSP_;
	ConstantSP filterSP_;
	/**
	 * bit0: 0: not cache columns, 1: cache
	 * bit1: 0: not for meta code parsing, 1: for meta code parsing
	 * bit2: 0: parse as column reference, 1: not parse as column reference
	 * bit3: 0: allow comma as cross join, 1: not allow as cross join
	 * bit4: 0: out of SQL FROM clause, 1: within SQL FROM clause
	 * bit5: 0: not within analytic function, 1: within analytic function
	 * bit6: 0: not define an analytic function, 1: define an analytic function
	 * bit7: 0: not use macro object, 1: macro object used.
	 * bit8: 0: not within SQL WHERE clause, 1: within SQL WHERE clause
	 */
	int flag_;
	DictionarySP cachedCols_;
	SQLTransactionSP transSP_;
	vector<INDEX>* groups_;
};

class ColumnRef: public Object{
public:
	ColumnRef(const SQLContextSP& contextSP, const string& name): Object(COLUMN), contextSP_(contextSP),name_(name),index_(-1),acceptFunctionDef_(true){}
	ColumnRef(const SQLContextSP& contextSP, const string& qualifier, const string& name): Object(COLUMN), contextSP_(contextSP),
			qualifier_(qualifier),name_(name),index_(-1),acceptFunctionDef_(true){}
	ColumnRef(const SQLContextSP& contextSP, const string& name, int index): Object(COLUMN), contextSP_(contextSP),name_(name),index_(index),acceptFunctionDef_(true){}
	ColumnRef(const SQLContextSP& contextSP, const string& qualifier, const string& name, int index): Object(COLUMN), contextSP_(contextSP),
				qualifier_(qualifier),name_(name),index_(index),acceptFunctionDef_(true){}
	ColumnRef(const SQLContextSP& context, const DataInputStreamSP& in);
	virtual ~ColumnRef(){}
	virtual ObjectSP deepCopy() const;
	virtual ConstantSP getValue(Heap* pHeap);
	virtual ConstantSP getReference(Heap* pHeap);
	virtual ConstantSP getComponent() const;
	const SQLContextSP getSQLContext() const {return contextSP_;}
	const string& getQualifier() const { return qualifier_;}
	const string& getName() const { return name_;}
	string getFullname() const { return qualifier_.empty()? name_: qualifier_+"."+name_;}
	int getIndex() const { return index_;}
	DATA_TYPE getType() const;
	int getPartitionDimension() const;
	void setPartitionColumn(DATA_TYPE type, int dimensionIndex);
	void setAcceptFunctionDef(bool option) { acceptFunctionDef_ = option;}
	bool acceptFunctionDef() const { return acceptFunctionDef_;}
	virtual string getScript() const;
	string getNormalizedScript() const;
	virtual IO_ERR serialize(Heap* pHeap, const ByteArrayCodeBufferSP& buffer) const;
	ColumnRef* copy(const SQLContextSP& contextSP) const{ return new ColumnRef(contextSP, qualifier_, name_, index_);}
	ColumnRef* localize(const SQLContextSP& contextSP) const{ return new ColumnRef(contextSP, qualifier_, name_);}
	ColumnRef* localize() const{ return new ColumnRef(contextSP_, qualifier_, name_);}
	bool operator ==(const ColumnRef& target);
	bool operator ==(const ColumnRef& target) const;
	virtual ObjectSP copy(Heap* pHeap, const SQLContextSP& context, bool localize) const;
	virtual ObjectSP copyAndMaterialize(Heap* pHeap, const SQLContextSP& context, const TableSP& table) const;
	virtual bool mayContainColumnRefOrVariable() const { return true;}
	void bindColIndex();

private:
	SQLContextSP contextSP_;
	string qualifier_;
	string name_;
	int index_;
	bool acceptFunctionDef_;
    int colIndex_ = -1;
};

class Operator{
public:
	Operator(int priority, bool unary): priority_(priority), unary_(unary){}
	virtual ~Operator(){}
	inline int getPriority() const {return priority_;}
	inline bool isUnary() const {return unary_;}
	inline const string& getName() const {return funcDef_->getName();}
	inline string getTemplateSymbol() const { return templateSymbol_;}
	inline const FunctionDefSP& getFunctionDef() const { return funcDef_;}
	inline FastFunc getFastImplementation() const {return funcDef_->getFastImplementation();}
	virtual ConstantSP evaluate(Heap* heap, const ConstantSP& a, const ConstantSP& b) = 0;
	virtual string getOperatorSymbol() const  = 0;
	virtual bool isPrimitiveOperator() const = 0;
	virtual IO_ERR serialize(Heap* pHeap, const ByteArrayCodeBufferSP& buffer) const = 0;
	virtual void collectUserDefinedFunctions(unordered_map<string,FunctionDef*>& functionDefs) const = 0;

protected:
	FunctionDefSP funcDef_;
	string templateSymbol_;

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
	virtual ~Output(){};
	virtual void close() = 0;
	virtual OUTPUT_TYPE getOutputType() const = 0;
	virtual void setWindow(INDEX offset, INDEX size) = 0;
	virtual IO_ERR flush() = 0;
};

class Session {
public:
	Session(const HeapSP& heap);
	Session(const HeapSP& heap, bool systemSession);
	virtual ~Session(){}
	virtual SessionSP copy(bool forComputing = false) = 0;
	virtual bool run(const vector<string>& source, const string& currentPath = "", int firstLine = 0)=0;
	virtual bool run(const string& scriptFile)=0;
	virtual bool run(const ObjectSP& script)=0;
	virtual bool run(const ObjectSP& script, ConstantSP& result)=0;
	virtual bool run(const string& function, vector<ConstantSP>& params)=0;
	virtual bool run(const FunctionDefSP& function, vector<ConstantSP>& params)=0;
	virtual bool run(const vector<string>& variables, vector<ConstantSP>& params)=0;
	virtual bool test(const string& scriptFile, const string& resultFile, bool testMemoryLeaking)=0;
	virtual FunctionDefSP parseFunctionDef(const string& script) = 0;
	virtual vector<string> parseModuleStatement(const string& script) = 0;
	virtual vector<vector<string>> parseUseStatement(const string& script) = 0;
    virtual DictionarySP parseScript(const vector<string> &source, const string &currentPath = "", int firstLine = 0)= 0;
	virtual vector<FunctionDefSP> parseFunctionDefInModule(const string &moduleName) {
		throw RuntimeException("Session::parseFunctionDefInModule isn't implement yet.");
	}
	virtual bool contain(const string& key) const =0;
	virtual ConstantSP get(const string& key) const=0;
	virtual vector<pair<string,ConstantSP>> getAll() const=0;
	virtual void set(const string& key, const ConstantSP& value, bool copyIfDifferent = false)=0;
	virtual long long getLastActiveTime()=0;
	OutputSP getOutput() const { return out_;}
	void setOutput(const OutputSP& out) {out_ = out;}
	long long getSessionID() const {return sessionID_;}
	void setSessionID(long long sessionID){sessionID_=sessionID;}
	AuthenticatedUserSP getUser();
	AuthenticatedUserSP getUserAsIs();
	bool setUser(const AuthenticatedUserSP& user);
	const string& getRemoteSiteAlias() const { return remoteSiteAlias_;}
	void setRemoteSiteAlias(const string& alias){ remoteSiteAlias_ = alias;}
	int getRemoteSiteIndex() const { return remoteSiteIndex_;}
	void setRemoteSiteIndex(int siteIndex) { remoteSiteIndex_ = siteIndex;}
	const string& getRemoteIP() const { return remoteIP_;}
	void setRemoteIP(const string& ip){ remoteIP_ = ip;}
	int getRemotePort() const { return remotePort_;}
	void setRemotePort(int port) { remotePort_ = port;}
	HeapSP getHeap() const {return heap_;}
	bool isSystemSession() const {return flag_ & 1;}
	void setSystemSession(bool option) { if(option) flag_ |= 1; else flag_ &= ~1;}
	bool isWithinThreadCall() const { return flag_ & 2;}
	void setWithinThreadCall(bool option){ if(option) flag_ |= 2; else flag_ &= ~2;}
	bool isCancelled() const { return flag_ & 4;}
	void setCancelled(bool option){ if(option) flag_ |= 4; else flag_ &= ~4;}
	bool isAPIClient() const { return flag_ & 32;}
	void setAPIClient(bool option){ if(option) flag_ |= 32; else flag_ &= ~32;}
	int getFlag() const { return flag_;}
	bool setFlag(int flag);
	inline void setJobId(const Guid& jobId);
	inline const Guid& getJobId() const { return jobId_;}
	inline void setRootJobId(const Guid& rootJobId) { rootJobId_ = rootJobId;}
	inline const Guid& getRootJobId() const { return rootJobId_;}
	inline void setPriority(int priority) { priority_ = priority;}
	inline int getPriority() const { return priority_;}
	inline void setParallelism(int parallelism) { parallelism_ = parallelism;}
	inline int getParallelism() const { return parallelism_;}
	void setJob(const Guid& rootJobId, const Guid& jobId, int priority, int parallelism);
	virtual SysFunc getTableJoiner(const string& name) const = 0;
	virtual FunctionDefSP getFunctionDef(const string& name) const = 0;
	virtual FunctionDefSP getFunctionView(const string& name) const = 0;
	virtual TemplateOptr getTemplateOperator(const string& name) const = 0;
	virtual TemplateUserOptr getTemplateUserOperator(const string& name) const = 0;
	virtual OptrFunc getOperator(const string& optrSymbol, bool unary, const string& templateName) const = 0;
	virtual OperatorSP getOperator(const FunctionDefSP& func, bool unary) const = 0;
	virtual bool addPluginFunction(const FunctionDefSP& funcDef) = 0;
	virtual bool replaceFunction(const FunctionDefSP& funcDef) = 0;
	virtual void addFunctionDeclaration(FunctionDefSP& func) = 0;
	virtual void addFunctionalView(const FunctionDefSP& funcDef) = 0;
	virtual bool removeFunctionalView(const string& name) = 0;
	virtual void completePendingFunctions(bool commit) = 0;
	virtual void getFunctions(vector<pair<string,FunctionDefSP> >& functions) = 0;
	virtual void undefine(const string& objectName, OBJECT_TYPE objectType, void* objAddr = 0) = 0;
	virtual bool isEnum(const string& word, int& value) const = 0;
	virtual bool isDebugMode() const  = 0;
	virtual void setDebugContext(const DebugContextSP& context) = 0;
	virtual DebugContext* getDebugContext() const  = 0;
	virtual string getCurrentModule() const = 0;
	virtual void pushTemporaryObject(const ConstantSP& obj) = 0;
	virtual ConstantSP getTemoraryObject(int index) = 0;
	virtual void clearTemporaryObject() = 0;
	virtual string getLastErrorMessage() const = 0;
	virtual void* getPrivateKey() const = 0;
	virtual void setPrivateKey(void* key) = 0;
	inline bool getCompressionOption() const { return flag_ & 64;}
	inline void setCompressionOption(bool option){ if(option) flag_ |= 64; else flag_ &= ~64;}
	inline int getDepth() const { return (flag_ >> 8) & 7;}
	inline void setDepth(int depth) { flag_ = (flag_ & ~(7<<8)) | (depth << 8);}
	inline bool getEnableTransactionStatement() const { return flag_ & 2048; }
	inline void setEnableTransactionStatement(bool option) { if(option) flag_ |= 2048; else flag_ &= ~2048; }
	inline bool isStreamingEnv() const { return flag_ & 4096;}
	inline void setStreamingEnv(bool option) {if(option) flag_ |= 4096; else flag_ &= ~4096; }
	inline int getSQLStandard() const { return (flag_ >> 13) & 15;}
	inline void setSQLStandard(int sqlStandard) {flag_ = (flag_ & ~(15<<13)) | (sqlStandard << 13); }
	inline bool isTracing() { return flag_ & (1 << 18); }
	inline void setTracing(bool option) { if (option) flag_ |= (1 << 18); else flag_ &= ~(1 << 18); }
	inline TransactionSP getTransaction() { return transaction_; }
	inline void setTransaction(const TransactionSP& transaction) { transaction_ =  transaction; }
	inline long long getAsyncReplicationTaskId() { return asyncReplicationTaskId_; }
	inline void setAsyncReplicationTaskId(long long taskId) { asyncReplicationTaskId_ = taskId; }
	inline const Guid& getClientId() const { return clientId_;}
	inline void setClientId(const Guid& clientId) { clientId_ = clientId;}
	inline long long getSeqNo() const { return seqNo_;}
	inline void setSeqNo(long long seqNo) { seqNo_ = seqNo;}

protected:
	long long sessionID_;
	HeapSP heap_;
	OutputSP out_;
	AuthenticatedUserSP user_;
	string remoteIP_;
	string remoteSiteAlias_;
	int remotePort_;
	int remoteSiteIndex_;
	Guid rootJobId_;
	Guid jobId_;
	int priority_;
	int parallelism_;
	int flag_;
	TransactionSP transaction_;
	// for async replication task execution
	long long asyncReplicationTaskId_ = 0;

private:
    Guid clientId_;
    long long seqNo_;
    Mutex userMutex_;
};

class Transaction {
public:
	virtual TABLE_TYPE getTableType() = 0;
	virtual void endTransaction(TransactionSP transaction) = 0;
	virtual void commit() = 0;
	virtual void abort() = 0;
	virtual bool needRetry() = 0;
	virtual uint64_t getTxnId() const = 0;
	virtual ~Transaction() {}
};

class Console {
public:
	Console(const SessionSP& session, const OutputSP& out);
	virtual ~Console(){}
	virtual void cancel(bool running){}
	Output* getOutput(){return out_.get();}
	SessionSP getSession(){return session_;}
	long long getLastActiveTime() const { return lastActiveTime_;}
	void setLastActiveTime(long long lastUpdate) { lastActiveTime_ = lastUpdate;}
	inline void setJobId(const Guid& jobId);
	inline const Guid& getJobId() const { return jobId_;}
	inline const Guid& getTaskId() const { return jobId_;}
	inline void setRootJobId(const Guid& rootJobId) { rootJobId_ = rootJobId;}
	inline const Guid& getRootJobId() const { return rootJobId_;}
	inline void setPriority(int priority) { priority_ = priority;}
	inline int getPriority() const { return priority_;}
	inline void setParallelism(int parallelism) { parallelism_ = parallelism;}
	inline int getParallelism() const { return parallelism_;}
	inline bool isCancellable() const {return cancellable_;}
	inline void setCancellable(bool option){cancellable_ = option;}
	void set(const Guid& rootJobId, const Guid& jobId, int parallelism, int priority);
	inline void setFlag(long long flag) { flag_ = flag;}
	inline long long getFlag() const { return flag_;}
	inline bool isUrgent() const { return flag_ & 1;}
	inline bool isSecondaryJob() const { return flag_ & 2;}
	inline bool hasAsynTask() const { return flag_ & 4;}
	inline bool isPickle() const { return flag_ & 8;}
	inline bool isStateless() const { return flag_ & 16;}
	inline bool isAPIClient() const { return flag_ & 32;}
	inline bool compressOutput() const { return flag_ & 64;}
	inline bool isInfra() const { return flag_ & 128;}
	inline int getDepth() const { return (flag_ >> 8) & 7;}
	inline void setDepth(int depth) { flag_ = (flag_ & ~(7<<8)) | (depth << 8);}
    inline bool isTracing() const { return flag_ & 65536;}
    inline void setTracing() { flag_ |= 65536;}
    inline void setNonTracing() { flag_ &= ~65536;}
	inline int getSQLStandard() const { return (flag_ >> 19) & 15;}
	inline void setSQLStandard(int sqlStandard) {flag_ = (flag_ & ~(15<<19)) | (sqlStandard << 19); }
    inline void setParentSpanId(const Guid &spanId) { parentSpanId_ = spanId; }
    inline Guid &getParentSpanId() { return parentSpanId_; }
	virtual IO_ERR readReady()=0;
	virtual IO_ERR execute()=0;
	virtual CONSOLE_TYPE getConsoleType() const = 0;
	virtual void run() = 0;
	virtual void getTaskDesc(string& type, string& desc) const = 0;
	inline bool pickleTableList() const { return flag_ & 32768;}
protected:
	Guid rootJobId_;
	Guid jobId_;
	int priority_;
	int parallelism_;
	bool cancellable_;
	SessionSP session_;
	OutputSP out_;
	long long lastActiveTime_;
	/**
	 * bit0: 0: normal request, 1:urgent request
	 * bit1: 0: normal job, 1:secondary job
	 * bit2: 0: sync request, 1: async request
	 * bit3: 0: inhouse protocol, 1: pickle protocol
	 * bit4: 0: state, 1:stateless (clear session variables upon completion of request)
	 * bit5: 0: normal, 1:optimize for api client
	 * bit6: 0: no compression, 1: compress the output
	 * bit7: 0: not for infra, 1: for infra
	 * bit8~10 the depth of the task
	 * bit15: 0: pickle table to dataFrame    1: pickle table to list
	 * bit16: 0: disable tracing, 1: enable tracing
	 * bit17: 0: api, 1: stream subscription
	 * bit18: reserved for output format
	 * bit19~22: sql standard, 0: dolphindb, 1: oracle, 2: mysql
	 */
	long long flag_;
    Guid parentSpanId_;
};

class ConstantMarshal {
public:
	virtual ~ConstantMarshal(){}
	virtual bool start(const ConstantSP& target, bool blocking, IO_ERR& ret)=0;
	virtual bool start(const char* requestHeader, size_t headerSize, const ConstantSP& target, bool blocking, IO_ERR& ret)=0;
	virtual bool resume(IO_ERR& ret)=0;
	virtual void reset() = 0;
	virtual IO_ERR flush() = 0;
};

class ConstantUnmarshal{
public:
	virtual ~ConstantUnmarshal(){}
	virtual bool start(short flag, bool blocking, IO_ERR& ret)=0;
	virtual bool resume(IO_ERR& ret)=0;
	virtual void reset() = 0;
	ConstantSP getConstant(){return obj_;}

protected:
	ConstantSP obj_;
};
class Heap{
public:
	Heap():meta_(0), session_(0), size_(0), status_(0){}
	Heap(Session* session);
	Heap(int size, Session* session);
	~Heap();

	ConstantSP getValue(int index) const;
	ConstantSP getReference(int index) const;
	ConstantSP getReference(const string& name) const;
    void getReference(INDEX index, Constant*& ptr, ConstantSP& ref);
    inline int size() const {return size_;}
	inline bool isViewMode() const { return status_ & 1;}
	inline void setViewMode(bool enabled = true) { if(enabled) status_ |= 1; else status_ &= ~1;}
	inline bool isDefMode() const { return status_ & 2;}
	inline void setDefMode() { status_ |= 2; }
	int getIndex(const string& name) const;
	int getLocalIndex(const string& name) const;
	string getName(int index) const;
	bool contains(const string& name) const;
	int addItem(const string& name, const ConstantSP& value){ return addItem(name, value, false);}
	int addItem(const string& name, const ConstantSP& value, bool constant);
	void removeItem(const string& name);
	void removeAllItems();
	bool set(unsigned int index,const ConstantSP& value, bool constant);
	bool set(unsigned int index,const ConstantSP& value);
	void setConstant(int index, bool constant);
	inline bool isConstant(int index) const {return index>= MAX_SHARED_OBJ_INDEX && (flags_[index - MAX_SHARED_OBJ_INDEX] & 1);}
	bool isInitialized(int index) const;
	inline bool isMetaInitalized() const { return meta_ != nullptr;}
	bool isSameObject(int index, Constant* obj) const;
	void rollback();
	void reset();
	void clearFlags();
	void currentSession(Session* session){session_=session;}
	Session* currentSession(){return session_;}
	long long getAllocatedMemory();
	/**
	 * getLocalVariable and setLocalVariable can only be used in very limited environment for optimization purpose.
	 */
	inline ConstantSP getLocalVariable(int index) const { return values_[index];}
	inline void setLocalVariable(int index, const ConstantSP& value){
		values_[index] = value;
		flags_[index] = 2;
        if(index != 0 && value->isTemporary())
            value->setTemporary(false);
	}
	bool copyMeta(Heap *heap);
	bool setMetaName(const string &name, int index);

	/** Do NOT call this method! */
	void initMetaWithDummyItemInUdfContext();

private:
	struct HeapMeta{
		Mutex mutex_;
		unordered_map<string,int> nameHash_;
		vector<string> names_;
	};
	HeapMeta* meta_;
	vector<ConstantSP> values_;
	vector<char> flags_;
	Session* session_;
	int size_;
	char status_;
};

struct JITCfgNode {
	JITCfgNode() : visited_(false){}
	string getInferredTypeCacheAsString() const {
		string out = "{";
			for (auto kv: inferredTypeCache) {
				out.append(kv.first + ": " +kv.second.getString() + ", ");
			}
		out.append("}");
		return out;
	}

	// control flow graph: next block edge
	vector<StatementSP> cfgNexts;
	// control flow graph: reverse edge to incoming block
	vector<StatementSP> cfgFroms;
	unordered_map<string, InferredType> inferredTypeCache;
	unordered_map<string, vector<InferredType>> upstreamTypes;
	bool visited_;
};

class StatementContext {
public:
	StatementContext() : status_(0){}
	inline bool shouldReturn() const { return status_ & 1;}
	inline bool shouldBreak() const { return status_ & 2;}
	inline bool shouldContinue() const { return status_ & 4;}
	inline bool shouldBreakOrContinue() const { return status_ & 6;}
	inline bool shouldBreakOrReturn() const { return status_ & 3;}
	inline bool shouldStop() const { return status_ & 7;}
	inline void setReturn() { status_ |= 1;}
	inline void setBreak() { status_ |= 2;}
	inline void setContinue() { status_ |= 4;}
	inline void reset() { status_ = 0;}
	inline void resetBreakContinue() { status_ &= 1;}

private:
	int status_;
};

class Statement{
public:
	Statement(STATEMENT_TYPE type):breakpoint_(nullptr), jitudfHeader_(nullptr), type_(type), line_(0), moduleName_(""){}
	virtual ~Statement(){}
	virtual StatementSP clone() = 0;
	STATEMENT_TYPE getType() const {return type_;}
	virtual void execute(Heap* pHeap, StatementContext& context)=0;
	virtual void execute(Heap* pHeap, StatementContext& context, DebugContext* debugContext);
	virtual string getScript(int indention) const = 0;
	virtual IO_ERR serialize(Heap* pHeap, const ByteArrayCodeBufferSP& buffer) const = 0;
	virtual void collectUserDefinedFunctions(unordered_map<string,FunctionDef*>& functionDefs) const {}
	void disableBreakpoint();
	void enableBreakpoint();
	void setBreakpoint(std::atomic<bool>* breakpoint) { breakpoint_ = breakpoint;}
	bool getBreakpoint() { return nullptr != breakpoint_ ? breakpoint_->load() : false; }

    JITCfgNodeSP getCFGNode() const { return cfgNode_; }
    void setCFGNode(const JITCfgNodeSP& cfg) { cfgNode_ = cfg; }
    Statement* getJITUDFHeader() const { return jitudfHeader_;}

    vector<StatementSP>& getCFGNexts();
	vector<StatementSP>& getCFGFroms();
	unordered_map<string, vector<InferredType>> & getUpstreamTypes() { return cfgNode_->upstreamTypes; }
	unordered_map<string, InferredType> & getInferredTypeCache();
	void addCFGNextBlock(const StatementSP& nextBlock);
	void addCFGFromBlock(const StatementSP& fromBlock);
	bool getCFGNodeVisited() const;
	bool setCFGNodeVisited(bool visited = true);
	string getInferredTypeCacheAsString() const;
	virtual IO_ERR buildCFG(const StatementSP& self, std::unordered_map<string, StatementSP> & context);
	virtual string getInferredTypesDebugString(int indention) const;
	virtual void traverseCFG(const StatementSP& self, unordered_set<void*> & visited, CFGTraversalFunc func);
	virtual vector<string> getVarNames() const;
	virtual void setJITUDFHeader(Statement* header);
    void cleanInferredType();
	void setLine(int line) { line_ = line;}
	int getLine() { return line_;}
    void setModuleName(string fileName) { moduleName_ = fileName; }
    string getModuleName() { return moduleName_; }

protected:
	std::atomic<bool>* breakpoint_;
	JITCfgNodeSP cfgNode_;
	Statement* jitudfHeader_;

private:
	STATEMENT_TYPE type_;
	int line_;
	string moduleName_;
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
	DomainSite(const string& host, int port, int index, const string& alias, const string & computeGroupName,
		const string& zoneName)
			: host_(host), port_(port), index_(index), alias_(alias), computeGroupName_(computeGroupName), zoneName_(zoneName) {}
	const string& getHost() const {return host_;}
	int getPort() const {return port_;}
	int getIndex() const {return index_;}
	string getString() const;
	const string& getAlias() const {return alias_;}
	const string& getComputeGroupName() const { return computeGroupName_; }
	void setComputeGroupName(const string& name) { computeGroupName_ = name; }
	const string& getZoneName() const { return zoneName_; }
	void setZoneName(const string& zoneName) { zoneName_ = zoneName; }
	inline bool operator ==(const DomainSite& target) const { return  host_ == target.host_ && port_ == target.port_;}
	static DomainSite emptySite_;
private:
	string host_;
	int port_;
	int index_;
	string alias_;
	// Empty string for datanodes and compute nodes that do not have a group
	// For compute nodes that do not have a group, all computations are pushed down to the data nodes
	string computeGroupName_;
	string zoneName_;
};

class DomainSitePool {
public:
	DomainSitePool() : lastSuccessfulSiteIndex_(-1), lastSiteIndex_(-1), nextSiteIndex_(-1), startSiteIndex_(-1), localSiteIndex_(-1){}
	DomainSitePool(const DomainPartitionSP& partition);
	void setDisabled(int index, bool v);
	void addSite(int siteIndex);
	int getLastSite() const;
	inline int getSiteCount() const { return sites_.size();}
	int getSite(int index) const;
	int nextSite() const;
	inline int getNextSite() const { return sites_[nextSiteIndex_].first;}
	inline bool hasNextSite() const { return nextSiteIndex_ >= 0;}
	inline bool containLocalSite() const { return localSiteIndex_ >= 0;}
	void initiateSite(bool useLastSuccessfulSite = false);
	void setIntialSite(int index);
	void resetInitialSite();
	void setLastSuccessfulSite();
	void markUsed(int siteIndex);
	inline bool isDisabled(int index) { return disabled_[index]; }

	void disableAllComputeNode();
    string getScript() const;
private:
	vector<pair<int, bool>> sites_;
	vector<bool> disabled_;
	int lastSuccessfulSiteIndex_;
	mutable int lastSiteIndex_;
	mutable int nextSiteIndex_;
	int startSiteIndex_;
	int localSiteIndex_;
};

class DomainPartition{
public:
	DomainPartition(int key, const string& path) : key_(key), version_(0), cid_(-1), path_(path), id_(false), tables_(0){}
	DomainPartition(int key, const string& path, const Guid& id, int version, long long cid) : key_(key), version_(version),cid_(cid), path_(path), id_(id), tables_(0){}
	DomainPartition(int key, const DFSChunkMetaSP& chunkMeta) : key_(key), version_(chunkMeta->getVersion()), cid_(chunkMeta->getCommitId()),
			path_(chunkMeta->getPath()), id_(chunkMeta->getId()), tables_(0){}
	DomainPartition(Session* session, const DataInputStreamSP& in);
	virtual ~DomainPartition();
	const int getKey() const {return key_;}
	virtual bool addSite(int siteIndex) {return false;}
	virtual int getSiteCount() const { return 0;}
	virtual int getSiteIndex(int index) const { throw RuntimeException("DomainPartition::getSiteIndex method not supported.");}
	virtual bool isLocalPartition() const { return true;}
	virtual bool containLocalCopy() const { return true;}
	virtual bool getPrefetchComputeNodeData()  const { return true; }
	string getString() const;
	inline const string& getPath() const {return path_;}
	inline const Guid& getId() const {return id_;}
	inline int getVersion() const {return version_;}
	int size(const string& table) const;
	void setSize(const string& table, INDEX size);
	inline long long getCommitId() const { return cid_;}
	inline void setCommitId(long long cid) { cid_ = cid;}
	inline bool operator ==(const DomainPartition& target){ return  (key_ == target.key_) && (id_ == target.id_);}
	IO_ERR serialize(Heap* pHeap, const ByteArrayCodeBufferSP& buffer) const;
	static string processPartitionId(const string& id, bool removeSpecialChar = true);
	static ConstantSP parsePartitionId(const string& id, DATA_TYPE type);

private:
	int key_;
	int version_;
	long long cid_;
	string path_;
	Guid id_;

	struct TableSize {
		string name_;
		int size_;
		TableSize* next_;
	};
	TableSize* tables_;
};

struct ClusterNodes {
	const string controllerSite;
	const string controllerAlias;
	const int controllerSiteIndex;
	const vector<int> controllerSiteIndexPool;
	const unordered_map<int, DomainSite> sites;
	const unordered_map<string, int> sitesMap;
    SmartPointer<unordered_map<string, SERVER_TYPE>>  sitesTypeMap;

	ClusterNodes(const string& ctrlSite, const string& ctrlAlias) : controllerSite(ctrlSite), controllerAlias(ctrlAlias), controllerSiteIndex(-1), sitesTypeMap( new unordered_map<string, SERVER_TYPE>()){}

	ClusterNodes(const string& ctrlSite, const string& ctrlAlias, int ctrlSiteIndex, const vector<int>& ctrlSiteIndexPool,
			const unordered_map<int, DomainSite>& nodes, const unordered_map<string, int>& nodesMap, const SmartPointer<unordered_map<string, SERVER_TYPE>>& nodesTypeMap) : controllerSite(ctrlSite), controllerAlias(ctrlAlias),
			controllerSiteIndex(ctrlSiteIndex), controllerSiteIndexPool(ctrlSiteIndexPool), sites(nodes), sitesMap(nodesMap), sitesTypeMap(nodesTypeMap){
	}

	void getDataNodeAliases(vector<string>& aliases, bool includeComputeNode = true);
	void getDataNodeIndices(vector<int>& indices);
    void updateSiteType(const SmartPointer<unordered_map<string, SERVER_TYPE>>& map) {sitesTypeMap = map;}

	inline int getSiteIndex(const string& alias) const {
		unordered_map<string, int>::const_iterator it = sitesMap.find(alias);
		if(it == sitesMap.end())
			return -1;
		else
			return sites.find(it->second)->second.getIndex();
	}
	inline int getSiteIndex(const string& host, int port) const {
		unordered_map<string, int>::const_iterator it = sitesMap.find(host + ":" + std::to_string(port));
		if(it == sitesMap.end())
			return -1;
		else
			return sites.find(it->second)->second.getIndex();
	}
	inline const DomainSite& getSite(const string& alias) const {
		unordered_map<string, int>::const_iterator it = sitesMap.find(alias);
		if(it == sitesMap.end())
			return DomainSite::emptySite_;
		else
			return sites.find(it->second)->second;
	}
	inline const DomainSite& getSite(const string& host, int port) const {
		unordered_map<string, int>::const_iterator it = sitesMap.find(host + ":" + std::to_string(port));
		if(it == sitesMap.end())
			return DomainSite::emptySite_;
		else
			return sites.find(it->second)->second;
	}
	const DomainSite& getSite(int siteIndex) const {
		auto it = sites.find(siteIndex);
		if (it == sites.end()) {
			return DomainSite::emptySite_;
		}
		return it->second;
	}

	inline bool isController(int siteIndex) const {
		for(int index : controllerSiteIndexPool){
			if(index == siteIndex)
				return true;
		}
		return false;
	}
};

class ColumnDesc {
public:
	ColumnDesc(const string& name, DATA_TYPE type, int extra, const string& comment = "") : name_(name), comment_(comment), type_(type), extra_(extra){}
	const string& getName() const {return name_;}
	DATA_TYPE getType() const {return type_;}
	int getExtra() const {return extra_;}
	int getCompressionMethod(int defaultMethod) const;
	const string& getComment() const { return comment_;}
	void setComment(const string& comment) { comment_ = comment;}

private:
	string name_;
	string comment_;
	DATA_TYPE type_;
	int extra_;
};

struct SensitiveColumn {
	SensitiveColumn(){}
	SensitiveColumn(int columnIdx, const FunctionDefSP& func = nullptr): columnIdx(columnIdx), sensitiveFunc(func) {}
	int columnIdx = -1;
	FunctionDefSP sensitiveFunc = nullptr;
};

struct TableHeader {
	TableHeader() = default;
	TableHeader(const string& owner, const string& physicalIndex, const vector<ColumnDesc>& tablesType, const vector<int>& partitionKeys, CIPHER_MODE mo = PLAIN_TEXT, const int64_t keyVer = 0, const string& encryptKey = ""): owner(owner),
		physicalIndex(physicalIndex), colDescs(tablesType), partitionKeys(partitionKeys), mode(mo),keyVersion(keyVer), encryptedTableKey(encryptKey) {}
	TableHeader(const string& owner, const string& physicalIndex, const vector<ColumnDesc>& tablesType,
			const vector<int>& partitionKeys, const vector<pair<int, bool>>& sortKeys,
			DUPLICATE_POLICY rowDuplicatePolicy, const vector<pair<int, FunctionDefSP>>& sortKeyMappingFunction,
			bool appendForDelete, const string& tableComment,
			bool latestKeyCache, bool compressHashSortKey,
			CIPHER_MODE mo = PLAIN_TEXT, const int64_t keyVer = 0, const string& encryptKey = "", const vector<SensitiveColumn>& scols = {}):
			rowDuplicatePolicy(rowDuplicatePolicy), owner(owner), physicalIndex(physicalIndex),
			colDescs(tablesType), partitionKeys(partitionKeys), sortKeys(sortKeys),
			sortKeyMappingFunction(sortKeyMappingFunction), appendForDelete(appendForDelete), tableComment(tableComment),
			latestKeyCache(latestKeyCache), compressHashSortKey(compressHashSortKey),
			mode(mo), keyVersion(keyVer),encryptedTableKey(encryptKey), sensitiveCol(scols) {}
	DUPLICATE_POLICY rowDuplicatePolicy = DUPLICATE_POLICY::KEEP_ALL;
	string owner;
	string physicalIndex;
	vector<ColumnDesc> colDescs;
	vector<int> partitionKeys;
	vector<pair<int, bool>> sortKeys;
	vector<pair<int, FunctionDefSP>> sortKeyMappingFunction;
	bool appendForDelete = false;
	string tableComment;

	/**
	 * The following two fields are used by the compute node to verify
	 * whether the cached data has become invalid due to DDL:
	 * - `chunkId`: used to detect table creation and deletion.
	 * - `chunkCid`: used to detect schema changes.
	 */
	Guid chunkId;
	long long chunkCid;

	bool latestKeyCache = false;
	bool compressHashSortKey = false;
	CIPHER_MODE mode = CIPHER_MODE::PLAIN_TEXT;
	int64_t keyVersion;
	string encryptedTableKey;

	vector<SensitiveColumn> sensitiveCol;
};

class Domain{
public:
	Domain(const string& owner, PARTITION_TYPE partitionType, bool isLocalDomain, DBENGINE_TYPE engineType = DBENGINE_TYPE::OLAP, ATOMIC atomic = ATOMIC::TRANS, int flag = 0) : partitionType_(partitionType), isLocalDomain_(isLocalDomain), isExpired_(false),
			tableIndependentChunk_(false), retentionPeriod_(-1), retentionDimension_(-1), hoursToColdVolume_(-1), tzOffset_(INT_MIN), key_(false), owner_(owner), engineType_(engineType), atomic_(atomic), flag_(flag){}
	Domain(const string& owner, PARTITION_TYPE partitionType, bool isLocalDomain, const Guid& key, DBENGINE_TYPE engineType = DBENGINE_TYPE::OLAP, ATOMIC atomic = ATOMIC::TRANS, int flag = 0) : partitionType_(partitionType), isLocalDomain_(isLocalDomain),
			isExpired_(false), tableIndependentChunk_(false), retentionPeriod_(-1), retentionDimension_(-1),  hoursToColdVolume_(-1), tzOffset_(INT_MIN), key_(key), owner_(owner), engineType_(engineType), atomic_(atomic), flag_(flag){}
	virtual ~Domain(){}
	int getPartitionCount() const { return partitions_.size();}
	DomainPartitionSP getPartition(int index) const { return partitions_[index];}
	PARTITION_TYPE getPartitionType() const { return partitionType_;}
	bool isLocalDomain() const {return isLocalDomain_;}
	const Guid& getKey() const { return key_;}
	void setKey(const Guid& key) { key_ = key;}
	const string& getName() const { return name_;}
	void setName(const string& name) { name_ = name;}
	const string& getDatabaseDir()const {return dir_;}
	void setDatabaseDir(const string& dir) { dir_ = dir;}
	SymbolBaseManagerSP getSymbolBaseManager() const {return symbaseManager_;}
	void setSymbolBaseManager(const SymbolBaseManagerSP& symbaseManager){ symbaseManager_ = symbaseManager;}
	bool isDFSMode() const;
	virtual ConstantSP getPartitionSites() const { return formatSites(partitions_);}
	virtual ConstantSP getPartitionSchema() const = 0;
	virtual void retrievePartitionsInRange(const ConstantSP& start, bool startInclusive, const ConstantSP& end, bool endInclusive, vector<DomainPartitionSP>& partitions, bool localOnly) const = 0;
	virtual void retrievePartitionsIn(const ConstantSP& values, vector<DomainPartitionSP>& partitions, bool localOnly) const = 0;
	virtual void retrievePartitionAt(const ConstantSP& value, vector<DomainPartitionSP>& partitions, bool localOnly) const = 0;
	virtual void retrieveAllPartitions(vector<DomainPartitionSP>& partitions, bool localOnly) const;
	virtual IO_ERR saveDomain(const string& filename) const;
	virtual IO_ERR saveDomain(const DataOutputStreamSP& out) const = 0;
	virtual ConstantSP getPartitionKey(const ConstantSP& partitionColumn) const = 0;
	virtual bool equals(const DomainSP& domain) const = 0;
	virtual DATA_TYPE getPartitionColumnType(int dimIndex = 0) const = 0;
	virtual int getPartitionDimensions() const { return 1;}
	virtual DomainSP getDimensionalDomain(int dimension) const;
	virtual DomainSP copy() const = 0;
	bool addTable(const string& tableName, const string& owner, const string& physicalIndex, vector<ColumnDesc>& cols, vector<int>& partitionColumns);
	bool addTable(const string& tableName, const TableHeader& tableHeader);
	bool getTable(const string& tableName, string& owner, string& physicalIndex, vector<ColumnDesc>& cols, vector<int>& partitionColumns) const;
	bool getTable(const string& tableName, TableHeader& tableHeader) const;
	string getTabletPhysicalIndex(const string& tableName);
	bool existsTable(const string& tableName);
	bool removeTable(const string& tableName);
	bool listTables(vector<string>& tableNames);
	void loadTables(const string& dir);
	void setExpired(bool option) { isExpired_ = option;}
	inline bool isExpired() const { return isExpired_;}
	inline int getRentionPeriod() const { return retentionPeriod_;}
	inline int getRentionDimension() const { return retentionDimension_;}
    inline int getHoursToColdVolume() const {return hoursToColdVolume_;}
	inline int getTimeZoneOffset() const { return tzOffset_;}
	inline int getFlag() const { return flag_; }
	void setRentionPeriod(int retentionPeriod, int retentionDimension, int tzOffset, int hoursToColdVolume);
	string getOwner() const { return owner_;}
	void setOwner(const string& owner) { owner_ = owner; }
	bool isOwner(const string& owner) const { return owner == owner_;}
	void setEngineType(DBENGINE_TYPE type) { engineType_ = type;}
	DBENGINE_TYPE getEngineType() const { return engineType_;}
	void setAtomicLevel(ATOMIC atomicLevel) { atomic_ = atomicLevel;}
	ATOMIC getAtomicLevel() const { return atomic_;}
	void setSingleTablet(bool option = true){ if(option) flag_ |= 1; else flag_ &= ~1;}
    bool isSingleTableTablet() const {return flag_ & 1;}
	bool enableReplication() const { return flag_ & (1 << 1); }
	void setReplication(bool enable) { if (enable) flag_ |= ((unsigned long)1) << 1; else flag_ &= ~(((unsigned long)1) << 1); }
	bool isRemoveSpecialChar() const { return flag_ & (1 << 2); }
	void setRemoveSpecialChar(bool enable) { if (enable) flag_ |= ((unsigned long)1) << 2; else flag_ &= ~(((unsigned long)1) << 2); }
	bool enableIOTDB() const { return flag_ & (1 << 3); }
	void setIOTDB(bool enable) { if (enable) flag_ |= ((unsigned long)1) << 3; else flag_ &= ~(((unsigned long)1) << 3); }
	void setTableIndependentChunk(bool option) { tableIndependentChunk_ = option;}
	bool isTableIndependentChunk() const { return tableIndependentChunk_;}
    static string getUniqueIndex(long long id);
	/*
	 * The input arguments set1 and set2 must be sorted by the key value of domain partitions. The ranking of key values must be the same as
	 * the ranking of partitioning column values when the partition is range-based or value-based.
	 */
	static void set_interaction(const vector<DomainPartitionSP>& set1, const vector<DomainPartitionSP>& set2, vector<DomainPartitionSP>& result);
	static void set_union(const vector<DomainPartitionSP>& set1, const vector<DomainPartitionSP>& set2, vector<DomainPartitionSP>& result);
	static DomainSP loadDomain(const string& domainFile);
	static DomainSP loadDomain(const DataInputStreamSP& in, const string& dbName = "");
	static DomainSP createDomain(const string& owner, PARTITION_TYPE  partitionType, const ConstantSP& scheme);
	static DomainSP createDomain(const string& owner, PARTITION_TYPE  partitionType, const ConstantSP& scheme, const ConstantSP& sites);

    static int codeDimension(int d, int h){
        return (h<<8) + (d&0xff);
    }
    static void decodeDimension(int c, int& d, int& h){
        d = c&0xff;
        h = c>>8;
    }
protected:
	static VectorSP parseSites(const ConstantSP& sites);
	static ConstantSP formatSites(const vector<DomainPartitionSP>& partitions);
	static bool addSiteToPartitions(const vector<DomainPartitionSP>& partitions, const VectorSP& sites);
	static ConstantSP temporalConvert(const ConstantSP& obj, DATA_TYPE targetType);

protected:
	vector<DomainPartitionSP> partitions_;
	PARTITION_TYPE partitionType_;
	bool isLocalDomain_;
	bool isExpired_;
	bool tableIndependentChunk_;
	int retentionPeriod_; // in hours
	int retentionDimension_;
    int hoursToColdVolume_;
	int tzOffset_;
	Guid key_; //the unique identity for this domain
	string name_;
	string dir_;
	string owner_;
	DBENGINE_TYPE engineType_;
	ATOMIC atomic_;
	/*
		bit0: isSingleTablet
		bit1: enableReplication
		bit2: removeSpecialChar
		bit3: isIOTDB
	*/
    int flag_;
	SymbolBaseManagerSP symbaseManager_;
	unordered_map<string, TableHeader> tables_;
	mutable Mutex mutex_;
};

class PartitionGuard {
public:
	PartitionGuard(const TableSP& table);
	PartitionGuard(){}
	~PartitionGuard();
	void setPartition(const TableSP& table);
private:
	TableSP table_;
};

struct TableUpdate{
	TableUpdate(const string& topic, long long offset, int length, int flag, const TableSP& table, const TableSP& logTable) : topic_(topic), offset_(offset), length_(length), flag_(flag), filter_(0), table_(table), logTable_(logTable){}
	TableUpdate(const string& topic, long long offset, int length, int flag, const ObjectSP& filter, const TableSP& table, const TableSP& logTable) : topic_(topic), offset_(offset), length_(length), flag_(flag), filter_(filter), table_(table), logTable_(logTable){}
	TableUpdate() : offset_(0), length_(0), flag_(0), filter_(0), table_(0), logTable_(0) {}
	string topic_;
	long long offset_;
	int length_;
	int flag_;
	ObjectSP filter_;
	TableSP table_;
	// for multicast publish, logTable_ is always nullptr and use table_ for LogRowTable
	TableSP logTable_;
};

struct TableUpdateSizer {
	inline int operator()(const TableUpdate& update){
		return update.length_;
	}
};

struct TableUpdateUrgency {
	inline bool operator()(const TableUpdate& update){
		return update.flag_ & 1;
	}
};

struct TopicSubscribe {
	TopicSubscribe(const string& topic, int hashValue, vector<string> attributes, const FunctionDefSP& handler, const AuthenticatedUserSP& user,
			bool msgAsTable, int batchSize, int throttleTime, bool persistOffset, bool timeTrigger, bool handlerNeedMsgId,
			const string& userId = "", const string& pwd = "", long long sessionID = 0) : msgAsTable_(msgAsTable),
			persistOffset_(persistOffset), timeTrigger_(timeTrigger), handlerNeedMsgId_(handlerNeedMsgId), hashValue_(hashValue), batchSize_(batchSize),
			throttleTime_(throttleTime), userId_(userId), pwd_(pwd), sessionID_(sessionID), cumSize_(0), messageId_(-1), expired_(-1), topic_(topic), attributes_(attributes), handler_(handler), user_(user){}
	bool append(long long msgId, const ConstantSP& msg, long long& outMsgId, ConstantSP& outMsg);
	bool getMessage(long long now, long long& outMsgId, ConstantSP& outMsg);
	bool updateSchema(const TableSP& emptyTable);
	bool isUnsubscribed() { return isUnsubscribed_; }
	void setUnsubscribed() { isUnsubscribed_ = true; }
    void setSubscribed() { isUnsubscribed_ = false; }

	const bool msgAsTable_;
	const bool persistOffset_;
	/*
	 * trigger the message handler as long as a fixed time period (specified in throttleTime_) elapses
	 * even if there is no incoming message in the time window when timeTrigger_ is set to true.
	 */
	const bool timeTrigger_;
	/*
	 * if this value is true, the handler accepts two arguments, message body and message id.
	 * Otherwise, the handler accepts only one argument, i.e. message body.
	 */
	const bool handlerNeedMsgId_;
	const int hashValue_;
	const int batchSize_;
	const int throttleTime_; //in millisecond
	const string userId_;
	const string pwd_;
    const long long sessionID_;
	int cumSize_;
	std::atomic<long long> messageId_;
	long long expired_;
	const string topic_;
	vector<string> attributes_;
	const FunctionDefSP handler_;
	AuthenticatedUserSP user_;
	ConstantSP body_;
	ConstantSP filter_;
	Mutex mutex_;
	bool isUnsubscribed_ = false;
};

class SessionThreadCallGuard {
public:
	SessionThreadCallGuard() : session_(0){}
	SessionThreadCallGuard(Session* session);
	~SessionThreadCallGuard();
	void setThreadCallMode(Session* session);
	void releaseThreadCallMode();

private:
	Session* session_;
};

class ReducerContainer {
public:
	ReducerContainer(Heap* heap, const FunctionDefSP& reducer) : heap_(heap), reducer_(reducer), objCount_(0){}
	bool addObject(const ConstantSP& obj, string& errMsg);
	ConstantSP getResult() const {return result_;}
	int getObjectCount() const {return objCount_;}
	void reset();

private:
	Heap* heap_;
	FunctionDefSP reducer_;
	ConstantSP result_;
	int objCount_;
	Mutex mutex_;
};

class DistributedCall{
public:
	DistributedCall(const ObjectSP& obj, bool local);
	DistributedCall(const ObjectSP& obj, const ReducerContainerSP& reducer, bool local);
	DistributedCall(const ObjectSP& obj, std::function<void (bool, const ConstantSP&)> callback, bool local);
	DistributedCall(const CountDownLatchSP latch, const ObjectSP& obj, bool local);
	DistributedCall(const Guid& jobId, const CountDownLatchSP latch, const ObjectSP& obj, bool local);
	DistributedCall(const Guid& jobId, const CountDownLatchSP latch, const ObjectSP& obj, const ReducerContainerSP& reducer, bool local);
	virtual ~DistributedCall(){}
	virtual void cancel(bool running);
	void getTaskDesc(string& type, string& desc);
	inline void start(){ startTime_ = std::chrono::system_clock::now();}
	inline const ObjectSP& getObject() const { return obj_;}
	inline void setObject(const ObjectSP& obj){obj_ = obj;}
	inline void clearObject(){obj_.clear();}
	inline Heap* getHeap() const { return heap_;}
	inline SessionSP getSession() const {return session_;}
	inline CountDownLatchSP getCountDownLatch() const { return latch_;}
	inline void setCountDownLatch(const CountDownLatchSP& latch){ latch_ = latch;}
	inline void setCarryoverOption(bool option){ carryover_ = option;}
	inline bool getCarryoverOption() const { return carryover_;}
	void setJobId(const Guid& jobId);
	inline const Guid& getJobId() const { return jobId_;}
	inline void setRootJobId(const Guid& rootJobId) { rootJobId_ = rootJobId;}
	inline const Guid& getRootJobId() const { return rootJobId_;}
	inline const Guid& getTaskId() const { return taskId_;}
	inline void setPriority(int priority) { priority_ = priority;}
	inline int getPriority() const { return priority_;}
	inline void setParallelism(int parallelism) { parallelism_ = parallelism;}
	inline int getParallelism() const { return parallelism_;}
    inline const Guid &getSpanId() const { return spanId_; }
    inline void setSpanId(const Guid &spanId) { spanId_ = spanId; }
    inline const Guid &getParentSpanId() const { return parentSpanId_; }
    inline void setParentSpanId(const Guid &spanId) { parentSpanId_ = spanId; }
	void set(Heap* heap, const SessionSP& session, const Guid& jobId, const CountDownLatchSP& latch, bool addDepth = true, bool setViewMode = false);
	void set(Heap* heap, const SessionSP& session, bool setViewMode = false);
	void done(const ConstantSP& result);
	void done(const string& errMsg);
	inline const string& getErrorMessage() const {return errMsg_;}
	inline ConstantSP getResultObject() const {return execResult_;}
	inline ConstantSP& getResultObjectRef()  {return execResult_;}
	inline ConstantSP getCarryoverObject() const {return carryoverResult_;}
	inline bool isLocalData() const { return local_;}
	inline bool isViewMode() const { return viewMode_;}
	inline bool isCancellable() const {return cancellable_;}
	inline void setCancellable(bool option){cancellable_ = option;}
    inline bool isTracing() const { return tracing_; }
    inline void setTracing(bool tracing) { tracing_ = tracing; }
	inline int getDepth() const { return depth_;}
	inline void setDepth(int depth){ depth_ = depth;}
	virtual void setLastSuccessfulSite(){}
	virtual int getLastSite(){return -1;}
	static ConstantSP mergeDistributedCallResult(vector<DistributedCallSP>& calls);

private:
	Guid rootJobId_;
	Guid jobId_;
	Guid taskId_;
	int priority_;
	int parallelism_;
	CountDownLatchSP latch_;
	ObjectSP obj_;
	string errMsg_;
	ConstantSP execResult_;
	ConstantSP carryoverResult_;
	ReducerContainerSP reducer_;
	std::function<void (bool, const ConstantSP&)> callback_;
	std::chrono::system_clock::time_point receivedTime_;
	std::chrono::system_clock::time_point startTime_;
	std::chrono::system_clock::time_point completeTime_;
	bool local_;
	bool viewMode_;
	bool cancellable_;
	bool carryover_ = false;
	bool callbackMode_ = false;
    bool tracing_ = false;
    Guid spanId_;
    Guid parentSpanId_;
	unsigned char depth_ = 0;
	Heap* heap_;
	SessionSP session_;
};

struct JobProperty {
	JobProperty() : rootId_(false), taskId_(false), clientId_(false), seqNo_(0), priority_(0), parallelism_(1),cancellable_(true) {}
	JobProperty(const Guid& rootId, const Guid& taskId, int priority, int parallelism, bool cancellable) : rootId_(rootId),
			taskId_(taskId), clientId_(false), seqNo_(0), priority_(priority), parallelism_(parallelism), cancellable_(cancellable){}
	JobProperty(const Guid& rootId, const Guid& taskId, const Guid& clientId, long long seqNo, int priority, int parallelism, bool cancellable) : rootId_(rootId),
			taskId_(taskId), clientId_(clientId), seqNo_(seqNo), priority_(priority), parallelism_(parallelism), cancellable_(cancellable){}
	JobProperty(const DataInputStreamSP& in);
	IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const;
	void setJob(const DistributedCallSP& call);

	Guid rootId_;
	Guid taskId_;
	Guid clientId_;
	long long seqNo_;
	int priority_;
	int parallelism_;
	bool cancellable_;
};

class WindowJoinFunction {
public:
	WindowJoinFunction(const string& name, INDEX rows) : name_(name), rows_(rows){}
	virtual ~WindowJoinFunction(){};
	virtual VectorSP createNullReturn(Heap* heap) = 0;
	virtual void startGroup(Heap* heap, INDEX startingRows) = 0;
	virtual void addMap(Heap* heap, INDEX startingRows, int count, vector<pair<INDEX,INDEX>>& indices) = 0;
	virtual void addMap(Heap* heap, INDEX startingRows, int count);
	VectorSP getReturn() const {return ret_;}

protected:
	string name_;
	INDEX rows_;
	VectorSP ret_;
};

class ColumnContext {
public:
	ColumnContext() : index_(-1){}
	int getIndex() const { return index_;}
	void setIndex(int index) { index_ = index;}

private:
	int index_;
};


namespace SSLWrapper {
	void OPENSSL_cleanse(void *ptr, size_t len);
}

#ifndef _WIN32

template <typename T>
struct zallocator
{
public:
    typedef T value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;

    pointer address (reference v) const {return &v;}
    const_pointer address (const_reference v) const {return &v;}

    pointer allocate (size_type n, const void* hint = 0) {
        if (n > std::numeric_limits<size_type>::max() / sizeof(T))
            throw std::bad_alloc();
        return static_cast<pointer> (::operator new (n * sizeof (value_type)));
    }

    void deallocate(pointer p, size_type n) {
        SSLWrapper::OPENSSL_cleanse(p, n*sizeof(T));
        ::operator delete(p);
    }

    size_type max_size() const {
        return std::numeric_limits<size_type>::max() / sizeof (T);
    }

    template<typename U>
    struct rebind
    {
        typedef zallocator<U> other;
    };

    void construct (pointer ptr, const T& val) {
        new (static_cast<T*>(ptr) ) T (val);
    }

    void destroy(pointer ptr) {
        static_cast<T*>(ptr)->~T();
    }

#if __cpluplus >= 201103L
    template<typename U, typename... Args>
    void construct (U* ptr, Args&&  ... args) {
        ::new (static_cast<void*> (ptr) ) U (std::forward<Args> (args)...);
    }

    template<typename U>
    void destroy(U* ptr) {
        ptr->~U();
    }
#endif
};

template <class T>
inline bool operator==(const zallocator<T>&, const zallocator<T>&) {
    return true;
}

template <class T>
inline bool operator!=(const zallocator<T>&, const zallocator<T>&) {
    return false;
}


typedef unsigned char byte;
typedef std::basic_string<char, std::char_traits<char>, zallocator<char> > secure_string;

#else
typedef std::string secure_string;
#endif

class StageExecutor {
public:
	virtual ~StageExecutor(){}
	virtual vector<DistributedCallSP> execute(Heap* heap, const vector<DistributedCallSP>& tasks, bool forceParallel = false) = 0;
	virtual vector<DistributedCallSP> execute(Heap* heap, const vector<DistributedCallSP>& tasks, const JobProperty& jobProp, bool forceParallel = false) = 0;
};

class StaticStageExecutor : public StageExecutor{
public:
	StaticStageExecutor(bool parallel, bool reExecuteOnOOM, bool trackJobs, bool resumeOnError = false, bool scheduleRemoteSite = true) :  parallel_(parallel),
		reExecuteOnOOM_(reExecuteOnOOM), trackJobs_(trackJobs),	resumeOnError_(resumeOnError), scheduleRemoteSite_(scheduleRemoteSite){}
	virtual ~StaticStageExecutor(){}
	vector<DistributedCallSP> execute(Heap* heap, const vector<DistributedCallSP>& tasks, const JobProperty& jobProp, bool forceParallel = false);
	vector<DistributedCallSP> execute(Heap* heap, const vector<DistributedCallSP>& tasks, bool forceParallel = false);
	void execute(Heap* heap, const vector<DistributedCallSP>& tasks, const std::function<void(const vector<DistributedCallSP>&, int)>& callback,
        long long batchTaskSize = -1);
    void setForbidProbingGroupSize(bool flag){forbidProbingGroupSize_ = flag;}
    void setWaitRunningTaskFinishedOnError(bool flag){waitRunningTaskFinishedOnError_ = flag;}
    bool getForbidProbingGroupSize() const {return forbidProbingGroupSize_;}
    void setMonitorProcessAndMemory(bool flag, const string& script){monitorProcessAndMemory_ = flag; script_ = script;}
    bool getMonitorProcessAndMemory() const {return monitorProcessAndMemory_;}

private:
    void groupRemoteCalls(const unordered_map<int, vector<DistributedCallSP>>& tasks,
                          vector<DistributedCallSP>& groupedCalls, const ClusterNodesSP& clusterNodes, int groupSize);

    bool probingGroupSize(bool& groupCall, const vector<DistributedCallSP>& tasks, const ClusterNodesSP& clusterNodes,
                          unordered_map<int, vector<DistributedCallSP>>& siteCalls,
                          vector<std::pair<int,DistributedCallSP>>& needCheckTasks);
private:
    const int MIN_TASK_COUNT_FOR_PROBING_GROUP_SIZE = 4; // if all site's task count is less or equal than it, will group call directly and won't prob group size
    const int TASK_LIMIT_OF_A_GROUP = 1024; // the max task size of a group
	bool parallel_;
	bool reExecuteOnOOM_;
	bool trackJobs_;
	bool resumeOnError_;
	bool scheduleRemoteSite_;
    bool forbidProbingGroupSize_ = true;
    bool monitorProcessAndMemory_ = false;
    bool waitRunningTaskFinishedOnError_ = false;
    long long monitorId_ = 0;
    string script_;
};

class PipelineStageExecutor : public StageExecutor {
public:
	PipelineStageExecutor(vector<FunctionDefSP>& followingFunctors, bool trackJobs, int queueDepth = 2, int parallel = 1) : followingFunctors_(followingFunctors), trackJobs_(trackJobs),
		queueDepth_(queueDepth), parallel_(parallel){}
	virtual ~PipelineStageExecutor(){}
	virtual vector<DistributedCallSP> execute(Heap* heap, const vector<DistributedCallSP>& tasks, bool forceParallel = false);
	virtual vector<DistributedCallSP> execute(Heap* heap, const vector<DistributedCallSP>& tasks, const JobProperty& jobProp, bool forceParallel = false);

private:
	void parallelExecute(Heap* heap, vector<DistributedCallSP>& tasks);

private:
	vector<FunctionDefSP> followingFunctors_;
	bool trackJobs_;
	int queueDepth_;
	int parallel_;
};

class Decoder {
public:
	Decoder(int id, bool appendable) : id_(id), appendable_(appendable), codeSymbolAsString_(false){}
	virtual ~Decoder(){}
	virtual VectorSP code(const VectorSP& vec, bool lsnFlag, CIPHER_MODE mode = CIPHER_MODE::PLAIN_TEXT, const secure_string& key = {}) = 0 ;
	virtual IO_ERR code(const VectorSP& vec, bool lsnFlag, const DataOutputStreamSP& out, int& checksum, int offset = 0, CIPHER_MODE mode = CIPHER_MODE::PLAIN_TEXT, const secure_string& key = {}) = 0;
	virtual IO_ERR decode(const VectorSP& vec, INDEX rowOffset, INDEX skipRows, bool fullLoad, int checksum, const DataInputStreamSP& in,
			long long byteSize, long long byteOffset, INDEX& postRows, INDEX& postRowOffset, long long& postByteOffset, long long& lsn, int& partial, CIPHER_MODE mode = CIPHER_MODE::PLAIN_TEXT, const secure_string& key = {}) = 0;
	/**
	 * Calculate the CRC32 checksum of the first given number of rows of the decompressed vector.
	 *
	 * in: the input data stream. The cursor must be pointed to the first row of the vector. If the input column file has a header,
	 *     the pointer must be after the header.
	 * type: the data type of the column.
	 * rows: the number of rows to calculate
	 * symbase: it is must be provided if the data type is SYMBOL or SYMBOL[].
	 */
	virtual int checksum(const DataInputStreamSP& in, DATA_TYPE type, INDEX rows, const SymbolBaseSP& symbase = nullptr, CIPHER_MODE mode = CIPHER_MODE::PLAIN_TEXT, const secure_string& key = {}) = 0;
	inline int getID() const {return id_;}
	inline bool isAppendable() const {return appendable_;}
	inline bool codeSymbolAsString() const { return codeSymbolAsString_;}
	void codeSymbolAsString(bool enabled) { codeSymbolAsString_ = enabled;}
private:
	int id_;
	bool appendable_;
	bool codeSymbolAsString_;
};

class VolumeMapper {
public:
	VolumeMapper(vector<string>& volumes, int workers);
	int getMappedDeviceId(const string& path);

private:
	int workers_;
	unordered_map<int, int> deviceMap_;
};

struct ColumnHeader{
	ColumnHeader(const char* header);
	ColumnHeader();
	static int getHeaderSize(){ return 20;}
	void serialize(ByteArrayCodeBuffer& buf);
	void deserialize(const char* header);
	inline bool isLittleEndian() const { return flag & 1;}
	inline bool containChecksum() const {return flag & 2;}
	inline bool lsnFlag() const{return flag & 4;}
	inline void setLittleEndian(bool val){ if(val) flag |= 1; else flag &= ~1;}
	inline void setChecksumOption(bool val){ if(val) flag |= 2; else flag &= ~2;}
	inline void setLSNFlag(bool val){ if(val) flag |= 4; else flag &= ~4;}
	inline bool isEncrypt() const{return flag & 8;}
	inline void setIsEncrypt(bool val){ if(val) flag |= 8; else flag &= ~8;}

	inline int getScaleForDecimal() const { return reserved & 0xff; }
	inline void setScaleForDecimal(int scale) { reserved = (reserved & 0xff00) | (scale & 0xff); }

	char version;
	char flag; //bit0: littleEndian bit1: containChecksum bit2: lsnFlag bit3: isEncrypt
	char charCode;
	char compression;
	char dataType;
	char unitLength;

	/**
	 * 16              8               0
	 *  +--------------+---------------+
	 *  |    unused    |     scale     |
	 *  +--------------+---------------+
	 *
	 * Lower 8 bits: scale for decimal data type.
	 * Higher 8 bits: reserved.
	 */
	short reserved;

	int extra;
	int count;
	int checksum;
};

class DBFileIO {
public:
	static bool saveBasicTable(Session* session, const string& directory, Table* table, const string& tableName, IoTransaction* tran,  bool append = false, int compressionMode = 0, bool saveSymbolBase = true, CIPHER_MODE mode = PLAIN_TEXT, const secure_string& tableKey = "");
	static bool saveBasicTable(Session* session, SystemHandle* db, Table* table, const string& tableName, IoTransaction* tran, bool append = false, int compressionMode = 0, bool saveSymbolBase = true, CIPHER_MODE mode = PLAIN_TEXT, const secure_string& tableKey = "");
	static bool saveBasicTable(Session* session, const string& directory, const string& tableDir, Table* table, const string& tableName, const vector<ColumnDesc>& cols, SymbolBaseSP& symbase, IoTransaction* tran, bool chunkMode, bool append, int compressionMode, bool saveSymbolBase, CIPHER_MODE mode = PLAIN_TEXT, const secure_string& tableKey = "");
	static bool saveBasicTable(const string& directory, const string& tableName, Table* table, const SymbolBaseSP& symBase, IoTransaction* tran, int compressionMode, const vector<string>& comments, CIPHER_MODE mode = PLAIN_TEXT, const secure_string& tableKey = "");
	static bool saveBasicTable(Session* session, const string& tableDir, INDEX existingTblSize, Table* table, const vector<ColumnDesc>& cols, const SymbolBaseSP& symBase, IoTransaction* tran, int compressionMode, bool saveSymbolBase, long long lsn, vector<long long>& colsFileOffset, CIPHER_MODE mode = PLAIN_TEXT, const secure_string& tableKey = "");
	static bool savePartitionedTable(Session* session, const DomainSP& domain, TableSP table, const string& tableName, IoTransaction* tran, int compressionMode = 0, bool saveSymbolBase = true, CIPHER_MODE mode = PLAIN_TEXT, const secure_string& tableKey = "" );
	static bool saveDualPartitionedTable(Session* session, SystemHandle* db, const DomainSP& secDomain, TableSP table, const string& tableName,
			const string& partitionColName, vector<string>& secPartitionColNames, IoTransaction* tran, int compressionMode = 0);
	static Table* loadTable(Session* session, const string& directory, const string& tableName, const SymbolBaseManagerSP& symbaseManager, const DomainSP& domain, const ConstantSP& partitions, TABLE_TYPE tableType, bool memoryMode, CIPHER_MODE mode = PLAIN_TEXT, const secure_string& tableKey = "");
	static Table* loadTable(Session* session, SystemHandle* db, const string& tableName, const ConstantSP& partitions, TABLE_TYPE tableType, bool memoryMode, CIPHER_MODE mode = PLAIN_TEXT, const secure_string& tableKey = "");
	// This function will load data from both cacheEngine and disk
	static Table* loadTable(const string& tableName, const string& physicalDir, const Guid& chunkID, const string& chunkPath, const vector<ColumnDesc>& colDescs, INDEX startRows, INDEX endRows, CIPHER_MODE mode = PLAIN_TEXT, const secure_string& tableKey = "");
	static void removeTable(SystemHandle* db, const string& tableName);
	static SystemHandle* openDatabase(const string& directory, const DomainSP& localDomain);
	static bool saveDatabase(SystemHandle* db);
	static bool removeDatabase(const string& dbDir);

	static ColumnHeader loadColumnHeader(const string& colFile);
	static VectorSP loadColumn(const string& colFile, int devId, const SymbolBaseManagerSP& symbaseManager, CIPHER_MODE mode = PLAIN_TEXT, const secure_string& tableKey = "");
	static VectorSP loadColumn(const string& colFile, int devId, const SymbolBaseSP& symbase, CIPHER_MODE mode = PLAIN_TEXT, const secure_string& tableKey = "");
	static VectorSP loadColumn(const string& colFile, int devId, const SymbolBaseSP& symbase, int rows, long long& postFileOffset, INDEX& rowOffset, bool& isLittleEndian, char& compressType, CIPHER_MODE mode = PLAIN_TEXT, const secure_string& tableKey = "");
	/**
	 * colFile: the full path of the column file
	 * fileOffset: the starting point to read data from the column file.
	 * isLittleEndian: true if the machine is little endian.
	 * compressType: 0, no compression; 1, LZ4; 2, Delta of delta. If compressType is -1 and fileOffset  is zero, the system will read the compress type from the column file.
	 * devId: use DBFileIO::getMappedDeviceId(colFile) to get the devId
	 * symbase: symbol base if the column is type of SYMBOL.
	 * skipRows: the number of rows to skip before appending to the given vector.
	 * rows: the number of rows to append to the given vector.
	 * col: the in-memory vector to store the data. The vector may already contains data. The new data is appended to the vector.
	 * rowOffset: the row number (starting from zero) of the file block next to the last read block.
	 *
	 * return: the file offset of the file block next to the last read block.
	 */
	static long long loadColumn(const string& colFile, long long fileOffset, bool& isLittleEndian, char& compressType, int devId, const SymbolBaseSP& symbase, INDEX skipRows, INDEX rows, const VectorSP& col, INDEX& rowOffset, CIPHER_MODE mode = PLAIN_TEXT, const secure_string& tableKey = "");
	static Vector* loadTextVector(bool includeHeader, DATA_TYPE type, const string& path);
	static long long saveColumn(const VectorSP& col, const string& colFile, int devId, INDEX existingTableSize, bool chunkNode, bool append, int compressionMode, IoTransaction* tran = NULL, long long lsn = -1, CIPHER_MODE mode = PLAIN_TEXT, const secure_string& tableKey = "");
	static bool saveTableHeader(const string& owner, const string& physicalIndex, const vector<ColumnDesc>& cols, vector<int>& partitionColumnIndices, long long rows, const string& tableFile, IoTransaction* tran);
	static bool saveTableHeader(const string& owner, const string& physicalIndex, const vector<ColumnDesc>& cols, vector<int>& partitionColumnIndices, vector<pair<int, bool>>& sortKeys,
			DUPLICATE_POLICY rowDuplicatePolicy, long long rows, const string& tableFile, IoTransaction* tran);
	static bool loadTableHeader(const DataInputStreamSP& in, string& owner, string& physicalIndex, vector<ColumnDesc>& cols, vector<int>& partitionColumnIndices, DBENGINE_TYPE engineType);
	static bool loadTableHeader(const DataInputStreamSP& in, TableHeader& header, DBENGINE_TYPE engineType);
	static void removeBasicTable(const string& directory, const string& tableName);
	/**
	 * @param extras extra param for data type (i.e., scale for decimal)
	 */
	static TableSP createEmptyTableFromSchema(const TableSP& schema, const std::vector<long long> &extras = {});
	static long long truncateColumnByLSN(const string& colFile, int devId, long long expectedLSN, bool sync=true, CIPHER_MODE mode = PLAIN_TEXT, const secure_string& tableKey = "");
	static long long truncateColumnByRows(const string& colFile, int devId, INDEX rows, bool sync=true, CIPHER_MODE mode = PLAIN_TEXT, const secure_string& tableKey = "");
	static void truncateSymbolBase(const string& symFile, int devId, INDEX rows, bool sync=true);

	static void checkTypeCompatibility(Table* table, vector<string>& partitionColumns, vector<ColumnDesc>& cols, vector<int>& partitionColumnIndices);
	static bool checkTypeCompatibility(DATA_TYPE type1, DATA_TYPE type2);
	static bool checkPartitionColumnCompatibility(DATA_TYPE partitionSchemeType, DATA_TYPE partitionDataType);
	static void saveSymbolBases(const SymbolBaseSP& symbase, IoTransaction* tran);
	static void collectColumnDesc(Table* table, vector<ColumnDesc>& cols);
	static void collectColumnDesc(Table* table, vector<ColumnDesc>& cols, vector<int>& compressMethods);
	static ConstantSP convertColumn(const ConstantSP& col, const ColumnDesc& desiredType, SymbolBaseSP& symbase);
	static ConstantSP convertColumn(const ConstantSP& col, const ColumnDesc& desiredType, const SymbolBaseSP& symbase);
	static int getCompressionMethod(const ColumnDesc& col, int defaultMethod);
	static VectorSP decompress(const VectorSP& col, CIPHER_MODE mode = PLAIN_TEXT, const secure_string& tableKey = "");
	static VectorSP decompress(const VectorSP& col, const DecoderSP decoder, CIPHER_MODE mode = PLAIN_TEXT, const secure_string& tableKey = "");
	static TableSP decompressTable(const TableSP& tbl, CIPHER_MODE mode = PLAIN_TEXT, const secure_string& tableKey = "");
	static VectorSP compress(const VectorSP& col, int compressMode = 1, CIPHER_MODE mode = PLAIN_TEXT, const secure_string& tableKey = "");
	static TableSP compressTable(const TableSP& table, CIPHER_MODE mode = PLAIN_TEXT, const secure_string& tableKey = "");
	static TableSP compressTable(const TableSP& table, vector<ColumnDesc>& types, CIPHER_MODE mode = PLAIN_TEXT, const secure_string& tableKey = "");
	static ConstantSP appendDataToFile(Heap* heap, vector<ConstantSP>& arguments);
	static int getMappedDeviceId(const string& path);
	static void setVolumeMapper(const VolumeMapperSP& volumeMapper) { volumeMapper_ = volumeMapper;}
	static unsigned int checksum(const DataInputStreamSP& in, long long offset, long long length);
    static long long getColumnLeastLSN(const string& colFile);
private:
	static VectorSP loadColumn(const string& colFile, int devId, const SymbolBaseManagerSP& symbaseManager,	const SymbolBaseSP& symbase,
			int rows, long long& postFileOffset, INDEX& rowOffset, bool& isLittleEndian, char& compressType, CIPHER_MODE mode = PLAIN_TEXT, const secure_string& tableKey = "");
	static inline DATA_TYPE getCompressedDataType(const VectorSP& vec){return (DATA_TYPE)vec->getChar(4);}
	static VolumeMapperSP volumeMapper_;
};

class Tuple : public Object {
public:
	Tuple(const vector<ObjectSP>& arguments, bool isFunctionArgument = false, bool isDynamicVector = false, bool readonly=false): Object(TUPLE), arguments_(arguments),
		isFunctionArgument_(isFunctionArgument), isDynamicVector_(isDynamicVector), readonly_(readonly){}
	Tuple(Session* session, const DataInputStreamSP& in);
	inline bool isFunctionArgument() const { return isFunctionArgument_;}
	inline bool isDynamicVector() const { return isDynamicVector_;}
	inline bool readonly() const { return readonly_;}
	ObjectSP getElement(int index) const {return arguments_[index];}
	int getElementCount() const { return arguments_.size();}
	virtual ConstantSP getValue(Heap* pHeap) { return getReference(pHeap);}
	virtual ConstantSP getReference(Heap* pHeap);
	virtual ConstantSP getComponent() const;
	virtual string getScript() const;
	virtual IO_ERR serialize(Heap* pHeap, const ByteArrayCodeBufferSP& buffer) const;
	virtual void collectUserDefinedFunctions(unordered_map<string,FunctionDef*>& functionDefs) const;
	virtual ObjectSP copy(Heap* pHeap, const SQLContextSP& context, bool localize) const;
	virtual ObjectSP copyAndMaterialize(Heap* pHeap, const SQLContextSP& context, const TableSP& table) const;
	virtual bool mayContainColumnRefOrVariable() const { return true;}
	virtual void collectObjects(vector<const Object*>& vec) const;

private:
	vector<ObjectSP> arguments_;
	bool isFunctionArgument_;
	bool isDynamicVector_;
	bool readonly_;
};


class Expression: public Object{
public:
	Expression(const vector<ObjectSP> & objs, const vector<OperatorSP>&  optrs): Object(EXPRESSION),
		objs_(objs),optrs_(optrs), annotation_(0){}
	Expression(const vector<ObjectSP> & objs, const vector<OperatorSP>&  optrs, int annotation): Object(EXPRESSION),
		objs_(objs),optrs_(optrs), annotation_(annotation){}
	Expression(const SQLContextSP& context, Session* session, const DataInputStreamSP& in);
	virtual ~Expression(){}
	virtual ConstantSP getComponent() const;
	virtual ConstantSP getReference(Heap* pHeap);
	virtual ConstantSP getValue(Heap* pHeap);
	int getObjectCount() const {return objs_.size();}
	const ObjectSP& getObject(int index) const {return objs_[index];}
	const vector<ObjectSP>& getObjects() const {return objs_;}
	int getOperatorCount() const {return optrs_.size();}
	const OperatorSP& getOperator(int index) const {return optrs_[index];}
	const vector<OperatorSP>& getOperators() const {return optrs_;}
	virtual string getScript() const;
	virtual IO_ERR serialize(Heap* pHeap, const ByteArrayCodeBufferSP& buffer) const;
	virtual void collectUserDefinedFunctions(unordered_map<string,FunctionDef*>& functionDefs) const;
	ObjectSP realizeNonColumnExpression(Heap* pHeap, const TableSP& table);
	//Assume all local variables in the expression has been materialized
	ObjectSP realizeNonColumnExpression(Heap* pHeap);
	inline int getAnnotation() const {return annotation_;}
	inline bool getAnnotation(int bitOffset) const {return annotation_ & (bitOffset << 1);}
	void setAnnotation(int bitOffset, bool bitOn);
	virtual ObjectSP copy(Heap* pHeap, const SQLContextSP& context, bool localize) const;
	virtual ObjectSP copyAndMaterialize(Heap* pHeap, const SQLContextSP& context, const TableSP& table) const;
	virtual bool mayContainColumnRefOrVariable() const { return true;}
	void collectObjects(vector<const Object*>& vec) const override;

	static ConstantSP void_;
	static ConstantSP null_;
	static ConstantSP default_;
	static ConstantSP true_;
	static ConstantSP false_;
	static ConstantSP one_;
	static ConstantSP zero_;
	static ConstantSP voidDouble2_;
	static OperatorSP logicAnd_;
	static SQLContextSP context_;

private:
	static ObjectSP partialEvaluate(Heap* pHeap, const vector<ObjectSP>& objs, const vector<OperatorSP>& optrs, int annotation);

private:
	vector<ObjectSP> objs_;
    vector<OperatorSP> optrs_;
    /**
     * The field doesn't involve in the calculation of the expression. But it is used to annotate the
     * expression. For example, expressions in the SQL where clause may help decide which partition is
     * relevant to the query. In the case of range partition or value partition, this flag can further
     * tell if one can optimize the where clause, i.e. removal of the whole or part of the expression
     * from the where clause. In this use case:
     * 	bit0: 0 the part of the expression is used to decide the relevant partition.
     * 	      1 the whole expression is used to decide the relevant partition.
     * 	bit1: value partitioning column is used to decide the relevant partition.
     * 	bit2: range partitioning column is used to decide the relevant partition.
     * 	bit3: list partitioning column is used to decide the relevant partition.
     */
    int annotation_;
};

class InternalUtil {
public:
	static IO_ERR readBytes(const DataInputStreamSP& in, char* buf, int length);
	static Vector* createArrayVector(DATA_TYPE type, INDEX size, INDEX valueSize=0, INDEX capacity=0, INDEX valueCapacity=0, bool fastMode=true, int extraParam=0);
};
template<class T>
class CircularQueue{
public:
	CircularQueue(int capacity = 16) : capacity_(capacity), size_(0), head_(0), tail_(0), buf_(capacity, T{}){}
	IO_ERR snapshotState(const DataOutputStreamSP& out){
		int buf[3] = {size_, head_, tail_};
		IO_ERR ret = out->write((const char*)buf, 12);
		if(ret != OK) return ret;
        T *pbuf = &(buf_[0]);
		return out->write((const char*)pbuf, sizeof(T) * capacity_);
	}
	IO_ERR snapshotState(Buffer& out, int offset, int length){
		int base = head_ + offset;
		int end = std::min(length, capacity_ - base);
		for(int i=0; i<end; ++i){
			IO_ERR ret = out.write(buf_[base + i]);
			if(ret != OK) return ret;
		}

		end = std::max(0, base + length - capacity_);
		for(int i=0; i<end; ++i){
			IO_ERR ret = out.write(buf_[i]);
			if(ret != OK) return ret;
		}
		return OK;
	}
	IO_ERR restoreState(const DataInputStreamSP& in){
		IO_ERR ret = in->readInt(size_);
		if(ret != OK) return ret;
		in->readInt(head_);
		in->readInt(tail_);
		if(head_ > size_ || tail_ > size_)
			return INVALIDDATA;
        T *pbuf = &(buf_[0]);
		return in->readBytes((char*)pbuf, sizeof(T) * capacity_, false);
	}
	void push(T& value){
		if(size_ >= capacity_){
			// equivalent to head_ = (head_ + 1) % capacity_;
			int head = (head_ + 1);
			int cond = -(int)(head < capacity_);
			head_ = (cond & head) | (~cond & (head - capacity_));

			buf_[tail_] = value;
			tail_ = head_;
		}
		else{
			buf_[tail_] = value;

			// equivalent to tail_ = (tail_ + 1) % capacity_;
			int tail = tail_ + 1;
			int cond = - (int)(tail < capacity_);
			tail_ = (cond & tail) | (~cond & (tail - capacity_));

			++size_;
		}
	}
	inline int size() const {return size_;}
	inline const T& get(int index) const {
		// equivalent to buf_[(head_ + index) % capacity_]
		index = head_ + index;
		int cond = - (int)(index < capacity_);
		return buf_[(cond & index) | (~cond & (index - capacity_))];
	}
	inline const T& tail() const {
		//equivalent to tail_ == 0 ? buf_[capacity_ - 1] : buf_[tail_ - 1]
		int cond = - (int)(tail_ == 0);
		return buf_[(cond & (capacity_ - 1)) | (~cond & (tail_ - 1))];
	}
	inline const T& head() const { return buf_[head_];}

private:
	int capacity_;
	int size_;
	int head_;
	int tail_;
	std::vector<T> buf_;
};

struct DoubleReader {
	double operator()(Constant* vec, INDEX index) const {
		return vec->getDouble(index);
	}
	double operator()(Constant* x) const {
		return x->getDouble();
	}
	const double* getConst(Constant* x, INDEX start, int count, double* buf) const {
		return x->getDoubleConst(start, count, buf);
	}
	double* getBuffer(Constant* x, INDEX start, int count, double* buf) const {
		return x->getDoubleBuffer(start, count, buf);
	}
	bool operator()(Constant* vec, INDEX start, int count, double* buf) const {
		return vec->getDouble(start, count, buf);
	}
	bool operator()(Constant* vec, INDEX* indices, int count, double* buf) const {
		return vec->getDouble(indices, count, buf);
	}
	bool operator()(Vector* vec, INDEX offset, INDEX* sortedIndices, int count, double* buf) const {
		return vec->getDoubleSafe(offset, sortedIndices, count, buf);
	}
};

struct FloatReader {
	float operator()(Constant* vec, INDEX index) const {
		return vec->getFloat(index);
	}
	float operator()(Constant* x) const {
		return x->getFloat();
	}
	const float* getConst(Constant* x, INDEX start, int count, float* buf) const {
		return x->getFloatConst(start, count, buf);
	}
	float* getBuffer(Constant* x, INDEX start, int count, float* buf) const {
		return x->getFloatBuffer(start, count, buf);
	}
	bool operator()(Constant* vec, INDEX start, int count, float* buf) const {
		return vec->getFloat(start, count, buf);
	}
	bool operator()(Constant* vec, INDEX* indices, int count, float* buf) const {
		return vec->getFloat(indices, count, buf);
	}
	bool operator()(Vector* vec, INDEX offset, INDEX* sortedIndices, int count, float* buf) const {
		return vec->getFloatSafe(offset, sortedIndices, count, buf);
	}
};

struct LongReader {
	long long operator()(Constant* vec, INDEX index) const {
		return vec->getLong(index);
	}
	long long operator()(Constant* x) const {
		return x->getLong();
	}
	const long long* getConst(Constant* x, INDEX start, int count, long long* buf) const {
		return x->getLongConst(start, count, buf);
	}
	long long* getBuffer(Constant* x, INDEX start, int count, long long* buf) const {
		return x->getLongBuffer(start, count, buf);
	}
	bool operator()(Constant* vec, INDEX start, int count, uint64_t* buf) const {
		return vec->getLong(start, count, (long long*)buf);
	}
	bool operator()(Constant* vec, INDEX start, int count, long long* buf) const {
		return vec->getLong(start, count, buf);
	}
	bool operator()(Constant* vec, INDEX* indices, int count, long long* buf) const {
		return vec->getLong(indices, count, buf);
	}
	bool operator()(Vector* vec, INDEX offset, INDEX* sortedIndices, int count, long long* buf) const {
		return vec->getLongSafe(offset, sortedIndices, count, buf);
	}
};

struct IntReader {
	int operator()(Constant* vec, INDEX index) const {
		return vec->getInt(index);
	}
	int operator()(Constant* x) const {
		return x->getInt();
	}
	const int* getConst(Constant* x, INDEX start, int count, int* buf) const {
		return x->getIntConst(start, count, buf);
	}
	int* getBuffer(Constant* x, INDEX start, int count, int* buf) const {
		return x->getIntBuffer(start, count, buf);
	}
	bool operator()(Constant* vec, INDEX start, int count, uint32_t* buf) const{
		return vec->getInt(start, count, (int*)buf);
	}
	bool operator()(Constant* vec, INDEX start, int count, int* buf) const{
		return vec->getInt(start, count, buf);
	}
	bool operator()(Constant* vec, INDEX* indices, int count, int* buf) const{
		return vec->getInt(indices, count, buf);
	}
	bool operator()(Vector* vec, INDEX offset, INDEX* sortedIndices, int count, int* buf) const{
		return vec->getIntSafe(offset, sortedIndices, count, buf);
	}
};

struct ShortReader {
	short operator()(Constant* vec, INDEX index) const {
		return vec->getShort(index);
	}
	short operator()(Constant* x) const {
		return x->getShort();
	}
	const short* getConst(Constant* x, INDEX start, int count, short* buf) const {
		return x->getShortConst(start, count, buf);
	}
	short* getBuffer(Constant* x, INDEX start, int count, short* buf) const {
		return x->getShortBuffer(start, count, buf);
	}
	bool operator()(Constant* vec, INDEX start, int count, uint16_t* buf) const {
		return vec->getShort(start, count, (short*)buf);
	}
	bool operator()(Constant* vec, INDEX start, int count, short* buf) const {
		return vec->getShort(start, count, buf);
	}
	bool operator()(Constant* vec, INDEX* indices, int count, short* buf) const {
		return vec->getShort(indices, count, buf);
	}
	bool operator()(Vector* vec, INDEX offset, INDEX* sortedIndices, int count, short* buf) const {
		return vec->getShortSafe(offset, sortedIndices, count, buf);
	}
};

struct CharReader {
	char operator()(Constant* vec, INDEX index) const {
		return vec->getChar(index);
	}
	char operator()(Constant* x) const {
		return x->getChar();
	}
	const char* getConst(Constant* x, INDEX start, int count, char* buf) const {
		return x->getCharConst(start, count, buf);
	}
	char* getBuffer(Constant* x, INDEX start, int count, char* buf) const {
		return x->getCharBuffer(start, count, buf);
	}
	bool operator()(Constant* vec, INDEX start, int count, uint8_t* buf) const {
		return vec->getChar(start, count, (char*)buf);
	}
	bool operator()(Constant* vec, INDEX start, int count, char* buf) const {
		return vec->getChar(start, count, buf);
	}
	bool operator()(Constant* vec, INDEX* indices, int count, char* buf) const {
		return vec->getChar(indices, count, buf);
	}
	bool operator()(Vector* vec, INDEX offset, INDEX* sortedIndices, int count, char* buf) const {
		return vec->getCharSafe(offset, sortedIndices, count, buf);
	}
};

struct BoolReader {
	char operator()(Constant* vec, INDEX index) const {
		return vec->getBool(index);
	}
	char operator()(Constant* x) const {
		return x->getBool();
	}
	const char* getConst(Constant* x, INDEX start, int count, char* buf) const {
		return x->getBoolConst(start, count, buf);
	}
	char* getBuffer(Constant* x, INDEX start, int count, char* buf) const {
		return x->getBoolBuffer(start, count, buf);
	}
	bool operator()(Constant* vec, INDEX start, int count, char* buf) const {
		return vec->getBool(start, count, buf);
	}
	bool operator()(Constant* vec, INDEX* indices, int count, char* buf) const {
		return vec->getBool(indices, count, buf);
	}
	bool operator()(Vector* vec, INDEX offset, INDEX* sortedIndices, int count, char* buf) const {
		return vec->getBoolSafe(offset, sortedIndices, count, buf);
	}
};

struct GuidReader {
	Guid operator()(Constant* x, INDEX index) const {
		return x->getInt128(index);
	}
	Guid operator()(Constant* x) const {
		return x->getInt128();
	}
	const Guid* getConst(Constant* x, INDEX start, int count, Guid* buf) const {
		return (Guid*)x->getBinaryConst(start, count, 16, (unsigned char*)buf);
	}
	Guid* getBuffer(Constant* x, INDEX start, int count, Guid* buf) const {
		return (Guid*)x->getBinaryBuffer(start, count, 16, (unsigned char*)buf);
	}
	bool operator()(Constant* vec, INDEX start, int count, Guid* buf) const {
		return vec->getBinary(start, count, 16, (unsigned char*)buf);
	}
	bool operator()(Constant* vec, INDEX* indices, int count, Guid* buf) const {
		return vec->getBinary(indices, count, 16, (unsigned char*)buf);
	}
	bool operator()(Vector* vec, INDEX offset, INDEX* sortedIndices, int count, Guid* buf) const {
		return vec->getBinarySafe(offset, sortedIndices, count, 16, (unsigned char*)buf);
	}
};

class AbstractFunctionDef : public FunctionDef{
public:
	AbstractFunctionDef(FUNCTIONDEF_TYPE defType, const string& name, const vector<ParamSP>& params, bool hasReturnValue=true, bool aggregation=false, bool sequential=false, bool transform=false): FunctionDef(defType, name, params,
		hasReturnValue, aggregation, sequential, transform), val_(name){}
	AbstractFunctionDef(FUNCTIONDEF_TYPE defType, const string& name, int minParamNum, int maxParamNum, bool hasReturnValue, bool aggregation=false, bool sequential=false, bool transform=false) : FunctionDef(defType, name, minParamNum,
		maxParamNum, hasReturnValue, aggregation, sequential, transform), val_(name){}
	virtual ~AbstractFunctionDef(){}
	virtual char getBool() const {throw IncompatibleTypeException(DT_BOOL,DT_FUNCTIONDEF);}
	virtual char getChar() const {throw IncompatibleTypeException(DT_CHAR,DT_FUNCTIONDEF);}
	virtual short getShort() const {throw IncompatibleTypeException(DT_SHORT,DT_FUNCTIONDEF);}
	virtual int getInt() const {throw IncompatibleTypeException(DT_INT,DT_FUNCTIONDEF);}
	virtual long long getLong() const {throw IncompatibleTypeException(DT_LONG,DT_FUNCTIONDEF);}
	virtual INDEX getIndex() const {throw IncompatibleTypeException(DT_INDEX,DT_FUNCTIONDEF);}
	virtual float getFloat() const {throw IncompatibleTypeException(DT_FLOAT,DT_FUNCTIONDEF);}
	virtual double getDouble() const {throw IncompatibleTypeException(DT_DOUBLE,DT_FUNCTIONDEF);}
	virtual string getString() const {
		if (isView() || !getModule().empty()) {
			return getFullName();
		}
		else {
			return val_;
		}
	}
	virtual string getBody() const { return val_; }
	virtual bool isNull() const {return false;}
	virtual void setString(const DolphinString& val) {val_ = val.getString();}
	virtual void setNull(){}
	virtual void nullFill(const ConstantSP& val){}
	virtual bool isNull(INDEX start, int len, char* buf) const {
		for(int i=0;i<len;++i)
			buf[i]=false;
		return true;
	}
	virtual bool isValid(INDEX start, int len, char* buf) const {
		for(int i=0;i<len;++i)
			buf[i]=true;
		return true;
	}
	virtual ConstantSP getInstance() const { throw RuntimeException("FunctionDef::getInstance method not supported.");}
	virtual ConstantSP getValue() const { throw RuntimeException("Function definition [" + name_ + "] is not copyable and can't serve as an operand in numeric calculation.");}
	virtual DATA_FORM getForm() const {return DF_SCALAR;}
	virtual long long getAllocatedMemory() const {return sizeof(string)+val_.size();}
	virtual int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int& numElement, int& partial) const;
	virtual IO_ERR deserialize(DataInputStream* in, INDEX indexStart, INDEX& numElement);
	virtual int compare(int index, const ConstantSP& target) const {
		if (target->getType() == DT_FUNCTIONDEF) {
			return val_.compare(((AbstractFunctionDef *) target.get())->getBody());
		} else
			return val_.compare(target->getString());
	}
	inline void setSequentialFunction(bool option) {
		if(option) extraFlag_ |= 8; else extraFlag_ &= ~8;
	}

private:
	mutable string val_;
};

class PartialFunction : public AbstractFunctionDef{
public:
	PartialFunction(FunctionDefSP funcDef, const vector<ConstantSP>& arguments);
	PartialFunction(Session* session, const DataInputStreamSP& buffer);
	virtual ~PartialFunction();
	virtual ConstantSP call(Heap* heap, vector<ConstantSP>& arguments);
	virtual ConstantSP call(Heap* heap, const ConstantSP& a, const ConstantSP& b);
	virtual ConstantSP call(Heap* heap,vector<ObjectSP>& arguments);
	virtual string getScript() const {return getString();}
	virtual IO_ERR serialize(Heap* pHeap, const ByteArrayCodeBufferSP& buffer) const;
	virtual void collectUserDefinedFunctions(unordered_map<string,FunctionDef*>& functionDefs) const;
	inline FunctionDefSP getFunctionDef() const { return funcDef_;}
	inline int getSetArgumentCount() const { return args_.size();}
	inline ObjectSP getSetArgument(int index) const {return args_[index];}
	inline int getMissingArgumentCount() const { return missingArgs_.size();}
	inline int getMissingArgumentIndex(int index) const { return missingArgs_[index];}
	string generateScript() const;
	inline bool isStaticPartialFunction() const { return static_;}

private:
	FunctionDefSP funcDef_;
	vector<ObjectSP> args_;
	vector<int> missingArgs_;
	/**
	 * The attribute static is created for version compatibility.
	 * Before version 2.00.14 and 3.00.2, static_ could be false
	 * or true. Since version 2.00.14 and 3.00.2, static_ always true.
	 */
	bool static_;
};

class ReactiveState;

typedef SmartPointer<ReactiveState> ReactiveStateSP;

typedef ReactiveStateSP(*StateFuncFactory)(const vector<ObjectSP>& args, const vector<int>& inputColIndices, const vector<DATA_TYPE>& inputColTypes, const vector<int>& outputColIndices);
typedef ReactiveStateSP (*StateFuncFactoryWithContext)(const vector<ObjectSP> &args, const vector<int> &inputColIndices,
                                                       const vector<DATA_TYPE> &inputColTypes,
                                                       const vector<int> &outputColIndices, SQLContextSP &context,
                                                       Heap *heap);

class ReactiveState {
public:
	virtual ~ReactiveState(){}
	virtual IO_ERR snapshotState(const DataOutputStreamSP& out) = 0;
	virtual IO_ERR restoreState(const DataInputStreamSP& in) = 0;
	virtual void append(Heap* heap, const ConstantSP& keyIndex) = 0;
	virtual void addKeys(int count) = 0;
	virtual void removeKeys(const vector<int>& keyIndices) = 0;
	virtual void reserveKeys(int count) = 0;
	virtual void getMemoryUsed(long long& fixedMemUsed, long long& variableMemUsedPerKey) = 0;
	// clearkeys() will NOT remove keys, but reset their state to initial state, used in segmentby
	// add clearKeys for [cumsum, cummax, cummin, cumcount, cumavg, cumstd, cumvar, cumstdp, cumvarp], others not support.
	// for those ReactiveState which NOT support segmentby, throw an execption
	virtual void clearKeys(const ConstantSP& keyIndex) {
		throw RuntimeException("ReactiveState NOT supported clearKeys()");
	}
	virtual void setTable(const TableSP& table){
		table_ = table;
	}
	TableSP getTable() const {
		return table_;
	}
	virtual void setSession(const SessionSP& session){
		session_ = session;
	}

	virtual long long getAdditionalMemoryUsed(){ return 0; }
	virtual bool isTimeMovingFunction(){return false;}
protected:
	template<class T>
	void setData(int outputColIndex, INDEX* indices, int count, T* buf){
		ConstantSP result = table_->getColumn(outputColIndex);
		if(LIKELY(result->isFastMode())){
			T* data = (T*)result->getDataArray();
			for(int i=0; i<count; ++i){
				data[indices[i]] = buf[i];
			}
		}
		else{
			T** dataSegment = (T**)result->getDataSegment();
			int segmentSizeInBit = result->getSegmentSizeInBit();
			int segmentMask = (1<<segmentSizeInBit) - 1;
			for(int i=0; i<count; ++i){
				dataSegment[indices[i] >> segmentSizeInBit][indices[i] & segmentMask] = buf[i];
			}
		}
	}

	template<class T>
	void setData(int outputColIndex, INDEX* indices, int count, const T& value){
		ConstantSP result = table_->getColumn(outputColIndex);
		if(LIKELY(result->isFastMode())){
			T* data = (T*)result->getDataArray();
			for(int i=0; i<count; ++i){
				data[indices[i]] = value;
			}
		}
		else {
			T** dataSegment = (T**)result->getDataSegment();
			int segmentSizeInBit = result->getSegmentSizeInBit();
			int segmentMask = (1<<segmentSizeInBit) - 1;
			for(int i=0; i<count; ++i){
				dataSegment[indices[i] >> segmentSizeInBit][indices[i] & segmentMask] = value;
			}
		}
	}

	void setString(int outputColIndex, INDEX* indices, int count, DolphinString* buf);
	void setString(int outputColIndex, INDEX* indices, int count, const DolphinString& value);

	template<class T>
	inline void reserveElements(vector<T>& data, int keyIndices) {
		data.reserve(keyIndices);
	}

	template<class T>
	void removeElements(vector<T>& data, const vector<int>& keyIndices){
		if(data.empty())
			return;
	    size_t dstCursor = keyIndices[0];
	    size_t srcCursor = keyIndices[0] + 1;
	    size_t indicesCursor = 1;
	    size_t count = data.size();
	    size_t indicesCount = keyIndices.size();
	    while(srcCursor < count){
	        if (indicesCursor < indicesCount) {
	            while (srcCursor < (size_t)keyIndices[indicesCursor]) {
	                data[dstCursor++] = std::move(data[srcCursor++]);
	            }
	            ++srcCursor;
	            ++indicesCursor;
	        }
	        else {
	            while (srcCursor < count) {
	                data[dstCursor++] = std::move(data[srcCursor++]);
	            }
	        }
	    }
		data.erase(data.begin() + count - indicesCount, data.end());
	    //data.resize(count - indicesCount);
	}

protected:
	SessionSP session_;
	TableSP table_;
};

class DigitalSign {
public:
	bool verifySignature(const string& publicKey, const string& plainText, const char* signatureBase64) const;
	bool signMessage(const string& privateKey, const string& plainText, string& signature) const;
	static string sha256(const string& content);
	static bool sha256(const string& filePath, string& output);
	static void base64Encode(const unsigned char* buffer, size_t length, string& base64Text, bool noWrap = false);
	static void base64Decode(const char* b64message, unsigned char** buffer, int& length, bool noWrap = false);
	/**
	 * @brief aes-256-cfb encrypt
	 *
	 * @param key encrypt key, fill zero if key length < 32 bytes
	 * @param iv output 16 bytes random vector, it's ok to public it.
	 * @param ptext text need to be encrypted
	 * @param ctext output encrypted text
	 */
	static bool aesEncrypt(string key, string& iv, const string& ptext, string& ctext);
	/**
	 * @brief aes-256-cfb decrypt
	 *
	 * @param key decrypt key, fill zero if key length < 32 bytes
	 * @param iv 16 bytes random vector
	 * @param ctext encrypted text
	 * @param rtext output decrypted text
	 */
	static bool aesDecrypt(string key, const string& iv, const string& ctext, string& rtext);
	static bool hmac(const char* key, size_t keyLen, const char* data, size_t dataLen, const string& digest, string& output);

private:
	static size_t calcDecodeLength(const char* b64input);
	bool rsaSign(const string& privateKey, const unsigned char* Msg, size_t MsgLen, unsigned char** EncMsg, size_t* MsgLenEnc) const;
	bool rsaVerifySignature(const string& publicKey, unsigned char* MsgHash, size_t MsgHashLen, const char* Msg, size_t MsgLen, bool* Authentic) const;
};

#endif /* CORECONCEPT_H_ */
