/*
 * SpecialConstant.h
 *
 *  Created on: Mar 22, 2021
 *      Author: dzhou
 */

#ifndef SPECIALCONSTANT_H_
#define SPECIALCONSTANT_H_

#include <string>
#include "CoreConcept.h"
#include "Exceptions.h"
#include "Types.h"
#include "Util.h"

namespace ddb {
class AnyVector;

typedef ObjectPtr<AnyVector> AnyVectorSP;

class AnyVector:public Vector{
public:
	AnyVector(int size, bool isColumnarTuple = false, DATA_TYPE dt = DT_VOID, int decimalExtra = -1, bool needInit = true);

	AnyVector(const vector<ConstantSP>& data, bool containNull, bool isColumnarTuple = false, DATA_TYPE dt = DT_VOID,
			int decimalExtra = -1);

	AnyVector(const deque<ConstantSP>& data, bool containNull, bool isColumnarTuple = false, DATA_TYPE dt = DT_VOID,
			int decimalExtra = -1);

	void initialize() override;

	void extend(const ConstantSP& iterator);
	void insert(INDEX index, const ConstantSP& item);
	void removeItem(const ConstantSP& item);
	ConstantSP popItem(INDEX index = -1);
	INDEX countItem(const ConstantSP& item);
	INDEX findItem(const ConstantSP& item, INDEX start = 0, INDEX end = -1);
	~AnyVector() override{}
	bool equal(const ConstantSP& other) const override;
	bool containNotMarshallableObject() const override;
	bool isLargeConstant() const override;
	bool getNullFlag() const override {return containNull_;}
	void setNullFlag(bool containNull) override{containNull_=containNull;}
	INDEX getCapacity() const override {return data_.capacity();}
	bool isFastMode() const override {return false;}
	short getUnitLength() const override {return 0;}
	void clear() override;
	bool sizeable() const override {return true;}
	DATA_TYPE getRawType() const override { return DT_ANY;}
	string getString(INDEX index) const override {return data_[index]->getString();}
	string getString(Heap* heap, INDEX index) const override {return data_[index]->getString(heap);}
	const DolphinString& getStringRef(INDEX index) const override { throw RuntimeException("getStringRef method not supported for AnyVector");}
	bool set(INDEX index, const ConstantSP& value, INDEX valueIndex) override;
	bool set(INDEX index, const ConstantSP& value) override;
	bool set(const ConstantSP& index, const ConstantSP& value) override;
	bool set(Heap* heap, const ConstantSP& index, const ConstantSP& value, int dim) override;
	bool set(const ConstantSP& index, const ConstantSP& value, const ConstantSP& valueIndex) override { return set(index, value->get(valueIndex)); }
	bool setItem(INDEX index, const ConstantSP& value) override;
	bool modifyMember(Heap* heap, const FunctionDefSP& func, const ConstantSP& index, const ConstantSP& parameters, int dim) override;
	bool assign(const ConstantSP& value) override;
	using Vector::get;
	ConstantSP get(INDEX index) const override {return data_[index];}
	ConstantSP get(const ConstantSP& index) const override;
	ConstantSP get(INDEX offset, const ConstantSP& index) const override;
	const ConstantSP& getItem(INDEX index, ConstantSP& cache) const override {return data_[index];}
	bool hasNull() override{return  hasNull(0, data_.size());}
	bool hasNull(INDEX start, INDEX length) override;
	bool isNull(INDEX index) const override;
	bool isNull() const override {return false;}
	void setNull(INDEX index) override;
	void setNull() override{}
	void fill(INDEX start, INDEX length, const ConstantSP& value, INDEX valueOffset = 0) override;
	void nullFill(const ConstantSP& val) override;
	bool isNull(INDEX start, int len, char* buf) const override;
	bool isNull(INDEX* indices, int len, char* buf) const override;
	bool isValid(INDEX start, int len, char* buf) const override;
	bool isValid(INDEX* indices, int len, char* buf) const override;
	ConstantSP getSubVector(INDEX start, INDEX length) const override;
	ConstantSP getSubVector(INDEX start, INDEX length, INDEX capacity) const override;
	bool getSubVector(INDEX start, INDEX length, ConstantSP &result) const override;
	ConstantSP getInstance(INDEX size) const override;
	ConstantSP getValue() const override;
	ConstantSP getValue(INDEX capacity) const override;
	ObjectSP deepCopy() const override;
	using Vector::append;
	bool append(const ConstantSP& value, bool wholistic);
	bool append(const ConstantSP& value) override;
	bool append(const ConstantSP& value, INDEX appendSize) override;
	bool append(const ConstantSP& value, INDEX start, INDEX count) override;
	bool remove(INDEX count) override;
	bool remove(const ConstantSP& index) override;
	void resize(INDEX size) override;
	void prev(INDEX steps) override;
	void next(INDEX steps) override;
	void contain(const ConstantSP& targetSP, const ConstantSP& resultSP) const override;
	void find(INDEX start, INDEX length, const ConstantSP& targetSP, const ConstantSP& resultSP) override{
		throw RuntimeException("find method not supported for AnyVector");
	}
	char getBool() const override;
	char getChar() const override;
	short getShort() const override;
	int getInt() const override;
	long long getLong() const override;
	INDEX getIndex() const override;
	float getFloat() const override;
	double getDouble() const override;
	char getBool(INDEX index) const override {return get(index)->getBool();}
	char getChar(INDEX index) const override { return get(index)->getChar();}
	short getShort(INDEX index) const override { return get(index)->getShort();}
	int getInt(INDEX index) const override {return get(index)->getInt();}
	long long getLong(INDEX index) const override {return get(index)->getLong();}
	INDEX getIndex(INDEX index) const override {return get(index)->getIndex();}
	float getFloat(INDEX index) const override {return get(index)->getFloat();}
	double getDouble(INDEX index) const override {return get(index)->getDouble();}
	IO_ERR serialize(Heap* pHeap, const ByteArrayCodeBufferSP& buffer) const override;
	int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int& numElement, int& partial) const override;
    int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int targetNumElement, int& numElement, int& partial) const override;
	bool getBool(INDEX start, int len, char* buf) const override;
	bool getChar(INDEX start, int len,char* buf) const override;
	bool getShort(INDEX start, int len, short* buf) const override;
	bool getInt(INDEX start, int len, int* buf) const override;
	bool getLong(INDEX start, int len, long long* buf) const override;
	bool getIndex(INDEX start, int len, INDEX* buf) const override;
	bool getFloat(INDEX start, int len, float* buf) const override;
	bool getDouble(INDEX start, int len, double* buf) const override;
	const char* getBoolConst(INDEX start, int len, char* buf) const override;
	const char* getCharConst(INDEX start, int len,char* buf) const override;
	const short* getShortConst(INDEX start, int len, short* buf) const override;
	const int* getIntConst(INDEX start, int len, int* buf) const override;
	const long long* getLongConst(INDEX start, int len, long long* buf) const override;
	const INDEX* getIndexConst(INDEX start, int len, INDEX* buf) const override;
	const float* getFloatConst(INDEX start, int len, float* buf) const override;
	const double* getDoubleConst(INDEX start, int len, double* buf) const override;
    void setItemToHeap(Heap* pHeap,INDEX heapIndex,  INDEX itemIndex, const string& name) override;
    bool getSymbol(INDEX start, int len, int* buf, SymbolBase* symBase, bool insertIfNotThere) const override {
		throw RuntimeException("getSymbol method not supported for AnyVector");
	}
	const int* getSymbolConst(INDEX start, int len, int* buf, SymbolBase* symBase, bool insertIfNotThere) const override {
		throw RuntimeException("getSymbolConst method not supported for AnyVector");
	}
	bool getString(INDEX start, int len, DolphinString** buf) const override {
		throw RuntimeException("getString method not supported for AnyVector");
	}

	using Vector::getString;
	bool getString(INDEX start, int len, char** buf) const override {
		throw RuntimeException("getString method not supported for AnyVector");
	}

	DolphinString** getStringConst(INDEX start, int len, DolphinString** buf) const override {
		throw RuntimeException("getStringConst method not supported for AnyVector");
	}

	char** getStringConst(INDEX start, int len, char** buf) const override {
		throw RuntimeException("getStringConst method not supported for AnyVector");
	}

	const ConstantSP& getExactItem(INDEX index, const ConstantSP& result) const override;
	void getRowSlice(INDEX index, ConstantSP& result) const;

public:  /// getDecimal{32,64,128}
	int getDecimal32(int scale) const override;
	long long getDecimal64(int scale) const override;
	int128 getDecimal128(int scale) const override;

	int getDecimal32(INDEX index, int scale) const override;
	long long getDecimal64(INDEX index, int scale) const override;
	int128 getDecimal128(INDEX index, int scale) const override;

	bool getDecimal32(INDEX start, int len, int scale, int *buf) const override;
	bool getDecimal64(INDEX start, int len, int scale, long long *buf) const override;
	bool getDecimal128(INDEX start, int len, int scale, int128 *buf) const override;

	const int* getDecimal32Const(INDEX start, int len, int scale, int *buf) const override;
	const long long* getDecimal64Const(INDEX start, int len, int scale, long long *buf) const override;
	const int128* getDecimal128Const(INDEX start, int len, int scale,
			int128 *buf) const override;

    IO_ERR deserialize(DataInputStream* in, INDEX indexStart, int offset, INDEX targetNumElement, INDEX& numElement, int& partial) override;

