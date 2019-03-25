/*
 * ObjectImp.h
 *
 *  Created on: Apr 15, 2017
 *      Author: dzhou
 */

#ifndef OBJECTIMP_H_
#define OBJECTIMP_H_

#include "Concepts.h"

class JITValue;
class Codegen;
class PrimitiveOperator : public Operator{
public:
	PrimitiveOperator(OptrFunc optrFunc, const string& optrSymbol, const string& optrFuncName, int priority, bool unary, TemplateOptr advFunc=0, const string& templateSymbol = "")
		: Operator(priority, unary), optrFunc_(optrFunc), optrSymbol_(optrSymbol), optrFuncName_(optrFuncName), advFunc_(advFunc), templateSymbol_(templateSymbol){}
	PrimitiveOperator(int priority, bool unary, Session* session, const DataInputStreamSP& buffer);
	virtual ConstantSP evaluate(Heap* heap, const ConstantSP& a, const ConstantSP& b);
	virtual const string& getName() const { return optrFuncName_;}
	virtual string getOperatorSymbol() const { return optrSymbol_;}
	virtual string getTemplateSymbol() const { return templateSymbol_;}
	virtual bool isPrimitiveOperator() const { return true;}
	virtual IO_ERR serialize(Heap* pHeap, const ByteArrayCodeBufferSP& buffer) const;
	virtual void collectUserDefinedFunctions(unordered_map<string,FunctionDef*>& functionDefs) const {}
	virtual InferredType inferType(Heap* heap, unordered_set<ControlFlowEdge> & vis, std::vector<StatementSP> & fromCFGEdges, void * edgeStartNode, const ConstantSP& a, const ConstantSP& b) override;
private:
	OptrFunc optrFunc_;
	string optrSymbol_;
	string optrFuncName_;
	TemplateOptr advFunc_;
	string templateSymbol_;
};

class FunctionOperator : public Operator{
public:
	FunctionOperator(FunctionDefSP funcDefSP, bool unary, TemplateUserOptr advUserFunc = 0, const string& templateSymbol = "")
		: Operator(unary ? MONAD_PRIORITY : DYAD_PRIORITY, unary), funcDef_(funcDefSP), advUserFunc_(advUserFunc), templateSymbol_(templateSymbol){

		if(funcDefSP->getMinParamCount()>2 || funcDefSP->getMaxParamCount()<1)
			throw SyntaxException("Functions used as an operator in expression must have one or two parameters.");
		for(int i=0;i<funcDefSP->getParamCount();i++){
			if(!funcDefSP->getParam(i)->isReadOnly())
				throw SyntaxException("Functions used in expression must have read only parameters.");
		}
	}
	FunctionOperator(int priority, bool unary, Session* session, const DataInputStreamSP& buffer);
	virtual ConstantSP evaluate(Heap* heap, const ConstantSP& a, const ConstantSP& b);
	virtual const string& getName() const { return funcDef_->getName();}
	virtual string getOperatorSymbol() const { return funcDef_->getName();}
	virtual string getTemplateSymbol() const { return templateSymbol_;}
	virtual bool isPrimitiveOperator() const { return false;}
	virtual IO_ERR serialize(Heap* pHeap, const ByteArrayCodeBufferSP& buffer) const;
	const FunctionDefSP& getFunctionDef() const { return funcDef_;}
	virtual void collectUserDefinedFunctions(unordered_map<string,FunctionDef*>& functionDefs) const {funcDef_->collectUserDefinedFunctions(functionDefs);}
private:
	FunctionDefSP funcDef_;
	TemplateUserOptr advUserFunc_;
	string templateSymbol_;
};

