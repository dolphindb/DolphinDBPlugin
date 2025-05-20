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
#include <map>


#include "Types.h"
#include "SmartPointer.h"
#include "Exceptions.h"
#include "Concurrent.h"
#include "LocklessContainer.h"
#include "FlatHashmap.h"
#include "SysIOTypes.h"
#include "DolphinString.h"

#define serverVersion "3.00.3"

#if defined(__GNUC__) && __GNUC__ >= 4
#define LIKELY(x) (__builtin_expect((x), 1))
#define UNLIKELY(x) (__builtin_expect((x), 0))
#else
#define LIKELY(x) (x)
#define UNLIKELY(x) (x)
#endif
#define TRANSIENT(x) x->isTransient() ? x->getValue() : x

namespace ddb {
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
class Iterator;
class Constant;
class Vector;
class Matrix;
class Tensor;
class Table;
class Set;
class Dictionary;
class DFSChunkMeta;
class OOClass;
class OOInstance;
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
class ClusterNodes;
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
class JobProperty;
struct JITCfgNode;
struct InferredType;
struct FunctionSignature;
class WindowJoinFunction;
class ColumnContext;
class Transaction;
class Parser;
class ParserData;
class ColumnDef;

typedef SmartPointer<AuthenticatedUser> AuthenticatedUserSP;
typedef SmartPointer<ByteArrayCodeBuffer> ByteArrayCodeBufferSP;
typedef SmartPointer<Iterator> IteratorSP;
typedef SmartPointer<Constant> ConstantSP;
typedef SmartPointer<Vector> VectorSP;
typedef SmartPointer<Matrix> MatrixSP;
typedef SmartPointer<Tensor> TensorSP;
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
typedef SmartPointer<OOClass> OOClassSP;
typedef SmartPointer<OOInstance> OOInstanceSP;
typedef SmartPointer<SQLTransaction> SQLTransactionSP;
typedef SmartPointer<SQLContext> SQLContextSP;
typedef SmartPointer<ColumnRef> ColumnRefSP;
typedef SmartPointer<ColumnDef> ColumnDefSP;
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
typedef SmartPointer<ParserData> ParserDataSP;

typedef ConstantSP(*OptrFunc)(const ConstantSP&, const ConstantSP&);
typedef ConstantSP(*OptrFunc2)(Heap* heap, const ConstantSP&, const ConstantSP&);
typedef ConstantSP(*SysFunc)(Heap* heap, vector<ConstantSP>& arguments);
typedef INDEX(*FastFunc)(vector<ConstantSP>& arguments, const ConstantSP& result, INDEX outputStart, bool validate, INDEX inputStart, INDEX inputLen);
typedef ConstantSP(*TemplateOptr)(const ConstantSP&,const ConstantSP&,const string&, OptrFunc, FastFunc, int);
typedef ConstantSP(*TemplateUserOptr)(Heap* heap, const ConstantSP&,const ConstantSP&, const FunctionDefSP&, int);
typedef void (*SysProc)(Heap* heap,vector<ConstantSP>& arguments);
typedef std::function<void (StatementSP)> CFGTraversalFunc;
typedef Statement*(*StatementFunc)(Session* session, const DataInputStreamSP& buffer);
typedef ObjectSP(*ObjectFunc)(const SQLContextSP& context, Session* session, const DataInputStreamSP& buffer);
typedef ConstantSP(*SysObjFunc)(Session* session, const DataInputStreamSP& buffer);
typedef OOClassSP(*ClassFunc)(const string& qualifier, const string& name);
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
	inline bool canManageCatalog(const string& catalog) const { return accessCatalogRule((permissionFlag_ & 16), "DM_", "DDM_", catalog); }
	inline bool canManageDatabase(const string& name) const { return accessDBRule((permissionFlag_ & 16), "DM_", "DDM_", name); }
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
	bool canReadTableInCatalog(const string& catalog) const { return accessCatalogRule(canReadTable(), "RT_", "DRT_", catalog); }
	bool canInsertTableInCatalog(const string& catalog) const { return accessCatalogRule(canInsertTable(), "IT_", "DIT_", catalog); }
	bool canUpdateTableInCatalog(const string& catalog) const { return accessCatalogRule(canUpdateTable(), "UT_", "DUT_", catalog); }
	bool canDeleteTableInCatalog(const string& catalog) const { return accessCatalogRule(canDeleteTable(), "DT_", "DDT_", catalog); }
	bool canReadView(const string& viewName) const { return accessViewRule(canUseView(), viewName); }
	bool canCreateDBObject(const string& name) const { return accessDBRule(canCreateDBObject(), "CD_", "DCD_", name, ""); }
	bool canDeleteDBObject(const string& name) const { return accessDBRule(canDeleteDBObject(), "DD_", "DDD_", name, ""); }
	bool canExecGroup(const string& group) const;
	bool canAccessSensitiveCol(const string& tableUrl, const string& colName) const;
	// return 0 if no limit
	long long queryResultMemLimit() { return queryResultMemLimit_; }
	long long taskGroupMemLimit() { return taskGroupMemLimit_; }
	long long maxPartitionPerQuery() { return maxPartitionPerQuery_; }
	bool isExpired() const {return expired_;}
	void expire();

    static AuthenticatedUserSP createAdminUser();
    static AuthenticatedUserSP createGuestUser();

private:
	bool accessCatalogRule(bool global, const char* prefix, const char* denyPrefix, const string& catName) const;
	bool accessDBRule(bool global, const char* prefix, const char* denyPrefix, const string& objName, const char* objPrefix = "$DB$") const;
	bool accessTableRule(bool global, const char* prefix, const char* denyPrefix, const string& tableName) const;
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

class SWORDFISH_API ByteArrayCodeBuffer{
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
	inline IO_ERR write(const Guid &val){ return write((const char*)val.bytes(), 16);}
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

class SWORDFISH_API SymbolBase{
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

class SWORDFISH_API SymbolBaseManager{
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

class SWORDFISH_API Object {
public:
	Object(OBJECT_TYPE type) : objType_(type){}
	inline OBJECT_TYPE getObjectType() const { return objType_;}
	bool isConstant() const {return objType_ == OBJECT_TYPE::CONSTOBJ;}
	bool isVariable() const {return objType_ == OBJECT_TYPE::VAR;}
	virtual ConstantSP getValue(Heap* pHeap) = 0;
	virtual ConstantSP getReference(Heap* pHeap) = 0;
    virtual void getReference(Heap* pHeap, Constant*& ptr, ConstantSP& ref) {ptr = nullptr; ref = getReference(pHeap);}
	virtual ~Object(){}
	virtual string getScript() const = 0;
	virtual string getScript(Heap* pHeap) const { return getScript();}
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
	virtual void collectUserDefinedFunctionsAndClasses(Heap* pHeap, unordered_map<string,FunctionDef*>& functionDefs, unordered_map<string,OOClass*>& classes) const {
		collectUserDefinedFunctions(functionDefs);
	}
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
	virtual void collectObjects(vector<const Object*>& vec) const {}

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

class SWORDFISH_API Constant: public Object{
public:
	/**
	 * @brief An empty DolphinString.
	 */
	static DolphinString DEMPTY;
	/**
	 * @brief An empty string.
	 */
	static string EMPTY;
	/**
	 * @brief The value of this string is "NULL".
	 */
	static string NULL_STR;

	Constant() : Object(OBJECT_TYPE::CONSTOBJ), flag_(3){}
	Constant(unsigned short flag) : Object(OBJECT_TYPE::CONSTOBJ), flag_(flag){}
	Constant(DATA_FORM df, DATA_TYPE dt, DATA_CATEGORY dc) : Object(OBJECT_TYPE::CONSTOBJ), flag_(3 + (df<<8) + (dt<<16) + (dc<<24)){}
	virtual ~Constant(){}
	/**
	 * @brief Return whether this constant is temporary or not.
	 * 		  The value of a temporary constant may be changed by other function.
	 * 		  Constant is set to temporary by default.
	 */
	inline bool isTemporary() const {return flag_ & 1;}
	/**
	 * @brief Set the constant to be temporary or not.
	 */
	inline void setTemporary(bool val){ if(val) flag_ |= 1; else flag_ &= ~1;}
	/**
	 * @brief Return whether this constant is independent or not.
	 * 		  A none independent constant means contained by other constants, e.g. A Vector contained by Table.
	 * 		  Constant is set to independent by default.
	 */
	inline bool isIndependent() const {return flag_ & 2;}
	/**
	 * @brief Set the constant to be independent or not.
	 */
	inline void setIndependent(bool val){ if(val) flag_ |= 2; else flag_ &= ~2;}
	/**
	 * @brief Return whether this constant is read-only or not.
	 * 		  The value of a read-only constant will not be changed.
	 * 		  Constant is not read-only by default.
	 */
	inline bool isReadOnly() const {return flag_ & 4;}
	/**
	 * @brief Set the constant to be read-only or not.
	 */
	inline void setReadOnly(bool val){ if(val) flag_ |= 4; else flag_ &= ~4;}
	/**
	 * @brief Return whether this constant is a read-only argument or not.
	 * 		  Read only argument can't be applied to mutable function.
	 * 		  Constant is not set to be a read-only argument by default
	 */
	inline bool isReadOnlyArgument() const {return flag_ & 8;}
	/**
	 * @brief Set the constant to be a read-only argument or not.
	 */
	inline void setReadOnlyArgument(bool val){ if(val) flag_ |= 8; else flag_ &= ~8;}
	/**
	 * @brief Return whether this constant is nothing or not.
	 * 		  A nothing constant is used as a default argument to function in most cases.
	 * 		  Only a void scalar will be set to nothing by default.
	 */
	inline bool isNothing() const {return flag_ & 16;}
	/**
	 * @brief Set the constant to be nothing or not.
	 */
	inline void setNothing(bool val){ if(val) flag_ |= 16; else flag_ &= ~16;}
	/**
	 * @brief Return whether this constant is static.
	 * 		  Static means initialized by constant in sql, so its value cannot be changed.
	 * 		  Constant is not static by default
	 */
	inline bool isStatic() const {return flag_ & 32;}
	/**
	 * @brief Set the static flag.
	 */
	inline void setStatic(bool val){ if(val) flag_ |= 32; else flag_ &= ~32;}
	/**
	 * @brief Return the transferAsString flag.
	 * 		  TransferAsString flag is to boost the performance of serializing when appending data to a remote chunk.
	 */
	inline bool transferAsString() const {return flag_ & 64;}
	/**
	 * @brief Set the transferAsString flag.
	 */
	inline void transferAsString(bool val){ if(val) flag_ |= 64; else flag_ &= ~64;}
	/**
	 * @brief Return whether this constant can be accessed by multiple threads or not.
	 * 		  A synchronized constant must be accessed while holding the lock.
	 * 		  Constant is not synchronized by default.
	 */
	inline bool isSynchronized() const {return flag_ & 128;}
	/**
	 * @brief Set the synchronized flag.
	 */
	inline void setSynchronized(bool val){ if(val) flag_ |= 128; else flag_ &= ~128;}
    /**
	 * @brief Return whether this constant is an object oriented instance.
	 * 		  Constant is not an object oriented instance by default.
	 */
	inline bool isOOInstance() const {return flag_ & 4096;}
	/**
	 * @brief Set the object oriented instance flag.
	 */
	inline void setOOInstance(bool val){ if(val) flag_ |= 4096; else flag_ &= ~4096;}
    /**
	 * @brief Return whether this constant has an index or not.
	 * 		  Constant is not indexed by default.
	 */
	inline bool isIndexed() const {return flag_ & 8192;}
	/**
	 * @brief Set the indexed flag.
	 */
	inline void setIndexed(bool val){ if(val) flag_ |= 8192; else flag_ &= ~8192;}
	/**
	 * @brief Return whether this constant has an index or not.
	 * 		  Series means an indexed single-column matrix.
	 * 		  Constant is not series by default.
	 */
	inline bool isSeries() const {return flag_ & 16384;}
	/**
	 * @brief Set the series flag.
	 */
	inline void setSeries(bool val){ if(val) flag_ |= 16384; else flag_ &= ~16384;}
	/**
	 * @brief Return whether this constant is transient or not.
	 * 		  A copy must be obtained through getValue() to use transient constant.
	 * 		  Constant is not transient by default.
	 */
	inline bool isTransient() const {return flag_ & 32768;}
	/**
	 * @brief set the transient flag.
	 */
	inline void setTransient(bool val){ if(val) flag_ |= 32768; else flag_ &= ~32768;}
	/**
	 * @brief Return the DATA_FORM of this constant.
	 */
	inline DATA_FORM getForm() const {return DATA_FORM((flag_ >> 8) & 15);}
	/**
	 * @brief Set the DATA_FORM of this constant.
	 */
	inline void setForm(DATA_FORM df){ flag_ = (flag_ & 4294963455U) + (df << 8);}
	/**
	 * @brief Return the DATA_TYPE of this constant.
	 */
	inline DATA_TYPE getType() const {return DATA_TYPE((flag_ >> 16) & 255);}
	/**
	 * @brief Return the DATA_CATEGORY of this constant.
	 */
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
	/**
	 * @brief Return whether this constant is array.
	 */
	inline bool isArray() const { return getForm()==DF_VECTOR;}
	/**
	 * @brief Return whether this constant is pair.
	 */
	inline bool isPair() const { return getForm()==DF_PAIR;}
	/**
	 * @brief Return whether this constant is matrix.
	 */
	inline bool isMatrix() const {return getForm()==DF_MATRIX;}
	/**
	 * @brief Return whether this constant is vector.
	 * 		  A vector could be array, pair or matrix.
	 */
	inline bool isVector() const { DATA_FORM df = getForm();return df>=DF_VECTOR && df<=DF_MATRIX;}
	/**
	 * @brief Return whether this constant is table.
	 */
	inline bool isTable() const { return getForm()==DF_TABLE;}
	/**
	 * @brief Return whether this constant is set.
	 */
	inline bool isSet() const {return getForm()==DF_SET;}
	/**
	 * @brief Return whether this constant is dictionary.
	 */
	inline bool isDictionary() const {return getForm()==DF_DICTIONARY;}
	/**
	 * @brief Return whether this constant is chunk.
	 */
	inline bool isChunk() const {return getForm()==DF_CHUNK;}
	/**
	 * @brief Return whether this constant is system object.
	 */
	inline bool isSysObj() const {return getForm()==DF_SYSOBJ;}
	/**
	 * @brief Return whether this constant is tuple.
	 * 		  A tuple must be a any vector.
	 */
	bool isTuple() const {return getForm()==DF_VECTOR && getType()==DT_ANY;}
	/**
	 * @brief Return whether this constant is number.
	 * 		  A number could be integral, floating or denary.
	 */
	bool isNumber() const { DATA_CATEGORY cat = getCategory(); return cat == INTEGRAL || cat == FLOATING || cat == DENARY; }
	/**
	 * @brief Get the iterator of this constant itself.
	 */
	virtual ConstantSP getIterator(const ConstantSP& self) const {throw RuntimeException("getIterator method not supported.");}
	/**
	 * @brief Return the next item. If the iterator has reached the end, it returns a null pointer, i.e. item.isNull() is true.
	 */
	virtual ConstantSP next() {throw RuntimeException("next method not supported.");}
	virtual uint64_t hash() const { return (uint64_t)this;}
	virtual bool equal(const ConstantSP& other) const { return this == other.get();}
	/**
	 * @brief Return whether this constant is a database handle.
	 */
	virtual bool isDatabase() const {return false;}
	virtual ObjectSP deepCopy() const { return getValue();}

	/**
	 * @brief Return the bool value of this constant.
	 */
	virtual char getBool() const {throw RuntimeException("The object can't be converted to boolean scalar.");}
	/**
	 * @brief Return the char value of this constant.
	 */
	virtual char getChar() const {throw RuntimeException("The object can't be converted to char scalar.");}
	/**
	 * @brief Return the short value of this constant.
	 */
	virtual short getShort() const {throw RuntimeException("The object can't be converted to short scalar.");}
	/**
	 * @brief Return the int value of this constant.
	 */
	virtual int getInt() const {throw RuntimeException("The object can't be converted to int scalar.");}
	/**
	 * @brief Return the long value of this constant.
	 */
	virtual long long  getLong() const {throw RuntimeException("The object can't be converted to long scalar.");}
	/**
	 * @brief Return the index value of this constant.
	 */
	virtual INDEX getIndex() const {throw RuntimeException("The object can't be converted to index scalar.");}
	/**
	 * @brief Return the float value of this constant.
	 */
	virtual float getFloat() const {throw RuntimeException("The object can't be converted to float scalar.");}
	/**
	 * @brief Return the double value of this constant.
	 */
	virtual double getDouble() const {throw RuntimeException("The object can't be converted to double scalar.");}
	/**
	 * @brief Get the string version of this constant.
	 * 		  This function is very helpful for debugging.
	 */
	virtual string getString() const {return "";}
	/**
	 * @brief Get the string version of this constant according to the session type in the heap.
	 * 		  This function is very helpful for debugging.
	 *
	 * @param heap: A heap indicate different forms of results.
	 */
	virtual string getString(Heap* heap) const {return getString();}
	/**
	 * @brief Return a description of this constant.
	 */
	virtual string getScript() const { return getString();}
	/**
	 * @brief Return a description of this constant according to the session type in the heap.
	 *
	 * @param heap: A heap indicate different forms of results.
	 */
	virtual string getScript(Heap* heap) const { return getScript();}
	/**
	 * @brief Return a dolphinString reference of this constant
	 */
	virtual const DolphinString& getStringRef() const {return DEMPTY;}
	/**
	 * @brief Return the guid value of this constant.
	 */
    virtual const Guid getInt128() const {throw RuntimeException("The object can't be converted to int128 scalar.");}
	/**
	 * @brief Get the binary data of a int128 scalar.
	 * @return A pointer to the binary data.
	 */
    virtual const unsigned char* getBinary() const {throw RuntimeException("The object can't be converted to int128 scalar.");}
	/**
	 * @brief Return whether this constant is null.
	 */
	virtual bool isNull() const {return false;}

	virtual int getDecimal32(int scale) const { NOT_IMPLEMENT; }
	virtual long long getDecimal64(int scale) const { NOT_IMPLEMENT; }
	virtual int128 getDecimal128(int scale) const { NOT_IMPLEMENT; }

	/**
	 * @brief Set the bool value to this constant.
	 *
	 * @param val: The value to be set.
	 */
	virtual void setBool(char val){}
	/**
	 * @brief Set the char value to this constant.
	 *
	 * @param val: The value to be set.
	 */
	virtual void setChar(char val){}
	/**
	 * @brief Set the short value to this constant.
	 *
	 * @param val: The value to be set.
	 */
	virtual void setShort(short val){}
	/**
	 * @brief Set the int value to this constant.
	 *
	 * @param val: The value to be set.
	 */
	virtual void setInt(int val){}
	/**
	 * @brief Set the long value to this constant.
	 *
	 * @param val: The value to be set.
	 */
	virtual void setLong(long long val){}
	/**
	 * @brief Set the index value to this constant.
	 *
	 * @param val: The value to be set.
	 */
	virtual void setIndex(INDEX val){}
	/**
	 * @brief Set the float value to this constant.
	 *
	 * @param val: The value to be set.
	 */
	virtual void setFloat(float val){}
	/**
	 * @brief Set the double value to this constant.
	 *
	 * @param val: The value to be set.
	 */
	virtual void setDouble(double val){}
	/**
	 * @brief Set the DolphinString value to this constant.
	 *
	 * @param val: The value to be set.
	 */
	virtual void setString(const DolphinString& val){}
	/**
	 * @brief Set the binary data to Constant.
	 * 		  Note that this interface is only implemented in int128 scalar.
	 *
	 * @param val: A pointer to the binary data.
	 * @param unitLength: The length of the data type of this constant, but not actually used.
	 */
	virtual void setBinary(const unsigned char* val, int unitLength){}
	/**
	 * @brief Set null value to this constant.
	 */
	virtual void setNull(){}

	/**
	 * @brief Get the bool value of the index-th element in this constant.
	 * 		  Note that index should be valid, otherwise out-of-bounds access will occur.
	 *
	 * @param index: The index-th element to get.
	 */
	virtual char getBool(INDEX index) const {return getBool();}
	/**
	 * @brief Get the char value of the index-th element in this constant.
	 * 		  Note that index should be valid, otherwise out-of-bounds access will occur.
	 *
	 * @param index: The index-th element to get.
	 */
	virtual char getChar(INDEX index) const { return getChar();}
	/**
	 * @brief Get the short value of the index-th element in this constant.
	 * 		  Note that index should be valid, otherwise out-of-bounds access will occur.
	 *
	 * @param index: The index-th element to get.
	 */
	virtual short getShort(INDEX index) const { return getShort();}
	/**
	 * @brief Get the int value of the index-th element in this constant.
	 * 		  Note that index should be valid, otherwise out-of-bounds access will occur.
	 *
	 * @param index: The index-th element to get.
	 */
	virtual int getInt(INDEX index) const {return getInt();}
	/**
	 * @brief Get the long value of the index-th element in this constant.
	 * 		  Note that index should be valid, otherwise out-of-bounds access will occur.
	 *
	 * @param index: The index-th element to get.
	 */
	virtual long long getLong(INDEX index) const {return getLong();}
	/**
	 * @brief Get the index value of the index-th element in this constant.
	 * 		  Note that index should be valid, otherwise out-of-bounds access will occur.
	 *
	 * @param index: The index-th element to get.
	 */
	virtual INDEX getIndex(INDEX index) const {return getIndex();}
	/**
	 * @brief Get the float value of the index-th element in this constant.
	 * 		  Note that index should be valid, otherwise out-of-bounds access will occur.
	 *
	 * @param index: The index-th element to get.
	 */
	virtual float getFloat(INDEX index) const {return getFloat();}
	/**
	 * @brief Get the double value of the index-th element in this constant.
	 * 		  Note that index should be valid, otherwise out-of-bounds access will occur.
	 *
	 * @param index: The index-th element to get.
	 */
	virtual double getDouble(INDEX index) const {return getDouble();}
	/**
	 * @brief Get the string value of the index-th element in this constant.
	 * 		  Note that index should be valid, otherwise out-of-bounds access will occur.
	 *
	 * @param index: The index-th element to get.
	 */
	virtual string getString(INDEX index) const {return getString();}
	/**
	 * @brief Get a dolphinString reference of the index-th element in this constant.
	 * 		  Note that index should be valid, otherwise out-of-bounds access will occur.
	 *
	 * @param index: The index-th element to get.
	 */
	virtual const DolphinString& getStringRef(INDEX index) const {return DEMPTY;}
	/**
	 * @brief Get the guid value of the index-th element in this constant.
	 * 		  Note that index should be valid, otherwise out-of-bounds access will occur.
	 *
	 * @param index: The index-th element to get.
	 */
    virtual const Guid getInt128(INDEX index) const {return getInt128();}
	/**
	 * @brief Get the binary data of the index-th element in this constant.
	 * 		  Note that index should be valid, otherwise out-of-bounds access will occur.
	 *
	 * @param index: The index-th element to get.
	 * @return A pointer to the binary data of index-th element.
	 */
    virtual const unsigned char* getBinary(INDEX index) const {return getBinary();}
	/**
	 * @brief Return whether the index-th element is null.
	 */
	virtual bool isNull(INDEX index) const {return isNull();}

