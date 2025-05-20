#pragma once

#include "DolphinDBEverything.h"
#include "CoreConcept.h"
#include "Decimal128.h"
#include "DolphinString.h"
#include "ScalarImp.h"
#include "SpecialConstant.h"

#ifdef DOLPHINDB_JIT
#include "TurboJetInterface.h"
#endif

#ifndef THROW_INVALID_INPUT
#define THROW_INVALID_INPUT(s) throw IllegalArgumentException(__FUNCTION__, std::string("[PLUGIN::BACKTEST] ") + (s))
#endif

namespace ddb {

#define DT_INT_ARRAY DATA_TYPE(DT_INT + ARRAY_TYPE_BASE)
#define DT_LONG_ARRAY DATA_TYPE(DT_LONG + ARRAY_TYPE_BASE)
#define DT_DOUBLE_ARRAY DATA_TYPE(DT_DOUBLE + ARRAY_TYPE_BASE)

struct Field {
    std::string name;
    DATA_TYPE type;
    DATA_FORM form;
};
vector<Field> convertSchemaField(const vector<string> &names, const vector<DATA_TYPE> &types);

// Constant

template <typename T>
inline T getValue(const ConstantSP &c) {
    std::cerr << "unknown type in " << __PRETTY_FUNCTION__ << std::endl;
    return T();
}

template <>
inline bool getValue(const ConstantSP &c) {
    return c->getBool();
}
template <>
inline int getValue(const ConstantSP &c) {
    return c->getInt();
}
#ifndef _WIN32
template <>
inline int64_t getValue(const ConstantSP &c) {
    return c->getLong();
}
#endif
template <>
inline long long getValue(const ConstantSP &c) {
    return c->getLong();
}
template <>
inline double getValue(const ConstantSP &c) {
    return c->getDouble();
}
template <>
inline std::string getValue(const ConstantSP &c) {
    return c->getString();
}
template <>
inline decimal128 getValue(const ConstantSP &c) {
    return decimal128(c->getDecimal128(decimalScale));
}
template <>
inline std::vector<int64_t> getValue(const ConstantSP &c) {
    std::vector<int64_t> values;
    if (c.isNull() || !c->isVector()) {
        return values;
    }
    for (int i = 0; i < c->size(); ++i) {
        values.push_back(c->getLong(i));
    }
    return values;
}

inline void setValue(ConstantSP &c, int v) { return c->setInt(v); }
#ifndef _WIN32
inline void setValue(ConstantSP &c, int64_t v) { return c->setLong(v); }
#endif
inline void setValue(ConstantSP &c, long long v) { return c->setLong(v); }
inline void setValue(ConstantSP &c, double v) { return c->setDouble(v); }
inline void setValue(ConstantSP &c, const std::string &v) { return c->setString(v); }
inline void setValue(ConstantSP &c, decimal128 v) { return c->setDecimal128(0, decimalScale, static_cast<int128>(v)); }

inline ConstantSP getConstantSP() { return new Void(); }
inline ConstantSP getConstantSP(double x) { return new Double(x); }
#ifndef _WIN32
inline ConstantSP getConstantSP(int64_t x) { return new Long(x); }
#endif
inline ConstantSP getConstantSP(long long x) { return new Long(x); }
inline ConstantSP getConstantSP(int x) { return new Int(x); }
inline ConstantSP getConstantSP(const std::string &x) { return new String(x); }
inline ConstantSP getConstantSP(decimal128 x) { return new Decimal128(decimalScale, decimal128::valueT(x)); }

template <typename T>
inline bool isNull(T v) {
    return v == std::numeric_limits<T>::lowest();
}

template <typename T>
inline T getNull() {
    return std::numeric_limits<T>::lowest();
}

struct ArgField {
    std::string name;
    DATA_TYPE type;
    DATA_FORM form;
    DATA_CATEGORY category;
};
string getCategoryString(DATA_CATEGORY category);
class IArgStream {
  public:
    IArgStream(std::vector<ConstantSP> &args) : args_(args), argIt_(args_.begin()) {}

    IArgStream(std::vector<ConstantSP> &args, const std::vector<ArgField> &schema, const std::pair<int, int> &argNum)
        : args_(args),
          argIt_(args_.begin()),
          schema_(schema),
          schemaIt_(schema.cbegin()),
          checkInvalid_(true),
          argNum_(argNum) {}

