/*
 * SpecialConstant.h
 *
 *  Created on: Mar 22, 2021
 *      Author: dzhou
 */

#ifndef SPECIALCONSTANT_H_
#define SPECIALCONSTANT_H_

#include <map>

#include "CoreConcept.h"
#include "Util.h"

class AnyVector:public Vector{
public:
	AnyVector(int size, bool isTableColumn = false, DATA_TYPE dt = DT_VOID, int decimalExtra = -1);

	AnyVector(const deque<ConstantSP>& data, bool containNull, bool isTableColumn = false, DATA_TYPE dt = DT_VOID,
			int decimalExtra = -1);

	AnyVector(const vector<ConstantSP>& data, bool containNull, bool isTableColumn = false, DATA_TYPE dt = DT_VOID,
			int decimalExtra = -1);

	virtual ~AnyVector(){}
	virtual bool containNotMarshallableObject() const;
	virtual bool isLargeConstant() const {return !isStatic() && !containNotMarshallableObject();}
	virtual bool getNullFlag() const {return containNull_;}
	virtual void setNullFlag(bool containNull){containNull_=containNull;}
	virtual INDEX getCapacity() const {return 0;}
	virtual bool isFastMode() const {return false;}
	virtual short getUnitLength() const {return 0;}
	virtual void clear();
	virtual bool sizeable() const {return true;}
	virtual DATA_TYPE getRawType() const { return DT_ANY;}
	virtual string getString(INDEX index) const {return data_[index]->getString();}
	virtual const DolphinString& getStringRef(INDEX index) const { throw RuntimeException("getStringRef method not supported for AnyVector");}
	virtual bool set(INDEX index, const ConstantSP& value, INDEX valueIndex);
	virtual bool set(INDEX index, const ConstantSP& value);
	virtual bool set(const ConstantSP& index, const ConstantSP& value);
	virtual bool set(Heap* heap, const ConstantSP& index, const ConstantSP& value, int dim);
	virtual bool setItem(INDEX index, const ConstantSP& value);
	virtual bool modifyMember(Heap* heap, const FunctionDefSP& func, const ConstantSP& index, const ConstantSP& parameters, int dim);
	virtual bool assign(const ConstantSP& value);
	virtual ConstantSP get(INDEX index) const {return data_[index];}
	virtual ConstantSP get(const ConstantSP& index) const;
	virtual ConstantSP get(INDEX offset, const ConstantSP& index) const override;
	virtual bool hasNull(){return  hasNull(0, data_.size());}
	virtual bool hasNull(INDEX start, INDEX length);
	virtual bool isNull(INDEX index) const;
	virtual bool isNull() const {return false;}
	virtual void setNull(INDEX index);
	virtual void setNull(){}
	virtual void fill(INDEX start, INDEX length, const ConstantSP& value, INDEX valueOffset = 0) override;
	virtual void nullFill(const ConstantSP& val);
	virtual bool isNull(INDEX start, int len, char* buf) const override;
	virtual bool isNull(INDEX* indices, int len, char* buf) const override;
	virtual bool isValid(INDEX start, int len, char* buf) const override;
	virtual bool isValid(INDEX* indices, int len, char* buf) const override;
	virtual ConstantSP getSubVector(INDEX start, INDEX length) const;
	virtual ConstantSP getInstance(INDEX size) const {return ConstantSP(new AnyVector(size, isTableColumn_, dt_, decimalExtra_));}
	virtual ConstantSP getValue() const;
	virtual ConstantSP getValue(INDEX capacity) const {return ConstantSP(new AnyVector(data_, containNull_, isTableColumn_, dt_, decimalExtra_));}
	bool append(const ConstantSP& value, bool wholistic);
	virtual bool append(const ConstantSP& value);
	virtual bool append(const ConstantSP& value, INDEX appendSize);
	virtual bool remove(INDEX count);
	virtual bool remove(const ConstantSP& index) override;
	virtual void resize(INDEX size) override;
	virtual void prev(INDEX steps);
	virtual void next(INDEX steps);
	virtual void contain(const ConstantSP& targetSP, const ConstantSP& resultSP) const;
	virtual void find(INDEX start, INDEX length, const ConstantSP& targetSP, const ConstantSP& resultSP){
		throw RuntimeException("find method not supported for AnyVector");
	}
	virtual char getBool() const;
	virtual char getChar() const;
	virtual short getShort() const;
	virtual int getInt() const;
	virtual long long getLong() const;
	virtual INDEX getIndex() const;
	virtual float getFloat() const;
	virtual double getDouble() const;
	virtual char getBool(INDEX index) const {return get(index)->getBool();}
	virtual char getChar(INDEX index) const { return get(index)->getChar();}
	virtual short getShort(INDEX index) const { return get(index)->getShort();}
	virtual int getInt(INDEX index) const {return get(index)->getInt();}
	virtual long long getLong(INDEX index) const {return get(index)->getLong();}
	virtual INDEX getIndex(INDEX index) const {return get(index)->getIndex();}
	virtual float getFloat(INDEX index) const {return get(index)->getFloat();}
	virtual double getDouble(INDEX index) const {return get(index)->getDouble();}
	virtual IO_ERR serialize(Heap* pHeap, const ByteArrayCodeBufferSP& buffer) const;
	virtual int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int& numElement, int& partial) const {
		throw RuntimeException("serialize method not supported for AnyVector");
	}
	virtual bool getBool(INDEX start, int len, char* buf) const;
	virtual bool getChar(INDEX start, int len,char* buf) const;
	virtual bool getShort(INDEX start, int len, short* buf) const;
	virtual bool getInt(INDEX start, int len, int* buf) const;
	virtual bool getLong(INDEX start, int len, long long* buf) const;
	virtual bool getIndex(INDEX start, int len, INDEX* buf) const;
	virtual bool getFloat(INDEX start, int len, float* buf) const;
	virtual bool getDouble(INDEX start, int len, double* buf) const;
	virtual const char* getBoolConst(INDEX start, int len, char* buf) const;
	virtual const char* getCharConst(INDEX start, int len,char* buf) const;
	virtual const short* getShortConst(INDEX start, int len, short* buf) const;
	virtual const int* getIntConst(INDEX start, int len, int* buf) const;
	virtual const long long* getLongConst(INDEX start, int len, long long* buf) const;
	virtual const INDEX* getIndexConst(INDEX start, int len, INDEX* buf) const;
	virtual const float* getFloatConst(INDEX start, int len, float* buf) const;
	virtual const double* getDoubleConst(INDEX start, int len, double* buf) const;
	virtual bool getSymbol(INDEX start, int len, int* buf, SymbolBase* symBase, bool insertIfNotThere) const {
		throw RuntimeException("getSymbol method not supported for AnyVector");
	}
	virtual const int* getSymbolConst(INDEX start, int len, int* buf, SymbolBase* symBase, bool insertIfNotThere) const {
		throw RuntimeException("getSymbolConst method not supported for AnyVector");
	}
	virtual bool getString(INDEX start, int len, DolphinString** buf) const {
		throw RuntimeException("getString method not supported for AnyVector");
	}

	virtual bool getString(INDEX start, int len, char** buf) const {
		throw RuntimeException("getString method not supported for AnyVector");
	}

	virtual DolphinString** getStringConst(INDEX start, int len, DolphinString** buf) const {
		throw RuntimeException("getStringConst method not supported for AnyVector");
	}

	virtual char** getStringConst(INDEX start, int len, char** buf) const {
		throw RuntimeException("getStringConst method not supported for AnyVector");
	}

public:  /// getDecimal{32,64,128}
	virtual int getDecimal32(int scale) const override;
	virtual long long getDecimal64(int scale) const override;
	virtual int128 getDecimal128(int scale) const override;

	virtual int getDecimal32(INDEX index, int scale) const override;
	virtual long long getDecimal64(INDEX index, int scale) const override;
	virtual int128 getDecimal128(INDEX index, int scale) const override;

	virtual bool getDecimal32(INDEX start, int len, int scale, int *buf) const override;
	virtual bool getDecimal64(INDEX start, int len, int scale, long long *buf) const override;
	virtual bool getDecimal128(INDEX start, int len, int scale, int128 *buf) const override;

	virtual const int* getDecimal32Const(INDEX start, int len, int scale, int *buf) const override;
	virtual const long long* getDecimal64Const(INDEX start, int len, int scale, long long *buf) const override;
	virtual const int128* getDecimal128Const(INDEX start, int len, int scale,
			int128 *buf) const override;