	/**
	 * @brief Get the decimal32 data in specific scale of the index-th element in this constant.
	 * 		  Note that index should be valid, otherwise out-of-bounds access will occur.
	 *
	 * @param index: The index-th element to get.
	 * @param scale: Fractional digits.
	 * @return Integer representation of decimal32.
	 */
	virtual int getDecimal32(INDEX index, int scale) const { NOT_IMPLEMENT; }
    /**
	 * @brief Get the decimal64 data in specific scale of the index-th element in this constant.
	 * 		  Note that index should be valid, otherwise out-of-bounds access will occur.
	 *
	 * @param index: The index-th element to get.
	 * @param scale: Fractional digits.
	 * @return Integer representation of decimal64.
	 */
	virtual long long getDecimal64(INDEX index, int scale) const { NOT_IMPLEMENT; }
	virtual int128 getDecimal128(INDEX index, int scale) const { NOT_IMPLEMENT; }

	/**
	 * @brief Get the data of index-th element in this constant.
	 * 		  Note that index should be valid, otherwise out-of-bounds access will occur.
	 *
	 * @param index: The index-th element to get.
	 * @return ConstantSP: The data.
	 */
	virtual ConstantSP get(INDEX index) const {return getValue();}
	/**
	 * @brief Get the data according to row and column index in a table or matrix.
	 * 		  Note that both column and row index should be valid, otherwise out-of-bounds access will occur.
	 *
	 * @param column: column index.
	 * @param row: row index.
	 * @return ConstantSP: The data.
	 */
	virtual ConstantSP get(INDEX column, INDEX row) const {return get(row);}
	/**
	 * @brief Get the data according to the index.
	 *
	 * @param index: The index gives the indices of data to get. If the index is out of range,
	 * 		i.e. negative or larger than the size, the null value is returned correspondingly.
	 * @return ConstantSP: The data.
	 */
	virtual ConstantSP get(const ConstantSP& index) const {return getValue();}
	/**
	 * @brief Get the data according to the index.
	 *
	 * @param offset: The index is offseted by offset.
	 * @param index: The index gives the indices of data to get.
	 * @return ConstantSP: The data.
	 */
	virtual ConstantSP get(INDEX offset, const ConstantSP& index) const {return getValue();}
	/**
	 * @brief Get the data of the specified column in a table or matrix according to index.
	 * 		  Note that index should be valid, otherwise out-of-bounds access will occur.
	 *
	 * @param index: Column index.
	 * @return ConstantSP: The data.
	 */
	virtual ConstantSP getColumn(INDEX index) const {return getValue();}
	/**
	 * @brief Get the data of the specified row in a matrix according to index.
	 * 		  Note that index should be valid, otherwise out-of-bounds access will occur.
	 *
	 * @param index: Row index.
	 * @return ConstantSP: The data.
	 */
	virtual ConstantSP getRow(INDEX index) const {return get(index);}
	/**
	 * @brief Get the data of the specified item according to the index.
	 * 		  Note that the index-th column will be returned in a table or matrix, and index should be valid.
	 *
	 * @param index: item index.
	 * @return ConstantSP: The data.
	 */
	virtual ConstantSP getItem(INDEX index) const {return get(index);}
	/**
	 * @brief Get the data of the specified items according to the index.
	 * 		  If the index is out of range, the null value is returned correspondingly.
	 * 		  Note that the indices column will be returned in a table or matrix.
	 *
	 * @param index: The index gives the indices of data to get.
	 * @return ConstantSP: The data.
	 */
	virtual ConstantSP getItems(const ConstantSP& index) const {return get(index);}
	/**
	 * @brief Get a sub-table from this constant.
	 * 		  Note that the sub-table is a copy of the original table.
	 *
	 * @param colStart: index of start column, should be valid.
	 * @param colLength: column length. Can be negative, indicating the reverse length.
	 * @param rowStart: index of start row, should be valid.
	 * @param rowLength: row length. Can be negative, indicating the reverse length.
	 * @return ConstantSP: The sub-table.
	 */
	virtual ConstantSP getWindow(INDEX colStart, int colLength, INDEX rowStart, int rowLength) const {return getValue();}
	/**
	 * @brief Get the sliced data according to the rowIndex and colIndex.
	 * 		  If the index is out of range, the null value is returned correspondingly.
	 *
	 * @param rowIndex: The rowIndex gives the indices of rows to get.
	 * @param colIndex: The colIndex gives the indices of columns to get.
	 * @return ConstantSP: The sliced data.
	 */
	virtual ConstantSP getSlice(const ConstantSP& rowIndex, const ConstantSP& colIndex) const {throw RuntimeException("getSlice method not supported");}
	/**
	 * @brief Get the row lable from a matrix.
	 */
	virtual ConstantSP getRowLabel() const;
	/**
	 * @brief Get the column lable from a matrix.
	 */
	virtual ConstantSP getColumnLabel() const;

	/**
	 * @brief Judge the data from start to (start + len - 1) is null or not
	 *
	 * @param start: The start index.
	 * @param len: The length of data to be judged.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool isNull(INDEX start, int len, char* buf) const {return false;}
	/**
	 * @brief Judge the data from start to (start + len - 1) is valid or not.
	 * 		  All the param and return is the same as in isNull.
	 *
	 * @param start: The start index.
	 * @param len: The length of data to be judged.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool isValid(INDEX start, int len, char* buf) const {return false;}
	/**
	 * @brief Get the boolean data from start to (start + len - 1).
	 *
	 * @param start: The start index.
	 * @param len: The length of data to be retrieved.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool getBool(INDEX start, int len, char* buf) const {return false;}
	/**
	 * @brief Get the character data from start to (start + len - 1).
	 *
	 * @param start: The start index.
	 * @param len: The length of data to be retrieved.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool getChar(INDEX start, int len,char* buf) const {return false;}
	/**
	 * @brief Get the short value of data from start to (start + len - 1).
	 *
	 * @param start: The start index.
	 * @param len: The length of data to be retrieved.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool getShort(INDEX start, int len, short* buf) const {return false;}
	/**
	 * @brief Get the int value of data from start to (start + len - 1).
	 *
	 * @param start: The start index.
	 * @param len: The length of data to be retrieved.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool getInt(INDEX start, int len, int* buf) const {return false;}
	/**
	 * @brief Get the long long value of data from start to (start + len - 1).
	 *
	 * @param start: The start index.
	 * @param len: The length of data to be retrieved.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool getLong(INDEX start, int len, long long* buf) const {return false;}
	/**
	 * @brief Get the index value of data from start to (start + len - 1).
	 *
	 * @param start: The start index.
	 * @param len: The length of data to be retrieved.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool getIndex(INDEX start, int len, INDEX* buf) const {return false;}
	/**
	 * @brief Get the float value of data from start to (start + len - 1).
	 *
	 * @param start: The start index.
	 * @param len: The length of data to be retrieved.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool getFloat(INDEX start, int len, float* buf) const {return false;}
	/**
	 * @brief Get the double value of data from start to (start + len - 1).
	 *
	 * @param start: The start index.
	 * @param len: The length of data to be retrieved.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool getDouble(INDEX start, int len, double* buf) const {return false;}
	/**
	 * @brief Get the symbol data from start to (start + len - 1).
	 *
	 * @param start: The start index.
	 * @param len: The length of data to be retrieved.
	 * @param buf: The result is stored in buf.
	 * @param symBase: symbolBase is a mapping from string to int.
	 * @param insertIfNotThere: Should a string that is not found in Symbase be inserted into Symbase.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool getSymbol(INDEX start, int len, int* buf, SymbolBase* symBase,bool insertIfNotThere) const {return false;}
	/**
	 * @brief Get the DolphinString data from start to (start + len - 1).
	 *
	 * @param start: The start index.
	 * @param len: The length of data to be retrieved.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool getString(INDEX start, int len, DolphinString** buf) const {return false;}
	/**
	 * @brief Get the string data from start to (start + len - 1).
	 *
	 * @param start: The start index.
	 * @param len: The length of data to be retrieved.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool getString(INDEX start, int len, char** buf) const {return false;}
	/**
	 * @brief Get the binary data from start to (start + len - 1).
	 *
	 * @param start: The start index.
	 * @param len: The length of data to be retrieved.
	 * @param unitLength: The length of the data type of this constant.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool getBinary(INDEX start, int len, int unitLength, unsigned char* buf) const {return false;}

	/**
	 * @brief Get the decimal32 data in specific scale from start to (start + len - 1).
	 *
	 * @param start: The start index.
	 * @param len: The length of data to be retrieved.
	 * @param scale: Fractional digits.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool getDecimal32(INDEX start, int len, int scale, int *buf) const { NOT_IMPLEMENT; }
	/**
	 * @brief Get the decimal64 data in specific scale from start to (start + len - 1).
	 *
	 * @param start: The start index.
	 * @param len: The length of data to be retrieved.
	 * @param scale: Fractional digits.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool getDecimal64(INDEX start, int len, int scale, long long *buf) const { NOT_IMPLEMENT; }
	virtual bool getDecimal128(INDEX start, int len, int scale, int128 *buf) const { NOT_IMPLEMENT; }

	/**
	 * @brief Judge the data according to indices is null or not.
	 *
	 * @param indices: The maybe out-of-ordered indices to judge.
	 * @param len: The length of data to be judged.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool isNull(INDEX* indices, int len, char* buf) const {return false;}
	/**
	 * @brief Judge the data according to indices is valid or not.
	 *
	 * @param indices: The maybe out-of-ordered indices to judge.
	 * @param len: The length of data to be judged.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool isValid(INDEX* indices, int len, char* buf) const {return false;}
	/**
	 * @brief Get the boolean data according to indices.
	 *
	 * @param indices: The maybe out-of-ordered indices to retrieve.
	 * @param len: The length of data to be retrieved.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool getBool(INDEX* indices, int len, char* buf) const {return false;}
	/**
	 * @brief Get the character data according to indices.
	 *
	 * @param indices: The maybe out-of-ordered indices to retrieve.
	 * @param len: The length of data to be retrieved.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool getChar(INDEX* indices, int len,char* buf) const {return false;}
	/**
	 * @brief Get the short value of data according to indices.
	 *
	 * @param indices: The maybe out-of-ordered indices to retrieve.
	 * @param len: The length of data to be retrieved.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool getShort(INDEX* indices, int len, short* buf) const {return false;}
	/**
	 * @brief Get the int value of data according to indices.
	 *
	 * @param indices: The maybe out-of-ordered indices to retrieve.
	 * @param len: The length of data to be retrieved.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool getInt(INDEX* indices, int len, int* buf) const {return false;}
	/**
	 * @brief Get the long long value of data according to indices.
	 *
	 * @param indices: The maybe out-of-ordered indices to retrieve.
	 * @param len: The length of data to be retrieved.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool getLong(INDEX* indices, int len, long long* buf) const {return false;}
	/**
	 * @brief Get the index value of data according to indices.
	 *
	 * @param indices: The maybe out-of-ordered indices to retrieve.
	 * @param len: The length of data to be retrieved.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool getIndex(INDEX* indices, int len, INDEX* buf) const {return false;}
	/**
	 * @brief Get the float value of data according to indices.
	 *
	 * @param indices: The maybe out-of-ordered indices to retrieve.
	 * @param len: The length of data to be retrieved.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool getFloat(INDEX* indices, int len, float* buf) const {return false;}
	/**
	 * @brief Get the double value of data according to indices.
	 *
	 * @param indices: The maybe out-of-ordered indices to retrieve.
	 * @param len: The length of data to be retrieved.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool getDouble(INDEX* indices, int len, double* buf) const {return false;}
	/**
	 * @brief Get the symbol data according to indices.
	 *
	 * @param start: The start index.
	 * @param len: The length of data to be retrieved.
	 * @param buf: The result is stored in buf.
	 * @param symBase: SymbolBase is a mapping from string to int.
	 * @param insertIfNotThere: Should a string that is not found in Symbase be inserted into Symbase.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool getSymbol(INDEX* indices, int len, int* buf, SymbolBase* symBase,bool insertIfNotThere) const {return false;}
	/**
	 * @brief Get the DolphinString data according to indices.
	 *
	 * @param indices: The maybe out-of-ordered indices to retrieve.
	 * @param len: The length of data to be retrieved.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool getString(INDEX* indices, int len, DolphinString** buf) const {return false;}
	/**
	 * @brief Get the string data according to indices.
	 *
	 * @param indices: The maybe out-of-ordered indices to retrieve.
	 * @param len: The length of data to be retrieved.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool getString(INDEX* indices, int len, char** buf) const {return false;}
	/**
	 * @brief Get the binary data according to indices.
	 *
	 * @param indices: The maybe out-of-ordered indices to retrieve.
	 * @param len: The length of data to be retrieved.
	 * @param unitLength: The length of the data type of this constant.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool getBinary(INDEX* indices, int len, int unitLength, unsigned char* buf) const {return false;}

	/**
	 * @brief Get the decimal32 data in specific scale according to indices.
	 *
	 * @param indices: The maybe out-of-ordered indices to retrieve.
	 * @param len: The length of data to be retrieved.
	 * @param scale: Fractional digits.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool getDecimal32(INDEX *indices, int len, int scale, int *buf) const { NOT_IMPLEMENT; }
	/**
	 * @brief Get the decimal64 data in specific scale according to indices.
	 *
	 * @param indices: The maybe out-of-ordered indices to retrieve.
	 * @param len: The length of data to be retrieved.
	 * @param scale: Fractional digits.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool getDecimal64(INDEX *indices, int len, int scale, long long *buf) const { NOT_IMPLEMENT; }
	virtual bool getDecimal128(INDEX *indices, int len, int scale, int128 *buf) const { NOT_IMPLEMENT; }

	/**
	 * @brief Get the boolean data from start to (start + len - 1).
	 * 		  This is the recommended method to view/iterate data in Constant.
	 * 		  Note that if the required underlying data is contiguous, then there is
	 *			no copy happened in this function and the underlying buffer is directly returned;
	 * 			otherwise, the data is copied into buf, and buf is returned.
	 *
	 * @param start: The start index.
	 * @param len: The length of data to get.
	 * @param buf: A buffer with at least length len.
	 * @return A buffer with the data required.
	 */
	virtual const char* getBoolConst(INDEX start, int len, char* buf) const {throw RuntimeException("getBoolConst method not supported");}
	/**
	 * @brief Get the char value from start to (start + len - 1).
	 * 		  This is the recommended method to view/iterate data in Constant.
	 * 		  Note that if the required underlying data is contiguous, then there is
	 *			no copy happened in this function and the underlying buffer is directly returned;
	 * 			otherwise, the data is copied into buf, and buf is returned.
	 *
	 * @param start: The start index.
	 * @param len: The length of data to get.
	 * @param buf: A buffer with at least length len.
	 * @return A buffer with the data required.
	 */
	virtual const char* getCharConst(INDEX start, int len,char* buf) const {throw RuntimeException("getCharConst method not supported");}
	/**
	 * @brief Get the short value from start to (start + len - 1).
	 * 		  This is the recommended method to view/iterate data in Constant.
	 * 		  Note that if the required underlying data is contiguous, then there is
	 *			no copy happened in this function and the underlying buffer is directly returned;
	 * 			otherwise, the data is copied into buf, and buf is returned.
	 *
	 * @param start: The start index.
	 * @param len: The length of data to get.
	 * @param buf: A buffer with at least length len.
	 * @return A buffer with the data required.
	 */
	virtual const short* getShortConst(INDEX start, int len, short* buf) const {throw RuntimeException("getShortConst method not supported");}
	/**
	 * @brief Get the int value from start to (start + len - 1).
	 * 		  This is the recommended method to view/iterate data in Constant.
	 * 		  Note that if the required underlying data is contiguous, then there is
	 *			no copy happened in this function and the underlying buffer is directly returned;
	 * 			otherwise, the data is copied into buf, and buf is returned.
	 *
	 * @param start: The start index.
	 * @param len: The length of data to get.
	 * @param buf: A buffer with at least length len.
	 * @return A buffer with the data required.
	 */
	virtual const int* getIntConst(INDEX start, int len, int* buf) const {throw RuntimeException("getIntConst method not supported");}
	/**
	 * @brief Get the long long value from start to (start + len - 1).
	 * 		  This is the recommended method to view/iterate data in Constant.
	 * 		  Note that if the required underlying data is contiguous, then there is
	 *			no copy happened in this function and the underlying buffer is directly returned;
	 * 			otherwise, the data is copied into buf, and buf is returned.
	 *
	 * @param start: The start index.
	 * @param len: The length of data to get.
	 * @param buf: A buffer with at least length len.
	 * @return A buffer with the data required.
	 */
	virtual const long long* getLongConst(INDEX start, int len, long long* buf) const {throw RuntimeException("getLongConst method not supported");}
	/**
	 * @brief Get the index value from start to (start + len - 1).
	 * 		  This is the recommended method to view/iterate data in Constant.
	 * 		  Note that if the required underlying data is contiguous, then there is
	 *			no copy happened in this function and the underlying buffer is directly returned;
	 * 			otherwise, the data is copied into buf, and buf is returned.
	 *
	 * @param start: The start index.
	 * @param len: The length of data to get.
	 * @param buf: A buffer with at least length len.
	 * @return A buffer with the data required.
	 */
	virtual const INDEX* getIndexConst(INDEX start, int len, INDEX* buf) const {throw RuntimeException("getIndexConst method not supported");}
	/**
	 * @brief Get the float value start to (start + len - 1).
	 * 		  This is the recommended method to view/iterate data in Constant.
	 * 		  Note that if the required underlying data is contiguous, then there is
	 *			no copy happened in this function and the underlying buffer is directly returned;
	 * 			otherwise, the data is copied into buf, and buf is returned.
	 *
	 * @param start: The start index.
	 * @param len: The length of data to get.
	 * @param buf: A buffer with at least length len.
	 * @return A buffer with the data required.
	 */
	virtual const float* getFloatConst(INDEX start, int len, float* buf) const {throw RuntimeException("getFloatConst method not supported");}
	/**
	 * @brief Get the double value from start to (start + len - 1).
	 * 		  This is the recommended method to view/iterate data in Constant.
	 * 		  Note that if the required underlying data is contiguous, then there is
	 *			no copy happened in this function and the underlying buffer is directly returned;
	 * 			otherwise, the data is copied into buf, and buf is returned.
	 *
	 * @param start: The start index.
	 * @param len: The length of data to get.
	 * @param buf: A buffer with at least length len.
	 * @return A buffer with the data required.
	 */
	virtual const double* getDoubleConst(INDEX start, int len, double* buf) const {throw RuntimeException("getDoubleConst method not supported");}
	/**
	 * @brief Get the symbol data from start to (start + len - 1).
	 * 		  This is the recommended method to view/iterate data in Constant.
	 * 		  Note that if the required underlying data is contiguous, then there is
	 *			no copy happened in this function and the underlying buffer is directly returned;
	 * 			otherwise, the data is copied into buf, and buf is returned.
	 *
	 * @param start: The start index.
	 * @param len: The length of data to get.
	 * @param buf: A buffer with at least length len.
	 * @param symBase: SymbolBase is a mapping from string to int.
	 * @param insertIfNotThere: Should a string that is not found in symbase be inserted into symbase.
	 * @return A buffer with the data required.
	 */
	virtual const int* getSymbolConst(INDEX start, int len, int* buf, SymbolBase* symBase, bool insertIfNotThere) const {throw RuntimeException("getSymbolConst method not supported");}
	/**
	 * @brief Get the DolphinString data from start to (start + len - 1).
	 * 		  This is the recommended method to view/iterate data in Constant.
	 * 		  Note that if the required underlying data is contiguous, then there is
	 *			no copy happened in this function and the underlying buffer is directly returned;
	 * 			otherwise, the data is copied into buf, and buf is returned.
	 *
	 * @param start: The start index.
	 * @param len: The length of data to get.
	 * @param buf: A buffer with at least length len.
	 * @return A buffer with the data required.
	 */
	virtual DolphinString** getStringConst(INDEX start, int len, DolphinString** buf) const {throw RuntimeException("getStringConst method not supported");}
	/**
	 * @brief Get the string data from start to (start + len - 1).
	 * 		  This is the recommended method to view/iterate data in Constant.
	 * 		  Note that if the required underlying data is contiguous, then there is
	 *			no copy happened in this function and the underlying buffer is directly returned;
	 * 			otherwise, the data is copied into buf, and buf is returned.
	 *
	 * @param start: The start index.
	 * @param len: The length of data to get.
	 * @param buf: A buffer with at least length len.
	 * @return A buffer with the data required.
	 */
	virtual char** getStringConst(INDEX start, int len, char** buf) const {throw RuntimeException("getStringConst method not supported");}
	/**
	 * @brief Get the binary data from start to (start + len - 1).
	 * 		  This is the recommended method to view/iterate data in Constant.
	 * 		  Note that if the required underlying data is contiguous, then there is
	 *			no copy happened in this function and the underlying buffer is directly returned;
	 * 			otherwise, the data is copied into buf, and buf is returned.
	 *
	 * @param start: The start index.
	 * @param len: The length of data to get.
	 * @param unitLength: The length of the data type of this constant.
	 * @param buf: A buffer with at least length len*unitLength.
	 * @return A buffer with the data required.
	 */
	virtual const unsigned char* getBinaryConst(INDEX start, int len, int unitLength, unsigned char* buf) const {throw RuntimeException("getBinaryConst method not supported");}