    ConstantSP &getConstantSP() { return *argIt_++; }

    void ignore(int num) { std::advance(argIt_, num); }

    bool nextIsDouble() { return (*argIt_)->getType() == DT_DOUBLE; }

    operator bool() { return argIt_ != args_.end(); }

    bool hasError() const { return readEOF_; }

    template <typename valueT>
    inline IArgStream &operator>>(valueT &arg) {
        if (checkInvalid_) {
            ++argIdx_;
            // max param num, avoid reading account param in single engine backtest
            if (argIdx_ > argNum_.second) {
                return *this;
            }

            // hanle optional arguments
            if (argIdx_ > argNum_.first) {
                // optional argument is void or not exist
                if ((argIt_ != args_.end() && (*argIt_)->getType() == DT_VOID) || (argIt_ == args_.end())) {
                    if (argIt_ != args_.end()) ++argIt_;
                    if (schemaIt_ != schema_.cend()) ++schemaIt_;
                    return *this;
                }
            }

            if (schemaIt_ != schema_.cend() && *this) {
                // check data form
                if ((*argIt_)->getForm() != (*schemaIt_).form) {
                    throw RuntimeException("[PLUGIN::BACKTEST] Invalid data form of argument " + (*schemaIt_).name +
                                           ", shouble be " + Util::getDataFormString((*schemaIt_).form) + ".");
                }

                // check data category or type
                DATA_TYPE dt = (*argIt_)->getType();
                if ((*schemaIt_).category != NOTHING) {
                    if ((*schemaIt_).form == DF_VECTOR && dt == DT_ANY) {
                        int sz = (*argIt_)->size();
                        ConstantSP tmp;
                        for (int i = 0; i < sz; ++i) {
                            tmp = (*argIt_)->get(i);
                            if (tmp->getCategory() != (*schemaIt_).category || !tmp->isScalar()) {
                                throw RuntimeException("[PLUGIN::BACKTEST] Invalid data category of arg " +
                                                       (*schemaIt_).name + ", shouble be " +
                                                       getCategoryString((*schemaIt_).category) + " type.");
                            }
                        }
                        // TODO: JIT parse bug, delete after fix https://dolphindb1.atlassian.net/browse/BACKTESTME-137
                    } else if (Util::getCategory(dt) != (*schemaIt_).category) {
                        throw RuntimeException("[PLUGIN::BACKTEST] Invalid data category of arg " + (*schemaIt_).name +
                                               ", shouble be " + getCategoryString((*schemaIt_).category) + " type.");
                    }
                } else if ((dt != (*schemaIt_).type)) {
                    if (dt != DT_LONG || (*schemaIt_).type != DT_RESOURCE) {  // resource handle can be replaced by long
                        throw RuntimeException("[PLUGIN::BACKTEST] Invalid data type of argument " + (*schemaIt_).name +
                                               ", shouble be " + Util::getDataTypeString((*schemaIt_).type) + ".");
                    }
                }

                schemaIt_++;
            } else {
                throw RuntimeException("[PLUGIN::BACKTEST] Failed to read argument " + (*schemaIt_).name + ".");
            }
        }

        if (*this) {
            arg = getValue<valueT>(getConstantSP());
        } else {
            readEOF_ = true;
        }
        return *this;
    }

  private:
    std::vector<ConstantSP> args_;
    std::vector<ConstantSP>::iterator argIt_;
    bool readEOF_{false};

