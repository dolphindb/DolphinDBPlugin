//
// Created by jccai on 19-5-31.
//

#ifndef DOLPHINDB_STREAMENGINE_H
#define DOLPHINDB_STREAMENGINE_H

#include "Util.h"
#include "ScalarImp.h"
#include "CoreConcept.h"
#include "SmartPointer.h"

class ReactiveState;
class AbstractStreamEngine;

typedef SmartPointer<ReactiveState> ReactiveStateSP;
typedef SmartPointer<AbstractStreamEngine> AbstractStreamEngineSP;

typedef ReactiveStateSP(*StateFuncFactory)(const vector<ObjectSP>& args, const vector<int>& inputColIndices, const vector<DATA_TYPE>& inputColTypes, const vector<int>& outputColIndices);

class StreamEngineManager {
public:
    StreamEngineManager(const StreamEngineManager &) = delete;
    StreamEngineManager operator=(const StreamEngineManager &) = delete;
    static StreamEngineManager& instance();

    void insert(const AbstractStreamEngineSP &aggregator);
    void remove(const string &name);
    AbstractStreamEngineSP find(const string &name);
    ConstantSP getStat();
private:
    StreamEngineManager() = default;
    mutable Mutex mutex_;
    unordered_map<string, AbstractStreamEngineSP> engines_;
    unordered_set<string> engineTypes_;
};

class AbstractStreamEngine : public Table {
public:
    AbstractStreamEngine(const string &type, const string &name, const string &user, const TableSP& dummy, const string& snapshotDir = "", long long snapshotIntervalInMsgCount = LLONG_MAX);
    ~AbstractStreamEngine() override;

    /**
     * (optional) implement addMetrics if your engine support it
     * arguments[0]: meta code, new metrics
     * arguments[1]: table, indicating the scheme of the new metrics
     */
    virtual bool addMetrics(Heap *heap, vector<ConstantSP> &arguments) { return false; }

    /**
     * (optional) called when removed from StreamEngineManager
     * should be non-blocking, e.g. set some flags and return immediately
     */
    virtual void finalize() {}

    /**
     * (optional) use historical data (state) to warm up the stream engine.
     * Warmup is same as formal messages except that warmup doesn't produce
     * any message.
     */
    virtual bool warmup(vector<ConstantSP>& values, string& errMsg){
    	errMsg = "warmup() not supported.";
    	return false;
    }

    /**
     * (required) save the current state of the stream engine
     */
    virtual IO_ERR snapshotState(const DataOutputStreamSP& out) = 0;

    /**
     * (required) restore the state of the stream engine
     */
    virtual IO_ERR restoreState(const DataInputStreamSP& in) = 0;

    /**
     * (required) the scheme of the generated table must be consistent with the engineStat_ vector
     * generateEngineStatTable will be called asynchronously by StreamEngineManager, should be thread safe
     */
    virtual TableSP generateEngineStatTable() = 0;

    /**
     * (required) initialize the engineStat_ which is used as a buffer
     * must be called immediately and only once after the engine's construction
     */
    virtual void initEngineStat() = 0;

    /**
     * (required) engine statues are represented by a Table
     * updateEngineStat will be called asynchronously by StreamEngineManager, should be thread safe
     * updateEngineStat should only update the changed fields in the engineStat_ to reduce overhead
     */
    virtual void updateEngineStat() = 0;

    virtual void appendMsg(const ConstantSP& body, long long msgId);
    virtual void restoreEngineState();

    string getEngineType();
    string getEngineName();
    string getEngineCreator();

    vector<ConstantSP> &getEngineStatRef();

    bool sizeable() const override {
        return false;
    }

    bool update(vector<ConstantSP>& values, const ConstantSP& indexSP, vector<string>& colNames, string& errMsg) override {
        errMsg = "StreamEngine doesn't support data update.";
        return false;
    }

    bool remove(const ConstantSP& indexSP, string& errMsg) override {
        errMsg = "StreamEngine doesn't support data deletion.";
        return false;
    }