	/**
	 * @brief Get the decimal32 data with specific scale from start to (start + len - 1).
	 * 		  This is the recommended method to view/iterate data in Constant.
	 * 		  Note that if the required underlying data is contiguous, then there is
	 *			no copy happened in this function and the underlying buffer is directly returned;
	 * 			otherwise, the data is copied into buf, and buf is returned.
	 *
	 * @param start: The start index.
	 * @param len: The length of data to get.
	 * @param scale: Fractional digits.
	 * @param buf: A buffer with at least length len.
	 * @return A buffer with the data required.
	 */
	virtual const int* getDecimal32Const(INDEX start, int len, int scale, int *buf) const { NOT_IMPLEMENT; }
	/**
	 * @brief Get the decimal64 data with specific scale from start to (start + len - 1).
	 * 		  This is the recommended method to view/iterate data in Constant.
	 * 		  Note that if the required underlying data is contiguous, then there is
	 *			no copy happened in this function and the underlying buffer is directly returned;
	 * 			otherwise, the data is copied into buf, and buf is returned.
	 *
	 * @param start: The start index.
	 * @param len: The length of data to get.
	 * @param scale: Fractional digits.
	 * @param buf: A buffer with at least length len.
	 * @return A buffer with the data required.
	 */
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
	 * @param start: The start index.
	 * @param len: The length of data to get.
	 * @param buf: A buffer with at least length len.
	 * @return A buffer to write data.
	 */
	virtual char* getBoolBuffer(INDEX start, int len, char* buf) const {return buf;}
	/**
	 * @brief Get a buffer for writing data from start to (start + len - 1).
	 * 		  This is the recommended method to write data in Constant.
	 * 		  This function is always used together with setChar(start, len, buf).
	 * 		  Note that if the required underlying data is contiguous,
	 * 			then the underlying buffer is directly returned;
	 * 			otherwise, the buf is returned.
	 *
	 * @param start: The start index.
	 * @param len: The length of data to get.
	 * @param buf: A buffer with at least length len.
	 * @return A buffer to write data.
	 */
	virtual char* getCharBuffer(INDEX start, int len,char* buf) const {return buf;}
	/**
	 * @brief Get a buffer for writing data from start to (start + len - 1).
	 * 		  This is the recommended method to write data in Constant.
	 * 		  This function is always used together with setShort(start, len, buf).
	 * 		  Note that if the required underlying data is contiguous,
	 * 			then the underlying buffer is directly returned;
	 * 			otherwise, the buf is returned.
	 *
	 * @param start: The start index.
	 * @param len: The length of data to get.
	 * @param buf: A buffer with at least length len.
	 * @return A buffer to write data.
	 */
	virtual short* getShortBuffer(INDEX start, int len, short* buf) const {return buf;}
	/**
	 * @brief Get a buffer for writing data from start to (start + len - 1).
	 * 		  This is the recommended method to write data in Constant.
	 * 		  This function is always used together with setInt(start, len, buf).
	 * 		  Note that if the required underlying data is contiguous,
	 * 			then the underlying buffer is directly returned;
	 * 			otherwise, the buf is returned.
	 *
	 * @param start: The start index.
	 * @param len: The length of data to get.
	 * @param buf: A buffer with at least length len.
	 * @return A buffer to write data.
	 */
	virtual int* getIntBuffer(INDEX start, int len, int* buf) const {return NULL;}
	/**
	 * @brief Get a buffer for writing data from start to (start + len - 1).
	 * 		  This is the recommended method to write data in Constant.
	 * 		  This function is always used together with setLong(start, len, buf).
	 * 		  Note that if the required underlying data is contiguous,
	 * 			then the underlying buffer is directly returned;
	 * 			otherwise, the buf is returned.
	 *
	 * @param start: The start index.
	 * @param len: The length of data to get.
	 * @param buf: A buffer with at least length len.
	 * @return A buffer to write data.
	 */
	virtual long long* getLongBuffer(INDEX start, int len, long long* buf) const {return buf;}
	/**
	 * @brief Get a buffer for writing data from start to (start + len - 1).
	 * 		  This is the recommended method to write data in Constant.
	 * 		  This function is always used together with setIndex(start, len, buf).
	 * 		  Note that if the required underlying data is contiguous,
	 * 			then the underlying buffer is directly returned;
	 * 			otherwise, the buf is returned.
	 *
	 * @param start: The start index.
	 * @param len: The length of data to get.
	 * @param buf: A buffer with at least length len.
	 * @return A buffer to write data.
	 */
	virtual INDEX* getIndexBuffer(INDEX start, int len, INDEX* buf) const {return buf;}
	/**
	 * @brief Get a buffer for writing data from start to (start + len - 1).
	 * 		  This is the recommended method to write data in Constant.
	 * 		  This function is always used together with setFloat(start, len, buf).
	 * 		  Note that if the required underlying data is contiguous,
	 * 			then the underlying buffer is directly returned;
	 * 			otherwise, the buf is returned.
	 *
	 * @param start: The start index.
	 * @param len: The length of data to get.
	 * @param buf: A buffer with at least length len.
	 * @return A buffer to write data.
	 */
	virtual float* getFloatBuffer(INDEX start, int len, float* buf) const {return buf;}
	/**
	 * @brief Get a buffer for writing data from start to (start + len - 1).
	 * 		  This is the recommended method to write data in Constant.
	 * 		  This function is always used together with setDouble(start, len, buf).
	 * 		  Note that if the required underlying data is contiguous,
	 * 			then the underlying buffer is directly returned;
	 * 			otherwise, the buf is returned.
	 *
	 * @param start: The start index.
	 * @param len: The length of data to get.
	 * @param buf: A buffer with at least length len.
	 * @return A buffer to write data.
	 */
	virtual double* getDoubleBuffer(INDEX start, int len, double* buf) const {return buf;}
	/**
	 * @brief Get a buffer for writing data from start to (start + len - 1).
	 * 		  This is the recommended method to write data in Constant.
	 * 		  This function is always used together with setBinary(start, len, unitLength, buf).
	 * 		  Note that if the required underlying data is contiguous,
	 * 			then the underlying buffer is directly returned;
	 * 			otherwise, the buf is returned.
	 *
	 * @param start: The start index.
	 * @param len: The length of data to get.
	 * @param unitLength: The length of the data type of this constant.
	 * @param buf: A buffer with at least length len*unitLength.
	 * @return A buffer to write data.
	 */
	virtual unsigned char* getBinaryBuffer(INDEX start, int len, int unitLength, unsigned char* buf) const {return buf;}
	/**
	 * @brief Get a buffer for writing data from start to (start + len - 1).
	 * 		  This is the recommended method to write data in Constant.
	 * 		  This function is always used together with setData(start, len, buf).
	 * 		  Note that if the required underlying data is contiguous,
	 * 			then the underlying buffer is directly returned;
	 * 			otherwise, the buf is returned.
	 *
	 * @param start: The start index.
	 * @param len: The length of data to get.
	 * @param buf: A buffer with at least length len.
	 * @return A buffer to write data.
	 */
	virtual void* getDataBuffer(INDEX start, int len, void* buf) const {return buf;}

	/**
	 * @brief Get a buffer for writing data from start to (start + len - 1).
	 * 		  This is the recommended method to write data in Constant.
	 * 		  This function is always used together with setDecimal32(start, len, scale, buf).
	 * 		  Note that if the required underlying data is contiguous,
	 * 			then the underlying buffer is directly returned;
	 * 			otherwise, the buf is returned.
	 *
	 * @param start: The start index.
	 * @param len: The length of data to get.
	 * @param scale: Fractional digits.
	 * @param buf: A buffer with at least length len.
	 * @return A buffer to write data.
	 */
	virtual int* getDecimal32Buffer(INDEX start, int len, int scale, int *buf) const { NOT_IMPLEMENT; }
	/**
	 * @brief Get a buffer for writing data from start to (start + len - 1).
	 * 		  This is the recommended method to write data in Constant.
	 * 		  This function is always used together with setDecimal64(start, len, scale, buf).
	 * 		  Note that if the required underlying data is contiguous,
	 * 			then the underlying buffer is directly returned;
	 * 			otherwise, the buf is returned.
	 *
	 * @param start: The start index.
	 * @param len: The length of data to get.
	 * @param scale: Fractional digits.
	 * @param buf: A buffer with at least length len.
	 * @return A buffer to write data.
	 */
	virtual long long* getDecimal64Buffer(INDEX start, int len, int scale, long long *buf) const { NOT_IMPLEMENT; }
	virtual int128* getDecimal128Buffer(INDEX start, int len, int scale,
			int128 *buf) const {
		NOT_IMPLEMENT;
	}

	/**
	 * @brief serialize a constant with type code or datasource to buffer.
	 *
	 * @param pHeap: A heap indicate different method for serializing.
	 * @param buffer: The serialized data is stored in buffer.
	 * @return The result of serializing.
	*/
	virtual IO_ERR serialize(Heap* pHeap, const ByteArrayCodeBufferSP& buffer) const {return serialize(buffer);}
	/**
	 * @brief serialize constant to buffer.
	 *
	 * @param buffer: The serialized data is stored in buffer.
	 * @return The result of serializing.
	*/
	virtual IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const {throw RuntimeException("code serialize method not supported");}
	/**
	 * @brief Serialize constant to buffer.
	 * 		  Note that a literal constant may be partially serialized.
	 *
	 * @param buf: The serialized data is stored in buf.
	 * @param bufSize: The length of buf.
	 * @param indexStart: The index of the element to start serializing
	 * @param offset: The number of bytes in the indexStart-th element that have been partially serialized.
	 * @param numElement: Return the number of elements that have been completely serialized.
	 * @param partial: Return the number of bytes of a element that have been partially serialized.
	 * @return The number of bytes that have been actually serialized into buf.
	*/
    virtual int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int& numElement, int& partial) const {throw RuntimeException("serialize method not supported");}
	/**
	 * @brief Serialize constant to buffer.
	 * 		  Note that a literal constant may be partially serialized.
	 *
	 * @param buf: The serialized data is stored in buf.
	 * @param bufSize: The length of buf.
	 * @param indexStart: The index of the element to start serializing.
	 * @param offset: The number of bytes in the indexStart-th element that have been partially serialized.
	 * @param targetNumElement: Indicating the number of elements to be serialized.
	 * @param numElement: Return the number of elements that have been completely serialized.
	 * @param partial: Return the number of bytes of a element that have been partially serialized.
	 * @return The number of bytes that have been actually serialized into buf.
	*/
    virtual int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int targetNumElement, int& numElement, int& partial) const;
	/**
	 * @brief Read data from DataInputStream and deserialize it into a constant.
	 * 		  Note that a literal constant may be partially deserialized.
	 *
	 * @param in: A DataInputStream for reading data.
	 * @param indexStart: The index of the element to start deserializing.
	 * @param offset: The number of bytes in the indexStart-th element that have been partially deserialized.
	 * @param targetNumElement: Indicating the number of elements to be deserialized.
	 * @param numElement: Return the number of elements that have been completely deserialized.
	 * @param partial: Return the number of bytes of a element that have been partially deserialized.
	 * @return The result of deserializing.
	*/
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
	 * @param index: The index of element to be set.
	 * @param val: The value to be set.
	 */
	virtual void setBool(INDEX index,char val){setBool(val);}
	/**
	 * @brief Set the index-th element to be val.
	 *
	 * @param index: The index of element to be set.
	 * @param val: The value to be set.
	 */
	virtual void setChar(INDEX index,char val){setChar(val);}
	/**
	 * @brief Set the index-th element to be val.
	 *
	 * @param index: The index of element to be set.
	 * @param val: The value to be set.
	 */
	virtual void setShort(INDEX index,short val){setShort(val);}
	/**
	 * @brief Set the index-th element to be val.
	 *
	 * @param index: The index of element to be set.
	 * @param val: The value to be set.
	 */
	virtual void setInt(INDEX index,int val){setInt(val);}
	/**
	 * @brief Set the index-th element to be val.
	 *
	 * @param index: The index of element to be set.
	 * @param val: The value to be set.
	 */
	virtual void setLong(INDEX index,long long val){setLong(val);}
	/**
	 * @brief Set the index-th element to be val.
	 *
	 * @param index: The index of element to be set.
	 * @param val: The value to be set.
	 */
	virtual void setIndex(INDEX index,INDEX val){setIndex(val);}
	/**
	 * @brief Set the index-th element to be val.
	 *
	 * @param index: The index of element to be set.
	 * @param val: The value to be set.
	 */
	virtual void setFloat(INDEX index,float val){setFloat(val);}
	/**
	 * @brief Set the index-th element to be val.
	 *
	 * @param index: The index of element to be set.
	 * @param val: The value to be set.
	 */
	virtual void setDouble(INDEX index, double val){setDouble(val);}
	/**
	 * @brief Set the index-th element to be val.
	 *
	 * @param index: The index of element to be set.
	 * @param val: The value to be set.
	 */
	virtual void setString(INDEX index, const DolphinString& val){setString(val);}
	/**
	 * @brief Set the index-th element to be val.
	 *
	 * @param index: The index of element to be set.
	 * @param unitLength: The length of the data type of this constant.
	 * @param val: The value to be set.
	 */
	virtual void setBinary(INDEX index, int unitLength, const unsigned char* val){setBinary(val, unitLength);}
	/**
	 * @brief Set the index-th element to be null.
	 *
	 * @param index: The index of element to be set.
	 */
	virtual void setNull(INDEX index){setNull();}

	/**
	 * @brief Set the index-th element to be val in specific scale.
	 *
	 * @param index: The index of element to be set.
	 * @param scale: Fractional digits.
	 * @param val: The value to be set.
	 */
	virtual void setDecimal32(INDEX index, int scale, int val) { NOT_IMPLEMENT; }
	/**
	 * @brief Set the index-th element to be val in specific scale.
	 *
	 * @param index: The index of element to be set.
	 * @param scale: Fractional digits.
	 * @param val: The value to be set.
	 */
	virtual void setDecimal64(INDEX index, int scale, long long val) { NOT_IMPLEMENT; }
	virtual void setDecimal128(INDEX index, int scale, int128 val) { NOT_IMPLEMENT; }

	/**
	 * @brief Replace the cell value specified by the index with the new value specified by valueIndex.
	 *
	 * @param index: Make sure index is valid, i.e. no less than zero and less than the size of the object.
	 * @param value: The value to be set.
	 * @param valueIndex: Make sure valueIndex is valid, i.e. no less than zero and less than the size of the value.
	 * @return True if set succeed, false else.
	 */
	virtual bool set(INDEX index, const ConstantSP& value, INDEX valueIndex){return set(index, value->get(valueIndex));}
	/**
	 * @brief Replace the cell value specified by the index with the new value.
	 *
	 * @param index: Make sure index is valid, i.e. no less than zero and less than the size of the object.
	 * @param value: The value to be set.
	 * @return True if set succeed, false else.
	 */
	virtual bool set(INDEX index, const ConstantSP& value){return false;}
	/**
	 * @brief Replace the cell value specified by the column and row index with the new value.
	 *
	 * @param column: Column index.
	 * @param row: Row index.
	 * @param value: The value to be set.
	 * @return True if set succeed, false else.
	 */
	virtual bool set(INDEX column, INDEX row, const ConstantSP& value){return false;}
	/**
	 * @brief Replace the cell value specified by the index with value.
	 *
	 * @param index: Scalar or vector. Make sure all indices in are valid,
	 * 		i.e. no less than zero and less than the size of the value.
	 * @param value: The value to be set.
	 * @return True if set succeed, false else.
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
	/**
	 * @brief Replace the index-th item with the new value.
	 * 		  Note that the index-th column with be replaced with new value in a matrix.
	 *
	 * @param index: Make sure index is valid, i.e. no less than zero and less than the size of the object.
	 * @param value: The value to be set.
	 * @return True if set succeed, false else.
	 */
	virtual bool setItem(INDEX index, const ConstantSP& value){return set(index,value);}
	/**
	 * @brief Replace the index-th column with the new value.
	 *
	 * @param index: Make sure index is valid, i.e. no less than zero and less than the size of the columns.
	 * @param value: The value of new column.
	 * @return True if set succeed, false else.
	 */
	virtual bool setColumn(INDEX index, const ConstantSP& value){return assign(value);}
	/**
	 * @brief Set the row lable from a matrix.
	 */
        virtual void setItemToHeap(Heap* pHeap, INDEX heapIndex, INDEX itemIndex, const string& name);
	virtual void setRowLabel(const ConstantSP& label){}
	/**
	 * @brief Set the column lable from a matrix.
	 */
	virtual void setColumnLabel(const ConstantSP& label){}
	/**
	 * @brief Reshape a matrix with new columns and rows.
	 * 		  Note that if the columns or rows is smaller than the current size the matrix is truncated,
	 * 			otherwise void is appended.
	 * @param cols: New columns.
	 * @param rows: New rows.
	 * @return True if reshape succeed, false else.
	 */
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
	 * @param start: The start index.
	 * @param len: The length of data to set.
	 * @param buf: A buffer with at least length len.
	 * @return True if set succeed, else false.
	 */
	virtual bool setBool(INDEX start, int len, const char* buf){return false;}
	/**
	 * @brief Set the data from start to (start + len - 1) with buf.
	 * 		  This is the recommended method to write data in Constant.
	 * 		  This function is often used together with getCharBuffer(start, len, buf).
	 * 		  Note that if buf is already among the underlying data, then actually
	 * 		  		nothing happens in this function; otherwise, the data of buf will be
	 * 				copied.
	 *
	 * @param start: The start index.
	 * @param len: The length of data to set.
	 * @param buf: A buffer with at least length len.
	 * @return True if set succeed, else false.
	 */
	virtual bool setChar(INDEX start, int len, const char* buf){return false;}
	/**
	 * @brief Set the data from start to (start + len - 1) with buf.
	 * 		  This is the recommended method to write data in Constant.
	 * 		  This function is often used together with getShortBuffer(start, len, buf).
	 * 		  Note that if buf is already among the underlying data, then actually
	 * 		  		nothing happens in this function; otherwise, the data of buf will be
	 * 				copied.
	 *
	 * @param start: The start index.
	 * @param len: The length of data to set.
	 * @param buf: A buffer with at least length len.
	 * @return True if set succeed, else false.
	 */
	virtual bool setShort(INDEX start, int len, const short* buf){return false;}
	/**
	 * @brief Set the data from start to (start + len - 1) with buf.
	 * 		  This is the recommended method to write data in Constant.
	 * 		  This function is often used together with getIntBuffer(start, len, buf).
	 * 		  Note that if buf is already among the underlying data, then actually
	 * 		  		nothing happens in this function; otherwise, the data of buf will be
	 * 				copied.
	 *
	 * @param start: The start index.
	 * @param len: The length of data to set.
	 * @param buf: A buffer with at least length len.
	 * @return True if set succeed, else false.
	 */
	virtual bool setInt(INDEX start, int len, const int* buf){return false;}
	/**
	 * @brief Set the data from start to (start + len - 1) with buf.
	 * 		  This is the recommended method to write data in Constant.
	 * 		  This function is often used together with getLongBuffer(start, len, buf).
	 * 		  Note that if buf is already among the underlying data, then actually
	 * 		  		nothing happens in this function; otherwise, the data of buf will be
	 * 				copied.
	 *
	 * @param start: The start index.
	 * @param len: The length of data to set.
	 * @param buf: A buffer with at least length len.
	 * @return True if set succeed, else false.
	 */
	virtual bool setLong(INDEX start, int len, const long long* buf){return false;}
	/**
	 * @brief Set the data from start to (start + len - 1) with buf.
	 * 		  This is the recommended method to write data in Constant.
	 * 		  This function is often used together with getIndexBuffer(start, len, buf).
	 * 		  Note that if buf is already among the underlying data, then actually
	 * 		  		nothing happens in this function; otherwise, the data of buf will be
	 * 				copied.
	 *
	 * @param start: The start index.
	 * @param len: The length of data to set.
	 * @param buf: A buffer with at least length len.
	 * @return True if set succeed, else false.
	 */
	virtual bool setIndex(INDEX start, int len, const INDEX* buf){return false;}
	/**
	 * @brief Set the data from start to (start + len - 1) with buf.
	 * 		  This is the recommended method to write data in Constant.
	 * 		  This function is often used together with getFloatBuffer(start, len, buf).
	 * 		  Note that if buf is already among the underlying data, then actually
	 * 		  		nothing happens in this function; otherwise, the data of buf will be
	 * 				copied.
	 *
	 * @param start: The start index.
	 * @param len: The length of data to set.
	 * @param buf: A buffer with at least length len.
	 * @return True if set succeed, else false.
	 */
	virtual bool setFloat(INDEX start, int len, const float* buf){return false;}
	/**
	 * @brief Set the data from start to (start + len - 1) with buf.
	 * 		  This is the recommended method to write data in Constant.
	 * 		  This function is often used together with getDoubleBuffer(start, len, buf).
	 * 		  Note that if buf is already among the underlying data, then actually
	 * 		  		nothing happens in this function; otherwise, the data of buf will be
	 * 				copied.
	 *
	 * @param start: The start index.
	 * @param len: The length of data to set.
	 * @param buf: A buffer with at least length len.
	 * @return True if set succeed, else false.
	 */
	virtual bool setDouble(INDEX start, int len, const double* buf){return false;}
	/**
	 * @brief Set the data from start to (start + len - 1) with buf.
	 * 		  This is the recommended method to write data in Constant.
	 * 		  This function is often used together with getStringBuffer(start, len, buf).
	 * 		  Note that if buf is already among the underlying data, then actually
	 * 		  		nothing happens in this function; otherwise, the data of buf will be
	 * 				copied.
	 *
	 * @param start: The start index.
	 * @param len: The length of data to set.
	 * @param buf: A buffer with at least length len.
	 * @return True if set succeed, else false.
	 */
	virtual bool setString(INDEX start, int len, const string* buf){return false;}
	/**
	 * @brief Set the data from start to (start + len - 1) with buf.
	 * 		  This is the recommended method to write data in Constant.
	 * 		  Note that if buf is already among the underlying data, then actually
	 * 		  		nothing happens in this function; otherwise, the data of buf will be
	 * 				copied.
	 *
	 * @param start: The start index.
	 * @param len: The length of data to set.
	 * @param buf: A buffer with at least length len.
	 * @return True if set succeed, else false.
	 */
	virtual bool setString(INDEX start, int len, char** buf){return false;}
	/**
	 * @brief Set the data from start to (start + len - 1) with buf.
	 * 		  This is the recommended method to write data in Constant.
	 * 		  Note that if buf is already among the underlying data, then actually
	 * 		  		nothing happens in this function; otherwise, the data of buf will be
	 * 				copied.
	 *
	 * @param start: The start index.
	 * @param len: The length of data to set.
	 * @param buf: A buffer with at least length len.
	 * @return True if set succeed, else false.
	 */
	virtual bool setString(INDEX start, int len, const DolphinString** buf){return false;}
	/**
	 * @brief Set the binary data from start to (start + len - 1) with buf.
	 * 		  Note that if buf is already among the underlying data, then actually
	 * 		  		nothing happens in this function; otherwise, the data of buf will be
	 * 				copied.
	 *
	 * @param start: The start index.
	 * @param len: The length of data to set.
	 * @param unitLength: The length of the data type of this constant.
	 * @param buf: A buffer with at least length len*unitLength.
	 * @return True if set succeed, else false.
	 */
	virtual bool setBinary(INDEX start, int len, int unitLength, const unsigned char* buf){return false;}
	/**
	 * @brief Set the data from start to (start + len - 1) with buf.
	 * 		  Note that if buf is already among the underlying data, then actually
	 * 		  		nothing happens in this function; otherwise, the data of buf will be
	 * 				copied.
	 *
	 * @param start: The start index.
	 * @param len: The length of elements to set.
	 * @param buf: A buffer with at least length len * (the length of the data type of this constant).
	 * @return True if set succeed, else false.
	 */
	virtual bool setData(INDEX start, int len, void* buf) {return false;}

