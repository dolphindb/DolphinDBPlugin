#ifndef TURBOJET_INTERFACE_H_
#define TURBOJET_INTERFACE_H_

#include <cstdint>

#include "Concepts.h"
#include "CoreConcept.h"
#include "OperatorImp.h"
#include "ScalarImp.h"
#include "Types.h"
namespace ddb {
class UserDefinedFunctionImp;
namespace TurboJet {

// flags
constexpr uint32_t ISNULL = 1;
constexpr uint32_t ISINSTANCE = 2;

enum class ValueType : int32_t {
    // logical
    BOOL,
    // integral
    CHAR,
    SHORT,
    INT,
    LONG,
    // temporal
    TIME,
    TIMESTAMP,
    SECOND,
    DATE,
    // floating
    FLOAT,
    DOUBLE,
    // decimal64
    DEICMAL64,
    // other
    COUNTER,
};

inline bool ValueTypeIsTemporal(ValueType t) { return t >= ValueType::TIME && t < ValueType::FLOAT; }

inline bool ValueTypeIsFloating(ValueType t) { return t == ValueType::FLOAT || t == ValueType::DOUBLE; }

inline string ValueTypeToString(ValueType t) {
    const static vector<string> TypeString = {// logical
                                              "BOOL",
                                              // integral
                                              "CHAR", "SHORT", "INT", "LONG",
                                              // temporal
                                              "TIME", "TIMESTAMP", "SECOND", "DATE",
                                              // floating
                                              "FLOAT", "DOUBLE",
                                              // decimal64
                                              "DECIMAL64",
                                              // other
                                              "COUNTER"};
    return TypeString[static_cast<int>(t)];
}

template <typename T>
struct RawTypeToValueType {
    static constexpr ValueType type() { return ValueType::COUNTER; }
};

template <>
struct RawTypeToValueType<int> {
    static constexpr ValueType type() { return ValueType::INT; }
};

template <>
struct RawTypeToValueType<short> {
    static constexpr ValueType type() { return ValueType::SHORT; }
};

template <>
struct RawTypeToValueType<bool> {
    static constexpr ValueType type() { return ValueType::BOOL; }
};

template <>
struct RawTypeToValueType<long long> {
    static constexpr ValueType type() { return ValueType::LONG; }
};

template <>
struct RawTypeToValueType<char> {
    static constexpr ValueType type() { return ValueType::CHAR; }
};

template <>
struct RawTypeToValueType<float> {
    static constexpr ValueType type() { return ValueType::FLOAT; }
};

template <>
struct RawTypeToValueType<double> {
    static constexpr ValueType type() { return ValueType::DOUBLE; }
};

class JITDolphinInstance;
struct TurboJetValue {
    ValueType type_;
    uint32_t flags_;
    union {
        int i;
        int64_t l;
        double d;
        char c;  // dolphindb uses char to represent bool
        float f;
        short s;
        Counter *counter_;
    } value_;

    TurboJetValue() : type_(static_cast<ValueType>(0)), flags_(0) { value_.l = 0; }
	// TurboJetValue(const TurboJetValue& sp) noexcept{
    //     type_ = sp.type_;
    //     flags_ = sp.flags_;
    //     value_ = sp.value_;
    //     if (type_ != ValueType::COUNTER || value_.counter_ == nullptr) return;
    //     value_.counter_->addRef();
	// }

	// TurboJetValue(TurboJetValue&& sp) noexcept{
    //     type_ = sp.type_;
    //     flags_ = sp.flags_;
    //     value_ = sp.value_;
    //     sp.value_.l = 0;
    //     sp.flags_ = 0;
    //     sp.type_ = static_cast<ValueType>(0);
	// }

	// TurboJetValue& operator =(TurboJetValue sp) noexcept{
    //     std::swap(type_, sp.type_);
    //     std::swap(flags_, sp.flags_);
    //     std::swap(value_, sp.value_);
    //     return *this;
	// }

