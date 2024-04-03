/*
 * ConstantMarshall.h
 *
 *  Created on: Oct 3, 2013
 *      Author: dzhou
 */

#ifndef CONSTANTMARSHAL_H_
#define CONSTANTMARSHAL_H_

#include "CoreConcept.h"

#define MARSHALL_BUFFER_SIZE 4096

class CodeMarshal;
class CodeUnmarshal;
class ConstantMarshalFactory;
class ConstantUnmarshalFactory;
class SymbolBaseMarshal;
class SymbolBaseUnmarshal;

typedef SmartPointer<CodeMarshal> CodeMarshalSP;
typedef SmartPointer<CodeUnmarshal> CodeUnmarshalSP;
typedef SmartPointer<ConstantMarshalFactory> ConstantMarshalFactorySP;
typedef SmartPointer<ConstantUnmarshalFactory> ConstantUnmarshalFactorySP;
typedef SmartPointer<SymbolBaseMarshal> SymbolBaseMarshalSP;
typedef SmartPointer<SymbolBaseUnmarshal> SymbolBaseUnmarshalSP;

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

class CodeHelper{
public:
	static void collectUserDefinedFunctions(const ObjectSP& obj, bool skipSystemUDF, unordered_map<string, FunctionDef*>& dependency);
	static void collectUserDefinedFunctions(const ObjectSP& obj, const unordered_map<string, FunctionDef*>& extraDependencies, bool skipSystemUDF, unordered_map<string, FunctionDef*>& dependency);
	static void collectUserDefinedFunctionsAndClasses(Heap* pHeap, const ObjectSP& obj, bool skipSystemUDF, unordered_map<string, FunctionDef*>& udfDependency,
			unordered_map<string, OOClass*>& classDependency);
	static void collectUserDefinedFunctionsAndClasses(Heap* pHeap, const ObjectSP& obj, const unordered_map<string, FunctionDef*>& extraDependencies, bool skipSystemUDF,
			unordered_map<string, FunctionDef*>& udfDependency, unordered_map<string, OOClass*>& classDependency);
	static IO_ERR serializeObjectAndDependency(Heap* pHeap, const Guid& id, const ObjectSP& obj, int minimumVer, bool skipSystemUDF, const ByteArrayCodeBufferSP& buffer);
	static IO_ERR serializeObjectAndDependency(Heap* pHeap, const Guid& id, const ObjectSP& obj, const unordered_map<string, FunctionDef*>& extraDependencies, int minimumVer, bool skipSystemUDF, bool checkDependency, const ByteArrayCodeBufferSP& buffer);
	static ObjectSP readObjectAndDependency(Session* session, const DataInputStreamSP& in, Guid& id);
	static string getUniqueFuncName(const FunctionDef* func);
	static string getUniqueClassName(const OOClass* cls);
	static string getNameFromUnqiueName(const string& uniqueName);
	static bool isUniqueFuncName(const string& name);
	// after D20-11228, getString() of funcDef contains unique funcName, we need regenerate script to fix it.
	static void rmUniqueNameFromScript(FunctionDef* func);
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

/**
 * When we serialize a symbol vector, we serialize the symbol base first.
 */
class SymbolBaseMarshal {
public:
	SymbolBaseMarshal(const DataOutputStreamSP& out): out_(out), complete_(false), nextStart_(0), partial_(0), syms_(0){}
	~SymbolBaseMarshal(){}
	bool start(const SymbolBaseSP target, bool blocking, IO_ERR& ret);
	bool resume(IO_ERR& ret);
	void reset();

private:
	BufferWriter<DataOutputStreamSP> out_;
	SymbolBaseSP target_;
	unordered_map<long long, int> dict_;
	bool complete_;
	int nextStart_;
	int partial_;
	int syms_;
	alignas(16) char buf_[MARSHALL_BUFFER_SIZE];
};

class SymbolBaseUnmarshal {
public:
	SymbolBaseUnmarshal(const DataInputStreamSP& in):symbaseId_(0), size_(0), in_(in){}
	~SymbolBaseUnmarshal(){}
	bool start(bool blocking, IO_ERR& ret);
	bool resume(IO_ERR& ret);
	void reset();
	SymbolBaseSP getSymbolBase() const {
		return obj_;
	}

private:
	int symbaseId_;
	int size_;
	DataInputStreamSP in_;
	SymbolBaseSP obj_;
	vector<DolphinString> symbols_;
	unordered_map<int, SymbolBaseSP> dict_;
};


class ScalarMarshal: public ConstantMarshalImp{
public:
	ScalarMarshal(const DataOutputStreamSP& out):ConstantMarshalImp(out), partial_(0){}
	virtual ~ScalarMarshal(){}
	virtual bool start(const char* requestHeader, size_t headerSize, const ConstantSP& target, bool blocking, IO_ERR& ret);
	virtual bool resume(IO_ERR& ret);
	virtual void reset();
private:
	int partial_;
};

class VectorMarshal: public ConstantMarshalImp{
public:
	VectorMarshal(const DataOutputStreamSP& out):ConstantMarshalImp(out),nextStart_(0),partial_(0), type_(DT_VOID), symbaseProgress_(-1){}
	virtual ~VectorMarshal(){}
	virtual bool start(const char* requestHeader, size_t headerSize, const ConstantSP& target, bool blocking, IO_ERR& ret);
	virtual bool start(const ConstantSP& target, bool blocking, IO_ERR& ret);
	virtual bool resume(IO_ERR& ret);
	virtual void reset();
	void resetSymbolBaseMarshal(bool createIfNotExist);

private:
	INDEX nextStart_;
	int partial_;
	DATA_TYPE type_;
	int symbaseProgress_;
	SymbolBaseMarshalSP symbaseMarshal_;
	ConstantMarshalSP marshal_;
};

class MatrixMarshal: public ConstantMarshalImp{
public:
	MatrixMarshal(const DataOutputStreamSP& out) : ConstantMarshalImp(out), rowLabelSent_(false),
		columnLabelSent_(false), inProgress_(false), vectorMarshal_(out){}
	virtual ~MatrixMarshal(){}
	virtual bool start(const char* requestHeader, size_t headerSize, const ConstantSP& target, bool blocking, IO_ERR& ret);
	virtual bool resume(IO_ERR& ret);
	virtual void reset();
private:
	bool sendMeta(const char* requestHeader, size_t headerSize, const ConstantSP& target, bool blocking, IO_ERR& ret);

private:
	bool rowLabelSent_;
	bool columnLabelSent_;
	bool inProgress_;
	VectorMarshal vectorMarshal_;
};

class TableMarshal: public ConstantMarshalImp{
public:
	TableMarshal(const DataOutputStreamSP& out) : ConstantMarshalImp(out) ,columnNamesSent_(0), wideColumnMapSent_(-1), nextColumn_(0),
		columnInProgress_(false), vectorMarshal_(out){}
	virtual ~TableMarshal(){}
	virtual bool start(const char* requestHeader, size_t headerSize, const ConstantSP& target, bool blocking, IO_ERR& ret);
	virtual bool resume(IO_ERR& ret);
	virtual void reset();
private:
	bool sendMeta(const char* requestHeader, size_t headerSize, const ConstantSP& target, bool blocking, IO_ERR& ret);

private:
	int columnNamesSent_;
	int wideColumnMapSent_;
	int nextColumn_;
	bool columnInProgress_;
	VectorMarshal vectorMarshal_;
};

class SetMarshal: public ConstantMarshalImp{
public:
	SetMarshal(const DataOutputStreamSP& out):ConstantMarshalImp(out), vectorMarshal_(out){}
	virtual ~SetMarshal(){}
	virtual bool start(const char* requestHeader, size_t headerSize, const ConstantSP& target, bool blocking, IO_ERR& ret);
	virtual bool resume(IO_ERR& ret);
	virtual void reset();

private:
	bool sendMeta(const char* requestHeader, size_t headerSize, const ConstantSP& target, bool blocking, IO_ERR& ret);

private:
	VectorMarshal vectorMarshal_;
};

class DictionaryMarshal: public ConstantMarshalImp{
public:
	DictionaryMarshal(const DataOutputStreamSP& out):ConstantMarshalImp(out), keySent_(false),
		inProgress_(false), vectorMarshal_(out){}
	virtual ~DictionaryMarshal(){}
	virtual bool start(const char* requestHeader, size_t headerSize, const ConstantSP& target, bool blocking, IO_ERR& ret);
	virtual bool resume(IO_ERR& ret);
	virtual void reset();
private:
	bool sendMeta(const char* requestHeader, size_t headerSize, const ConstantSP& target, bool blocking, IO_ERR& ret);

private:
	bool keySent_;
	bool inProgress_;
	VectorMarshal vectorMarshal_;
};

class ChunkMarshal: public ConstantMarshalImp{
public:
	ChunkMarshal(const DataOutputStreamSP& out):ConstantMarshalImp(out){}
	virtual ~ChunkMarshal(){}
	virtual bool start(const char* requestHeader, size_t headerSize, const ConstantSP& target, bool blocking, IO_ERR& ret);
	virtual bool resume(IO_ERR& ret);
	virtual void reset();
};

class SysObjMarshal: public ConstantMarshalImp{
public:
	SysObjMarshal(const DataOutputStreamSP& out) : ConstantMarshalImp(out), doneConstants_(-1), doneSize_(-1){}
	virtual ~SysObjMarshal(){}
	virtual bool start(const char* requestHeader, size_t headerSize, const ConstantSP& target, bool blocking, IO_ERR& ret);
	virtual bool resume(IO_ERR& ret);
	virtual void reset();

private:
	int doneConstants_;
	int doneSize_;
	ByteArrayCodeBufferSP buffer_;
	ConstantMarshalSP marshal_;
};

class ScalarUnmarshal: public ConstantUnmarshalImp{
public:
	ScalarUnmarshal(const DataInputStreamSP& in, Session* session):ConstantUnmarshalImp(in, session), isCodeObject_(false), functionType_(-1), partial_(0){}
	virtual ~ScalarUnmarshal(){}
	virtual bool start(short flag, bool blocking, IO_ERR& ret);
	virtual bool resume(IO_ERR& ret);
	virtual void reset();
private:
	FunctionDefSP parseFunctionDef(FUNCTIONDEF_TYPE type, const string& script) const;

private:
	bool isCodeObject_;
	char functionType_;
	int partial_;