public:
	INDEX size() const override {return data_.size();}
	long long count() const override{
		return count(0, data_.size());
	}
	long long count(INDEX start, INDEX length) const override;
    int compare(INDEX indexLeft, INDEX indexRight) const override {
        return data_[indexLeft] == data_[indexRight];
	}
    /**
	 * @param rightMost If there are multiple maximum/minimum values, choose the last one if `rightMost` is true.
	 */
	INDEX imax(bool rightMost = false) const override {throw RuntimeException("imax method not supported for AnyVector");}
	INDEX imin(bool rightMost = false) const override {throw RuntimeException("imin method not supported for AnyVector");}
	INDEX imax(INDEX start, INDEX length, bool rightMost = false) const override {throw RuntimeException("imax method not supported for AnyVector");}
	INDEX imin(INDEX start, INDEX length, bool rightMost = false) const override {throw RuntimeException("imin method not supported for AnyVector");}

	ConstantSP avg() const override {return avg(0, data_.size());}
	ConstantSP avg(INDEX start, INDEX length) const override;
	void avg(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override { out->set(outputStart, avg(start, length)); }
	ConstantSP sum() const override {return sum(0, data_.size());}
	ConstantSP sum(INDEX start, INDEX length) const override;
	void sum(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override { out->set(outputStart, sum(start, length)); }
	ConstantSP sum2() const override {return sum2(0, data_.size());}
	ConstantSP sum2(INDEX start, INDEX length) const override;
	void sum2(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override { out->set(outputStart, sum2(start, length)); }
	ConstantSP prd() const override {return prd(0, data_.size());}
	ConstantSP prd(INDEX start, INDEX length) const override;
	void prd(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override { out->set(outputStart, prd(start, length)); }
	ConstantSP var() const override {return var(0, data_.size());}
	ConstantSP var(INDEX start, INDEX length) const override;
	void var(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override { out->set(outputStart, var(start, length)); }
	ConstantSP std() const override {return std(0, data_.size());}
	ConstantSP std(INDEX start, INDEX length) const override;
	void std(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override { out->set(outputStart, std(start, length)); }
	ConstantSP median() const override {return median(0, data_.size());}
	ConstantSP median(INDEX start, INDEX length) const override;
	void median(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart=0) const override { out->set(outputStart, median(start, length)); }

	ConstantSP firstNot(const ConstantSP& exclude) const override { return firstNot(0, data_.size(), exclude); }
	ConstantSP firstNot(INDEX start, INDEX length, const ConstantSP& exclude) const override;
	void firstNot(INDEX start, INDEX length, const ConstantSP& exclude, const ConstantSP& out, INDEX outputStart = 0) const override {
		out->set(outputStart, firstNot(start, length, exclude));
	}
	ConstantSP lastNot(const ConstantSP& exclude) const override { return lastNot(0, data_.size(), exclude); }
	ConstantSP lastNot(INDEX start, INDEX length, const ConstantSP& exclude) const override;
	void lastNot(INDEX start, INDEX length, const ConstantSP& exclude, const ConstantSP& out, INDEX outputStart = 0) const override {
		out->set(outputStart, lastNot(start, length, exclude));
	}
	ConstantSP searchK(INDEX k) const override {throw RuntimeException("searchK method not supported for AnyVector");}
	ConstantSP searchK(INDEX start, INDEX length, INDEX k) const override {return searchK(k);}
	void searchK(INDEX start, INDEX length, INDEX k, const ConstantSP& out, INDEX outputStart=0) const override {
		throw RuntimeException("searchK method not supported for AnyVector");
	}
	ConstantSP mode() const override {throw RuntimeException("mode method not supported for AnyVector");}
	ConstantSP mode(INDEX start, INDEX length) const override { return mode();}
	void mode(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart=0) const override {
		throw RuntimeException("mode method not supported for AnyVector");
	}
	ConstantSP min() const override { return min(0, data_.size()); }
	ConstantSP min(INDEX start, INDEX length) const override;
	void min(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart=0) const override { out->set(outputStart, min(start, length)); }
	ConstantSP max() const override { return max(0, data_.size()); }
	ConstantSP max(INDEX start, INDEX length) const override;
	void max(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart=0) const override { out->set(outputStart, max(start, length)); }

	void neg() override{throw RuntimeException("neg method not supported for AnyVector");}
	void reverse() override{std::reverse(data_.begin(),data_.end());}
	void reverse(INDEX start, INDEX length) override{
		std::reverse(data_.begin()+start,data_.begin()+ start + length);
	}
	void replace(const ConstantSP& oldVal, const ConstantSP& newVal) override;
	void shuffle() override;
	bool findDuplicatedElements(Vector* indices, INDEX start, INDEX length, vector<pair<INDEX,INDEX> >& duplicates) override{return false;}
	bool findDuplicatedElements(INDEX start, INDEX length, vector<pair<INDEX,INDEX> >& duplicates) override{return false;}
	bool findUniqueElements(INDEX start, INDEX length, vector<pair<INDEX,INDEX> >& uniques) override{return false;}
	bool findRange(INDEX* ascIndices,const ConstantSP& target,INDEX* targetIndices,vector<pair<INDEX,INDEX> >& ranges) override{return false;}
	bool findRange(const ConstantSP& target,INDEX* targetIndices,vector<pair<INDEX,INDEX> >& ranges) override{return false;}
	INDEX lowerBound(INDEX start, const ConstantSP& target) override{
		throw RuntimeException("lowerBound method not supported for AnyVector");
	}
	virtual bool rank(bool sorted, INDEX* indices, INDEX* ranking){return false;}
	bool sortSelectedIndices(Vector* indices, INDEX start, INDEX length, bool asc, char nullsOrder) override{	return false;}
	bool isSorted(INDEX start, INDEX length, bool asc, bool strict, char nullsOrder) const override { return false;}
	bool sort(bool asc, char nullsOrder) override{return false;}
	bool sort(bool asc, Vector* indices, char nullsOrder) override{ return false;}
	INDEX sortTop(bool asc, Vector* indices, INDEX top, char nullsOrder) override{ return -1;}
	using Vector::getAllocatedMemory;
	virtual long long getAllocatedMemory();
	int getExtraParamForType() const override { return dt_; }
	const ConstantSP& getColumnRef(INDEX index) override { return data_[index]; }

	ConstantSP flatten(INDEX rowStart, INDEX count) const override;
	ConstantSP rowFirst(INDEX rowStart, INDEX count) const override;
	ConstantSP rowLast(INDEX rowStart, INDEX count) const override;
	ConstantSP rowFirstNot(INDEX rowStart, INDEX count, const ConstantSP& exclude) const override;
	ConstantSP rowLastNot(INDEX rowStart, INDEX count, const ConstantSP& exclude) const override;
	ConstantSP rowSum(INDEX rowStart, INDEX count) const override;
	ConstantSP rowSum2(INDEX rowStart, INDEX count) const override;
	ConstantSP rowCount(INDEX rowStart, INDEX count) const override;
	ConstantSP rowSize(INDEX rowStart, INDEX count) const override;
	ConstantSP rowAvg(INDEX rowStart, INDEX count) const override;
	ConstantSP rowStd(INDEX rowStart, INDEX count) const override;
	ConstantSP rowStdp(INDEX rowStart, INDEX count) const override;
	ConstantSP rowVar(INDEX rowStart, INDEX count) const override;
	ConstantSP rowVarp(INDEX rowStart, INDEX count) const override;
	ConstantSP rowMin(INDEX rowStart, INDEX count) const override;
	ConstantSP rowMax(INDEX rowStart, INDEX count) const override;
	ConstantSP rowProd(INDEX rowStart, INDEX count) const override;
	ConstantSP rowAnd(INDEX rowStart, INDEX count) const override;
	ConstantSP rowOr(INDEX rowStart, INDEX count) const override;
	ConstantSP rowXor(INDEX rowStart, INDEX count) const override;
	ConstantSP rowKurtosis(INDEX rowStart, INDEX count, bool biased) const override;
	ConstantSP rowSkew(INDEX rowStart, INDEX count, bool biased) const override;

	const ConstantSP & getElement(INDEX index) const;
	ConstantSP & getElement(INDEX index);
	void setElement(INDEX index, const ConstantSP &value);
	void setElement(INDEX index, ConstantSP &&value);
	void collectUserDefinedFunctions(unordered_map<string,FunctionDef*>& functionDefs) const override;
	bool isHomogeneousScalar(DATA_TYPE& type) const;
	/// @param allowVoid If true, treat void as homogeneous with any type.
	bool isHomogeneousScalar(DATA_TYPE &type, bool allowVoid) const;
	bool isHomogeneousScalarOrArray(DATA_TYPE& type, int& decimalExtra) const;
	bool isHomogeneousExtendedObj(DATA_TYPE& type) const;
	bool isConsistent() const;
	bool isConsistentArray(int& len) const;
	bool isTabular(DATA_TYPE& type, bool& isArray) const;
	bool isStrictTabular(DATA_TYPE& type, bool& isArray) const;
	ConstantSP convertToRegularVector() const;
	bool isDimension() const { return isDim_;}
	void setDimension(bool option) { isDim_ = option;}
	bool isTableColumn() const override { return isColumnarTuple_;}
	bool isColumnarTuple() const override { return isColumnarTuple_;}
	bool isObjectTuple() const override { return !isColumnarTuple_ && dt_ != DT_VOID;}
	void setTableColumn(bool option) { isColumnarTuple_ = option;}
	void setColumnarTuple(bool option) { isColumnarTuple_ = option;}
	void setExtraParamForType(int extra){
		DATA_TYPE type = (DATA_TYPE)extra;
		if(type != DT_ANY)
			dt_ = type;
	}
	void setDecimalExtra(int extra){ decimalExtra_ = extra; }
	int getDecimalExtra() { return decimalExtra_; }
	INDEX reserve(INDEX capacity) override;
	long long getAllocatedMemory() const override;
	const ConstantSP& getConstant(INDEX index) const { return data_[index];}
	void toVector(vector<ConstantSP>& v) const {
		if(!v.empty())
			v.clear();
		v.insert(v.begin(), data_.begin(), data_.end());
	}
	IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const override {
		if(!data_.empty())
			throw RuntimeException("Code serialization is supported for a non-empty tuple.");
		/*
		 * We want to keep the property isStatic after serialization.
		 * For this reason, we add 128 to the type if the object is static.
		 */
		DATA_TYPE type = getType();
		if(isStatic())
			type = (DATA_TYPE)((int)type + 128);
		short flag = (getForm() <<8) + type;
		buffer->write((char)OBJECT_TYPE::CONSTOBJ);
		buffer->write(flag);
		buffer->write((int)rows());
		return buffer->write((int)columns());
	}

	/**
	 * @brief cast a Vector to AnyVector
	 * @param v MUST BE a tuple Vector(isTuple() is true)
	 * @return nullptr if v isn't a tuple vector
	 * @return a copyed vector if v is subVector or SlicedVector(isView() is true)
	 * @return v if v is an AnyVector
	 */
	static AnyVectorSP toAnyVector(const VectorSP& v) {
		if (LIKELY(!v->isView()))
			return v;
		// v may be a SubVector or SlicedVector
		return ((Constant*)v.get())->getValue();
	}

	// for extendedObj
	virtual ConstantSP getMember(const ConstantSP& key) const override;
	virtual ConstantSP getMember(const string& key) const override;

private:
	ConstantSP calcRowMetric(INDEX rowStart, INDEX count, OptrFunc func, FastFunc fastFunc, const ConstantSP& b, DATA_TYPE type) const;
	ConstantSP sliceOneColumn(int colIndex, INDEX rowStart, INDEX rowEnd) const;
	/**
	 * colStart: inclusive
	 * colEnd: exclusive
	 * rowStart: inclusive
	 * rowEnd: exclusive
	 */
	ConstantSP sliceColumnRange(int colStart, int colEnd, INDEX rowStart, INDEX rowEnd) const;


private:
	mutable std::vector<ConstantSP> data_;
	// D20-27380: acquire lock only in getAllocatedMemory and modify operation.
	mutable Mutex dataMutex_;
	bool containNull_;
	bool isDim_;
	bool isColumnarTuple_;
	DATA_TYPE dt_;
	int decimalExtra_;
};

class SlicedVector : public Vector {
public:
	/**
	 * source: a regular array or huge array. Can't be a SubVector, SlicedVector, RepeatingVector, or ArrayVector.
	 * index: a regular index array.
	 */
	SlicedVector(const VectorSP& source, const VectorSP& index);
	~SlicedVector() override{}
	VECTOR_TYPE getVectorType() const override {return VECTOR_TYPE::SLICEDVECTOR;}
	VectorSP getSourceVector() const { return source_;}
	VectorSP getIndexVector() const { return index_;}
	void reset(const VectorSP& source, const VectorSP& index);
	bool copyable() const override {return false;}
	bool isView() const override {return true;}
	INDEX size() const override { return size_;}
	DATA_TYPE getRawType() const override {return source_->getRawType();}
	int getExtraParamForType() const override { return source_->getExtraParamForType();}
	SymbolBaseSP getSymbolBase() const override {return source_->getSymbolBase();}
	virtual DATA_FORM getForm() const { return DF_VECTOR;}
	ConstantSP getInstance() const override {return source_->getInstance(size_);}
	ConstantSP getInstance(INDEX size) const override {return source_->getInstance(size);}
	ConstantSP getValue() const override { return ((Constant*)source_.get())->get(index_);}
	ConstantSP getValue(INDEX capacity) const override;
	ConstantSP getWindow(INDEX colStart, int colLength, INDEX rowStart, int rowLength) const override;
	ConstantSP getSubVector(INDEX start, INDEX length, INDEX capacity) const override;
	ConstantSP getSubVector(INDEX start, INDEX length) const override;
	void fill(INDEX start, INDEX length, const ConstantSP& value, INDEX valueOffset = 0) override{throw RuntimeException("Sliced vector doesn't support method fill");}
	bool validIndex(INDEX uplimit) override{throw RuntimeException("Sliced vector doesn't support method validIndex");}
	bool validIndex(INDEX start, INDEX length, INDEX uplimit) override{throw RuntimeException("Sliced vector doesn't support method validIndex");}
	int compare(INDEX index, const ConstantSP& target) const override {return source_->compare(pindex_[index], target);}
    int compare(INDEX indexLeft, INDEX indexRight) const override {
		return source_->compare(pindex_[indexLeft], pindex_[indexRight]);
	}
	bool getNullFlag() const override {return source_->getNullFlag();}
	void setNullFlag(bool containNull) override{}
	bool hasNull() override{return hasNull(0, size_);}
	bool hasNull(INDEX start, INDEX length) override;
	INDEX getCapacity() const override {return size_;}
	bool isFastMode() const override { return source_->isFastMode();}
	void* getDataArray() const override { return nullptr;}
	bool isIndexArray() const override { return false;}
	INDEX* getIndexArray() const override { return nullptr;}
	short getUnitLength() const override {return source_->getUnitLength();}
	void** getDataSegment() const override {return nullptr;}
	bool sizeable() const override {return false;}
	char getBool() const override {return source_->getBool(pindex_[0]);}
	char getChar() const override { return source_->getChar(pindex_[0]);}
	short getShort() const override { return source_->getShort(pindex_[0]);}
	int getInt() const override {return source_->getInt(pindex_[0]);}
	long long getLong() const override {return source_->getLong(pindex_[0]);}
	INDEX getIndex() const override {return source_->getIndex(pindex_[0]);}
	float getFloat() const override {return source_->getFloat(pindex_[0]);}
	double getDouble() const override {return source_->getDouble(pindex_[0]);}
	ConstantSP get(INDEX index) const override { return source_->get(pindex_[index]);}
	using Vector::get;
	ConstantSP get(const ConstantSP& index) const override;
	ConstantSP get(INDEX offset, const ConstantSP& index) const override;
	using Vector::setBool;
	virtual void setBool(bool val){throw RuntimeException("Sliced vector doesn't support method setBool");}
	void setChar(char val) override{throw RuntimeException("Sliced vector doesn't support method setChar");}
	void setShort(short val) override{throw RuntimeException("Sliced vector doesn't support method setShort");}
	void setInt(int val) override{throw RuntimeException("Sliced vector doesn't support method setInt");}
	void setLong(long long val) override{throw RuntimeException("Sliced vector doesn't support method setLong");}
	void setIndex(INDEX val) override{throw RuntimeException("Sliced vector doesn't support method setIndex");}
	void setFloat(float val) override{throw RuntimeException("Sliced vector doesn't support method setFloat");}
	void setDouble(double val) override{throw RuntimeException("Sliced vector doesn't support method setDouble");}
	using Vector::setString;
	virtual void setString(const string& val){throw RuntimeException("Sliced vector doesn't support method setString");}
	void setNull() override{throw RuntimeException("Sliced vector doesn't support method setNull");}
	char getBool(INDEX index) const override {return source_->getBool(pindex_[index]);}
	char getChar(INDEX index) const override { return source_->getChar(pindex_[index]);}
	short getShort(INDEX index) const override { return source_->getShort(pindex_[index]);}
	int getInt(INDEX index) const override {return source_->getInt(pindex_[index]);}
	long long getLong(INDEX index) const override {return source_->getLong(pindex_[index]);}
	INDEX getIndex(INDEX index) const override {return source_->getIndex(pindex_[index]);}
	float getFloat(INDEX index) const override {return source_->getFloat(pindex_[index]);}
	double getDouble(INDEX index) const override {return source_->getDouble(pindex_[index]);}
	bool isNull(INDEX index) const override {return source_->isNull(pindex_[index]);}
	string getString(INDEX index) const override {return source_->getString(pindex_[index]);}
	const DolphinString& getStringRef(INDEX index) const override {return source_->getStringRef(pindex_[index]);}
	void clear() override{throw RuntimeException("Sliced vector doesn't support method clear");}
	bool remove(INDEX count) override{throw RuntimeException("Sliced vector doesn't support method remove");}
	bool remove(const ConstantSP& index) override{throw RuntimeException("Indexed vector doesn't support method remove");}
	void next(INDEX steps) override{throw RuntimeException("Sliced vector doesn't support method next");}
	void prev(INDEX steps) override{throw RuntimeException("Sliced vector doesn't support method prev");}
	void nullFill(const ConstantSP& val) override{throw RuntimeException("Sliced vector doesn't support method nullFill");}
	bool isNull(INDEX start, int len, char* buf) const override { return source_->isNull(pindex_ + start, len, buf);}
	bool isValid(INDEX start, int len, char* buf) const override {return source_->isValid(pindex_ + start, len, buf);}
	bool getBool(INDEX start, int len, char* buf) const override {return source_->getBool(pindex_ + start, len, buf);}
	const char* getBoolConst(INDEX start, int len, char* buf) const override {
		source_->getBool(pindex_ + start, len, buf);
		return buf;
	}
	char* getBoolBuffer(INDEX start, int len, char* buf) const override {return buf;}
	bool getChar(INDEX start, int len, char* buf) const override {return source_->getChar(pindex_ + start, len, buf);}
	const char* getCharConst(INDEX start, int len, char* buf) const override {
		source_->getChar(pindex_ + start, len, buf);
		return buf;
	}
	char* getCharBuffer(INDEX start, int len, char* buf) const override {return buf;}
	bool getShort(INDEX start, int len, short* buf) const override {return source_->getShort(pindex_ + start, len, buf);}
	const short* getShortConst(INDEX start, int len, short* buf) const override {
		source_->getShort(pindex_ + start, len, buf);
		return buf;
	}
	short* getShortBuffer(INDEX start, int len, short* buf) const override {return buf;}
	bool getInt(INDEX start, int len, int* buf) const override {return source_->getInt(pindex_ + start, len, buf);}
	const int* getIntConst(INDEX start, int len, int* buf) const override {
		source_->getInt(pindex_ + start, len, buf);
		return buf;
	}
	int* getIntBuffer(INDEX start, int len, int* buf) const override {return buf;}
	bool getLong(INDEX start, int len, long long* buf) const override {return source_->getLong(pindex_ + start, len, buf);}
	const long long* getLongConst(INDEX start, int len, long long* buf) const override {
		source_->getLong(pindex_ + start, len, buf);
		return buf;
	}
	long long* getLongBuffer(INDEX start, int len, long long* buf) const override {return buf;}
	bool getBinary(INDEX start, int len, int unitLength, unsigned char* buf) const override {return source_->getBinary(pindex_ + start, len, unitLength, buf);}
	const unsigned char* getBinaryConst(INDEX start, int len, int unitLength, unsigned char* buf) const override {
		source_->getBinary(pindex_ + start, len, unitLength, buf);
		return buf;
	}
	unsigned char* getBinaryBuffer(INDEX start, int len, int unitLength, unsigned char* buf) const override {return buf;}
	bool getIndex(INDEX start, int len, INDEX* buf) const override {return source_->getIndex(pindex_ + start, len, buf);}
	const INDEX* getIndexConst(INDEX start, int len, INDEX* buf) const override {
		source_->getIndex(pindex_ + start, len, buf);
		return buf;
	}
	INDEX* getIndexBuffer(INDEX start, int len, INDEX* buf) const override {return buf;}
	bool getFloat(INDEX start, int len, float* buf) const override {return source_->getFloat(pindex_ + start, len, buf);}
	const float* getFloatConst(INDEX start, int len, float* buf) const override {
		source_->getFloat(pindex_ + start, len, buf);
		return buf;
	}
	float* getFloatBuffer(INDEX start, int len, float* buf) const override {return buf;}
	bool getDouble(INDEX start, int len, double* buf) const override {return source_->getDouble(pindex_ + start, len, buf);}
	const double* getDoubleConst(INDEX start, int len, double* buf) const override {
		source_->getDouble(pindex_ + start, len, buf);
		return buf;
	}
	double* getDoubleBuffer(INDEX start, int len, double* buf) const override {return buf;}
	bool getString(INDEX start, int len, char** buf) const override {return ((Constant*)source_.get())->getString(pindex_ + start, len, buf);}
	char** getStringConst(INDEX start, int len, char** buf) const override {
		((Constant*)source_.get())->getString(pindex_ + start, len, buf);
		return buf;
	}
	bool getString(INDEX start, int len, DolphinString** buf) const override {return ((Constant*)source_.get())->getString(pindex_ + start, len, buf);}
	DolphinString** getStringConst(INDEX start, int len, DolphinString** buf) const override {
		((Constant*)source_.get())->getString(pindex_ + start, len, buf);
		return buf;
	}
	const int* getSymbolConst(INDEX start, int len, int* buf, SymbolBase* symBase, bool insertIfNotThere) const override {
		source_->getSymbol(pindex_ + start, len, buf, symBase, insertIfNotThere);
		return buf;
	}
	bool getSymbol(INDEX start, int len, int* buf, SymbolBase* symBase, bool insertIfNotThere) const override { return source_->getSymbol(pindex_ + start, len, buf, symBase, insertIfNotThere);}

	bool isNull(INDEX* indices, int len, char* buf) const override;
	bool isValid(INDEX* indices, int len, char* buf) const override;
	bool getBool(INDEX* indices, int len, char* buf) const override;
	bool getChar(INDEX* indices, int len,char* buf) const override;
	bool getShort(INDEX* indices, int len, short* buf) const override;
	bool getInt(INDEX* indices, int len, int* buf) const override;
	bool getLong(INDEX* indices, int len, long long* buf) const override;
	bool getIndex(INDEX* indices, int len, INDEX* buf) const override;
	bool getFloat(INDEX* indices, int len, float* buf) const override;
	bool getDouble(INDEX* indices, int len, double* buf) const override;
	bool getSymbol(INDEX* indices, int len, int* buf, SymbolBase* symBase,bool insertIfNotThere) const override;
	using Vector::getString;
	bool getString(INDEX* indices, int len, DolphinString** buf) const override;
	bool getString(INDEX* indices, int len, char** buf) const override;
	bool getBinary(INDEX* indices, int len, int unitLength, unsigned char* buf) const override;

	bool isNullSafe(INDEX offset, INDEX* indices, int len, char* buf) const override;
	bool isValidSafe(INDEX offset, INDEX* indices, int len, char* buf) const override;
	bool getBoolSafe(INDEX offset, INDEX* indices, int len, char* buf) const override;
	bool getCharSafe(INDEX offset, INDEX* indices, int len,char* buf) const override;
	bool getShortSafe(INDEX offset, INDEX* indices, int len, short* buf) const override;
	bool getIntSafe(INDEX offset, INDEX* indices, int len, int* buf) const override;
	bool getLongSafe(INDEX offset, INDEX* indices, int len, long long* buf) const override;
	bool getIndexSafe(INDEX offset, INDEX* indices, int len, INDEX* buf) const override;
	bool getFloatSafe(INDEX offset, INDEX* indices, int len, float* buf) const override;
	bool getDoubleSafe(INDEX offset, INDEX* indices, int len, double* buf) const override;
	bool getSymbolSafe(INDEX offset, INDEX* indices, int len, int* buf, SymbolBase* symBase,bool insertIfNotThere) const override;
	bool getStringSafe(INDEX offset, INDEX* indices, int len, DolphinString** buf) const override;
	bool getStringSafe(INDEX offset, INDEX* indices, int len, char** buf) const override;
	bool getBinarySafe(INDEX offset, INDEX* indices, int len, int unitLength, unsigned char* buf) const override;

	long long count() const override {return count(0, index_->size());}
	long long count(INDEX start, INDEX length) const override;
	ConstantSP minmax() const override {return minmax(0, size_);}
	ConstantSP minmax(INDEX start, INDEX length) const override;
	/**
	 * @param rightMost If there are multiple maximum/minimum values, choose the last one if `rightMost` is true.
	 */
	INDEX imax(bool rightMost = false) const override { return imax(0, size_, rightMost); }
	INDEX imax(INDEX start, INDEX length, bool rightMost = false) const override;
	INDEX imin(bool rightMost = false) const override { return imin(0, size_, rightMost); }
	INDEX imin(INDEX start, INDEX length, bool rightMost = false) const override;

	ConstantSP max() const override {return max(0, index_->size());}
	ConstantSP max(INDEX start, INDEX length) const override;
	void max(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override;
	ConstantSP min() const override {return min(0, index_->size());}
	ConstantSP min(INDEX start, INDEX length) const override;
	void min(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override;
	ConstantSP avg() const override {return avg(0, index_->size());}
	ConstantSP avg(INDEX start, INDEX length) const override;
	void avg(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override;
	ConstantSP sum() const override {return sum(0, index_->size());}
	ConstantSP sum(INDEX start, INDEX length) const override;
	void sum(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override;
	ConstantSP sum2() const override {return sum2(0, index_->size());}
	ConstantSP sum2(INDEX start, INDEX length) const override;
	void sum2(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override;
	ConstantSP prd() const override {return prd(0, index_->size());}
	ConstantSP prd(INDEX start, INDEX length) const override;
	void prd(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override;
	ConstantSP var() const override {return var(0, index_->size());}
	ConstantSP var(INDEX start, INDEX length) const override;
	void var(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override;
	ConstantSP std() const override {return std(0, index_->size());}
	ConstantSP std(INDEX start, INDEX length) const override;
	void std(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override;
	ConstantSP mode() const override {return mode(0, index_->size());}
	ConstantSP mode(INDEX start, INDEX length) const override;
	void mode(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override;
	ConstantSP median() const override {return median(0, index_->size());}
	ConstantSP median(INDEX start, INDEX length) const override;
	void median(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override;
	ConstantSP searchK(INDEX k) const override {return searchK(0, size_, k);}
	ConstantSP searchK(INDEX start, INDEX length, INDEX k) const override;
	void searchK(INDEX start, INDEX length, INDEX k, const ConstantSP& out, INDEX outputStart = 0) const override;
	ConstantSP firstNot(const ConstantSP& exclude) const override { return firstNot(0, size_, exclude);}
	ConstantSP firstNot(INDEX start, INDEX length, const ConstantSP& exclude) const override;
	void firstNot(INDEX start, INDEX length, const ConstantSP& exclude, const ConstantSP& out, INDEX outputStart = 0) const override;
	ConstantSP lastNot(const ConstantSP& exclude) const override { return lastNot(0, size_, exclude);}
	ConstantSP lastNot(INDEX start, INDEX length, const ConstantSP& exclude) const override;
	void lastNot(INDEX start, INDEX length, const ConstantSP& exclude, const ConstantSP& out, INDEX outputStart = 0) const override;
	void neg() override{throw RuntimeException("Sliced vector doesn't support method neg");}
	void replace(const ConstantSP& oldVal, const ConstantSP& newVal) override{throw RuntimeException("Sliced vector doesn't support method replace");}
	void shuffle() override{throw RuntimeException("Sliced vector doesn't support method shuffle");}
	void reverse() override{throw RuntimeException("Sliced vector doesn't support method reverse");}
	void reverse(INDEX start, INDEX length) override{throw RuntimeException("Sliced vector doesn't support method reverse");}
	virtual bool rank(bool sorted, INDEX* indices, INDEX* ranking){throw RuntimeException("Sliced vector doesn't support method rank");}
	void find(INDEX start, INDEX length, const ConstantSP& target, const ConstantSP& resultSP) override;
	void binarySearch(INDEX start, INDEX length, const ConstantSP& target, const ConstantSP& resultSP) override;
	void asof(INDEX start, INDEX length, const ConstantSP& target, const ConstantSP& resultSP) override;
	bool findDuplicatedElements(Vector*  indices, INDEX start, INDEX length, vector<pair<INDEX,INDEX> >& duplicates) override{
		throw RuntimeException("Sliced vector doesn't support method findDuplicatedElements");
	}
	bool findDuplicatedElements(INDEX start, INDEX length, vector<pair<INDEX,INDEX> >& duplicates) override{
		throw RuntimeException("Sliced vector doesn't support method findDuplicatedElements");
	}
	bool findUniqueElements(INDEX start, INDEX length, vector<pair<INDEX,INDEX> >& uniques) override{
		throw RuntimeException("Sliced vector doesn't support method findUniqueElements");
	}
	bool findRange(INDEX* ascIndices,const ConstantSP& target,INDEX* targetIndices,vector<pair<INDEX,INDEX> >& ranges) override{
		throw RuntimeException("Sliced vector doesn't support method findRange");
	}
	bool findRange(const ConstantSP& target,INDEX* targetIndices,vector<pair<INDEX,INDEX> >& ranges) override{
		throw RuntimeException("Sliced vector doesn't support method findRange");
	}
	INDEX lowerBound(INDEX start, const ConstantSP& target) override;
	long long getAllocatedMemory() const override {return sizeof(SlicedVector);}
	int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int& numElement, int& partial) const override {
		throw RuntimeException("Sliced vector doesn't support method serialize");
	}
	int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int targetNumElement, int& numElement, int& partial) const override {
		throw RuntimeException("Sliced vector doesn't support method serialize");
	}
	IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const override {throw RuntimeException("Sliced vector doesn't support method serialize");}
	bool isSorted(INDEX start, INDEX length, bool asc, bool strict, char nullsOrder = 0) const override {throw RuntimeException("Sliced vector doesn't support method isSorted");}
	using Vector::topK;
	virtual ConstantSP topK(INDEX start, INDEX length, INDEX top, bool asc, bool extendEqualValue) const {throw RuntimeException("Sliced vector doesn't support method topK");}
	bool sort(bool asc, char nullsOrder = 0) override {throw RuntimeException("Sliced vector doesn't support method sort");}
	bool sort(bool asc, Vector* indices, char nullsOrder = 0) override {throw RuntimeException("Sliced vector doesn't support method sort");}
	bool sortSelectedIndices(Vector* indices, INDEX start, INDEX length, bool asc, char nullsOrder = 0) override {
		throw RuntimeException("Sliced vector doesn't support method sortSelectedIndices");
	}

private:
	VectorSP source_;
	VectorSP index_;
	INDEX* pindex_;
	INDEX size_;
};

class SubVector : public Vector {
public:
	SubVector(const VectorSP& target, INDEX offset, INDEX length, bool updatable = false);
	void reset(const VectorSP& target, INDEX offset, INDEX length);
	void reset(INDEX offset, INDEX length);
	~SubVector() override{}
	bool isLargeConstant() const override {return true;}
	VECTOR_TYPE getVectorType() const override {return VECTOR_TYPE::SUBVECTOR;}
	VectorSP getSourceVector() const { return source_;}
	INDEX getSubVectorStart() const { return offset_;}
	INDEX getSubVectorLength() const { return size_;}
	int getSegmentSizeInBit() const override { return source_->getSegmentSizeInBit(); }
	bool copyable() const override {return true;}
	bool isView() const override {return true;}
	DATA_TYPE getRawType() const override {return source_->getRawType();}
	int getExtraParamForType() const override { return source_->getExtraParamForType();}
	bool isTableColumn() const override { return source_->isTableColumn();}
	bool isColumnarTuple() const override { return source_->isColumnarTuple();}
	SymbolBaseSP getSymbolBase() const override {return source_->getSymbolBase();}
	virtual DATA_FORM getForm() const { return DF_VECTOR;}
	ConstantSP getInstance() const override {return getInstance(size_);}
	ConstantSP getInstance(INDEX size) const override;
	ConstantSP getValue() const override { return source_->getSubVector(offset_, size_);}
	ConstantSP getValue(INDEX capacity) const override { return source_->getSubVector(offset_, size_, capacity);}
	ConstantSP get(INDEX column, INDEX rowStart,INDEX rowEnd) const override {return source_->getSubVector(offset_ + rowStart,rowEnd-rowStart);}
	ConstantSP getWindow(INDEX colStart, int colLength, INDEX rowStart, int rowLength) const override;
	ConstantSP getSlice(const ConstantSP& rowIndex, const ConstantSP& colIndex) const override;
	ConstantSP getSubVector(INDEX start, INDEX length, INDEX capacity) const override;
	ConstantSP getSubVector(INDEX start, INDEX length) const override {
		return getSubVector(start, length, std::abs(length));
	}
	void fill(INDEX start, INDEX length, const ConstantSP& value, INDEX valueOffset = 0) override{
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method fill");
		else
			source_->fill(offset_ + start, length, value, valueOffset);
	}
	bool validIndex(INDEX uplimit) override{return source_->validIndex(offset_, size_, uplimit);}
	bool validIndex(INDEX start, INDEX length, INDEX uplimit) override{return source_->validIndex(start + offset_, length, uplimit);}
	int compare(INDEX index, const ConstantSP& target) const override {return source_->compare(offset_ + index, target);}
	bool getNullFlag() const override {
		if (offset_ >= 0 && offset_ + size_ <= source_->size())
			return source_->getNullFlag();
		return true;
	}
	void setNullFlag(bool containNull) override{}
	bool hasNull() override{
		if (offset_ >= 0 && offset_ + size_ <= source_->size())
			return source_->hasNull(offset_, size_);
		return true;
	}
	bool hasNull(INDEX start, INDEX length) override{
		if (start + offset_ >= 0 && start + offset_ + length <= source_->size())
			return source_->hasNull(start + offset_, length);
		return true;
	}
	INDEX getCapacity() const override {return size_;}
	bool isFastMode() const override { return source_->isFastMode();}
	void* getDataArray() const override;
	bool isIndexArray() const override { return source_->isIndexArray();}
	INDEX* getIndexArray() const override { return source_->isIndexArray() ? source_->getIndexArray() + offset_ : NULL;}
	short getUnitLength() const override {return source_->getUnitLength();}
	void** getDataSegment() const override;
	bool sizeable() const override {return source_->sizeable();}
	char getBool() const override {return source_->getBool(offset_);}
	char getChar() const override { return source_->getChar(offset_);}
	short getShort() const override { return source_->getShort(offset_);}
	int getInt() const override {return source_->getInt(offset_);}
	long long getLong() const override {return source_->getLong(offset_);}
	INDEX getIndex() const override {return source_->getIndex(offset_);}
	float getFloat() const override {return source_->getFloat(offset_);}
	double getDouble() const override {return source_->getDouble(offset_);}
	void contain(const ConstantSP& target, const ConstantSP& resultSP) const override;

	ConstantSP get(INDEX index) const override;

	ConstantSP get(const ConstantSP& index) const override;
	ConstantSP get(INDEX offset, const ConstantSP& index) const override;
	using Vector::setBool;
	virtual void setBool(bool val){throw RuntimeException("Immutable sub vector doesn't support method setBool");}
	void setChar(char val) override{throw RuntimeException("Immutable sub vector doesn't support method setChar");}
	void setShort(short val) override{throw RuntimeException("Immutable sub vector doesn't support method setShort");}
	void setInt(int val) override{throw RuntimeException("Immutable sub vector doesn't support method setInt");}
	void setLong(long long val) override{throw RuntimeException("Immutable sub vector doesn't support method setLong");}
	void setIndex(INDEX val) override{throw RuntimeException("Immutable sub vector doesn't support method setIndex");}
	void setFloat(float val) override{throw RuntimeException("Immutable sub vector doesn't support method setFloat");}
	void setDouble(double val) override{throw RuntimeException("Immutable sub vector doesn't support method setDouble");}
	using Vector::setString;
	virtual void setString(const string& val){throw RuntimeException("Immutable sub vector doesn't support method setString");}
	void setNull() override{throw RuntimeException("Immutable sub vector doesn't support method setNull");}
	char getBool(INDEX index) const override {
		if (index < 0 || index >= size_ || offset_ + index < 0 || offset_ + index >= source_->size()) {
			return CHAR_MIN;
		}
		return source_->getBool(offset_ + index);
	}
	char getChar(INDEX index) const override {
		if (index < 0 || index >= size_ || offset_ + index < 0 || offset_ + index >= source_->size()) {
			return CHAR_MIN;
		}
		return source_->getChar(offset_ + index);
	}
	short getShort(INDEX index) const override {
		if (index < 0 || index >= size_ || offset_ + index < 0 || offset_ + index >= source_->size()) {
			return SHRT_MIN;
		}
		return source_->getShort(offset_ + index);
	}
	int getInt(INDEX index) const override {
		if (index < 0 || index >= size_ || offset_ + index < 0 || offset_ + index >= source_->size()) {
			return INT_MIN;
		}
		return source_->getInt(offset_ + index);
	}
	long long getLong(INDEX index) const override {
		if (index < 0 || index >= size_ || offset_ + index < 0 || offset_ + index >= source_->size()) {
			return LONG_MIN;
		}
		return source_->getLong(offset_ + index);
	}
	INDEX getIndex(INDEX index) const override {
		if (index < 0 || index >= size_ || offset_ + index < 0 || offset_ + index >= source_->size()) {
			return INDEX_MIN;
		}
		return source_->getIndex(offset_ + index);
	}
	float getFloat(INDEX index) const override {
		if (index < 0 || index >= size_ || offset_ + index < 0 || offset_ + index >= source_->size()) {
			return FLT_NMIN;
		}
		return source_->getFloat(offset_ + index);
	}
	double getDouble(INDEX index) const override {
		if (index < 0 || index >= size_ || offset_ + index < 0 || offset_ + index >= source_->size()) {
			return DBL_NMIN;
		}
		return source_->getDouble(offset_ + index);
	}
	bool isNull(INDEX index) const override {
		if (index < 0 || index >= size_ || offset_ + index < 0 || offset_ + index >= source_->size()) {
			return true;
		}
		return source_->isNull(offset_ + index);
	}
	string getString(INDEX index) const override {
		if (index < 0 || index >= size_ || offset_ + index < 0 || offset_ + index >= source_->size()) {
			return "";
		}
		return source_->getString(offset_ + index);
	}
	const DolphinString& getStringRef(INDEX index) const override {
		if (index < 0 || index >= size_ || offset_ + index < 0 || offset_ + index >= source_->size()) {
			throw RuntimeException("Index out of range.");
		}
		return source_->getStringRef(offset_ + index);
	}
	void setBool(INDEX index,char val) override {
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setBool");
		else
			source_->setBool(offset_ + index, val);
	}
	void setChar(INDEX index,char val) override{
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setChar");
		else
			source_->setChar(offset_ + index, val);
	}
	void setShort(INDEX index,short val) override{
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setShort");
		else
			source_->setShort(offset_ + index, val);
	}
	void setInt(INDEX index,int val) override{
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setInt");
		else
			source_->setInt(offset_ + index, val);
	}
	void setLong(INDEX index,long long val) override{
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setLong");
		else
			source_->setLong(offset_ + index, val);
	}
	void setIndex(INDEX index, INDEX val) override{
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setIndex");
		else
			source_->setIndex(offset_ + index, val);
	}
	void setFloat(INDEX index,float val) override{
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setFloat");
		else
			source_->setFloat(offset_ + index, val);
	}
	void setDouble(INDEX index, double val) override{
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setDouble");
		else
			source_->setDouble(offset_ + index, val);
	}
	void setString(INDEX index, const DolphinString& val) override{
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setString");
		else
			source_->setString(offset_ + index, val);
	}
	void setBinary(INDEX index, int unitLength, const unsigned char* val) override{
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setBinary");
		else
			source_->setBinary(offset_ + index, unitLength, val);
	}
	void setNull(INDEX index) override{
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setNull");
		else
			source_->setNull(offset_ + index);
	}
	void clear() override{throw RuntimeException("Immutable sub vector doesn't support method clear");}
	bool remove(INDEX count) override{throw RuntimeException("Immutable sub vector doesn't support method remove");}
	bool remove(const ConstantSP& index) override{throw RuntimeException("Immutable sub vector doesn't support method remove");}
	void next(INDEX steps) override{throw RuntimeException("Immutable sub vector doesn't support method next");}
	void prev(INDEX steps) override{throw RuntimeException("Immutable sub vector doesn't support method prev");}
	INDEX size() const override {
		if(source_->size() < offset_ + size_ && source_->size() == 0)
			throw RuntimeException("The source vector has been shortened and the sub vector is not valid any more.");
		return size_;
	}
	void nullFill(const ConstantSP& val) override{throw RuntimeException("Immutable sub vector doesn't support method nullFill");}
	bool isNull(INDEX start, int len, char* buf) const override {
		if (start < 0 || offset_ + start < 0) {
			INDEX cur = 0;
			if (start < 0) {
				cur = std::max(cur, std::abs(start));
			}
			if (offset_ + start < 0) {
				cur = std::max(cur, std::abs(offset_ + start));
			}
			cur = std::min(len, cur);
			for (INDEX i = 0; i < cur; ++i) {
				buf[i] = true;
			}
			start += cur;
			len -= cur;
			buf += cur;
		}
		int validLen = std::max(0, std::min(len, std::min(size_ - start, source_->size() - offset_ - start)));
		if(offset_ + start < source_->size() && validLen > 0 && !source_->isNull(offset_ + start, validLen, buf)) {
			return false;
		}
		if (validLen < len) {
			for (int i = validLen; i < len; ++i) {
				buf[i] = true;
			}
		}
		return true;
	}
	bool isValid(INDEX start, int len, char* buf) const override {
		if (start < 0 || offset_ + start < 0) {
			INDEX cur = 0;
			if (start < 0) {
				cur = std::max(cur, std::abs(start));
			}
			if (offset_ + start < 0) {
				cur = std::max(cur, std::abs(offset_ + start));
			}
			cur = std::min(len, cur);
			for (INDEX i = 0; i < cur; ++i) {
				buf[i] = false;
			}
			start += cur;
			len -= cur;
			buf += cur;
		}
		int validLen = std::max(0, std::min(len, std::min(size_ - start, source_->size() - offset_ - start)));
		if(offset_ + start < source_->size() && validLen > 0 && !source_->isValid(offset_ + start, validLen, buf)) {
			return false;
		}
		if (validLen < len) {
			for (int i = validLen; i < len; ++i) {
				buf[i] = false;
			}
		}
		return true;
	}

	using Vector::getString;
	bool getString(INDEX* indices, int len, DolphinString** buf) const override;
	bool getString(INDEX* indices, int len, char** buf) const override;
    bool getBool(INDEX *indices, int len, char *buf) const override;
    bool getChar(INDEX *indices, int len, char *buf) const override;
    bool getShort(INDEX *indices, int len, short *buf) const override;
    bool getInt(INDEX *indices, int len, int *buf) const override;
    bool getIndex(INDEX *indices, int len, INDEX *buf) const override;
    bool getLong(INDEX *indices, int len, long long *buf) const override;
    bool getFloat(INDEX *indices, int len, float *buf) const override;
    bool getDouble(INDEX *indices, int len, double *buf) const override;
    bool getDecimal32(INDEX *indices, int len, int scale, int *buf) const override;

	bool getBool(INDEX start, int len, char* buf) const override {
		if (start < 0 || offset_ + start < 0) {
			INDEX cur = 0;
			if (start < 0) {
				cur = std::max(cur, std::abs(start));
			}
			if (offset_ + start < 0) {
				cur = std::max(cur, std::abs(offset_ + start));
			}
			cur = std::min(len, cur);
			for (INDEX i = 0; i < cur; ++i) {
				buf[i] = CHAR_MIN;
			}
			start += cur;
			len -= cur;
			buf += cur;
		}
		int validLen = std::max(0, std::min(len, std::min(size_ - start, source_->size() - offset_ - start)));
		if(offset_ + start < source_->size() && validLen > 0 && !source_->getBool(offset_ + start, validLen, buf)) {
			return false;
		}
		if (validLen < len) {
			for (int i = validLen; i < len; ++i) {
				buf[i] = CHAR_MIN;
			}
		}
		return true;
	}

	const char* getBoolConst(INDEX start, int len, char* buf) const override {
		if (start >= 0 && start + len <= size_ && offset_ + start >= 0 && offset_ + start + len <= source_->size()) {
			return source_->getBoolConst(offset_ + start, len, buf);
		}
		char* originalBuf = buf;
		if (start < 0 || offset_ + start < 0) {
			INDEX cur = 0;
			if (start < 0) {
				cur = std::max(cur, std::abs(start));
			}
			if (offset_ + start < 0) {
				cur = std::max(cur, std::abs(offset_ + start));
			}
			cur = std::min(len, cur);
			for (INDEX i = 0; i < cur; ++i) {
				buf[i] = CHAR_MIN;
			}
			start += cur;
			len -= cur;
			buf += cur;
		}
		int validLen = std::max(0, std::min(len, std::min(size_ - start, source_->size() - offset_ - start)));
		if (offset_ + start < source_->size() && validLen > 0)
			source_->getBool(offset_ + start, validLen, buf);
		if (validLen < len) {
			for (int i = validLen; i < len; ++i) {
				buf[i] = CHAR_MIN;
			}
		}
		return originalBuf;
	}
	char* getBoolBuffer(INDEX start, int len, char* buf) const override {return source_->getBoolBuffer(offset_ + start, len, buf);}
	bool getChar(INDEX start, int len, char* buf) const override {
		if (start < 0 || offset_ + start < 0) {
			INDEX cur = 0;
			if (start < 0) {
				cur = std::max(cur, std::abs(start));
			}
			if (offset_ + start < 0) {
				cur = std::max(cur, std::abs(offset_ + start));
			}
			cur = std::min(len, cur);
			for (INDEX i = 0; i < cur; ++i) {
				buf[i] = CHAR_MIN;
			}
			start += cur;
			len -= cur;
			buf += cur;
		}
		int validLen = std::max(0, std::min(len, std::min(size_ - start, source_->size() - offset_ - start)));
		if(offset_ + start < source_->size() && validLen > 0 && !source_->getChar(offset_ + start, validLen, buf)) {
			return false;
		}
		if (validLen < len) {
			for (int i = validLen; i < len; ++i) {
				buf[i] = CHAR_MIN;
			}
		}
		return true;
	}
	const char* getCharConst(INDEX start, int len, char* buf) const override {
		if (start >= 0 && start + len <= size_ && offset_ + start >= 0 && offset_ + start + len <= source_->size()) {
			return source_->getCharConst(offset_ + start, len, buf);
		}
		char* originalBuf = buf;
		if (start < 0 || offset_ + start < 0) {
			INDEX cur = 0;
			if (start < 0) {
				cur = std::max(cur, std::abs(start));
			}
			if (offset_ + start < 0) {
				cur = std::max(cur, std::abs(offset_ + start));
			}
			cur = std::min(len, cur);
			for (INDEX i = 0; i < cur; ++i) {
				buf[i] = CHAR_MIN;
			}
			start += cur;
			len -= cur;
			buf += cur;
		}
		int validLen = std::max(0, std::min(len, std::min(size_ - start, source_->size() - offset_ - start)));
		if (offset_ + start < source_->size() && validLen > 0)
			source_->getChar(offset_ + start, validLen, buf);
		if (validLen < len) {
			for (int i = validLen; i < len; ++i) {
				buf[i] = CHAR_MIN;
			}
		}
		return originalBuf;
	
	}
	char* getCharBuffer(INDEX start, int len, char* buf) const override {return source_->getCharBuffer(offset_ + start, len, buf);}
	bool getShort(INDEX start, int len, short* buf) const override {
		if (start < 0 || offset_ + start < 0) {
			INDEX cur = 0;
			if (start < 0) {
				cur = std::max(cur, std::abs(start));
			}
			if (offset_ + start < 0) {
				cur = std::max(cur, std::abs(offset_ + start));
			}
			cur = std::min(len, cur);
			for (INDEX i = 0; i < cur; ++i) {
				buf[i] = SHRT_MIN;
			}
			start += cur;
			len -= cur;
			buf += cur;
		}
		int validLen = std::max(0, std::min(len, std::min(size_ - start, source_->size() - offset_ - start)));
		if(offset_ + start < source_->size() && validLen > 0 && !source_->getShort(offset_ + start, validLen, buf)) {
			return false;
		}
		if (validLen < len) {
			for (int i = validLen; i < len; ++i) {
				buf[i] = SHRT_MIN;
			}
		}
		return true;
	}
	const short* getShortConst(INDEX start, int len, short* buf) const override {
		if (start >= 0 && start + len <= size_ && offset_ + start >= 0 && offset_ + start + len <= source_->size()) {
			return source_->getShortConst(offset_ + start, len, buf);
		}
		short* originalBuf = buf;
		if (start < 0 || offset_ + start < 0) {
			INDEX cur = 0;
			if (start < 0) {
				cur = std::max(cur, std::abs(start));
			}
			if (offset_ + start < 0) {
				cur = std::max(cur, std::abs(offset_ + start));
			}
			cur = std::min(len, cur);
			for (INDEX i = 0; i < cur; ++i) {
				buf[i] = SHRT_MIN;
			}
			start += cur;
			len -= cur;
			buf += cur;
		}
		int validLen = std::max(0, std::min(len, std::min(size_ - start, source_->size() - offset_ - start)));
		if (offset_ + start < source_->size() && validLen > 0)
			source_->getShort(offset_ + start, validLen, buf);
		if (validLen < len) {
			for (int i = validLen; i < len; ++i) {
				buf[i] = SHRT_MIN;
			}
		}
		return originalBuf;
	}
	short* getShortBuffer(INDEX start, int len, short* buf) const override {return source_->getShortBuffer(offset_ + start, len, buf);}
	bool getInt(INDEX start, int len, int* buf) const override {
		if (start < 0 || offset_ + start < 0) {
			INDEX cur = 0;
			if (start < 0) {
				cur = std::max(cur, std::abs(start));
			}
			if (offset_ + start < 0) {
				cur = std::max(cur, std::abs(offset_ + start));
			}
			cur = std::min(len, cur);
			for (INDEX i = 0; i < cur; ++i) {
				buf[i] = INT_MIN;
			}
			start += cur;
			len -= cur;
			buf += cur;
		}
		int validLen = std::max(0, std::min(len, std::min(size_ - start, source_->size() - offset_ - start)));
		if(offset_ + start < source_->size() && validLen > 0 && !source_->getInt(offset_ + start, validLen, buf)) {
			return false;
		}
		if (validLen < len) {
			for (int i = validLen; i < len; ++i) {
				buf[i] = INT_MIN;
			}
		}
		return true;
	}
	const int* getIntConst(INDEX start, int len, int* buf) const override {
		if (start >= 0 && start + len <= size_ && offset_ + start >= 0 && offset_ + start + len <= source_->size()) {
			return source_->getIntConst(offset_ + start, len, buf);
		}
		int* originalBuf = buf;
		if (start < 0 || offset_ + start < 0) {
			INDEX cur = 0;
			if (start < 0) {
				cur = std::max(cur, std::abs(start));
			}
			if (offset_ + start < 0) {
				cur = std::max(cur, std::abs(offset_ + start));
			}
			cur = std::min(len, cur);

			for (INDEX i = 0; i < cur; ++i) {
				buf[i] = INT_MIN;
			}
			start += cur;
			len -= cur;
			buf += cur;
		}
		int validLen = std::max(0, std::min(len, std::min(size_ - start, source_->size() - offset_ - start)));
		if (offset_ + start < source_->size() && validLen > 0)
			source_->getInt(offset_ + start, validLen, buf);
		if (validLen < len) {
			for (int i = validLen; i < len; ++i) {
				buf[i] = INT_MIN;
			}
		}
		return originalBuf;
	}
	int* getIntBuffer(INDEX start, int len, int* buf) const override {return source_->getIntBuffer(offset_ + start, len, buf);}
	bool getLong(INDEX start, int len, long long* buf) const override {
		if (start < 0 || offset_ + start < 0) {
			INDEX cur = 0;
			if (start < 0) {
				cur = std::max(cur, std::abs(start));
			}
			if (offset_ + start < 0) {
				cur = std::max(cur, std::abs(offset_ + start));
			}
			cur = std::min(len, cur);
			for (INDEX i = 0; i < cur; ++i) {
				buf[i] = LONG_MIN;
			}
			start += cur;
			len -= cur;
			buf += cur;
		}
		int validLen = std::max(0, std::min(len, std::min(size_ - start, source_->size() - offset_ - start)));
		if(offset_ + start < source_->size() && validLen > 0 && !source_->getLong(offset_ + start, validLen, buf)) {
			return false;
		}
		if (validLen < len) {
			for (int i = validLen; i < len; ++i) {
				buf[i] = LONG_MIN;
			}
		}
		return true;
	}
	const long long* getLongConst(INDEX start, int len, long long* buf) const override {
		if (start >= 0 && start + len <= size_ && offset_ + start >= 0 && offset_ + start + len <= source_->size()) {
			return source_->getLongConst(offset_ + start, len, buf);
		}
		long long* originalBuf = buf;
		if (start < 0 || offset_ + start < 0) {
			INDEX cur = 0;
			if (start < 0) {
				cur = std::max(cur, std::abs(start));
			}
			if (offset_ + start < 0) {
				cur = std::max(cur, std::abs(offset_ + start));
			}
			cur = std::min(len, cur);
			for (INDEX i = 0; i < cur; ++i) {
				buf[i] = LONG_MIN;
			}
			start += cur;
			len -= cur;
			buf += cur;
		}
		int validLen = std::max(0, std::min(len, std::min(size_ - start, source_->size() - offset_ - start)));
		if (offset_ + start < source_->size() && validLen > 0)
			source_->getLong(offset_ + start, validLen, buf);
		if (validLen < len) {
			for (int i = validLen; i < len; ++i) {
				buf[i] = LONG_MIN;
			}
		}
		return originalBuf;
	}
	long long* getLongBuffer(INDEX start, int len, long long* buf) const override {return source_->getLongBuffer(offset_ + start, len, buf);}
	bool getBinary(INDEX start, int len, int unitLength, unsigned char* buf) const override {
		if (start < 0 || offset_ + start < 0) {
			INDEX cur = 0;
			if (start < 0) {
				cur = std::max(cur, std::abs(start));
			}
			if (offset_ + start < 0) {
				cur = std::max(cur, std::abs(offset_ + start));
			}
			cur = std::min(len, cur);
			if (source_->getCategory() == DENARY) {
				fillWithNullForDecimal(buf, cur, unitLength);
			} else {
				memset(buf, 0, cur * unitLength);
			}
			start += cur;
			len -= cur;
			buf += cur * unitLength;
		}
		int validLen = std::max(0, std::min(len, std::min(size_ - start, source_->size() - offset_ - start)));
		if(offset_ + start < source_->size() && validLen > 0 && !source_->getBinary(offset_ + start, validLen, unitLength, buf)) {
			return false;
		}
		if (validLen < len) {
			if (source_->getCategory() == DENARY) {
				fillWithNullForDecimal(buf + unitLength * validLen, len - validLen, unitLength);
			} else {
				memset(buf + unitLength * validLen, 0, (len - validLen) * unitLength);
			}
		}
		return true;
	}
	const unsigned char* getBinaryConst(INDEX start, int len, int unitLength, unsigned char* buf) const override {
		if (start >= 0 && start + len <= size_ && offset_ + start >= 0 && offset_ + start + len <= source_->size()) {
			return source_->getBinaryConst(offset_ + start, len, unitLength, buf);
		}
		unsigned char* originalBuf = buf;
		if (start < 0 || offset_ + start < 0) {
			INDEX cur = 0;
			if (start < 0) {
				cur = std::max(cur, std::abs(start));
			}
			if (offset_ + start < 0) {
				cur = std::max(cur, std::abs(offset_ + start));
			}
			cur = std::min(len, cur);
			if (source_->getCategory() == DENARY) {
				fillWithNullForDecimal(buf, cur, unitLength);
			} else {
				memset(buf, 0, cur * unitLength);
			}
			start += cur;
			len -= cur;
			buf += cur * unitLength;
		}
		int validLen = std::max(0, std::min(len, std::min(size_ - start, source_->size() - offset_ - start)));
		if (offset_ + start < source_->size() && validLen > 0)
			source_->getBinary(offset_ + start, validLen, unitLength, buf);
		if (validLen < len) {
			if (source_->getCategory() == DENARY) {
				fillWithNullForDecimal(buf + unitLength * validLen, len - validLen, unitLength);
			} else {
				memset(buf + unitLength * validLen, 0, (len - validLen) * unitLength);
			}
		}
		return originalBuf;
	}
	unsigned char* getBinaryBuffer(INDEX start, int len, int unitLength, unsigned char* buf) const override {return source_->getBinaryBuffer(offset_ + start, len, unitLength, buf);}
	bool getIndex(INDEX start, int len, INDEX* buf) const override {
		if (start < 0 || offset_ + start < 0) {
			INDEX cur = 0;
			if (start < 0) {
				cur = std::max(cur, std::abs(start));
			}
			if (offset_ + start < 0) {
				cur = std::max(cur, std::abs(offset_ + start));
			}
			cur = std::min(len, cur);
			for (INDEX i = 0; i < cur; ++i) {
				buf[i] = INDEX_MIN;
			}
			start += cur;
			len -= cur;
			buf += cur;
		}
		int validLen = std::max(0, std::min(len, std::min(size_ - start, source_->size() - offset_ - start)));
		if(offset_ + start < source_->size() && validLen > 0 && !source_->getIndex(offset_ + start, validLen, buf)) {
			return false;
		}
		if (validLen < len) {
			for (int i = validLen; i < len; ++i) {
				buf[i] = INDEX_MIN;
			}
		}
		return true;
	}
	const INDEX* getIndexConst(INDEX start, int len, INDEX* buf) const override {
		if (start >= 0 && start + len <= size_ && offset_ + start >= 0 && offset_ + start + len <= source_->size()) {
			return source_->getIndexConst(offset_ + start, len, buf);
		}
		INDEX* originalBuf = buf;
		if (start < 0 || offset_ + start < 0) {
			INDEX cur = 0;
			if (start < 0) {
				cur = std::max(cur, std::abs(start));
			}
			if (offset_ + start < 0) {
				cur = std::max(cur, std::abs(offset_ + start));
			}
			cur = std::min(len, cur);
			for (INDEX i = 0; i < cur; ++i) {
				buf[i] = INDEX_MIN;
			}
			start += cur;
			len -= cur;
			buf += cur;
		}
		int validLen = std::max(0, std::min(len, std::min(size_ - start, source_->size() - offset_ - start)));
		if (offset_ + start < source_->size() && validLen > 0)
			source_->getIndex(offset_ + start, validLen, buf);
		if (validLen < len) {
			for (int i = validLen; i < len; ++i) {
				buf[i] = INDEX_MIN;
			}
		}
		return originalBuf;	
	}
	INDEX* getIndexBuffer(INDEX start, int len, INDEX* buf) const override {return source_->getIndexBuffer(offset_ + start, len, buf);}
	bool getFloat(INDEX start, int len, float* buf) const override {
		if (start < 0 || offset_ + start < 0) {
			INDEX cur = 0;
			if (start < 0) {
				cur = std::max(cur, std::abs(start));
			}
			if (offset_ + start < 0) {
				cur = std::max(cur, std::abs(offset_ + start));
			}
			cur = std::min(len, cur);
			for (INDEX i = 0; i < cur; ++i) {
				buf[i] = FLT_NMIN;
			}
			start += cur;
			len -= cur;
			buf += cur;
		}
		int validLen = std::max(0, std::min(len, std::min(size_ - start, source_->size() - offset_ - start)));
		if(offset_ + start < source_->size() && validLen > 0 && !source_->getFloat(offset_ + start, validLen, buf)) {
			return false;
		}
		if (validLen < len) {
			for (int i = validLen; i < len; ++i) {
				buf[i] = FLT_NMIN;
			}
		}
		return true;
	}
	const float* getFloatConst(INDEX start, int len, float* buf) const override {
		if (start >= 0 && start + len <= size_ && offset_ + start >= 0 && offset_ + start + len <= source_->size()) {
			return source_->getFloatConst(offset_ + start, len, buf);
		}
		float* originalBuf = buf;
		if (start < 0 || offset_ + start < 0) {
			INDEX cur = 0;
			if (start < 0) {
				cur = std::max(cur, std::abs(start));
			}
			if (offset_ + start < 0) {
				cur = std::max(cur, std::abs(offset_ + start));
			}
			cur = std::min(len, cur);
			for (INDEX i = 0; i < cur; ++i) {
				buf[i] = FLT_NMIN;
			}
			start += cur;
			len -= cur;
			buf += cur;
		}
		int validLen = std::max(0, std::min(len, std::min(size_ - start, source_->size() - offset_ - start)));
		if (offset_ + start < source_->size() && validLen > 0)
			source_->getFloat(offset_ + start, validLen, buf);
		if (validLen < len) {
			for (int i = validLen; i < len; ++i) {
				buf[i] = FLT_NMIN;
			}
		}
		return originalBuf;
	}
	float* getFloatBuffer(INDEX start, int len, float* buf) const override {return source_->getFloatBuffer(offset_ + start, len, buf);}
	bool getDouble(INDEX start, int len, double* buf) const override {
		if (start < 0 || offset_ + start < 0) {
			INDEX cur = 0;
			if (start < 0) {
				cur = std::max(cur, std::abs(start));
			}
			if (offset_ + start < 0) {
				cur = std::max(cur, std::abs(offset_ + start));
			}
			cur = std::min(len, cur);
			for (INDEX i = 0; i < cur; ++i) {
				buf[i] = DBL_NMIN;
			}
			start += cur;
			len -= cur;
			buf += cur;
		}
		int validLen = std::max(0, std::min(len, std::min(size_ - start, source_->size() - offset_ - start)));
		if(offset_ + start < source_->size() && validLen > 0 && !source_->getDouble(offset_ + start, validLen, buf)) {
			return false;
		}
		if (validLen < len) {
			for (int i = validLen; i < len; ++i) {
				buf[i] = DBL_NMIN;
			}
		}
		return true;
	}
	const double* getDoubleConst(INDEX start, int len, double* buf) const override {
		if (start >= 0 && start + len <= size_ && offset_ + start >= 0 && offset_ + start + len <= source_->size()) {
			return source_->getDoubleConst(offset_ + start, len, buf);
		}
		double* originalBuf = buf;
		if (start < 0 || offset_ + start < 0) {
			INDEX cur = 0;
			if (start < 0) {
				cur = std::max(cur, std::abs(start));
			}
			if (offset_ + start < 0) {
				cur = std::max(cur, std::abs(offset_ + start));
			}
			cur = std::min(len, cur);
			for (INDEX i = 0; i < cur; ++i) {
				buf[i] = DBL_NMIN;
			}
			start += cur;
			len -= cur;
			buf += cur;
		}
		int validLen = std::max(0, std::min(len, std::min(size_ - start, source_->size() - offset_ - start)));
		if (offset_ + start < source_->size() && validLen > 0) {
			source_->getDouble(offset_ + start, validLen, buf);
		}
		if (validLen < len) {
			for (int i = validLen; i < len; ++i) {
				buf[i] = DBL_NMIN;
			}
		}
		return originalBuf;
	}
	double* getDoubleBuffer(INDEX start, int len, double* buf) const override {return source_->getDoubleBuffer(offset_ + start, len, buf);}
	bool getString(INDEX start, int len, char** buf) const override {
		if (start < 0 || offset_ + start < 0) {
			INDEX cur = 0;
			if (start < 0) {
				cur = std::max(cur, std::abs(start));
			}
			if (offset_ + start < 0) {
				cur = std::max(cur, std::abs(offset_ + start));
			}
			cur = std::min(len, cur);
			for (INDEX i = 0; i < cur; ++i) {
				buf[i] = (char*)Constant::EMPTY.c_str();
			}
			start += cur;
			len -= cur;
			buf += cur;
		}
		int validLen = std::max(0, std::min(len, std::min(size_ - start, source_->size() - offset_ - start)));
		if(offset_ + start < source_->size() && validLen > 0 && !((Constant*)source_.get())->getString(offset_ + start, validLen, buf)) {
			return false;
		}
		if (validLen < len) {
			for (int i = validLen; i < len; ++i) {
				buf[i] = (char*)Constant::EMPTY.c_str();
			}
		}
		return true;
	}
	char** getStringConst(INDEX start, int len, char** buf) const override {
		if (start >= 0 && start + len <= size_ && offset_ + start >= 0 && offset_ + start + len <= source_->size()) {
			return source_->getStringConst(offset_ + start, len, buf);
		}
		char** originalBuf = buf;
		if (start < 0 || offset_ + start < 0) {
			INDEX cur = 0;
			if (start < 0) {
				cur = std::max(cur, std::abs(start));
			}
			if (offset_ + start < 0) {
				cur = std::max(cur, std::abs(offset_ + start));
			}
			cur = std::min(len, cur);
			for (INDEX i = 0; i < cur; ++i) {
				buf[i] = (char*)Constant::EMPTY.c_str();
			}
			start += cur;
			len -= cur;
			buf += cur;
		}
		int validLen = std::max(0, std::min(len, std::min(size_ - start, source_->size() - offset_ - start)));
		if (offset_ + start < source_->size() && validLen > 0)
			((Constant*)source_.get())->getString(offset_ + start, validLen, buf);
		if (validLen < len) {
			for (int i = validLen; i < len; ++i) {
				buf[i] = (char*)Constant::EMPTY.c_str();
			}
		}
		return originalBuf;
	}
	bool getString(INDEX start, int len, DolphinString** buf) const override {
		if (start < 0 || offset_ + start < 0) {
			INDEX cur = 0;
			if (start < 0) {
				cur = std::max(cur, std::abs(start));
			}
			if (offset_ + start < 0) {
				cur = std::max(cur, std::abs(offset_ + start));
			}
			cur = std::min(len, cur);
			for (INDEX i = 0; i < cur; ++i) {
				buf[i] = &Constant::DEMPTY;
			}
			start += cur;
			len -= cur;
			buf += cur;
		}
		int validLen = std::max(0, std::min(len, std::min(size_ - start, source_->size() - offset_ - start)));
		if(offset_ + start < source_->size() && validLen > 0 && !((Constant*)source_.get())->getString(offset_ + start, validLen, buf)) {
			return false;
		}
		if (validLen < len) {
			for (int i = validLen; i < len; ++i) {
				buf[i] = &Constant::DEMPTY;
			}
		}
		return true;
	}
	DolphinString** getStringConst(INDEX start, int len, DolphinString** buf) const override {
		if (start >= 0 && start + len <= size_ && offset_ + start >= 0 && offset_ + start + len <= source_->size()) {
			return source_->getStringConst(offset_ + start, len, buf);
		}
		DolphinString** originalBuf = buf;
		if (start < 0 || offset_ + start < 0) {
			INDEX cur = 0;
			if (start < 0) {
				cur = std::max(cur, std::abs(start));
			}
			if (offset_ + start < 0) {
				cur = std::max(cur, std::abs(offset_ + start));
			}
			cur = std::min(len, cur);
			for (INDEX i = 0; i < cur; ++i) {
				buf[i] = &Constant::DEMPTY;
			}
			start += cur;
			len -= cur;
			buf += cur;
		}
		int validLen = std::max(0, std::min(len, std::min(size_ - start, source_->size() - offset_ - start)));
		if (offset_ + start < source_->size() && validLen > 0)
			((Constant*)source_.get())->getString(offset_ + start, validLen, buf);
		if (validLen < len) {
			for (int i = validLen; i < len; ++i) {
				buf[i] = &Constant::DEMPTY;
			}
		}
		return originalBuf;
	}
	const int* getSymbolConst(INDEX start, int len, int* buf, SymbolBase* symBase, bool insertIfNotThere) const override {
		if (start >= 0 && start + len <= size_ && offset_ + start >= 0 && offset_ + start + len <= source_->size()) {
			return source_->getSymbolConst(offset_ + start, len, buf, symBase, insertIfNotThere);
		}
		int* originalBuf = buf;
		if (start < 0 || offset_ + start < 0) {
			INDEX cur = 0;
			if (start < 0) {
				cur = std::max(cur, std::abs(start));
			}
			if (offset_ + start < 0) {
				cur = std::max(cur, std::abs(offset_ + start));
			}
			cur = std::min(len, cur);
			for (INDEX i = 0; i < cur; ++i) {
				buf[i] = 0;
			}
			start += cur;
			len -= cur;
			buf += cur;
		}
		int validLen = std::max(0, std::min(len, std::min(size_ - start, source_->size() - offset_ - start)));
		if (offset_ + start < source_->size() && validLen > 0)
			source_->getSymbol(offset_ + start, validLen, buf, symBase, insertIfNotThere);
		if (validLen < len) {
			for (int i = validLen; i < len; ++i) {
				buf[i] = 0;
			}
		}
		return originalBuf;
	}
	bool getSymbol(INDEX start, int len, int* buf, SymbolBase* symBase, bool insertIfNotThere) const override {
		if (start < 0 || offset_ + start < 0) {
			INDEX cur = 0;
			if (start < 0) {
				cur = std::max(cur, std::abs(start));
			}
			if (offset_ + start < 0) {
				cur = std::max(cur, std::abs(offset_ + start));
			}
			cur = std::min(len, cur);
			for (INDEX i = 0; i < cur; ++i) {
				buf[i] = 0;
			}
			start += cur;
			len -= cur;
			buf += cur;
		}
		int validLen = std::max(0, std::min(len, std::min(size_ - start, source_->size() - offset_ - start)));
		if(offset_ + start < source_->size() && validLen > 0 && !((Constant*)source_.get())->getSymbol(offset_ + start, validLen, buf, symBase, insertIfNotThere)) {
			return false;
		}
		if (validLen < len) {
			for (int i = validLen; i < len; ++i) {
				buf[i] = 0;
			}
		}
		return true;
	}
	bool set(INDEX index, const ConstantSP& value, INDEX valueIndex) override{
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method set");
		else
			return source_->set(offset_ + index, value, valueIndex);
	}
	bool set(INDEX index, const ConstantSP& value) override{
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method set");
		else
			return source_->set(offset_ + index, value);
	}
	bool set(INDEX column, INDEX row, const ConstantSP& value) override{throw RuntimeException("Immutable sub vector doesn't support method set");}
	bool set(const ConstantSP& index, const ConstantSP& value) override {throw RuntimeException("Immutable sub vector doesn't support method set");}
	bool setNonNull(const ConstantSP& index, const ConstantSP& value) override {throw RuntimeException("Immutable sub vector doesn't support method setNonNull");}
	bool setBool(INDEX start, int len, const char* buf) override{
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setBool");
		else
			return source_->setBool(offset_ + start, len, buf);
	}
	bool setChar(INDEX start, int len, const char* buf) override{
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setChar");
		else
			return source_->setChar(offset_ + start, len, buf);
	}
	bool setShort(INDEX start, int len, const short* buf) override{
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setShort");
		else
			return source_->setShort(offset_ + start, len, buf);
	}
	bool setInt(INDEX start, int len, const int* buf) override{
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setInt");
		else
			return source_->setInt(offset_ + start, len, buf);
	}
	bool setLong(INDEX start, int len, const long long* buf) override{
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setLong");
		else
			return source_->setLong(offset_ + start, len, buf);
	}
	bool setIndex(INDEX start, int len, const INDEX* buf) override{
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setIndex");
		else
			return source_->setIndex(offset_ + start, len, buf);
	}
	bool setFloat(INDEX start, int len, const float* buf) override{
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setFloat");
		else
			return source_->setFloat(offset_ + start, len, buf);
	}
	bool setDouble(INDEX start, int len, const double* buf) override{
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setDouble");
		else
			return source_->setDouble(offset_ + start, len, buf);
	}
	bool setString(INDEX start, int len, const string* buf) override{
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setString");
		else
			return source_->setString(offset_ + start, len, buf);
	}
	bool setString(INDEX start, int len, char** buf) override{
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setString");
		else
			return source_->setString(offset_ + start, len, buf);
	}
	bool setBinary(INDEX start, int len, int unitLength, const unsigned char* buf) override{
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setBinary");
		else
			return source_->setBinary(offset_ + start, len, unitLength, buf);
	}
	bool setData(INDEX start, int len, void* buf) override {
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setData");
		else
			return source_->setData(offset_ + start, len, buf);
	}

public:  /// {get,set}Decimal{32,64,128}
	int getDecimal32(int scale) const override {
		return getDecimal32(/*index*/0, scale);
	}
	long long getDecimal64(int scale) const override {
		return getDecimal64(/*index*/0, scale);
	}
	int128 getDecimal128(int scale) const override {
		return getDecimal128(/*index*/0, scale);
	}

	int getDecimal32(INDEX index, int scale) const override {
		if (index < 0 || index >= size_ || offset_ + index < 0 || offset_ + index >= source_->size()) {
			return INT_MIN;
		}
		return source_->getDecimal32(offset_ + index, scale);
	}
	long long getDecimal64(INDEX index, int scale) const override {
		if (index < 0 || index >= size_ || offset_ + index < 0 || offset_ + index >= source_->size()) {
			return LONG_MIN;
		}
		return source_->getDecimal64(offset_ + index, scale);
	}
	int128 getDecimal128(INDEX index, int scale) const override {
		if (index < 0 || index >= size_ || offset_ + index < 0 || offset_ + index >= source_->size()) {
			return INT128_MIN;
		}
		return source_->getDecimal128(offset_ + index, scale);
	}

	bool getDecimal32(INDEX start, int len, int scale, int *buf) const override {
		if (start < 0 || offset_ + start < 0) {
			INDEX cur = 0;
			if (start < 0) {
				cur = std::max(cur, std::abs(start));
			}
			if (offset_ + start < 0) {
				cur = std::max(cur, std::abs(offset_ + start));
			}
			cur = std::min(len, cur);
			for (INDEX i = 0; i < cur; ++i) {
				buf[i] = INT_MIN;
			}
			start += cur;
			len -= cur;
			buf += cur;
		}
		int validLen = std::max(0, std::min(len, std::min(size_ - start, source_->size() - offset_ - start)));
		if (offset_ + start < source_->size() && validLen > 0 && !source_->getDecimal32(offset_ + start, validLen, scale, buf)) {
			return false;
		}
		if (validLen < len) {
			for (int i = validLen; i < len; ++i) {
				buf[i] = INT_MIN;
			}
		}
		return true;
	}
	bool getDecimal64(INDEX start, int len, int scale, long long *buf) const override {
		if (start < 0 || offset_ + start < 0) {
			INDEX cur = 0;
			if (start < 0) {
				cur = std::max(cur, std::abs(start));
			}
			if (offset_ + start < 0) {
				cur = std::max(cur, std::abs(offset_ + start));
			}
			cur = std::min(len, cur);
			for (INDEX i = 0; i < cur; ++i) {
				buf[i] = LONG_MIN;
			}
			start += cur;
			len -= cur;
			buf += cur;
		}
		int validLen = std::max(0, std::min(len, std::min(size_ - start, source_->size() - offset_ - start)));
		if (offset_ + start < source_->size() && validLen > 0 && !source_->getDecimal64(offset_ + start, validLen, scale, buf)) {
			return false;
		}
		if (validLen < len) {
			for (int i = validLen; i < len; ++i) {
				buf[i] = LONG_MIN;
			}
		}
		return true;
	}
	bool getDecimal128(INDEX start, int len, int scale, int128 *buf) const override {
		if (start < 0 || offset_ + start < 0) {
			INDEX cur = 0;
			if (start < 0) {
				cur = std::max(cur, std::abs(start));
			}
			if (offset_ + start < 0) {
				cur = std::max(cur, std::abs(offset_ + start));
			}
			cur = std::min(len, cur);
			for (INDEX i = 0; i < cur; ++i) {
				buf[i] = INT128_MIN;
			}
			start += cur;
			len -= cur;
			buf += cur;
		}
		int validLen = std::max(0, std::min(len, std::min(size_ - start, source_->size() - offset_ - start)));
		if (offset_ + start < source_->size() && validLen > 0 && !source_->getDecimal128(offset_ + start, validLen, scale, buf)) {
			return false;
		}
		if (validLen < len) {
			for (int i = validLen; i < len; ++i) {
				buf[i] = INT128_MIN;
			}
		}
		return true;
	}

	const int* getDecimal32Const(INDEX start, int len, int scale, int *buf) const override {
		if (start >= 0 && start + len <= size_ && offset_ + start >= 0 && offset_ + start + len <= source_->size()) {
			return source_->getDecimal32Const(offset_ + start, len, scale, buf);
		}
		int* originalBuf = buf;
		if (start < 0 || offset_ + start < 0) {
			INDEX cur = 0;
			if (start < 0) {
				cur = std::max(cur, std::abs(start));
			}
			if (offset_ + start < 0) {
				cur = std::max(cur, std::abs(offset_ + start));
			}
			cur = std::min(len, cur);
			for (INDEX i = 0; i < cur; ++i) {
				buf[i] = INT_MIN;
			}
			start += cur;
			len -= cur;
			buf += cur;
		}
		int validLen = std::max(0, std::min(len, std::min(size_ - start, source_->size() - offset_ - start)));
		if (offset_ + start < source_->size() && validLen > 0)
			source_->getDecimal32(offset_ + start, validLen, scale, buf);
		if (validLen < len) {
			for (int i = validLen; i < len; ++i) {
				buf[i] = INT_MIN;
			}
		}
		return originalBuf;
	}
	const long long* getDecimal64Const(INDEX start, int len, int scale, long long *buf) const override {
		if (start >= 0 && start + len <= size_ && offset_ + start >= 0 && offset_ + start + len <= source_->size()) {
			return source_->getDecimal64Const(offset_ + start, len, scale, buf);
		}
		long long* originalBuf = buf;
		if (start < 0 || offset_ + start < 0) {
			INDEX cur = 0;
			if (start < 0) {
				cur = std::max(cur, std::abs(start));
			}
			if (offset_ + start < 0) {
				cur = std::max(cur, std::abs(offset_ + start));
			}
			cur = std::min(len, cur);
			for (INDEX i = 0; i < cur; ++i) {
				buf[i] = LONG_MIN;
			}
			start += cur;
			len -= cur;
			buf += cur;
		}
		int validLen = std::max(0, std::min(len, std::min(size_ - start, source_->size() - offset_ - start)));
		if (offset_ + start < source_->size() && validLen > 0)
			source_->getDecimal64(offset_ + start, validLen, scale, buf);
		if (validLen < len) {
			for (int i = validLen; i < len; ++i) {
				buf[i] = LONG_MIN;
			}
		}
		return originalBuf;
	}
	const int128* getDecimal128Const(INDEX start, int len, int scale,
			int128 *buf) const override {
		if (start >= 0 && start + len <= size_ && offset_ + start >= 0 && offset_ + start + len <= source_->size()) {
			return source_->getDecimal128Const(offset_ + start, len, scale, buf);
		}
		int128* originalBuf = buf;
		if (start < 0 || offset_ + start < 0) {
			INDEX cur = 0;
			if (start < 0) {
				cur = std::max(cur, std::abs(start));
			}
			if (offset_ + start < 0) {
				cur = std::max(cur, std::abs(offset_ + start));
			}
			cur = std::min(len, cur);
			for (INDEX i = 0; i < cur; ++i) {
				buf[i] = INT128_MIN;
			}
			start += cur;
			len -= cur;
			buf += cur;
		}
		int validLen = std::max(0, std::min(len, std::min(size_ - start, source_->size() - offset_ - start)));
		if (offset_ + start < source_->size() && validLen > 0)
			source_->getDecimal128(offset_ + start, validLen, scale, buf);
		if (validLen < len) {
			for (int i = validLen; i < len; ++i) {
				buf[i] = INT128_MIN;
			}
		}
		return originalBuf;
	}

	int* getDecimal32Buffer(INDEX start, int len, int scale, int *buf) const override {
		return source_->getDecimal32Buffer(offset_ + start, len, scale, buf);
	}
	long long* getDecimal64Buffer(INDEX start, int len, int scale, long long *buf) const override {
		return source_->getDecimal64Buffer(offset_ + start, len, scale, buf);
	}
	int128* getDecimal128Buffer(INDEX start, int len, int scale,
			int128 *buf) const override {
		return source_->getDecimal128Buffer(offset_ + start, len, scale, buf);
	}

	void setDecimal32(INDEX index, int scale, int val) override {
		if (!updatable_) {
			throw RuntimeException("Immutable sub vector doesn't support method setDecimal32");
		} else {
			source_->setDecimal32(offset_ + index, scale, val);
		}
	}
	void setDecimal64(INDEX index, int scale, long long val) override {
		if (!updatable_) {
			throw RuntimeException("Immutable sub vector doesn't support method setDecimal64");
		} else {
			source_->setDecimal64(offset_ + index, scale, val);
		}
	}
	void setDecimal128(INDEX index, int scale, int128 val) override {
		if (!updatable_) {
			throw RuntimeException("Immutable sub vector doesn't support method setDecimal128");
		} else {
			source_->setDecimal128(offset_ + index, scale, val);
		}
	}

	bool setDecimal32(INDEX start, int len, int scale, const int *buf) override {
		if (!updatable_) {
			throw RuntimeException("Immutable sub vector doesn't support method setDecimal32");
		} else {
			return source_->setDecimal32(offset_ + start, len, scale, buf);
		}
	}
	bool setDecimal64(INDEX start, int len, int scale, const long long *buf) override {
		if (!updatable_) {
			throw RuntimeException("Immutable sub vector doesn't support method setDecimal64");
		} else {
			return source_->setDecimal64(offset_ + start, len, scale, buf);
		}
	}
	bool setDecimal128(INDEX start, int len, int scale, const int128 *buf) override {
		if (!updatable_) {
			throw RuntimeException("Immutable sub vector doesn't support method setDecimal128");
		} else {
			return source_->setDecimal128(offset_ + start, len, scale, buf);
		}
	}

public:
	using Vector::appendBool;
	virtual bool appendBool(char* buf, int len){throw RuntimeException("Immutable sub vector doesn't support method appendBool");}
	using Vector::appendChar;
	virtual bool appendChar(char* buf, int len){throw RuntimeException("Immutable sub vector doesn't support method appendChar");}
	using Vector::appendShort;
	virtual bool appendShort(short* buf, int len){throw RuntimeException("Immutable sub vector doesn't support method appendShort");}
	using Vector::appendInt;
	virtual bool appendInt(int* buf, int len){throw RuntimeException("Immutable sub vector doesn't support method appendInt");}
	using Vector::appendLong;
	virtual bool appendLong(long long* buf, int len){throw RuntimeException("Immutable sub vector doesn't support method appendLong");}
	using Vector::appendIndex;
	virtual bool appendIndex(INDEX* buf, int len){throw RuntimeException("Immutable sub vector doesn't support method appendIndex");}
	using Vector::appendFloat;
	virtual bool appendFloat(float* buf, int len){throw RuntimeException("Immutable sub vector doesn't support method appendFloat");}
	using Vector::appendDouble;
	virtual bool appendDouble(double* buf, int len){throw RuntimeException("Immutable sub vector doesn't support method appendDouble");}
	bool assign(const ConstantSP& value) override{throw RuntimeException("Immutable sub vector doesn't support method assign");}

	/**
	 * @param rightMost If there are multiple maximum/minimum values, choose the last one if `rightMost` is true.
	 */
	INDEX imax(bool rightMost = false) const override { return imax(0, size_, rightMost); }
	INDEX imax(INDEX start, INDEX length, bool rightMost = false) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		INDEX index = source_->imax(range.first, range.second, rightMost);
		return index >= 0 ? index - offset_ : index;
	}
	INDEX imin(bool rightMost = false) const override { return imin(0, size_, rightMost); }
	INDEX imin(INDEX start, INDEX length, bool rightMost = false) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		INDEX index = source_->imin(range.first, range.second, rightMost);
		return index >= 0 ? index - offset_ : index;
	}
	long long count() const override {return count(0, size_);}
	long long count(INDEX start, INDEX length) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		return source_->count(range.first, range.second);
	}
	ConstantSP minmax() const override {return minmax(0, size_);}
	ConstantSP minmax(INDEX start, INDEX length) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		return source_->minmax(range.first, range.second);
	}

	ConstantSP max() const override { return max(0, size_); }
	ConstantSP max(INDEX start, INDEX length) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		return source_->max(range.first, range.second);
	}
	void max(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		source_->max(range.first, range.second, out, outputStart);
	}

	ConstantSP min() const override { return min(0, size_); }
	ConstantSP min(INDEX start, INDEX length) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		return source_->min(range.first, range.second);
	}
	void min(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		source_->min(range.first, range.second, out, outputStart);
	}

	ConstantSP avg() const override { return avg(0, size_); }
	ConstantSP avg(INDEX start, INDEX length) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		return source_->avg(range.first, range.second);
	}
	void avg(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		source_->avg(range.first, range.second, out, outputStart);
	}

	ConstantSP sum() const override { return sum(0, size_); }
	ConstantSP sum(INDEX start, INDEX length) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		return source_->sum(range.first, range.second);
	}
	void sum(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		source_->sum(range.first, range.second, out, outputStart);
	}

	ConstantSP sum2() const override { return sum2(0, size_); }
	ConstantSP sum2(INDEX start, INDEX length) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		return source_->sum2(range.first, range.second);
	}
	void sum2(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		source_->sum2(range.first, range.second, out, outputStart);
	}

	ConstantSP std() const override { return std(0, size_); }
	ConstantSP std(INDEX start, INDEX length) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		return source_->std(range.first, range.second);
	}
	void std(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		source_->std(range.first, range.second, out, outputStart);
	}
	ConstantSP var() const override { return var(0, size_); }
	ConstantSP var(INDEX start, INDEX length) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		return source_->var(range.first, range.second);
	}
	void var(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		source_->var(range.first, range.second, out, outputStart);
	}
	ConstantSP prd() const override { return prd(0, size_); }
	ConstantSP prd(INDEX start, INDEX length) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		return source_->prd(range.first, range.second);
	}
	void prd(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		source_->prd(range.first, range.second, out, outputStart);
	}
	ConstantSP median() const override { return median(0, size_); }
	ConstantSP median(INDEX start, INDEX length) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		return source_->median(range.first, range.second);
	}
	void median(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		source_->median(range.first, range.second, out, outputStart);
	}
	ConstantSP mode() const override { return mode(0, size_); }
	ConstantSP mode(INDEX start, INDEX length) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		return source_->mode(range.first, range.second);
	}
	void mode(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		source_->mode(range.first, range.second, out, outputStart);
	}
	ConstantSP firstNot(const ConstantSP& exclude) const override { return firstNot(0, size_, exclude); }
	ConstantSP firstNot(INDEX start, INDEX length, const ConstantSP& exclude) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		return source_->firstNot(range.first, range.second, exclude);
	}
	void firstNot(INDEX start, INDEX length, const ConstantSP& exclude, const ConstantSP& out, INDEX outputStart = 0) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		source_->firstNot(range.first, range.second, exclude, out, outputStart);
	}
	ConstantSP lastNot(const ConstantSP& exclude) const override { return lastNot(0, size_, exclude); }
	ConstantSP lastNot(INDEX start, INDEX length, const ConstantSP& exclude) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		return source_->lastNot(range.first, range.second, exclude);
	}
	void lastNot(INDEX start, INDEX length, const ConstantSP& exclude, const ConstantSP& out, INDEX outputStart = 0) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		source_->lastNot(range.first, range.second, exclude, out, outputStart);
	}
	ConstantSP searchK(INDEX k) const override { return searchK(0, size_, k); }
	ConstantSP searchK(INDEX start, INDEX length, INDEX k) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		return source_->searchK(range.first, range.second, k);
	}
	void searchK(INDEX start, INDEX length, INDEX k, const ConstantSP& out, INDEX outputStart = 0) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		source_->searchK(range.first, range.second, k, out, outputStart);
	}

	void neg() override{throw RuntimeException("Immutable sub vector doesn't support method neg");}
	void replace(const ConstantSP& oldVal, const ConstantSP& newVal) override{throw RuntimeException("Immutable sub vector doesn't support method replace");}
	void shuffle() override{throw RuntimeException("Immutable sub vector doesn't support method shuffle");}
	void reverse() override{
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method reverse");
		else
			source_->reverse(offset_, size_);
	}
	void reverse(INDEX start, INDEX length) override{
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method reverse");
		else
			source_->reverse(start + offset_, length);
	}
	virtual bool rank(bool sorted, INDEX* indices, INDEX* ranking){
		throw RuntimeException("Immutable sub vector doesn't support method rank");
	}
	void find(INDEX start, INDEX length, const ConstantSP& target, const ConstantSP& resultSP) override{
		auto range = calculateOverlappedRange(start + offset_, length);
		source_->find(range.first, range.second, target, resultSP);
		if(offset_ > 0){
			if(resultSP->isScalar()){
				INDEX index = resultSP->getIndex();
				if(index >= 0)
					resultSP->setIndex(index - offset_);
			}
			else
				((Vector*)resultSP.get())->addIndex(0, resultSP->size(), -offset_);
		}
	}
	void binarySearch(INDEX start, INDEX length, const ConstantSP& target, const ConstantSP& resultSP) override{
		auto range = calculateOverlappedRange(start + offset_, length);
		source_->binarySearch(range.first, range.second, target, resultSP);
		if(offset_ > 0){
			if(resultSP->isScalar()){
				INDEX index = resultSP->getIndex();
				if(index >= 0)
					resultSP->setIndex(index - offset_);
			}
			else
				((Vector*)resultSP.get())->addIndex(0, resultSP->size(), -offset_);
		}
	}
	void asof(INDEX start, INDEX length, const ConstantSP& target, const ConstantSP& resultSP) override{
		auto range = calculateOverlappedRange(start + offset_, length);
		source_->asof(range.first, range.second, target, resultSP);
		if(offset_ > 0){
			if(resultSP->isScalar()){
				INDEX index = resultSP->getIndex();
				if(index >= 0)
					resultSP->setIndex(index - offset_);
			}
			else
				((Vector*)resultSP.get())->addIndex(0, resultSP->size(), -offset_);
		}
	}

	INDEX lowerBound(INDEX start, const ConstantSP& target) override;

	bool equalToPrior(INDEX start, INDEX length, bool* result) override{
		return source_->equalToPrior(offset_ + start, length, result);
	}

	bool equalToPrior(INDEX prior, const INDEX* indices, INDEX length, bool* result) override{
		INDEX indexBuf[Util::BUF_SIZE];
		INDEX start = 0;
		prior += offset_;
		while(start < length){
			int count = std::min(length - start, Util::BUF_SIZE);
			for(int i=0; i<count; ++i)
				indexBuf[i] = offset_ + indices[start + i];
			if(!source_->equalToPrior(prior, indexBuf, count, result + start))
				return false;
			prior = indexBuf[count - 1];
			start += count;
		}
		return true;
	}

	bool findDuplicatedElements(Vector*  indices, INDEX start, INDEX length, vector<pair<INDEX,INDEX> >& duplicates) override{
		if(offset_ == 0)
			return source_->findDuplicatedElements(indices, start, length, duplicates);
		else
			throw RuntimeException("Immutable sub vector doesn't support method findDuplicatedElements");
	}
	bool findDuplicatedElements(INDEX start, INDEX length, vector<pair<INDEX,INDEX> >& duplicates) override{
		throw RuntimeException("Immutable sub vector doesn't support method findDuplicatedElements");
	}
	bool findUniqueElements(INDEX start, INDEX length, vector<pair<INDEX,INDEX> >& uniques) override{
		throw RuntimeException("Immutable sub vector doesn't support method findUniqueElements");
	}
	bool findRange(INDEX* ascIndices,const ConstantSP& target,INDEX* targetIndices,vector<pair<INDEX,INDEX> >& ranges) override{
		throw RuntimeException("Immutable sub vector doesn't support method findRange");
	}
	bool findRange(const ConstantSP& target,INDEX* targetIndices,vector<pair<INDEX,INDEX> >& ranges) override{
		throw RuntimeException("Immutable sub vector doesn't support method findRange");
	}
	long long getAllocatedMemory() const override {
		// use the allocated memory of the underlying source vector, for the calculation of cacheEngine
		// 	in tsdb to be correct
		return ((Constant*)source_.get())->getAllocatedMemory() * (size_ * 1.0 / source_->size());
	}
	int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int& numElement, int& partial) const override;
	int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int targetNumElement, int& numElement, int& partial) const override;
	IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const override {throw RuntimeException("Immutable sub vector doesn't support method serialize");}
	bool isSorted(INDEX start, INDEX length, bool asc, bool strict, char nullsOrder = 0) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		return source_->isSorted(range.first, range.second, asc, strict, nullsOrder);
	}
	using Vector::topK;
	virtual ConstantSP topK(INDEX start, INDEX length, INDEX top, bool asc, bool extendEqualValue) const {
		ConstantSP result = source_->topK(offset_ + start, length, top, asc, extendEqualValue);
		if(offset_ > 0)
			((Vector*)result.get())->addIndex(0, result->size(), -offset_);
		return result;
	}
	int compare(INDEX indexLeft, INDEX indexRight) const override {throw RuntimeException("SubVector does not support compare(indexLeft, indexRight).");};
    bool sort(bool asc, char nullsOrder = 0) override {throw RuntimeException("Immutable sub vector doesn't support method sort");}
	bool sort(bool asc, Vector* indices, char nullsOrder = 0) override {throw RuntimeException("Immutable sub vector doesn't support method sort");}
	bool sortSelectedIndices(Vector* indices, INDEX start, INDEX length, bool asc, char nullsOrder = 0) override {
		if(!indices->add(start, length, (long long)offset_))
			return false;
		if(!source_->sortSelectedIndices(indices, start, length, asc, nullsOrder))
			return false;
		return indices->add(start, length, (long long)-offset_);
	}

	ConstantSP flatten(INDEX rowStart, INDEX count) const override {return source_->flatten(rowStart + offset_, count);}
	ConstantSP rowSum(INDEX rowStart, INDEX count) const override {return source_->rowSum(rowStart + offset_, count);}
	ConstantSP rowSum2(INDEX rowStart, INDEX count) const override {return source_->rowSum2(rowStart + offset_, count);}
	ConstantSP rowCount(INDEX rowStart, INDEX count) const override {return source_->rowCount(rowStart + offset_, count);}
	ConstantSP rowSize(INDEX rowStart, INDEX count) const override {return source_->rowSize(rowStart + offset_, count);}
	ConstantSP rowAvg(INDEX rowStart, INDEX count) const override {return source_->rowAvg(rowStart + offset_, count);}
	ConstantSP rowStd(INDEX rowStart, INDEX count) const override {return source_->rowStd(rowStart + offset_, count);}
	ConstantSP rowStdp(INDEX rowStart, INDEX count) const override {return source_->rowStdp(rowStart + offset_, count);}
	ConstantSP rowVar(INDEX rowStart, INDEX count) const override {return source_->rowVar(rowStart + offset_, count);}
	ConstantSP rowVarp(INDEX rowStart, INDEX count) const override {return source_->rowVarp(rowStart + offset_, count);}
	ConstantSP rowMin(INDEX rowStart, INDEX count) const override {return source_->rowMin(rowStart + offset_, count);}
	ConstantSP rowMax(INDEX rowStart, INDEX count) const override {return source_->rowMax(rowStart + offset_, count);}
	ConstantSP rowProd(INDEX rowStart, INDEX count) const override {return source_->rowProd(rowStart + offset_, count);}
	ConstantSP rowAnd(INDEX rowStart, INDEX count) const override {return source_->rowAnd(rowStart + offset_, count);}
	ConstantSP rowOr(INDEX rowStart, INDEX count) const override {return source_->rowOr(rowStart + offset_, count);}
	ConstantSP rowXor(INDEX rowStart, INDEX count) const override {return source_->rowXor(rowStart + offset_, count);}
	ConstantSP rowMed(INDEX rowStart, INDEX count) const override {return source_->rowMed(rowStart + offset_, count);}
	ConstantSP rowKurtosis(INDEX rowStart, INDEX count, bool biased) const override {return source_->rowKurtosis(rowStart + offset_, count, biased);}
	ConstantSP rowSkew(INDEX rowStart, INDEX count, bool biased) const override {return source_->rowSkew(rowStart + offset_, count, biased);}
	ConstantSP rowPercentile(INDEX rowStart, INDEX count, double percentile) const override {return source_->rowPercentile(rowStart + offset_, count, percentile);}
	ConstantSP rowRank(INDEX rowStart, INDEX count, bool ascending, int groupNum, bool ignoreNA, int tiesMethod, bool percent) const override {
		return source_->rowRank(rowStart + offset_, count, ascending, groupNum, ignoreNA, tiesMethod, percent);
	}
	ConstantSP rowDenseRank(INDEX rowStart, INDEX count, bool ascending, bool ignoreNA, bool percent) const override {
		return source_->rowDenseRank(rowStart + offset_, count, ascending, ignoreNA, percent);
	}

	/**
	 * The following series of safe operators assumes:
	 * (1) indices is ascending sorted
	 * (2) offset + indices are guaranteed valid ( between 0 and size - 1)
	 */
	bool isNullSafe(INDEX offset, INDEX* indices, int len, char* buf) const override {
		return source_->isNullSafe(offset + offset_, indices, len, buf);
	}

	bool isValidSafe(INDEX offset, INDEX* indices, int len, char* buf) const override {
		return source_->isValidSafe(offset + offset_, indices, len, buf);
	}

	bool getBoolSafe(INDEX offset, INDEX* indices, int len, char* buf) const override {
		return source_->getBoolSafe(offset + offset_, indices, len, buf);
	}

	bool getCharSafe(INDEX offset, INDEX* indices, int len,char* buf) const override {
		return source_->getCharSafe(offset + offset_, indices, len, buf);
	}

	bool getShortSafe(INDEX offset, INDEX* indices, int len, short* buf) const override {
		return source_->getShortSafe(offset + offset_, indices, len, buf);
	}

	bool getIntSafe(INDEX offset, INDEX* indices, int len, int* buf) const override {
		return source_->getIntSafe(offset + offset_, indices, len, buf);
	}

	bool getLongSafe(INDEX offset, INDEX* indices, int len, long long* buf) const override {
		return source_->getLongSafe(offset + offset_, indices, len, buf);
	}

	bool getIndexSafe(INDEX offset, INDEX* indices, int len, INDEX* buf) const override {
		return source_->getIndexSafe(offset + offset_, indices, len, buf);
	}

	bool getFloatSafe(INDEX offset, INDEX* indices, int len, float* buf) const override {
		return source_->getFloatSafe(offset + offset_, indices, len, buf);
	}

	bool getDoubleSafe(INDEX offset, INDEX* indices, int len, double* buf) const override {
		return source_->getDoubleSafe(offset + offset_, indices, len, buf);
	}

	bool getSymbolSafe(INDEX offset, INDEX* indices, int len, int* buf, SymbolBase* symBase,bool insertIfNotThere) const override {
		return source_->getSymbolSafe(offset + offset_, indices, len, buf, symBase, insertIfNotThere);
	}

	bool getStringSafe(INDEX offset, INDEX* indices, int len, DolphinString** buf) const override {
		return source_->getStringSafe(offset + offset_, indices, len, buf);
	}

	bool getStringSafe(INDEX offset, INDEX* indices, int len, char** buf) const override {
		return source_->getStringSafe(offset + offset_, indices, len, buf);
	}

	bool getBinarySafe(INDEX offset, INDEX* indices, int len, int unitLength, unsigned char* buf) const override {
		return source_->getBinarySafe(offset + offset_, indices, len, unitLength, buf);
	}