    // ~TurboJetValue() {
	// 	if(type_ == ValueType::COUNTER && LIKELY(value_.counter_ != nullptr) && value_.counter_->release()==0){
	// 		delete static_cast<Constant*>(value_.counter_->p_);
	// 		delete value_.counter_;
	// 		value_.counter_=0;
	// 	}
    // }

    ConstantSP toConstantSP(bool isMove = false) const {
        ConstantSP cons;
        switch (type_) {
            case ValueType::INT:
                cons = new Int(value_.i);
                break;
            case ValueType::TIME:
                cons = new Time(value_.i);
                break;
            case ValueType::SECOND:
                cons = new Second(value_.i);
                break;
            case ValueType::DATE:
                cons = new Date(value_.i);
                break;
            case ValueType::LONG:
                cons = new Long(value_.l);
                break;
            case ValueType::TIMESTAMP:
                cons = new Timestamp(value_.l);
                break;
            case ValueType::DOUBLE:
                cons = new Double(value_.d);
                break;
            case ValueType::BOOL:
                cons = new Bool(value_.c);
                break;
            case ValueType::CHAR:
                cons = new Char(value_.c);
                break;
            case ValueType::FLOAT:
                cons = new Float(value_.f);
                break;
            case ValueType::SHORT:
                cons = new Short(value_.s);
                break;
            case ValueType::DEICMAL64:
            case ValueType::COUNTER:
                if (value_.counter_ == nullptr) return Expression::void_;
                cons = ConstantSP(nullptr, value_.counter_);
                if (isMove) {
                    value_.counter_->release();
                }
                break;
            default:
                throw RuntimeException("JIT internal error.");
        }
        if (isNull()) {
            cons->setNull();
        }
        return cons;
    }
    
    std::string getString() {
        switch (type_) {
            case ValueType::INT:
                return std::to_string(value_.i);
            case ValueType::TIME:
                return Time(value_.i).getString();
            case ValueType::SECOND:
                return Second(value_.i).getString();
            case ValueType::DATE:
                return Date(value_.i).getString();
            case ValueType::TIMESTAMP:
                return Timestamp(value_.l).getString();
            case ValueType::LONG:
                return std::to_string(value_.l);
            case ValueType::DOUBLE:
                return std::to_string(value_.d);
            case ValueType::BOOL:
                return std::to_string(value_.c);
            case ValueType::CHAR:
                return std::to_string(value_.c);
            case ValueType::FLOAT:
                return std::to_string(value_.f);
            case ValueType::SHORT:
                return std::to_string(value_.s);
            case ValueType::DEICMAL64:
            case ValueType::COUNTER:
                if (value_.counter_ == nullptr) return "";
                return static_cast<Constant*>(value_.counter_->p_)->getString();
            default:
                throw RuntimeException("JIT internal error");
        }
    }
    
    const DolphinString &getStringRef() {
        if (type_ == ValueType::COUNTER) {
            if (value_.counter_ == nullptr) {
                throw RuntimeException("TurboJetValue::getStringRef the COUNTER is nullptr.");
            }
            return static_cast<Constant *>(value_.counter_->p_)->getStringRef();
        }
        throw RuntimeException("TurboJetValue::getStringRef only support COUNTER.");
    }
    
    inline bool isNull() const { return flags_ & ISNULL; }

    void setNull(bool set = true) {
        if (UNLIKELY(!set)) {
            flags_ &= ~ISNULL;
            return;
        }
        setNullFlag();
        switch (type_) {
            case ValueType::INT:
            case ValueType::TIME:
            case ValueType::SECOND:
            case ValueType::DATE:
                value_.i = INT_MIN;
                break;
            case ValueType::LONG:
            case ValueType::TIMESTAMP:
                value_.l = LLONG_MIN;
                break;
            case ValueType::DOUBLE:
                value_.d = DBL_NMIN;
                break;
            case ValueType::BOOL:
                value_.c = CHAR_MIN;
                break;
            case ValueType::CHAR:
                value_.c = CHAR_MIN;
                break;
            case ValueType::FLOAT:
                value_.f = FLT_NMIN;
                break;
            case ValueType::SHORT:
                value_.s = SHRT_MIN;
                break;
            case ValueType::DEICMAL64:
            case ValueType::COUNTER:
                static_cast<Constant *>(value_.counter_->p_)->setNull();
                break;
            default:
                throw RuntimeException("TurboJetValue::setNull not support type: " + ValueTypeToString(type_));
        }
    }

