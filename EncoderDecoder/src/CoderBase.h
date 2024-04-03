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

class CoderImpl;
class DolphinClass;
typedef SmartPointer<DolphinClass> DolphinClassSP;
class DolphinClass : public OOClass {
public:
    DolphinClass(const string& qualifier, const string& name, DolphinClassSP baseCls = nullptr);
    DolphinClass(const string& qualifier, const string& name, bool builtin);
    virtual ~DolphinClass(){}
    int getAttributeIndex(const string& name) const;
    int getPublicAttributeIndex(const string& name) const;
    const string& getAttribute(int index) const { return attributes_[index].first;}
    int getAttributeCount() const { return attributes_.size();}
    bool addAttributeWithType(const string& string, OO_ACCESS access, const ConstantSP &typeObj);
    bool addAttribute(const string& string, OO_ACCESS access);
    bool addAttributes(const vector<string>&, OO_ACCESS access);
    bool addMethod(const FunctionDefSP& method, OO_ACCESS access);
    bool addMethods(const vector<FunctionDefSP>& methods, OO_ACCESS access);
    bool overrideMethod(const std::string &methodName, const FunctionDefSP& method);
    int getMethodIndex(const string& name) const;
    virtual void collectInternalUserDefinedFunctionsAndClasses(Heap* pHeap, unordered_map<string,FunctionDef*>& functionDefs, unordered_map<string,OOClass*>& classes) const;
    virtual ConstantSP getValue() const;
    virtual bool hasMethod(const string& name) const;
    virtual bool hasOperator(const string& name) const;
    virtual FunctionDefSP getMethod(const string& name) const;
    virtual FunctionDefSP getMethod(INDEX index) const;
    virtual FunctionDefSP getOperator(const string& name) const;
    virtual void getMethods(vector<FunctionDefSP>& methods) const;
    virtual ConstantSP getMember(const string& key) const;
    virtual IO_ERR serializeClass(const ByteArrayCodeBufferSP& buffer) const;
    virtual IO_ERR deserializeClass(Session* session, const DataInputStreamSP& in);

    /** Call class constructor, return an instance of this class. */
    virtual ConstantSP call(Heap *heap, const ConstantSP &self, vector<ObjectSP> &arguments) override;
    virtual ConstantSP call(Heap *heap, const ConstantSP &self, vector<ConstantSP> &arguments) override;

    static OOClassSP createDolphinClass(const string& qualifier, const string& name){
        return new DolphinClass(qualifier, name);
    }
    bool isBodyDefined() { return bodyDefined_; }
    void setBodyDefined(bool b) { bodyDefined_ = b; }

    ConstantSP &getAttributeType(INDEX index);
    ConstantSP &getAttributeType(const std::string &name);
    void setConstructor(const FunctionDefSP & constructor) { constructor_ = constructor; }
    FunctionDefSP getConstructor() { return constructor_; }
    DolphinClassSP getBaseClass() { return baseCls_; }
    void setBaseClass(DolphinClassSP &baseCls) { baseCls_ = baseCls; }

protected:
    DolphinClass(const string& qualifier, const string& name, int flag, const vector<pair<string, char> >& attributes, const vector<FunctionDefSP>& methods);

private:
    vector<pair<string, char> > attributes_;
    vector<FunctionDefSP> methods_;
    unordered_map<string, int> dict_;
    vector<pair<string, ConstantSP>> types_;
    bool bodyDefined_ = false;
    FunctionDefSP constructor_;
    DolphinClassSP baseCls_;
};

class DolphinInstance : public OOInstance{
public:
    DolphinInstance(const DolphinClassSP& ddbClass);
    DolphinInstance(const DolphinClassSP& ddbClass, const vector<ConstantSP>& data);
    DolphinInstance(Session* session, const DataInputStreamSP& in);
    virtual ~DolphinInstance(){}
    inline ConstantSP getAttribute(int index) const { return data_[index];}
    ConstantSP getAttribute(const string& name) const;
    void setAttribute(int index, const ConstantSP& attr);
    void setAttribute(const string& name, const ConstantSP& attr);
    virtual ConstantSP getValue() const;
    virtual ConstantSP getMember(const string& key) const;
    virtual IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const;
    virtual string getString() const;

    static ConstantSP createDolphinInstance(Session* session, const DataInputStreamSP& in){
        return new DolphinInstance(session, in);
    }

protected:
    vector<ConstantSP> data_;

private:
    ConstantSP typeCheck(const std::string &name, const ConstantSP &attr, const ConstantSP &attrType);
};

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

#endif //CODER_RESOURCE_H