    std::vector<ArgField> schema_;
    std::vector<ArgField>::const_iterator schemaIt_;
    bool checkInvalid_{false};
    std::pair<int, int> argNum_{0, 0};  // <min, max>
    int argIdx_{0};
};

inline IArgStream &operator>>(IArgStream &s, ConstantSP &arg) {
    arg = s.getConstantSP();
    return s;
}

inline IArgStream &operator>>(IArgStream &s, TableSP &arg) {
    arg = s.getConstantSP();
    return s;
}

inline IArgStream &operator>>(IArgStream &s, FunctionDefSP &arg) {
    arg = s.getConstantSP();
    if (arg->getType() != DT_FUNCTIONDEF || arg->getForm() != DF_SCALAR) {
        arg = nullptr;
    }
    return s;
}

inline IArgStream &operator>>(IArgStream &s, std::map<std::string, std::string> &arg) {
    DictionarySP dict = s.getConstantSP();
    VectorSP keys = dict->keys();
    for (int i = 0; i < keys->size(); ++i) {
        auto key = keys->getString(i);
        auto value = dict->getMember(key)->getString();
        arg[key] = value;
    }
    return s;
}

#ifdef DOLPHINDB_JIT

using JITDolphinInstanceSP = SmartPointer<TurboJet::JITDolphinInstance>;

inline void setJitAttribute(JITDolphinInstanceSP &instance, int index, TurboJet::TurboJetValue &v, double data) {
    v.value_.d = data;
    instance->setAttribute(index, v);
}

inline void setJitAttribute(JITDolphinInstanceSP &instance, int index, TurboJet::TurboJetValue &v, long long data) {
    v.value_.l = data;
    instance->setAttribute(index, v);
}

inline void setJitAttribute(JITDolphinInstanceSP &instance, int index, TurboJet::TurboJetValue &v, int data) {
    v.value_.i = data;
    instance->setAttribute(index, v);
}

inline void setJitAttribute(JITDolphinInstanceSP &instance, int index, TurboJet::TurboJetValue v, DolphinString *data) {
    ConstantSP sp = new String(*data);
    TurboJet::TurboJetValue::fromConstantSP(sp, &v);
    instance->setAttribute(index, v);
}

inline void setJitAttribute(JITDolphinInstanceSP &instance, int index, TurboJet::TurboJetValue v, int128 data) {
    ConstantSP sp = new Decimal128(decimalScale, data);
    TurboJet::TurboJetValue::fromConstantSP(sp, &v);
    instance->setAttribute(index, v);
}

inline DolphinClassSP createJitClass(const std::vector<Field> &fields, const std::string &name) {
    DolphinClassSP jitClass_ = new DolphinClass("Backtest", name);
    for (auto &field : fields) {
        ConstantSP dummyValue;
        if (field.form == DF_SCALAR) {
            if (field.type == DT_STRING || field.type == DT_SYMBOL) {
                dummyValue = new String("string");
            } else if (field.type == DT_DECIMAL128) {
                dummyValue = new Decimal128(decimalScale, 0);
            } else {
                dummyValue = Util::createConstant(field.type);
            }
            dummyValue = Util::createNullConstant(field.type);
        } else if (field.form == DF_VECTOR) {
            dummyValue = Util::createVector(field.type, 0);
        } else {
            std::cerr << "Unsupported data form in " << __PRETTY_FUNCTION__ << std::endl;
        }
        jitClass_->addAttributeWithType(field.name, OO_ACCESS::PUBLIC, dummyValue);
    }
    jitClass_->setJit(true);
    jitClass_->setJitClassId(DolphinClass::genClsId());
    return jitClass_;
}

#else

using JITDolphinInstanceSP = ConstantSP;

inline DolphinClassSP createJitClass(const std::vector<Field> &fields, const std::string &name)
{
    std::ignore = fields;
    std::ignore = name;
    return nullptr;
}

#endif

// Vector

inline void setVectorData(Vector *v, INDEX start, std::size_t len, const double *p) { v->setDouble(start, len, p); }

inline void setVectorData(Vector *v, INDEX start, std::size_t len, const long long *p) { v->setLong(start, len, p); }

inline void setVectorData(Vector *v, INDEX start, std::size_t len, const int *p) { v->setInt(start, len, p); }

inline void setVectorData(Vector *v, INDEX start, std::size_t len, const DolphinString **p) {
    v->setString(start, len, p);
}

inline void setVectorData(Vector *v, INDEX start, std::size_t len, const int128 *p) {
    v->setDecimal128(start, len, decimalScale, p);
}

class DDBIterator {
  public:
    DDBIterator(const VectorSP &vec) : vec_(vec) {}
    virtual ~DDBIterator() {}

    operator bool() { return ptr_ != nullptr; }

    template <typename T>
    void getValue(T &v) {
        auto val = reinterpret_cast<const T *>(ptr_);
        v = *val++;
        ptr_ = val;
        incIndex();
    }

    void incIndex() {
        ++index_;
        if (index_ == end_ && index_ < size_) {
            this->setIndex(index_);
        }
    }

    virtual void setIndex(size_t index)
    {
        std::ignore = index;
    };

    std::size_t getIndex() { return index_; }