public:
	virtual INDEX size() const {return data_.size();}
	virtual long long count() const{
		return count(0, data_.size());
	}
	virtual long long count(INDEX start, INDEX length) const;
	/**
	 * @param rightMost If there are multiple maximum/minimum values, choose the last one if `rightMost` is true.
	 */
	virtual INDEX imax(bool rightMost = false) const override {throw RuntimeException("imax method not supported for AnyVector");}
	virtual INDEX imin(bool rightMost = false) const override {throw RuntimeException("imin method not supported for AnyVector");}
	virtual INDEX imax(INDEX start, INDEX length, bool rightMost = false) const override {throw RuntimeException("imax method not supported for AnyVector");}
	virtual INDEX imin(INDEX start, INDEX length, bool rightMost = false) const override {throw RuntimeException("imin method not supported for AnyVector");}

	virtual ConstantSP avg() const {return avg(0, data_.size());}
	virtual ConstantSP avg(INDEX start, INDEX length) const;
	virtual void avg(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const { out->set(outputStart, avg(start, length)); }
	virtual ConstantSP sum() const {return sum(0, data_.size());}
	virtual ConstantSP sum(INDEX start, INDEX length) const;
	virtual void sum(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const { out->set(outputStart, sum(start, length)); }
	virtual ConstantSP sum2() const {return sum2(0, data_.size());}
	virtual ConstantSP sum2(INDEX start, INDEX length) const;
	virtual void sum2(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const { out->set(outputStart, sum2(start, length)); }
	virtual ConstantSP prd() const {return prd(0, data_.size());}
	virtual ConstantSP prd(INDEX start, INDEX length) const;
	virtual void prd(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const { out->set(outputStart, prd(start, length)); }
	virtual ConstantSP var() const {return var(0, data_.size());}
	virtual ConstantSP var(INDEX start, INDEX length) const;
	virtual void var(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const { out->set(outputStart, var(start, length)); }
	virtual ConstantSP std() const {return std(0, data_.size());}
	virtual ConstantSP std(INDEX start, INDEX length) const;
	virtual void std(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const { out->set(outputStart, std(start, length)); }
	virtual ConstantSP median() const {return median(0, data_.size());}
	virtual ConstantSP median(INDEX start, INDEX length) const;
	virtual void median(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart=0) const { out->set(outputStart, median(start, length)); }

	virtual ConstantSP firstNot(const ConstantSP& exclude) const { return firstNot(0, data_.size(), exclude); }
	virtual ConstantSP firstNot(INDEX start, INDEX length, const ConstantSP& exclude) const;
	virtual void firstNot(INDEX start, INDEX length, const ConstantSP& exclude, const ConstantSP& out, INDEX outputStart = 0) const {
		out->set(outputStart, firstNot(start, length, exclude));
	}
	virtual ConstantSP lastNot(const ConstantSP& exclude) const { return lastNot(0, data_.size(), exclude); }
	virtual ConstantSP lastNot(INDEX start, INDEX length, const ConstantSP& exclude) const;
	virtual void lastNot(INDEX start, INDEX length, const ConstantSP& exclude, const ConstantSP& out, INDEX outputStart = 0) const {
		out->set(outputStart, lastNot(start, length, exclude));
	}
	virtual ConstantSP searchK(INDEX k) const {throw RuntimeException("searchK method not supported for AnyVector");}
	virtual ConstantSP searchK(INDEX start, INDEX length, INDEX k) const {return searchK(k);}
	virtual void searchK(INDEX start, INDEX length, INDEX k, const ConstantSP& out, INDEX outputStart=0) const {
		throw RuntimeException("searchK method not supported for AnyVector");
	}
	virtual ConstantSP mode() const {throw RuntimeException("mode method not supported for AnyVector");}
	virtual ConstantSP mode(INDEX start, INDEX length) const { return mode();}
	virtual void mode(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart=0) const {
		throw RuntimeException("mode method not supported for AnyVector");
	}
	virtual ConstantSP min() const { return min(0, data_.size()); }
	virtual ConstantSP min(INDEX start, INDEX length) const;
	virtual void min(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart=0) const { out->set(outputStart, min(start, length)); }
	virtual ConstantSP max() const { return max(0, data_.size()); }
	virtual ConstantSP max(INDEX start, INDEX length) const;
	virtual void max(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart=0) const { out->set(outputStart, max(start, length)); }

	virtual void neg(){throw RuntimeException("neg method not supported for AnyVector");}
	virtual void reverse(){std::reverse(data_.begin(),data_.end());}
	virtual void reverse(INDEX start, INDEX length){
		std::reverse(data_.begin()+start,data_.begin()+ start + length);
	}
	virtual void replace(const ConstantSP& oldVal, const ConstantSP& newVal);
	virtual void shuffle();
	virtual bool findDuplicatedElements(Vector* indices, INDEX start, INDEX length, vector<pair<INDEX,INDEX> >& duplicates){return false;}
	virtual bool findDuplicatedElements(INDEX start, INDEX length, vector<pair<INDEX,INDEX> >& duplicates){return false;}
	virtual bool findUniqueElements(INDEX start, INDEX length, vector<pair<INDEX,INDEX> >& uniques){return false;}
	virtual bool findRange(INDEX* ascIndices,const ConstantSP& target,INDEX* targetIndices,vector<pair<INDEX,INDEX> >& ranges){return false;}
	virtual bool findRange(const ConstantSP& target,INDEX* targetIndices,vector<pair<INDEX,INDEX> >& ranges){return false;}
	virtual INDEX lowerBound(INDEX start, const ConstantSP& target){
		throw RuntimeException("lowerBound method not supported for AnyVector");
	}
	virtual bool rank(bool sorted, INDEX* indices, INDEX* ranking){return false;}
	virtual bool sortSelectedIndices(Vector* indices, INDEX start, INDEX length, bool asc, char nullsOrder){	return false;}
	virtual bool isSorted(INDEX start, INDEX length, bool asc, bool strict, char nullsOrder) const { return false;}
	virtual bool sort(bool asc, char nullsOrder){return false;}
	virtual bool sort(bool asc, Vector* indices, char nullsOrder){ return false;}
	virtual INDEX sortTop(bool asc, Vector* indices, INDEX top, char nullsOrder){ return -1;}
	virtual long long getAllocatedMemory();
	virtual int getExtraParamForType() const override { return dt_; }

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

	void collectUserDefinedFunctions(unordered_map<string,FunctionDef*>& functionDefs) const;
	bool isHomogeneousScalar(DATA_TYPE& type) const;
	bool isHomogeneousScalarOrArray(DATA_TYPE& type, int& decimalExtra) const;
	bool isConsistent() const;
	bool isConsistentArray(int& len) const;
	bool isTabular() const;
	ConstantSP convertToRegularVector() const;
	bool isDimension() const { return isDim_;}
	void setDimension(bool option) { isDim_ = option;}
	virtual bool isTableColumn() const override { return isTableColumn_;}
	void setTableColumn(bool option) { isTableColumn_ = option;}
	void setExtraParamForType(int extra){
		DATA_TYPE type = (DATA_TYPE)extra;
		if(type != DT_ANY)
			dt_ = type;
	}
	void setDecimalExtra(int extra){ decimalExtra_ = extra; }
	int getDecimalExtra() { return decimalExtra_; }
	INDEX reserve(INDEX capacity);
	inline long long getAllocatedMemory() const override {
		long long memSize = 0;
		for (size_t i = 0; i < data_.size(); ++i) {
			memSize += data_[i]->getAllocatedMemory();
		}
		return memSize;
	}
	const ConstantSP& getConstant(INDEX index) const { return data_[index];}
	void toVector(vector<ConstantSP>& v) const {
		if(!v.empty())
			v.clear();
		v.insert(v.begin(), data_.begin(), data_.end());
	}
	virtual IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const {
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
		buffer->write((char)CONSTOBJ);
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
	static SmartPointer<AnyVector> toAnyVector(const VectorSP& v) {
		if (LIKELY(!v->isView()))
			return v;
		// v may be a SubVector or SlicedVector
		return ((Constant*)v.get())->getValue();
	}

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
	mutable deque<ConstantSP> data_;
	bool containNull_;
	bool isDim_;
	bool isTableColumn_;
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
	virtual ~SlicedVector(){}
	virtual VECTOR_TYPE getVectorType() const {return VECTOR_TYPE::SLICEDVECTOR;}
	VectorSP getSourceVector() const { return source_;}
	VectorSP getIndexVector() const { return index_;}
	void reset(const VectorSP& source, const VectorSP& index);
	virtual bool copyable() const {return false;}
	virtual bool isView() const {return true;}
	virtual INDEX size() const { return size_;}
	virtual DATA_TYPE getRawType() const {return source_->getRawType();}
	virtual int getExtraParamForType() const { return source_->getExtraParamForType();}
	virtual SymbolBaseSP getSymbolBase() const {return source_->getSymbolBase();}
	virtual DATA_FORM getForm() const { return DF_VECTOR;}
	virtual ConstantSP getInstance() const {return source_->getInstance(size_);}
	virtual ConstantSP getInstance(INDEX size) const {return source_->getInstance(size);}
	virtual ConstantSP getValue() const { return ((Constant*)source_.get())->get(index_);}
	virtual ConstantSP getValue(INDEX capacity) const;
	virtual ConstantSP getWindow(INDEX colStart, int colLength, INDEX rowStart, int rowLength) const;
	virtual ConstantSP getSubVector(INDEX start, INDEX length, INDEX capacity) const;
	virtual ConstantSP getSubVector(INDEX start, INDEX length) const;
	virtual void fill(INDEX start, INDEX length, const ConstantSP& value, INDEX valueOffset = 0) override{throw RuntimeException("Sliced vector doesn't support method fill");}
	virtual bool validIndex(INDEX uplimit){throw RuntimeException("Sliced vector doesn't support method validIndex");}
	virtual bool validIndex(INDEX start, INDEX length, INDEX uplimit){throw RuntimeException("Sliced vector doesn't support method validIndex");}
	virtual int compare(INDEX index, const ConstantSP& target) const {return source_->compare(pindex_[index], target);}
	virtual bool getNullFlag() const {return source_->getNullFlag();}
	virtual void setNullFlag(bool containNull){}
	virtual bool hasNull(){return hasNull(0, size_);}
	virtual bool hasNull(INDEX start, INDEX length);
	virtual INDEX getCapacity() const {return size_;}
	virtual bool isFastMode() const { return source_->isFastMode();}
	virtual void* getDataArray() const { return nullptr;}
	virtual bool isIndexArray() const { return false;}
	virtual INDEX* getIndexArray() const { return nullptr;}
	virtual short getUnitLength() const {return source_->getUnitLength();}
	virtual void** getDataSegment() const {return nullptr;}
	virtual bool sizeable() const {return false;}
	virtual char getBool() const {return source_->getBool(pindex_[0]);}
	virtual char getChar() const { return source_->getChar(pindex_[0]);}
	virtual short getShort() const { return source_->getShort(pindex_[0]);}
	virtual int getInt() const {return source_->getInt(pindex_[0]);}
	virtual long long getLong() const {return source_->getLong(pindex_[0]);}
	virtual INDEX getIndex() const {return source_->getIndex(pindex_[0]);}
	virtual float getFloat() const {return source_->getFloat(pindex_[0]);}
	virtual double getDouble() const {return source_->getDouble(pindex_[0]);}
	virtual ConstantSP get(INDEX index) const { return source_->get(pindex_[index]);}
	virtual ConstantSP get(const ConstantSP& index) const;
	virtual ConstantSP get(INDEX offset, const ConstantSP& index) const;
	virtual void setBool(bool val){throw RuntimeException("Sliced vector doesn't support method setBool");}
	virtual void setChar(char val){throw RuntimeException("Sliced vector doesn't support method setChar");}
	virtual void setShort(short val){throw RuntimeException("Sliced vector doesn't support method setShort");}
	virtual void setInt(int val){throw RuntimeException("Sliced vector doesn't support method setInt");}
	virtual void setLong(long long val){throw RuntimeException("Sliced vector doesn't support method setLong");}
	virtual void setIndex(INDEX val){throw RuntimeException("Sliced vector doesn't support method setIndex");}
	virtual void setFloat(float val){throw RuntimeException("Sliced vector doesn't support method setFloat");}
	virtual void setDouble(double val){throw RuntimeException("Sliced vector doesn't support method setDouble");}
	virtual void setString(const string& val){throw RuntimeException("Sliced vector doesn't support method setString");}
	virtual void setNull(){throw RuntimeException("Sliced vector doesn't support method setNull");}
	virtual char getBool(INDEX index) const {return source_->getBool(pindex_[index]);}
	virtual char getChar(INDEX index) const { return source_->getChar(pindex_[index]);}
	virtual short getShort(INDEX index) const { return source_->getShort(pindex_[index]);}
	virtual int getInt(INDEX index) const {return source_->getInt(pindex_[index]);}
	virtual long long getLong(INDEX index) const {return source_->getLong(pindex_[index]);}
	virtual INDEX getIndex(INDEX index) const {return source_->getIndex(pindex_[index]);}
	virtual float getFloat(INDEX index) const {return source_->getFloat(pindex_[index]);}
	virtual double getDouble(INDEX index) const {return source_->getDouble(pindex_[index]);}
	virtual bool isNull(INDEX index) const {return source_->isNull(pindex_[index]);}
	virtual string getString(INDEX index) const {return source_->getString(pindex_[index]);}
	virtual const DolphinString& getStringRef(INDEX index) const {return source_->getStringRef(pindex_[index]);}
	virtual void clear(){throw RuntimeException("Sliced vector doesn't support method clear");}
	virtual bool remove(INDEX count){throw RuntimeException("Sliced vector doesn't support method remove");}
	virtual bool remove(const ConstantSP& index){throw RuntimeException("Indexed vector doesn't support method remove");}
	virtual void next(INDEX steps){throw RuntimeException("Sliced vector doesn't support method next");}
	virtual void prev(INDEX steps){throw RuntimeException("Sliced vector doesn't support method prev");}
	virtual void nullFill(const ConstantSP& val){throw RuntimeException("Sliced vector doesn't support method nullFill");}
	virtual bool isNull(INDEX start, int len, char* buf) const { return source_->isNull(pindex_ + start, len, buf);}
	virtual bool isValid(INDEX start, int len, char* buf) const {return source_->isValid(pindex_ + start, len, buf);}
	virtual bool getBool(INDEX start, int len, char* buf) const {return source_->getBool(pindex_ + start, len, buf);}
	virtual const char* getBoolConst(INDEX start, int len, char* buf) const {
		source_->getBool(pindex_ + start, len, buf);
		return buf;
	}
	virtual char* getBoolBuffer(INDEX start, int len, char* buf) const {return buf;}
	virtual bool getChar(INDEX start, int len, char* buf) const {return source_->getChar(pindex_ + start, len, buf);}
	virtual const char* getCharConst(INDEX start, int len, char* buf) const {
		source_->getChar(pindex_ + start, len, buf);
		return buf;
	}
	virtual char* getCharBuffer(INDEX start, int len, char* buf) const {return buf;}
	virtual bool getShort(INDEX start, int len, short* buf) const {return source_->getShort(pindex_ + start, len, buf);}
	virtual const short* getShortConst(INDEX start, int len, short* buf) const {
		source_->getShort(pindex_ + start, len, buf);
		return buf;
	}
	virtual short* getShortBuffer(INDEX start, int len, short* buf) const {return buf;}
	virtual bool getInt(INDEX start, int len, int* buf) const {return source_->getInt(pindex_ + start, len, buf);}
	virtual const int* getIntConst(INDEX start, int len, int* buf) const {
		source_->getInt(pindex_ + start, len, buf);
		return buf;
	}
	virtual int* getIntBuffer(INDEX start, int len, int* buf) const {return buf;}
	virtual bool getLong(INDEX start, int len, long long* buf) const {return source_->getLong(pindex_ + start, len, buf);}
	virtual const long long* getLongConst(INDEX start, int len, long long* buf) const {
		source_->getLong(pindex_ + start, len, buf);
		return buf;
	}
	virtual long long* getLongBuffer(INDEX start, int len, long long* buf) const {return buf;}
	virtual bool getBinary(INDEX start, int len, int unitLength, unsigned char* buf) const {return source_->getBinary(pindex_ + start, len, unitLength, buf);}
	virtual const unsigned char* getBinaryConst(INDEX start, int len, int unitLength, unsigned char* buf) const {
		source_->getBinary(pindex_ + start, len, unitLength, buf);
		return buf;
	}
	virtual unsigned char* getBinaryBuffer(INDEX start, int len, int unitLength, unsigned char* buf) const {return buf;}
	virtual bool getIndex(INDEX start, int len, INDEX* buf) const {return source_->getIndex(pindex_ + start, len, buf);}
	virtual const INDEX* getIndexConst(INDEX start, int len, INDEX* buf) const {
		source_->getIndex(pindex_ + start, len, buf);
		return buf;
	}
	virtual INDEX* getIndexBuffer(INDEX start, int len, INDEX* buf) const {return buf;}
	virtual bool getFloat(INDEX start, int len, float* buf) const {return source_->getFloat(pindex_ + start, len, buf);}
	virtual const float* getFloatConst(INDEX start, int len, float* buf) const {
		source_->getFloat(pindex_ + start, len, buf);
		return buf;
	}
	virtual float* getFloatBuffer(INDEX start, int len, float* buf) const {return buf;}
	virtual bool getDouble(INDEX start, int len, double* buf) const {return source_->getDouble(pindex_ + start, len, buf);}
	virtual const double* getDoubleConst(INDEX start, int len, double* buf) const {
		source_->getDouble(pindex_ + start, len, buf);
		return buf;
	}
	virtual double* getDoubleBuffer(INDEX start, int len, double* buf) const {return buf;}
	virtual bool getString(INDEX start, int len, char** buf) const {return ((Constant*)source_.get())->getString(pindex_ + start, len, buf);}
	virtual char** getStringConst(INDEX start, int len, char** buf) const {
		((Constant*)source_.get())->getString(pindex_ + start, len, buf);
		return buf;
	}
	virtual bool getString(INDEX start, int len, DolphinString** buf) const {return ((Constant*)source_.get())->getString(pindex_ + start, len, buf);}
	virtual DolphinString** getStringConst(INDEX start, int len, DolphinString** buf) const {
		((Constant*)source_.get())->getString(pindex_ + start, len, buf);
		return buf;
	}
	virtual const int* getSymbolConst(INDEX start, int len, int* buf, SymbolBase* symBase, bool insertIfNotThere) const {
		source_->getSymbol(pindex_ + start, len, buf, symBase, insertIfNotThere);
		return buf;
	}
	virtual bool getSymbol(INDEX start, int len, int* buf, SymbolBase* symBase, bool insertIfNotThere) const { return source_->getSymbol(pindex_ + start, len, buf, symBase, insertIfNotThere);}

	virtual bool isNull(INDEX* indices, int len, char* buf) const;
	virtual bool isValid(INDEX* indices, int len, char* buf) const;
	virtual bool getBool(INDEX* indices, int len, char* buf) const;
	virtual bool getChar(INDEX* indices, int len,char* buf) const;
	virtual bool getShort(INDEX* indices, int len, short* buf) const;
	virtual bool getInt(INDEX* indices, int len, int* buf) const;
	virtual bool getLong(INDEX* indices, int len, long long* buf) const;
	virtual bool getIndex(INDEX* indices, int len, INDEX* buf) const;
	virtual bool getFloat(INDEX* indices, int len, float* buf) const;
	virtual bool getDouble(INDEX* indices, int len, double* buf) const;
	virtual bool getSymbol(INDEX* indices, int len, int* buf, SymbolBase* symBase,bool insertIfNotThere) const;
	virtual bool getString(INDEX* indices, int len, DolphinString** buf) const;
	virtual bool getString(INDEX* indices, int len, char** buf) const;
	virtual bool getBinary(INDEX* indices, int len, int unitLength, unsigned char* buf) const;

	virtual bool isNullSafe(INDEX offset, INDEX* indices, int len, char* buf) const;
	virtual bool isValidSafe(INDEX offset, INDEX* indices, int len, char* buf) const;
	virtual bool getBoolSafe(INDEX offset, INDEX* indices, int len, char* buf) const;
	virtual bool getCharSafe(INDEX offset, INDEX* indices, int len,char* buf) const;
	virtual bool getShortSafe(INDEX offset, INDEX* indices, int len, short* buf) const;
	virtual bool getIntSafe(INDEX offset, INDEX* indices, int len, int* buf) const;
	virtual bool getLongSafe(INDEX offset, INDEX* indices, int len, long long* buf) const;
	virtual bool getIndexSafe(INDEX offset, INDEX* indices, int len, INDEX* buf) const;
	virtual bool getFloatSafe(INDEX offset, INDEX* indices, int len, float* buf) const;
	virtual bool getDoubleSafe(INDEX offset, INDEX* indices, int len, double* buf) const;
	virtual bool getSymbolSafe(INDEX offset, INDEX* indices, int len, int* buf, SymbolBase* symBase,bool insertIfNotThere) const;
	virtual bool getStringSafe(INDEX offset, INDEX* indices, int len, DolphinString** buf) const;
	virtual bool getStringSafe(INDEX offset, INDEX* indices, int len, char** buf) const;
	virtual bool getBinarySafe(INDEX offset, INDEX* indices, int len, int unitLength, unsigned char* buf) const;

	virtual long long count() const {return count(0, index_->size());}
	virtual long long count(INDEX start, INDEX length) const;
	virtual ConstantSP minmax() const {return minmax(0, size_);}
	virtual ConstantSP minmax(INDEX start, INDEX length) const;
	/**
	 * @param rightMost If there are multiple maximum/minimum values, choose the last one if `rightMost` is true.
	 */
	virtual INDEX imax(bool rightMost = false) const override { return imax(0, size_, rightMost); }
	virtual INDEX imax(INDEX start, INDEX length, bool rightMost = false) const override;
	virtual INDEX imin(bool rightMost = false) const override { return imin(0, size_, rightMost); }
	virtual INDEX imin(INDEX start, INDEX length, bool rightMost = false) const override;

	virtual ConstantSP max() const {return max(0, index_->size());}
	virtual ConstantSP max(INDEX start, INDEX length) const;
	virtual void max(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const;
	virtual ConstantSP min() const {return min(0, index_->size());}
	virtual ConstantSP min(INDEX start, INDEX length) const;
	virtual void min(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const;
	virtual ConstantSP avg() const {return avg(0, index_->size());}
	virtual ConstantSP avg(INDEX start, INDEX length) const;
	virtual void avg(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const;
	virtual ConstantSP sum() const {return sum(0, index_->size());}
	virtual ConstantSP sum(INDEX start, INDEX length) const;
	virtual void sum(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const;
	virtual ConstantSP sum2() const {return sum2(0, index_->size());}
	virtual ConstantSP sum2(INDEX start, INDEX length) const;
	virtual void sum2(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const;
	virtual ConstantSP prd() const {return prd(0, index_->size());}
	virtual ConstantSP prd(INDEX start, INDEX length) const;
	virtual void prd(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const;
	virtual ConstantSP var() const {return var(0, index_->size());}
	virtual ConstantSP var(INDEX start, INDEX length) const;
	virtual void var(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const;
	virtual ConstantSP std() const {return std(0, index_->size());}
	virtual ConstantSP std(INDEX start, INDEX length) const;
	virtual void std(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const;
	virtual ConstantSP mode() const {return mode(0, index_->size());}
	virtual ConstantSP mode(INDEX start, INDEX length) const;
	virtual void mode(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const;
	virtual ConstantSP median() const {return median(0, index_->size());}
	virtual ConstantSP median(INDEX start, INDEX length) const;
	virtual void median(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const;
	virtual ConstantSP searchK(INDEX k) const {return searchK(0, size_, k);}
	virtual ConstantSP searchK(INDEX start, INDEX length, INDEX k) const;
	virtual void searchK(INDEX start, INDEX length, INDEX k, const ConstantSP& out, INDEX outputStart = 0) const;
	virtual ConstantSP firstNot(const ConstantSP& exclude) const { return firstNot(0, size_, exclude);}
	virtual ConstantSP firstNot(INDEX start, INDEX length, const ConstantSP& exclude) const;
	virtual void firstNot(INDEX start, INDEX length, const ConstantSP& exclude, const ConstantSP& out, INDEX outputStart = 0) const;
	virtual ConstantSP lastNot(const ConstantSP& exclude) const { return lastNot(0, size_, exclude);}
	virtual ConstantSP lastNot(INDEX start, INDEX length, const ConstantSP& exclude) const;
	virtual void lastNot(INDEX start, INDEX length, const ConstantSP& exclude, const ConstantSP& out, INDEX outputStart = 0) const;
	virtual void neg(){throw RuntimeException("Sliced vector doesn't support method neg");}
	virtual void replace(const ConstantSP& oldVal, const ConstantSP& newVal){throw RuntimeException("Sliced vector doesn't support method replace");}
	virtual void shuffle(){throw RuntimeException("Sliced vector doesn't support method shuffle");}
	virtual void reverse(){throw RuntimeException("Sliced vector doesn't support method reverse");}
	virtual void reverse(INDEX start, INDEX length){throw RuntimeException("Sliced vector doesn't support method reverse");}
	virtual bool rank(bool sorted, INDEX* indices, INDEX* ranking){throw RuntimeException("Sliced vector doesn't support method rank");}
	virtual void find(INDEX start, INDEX length, const ConstantSP& target, const ConstantSP& resultSP);
	virtual void binarySearch(INDEX start, INDEX length, const ConstantSP& target, const ConstantSP& resultSP);
	virtual void asof(INDEX start, INDEX length, const ConstantSP& target, const ConstantSP& resultSP);
	virtual bool findDuplicatedElements(Vector*  indices, INDEX start, INDEX length, vector<pair<INDEX,INDEX> >& duplicates){
		throw RuntimeException("Sliced vector doesn't support method findDuplicatedElements");
	}
	virtual bool findDuplicatedElements(INDEX start, INDEX length, vector<pair<INDEX,INDEX> >& duplicates){
		throw RuntimeException("Sliced vector doesn't support method findDuplicatedElements");
	}
	virtual bool findUniqueElements(INDEX start, INDEX length, vector<pair<INDEX,INDEX> >& uniques){
		throw RuntimeException("Sliced vector doesn't support method findUniqueElements");
	}
	virtual bool findRange(INDEX* ascIndices,const ConstantSP& target,INDEX* targetIndices,vector<pair<INDEX,INDEX> >& ranges){
		throw RuntimeException("Sliced vector doesn't support method findRange");
	}
	virtual bool findRange(const ConstantSP& target,INDEX* targetIndices,vector<pair<INDEX,INDEX> >& ranges){
		throw RuntimeException("Sliced vector doesn't support method findRange");
	}
	virtual INDEX lowerBound(INDEX start, const ConstantSP& target);
	virtual long long getAllocatedMemory() const {return sizeof(SlicedVector);}
	virtual int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int& numElement, int& partial) const {
		throw RuntimeException("Sliced vector doesn't support method serialize");
	}
	virtual int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int targetNumElement, int& numElement, int& partial) const {
		throw RuntimeException("Sliced vector doesn't support method serialize");
	}
	virtual IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const {throw RuntimeException("Sliced vector doesn't support method serialize");}
	virtual bool isSorted(INDEX start, INDEX length, bool asc, bool strict, char nullsOrder = 0) const {throw RuntimeException("Sliced vector doesn't support method isSorted");}
	virtual ConstantSP topK(INDEX start, INDEX length, INDEX top, bool asc, bool extendEqualValue) const {throw RuntimeException("Sliced vector doesn't support method topK");}
	virtual bool sort(bool asc, char nullsOrder = 0) {throw RuntimeException("Sliced vector doesn't support method sort");}
	virtual bool sort(bool asc, Vector* indices, char nullsOrder = 0) {throw RuntimeException("Sliced vector doesn't support method sort");}
	virtual bool sortSelectedIndices(Vector* indices, INDEX start, INDEX length, bool asc, char nullsOrder = 0) {
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
	virtual ~SubVector(){}
	virtual bool isLargeConstant() const {return true;}
	virtual VECTOR_TYPE getVectorType() const {return VECTOR_TYPE::SUBVECTOR;}
	VectorSP getSourceVector() const { return source_;}
	INDEX getSubVectorStart() const { return offset_;}
	INDEX getSubVectorLength() const { return size_;}
	virtual int getSegmentSizeInBit() const override { return source_->getSegmentSizeInBit(); }
	virtual bool copyable() const {return true;}
	virtual bool isView() const {return true;}
	virtual DATA_TYPE getRawType() const {return source_->getRawType();}
	virtual int getExtraParamForType() const { return source_->getExtraParamForType();}
	virtual bool isTableColumn() const { return source_->isTableColumn();}
	virtual SymbolBaseSP getSymbolBase() const {return source_->getSymbolBase();}
	virtual DATA_FORM getForm() const { return DF_VECTOR;}
	virtual ConstantSP getInstance() const {return getInstance(size_);}
	virtual ConstantSP getInstance(INDEX size) const;
	virtual ConstantSP getValue() const { return source_->getSubVector(offset_, size_);}
	virtual ConstantSP getValue(INDEX capacity) const { return source_->getSubVector(offset_, size_, capacity);}
	virtual ConstantSP get(INDEX column, INDEX rowStart,INDEX rowEnd) const {return source_->getSubVector(offset_ + rowStart,rowEnd-rowStart);}
	virtual ConstantSP getWindow(INDEX colStart, int colLength, INDEX rowStart, int rowLength) const;
	virtual ConstantSP getSlice(const ConstantSP& rowIndex, const ConstantSP& colIndex) const;
	virtual ConstantSP getSubVector(INDEX start, INDEX length, INDEX capacity) const;
	virtual ConstantSP getSubVector(INDEX start, INDEX length) const {
		return getSubVector(start, length, std::abs(length));
	}
	virtual void fill(INDEX start, INDEX length, const ConstantSP& value, INDEX valueOffset = 0) override{
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method fill");
		else
			source_->fill(offset_ + start, length, value, valueOffset);
	}
	virtual bool validIndex(INDEX uplimit){return source_->validIndex(offset_, size_, uplimit);}
	virtual bool validIndex(INDEX start, INDEX length, INDEX uplimit){return source_->validIndex(start + offset_, length, uplimit);}
	virtual int compare(INDEX index, const ConstantSP& target) const {return source_->compare(offset_ + index, target);}
	virtual bool getNullFlag() const {
		if (offset_ >= 0 && offset_ + size_ <= source_->size())
			return source_->getNullFlag();
		return true;
	}
	virtual void setNullFlag(bool containNull){}
	virtual bool hasNull(){
		if (offset_ >= 0 && offset_ + size_ <= source_->size())
			return source_->hasNull(offset_, size_);
		return true;
	}
	virtual bool hasNull(INDEX start, INDEX length){
		if (start + offset_ >= 0 && start + offset_ + length <= source_->size())
			return source_->hasNull(start + offset_, length);
		return true;
	}
	virtual INDEX getCapacity() const {return size_;}
	virtual bool isFastMode() const { return source_->isFastMode();}
	virtual void* getDataArray() const;
	virtual bool isIndexArray() const { return source_->isIndexArray();}
	virtual INDEX* getIndexArray() const { return source_->isIndexArray() ? source_->getIndexArray() + offset_ : NULL;}
	virtual short getUnitLength() const {return source_->getUnitLength();}
	virtual void** getDataSegment() const;
	virtual bool sizeable() const {return source_->sizeable();}
	virtual char getBool() const {return source_->getBool(offset_);}
	virtual char getChar() const { return source_->getChar(offset_);}
	virtual short getShort() const { return source_->getShort(offset_);}
	virtual int getInt() const {return source_->getInt(offset_);}
	virtual long long getLong() const {return source_->getLong(offset_);}
	virtual INDEX getIndex() const {return source_->getIndex(offset_);}
	virtual float getFloat() const {return source_->getFloat(offset_);}
	virtual double getDouble() const {return source_->getDouble(offset_);}
	virtual void contain(const ConstantSP& target, const ConstantSP& resultSP) const override;

	virtual ConstantSP get(INDEX index) const override;

	virtual ConstantSP get(const ConstantSP& index) const;
	virtual ConstantSP get(INDEX offset, const ConstantSP& index) const;
	virtual void setBool(bool val){throw RuntimeException("Immutable sub vector doesn't support method setBool");}
	virtual void setChar(char val){throw RuntimeException("Immutable sub vector doesn't support method setChar");}
	virtual void setShort(short val){throw RuntimeException("Immutable sub vector doesn't support method setShort");}
	virtual void setInt(int val){throw RuntimeException("Immutable sub vector doesn't support method setInt");}
	virtual void setLong(long long val){throw RuntimeException("Immutable sub vector doesn't support method setLong");}
	virtual void setIndex(INDEX val){throw RuntimeException("Immutable sub vector doesn't support method setIndex");}
	virtual void setFloat(float val){throw RuntimeException("Immutable sub vector doesn't support method setFloat");}
	virtual void setDouble(double val){throw RuntimeException("Immutable sub vector doesn't support method setDouble");}
	virtual void setString(const string& val){throw RuntimeException("Immutable sub vector doesn't support method setString");}
	virtual void setNull(){throw RuntimeException("Immutable sub vector doesn't support method setNull");}
	virtual char getBool(INDEX index) const {
		if (index < 0 || index >= size_ || offset_ + index < 0 || offset_ + index >= source_->size()) {
			return CHAR_MIN;
		}
		return source_->getBool(offset_ + index);
	}
	virtual char getChar(INDEX index) const {
		if (index < 0 || index >= size_ || offset_ + index < 0 || offset_ + index >= source_->size()) {
			return CHAR_MIN;
		}
		return source_->getChar(offset_ + index);
	}
	virtual short getShort(INDEX index) const {
		if (index < 0 || index >= size_ || offset_ + index < 0 || offset_ + index >= source_->size()) {
			return SHRT_MIN;
		}
		return source_->getShort(offset_ + index);
	}
	virtual int getInt(INDEX index) const {
		if (index < 0 || index >= size_ || offset_ + index < 0 || offset_ + index >= source_->size()) {
			return INT_MIN;
		}
		return source_->getInt(offset_ + index);
	}
	virtual long long getLong(INDEX index) const {
		if (index < 0 || index >= size_ || offset_ + index < 0 || offset_ + index >= source_->size()) {
			return LONG_MIN;
		}
		return source_->getLong(offset_ + index);
	}
	virtual INDEX getIndex(INDEX index) const {
		if (index < 0 || index >= size_ || offset_ + index < 0 || offset_ + index >= source_->size()) {
			return INDEX_MIN;
		}
		return source_->getIndex(offset_ + index);
	}
	virtual float getFloat(INDEX index) const {
		if (index < 0 || index >= size_ || offset_ + index < 0 || offset_ + index >= source_->size()) {
			return FLT_NMIN;
		}
		return source_->getFloat(offset_ + index);
	}
	virtual double getDouble(INDEX index) const {
		if (index < 0 || index >= size_ || offset_ + index < 0 || offset_ + index >= source_->size()) {
			return DBL_NMIN;
		}
		return source_->getDouble(offset_ + index);
	}
	virtual bool isNull(INDEX index) const {
		if (index < 0 || index >= size_ || offset_ + index < 0 || offset_ + index >= source_->size()) {
			return true;
		}
		return source_->isNull(offset_ + index);
	}
	virtual string getString(INDEX index) const {
		if (index < 0 || index >= size_ || offset_ + index < 0 || offset_ + index >= source_->size()) {
			return "";
		}
		return source_->getString(offset_ + index);
	}
	virtual const DolphinString& getStringRef(INDEX index) const {
		if (index < 0 || index >= size_ || offset_ + index < 0 || offset_ + index >= source_->size()) {
			throw RuntimeException("Index out of range.");
		}
		return source_->getStringRef(offset_ + index);
	}
	virtual void setBool(INDEX index,char val) {
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setBool");
		else
			source_->setBool(offset_ + index, val);
	}
	virtual void setChar(INDEX index,char val){
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setChar");
		else
			source_->setChar(offset_ + index, val);
	}
	virtual void setShort(INDEX index,short val){
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setShort");
		else
			source_->setShort(offset_ + index, val);
	}
	virtual void setInt(INDEX index,int val){
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setInt");
		else
			source_->setInt(offset_ + index, val);
	}
	virtual void setLong(INDEX index,long long val){
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setLong");
		else
			source_->setLong(offset_ + index, val);
	}
	virtual void setIndex(INDEX index, INDEX val){
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setIndex");
		else
			source_->setIndex(offset_ + index, val);
	}
	virtual void setFloat(INDEX index,float val){
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setFloat");
		else
			source_->setFloat(offset_ + index, val);
	}
	virtual void setDouble(INDEX index, double val){
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setDouble");
		else
			source_->setDouble(offset_ + index, val);
	}
	virtual void setString(INDEX index, const DolphinString& val){
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setString");
		else
			source_->setString(offset_ + index, val);
	}
	virtual void setBinary(INDEX index, int unitLength, const unsigned char* val){
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setBinary");
		else
			source_->setBinary(offset_ + index, unitLength, val);
	}
	virtual void setNull(INDEX index){
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setNull");
		else
			source_->setNull(offset_ + index);
	}
	virtual void clear(){throw RuntimeException("Immutable sub vector doesn't support method clear");}
	virtual bool remove(INDEX count){throw RuntimeException("Immutable sub vector doesn't support method remove");}
	virtual bool remove(const ConstantSP& index){throw RuntimeException("Immutable sub vector doesn't support method remove");}
	virtual void next(INDEX steps){throw RuntimeException("Immutable sub vector doesn't support method next");}
	virtual void prev(INDEX steps){throw RuntimeException("Immutable sub vector doesn't support method prev");}
	virtual INDEX size() const {
		if(source_->size() < offset_ + size_ && source_->size() == 0)
			throw RuntimeException("The source vector has been shortened and the sub vector is not valid any more.");
		return size_;
	}
	virtual void nullFill(const ConstantSP& val){throw RuntimeException("Immutable sub vector doesn't support method nullFill");}
	virtual bool isNull(INDEX start, int len, char* buf) const {
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
	virtual bool isValid(INDEX start, int len, char* buf) const {
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

    virtual bool getBool(INDEX *indices, int len, char *buf) const override;
    virtual bool getChar(INDEX *indices, int len, char *buf) const override;
    virtual bool getShort(INDEX *indices, int len, short *buf) const override;
    virtual bool getInt(INDEX *indices, int len, int *buf) const override;
    virtual bool getIndex(INDEX *indices, int len, INDEX *buf) const override;
    virtual bool getLong(INDEX *indices, int len, long long *buf) const override;
    virtual bool getFloat(INDEX *indices, int len, float *buf) const override;
    virtual bool getDouble(INDEX *indices, int len, double *buf) const override;
    virtual bool getDecimal32(INDEX *indices, int len, int scale, int *buf) const override;

	virtual bool getBool(INDEX start, int len, char* buf) const {
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

	virtual const char* getBoolConst(INDEX start, int len, char* buf) const {
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
	virtual char* getBoolBuffer(INDEX start, int len, char* buf) const {return source_->getBoolBuffer(offset_ + start, len, buf);}
	virtual bool getChar(INDEX start, int len, char* buf) const {
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
	virtual const char* getCharConst(INDEX start, int len, char* buf) const {
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
	virtual char* getCharBuffer(INDEX start, int len, char* buf) const {return source_->getCharBuffer(offset_ + start, len, buf);}
	virtual bool getShort(INDEX start, int len, short* buf) const {
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
	virtual const short* getShortConst(INDEX start, int len, short* buf) const {
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
	virtual short* getShortBuffer(INDEX start, int len, short* buf) const {return source_->getShortBuffer(offset_ + start, len, buf);}
	virtual bool getInt(INDEX start, int len, int* buf) const {
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
	virtual const int* getIntConst(INDEX start, int len, int* buf) const {
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
	virtual int* getIntBuffer(INDEX start, int len, int* buf) const {return source_->getIntBuffer(offset_ + start, len, buf);}
	virtual bool getLong(INDEX start, int len, long long* buf) const {
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
	virtual const long long* getLongConst(INDEX start, int len, long long* buf) const {
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
	virtual long long* getLongBuffer(INDEX start, int len, long long* buf) const {return source_->getLongBuffer(offset_ + start, len, buf);}
	virtual bool getBinary(INDEX start, int len, int unitLength, unsigned char* buf) const {
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
	virtual const unsigned char* getBinaryConst(INDEX start, int len, int unitLength, unsigned char* buf) const {
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
	virtual unsigned char* getBinaryBuffer(INDEX start, int len, int unitLength, unsigned char* buf) const {return source_->getBinaryBuffer(offset_ + start, len, unitLength, buf);}
	virtual bool getIndex(INDEX start, int len, INDEX* buf) const {
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
	virtual const INDEX* getIndexConst(INDEX start, int len, INDEX* buf) const {
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
	virtual INDEX* getIndexBuffer(INDEX start, int len, INDEX* buf) const {return source_->getIndexBuffer(offset_ + start, len, buf);}
	virtual bool getFloat(INDEX start, int len, float* buf) const {
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
	virtual const float* getFloatConst(INDEX start, int len, float* buf) const {
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
	virtual float* getFloatBuffer(INDEX start, int len, float* buf) const {return source_->getFloatBuffer(offset_ + start, len, buf);}
	virtual bool getDouble(INDEX start, int len, double* buf) const {
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
	virtual const double* getDoubleConst(INDEX start, int len, double* buf) const {
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
	virtual double* getDoubleBuffer(INDEX start, int len, double* buf) const {return source_->getDoubleBuffer(offset_ + start, len, buf);}
	virtual bool getString(INDEX start, int len, char** buf) const {
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
	virtual char** getStringConst(INDEX start, int len, char** buf) const {
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
	virtual bool getString(INDEX start, int len, DolphinString** buf) const {
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
	virtual DolphinString** getStringConst(INDEX start, int len, DolphinString** buf) const {
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
	virtual const int* getSymbolConst(INDEX start, int len, int* buf, SymbolBase* symBase, bool insertIfNotThere) const {
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
	virtual bool getSymbol(INDEX start, int len, int* buf, SymbolBase* symBase, bool insertIfNotThere) const {
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
	virtual bool set(INDEX index, const ConstantSP& value, INDEX valueIndex){
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method set");
		else
			return source_->set(offset_ + index, value, valueIndex);
	}
	virtual bool set(INDEX index, const ConstantSP& value){
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method set");
		else
			return source_->set(offset_ + index, value);
	}
	virtual bool set(INDEX column, INDEX row, const ConstantSP& value){throw RuntimeException("Immutable sub vector doesn't support method set");}
	virtual bool set(const ConstantSP& index, const ConstantSP& value) {throw RuntimeException("Immutable sub vector doesn't support method set");}
	virtual bool setNonNull(const ConstantSP& index, const ConstantSP& value) {throw RuntimeException("Immutable sub vector doesn't support method setNonNull");}
	virtual bool setBool(INDEX start, int len, const char* buf){
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setBool");
		else
			return source_->setBool(offset_ + start, len, buf);
	}
	virtual bool setChar(INDEX start, int len, const char* buf){
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setChar");
		else
			return source_->setChar(offset_ + start, len, buf);
	}
	virtual bool setShort(INDEX start, int len, const short* buf){
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setShort");
		else
			return source_->setShort(offset_ + start, len, buf);
	}
	virtual bool setInt(INDEX start, int len, const int* buf){
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setInt");
		else
			return source_->setInt(offset_ + start, len, buf);
	}
	virtual bool setLong(INDEX start, int len, const long long* buf){
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setLong");
		else
			return source_->setLong(offset_ + start, len, buf);
	}
	virtual bool setIndex(INDEX start, int len, const INDEX* buf){
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setIndex");
		else
			return source_->setIndex(offset_ + start, len, buf);
	}
	virtual bool setFloat(INDEX start, int len, const float* buf){
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setFloat");
		else
			return source_->setFloat(offset_ + start, len, buf);
	}
	virtual bool setDouble(INDEX start, int len, const double* buf){
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setDouble");
		else
			return source_->setDouble(offset_ + start, len, buf);
	}
	virtual bool setString(INDEX start, int len, const string* buf){
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setString");
		else
			return source_->setString(offset_ + start, len, buf);
	}
	virtual bool setString(INDEX start, int len, char** buf){
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setString");
		else
			return source_->setString(offset_ + start, len, buf);
	}
	virtual bool setBinary(INDEX start, int len, int unitLength, const unsigned char* buf){
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setBinary");
		else
			return source_->setBinary(offset_ + start, len, unitLength, buf);
	}
	virtual bool setData(INDEX start, int len, void* buf) {
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method setData");
		else
			return source_->setData(offset_ + start, len, buf);
	}

public:  /// {get,set}Decimal{32,64,128}
	virtual int getDecimal32(int scale) const override {
		return getDecimal32(/*index*/0, scale);
	}
	virtual long long getDecimal64(int scale) const override {
		return getDecimal64(/*index*/0, scale);
	}
	virtual int128 getDecimal128(int scale) const override {
		return getDecimal128(/*index*/0, scale);
	}

	virtual int getDecimal32(INDEX index, int scale) const override {
		if (index < 0 || index >= size_ || offset_ + index < 0 || offset_ + index >= source_->size()) {
			return INT_MIN;
		}
		return source_->getDecimal32(offset_ + index, scale);
	}
	virtual long long getDecimal64(INDEX index, int scale) const override {
		if (index < 0 || index >= size_ || offset_ + index < 0 || offset_ + index >= source_->size()) {
			return LONG_MIN;
		}
		return source_->getDecimal64(offset_ + index, scale);
	}
	virtual int128 getDecimal128(INDEX index, int scale) const override {
		if (index < 0 || index >= size_ || offset_ + index < 0 || offset_ + index >= source_->size()) {
			return INT128_MIN;
		}
		return source_->getDecimal128(offset_ + index, scale);
	}

	virtual bool getDecimal32(INDEX start, int len, int scale, int *buf) const override {
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
	virtual bool getDecimal64(INDEX start, int len, int scale, long long *buf) const override {
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
	virtual bool getDecimal128(INDEX start, int len, int scale, int128 *buf) const override {
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

	virtual const int* getDecimal32Const(INDEX start, int len, int scale, int *buf) const override {
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
	virtual const long long* getDecimal64Const(INDEX start, int len, int scale, long long *buf) const override {
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
			std::min(len, std::max(std::abs(start), std::abs(offset_ + start)));
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
	virtual const int128* getDecimal128Const(INDEX start, int len, int scale,
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
			std::min(len, std::max(std::abs(start), std::abs(offset_ + start)));
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

	virtual int* getDecimal32Buffer(INDEX start, int len, int scale, int *buf) const override {
		return source_->getDecimal32Buffer(offset_ + start, len, scale, buf);
	}
	virtual long long* getDecimal64Buffer(INDEX start, int len, int scale, long long *buf) const override {
		return source_->getDecimal64Buffer(offset_ + start, len, scale, buf);
	}
	virtual int128* getDecimal128Buffer(INDEX start, int len, int scale,
			int128 *buf) const override {
		return source_->getDecimal128Buffer(offset_ + start, len, scale, buf);
	}

	virtual void setDecimal32(INDEX index, int scale, int val) override {
		if (!updatable_) {
			throw RuntimeException("Immutable sub vector doesn't support method setDecimal32");
		} else {
			source_->setDecimal32(offset_ + index, scale, val);
		}
	}
	virtual void setDecimal64(INDEX index, int scale, long long val) override {
		if (!updatable_) {
			throw RuntimeException("Immutable sub vector doesn't support method setDecimal64");
		} else {
			source_->setDecimal64(offset_ + index, scale, val);
		}
	}
	virtual void setDecimal128(INDEX index, int scale, int128 val) override {
		if (!updatable_) {
			throw RuntimeException("Immutable sub vector doesn't support method setDecimal128");
		} else {
			source_->setDecimal128(offset_ + index, scale, val);
		}
	}

	virtual bool setDecimal32(INDEX start, int len, int scale, const int *buf) override {
		if (!updatable_) {
			throw RuntimeException("Immutable sub vector doesn't support method setDecimal32");
		} else {
			return source_->setDecimal32(offset_ + start, len, scale, buf);
		}
	}
	virtual bool setDecimal64(INDEX start, int len, int scale, const long long *buf) override {
		if (!updatable_) {
			throw RuntimeException("Immutable sub vector doesn't support method setDecimal64");
		} else {
			return source_->setDecimal64(offset_ + start, len, scale, buf);
		}
	}
	virtual bool setDecimal128(INDEX start, int len, int scale, const int128 *buf) override {
		if (!updatable_) {
			throw RuntimeException("Immutable sub vector doesn't support method setDecimal128");
		} else {
			return source_->setDecimal128(offset_ + start, len, scale, buf);
		}
	}

public:
	virtual bool appendBool(char* buf, int len){throw RuntimeException("Immutable sub vector doesn't support method appendBool");}
	virtual bool appendChar(char* buf, int len){throw RuntimeException("Immutable sub vector doesn't support method appendChar");}
	virtual bool appendShort(short* buf, int len){throw RuntimeException("Immutable sub vector doesn't support method appendShort");}
	virtual bool appendInt(int* buf, int len){throw RuntimeException("Immutable sub vector doesn't support method appendInt");}
	virtual bool appendLong(long long* buf, int len){throw RuntimeException("Immutable sub vector doesn't support method appendLong");}
	virtual bool appendIndex(INDEX* buf, int len){throw RuntimeException("Immutable sub vector doesn't support method appendIndex");}
	virtual bool appendFloat(float* buf, int len){throw RuntimeException("Immutable sub vector doesn't support method appendFloat");}
	virtual bool appendDouble(double* buf, int len){throw RuntimeException("Immutable sub vector doesn't support method appendDouble");}
	virtual bool assign(const ConstantSP& value){throw RuntimeException("Immutable sub vector doesn't support method assign");}

	/**
	 * @param rightMost If there are multiple maximum/minimum values, choose the last one if `rightMost` is true.
	 */
	virtual INDEX imax(bool rightMost = false) const override { return imax(0, size_, rightMost); }
	virtual INDEX imax(INDEX start, INDEX length, bool rightMost = false) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		INDEX index = source_->imax(range.first, range.second, rightMost);
		return index >= 0 ? index - offset_ : index;
	}
	virtual INDEX imin(bool rightMost = false) const override { return imin(0, size_, rightMost); }
	virtual INDEX imin(INDEX start, INDEX length, bool rightMost = false) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		INDEX index = source_->imin(range.first, range.second, rightMost);
		return index >= 0 ? index - offset_ : index;
	}
	virtual long long count() const override {return count(0, size_);}
	virtual long long count(INDEX start, INDEX length) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		return source_->count(range.first, range.second);
	}
	virtual ConstantSP minmax() const {return minmax(0, size_);}
	virtual ConstantSP minmax(INDEX start, INDEX length) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		return source_->minmax(range.first, range.second);
	}

	virtual ConstantSP max() const override { return max(0, size_); }
	virtual ConstantSP max(INDEX start, INDEX length) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		return source_->max(range.first, range.second);
	}
	virtual void max(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		source_->max(range.first, range.second, out, outputStart);
	}

	virtual ConstantSP min() const override { return min(0, size_); }
	virtual ConstantSP min(INDEX start, INDEX length) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		return source_->min(range.first, range.second);
	}
	virtual void min(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		source_->min(range.first, range.second, out, outputStart);
	}

	virtual ConstantSP avg() const override { return avg(0, size_); }
	virtual ConstantSP avg(INDEX start, INDEX length) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		return source_->avg(range.first, range.second);
	}
	virtual void avg(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		source_->avg(range.first, range.second, out, outputStart);
	}

	virtual ConstantSP sum() const override { return sum(0, size_); }
	virtual ConstantSP sum(INDEX start, INDEX length) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		return source_->sum(range.first, range.second);
	}
	virtual void sum(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		source_->sum(range.first, range.second, out, outputStart);
	}

	virtual ConstantSP sum2() const override { return sum2(0, size_); }
	virtual ConstantSP sum2(INDEX start, INDEX length) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		return source_->sum2(range.first, range.second);
	}
	virtual void sum2(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		source_->sum2(range.first, range.second, out, outputStart);
	}

	virtual ConstantSP std() const override { return std(0, size_); }
	virtual ConstantSP std(INDEX start, INDEX length) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		return source_->std(range.first, range.second);
	}
	virtual void std(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		source_->std(range.first, range.second, out, outputStart);
	}
	virtual ConstantSP var() const override { return var(0, size_); }
	virtual ConstantSP var(INDEX start, INDEX length) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		return source_->var(range.first, range.second);
	}
	virtual void var(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		source_->var(range.first, range.second, out, outputStart);
	}
	virtual ConstantSP prd() const override { return prd(0, size_); }
	virtual ConstantSP prd(INDEX start, INDEX length) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		return source_->prd(range.first, range.second);
	}
	virtual void prd(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		source_->prd(range.first, range.second, out, outputStart);
	}
	virtual ConstantSP median() const override { return median(0, size_); }
	virtual ConstantSP median(INDEX start, INDEX length) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		return source_->median(range.first, range.second);
	}
	virtual void median(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		source_->median(range.first, range.second, out, outputStart);
	}
	virtual ConstantSP mode() const override { return mode(0, size_); }
	virtual ConstantSP mode(INDEX start, INDEX length) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		return source_->mode(range.first, range.second);
	}
	virtual void mode(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		source_->mode(range.first, range.second, out, outputStart);
	}
	virtual ConstantSP firstNot(const ConstantSP& exclude) const override { return firstNot(0, size_, exclude); }
	virtual ConstantSP firstNot(INDEX start, INDEX length, const ConstantSP& exclude) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		return source_->firstNot(range.first, range.second, exclude);
	}
	virtual void firstNot(INDEX start, INDEX length, const ConstantSP& exclude, const ConstantSP& out, INDEX outputStart = 0) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		source_->firstNot(range.first, range.second, exclude, out, outputStart);
	}
	virtual ConstantSP lastNot(const ConstantSP& exclude) const override { return lastNot(0, size_, exclude); }
	virtual ConstantSP lastNot(INDEX start, INDEX length, const ConstantSP& exclude) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		return source_->lastNot(range.first, range.second, exclude);
	}
	virtual void lastNot(INDEX start, INDEX length, const ConstantSP& exclude, const ConstantSP& out, INDEX outputStart = 0) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		source_->lastNot(range.first, range.second, exclude, out, outputStart);
	}
	virtual ConstantSP searchK(INDEX k) const override { return searchK(0, size_, k); }
	virtual ConstantSP searchK(INDEX start, INDEX length, INDEX k) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		return source_->searchK(range.first, range.second, k);
	}
	virtual void searchK(INDEX start, INDEX length, INDEX k, const ConstantSP& out, INDEX outputStart = 0) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		source_->searchK(range.first, range.second, k, out, outputStart);
	}

	virtual void neg(){throw RuntimeException("Immutable sub vector doesn't support method neg");}
	virtual void replace(const ConstantSP& oldVal, const ConstantSP& newVal){throw RuntimeException("Immutable sub vector doesn't support method replace");}
	virtual void shuffle(){throw RuntimeException("Immutable sub vector doesn't support method shuffle");}
	virtual void reverse(){
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method reverse");
		else
			source_->reverse(offset_, size_);
	}
	virtual void reverse(INDEX start, INDEX length){
		if(!updatable_)
			throw RuntimeException("Immutable sub vector doesn't support method reverse");
		else
			source_->reverse(start + offset_, length);
	}
	virtual bool rank(bool sorted, INDEX* indices, INDEX* ranking){
		throw RuntimeException("Immutable sub vector doesn't support method rank");
	}
	virtual void find(INDEX start, INDEX length, const ConstantSP& target, const ConstantSP& resultSP){
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
	virtual void binarySearch(INDEX start, INDEX length, const ConstantSP& target, const ConstantSP& resultSP){
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
	virtual void asof(INDEX start, INDEX length, const ConstantSP& target, const ConstantSP& resultSP){
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

	virtual INDEX lowerBound(INDEX start, const ConstantSP& target) override;

	virtual bool equalToPrior(INDEX start, INDEX length, bool* result){
		return source_->equalToPrior(offset_ + start, length, result);
	}

	virtual bool equalToPrior(INDEX prior, const INDEX* indices, INDEX length, bool* result){
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

	virtual bool findDuplicatedElements(Vector*  indices, INDEX start, INDEX length, vector<pair<INDEX,INDEX> >& duplicates){
		if(offset_ == 0)
			return source_->findDuplicatedElements(indices, start, length, duplicates);
		else
			throw RuntimeException("Immutable sub vector doesn't support method findDuplicatedElements");
	}
	virtual bool findDuplicatedElements(INDEX start, INDEX length, vector<pair<INDEX,INDEX> >& duplicates){
		throw RuntimeException("Immutable sub vector doesn't support method findDuplicatedElements");
	}
	virtual bool findUniqueElements(INDEX start, INDEX length, vector<pair<INDEX,INDEX> >& uniques){
		throw RuntimeException("Immutable sub vector doesn't support method findUniqueElements");
	}
	virtual bool findRange(INDEX* ascIndices,const ConstantSP& target,INDEX* targetIndices,vector<pair<INDEX,INDEX> >& ranges){
		throw RuntimeException("Immutable sub vector doesn't support method findRange");
	}
	virtual bool findRange(const ConstantSP& target,INDEX* targetIndices,vector<pair<INDEX,INDEX> >& ranges){
		throw RuntimeException("Immutable sub vector doesn't support method findRange");
	}
	virtual long long getAllocatedMemory() const {
		// use the allocated memory of the underlying source vector, for the calculation of cacheEngine
		// 	in tsdb to be correct
		return ((Constant*)source_.get())->getAllocatedMemory() * (size_ * 1.0 / source_->size());
	}
	virtual int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int& numElement, int& partial) const;
	virtual int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int targetNumElement, int& numElement, int& partial) const;
	virtual IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const {throw RuntimeException("Immutable sub vector doesn't support method serialize");}
	virtual bool isSorted(INDEX start, INDEX length, bool asc, bool strict, char nullsOrder = 0) const {
		auto range = calculateOverlappedRange(start + offset_, length);
		return source_->isSorted(range.first, range.second, asc, strict, nullsOrder);
	}
	virtual ConstantSP topK(INDEX start, INDEX length, INDEX top, bool asc, bool extendEqualValue) const {
		ConstantSP result = source_->topK(offset_ + start, length, top, asc, extendEqualValue);
		if(offset_ > 0)
			((Vector*)result.get())->addIndex(0, result->size(), -offset_);
		return result;
	}
	virtual bool sort(bool asc, char nullsOrder = 0) {throw RuntimeException("Immutable sub vector doesn't support method sort");}
	virtual bool sort(bool asc, Vector* indices, char nullsOrder = 0) {throw RuntimeException("Immutable sub vector doesn't support method sort");}
	virtual bool sortSelectedIndices(Vector* indices, INDEX start, INDEX length, bool asc, char nullsOrder = 0) {
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
	virtual bool isNullSafe(INDEX offset, INDEX* indices, int len, char* buf) const {
		return source_->isNullSafe(offset + offset_, indices, len, buf);
	}

	virtual bool isValidSafe(INDEX offset, INDEX* indices, int len, char* buf) const {
		return source_->isValidSafe(offset + offset_, indices, len, buf);
	}

	virtual bool getBoolSafe(INDEX offset, INDEX* indices, int len, char* buf) const {
		return source_->getBoolSafe(offset + offset_, indices, len, buf);
	}

	virtual bool getCharSafe(INDEX offset, INDEX* indices, int len,char* buf) const {
		return source_->getCharSafe(offset + offset_, indices, len, buf);
	}

	virtual bool getShortSafe(INDEX offset, INDEX* indices, int len, short* buf) const {
		return source_->getShortSafe(offset + offset_, indices, len, buf);
	}

	virtual bool getIntSafe(INDEX offset, INDEX* indices, int len, int* buf) const {
		return source_->getIntSafe(offset + offset_, indices, len, buf);
	}

	virtual bool getLongSafe(INDEX offset, INDEX* indices, int len, long long* buf) const {
		return source_->getLongSafe(offset + offset_, indices, len, buf);
	}

	virtual bool getIndexSafe(INDEX offset, INDEX* indices, int len, INDEX* buf) const {
		return source_->getIndexSafe(offset + offset_, indices, len, buf);
	}

	virtual bool getFloatSafe(INDEX offset, INDEX* indices, int len, float* buf) const {
		return source_->getFloatSafe(offset + offset_, indices, len, buf);
	}

	virtual bool getDoubleSafe(INDEX offset, INDEX* indices, int len, double* buf) const {
		return source_->getDoubleSafe(offset + offset_, indices, len, buf);
	}

	virtual bool getSymbolSafe(INDEX offset, INDEX* indices, int len, int* buf, SymbolBase* symBase,bool insertIfNotThere) const {
		return source_->getSymbolSafe(offset + offset_, indices, len, buf, symBase, insertIfNotThere);
	}

	virtual bool getStringSafe(INDEX offset, INDEX* indices, int len, DolphinString** buf) const {
		return source_->getStringSafe(offset + offset_, indices, len, buf);
	}

	virtual bool getStringSafe(INDEX offset, INDEX* indices, int len, char** buf) const {
		return source_->getStringSafe(offset + offset_, indices, len, buf);
	}

	virtual bool getBinarySafe(INDEX offset, INDEX* indices, int len, int unitLength, unsigned char* buf) const {
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

class FastArrayVector : public Vector {
public:
	FastArrayVector(const VectorSP& index, const VectorSP& value, bool checkNull = true);
	INDEX getValueSize() const { return valueSize_;}
	VectorSP getSourceIndex() const { return index_;}
	VectorSP getSourceValue() const { return value_;}
	virtual ~FastArrayVector(){}
	virtual bool isLargeConstant() const {return true;}
	virtual VECTOR_TYPE getVectorType() const {return VECTOR_TYPE::ARRAYVECTOR;}
	virtual DATA_TYPE getRawType() const {return getType();}
	virtual int getExtraParamForType() const { return value_->getExtraParamForType(); }
	virtual SymbolBaseSP getSymbolBase() const {return value_->getSymbolBase();}
	virtual DATA_FORM getForm() const { return DF_VECTOR;}
	virtual ConstantSP getInstance() const {return getInstance(size_);}
	virtual ConstantSP getInstance(INDEX size) const;
	virtual ConstantSP getValue() const;
	virtual ConstantSP getValue(INDEX capacity) const;
	virtual ConstantSP get(INDEX column, INDEX rowStart,INDEX rowEnd) const;
	virtual ConstantSP getWindow(INDEX colStart, int colLength, INDEX rowStart, int rowLength) const;
	virtual ConstantSP getSubVector(INDEX start, INDEX length, INDEX capacity) const;
	virtual ConstantSP getSubVector(INDEX start, INDEX length) const;
	virtual void fill(INDEX start, INDEX length, const ConstantSP& value, INDEX valueOffset = 0) override;
	virtual bool validIndex(INDEX uplimit){throw RuntimeException("Array vector doesn't support method validIndex");}
	virtual bool validIndex(INDEX start, INDEX length, INDEX uplimit){throw RuntimeException("Array vector doesn't support method validIndex");}
	virtual int compare(INDEX index, const ConstantSP& target) const {throw RuntimeException("Array vector doesn't support method compare");}
	virtual bool getNullFlag() const {return containNull_;}
	virtual void setNullFlag(bool containNull){ containNull_ = containNull;}
	virtual bool hasNull(){return hasNull(0, size_);}
	virtual bool hasNull(INDEX start, INDEX length);
	virtual INDEX getCapacity() const {return index_->getCapacity();}
	virtual bool isFastMode() const { return value_->isFastMode();}
	virtual void* getDataArray() const {return value_->getDataArray();}
	virtual bool isIndexArray() const { return value_->isIndexArray();}
	virtual INDEX* getIndexArray() const { return value_->getIndexArray();}
	virtual short getUnitLength() const {return 0;}
	virtual void** getDataSegment() const { return value_->getDataSegment();}
	virtual bool sizeable() const {return value_->sizeable();}
	virtual char getBool() const {throw RuntimeException("Array vector doesn't support method getBool");}
	virtual char getChar() const {throw RuntimeException("Array vector doesn't support method getChar");}
	virtual short getShort() const {throw RuntimeException("Array vector doesn't support method getShort");}
	virtual int getInt() const {throw RuntimeException("Array vector doesn't support method getInt");}
	virtual long long getLong() const {throw RuntimeException("Array vector doesn't support method getLong");}
	virtual INDEX getIndex() const {throw RuntimeException("Array vector doesn't support method getIndex");}
	virtual float getFloat() const {throw RuntimeException("Array vector doesn't support method getFloat");}
	virtual double getDouble() const {throw RuntimeException("Array vector doesn't support method getDouble");}
	virtual ConstantSP get(INDEX index) const;
	virtual ConstantSP getRef(INDEX index) const;
	virtual ConstantSP get(const ConstantSP& index) const;
	virtual ConstantSP get(INDEX offset, const ConstantSP& index) const;
	virtual ConstantSP getSlice(const ConstantSP& rowIndex, const ConstantSP& colIndex) const;
	virtual void setBool(bool val){throw RuntimeException("Array vector doesn't support method setBool");}
	virtual void setChar(char val){throw RuntimeException("Array vector doesn't support method setChar");}
	virtual void setShort(short val){throw RuntimeException("Array vector doesn't support method setShort");}
	virtual void setInt(int val){throw RuntimeException("Array vector doesn't support method setInt");}
	virtual void setLong(long long val){throw RuntimeException("Array vector doesn't support method setLong");}
	virtual void setIndex(INDEX val){throw RuntimeException("Array vector doesn't support method setIndex");}
	virtual void setFloat(float val){throw RuntimeException("Array vector doesn't support method setFloat");}
	virtual void setDouble(double val){throw RuntimeException("Array vector doesn't support method setDouble");}
	virtual void setString(const string& val){throw RuntimeException("Array vector doesn't support method setString");}
	virtual void setNull(){throw RuntimeException("Array vector doesn't support method setNull");}
	virtual char getBool(INDEX index) const {throw RuntimeException("Array vector doesn't support method getBool");}
	virtual char getChar(INDEX index) const {throw RuntimeException("Array vector doesn't support method getChar");}
	virtual short getShort(INDEX index) const {throw RuntimeException("Array vector doesn't support method getShort");}
	virtual int getInt(INDEX index) const {throw RuntimeException("Array vector doesn't support method getInt");}
	virtual long long getLong(INDEX index) const {throw RuntimeException("Array vector doesn't support method getLong");}
	virtual INDEX getIndex(INDEX index) const {throw RuntimeException("Array vector doesn't support method getIndex");}
	virtual float getFloat(INDEX index) const {throw RuntimeException("Array vector doesn't support method getFloat");}
	virtual double getDouble(INDEX index) const {throw RuntimeException("Array vector doesn't support method getDouble");}
	virtual bool isNull(INDEX index) const ;
	virtual string getString(INDEX index) const;
	virtual const DolphinString& getStringRef(INDEX index) const {throw RuntimeException("Array vector doesn't support method getStringRef");}
	virtual void setBool(INDEX index,char val) {throw RuntimeException("Array vector doesn't support method setBool");}
	virtual void setChar(INDEX index,char val){throw RuntimeException("Array vector doesn't support method setChar");}
	virtual void setShort(INDEX index,short val){throw RuntimeException("Array vector doesn't support method setShort");}
	virtual void setInt(INDEX index,int val){throw RuntimeException("Array vector doesn't support method setInt");}
	virtual void setLong(INDEX index,long long val){throw RuntimeException("Array vector doesn't support method setLong");}
	virtual void setIndex(INDEX index, INDEX val){throw RuntimeException("Array vector doesn't support method setIndex");}
	virtual void setFloat(INDEX index,float val){throw RuntimeException("Array vector doesn't support method setBinary");}
	virtual void setDouble(INDEX index, double val){throw RuntimeException("Array vector doesn't support method setDouble");}
	virtual void setString(INDEX index, const DolphinString& val){throw RuntimeException("Array vector doesn't support method setString");}
	virtual void setBinary(INDEX index, int unitLength, const unsigned char* val){throw RuntimeException("Array vector doesn't support method setBinary");}
	virtual void setNull(INDEX index){throw RuntimeException("Array vector doesn't support method setNull");}
	virtual	INDEX reserve(INDEX capacity);
	virtual void clear();
	virtual bool remove(INDEX count);
	virtual bool remove(const ConstantSP& index);
	virtual bool append(const ConstantSP& value);
	virtual bool append(const ConstantSP& value, INDEX count);
	virtual bool append(const ConstantSP& value, INDEX start, INDEX count);
	virtual bool append(const ConstantSP& value, const ConstantSP& index);
	virtual void next(INDEX steps);
	virtual void prev(INDEX steps);
	virtual INDEX size() const { return size_;}
	virtual void nullFill(const ConstantSP& val){throw RuntimeException("Array vector doesn't support method nullFill");}
	virtual bool isNull(INDEX start, int len, char* buf) const;
	virtual bool isValid(INDEX start, int len, char* buf) const;
	virtual char* getBoolBuffer(INDEX start, int len, char* buf) const {throw RuntimeException("Array vector doesn't support method getBoolBuffer");}
	virtual char* getCharBuffer(INDEX start, int len, char* buf) const {throw RuntimeException("Array vector doesn't support method getCharBuffer");}
	virtual short* getShortBuffer(INDEX start, int len, short* buf) const {throw RuntimeException("Array vector doesn't support method getShortBuffer");}
	virtual int* getIntBuffer(INDEX start, int len, int* buf) const {throw RuntimeException("Array vector doesn't support method getIntBuffer");}
	virtual long long* getLongBuffer(INDEX start, int len, long long* buf) const {throw RuntimeException("Array vector doesn't support method getLongBuffer");}
	virtual unsigned char* getBinaryBuffer(INDEX start, int len, int unitLength, unsigned char* buf) const {throw RuntimeException("Array vector doesn't support method getBinaryBuffer");}
	virtual INDEX* getIndexBuffer(INDEX start, int len, INDEX* buf) const {throw RuntimeException("Array vector doesn't support method getIndexBuffer");}
	virtual float* getFloatBuffer(INDEX start, int len, float* buf) const {throw RuntimeException("Array vector doesn't support method getFloatBuffer");}
	virtual double* getDoubleBuffer(INDEX start, int len, double* buf) const {throw RuntimeException("Array vector doesn't support method getDoubleBuffer");}
	virtual bool set(INDEX index, const ConstantSP& value);
	virtual bool set(INDEX column, INDEX row, const ConstantSP& value);
	virtual bool set(const ConstantSP& index, const ConstantSP& value);
	virtual bool setNonNull(const ConstantSP& index, const ConstantSP& value) {throw RuntimeException("Array vector doesn't support method setNonNull");}
	virtual bool assign(const ConstantSP& value);

	virtual long long count() const { return count(0, size_);}
	virtual long long count(INDEX start, INDEX length) const;
	virtual ConstantSP minmax() const {return value_->minmax();}
	virtual ConstantSP minmax(INDEX start, INDEX length) const;
	/**
	 * @param rightMost If there are multiple maximum/minimum values, choose the last one if `rightMost` is true.
	 */
	virtual INDEX imax(bool rightMost = false) const override {
		throw RuntimeException("Array vector doesn't support method imax");
	}
	virtual INDEX imax(INDEX start, INDEX length, bool rightMost = false) const override {
		throw RuntimeException("Array vector doesn't support method imax");
	}
	virtual INDEX imin(bool rightMost = false) const override {
		throw RuntimeException("Array vector doesn't support method imin");
	}
	virtual INDEX imin(INDEX start, INDEX length, bool rightMost = false) const override {
		throw RuntimeException("Array vector doesn't support method imin");
	}

	virtual ConstantSP max() const {return value_->max();}
	virtual ConstantSP max(INDEX start, INDEX length) const;
	virtual void max(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const;
	virtual ConstantSP min() const {return value_->min();}
	virtual ConstantSP min(INDEX start, INDEX length) const;
	virtual void min(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const;
	virtual ConstantSP avg() const {return value_->avg();}
	virtual ConstantSP avg(INDEX start, INDEX length) const;
	virtual void avg(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const;
	virtual ConstantSP sum() const {return value_->sum();}
	virtual ConstantSP sum(INDEX start, INDEX length) const;
	virtual void sum(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const;
	virtual ConstantSP sum2() const {return value_->sum2();}
	virtual ConstantSP sum2(INDEX start, INDEX length) const;
	virtual void sum2(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const;
	virtual ConstantSP prd() const {return value_->prd();}
	virtual ConstantSP prd(INDEX start, INDEX length) const;
	virtual void prd(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const;
	virtual ConstantSP var() const {return value_->var();}
	virtual ConstantSP var(INDEX start, INDEX length) const;
	virtual void var(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const;
	virtual ConstantSP std() const {return value_->std();}
	virtual ConstantSP std(INDEX start, INDEX length) const;
	virtual void std(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const;
	virtual ConstantSP mode() const {return value_->mode();}
	virtual ConstantSP mode(INDEX start, INDEX length) const;
	virtual void mode(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const;
	virtual ConstantSP median() const {return value_->median();}
	virtual ConstantSP median(INDEX start, INDEX length) const;
	virtual void median(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const;
	virtual ConstantSP searchK(INDEX k) const {return value_->searchK(k);}
	virtual ConstantSP searchK(INDEX start, INDEX length, INDEX k) const;
	virtual void searchK(INDEX start, INDEX length, INDEX k, const ConstantSP& out, INDEX outputStart = 0) const;
	virtual ConstantSP firstNot(const ConstantSP& exclude) const {return value_->firstNot(exclude);}
	virtual ConstantSP firstNot(INDEX start, INDEX length, const ConstantSP& exclude) const;
	virtual void firstNot(INDEX start, INDEX length, const ConstantSP& exclude, const ConstantSP& out, INDEX outputStart = 0) const;
	virtual ConstantSP lastNot(const ConstantSP& exclude) const {return value_->lastNot(exclude);}
	virtual ConstantSP lastNot(INDEX start, INDEX length, const ConstantSP& exclude) const;
	virtual void lastNot(INDEX start, INDEX length, const ConstantSP& exclude, const ConstantSP& out, INDEX outputStart = 0) const;
	virtual void neg(){value_->neg();}
	virtual void replace(const ConstantSP& oldVal, const ConstantSP& newVal){throw RuntimeException("Array vector doesn't support method replace");}
	virtual void shuffle(){throw RuntimeException("Array vector doesn't support method shuffle");}
	virtual void reverse(){reverse(0, size_);}
	virtual void reverse(INDEX start, INDEX length);
	virtual bool rank(bool sorted, INDEX* indices, INDEX* ranking){
		throw RuntimeException("Array vector doesn't support method rank");
	}
	virtual void contain(const ConstantSP& target, const ConstantSP& resultSP) const {
		throw RuntimeException("Array vector doesn't support method contain");
	}
	virtual void find(INDEX start, INDEX length, const ConstantSP& target, const ConstantSP& resultSP){
		throw RuntimeException("Array vector doesn't support method find");
	}
	virtual void binarySearch(INDEX start, INDEX length, const ConstantSP& target, const ConstantSP& resultSP){
		throw RuntimeException("Array vector doesn't support method binarySearch");
	}
	virtual void asof(INDEX start, INDEX length, const ConstantSP& target, const ConstantSP& resultSP){
		throw RuntimeException("Array vector doesn't support method asof");
	}
	virtual bool findDuplicatedElements(Vector*  indices, INDEX start, INDEX length, vector<pair<INDEX,INDEX> >& duplicates){
		throw RuntimeException("Array vector doesn't support method findDuplicatedElements");
	}
	virtual bool findDuplicatedElements(INDEX start, INDEX length, vector<pair<INDEX,INDEX> >& duplicates){
		throw RuntimeException("Array vector doesn't support method findDuplicatedElements");
	}
	virtual bool findUniqueElements(INDEX start, INDEX length, vector<pair<INDEX,INDEX> >& uniques){
		throw RuntimeException("Array vector doesn't support method findUniqueElements");
	}
	virtual bool findRange(INDEX* ascIndices,const ConstantSP& target,INDEX* targetIndices,vector<pair<INDEX,INDEX> >& ranges){
		throw RuntimeException("Array vector doesn't support method findRange");
	}
	virtual bool findRange(const ConstantSP& target,INDEX* targetIndices,vector<pair<INDEX,INDEX> >& ranges){
		throw RuntimeException("Array vector doesn't support method findRange");
	}
	virtual INDEX lowerBound(INDEX start, const ConstantSP& target){throw RuntimeException("Array vector doesn't support method lowerBound");}
	virtual long long getAllocatedMemory() const;
	virtual bool isSorted(INDEX start, INDEX length, bool asc, bool strict, char nullOrders = 0) const { return false;}
	virtual ConstantSP topK(INDEX start, INDEX length, INDEX top, bool asc, bool extendEqualValue) const {
		throw RuntimeException("Array vector doesn't support method topK");
	}
	virtual bool sort(bool asc, char nullOrders = 0) {return false;}
	virtual bool sort(bool asc, Vector* indices, char nullOrders = 0) {return false;}
	virtual bool sortSelectedIndices(Vector* indices, INDEX start, INDEX length, bool asc, char nullOrders = 0) { return false;}

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
	virtual int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int& numElement, int& partial) const;
	virtual int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int targetNumElement, int& numElement, int& partial) const;
	virtual IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const {throw RuntimeException("Array vector doesn't support method serialize");}
	virtual IO_ERR deserialize(DataInputStream* in, INDEX indexStart, int offset, INDEX targetNumElement, INDEX& numElement, int& partial);

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

	ConstantSP sliceOneColumn(int colIndex, INDEX rowStart, INDEX rowEnd) const;
	/**
	 * colStart: inclusive
	 * colEnd: exclusive
	 * rowStart: inclusive
	 * rowEnd: exclusive
	 */
	ConstantSP sliceColumnRange(int colStart, int colEnd, INDEX rowStart, INDEX rowEnd) const;
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
	~IotAnyVector() = default;

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

	virtual bool equal(const ConstantSP& other) const;
	virtual bool containNotMarshallableObject() const {
		for (const auto& vec: subVec_) {
			if (!vec.second->containNotMarshallableObject()) {
				return false;
			}
		}
		return true;
	}
	virtual bool isLargeConstant() const {return !isStatic() && !containNotMarshallableObject();}
	virtual bool getNullFlag() const {return containNull_;}
	virtual void setNullFlag(bool containNull){containNull_=containNull;}
	virtual INDEX getCapacity() const {return 0;}
	virtual bool isFastMode() const {return false;}
	virtual short getUnitLength() const {return 0;}
	virtual void clear() {
		subVec_.clear();
		index_.clear();
		types_.clear();
		containNull_ = false;
	}
	virtual bool sizeable() const {return true;}
	virtual DATA_TYPE getRawType() const { return DT_IOTANY;}
	virtual string getString(INDEX index) const { return get(index)->getString(); }
	virtual string getString(Heap* heap, INDEX index) const { return get(index)->getString(); }
	virtual const DolphinString& getStringRef(INDEX index) const { throw RuntimeException("getStringRef method not supported for AnyVector");}
	virtual bool set(INDEX index, const ConstantSP& value, INDEX valueIndex);
	virtual bool set(INDEX index, const ConstantSP& value);
	virtual bool set(const ConstantSP& index, const ConstantSP& value);
	virtual bool setItem(INDEX index, const ConstantSP& value);
	virtual bool assign(const ConstantSP& value);
	virtual ConstantSP get(INDEX index) const;
	virtual ConstantSP get(const ConstantSP& index) const;
	virtual ConstantSP get(INDEX offset, const ConstantSP& index) const override;
	virtual bool hasNull() { return hasNull(0, size()); }
	virtual bool hasNull(INDEX start, INDEX length);
	virtual bool isNull(INDEX index) const;
	virtual bool isNull() const {return false;}
	virtual void setNull(INDEX index) { throw RuntimeException("setNull method not supported for IotAnyVector"); }
	virtual void setNull(){}
	virtual bool setNonNull(const ConstantSP& index, const ConstantSP& value) override { throw RuntimeException("setNonNull method not supported for IotAnyVector"); }
	virtual void fill(INDEX start, INDEX length, const ConstantSP& value, INDEX valueOffset = 0) override;
	virtual void nullFill(const ConstantSP& val);
	virtual bool isNull(INDEX start, int len, char* buf) const override;
	virtual bool isNull(INDEX* indices, int len, char* buf) const override;
	virtual bool isValid(INDEX start, int len, char* buf) const override;
	virtual bool isValid(INDEX* indices, int len, char* buf) const override;
	virtual ConstantSP getSubVector(INDEX start, INDEX length) const;
	virtual ConstantSP getInstance(INDEX size) const {
		VectorSP ret = new IotAnyVector(size);
		if (size > 0) {
			ret->resize(size);
		}
		return ret;
	}
	virtual ConstantSP getValue() const;
	virtual ConstantSP getValue(INDEX capacity) const { return getValue(); }
	virtual bool append(const ConstantSP& value, INDEX start, INDEX len) override;
	virtual bool append(const ConstantSP& value) override {return append(value, 0, value->size());}
	virtual bool append(const ConstantSP& value, INDEX count) override {return append(value, 0, count);}
	virtual bool append(const ConstantSP& value, const ConstantSP& index) override {return append(value->get(index), 0, index->size());}

	virtual bool remove(INDEX count);
	virtual void find(INDEX start, INDEX length, const ConstantSP& targetSP, const ConstantSP& resultSP){
		throw RuntimeException("find method not supported for AnyVector");
	}
	virtual char getBool() const;
	virtual char getChar() const;
	virtual short getShort() const;
	virtual int getInt() const;
	virtual long long getLong() const;
	virtual INDEX getIndex() const;
	virtual float getFloat() const;
	virtual double getDouble() const;
	virtual char getBool(INDEX index) const {return get(index)->getBool();}
	virtual char getChar(INDEX index) const { return get(index)->getChar();}
	virtual short getShort(INDEX index) const { return get(index)->getShort();}
	virtual int getInt(INDEX index) const {return get(index)->getInt();}
	virtual long long getLong(INDEX index) const {return get(index)->getLong();}
	virtual INDEX getIndex(INDEX index) const {return get(index)->getIndex();}
	virtual float getFloat(INDEX index) const {return get(index)->getFloat();}
	virtual double getDouble(INDEX index) const {return get(index)->getDouble();}
	virtual bool getBool(INDEX start, int len, char* buf) const;
	virtual bool getChar(INDEX start, int len,char* buf) const;
	virtual bool getShort(INDEX start, int len, short* buf) const;
	virtual bool getInt(INDEX start, int len, int* buf) const;
	virtual bool getLong(INDEX start, int len, long long* buf) const;
	virtual bool getIndex(INDEX start, int len, INDEX* buf) const;
	virtual bool getFloat(INDEX start, int len, float* buf) const;
	virtual bool getDouble(INDEX start, int len, double* buf) const;
	virtual const char* getBoolConst(INDEX start, int len, char* buf) const;
	virtual const char* getCharConst(INDEX start, int len,char* buf) const;
	virtual const short* getShortConst(INDEX start, int len, short* buf) const;
	virtual const int* getIntConst(INDEX start, int len, int* buf) const;
	virtual const long long* getLongConst(INDEX start, int len, long long* buf) const;
	virtual const INDEX* getIndexConst(INDEX start, int len, INDEX* buf) const;
	virtual const float* getFloatConst(INDEX start, int len, float* buf) const;
	virtual const double* getDoubleConst(INDEX start, int len, double* buf) const;
	virtual bool getSymbol(INDEX start, int len, int* buf, SymbolBase* symBase, bool insertIfNotThere) const {
		throw RuntimeException("getSymbol method not supported for AnyVector");
	}
	virtual const int* getSymbolConst(INDEX start, int len, int* buf, SymbolBase* symBase, bool insertIfNotThere) const {
		throw RuntimeException("getSymbolConst method not supported for AnyVector");
	}
	virtual bool getString(INDEX start, int len, DolphinString** buf) const {
		throw RuntimeException("getString method not supported for AnyVector");
	}

	virtual bool getString(INDEX start, int len, char** buf) const {
		throw RuntimeException("getString method not supported for AnyVector");
	}

	virtual DolphinString** getStringConst(INDEX start, int len, DolphinString** buf) const {
		throw RuntimeException("getStringConst method not supported for AnyVector");
	}

	virtual char** getStringConst(INDEX start, int len, char** buf) const {
		throw RuntimeException("getStringConst method not supported for AnyVector");
	}

	virtual	INDEX reserve(INDEX capacity) override {
		index_.reserve(capacity);
		types_.reserve(capacity);
		return capacity;
	}
	virtual void resize(INDEX size) override;

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
	virtual INDEX size() const {return index_.size();}
	virtual long long count() const{
		return count(0, size());
	}
	virtual long long count(INDEX start, INDEX length) const;
	/**
	 * @param rightMost If there are multiple maximum/minimum values, choose the last one if `rightMost` is true.
	 */
	virtual INDEX imax(bool rightMost = false) const override {throw RuntimeException("imax method not supported for AnyVector");}
	virtual INDEX imin(bool rightMost = false) const override {throw RuntimeException("imin method not supported for AnyVector");}
	virtual INDEX imax(INDEX start, INDEX length, bool rightMost = false) const override {throw RuntimeException("imax method not supported for AnyVector");}
	virtual INDEX imin(INDEX start, INDEX length, bool rightMost = false) const override {throw RuntimeException("imin method not supported for AnyVector");}

	virtual void prev(INDEX steps) {throw RuntimeException("prev method not supported for IotAnyVector");}
	virtual void next(INDEX steps) {throw RuntimeException("next method not supported for IotAnyVector");}

	virtual ConstantSP avg() const { return avg(0, size()); }
	virtual ConstantSP avg(INDEX start, INDEX length) const;
	virtual void avg(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const { out->set(outputStart, avg(start, length)); }
	virtual ConstantSP sum() const { return sum(0, size()); }
	virtual ConstantSP sum(INDEX start, INDEX length) const;
	virtual void sum(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const { out->set(outputStart, sum(start, length)); }
	virtual ConstantSP sum2() const {throw RuntimeException("sum2 method not supported for IotAnyVector");}
	virtual ConstantSP sum2(INDEX start, INDEX length) const {throw RuntimeException("sum2 method not supported for IotAnyVector");}
	virtual void sum2(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const {throw RuntimeException("sum2 method not supported for IotAnyVector");}
	virtual ConstantSP prd() const {throw RuntimeException("prd method not supported for IotAnyVector");}
	virtual ConstantSP prd(INDEX start, INDEX length) const {throw RuntimeException("prd method not supported for IotAnyVector");}
	virtual void prd(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const {throw RuntimeException("prd method not supported for IotAnyVector");}
	virtual ConstantSP var() const {throw RuntimeException("var method not supported for IotAnyVector");}
	virtual ConstantSP var(INDEX start, INDEX length) const {throw RuntimeException("var method not supported for IotAnyVector");}
	virtual void var(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const {throw RuntimeException("var method not supported for IotAnyVector");}
	virtual ConstantSP std() const {throw RuntimeException("std method not supported for IotAnyVector");}
	virtual ConstantSP std(INDEX start, INDEX length) const {throw RuntimeException("std method not supported for IotAnyVector");}
	virtual void std(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const {throw RuntimeException("std method not supported for IotAnyVector");}
	virtual ConstantSP median() const {throw RuntimeException("median method not supported for IotAnyVector");}
	virtual ConstantSP median(INDEX start, INDEX length) const {throw RuntimeException("median method not supported for IotAnyVector");}
	virtual void median(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart=0) const {throw RuntimeException("median method not supported for IotAnyVector");}

	virtual ConstantSP firstNot(const ConstantSP& exclude) const {throw RuntimeException("firstNot method not supported for IotAnyVector");}
	virtual ConstantSP firstNot(INDEX start, INDEX length, const ConstantSP& exclude) const {throw RuntimeException("firstNot method not supported for IotAnyVector");}
	virtual void firstNot(INDEX start, INDEX length, const ConstantSP& exclude, const ConstantSP& out, INDEX outputStart = 0) const {throw RuntimeException("firstNot method not supported for IotAnyVector");}
	virtual ConstantSP lastNot(const ConstantSP& exclude) const {throw RuntimeException("lastNot method not supported for IotAnyVector");}
	virtual ConstantSP lastNot(INDEX start, INDEX length, const ConstantSP& exclude) const {throw RuntimeException("lastNot method not supported for IotAnyVector");}
	virtual void lastNot(INDEX start, INDEX length, const ConstantSP& exclude, const ConstantSP& out, INDEX outputStart = 0) const {throw RuntimeException("lastNot method not supported for IotAnyVector");}
	virtual ConstantSP searchK(INDEX k) const {throw RuntimeException("searchK method not supported for AnyVector");}
	virtual ConstantSP searchK(INDEX start, INDEX length, INDEX k) const {throw RuntimeException("searchK method not supported for IotAnyVector");}
	virtual void searchK(INDEX start, INDEX length, INDEX k, const ConstantSP& out, INDEX outputStart=0) const {throw RuntimeException("searchK method not supported for IotAnyVector");}
	virtual ConstantSP mode() const {throw RuntimeException("mode method not supported for AnyVector");}
	virtual ConstantSP mode(INDEX start, INDEX length) const {throw RuntimeException("mode method not supported for IotAnyVector");}
	virtual void mode(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart=0) const {
		throw RuntimeException("mode method not supported for AnyVector");
	}
	virtual ConstantSP min() const { return min(0, size()); }
	virtual ConstantSP min(INDEX start, INDEX length) const;
	virtual void min(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart=0) const {out->set(outputStart, min(start, length));}
	virtual ConstantSP max() const { return max(0, size()); }
	virtual ConstantSP max(INDEX start, INDEX length) const;
	virtual void max(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart=0) const {out->set(outputStart, max(start, length));}
	virtual void neg(){throw RuntimeException("neg method not supported for IotAnyVector");}
	virtual void reverse(){std::reverse(index_.begin(),index_.end());}
	virtual void reverse(INDEX start, INDEX length){
		std::reverse(index_.begin()+start,index_.begin()+ start + length);
	}
	virtual void replace(const ConstantSP& oldVal, const ConstantSP& newVal) { throw RuntimeException("replace method not supported for IotAnyVector"); }
	virtual void shuffle();
	virtual bool findDuplicatedElements(Vector* indices, INDEX start, INDEX length, vector<pair<INDEX,INDEX> >& duplicates){return false;}
	virtual bool findDuplicatedElements(INDEX start, INDEX length, vector<pair<INDEX,INDEX> >& duplicates){return false;}
	virtual bool findUniqueElements(INDEX start, INDEX length, vector<pair<INDEX,INDEX> >& uniques){return false;}
	virtual bool findRange(INDEX* ascIndices,const ConstantSP& target,INDEX* targetIndices,vector<pair<INDEX,INDEX> >& ranges){return false;}
	virtual bool findRange(const ConstantSP& target,INDEX* targetIndices,vector<pair<INDEX,INDEX> >& ranges){return false;}
	virtual INDEX lowerBound(INDEX start, const ConstantSP& target){
		throw RuntimeException("lowerBound method not supported for AnyVector");
	}
	virtual bool rank(bool sorted, INDEX* indices, INDEX* ranking){return false;}
	virtual bool sortSelectedIndices(Vector* indices, INDEX start, INDEX length, bool asc, char nullsOrder){	return false;}
	virtual bool isSorted(INDEX start, INDEX length, bool asc, bool strict, char nullsOrder) const { return false;}
	virtual bool sort(bool asc, char nullsOrder){return false;}
	virtual bool sort(bool asc, Vector* indices, char nullsOrder){ return false;}
	virtual INDEX sortTop(bool asc, Vector* indices, INDEX top, char nullsOrder){ return -1;}
	virtual long long getAllocatedMemory() const override;
	// virtual int getExtraParamForType() const override { return dt_; }

	virtual IO_ERR serialize(Heap* pHeap, const ByteArrayCodeBufferSP& buffer) const override;

	virtual int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int& numElement, int& partial) const override {
		throw RuntimeException("the vector doesn't support that serialize method, use ConstantMarshal to serialize");
	}
	virtual IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const override {
		return serialize(nullptr, buffer);
	}

	void collectUserDefinedFunctions(unordered_map<string,FunctionDef*>& functionDefs) const;
	ConstantSP convertToRegularVector() const;

	// group elements by type
	struct TypeGroup {
		size_t start;
		size_t length;
		DATA_TYPE type;
	};
	vector<TypeGroup> findUniqueTypesElements(INDEX start, INDEX length) const;

    static SmartPointer<IotAnyVector> cast(const VectorSP& vec) {
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

#endif /* SPECIALCONSTANT_H_ */
