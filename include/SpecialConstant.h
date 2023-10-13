/*
 * SpecialConstant.h
 *
 *  Created on: Mar 22, 2021
 *      Author: dzhou
 */

#ifndef SPECIALCONSTANT_H_
#define SPECIALCONSTANT_H_

#include "CoreConcept.h"
#include "Util.h"

class AnyVector:public Vector{
public:
	AnyVector(int size);
	AnyVector(const deque<ConstantSP>& data, bool containNull): Vector(DT_ANY, MIXED), data_(data), containNull_(containNull), isDim_(false){}
	AnyVector(const vector<ConstantSP>& data, bool containNull): Vector(DT_ANY, MIXED), data_(data.begin(), data.end()), containNull_(containNull), isDim_(false){}
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
	virtual bool setItem(INDEX index, const ConstantSP& value);
	virtual bool assign(const ConstantSP& value);
	virtual ConstantSP get(INDEX index) const {return data_[index];}
	virtual ConstantSP get(const ConstantSP& index) const;
	virtual bool hasNull(){return  hasNull(0, data_.size());}
	virtual bool hasNull(INDEX start, INDEX length);
	virtual bool isNull(INDEX index) const {return data_[index]->isNull();}
	virtual bool isNull() const {return false;}
	virtual void setNull(INDEX index);
	virtual void setNull(){}
	virtual void fill(INDEX start, INDEX length, const ConstantSP& value, INDEX valueOffset = 0) override;
	virtual void nullFill(const ConstantSP& val);
	virtual bool isNull(INDEX start, int len, char* buf) const;
	virtual bool isValid(INDEX start, int len, char* buf) const;
	virtual ConstantSP getSubVector(INDEX start, INDEX length) const;
	virtual ConstantSP getInstance(INDEX size) const {return ConstantSP(new AnyVector(size));}
	virtual ConstantSP getValue() const;
	virtual ConstantSP getValue(INDEX capacity) const {return ConstantSP(new AnyVector(data_, containNull_));}
	virtual bool append(const ConstantSP& value);
	virtual bool append(const ConstantSP& value, INDEX appendSize){return false;}
	virtual bool remove(INDEX count);
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
	virtual INDEX size() const {return data_.size();}
	virtual long long count() const{
		return count(0, data_.size());
	}
	virtual long long count(INDEX start, INDEX length) const;
	virtual INDEX imax() const {throw RuntimeException("imax method not supported for AnyVector");}
	virtual INDEX imin() const {throw RuntimeException("imin method not supported for AnyVector");}
	virtual INDEX imax(INDEX start, INDEX length) const {throw RuntimeException("imax method not supported for AnyVector");}
	virtual INDEX imin(INDEX start, INDEX length) const {throw RuntimeException("imin method not supported for AnyVector");}