  protected:
    inline const double *getDataPtr(Vector *v, INDEX start, std::size_t len, double *p) {
        return v->getDoubleConst(start, len, p);
    }

    inline const long long *getDataPtr(Vector *v, INDEX start, std::size_t len, long long *p) {
        return v->getLongConst(start, len, p);
    }

    inline const int *getDataPtr(Vector *v, INDEX start, std::size_t len, int *p) {
        return v->getIntConst(start, len, p);
    }

    inline DolphinString **getDataPtr(Vector *v, INDEX start, std::size_t len, DolphinString **p) {
        return v->getStringConst(start, len, p);
    }

    inline const int128 *getDataPtr(Vector *v, INDEX start, std::size_t len, int128 *p) {
        return v->getDecimal128Const(start, len, v->getExtraParamForType(), p);
    }

  protected:
    VectorSP vec_;
    const void *ptr_{nullptr};
    std::size_t bufferSize{0};
    std::size_t index_{0};  // current index
    std::size_t start_{0};  // start index of cache
    std::size_t end_{0};    // end index of cache
    std::size_t size_{0};   // total size
    VectorSP idx_;
    // https://learn.microsoft.com/en-us/style-guide/a-z-word-list-term-collections/i/index-indexes-indices
    std::vector<INDEX> indexes_;
    const INDEX *indexPtr_{nullptr};
    std::size_t valueIndex_;
    std::size_t valueSize_;
    std::size_t valueEnd_;
};

template <>
inline void DDBIterator::getValue(decimal128 &v) {
    auto val = reinterpret_cast<const int128 *>(ptr_);
    v = decimal128(*val++);
    ptr_ = val;
    incIndex();
}

template <>
inline void DDBIterator::getValue(std::string &v) {
    auto val = reinterpret_cast<DolphinString *const *>(ptr_);
    v = (*val++)->getString();
    ptr_ = val;
    incIndex();
}

template <>
inline void DDBIterator::getValue(std::vector<int128> &v) {
    auto val = reinterpret_cast<const int128 *>(ptr_);
    v.resize(valueSize_);
    for (size_t i = 0; i < valueSize_; ++i) {
        v[i] = *val++;
    }
    ptr_ = val;
    this->setIndex(index_ + 1);
}

template <typename T>
class VectorIterator : public DDBIterator {
  public:
    VectorIterator(const VectorSP &vec) : DDBIterator(vec) {
        bufferSize = 4096 / sizeof(T);
        size_ = vec->size();
        data_.resize(bufferSize);
        setIndex(0);
    }

    void setIndex(std::size_t index) override {
        if (index < start_ || index >= end_) {
            if (index >= size_) {
                ptr_ = nullptr;
                return;
            }
            data_.resize(bufferSize);
            size_t len = std::min(size_ - index, bufferSize);
            start_ = index_;
            end_ = index_ + len;
            typedPtr_ = getDataPtr(vec_.get(), index, len, data_.data());
        } else {
            typedPtr_ += index - index_;
        }
        ptr_ = typedPtr_;
        index_ = index;
        return;
    }

    T operator*() const { return *typedPtr_; }

  private:
    std::vector<T> data_;
    const T *typedPtr_{nullptr};
};

template <typename T>
class VectorIterator<std::vector<T>> : public DDBIterator {
  public:
    VectorIterator(const VectorSP &vec) : DDBIterator(vec) {
        vec_ = dynamic_cast<FastArrayVector *>(vec.get())->getSourceValue();
        idx_ = dynamic_cast<FastArrayVector *>(vec.get())->getSourceIndex();
        size_ = vec->size();
        std::size_t avgLen = vec_->size() / size_;
        bufferSize = 4096 / sizeof(T) / avgLen;
        data_.resize(avgLen * bufferSize);
        indexes_.resize(bufferSize);
        // init index 0
        setIndex();
    }

    void setIndex() {
        index_ = 0;
        valueIndex_ = 0;
        size_t len = std::min(size_, bufferSize);
        start_ = index_;
        end_ = len;
        indexPtr_ = getDataPtr(idx_.get(), 0, len, indexes_.data());
        valueSize_ = getArraySize();
        if (data_.size() < valueSize_) {
            data_.resize(valueSize_);
        }
        typedPtr_ = getDataPtr(vec_.get(), 0, valueSize_, data_.data());
        ptr_ = typedPtr_;
    }