	/// Used for Decimal data type.
	DATA_TYPE dataType_ = DT_VOID;
	int scale_ = -1;
};

class VectorUnmarshal: public ConstantUnmarshalImp{
public:
	VectorUnmarshal(const DataInputStreamSP& in, Session* session):ConstantUnmarshalImp(in, session), flag_(0), rows_(0), columns_(0), nextStart_(0),
		partial_(0), symbaseProgress_(-1), unmarshal_(0){}
	virtual ~VectorUnmarshal(){}
	virtual bool start(short flag, bool blocking, IO_ERR& ret);
	virtual bool resume(IO_ERR& ret);
	virtual void reset();
	void resetSymbolBaseUnmarshal(bool createIfNotExist);

private:
	short flag_;
	int rows_;
	int columns_;
	/** Used for Decimal type. */
	int scale_ = -1;

	INDEX nextStart_;
	int partial_;
	int symbaseProgress_;
	SymbolBaseUnmarshalSP symbaseUnmarshal_;
	ConstantUnmarshalSP unmarshal_;
};

class MatrixUnmarshal: public ConstantUnmarshalImp{
public:
	MatrixUnmarshal(const DataInputStreamSP& in, Session* session):ConstantUnmarshalImp(in, session), labelFlag_(-1), rowLabelReceived_(false),
		columnLabelReceived_(false), inProgress_(false), vectorUnmarshal_(in, session){}
	virtual ~MatrixUnmarshal(){}
	virtual bool start(short flag, bool blocking, IO_ERR& ret);
	virtual bool resume(IO_ERR& ret);
	virtual void reset();
private:
	char labelFlag_;
	bool rowLabelReceived_;
	bool columnLabelReceived_;
	bool inProgress_;
	ConstantSP rowLabel_;
	ConstantSP columnLabel_;
	VectorUnmarshal vectorUnmarshal_;
};

class TableUnmarshal: public ConstantUnmarshalImp{
public:
	TableUnmarshal(const DataInputStreamSP& in, Session* session):ConstantUnmarshalImp(in, session), type_(BASICTBL), tableNameReceived_(false), columnNameReceived_(false),
		nextColumn_(0), inProgress_(false), rows_(0), columns_(0), wideColumns_(-1), vectorUnmarshal_(in, session){}
	virtual ~TableUnmarshal(){}
	virtual bool start(short flag, bool blocking, IO_ERR& ret);
	virtual bool resume(IO_ERR& ret);
	virtual void reset();

private:
	void createTable();

private:
	TABLE_TYPE type_;
	bool tableNameReceived_;
	bool columnNameReceived_;
	int nextColumn_;
	bool inProgress_;
	int rows_;
	int columns_;
	int wideColumns_;
	string tableName_;
	ConstantSP wideColumnMap_;
	vector<string> colNames_;
	vector<ConstantSP> colObjs_;
	VectorUnmarshal vectorUnmarshal_;
};

class SetUnmarshal: public ConstantUnmarshalImp{
public:
	SetUnmarshal(const DataInputStreamSP& in, Session* session):ConstantUnmarshalImp(in, session), inProgress_(false), vectorUnmarshal_(in, session) {}
	virtual ~SetUnmarshal(){}
	virtual bool start(short flag, bool blocking, IO_ERR& ret);
	virtual bool resume(IO_ERR& ret);
	virtual void reset();
private:
	bool inProgress_;
	VectorUnmarshal vectorUnmarshal_;
};

class DictionaryUnmarshal: public ConstantUnmarshalImp{
public:
	DictionaryUnmarshal(const DataInputStreamSP& in, Session* session):ConstantUnmarshalImp(in, session), keyReceived_(false),
		inProgress_(false), vectorUnmarshal_(in, session) {}
	virtual ~DictionaryUnmarshal(){}
	virtual bool start(short flag, bool blocking, IO_ERR& ret);
	virtual bool resume(IO_ERR& ret);
	virtual void reset();
private:
	bool keyReceived_;
	ConstantSP keyVector_;
	bool inProgress_;
	VectorUnmarshal vectorUnmarshal_;
};

class ChunkUnmarshal: public ConstantUnmarshalImp{
public:
	ChunkUnmarshal(const DataInputStreamSP& in, Session* session):ConstantUnmarshalImp(in, session), size_(-1){}
	virtual ~ChunkUnmarshal(){}
	virtual bool start(short flag, bool blocking, IO_ERR& ret);
	virtual bool resume(IO_ERR& ret);
	virtual void reset();

private:
	IO_ERR parsing(const char* buf);
private:
	short size_;
};

class SysObjUnmarshal: public ConstantUnmarshalImp {
public:
	SysObjUnmarshal(const DataInputStreamSP& in, Session* session) : ConstantUnmarshalImp(in, session),
		constantCount_(0), doneConstants_(0), codeLength_(0), doneSize_(0), buf_(0), capacity_(0){}
	virtual ~SysObjUnmarshal(){}
	virtual bool start(short flag, bool blocking, IO_ERR& ret);
	virtual bool resume(IO_ERR& ret);
	virtual void reset();

private:
	int constantCount_;
	int doneConstants_;
	int codeLength_;
	int doneSize_;
	char* buf_;
	int capacity_;
	ConstantUnmarshalSP unmarshal_;
};

class ConstantMarshalFactory{
public:
	ConstantMarshalFactory(const DataOutputStreamSP& out);
	~ConstantMarshalFactory();
	ConstantMarshal* getConstantMarshal(DATA_FORM form){return (form<0 || form>=MAX_DATA_FORMS) ? NULL: arrMarshal[form];}
	static ConstantMarshalSP getInstance(DATA_FORM form, const DataOutputStreamSP& out);

private:
	ConstantMarshal* arrMarshal[MAX_DATA_FORMS];
};

class ConstantUnmarshalFactory{
public:
	ConstantUnmarshalFactory(const DataInputStreamSP& in, Session* session);
	~ConstantUnmarshalFactory();
	ConstantUnmarshal* getConstantUnmarshal(DATA_FORM form){return (form<0 || form>=MAX_DATA_FORMS) ? NULL: arrUnmarshal[form];}
	static ConstantUnmarshalSP getInstance(DATA_FORM form, const DataInputStreamSP& in, Session* session);

private:
	ConstantUnmarshal* arrUnmarshal[MAX_DATA_FORMS];
};


#endif /* CONSTANTMARSHAL_H_ */
