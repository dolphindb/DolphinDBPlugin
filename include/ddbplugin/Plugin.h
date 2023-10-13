/*
* Plugin.h
*
*  Created on: Aug 22, 2022
*      Author: taoping.yu
*/

#ifndef DDBPLUGIN_H_
#define DDBPLUGIN_H_

#include "Concurrent.h"
#include "CoreConcept.h"
#include "Exceptions.h"
#include "Logger.h"
#include <functional>
#include <cassert>

namespace dolphindb {

template <typename T>
inline T getNullValue();
template <>
inline char getNullValue<char>() { return CHAR_MIN; }
template <>
inline short getNullValue<short>() { return SHRT_MIN; }
template <>
inline int getNullValue<int>() { return INT_MIN; }
template <>
inline long int getNullValue<long int>() { return LONG_MIN; }
template <>
inline long long getNullValue<long long>() { return LLONG_MIN; }
template <>
inline float getNullValue<float>() { return FLT_NMIN; }
template <>
inline double getNullValue<double>() { return DBL_NMIN; }
template <>
inline string getNullValue<string>() { return ""; }
template <>
inline Guid getNullValue<Guid>() { return Guid(); }

template <class T>
class DdbVector {
public:
	DdbVector(int size, int capacity = 0) : size_(size), capacity_(capacity), dataNeedRelease_(true), containNull_(false){
		if (capacity_ < size_)
			capacity_ = size_;
		if (capacity_ < 1) {
			throw RuntimeException("can't create empty DdbVector.");
		}
		data_ = new T[capacity_];
	}
	//DdbVector own data and it will be released, don't delete data in the future.
	DdbVector(T *data, int size, int capacity = 0) : data_(data), size_(size), capacity_(capacity), dataNeedRelease_(true), containNull_(false) {
		if (capacity_ < size_)
			capacity_ = size_;
	}
	DdbVector(const DdbVector &src) = delete;
	~DdbVector() {
		if (dataNeedRelease_) {
			delete[] data_;
		}
	}
	int size() const {
		return size_;
	}
	const T* data() const {
		assert(dataNeedRelease_);
		return data_;
	}
	T* data() {
		assert(dataNeedRelease_);
		return data_;
	}
	void addNull() {
		add(getNullValue<T>());
		containNull_ = true;
	}
	void add(const T& value) {
		assert(dataNeedRelease_);
		assert(size_  < capacity_);
		if(containNull_ == false && value == getNullValue<T>()){
			containNull_ = true;
		}
		data_[size_++] = value;
	}
	void add(T&& value) {
		assert(dataNeedRelease_);
		assert(size_  < capacity_);
		if (containNull_ == false && value == getNullValue<T>()) {
			containNull_ = true;
		}
		data_[size_++] = std::move(value);
	}
	void appendString(const string *buf, int len) {
		assert(dataNeedRelease_);
		assert(size_ + len <= capacity_);
		for (auto i = 0; i < len; i++) {
			if (containNull_ == false && buf[i] == getNullValue<T>()) {
				containNull_ = true;
			}
			data_[size_++] = std::move(buf[i]);
		}
	}
	void appendString(string *buf, int len) {
		assert(dataNeedRelease_);
		assert(size_ + len <= capacity_);
		for (auto i = 0; i < len; i++) {
			if (containNull_ == false && buf[i] == getNullValue<T>()) {
				containNull_ = true;
			}
			data_[size_++] = std::move(buf[i]);
		}
	}
	//This function is invalid for string DdbVector, please use appendString instead.
	void append(const T *buf, int len) {
		assert(dataNeedRelease_);
		assert(size_ + len <= capacity_);
		if (containNull_ == false) {
			for (auto i = 0; i < len; i++) {
				if (buf[i] == getNullValue<T>()) {
					containNull_ = true;
					break;
				}
			}
		}
		memcpy(data_ + size_, buf, len * sizeof(T));
		size_ += len;
	}
	void setNull(int index) {
		set(index, getNullValue<T>());
		containNull_ = true;
	}
	void set(int index, const T& value) {
		assert(dataNeedRelease_);
		assert(index < size_);
		if (containNull_ == false && value == getNullValue<T>()) {
			containNull_ = true;
		}
		data_[index] = value;
	}
	void set(int index, T&& value) {
		assert(dataNeedRelease_);
		assert(index < size_);
		if (containNull_ == false && value == getNullValue<T>()) {
			containNull_ = true;
		}
		data_[index] = std::move(value);
	}
	void setString(int start, int len, const string *buf) {
		assert(dataNeedRelease_);
		assert(start < size_);
		assert(start + len <= size_);
		for (auto i = 0, index = start; i < len; i++, index++) {
			if (containNull_ == false && buf[i] == getNullValue<T>()) {
				containNull_ = true;
			}
			data_[index] = std::move(buf[i]);
		}
	}
	void setString(int start, int len, string *buf) {
		assert(dataNeedRelease_);
		assert(start < size_);
		assert(start + len <= size_);
		for (auto i = 0, index = start; i < len; i++, index++) {
			if (containNull_ == false && buf[i] == getNullValue<T>()) {
				containNull_ = true;
			}
			data_[index] = std::move(buf[i]);
		}
	}
	//This function is invalid for string DdbVector, please use appendString instead.
	void set(int start, int len, const T *buf) {
		assert(dataNeedRelease_);
		assert(start < size_);
		assert(start + len <= size_);
		if (containNull_ == false) {
			for (auto i = 0; i < len; i++) {
				if (buf[i] == getNullValue<T>()) {
					containNull_ = true;
					break;
				}
			}
		}
		memcpy(data_ + start, buf, len * sizeof(T));
	}
	Vector* createVector(DATA_TYPE type, int extraParam = 0) {
		if (dataNeedRelease_ == false) {
			throw RuntimeException(Util::getDataTypeString(type) + "'s createVector can only be called once.");
		}
		if (type != DT_STRING && type != DT_BLOB && type != DT_SYMBOL) {
			assert(Util::getDataTypeSize(type) == sizeof(T));
			Vector* pVector;
			pVector = Util::createVector(type, size_, size_, true, extraParam, data_, 0, 0, containNull_);
			dataNeedRelease_ = false;
			return pVector;
		}
		else {
			Vector* pVector = Util::createVector(type, 0, size_, true, extraParam);
			pVector->appendString((string*)data_, size_);
			return pVector;
		}
	}
private:
	T * data_;
	int size_;
	int capacity_;
	bool dataNeedRelease_, containNull_;
};

class Executor : public Runnable {
    using Func = std::function<void()>;

public:
    explicit Executor(Func f) : func_(std::move(f)) {};
    void run() override {
        try{
            func_();
        }
        catch(...){
            LOG_ERR("an uncaught exception was found");
        }
    };

private:
    Func func_;
};

class PUtil {
public:
	static std::string  getVersionMarker(){
		return "Ddb_Plugin_Version: 1.30.20.1";
	}