    void inline setNullFlag() { flags_ |= ISNULL; }

    static void fromConstantSP(const ConstantSP &sp, TurboJetValue *out, std::vector<Counter *> *deRefList = nullptr,
                               bool insideJIT = false, bool ensureNotNull = false);
    static void fromConstantSP(const ObjectSP &sp, TurboJetValue *out, std::vector<Counter *> *deRefList = nullptr,
                               bool insideJIT = false, bool ensureNotNull = false);
    static void fromConstantSP(const FunctionDefSP &sp, TurboJetValue *out, std::vector<Counter *> *deRefList = nullptr,
                               bool insideJIT = false, bool ensureNotNull = false);
    static void fromConstantSP(const SmartPointer<JITDolphinInstance> &sp, TurboJetValue *out,
                               std::vector<Counter *> *deRefList = nullptr, bool insideJIT = false,
                               bool ensureNotNull = false);
    // for template use only
    template <class T>
    inline T &get() {
        return *reinterpret_cast<T *>(&value_);
    }

    template <class T>
    inline const T &get() const {
        return *const_cast<T *>(reinterpret_cast<const T *>(&value_));
    }

    static ValueType DDBTypeToValueType(DATA_TYPE dt) {
        ValueType valueType;
        switch (dt) {
            case DT_INT:
                valueType = ValueType::INT;
                break;
            case DT_TIME:
                valueType = ValueType::TIME;
                break;
            case DT_TIMESTAMP:
                valueType = ValueType::TIMESTAMP;
                break;
            case DT_SECOND:
                valueType = ValueType::SECOND;
                break;
            case DT_DATE:
                valueType = ValueType::DATE;
                break;
            case DT_LONG:
                valueType = ValueType::LONG;
                break;
            case DT_DOUBLE:
                valueType = ValueType::DOUBLE;
                break;
            case DT_BOOL:
                valueType = ValueType::BOOL;
                break;
            case DT_CHAR:
                valueType = ValueType::CHAR;
                break;
            case DT_FLOAT:
                valueType = ValueType::FLOAT;
                break;
            case DT_SHORT:
                valueType = ValueType::SHORT;
                break;
            default:
                valueType = ValueType::COUNTER;
        }
        return valueType;
    }

    template <class ExtractType>
    static inline ExtractType ExtractValue(const TurboJetValue *val) {
        switch (val->type_) {
            case ValueType::INT:
            case ValueType::TIME:
            case ValueType::SECOND:
            case ValueType::DATE:
                return static_cast<ExtractType>(val->value_.i);
            case ValueType::LONG:
            case ValueType::TIMESTAMP:
                return static_cast<ExtractType>(val->value_.l);
            case ValueType::DOUBLE:
                return static_cast<ExtractType>(val->value_.d);
            case ValueType::FLOAT:
                return static_cast<ExtractType>(val->value_.f);
            case ValueType::BOOL:
                return static_cast<ExtractType>(val->value_.c);
            case ValueType::CHAR:
                return static_cast<ExtractType>(val->value_.c);
            case ValueType::SHORT:
                return static_cast<ExtractType>(val->value_.s);
            default:
                throw RuntimeException("JIT internal error");
        }
    }

    template <class ExtractType>
    static inline ExtractType ExtractValue(TurboJetValue *val) {
        switch (val->type_) {
            case ValueType::INT:
            case ValueType::TIME:
            case ValueType::SECOND:
            case ValueType::DATE:
                return static_cast<ExtractType>(val->value_.i);
            case ValueType::LONG:
            case ValueType::TIMESTAMP:
                return static_cast<ExtractType>(val->value_.l);
            case ValueType::DOUBLE:
                return static_cast<ExtractType>(val->value_.d);
            case ValueType::FLOAT:
                return static_cast<ExtractType>(val->value_.f);
            case ValueType::BOOL:
                return static_cast<ExtractType>(val->value_.c);
            case ValueType::CHAR:
                return static_cast<ExtractType>(val->value_.c);
            case ValueType::SHORT:
                return static_cast<ExtractType>(val->value_.s);
            default:
                throw RuntimeException("unkown error");
        }
    }