    void setIndex(std::size_t index) {
        if (index == 0) {
            setIndex();
            return;
        }
        if (index < start_ || index >= end_) {
            if (size_ <= index) {
                return;
            }
            index_ = index;
            size_t len = std::min(size_ - index, bufferSize);
            start_ = index_;
            end_ = index_ + len - 1;
            indexPtr_ = getDataPtr(idx_.get(), index_ - 1, len, indexes_.data());
            size_t dataSize = indexPtr_[len - 1] - indexPtr_[0];
            if (data_.size() < dataSize) {
                data_.resize(dataSize);
            }
            typedPtr_ = getDataPtr(vec_.get(), *indexPtr_, dataSize, data_.data());
        } else {
            int currentTypedIndex = *indexPtr_;
            indexPtr_ += ((int)index - (int)index_);
            typedPtr_ += (*indexPtr_ - currentTypedIndex);
            index_ = index;
        }
        valueSize_ = getArraySize();
        ptr_ = typedPtr_;
    }

    const T *operator*() const { return typedPtr_; }

    std::size_t getArraySize() const {
        if (index_ == 0) {
            return *indexPtr_;
        }
        return *indexPtr_ - *(indexPtr_ - 1);
    }

  private:
    std::vector<T> data_;
    const T *typedPtr_{nullptr};
};

// Table

class Schema {
    using instanceIt = std::vector<JITDolphinInstanceSP>::iterator;

  public:
    void setFields(const std::vector<Field> &fields, const std::string &name = "") {
        name_ = name;
        fields_ = fields;
        jitClass_ = createJitClass(fields, name);
    }

    auto getNames() -> std::vector<std::string> {
        std::vector<std::string> names;
        names.reserve(fields_.size());
        for (auto &f : fields_) {
            names.push_back(f.name);
        }
        return names;
    }

    auto getOutputColumns() const -> std::vector<VectorSP> {
        std::vector<VectorSP> ret;
        ret.reserve(fields_.size());
        for (auto &f : fields_) {
            if (f.form == DF_VECTOR) {
                if (f.type == DT_DECIMAL128) {
                    ret.push_back(Util::createArrayVector(static_cast<DATA_TYPE>(f.type + ARRAY_TYPE_BASE), 0,
                                                                  0, 0, 0, true, decimalScale));
                } else {
                    ret.push_back(Util::createArrayVector(static_cast<DATA_TYPE>(f.type + ARRAY_TYPE_BASE), 0));
                }
            } else {
                if (f.type == DT_DECIMAL128) {
                    ret.push_back(Util::createVector(f.type, 0, 0, true, decimalScale));
                } else {
                    ret.push_back(Util::createVector(f.type, 0));
                }
            }
        }
        return ret;
    }

    auto getInputColumn(const VectorSP &vec, DATA_TYPE type, DATA_FORM form) -> std::shared_ptr<DDBIterator> {
        std::shared_ptr<DDBIterator> ddbIt;
        if (form == DF_SCALAR) {
            if (type == DT_DOUBLE) {
                ddbIt = std::dynamic_pointer_cast<DDBIterator>(std::make_shared<VectorIterator<double>>(vec));
            } else if (type == DT_DECIMAL128) {
                ddbIt = std::dynamic_pointer_cast<DDBIterator>(std::make_shared<VectorIterator<int128>>(vec));
            } else if (type == DT_LONG) {
                ddbIt = std::dynamic_pointer_cast<DDBIterator>(std::make_shared<VectorIterator<long long>>(vec));
            } else if (type == DT_INT) {
                ddbIt = std::dynamic_pointer_cast<DDBIterator>(std::make_shared<VectorIterator<int>>(vec));
            } else if (type == DT_TIMESTAMP || type == DT_NANOTIMESTAMP) {
                ddbIt = std::dynamic_pointer_cast<DDBIterator>(std::make_shared<VectorIterator<long long>>(vec));
            } else if (type == DT_DATE) {
                ddbIt = std::dynamic_pointer_cast<DDBIterator>(std::make_shared<VectorIterator<int>>(vec));
            } else if (type == DT_STRING || type == DT_SYMBOL) {
                ddbIt = std::dynamic_pointer_cast<DDBIterator>(std::make_shared<VectorIterator<DolphinString *>>(vec));
            } else {
                std::cerr << "unknown data type " << static_cast<int>(type) << std::endl;
            }
        } else if (form == DF_VECTOR) {
            if (type == DT_DOUBLE) {
                ddbIt =
                    std::dynamic_pointer_cast<DDBIterator>(std::make_shared<VectorIterator<std::vector<double>>>(vec));
            } else if (type == DT_LONG) {
                ddbIt = std::dynamic_pointer_cast<DDBIterator>(
                    std::make_shared<VectorIterator<std::vector<long long>>>(vec));
            } else if (type == DT_DECIMAL128) {
                ddbIt =
                    std::dynamic_pointer_cast<DDBIterator>(std::make_shared<VectorIterator<std::vector<int128>>>(vec));
            } else {
                std::cerr << "unknown data type with form=DF_VECTOR: " << static_cast<int>(type) << std::endl;
            }
        } else {
            std::cerr << "unknown data form " << static_cast<int>(form) << std::endl;
        }
        return ddbIt;
    }