    ConstantSP getColumn(INDEX index) const override {return dummy_->getColumn(index);}
    const string &getColumnName(int index) const override { return colNames_->at(index); }
    DATA_TYPE getColumnType(int index) const override { return dummy_->getColumnType(index); }
    INDEX columns() const override { return colNames_->size(); }
    INDEX size() const override { return 0; }
    const string &getColumnQualifier(int index) const override { return name_; }
    TABLE_TYPE getTableType() const override { return CUSTOMIZEDTBL; }
    long long int getAllocatedMemory() const override { return 0; }
    ConstantSP get(INDEX col, INDEX row) const override { throw RuntimeException("get() not supported"); }
    ConstantSP getColumn(const string &name) const override;
    ConstantSP getColumn(const string &qualifier, const string &name) const override;
    ConstantSP getColumn(const string &name, const ConstantSP &rowFilter) const override;
    ConstantSP getColumn(const string& qualifier, const string& name, const ConstantSP& rowFilter) const override;
    ConstantSP getColumn(INDEX index, const ConstantSP& rowFilter) const override;
    void setColumnName(int index, const string& name) override;
    int getColumnIndex(const string& name) const override;
    bool contain(const string& name) const override;
    bool contain(const string& qualifier, const string& name) const override;
    bool contain(ColumnRef* col) const override;
    bool contain(const ColumnRefSP& col) const override;
    bool containAll(const vector<ColumnRefSP>& cols) const override;
    ConstantSP getColumnLabel() const override;
    ConstantSP keys() const override;
    ConstantSP values() const override;
    string getString(INDEX index) const override;
    string getString() const override;
    bool set(INDEX index, const ConstantSP& value) override;
    void setName(const string &name);
    ConstantSP get(INDEX index) const override;
    ConstantSP get(const ConstantSP &index) const override;
    const string &getName() const override;
    ConstantSP getInstance() const override;
    ConstantSP getInstance(INDEX size) const override;
    ConstantSP getValue() const override;
    ConstantSP getValue(Heap *heap) override;
    ConstantSP getValue(INDEX capacity) const override;
    ConstantSP getReference(Heap *heap) override;
    ConstantSP getWindow(INDEX colStart, int colLength, INDEX rowStart, int rowLength) const override;
    ConstantSP getMember(const ConstantSP &key) const override;
    long long getSnapshotMessageId() const { return snapshotMessageId_;}

protected:
    const string engineType_;
    const string engineName_;
    const string engineUser_;

    /// Engine should update these fields. Basically a streaming engine should be confined to one thread
    /// thus AbstractStreamEngine::append should be guarded by a mutex
    /// these fields are recommended to be guarded by the same mutex
    string status_ = "OK";
    string lastErrMsg_ = "";
    uint64_t cumMessages_ = 0;
    uint64_t cumMessagesSnapshot_ = 0;
    uint64_t snapshotThreshold_ = LLONG_MAX;
    long long snapshotMessageId_ = -1;
    long long snapshotTimestamp_ = LLONG_MIN;
    string snapshotDir_;

    vector<ConstantSP> engineStat_;
    string name_;

    ConstantSP getInternal(INDEX index) const;
    ConstantSP getInternal(const ConstantSP& index) const;
    ConstantSP getMemberInternal(const ConstantSP& key) const;

private:
    void initialize();
    SmartPointer<vector<string>> colNames_;
    SmartPointer<unordered_map<string, int>> colMap_;
    TableSP dummy_;
};

class ReactiveState {
public:
	virtual ~ReactiveState(){}
	virtual IO_ERR snapshotState(const DataOutputStreamSP& out) = 0;
	virtual IO_ERR restoreState(const DataInputStreamSP& in) = 0;
	virtual void append(Heap* heap, const ConstantSP& keyIndex) = 0;
	virtual void addKeys(int count) = 0;
	virtual void getMemoryUsed(long long& fixedMemUsed, long long& variableMemUsedPerKey) = 0;
	void setTable(const TableSP& table){
		table_ = table;
	}
	TableSP getTable() const {
		return table_;
	}

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

	void setString(int outputColIndex, INDEX* indices, int count, DolphinString* buf);

protected:
	TableSP table_;
};

class ReactiveParser {
public:
	ReactiveParser(const string& title, const TableSP& dummy, const vector<string>& keyColumns, const TableSP& outputTable, const string& snapshotDir, long long snapshotIntervalInMsgCount, bool keepOrder);
	AbstractStreamEngineSP parse(Heap* heap, const vector<ObjectSP>& metrics, const ObjectSP& filter);