    static void castAssignment(TurboJetValue &lhs, const TurboJetValue &rhs, DolphinClass* cls, int attrIdx) {
        if (lhs.type_ == rhs.type_) {
            lhs = rhs;
            return;
        }

        switch (lhs.type_) {
            case ValueType::INT:
            case ValueType::TIME:
            case ValueType::SECOND:
            case ValueType::DATE:
                lhs.value_.i = ExtractValue<int>(&rhs);
                break;
            case ValueType::LONG:
            case ValueType::TIMESTAMP:
                lhs.value_.l = ExtractValue<long long>(&rhs);
                break;
            case ValueType::DOUBLE:
                lhs.value_.d = ExtractValue<double>(&rhs);
                break;
            case ValueType::FLOAT:
                lhs.value_.f = ExtractValue<float>(&rhs);
                break;
            case ValueType::BOOL:
            case ValueType::CHAR:
                lhs.value_.c = ExtractValue<char>(&rhs);
                break;
            case ValueType::SHORT:
                lhs.value_.s = ExtractValue<short>(&rhs);
                break;
            default:
                throw RuntimeException("JIT: Data type does not match when assign value to attribute '" +
                                           cls->getAttribute(attrIdx) + "' in class '" + cls->getName() + "'.");
        }
    }

   private:
    static void ExtractFromSP(const ConstantSP &sp, TurboJetValue *out, ValueType type) {
        switch (type) {
            case ValueType::INT:
            case ValueType::TIME:
            case ValueType::SECOND:
            case ValueType::DATE:
                out->value_.i = sp->getInt();
                break;
            case ValueType::LONG:
            case ValueType::TIMESTAMP:
                out->value_.l = sp->getLong();
                break;
            case ValueType::DOUBLE:
                out->value_.d = sp->getDouble();
                break;
            case ValueType::BOOL:
                out->value_.c = sp->getBool();
                break;
            case ValueType::CHAR:
                out->value_.c = sp->getChar();
                break;
            case ValueType::FLOAT:
                out->value_.f = sp->getFloat();
                break;
            case ValueType::SHORT:
                out->value_.s = sp->getShort();
                break;
            default: {
                // assert(out->value_.counter_ == nullptr);
                out->value_.counter_ = sp.getCounter();
                // out->value_.counter_->addRef();
            }
        }
    }
    static void DDBIncreaseRef(TurboJetValue *val, std::vector<Counter *> *deRefList, bool addToList) {
        if (val->type_ != ValueType::COUNTER) {
            return;
        }
        if (addToList) {
            deRefList->push_back(val->value_.counter_);
        }
        val->value_.counter_->addRef();
    }
};

class JITDolphinInstance : public OOInstance {
   public:
    JITDolphinInstance(const DolphinClassSP &ddbClass) : OOInstance(ddbClass, SYSOBJ_TYPE::JIT_DDB_INSTANCE) {
        DolphinClass *cls = static_cast<DolphinClass *>(class_.get());
        clsId_ = cls->getJitClassId();
        int count = cls->getAttributeCount();
        if (UNLIKELY(count > inlineNum_)) {
            extraData_ = std::unique_ptr<TurboJetValue[]>(new TurboJetValue[count - inlineNum_]);
            extraRefHolder_ = std::unique_ptr<ConstantSP[]>(new ConstantSP[count - inlineNum_]);
        }
        // Call ddbClass constructor function
        for (int i = 0; i < count; i++) {
            TurboJetValue *data;
            if (LIKELY(i < inlineNum_)) {
                data = &data_[i];
            } else {
                data = &extraData_[i - inlineNum_];
            }
            ConstantSP typeObj = cls->getAttributeType(i);
            ValueType valueType = TurboJetValue::DDBTypeToValueType(typeObj->getType());
            if (typeObj->getForm() == DF_SCALAR && valueType != ValueType::COUNTER) {
                data->type_ = valueType;
                data->setNull();
            } else {
                data->type_ = ValueType::COUNTER;
            }
        }
    }
    virtual ~JITDolphinInstance() {}