	virtual ConstantSP avg() const {return avg(0, data_.size());}
	virtual ConstantSP avg(INDEX start, INDEX length) const;
	virtual void avg(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const {out->setNull(outputStart);}
	virtual ConstantSP sum() const {return sum(0, data_.size());}
	virtual ConstantSP sum(INDEX start, INDEX length) const;
	virtual void sum(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const {out->setNull(outputStart);}
	virtual ConstantSP sum2() const {return sum2(0, data_.size());}
	virtual ConstantSP sum2(INDEX start, INDEX length) const;
	virtual void sum2(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const {out->setNull(outputStart);}
	virtual ConstantSP prd() const {return prd(0, data_.size());}
	virtual ConstantSP prd(INDEX start, INDEX length) const;
	virtual void prd(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const {out->setNull(outputStart);}
	virtual ConstantSP var() const {return var(0, data_.size());}
	virtual ConstantSP var(INDEX start, INDEX length) const;
	virtual void var(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const {out->setNull(outputStart);}
	virtual ConstantSP std() const {return std(0, data_.size());}
	virtual ConstantSP std(INDEX start, INDEX length) const;
	virtual void std(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart = 0) const {out->setNull(outputStart);}
	virtual ConstantSP median() const {return median(0, data_.size());}
	virtual ConstantSP median(INDEX start, INDEX length) const;
	virtual void median(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart=0) const {out->setNull(outputStart);}

	virtual ConstantSP firstNot(const ConstantSP& exclude) const {throw RuntimeException("firstNot method not supported for AnyVector");}
	virtual ConstantSP firstNot(INDEX start, INDEX length, const ConstantSP& exclude) const {return firstNot(exclude);}
	virtual void firstNot(INDEX start, INDEX length, const ConstantSP& exclude, const ConstantSP& out, INDEX outputStart = 0) const {
		throw RuntimeException("firstNot method not supported for AnyVector");
	}
	virtual ConstantSP lastNot(const ConstantSP& exclude) const { throw RuntimeException("lastNot method not supported for AnyVector");}
	virtual ConstantSP lastNot(INDEX start, INDEX length, const ConstantSP& exclude) const {return lastNot(exclude);}
	virtual void lastNot(INDEX start, INDEX length, const ConstantSP& exclude, const ConstantSP& out, INDEX outputStart = 0) const {
		throw RuntimeException("lastNot method not supported for AnyVector");
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
	virtual ConstantSP min() const { throw RuntimeException("min method not supported for AnyVector");}
	virtual ConstantSP min(INDEX start, INDEX length) const { throw RuntimeException("min method not supported for AnyVector");}
	virtual void min(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart=0) const {
		throw RuntimeException("min method not supported for AnyVector");
	}
	virtual ConstantSP max() const { throw RuntimeException("max method not supported for AnyVector");}
	virtual ConstantSP max(INDEX start, INDEX length) const { throw RuntimeException("max method not supported for AnyVector");}
	virtual void max(INDEX start, INDEX length, const ConstantSP& out, INDEX outputStart=0) const {
		throw RuntimeException("max method not supported for AnyVector");
	}

	virtual void neg(){throw RuntimeException("neg method not supported for AnyVector");}
	virtual void reverse(){std::reverse(data_.begin(),data_.end());}
	virtual void reverse(INDEX start, INDEX length){
		std::reverse(data_.begin()+start,data_.begin()+ start + length);
	}
	virtual void replace(const ConstantSP& oldVal, const ConstantSP& newVal){
		throw RuntimeException("replace method not supported for AnyVector");
	}
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
	void collectUserDefinedFunctions(unordered_map<string,FunctionDef*>& functionDefs) const;
	bool isHomogeneousScalar(DATA_TYPE& type) const;
	bool isHomogeneousScalarOrArray(DATA_TYPE& type) const;
	bool isConsistent() const;
	bool isTabular() const;
	ConstantSP convertToRegularVector() const;
	bool isDimension() const { return isDim_;}
	void setDimension(bool option) { isDim_ = option;}
	inline long long getAllocatedMemory() const override {
		long long memSize = 0;
		for (size_t i = 0; i < data_.size(); ++i) {
			memSize += data_[i]->getAllocatedMemory();
		}
		return memSize;
	}
	const ConstantSP& getConstant(INDEX index) const { return data_[index];}
	ConstantSP flatten(INDEX rowStart, INDEX count) const;

private:
	mutable deque<ConstantSP> data_;
	bool containNull_;
	bool isDim_;
};

class SubVector : public Vector {
public:
	SubVector(const VectorSP& target, INDEX offset, INDEX length, bool updatable = false);
	void reset(const VectorSP& target, INDEX offset, INDEX length);
	void reset(INDEX offset, INDEX length);
	virtual ~SubVector(){}
	virtual bool isLargeConstant() const {return true;}
	VectorSP getSourceVector() const { return source_;}
	INDEX getSubVectorStart() const { return offset_;}
	INDEX getSubVectorLength() const { return size_;}
	virtual bool copyable() const {return false;}
	virtual bool isView() const {return true;}
	virtual DATA_TYPE getRawType() const {return source_->getRawType();}
	virtual int getExtraParamForType() const { return source_->getExtraParamForType();}
	virtual SymbolBaseSP getSymbolBase() const {return source_->getSymbolBase();}
	virtual DATA_FORM getForm() const { return DF_VECTOR;}
	virtual ConstantSP getInstance() const {return getInstance(size_);}
	virtual ConstantSP getInstance(INDEX size) const;
	virtual ConstantSP getValue() const { return source_->getSubVector(offset_, size_);}
	virtual ConstantSP getValue(INDEX capacity) const { return source_->getSubVector(offset_, size_, capacity);}
	virtual ConstantSP get(INDEX column, INDEX rowStart,INDEX rowEnd) const {return source_->getSubVector(offset_ + rowStart,rowEnd-rowStart);}
	virtual ConstantSP getWindow(INDEX colStart, int colLength, INDEX rowStart, int rowLength) const {return source_->getSubVector(offset_ + rowStart,rowLength);}
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
			
			memset(buf, 0, cur * unitLength);

			start += cur;
			len -= cur;
			buf += cur * unitLength;
		}
		int validLen = std::max(0, std::min(len, std::min(size_ - start, source_->size() - offset_ - start)));
		if(offset_ + start < source_->size() && validLen > 0 && !source_->getBinary(offset_ + start, validLen, unitLength, buf)) {
			return false;
		}
		if (validLen < len) {
			memset(buf + unitLength * validLen, 0, (len - validLen) * unitLength);
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
			
			memset(buf, 0, cur * unitLength);

			start += cur;
			len -= cur;
			buf += cur * unitLength;
		}
		int validLen = std::max(0, std::min(len, std::min(size_ - start, source_->size() - offset_ - start)));
		if (offset_ + start < source_->size() && validLen > 0)
			source_->getBinary(offset_ + start, validLen, unitLength, buf);
		if (validLen < len) {
			memset(buf + unitLength * validLen, 0, (len - validLen) * unitLength);
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

	virtual bool appendBool(char* buf, int len){throw RuntimeException("Immutable sub vector doesn't support method appendBool");}
	virtual bool appendChar(char* buf, int len){throw RuntimeException("Immutable sub vector doesn't support method appendChar");}
	virtual bool appendShort(short* buf, int len){throw RuntimeException("Immutable sub vector doesn't support method appendShort");}
	virtual bool appendInt(int* buf, int len){throw RuntimeException("Immutable sub vector doesn't support method appendInt");}
	virtual bool appendLong(long long* buf, int len){throw RuntimeException("Immutable sub vector doesn't support method appendLong");}
	virtual bool appendIndex(INDEX* buf, int len){throw RuntimeException("Immutable sub vector doesn't support method appendIndex");}
	virtual bool appendFloat(float* buf, int len){throw RuntimeException("Immutable sub vector doesn't support method appendFloat");}
	virtual bool appendDouble(double* buf, int len){throw RuntimeException("Immutable sub vector doesn't support method appendDouble");}
	virtual bool assign(const ConstantSP& value){throw RuntimeException("Immutable sub vector doesn't support method assign");}

	virtual INDEX imax() const override { return imax(0, size_); }
	virtual INDEX imax(INDEX start, INDEX length) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		INDEX index = source_->imax(range.first, range.second);
		return index >= 0 ? index - offset_ : index;
	}
	virtual INDEX imin() const override { return imin(0, size_); }
	virtual INDEX imin(INDEX start, INDEX length) const override {
		auto range = calculateOverlappedRange(start + offset_, length);
		INDEX index = source_->imin(range.first, range.second);
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

	virtual INDEX lowerBound(INDEX start, const ConstantSP& target) override {
		auto range = calculateOverlappedRange(start + offset_, size_);
		return source_->lowerBound(range.first, target);  // FIXME: ???
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
	virtual long long getAllocatedMemory() const {return sizeof(SubVector);}
	virtual int serialize(char* buf, int bufSize, INDEX indexStart, int offset, int& numElement, int& partial) const {
		throw RuntimeException("Immutable sub vector doesn't support method serialize");
	}
	virtual IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const {throw RuntimeException("Immutable sub vector doesn't support method serialize");}
	virtual bool isSorted(INDEX start, INDEX length, bool asc, bool strict, char nullsOrder = 0) const {
		auto range = calculateOverlappedRange(start + offset_, length);
		return source_->isSorted(range.first, range.second, asc, strict);
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

private:
	VectorSP source_;
	INDEX offset_;
	INDEX size_;
	bool updatable_;
};

#endif /* SPECIALCONSTANT_H_ */