    void setTable(const TableSP &table) {
        table_ = table;
        tableSize_ = table->size();
        row_ = 0;
        colIterators_.clear();
        VectorSP column = nullptr;
        for (auto &field : fields_) {
            try {
                column = table->getColumn(field.name);
            } catch (std::exception &e) {
                THROW_INVALID_INPUT("Invalid table for " + name_ + ", missing column: " + field.name + ".");
            }
            int columnType = static_cast<int>(column->getType());
            int expect = static_cast<int>(field.type);
            if (field.form == DF_VECTOR) {
                expect += ARRAY_TYPE_BASE;
            }
            if (columnType != expect && !(expect == DT_SYMBOL && columnType == DT_STRING)) {
                THROW_INVALID_INPUT("Column type mismatch for table " + name_ + " column " + field.name +
                                    ": type() should be " + std::to_string(expect));
            }
            colIterators_.push_back(getInputColumn(column, field.type, field.form));
        }
        col_ = colIterators_.begin();
    }

    void setTable(std::vector<ConstantSP> &table) {
        auto columns = table.begin();
        tableSize_ = (static_cast<VectorSP>(*columns))->size();
        row_ = 0;
        if (table.size() != fields_.size()) {
            return;
        }
        for (auto &field : fields_) {
            VectorSP vec = *columns++;
            colIterators_.push_back(getInputColumn(vec, field.type, field.form));
        }
        col_ = colIterators_.begin();
    }

    INDEX getSize() { return tableSize_; }

    operator bool() { return row_ < tableSize_; }

    void newRow() {
        ++row_;
        col_ = colIterators_.begin();
    }

    bool isEnd() { return row_ == tableSize_; }

    template <typename T>
    Schema &operator>>(T &v) {
        if (colIterators_.empty()) {
            std::cerr << "Table is not set" << std::endl;
        }
        // ignore optional extra fields
        if (col_ == colIterators_.end()) {
            v = T();
            return *this;
        }
        (*col_++)->getValue(v);
        return *this;
    }

#ifdef DOLPHINDB_JIT

    template <typename T>
    void getVectorField(instanceIt instanceBegin, VectorIterator<std::vector<T>> &col, const long long *rowBegin,
                        const long long *rowEnd, int fieldIndex, DATA_TYPE type) {
        std::size_t arraySize = col.getArraySize();
        TurboJet::TurboJetValue jitValue;
        while (rowBegin != rowEnd) {
            col.setIndex(*rowBegin++);
            VectorSP ddbVector = Util::createVector(type, arraySize, arraySize);
            setVectorData(ddbVector.get(), 0, arraySize, *col);
            ConstantSP sp = ddbVector;
            TurboJet::TurboJetValue::fromConstantSP(sp, &jitValue);
            (*instanceBegin++)->setAttribute(fieldIndex, jitValue);
        }
    }

    template <typename T>
    void getScalarField(instanceIt instanceBegin, VectorIterator<T> &col, const long long *rowBegin,
                        const long long *rowEnd, int fieldIndex, TurboJet::TurboJetValue v) {
        auto rowIt = rowBegin;
        while (rowIt != rowEnd) {
            col.setIndex(*rowIt++);
            setJitAttribute(*instanceBegin++, fieldIndex, v, *col);
        }
    }