	/**
	 * @brief Set the data from start to (start + len - 1) with buf.
	 * 		  This is the recommended method to write data in Constant.
	 * 		  This function is often used together with getDecimal32Buffer(start, len, buf).
	 * 		  Note that if buf is already among the underlying data, then actually
	 * 		  		nothing happens in this function; otherwise, the data of buf will be
	 * 				copied.
	 *
	 * @param start: The start index.
	 * @param len: The length of data to set.
	 * @param scale: Fractional digits.
	 * @param buf: A buffer with at least length len.
	 * @return True if set succeed, else false.
	 */
	virtual bool setDecimal32(INDEX start, int len, int scale, const int *buf) { NOT_IMPLEMENT; }
	/**
	 * @brief Set the data from start to (start + len - 1) with buf.
	 * 		  This is the recommended method to write data in Constant.
	 * 		  This function is often used together with getDecimal64Buffer(start, len, buf).
	 * 		  Note that if buf is already among the underlying data, then actually
	 * 		  		nothing happens in this function; otherwise, the data of buf will be
	 * 				copied.
	 *
	 * @param start: The start index.
	 * @param len: The length of data to set.
	 * @param scale: Fractional digits.
	 * @param buf: A buffer with at least length len.
	 * @return True if set succeed, else false.
	 */
	virtual bool setDecimal64(INDEX start, int len, int scale, const long long *buf) { NOT_IMPLEMENT; }
	virtual bool setDecimal128(INDEX start, int len, int scale, const int128 *buf) { NOT_IMPLEMENT; }

	/**
	 * @brief Add inc to the underlying data from start to (start + length - 1).
	 * @return True if succeed, else false.
	 */
	virtual bool add(INDEX start, INDEX length, long long inc) { return false;}
	/**
	 * @brief Add inc to the underlying data from start to (start + length - 1).
	 * @return True if succeed, else false.
	 */
	virtual bool add(INDEX start, INDEX length, double inc) { return false;}
	/**
	 * @brief Check the validation for type time, minute, second, nanotime,
	 * 			and the invalid elements will be replaced by null.
	*/
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
	/**
	 * @brief Return whether this constant could be resized.
	 */
	virtual bool sizeable() const {return false;}
	/**
	 * @brief Return whether this constant could be copyed.
	 */
	virtual bool copyable() const {return true;}
	/**
	 * @brief Return size of this constant.
	 */
	virtual INDEX size() const {return 1;}
	/**
	 * @brief Return the count of items of this constant.
	 * 		  Note that a matrix will return the number of columns.
	 */
	virtual INDEX itemCount() const {return getForm()==DF_MATRIX?columns():size();}
	/**
	 * @brief Return the number of rows.
	 */
	virtual INDEX rows() const {return size();}
	/**
	 * @brief Return the number of uncompressed rows.
	 */
	virtual INDEX uncompressedRows() const {return size();}
	/**
	 * @brief Return the number of columns.
	 */
	virtual INDEX columns() const {return 1;};
	/**
	 * @brief Get data from dictionary according to key, null values will be used to fill in any missing data.
	 *
	 * @param key: The key for which data should be retrieved, could be a scalar or vector.
	 * @return If the key is a scalar, then the value will be a scalar.
	 * 		   If the key is a vector, then the value will be a vector.
	*/
	virtual ConstantSP getMember(const ConstantSP& key) const { throw RuntimeException("getMember method not supported");}
	/**
	 * @brief Get all the keys from a dictionary.
	 *
	 * @return All the keys.
	*/
	virtual ConstantSP keys() const {throw RuntimeException("keys method not supported");}
	/**
	 * @brief Get all the values from a dictionary.
	 *
	 * @return All the values.
	*/
	virtual ConstantSP values() const {throw RuntimeException("values method not supported");}

	/**
	 * @brief Return whether the specified method is in the class the current object belongs to.
	 *
	 * @param name: The name of the method.
	*/
	virtual bool hasMethod(const string& name) const { return false;}
	/**
	 * @brief Return whether the specified operator is in the class the current object belongs to.
	 * 		  A operator could be a method or attribute of the object.
	 *
	 * @param name: The name of the operator.
	*/
	virtual bool hasOperator(const string& name) const { return false;}
	/**
	 * @brief Return the specified method of the class the current object belongs to.
	 *
	 * @param name: The name of the method.
	 * @return A function definition. The first argument must be the object itself when calling the returned method.
	*/
	virtual FunctionDefSP getMethod(const string& name) const { throw RuntimeException("getMethod not supported");}
	/**
	 * @brief Return the specified operator of the class the current object belongs to.
	 * 		  A operator could be a method or attribute of the object.
	 *
	 * @param name: The name of the operator.
	 * @return A function definition. The first argument must be the object itself when calling the returned operator.
	*/
	virtual FunctionDefSP getOperator(const string& name) const { throw RuntimeException("getOperator not supported");}
	/**
	 * @brief Get data from dictionary according to key.
	 *
	 * @return ConstantSP: The data
	*/
	virtual ConstantSP getMember(const string& key) const { throw RuntimeException("getMember method not supported");}

	/**
	 * @brief Release memory of this constant if the used memory not less than target.
	 *
	 * @param target: The target memory.
	 * @param satisfied: Return true if memory released, else false.
	 * @return The actual released memory size.
	*/
	virtual long long releaseMemory(long long target, bool& satisfied) { satisfied = false; return 0;}
	/**
	 * @brief Get the allocated memory of this constant in bytes.
	 *
	 * @return Allocated memory in bytes.
	*/
	virtual long long getAllocatedMemory() const {return 0;}
	/**
	 * @brief Get the raw data type of this constant.
	 *
	 * @return The raw type.
	*/
	virtual DATA_TYPE getRawType() const =0;
	/**
	 * @brief Return the extra attribute of this constant.
	*/
	virtual int getExtraParamForType() const { return 0;}
	/**
	 * @brief Get a copy of this constant except the actual data.
	*/
	virtual ConstantSP getInstance() const =0;
	/**
	 * @brief Get a copy of this constant.
	*/
	virtual ConstantSP getValue() const =0;
	/**
	 * @brief Get a copy of this constant from specified heap.
	 *
	 * @param pHeap:
	*/
	virtual ConstantSP getValue (Heap* pHeap){return getValue();}
	/**
	 * @brief Return itself if this constant is temporary, else return a copy of this constant.
	 *
	 * @param pHeap:
	*/
	virtual ConstantSP getReference(Heap* pHeap){return getValue();}
	/**
	 * @brief Return object type of this constant.
	*/
	virtual OBJECT_TYPE getObjectType() const {return OBJECT_TYPE::CONSTOBJ;}
	/**
	 * @brief Get the SymbolBase from a constant with symbol type.
	 *
	 * @return The SymbolBase.
	*/
	virtual SymbolBaseSP getSymbolBase() const {return SymbolBaseSP();}
	/**
	 * @brief Return whether the target constant exists in the current set or dictionary key.
	 * 		  And set the returned boolean value to resultSP.
	 * @param target: A target constant, could be scalar or vector.
	 * @param resultSP: A result constant where the result boolean value will be set, should be at least as long as target.
	*/
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
	/**
	 * @brief Return whether this constant is a index array.
	*/
	virtual bool isIndexArray() const { return false;}
	/**
	 * @brief Get the underlying index data array of this constant. If the data is not contiguous, return 0.
	 */
	virtual INDEX* getIndexArray() const { return NULL;}
	/**
	 * @brief Return whether this vector is a huge index vector, i.e. all the data is not contiguous.
	 */
	virtual bool isHugeIndexArray() const { return false;}
	/**
	 * @brief Get the underlying index data array segment of this constant. If the data is contiguous, return 0.
	 */
	virtual INDEX** getHugeIndexArray() const { return NULL;}
	/**
	 * @brief Get the size of underlying data array segment of this constant.
	 */
	virtual int getSegmentSize() const { return 1;}
	/**
	 * @brief Get the size of the data segment in bits.
	 */
	virtual int getSegmentSizeInBit() const { return 0;}
	/**
	 * @brief Return whether this constant contain not marshallable object.
	*/
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

class SWORDFISH_API Vector : public Constant {
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
	virtual ConstantSP getIterator(const ConstantSP& self) const;
	virtual ConstantSP getColumnLabel() const;
	/**
	 * @brief Return the name of this vector.
	 *
	 * @return The vector name.
	*/
	const string& getName() const {return name_;}
	/**
	 * @brief Set the name of this vector.
	 *
	 * @param name: The vector name.
	*/
	void setName(const string& name){name_=name;}
	/**
	 * @brief Return whether this vector is a large constant.
	 * 		  Note that a matrix or a constant large than 1024 is a large constant.
	*/
	virtual bool isLargeConstant() const { return isMatrix() || size()>1024; }
	/**
	 * @brief Return whether this vector is a SicedVector or SubVector.
	*/
	virtual bool isView() const {return false;}
	/**
	 * @brief Return whether this vector is a repeating vector.
	*/
	virtual bool isRepeatingVector() const {return false;}
	/**
	 * @brief Get the vector type of this vector.
	*/
	virtual VECTOR_TYPE getVectorType() const {return VECTOR_TYPE::OTHER;}
	/**
	 * @brief Initialize the data of size length in vector with 0.
	*/
	virtual void initialize(){}
	/**
	 * @brief Get capacity of this vector.
	*/
	virtual INDEX getCapacity() const = 0;
	/**
	 * @brief Set the capacity of this vector.
	 *
	 * @return The actual capacity after reserve.
	*/
	virtual	INDEX reserve(INDEX capacity) {throw RuntimeException("Vector::reserve method not supported");}
	/**
	 * @brief Set the size of this vector.
	 *
	 * @return The actual size after resize.
	*/
	virtual	void resize(INDEX size) {throw RuntimeException("Vector::resize method not supported");}
	/**
	 * @brief Get the size of the data type of this vector.
	*/
	virtual short getUnitLength() const = 0;
	/**
	 * @brief Set size of this vector to 0, the memory has not been released.
	*/
	virtual void clear()=0;
	/**
	 * @brief Return whether this vector is a columnar tuple.
	*/
	virtual bool isTableColumn() const {return false;};
	/**
	 * @brief Remove the last count elements from this vector.
	 */
	virtual bool remove(INDEX count){return false;}
	/**
	 * @brief Remove elements from this vector according to index.
	 *
	 * @param index: Make sure index is valid, i.e. no less than zero and less than the size of the columns.
	*/
	virtual bool remove(const ConstantSP& index){return false;}
	/**
	 * @brief Append the value of the given vector to the end of this vector.
	 *
	 * @param value: The vector to be appended.
	 * @return True if succeed, else false.
	*/
	virtual bool append(const ConstantSP& value){return append(value, 0, value->size());}
	/**
	 * @brief Append the specified count of elements of the given vector to the end of this vector.
	 *
	 * @param value: The vector to be appended.
	 * @param count: The append size.
	 * @return True if succeed, else false.
	*/
	virtual bool append(const ConstantSP& value, INDEX count){return append(value, 0, count);}
	/**
	 * @brief Append the value of the given vector at the specified range to the end of the current vector.
	 *
	 * @param value: The vector to be appended.
	 * @param start: Then start index of specified range of the value.
	 * @param count: The range size.
	 * @return True if succeed, else false.
	*/
	virtual bool append(const ConstantSP& value, INDEX start, INDEX count){return false;}
	/**
	 * @brief Append the value specified by the index to the end of the vector.
	 *
	 * @param value: Vector.
	 * @param index: Index vector. Make sure all indices in the vector are valid, i.e. no less than zero and less than the size of the value.
	*/
	virtual bool append(const ConstantSP& value, const ConstantSP& index){return append(value->get(index), 0, index->size());}
	/**
	 * @brief Append data to this vector.
	 *
	 * @param buf: A data buffer.
	 * @param len: The buffer length.
	 * @return True if succeed, else false.
	*/

	virtual ConstantSP moveGet(const ConstantSP& index) { throw RuntimeException("Vector::moveGet method not supported"); }
	virtual bool moveAppend(ConstantSP& value, INDEX start, INDEX len) { throw RuntimeException("Vector::moveAppend method not supported"); }