class ThreadFunctionCall: public Function{
public:
	ThreadFunctionCall(const FunctionDefSP& funcSP, const vector<ConstantSP>& arguments): Function(funcSP), arguments_(arguments){}
	ThreadFunctionCall(const FunctionDefSP& funcSP, Session* session, const DataInputStreamSP& buffer): Function(funcSP){
		throw RuntimeException("ThreadFunctionCall is not supposed to deserialize from stream");
	}
	virtual ~ThreadFunctionCall(){}
	virtual ConstantSP getReference(Heap* heap){ return funcSP_->call(heap, arguments_);}
	virtual int getArgumentCount() const {return (int)arguments_.size();}
	virtual ObjectSP getArgument(int index) const {return arguments_[index];}
	virtual string getScript() const { throw RuntimeException("ThreadFunctionCall::getScript() is not supported.");}
	virtual string getTemplateName() const { return "";}
	virtual FUNCTIONCALL_TYPE getFunctionCallType() const {return THREADCALL;}
	virtual bool isPartialCall() const {return false;}
	virtual IO_ERR serialize(Heap* pHeap, const ByteArrayCodeBufferSP& buffer) const {throw RuntimeException("ThreadFunctionCall::serialize not supported");}
	virtual Function* getInstance(const vector<ObjectSP>& arguments) const { throw RuntimeException("ThreadFunctionCall::getInstance not supported");}
private:
	vector<ConstantSP> arguments_;
};

class RegularFunctionCall: public Function{
public:
	RegularFunctionCall(const FunctionDefSP& funcSP, const vector<ObjectSP>& arguments, bool qualifier, bool partialCall): Function(funcSP),
		arguments_(arguments), qualifier_(qualifier), partialCall_(partialCall){}
	RegularFunctionCall(const FunctionDefSP& funcSP, const vector<ConstantSP>& arguments, bool qualifier, bool partialCall);
	RegularFunctionCall(const FunctionDefSP& funcSP, Session* session, const DataInputStreamSP& buffer);
	RegularFunctionCall(const SQLContextSP& context, const FunctionDefSP& funcSP, Session* session, const DataInputStreamSP& buffer);
	virtual ~RegularFunctionCall(){}
	virtual ConstantSP getReference(Heap* pHeap);
	virtual int getArgumentCount() const {return (int)arguments_.size();}
	virtual ObjectSP getArgument(int index) const {return arguments_[index];}
	virtual string getScript() const;
	virtual string getTemplateName() const { return "";}
	virtual FUNCTIONCALL_TYPE getFunctionCallType() const {return REGULARCALL;}
	virtual bool isPartialCall() const { return partialCall_;}
	virtual IO_ERR serialize(Heap* pHeap, const ByteArrayCodeBufferSP& buffer) const;
	virtual Function* getInstance(const vector<ObjectSP>& arguments) const { return new RegularFunctionCall(funcSP_, arguments, qualifier_, partialCall_);}
	virtual void collectVariables(vector<int>& vars, int minIndex, int maxIndex) const;

private:
	vector<ObjectSP> arguments_;
	bool qualifier_;
	bool partialCall_;
};

