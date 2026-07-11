#ifndef TYPEDEF_H_
#define TYPEDEF_H_

#include <variant>
#include <memory>
#include <string>
#include <optional>
#include <cstdint>
#include <cassert>
#include <limits>

#include "CoreConcept.h"
#include "SmartPointer.h"
#include "Types.h"
#include "SmartPointer.h"
#include "Logger.h"
#include "Util.h"

namespace ddb {


class DolphinClass;
class DolphinInstance;
typedef ObjectPtr<DolphinClass> DolphinClassSP;

struct DictDef;
struct VectorDef;
struct PairDef;
struct MatrixDef;
struct SetDef;
struct TableDef;
struct ClassDef;
struct ConstantDef;
struct AnyDef;

using VectorDefSP = SmartPointer<VectorDef>;
using DictDefSP = SmartPointer<DictDef>;
using PairDefSP = SmartPointer<PairDef>;
using MatrixDefSP = SmartPointer<MatrixDef>;
using SetDefSP = SmartPointer<SetDef>;
using TableDefSP = SmartPointer<TableDef>;
using ClassDefSP = SmartPointer<ClassDef>;
using ConstantDefSP = SmartPointer<ConstantDef>;
using AnyDefSP = SmartPointer<AnyDef>;

using ScalarDef = std::variant<ConstantDefSP, AnyDefSP>;
using AllTypesDef = std::variant<ClassDefSP, ConstantDefSP, AnyDefSP, VectorDefSP, DictDefSP, PairDefSP, MatrixDefSP, SetDefSP, TableDefSP>;


// Helper function to convert variant to TypeDefBase reference using template
// Usage: toTypeDefBase<ScalarDef>(def) or toTypeDefBase<AllTypesDef>(def)
template<typename Variant>
inline TypeDefBase& toTypeDefBase(Variant& def) {
    return std::visit([](auto& arg) -> TypeDefBase& {
        return *arg;
    }, def);
}

template<typename Variant>
inline const TypeDefBase& toTypeDefBase(const Variant& def) {
    return std::visit([](const auto& arg) -> const TypeDefBase& {
        return *arg;
    }, def);
}

template<typename Variant>
inline TypeDefBaseSP toTypeDefBaseSP(const Variant& def) {
    return std::visit([](const auto& arg) -> TypeDefBaseSP {
        return arg;
    }, def);
}

struct AnyDef : TypeDefBase {
public:
    AnyDef() : TypeDefBase(DefEnum::DF_ANY, DF_SCALAR, DT_ANY) {}
    ~AnyDef() {}
    
    IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const override;
    IO_ERR deserialize(Session* session, const DataInputStreamSP& in) override;
};

struct ClassDef : TypeDefBase {
public:
    ClassDef() : TypeDefBase(DefEnum::DF_CLASS, DF_SYSOBJ) { setComplexMeta(DF_SYSOBJ); }
    ClassDef(const DolphinClassSP &cls)
        : TypeDefBase(DefEnum::DF_CLASS, DF_SYSOBJ), cls(cls) {
        setComplexMeta(DF_SYSOBJ);
    }
    ~ClassDef() {}

    IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const override;
    IO_ERR deserialize(Session* session, const DataInputStreamSP& in) override;
    string getScript(int indention) const override;

    DolphinClassSP cls;
};

struct ConstantDef : TypeDefBase {
public:
    ConstantDef() : TypeDefBase(DefEnum::DF_CONSTANT, DF_SCALAR, DT_VOID) {}
    ConstantDef(DATA_TYPE type, int extra = 0) : TypeDefBase(DefEnum::DF_CONSTANT, DF_SCALAR, type, DT_VOID, extra) {}
    ~ConstantDef() {}

    IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const override;
    IO_ERR deserialize(Session* session, const DataInputStreamSP& in) override;
};

struct VectorDef : TypeDefBase {
public:
    VectorDef();
    VectorDef(const ScalarDef &def);
    ~VectorDef() {}

    IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const override;
    IO_ERR deserialize(Session* session, const DataInputStreamSP& in) override;
    void refreshMeta();

    ScalarDef def;
};

struct DictDef : TypeDefBase {
public:
    DictDef();
    DictDef(const ConstantDefSP &key, const AllTypesDef &valueDef);
    ~DictDef() {}

    IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const override;
    IO_ERR deserialize(Session* session, const DataInputStreamSP& in) override;
    string getScript(int indention) const override;
    void refreshMeta();

    ConstantDefSP keyDef;
    AllTypesDef valueDef;
};

struct PairDef : TypeDefBase {
public:
    PairDef();
    PairDef(const ConstantDefSP &def);
    ~PairDef() {}

    IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const override;
    IO_ERR deserialize(Session* session, const DataInputStreamSP& in) override;
};

struct MatrixDef : TypeDefBase {
public:
    MatrixDef();
    MatrixDef(const ConstantDefSP &def);
    ~MatrixDef() {}

    IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const override;
    IO_ERR deserialize(Session* session, const DataInputStreamSP& in) override;
};

struct SetDef : TypeDefBase {
public:
    SetDef();
    SetDef(const ConstantDefSP &def);
    ~SetDef() {}

    IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const override;
    IO_ERR deserialize(Session* session, const DataInputStreamSP& in) override;
};

struct TableDef : TypeDefBase {
public:
    TableDef();
    ~TableDef() {}

    IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const override;
    IO_ERR deserialize(Session* session, const DataInputStreamSP& in) override;
};

class TypeDefVisitor {
public:
    virtual ~TypeDefVisitor() = default;

    virtual void visit(const ClassDef &def, int depth) = 0;
    virtual void visit(const ConstantDef &def, int depth) = 0;
    virtual void visit(const AnyDef &def, int depth) = 0;
    virtual void visit(const VectorDef &def, int depth) = 0;
    virtual void visit(const DictDef &def, int depth) = 0;
    virtual void visit(const PairDef &def, int depth) = 0;
    virtual void visit(const MatrixDef &def, int depth) = 0;
    virtual void visit(const SetDef &def, int depth) = 0;
    virtual void visit(const TableDef &def, int depth) = 0;
};


template<typename T, typename Variant>
std::optional<T> try_get(const Variant& v) {
    if (auto* p = std::get_if<T>(&v)) {
        return *p;
    }
    return std::nullopt;
}

// Convert DefEnum to string representation
inline const char* defEnumToString(DefEnum def) {
    switch (def) {
        case DefEnum::DF_ANY:
            return "ANY";
        case DefEnum::DF_CONSTANT:
            return "CONSTANT";
        case DefEnum::DF_CLASS:
            return "CLASS";
        case DefEnum::DF_VECTOR:
            return "VECTOR";
        case DefEnum::DF_DICTIONARY:
            return "DICTIONARY";
        case DefEnum::DF_PAIR:
            return "PAIR";
        case DefEnum::DF_MATRIX:
            return "MATRIX";
        case DefEnum::DF_SET:
            return "SET";
        case DefEnum::DF_TABLE:
            return "TABLE";
        default:
            return "UNKNOWN";
    }
}

// Serialization helper functions
IO_ERR serializeAllTypes(const AllTypesDef& def, const ByteArrayCodeBufferSP& buffer);
TypeDefBaseSP deserializeTypeDef(Session* session, const DataInputStreamSP& in, IO_ERR& ret);
AllTypesDef deserializeAllTypes(Session* session, const DataInputStreamSP& in, IO_ERR& ret);
// Convert ObjectSP to TypeDef based on its form and type
AllTypesDef objectToTypeDef(const ObjectSP& obj);
std::optional<AllTypesDef> createTypeDefFromCompactMeta(int32_t packedMeta, Session* session, const DataInputStreamSP& in, IO_ERR& ret);
} // namespace ddb

#endif
