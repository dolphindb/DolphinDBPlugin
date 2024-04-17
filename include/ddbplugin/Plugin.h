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
#include "Types.h"
#include "Util.h"
#include "Exceptions.h"
#include "Logger.h"
#include <exception>
#include <functional>
#include <cassert>
#include <iostream>
#include <map>
#include <string>
#include <utility>

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

template <typename Key, typename Value>
class BiMap {
  public:
    bool insert(const Key& key, const Value& value, string& errMsg) {
        if(dataMap_.find(key)!=dataMap_.end()) {
            errMsg = "Duplicated key";
            return false;
        }
        if(reverseDataMap_.find(value)!=reverseDataMap_.end()) {
            errMsg = "Duplicated value";
            return false;
        }
        dataMap_[key] = value;
        reverseDataMap_[value] = key;
        return true;
    }

    bool removeKey(const Key& key) {
        if(dataMap_.find(key)==dataMap_.end()) {
            return false;
        }
        Value value = dataMap_[key];
        dataMap_.erase(key);
        reverseDataMap_.erase(value);
        return true;
    }

    bool removeValue(const Value& value) {
        if(reverseDataMap_.find(value)==reverseDataMap_.end()) {
            return false;
        }
        Key key = reverseDataMap_[value];
        dataMap_.erase(key);
        reverseDataMap_.erase(value);
        return true;
    }

    bool findKey(const Key& key, Value& value) const {
        if(dataMap_.find(key)==dataMap_.end()) {
            return false;
        }
        value = dataMap_.at(key);
        return true;
    }

    bool findValue(const Value& value, Key& key) const {
        if(reverseDataMap_.find(value)==reverseDataMap_.end()) {
            return false;
        }
        key = reverseDataMap_.at(value);
        return true;
    }

    vector<Key> getKeys() const {
        vector<Key> ret;
        ret.reserve(dataMap_.size());
        for(auto it = dataMap_.begin(); it != dataMap_.end(); ++it) {
            ret.push_back(it->first);
        }
        return ret;
    }

    vector<Value> getValues() const {
        vector<Value> ret;
        ret.reserve(dataMap_.size());
        for(auto it = dataMap_.begin(); it != dataMap_.end(); ++it) {
            ret.push_back(it->second);
        }
        return ret;
    }

    void clear() {
        dataMap_.clear();
        reverseDataMap_.clear();
    }
  private:
    std::map<Key, Value> dataMap_;
    std::map<Value, Key> reverseDataMap_;
};

/**
 * @brief Background resource map class
 * This class is used to manage handles that need to run in the background,
 * ensuring they shouldn't be destructed when the session ends.
 *
 * ATTENTION!!!
 * If you intend to utilize this class, all functionalities involving onClose(),
 * resource creation, retrieval and closure should exclusively use functions
 * within the class. Avoid direct manipulation of the ResourceSP handle pointer.
 *
 * @tparam T The data types that can be added to the map.
 */
template <typename T>
class BackgroundResourceMap {
  public:
    /**
     * @brief Construct a new background Resource Map object.
     *
     * @param prefix    The prefix used int exception.
     * @param keyWord   The keyWord used to verify if it's an expected handle.
     * user should make sure handle desc contains the keyWord. If need to ignore
     * the verification, just pass in en empty string.
     */
    explicit BackgroundResourceMap(const string &prefix, const string &keyWord = "")
        : prefix_(prefix), keyword_(keyWord) {}
    ~BackgroundResourceMap(){
        clear();
    }
    BackgroundResourceMap(const BackgroundResourceMap &) = delete;
    BackgroundResourceMap(BackgroundResourceMap &&) = delete;
    BackgroundResourceMap &operator=(const BackgroundResourceMap &) = delete;
    BackgroundResourceMap &operator=(BackgroundResourceMap &&) = delete;

    /**
     * @brief Add resource handle to the map
     *
     * @param handle    The handle need to be added in the map
     * @param handlePtr Data pointer need to be added in the map.
     * HandlePtr must be the same as the pointer stored in the handle.
     */
    void safeAdd(ConstantSP &handle, SmartPointer<T> handlePtr) {
        LockGuard<Mutex> guard(&resourceLock_);
        if(inDestruction_) {
            return;
        }
        handleValidCheck(handle);
        long long ptrValue = handle->getLong();
        if (ptrValue == 0) {
            throw RuntimeException(prefix_ + "Handle has already been destructed.");
        }
        if (handlePtr.get() == nullptr) {
            throw RuntimeException(prefix_ + "HandlePtr should not be nullptr.");
        }
        if (ptrValue != (long long)(handlePtr.get())) {
            throw RuntimeException(prefix_ + "Handle and handlePtr does not match.");
        }
        string name;
        if (resourceMap_.find(ptrValue) != resourceMap_.end() ||
            nameMap_.findValue(ptrValue, name)){
            throw RuntimeException(prefix_ + "Handle already exists in the map.");
        }
        resourceMap_[ptrValue] = std::make_pair(handle, handlePtr);
    }