    void getVectorField(instanceIt instanceBegin, const long long *rowBegin, const long long *rowEnd,
                        std::shared_ptr<DDBIterator> &col, DATA_TYPE type, int fieldIndex) {
        if (type == DT_DOUBLE) {
            getVectorField(instanceBegin, *std::dynamic_pointer_cast<VectorIterator<std::vector<double>>>(col),
                           rowBegin, rowEnd, fieldIndex, type);
        } else if (type == DT_LONG) {
            getVectorField(instanceBegin, *std::dynamic_pointer_cast<VectorIterator<std::vector<long long>>>(col),
                           rowBegin, rowEnd, fieldIndex, type);
        } else if (type == DT_DECIMAL128) {
            getVectorField(instanceBegin, *std::dynamic_pointer_cast<VectorIterator<std::vector<int128>>>(col),
                           rowBegin, rowEnd, fieldIndex, type);
        }
    }

    void getScalarField(instanceIt instanceBegin, const long long *rowBegin, const long long *rowEnd,
                        std::shared_ptr<DDBIterator> &col, DATA_TYPE type, int fieldIndex) {
        TurboJet::TurboJetValue v;
        if (type == DT_DOUBLE) {
            v.type_ = TurboJet::ValueType::DOUBLE;
            getScalarField(instanceBegin, *std::dynamic_pointer_cast<VectorIterator<double>>(col), rowBegin, rowEnd,
                           fieldIndex, v);
        } else if (type == DT_LONG) {
            v.type_ = TurboJet::ValueType::LONG;
            getScalarField(instanceBegin, *std::dynamic_pointer_cast<VectorIterator<long long>>(col), rowBegin, rowEnd,
                           fieldIndex, v);
        } else if (type == DT_TIMESTAMP) {
            v.type_ = TurboJet::ValueType::TIMESTAMP;
            getScalarField(instanceBegin, *std::dynamic_pointer_cast<VectorIterator<long long>>(col), rowBegin, rowEnd,
                           fieldIndex, v);
        } else if (type == DT_DATE) {
            v.type_ = TurboJet::ValueType::DATE;
            getScalarField(instanceBegin, *std::dynamic_pointer_cast<VectorIterator<int>>(col), rowBegin, rowEnd,
                           fieldIndex, v);
        } else if (type == DT_STRING || type == DT_SYMBOL) {
            v.type_ = TurboJet::ValueType::COUNTER;
            getScalarField(instanceBegin, *std::dynamic_pointer_cast<VectorIterator<DolphinString *>>(col), rowBegin,
                           rowEnd, fieldIndex, v);
        } else if (type == DT_INT) {
            v.type_ = TurboJet::ValueType::INT;
            getScalarField(instanceBegin, *std::dynamic_pointer_cast<VectorIterator<int>>(col), rowBegin, rowEnd,
                           fieldIndex, v);
        } else if (type == DT_DECIMAL128) {
            v.type_ = TurboJet::ValueType::COUNTER;
            getScalarField(instanceBegin, *std::dynamic_pointer_cast<VectorIterator<int128>>(col), rowBegin, rowEnd,
                           fieldIndex, v);
        }
    }

    void getField(instanceIt instanceIt, const long long *rowBegin, const long long *rowEnd,
                  std::shared_ptr<DDBIterator> &col, Field &field, int fieldIndex) {
        ConstantSP sp;
        if (field.form == DF_VECTOR) {
            getVectorField(instanceIt, rowBegin, rowEnd, col, field.type, fieldIndex);
        } else if (field.form == DF_SCALAR) {
            getScalarField(instanceIt, rowBegin, rowEnd, col, field.type, fieldIndex);
        }
    }

    void getInstances(std::vector<JITDolphinInstanceSP> &instances, int startIdx, Vector *rowIds) {
        int n = rowIds->size();
        while (instances.size() < static_cast<std::size_t>(startIdx + n)) {
            instances.push_back(new TurboJet::JITDolphinInstance(jitClass_));
        }
        instances.resize(startIdx + n);
        std::vector<long long> rowIdVec(n);
        const long long *rowId = rowIds->getLongConst(0, n, rowIdVec.data());
        TurboJet::TurboJetValue v;
        int index{0};
        std::vector<std::shared_ptr<DDBIterator>>::iterator colIt = colIterators_.begin();
        for (auto &field : fields_) {
            getField(instances.begin() + startIdx, rowId, rowId + n, *colIt, field, index);
            ++colIt;
            ++index;
        }
    }
#endif

