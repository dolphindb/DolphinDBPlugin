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

class IncompatibleTypeException: public exception{
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

class IllegalArgumentException : public exception{
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

class RuntimeException: public exception{
public:
	RuntimeException(const string& errMsg):errMsg_(errMsg){}
	virtual const char* what() const throw(){
		return errMsg_.c_str();
	}
	virtual ~RuntimeException() throw(){}

private:
	const string errMsg_;
};

class OperatorRuntimeException: public exception{
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

class TableRuntimeException: public exception{
public:
	TableRuntimeException(const string& errMsg): errMsg_(errMsg){}
	virtual const char* what() const throw(){
		return errMsg_.c_str();
	}
	virtual ~TableRuntimeException() throw(){}

private:
	const string errMsg_;
};

class MemoryException: public exception{
public:
	MemoryException():errMsg_("Out of memory"){}
	virtual const char* what() const throw(){
		return errMsg_.c_str();
	}
	virtual ~MemoryException() throw(){}

private:
	const string errMsg_;
};

class IOException: public exception{
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

class MathException: public exception {
public:
	MathException(const string& errMsg) : errMsg_(errMsg){}
	virtual const char* what() const throw(){
		return errMsg_.c_str();
	}
	virtual ~MathException() throw(){}

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

class UserException: public exception{
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

#endif /* EXCEPTIONS_H_ */
