/*
 * Exceptions.h
 *
 *  Created on: Jul 22, 2012
 *      Author: dzhou
 */

#ifndef EXCEPTIONS_H_
#define EXCEPTIONS_H_

#include <exception>
#include <string>
#include "Types.h"

using std::exception;
using std::string;

class TraceableException : public exception {
public:
	void addPath(const string& path);
	const string& getPath() const {return  path_;}
	const string& getLastThrow() const { return lastThrow_;}
	void setLastThrow(const string& lastThrow);

protected:
	string path_;
	string lastThrow_;
};

class IncompatibleTypeException: public TraceableException{
public:
	IncompatibleTypeException(DATA_TYPE expected, DATA_TYPE actual);
	virtual ~IncompatibleTypeException() throw(){}
	virtual const char* what() const throw() { return errMsg_.c_str();}
	DATA_TYPE expectedType(){return expected_;}
	DATA_TYPE actualType(){return actual_;}
private:
	DATA_TYPE expected_;
	DATA_TYPE actual_;
	string errMsg_;
};

class IllegalArgumentException : public TraceableException{
public:
	IllegalArgumentException(const string& functionName, const string& errMsg): functionName_(functionName), errMsg_(errMsg){}
	virtual const char* what() const throw(){
		return errMsg_.c_str();
	}
	virtual ~IllegalArgumentException() throw(){}
	const string& getFunctionName() const { return functionName_;}

private:
	const string functionName_;
	const string errMsg_;
};

class RuntimeException: public TraceableException{
public:
	RuntimeException(const string& errMsg):errMsg_(errMsg){}
	virtual const char* what() const throw(){
		return errMsg_.c_str();
	}
	virtual ~RuntimeException() throw(){}

private:
	const string errMsg_;
};

class OperatorRuntimeException: public TraceableException{
public:
	OperatorRuntimeException(const string& optr,const string& errMsg): operator_(optr),errMsg_(errMsg){}
	virtual const char* what() const throw(){
		return errMsg_.c_str();
	}
	virtual ~OperatorRuntimeException() throw(){}
	const string& getOperatorName() const { return operator_;}

private:
	const string operator_;
	const string errMsg_;
};

class TableRuntimeException: public TraceableException{
public:
	TableRuntimeException(const string& errMsg): errMsg_(errMsg){}
	virtual const char* what() const throw(){
		return errMsg_.c_str();
	}
	virtual ~TableRuntimeException() throw(){}

private:
	const string errMsg_;
};

class MemoryException: public TraceableException{
public:
	MemoryException():errMsg_("Out of memory"){}
	virtual const char* what() const throw(){
		return errMsg_.c_str();
	}
	virtual ~MemoryException() throw(){}

private:
	const string errMsg_;
};

class IOException: public TraceableException{
public:
	IOException(const string& errMsg): errMsg_(errMsg), errCode_(OTHERERR){}
	IOException(const string& errMsg, IO_ERR errCode): errMsg_(errMsg + ". " + getCodeDescription(errCode)), errCode_(errCode){}
	IOException(IO_ERR errCode): errMsg_(getCodeDescription(errCode)), errCode_(errCode){}
	virtual const char* what() const throw(){
		return errMsg_.c_str();
	}
	virtual ~IOException() throw(){}
	IO_ERR getErrorCode() const {return errCode_;}
private:
	string getCodeDescription(IO_ERR errCode) const;

private:
	const string errMsg_;
	const IO_ERR errCode_;
};

class FileChunkVersionCheckException : public exception{
public:
    FileChunkVersionCheckException(const string& errMsg) : errMsg_(errMsg){}
    virtual const char* what() const throw(){
        return errMsg_.c_str();
    }
    virtual ~FileChunkVersionCheckException() throw(){}

private:
    const string errMsg_;
};

class DataCorruptionException: public exception {
public:
	DataCorruptionException(const string& errMsg) : errMsg_("<DataCorruption>" + errMsg){}
	virtual const char* what() const throw(){
		return errMsg_.c_str();
	}
	virtual ~DataCorruptionException() throw(){}

private:
	const string errMsg_;
};

class NotLeaderException: public exception {
public:
	//Electing a leader. Wait for a while to retry.
	NotLeaderException() : errMsg_("<NotLeader>"){}
	//Use the new leader specified in the input argument. format: <host>:<port>:<alias>, e.g. 192.168.1.10:8801:nodeA
	NotLeaderException(const string& newLeader) : errMsg_("<NotLeader>" + newLeader), newLeader_(newLeader){}
	const string& getNewLeader() const {return newLeader_;}
	virtual const char* what() const throw(){
		return errMsg_.c_str();
	}
	virtual ~NotLeaderException() throw(){}

private:
	const string errMsg_;
	const string newLeader_;
};