    TurboJetValue *getAttributeRef(int index) {
        TurboJetValue *data;
        if (LIKELY(index < inlineNum_)) {
            data = &data_[index];
        } else {
            data = &extraData_[index - inlineNum_];
        }
        return data;
    }
    TurboJetValue getAttribute(int index) const {
        auto self = const_cast<JITDolphinInstance *>(this);
        return *(self->getAttributeRef(index));
    }
    TurboJetValue getAttribute(const string &name) const {
        int index = ((DolphinClass *)class_.get())->getAttributeIndex(name);
        return getAttribute(index);
    }

    void setAttribute(int index, const TurboJetValue &attr) {
        // TODO: type check
        TurboJetValue *data;
        ConstantSP *ref;
        if (LIKELY(index < inlineNum_)) {
            data = &data_[index];
            ref = &refHolder_[index];
        } else {
            data = &extraData_[index - inlineNum_];
            ref = &extraRefHolder_[index - inlineNum_];
        }
        TurboJetValue::castAssignment(*data, attr, ((DolphinClass *)class_.get()), index);
        if (attr.type_ == ValueType::COUNTER) {
            ConstantSP c = ConstantSP(nullptr, attr.value_.counter_);
            *ref = c;
            (*ref)->setTemporary(false);
        }
        // set flag
        data->flags_ = attr.flags_;
    }

    void setAttribute(const string &name, const TurboJetValue &attr) {
        int index = ((DolphinClass *)class_.get())->getAttributeIndex(name);
        setAttribute(index, attr);
    }
    virtual bool set(const ConstantSP &index, const ConstantSP &value) override {
        TurboJetValue tmpValue;
        TurboJetValue::fromConstantSP(value, &tmpValue);
        if (index->isNumber()) {
            setAttribute(index->getInt(), tmpValue);
        } else {
            setAttribute(index->getString(), tmpValue);
        }
        return true;
    }

    virtual ConstantSP getValue() const override {
        SmartPointer<JITDolphinInstance> ins = new JITDolphinInstance(getClass());
        DolphinClass *cls = static_cast<DolphinClass *>(class_.get());
        int count = cls->getAttributeCount();
        for (int i = 0; i < count; i++) {
            ins->setAttribute(i, getAttribute(i));
        }
        ins->clsId_ = clsId_;
        return ins;
    }

    virtual ConstantSP getMember(const string &name) const override {
        int idx = ((DolphinClass *)class_.get())->getMemberIndex(name);
        if (idx < 0) throw RuntimeException("JIT: member '" + name + "' not found in '" + class_->getFullName() + "'.");
        if (idx >= 65536) return ((DolphinClass *)class_.get())->getMethod(idx);
        return getAttribute(idx).toConstantSP();
    }

    virtual ConstantSP getMember(const ConstantSP &key) const override { return getMember(key->getString()); }

    ConstantSP getMember(int idx) const {
        if (idx < 0) throw RuntimeException("Illegal idx: " + std::to_string(idx));
        if (idx >= 65536) return ((DolphinClass *)class_.get())->getMethod(idx);
        return getAttribute(idx).toConstantSP();
    }

    virtual IO_ERR serialize(const ByteArrayCodeBufferSP &buffer) const override {
        throw RuntimeException{"serialize for OOP instance is not supported in JIT."};
    }

    virtual string getString() const override {
        string str("<Instance of ");
        str.append(getClass()->getQualifier() + "::" + getClass()->getName());
        str.append(" ");
        str.append(std::to_string((long long)this));
        str.append(1, '>');
        str.append(1, '\n');

        DolphinClass *cls = static_cast<DolphinClass *>(class_.get());
        int count = cls->getAttributeCount();        
        for (int i = 0;i < count; ++i) {
            str.append(cls->getAttribute(i) + ": ");
            if (LIKELY(i < inlineNum_)) {
                str.append(data_[i].toConstantSP()->getString());
            } else {
                str.append(extraData_[i-inlineNum_].toConstantSP()->getString());
            }
            str.append(1, '\n');
        }
        return str;
    }

