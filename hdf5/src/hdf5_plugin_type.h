
#ifndef HDF5_PLUGIN_TYPE_COLUMN_H
#define HDF5_PLUGIN_TYPE_COLUMN_H
#include <CoreConcept.h>
#include <Logger.h>
#include <ScalarImp.h>
#include <Util.h>
#include <hdf5_plugin_obj.h>
#include <hdf5_plugin_util.h>

namespace H5PluginImp {
class TypeColumn;
class H5DataType;
using H5ColumnSP = SmartPointer<TypeColumn>;
struct pack_info_t {
    char *raw_data;
    char *buffer;
    int stride;
    int len;
};
class TypeColumn {
  public:
    TypeColumn() = delete;
    // create column from simple type
    static H5ColumnSP createNewColumn(H5DataType &srcType);

    // convert simple type
    static bool convertHdf5SimpleType(H5DataType &srcType, H5DataType &convertedType);
    static bool createCompoundColumns(H5DataType &srcType, vector<H5ColumnSP> &cols, H5DataType &convertedType);
    static bool createArrayColumns(H5DataType &srcType, vector<H5ColumnSP> &cols, H5DataType &convertedType);
    static bool createComplexColumns(H5DataType &srcType, vector<H5ColumnSP> &cols, H5DataType &convertedType);
    static bool createHdf5TypeColumns(H5DataType &srcType, vector<H5ColumnSP> &cols, H5DataType &convertedType);

    VectorSP createDolphinDBColumnVector(DATA_TYPE dt, int size, int cap);
    VectorSP createDolphinDBColumnVector(const VectorSP &destVec, int size, int cap);
    VectorSP createDolphinDBColumnVector(int size, int cap);

    DATA_TYPE srcType() const { return srcType_; }
    VectorSP colVec() { return colVec_; }
    int appendData(char *raw_data, int offset, int stride, int len, vector<char> &buffer);
    virtual ~TypeColumn() = default;
    virtual bool compatible(DATA_TYPE destType) const { return false; };
    virtual int h5size() const = 0;
    virtual DATA_TYPE packData(pack_info_t t) { return DT_VOID; }

  protected:
    virtual VectorSP createCompatibleVector(VectorSP destVec, DATA_TYPE destType, int size, int cap);
    int doAppend(void *data, int len, DATA_TYPE basicType);

    explicit TypeColumn(DATA_TYPE srcType) : srcType_(srcType) {}

    DATA_TYPE srcType_ = DT_VOID;
    DATA_TYPE destType_ = DT_VOID;

    int srcTypeSize_ = 0;
    int destTypeSize_ = 0;

    VectorSP colVec_ = nullptr;
};

class IntegerColumn : public TypeColumn {
  public:
    bool compatible(DATA_TYPE destType) const override;

  protected:
    IntegerColumn(DATA_TYPE srcType) : TypeColumn(srcType) {}
};

class CharColumn : public IntegerColumn {
  public:
    CharColumn() : IntegerColumn(DT_CHAR) {}
    int h5size() const override { return sizeof(char); }
    DATA_TYPE packData(pack_info_t t) override;
};

class ShortColumn : public IntegerColumn {
  public:
    ShortColumn() : IntegerColumn(DT_SHORT) {}
    int h5size() const override { return sizeof(short); }
    DATA_TYPE packData(pack_info_t t) override;
};

class IntColumn : public IntegerColumn {
  public:
    IntColumn() : IntegerColumn(DT_INT) {}
    int h5size() const override { return sizeof(int); }
    DATA_TYPE packData(pack_info_t t) override;
};

class LLongColumn : public IntegerColumn {
  public:
    LLongColumn() : IntegerColumn(DT_LONG) {}
    int h5size() const override { return sizeof(long long); }
    DATA_TYPE packData(pack_info_t t) override;
};

class FloatPointNumColumn : public TypeColumn {
  public:
    bool compatible(DATA_TYPE destType) const override;

  protected:
    FloatPointNumColumn(DATA_TYPE srcType) : TypeColumn(srcType) {}
};

class FloatColumn : public FloatPointNumColumn {
  public:
    FloatColumn() : FloatPointNumColumn(DT_FLOAT) {}
    int h5size() const override { return sizeof(float); }
    DATA_TYPE packData(pack_info_t t) override;
};

class DoubleColumn : public FloatPointNumColumn {
  public:
    DoubleColumn() : FloatPointNumColumn(DT_DOUBLE) {}
    int h5size() const override { return sizeof(double); }
    DATA_TYPE packData(pack_info_t t) override;
};

class StringColumn : public TypeColumn {
  public:
    bool compatible(DATA_TYPE destType) const override;

  protected:
    VectorSP createCompatibleVector(VectorSP destVec, DATA_TYPE destType, int size, int cap) override;
    StringColumn(DATA_TYPE srcType) : TypeColumn(srcType) {}
    SymbolBaseSP symBaseSP_;
};

class FixedStringColumn : public StringColumn {
  public:
    FixedStringColumn(int size) : StringColumn(DT_STRING), size_(size) {}
    DATA_TYPE packData(pack_info_t t) override;
    int h5size() const override { return size_; }

  private:
    int size_;
};

class VariableStringColumn : public StringColumn {
  public:
    VariableStringColumn() : StringColumn(DT_STRING) {}
    int h5size() const override { return sizeof(char *); }
    DATA_TYPE packData(pack_info_t t) override;
};

class SymbolColumn : public StringColumn {
  public:
    void createEnumMap(hid_t nativeEnumId);
    VectorSP createCompatibleVector(VectorSP destVec, DATA_TYPE destType, int size, int cap) override;
    SymbolColumn(hid_t nativeEnumId) : StringColumn(DT_SYMBOL) { createEnumMap(nativeEnumId); }
    int h5size() const override { return baseSize_; }
    DATA_TYPE packData(pack_info_t t) override;

  private:
    FlatHashmap<long long, string> enumMap_;
    FlatHashmap<long long, int> symbolMap_;
    int baseSize_;
};

class UNIX64BitTimestampColumn : public TypeColumn {
  public:
    bool compatible(DATA_TYPE destType) const override;
    DATA_TYPE packData(pack_info_t t) override;
    int h5size() const override {
        assert(sizeof(long long) == 8);
        return sizeof(long long);
    }
    UNIX64BitTimestampColumn() : TypeColumn(DT_TIMESTAMP) {}
};

class BoolColumn : public TypeColumn {
  public:
    bool compatible(DATA_TYPE destType) const override;
    int h5size() const override { return sizeof(bool); }
    DATA_TYPE packData(pack_info_t t) override;
    BoolColumn() : TypeColumn(DT_BOOL) {}
};

}  // namespace H5PluginImp

#endif