private:
	std::pair<INDEX, INDEX> calculateOverlappedRange(INDEX offset, INDEX length) const {
		std::pair<INDEX, INDEX> overlappedRange;

		if (offset >= 0) {
			if (offset < source_->size()) {
				overlappedRange.first = offset;
				overlappedRange.second = std::min(length, source_->size() - offset);
			} else {
				overlappedRange.first = 0;
				overlappedRange.second = 0;
			}
		} else {
			overlappedRange.first = 0;
			overlappedRange.second = std::min(offset + length, source_->size());
		}

		return overlappedRange;
	}

	static void fillWithNullForDecimal(void *buf, int length, int unitLength) {
		if (unitLength == 4) {
			int *dst = reinterpret_cast<int *>(buf);
			for (int i = 0; i < length; ++i) {
				dst[i] = INT_MIN;
			}
		} else if (unitLength == 8) {
			long long *dst = reinterpret_cast<long long *>(buf);
			for (int i = 0; i < length; ++i) {
				dst[i] = LLONG_MIN;
			}
		} else if (unitLength == 16) {
			int128 *dst = reinterpret_cast<int128 *>(buf);
			for (int i = 0; i < length; ++i) {
				dst[i] = INT128_MIN;
			}
		} else {
			throw RuntimeException("Unknown decimal type: unitLength = " + std::to_string(unitLength));
		}
	}

