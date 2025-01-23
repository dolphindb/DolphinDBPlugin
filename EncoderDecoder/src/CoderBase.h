#ifndef CODER_RESOURCE_H
#define CODER_RESOURCE_H

#include "EncoderDecoder.h"
#include "ddbplugin/Plugin.h"
#include <CoreConcept.h>
#include <Exceptions.h>
#include <ScalarImp.h>
#include <SysIO.h>
#include <Types.h>
#include <Util.h>

using BatchProcessorSP = SmartPointer<BatchProcessor>;

class CoderImpl : public Table {
private:
    SessionSP session_;
    FunctionDefSP func_;
    ConstantSP handler_;
    TableSP dummyTable_;
    BatchProcessorSP batchProc_;

public:
    CoderImpl(Session* session, FunctionDefSP func)
        : func_(func)
    {
        SessionSP ssn = session->copy();
        ssn->setUser(session->getUser());
        session_ = ssn;
        dummyTable_ = Util::createTable({ "col1" }, { Util::createConstant(DT_BOOL) });
        handler_ = new Void();
    }

    CoderImpl(Session* session, FunctionDefSP func, ConstantSP handler, TableSP dummyTable, int workerNum, int batchSize, int timeout)
        : func_(func)
        , handler_(handler)
        , dummyTable_(dummyTable)
    {
        SessionSP ssn = session->copy();
        ssn->setUser(session->getUser());
        session_ = ssn;
        batchProc_ = new BatchProcessor(ssn, handler, dummyTable, func, workerNum, batchSize, timeout);
    }
    ~CoderImpl();
    void appendTable(ConstantSP items) const;
    virtual bool append(vector<ConstantSP>& values, INDEX& insertedRows, string& errMsg);
    virtual ConstantSP callMethod(const string& name, Heap* heap, vector<ConstantSP>& args) const;

    FunctionDefSP getFunction() const { return func_; };
    virtual string getString() const { return "coder instance"; };
    virtual string getString(INDEX index) const { return dummyTable_->getString(index); };
    virtual ConstantSP get(const ConstantSP& index) const { return dummyTable_->get(index); };
    virtual ConstantSP getColumn(INDEX index) const { return dummyTable_->getColumn(index); };
    virtual ConstantSP getColumn(const string& name) const { return dummyTable_->getColumn(name); };
    virtual ConstantSP getColumn(const string& qualifier, const string& name) const { return dummyTable_->getColumn(qualifier, name); };
    virtual ConstantSP getColumn(INDEX index, const ConstantSP& rowFilter) const { return dummyTable_->getColumn(index, rowFilter); };
    virtual ConstantSP getColumn(const string& name, const ConstantSP& rowFilter) const { return dummyTable_->getColumn(name, rowFilter); };
    virtual ConstantSP getColumn(const string& qualifier, const string& name, const ConstantSP& rowFilter) const { return dummyTable_->getColumn(qualifier, name, rowFilter); };
    virtual ConstantSP getWindow(INDEX colStart, int colLength, INDEX rowStart, int rowLength) const { return dummyTable_->getWindow(colStart, colLength, rowStart, rowLength); };
    virtual bool sizeable() const { return dummyTable_->sizeable(); };
    virtual INDEX size() const { return dummyTable_->size(); };
    virtual INDEX columns() const { return dummyTable_->columns(); };
    virtual ConstantSP getMember(const ConstantSP& key) const { return dummyTable_->getMember(key); };
    virtual ConstantSP keys() const { return dummyTable_->keys(); };
    virtual ConstantSP values() const { return dummyTable_->values(); };
    virtual long long getAllocatedMemory() const { return dummyTable_->getAllocatedMemory(); };
    virtual ConstantSP getValue() const { return dummyTable_->getValue(); };
    virtual ConstantSP getValue(INDEX capacity) const { return dummyTable_->getValue(capacity); };
    virtual const string& getColumnName(int index) const { return dummyTable_->getColumnName(index); };
    virtual const string& getColumnQualifier(int index) const { return dummyTable_->getColumnQualifier(index); };
    virtual void setColumnName(int index, const string& name) { return dummyTable_->setColumnName(index, name); };
    virtual int getColumnIndex(const string& name) const { return dummyTable_->getColumnIndex(name); };
    virtual bool contain(const string& qualifier, const string& name) const { return dummyTable_->contain(qualifier, name); };
    virtual bool contain(const ColumnRef* col) const { return dummyTable_->contain(col); };
    virtual bool contain(const ColumnRefSP& col) const { return dummyTable_->contain(col); };
    virtual bool contain(const string& name) const { return dummyTable_->contain(name); };
    virtual bool containAll(const vector<ColumnRefSP>& cols) const { return dummyTable_->containAll(cols); };
    virtual void setName(const string& name) { return dummyTable_->setName(name); };
    virtual bool remove(const ConstantSP& indexSP, string& errMsg) { return dummyTable_->remove(indexSP, errMsg); };
    virtual DATA_TYPE getColumnType(int index) const { return dummyTable_->getColumnType(index); };
    virtual TABLE_TYPE getTableType() const { return dummyTable_->getTableType(); };
    virtual ConstantSP getInstance(INDEX size) const { return dummyTable_->getInstance(size); };
    virtual ConstantSP getColumn(const string& name, const ConstantSP& rowFilter) { return dummyTable_->getColumn(name, rowFilter); };
    virtual const string& getName() const { return dummyTable_->getName(); };
    virtual bool update(vector<ConstantSP>& values, const ConstantSP& indexSP, vector<string>& colNames, string& errMsg) { return dummyTable_->update(values, indexSP, colNames, errMsg); };
    virtual ConstantSP getInstance() const { return ((ConstantSP)dummyTable_)->getInstance(); };
};

#endif //CODER_RESOURCE_H