  private:
    std::string name_;
    TableSP table_;
    std::vector<Field> fields_;
    std::vector<std::shared_ptr<DDBIterator>> colIterators_;
    DolphinClassSP jitClass_;
    INDEX tableSize_{0};
    INDEX row_{0};
    std::vector<std::shared_ptr<DDBIterator>>::iterator col_;
};

// Table output: Convert C++ type to TableSP.
class OTableStream {
  public:
    OTableStream() = default;
    OTableStream(const Schema &schema) : schema_(schema) { table_ = schema.getOutputColumns(); }

    void setValue(int value) { table_[col_++]->appendInt(&value, 1); }

    void setValue(long long value) { table_[col_++]->appendLong(&value, 1); }

    void setValue(double value) { table_[col_++]->appendDouble(&value, 1); }

    void setValue(const std::string &value) { table_[col_++]->appendString(&value, 1); }

    void setValue(const decimal128 value) { table_[col_++]->append(getConstantSP(value)); }

    auto getTable() -> TableSP {
        return Util::createTable(schema_.getNames(), std::vector<ConstantSP>(table_.begin(), table_.end()));
    }

    void newRow() { col_ = 0; }

  private:
    Schema schema_;
    std::vector<VectorSP> table_;
    std::size_t col_{0};
};

template <typename T>
inline OTableStream &operator<<(OTableStream &s, T x) {
    s.setValue(x);
    return s;
}

// Dictionary output: Convert C++ type to DictSP.
class ODictStream {
  public:
    ODictStream() = default;
    ~ODictStream() {
        dict_ = nullptr;
        values_.clear();
    }
    void setFields(std::vector<Field> fields) {
        fields_ = std::move(fields);
        dict_ = Util::createDictionary(DT_STRING, nullptr, DT_ANY, nullptr);
        for (auto &f : fields_) {
            ConstantSP sp = Util::createConstant(f.type, decimalScale);
            dict_->set(f.name, sp);
            values_.push_back(sp);
        }
        reset();
    }

    void setValue(int value) { (it_++)->getAs<Int>()->setInt(value); }

    void setValue(long long value) { (it_++)->getAs<Long>()->setLong(value); }

    void setValue(double value) { (it_++)->getAs<Double>()->setDouble(value); }

    void setValue(const std::string &value) { (it_++)->getAs<String>()->setString(value); }

    void setValue(const decimal128 value) {
        (it_++)->getAs<Decimal128>()->setDecimal128(0, decimalScale, static_cast<int128>(value));
    }

    auto getDict() -> DictionarySP { return dict_; }

    void reset() { it_ = values_.begin(); }

  private:
    std::vector<Field> fields_;
    std::vector<ConstantSP> values_;
    std::vector<ConstantSP>::iterator it_;
    DictionarySP dict_;
};

template <typename T>
inline ODictStream &operator<<(ODictStream &s, T x) {
    s.setValue(x);
    return s;
}

// Retrieve the enumeration corresponding to the type, used for constructing output tables in the template.
inline constexpr DATA_TYPE getTypeEnum(double) { return DT_DOUBLE; }

// Misc

enum class DDBfunc : int {
    max,
    imax,
    deltas,
    prev,
    cummax,
    sub,
    ratio,
    std,
    skew,
    kurtosis,
    covar,
};

class DolphinDBFunctions {
  public:
    DolphinDBFunctions(Heap *heap);
    inline ConstantSP call(Heap *heap, DDBfunc func, std::vector<ConstantSP> &&args) const {
        return funcs_.at(static_cast<int>(func))->call(heap, args);
    }

  private:
    std::unordered_map<int, FunctionDefSP> funcs_;
};

// optimize to extract user order info
enum OrderEnum {
    SYMBOL = 0,
    SYMBOL_SOURCE,
    TIMESTAMP,
    ORDER_TYPE,
    PRICE,
    STOP_PRICE,
    ORDER_QTY,
    DIRECTION,
    TIME_IN_FORCE,
    TAKE_PRICE,
    SLIPPAGE,
    EXPIRE_TIME,
    SETTL_TYPE,
    BID_PRICE,
    BID_QTY,
    ASK_PRICE,
    ASK_QTY,
    ORDER_ID,
    CHANNEL
};

}  // namespace ddb