class AbstractFunctionDef : public FunctionDef{
public:
	AbstractFunctionDef(FUNCTIONDEF_TYPE defType, const string& name, const vector<ParamSP>& params, bool hasReturnValue=true, bool aggregation=false, bool sequential=false): FunctionDef(defType, name, params,
		hasReturnValue, aggregation, sequential), val_(name){}
	AbstractFunctionDef(FUNCTIONDEF_TYPE defType, const string& name, int minParamNum, int maxParamNum, bool hasReturnValue, bool aggregation=false, bool sequential=false) : FunctionDef(defType, name, minParamNum,
		maxParamNum, hasReturnValue, aggregation, sequential), val_(name){}
	virtual ~AbstractFunctionDef(){}
	virtual char getBool() const {throw IncompatibleTypeException(DT_BOOL,DT_FUNCTIONDEF);}
	virtual char getChar() const {throw IncompatibleTypeException(DT_CHAR,DT_FUNCTIONDEF);}
	virtual short getShort() const {throw IncompatibleTypeException(DT_SHORT,DT_FUNCTIONDEF);}
	virtual int getInt() const {throw IncompatibleTypeException(DT_INT,DT_FUNCTIONDEF);}
	virtual long long getLong() const {throw IncompatibleTypeException(DT_LONG,DT_FUNCTIONDEF);}
	virtual INDEX getIndex() const {throw IncompatibleTypeException(DT_INDEX,DT_FUNCTIONDEF);}
	virtual float getFloat() const {throw IncompatibleTypeException(DT_FLOAT,DT_FUNCTIONDEF);}
	virtual double getDouble() const {throw IncompatibleTypeException(DT_DOUBLE,DT_FUNCTIONDEF);}
	virtual string getString() const {return val_;}
	virtual const string& getStringRef() const {return val_;}
	virtual const string& getStringRef(int index) const {return val_;}
	virtual bool isNull() const {return false;}
	virtual void setString(const string& val) {val_=val;}
	virtual void setNull(){}
	virtual void nullFill(const ConstantSP& val){}
	virtual bool isNull(INDEX start, int len, char* buf) const {
		for(int i=0;i<len;++i)
			buf[i]=false;
		return true;
	}
	virtual bool isValid(INDEX start, int len, char* buf) const {
		for(int i=0;i<len;++i)
			buf[i]=true;
		return true;
	}
	virtual ConstantSP getInstance() const { throw RuntimeException("FunctionDef::getInstance method not supported.");}
	virtual ConstantSP getValue() const { throw RuntimeException("Function definition [" + name_ + "] is not copyable and can't serve as an operand in numeric calculation.");}
	virtual DATA_FORM getForm() const {return DF_SCALAR;}
	virtual long long getAllocatedMemory() const {return sizeof(string)+val_.size();}
	virtual int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int& numElement, int& partial) const;
	virtual IO_ERR deserialize(DataInputStream* in, INDEX indexStart, INDEX& numElement);
	virtual int compare(int index, const ConstantSP& target) const {
		return val_.compare(target->getString());
	}
protected:
	void checkArgumentSize(int actualArgCount);

private:
	mutable string val_;
};

class SystemFunction : public AbstractFunctionDef{
public:
	SystemFunction(const string& name, SysFunc func, int minParamNum, int maxParamNum, bool aggregation=false, bool sequential=false, bool internal=false) : AbstractFunctionDef(SYSFUNC, name, minParamNum, maxParamNum, true, aggregation,sequential), sysFunc_(func){
		setInternal(internal);
	}
	SystemFunction(const string& name, SysFunc func, int minParamNum, int maxParamNum, const vector<ParamSP>& params, bool aggregation=false, bool sequential=false);
	virtual ~SystemFunction(){}
	virtual ConstantSP call(Heap* heap, vector<ConstantSP>& arguments);
	virtual ConstantSP call(Heap* heap, const ConstantSP& a, const ConstantSP& b);
	virtual ConstantSP call(Heap* heap,vector<ObjectSP>& arguments);
	virtual IO_ERR serialize(Heap* pHeap, const ByteArrayCodeBufferSP& buffer) const;
	SysFunc getFunctionPointer() const {return sysFunc_;}

private:
	SysFunc sysFunc_;
};

class SystemProcedure : public AbstractFunctionDef{
public:
	SystemProcedure(const string& name, SysProc proc, int minParamNum, int maxParamNum, bool internal=false) : AbstractFunctionDef(SYSPROC, name, minParamNum, maxParamNum, false, false), sysProc_(proc){
		setInternal(internal);
	}
	SystemProcedure(const string& name, SysProc proc, int minParamNum, int maxParamNum, const vector<ParamSP>&);
	virtual ~SystemProcedure(){}
	virtual ConstantSP call(Heap* heap, vector<ConstantSP>& arguments);
	virtual ConstantSP call(Heap* heap, const ConstantSP& a, const ConstantSP& b);
	virtual ConstantSP call(Heap* heap,vector<ObjectSP>& arguments);
	virtual IO_ERR serialize(Heap* pHeap, const ByteArrayCodeBufferSP& buffer) const;
	SysProc getFunctionPointer() const {return sysProc_;}

private:
	SysProc sysProc_;
};

