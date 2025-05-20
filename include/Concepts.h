/*
 * Concepts.h
 * Header for various objects and statements used for ZBase
 * Created on: Jul 21, 2012
 *      Author: dzhou
 */

#ifndef CONCEPTS_H_
#define CONCEPTS_H_

#include <cstdint>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>
#include <stack>
#include <set>
#include <sstream>
#include <stdlib.h>
#include <iterator>
#include <iostream>
#include <random>

#include "CoreConcept.h"
#include "DolphinClass.h"
#include "OperatorImp.h"
#include "ScalarImp.h"
#include "Types.h"
namespace ddb {
class Tuple : public Object {
public:
	Tuple(const vector<ObjectSP>& arguments, bool isFunctionArgument = false, bool isDynamicVector = false, bool readonly=false): Object(OBJECT_TYPE::TUPLE), arguments_(arguments),
		isFunctionArgument_(isFunctionArgument), isDynamicVector_(isDynamicVector), readonly_(readonly){}
	Tuple(Session* session, const DataInputStreamSP& in);
	inline bool isFunctionArgument() const { return isFunctionArgument_;}
	inline bool isDynamicVector() const { return isDynamicVector_;}
	inline bool readonly() const { return readonly_;}
	ObjectSP getElement(int index) const {return arguments_[index];}
	int getElementCount() const { return arguments_.size();}
	virtual ConstantSP getValue(Heap* pHeap) { return getReference(pHeap);}
	virtual ConstantSP getReference(Heap* pHeap);
	virtual ConstantSP getComponent() const;
	virtual string getScript() const;
	virtual IO_ERR serialize(Heap* pHeap, const ByteArrayCodeBufferSP& buffer) const;
	virtual void collectUserDefinedFunctions(unordered_map<string,FunctionDef*>& functionDefs) const;
	virtual void collectUserDefinedFunctionsAndClasses(Heap* pHeap, unordered_map<string,FunctionDef*>& functionDefs, unordered_map<string,OOClass*>& classes) const;
	virtual ObjectSP copy(Heap* pHeap, const SQLContextSP& context, bool localize) const;
	virtual ObjectSP copyAndMaterialize(Heap* pHeap, const SQLContextSP& context, const TableSP& table) const;
	virtual bool mayContainColumnRefOrVariable() const { return true;}
	virtual void collectObjects(vector<const Object*>& vec) const;

private:
	vector<ObjectSP> arguments_;
	bool isFunctionArgument_;
	bool isDynamicVector_;
	bool readonly_;
};

class Expression: public Object{
public:
	Expression(const vector<ObjectSP> & objs, const vector<OperatorSP>&  optrs): Object(OBJECT_TYPE::EXPRESSION),
		objs_(objs),optrs_(optrs), annotation_(0){}
	Expression(const vector<ObjectSP> & objs, const vector<OperatorSP>&  optrs, int annotation): Object(OBJECT_TYPE::EXPRESSION),
		objs_(objs),optrs_(optrs), annotation_(annotation){}
	Expression(const SQLContextSP& context, Session* session, const DataInputStreamSP& in);
	virtual ~Expression(){}
	virtual ConstantSP getComponent() const;
	virtual ConstantSP getReference(Heap* pHeap);
	virtual ConstantSP getValue(Heap* pHeap);
	int getObjectCount() const {return objs_.size();}
	const ObjectSP& getObject(int index) const {return objs_[index];}
	const vector<ObjectSP>& getObjects() const {return objs_;}
	int getOperatorCount() const {return optrs_.size();}
	const OperatorSP& getOperator(int index) const {return optrs_[index];}
	const vector<OperatorSP>& getOperators() const {return optrs_;}
	virtual string getScript() const;
	virtual IO_ERR serialize(Heap* pHeap, const ByteArrayCodeBufferSP& buffer) const;
	virtual void collectUserDefinedFunctions(unordered_map<string,FunctionDef*>& functionDefs) const;
	virtual void collectUserDefinedFunctionsAndClasses(Heap* pHeap, unordered_map<string,FunctionDef*>& functionDefs, unordered_map<string,OOClass*>& classes) const;
	virtual ObjectSP realizeNonColumnExpression(Heap* pHeap, const TableSP& table);
	//Assume all local variables in the expression has been materialized
	virtual ObjectSP realizeNonColumnExpression(Heap* pHeap);
	inline int getAnnotation() const {return annotation_;}
	inline bool getAnnotation(int bitOffset) const {return annotation_ & (bitOffset << 1);}
	void setAnnotation(int bitOffset, bool bitOn);
	virtual ObjectSP copy(Heap* pHeap, const SQLContextSP& context, bool localize) const;
	virtual ObjectSP copyAndMaterialize(Heap* pHeap, const SQLContextSP& context, const TableSP& table) const;
	virtual bool mayContainColumnRefOrVariable() const { return true;}
	void collectObjects(vector<const Object*>& vec) const override;

	SWORDFISH_API static ConstantSP void_;
	SWORDFISH_API static ConstantSP null_;
	SWORDFISH_API static ConstantSP default_;
	SWORDFISH_API static ConstantSP true_;
	SWORDFISH_API static ConstantSP false_;
	SWORDFISH_API static ConstantSP one_;
	SWORDFISH_API static ConstantSP zero_;
	SWORDFISH_API static ConstantSP voidDouble2_;
	SWORDFISH_API static OperatorSP logicAnd_;
	static SQLContextSP context_;

protected:
	static ObjectSP partialEvaluate(Heap* pHeap, const vector<ObjectSP>& objs, const vector<OperatorSP>& optrs, int annotation);

protected:
	vector<ObjectSP> objs_;
    vector<OperatorSP> optrs_;
    /**
     * The field doesn't involve in the calculation of the expression. But it is used to annotate the
     * expression. For example, expressions in the SQL where clause may help decide which partition is
     * relevant to the query. In the case of range partition or value partition, this flag can further
     * tell if one can optimize the where clause, i.e. removal of the whole or part of the expression
     * from the where clause. In this use case:
     * 	bit0: 0 the part of the expression is used to decide the relevant partition.
     * 	      1 the whole expression is used to decide the relevant partition.
     * 	bit1: value partitioning column is used to decide the relevant partition.
     * 	bit2: range partitioning column is used to decide the relevant partition.
     * 	bit3: list partitioning column is used to decide the relevant partition.
     */
    int annotation_;
};
}
#endif /* CONCEPTS_H_ */