	virtual bool appendBool(const char* buf, int len){return false;}
	/**
	 * @brief Append data to this vector.
	 *
	 * @param buf: A data buffer.
	 * @param len: The buffer length.
	 * @return True if succeed, else false.
	*/
	virtual bool appendChar(const char* buf, int len){return false;}
	/**
	 * @brief Append data to this vector.
	 *
	 * @param buf: A data buffer.
	 * @param len: The buffer length.
	 * @return True if succeed, else false.
	*/
	virtual bool appendShort(const short* buf, int len){return false;}
	/**
	 * @brief Append data to this vector.
	 *
	 * @param buf: A data buffer.
	 * @param len: The buffer length.
	 * @return True if succeed, else false.
	*/
	virtual bool appendInt(const int* buf, int len){return false;}
	/**
	 * @brief Append data to this vector.
	 *
	 * @param buf: A data buffer.
	 * @param len: The buffer length.
	 * @return True if succeed, else false.
	*/
	virtual bool appendLong(const long long* buf, int len){return false;}
	/**
	 * @brief Append data to this vector.
	 *
	 * @param buf: A data buffer.
	 * @param len: The buffer length.
	 * @return True if succeed, else false.
	*/
	virtual bool appendIndex(const INDEX* buf, int len){return false;}
	/**
	 * @brief Append data to this vector.
	 *
	 * @param buf: A data buffer.
	 * @param len: The buffer length.
	 * @return True if succeed, else false.
	*/
	virtual bool appendFloat(const float* buf, int len){return false;}
	/**
	 * @brief Append data to this vector.
	 *
	 * @param buf: A data buffer.
	 * @param len: The buffer length.
	 * @return True if succeed, else false.
	*/
	virtual bool appendDouble(const double* buf, int len){return false;}
	/**
	 * @brief Append data to this vector.
	 *
	 * @param buf: A data buffer.
	 * @param len: The buffer length.
	 * @return True if succeed, else false.
	*/
	virtual bool appendString(const DolphinString** buf, int len){return false;}
	/**
	 * @brief Append data to this vector.
	 *
	 * @param buf: A data buffer.
	 * @param len: The buffer length.
	 * @return True if succeed, else false.
	*/
	virtual bool appendString(const string* buf, int len){return false;}
	/**
	 * @brief Append data to this vector.
	 *
	 * @param buf: A data buffer.
	 * @param len: The buffer length.
	 * @return True if succeed, else false.
	*/
	virtual bool appendString(const char** buf, int len){return false;}
	/**
	 * @brief Append data to this vector.
	 *
	 * @param buf: A data buffer.
	 * @param len: The buffer length.
	 * @return True if succeed, else false.
	*/
	virtual bool appendGuid(const Guid* buf, int len){return appendBinary((const unsigned char*)buf, len, 16);}
	/**
	 * @brief Append binary data to this vector.
	 *
	 * @param buf: A data buffer.
	 * @param len: The buffer length.
	 * @param unitLength: The size of the data type of this vector.
	 * @return True if succeed, else false.
	*/
	virtual bool appendBinary(const unsigned char* buf, int len, int unitLength){return false;}
	virtual string getString() const;
	virtual string getString(Heap* heap) const;
	virtual string getScript() const;
	virtual string getString(INDEX index) const = 0;
	virtual string getString(Heap* heap, INDEX index) const { return getString(index);}
	virtual ConstantSP getInstance() const {return getInstance(size());}
	/**
	 * @brief Get a copy of this vector with empty data and specified size.
	*/
	virtual ConstantSP getInstance(INDEX size) const = 0;
	/**
	 * @brief Copy this vector.
	 *
	 * @param capacity: The capacity of the new vector.
	 * @return ConstantSP: The new vector.
	*/
	virtual ConstantSP getValue(INDEX capacity) const {throw RuntimeException("Vector::getValue method not supported");}
	/**
	 * @brief Get the subVector of the specified column in this vertor.
	 * 		  Note that the sub-vector is a copy from this vector.
	 *
	 * @param column: Column index.
	 * @param rowStart
	 * @param rowEnd
	 * @return ConstantSP: The sub-vector.
	*/
	virtual ConstantSP get(INDEX column, INDEX rowStart,INDEX rowEnd) const {return getSubVector(column*rows()+rowStart,rowEnd-rowStart);}
	virtual ConstantSP get(INDEX index) const = 0;
	virtual ConstantSP getWindow(INDEX colStart, int colLength, INDEX rowStart, int rowLength) const {return getSubVector(rowStart,rowLength);}
	/**
	 * @brief Get the sub-vector of this vector.
	 * 		  Note that the sub-vector is a copy from this vector.
	 *
	 * @param start
	 * @param length
	 * @return ConstantSP: The sub-vector.
	*/
	virtual ConstantSP getSubVector(INDEX start, INDEX length) const { throw RuntimeException("getSubVector method not supported");}
	/**
	 * @brief Get the sub-vector of this vector with the specified capacity.
	 * 		  Note that the sub-vector is a copy from this vector.
	 *
	 * @param start
	 * @param length
	 * @return ConstantSP: The sub-vector.
	*/
	virtual ConstantSP getSubVector(INDEX start, INDEX length, INDEX capacity) const { return getSubVector(start, length);}
	/**
	 * @brief Fill the value of the vector at the specified range by the value of the given vector.
	 *
	 * @param start: The starting position of the current vector to fill.
	 * @param length: The number of cells of the current vector to fill.
	 * @param value: vector or scalar
	 * @param valueOffset: a valid index
	 */
	virtual void fill(INDEX start, INDEX length, const ConstantSP& value, INDEX valueOffset = 0) = 0;
	/**
	 *  @brief Fill the value of the vector at the specified range by the value of the given vector at the specified index.
	 *
	 * @param start: The starting position of the current vector to fill.
	 * @param length: The number of cells of the current vector to fill.
	 * @param value: vector
	 * @param index: index vector. Make sure all indices in the vector are valid, i.e. no less than zero and less than the size of the value.
	 */
	virtual void fill(INDEX start, INDEX length, const ConstantSP& value, const ConstantSP& index) { fill(start, length, value->get(index));}
	/**
	 * @brief Move the elements of this vector to the left for some positions.
	 *
	 * @param steps: Indicate the lengths to move.
	*/
	virtual void next(INDEX steps)=0;
	/**
	 * @brief Move the elements of this vector to the right for some positions.
	 *
	 * @param steps: Indicate how many positions to move.
	*/
	virtual void prev(INDEX steps)=0;
	/**
	 * @brief Reverse this vector.
	*/
	virtual void reverse()=0;
	/**
	 * @brief Reverse the specified range of this vector.
	 *
	 * @param start: The starting position to reverse.
	 * @param length: The number of elements to reverse.
	*/
	virtual void reverse(INDEX start, INDEX length)=0;
	/**
	 * @brief Taking a shuffle on the data of this vector.
	*/
	virtual void shuffle(){}
	/**
	 * @brief Replace all elements with value oldVal in this vector with newVal.
	 *
	 * @param oldVal: The value to be replaced.
	 * @param newVal: The new value.
	*/
	virtual void replace(const ConstantSP& oldVal, const ConstantSP& newVal){}
	/**
	 * @brief Return whether this vector is a valid index vector.
	 * 		  A valid index vector should not contain null values, and the maximum value should not exceed uplimit.
	 *
	 * @param uplimit: The maximum value of a valid index vector should not exceed uplimit.
	 * @return True if valid, else false
	*/
	virtual bool validIndex(INDEX uplimit){return false;}
	/**
	 * @brief Return whether the sub-vecotr of this vector is a valid index vector.
	 * 		  A valid index vector should not contain null values, and the maximum value should not exceed uplimit.
	 *
	 * @param start: The starting position of the sub-vector.
	 * @param length: The length of the sub-vector.
	 * @param uplimit: The maximum value of a valid index vector should not exceed uplimit.
	 * @return True if valid, else false
	*/
	virtual bool validIndex(INDEX start, INDEX length, INDEX uplimit){return false;}
	/**
	 * @brief Add value offset to the specified range of values in this vector.
	 *
	 * @param start: The starting position of the specified range.
	 * @param length: The length of the specified range.
	 * @param offset: The value that will be added.
	*/
	virtual void addIndex(INDEX start, INDEX length, INDEX offset){}
	/**
	 * @brief Set this vector to the opposite value.
	*/
	virtual void neg()=0;
	/**
	 * @brief Find the index where the target appears for the first time of the specified range in this vector.
	 * 		  If the target doesn't appear in this vector, resultSP will be set to -1.
	 *
	 * @param start: The starting position of the specified range.
	 * @param length: The length of the specified range.
	 * @param target: A scalar
	 * @param resultSP: A scalar, Will be set as the resulting index.
	*/
	virtual void find(INDEX start, INDEX length, const ConstantSP& target, const ConstantSP& resultSP){}
	/**
	 * @brief Find the index where the target appears for the first time in this vector.
	 * 		  If the target doesn't appear in this vector, resultSP will be set to -1.
	 *
	 * @param target: A scalar
	 * @param resultSP: A scalar, Will be set as the resulting index.
	*/
	virtual void find(const ConstantSP& target, const ConstantSP& resultSP){
		find(0, size(), target, resultSP);
	}
	/**
	 * @brief Find the index where the target appears of the specified range in this vector by using binary search.
	 * 		  If the target doesn't appear in this vector, resultSP will be set to -1.
	 *
	 * @param start: The starting position of the specified range.
	 * @param length: The length of the specified range.
	 * @param target: A scalar
	 * @param resultSP: A scalar, Will be set as the resulting index.
	*/
	virtual void binarySearch(INDEX start, INDEX length, const ConstantSP& target, const ConstantSP& resultSP){}
	/**
	 * @brief Find the index where the target appears in this vector by using binary search.
	 * 		  If the target doesn't appear in this vector, resultSP will be set to -1.
	 *
	 * @param target: A scalar.
	 * @param resultSP: A scalar, Will be set as the resulting index.
	*/
	virtual void binarySearch(const ConstantSP& target, const ConstantSP& resultSP){
		binarySearch(0, size(), target, resultSP);
	}
	/**
	 * @brief Find the index of the last element of the specified range in this vector that is no greater than target.
	 * 		  If nothing is found, return -1.
	 *
	 * @param start: The starting position of the specified range.
	 * @param length: The length of the specified range.
	 * @param target: A scalar.
	 * @param resultSP: A scalar, Will be set as the resulting index.
	*/
	virtual void asof(INDEX start, INDEX length, const ConstantSP& target, const ConstantSP& resultSP){ throw RuntimeException("asof method not supported.");}
	/**
	 * @brief Find the index of the last element in this vector that is no greater than target.
	 * 		  If nothing is found, return -1.
	 *
	 * @param target: A scalar.
	 * @param resultSP: A scalar, Will be set as the resulting index.
	*/
	virtual void asof(const ConstantSP& target, const ConstantSP& resultSP){
		asof(0, size(), target, resultSP);
	}
	/**
	 * @brief Convert all characters in this vector into upper cases.
	*/
	virtual void upper(){throw RuntimeException("upper method not supported");}
	/**
	 * @brief Convert all characters in this vector into lower cases.
	*/
	virtual void lower(){throw RuntimeException("lower method not supported");}
	/**
	 * @brief Trim all white spaces in this vector.
	*/
	virtual void trim(){throw RuntimeException("trim method not supported");}
	/**
	 * @brief Remove all space, tab, new line, and carriage characters in both head and tail of the elements in this vector.
	*/
	virtual void strip(){throw RuntimeException("strip method not supported");}
	/**
	 * @brief Return the number of non-null elements in this vector.
	*/
	virtual long long count() const = 0;
	/**
	 * @brief Return the number of non-null elements of the specified range in this vector.
	 *
	 * @param start: The starting position of the specified range.
	 * @param length: The length of the specified range.
	*/
	virtual long long count(INDEX start, INDEX length) const = 0;
	/**
	 * @brief Return the minimum and maximum value of this vector.
	 *
	 * @return ConstantSP: A pair, the first element is the minimum value and the second element is the maximum value.
	*/
	virtual ConstantSP minmax() const;
	/**
	 * @brief Return the minimum and maximum value of the specified range in this vector.
	 *
	 * @param start: The starting position of the specified range.
	 * @param length: The length of the specified range.
	 * @return ConstantSP: A pair, the first element is the minimum value and the second element is the maximum value.
	*/
	virtual ConstantSP minmax(INDEX start, INDEX length) const;
	/**
	 * @brief Return the maximum value of this vector.
	 *
	 * @return ConstantSP: The maximum value.
	*/
	virtual ConstantSP max() const = 0;
	/**
	 * @brief Return the maximum value of the specified range in this vector.
	 *
	 * @param start: The starting position of the specified range.
	 * @param length: The length of the specified range.
	 * @return ConstantSP: The maximum value.
	*/
	virtual ConstantSP max(INDEX start, INDEX length) const = 0;
	/**
	 * @brief Find the maximum value of the specified range in this vector, and set the result to out according to outputStart.
	 *
	 * @param start: The starting position of the specified range.
	 * @param length: The length of the specified range.
	 * @param out: Will be set as the result value.
	 * @param outputStart: The index indicates which element of out will be set as the result value.
	*/
	virtual void max(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart=0) const = 0;
	/**
	 * @brief Return the index of the maximum value in this vector.
	 *
	 * @param rightMost If there are multiple maximum/minimum values, choose the last one if `rightMost` is true.
	 * @return The index of the maximum value.
	*/
	virtual INDEX imax(bool rightMost = false) const = 0;
	/**
	 * @brief Return the index of the maximum value in the specified range of this vector.
	 *
	 * @param start: The starting position of the specified range.
	 * @param length: The length of the specified range.
	 * @param rightMost If there are multiple maximum/minimum values, choose the last one if `rightMost` is true.
	 * @return The index of the maximum value.
	*/
	virtual INDEX imax(INDEX start, INDEX length, bool rightMost = false) const = 0;
	/**
	 * @brief Return the minimum value of this vector.
	 *
	 * @return ConstantSP: The minimum value.
	*/
	virtual ConstantSP min() const = 0;
	/**
	 * @brief Return the minimum value in the specified range of this vector.
	 *
	 * @param start: The starting position of the specified range.
	 * @param length: The length of the specified range.
	 * @return ConstantSP: The minimum value.
	*/
	virtual ConstantSP min(INDEX start, INDEX length) const = 0;
	/**
	 * @brief Find the minimum value in the specified range of this vector, and set the result to out according to outputStart.
	 *
	 * @param start: The starting position of the specified range.
	 * @param length: The length of the specified range.
	 * @param out: Will be set as the result value.
	 * @param outputStart: The index indicates which element of out will be set as the result value.
	*/
	virtual void min(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart=0) const = 0;
	/**
	 * @brief Return the index of the minimum value in this vector.
	 *
	 * @param rightMost If there are multiple maximum/minimum values, choose the last one if `rightMost` is true.
	 * @return The index of the minimum value.
	*/
	virtual INDEX imin(bool rightMost = false) const = 0;
	/**
	 * @brief Return the index of the minimum value in the specified range of this vector.
	 *
	 * @param start: The starting position of the specified range.
	 * @param length: The length of the specified range.
	 * @param rightMost If there are multiple maximum/minimum values, choose the last one if `rightMost` is true.
	 * @return The index of the minimum value.
	*/
	virtual INDEX imin(INDEX start, INDEX length, bool rightMost = false) const = 0;
	/**
	 * @brief Return the average value of this vector.
	 *
	 * @return ConstantSP: The minimum value.
	*/
	virtual ConstantSP avg() const = 0;
	/**
	 * @brief Return the average value in the specified range of this vector.
	 *
	 * @param start: The starting position of the specified range.
	 * @param length: The length of the specified range.
	 * @return ConstantSP: The average value.
	*/
	virtual ConstantSP avg(INDEX start, INDEX length) const = 0;
	/**
	 * @brief  Calculate average value in the specified range of this vector, and set the result to out according to outputStart.
	 *
	 * @param start: The starting position of the specified range.
	 * @param length: The length of the specified range.
	 * @param out: Will be set as the result value.
	 * @param outputStart: The index indicates which element of out will be set as the result value.
	*/
	virtual void avg(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart=0) const = 0;
	/**
	 * @brief Return the sum of all the elements in this vector.
	 *
	 * @return ConstantSP: The sum.
	*/
	virtual ConstantSP sum() const = 0;
	/**
	 * @brief Return the sum of all the elements in the specified range of this vector.
	 *
	 * @param start: The starting position of the specified range.
	 * @param length: The length of the specified range.
	 * @return ConstantSP: The sum.
	*/
	virtual ConstantSP sum(INDEX start, INDEX length) const = 0;
	/**
	 * @brief Calculate the sum of all the elements in the specified range of this vector, and set the result to out according to outputStart.
	 *
	 * @param start: The starting position of the specified range.
	 * @param length: The length of the specified range.
	 * @param out: Will be set as the result value.
	 * @param outputStart: The index indicates which element of out will be set as the result value.
	*/
	virtual void sum(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart=0) const = 0;
	/**
	 * @brief Return the sum of squares of all the elements in this vector.
	 *
	 * @return ConstantSP: the sum of squares.
	*/
	virtual ConstantSP sum2() const = 0;
	/**
	 * @brief Return the sum of squares of all the elements in the specified range of this vector.
	 *
	 * @param start: The starting position of the specified range.
	 * @param length: The length of the specified range.
	 * @return ConstantSP: The sum of squares.
	*/
	virtual ConstantSP sum2(INDEX start, INDEX length) const = 0;
	/**
	 * @brief Calculate the sum of squares of all the elements in the specified range of this vector, and set the result to out according to outputStart.
	 *
	 * @param start: The starting position of the specified range.
	 * @param length: The length of the specified range.
	 * @param out: Will be set as the result value.
	 * @param outputStart: The index indicates which element of out will be set as the result value.
	*/
	virtual void sum2(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart=0) const = 0;
	/**
	 * @brief Return the product of all the elements in this vector.
	 *
	 * @return ConstantSP: The product.
	*/
	virtual ConstantSP prd() const = 0;
	/**
	 * @brief Return the product of all the elements in the specified range of this vector.
	 *
	 * @param start: The starting position of the specified range.
	 * @param length: The length of the specified range.
	 * @return ConstantSP: The product.
	*/
	virtual ConstantSP prd(INDEX start, INDEX length) const = 0;
	/**
	 * @brief Calculate the product of all the elements in the specified range of this vector, and set the result to out according to outputStart.
	 *
	 * @param start: The starting position of the specified range.
	 * @param length: The length of the specified range.
	 * @param out: Will be set as the result value.
	 * @param outputStart: The index indicates which element of out will be set as the result value.
	*/
	virtual void prd(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart=0) const = 0;
	/**
	 * @brief Return the variance of this vector.
	 *
	 * @return ConstantSP: The variance.
	*/
	virtual ConstantSP var() const = 0;
	/**
	 * @brief Return the variance of the specified range in this vector.
	 *
	 * @param start: The starting position of the specified range.
	 * @param length: The length of the specified range.
	 * @return ConstantSP: The variance.
	*/
	virtual ConstantSP var(INDEX start, INDEX length) const = 0;
	/**
	 * @brief Calculate the variance of the specified range in this vector, and set the result to out according to outputStart.
	 *
	 * @param start: The starting position of the specified range.
	 * @param length: The length of the specified range.
	 * @param out: Will be set as the result value.
	 * @param outputStart: The index indicates which element of out will be set as the result value.
	*/
	virtual void var(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart=0) const = 0;
	/**
	 * @brief Return the standard deviation of this vector.
	 *
	 * @return ConstantSP: The sum.
	*/
	virtual ConstantSP std() const = 0;
	/**
	 * @brief Return the standard deviation of the specified range in this vector.
	 *
	 * @param start: The starting position of the specified range.
	 * @param length: The length of the specified range.
	 * @return ConstantSP: The standard deviation.
	*/
	virtual ConstantSP std(INDEX start, INDEX length) const = 0;
	/**
	 * @brief Calculate the standard deviation of the specified range in this vector, and set the result to out according to outputStart.
	 *
	 * @param start: The starting position of the specified range.
	 * @param length: The length of the specified range.
	 * @param out: Will be set as the result value.
	 * @param outputStart: The index indicates which element of out will be set as the result value.
	*/
	virtual void std(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart=0) const = 0;
	/**
	 * @brief Return the median of this vector.
	 *
	 * @return ConstantSP: The median.
	*/
	virtual ConstantSP median() const = 0;
	/**
	 * @brief Return the median of the specified range in this vector.
	 *
	 * @param start: The starting position of the specified range.
	 * @param length: The length of the specified range.
	 * @return ConstantSP: The median.
	*/
	virtual ConstantSP median(INDEX start, INDEX length) const = 0;
	/**
	 * @brief Calculate the median of the specified range in this vector, and set the result to out according to outputStart.
	 *
	 * @param start: The starting position of the specified range.
	 * @param length: The length of the specified range.
	 * @param out: Will be set as the result value.
	 * @param outputStart: The index indicates which element of out will be set as the result value.
	*/
	virtual void median(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart=0) const = 0;
	/**
	 * @brief Return the k-th smallest element of this vector.
	 * 		  And the NULL value will be ignored.
	 *
	 * @param k:
	 * @return ConstantSP: The k-th smallest element.
	*/
	virtual ConstantSP searchK(INDEX k) const = 0;
	/**
	 * @brief Return the k-th smallest element of the specified range in this vector.
	 * 		  And the NULL value will be ignored.
	 *
	 * @param start: The starting position of the specified range.
	 * @param length: The length of the specified range.
	 * @param k:
	 * @return ConstantSP: The k-th smallest element.
	*/
	virtual ConstantSP searchK(INDEX start, INDEX length, INDEX k) const = 0;
	virtual void searchK(INDEX start, INDEX length, INDEX k, const ConstantSP& out, INDEX outputStart=0) const = 0;
	/**
	 * @brief Return the most frequently occurring value in this vector.
	 *
	 * @return ConstantSP: The value.
	*/
	virtual ConstantSP mode() const = 0;
	/**
	 * @brief Return the most frequently occurring value of the specified range in this vector.
	 *
	 * @param start: The starting position of the specified range.
	 * @param length: The length of the specified range.
	 * @return ConstantSP: The value.
	*/
	virtual ConstantSP mode(INDEX start, INDEX length) const = 0;
	/**
	 * @brief Find the most frequently occurring value of the specified range in this vector, and set the result to out according to outputStart.
	 *
	 * @param start: The starting position of the specified range.
	 * @param length: The length of the specified range.
	 * @param out: Will be set as the result value.
	 * @param outputStart: The index indicates which element of out will be set as the result value.
	*/
	virtual void mode(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart=0) const = 0;
	/**
	 * @brief Return a dictionary about the descriptive statistics of this vector including avg, max, min, count, median and std.
	 *
	 * @return ConstantSP: The dictionary.
	*/
	virtual ConstantSP stat() const;
	/**
	 * @brief Return a dictionary about the descriptive statistics of the specified range in this vector including avg, max, min, count, median and std.
	 *
	 * @return ConstantSP: The dictionary.
	*/
	virtual ConstantSP stat(INDEX start, INDEX length) const;
	/**
	 * @brief Return the first element of this vector that is neither exclude nor NULL.
	 *
	 * @param exclude: A scalar.
	 * @return ConstantSP: The first element.
	*/
	virtual ConstantSP firstNot(const ConstantSP& exclude) const = 0;
	/**
	 * @brief Return the first element of the specified range in this vector that is neither exclude nor NULL.
	 *
	 * @param start: The starting position of the specified range.
	 * @param length: The length of the specified range.
	 * @param exclude: A scalar.
	 * @return ConstantSP: The first element.
	*/
	virtual ConstantSP firstNot(INDEX start, INDEX length, const ConstantSP& exclude) const = 0;
	/**
	 * @brief Find the first element of the specified range in this vector that is neither exclude nor NULL, and set the result to out according to outputStart.
	 *
	 * @param start: The starting position of the specified range.
	 * @param length: The length of the specified range.
	 * @param exclude: A scalar
	 * @param out: Will be set as the result value.
	 * @param outputStart: The index indicates which element of out will be set as the result value.
	*/
	virtual void firstNot(INDEX start, INDEX length, const ConstantSP& exclude, const ConstantSP& out, INDEX outputStart=0) const = 0;
	/**
	 * @brief Return the last element of this vector that is neither exclude nor NULL.
	 *
	 * @param exclude: A scalar
	 * @return ConstantSP: The first element.
	*/
	virtual ConstantSP lastNot(const ConstantSP& exclude) const = 0;
	/**
	 * @brief Return the last element of the specified range in this vector that is neither exclude nor NULL.
	 *
	 * @param start: The starting position of the specified range.
	 * @param length: The length of the specified range.
	 * @param exclude: A scalar
	 * @return ConstantSP: The last element.
	*/
	virtual ConstantSP lastNot(INDEX start, INDEX length, const ConstantSP& exclude) const = 0;
	/**
	 * @brief Find the last element of the specified range in this vector that is neither exclude nor NULL, and set the result to out according to outputStart.
	 *
	 * @param start: The starting position of the specified range.
	 * @param length: The length of the specified range.
	 * @param exclude: A scalar
	 * @param out: Will be set as the result value.
	 * @param outputStart: The index indicates which element of out will be set as the result value.
	*/
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
	 * @brief Find the first element that is no less than the target value in the sorted vector. If all elements are
	 * 			less than the target value, return the size of the vector.
	 * @param start: The starting point of the search.
	 * @return The index of found element.
	 */
	virtual INDEX lowerBound(INDEX start, const ConstantSP& target)=0;
	/**
	 * @brief Find the range of the specified value in a sorted vector.
	 *
	 * @param target: The target value. it must be a scalar.
	 * @param range: in/out parameter in the format of pair<offset, length>. When serving as a input parameter,
	 * 					it specifies the range to search. The search result is stored in this parameter too.
	 * 					If no element equals to the target value, the length of the output is set to 0.
	 */
	virtual void equalRange(const ConstantSP& target, pair<INDEX, INDEX>& range) const {throw RuntimeException("equalRange method not supported");}
	/**
	 * @brief Get whether the value of an element in the the specified range of this vector is the same as the previous element.
	 *
	 * @param start: The starting position of the specified range. Note that start-1 must be a valid index.
	 * @param length: The length of the specified range.
	 * @param result: A buffer where the result boolean value will be set, should be at least as long as length.
	 * @return True if succeed, else false.
	 */
	virtual bool equalToPrior(INDEX start, INDEX length, bool* result){ return false;}
	/**
	 * @brief Get whether the value of the indices[i]-th element in this vector is the same as the indices[i-1]-th element.
	 *
	 * @param prior: The prior-th element in this vector will compare to indices[0]-th element.
	 * @param indices: A index vector.
	 * @param length: The length of the specified range.
	 * @param result: A buffer where the result boolean value will be set, should be at least as long as length.
	 * @return True if succeed, else false.
	 */
	virtual bool equalToPrior(INDEX prior, const INDEX* indices, INDEX length, bool* result){ return false;}
	/**
	 * @brief Return the the index of top k-th elements in the the specified range of this vector.
	 *
	 * @param start: The starting position of the specified range. Note that start-1 must be a valid index.
	 * @param length: The length of the specified range.
	 * @param top: The number of top elements.
	 * @param asc: In ascending order(true) or descending order(false).
	 * @param extendEqualValue: If false: Returns the index of the topk elements.
	 * 							If true : Find the k-th element according to asc,
	 * 									  return the indices of all elements greater than or equal to it.
	 * @return : ConstantSP: A index vector.
	 */
    virtual ConstantSP topK(INDEX start, INDEX length, INDEX top, bool asc, bool extendEqualValue) {throw RuntimeException("topK method not supported");}
	/**
	 * @brief Return the the index of top k-th elements in this vector.
	 *
	 * @param top: The number of top elements.
	 * @param asc: In ascending order(true) or descending order(false).
	 * @param extendEqualValue: If false: Returns the index of the topk elements.
	 * 							If true : Find the k-th element according to asc,
	 * 									  return the indices of all elements greater than or equal to it.
	 * @return : ConstantSP: A index vector.
	 */
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
     * @brief Find duplicated elements in an ascending-sorted array.
	 *
     * @param indices: Indices of data to search. The underlying data was not sorted. However the indices
     * 				   are in the ascending order.
	 * @param start: The start position in indices vector.
	 * @param length: The number of indices to process.
	 * @param duplicates: Output vector of the duplicated elements. pair.first stores the starting position
	 * 					  of the duplicated elements in the indices array and pair.second has the number of duplicated elements.
	 * @return True if find succeed, else false.
     */
	virtual bool findDuplicatedElements(Vector* indices, INDEX start, INDEX length, vector<pair<INDEX,INDEX> >& duplicates) = 0;

	/**
	 * @brief Find duplicated elements in the segment of an ascending-sorted array.
	 *
	 * @param start: The starting position of the segment.
	 * @param length: The length of the segment.
	 * @param duplicates: Output vector of the duplicated elements. pair.first stores the starting position
	 * 					  of the duplicated elements in the array and pair.second has the number of duplicated elements.
	 * @return True if find succeed, else false.
	 */
	virtual bool findDuplicatedElements(INDEX start, INDEX length, vector<pair<INDEX,INDEX> >& duplicates)=0;

	/**
	 * @brief Find unique elements in the segment of an ascending-sorted array.
	 *
	 * @param start: The starting position of the segment.
	 * @param length: The length of the segment.
	 * @param duplicates: Output vector of the unique elements. pair.first stores the starting position
	 * 					  of the unique elements in the array and pair.second has the number of unique elements.
	 * @return True if find succeed, else false.
	 */
	virtual bool findUniqueElements(INDEX start, INDEX length, vector<pair<INDEX,INDEX> >& uniques)=0;

	/**
	 * @brief Find the first and last index of the target in a vector.
	 *
	 * @param ascIndices: This vector is in ascending-ordered with ascIndices.
	 * @param target: The target elements.
	 * @param targetIndices: The indices of target, assume that the length is the same as ranges.
	 * @param ranges: Set the result of find to ranges.
	 * @return True if find succeed, else false.
	 */
	virtual bool findRange(INDEX* ascIndices,const ConstantSP& target,INDEX* targetIndices,vector<pair<INDEX,INDEX> >& ranges)=0;
	/**
	 * @brief Find the first and last index of the target in a sorted vector.
	 *
	 * @param target: The target elements.
	 * @param targetIndices: The indices of target, assume that the length is the same as ranges.
	 * @param ranges: Set the result of find to ranges.
	 * @return True if find succeed, else false.
	 */
	virtual bool findRange(const ConstantSP& target,INDEX* targetIndices,vector<pair<INDEX,INDEX> >& ranges)=0;
	virtual long long getAllocatedMemory(INDEX size) const {return Constant::getAllocatedMemory();}
    virtual long long getAllocatedMemory() const {return getAllocatedMemory(size());}
	virtual int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int& numElement, int& partial) const {throw RuntimeException("serialize method not supported");}
	virtual int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int targetNumElement, int& numElement, int& partial) const;

