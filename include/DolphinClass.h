#pragma once

#include "CoreConcept.h"

namespace ddb {

class DolphinClass;
class DolphinInstance;
typedef SmartPointer<DolphinClass> DolphinClassSP;
typedef SmartPointer<DolphinInstance> DolphinInstanceSP;

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
	INDEX getMemberIndex(const string& key) const {
		auto it = dict_.find(key);
		if (it == dict_.end()) return -1;
		return it->second;
	}
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
	void setJit(bool isJit) { isJit_ = isJit; }
	bool isJit() { return isJit_; }
	void setJitClassId(int id) { jitClassId_ = id; }
	int getJitClassId() { return jitClassId_; }
	static int genClsId() { return clsIdCnt_.fetch_add(1, std::memory_order_relaxed); }

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
	bool isJit_ = false;
	int jitClassId_ = 0;
	static std::atomic<int> clsIdCnt_;
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
	virtual bool set(const ConstantSP &index, const ConstantSP &val) override;
	virtual bool modifyMember(Heap* heap, const FunctionDefSP& func, const ConstantSP& index, const ConstantSP& parameters, int dim) override;

	static ConstantSP createDolphinInstance(Session* session, const DataInputStreamSP& in){
		return new DolphinInstance(session, in);
	}

protected:
	vector<ConstantSP> data_;

private:
	ConstantSP typeCheck(const std::string &name, const ConstantSP &attr, const ConstantSP &attrType);
};

}