    /**
     * @brief Safely add a handle into map with a name
     *
     * @param handle The handle need to be added in the map
     * @param handlePtr Data pointer need to be added in the map.
     * HandlePtr must be the same as the pointer stored in the handle.
     * @param name   The name assigned to this handle.
     */
    void safeAdd(ConstantSP handle, SmartPointer<T> handlePtr, const string &name) {
        LockGuard<Mutex> guard(&resourceLock_);
        if(inDestruction_) {
            return;
        }
        handleValidCheck(handle);
        long long ptrValue = handle->getLong();
        if (ptrValue == 0) {
            throw RuntimeException(prefix_ + "Handle has already been destructed.");
        }
        if (handlePtr.get() == nullptr) {
            throw RuntimeException(prefix_ + "HandlePtr should not be nullptr.");
        }
        if (ptrValue != (long long)(handlePtr.get())) {
            throw RuntimeException(prefix_ + "Handle and handlePtr does not match.");
        }
        string nameResult;
        long long ptrResult;
        if (resourceMap_.find(ptrValue) != resourceMap_.end() ||
            nameMap_.findValue(ptrValue, nameResult)) {
            throw RuntimeException(prefix_ + "Handle already exists in the map.");
        }
        if (nameMap_.findKey(name, ptrResult)) {
            throw RuntimeException(prefix_ + "name [" + name + "] already exists.");
        }
        resourceMap_[ptrValue] = std::make_pair(handle, handlePtr);
        string errMsg;
        if(!nameMap_.insert(name, ptrValue, errMsg)) {
            throw RuntimeException(prefix_ + errMsg);
        }
    }

    /**
     * @brief Safely retrieve the data pointer stored in the passed-in handle.
     * This function performs type and availability validation, ensuring that
     * the data pointer is not destructed.
     *
     * @param handle  A handle that requires extracting the internal data pointer.
     * @return SmartPointer<T> Retrieved data pointer of the handle.
     */
    SmartPointer<T> safeGet(ConstantSP handle) {
        LockGuard<Mutex> guard(&resourceLock_);
        if(inDestruction_) {
            throw RuntimeException(prefix_ + "BackgroundResourceMap is in destruction.");
        }
        handleValidCheck(handle);
        long long ptrValue = handle->getLong();
        if (resourceMap_.find(ptrValue) == resourceMap_.end()) {
            throw RuntimeException(prefix_ + "Handle is unregistered.");
        }
        return resourceMap_[ptrValue].second;
    }

    /**
     * @brief Safely retrieve the data pointer stored in the handle corresponding to the name.
     *
     * @param name A name corresponding to a handle from which the data pointer
     * needs to be extracted.
     * @return SmartPointer<T> Retrieved data pointer of the handle.
     */
    SmartPointer<T> safeGetByName(const string &name) {
        return safeGet(getHandleByName(name));
    }

    /**
     * @brief Get the Handle By Name.
     *
     * @param name A name corresponding to the wanted handle.
     * @return ConstantSP The handle corresponding to the name.
     */
    ConstantSP getHandleByName(const string &name) {
        LockGuard<Mutex> guard(&resourceLock_);
        if(inDestruction_) {
            throw RuntimeException(prefix_ + "BackgroundResourceMap is in destruction.");
        }
        long long ptrValue;
        if (!nameMap_.findKey(name, ptrValue)) {
            throw RuntimeException(prefix_ + "Unknown handle name [" + name + "].");
        }
        if (resourceMap_.find(ptrValue) == resourceMap_.end()) {
            throw RuntimeException(prefix_ + "Unable to find handle corresponding to name [" + name + "].");
        }
        return resourceMap_[ptrValue].first;
    }

    /**
     * @brief Safely remove a handle in the map after destructing it
     *
     * @param handle The handle to be removed.
     */
    void safeRemove(ConstantSP handle) {
        LockGuard<Mutex> guard(&resourceLock_);
        if(inDestruction_) {
            return;
        }
        handleValidCheck(handle);
        long long ptrValue = handle->getLong();
        if (resourceMap_.find(ptrValue) == resourceMap_.end()) {
            throw RuntimeException(prefix_ + "Handle is unregistered.");
        }
        handle->setLong(0);
        nameMap_.removeValue(ptrValue);
        resourceMap_.erase(ptrValue);
    }

