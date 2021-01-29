/*
 * ConstantMarshall.h
 *
 *  Created on: Oct 3, 2013
 *      Author: dzhou
 */

#ifndef CONSTANTMARSHALL_H_
#define CONSTANTMARSHALL_H_

#include "DolphinDB.h"
#include "SysIO.h"
#ifdef _MSC_VER
#define EXPORT_DECL _declspec(dllexport)
#else
#define EXPORT_DECL 
#endif

#define MARSHALL_BUFFER_SIZE 4096

namespace dolphindb {

class CodeMarshall;
class CodeUnmarshall;
class ConstantMarshallFactory;
class ConstantUnmarshallFactory;

typedef SmartPointer<CodeMarshall> CodeMarshallSP;
typedef SmartPointer<CodeUnmarshall> CodeUnmarshallSP;
typedef SmartPointer<ConstantMarshallFactory> ConstantMarshallFactorySP;
typedef SmartPointer<ConstantUnmarshallFactory> ConstantUnmarshallFactorySP;

class EXPORT_DECL ConstantMarshallImp : public ConstantMarshall {
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

class EXPORT_DECL ConstantUnmarshallImp : public ConstantUnmarshall{
public:
	ConstantUnmarshallImp(const DataInputStreamSP& in):in_(in){}
	virtual ~ConstantUnmarshallImp(){}

protected:
	void decodeFlag(short flag, DATA_FORM& form, DATA_TYPE& type);
protected:
	DataInputStreamSP in_;
};


class EXPORT_DECL ScalarMarshall: public ConstantMarshallImp{
public:
	ScalarMarshall(const DataOutputStreamSP& out):ConstantMarshallImp(out), partial_(0){}
	virtual ~ScalarMarshall(){}
	virtual bool start(const char* requestHeader, size_t headerSize, const ConstantSP& target, bool blocking, IO_ERR& ret);
	virtual void reset();
private:
	int partial_;
};

class EXPORT_DECL VectorMarshall: public ConstantMarshallImp{
public:
	VectorMarshall(const DataOutputStreamSP& out):ConstantMarshallImp(out),nextStart_(0),partial_(0){}
	virtual ~VectorMarshall(){}
	virtual bool start(const char* requestHeader, size_t headerSize, const ConstantSP& target, bool blocking, IO_ERR& ret);
	virtual bool start(const ConstantSP& target, bool blocking, IO_ERR& ret);
	virtual void reset();
private:
	INDEX nextStart_;
	int partial_;
	ConstantMarshallSP marshall_;
};

class EXPORT_DECL MatrixMarshall: public ConstantMarshallImp{
public:
	MatrixMarshall(const DataOutputStreamSP& out) : ConstantMarshallImp(out), rowLabelSent_(false),
		columnLabelSent_(false), inProgress_(false), vectorMarshall_(out){}
	virtual ~MatrixMarshall(){}
	virtual bool start(const char* requestHeader, size_t headerSize, const ConstantSP& target, bool blocking, IO_ERR& ret);
	virtual void reset();
private:
	bool sendMeta(const char* requestHeader, size_t headerSize, const ConstantSP& target, bool blocking, IO_ERR& ret);

private:
	bool rowLabelSent_;
	bool columnLabelSent_;
	bool inProgress_;
	VectorMarshall vectorMarshall_;
};

class EXPORT_DECL TableMarshall: public ConstantMarshallImp{
public:
	TableMarshall(const DataOutputStreamSP& out) : ConstantMarshallImp(out) ,columnNamesSent_(0), nextColumn_(0),
		columnInProgress_(false), vectorMarshall_(out){}
	virtual ~TableMarshall(){}
	virtual bool start(const char* requestHeader, size_t headerSize, const ConstantSP& target, bool blocking, IO_ERR& ret);
	virtual void reset();
private:
	bool sendMeta(const char* requestHeader, size_t headerSize, const ConstantSP& target, bool blocking, IO_ERR& ret);

private:
	int columnNamesSent_;
	int nextColumn_;
	bool columnInProgress_;
	VectorMarshall vectorMarshall_;
};

class EXPORT_DECL SetMarshall: public ConstantMarshallImp{
public:
	SetMarshall(const DataOutputStreamSP& out):ConstantMarshallImp(out), vectorMarshall_(out){}
	virtual ~SetMarshall(){}
	virtual bool start(const char* requestHeader, size_t headerSize, const ConstantSP& target, bool blocking, IO_ERR& ret);
	virtual void reset();

private:
	bool sendMeta(const char* requestHeader, size_t headerSize, const ConstantSP& target, bool blocking, IO_ERR& ret);

private:
	VectorMarshall vectorMarshall_;
};

class EXPORT_DECL DictionaryMarshall: public ConstantMarshallImp{
public:
	DictionaryMarshall(const DataOutputStreamSP& out):ConstantMarshallImp(out), keySent_(false),
		inProgress_(false), vectorMarshall_(out){}
	virtual ~DictionaryMarshall(){}
	virtual bool start(const char* requestHeader, size_t headerSize, const ConstantSP& target, bool blocking, IO_ERR& ret);
	virtual void reset();
private:
	bool sendMeta(const char* requestHeader, size_t headerSize, const ConstantSP& target, bool blocking, IO_ERR& ret);

private:
	bool keySent_;
	bool inProgress_;
	VectorMarshall vectorMarshall_;
};

class EXPORT_DECL ChunkMarshall: public ConstantMarshallImp{
public:
	ChunkMarshall(const DataOutputStreamSP& out):ConstantMarshallImp(out){}
	virtual ~ChunkMarshall(){}
	virtual bool start(const char* requestHeader, size_t headerSize, const ConstantSP& target, bool blocking, IO_ERR& ret);
	virtual void reset();
};

class EXPORT_DECL ScalarUnmarshall: public ConstantUnmarshallImp{
public:
	ScalarUnmarshall(const DataInputStreamSP& in):ConstantUnmarshallImp(in), isCodeObject_(false), functionType_(-1){}
	virtual ~ScalarUnmarshall(){}
	virtual bool start(short flag, bool blocking, IO_ERR& ret);
	virtual void reset();

private:
	bool isCodeObject_;
	char functionType_;
};

class EXPORT_DECL VectorUnmarshall: public ConstantUnmarshallImp{
public:
	VectorUnmarshall(const DataInputStreamSP& in):ConstantUnmarshallImp(in), flag_(0), rows_(0), columns_(0), nextStart_(0), unmarshall_(0){}
	virtual ~VectorUnmarshall(){}
	virtual bool start(short flag, bool blocking, IO_ERR& ret);
	virtual void reset();
private:
	short flag_;
	int rows_;
	int columns_;
	INDEX nextStart_;
	ConstantUnmarshallSP unmarshall_;
};

class EXPORT_DECL MatrixUnmarshall: public ConstantUnmarshallImp{
public:
	MatrixUnmarshall(const DataInputStreamSP& in):ConstantUnmarshallImp(in), labelFlag_(-1), rowLabelReceived_(false),
		columnLabelReceived_(false), inProgress_(false), vectorUnmarshall_(in){}
	virtual ~MatrixUnmarshall(){}
	virtual bool start(short flag, bool blocking, IO_ERR& ret);
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

class EXPORT_DECL TableUnmarshall: public ConstantUnmarshallImp{
public:
	TableUnmarshall(const DataInputStreamSP& in):ConstantUnmarshallImp(in), type_(BASICTBL), tableNameReceived_(false), columnNameReceived_(false),
		nextColumn_(0), inProgress_(false), rows_(0), columns_(0), vectorUnmarshall_(in){}
	virtual ~TableUnmarshall(){}
	virtual bool start(short flag, bool blocking, IO_ERR& ret);
	virtual void reset();
private:
	TABLE_TYPE type_;
	bool tableNameReceived_;
	bool columnNameReceived_;
	int nextColumn_;
	bool inProgress_;
	int rows_;
	int columns_;
	string tableName_;
	vector<string> colNames_;
	vector<ConstantSP> colObjs_;
	VectorUnmarshall vectorUnmarshall_;
};

class EXPORT_DECL SetUnmarshall: public ConstantUnmarshallImp{
public:
	SetUnmarshall(const DataInputStreamSP& in):ConstantUnmarshallImp(in), inProgress_(false), vectorUnmarshall_(in) {}
	virtual ~SetUnmarshall(){}
	virtual bool start(short flag, bool blocking, IO_ERR& ret);
	virtual void reset();
private:
	bool inProgress_;
	VectorUnmarshall vectorUnmarshall_;
};

class EXPORT_DECL DictionaryUnmarshall: public ConstantUnmarshallImp{
public:
	DictionaryUnmarshall(const DataInputStreamSP& in):ConstantUnmarshallImp(in), keyReceived_(false),
		inProgress_(false), vectorUnmarshall_(in) {}
	virtual ~DictionaryUnmarshall(){}
	virtual bool start(short flag, bool blocking, IO_ERR& ret);
	virtual void reset();
private:
	bool keyReceived_;
	ConstantSP keyVector_;
	bool inProgress_;
	VectorUnmarshall vectorUnmarshall_;
};

class EXPORT_DECL ChunkUnmarshall: public ConstantUnmarshallImp{
public:
	ChunkUnmarshall(const DataInputStreamSP& in):ConstantUnmarshallImp(in), size_(-1){}
	virtual ~ChunkUnmarshall(){}
	virtual bool start(short flag, bool blocking, IO_ERR& ret);
	virtual void reset();

private:
	IO_ERR parsing(const char* buf);
private:
	short size_;
};

class EXPORT_DECL ConstantMarshallFactory{
public:
	ConstantMarshallFactory(const DataOutputStreamSP& out);
	~ConstantMarshallFactory();
	ConstantMarshall* getConstantMarshall(DATA_FORM form){return (form<0 || form>DF_CHUNK) ? NULL: arrMarshall[form];}
	static ConstantMarshallSP getInstance(DATA_FORM form, const DataOutputStreamSP& out);

private:
	ConstantMarshall* arrMarshall[9];
};

class EXPORT_DECL ConstantUnmarshallFactory{
public:
	ConstantUnmarshallFactory(const DataInputStreamSP& in);
	~ConstantUnmarshallFactory();
	ConstantUnmarshall* getConstantUnmarshall(DATA_FORM form){return (form<0 || form>DF_CHUNK) ? NULL: arrUnmarshall[form];}
	static ConstantUnmarshallSP getInstance(DATA_FORM form, const DataInputStreamSP& in);

private:
	ConstantUnmarshall* arrUnmarshall[9];
};

};
#endif /* CONSTANTMARSHALL_H_ */