class OperatorFunction : public AbstractFunctionDef{
public:
	OperatorFunction(const string& name, OptrFunc func, const vector<ParamSP>& params, bool aggregation=false, bool sequential=false) : AbstractFunctionDef(OPTRFUNC, name, params, true, aggregation, sequential), optrFunc_(func){}
	OperatorFunction(const string& name, OptrFunc func, int minParamNum, int maxParamNum, bool aggregation=false, bool sequential=false) : AbstractFunctionDef(OPTRFUNC, name, minParamNum, maxParamNum, true, aggregation, sequential), optrFunc_(func){}
	virtual ~OperatorFunction(){}
	OptrFunc getOptrFunc(){ return optrFunc_;}
	virtual ConstantSP call(Heap* heap, vector<ConstantSP>& arguments);
	virtual ConstantSP call(Heap* heap, const ConstantSP& a, const ConstantSP& b);
	virtual ConstantSP call(Heap* heap, vector<ObjectSP>& arguments);
	virtual IO_ERR serialize(Heap* pHeap, const ByteArrayCodeBufferSP& buffer) const;
	OptrFunc getFunctionPointer() const {return optrFunc_;}

private:
	OptrFunc optrFunc_;
};

class UserDefinedFunction : public AbstractFunctionDef{
public:
	UserDefinedFunction(const string& name, const vector<ParamSP>& params, bool hasReturnValue=true, bool aggregation=false): AbstractFunctionDef(USERDEFFUNC, name,params,hasReturnValue, aggregation){}
	UserDefinedFunction(const string& name, const HeapSP& heapSP, const vector<ParamSP>& params, const vector<StatementSP>& statements,	bool hasReturnValue=true,
			bool aggregation=false) : AbstractFunctionDef(USERDEFFUNC, name,params,hasReturnValue, aggregation), heap_(heapSP), statements_(statements){
		heap_->setDefMode();
	}
	UserDefinedFunction(const string& name, Session* session, const DataInputStreamSP& buffer);
	virtual ~UserDefinedFunction(){}
	int getHeapSize() const { return heap_->size();}
	HeapSP getHeap() const { return heap_;}
	Session* getSession() const { return heap_->currentSession();}
	int getStatementCount() const { return statements_.size();}
	StatementSP getStatement(int index) const {	return statements_[index];}
	void setDefinition(HeapSP& heap, const vector<StatementSP>& statements);
	bool isBodyDefined() const {return !heap_.isNull();}
	virtual string getScript() const;
	virtual IO_ERR serialize(Heap* pHeap, const ByteArrayCodeBufferSP& buffer) const;
	virtual void collectUserDefinedFunctions(unordered_map<string,FunctionDef*>& functionDefs) const;

private:
	string generateScript() const;

protected:
	HeapSP heap_;
	vector<StatementSP> statements_;
};

class PartialFunction : public AbstractFunctionDef{
public:
	PartialFunction(Function* pFuncCall);
	PartialFunction(FunctionDefSP funcDef, const vector<ConstantSP>& arguments);
	PartialFunction(Session* session, const DataInputStreamSP& buffer);
	virtual ~PartialFunction(){}
	virtual ConstantSP call(Heap* heap, vector<ConstantSP>& arguments);
	virtual ConstantSP call(Heap* heap, const ConstantSP& a, const ConstantSP& b);
	virtual ConstantSP call(Heap* heap,vector<ObjectSP>& arguments);
	virtual FunctionDefSP materializeFunctionDef(Heap* pHeap);
	virtual string getScript() const {return getString();}
	virtual IO_ERR serialize(Heap* pHeap, const ByteArrayCodeBufferSP& buffer) const;
	virtual void collectUserDefinedFunctions(unordered_map<string,FunctionDef*>& functionDefs) const;

private:
	string generateScript() const;

private:
	FunctionDefSP funcDef_;
	FunctionDefSP materializedFuncDef_;
	vector<ObjectSP> args_;
	vector<ConstantSP> objs_;
	vector<int> missingArgs_;
};