    virtual string getScript() const override {
       return getString();
    }
    int getClsId() { return clsId_; }

   protected:
    static constexpr int inlineNum_ = 8;
    int clsId_ = 0;
    TurboJetValue data_[inlineNum_];
    ConstantSP refHolder_[inlineNum_];
    std::unique_ptr<TurboJetValue[]> extraData_;
    std::unique_ptr<ConstantSP[]> extraRefHolder_;
};

inline bool operator==(const TurboJetValue &lhs, const TurboJetValue &rhs) {
    if (lhs.type_ != ValueType::COUNTER && rhs.type_ != ValueType::COUNTER) {
        return lhs.value_.l == rhs.value_.l;
    }
    return OperatorImp::equal(lhs.toConstantSP(), rhs.toConstantSP())->getBool();
}

inline void TurboJetValue::fromConstantSP(const ConstantSP &sp, TurboJetValue *out,
                                          std::vector<Counter *> *deRefList, bool insideJIT, bool ensureNotNull) {
    auto type = sp->getType();
    out->value_.l = 0;  // reset union
    out->flags_ = 0;
    if (sp->getForm() != DF_SCALAR) {
        out->value_.counter_ = sp.getCounter();
        // out->value_.counter_->addRef();
        out->type_ = ValueType::COUNTER;
    } else {
        out->type_ = DDBTypeToValueType(type);
        ExtractFromSP(sp, out, out->type_);
    }
    if (!ensureNotNull && sp->isNull()) {
        out->setNull();
    }
    if (!insideJIT) {
        sp->setTemporary(false);
    }
    if (deRefList && out->type_ == ValueType::COUNTER) {
        DDBIncreaseRef(out, deRefList, insideJIT);
    }
}
inline void TurboJetValue::fromConstantSP(const ObjectSP &sp, TurboJetValue *out, std::vector<Counter *> *deRefList,
                                          bool insideJIT, bool ensureNotNull) {
    return fromConstantSP(static_cast<ConstantSP>(sp), out, deRefList, insideJIT, ensureNotNull);
}
inline void TurboJetValue::fromConstantSP(const FunctionDefSP &sp, TurboJetValue *out, std::vector<Counter *> *deRefList,
                                          bool insideJIT, bool ensureNotNull) {
    return fromConstantSP(static_cast<ConstantSP>(sp), out, deRefList, insideJIT, ensureNotNull);
}
inline void TurboJetValue::fromConstantSP(const SmartPointer<JITDolphinInstance> &sp, TurboJetValue *out,
                                          std::vector<Counter *> *deRefList, bool insideJIT, bool ensureNotNull) {
    out->value_.l = 0;  // reset union
    out->flags_ = 0;
    out->type_ = ValueType::COUNTER;
    // assert(out->value_.counter_ == nullptr);
    out->value_.counter_ = sp.getCounter();
    // out->value_.counter_->addRef();
    if (!ensureNotNull && sp->isNull()) {
        out->setNull();
    }
    if (!insideJIT) {
        sp->setTemporary(false);
    }
    if (deRefList) {
        DDBIncreaseRef(out, deRefList, insideJIT);
    }
}

using TJFunc_t = void (*)(TurboJetValue *, Heap *, TurboJetValue *, int);

class TurboJetUDFBase {
public:
    virtual ~TurboJetUDFBase() = default;
};
using TurboJetUDFBaseSP = SmartPointer<TurboJetUDFBase>;

ConstantSP callTurboJetCompiledCode(UserDefinedFunctionImp* udf, Heap *heap, vector<ConstantSP> &arguments);
FunctionDef::TurboJetFunc getTurboJetJitFuncPtr(const UserDefinedFunctionImp* udf);
void removeTurboJetFunc(UserDefinedFunctionImp* udf);

}  // namespace TurboJet
}
#endif