	/**
	 * @brief Judge the data according to indices is null or not.
	 * 		  The safe operators assumes:
	 * 		  (1) indices is ascending sorted
	 * 		  (2) offset + indices are guaranteed valid ( between 0 and size - 1)
	 *
	 * @param offset: The value of indices is offseted by offset.
	 * @param indices: A sorted index vector.
	 * @param len: The length of data to be judged.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	*/
	virtual bool isNullSafe(INDEX offset, INDEX* indices, int len, char* buf) const {return false;}
	/**
	 * @brief Judge the data according to indices is valid or not.
	 * 		  The safe operators assumes:
	 * 		  (1) indices is ascending sorted
	 * 		  (2) offset + indices are guaranteed valid ( between 0 and size - 1)
	 *
	 * @param offset: The value of indices is offseted by offset.
	 * @param indices: A sorted index vector.
	 * @param len: The length of data to be judged.
	 * @param buf: The result is stored in buf.
	 * @return true: The function call succeed.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool isValidSafe(INDEX offset, INDEX* indices, int len, char* buf) const {return false;}
	/**
	 * @brief Get the boolean data according to indices.
	 * 		  The safe operators assumes:
	 * 		  (1) indices is ascending sorted
	 * 		  (2) offset + indices are guaranteed valid ( between 0 and size - 1)
	 *
	 * @param offset: The value of indices is offseted by offset.
	 * @param len: The length of data to be retrieved.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool getBoolSafe(INDEX offset, INDEX* indices, int len, char* buf) const {return false;}
	/**
	 * @brief Get the character data according to indices.
	 * 		  The safe operators assumes:
	 * 		  (1) indices is ascending sorted
	 * 		  (2) offset + indices are guaranteed valid ( between 0 and size - 1)
	 *
	 * @param offset: The value of indices is offseted by offset.
	 * @param len: The length of data to be retrieved.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool getCharSafe(INDEX offset, INDEX* indices, int len,char* buf) const {return false;}
	/**
	 * @brief Get the short value of data according to indices.
	 * 		  The safe operators assumes:
	 * 		  (1) indices is ascending sorted
	 * 		  (2) offset + indices are guaranteed valid ( between 0 and size - 1)
	 *
	 * @param offset: The value of indices is offseted by offset.
	 * @param len: The length of data to be retrieved.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool getShortSafe(INDEX offset, INDEX* indices, int len, short* buf) const {return false;}
	/**
	 * @brief Get the int value of data according to indices.
	 * 		  The safe operators assumes:
	 * 		  (1) indices is ascending sorted
	 * 		  (2) offset + indices are guaranteed valid ( between 0 and size - 1)
	 *
	 * @param offset: The value of indices is offseted by offset.
	 * @param len: The length of data to be retrieved.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool getIntSafe(INDEX offset, INDEX* indices, int len, int* buf) const {return false;}
	/**
	 * @brief Get the long long value of data according to indices.
	 * 		  The safe operators assumes:
	 * 		  (1) indices is ascending sorted
	 * 		  (2) offset + indices are guaranteed valid ( between 0 and size - 1)
	 *
	 * @param offset: The value of indices is offseted by offset.
	 * @param len: The length of data to be retrieved.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool getLongSafe(INDEX offset, INDEX* indices, int len, long long* buf) const {return false;}
	/**
	 * @brief Get the index value of data according to indices.
	 * 		  The safe operators assumes:
	 * 		  (1) indices is ascending sorted
	 * 		  (2) offset + indices are guaranteed valid ( between 0 and size - 1)
	 *
	 * @param offset: The value of indices is offseted by offset.
	 * @param len: The length of data to be retrieved.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool getIndexSafe(INDEX offset, INDEX* indices, int len, INDEX* buf) const {return false;}
	/**
	 * @brief Get the float value of data according to indices.
	 * 		  The safe operators assumes:
	 * 		  (1) indices is ascending sorted
	 * 		  (2) offset + indices are guaranteed valid ( between 0 and size - 1)
	 *
	 * @param offset: The value of indices is offseted by offset.
	 * @param len: The length of data to be retrieved.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool getFloatSafe(INDEX offset, INDEX* indices, int len, float* buf) const {return false;}
	/**
	 * @brief Get the double value of data according to indices.
	 * 		  The safe operators assumes:
	 * 		  (1) indices is ascending sorted
	 * 		  (2) offset + indices are guaranteed valid ( between 0 and size - 1)
	 *
	 * @param offset: The value of indices is offseted by offset.
	 * @param len: The length of data to be retrieved.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool getDoubleSafe(INDEX offset, INDEX* indices, int len, double* buf) const {return false;}
	/**
	 * @brief Get the symbol data according to indices.
	 * 		  The safe operators assumes:
	 * 		  (1) indices is ascending sorted
	 * 		  (2) offset + indices are guaranteed valid ( between 0 and size - 1)
	 *
	 * @param start: The start index.
	 * @param len: The length of data to be retrieved.
	 * @param buf: The result is stored in buf.
	 * @param symBase: SymbolBase is a mapping from string to int.
	 * @param insertIfNotThere: Should a string that is not found in Symbase be inserted into Symbase.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool getSymbolSafe(INDEX offset, INDEX* indices, int len, int* buf, SymbolBase* symBase,bool insertIfNotThere) const {return false;}
	/**
	 * @brief Get the DolphinString data according to indices.
	 * 		  The safe operators assumes:
	 * 		  (1) indices is ascending sorted
	 * 		  (2) offset + indices are guaranteed valid ( between 0 and size - 1)
	 *
	 * @param offset: The value of indices is offseted by offset.
	 * @param len: The length of data to be retrieved.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool getStringSafe(INDEX offset, INDEX* indices, int len, DolphinString** buf) const {return false;}
	/**
	 * @brief Get the string data according to indices.
	 * 		  The safe operators assumes:
	 * 		  (1) indices is ascending sorted
	 * 		  (2) offset + indices are guaranteed valid ( between 0 and size - 1)
	 *
	 * @param offset: The value of indices is offseted by offset.
	 * @param len: The length of data to be retrieved.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool getStringSafe(INDEX offset, INDEX* indices, int len, char** buf) const {return false;}
	/**
	 * @brief Get the binary data according to indices.
	 * 		  The safe operators assumes:
	 * 		  (1) indices is ascending sorted
	 * 		  (2) offset + indices are guaranteed valid ( between 0 and size - 1)
	 *
	 * @param offset: The value of indices is offseted by offset.
	 * @param len: The length of data to be retrieved.
	 * @param unitLength: The length of the data type of this constant.
	 * @param buf: The result is stored in buf.
	 * @return True if the function call succeed, else false.
	 */
	virtual bool getBinarySafe(INDEX offset, INDEX* indices, int len, int unitLength, unsigned char* buf) const {return false;}

	/**
	 * An array vector must implement following methods.
	 */

	/**
	 * @brief Convert specified rows into vector.
	 *
	 * @param rowStart: Row index to start converting.
	 * @param count: The number of rows.
	 * @return ConstantSP: A vector.
	 */
	virtual ConstantSP flatten(INDEX rowStart, INDEX count) const {throw RuntimeException("flatten method not supported");}
	/**
	 * @brief Return the first element of specified rows of this vector.
	 *
	 * @param rowStart: Row index.
	 * @param count: The number of rows.
	 * @return ConstantSP: A vector.
	 */
	virtual ConstantSP rowFirst(INDEX rowStart, INDEX count) const {throw RuntimeException("rowFirst method not supported");}
	/**
	 * @brief Return the last element of specified rows of this vector.
	 *
	 * @param rowStart: Row index.
	 * @param count: The number of rows.
	 * @return ConstantSP: A vector.
	 */
	virtual ConstantSP rowLast(INDEX rowStart, INDEX count) const {throw RuntimeException("rowLast method not supported");}
	/**
	 * @brief Return the first element of specified rows of this vector that is neither exclude nor NULL.
	 *
	 * @param rowStart: Row index.
	 * @param count: The number of rows.
	 * @param exclude: A scalar.
	 * @return ConstantSP: A vector.
	 */
	virtual ConstantSP rowFirstNot(INDEX rowStart, INDEX count, const ConstantSP& exclude) const {throw RuntimeException("rowFirstNot method not supported");}
	/**
	 * @brief Return the last element of specified rows of this vector that is neither exclude nor NULL.
	 *
	 * @param rowStart: Row index.
	 * @param count: The number of rows.
	 * @param exclude: A scalar.
	 * @return ConstantSP: A vector.
	 */
	virtual ConstantSP rowLastNot(INDEX rowStart, INDEX count, const ConstantSP& exclude) const {throw RuntimeException("rowLastNot method not supported");}
	/**
	 * @brief Calculate the sum of specified rows of this vector.
	 *
	 * @param rowStart: Row index to start calculating.
	 * @param count: The number of rows.
	 * @return ConstantSP: A vector, contains the result of the calculation for specified row.
	 */
	virtual ConstantSP rowSum(INDEX rowStart, INDEX count) const {throw RuntimeException("rowSum method not supported");}
	/**
	 * @brief Calculate the sum of squares of specified rows of this vector.
	 *
	 * @param rowStart: Row index to start calculating.
	 * @param count: The number of rows.
	 * @return ConstantSP: A vector, contains the result of the calculation for specified row.
	 */
	virtual ConstantSP rowSum2(INDEX rowStart, INDEX count) const {throw RuntimeException("rowSum2 method not supported");}
	/**
	 * @brief Return the number of non-null elements of specified rows of this vector.
	 *
	 * @param rowStart: Row index to start counting.
	 * @param count: The number of rows.
	 * @return ConstantSP: A vector.
	 */
	virtual ConstantSP rowCount(INDEX rowStart, INDEX count) const {throw RuntimeException("rowCount method not supported");}
	/**
	 * @brief Return the size of specified rows of this vector.
	 *
	 * @param rowStart: Row index.
	 * @param count: The number of rows.
	 * @return ConstantSP: A vector.
	 */
	virtual ConstantSP rowSize(INDEX rowStart, INDEX count) const {throw RuntimeException("rowSize method not supported");}
	/**
	 * @brief Calculate the average of specified rows of this vector.
	 *
	 * @param rowStart: Row index to start calculating.
	 * @param count: The number of rows.
	 * @return ConstantSP: A vector, contains the result of the calculation for specified row.
	 */
	virtual ConstantSP rowAvg(INDEX rowStart, INDEX count) const {throw RuntimeException("rowAvg method not supported");}
	/**
	 * @brief Calculate standard deviation of specified rows of this vector.
	 *
	 * @param rowStart: Row index to start calculating.
	 * @param count: The number of rows.
	 * @return ConstantSP: A vector, contains the result of the calculation for specified row.
	 */
	virtual ConstantSP rowStd(INDEX rowStart, INDEX count) const {throw RuntimeException("rowStd method not supported");}
	/**
	 * @brief Calculate the population standard deviation of specified rows of this vector.
	 *
	 * @param rowStart: Row index to start calculating.
	 * @param count: The number of rows.
	 * @return ConstantSP: A vector, contains the result of the calculation for specified row.
	 */
	virtual ConstantSP rowStdp(INDEX rowStart, INDEX count) const {throw RuntimeException("rowStdp method not supported");}
	/**
	 * @brief Calculate the variance of specified rows of this vector.
	 *
	 * @param rowStart: Row index to start calculating.
	 * @param count: The number of rows.
	 * @return ConstantSP: A vector, contains the result of the calculation for specified row.
	 */
	virtual ConstantSP rowVar(INDEX rowStart, INDEX count) const {throw RuntimeException("rowVar method not supported");}
	/**
	 * @brief Calculate the population variance of specified rows of this vector.
	 *
	 * @param rowStart: Row index to start calculating.
	 * @param count: The number of rows.
	 * @return ConstantSP: A vector, contains the result of the calculation for specified row.
	 */
	virtual ConstantSP rowVarp(INDEX rowStart, INDEX count) const {throw RuntimeException("rowVarp method not supported");}
	/**
	 * @brief Return the minimum elements of specified rows of this vector.
	 *
	 * @param rowStart: Row index.
	 * @param count: The number of rows.
	 * @return ConstantSP: A vector.
	 */
	virtual ConstantSP rowMin(INDEX rowStart, INDEX count) const {throw RuntimeException("rowMin method not supported");}
	/**
	 * @brief Return the maximum elements of specified rows of this vector.
	 *
	 * @param rowStart: Row index.
	 * @param count: The number of rows.
	 * @return ConstantSP : A vector.
	 */
	virtual ConstantSP rowMax(INDEX rowStart, INDEX count) const {throw RuntimeException("rowMax method not supported");}
	/**
	 * @brief Calculate the product of specified rows of this vector.
	 *
	 * @param rowStart: Row index to start calculating.
	 * @param count: The number of rows.
	 * @return ConstantSP : A vector, contains the result of the calculation for specified row.
	 */
	virtual ConstantSP rowProd(INDEX rowStart, INDEX count) const {throw RuntimeException("rowProd method not supported");}
	/**
	 * @brief Calculate the and result of all elements of specified rows of this vector.
	 *
	 * @param rowStart: Row index to start calculating.
	 * @param count: The number of rows.
	 * @return ConstantSP : A vector, contains the result of the calculation for specified row.
	 */
	virtual ConstantSP rowAnd(INDEX rowStart, INDEX count) const {throw RuntimeException("rowAnd method not supported");}
	/**
	 * @brief Calculate the or result of all elements of specified rows of this vector.
	 *
	 * @param rowStart: Row index to start calculating.
	 * @param count: The number of rows.
	 * @return ConstantSP : A vector, contains the result of the calculation for specified row.
	 */
	virtual ConstantSP rowOr(INDEX rowStart, INDEX count) const {throw RuntimeException("rowOr method not supported");}
	/**
	 * @brief Calculate the xor result of all elements of specified rows of this vector.
	 *
	 * @param rowStart: Row index to start calculating.
	 * @param count: The number of rows.
	 * @return ConstantSP : A vector, contains the result of the calculation for specified row.
	 */
	virtual ConstantSP rowXor(INDEX rowStart, INDEX count) const {throw RuntimeException("rowXor method not supported");}
	/**
	 * @brief Calculate the median of specified rows of this vector.
	 *
	 * @param rowStart: Row index to start calculating.
	 * @param count: The number of rows.
	 * @return ConstantSP : A vector, contains the result of the calculation for specified row.
	 */
	virtual ConstantSP rowMed(INDEX rowStart, INDEX count) const {throw RuntimeException("rowMed method not supported");}
	/**
	 * @brief Calculate the kurtosis of specified rows of this vector.
	 *
	 * @param rowStart: Row index to start calculating.
	 * @param count: The number of rows.
	 * @return ConstantSP : A vector, contains the result of the calculation for specified row.
	 */
	virtual ConstantSP rowKurtosis(INDEX rowStart, INDEX count, bool biased) const {throw RuntimeException("rowKurtosis method not supported");}
	/**
	 * @brief Calculate the skewness of specified rows of this vector.
	 *
	 * @param rowStart: Row index to start calculating.
	 * @param count: The number of rows.
	 * @param biased: Indicate whether the result is biased.
	 * @return ConstantSP : A vector, contains the result of the calculation for specified row.
	 */
	virtual ConstantSP rowSkew(INDEX rowStart, INDEX count, bool biased) const {throw RuntimeException("rowSkew method not supported");}
	/**
	 * @brief Calculate the percentile of specified rows of this vector.
	 * 		  Note that this function is not implemented yet.
	 */
	virtual ConstantSP rowPercentile(INDEX rowStart, INDEX count, double percentile) const {throw RuntimeException("rowPercentile method not supported");}
	/**
	 * @brief Calculate the rank of specified rows of this vector.
	 * 		  Note that this function is not implemented yet.
	 */
	virtual ConstantSP rowRank(INDEX rowStart, INDEX count, bool ascending, int groupNum, bool ignoreNA, int tiesMethod, bool percent) const {throw RuntimeException("rowRank method not supported");}
	/**
	 * @brief Calculate the denseRank of specified rows of this vector.
	 * 		  Note that this function is not implemented yet.
	 */
	virtual ConstantSP rowDenseRank(INDEX rowStart, INDEX count, bool ascending, bool ignoreNA, bool percent) const {throw RuntimeException("rowDenseRank method not supported");}

	/**
	 * @brief Set this vector as external buffer.
	 * 		  Note the momery of a vector as external buffer will not be released when destruct.
	 *
	 * @param option: Indicate whether this vector is external buffer.
	 */
	virtual void setExternalBuffer(bool option) {throw RuntimeException("Vector::setExternalBuffer method not supported");}
	/**
	 * @brief Return wheher this vector is external buffer.
	 * 		  Note the momery of a vector as external buffer will not be released when destruct.
	 */
	virtual bool isExternalBuffer() const { return false;}
	/**
	 * @brief Return whether this vector has internal vectors.
	 */
    virtual bool hasInternalVector() const { return false; }
	/**
	 * @brief Return the internal vector of this vector.
	 */
    virtual ConstantSP getInternalVector() { throw RuntimeException("getInternalVector method not supported"); }
	/**
	 * @brief Return whether the vector supports getMaterialized.
	 */
    virtual bool canGetMaterialized() const { return false; }
    virtual VectorSP getMaterialized() const { throw RuntimeException("getMaterialized method not supported"); }
    virtual void materialize() const { throw RuntimeException("materialize method not supported"); }

private:
	string name_;
};

class SWORDFISH_API Matrix{
public:
	Matrix(int cols, int rows);
	virtual ~Matrix(){}
	void setRowLabel(const ConstantSP& label);
	void setColumnLabel(const ConstantSP& label);
	bool reshape(INDEX cols, INDEX rows);
	string getString() const;
	string getString(Heap* pHeap) const;
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

class SWORDFISH_API Tensor : public Constant {
public:
	// Copy constructor.
	Tensor(const Tensor &) = default;
	// Move constructor.
	Tensor(Tensor &&) = default;
	// Copy assignment.
	Tensor &operator=(const Tensor &) = default;
	// Move assignment.
	Tensor &operator=(Tensor &&) = default;

	Tensor(DATA_TYPE dataType, TensorType tensorType, const std::vector<long long> &shape,
			const std::vector<long long> &strides = {}, DeviceType deviceType = DeviceType::CPU);

	virtual ~Tensor() override;

	TensorType getTensorType() const noexcept { return tensorType_; }
	DeviceType getDeviceType() const noexcept { return deviceType_; }
	unsigned int getTensorFlags() const noexcept { return tensorFlags_; }
	long long ndim() const noexcept { return shape_.size(); }
	const vector<long long> & shape() const noexcept { return shape_; }
	const vector<long long> & strides() const noexcept { return strides_; }
	/**
	 * @brief Return index of the last element, calculated with the following formula:
	 * index = (shape[0]-1) * strides[0] + (shape[1]-1) * strides[1] + ...
	 */
	long long indexOfLastElement() const;
	/**
	 * @brief Check if this tensor is contiguous.
	 */
	bool isContiguous() const;
	/**
	 * @brief Create a NEW contiguous tensor base on this tensor.
	 * @note This tensor remain unaffected.
	 */
	virtual TensorSP contiguous() const = 0;
	virtual TensorSP reshape(const vector<long long>& shape) const = 0;
	/**
	 * @brief Deep copy this tensor.
	 */
	virtual TensorSP clone() const = 0;

	virtual string getScript() const override;

public:
	static vector<long long> makeContiguousStrides(const std::vector<long long> &shape);
	static bool isContiguous(const std::vector<long long> &shape, const std::vector<long long> &strides);
	static bool eqObj(const Tensor &lhs, const Tensor &rhs, double precision);
	static bool isDataTypeSupported(DATA_TYPE type);
	static long long computeSize(const std::vector<long long> &shape);
	static Tensor * makeTensor(DATA_TYPE dataType, void *ptr, const std::vector<long long> &shape, const std::vector<long long> &strides = {},
							   TensorType tensorType = TensorType::Basic, DeviceType deviceType = DeviceType::CPU);

protected:
	TensorType tensorType_;
	DeviceType deviceType_;
	unsigned int tensorFlags_ = 0;

