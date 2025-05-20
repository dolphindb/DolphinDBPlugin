/*
 * ConstantMarshall.h
 *
 *  Created on: Oct 3, 2013
 *      Author: dzhou
 */

#ifndef CONSTANTMARSHAL_H_
#define CONSTANTMARSHAL_H_

#include "BufferWriter.h"
#include "CoreConcept.h"
#include "Types.h"

#include <cstddef>
#include <cstdint>

#define MARSHALL_BUFFER_SIZE 4096

namespace ddb {
class CodeMarshal;
class CodeUnmarshal;
class ConstantMarshalFactory;
class ConstantUnmarshalFactory;

typedef SmartPointer<CodeMarshal> CodeMarshalSP;
typedef SmartPointer<CodeUnmarshal> CodeUnmarshalSP;
typedef SmartPointer<ConstantMarshalFactory> ConstantMarshalFactorySP;
typedef SmartPointer<ConstantUnmarshalFactory> ConstantUnmarshalFactorySP;

class ConstantMarshalImp : public ConstantMarshal {
public:
	ConstantMarshalImp(const DataOutputStreamSP& out):out_(out), complete_(false){}
	virtual ~ConstantMarshalImp(){}
	virtual bool start(const ConstantSP& target, bool blocking, IO_ERR& ret){
		return start(0, 0, target, blocking, ret);
	}
	virtual bool start(const char* requestHeader, size_t headerSize, const ConstantSP& target, bool blocking, IO_ERR& ret)=0;
	virtual IO_ERR flush();

protected:
	short encodeFlag(const ConstantSP& target);
protected:
	BufferWriter<DataOutputStreamSP> out_;
	ConstantSP target_;
	bool complete_;
	alignas(16) char buf_[MARSHALL_BUFFER_SIZE];
};

class ConstantUnmarshalImp : public ConstantUnmarshal{
public:
	ConstantUnmarshalImp(const DataInputStreamSP& in, Session* session):in_(in),session_(session){}
	virtual ~ConstantUnmarshalImp(){}

protected:
	void decodeFlag(short flag, DATA_FORM& form, DATA_TYPE& type);
protected:
	DataInputStreamSP in_;
	Session* session_;
};

class CodeMarshal: public ConstantMarshalImp{
public:
	CodeMarshal(Heap* heap, const DataOutputStreamSP& out, bool skipSystemUDF=false, bool uniqueFuncName=false):ConstantMarshalImp(out),
		heap_(heap), id_(false), marshalDependency_(true), skipSystemUDF_(skipSystemUDF), mininumVerRequired_(0),extraDependency_(0),
		doneConstants_(0), doneSize_(0), uniqueFuncName_(uniqueFuncName){}
	CodeMarshal(Heap* heap, const DataOutputStreamSP& out, bool marshallDependency, int mininumVerRequired, bool skipSystemUDF = false, bool uniqueFuncName=false):ConstantMarshalImp(out),
		heap_(heap), id_(false), marshalDependency_(marshallDependency), skipSystemUDF_(skipSystemUDF), mininumVerRequired_(mininumVerRequired),
		extraDependency_(0), doneConstants_(0), doneSize_(0), uniqueFuncName_(uniqueFuncName){}
	virtual ~CodeMarshal(){}
	void setId(const Guid& id) { id_ = id;}
	bool marshal(const char* requestHeader, size_t headerSize, const ObjectSP& target, bool blocking, IO_ERR& ret);
	virtual bool start(const char* requestHeader, size_t headerSize, const ConstantSP& target, bool blocking, IO_ERR& ret);
	virtual bool resume(IO_ERR& ret);
	virtual void reset();
	void setExtraDependency(unordered_map<string, FunctionDef*>* extraDependency){extraDependency_ = extraDependency;}
private:
	Heap* heap_;
	Guid id_;
	bool marshalDependency_;
	bool skipSystemUDF_;
	int mininumVerRequired_;
	unordered_map<string, FunctionDef*>* extraDependency_;
	int doneConstants_;
	int doneSize_;
	ByteArrayCodeBufferSP buffer_;
	ConstantMarshalSP marshal_;
	bool uniqueFuncName_;
};

class CodeUnmarshal: public ConstantUnmarshalImp{
public:
	CodeUnmarshal(const DataInputStreamSP& in, Session* session):ConstantUnmarshalImp(in, session),
		constantCount_(0), doneConstants_(0), codeLength_(0), doneSize_(0), id_(false), buf_(0), capacity_(0){}
	virtual ~CodeUnmarshal();
	virtual bool start(short flag, bool blocking, IO_ERR& ret);
	virtual bool resume(IO_ERR& ret);
	virtual void reset();
	const string& getErrorMessage() const { return errMsg_;}
	const Guid& getId() const { return id_;}

private:
	JobProperty job_;
	int constantCount_;
	int doneConstants_;
	int codeLength_;
	int doneSize_;
	Guid id_;
	string errMsg_;
	char* buf_;
	int capacity_;
	ConstantUnmarshalSP unmarshal_;
};

class SWORDFISH_API ConstantMarshalFactory{
public:
	ConstantMarshalFactory(const DataOutputStreamSP& out);
	~ConstantMarshalFactory();
	ConstantMarshal* getConstantMarshal(DATA_FORM form){return (form<0 || form>=MAX_DATA_FORMS) ? NULL: arrMarshal[form];}
	static ConstantMarshalSP getInstance(DATA_FORM form, const DataOutputStreamSP& out);

private:
	ConstantMarshal* arrMarshal[MAX_DATA_FORMS];
};

class SWORDFISH_API ConstantUnmarshalFactory{
public:
	ConstantUnmarshalFactory(const DataInputStreamSP& in, Session* session);
	~ConstantUnmarshalFactory();
	ConstantUnmarshal* getConstantUnmarshal(DATA_FORM form){return (form<0 || form>=MAX_DATA_FORMS) ? NULL: arrUnmarshal[form];}
	static ConstantUnmarshalSP getInstance(DATA_FORM form, const DataInputStreamSP& in, Session* session);

private:
	ConstantUnmarshal* arrUnmarshal[MAX_DATA_FORMS];
};
}

#endif /* CONSTANTMARSHAL_H_ */
