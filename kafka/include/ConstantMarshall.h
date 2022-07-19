/*
 * ConstantMarshall.h
 *
 *  Created on: Oct 3, 2013
 *      Author: dzhou
 */

#ifndef CONSTANTMARSHALL_H_
#define CONSTANTMARSHALL_H_

#include "CoreConcept.h"

#define MARSHALL_BUFFER_SIZE 4096

class CodeMarshall;
class CodeUnmarshall;
class ConstantMarshallFactory;
class ConstantUnmarshallFactory;
class SymbolBaseMarshall;
class SymbolBaseUnmarshall;

typedef SmartPointer<CodeMarshall> CodeMarshallSP;
typedef SmartPointer<CodeUnmarshall> CodeUnmarshallSP;
typedef SmartPointer<ConstantMarshallFactory> ConstantMarshallFactorySP;
typedef SmartPointer<ConstantUnmarshallFactory> ConstantUnmarshallFactorySP;
typedef SmartPointer<SymbolBaseMarshall> SymbolBaseMarshallSP;
typedef SmartPointer<SymbolBaseUnmarshall> SymbolBaseUnmarshallSP;

class ConstantMarshallImp : public ConstantMarshall {
public:
	ConstantMarshallImp(const DataOutputStreamSP& out):out_(out), complete_(false){}
	virtual ~ConstantMarshallImp(){}
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
	char buf_[MARSHALL_BUFFER_SIZE];
};

class ConstantUnmarshallImp : public ConstantUnmarshall{
public:
	ConstantUnmarshallImp(const DataInputStreamSP& in, Session* session):in_(in),session_(session){}
	virtual ~ConstantUnmarshallImp(){}

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
	static IO_ERR serializeObjectAndDependency(Heap* pHeap, const Guid& id, const ObjectSP& obj, int minimumVer, bool skipSystemUDF, const ByteArrayCodeBufferSP& buffer);
	static IO_ERR serializeObjectAndDependency(Heap* pHeap, const Guid& id, const ObjectSP& obj, const unordered_map<string, FunctionDef*>& extraDependencies, int minimumVer, bool skipSystemUDF, bool checkDependency, const ByteArrayCodeBufferSP& buffer);
	static ObjectSP readObjectAndDependency(Session* session, const DataInputStreamSP& in, Guid& id);
};

class CodeMarshall: public ConstantMarshallImp{
public:
	CodeMarshall(Heap* heap, const DataOutputStreamSP& out, bool skipSystemUDF=false):ConstantMarshallImp(out),
		heap_(heap), id_(false), marshallDependency_(true), skipSystemUDF_(skipSystemUDF), mininumVerRequired_(0),extraDependency_(0), doneConstants_(0), doneSize_(0){}
	CodeMarshall(Heap* heap, const DataOutputStreamSP& out, bool marshallDependency, int mininumVerRequired, bool skipSystemUDF = false):ConstantMarshallImp(out),
		heap_(heap), id_(false), marshallDependency_(marshallDependency), skipSystemUDF_(skipSystemUDF), mininumVerRequired_(mininumVerRequired),
		extraDependency_(0), doneConstants_(0), doneSize_(0){}
	virtual ~CodeMarshall(){}
	void setId(const Guid& id) { id_ = id;}
	bool marshall(const char* requestHeader, size_t headerSize, const ObjectSP& target, bool blocking, IO_ERR& ret);
	virtual bool start(const char* requestHeader, size_t headerSize, const ConstantSP& target, bool blocking, IO_ERR& ret);
	virtual bool resume(IO_ERR& ret);
	virtual void reset();
	void setExtraDependency(unordered_map<string, FunctionDef*>* extraDependency){extraDependency_ = extraDependency;}
private:
	Heap* heap_;
	Guid id_;
	bool marshallDependency_;
	bool skipSystemUDF_;
	int mininumVerRequired_;
	unordered_map<string, FunctionDef*>* extraDependency_;
	int doneConstants_;
	int doneSize_;
	ByteArrayCodeBufferSP buffer_;
	ConstantMarshallSP marshall_;
};

class CodeUnmarshall: public ConstantUnmarshallImp{
public:
	CodeUnmarshall(const DataInputStreamSP& in, Session* session):ConstantUnmarshallImp(in, session),
		constantCount_(0), doneConstants_(0), codeLength_(0), doneSize_(0), id_(false), buf_(0), capacity_(0){}
	virtual ~CodeUnmarshall();
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
	ConstantUnmarshallSP unmarshall_;
};

/**
 * When we serialize a symbol vector, we serialize the symbol base first.
 */
class SymbolBaseMarshall {
public:
	SymbolBaseMarshall(const DataOutputStreamSP& out): out_(out), complete_(false), nextStart_(0), partial_(0){}
	~SymbolBaseMarshall(){}
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
	char buf_[MARSHALL_BUFFER_SIZE];
};