class ChunkInTransactionException: public exception {
public:
	ChunkInTransactionException(const string& errMsg) : errMsg_("<ChunkInTransaction>" + errMsg){}
	virtual const char* what() const throw(){
		return errMsg_.c_str();
	}
	virtual ~ChunkInTransactionException() throw(){}

private:
	const string errMsg_;
};

class ChunkInRecoveryException: public exception {
public:
	ChunkInRecoveryException(const string& errMsg) : errMsg_("<ChunkInRecovery>" + errMsg){}
	virtual const char* what() const throw(){
		return errMsg_.c_str();
	}
	virtual ~ChunkInRecoveryException() throw(){}

private:
	const string errMsg_;
};

class DataNodeNotAvailException : public exception {
public:
	DataNodeNotAvailException(const string& errMsg) : errMsg_("<DataNodeNotAvail>" + errMsg){}
	virtual const char* what() const throw(){
		return errMsg_.c_str();
	}
	virtual ~DataNodeNotAvailException() throw(){}

private:
	const string errMsg_;
};

class ControllerNotAvailException : public exception {
public:
	ControllerNotAvailException(const string& errMsg) : errMsg_("<ControllerNotAvail>" + errMsg){}
	virtual const char* what() const throw(){
		return errMsg_.c_str();
	}
	virtual ~ControllerNotAvailException() throw(){}

private:
	const string errMsg_;
};

class ControllerNotReadyException : public exception {
public:
	ControllerNotReadyException() : errMsg_("<ControllerNotReady>"){}
	virtual const char* what() const throw(){
		return errMsg_.c_str();
	}
	virtual ~ControllerNotReadyException() throw(){}

private:
	const string errMsg_;
};

class MathException: public TraceableException {
public:
	MathException(const string& errMsg) : errMsg_(errMsg){}
	virtual const char* what() const throw(){
		return errMsg_.c_str();
	}
	virtual ~MathException() throw(){}

private:
	const string errMsg_;
};

class DataNodeNotReadyException: public TraceableException{
public:
    DataNodeNotReadyException(const string& errMsg) : errMsg_("<DataNodeNotReady>" + errMsg){}
    virtual const char* what() const throw(){
        return errMsg_.c_str();
    }
    virtual ~DataNodeNotReadyException() throw(){}

private:
    const string errMsg_;
};

class NoPrivilegeException: public TraceableException{
public:
	NoPrivilegeException(const string& errMsg, bool notAuthenticated) : errMsg_((notAuthenticated ? "<NotAuthenticated>" : "<NoPrivilege>") + errMsg){}
    virtual const char* what() const throw(){
        return errMsg_.c_str();
    }
    virtual ~NoPrivilegeException() throw(){}

private:
    const string errMsg_;
};

class UserException: public TraceableException{
public:
	UserException(const string exceptionType, const string& msg) : exceptionType_(exceptionType), msg_(msg){}
	virtual const char* what() const throw(){
		return msg_.c_str();
	}
	const string& getExceptionType() const { return exceptionType_;}
	const string& getMessage() const { return msg_;}
	virtual ~UserException() throw(){}
private:
	string exceptionType_;
	string msg_;
};

class SyntaxException: public exception{
public:
	SyntaxException(const string& errMsg): errMsg_(errMsg){}
	virtual const char* what() const throw(){
		return errMsg_.c_str();
	}
	virtual ~SyntaxException() throw(){}

private:
	const string errMsg_;
};

class TestingException: public exception{
public:
	TestingException(const string& caseName,const string& subCaseName): name_(caseName),subName_(subCaseName){
		if(subName_.empty())
			errMsg_="Testing case "+name_+" failed";
		else
			errMsg_="Testing case "+name_+"_"+subName_+" failed";
	}
	virtual const char* what() const throw(){
		return errMsg_.c_str();
	}
	const string& getCaseName() const {return name_;}
	const string& getSubCaseName() const {return subName_;}
	virtual ~TestingException() throw(){}

private:
	const string name_;
	const string subName_;
	string errMsg_;

};


/**
 * @brief When execute a "rollback" or "commit" statement,
 *		  should throw this exception.
 */
class TransactionFinishException : public exception {
public:
	TransactionFinishException(bool abort) : abort_(abort) {}

	virtual const char* what() const throw() {
		if (abort_) {
			return "Rollback statement must be wrapped in a transaction block";
		} else {
			return "Commit statement must be wrapped in a transaction block";
		}
	}

	bool abort() const { return abort_; }

private:
	const bool abort_;
};


#endif /* EXCEPTIONS_H_ */