	static ReactiveStateSP createReactiveState(const FunctionDefSP& func, vector<ObjectSP>& args, vector<int>& inputColIndices, vector<DATA_TYPE>& inputColTypes, vector<int>& outputColIndices);
	static bool registerReactiveState(const string& name, StateFuncFactory f, int outputColumns = 1);
	static bool registerStateFunctionReturnType(const string& name, DATA_TYPE type);
	static bool isStateFunction(const FunctionDefSP& func);
	static bool isStateFunction(const FunctionDefSP& func, int& outputColumns);
	static DATA_TYPE getStateFunctionReturnType(const FunctionDefSP& func, DATA_TYPE type);
	static void initialize();

private:
	ObjectSP parseObject(Heap* heap, const ObjectSP& metric, unordered_map<int, ObjectSP>* pindexMap);
	void reset();
	void addTestColumn(const string& name, DATA_TYPE type);
	void parseUDF(Heap* heap, const FunctionDefSP& func, const vector<ObjectSP>& args, vector<ObjectSP>& returnObjs);


private:
	vector<int> keyColIndex_;
	string title_;
	TableSP dummy_;
	TableSP outputTable_;
	string snapshotDir_;
	long long snapshotIntervalInMsgCount_;
	TableSP testTable_;
	SymbolBaseSP symbase_;
	SQLContextSP context_;
	vector<ReactiveStateSP> states_;
	unordered_map<string, int> metricMap_;
	unordered_map<string, int> columnMap_;
	unordered_map<string, ObjectSP> multiReturnMap_;
	vector<string> colNames_;
	vector<DATA_TYPE> colTypes_;
	vector<int> inputCols_;
	vector<int> stateCols_;
	vector<int> outputCols_;
	bool keepOrder_;
	static LocklessFlatHashmap<string, pair<StateFuncFactory, int>> STATE_FUNCS;
	static LocklessFlatHashmap<string, DATA_TYPE> STATEFUNC_TYPES;
};

class ReactiveStreamEngine : public AbstractStreamEngine {
public:
	ReactiveStreamEngine(const SessionSP& session, const string& title, const string& user, const vector<int>& keyColIndex, const TableSP& dummyTable, const TableSP& stateTable,
			const TableSP& outputTable,	vector<ReactiveStateSP>& states, vector<int>& initInputCols, vector<int>& initStateCols, vector<int>& outputCols, int filterColIndex,
			const string& snapshotDir, long long snapshotIntervalInMsgCount, bool keepOrder);
    virtual IO_ERR snapshotState(const DataOutputStreamSP& out);
    virtual IO_ERR restoreState(const DataInputStreamSP& in);
	virtual bool append(vector<ConstantSP>& values, INDEX& insertedRows, string& errMsg);
	virtual bool warmup(vector<ConstantSP>& values, string& errMsg);
    virtual TableSP generateEngineStatTable();
    virtual void initEngineStat();
    virtual void updateEngineStat();
    virtual long long getAllocatedMemory() const;

    static ConstantSP createReactiveStateEngine(Heap *heap, vector<ConstantSP> &arguments);
    static void warmupStreamEngine(Heap *heap, vector<ConstantSP> &arguments);

private:
	void addKeys(int count);
	DolphinString* retrieveKeys(const vector<VectorSP>& keyVectors);
	template<class T>
	bool handleMessage(T& msg, bool outputMsg, INDEX& insertedRows, string& errMsg);
	template<class T>
	bool trigger(T& msg, INDEX offset, INDEX length, const ConstantSP& keyIndex, int newKeys, bool outputMsg, string& errMsg);
	void sortKeyIndex(const vector<bool>& keyUsed, const ConstantSP& tmpIndices, int keyCount);
	INDEX prepareFilterIndex(const ConstantSP& keyIndex, INDEX length);

private:
	vector<int> keyColIndex_;
	int inputColCount_;
	int groupCount_;
	TableSP stateTable_;
	TableSP outputTable_;
	vector<ReactiveStateSP> states_;
    vector<int> initInputCols_;
    vector<int> initStateCols_;
    vector<int> outputCols_;
    int filterColIndex_;
	unordered_map<DolphinString, INDEX> keyIndex_;
	vector<bool> keyUsed_;
	vector<DolphinString> keyBuf_;
	ConstantSP tmpIndices_;
	ConstantSP originalIndices_;
	VectorSP filterIndices_;
	Mutex mutex_;
	SessionSP session_;
	bool resetKeyUsed_;
	bool outputAsDownstream_;
	bool keepOrder_;
	long long fixedMemUsed_;
	long long varialeMemUsedPerKey_;
	int keyCount_;
	Mutex* outputLock_;
};

#endif //DOLPHINDB_STREAMENGINE_H