	static void enumBoolVector(const VectorSP &pVector, std::function<bool(const char *pbuf, INDEX startIndex, INDEX size)> func, INDEX offset = 0) {
		enumDdbVector<char>(pVector, &Vector::getBoolConst, func, offset);
	}
	static void enumIntVector(const VectorSP &pVector, std::function<bool(const int *pbuf, INDEX startIndex, int length)> func, INDEX offset = 0) {
		enumDdbVector<int>(pVector, &Vector::getIntConst, func, offset);
	}
	static void enumShortVector(const VectorSP &pVector, std::function<bool(const short *pbuf, INDEX startIndex, int length)> func, INDEX offset = 0) {
		enumDdbVector<short>(pVector, &Vector::getShortConst, func, offset);
	}
	static void enumCharVector(const VectorSP &pVector, std::function<bool(const char *pbuf, INDEX startIndex, int length)> func, INDEX offset = 0) {
		enumDdbVector<char>(pVector, &Vector::getCharConst, func, offset);
	}
	static void enumLongVector(const VectorSP &pVector, std::function<bool(const long long *pbuf, INDEX startIndex, int length)> func, INDEX offset = 0) {
		enumDdbVector<long long>(pVector, &Vector::getLongConst, func, offset);
	}
	static void enumFloatVector(const VectorSP &pVector, std::function<bool(const float *pbuf, INDEX startIndex, int length)> func, INDEX offset = 0) {
		enumDdbVector<float>(pVector, &Vector::getFloatConst, func, offset);
	}
	static void enumDoubleVector(const VectorSP &pVector, std::function<bool(const double *pbuf, INDEX startIndex, int length)> func, INDEX offset = 0) {
		enumDdbVector<double>(pVector, &Vector::getDoubleConst, func, offset);
	}
	static void enumStringVector(const VectorSP &pVector, std::function<bool(DolphinString **pbuf, INDEX startIndex, int length)> func, INDEX offset = 0) {
		if(offset >= pVector->size()) return;
		DolphinString* buffer[Util::BUF_SIZE];
		DolphinString** pbuf;
		INDEX startIndex = offset;
		int size;
		INDEX leftSize = pVector->size() - offset;
		while (leftSize > 0) {
			size = leftSize;
			if (size > Util::BUF_SIZE)
				size = Util::BUF_SIZE;
			pbuf = pVector->getStringConst(startIndex, size, buffer);
			if (func(pbuf, startIndex, size) == false)
				break;
			leftSize -= size;
			startIndex += size;
		}
	}
	static void enumInt128Vector(const VectorSP &pVector, std::function<bool(const Guid *pbuf, INDEX startIndex, int length)> func, INDEX offset = 0) {
		enumBinaryVector<Guid>(pVector, func, offset);
	}
	static void enumDecimal32Vector(const VectorSP &pVector, std::function<bool(const int32_t *pbuf, INDEX startIndex, int length)> func, INDEX offset = 0) {
		enumBinaryVector<int32_t>(pVector, func, offset);
	}
	static void enumDecimal64Vector(const VectorSP &pVector, std::function<bool(const int64_t *pbuf, INDEX startIndex, int length)> func, INDEX offset = 0) {
		enumBinaryVector<int64_t>(pVector, func, offset);
	}
private:
	template <class T>
	static void enumBinaryVector(const VectorSP &pVector, std::function<bool(const T *pbuf, INDEX startIndex, int length)> func, INDEX offset = 0) {
		if(offset >= pVector->size()) return;
		vector<T> buffer(Util::BUF_SIZE);
		const T* pbuf;
		INDEX startIndex = offset;
		int size;
		INDEX leftSize = pVector->size() - offset;
		while (leftSize > 0) {
			size = leftSize;
			if (size > Util::BUF_SIZE)
				size = Util::BUF_SIZE;
			pbuf = (const T*)pVector->getBinaryConst(startIndex, size, sizeof(T), (unsigned char*)buffer.data());
			if (func(pbuf, startIndex, size) == false)
				break;
			leftSize -= size;
			startIndex += size;
		}
	}
	template <class T>
	static void enumDdbVector(const VectorSP &pVector,
		const T* (Vector::*getConst)(INDEX, int, T*) const,
		std::function<bool(const T *buf, INDEX startIndex, int length)> func, INDEX offset = 0) {
		if(offset >= pVector->size()) return;
		T buffer[Util::BUF_SIZE];
		const T *pbuf;
		INDEX startIndex = offset;
		int size;
		INDEX leftSize = pVector->size() - offset;
		while (leftSize > 0) {
			size = leftSize;
			if (size > Util::BUF_SIZE)
				size = Util::BUF_SIZE;
			pbuf = (pVector.get()->*getConst)(startIndex, size, buffer);
			if (func(pbuf, startIndex, size) == false)
				break;
			leftSize -= size;
			startIndex += size;
		}
	}
};



class ResultSet {
public:
	ResultSet(const TableSP &table)
		: table_(table), position_(0)
	{
		rows_ = table_->rows();
		cols_ = table_->columns();
		column_ = new ColumnPointer[cols_];
		for (int i = 0; i < cols_; i++) {
			column_[i].pVector = table_->getColumn(i);
			VectorSP &pVector = column_[i].pVector;
			DATA_TYPE type = pVector->getRawType();
			switch (type) {
			case DT_BOOL:
				column_[i].charCol = new Column<char>(pVector, [=](const VectorSP &pVector, INDEX position, int len, char *buf) {
					return pVector->getBoolConst(position, len, buf);
				});
				break;
			case DT_CHAR:
				column_[i].charCol = new Column<char>(pVector, [=](const VectorSP &pVector, INDEX position, int len, char *buf) {
					return pVector->getCharConst(position, len, buf);
				});
				break;
			case DT_SHORT:
				column_[i].shortCol = new Column<short>(pVector, [=](const VectorSP &pVector, INDEX position, int len, short *buf) {
					return pVector->getShortConst(position, len, buf);
				});
				break;
			case DT_INT:
				if (pVector->getType() == DT_SYMBOL) {
					column_[i].stringCol = new Column<DolphinString*>(pVector, [=](const VectorSP &pVector, INDEX position, int len, DolphinString** buf) {
						return pVector->getStringConst(position, len, buf);
					});
				}
				else{
					column_[i].intCol = new Column<int>(pVector, [=](const VectorSP &pVector, INDEX position, int len, int *buf) {
						return pVector->getIntConst(position, len, buf);
					});
				}
				break;
			case DT_TIMESTAMP:
			case DT_LONG:
				column_[i].longCol = new Column<long long>(pVector, [=](const VectorSP &pVector, INDEX position, int len, long long *buf) {
					return pVector->getLongConst(position, len, buf);
				});
				break;
			case DT_FLOAT:
				column_[i].floatCol = new Column<float>(pVector, [=](const VectorSP &pVector, INDEX position, int len, float *buf) {
					return pVector->getFloatConst(position, len, buf);
				});
				break;
			case DT_DOUBLE:
				column_[i].doubleCol = new Column<double>(pVector, [=](const VectorSP &pVector, INDEX position, int len, double *buf) {
					return pVector->getDoubleConst(position, len, buf);
				});
				break;
			case DT_BLOB:
			case DT_STRING:
				column_[i].stringCol = new Column<DolphinString*>(pVector, [=](const VectorSP &pVector, INDEX position, int len, DolphinString** buf) {
					return pVector->getStringConst(position, len, buf);
				});
				break;
			case DT_INT128:
				column_[i].int128Col = new Column<Guid>(pVector, [=](const VectorSP &pVector, INDEX position, int len, Guid *buf) {
					return (const Guid*)pVector->getBinaryConst(position, len, sizeof(Guid), (unsigned char*)buf);
				});
				break;
			case DT_DECIMAL:
				if(pVector->getUnitLength() == sizeof(int32_t)){
					column_[i].decimal32Col = new Column<int32_t>(pVector, [=](const VectorSP &pVector, INDEX position, int len, int32_t *buf) {
						return (const int32_t*)pVector->getBinaryConst(position, len, sizeof(int32_t), (unsigned char*)buf);
					});
				}
				else if(pVector->getUnitLength() == sizeof(int64_t)){
					column_[i].decimal64Col = new Column<int64_t>(pVector, [=](const VectorSP &pVector, INDEX position, int len, int64_t *buf) {
						return (const int64_t*)pVector->getBinaryConst(position, len, sizeof(int64_t), (unsigned char*)buf);
					});
				}
				break;
			default:
				throw RuntimeException("ResultSet doesn't support data type " + Util::getDataTypeString(pVector->getType()));
				break;
			}
		}
	}
	~ResultSet() {
		delete[] column_;
	}
	INDEX position() {
		return position_;
	}
	void position(INDEX position) {
		if (position >= rows_ || position < 0) {
			throw RuntimeException("Position exceed row limit.");
		}
		position_ = position;
	}
	bool next() {
		if (position_ < rows_) {
			position_++;
			return true;
		}
		return false;
	}
	bool first() {
		if (rows_ > 0) {
			position_ = 0;
			return true;
		}
		return false;
	}
	bool isFirst() {
		return position_ == 0;
	}
	bool last() {
		if (rows_ > 0) {
			position_ = rows_ - 1;
			return true;
		}
		return false;
	}
	bool isLast() {
		return position_ + 1 == rows_;
	}
	bool isAfterLast() {
		return position_ >= rows_;
	}
	bool isBeforeFirst() {
		return position_ < 0;
	}
	DATA_TYPE getDataType(int col) {
		return table_->getColumnType(col);
	}
	char getBool(int col) {
		assert(column_[col].charCol != nullptr);
		return column_[col].charCol->getValue(position_);
	}
	char getChar(int col) {
		assert(column_[col].charCol != nullptr);
		return column_[col].charCol->getValue(position_);
	}
	short getShort(int col) {
		assert(column_[col].shortCol != nullptr);
		return column_[col].shortCol->getValue(position_);
	}
	int getInt(int col) {
		assert(column_[col].intCol != nullptr);
		return column_[col].intCol->getValue(position_);
	}
	long long getLong(int col) {
		assert(column_[col].longCol != nullptr);
		return column_[col].longCol->getValue(position_);
	}
	float getFloat(int col) {
		assert(column_[col].floatCol != nullptr);
		return column_[col].floatCol->getValue(position_);
	}
	double getDouble(int col) {
		assert(column_[col].doubleCol != nullptr);
		return column_[col].doubleCol->getValue(position_);
	}
	const DolphinString& getString(int col) const {
		assert(column_[col].stringCol != nullptr);
		return *column_[col].stringCol->getValue(position_);
	}
	const unsigned char* getBinary(int col) const {
		ColumnPointer &column = column_[col];
		if(column.charCol!=nullptr)
			return (unsigned char*)&column.charCol->getValue(position_);
		else if (column.shortCol != nullptr)
			return (unsigned char*)&column.shortCol->getValue(position_);
		else if (column.intCol != nullptr)
			return (unsigned char*)&column.intCol->getValue(position_);
		else if (column.longCol != nullptr)
			return (unsigned char*)&column.longCol->getValue(position_);
		else if (column.floatCol != nullptr)
			return (unsigned char*)&column.floatCol->getValue(position_);
		else if (column.doubleCol != nullptr)
			return (unsigned char*)&column.doubleCol->getValue(position_);
		else if (column.stringCol != nullptr)
			return (unsigned char*)column.stringCol->getValue(position_)->data();
		else if(column.int128Col != nullptr)
			return (unsigned char*)&column.int128Col->getValue(position_);
		else if (column.decimal32Col != nullptr)
			return (unsigned char*)&column.decimal32Col->getValue(position_);
		else if (column.decimal64Col != nullptr)
			return (unsigned char*)&column.decimal64Col->getValue(position_);
		else {
			throw RuntimeException("This instance doesn't support getBinary.");
		}
	}
	ConstantSP getObject(int col) const {
		return column_[col].pVector->get(position_);
	}
private:
	template <class T>
	class Column {
	public:
		Column(const VectorSP &vector,
			std::function<const T*(const VectorSP &pVector, INDEX position,int len, T *buf)> getBufConst)
			: pVector_(vector), getBufConst_(getBufConst),
				constRefBegin_(0), constRefEnd_(0) {
			rows_ = pVector_->rows();
			buffer_=new T[Util::BUF_SIZE];
		}
		~Column() {
			delete[] buffer_;
		}
		const T& getValue(INDEX position) {
			int offset = offsetConst(position);
			return constRef_[offset];
		}
	private:
		VectorSP pVector_;
		std::function<const T*(const VectorSP &pVector, INDEX position, int len, T *buf)> getBufConst_;
		T *buffer_;
		const T *constRef_;
		INDEX constRefBegin_, constRefEnd_;
		INDEX rows_;
		int offsetConst(INDEX position) {
			if (position >= constRefBegin_ && position < constRefEnd_) {
				return position - constRefBegin_;
			}
			if (position < 0 || position >= rows_) {
				throw RuntimeException("Position is out of range.");
			}
			int size = std::min(Util::BUF_SIZE, rows_ - position);
			constRef_ = getBufConst_(pVector_, position, size, buffer_);
			constRefBegin_ = position;
			constRefEnd_ = position + size;
			return 0;
		}
	};
	struct ColumnPointer {
		VectorSP pVector;
		Column<char> *charCol = nullptr;
		Column<short> *shortCol = nullptr;
		Column<int> *intCol = nullptr;
		Column<long long> *longCol = nullptr;
		Column<float> *floatCol = nullptr;
		Column<double> *doubleCol = nullptr;
		Column<DolphinString*> *stringCol = nullptr;
		Column<Guid> *int128Col = nullptr;
		Column<int32_t> *decimal32Col = nullptr;
		Column<int64_t> *decimal64Col = nullptr;
		~ColumnPointer() {
			delete charCol;
			delete shortCol;
			delete intCol;
			delete longCol;
			delete floatCol;
			delete doubleCol;
			delete stringCol;
			delete int128Col;
			delete decimal32Col;
			delete decimal64Col;
		}
	};
	TableSP table_;
	long position_;
	int rows_, cols_;
	ColumnPointer *column_;
};

}//dolphindb namespace

#endif //DDBPLUGIN_H_