private:
	VectorSP source_;
	INDEX offset_;
	INDEX size_;
	bool updatable_;
};

class SWORDFISH_API FastArrayVector : public Vector {
public:
	FastArrayVector(const VectorSP& index, const VectorSP& value, bool checkNull = true);
	INDEX getValueSize() const { return valueSize_;}
	VectorSP getSourceIndex() const { return index_;}
	VectorSP getSourceValue() const { return value_;}
	void resetSourceValue(const VectorSP& vec);
	~FastArrayVector() override{}
	bool isLargeConstant() const override {return true;}
	VECTOR_TYPE getVectorType() const override {return VECTOR_TYPE::ARRAYVECTOR;}
	DATA_TYPE getRawType() const override {return getType();}
	int getExtraParamForType() const override { return value_->getExtraParamForType(); }
	SymbolBaseSP getSymbolBase() const override {return value_->getSymbolBase();}
	virtual DATA_FORM getForm() const { return DF_VECTOR;}
	ConstantSP getInstance() const override {return getInstance(size_);}
	ConstantSP getInstance(INDEX size) const override;
	ConstantSP getValue() const override;
	ConstantSP getValue(INDEX capacity) const override;
	ConstantSP get(INDEX column, INDEX rowStart,INDEX rowEnd) const override;
	ConstantSP getWindow(INDEX colStart, int colLength, INDEX rowStart, int rowLength) const override;
	ConstantSP getSubVector(INDEX start, INDEX length, INDEX capacity) const override;
	ConstantSP getSubVector(INDEX start, INDEX length) const override;
	void fill(INDEX start, INDEX length, const ConstantSP& value, INDEX valueOffset = 0) override;
	bool validIndex(INDEX uplimit) override{throw RuntimeException("Array vector doesn't support method validIndex");}
	bool validIndex(INDEX start, INDEX length, INDEX uplimit) override{throw RuntimeException("Array vector doesn't support method validIndex");}
	int compare(INDEX index, const ConstantSP& target) const override {throw RuntimeException("Array vector doesn't support method compare");}
	bool getNullFlag() const override {return containNull_;}
	void setNullFlag(bool containNull) override{ containNull_ = containNull;}
	bool hasNull() override{return hasNull(0, size_);}
	bool hasNull(INDEX start, INDEX length) override;
	INDEX getCapacity() const override {return index_->getCapacity();}
	bool isFastMode() const override { return value_->isFastMode();}
	void* getDataArray() const override {return value_->getDataArray();}
	bool isIndexArray() const override { return value_->isIndexArray();}
	INDEX* getIndexArray() const override { return value_->getIndexArray();}
	short getUnitLength() const override {return 0;}
	void** getDataSegment() const override { return value_->getDataSegment();}
	bool sizeable() const override {return value_->sizeable();}
	char getBool() const override {throw RuntimeException("Array vector doesn't support method getBool");}
	char getChar() const override {throw RuntimeException("Array vector doesn't support method getChar");}
	short getShort() const override {throw RuntimeException("Array vector doesn't support method getShort");}
	int getInt() const override {throw RuntimeException("Array vector doesn't support method getInt");}
	long long getLong() const override {throw RuntimeException("Array vector doesn't support method getLong");}
	INDEX getIndex() const override {throw RuntimeException("Array vector doesn't support method getIndex");}
	float getFloat() const override {throw RuntimeException("Array vector doesn't support method getFloat");}
	double getDouble() const override {throw RuntimeException("Array vector doesn't support method getDouble");}
	ConstantSP get(INDEX index) const override;
	virtual ConstantSP getRef(INDEX index) const;
	ConstantSP get(const ConstantSP& index) const override;
	ConstantSP get(INDEX offset, const ConstantSP& index) const override;
	ConstantSP getSlice(const ConstantSP& rowIndex, const ConstantSP& colIndex) const override;
	using Vector::setBool;
	virtual void setBool(bool val){throw RuntimeException("Array vector doesn't support method setBool");}
	void setChar(char val) override{throw RuntimeException("Array vector doesn't support method setChar");}
	void setShort(short val) override{throw RuntimeException("Array vector doesn't support method setShort");}
	void setInt(int val) override{throw RuntimeException("Array vector doesn't support method setInt");}
	void setLong(long long val) override{throw RuntimeException("Array vector doesn't support method setLong");}
	void setIndex(INDEX val) override{throw RuntimeException("Array vector doesn't support method setIndex");}
	void setFloat(float val) override{throw RuntimeException("Array vector doesn't support method setFloat");}
	void setDouble(double val) override{throw RuntimeException("Array vector doesn't support method setDouble");}
	using Vector::setString;
	virtual void setString(const string& val){throw RuntimeException("Array vector doesn't support method setString");}
	void setNull() override{throw RuntimeException("Array vector doesn't support method setNull");}
	char getBool(INDEX index) const override {throw RuntimeException("Array vector doesn't support method getBool");}
	char getChar(INDEX index) const override {throw RuntimeException("Array vector doesn't support method getChar");}
	short getShort(INDEX index) const override {throw RuntimeException("Array vector doesn't support method getShort");}
	int getInt(INDEX index) const override {throw RuntimeException("Array vector doesn't support method getInt");}
	long long getLong(INDEX index) const override {throw RuntimeException("Array vector doesn't support method getLong");}
	INDEX getIndex(INDEX index) const override {throw RuntimeException("Array vector doesn't support method getIndex");}
	float getFloat(INDEX index) const override {throw RuntimeException("Array vector doesn't support method getFloat");}
	double getDouble(INDEX index) const override {throw RuntimeException("Array vector doesn't support method getDouble");}
	bool isNull(INDEX index) const override ;
	string getString(INDEX index) const override;
	const DolphinString& getStringRef(INDEX index) const override {throw RuntimeException("Array vector doesn't support method getStringRef");}
	void setBool(INDEX index,char val) override {throw RuntimeException("Array vector doesn't support method setBool");}
	void setChar(INDEX index,char val) override{throw RuntimeException("Array vector doesn't support method setChar");}
	void setShort(INDEX index,short val) override{throw RuntimeException("Array vector doesn't support method setShort");}
	void setInt(INDEX index,int val) override{throw RuntimeException("Array vector doesn't support method setInt");}
	void setLong(INDEX index,long long val) override{throw RuntimeException("Array vector doesn't support method setLong");}
	void setIndex(INDEX index, INDEX val) override{throw RuntimeException("Array vector doesn't support method setIndex");}
	void setFloat(INDEX index,float val) override{throw RuntimeException("Array vector doesn't support method setBinary");}
	void setDouble(INDEX index, double val) override{throw RuntimeException("Array vector doesn't support method setDouble");}
	void setString(INDEX index, const DolphinString& val) override{throw RuntimeException("Array vector doesn't support method setString");}
	void setBinary(INDEX index, int unitLength, const unsigned char* val) override{throw RuntimeException("Array vector doesn't support method setBinary");}
	void setNull(INDEX index) override{throw RuntimeException("Array vector doesn't support method setNull");}
	INDEX reserve(INDEX capacity) override;
	void clear() override;
	bool remove(INDEX count) override;
	bool remove(const ConstantSP& index) override;
	bool append(const ConstantSP& value) override;
	bool append(const ConstantSP& value, INDEX count) override;
	bool append(const ConstantSP& value, INDEX start, INDEX count) override;
	bool append(const ConstantSP& value, const ConstantSP& index) override;
	void next(INDEX steps) override;
	void prev(INDEX steps) override;
	INDEX size() const override { return size_;}
	void nullFill(const ConstantSP& val) override{throw RuntimeException("Array vector doesn't support method nullFill");}
	bool isNull(INDEX start, int len, char* buf) const override;
	bool isValid(INDEX start, int len, char* buf) const override;
	char* getBoolBuffer(INDEX start, int len, char* buf) const override {throw RuntimeException("Array vector doesn't support method getBoolBuffer");}
	char* getCharBuffer(INDEX start, int len, char* buf) const override {throw RuntimeException("Array vector doesn't support method getCharBuffer");}
	short* getShortBuffer(INDEX start, int len, short* buf) const override {throw RuntimeException("Array vector doesn't support method getShortBuffer");}
	int* getIntBuffer(INDEX start, int len, int* buf) const override {throw RuntimeException("Array vector doesn't support method getIntBuffer");}
	long long* getLongBuffer(INDEX start, int len, long long* buf) const override {throw RuntimeException("Array vector doesn't support method getLongBuffer");}
	unsigned char* getBinaryBuffer(INDEX start, int len, int unitLength, unsigned char* buf) const override {throw RuntimeException("Array vector doesn't support method getBinaryBuffer");}
	INDEX* getIndexBuffer(INDEX start, int len, INDEX* buf) const override {throw RuntimeException("Array vector doesn't support method getIndexBuffer");}
	float* getFloatBuffer(INDEX start, int len, float* buf) const override {throw RuntimeException("Array vector doesn't support method getFloatBuffer");}
	double* getDoubleBuffer(INDEX start, int len, double* buf) const override {throw RuntimeException("Array vector doesn't support method getDoubleBuffer");}
	bool set(INDEX index, const ConstantSP& value) override;
	bool set(INDEX column, INDEX row, const ConstantSP& value) override;
	bool set(const ConstantSP& index, const ConstantSP& value) override;
	bool setNonNull(const ConstantSP& index, const ConstantSP& value) override {throw RuntimeException("Array vector doesn't support method setNonNull");}
	bool assign(const ConstantSP& value) override;