class DynamicFunction : public AbstractFunctionDef{
public:
	DynamicFunction(const ObjectSP& var) : AbstractFunctionDef(DYNAMICFUNC, var->getScript(), 0, 0, true), var_(var){}
	DynamicFunction(Session* session, const DataInputStreamSP& buffer);
	virtual ~DynamicFunction(){}
	virtual ConstantSP call(Heap* heap, vector<ConstantSP>& arguments);
	virtual ConstantSP call(Heap* heap, const ConstantSP& a, const ConstantSP& b);
	virtual ConstantSP call(Heap* heap,vector<ObjectSP>& arguments);
	virtual FunctionDefSP materializeFunctionDef(Heap* pHeap);
	virtual IO_ERR serialize(Heap* pHeap, const ByteArrayCodeBufferSP& buffer) const;
	virtual void collectUserDefinedFunctions(unordered_map<string,FunctionDef*>& functionDefs) const;

private:
	ObjectSP var_;
};

/**
 * Users couldn't define a piecewise function directly. It must be returned by the function of 'piecewise'
 */
class PiecewiseFunction : public AbstractFunctionDef{
public:
	PiecewiseFunction(const vector<FunctionDefSP>& funcs, const ConstantSP& pieces, int flag);
	virtual ~PiecewiseFunction(){}
	virtual ConstantSP call(Heap* heap, vector<ConstantSP>& arguments);
	virtual ConstantSP call(Heap* heap, const ConstantSP& a, const ConstantSP& b);
	virtual ConstantSP call(Heap* heap,vector<ObjectSP>& arguments);
	virtual IO_ERR serialize(Heap* pHeap, const ByteArrayCodeBufferSP& buffer) const {
		//Not supposed to be called
		return OTHERERR;
	}

private:
	string generateScript() const;

private:
	vector<FunctionDefSP> funcs_;
	ConstantSP pieces_;
	int flag_;
};

class CodeFactory{
	typedef ObjectSP(CodeFactory::*ObjectFactoryFunc)(const SQLContextSP& context, Session* session, const DataInputStreamSP& buffer);

public:
	CodeFactory(){init();}
	ObjectSP readObject(const SQLContextSP& context, Session* session, const DataInputStreamSP& buffer);
	Operator* readOperator(Session* session, const DataInputStreamSP& buffer);

private:
	void init();

	ObjectSP readConstant(const SQLContextSP& context, Session* session, const DataInputStreamSP& buffer);
	ObjectSP readVirtualConstant(const SQLContextSP& context, Session* session, const DataInputStreamSP& buffer);

	ObjectSP readVariable(const SQLContextSP& context, Session* session, const DataInputStreamSP& buffer){
		return new Variable(buffer);
	}

	ObjectSP readGlobal(const SQLContextSP& context, Session* session, const DataInputStreamSP& buffer){
		return new Global(buffer);
	}

	ObjectSP readGlobalTable(const SQLContextSP& context, Session* session, const DataInputStreamSP& buffer){
		return new GlobalTable(session, buffer);
	}

	ObjectSP readDimension(const SQLContextSP& context, Session* session, const DataInputStreamSP& buffer){
		return new Dimension(session, buffer);
	}

	ObjectSP readFunctionCall(const SQLContextSP& context, Session* session, const DataInputStreamSP& buffer);

	ObjectSP readExpression(const SQLContextSP& context, Session* session, const DataInputStreamSP& buffer){
		return new Expression(context, session, buffer);
	}

	ObjectSP readColumnRef(const SQLContextSP& context, Session* session, const DataInputStreamSP& buffer){
		return new ColumnRef(context, buffer);
	}

	ObjectSP readColumnDef(const SQLContextSP& context, Session* session, const DataInputStreamSP& buffer){
		return new ColumnDef(context, session, buffer);
	}

	ObjectSP readTuple(const SQLContextSP& context, Session* session, const DataInputStreamSP& buffer){
		return new Tuple(session, buffer);
	}

	ObjectSP readSQLQuery(const SQLContextSP& context, Session* session, const DataInputStreamSP& buffer);

	ObjectSP readTableJoiner(const SQLContextSP& context, Session* session, const DataInputStreamSP& buffer){
		return new TableJoiner(session, buffer);
	}

	ObjectSP readGroupTask(const SQLContextSP& context, Session* session, const DataInputStreamSP& buffer){
		return new GroupTask(session, buffer);
	}

private:
	ObjectFactoryFunc arrObjectFactory[MAX_OBJECT_TYPES];
};


#endif /* OBJECTIMP_H_ */
