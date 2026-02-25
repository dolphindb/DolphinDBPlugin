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


namespace ddb {

/**
 * @brief Enumeration of all exception types for tracking and logging purposes.
 *        Using int8_t as underlying type for memory efficiency.
 */
enum class ExceptionType : int8_t {
    // std::exception
    DEFAULT_EXCEPTION = 0,

    // Base traceable exception
    TRACEABLE_EXCEPTION = 16,

    // Exceptions inheriting from TraceableException
    INCOMPATIBLE_TYPE_EXCEPTION = 17,
    ILLEGAL_ARGUMENT_EXCEPTION = 18,
    RUNTIME_EXCEPTION = 19,
    OPERATOR_RUNTIME_EXCEPTION = 20,
    TABLE_RUNTIME_EXCEPTION = 21,
    MEMORY_EXCEPTION = 22,
    IO_EXCEPTION = 23,
    MATH_EXCEPTION = 24,
    DATA_NODE_NOT_READY_EXCEPTION = 25,
    NO_PRIVILEGE_EXCEPTION = 26,
    USER_EXCEPTION = 27,
    CACHE_INVALID_EXCEPTION = 28,

    // Exceptions inheriting directly from std::exception
    FILE_CHUNK_VERSION_CHECK_EXCEPTION = 64,
    DATA_CORRUPTION_EXCEPTION = 65,
    NOT_LEADER_EXCEPTION = 66,
    CHUNK_IN_TRANSACTION_EXCEPTION = 67,
    CHUNK_RESOLUTION_EXCEPTION = 68,
    CHUNK_IN_RECOVERY_EXCEPTION = 69,
    DATA_NODE_NOT_AVAIL_EXCEPTION = 70,
    CONTROLLER_NOT_AVAIL_EXCEPTION = 71,
    CONTROLLER_NOT_READY_EXCEPTION = 72,
    SYNTAX_EXCEPTION = 73,
    TESTING_EXCEPTION = 74,
    TRANSACTION_FINISH_EXCEPTION = 75,
    OLTP_NEED_RETRY_EXCEPTION = 76,
    OLTP_NOT_RETRY_EXCEPTION = 77
};

/**
 * @brief Error severity levels for exceptions
 */
 enum class ErrorLevel : int8_t {
    Warning = 0,    // Warning, operation can continue or retry
    Error = 1,      // Error, operation failed
    Fatal = 2       // Fatal error, should crash the system
};

/**
 * @brief Error scope categories for exceptions
 *        Used to categorize exceptions by functional domain
 * @URL: https://docs.dolphindb.cn/zh/error_codes/err_codes.html
 */
enum class ErrorScope : int8_t {
    DEFAULT = 5, // General/default scope, no specifics

    SYSTEM = 0,     // System-level errors
    STORAGE = 1,    // Storage and I/O errors
    SQL = 2,        // SQL execution errors
    STREAM = 3,     // Streaming engine errors
    MANAGEMENT = 4, // Cluster management errors
    DLANG = 6,      // DLang (a.k.a., Dolphin Script) language errors (e.g., syntax or semantic)

    ORCA = 7,         // Orca engine errors
    SHARK = 8,        // Shark engine errors
    SECURITY = 9,     // Security and authentication errors
    NETWORK = 10,     // Network and communication errors
    TRANSACTION = 11, // Transaction processing errors
    MEMORY = 12,      // Memory management errors