	long long count() const override { return count(0, size_);}
	long long count(INDEX start, INDEX length) const override;
	ConstantSP minmax() const override {return value_->minmax();}
	ConstantSP minmax(INDEX start, INDEX length) const override;
	/**
	 * @param rightMost If there are multiple maximum/minimum values, choose the last one if `rightMost` is true.
	 */
	INDEX imax(bool rightMost = false) const override {
		throw RuntimeException("Array vector doesn't support method imax");
	}
	INDEX imax(INDEX start, INDEX length, bool rightMost = false) const override {
		throw RuntimeException("Array vector doesn't support method imax");
	}
	INDEX imin(bool rightMost = false) const override {
		throw RuntimeException("Array vector doesn't support method imin");
	}
	INDEX imin(INDEX start, INDEX length, bool rightMost = false) const override {
		throw RuntimeException("Array vector doesn't support method imin");
	}

	ConstantSP max() const override {return value_->max();}
	ConstantSP max(INDEX start, INDEX length) const override;
	void max(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override;
	ConstantSP min() const override {return value_->min();}
	ConstantSP min(INDEX start, INDEX length) const override;
	void min(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override;
	ConstantSP avg() const override {return value_->avg();}
	ConstantSP avg(INDEX start, INDEX length) const override;
	void avg(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override;
	ConstantSP sum() const override {return value_->sum();}
	ConstantSP sum(INDEX start, INDEX length) const override;
	void sum(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override;
	ConstantSP sum2() const override {return value_->sum2();}
	ConstantSP sum2(INDEX start, INDEX length) const override;
	void sum2(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override;
	ConstantSP prd() const override {return value_->prd();}
	ConstantSP prd(INDEX start, INDEX length) const override;
	void prd(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override;
	ConstantSP var() const override {return value_->var();}
	ConstantSP var(INDEX start, INDEX length) const override;
	void var(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override;
	ConstantSP std() const override {return value_->std();}
	ConstantSP std(INDEX start, INDEX length) const override;
	void std(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override;
	ConstantSP mode() const override {return value_->mode();}
	ConstantSP mode(INDEX start, INDEX length) const override;
	void mode(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override;
	ConstantSP median() const override {return value_->median();}
	ConstantSP median(INDEX start, INDEX length) const override;
	void median(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override;
	ConstantSP searchK(INDEX k) const override {return value_->searchK(k);}
	ConstantSP searchK(INDEX start, INDEX length, INDEX k) const override;
	void searchK(INDEX start, INDEX length, INDEX k, const ConstantSP& out, INDEX outputStart = 0) const override;
	ConstantSP firstNot(const ConstantSP& exclude) const override {return value_->firstNot(exclude);}
	ConstantSP firstNot(INDEX start, INDEX length, const ConstantSP& exclude) const override;
	void firstNot(INDEX start, INDEX length, const ConstantSP& exclude, const ConstantSP& out, INDEX outputStart = 0) const override;
	ConstantSP lastNot(const ConstantSP& exclude) const override {return value_->lastNot(exclude);}
	ConstantSP lastNot(INDEX start, INDEX length, const ConstantSP& exclude) const override;
	void lastNot(INDEX start, INDEX length, const ConstantSP& exclude, const ConstantSP& out, INDEX outputStart = 0) const override;
	void neg() override{value_->neg();}
	void replace(const ConstantSP& oldVal, const ConstantSP& newVal) override{throw RuntimeException("Array vector doesn't support method replace");}
	void shuffle() override{throw RuntimeException("Array vector doesn't support method shuffle");}
	void reverse() override{reverse(0, size_);}
	void reverse(INDEX start, INDEX length) override;
	virtual bool rank(bool sorted, INDEX* indices, INDEX* ranking){
		throw RuntimeException("Array vector doesn't support method rank");
	}
	void contain(const ConstantSP& target, const ConstantSP& resultSP) const override {
		throw RuntimeException("Array vector doesn't support method contain");
	}
	void find(INDEX start, INDEX length, const ConstantSP& target, const ConstantSP& resultSP) override{
		throw RuntimeException("Array vector doesn't support method find");
	}
	void binarySearch(INDEX start, INDEX length, const ConstantSP& target, const ConstantSP& resultSP) override{
		throw RuntimeException("Array vector doesn't support method binarySearch");
	}
	void asof(INDEX start, INDEX length, const ConstantSP& target, const ConstantSP& resultSP) override{
		throw RuntimeException("Array vector doesn't support method asof");
	}
	bool findDuplicatedElements(Vector*  indices, INDEX start, INDEX length, vector<pair<INDEX,INDEX> >& duplicates) override{
		throw RuntimeException("Array vector doesn't support method findDuplicatedElements");
	}
	bool findDuplicatedElements(INDEX start, INDEX length, vector<pair<INDEX,INDEX> >& duplicates) override{
		throw RuntimeException("Array vector doesn't support method findDuplicatedElements");
	}
	bool findUniqueElements(INDEX start, INDEX length, vector<pair<INDEX,INDEX> >& uniques) override{
		throw RuntimeException("Array vector doesn't support method findUniqueElements");
	}
	bool findRange(INDEX* ascIndices,const ConstantSP& target,INDEX* targetIndices,vector<pair<INDEX,INDEX> >& ranges) override{
		throw RuntimeException("Array vector doesn't support method findRange");
	}
	bool findRange(const ConstantSP& target,INDEX* targetIndices,vector<pair<INDEX,INDEX> >& ranges) override{
		throw RuntimeException("Array vector doesn't support method findRange");
	}
	INDEX lowerBound(INDEX start, const ConstantSP& target) override{throw RuntimeException("Array vector doesn't support method lowerBound");}
	long long getAllocatedMemory() const override;
	bool isSorted(INDEX start, INDEX length, bool asc, bool strict, char nullOrders = 0) const override { return false;}
	using Vector::topK;
	virtual ConstantSP topK(INDEX start, INDEX length, INDEX top, bool asc, bool extendEqualValue) const {
		throw RuntimeException("Array vector doesn't support method topK");
	}
	int compare(INDEX indexLeft, INDEX indexRight) const override {throw RuntimeException("Array vector doesn't support method compare(indexLeft, indexRight)");}
	bool sort(bool asc, char nullOrders = 0) override {return false;}
	bool sort(bool asc, Vector* indices, char nullOrders = 0) override {return false;}
	bool sortSelectedIndices(Vector* indices, INDEX start, INDEX length, bool asc, char nullOrders = 0) override { return false;}

	/**
	 * Array vector serialization protocol
	 *
	 * An array vector split into multiple blocks. The format of one block is as follows.
	 *
	 * <BlockHeader> <Array of count> <Array of data>
	 *
	 * <BlockHeader> : 4 bytes = <2-byte RowCount><1-byte unit length of the array of count><1-byte reserved>
	 * <Array of count>: unit length * RowCount
	 * <Array of data>: byte array of serialized data
	 *
	 * Special case: if one row contains too many elements, breaks down to multiple blocks. The out parameter <partial>
	 * indicates how many cells of this row has been serialized. This parameter will be carried in by <offset> in next call.
	 */
	int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int& numElement, int& partial) const override;
	int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int targetNumElement, int& numElement, int& partial) const override;
	IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const override {throw RuntimeException("Array vector doesn't support method serialize");}
	IO_ERR deserialize(DataInputStream* in, INDEX indexStart, int offset, INDEX targetNumElement, INDEX& numElement, int& partial) override;

	ConstantSP flatten(INDEX rowStart, INDEX count) const override;
	ConstantSP rowSum(INDEX rowStart, INDEX count) const override;
	ConstantSP rowSum2(INDEX rowStart, INDEX count) const override;
	ConstantSP rowCount(INDEX rowStart, INDEX count) const override;
	ConstantSP rowSize(INDEX rowStart, INDEX count) const override;
	ConstantSP rowAvg(INDEX rowStart, INDEX count) const override;
	ConstantSP rowStd(INDEX rowStart, INDEX count) const override;
	ConstantSP rowStdp(INDEX rowStart, INDEX count) const override;
	ConstantSP rowVar(INDEX rowStart, INDEX count) const override;
	ConstantSP rowVarp(INDEX rowStart, INDEX count) const override;
	ConstantSP rowMin(INDEX rowStart, INDEX count) const override;
	ConstantSP rowMax(INDEX rowStart, INDEX count) const override;
	ConstantSP rowProd(INDEX rowStart, INDEX count) const override;
	ConstantSP rowAnd(INDEX rowStart, INDEX count) const override;
	ConstantSP rowOr(INDEX rowStart, INDEX count) const override;
	ConstantSP rowXor(INDEX rowStart, INDEX count) const override;
	ConstantSP rowMed(INDEX rowStart, INDEX count) const override;
	ConstantSP rowKurtosis(INDEX rowStart, INDEX count, bool biased) const override;
	ConstantSP rowSkew(INDEX rowStart, INDEX count, bool biased) const override;
	ConstantSP rowPercentile(INDEX rowStart, INDEX count, double percentile) const override;
	ConstantSP rowRank(INDEX rowStart, INDEX count, bool ascending, int groupNum, bool ignoreNA, int tiesMethod, bool percent) const override;
	ConstantSP rowDenseRank(INDEX rowStart, INDEX count, bool ascending, bool ignoreNA, bool percent) const override;

    //resize value_ vector to valueSize_+size, return old valueSize_
    int increaseValueVecSize(int size);

    // leave only the range [rowOffset, rowLimit) in the index
    static ConstantSP pruneIndexForRetrieval(const ConstantSP &index, INDEX rowOffset, INDEX rowLimit);

protected:
    friend class SubVector;
	ConstantSP sliceOneColumn(int colIndex, INDEX rowStart, INDEX rowEnd) const;
	/**
	 * colStart: inclusive
	 * colEnd: exclusive
	 * rowStart: inclusive
	 * rowEnd: exclusive
	 */
    ConstantSP sliceColumnRange(int colStart, int colEnd, INDEX rowStart, INDEX rowEnd) const;

private:
	inline void getRangeOfValueVector(INDEX start, INDEX length, INDEX& actualStart, INDEX& actualLength) const {
		INDEX* pindex = index_->getIndexArray();
		actualStart = start == 0 ? 0 : pindex[start-1];
		actualLength = (start + length == 0) ? 0 : (pindex[start + length - 1] - actualStart);
	}
	INDEX lowerBoundIndex(INDEX* data, INDEX size, INDEX start, INDEX value) const;

	int serializeFixedLength(char* buf, int bufSize, INDEX indexStart, int offset, int targetNumElement, int& numElement, int& partial) const;
	int serializeVariableLength(char* buf, int bufSize, INDEX indexStart, int offset, int targetNumElement, int& numElement, int& partial) const;
	IO_ERR deserializeFixedLength(DataInputStream* in, INDEX indexStart, int offset, INDEX targetNumElement, INDEX& numElement, int& partial);
	IO_ERR deserializeVariableLength(DataInputStream* in, INDEX indexStart, int offset, INDEX targetNumElement, INDEX& numElement, int& partial);

	ConstantSP sliceRows(INDEX offset, const ConstantSP& rowIndexVector) const;
	ConstantSP convertRowIndexToValueIndex(INDEX offset, const ConstantSP& rowIndexVector) const;
	VectorSP createBigArrayForValue(INDEX capacity) const;

private:
	bool containNull_;
	bool isBigArray_;
	INDEX thresholdForBigArray_;
	DATA_TYPE baseType_;
	INDEX baseUnitLength_;
	INDEX size_;
	INDEX valueSize_;
	INDEX valueCapacity_;
	VectorSP index_;
	VectorSP value_;

	/* Variables related to deserialization */
	unsigned char stage_; //0: block header, 1: array of count, 2:array of data
	unsigned char countBytes_; //1: unsigned char, 2: unsigned short, 4: unsigned int
	unsigned short rowCount_; // number of rows in this block
	int rowsRead_; //applicable when stage_ = 1 or 2
};

class ConstantIterator : public SysObj {
public:
	ConstantIterator(const ConstantSP& obj);
	ConstantIterator(Session* session, const DataInputStreamSP& in);
	~ConstantIterator() override{}
	ConstantSP next() override;
	ConstantSP getInstance() const override { return new ConstantIterator(obj_, curIndex_);}
	ConstantSP getValue() const override { return new ConstantIterator(obj_, curIndex_);}
	ConstantSP getIterator(const ConstantSP& self) const override;
	IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const override;
	string getString() const override;
	static ConstantSP createConstantIterator(Session* session, const DataInputStreamSP& in){
		return new ConstantIterator(session, in);
	}

protected:
	ConstantIterator(const ConstantSP& obj, INDEX curIndex);

private:
	ConstantSP obj_;
	INDEX curIndex_;
	INDEX end_;
};

class RangeIterator : public SysObj {
public:
	RangeIterator(const ConstantSP& range, long long step);
	RangeIterator(Session* session, const DataInputStreamSP& in);
	~RangeIterator() override{}
	ConstantSP next() override;
	ConstantSP getInstance() const override { return getValue();}
	ConstantSP getValue() const override;
	ConstantSP getIterator(const ConstantSP& self) const override;
	IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const override;
	string getString() const override;
	static ConstantSP createRangeIterator(Session* session, const DataInputStreamSP& in){
		return new RangeIterator(session, in);
	}

private:
	ConstantSP obj_;
	long long curValue_;
	long long endValue_;
	long long step_;
	int sign_;
};

class IotAnyVector : public Vector {
public:
	using Vector::getType;
	IotAnyVector(): Vector(DT_IOTANY, MIXED) {}
	IotAnyVector(size_t capacity): Vector(DT_IOTANY, MIXED) { index_.reserve(capacity); types_.reserve(capacity); }
	IotAnyVector(const IotAnyVector& rhs): Vector(DT_IOTANY, MIXED), types_(rhs.types_), index_(rhs.index_), containNull_(rhs.containNull_) {
		for (const auto& v: rhs.subVec_) {
			ConstantSP newVec = v.second->getValue(v.second->size());
			subVec_.insert({v.first, newVec});
		}
	}
	~IotAnyVector() override = default;

	VECTOR_TYPE getVectorType() const override {return VECTOR_TYPE::IOT_ANYVECTOR;}

	static bool supportType(DATA_TYPE t) {
		switch (t) {
		case DT_BOOL:
		case DT_CHAR:
		case DT_SHORT:
		case DT_INT:
		case DT_LONG:
		case DT_FLOAT:
		case DT_DOUBLE:
		case DT_STRING:
			return true;
		default:
			return false;
		}
	}

	static void checkSupportType(const ConstantSP& t) {
		if (!supportType(t->getType())) {
			throw RuntimeException("Unsupported data type " + std::to_string((int) t->getType()) + " for IotAnyVector.");
		}
	}

	bool equal(const ConstantSP& other) const override;
	bool containNotMarshallableObject() const override {
		for (const auto& vec: subVec_) {
			if (!vec.second->containNotMarshallableObject()) {
				return false;
			}
		}
		return true;
	}
	bool isLargeConstant() const override {return !isStatic() && !containNotMarshallableObject();}
	bool getNullFlag() const override {return containNull_;}
	void setNullFlag(bool containNull) override{containNull_=containNull;}
	INDEX getCapacity() const override {return 0;}
	bool isFastMode() const override {return false;}
	short getUnitLength() const override {return 0;}
	void clear() override {
		subVec_.clear();
		index_.clear();
		types_.clear();
		containNull_ = false;
	}
	bool sizeable() const override {return true;}
	DATA_TYPE getRawType() const override { return DT_IOTANY;}
	string getString(INDEX index) const override { return get(index)->getString(); }
	string getString(Heap* heap, INDEX index) const override { return get(index)->getString(heap); }
	const DolphinString& getStringRef(INDEX index) const override { throw RuntimeException("getStringRef method not supported for AnyVector");}
	bool set(INDEX index, const ConstantSP& value, INDEX valueIndex) override;
	bool set(INDEX index, const ConstantSP& value) override;
	bool set(const ConstantSP& index, const ConstantSP& value) override;
	bool setItem(INDEX index, const ConstantSP& value) override;
	bool assign(const ConstantSP& value) override;
	using Vector::get;
	ConstantSP get(INDEX index) const override;
	ConstantSP get(const ConstantSP& index) const override;
	ConstantSP get(INDEX offset, const ConstantSP& index) const override;
	bool hasNull() override { return hasNull(0, size()); }
	bool hasNull(INDEX start, INDEX length) override;
	bool isNull(INDEX index) const override;
	bool isNull() const override {return false;}
	void setNull(INDEX index) override { throw RuntimeException("setNull method not supported for IotAnyVector"); }
	void setNull() override{}
	bool setNonNull(const ConstantSP& index, const ConstantSP& value) override { throw RuntimeException("setNonNull method not supported for IotAnyVector"); }
	void fill(INDEX start, INDEX length, const ConstantSP& value, INDEX valueOffset = 0) override;
	void nullFill(const ConstantSP& val) override;
	bool isNull(INDEX start, int len, char* buf) const override;
	bool isNull(INDEX* indices, int len, char* buf) const override;
	bool isValid(INDEX start, int len, char* buf) const override;
	bool isValid(INDEX* indices, int len, char* buf) const override;
	ConstantSP getSubVector(INDEX start, INDEX length) const override;
	ConstantSP getInstance(INDEX size) const override {
		VectorSP ret = new IotAnyVector(size);
		if (size > 0) {
			ret->resize(size);
		}
		return ret;
	}
	ConstantSP getValue() const override;
	ConstantSP getValue(INDEX capacity) const override { return getValue(); }
	bool append(const ConstantSP& value, INDEX start, INDEX len) override;
	bool append(const ConstantSP& value) override {return append(value, 0, value->size());}
	bool append(const ConstantSP& value, INDEX count) override {return append(value, 0, count);}
	bool append(const ConstantSP& value, const ConstantSP& index) override {return append(value->get(index), 0, index->size());}

	bool remove(INDEX count) override;
	void find(INDEX start, INDEX length, const ConstantSP& targetSP, const ConstantSP& resultSP) override{
		throw RuntimeException("find method not supported for AnyVector");
	}
	char getBool() const override;
	char getChar() const override;
	short getShort() const override;
	int getInt() const override;
	long long getLong() const override;
	INDEX getIndex() const override;
	float getFloat() const override;
	double getDouble() const override;
	char getBool(INDEX index) const override {return get(index)->getBool();}
	char getChar(INDEX index) const override { return get(index)->getChar();}
	short getShort(INDEX index) const override { return get(index)->getShort();}
	int getInt(INDEX index) const override {return get(index)->getInt();}
	long long getLong(INDEX index) const override {return get(index)->getLong();}
	INDEX getIndex(INDEX index) const override {return get(index)->getIndex();}
	float getFloat(INDEX index) const override {return get(index)->getFloat();}
	double getDouble(INDEX index) const override {return get(index)->getDouble();}
	bool getBool(INDEX start, int len, char* buf) const override;
	bool getChar(INDEX start, int len,char* buf) const override;
	bool getShort(INDEX start, int len, short* buf) const override;
	bool getInt(INDEX start, int len, int* buf) const override;
	bool getLong(INDEX start, int len, long long* buf) const override;
	bool getIndex(INDEX start, int len, INDEX* buf) const override;
	bool getFloat(INDEX start, int len, float* buf) const override;
	bool getDouble(INDEX start, int len, double* buf) const override;
	const char* getBoolConst(INDEX start, int len, char* buf) const override;
	const char* getCharConst(INDEX start, int len,char* buf) const override;
	const short* getShortConst(INDEX start, int len, short* buf) const override;
	const int* getIntConst(INDEX start, int len, int* buf) const override;
	const long long* getLongConst(INDEX start, int len, long long* buf) const override;
	const INDEX* getIndexConst(INDEX start, int len, INDEX* buf) const override;
	const float* getFloatConst(INDEX start, int len, float* buf) const override;
	const double* getDoubleConst(INDEX start, int len, double* buf) const override;
	bool getSymbol(INDEX start, int len, int* buf, SymbolBase* symBase, bool insertIfNotThere) const override {
		throw RuntimeException("getSymbol method not supported for AnyVector");
	}
	const int* getSymbolConst(INDEX start, int len, int* buf, SymbolBase* symBase, bool insertIfNotThere) const override {
		throw RuntimeException("getSymbolConst method not supported for AnyVector");
	}
	using Vector::getString;
	bool getString(INDEX start, int len, DolphinString** buf) const override {
		throw RuntimeException("getString method not supported for AnyVector");
	}

	bool getString(INDEX start, int len, char** buf) const override {
		throw RuntimeException("getString method not supported for AnyVector");
	}

	DolphinString** getStringConst(INDEX start, int len, DolphinString** buf) const override {
		throw RuntimeException("getStringConst method not supported for AnyVector");
	}

	char** getStringConst(INDEX start, int len, char** buf) const override {
		throw RuntimeException("getStringConst method not supported for AnyVector");
	}

	INDEX reserve(INDEX capacity) override {
		index_.reserve(capacity);
		types_.reserve(capacity);
		return capacity;
	}
	void resize(INDEX size) override;

public:  /// getDecimal{32,64,128}
	// virtual int getDecimal32(int scale) const override;
	// virtual long long getDecimal64(int scale) const override;
	// virtual int128 getDecimal128(int scale) const override;

	// virtual int getDecimal32(INDEX index, int scale) const override;
	// virtual long long getDecimal64(INDEX index, int scale) const override;
	// virtual int128 getDecimal128(INDEX index, int scale) const override;

	// virtual bool getDecimal32(INDEX start, int len, int scale, int *buf) const override;
	// virtual bool getDecimal64(INDEX start, int len, int scale, long long *buf) const override;
	// virtual bool getDecimal128(INDEX start, int len, int scale, int128 *buf) const override;

	// virtual const int* getDecimal32Const(INDEX start, int len, int scale, int *buf) const override;
	// virtual const long long* getDecimal64Const(INDEX start, int len, int scale, long long *buf) const override;
	// virtual const int128* getDecimal128Const(INDEX start, int len, int scale,
	// 		int128 *buf) const override;

public:
	INDEX size() const override {return index_.size();}
	long long count() const override{
		return count(0, size());
	}
	long long count(INDEX start, INDEX length) const override;
	/**
	 * @param rightMost If there are multiple maximum/minimum values, choose the last one if `rightMost` is true.
	 */
	INDEX imax(bool rightMost = false) const override {throw RuntimeException("imax method not supported for AnyVector");}
	INDEX imin(bool rightMost = false) const override {throw RuntimeException("imin method not supported for AnyVector");}
	INDEX imax(INDEX start, INDEX length, bool rightMost = false) const override {throw RuntimeException("imax method not supported for AnyVector");}
	INDEX imin(INDEX start, INDEX length, bool rightMost = false) const override {throw RuntimeException("imin method not supported for AnyVector");}

	void prev(INDEX steps) override {throw RuntimeException("prev method not supported for IotAnyVector");}
	void next(INDEX steps) override {throw RuntimeException("next method not supported for IotAnyVector");}

	ConstantSP avg() const override { return avg(0, size()); }
	ConstantSP avg(INDEX start, INDEX length) const override;
	void avg(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override { out->set(outputStart, avg(start, length)); }
	ConstantSP sum() const override { return sum(0, size()); }
	ConstantSP sum(INDEX start, INDEX length) const override;
	void sum(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override { out->set(outputStart, sum(start, length)); }
	ConstantSP sum2() const override {throw RuntimeException("sum2 method not supported for IotAnyVector");}
	ConstantSP sum2(INDEX start, INDEX length) const override {throw RuntimeException("sum2 method not supported for IotAnyVector");}
	void sum2(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override {throw RuntimeException("sum2 method not supported for IotAnyVector");}
	ConstantSP prd() const override {throw RuntimeException("prd method not supported for IotAnyVector");}
	ConstantSP prd(INDEX start, INDEX length) const override {throw RuntimeException("prd method not supported for IotAnyVector");}
	void prd(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override {throw RuntimeException("prd method not supported for IotAnyVector");}
	ConstantSP var() const override {throw RuntimeException("var method not supported for IotAnyVector");}
	ConstantSP var(INDEX start, INDEX length) const override {throw RuntimeException("var method not supported for IotAnyVector");}
	void var(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override {throw RuntimeException("var method not supported for IotAnyVector");}
	ConstantSP std() const override {throw RuntimeException("std method not supported for IotAnyVector");}
	ConstantSP std(INDEX start, INDEX length) const override {throw RuntimeException("std method not supported for IotAnyVector");}
	void std(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override {throw RuntimeException("std method not supported for IotAnyVector");}
	ConstantSP median() const override {throw RuntimeException("median method not supported for IotAnyVector");}
	ConstantSP median(INDEX start, INDEX length) const override {throw RuntimeException("median method not supported for IotAnyVector");}
	void median(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart=0) const override {throw RuntimeException("median method not supported for IotAnyVector");}

	ConstantSP firstNot(const ConstantSP& exclude) const override {throw RuntimeException("firstNot method not supported for IotAnyVector");}
	ConstantSP firstNot(INDEX start, INDEX length, const ConstantSP& exclude) const override {throw RuntimeException("firstNot method not supported for IotAnyVector");}
	void firstNot(INDEX start, INDEX length, const ConstantSP& exclude, const ConstantSP& out, INDEX outputStart = 0) const override {throw RuntimeException("firstNot method not supported for IotAnyVector");}
	ConstantSP lastNot(const ConstantSP& exclude) const override {throw RuntimeException("lastNot method not supported for IotAnyVector");}
	ConstantSP lastNot(INDEX start, INDEX length, const ConstantSP& exclude) const override {throw RuntimeException("lastNot method not supported for IotAnyVector");}
	void lastNot(INDEX start, INDEX length, const ConstantSP& exclude, const ConstantSP& out, INDEX outputStart = 0) const override {throw RuntimeException("lastNot method not supported for IotAnyVector");}
	ConstantSP searchK(INDEX k) const override {throw RuntimeException("searchK method not supported for AnyVector");}
	ConstantSP searchK(INDEX start, INDEX length, INDEX k) const override {throw RuntimeException("searchK method not supported for IotAnyVector");}
	void searchK(INDEX start, INDEX length, INDEX k, const ConstantSP& out, INDEX outputStart=0) const override {throw RuntimeException("searchK method not supported for IotAnyVector");}
	ConstantSP mode() const override {throw RuntimeException("mode method not supported for AnyVector");}
	ConstantSP mode(INDEX start, INDEX length) const override {throw RuntimeException("mode method not supported for IotAnyVector");}
	void mode(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart=0) const override {
		throw RuntimeException("mode method not supported for AnyVector");
	}
	ConstantSP min() const override { return min(0, size()); }
	ConstantSP min(INDEX start, INDEX length) const override;
	void min(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart=0) const override {out->set(outputStart, min(start, length));}
	ConstantSP max() const override { return max(0, size()); }
	ConstantSP max(INDEX start, INDEX length) const override;
	void max(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart=0) const override {out->set(outputStart, max(start, length));}
	void neg() override{throw RuntimeException("neg method not supported for IotAnyVector");}
	void reverse() override{std::reverse(index_.begin(),index_.end());}
	void reverse(INDEX start, INDEX length) override{
		std::reverse(index_.begin()+start,index_.begin()+ start + length);
	}
	void replace(const ConstantSP& oldVal, const ConstantSP& newVal) override { throw RuntimeException("replace method not supported for IotAnyVector"); }
	void shuffle() override;
	bool findDuplicatedElements(Vector* indices, INDEX start, INDEX length, vector<pair<INDEX,INDEX> >& duplicates) override{return false;}
	bool findDuplicatedElements(INDEX start, INDEX length, vector<pair<INDEX,INDEX> >& duplicates) override{return false;}
	bool findUniqueElements(INDEX start, INDEX length, vector<pair<INDEX,INDEX> >& uniques) override{return false;}
	bool findRange(INDEX* ascIndices,const ConstantSP& target,INDEX* targetIndices,vector<pair<INDEX,INDEX> >& ranges) override{return false;}
	bool findRange(const ConstantSP& target,INDEX* targetIndices,vector<pair<INDEX,INDEX> >& ranges) override{return false;}
	INDEX lowerBound(INDEX start, const ConstantSP& target) override{
		throw RuntimeException("lowerBound method not supported for AnyVector");
	}
	virtual bool rank(bool sorted, INDEX* indices, INDEX* ranking){return false;}
	bool sortSelectedIndices(Vector* indices, INDEX start, INDEX length, bool asc, char nullsOrder) override{	return false;}
	bool isSorted(INDEX start, INDEX length, bool asc, bool strict, char nullsOrder) const override { return false;}
    int compare(INDEX indexLeft, INDEX indexRight) const override {
        if (types_[indexLeft] != types_[indexRight]) {
            throw RuntimeException("Comparing two values in IotAnyVector with different types is not supported.");
        } else {
            const auto typeIndex = types_[indexLeft];
            const auto &vec = subVec_.at(typeIndex);
            return vec->compare(index_[indexLeft], index_[indexRight]);
		}
	}
	bool sort(bool asc, char nullsOrder) override{return false;}
	bool sort(bool asc, Vector* indices, char nullsOrder) override{ return false;}
	INDEX sortTop(bool asc, Vector* indices, INDEX top, char nullsOrder) override{ return -1;}
	long long getAllocatedMemory() const override;
	// virtual int getExtraParamForType() const override { return dt_; }
	using Vector::serialize;
	IO_ERR serialize(Heap* pHeap, const ByteArrayCodeBufferSP& buffer) const override;

	int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int& numElement, int& partial) const override {
		throw RuntimeException("the vector doesn't support that serialize method, use ConstantMarshal to serialize");
	}
	IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const override {
		return serialize(nullptr, buffer);
	}

	void collectUserDefinedFunctions(unordered_map<string,FunctionDef*>& functionDefs) const override;
	ConstantSP convertToRegularVector() const;

	// group elements by type
	struct TypeGroup {
		size_t start;
		size_t length;
		DATA_TYPE type;
	};
	vector<TypeGroup> findUniqueTypesElements(INDEX start, INDEX length) const;

    static ObjectPtr<IotAnyVector> cast(const VectorSP& vec) {
        if (vec->getType() != DT_IOTANY) {
            return nullptr;
        }
        if (vec->getVectorType() != VECTOR_TYPE::IOT_ANYVECTOR) {
            return ((Constant *) vec.get())->getValue();
        }
        return vec;
    }

	DATA_TYPE getType(INDEX i) const {
		return types_[i];
	}

	ConstantSP getSerializedObject() const;
	IO_ERR fromSerializedObject(const ConstantSP& object);

	bool isContinuousRange(INDEX start, INDEX length) const;

	// get sub vector
	VectorSP getSubVec(DATA_TYPE t) const {
		if (t == DT_VOID) {
			return nullptr;
		}
		auto it = subVec_.find(t);
		if (it == subVec_.end()) {
			return nullptr;
		} else {
			return it->second;
		}
	}

	const vector<INDEX>& getInternalIdx() const { return index_; }

protected:
	void getByIndex(INDEX* idx, size_t len, INDEX idxOffset, IotAnyVector* output) const ;

	VectorSP getSubVec(DATA_TYPE t) {
		if (t == DT_VOID) {
			return nullptr;
		}
		auto it = subVec_.find(t);
		if (it == subVec_.end()) {
			VectorSP newVec = Util::createVector(t, 0);
			subVec_.insert({t, newVec});
			return newVec;
		} else {
			return it->second;
		}
	}
	std::map<DATA_TYPE, VectorSP> subVec_;
	std::vector<DATA_TYPE> types_;
	std::vector<INDEX> index_;
	bool containNull_ = false;
	SymbolBaseSP sym_;
};
} // namespace ddb
#endif /* SPECIALCONSTANT_H_ */