    /**
     * @brief Get the Handle Names list
     *
     * @return vector<string> a vector of all names stored in map
     */
    vector<string> getHandleNames() {
        LockGuard<Mutex> guard(&resourceLock_);
        if(inDestruction_) {
            throw RuntimeException(prefix_ + "BackgroundResourceMap is in destruction.");
        }
        return nameMap_.getKeys();
    }

    /**
     * @brief Safely remove a handle in the map after destructing it
     * This function would not throw exception, suitable for use in onClose().
     *
     * @param handle The handle to be removed.
     * @return true If removal successfully.
     * @return false If removal failed.
     */
    bool safeRemoveWithoutException(ConstantSP &handle) noexcept {
        try {
            safeRemove(handle);
            return true;
        } catch (RuntimeException &ex) {
            LOG_ERR(ex.what());
        } catch (std::exception &ex) {
            LOG_ERR(prefix_ + ex.what());
        } catch (...) {
            LOG_ERR(prefix_ + "Remove handle failed");
        }
        return false;
    }

    void clear() {
        LockGuard<Mutex> guard(&resourceLock_);
        if(inDestruction_) {
            return;
        }
        inDestruction_ = true;
        nameMap_.clear();
        resourceMap_.clear();
        inDestruction_ = false;
    }

    /**
     * @brief Get the count of handles in the map.
     *
     * @return int Count of handles in the map
     */
    int size() {
        LockGuard<Mutex> guard(&resourceLock_);
        return resourceMap_.size();
    }

  private:
    void handleValidCheck(ConstantSP handle) const {
        if(handle->getType() == DT_LONG) {
            long long ptrValue = handle->getLong();
            if (ptrValue == 0) {
                throw RuntimeException(prefix_ + "Resource object already be destructed.");
            }
            return;
        }
        if (handle->getType() != DT_RESOURCE || handle->getForm() != DF_SCALAR) {
            throw RuntimeException(prefix_ + "Invalid object, expect RESOURCE here.");
        }
        long long ptrValue = handle->getLong();
        string desc = handle->getString();
        if (ptrValue == 0) {
            throw RuntimeException(prefix_ + "Resource object already be destructed.");
        }
        if (keyword_ != "" && desc.find(keyword_) == desc.npos) {
            throw RuntimeException(prefix_ + "Type verification failed due to mismatched resource desc, expect \"" +
                keyword_ + "\" in resource desc.");
        }
    }

    bool inDestruction_ = false;
    string prefix_;   // prefix of exception sentence
    string keyword_;  // keyword need to be found in resource desc
    Mutex resourceLock_;
    BiMap<string, long long> nameMap_;
    unordered_map<long long, std::pair<ConstantSP, SmartPointer<T>>> resourceMap_;
};

/**
 * @brief Resource map class
 * This class is used to manage handles that need to be destructed when the session ends.
 *
 * ATTENTION!!!
 * If you intend to utilize this class, all functionalities involving onClose(),
 * resource creation, retrieval and closure should exclusively use functions
 * within the class. Avoid direct manipulation of the ResourceSP handle pointer.
 *
 * @tparam T The data types that can be added to the map.
 */
template <typename T>
class ResourceMap {
  public:
    /**
     * @brief Construct a new background Resource Map object.
     *
     * @param prefix    The prefix used int exception.
     * @param keyWord   The keyWord used to verify if it's an expected handle.
     * user should make sure handle desc contains the keyWord. If need to ignore
     * the verification, just pass in en empty string.
     */
    explicit ResourceMap(const string &prefix, const string &keyWord = "") : prefix_(prefix), keyword_(keyWord) {}
    ~ResourceMap(){
        clear();
    }
    ResourceMap(const ResourceMap &) = delete;
    ResourceMap(ResourceMap &&) = delete;
    ResourceMap &operator=(const ResourceMap &) = delete;
    ResourceMap &operator=(ResourceMap &&) = delete;

    /**
     * @brief Add resource handle to the map
     *
     * @param handle    The handle need to be added in the map
     * @param handlePtr Data pointer need to be added in the map.
     * HandlePtr must be the same as the pointer stored in the handle.
     */
    void safeAdd(ConstantSP &handle, SmartPointer<T> handlePtr) {
        LockGuard<Mutex> guard(&resourceLock_);
        if(inDestruction_) {
            return;
        }
        handleValidCheck(handle);
        long long ptrValue = handle->getLong();
        if (ptrValue == 0) {
            throw RuntimeException(prefix_ + "Handle has already been destructed.");
        }
        if (handlePtr.get() == nullptr) {
            throw RuntimeException(prefix_ + "HandlePtr should not be nullptr.");
        }
        if (ptrValue != (long long)(handlePtr.get())) {
            throw RuntimeException(prefix_ + "Handle and handlePtr does not match.");
        }
        if (resourceMap_.find(ptrValue) != resourceMap_.end()) {
            throw RuntimeException(prefix_ + "Handle already exists in the map.");
        }
        resourceMap_[ptrValue] = handlePtr;
    }

