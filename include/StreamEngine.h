//
// Created by jccai on 19-5-31.
//

#ifndef DOLPHINDB_STREAMENGINE_H
#define DOLPHINDB_STREAMENGINE_H

#include "Util.h"
#include "ScalarImp.h"
#include "CoreConcept.h"
#include "SmartPointer.h"
#include "SysIO.h"

class AbstractStreamEngine;

typedef SmartPointer<AbstractStreamEngine> AbstractStreamEngineSP;
typedef AbstractStreamEngineSP(*StreamEngineFactory)(Heap* heap, const DataInputStreamSP& in);


class StreamEngineManager {
public:
    StreamEngineManager(const StreamEngineManager &) = delete;
    StreamEngineManager operator=(const StreamEngineManager &) = delete;
    static StreamEngineManager& instance();

    void insert(const AbstractStreamEngineSP &aggregator);
    void remove(const string &name);
    AbstractStreamEngineSP find(const string &name);
    ConstantSP getStat();
    void registerEngineFactory(const string& engineType, StreamEngineFactory factory);
    StreamEngineFactory getEngineFactory(const string& engineType) const;
    AbstractStreamEngineSP createStreamEngine(Heap* heap, const DataInputStreamSP& in) const;

private:
    StreamEngineManager() = default;
    mutable Mutex mutex_;
    unordered_map<string, AbstractStreamEngineSP> engines_;
    unordered_set<string> engineTypes_;
    unordered_map<string, StreamEngineFactory> engineFactories_;
};

class AbstractStreamEngine : public Table {
public:
    AbstractStreamEngine(const string &type, const string &name, const string &user, const TableSP& dummy, const string& snapshotDir = "", long long snapshotIntervalInMsgCount = LLONG_MAX, int raftGroup = -1);
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

    virtual IO_ERR serialize(BufferSP& buffer) const {
    	throw RuntimeException("AbstractStreamEngine::serialize not implemented yet.");
    }

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

    string getEngineType() const;
    string getEngineName() const;
    string getEngineCreator() const;
    int getRaftGroup() const { return raftGroup_;}

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
    int getColumnExtraParam(int index) const override { return dummy_->getColumnExtraParam(index);}
    INDEX columns() const override { return colNames_->size(); }
    INDEX size() const override { return 0; }
    const string &getColumnQualifier(int index) const override { return name_; }
    TABLE_TYPE getTableType() const override { return STREAMENGINE; }
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
    bool contain(const ColumnRef* col) const override;
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
    virtual bool readPermitted(const AuthenticatedUserSP& user) const;
    virtual bool writePermitted(const AuthenticatedUserSP& user) const;
    virtual bool isJoinEngine() {return false;}

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
    int raftGroup_;

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

#endif //DOLPHINDB_STREAMENGINE_H