class SymbolBaseUnmarshall {
public:
	SymbolBaseUnmarshall(const DataInputStreamSP& in):symbaseId_(0), size_(0), in_(in){}
	~SymbolBaseUnmarshall(){}
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


class ScalarMarshall: public ConstantMarshallImp{
public:
	ScalarMarshall(const DataOutputStreamSP& out):ConstantMarshallImp(out), partial_(0){}
	virtual ~ScalarMarshall(){}
	virtual bool start(const char* requestHeader, size_t headerSize, const ConstantSP& target, bool blocking, IO_ERR& ret);
	virtual bool resume(IO_ERR& ret);
	virtual void reset();
private:
	int partial_;
};

class VectorMarshall: public ConstantMarshallImp{
public:
	VectorMarshall(const DataOutputStreamSP& out):ConstantMarshallImp(out),nextStart_(0),partial_(0), type_(DT_VOID), symbaseProgress_(-1){}
	virtual ~VectorMarshall(){}
	virtual bool start(const char* requestHeader, size_t headerSize, const ConstantSP& target, bool blocking, IO_ERR& ret);
	virtual bool start(const ConstantSP& target, bool blocking, IO_ERR& ret);
	virtual bool resume(IO_ERR& ret);
	virtual void reset();
	void resetSymbolBaseMarshall(bool createIfNotExist);

private:
	INDEX nextStart_;
	int partial_;
	DATA_TYPE type_;
	int symbaseProgress_;
	SymbolBaseMarshallSP symbaseMarshall_;
	ConstantMarshallSP marshall_;
};

class MatrixMarshall: public ConstantMarshallImp{
public:
	MatrixMarshall(const DataOutputStreamSP& out) : ConstantMarshallImp(out), rowLabelSent_(false),
		columnLabelSent_(false), inProgress_(false), vectorMarshall_(out){}
	virtual ~MatrixMarshall(){}
	virtual bool start(const char* requestHeader, size_t headerSize, const ConstantSP& target, bool blocking, IO_ERR& ret);
	virtual bool resume(IO_ERR& ret);
	virtual void reset();
private:
	bool sendMeta(const char* requestHeader, size_t headerSize, const ConstantSP& target, bool blocking, IO_ERR& ret);

private:
	bool rowLabelSent_;
	bool columnLabelSent_;
	bool inProgress_;
	VectorMarshall vectorMarshall_;
};

class TableMarshall: public ConstantMarshallImp{
public:
	TableMarshall(const DataOutputStreamSP& out) : ConstantMarshallImp(out) ,columnNamesSent_(0), wideColumnMapSent_(-1), nextColumn_(0),
		columnInProgress_(false), vectorMarshall_(out){}
	virtual ~TableMarshall(){}
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
	VectorMarshall vectorMarshall_;
};

class SetMarshall: public ConstantMarshallImp{
public:
	SetMarshall(const DataOutputStreamSP& out):ConstantMarshallImp(out), vectorMarshall_(out){}
	virtual ~SetMarshall(){}
	virtual bool start(const char* requestHeader, size_t headerSize, const ConstantSP& target, bool blocking, IO_ERR& ret);
	virtual bool resume(IO_ERR& ret);
	virtual void reset();

private:
	bool sendMeta(const char* requestHeader, size_t headerSize, const ConstantSP& target, bool blocking, IO_ERR& ret);

private:
	VectorMarshall vectorMarshall_;
};

class DictionaryMarshall: public ConstantMarshallImp{
public:
	DictionaryMarshall(const DataOutputStreamSP& out):ConstantMarshallImp(out), keySent_(false),
		inProgress_(false), vectorMarshall_(out){}
	virtual ~DictionaryMarshall(){}
	virtual bool start(const char* requestHeader, size_t headerSize, const ConstantSP& target, bool blocking, IO_ERR& ret);
	virtual bool resume(IO_ERR& ret);
	virtual void reset();
private:
	bool sendMeta(const char* requestHeader, size_t headerSize, const ConstantSP& target, bool blocking, IO_ERR& ret);

private:
	bool keySent_;
	bool inProgress_;
	VectorMarshall vectorMarshall_;
};

class ChunkMarshall: public ConstantMarshallImp{
public:
	ChunkMarshall(const DataOutputStreamSP& out):ConstantMarshallImp(out){}
	virtual ~ChunkMarshall(){}
	virtual bool start(const char* requestHeader, size_t headerSize, const ConstantSP& target, bool blocking, IO_ERR& ret);
	virtual bool resume(IO_ERR& ret);
	virtual void reset();
};

class ScalarUnmarshall: public ConstantUnmarshallImp{
public:
	ScalarUnmarshall(const DataInputStreamSP& in, Session* session):ConstantUnmarshallImp(in, session), isCodeObject_(false), functionType_(-1), partial_(0){}
	virtual ~ScalarUnmarshall(){}
	virtual bool start(short flag, bool blocking, IO_ERR& ret);
	virtual bool resume(IO_ERR& ret);
	virtual void reset();
private:
	FunctionDefSP parseFunctionDef(FUNCTIONDEF_TYPE type, const string& script) const;

private:
	bool isCodeObject_;
	char functionType_;
	int partial_;
};

class VectorUnmarshall: public ConstantUnmarshallImp{
public:
	VectorUnmarshall(const DataInputStreamSP& in, Session* session):ConstantUnmarshallImp(in, session), flag_(0), rows_(0), columns_(0), nextStart_(0),
		partial_(0), symbaseProgress_(-1), unmarshall_(0){}
	virtual ~VectorUnmarshall(){}
	virtual bool start(short flag, bool blocking, IO_ERR& ret);
	virtual bool resume(IO_ERR& ret);
	virtual void reset();
	void resetSymbolBaseUnmarshall(bool createIfNotExist);

private:
	short flag_;
	int rows_;
	int columns_;
	INDEX nextStart_;
	int partial_;
	int symbaseProgress_;
	SymbolBaseUnmarshallSP symbaseUnmarshall_;
	ConstantUnmarshallSP unmarshall_;
};

class MatrixUnmarshall: public ConstantUnmarshallImp{
public:
	MatrixUnmarshall(const DataInputStreamSP& in, Session* session):ConstantUnmarshallImp(in, session), labelFlag_(-1), rowLabelReceived_(false),
		columnLabelReceived_(false), inProgress_(false), vectorUnmarshall_(in, session){}
	virtual ~MatrixUnmarshall(){}
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
	VectorUnmarshall vectorUnmarshall_;
};

class TableUnmarshall: public ConstantUnmarshallImp{
public:
	TableUnmarshall(const DataInputStreamSP& in, Session* session):ConstantUnmarshallImp(in, session), type_(BASICTBL), tableNameReceived_(false), columnNameReceived_(false),
		nextColumn_(0), inProgress_(false), rows_(0), columns_(0), wideColumns_(-1), vectorUnmarshall_(in, session){}
	virtual ~TableUnmarshall(){}
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
	VectorUnmarshall vectorUnmarshall_;
};

class SetUnmarshall: public ConstantUnmarshallImp{
public:
	SetUnmarshall(const DataInputStreamSP& in, Session* session):ConstantUnmarshallImp(in, session), inProgress_(false), vectorUnmarshall_(in, session) {}
	virtual ~SetUnmarshall(){}
	virtual bool start(short flag, bool blocking, IO_ERR& ret);
	virtual bool resume(IO_ERR& ret);
	virtual void reset();
private:
	bool inProgress_;
	VectorUnmarshall vectorUnmarshall_;
};

class DictionaryUnmarshall: public ConstantUnmarshallImp{
public:
	DictionaryUnmarshall(const DataInputStreamSP& in, Session* session):ConstantUnmarshallImp(in, session), keyReceived_(false),
		inProgress_(false), vectorUnmarshall_(in, session) {}
	virtual ~DictionaryUnmarshall(){}
	virtual bool start(short flag, bool blocking, IO_ERR& ret);
	virtual bool resume(IO_ERR& ret);
	virtual void reset();
private:
	bool keyReceived_;
	ConstantSP keyVector_;
	bool inProgress_;
	VectorUnmarshall vectorUnmarshall_;
};

class ChunkUnmarshall: public ConstantUnmarshallImp{
public:
	ChunkUnmarshall(const DataInputStreamSP& in, Session* session):ConstantUnmarshallImp(in, session), size_(-1){}
	virtual ~ChunkUnmarshall(){}
	virtual bool start(short flag, bool blocking, IO_ERR& ret);
	virtual bool resume(IO_ERR& ret);
	virtual void reset();

private:
	IO_ERR parsing(const char* buf);
private:
	short size_;
};

class ConstantMarshallFactory{
public:
	ConstantMarshallFactory(const DataOutputStreamSP& out);
	~ConstantMarshallFactory();
	ConstantMarshall* getConstantMarshall(DATA_FORM form){return (form<0 || form>DF_CHUNK) ? NULL: arrMarshall[form];}
	static ConstantMarshallSP getInstance(DATA_FORM form, const DataOutputStreamSP& out);

private:
	ConstantMarshall* arrMarshall[9];
};

class ConstantUnmarshallFactory{
public:
	ConstantUnmarshallFactory(const DataInputStreamSP& in, Session* session);
	~ConstantUnmarshallFactory();
	ConstantUnmarshall* getConstantUnmarshall(DATA_FORM form){return (form<0 || form>DF_CHUNK) ? NULL: arrUnmarshall[form];}
	static ConstantUnmarshallSP getInstance(DATA_FORM form, const DataInputStreamSP& in, Session* session);

private:
	ConstantUnmarshall* arrUnmarshall[9];
};


#endif /* CONSTANTMARSHALL_H_ */
