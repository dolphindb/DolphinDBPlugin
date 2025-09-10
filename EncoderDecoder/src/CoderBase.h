#ifndef CODER_RESOURCE_H
#define CODER_RESOURCE_H

#include "DolphinDBEverything.h"
#include "EncoderDecoder.h"
#include "ddbplugin/Plugin.h"
#include <CoreConcept.h>
#include <Exceptions.h>
#include <DolphinClass.h>
#include <ScalarImp.h>
#include <SysIO.h>
#include <Types.h>
#include <Util.h>
#include "protobufUtil.h"

using namespace ddb;

using std::string;
using std::vector;

using BatchProcessorSP = SmartPointer<BatchProcessor>;

class CoderImpl;

class CoderImplClass : public DolphinClass {
  public:
    CoderImplClass(FunctionDefSP funcWrapper, const string &instName): DolphinClass("EncoderDecoder", instName), funcWrapper_(funcWrapper) {
    }
    virtual FunctionDefSP getMethod(const string& name) const override;
    virtual bool hasMethod(const string& name) const override;
	virtual string getString() const override {
		return DolphinClass::getString();
	}
  private:
    FunctionDefSP funcWrapper_;
};

class CoderImpl : public DolphinInstance {
private:
    SessionSP session_;
    FunctionDefSP func_;
    FunctionDefSP funcWrapper_;
    ConstantSP handler_;
    TableSP dummyTable_;
    BatchProcessorSP batchProc_;

public:
    CoderImpl(Session* session, const DolphinClassSP& ddbClass, FunctionDefSP func, FunctionDefSP funcWrapper)
        : DolphinInstance(ddbClass), func_(func), funcWrapper_(funcWrapper)
    {
        SessionSP ssn = session->copy();
        ssn->setUser(session->getUser());
        session_ = ssn;
        dummyTable_ = Util::createTable({ "col1" }, { Util::createConstant(DT_BOOL) });
        handler_ = new Void();
    }
// "coder instance", "EncoderDecoder"
    CoderImpl(Session* session, const DolphinClassSP& ddbClass, FunctionDefSP func, FunctionDefSP funcWrapper, ConstantSP handler, TableSP dummyTable, int workerNum, int batchSize, int timeout)
        : DolphinInstance(ddbClass)
        , func_(func)
        , funcWrapper_(funcWrapper)
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
    string getString() const {return "coder instance"; };
};

class EncoderClass : public DolphinClass {
public:
    explicit EncoderClass(const string &instName): DolphinClass("EncoderDecoder", instName) {}
    FunctionDefSP getMethod(const string& name) const override;
    bool hasMethod(const string& name) const override;
    string getString() const override { return DolphinClass::getString(); }
};

class EncoderInstance : public DolphinInstance {
public:
    explicit EncoderInstance(DolphinClassSP cls) : DolphinInstance(cls) {}
    void initialize(const std::string& filePath, const std::string& protoName);
    ConstantSP protobufSerialize(ConstantSP obj);
    string getString() const override{return "encoder instance"; }

private:
    google::protobuf::DescriptorPool pool;
    google::protobuf::DynamicMessageFactory factory;
    const google::protobuf::Message* protoType_;
};

#endif //CODER_RESOURCE_H