	/**
	 * E.g., a tensor with shape [D, H, W] means:
	 * 1. It is 3-dimensional.
	 * 2. Its depth is D, height (or rows) is H, width (or columns) is W.
	 * If this tensor is contiguous, its strides are [H*W, W, 1].
	 */
	vector<long long> shape_;
	vector<long long> strides_;
};

class SWORDFISH_API Set: public Constant {
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

class SWORDFISH_API Dictionary:public Constant{
public:
	Dictionary(DATA_TYPE dt, DATA_CATEGORY dc) : Constant(DF_DICTIONARY, dt, dc), lock_(0){}
	virtual ~Dictionary();
	/**
	 * @brief Return the size of this dictionary.
	 */
	virtual INDEX size() const = 0;
	/**
	 * @brief Return the size of this dictionary.
	 */
	virtual INDEX count() const = 0;
	/**
	 * @brief Erase all the elements of this dictionary.
	 */
	virtual void clear()=0;
	virtual ConstantSP getMember(const ConstantSP& key) const =0;
	virtual ConstantSP getMember(const string& key) const {throw RuntimeException("String key not supported");}
	virtual ConstantSP get(INDEX column, INDEX row){throw RuntimeException("Dictionary does not support cell function");}
	/**
	 * @brief Return the symbolBase of the keys of this dictionary.
	 */
	virtual SymbolBaseSP getKeySymbolBase() const { return nullptr;}
	/**
	 * @brief Return the data type of the keys of this dictionary.
	 */
	virtual DATA_TYPE getKeyType() const = 0;
	/**
	 * @brief Return the data category of the keys of this dictionary.
	 */
	virtual DATA_CATEGORY getKeyCategory() const = 0;
	virtual ConstantSP keys() const = 0;
	virtual ConstantSP values() const = 0;
	virtual string getString() const = 0;
	virtual string getScript() const {return "dict()";}
	virtual string getString(int index) const {throw RuntimeException("Dictionary::getString(int index) not supported");}
	/**
	 * @brief Remove the elements from this dictionary according to key.
	 *
	 * @param key:A scalar or vector, indicate the key of the elements to be removed.
	 * @return True if remove succeed, else false.
	 */
	virtual bool remove(const ConstantSP& key) = 0;
	/**
	 * @brief Set the element values accoreding to key.
	 *
	 * @param key:A scalar or vector, indicate the key of the elements to set.
	 * @param value:A scalar or vector, assume that has the same size as key.
	 * @return True if set succeed, else false.
	 */
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
	/**
	 * @brief Apply optr between value and the elements of this dictionary according to key.
	 * 		  If there is a new key to this dictionary, the value of the key will be set to value directly.
	 *
	 * @param optr:A BinaryOperator.
	 * @param key:A scalar or vector, indicate the key of the elements to set.
	 * @param value:A scalar or vector, assume that has the same size as key.
	 * @return True if reduce succeed, else false.
	 */
	virtual bool reduce(BinaryOperator& optr, const ConstantSP& key, const ConstantSP& value)=0;
	/**
	 * @brief Apply optr between value and the elements of this dictionary according to key in specified heap.
	 * 		  If there is a new key to this dictionary, the value of the key will be set to the result of applying initOptr on value.
	 *
	 * @param optr:A BinaryOperator.
	 * @param key:A scalar or vector, indicate the key of the elements to set.
	 * @param value:A scalar or vector, assume that has the same size as key.
	 * @return True if reduce succeed, else false.
	 */
	virtual bool reduce(Heap* heap, const FunctionDefSP& optr, const FunctionDefSP& initOptr, const ConstantSP& key, const ConstantSP& value)=0;
	virtual bool modifyMember(Heap* heap, const FunctionDefSP& func, const ConstantSP& index, const ConstantSP& parameters, int dim){return false;}
	/**
	 * @brief Get specified elements according to key.
	 *
	 * @param key:A scalar or vector, indicate the key of the elements to return.
	 * @return ConstantSP: The specified elements.
	 */
	virtual ConstantSP get(const ConstantSP& key) const {return getMember(key);}
	virtual void contain(const ConstantSP& target, const ConstantSP& resultSP) const = 0;
	virtual bool isLargeConstant() const {return true;}
	/**
	 * @brief Return the underlying map of this dictionary.
	 *
	 * @return A pointer to the map.
	 */
	virtual void* getRawMap() const = 0;
	/**
	 * @brief Return whether the dictionary is ordered.
	 */
	virtual bool isOrdered() const = 0;
	/**
	 * @brief Return the lock of this dictionary.
	 *
	 * @return A pointer to the lock.
	 */
	inline Mutex* getLock() const { return lock_;}
	/**
	 * @brief Set the lock of this dictionary.
	 *
	 * @param lock:A pointer to the lock.
	 */
	inline void setLock(Mutex* lock) { lock_ = lock;}

private:
	Mutex* lock_;
};

class SWORDFISH_API Table: public Constant{
public:
	/// Once you overload a function (virtual function or normal function) from Base class
	/// in Derived class all functions with the same name in the Base class get hidden in
	/// Derived class.
	/// ref: https://stackoverflow.com/questions/8816794/overloading-a-virtual-function-in-a-child-class
	using Constant::get;

public:
	Table() : Constant(DF_TABLE, DT_DICTIONARY, MIXED), flag_(0), engineType_((char)DBENGINE_TYPE::OLAP), lock_(0){}
	virtual ~Table();
	virtual ConstantSP getIterator(const ConstantSP& self) const;
	virtual string getScript() const {return getName();}
	/**
	 * @brief Get specified column according to column name.
	 *
	 * @param name: A column name.
	 * @return ConstantSP: The specified column.
	 */
	virtual ConstantSP getColumn(const string& name) const = 0;
	/**
	 * @brief Get specified column according to column name.
	 *
	 * @param qualifier: The name of this table.
	 * @param name: A column name.
	 * @return ConstantSP: The specified column.
	 */
	virtual ConstantSP getColumn(const string& qualifier, const string& name) const = 0;
	/**
	 * @brief Get specified column according to index.
	 *
	 * @param index: A column index.
	 * @return ConstantSP: The specified column.
	 */
	virtual ConstantSP getColumn(INDEX index) const = 0;
	/**
	 * @brief Get specified column and rows according to column name and rowFilter.
	 *
	 * @param name: A column name.
	 * @param rowFilter: If it is a pair, indicate the range of rows to get.
	 * 					 If it is a index vector, indicate the indices of rows to get.
	 * @return ConstantSP: The specified column.
	 */
	virtual ConstantSP getColumn(const string& name, const ConstantSP& rowFilter) const = 0;
	/**
	 * @brief Get specified column and rows according to column name and rowFilter.
	 *
	 * @param qualifier: The name of this table.
	 * @param name: A column name.
	 * @param rowFilter: If it is a pair, indicate the range of rows to get.
	 * 					 If it is a index vector, indicate the indices of rows to get.
	 * @return ConstantSP: The specified column.
	 */
	virtual ConstantSP getColumn(const string& qualifier, const string& name, const ConstantSP& rowFilter) const = 0;
	/**
	 * @brief Get specified column and rows according to column index and rowFilter.
	 *
	 * @param index: A column index.
	 * @param rowFilter: If it is a pair, indicate the range of rows to get.
	 * 					 If it is a index vector, indicate the indices of rows to get.
	 * @return ConstantSP: The specified column.
	 */
	virtual ConstantSP getColumn(INDEX index, const ConstantSP& rowFilter) const = 0;
	virtual INDEX columns() const = 0;
	/**
	 * @brief Return name of the specified column.
	 */
	virtual const string& getColumnName(int index) const = 0;
	/**
	 * @brief Get the qualifier of origin table according to column index.
	 * 		  Usually used in a join table.
	 *
	 * @param index: A column index.
	 */
	virtual const string& getColumnQualifier(int index) const = 0;
	/**
	 * @brief Set name of the specified column.
	 */
	virtual void setColumnName(int index, const string& name)=0;
	/**
	 * @brief Return index of the specified column.
	 */
	virtual int getColumnIndex(const string& name) const = 0;
	/**
	 * @brief Return data type of the specified column.
	 */
	virtual DATA_TYPE getColumnType(int index) const = 0;
	/**
	 * @brief Return the extra attribute of the specified column.
	 */
	virtual int getColumnExtraParam(int index) const { throw RuntimeException("Table::getColumnExtraParam() not supported"); }
	/**
	 * @brief Return whether this table contain a column with the specified name.
	 */
	virtual bool contain(const string& name) const = 0;
	/**
	 * @brief Return true when qualifier equal to the name of this table
	 * 		  and this table contain a column with specified name.
	 */
	virtual bool contain(const string& qualifier, const string& name) const = 0;
	/**
	 * @brief Get the qualifier and name from col.
	 * 		  Return true when qualifier equal to the name of this table
	 * 		  and this table contain a column with specified name.
	 */
	virtual bool contain(const ColumnRef* col) const = 0;
	/**
	 * @brief Get the qualifier and name from col.
	 * 		  Return true when qualifier equal to the name of this table
	 * 		  and this table contain a column with specified name.
	 */
	virtual bool contain(const ColumnRefSP& col) const = 0;
	/**
	 * @brief Get the qualifier and name from each ColumnRef.
	 * 		  Return true when each qualifier equal to the name of this table
	 * 		  and this table contain a column with specified name.
	 */
	virtual bool containAll(const vector<ColumnRefSP>& cols) const = 0;
	/**
	 * @brief Set the name of this table.
	 */
	virtual void setName(const string& name)=0;
	/**
	 * @brief Return the name of this table.
	 */
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
	/**
	 * @brief Return the table type of this table.
	 */
	virtual TABLE_TYPE getTableType() const = 0;
	/**
	 * @brief Add columns to this table.
	 * 		  Note that each name of columns should be differert from this table.
	 *
	 * @return True if join succeed, else false.
	 */
	virtual bool join(vector<ConstantSP>& columns) { return false;}
	virtual	bool clear() { return false;}
	/**
	 * @brief Reorder the columns of the table with the given new orders.
	 *
	 * @param newOrders: indices of the columns of the new orders.
	 * @return True if reorder succeed, else false.
	 */
	virtual bool reorderColumns(const vector<int>& newOrders) { return false;}
	/**
	 * @brief Replace the specified column of this table with new column.
	 *
	 * @param index: The index of replaced column.
	 * @param col: New column.
	 * @return True if replace succeed, else false.
	 */
	virtual bool replaceColumn(int index, const ConstantSP& col) {return false;}
	/**
	 * @brief Drop specified columns.
	 *
	 * @param columns: Indices of the columns to drop
	 * @return True if drop succeed, else false
	 */
	virtual bool drop(vector<int>& columns) {return false;}
	/**
	 * @brief Remove rows from this table according to filterExprs.
	 *
	 * @param Heap:
	 * @param context:
	 * @param filterExprs:A MetaCode vector that indicate which rows need to remove.
	 */
	virtual void remove(Heap* heap, const SQLContextSP& context, const ConstantSP& filterExprs) {throw RuntimeException("Table::remove() not supported");}
	/**
	 * @brief Sort some columns of this table.
	 *
	 * @param Heap:
	 * @param sortExpr: An expression that indicate the columns need to sort.
	 * @param sortOrder: A bool scalar, indicate sort by ascending order(true) or descending order(false).
	 */
	virtual void sortBy(Heap* heap, const ObjectSP& sortExpr, const ConstantSP& sortOrder) {throw RuntimeException("Table::sortBy() not supported");}
	/**
	 * @brief Update some columns of this table according to updateExpr and filterExprs.
	 *
	 * @param Heap:
	 * @param context:
	 * @param updateColNames: A string vector, indicate the column names need to update.
	 * @param updateExpr: An expression that indicate how to update columns.
	 * @param filterExprs:A MetaCode vector that indicate which rows need to update.
	 */
	virtual void update(Heap* heap, const SQLContextSP& context, const ConstantSP& updateColNames, const ObjectSP& updateExpr, const ConstantSP& filterExprs) {throw RuntimeException("Table::update() not supported");}
	/**
	 * @brief Update some columns of this table with new values according to index.
	 *
	 * @param values: New value columns, assume that the vector size of values the same as colNames.
	 * @param indexSP: Indices of rows.
	 * @param colNames: Column names need to update.
	 * @param errMsg: If the append fails, the error message is stored in errMsg.
	 * @return True if update succeed, eles false.
	 */
	virtual bool update(vector<ConstantSP>& values, const ConstantSP& indexSP, vector<string>& colNames, string& errMsg) = 0;
	/**
	 * @brief Append values to this table.
	 *
	 * @param values: The values to append.
	 * @param insertedRows: Out parameter, means how many rows are actually inserted.
	 * @param errMsg: If the append fails, the error message is stored in errMsg.
	 * @return True if append succeed, else false.
	 */
	virtual bool append(vector<ConstantSP>& values, INDEX& insertedRows, string& errMsg) = 0;
	/**
	 * @brief Append values to this table.
	 *
	 * @param heap:
	 * @param values: The values to append.
	 * @param insertedRows: Out parameter, means how many rows are actually inserted.
	 * @param errMsg: If the append fails, the error message is stored in errMsg.
	 * @return True if append succeed, else false.
	 */
    virtual bool append(Heap* heap, vector<ConstantSP>& values, INDEX& insertedRows, string& errMsg){
        return append(values, insertedRows, errMsg);
    }
	/**
	 * @brief Remove rows from this table.
	 *
	 * @param indexSP: Indices of rows to remove.
	 * @param errMsg: If the romove fails, the error message is stored in errMsg.
	 * @return True if romove succeed, eles false.
	 */
	virtual bool remove(const ConstantSP& indexSP, string& errMsg) = 0;
	/**
	 * @brief Insert rows into a keyed table or indexed table if the values of the
	 * 			primary key do not already exist, or update them if they do.
	 *
	 * @param values: The values to upsert.
	 * @param ignoreNull: If set to true, for the NULL value in values, the correponding elements are not updated.
	 * @param insertedRows: Out parameter, means how many rows are actually upsert.
	 * @param errMsg: If the append fails, the error message is stored in errMsg.
	 * @return True if romove succeed, eles false.
	 */
	virtual bool upsert(vector<ConstantSP>& values, bool ignoreNull, INDEX& insertedRows, string& errMsg) {throw RuntimeException("Table::upsert() not supported");}
	virtual bool upsert(vector<ConstantSP>& values, bool ignoreNull, INDEX& insertedRows, INDEX& updatedRows,
						string& errMsg) {
		throw RuntimeException("Table::upsert() not supported");
	}
	virtual DATA_TYPE getRawType() const {return DT_DICTIONARY;}
	/**
	 * @brief Return whether this table is a distributed table.
	 */
	virtual bool isDistributedTable() const {return false;}
	/**
	 * @brief Return whether this table is a segmented table.
	 */
	virtual bool isSegmentedTable() const {return false;}
	/**
	 * @brief Return whether this table is a dimensional table.
	 */
	virtual bool isDimensionalTable() const {return false;}
	/**
	 * @brief Return whether this table is a basic table.
	 */
	virtual bool isBasicTable() const {return false;}
	/**
	 * @brief Return whether this table is a dfs table.
	 */
	virtual bool isDFSTable() const {return false;}
	/**
	 * @brief Return whether this table could be append.
	 */
	virtual bool isAppendable() const {return false;}
	/**
	 * @brief Return whether this table could be append.
	 */
	virtual bool isEditable() const {return false;}
	/**
	 * @brief Return whether the schema of this table could be edit.
	 */
	virtual bool isSchemaEditable() const {return false;}
	/**
	 * @brief Return the count of sort keys of this table.
	 */
	virtual int getSortKeyCount() const { return 0;}
	/**
	 * @brief Return the column index of the index-th sort key in this table.
	 *
	 * @param index: The index of sort key.
	 * @return The column index of this table.
	 */
	virtual int getSortKeyColumnIndex(int index){return -1;}
	/**
	 * @brief Return whether the index-th sort key is in ascending order.
	 *
	 * @param index: The index of sort key.
	 */
	virtual int isAscendingKey(int index) { return true;}
	/**
	 * @brief Return the domain of a shared dfs table.
	 *
	 * @param index: The index of sort key.
	 */
	virtual DomainSP getGlobalDomain() const {return DomainSP();}
	/**
	 * @brief Return the domain of this table.
	 */
	virtual DomainSP getLocalDomain() const {return DomainSP();}
	/**
	 * @brief Return the partitioning column index of this table.
	 */
	virtual int getGlobalPartitionColumnIndex() const {return -1;}
	/**
	 * @brief Return the partitioning column index of this table according to domain dimension.
	 *
	 * @param dim: The dimension of the composite domain of this table.
	 */
	virtual int getLocalPartitionColumnIndex(int dim) const {return -1;}
	/**
	 * @brief Set the global domain of this table.
	 *
	 * @param domain: The global domain.
	 * @param partitionColumn: The name of partitioning column of this table.
	 */
	virtual void setGlobalPartition(const DomainSP& domain, const string& partitionColumn){throw RuntimeException("Table::setGlobalPartition() not supported");}
	virtual bool isLargeConstant() const {return true;}
	/**
	 * @brief Add subscriber to this stream table.
	 *
	 * @param queue:
	 * @param topic: The subscription task name.
	 * @param local: Whether the subscribe is from localhost.
	 * @param offset: The position of the first message where the subscription begins.
	 * @param filter: Selected values in the filtering column.
	 * @return True if subscribe succeed, eles false.
	 */
	virtual bool addSubscriber(const TableUpdateQueueSP& queue, const string& topic, bool local, long long offset = -1, const ObjectSP& filter = 0) { return false;}
	/**
	 * @brief Remove subscriber from this stream table according to topic name.
	 *
	 * @param queue:
	 * @param topic: The subscription task name.
	 * @return True if remove succeed, eles false.
	 */
	virtual bool removeSubscriber(const TableUpdateQueueSP& queue, const string& topic) { return false;}
	/**
	 * @brief Return whether the subscribe exists according to subscription task name.
	 */
	virtual bool subscriberExists(const TableUpdateQueueSP& queue, const string& topic) const { return false;}
	/**
	 * @brief Release the usage count of this table.
	 */
	virtual void release() const {}
	/**
	 * @brief Add the usage count of this table.
	 */
	virtual void checkout() const {}
	/**
	 * @brief Retrieve the data of a partition from this dfs table.
	 *
	 * @param heap:
	 * @param partition: The partition information.
	 * @param guard: The memory of the returned table will not be released until the guard destructed.
	 * @return An in-memory table containing data for the specified partition.
	 */
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
	/**
	 * @brief Return whether the specified partition exists in this table.
	 *
	 * @param partition : The partition information.
	 */
	virtual bool segmentExists(const DomainPartitionSP& partition) const { throw RuntimeException("Table::segmentExists() not supported");}
	/**
	 * @brief Return how many partitions the current table contains.
	 */
	virtual int getPartitionCount() const { throw RuntimeException("Table::getPartitionCount() not supported"); }
	virtual long long getAllocatedMemory() const = 0;
	/**
	 * @brief Retrieve message from this table.
	 *
	 * @param offset: The position of the first message where the retrieve begins.
	 * @param length: The max length of the returned messages.
	 * @param msgAsTable: If true, messages are returned in the form of table.
	 * 					  If false, messages are returned in the form of any vector.
	 * @param filter: The returned messages will be filtered with filter.
	 * @param messageId: An out parameter, return the index of the last message in this table.
	 * @return The specified messages.
	 */
	virtual ConstantSP retrieveMessage(long long offset, int length, bool msgAsTable, const ObjectSP& filter, long long& messageId) { throw RuntimeException("Table::retrieveMessage() not supported"); }
	virtual INDEX getFilterColumnIndex() const { return -1; };
	/**
	 * @brief Return whether this table is snapshot isolate.
	 */
	virtual bool snapshotIsolate() const { return false;}
	/**
	 * @brief Get the snapshot of this table.
	 *
	 * @param copy: An out parameter, return the snapshot of this table.
	 */
	virtual void getSnapshot(TableSP& copy) const {}
	/**
	 * @brief Returns whether the user has read permission on this table.
	 */
	virtual bool readPermitted(const AuthenticatedUserSP& user) const {return true;}
	/**
	 * @brief Returns whether this table is expired.
	 */
	virtual bool isExpired() const { return flag_ & 8;}
	virtual void transferAsString(bool option){throw RuntimeException("Table::transferAsString() not supported");}
	/**
	 * @brief Returns the count of key column of this table.
	 */
	virtual int getKeyColumnCount() const { return 0;}
	/**
	 * @brief Return the column index of the index-th key column in this table.
	 *
	 * @param index: The index of key column.
	 * @return The column index of this table.
	 */
	virtual int getKeyColumnIndex(int index) const { throw RuntimeException("Table::getKeyColumnIndex() not supported");}
	/**
	 * @brief Returns the index of time keyed column of this table.
	 */
	virtual int getKeyTimeColumnIndex() const { throw RuntimeException("Table::getKeyTimeColumnIndex() not supported");}
	/**
	 * @brief Returns the chunk path of this table.
	 */
	virtual string getChunkPath() const { return "";}
	/**
	 * @brief Set this table to shared.
	 */
	virtual void share(){};

	/**
	 * @brief Filter the table by a set of column and value relationships.
	 *
	 * @param filterExprs: An in/out parameter. If a filter is applied, it will be removed from filters.
	 * @param limit : 0: no limit on each key,
	 *				  positive: first n rows,
	 *				  negative: last n rows.
	 * @param byKey: true: limit by key, false: global limit.
	 * @return An index vector. If no filter is applied or applied filters satisfy all rows of the table, return a null pointer.
	 */
	virtual ConstantSP filter(vector<ObjectSP>& filterExprs, INDEX limit = 0, bool byKey = true) const { return nullptr;}
	/**
	 * @brief Return whether this table support block filter.
	 */
	virtual bool supportBlockFilter() const {return false;}
	/**
	 * @brief Prepare data in advance.
	 *
	 * @param rows: The row indices. If rows is a null pointer, load all rows. The indices are in ascending order.
	 * @param cols: The column indices. if empty, load all columns. The indices are in ascending order.
	 * @return: True if this table supports the feature.
	 */
	virtual bool prepareData(const ConstantSP& rows, const vector<int>& cols) { return false;}

	/**
     * @brief Group by the data according to sortkeys.
     *
     * @param groupBy: an in-out parameter. If there are sortKeys in groupBy columns, then we fill group the data by sortkeys, and remove
     * @param sortKey: columns from groupBy.
     * @param filter: an input parameter. The filter for the whole table. If the filter is specified, it must be in strictly ascending order.
     * @param tablets: an outout parameter. If there are sortKeys in groupBy columns, then we will group the data and save them in tablets,
     * 				   i.e. each tablet in tablets is a group. If there is no sortKey column in groupBy, then tablets will be empty.
     * @param removedGroupBys: an output parameter. The groupBy objects for sortkeys. These objects are removed from the input groupBy.
     * @param groupedFilters: an output parameter. the filter for each tablet.
     */
    virtual void groupBySortKeys(vector<string>& groupBy, const ConstantSP& filter, vector<TableSP>& tablets, vector<string>& removedGroupBys, vector<ConstantSP>& groupedFilters){}

	/**
	 * @brief Return the row duplicate policy of this table.
	 */
    virtual DUPLICATE_POLICY getRowDuplicatePolicy() const { return DUPLICATE_POLICY::KEEP_ALL;}
	virtual vector<FunctionDefSP> getPartitionFunction() const { return {}; }

	/**
	 * @brief Return whether this table is shared table.
	 */
    inline bool isSharedTable() const {return flag_ & 1;}
	/**
	 * @brief Return whether this table is stream table.
	 */
	inline bool isRealtimeTable() const {return flag_ & 2;}
	/**
	 * @brief Return whether this table is alias table.
	 */
	inline bool isAliasTable() const {return flag_ & 4;}
	/**
	 * @brief Return the name of the owner of this table.
	 */
	inline const string& getOwner() const {return owner_;}
	/**
	 * @brief Set the name of the owner of this table.
	 */
	inline void setOwner(const string& owner){ owner_ = owner;}
	/**
	 * @brief Return the storage engine type of this table.
	 */
	inline DBENGINE_TYPE getEngineType() const { return (DBENGINE_TYPE)engineType_;}
	/**
	 * @brief Set the storage engine type of this table.
	 */
	inline void setEngineType(DBENGINE_TYPE engineType) { engineType_ = (char)engineType;}
	/**
	 * @brief Set the share attribute of this table.
	 *
	 * @param option: True if set to shared, else false.
	 */
	void setSharedTable(bool option);
	/**
	 * @brief Set the table type flag of this table.
	 *
	 * @param option: True if set to stream table, else false.
	 */
	void setRealtimeTable(bool option);
	/**
	 * @brief Set the table type flag of this table.
	 *
	 * @param option: True if set to alias table, else false.
	 */
	void setAliasTable(bool option);
	/**
	 * @brief Set the expired flag of this table.
	 *
	 * @param option: True if set to expired, else false.
	 */
	void setExpired(bool option);
	/**
	 * @brief Return the lock of this dictionary.
	 *
	 * @return A pointer to the lock.
	 */
	inline Mutex* getLock() const { return lock_;}
	/**
	 * @brief Returns whether the user has write permission on this table.
	 */
	virtual bool writePermitted(const AuthenticatedUserSP& user) const { return true; }
	/**
	 * @brief Returns the physical name of this table.
	 */
    virtual const string& getPhysicalName() const {return getName();}
	/**
	 * @brief Set the access control flag of this table.
	 *
	 * @param option: True if the operation on this table requires corresponding permissions, else false.
	 */
	virtual void setAccessControl(bool option) {
		if (option) {
			flag_ |= 16;
		}
		else {
			flag_ &= ~16;
		}
	}
	/**
	 * @brief Returns whether the operation on this table requires corresponding permissions.
	*/
	virtual bool isAccessControl() const { return flag_ & 16; }

    /**
     * @brief We may use a random string as the name of tables with no user input names. We also would like to hide this
	 * internal name from the users. Hence, we set this flag.
	*/
    void setTableUsingInternalName() {
		flag_ |= 32;
    }
	/**
	 * @brief Reset the flag to indicate that the table is going to have a valid input name.
	*/
    void unsetTableUsingInternalName() {
		flag_ &= ~32;
    }