    // add more if needed
};

class SWORDFISH_API SWORDFISH_API TraceableException : public exception {
public:
	void addPath(const string& path);
	const string& getPath() const {return  path_;}
	const string& getLastThrow() const { return lastThrow_;}
	void setLastThrow(const string& lastThrow);

protected:
	string path_;
	string lastThrow_;
};

class SWORDFISH_API IncompatibleTypeException: public TraceableException{
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

class SWORDFISH_API IllegalArgumentException : public TraceableException{
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

class SWORDFISH_API RuntimeException: public TraceableException{
public:
	RuntimeException(const string& errMsg):errMsg_(errMsg){}
	virtual const char* what() const throw(){
		return errMsg_.c_str();
	}
	virtual ~RuntimeException() throw(){}

private:
	const string errMsg_;
};

class SWORDFISH_API OperatorRuntimeException: public TraceableException{
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

class SWORDFISH_API TableRuntimeException: public TraceableException{
public:
	TableRuntimeException(const string& errMsg): errMsg_(errMsg){}
	virtual const char* what() const throw(){
		return errMsg_.c_str();
	}
	virtual ~TableRuntimeException() throw(){}

private:
	const string errMsg_;
};

class SWORDFISH_API MemoryException: public TraceableException{
public:
	MemoryException():errMsg_("Out of memory"){}
	virtual const char* what() const throw(){
		return errMsg_.c_str();
	}
	virtual ~MemoryException() throw(){}

private:
	const string errMsg_;
};

class SWORDFISH_API IOException: public TraceableException{
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

class SWORDFISH_API FileChunkVersionCheckException : public exception{
public:
    FileChunkVersionCheckException(const string& errMsg) : errMsg_(errMsg){}
    virtual const char* what() const throw(){
        return errMsg_.c_str();
    }
    virtual ~FileChunkVersionCheckException() throw(){}

private:
    const string errMsg_;
};

class SWORDFISH_API DataCorruptionException: public exception {
public:
	DataCorruptionException(const string& errMsg) : errMsg_("<DataCorruption>" + errMsg){}
	virtual const char* what() const throw(){
		return errMsg_.c_str();
	}
	virtual ~DataCorruptionException() throw(){}

private:
	const string errMsg_;
};

class SWORDFISH_API NotLeaderException: public exception {
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

class SWORDFISH_API ChunkInTransactionException: public exception {
public:
	ChunkInTransactionException(const string& errMsg) : errMsg_("<ChunkInTransaction>" + errMsg){}
	virtual const char* what() const throw(){
		return errMsg_.c_str();
	}
	virtual ~ChunkInTransactionException() throw(){}

private:
	const string errMsg_;
};

class SWORDFISH_API ChunkResolutionException: public exception {
public:
	ChunkResolutionException(const string& errMsg) : errMsg_("<ChunkResolutionException>" + errMsg){}
	virtual const char* what() const throw(){
		return errMsg_.c_str();
	}
	virtual ~ChunkResolutionException() throw(){}

private:
	const string errMsg_;
};

class SWORDFISH_API ChunkInRecoveryException: public exception {
public:
	ChunkInRecoveryException(const string& errMsg) : errMsg_("<ChunkInRecovery>" + errMsg){}
	virtual const char* what() const throw(){
		return errMsg_.c_str();
	}
	virtual ~ChunkInRecoveryException() throw(){}

private:
	const string errMsg_;
};

class SWORDFISH_API DataNodeNotAvailException : public exception {
public:
	DataNodeNotAvailException(const string& errMsg) : errMsg_("<DataNodeNotAvail>" + errMsg){}
	virtual const char* what() const throw(){
		return errMsg_.c_str();
	}
	virtual ~DataNodeNotAvailException() throw(){}

private:
	const string errMsg_;
};

class SWORDFISH_API ControllerNotAvailException : public exception {
public:
	ControllerNotAvailException(const string& errMsg) : errMsg_("<ControllerNotAvail>" + errMsg){}
	virtual const char* what() const throw(){
		return errMsg_.c_str();
	}
	virtual ~ControllerNotAvailException() throw(){}

private:
	const string errMsg_;
};

class SWORDFISH_API ControllerNotReadyException : public exception {
public:
	ControllerNotReadyException() : errMsg_("<ControllerNotReady>"){}
	virtual const char* what() const throw(){
		return errMsg_.c_str();
	}
	virtual ~ControllerNotReadyException() throw(){}

private:
	const string errMsg_;
};

class SWORDFISH_API MathException: public TraceableException {
public:
	MathException(const string& errMsg) : errMsg_(errMsg){}
	virtual const char* what() const throw(){
		return errMsg_.c_str();
	}
	virtual ~MathException() throw(){}

private:
	const string errMsg_;
};

class SWORDFISH_API DataNodeNotReadyException: public TraceableException{
public:
    DataNodeNotReadyException(const string& errMsg) : errMsg_("<DataNodeNotReady>" + errMsg){}
    virtual const char* what() const throw(){
        return errMsg_.c_str();
    }
    virtual ~DataNodeNotReadyException() throw(){}

private:
    const string errMsg_;
};

class SWORDFISH_API NoPrivilegeException: public TraceableException{
public:
	NoPrivilegeException(const string& errMsg, bool notAuthenticated) : errMsg_((notAuthenticated ? "<NotAuthenticated>" : "<NoPrivilege>") + errMsg){}
    virtual const char* what() const throw(){
        return errMsg_.c_str();
    }
    virtual ~NoPrivilegeException() throw(){}

private:
    const string errMsg_;
};

class SWORDFISH_API UserException: public TraceableException{
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

class SWORDFISH_API SyntaxException: public exception{
public:
	SyntaxException(const string& errMsg): errMsg_(errMsg){}
	virtual const char* what() const throw(){
		return errMsg_.c_str();
	}
	virtual ~SyntaxException() throw(){}

private:
	const string errMsg_;
};

class SWORDFISH_API TestingException: public exception{
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
class SWORDFISH_API TransactionFinishException : public exception {
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

/** Used by compute nodes to invalidate cached data. */
class SWORDFISH_API CacheInvalidException : public TraceableException {
public:
	CacheInvalidException(const string &errMsg) : errMsg_("<CacheInvalid>" + errMsg) {
	}
	virtual ~CacheInvalidException() throw() {
	}

	virtual const char* what() const throw() {
		return errMsg_.c_str();
	}

private:
	const string errMsg_;
};

//==============================================================================
// OLTP Exception
//==============================================================================

/**
 * DML operations (query/insert/delete/update) may throw NeedRetryException.
 * When catch this exception, should rollback current transaction and begin a
 * new transaction to retry.
 */
class SWORDFISH_API OLTPNeedRetryException : public std::exception {
   public:
    OLTPNeedRetryException(const string& err) : err_(err) {}

    virtual const char* what() const noexcept override { return err_.c_str(); }

   private:
    string err_;
};

/**
 * DML operations (query/insert/delete/update) may throw NotRetryException
 * When catch this exception, just rollback current transaction and do NOT
 * retry.
 */
class SWORDFISH_API OLTPNotRetryException : public std::exception {
   public:
    OLTPNotRetryException(const string& err) : err_(err) {}

    virtual const char* what() const noexcept override { return err_.c_str(); }

   private:
    string err_;
};

}  // namepsace ddb

#endif /* EXCEPTIONS_H_ */
