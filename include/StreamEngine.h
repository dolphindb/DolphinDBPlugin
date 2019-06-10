//
// Created by jccai on 19-5-31.
//

#ifndef DOLPHINDB_STREAMENGINE_H
#define DOLPHINDB_STREAMENGINE_H

#include <Util.h>
#include <ScalarImp.h>
#include "CoreConcept.h"
#include "SmartPointer.h"

class AbstractStreamEngine;
using AbstractStreamEngineSP = SmartPointer<AbstractStreamEngine>;

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
    AbstractStreamEngine(const string &type, const string &name, const string &user);

    AbstractStreamEngine(const string &type, const string &name, const string &user,
                         const SmartPointer<vector<string>> &colNames);

    AbstractStreamEngine(const string &type, const string &name, const string &user,
                         const SmartPointer<vector<string>> &colNames, SmartPointer<unordered_map<string, int>> colMap);

    ~AbstractStreamEngine() override;

    /// (optional) implement addMetrics if your engine support it
    /// arguments[0]: meta code, new metrics
    /// arguments[1]: table, indicating the scheme of the new metrics
    virtual bool addMetrics(Heap *heap, vector<ConstantSP> &arguments) { return false; }

    /// (optional) called when removed from StreamEngineManager
    /// should be non-blocking, e.g. set some flags and return immediately
    virtual void finalize() {}

    /// (required) the scheme of the generated table must be consistent with the engineStat_ vector
    /// generateEngineStatTable will be called asynchronously by StreamEngineManager, should be thread safe
    /// e.g.
    /// TableSP YourEngine::generateEngineTable() {
    ///     static vector<string> colNames{"name", "user", "status", "lastErrMsg", ...};
    ///     vector<ConstantSP> cols;
    ///     cols.push_back(Util::createVector(DT_STRING, 0));
    ///     cols.push_back(Util::createVector(DT_STRING, 0));
    ///     cols.push_back(Util::createVector(DT_STRING, 0));
    ///     cols.push_back(Util::createVector(DT_STRING, 0));
    ///     ...
    ///     return Util::createTable(colNames, cols);
    /// }
    virtual TableSP generateEngineStatTable() = 0;

    /// (required) initialize the engineStat_ which is used as a buffer
    /// must be called immediately and only once after the engine's construction
    /// e.g.
    /// void YourEngine::initEngineStat() {
    ///     engineStat_.push_back(new String(engineName_));
    ///     engineStat_.push_back(new String(engineUser_));
    ///     engineStat_.push_back(new String(status_));
    ///     engineStat_.push_back(new String(lastErrMsg_));
    ///     ...
    /// }
    virtual void initEngineStat() = 0;

    /// (required) engine statues are represented by a Table
    /// updateEngineStat will be called asynchronously by StreamEngineManager, should be thread safe
    /// updateEngineStat should only update the changed fields in the engineStat_ to reduce overhead
    /// e.g.
    /// void YourEngine::updateEngineStat() {
    ///     LockGuard<Mutex> guard(&mutex_);
    ///     engineStat_[2]->setString(status_);
    ///     engineStat_[3]->setString(lastErrMsg_);
    ///     ...
    /// }
    virtual void updateEngineStat() = 0;

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

    ConstantSP getColumn(INDEX index) const override {return title_;}
    const string &getColumnName(int index) const override { return colNames_->at(index); }
    DATA_TYPE getColumnType(int index) const override { return DT_STRING; }
    INDEX columns() const override { return colNames_->size(); }
    INDEX size() const override { return 0; }
    const string &getColumnQualifier(int index) const override { return name_; }
    TABLE_TYPE getTableType() const override { return CUSTOMIZEDTBL; }
    long long int getAllocatedMemory() const override { return 0; }
    ConstantSP get(INDEX col, INDEX row) const override { throw RuntimeException("get not supported"); }

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

    vector<ConstantSP> engineStat_;

    string name_;

    ConstantSP getInternal(INDEX index) const;
    ConstantSP getInternal(const ConstantSP& index) const;
    ConstantSP getMemberInternal(const ConstantSP& key) const;
private:
    void initialize();
    ConstantSP title_;
    SmartPointer<vector<string>> colNames_;
    SmartPointer<unordered_map<string, int>> colMap_;
};

#endif //DOLPHINDB_STREAMENGINE_H