    string getTableNameForSerialization() const {
        // only return the getName() when it is not an internal name.
        // otherwise, there is no user input name, return an empty string.
        if (flag_ & 32) {
            return "";
        } else {
			return getName();
		}
	}
	/**
	 * @brief Begin a new query transaction.
	 *
	 * @param pHeap: The heap of the related transaction.
	 * @param originalTable: The smart pointer of the current table.
	 *
	 * @return: The new version of the table (optional).
	 */
    virtual TableSP beginQueryTransaction(Heap* pHeap, const TableSP& originalTable) { return nullptr; }
	/**
	 * @brief Begin a new update transaction.
	 *
	 * @param pHeap: The heap of the related transaction.
	 * @param originalTable: The smart pointer of the current table.
	 *
	 * @return: The new version of the table (optional).
	 */
    virtual TableSP beginUpdateTransaction(Heap* pHeap, const TableSP& originalTable) { return nullptr; }
	/**
	 * @brief Begin a new delete transaction.
	 *
	 * @param pHeap: The heap of the related transaction.
	 * @param originalTable: The smart pointer of the current table.
	 *
	 * @return: The new version of the table (optional).
	 */
    virtual TableSP beginDeleteTransaction(Heap* pHeap, const TableSP& originalTable) { return nullptr; }
    /**
     * @brief Retrieve data.
     *
     * @param pHeap: The heap.
     * @param filters: The filter conditions.
     * @param colNames: The desired columns.
     * @param originalTable: The smart pointer of the current table.
     *
     * @return: The retrieved data.
     */
    virtual TableSP scan(
            Heap* pHeap,
            std::vector<ObjectSP> &filters,
            const std::vector<std::string> &colNames,
            const TableSP& originalTable
    ) { return originalTable; }
    /**
     * @brief Update data.
     *
     * @param pHeap: The heap.
     * @param filters: The filter conditions.
     * @param updateCols: The columns to update.
     */
    virtual void update(
            Heap* pHeap,
            std::vector<ObjectSP> &filters,
            const std::vector<ColumnDefSP> &updateCols
    ) { throw RuntimeException("Table::update() not supported"); }
    /**
     * @brief Remove data.
     *
     * @param pHeap: The heap.
     * @param filters: The filter conditions.
     */
    virtual void remove(
            Heap* pHeap,
            std::vector<ObjectSP> &filters
    ) { throw RuntimeException("Table::remove() not supported"); }
    /**
     * @brief Get a filter according to the conditions.
     *
     * @param pHeap: The heap.
     * @param filters: The filter conditions.
     */
    virtual ConstantSP getFilter(
            Heap* pHeap,
            std::vector<ObjectSP> &filters
    ) const { throw RuntimeException("Table::getFilter() not supported"); }
    /**
     * @brief Get the size of block start indices (for IotTablet).
     */
    virtual int getBlockStartIdxsSize() { throw RuntimeException("Table::getBlockStartIdxsSize() not supported"); }
    /**
     * @brief Filter top-K vector (for IotTablet)
     *
     * @param inputFilter: The input inddex filter.
     * @param queryVec: The query vector.
	 * @param k
	 * @param sorted: is sorted
     */
    virtual ConstantSP filterTopKVector(const ConstantSP& inputFilter, const VectorSP & queryVec, int k, bool& sorted) { throw RuntimeException("Table::filterTopKVector() not supported"); }
    /**
     * @brief Return whether a column is sorted.
     *
     * @param colName: The column name.
     */
    virtual bool isSortedOnColumn(const string& colName) const { throw RuntimeException("Table::isSortedOnColumn() not supported"); };
    /**
     * @brief set cast iot any column
     *
     * @param colName: The column name.
     */
	virtual void setCastIotAnyCol(bool option) { throw RuntimeException("Table::setCastIotAnyCol() not supported"); }
private:
    /*
     * BIT0: shared table
     * BIT1: stream table
     * BIT2: alias table
     * BIT3: expired
     * BIT4: access control or not
     * BIT5: user internal name or not
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

class SysObj : public Constant {
public:
	SysObj(SYSOBJ_TYPE type);
	inline SYSOBJ_TYPE getSysObjType() const { return type_;}
	virtual DATA_TYPE getRawType() const {return DT_OBJECT;}
	virtual IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const;
	virtual bool isView() const {return false;}

protected:
	void setSysObjType(SYSOBJ_TYPE type) { type_ = type;}

private:
	SYSOBJ_TYPE type_;
};

class OOClass : public SysObj {
public:
	OOClass(const string& qualifier, const string& name, bool builtin, SYSOBJ_TYPE type);
	virtual ~OOClass(){}
	virtual ConstantSP getInstance() const { return getValue();}
	const string& getName() const { return name_;}
	const string& getQualifier() const { return qualifier_;}
	void setQulifier(const string &s) { qualifier_ = s; }
	string getFullName() const;
	inline bool isBuiltin() const { return flag_ & 1;}
	virtual void collectUserDefinedFunctionsAndClasses(Heap* pHeap, unordered_map<string,FunctionDef*>& functionDefs, unordered_map<string,OOClass*>& classes) const;
	virtual void collectInternalUserDefinedFunctionsAndClasses(Heap* pHeap, unordered_map<string,FunctionDef*>& functionDefs, unordered_map<string,OOClass*>& classes) const = 0;
	virtual FunctionDefSP getMethod(const string& name) const = 0;
	virtual FunctionDefSP getOperator(const string& name) const = 0;
	virtual bool hasMethod(const string& name) const = 0;
	virtual bool hasOperator(const string& name) const = 0;
	virtual void getMethods(vector<FunctionDefSP>& methods) const = 0;
	virtual ConstantSP getMember(const string& key) const = 0;
	virtual ConstantSP getMember(const ConstantSP& key) const { return getMember(key->getString());}
	virtual string getString() const;
	virtual IO_ERR serializeClass(const ByteArrayCodeBufferSP& buffer) const = 0;
	virtual IO_ERR deserializeClass(Session* session, const DataInputStreamSP& in) = 0;

	/** Call class constructor, return an instance of this class. */
	virtual ConstantSP call(Heap *heap, const ConstantSP &self, vector<ObjectSP> &arguments) = 0;
	virtual ConstantSP call(Heap *heap, const ConstantSP &self, vector<ConstantSP> &arguments) = 0;

	/**
	 * ATTENTION: The extended classes must not override this method. FunctionDef and Class are
	 * two special citizen in our code. We always serialize classes and function definitions separately
	 * before serializing other code. For this reason, we simply serialize the class name for an
	 * OO class, and the function name for a function definition.
	 *
	 */
	virtual IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const;
	static ConstantSP createOOClass(Session* session, const DataInputStreamSP& in);
	static string composeFullName(const string& name, const string& qualifier);
	static bool decomposeFullName(const string& fullName, string& name, string& qualifier);

protected:
	inline void setBuiltin(bool option) { if(option) flag_ |= 1; else flag_ &= ~1;}
	OOClass(const string& qualifier, const string& name, int flag,  SYSOBJ_TYPE type);

protected:
	string name_;
	string qualifier_;
	int flag_;
};

class OOInstance : public SysObj {
public:
	OOInstance(const OOClassSP& ooClass, SYSOBJ_TYPE type);
	OOClassSP getClass() const { return class_;}
	virtual ConstantSP getInstance() const { return getValue();}
	virtual FunctionDefSP getMethod(const string& name) const { return class_->getMethod(name);}
	virtual bool hasMethod(const string& name) const { return class_->hasMethod(name);}
	virtual FunctionDefSP getOperator(const string& name) const { return class_->getOperator(name);}
	virtual ConstantSP getMember(const string& key) const = 0;
	virtual ConstantSP getMember(const ConstantSP& key) const { return getMember(key->getString());}
	virtual void collectUserDefinedFunctionsAndClasses(Heap* pHeap, unordered_map<string,FunctionDef*>& functionDefs, unordered_map<string,OOClass*>& classes) const;

protected:
	OOClassSP class_;
};

class SWORDFISH_API Param{
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

class SWORDFISH_API FunctionDef : public Constant{
public:
	using JitFunc = void (*)(char*, char*, char*);
	using TurboJetFunc = void (*)(char *, Heap *, char *, int);
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
	inline void setSequentialFunction(bool option) {if(option) extraFlag_ |= 8; else extraFlag_ &= ~8;}
	inline void setScalarFunction(bool option){ if(option) extraFlag_ |= 16; else extraFlag_ &= ~16;}
	inline bool isScalarFunction() const { return extraFlag_ & 16;}
	inline void setHigherOrderFunction(bool option){ if(option) extraFlag_ |= 32; else extraFlag_ &= ~32;}
	inline bool isHigherOrderFunction() const { return extraFlag_ & 32;}
	inline void setArrayAggFunction(bool option){ if(option) extraFlag_ |= 64; else extraFlag_ &= ~64;}
	inline bool isArrayAggFunction() const { return extraFlag_ & 64;}
	inline void setParserType(PARSER_TYPE type) { parserType_ = type;}
	inline PARSER_TYPE getParserType() const { return parserType_;}
	inline void setGPU(bool option){ if(option) extraFlag_ |= 128; else extraFlag_ &= ~128;}
	inline bool isGPU() const { return extraFlag_ & 128;}
	inline void setOOMethod(bool option){ if(option) extraFlag_ |= 256; else extraFlag_ &= ~256;}
	inline bool isOOMethod() const { return extraFlag_ & 256;}
	inline void setOOConstructor(bool option){ if(option) extraFlag_ |= 512; else extraFlag_ &= ~512;}
	inline bool isOOConstructor() const { return extraFlag_ & 512;}
	inline void setStaticMember(bool option){ if(option) extraFlag_ |= 1024; else extraFlag_ &= ~1024;}
	inline bool isStaticMember() const { return extraFlag_ & 1024;}
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
	virtual std::tuple<DATA_FORM, DATA_TYPE, JitFunc> getJitFuncPtr(Heap *heap, std::vector<ConstantSP> testArgs) { return std::make_tuple(DF_SCALAR, DT_VOID, nullptr); }
	virtual void registerTurboJetImplementation(TurboJetFunc funcPtr) { throw RuntimeException("registerTurboJetImplementation not implemented."); }
	virtual TurboJetFunc getTurboJetJitFuncPtr() const { throw RuntimeException(getScript() + " is not a function that have a JIT-compatible implementation or can be JIT-compiled."); }
	virtual ConstantSP getTurboJetReturnType() const { return nullptr; }
	virtual void registerTurboJetReturnType(ConstantSP type) {}

protected:
	void setConstantParameterFlag();
	void setReturnValueFlag(bool val);
	void setAggregationFlag(bool val);
	void setSequentialFlag(bool val);
	void setTransformFlag(bool val);

protected:
	static ParamSP constParam_;

	FUNCTIONDEF_TYPE defType_;
	PARSER_TYPE parserType_;
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
	 * bit9: 0: cast IotAny column, 1: don't cast IotAny column
	 */
	int flag_;
	DictionarySP cachedCols_;
	SQLTransactionSP transSP_;
	vector<INDEX>* groups_;
};

class ColumnRef: public Object{
public:
	ColumnRef(const SQLContextSP& contextSP, const string& name): Object(OBJECT_TYPE::COLUMN), contextSP_(contextSP),name_(name),index_(-1),acceptFunctionDef_(true){}
	ColumnRef(const SQLContextSP& contextSP, const string& qualifier, const string& name): Object(OBJECT_TYPE::COLUMN), contextSP_(contextSP),
			qualifier_(qualifier),name_(name),index_(-1),acceptFunctionDef_(true){}
	ColumnRef(const SQLContextSP& contextSP, const string& name, int index): Object(OBJECT_TYPE::COLUMN), contextSP_(contextSP),name_(name),index_(index),acceptFunctionDef_(true){}
	ColumnRef(const SQLContextSP& contextSP, const string& qualifier, const string& name, int index): Object(OBJECT_TYPE::COLUMN), contextSP_(contextSP),
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
	virtual void collectUserDefinedFunctionsAndClasses(Heap* pHeap, unordered_map<string,FunctionDef*>& functionDefs, unordered_map<string,OOClass*>& classes) const = 0;

protected:
	FunctionDefSP funcDef_;
	string templateSymbol_;

private:
	int priority_;
	bool unary_;
};

class SWORDFISH_API Output {
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

class DummyOutput: public Output {
public:
	virtual ~DummyOutput(){}
    bool timeElapsed(long long nanoSeconds) override {return true;}
    bool write(const ConstantSP& obj) override {return true;}
    bool message(const string& msg) override {return true;}
    void enableIntermediateMessage(bool enabled) override {}
    bool start() override {return true;}
    bool start(const string& message) override {return true;}
    IO_ERR done() override {return OK;}
    IO_ERR done(const string& errMsg) override {return OK;}
    IO_ERR writeReady() override {return OK;}
    OUTPUT_TYPE getOutputType() const override {return STDOUT;}
    void close() override {}
    void setWindow(INDEX index,INDEX size) override {};
    IO_ERR flush() override {return OK;}
};

class SWORDFISH_API Session {
public:
	Session(const HeapSP& heap, PARSER_TYPE parserType = PARSER_TYPE::DDB);
	Session(const HeapSP& heap, bool systemSession, PARSER_TYPE parserType = PARSER_TYPE::DDB);
	virtual ~Session(){}
	virtual SessionSP getValue() = 0;
	virtual SessionSP copy(bool forComputing = false) = 0;
	virtual bool run(const vector<string>& source, const string& currentPath = "", int firstLine = 0)=0;
	virtual bool run(const string& scriptFile)=0;
	virtual bool run(const ObjectSP& script)=0;
	virtual bool run(const ObjectSP& script, ConstantSP& result)=0;
	virtual bool run(const string& function, vector<ConstantSP>& params)=0;
	virtual bool run(const FunctionDefSP& function, vector<ConstantSP>& params)=0;
	virtual bool run(const vector<string>& variables, vector<ConstantSP>& params)=0;

    /**
     * @brief Parser inputr for dolphin terminal.
     * @return Exec flag, if flag == -1, the dolphin terminal will exit.
     * if flag == 0: do nothing, the dolphin terminal will continue to accept input.
     * if flag == 1: the dolphin terminal will run the scirpt, and the lines will be clear.
     */
	virtual char stdRun(vector<string>& lines, std::istream& sin, std::ostream& sout) = 0;
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
	PARSER_TYPE getParserType() const { return parserType_;}
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
	virtual void clear() = 0;
	virtual SysFunc getTableJoiner(const string& name) const = 0;
	virtual FunctionDefSP getFunctionDef(const string& name) const = 0;
	virtual FunctionDefSP getBuiltinFunctionDef(const string& name) const = 0;
	virtual FunctionDefSP getFunctionView(const string& name) const = 0;
	virtual TemplateOptr getTemplateOperator(const string& name) const = 0;
	virtual TemplateUserOptr getTemplateUserOperator(const string& name) const = 0;
	virtual OptrFunc getOperator(const string& optrSymbol, bool unary, const string& templateName) const = 0;
	virtual OperatorSP getOperator(const FunctionDefSP& func, bool unary) const = 0;
	virtual OOClassSP getOOClass(const string& name, const string& qualifier = "") const = 0;
	virtual bool addOOClass(const OOClassSP& ooClass) = 0;
	virtual bool addPluginFunction(const FunctionDefSP& funcDef) = 0;
	virtual bool replaceFunction(const FunctionDefSP& funcDef) = 0;
	virtual void addFunctionDeclaration(FunctionDefSP& func) = 0;
	virtual void addFunctionalView(const FunctionDefSP& funcDef) = 0;
	virtual bool removeFunctionalView(const string& name) = 0;
	virtual void completePendingFunctions(bool commit) = 0;
	virtual void completePendingClasses(bool commit) = 0;
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
	inline const string& getCurrentCatalog() const { return currentCatalog_;}
	inline void setCurrentCatalog(const string& catalog){currentCatalog_ = catalog;}

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
	string currentCatalog_;

private:
	PARSER_TYPE parserType_;
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
	const SessionSP getSession(PARSER_TYPE parserType) const { return sessions_[(int)parserType];}
	void setSession(const SessionSP& session);
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
	inline PARSER_TYPE getParserType() const { return (PARSER_TYPE)((flag_ >> 11) & 15);}
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
	/**
	 * The console keeps one session for each parser, for example Python and DDB.
	 */
	vector<SessionSP> sessions_;
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
	 * bit11~14 the parser type
	 * bit15: 0: pickle table to dataFrame    1: pickle table to list
	 * bit16: 0: disable tracing, 1: enable tracing
	 * bit17: 0: api, 1: stream subscription
	 * bit18: reserved for output format
	 * bit19~22: sql standard, 0: dolphindb, 1: oracle, 2: mysql
	 */
	long long flag_;
    Guid parentSpanId_;
};

class SWORDFISH_API ConstantMarshal {
public:
	virtual ~ConstantMarshal(){}
	virtual bool start(const ConstantSP& target, bool blocking, IO_ERR& ret)=0;
	virtual bool start(const char* requestHeader, size_t headerSize, const ConstantSP& target, bool blocking, IO_ERR& ret)=0;
	virtual bool resume(IO_ERR& ret)=0;
	virtual void reset() = 0;
	virtual IO_ERR flush() = 0;
};

class SWORDFISH_API ConstantUnmarshal{
public:
	virtual ~ConstantUnmarshal(){}
	virtual bool start(short flag, bool blocking, IO_ERR& ret)=0;
	virtual bool resume(IO_ERR& ret)=0;
	virtual void reset() = 0;
	ConstantSP getConstant(){return obj_;}

protected:
	ConstantSP obj_;
};

class SWORDFISH_API Heap{
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
	inline bool isReturnMode() const { return status_ & 4;}
	inline void setReturnMode(bool enabled = true) { if(enabled) status_ |= 4; else status_ &= ~4;}
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
	// For OOP
	ConstantSP getSelf() { return self_; }
	void setSelf(ConstantSP &self) { self_ = self; }

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
	// For OOP
	ConstantSP self_;
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
	virtual void collectUserDefinedFunctionsAndClasses(Heap* pHeap, unordered_map<string,FunctionDef*>& functionDefs, unordered_map<string,OOClass*>& classes) const {}
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
    void setStatementType(STATEMENT_TYPE type) { type_ = type; }
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
	virtual void registerStatementCreator(STATEMENT_TYPE stType, StatementFunc func) = 0;
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
    using IndexMap = std::map<int, vector<std::pair<string, string>>>;

	TableHeader() = default;
	TableHeader(const string& owner, const string& physicalIndex, const vector<ColumnDesc>& tablesType,
		const vector<int>& partitionKeys, const vector<FunctionDefSP>& partitionFunc, CIPHER_MODE mo = CIPHER_MODE::PLAIN_TEXT, const int64_t keyVer = 0, const string& encryptKey = ""):
        owner(owner), physicalIndex(physicalIndex), colDescs(tablesType), partitionKeys(partitionKeys),
        partitionFunction(partitionFunc), mode(mo), keyVersion(keyVer),encryptedTableKey(encryptKey) {}
	TableHeader(const string& owner, const string& physicalIndex, const vector<ColumnDesc>& tablesType,
			const vector<int>& partitionKeys, const vector<pair<int, bool>>& sortKeys,
			DUPLICATE_POLICY rowDuplicatePolicy, const vector<pair<int, FunctionDefSP>>& sortKeyMappingFunction,
			bool appendForDelete, const string& tableComment, const vector<FunctionDefSP>& partitionFunc,
            const vector<int> &primaryKeys, const IndexMap &indexes, bool latestKeyCache, bool compressHashSortKey, CIPHER_MODE mo = CIPHER_MODE::PLAIN_TEXT,
			const int64_t keyVer = 0, const string& encryptKey = "", const vector<SensitiveColumn>& scols = {}):
			rowDuplicatePolicy(rowDuplicatePolicy), owner(owner), physicalIndex(physicalIndex), colDescs(tablesType),
			partitionKeys(partitionKeys), sortKeys(sortKeys), sortKeyMappingFunction(sortKeyMappingFunction),
			appendForDelete(appendForDelete), tableComment(tableComment), partitionFunction(partitionFunc),
            primaryKeys(primaryKeys), indexes(indexes), latestKeyCache(latestKeyCache), compressHashSortKey(compressHashSortKey), mode(mo),
			keyVersion(keyVer),encryptedTableKey(encryptKey), sensitiveCol(scols)  {}
	DUPLICATE_POLICY rowDuplicatePolicy = DUPLICATE_POLICY::KEEP_ALL;
	string owner;
	string physicalIndex;
	vector<ColumnDesc> colDescs;
	vector<int> partitionKeys;
	vector<pair<int, bool>> sortKeys;
	vector<pair<int, FunctionDefSP>> sortKeyMappingFunction;
	bool appendForDelete = false;
	string tableComment;
	vector<FunctionDefSP> partitionFunction;
    vector<int> primaryKeys;
    IndexMap indexes;

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
	virtual int getPartitionCount() const { return partitions_.size();}
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
	virtual void retrievePartitionsIn(const ConstantSP& values, vector<DomainPartitionSP>& partitions, bool localOnly, const FunctionDefSP& partitionFunction = nullptr) const = 0;
	virtual void retrievePartitionAt(const ConstantSP& value, vector<DomainPartitionSP>& partitions, bool localOnly, const FunctionDefSP& partitionFunction = nullptr) const = 0;
	virtual void retrieveAllPartitions(vector<DomainPartitionSP>& partitions, bool localOnly) const;
	virtual IO_ERR saveDomain(const string& filename) const;
	virtual IO_ERR saveDomain(const DataOutputStreamSP& out) const = 0;
	virtual ConstantSP getPartitionKey(const ConstantSP& partitionColumn) const = 0;
	virtual bool equals(const DomainSP& domain) const = 0;
	virtual DATA_TYPE getPartitionColumnType(int dimIndex = 0) const = 0;
	virtual int getPartitionDimensions() const { return 1;}
	virtual DomainSP getDimensionalDomain(int dimension) const;
	virtual DomainSP copy() const = 0;
	bool addTable(const string& tableName, const string& owner, const string& physicalIndex, vector<ColumnDesc>& cols,
				  vector<int>& partitionColumns, const vector<FunctionDefSP>& partitionFunction);
	bool addTable(const string& tableName, const TableHeader& tableHeader);
	bool getTable(const string& tableName, string& owner, string& physicalIndex, vector<ColumnDesc>& cols,
				  vector<int>& partitionColumns, vector<FunctionDefSP>& partitionFunction) const;
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
	TableUpdate(const string& topic, long long offset, int length, int flag, const TableSP& table, const TableSP& logTable) : topic_(topic), offset_(offset), length_(length), flag_(flag), filter_(0), table_(table), logTable_(logTable), isBarrier_(false){}
	TableUpdate(const string& topic, long long offset, int length, int flag, const ObjectSP& filter, const TableSP& table, const TableSP& logTable) : topic_(topic), offset_(offset), length_(length), flag_(flag), filter_(filter), table_(table), logTable_(logTable), isBarrier_(false){}
	TableUpdate() : offset_(0), length_(0), flag_(0), filter_(0), table_(0), logTable_(0), isBarrier_(false) {}
	string topic_;
	long long offset_;
	int length_;
	int flag_;
	ObjectSP filter_;
	TableSP table_;
	// for multicast publish, logTable_ is always nullptr and use table_ for LogRowTable
	TableSP logTable_;
	// check barrier info, default value is false
	bool isBarrier_;
	string barrierStr_;
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
			const string& userId = "", const string& pwd = "", long long sessionID = 0, const bool& multicast = false) : msgAsTable_(msgAsTable),
			persistOffset_(persistOffset), timeTrigger_(timeTrigger), handlerNeedMsgId_(handlerNeedMsgId), hashValue_(hashValue), batchSize_(batchSize),
			throttleTime_(throttleTime), userId_(userId), pwd_(pwd), sessionID_(sessionID), cumSize_(0), messageId_(-1), expired_(-1), topic_(topic), attributes_(attributes), handler_(handler), user_(user), multicast_(multicast){}
	bool append(long long msgId, const ConstantSP& msg, long long& outMsgId, ConstantSP& outMsg);
	bool getMessage(long long now, long long& outMsgId, ConstantSP& outMsg);
	bool updateSchema(const TableSP& emptyTable);
	bool isUnsubscribed() { return isUnsubscribed_; }
	void setUnsubscribed() { isUnsubscribed_ = true; }
    void setSubscribed() { isUnsubscribed_ = false; }
	bool isOrca() const { return isOrcaSubscription_; }
	void setOrca(bool isOrca) { isOrcaSubscription_ = isOrca; }

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
    bool multicast_ = false;
	bool isOrcaSubscription_ = false;
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
}

namespace std {
	template<>
	struct hash<ddb::ControlFlowEdge> {
		std::size_t operator()(const ddb::ControlFlowEdge & k) const {
			using std::size_t;
			using std::hash;
			using std::string;
			return hash<void*>{}(k.edgeFrom)
				   ^ (hash<void*>{}(k.edgeTo) >> 1)
				   ^ (hash<string>{}(k.varName) << 1);
		}
	};

	template<>
	struct hash<ddb::InferredType> {
		std::size_t operator()(const ddb::InferredType & k) const {
			using std::size_t;
			using std::hash;
			using std::string;
			return hash<int8_t>{}(k.form) ^ hash<int8_t>{}(k.type);
		}
	};
}
#endif /* CORECONCEPT_H_ */