    /**
     * @brief Safely retrieve the data pointer stored in the passed-in handle.
     *
     * @param handle  A handle that requires extracting the internal data pointer.
     * @return SmartPointer<T> Retrieved data pointer of the handle.
     */
    SmartPointer<T> safeGet(ConstantSP &handle) {
        LockGuard<Mutex> guard(&resourceLock_);
        if(inDestruction_) {
            throw RuntimeException(prefix_ + "ResourceMap is in destruction.");
        }
        handleValidCheck(handle);
        long long cp = handle->getLong();
        if (resourceMap_.find(cp) == resourceMap_.end()) {
            throw RuntimeException(prefix_ + "Handle is unregistered.");
        }
        auto ret = resourceMap_[cp];
        if (ret.isNull()) {
            throw RuntimeException(prefix_ + "Data pointer of handle has already been destructed.");
        }
        return ret;
    }

    /**
     * @brief Safely remove a handle in the map.
     * After performing the safeRemove() operation to remove a handle, the inner
     * data pointer will not be immediately destructed due to the returned SmartPointer.
     * Instead, it will be destructed after the SmartPointer loses all its references.
     * Exceptions need to be manually caught when this function is called externally.
     *
     * @param handle The handle to be removed.
     */
    void safeRemove(ConstantSP &handle) {
        LockGuard<Mutex> guard(&resourceLock_);
        if(inDestruction_) {
            return;
        }
        handleValidCheck(handle);
        long long cp = handle->getLong();
        if (resourceMap_.find(cp) == resourceMap_.end()) {
            throw RuntimeException(prefix_ + "Handle is unregistered.");
        }
        resourceMap_.erase(cp);
        handle->setLong(0);
    }

    /**
     * @brief Safely remove a handle in the map.
     * After performing the safeRemove() operation to remove a handle, the inner
     * data pointer will not be immediately destructed due to the returned SmartPointer.
     * Instead, it will be destructed after the SmartPointer loses all its references.
     * Exceptions need to be manually caught when this function is called externally.
     *
     * @param handle The handle to be removed.
     * @return true If removal successfully.
     * @return false If removal failed.
     */
    bool safeRemoveWithoutException(ConstantSP &handle) noexcept {
        try {
            safeRemove(handle);
            return true;
        } catch (RuntimeException &ex) {
            LOG_ERR(ex.what());
        } catch (std::exception &ex) {
            LOG_ERR(prefix_ + ex.what());
        } catch (...) {
            LOG_ERR(prefix_ + "remove handle failed");
        }
        return false;
    }

    void clear() {
        LockGuard<Mutex> guard(&resourceLock_);
        if(inDestruction_) {
            return;
        }
        inDestruction_ = true;
        resourceMap_.clear();
    }

  private:
    void handleValidCheck(ConstantSP handle) const {
        if(handle->getType() == DT_LONG) {
            long long ptrValue = handle->getLong();
            if (ptrValue == 0) {
                throw RuntimeException(prefix_ + "Resource object already be destructed.");
            }
            return;
        }
        if (handle->getType() != DT_RESOURCE || handle->getForm() != DF_SCALAR) {
            throw RuntimeException(prefix_ + "Invalid object, expect RESOURCE here.");
        }
        long long ptrValue = handle->getLong();
        string desc = handle->getString();
        if (ptrValue == 0) {
            throw RuntimeException(prefix_ + "Resource object already be destructed.");
        }
        if (keyword_ != "" && desc.find(keyword_) == desc.npos) {
            throw RuntimeException(prefix_ + "Type verification failed due to mismatched resource desc, expect \"" +
                keyword_ + "\" in resource desc.");
        }
    }

    bool inDestruction_ = false;
    string prefix_;   // prefix of exception sentence
    string keyword_;  // keyword need to be found in resource desc
    Mutex resourceLock_;

    // use SmartPointer to control the lifecycle of T*
    unordered_map<long long, SmartPointer<T>> resourceMap_;
};

class PluginDefer {
  public:
    PluginDefer(std::function<void()> code): code_(code) {}
    ~PluginDefer() {code_(); }
  private:
    std::function<void()> code_;
};

}  // dolphindb namespace

#endif  // DDBPLUGIN_H_