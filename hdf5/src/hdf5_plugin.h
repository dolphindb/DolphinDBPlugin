#ifndef HDF5_PLUGIN_H
#define HDF5_PLUGIN_H

#include <vector>
#include <list>
#include <string>
#include <numeric>

#include <Concurrent.h>
#include <CoreConcept.h>
#include <Exceptions.h>
#include <FlatHashmap.h>
#include <ScalarImp.h>
#include <SysIO.h>
#include <Util.h>
#include<Logger.h>

#include <H5Cpp.h>
#include <hdf5_hl.h>
#include <blosc_filter.h>

extern "C" ConstantSP h5ls(const ConstantSP &h5_path);
extern "C" ConstantSP h5lsTable(const ConstantSP &filename);
extern "C" ConstantSP extractHDF5Schema(const ConstantSP &filename, const ConstantSP &datasetName);
extern "C" ConstantSP loadHDF5(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP loadPandasHDF5(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP loadHDF5Ex(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP HDF5DS(Heap *heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP saveHDF5(Heap *heap, vector<ConstantSP> &arguments);

namespace H5PluginImp
{
ConstantSP nullSP = Util::createNullConstant(DT_VOID);
void h5ls(const string &h5_path, vector<string> &objs_name, vector<string> &objs_type);
TableSP h5read(const string &h5_path, const string &dataset_name, TableSP tb);
void h5lsTable(const string &filename, vector<string> &datasetName, vector<string> &datasetDims,
               vector<string> &dataType);
TableSP extractHDF5Schema(const string &filename, const string &datasetName);
ConstantSP loadHDF5(const string &filename, const string &datasetName,
                    const ConstantSP &schema, const size_t startRow, const size_t rowNum);
ConstantSP loadHDF5Ex(Heap* heap, const SystemHandleSP &db, const string &tableName, const ConstantSP &partitionColumnNames,
                      const string &filename, const string &datasetName, const TableSP &schema,
                      const size_t startRow, const size_t rowNum, const FunctionDefSP &transform=nullSP);
ConstantSP loadPandasHDF5(const string &filename, const string &groupName,
                    const ConstantSP &schema, const size_t startRow, const size_t rowNum);
ConstantSP HDF5DS(const ConstantSP &filename, const ConstantSP &datasetName,
                  const ConstantSP &schema, const size_t dsNum);
ConstantSP saveHDF5(const TableSP &table, const string &fileName, const string &datasetName, bool append, unsigned stringMaxLength);
void appendHDF5(const TableSP &table, const string &fileName, const string &datasetName, unsigned stringMaxLength);
void writeHDF5(const TableSP &table, const string &fileName, const string &datasetName, unsigned stringMaxLength);
void extractDolphinDBSchema(const TableSP &table, size_t &type_size, char *field_names[], size_t *field_offset, hid_t *field_types, unsigned stringMaxLength);
void extractDolphinDBSchema(const TableSP &table, size_t &type_size, size_t *field_offset, size_t *field_sizes, unsigned stringMaxLength);
void extractDolphinDBData(const TableSP &table, const size_t &type_size, const size_t *field_offst, char *buf, unsigned stringMaxLength);

extern const size_t insertThreshold;

// class SegmentedInMemoryTableBuiler : public String {
// public:
// 	SegmentedInMemoryTableBuiler(Heap *heap, const SystemHandleSP& db,
//         vector<int>& partitionColumnIndices, const ConstantSP &partitionColumnNames,
//         const TableSP &schema)
//         : String("SegmentedInMemoryTableBuiler"),
//         heap_(heap),
//         db_(db),
//         partitionColumnIndices_(partitionColumnIndices),
//         partitionColumnNames_(partitionColumnNames),
//         schema_(schema) {};
// 	virtual ~SegmentedInMemoryTableBuiler(){}
// 	// void insert(TableSP& table);
// 	ConstantSP getSegmentedTable();

// private:
//     Heap *heap_;
// 	SystemHandleSP db_;
// 	vector<int> partitionColumnIndices_;
//     ConstantSP partitionColumnNames_;
//     TableSP schema_;
// 	vector<TableSP> partitions_;
// 	vector<string> segmentPaths_;
// 	vector<int> segmentKeys_;
// 	unordered_map<string, int> pathMap_;
// 	unordered_map<int, int> keyMap_;
// };

// class H5InputStream : public BlockFileInputStream {
//   public:
//     H5InputStream(const string &filename, const string &datasetName);

//   protected:
//     virtual IO_ERR internalStreamRead(char *buf, size_t length, size_t &actualLength);
//     virtual IO_ERR internalClose();
//     virtual bool internalMoveToPosition(long long offset);

//   private:

// };

class H5Object
{
  public:
    inline hid_t id() const { return id_; }
    // static char *getName(hid_t id);
    virtual void close(){};
    virtual ~H5Object() { close(); };

    hid_t accept(hid_t new_id)
    {
        close();
        return (id_ = new_id);
    }

    hid_t release()
    {
        hid_t old_id = id_;
        id_ = H5I_INVALID_HID;
        return old_id;
    }

    hid_t valid()
    {
        return H5Iis_valid(id_) > 0;
    }

  protected:
    hid_t id_ = H5I_INVALID_HID;
};

class H5ReadOnlyFile : public H5Object
{
  public:
    H5ReadOnlyFile() {};
    H5ReadOnlyFile(const std::string &filename) { open(filename); }
    H5ReadOnlyFile(const H5ReadOnlyFile &rhs) = delete;
    H5ReadOnlyFile &operator=(const H5ReadOnlyFile &rhs) = delete;
    hid_t open(const std::string &filename);
    void close() override;
    ~H5ReadOnlyFile() { close(); }
};

class H5DataSet : public H5Object
{
  public:
    H5DataSet() {}
    H5DataSet(const std::string &dset_name, hid_t loc_id) { open(dset_name, loc_id); }
    H5DataSet(const H5DataSet &rhs) = delete;
    H5DataSet &operator=(const H5DataSet &rhs) = delete;
    hid_t open(const std::string &dset_name, hid_t loc_id);
    void close() override;
    ~H5DataSet() { close(); }
};

class H5DataSpace : public H5Object
{
  public:
    H5DataSpace() {}
    H5DataSpace(hid_t dset_id) { openFromDataset(dset_id); }
    H5DataSpace(int rank, const hsize_t *current_dims, const hsize_t *maximum_dims)
    {
        create(rank, current_dims, maximum_dims);
    }

    H5DataSpace(const H5DataSpace &rhs) = delete;
    H5DataSpace &operator=(const H5DataSpace &rhs) = delete;
    hid_t openFromDataset(hid_t dset_id);
    hid_t create(int rank, const hsize_t *current_dims, const hsize_t *maximum_dims);
    int rank() const;
    int currentDims(std::vector<hsize_t> &dims) const;
    void close() override;
    ~H5DataSpace() { close(); }
};

class H5DataType : public H5Object
{
  public:
    H5DataType() {}
    H5DataType(hid_t id) { id_ = id; }
    H5DataType(const H5DataType &rhs) = delete;
    H5DataType(H5DataType &&rhs)
    {
        this->accept(rhs.id());
        rhs.release();
    }

    H5DataType &operator=(const H5DataType &rhs) = delete;
    hid_t openFromDataset(hid_t dset_id);
    size_t size() const;
    void close() override;
    ~H5DataType() { close(); }
};

class H5Property : public H5Object
{
  public:
    H5Property() {}
    H5Property(hid_t id) { id_ = id; }
    H5Property(const H5Property &rhs) = delete;
    H5Property &operator=(const H5Property &rhs) = delete;
    void close() override;
    ~H5Property() { close(); }
};
enum h5_type_flag {
    IS_FIXED_STR = 0,
    IS_VARIABLE_STR,

    IS_ENUM,
    IS_COMPOUND,
    IS_ARRAY,

    IS_S_CHAR_INTEGER,
    IS_S_SHORT_INTEGER,
    IS_S_INT_INTEGER,
    IS_S_LLONG_INTEGER,

    IS_U_CHAR_INTEGER,
    IS_U_SHORT_INTEGER,
    IS_U_INT_INTEGER,
    IS_U_LLONG_INTEGER,

    IS_FLOAT_FLOAT,
    IS_DOUBLE_FLOAT,

    IS_UNIX_TIME,

    IS_BIT
};

struct hdf5_type_layout
{
    std::string name;
    int offset;
    int belong_to;
    int flag;
    int size;
};

bool getHdf5ComplexLayout(hid_t type, std::vector<hdf5_type_layout> &layout);
bool getHdf5SimpleLayout(hid_t type, hdf5_type_layout &layout);
bool acceptNativeType(H5DataType &type, hid_t src_type);
inline bool isClassEqual(hid_t type, H5T_class_t target_class)
{
    return H5Tget_class(type) == target_class;
}

size_t getArrayElementNum(hid_t array);
const char *getHdf5NativeTypeStr(H5DataType &type);
void registerUnixTimeConvert();

class TypeColumn;

typedef SmartPointer<TypeColumn> H5ColumnSP;

struct pack_info_t
{
    char *raw_data;
    char *buffer;
    int stride;
    int len;
};

class TypeColumn
{
  public:
    TypeColumn() = delete;
    //create column from simple type
    static H5ColumnSP createNewColumn(H5DataType &srcType);

    //convert simple type
    static bool convertHdf5SimpleType(H5DataType &srcType,
                                      H5DataType &convertedType);
    static bool createCompoundColumns(H5DataType &srcType, vector<H5ColumnSP> &cols,
                                      H5DataType &convertedType);
    static bool createArrayColumns(H5DataType &srcType, vector<H5ColumnSP> &cols,
                                   H5DataType &convertedType);
    static bool createComplexColumns(H5DataType &srcType, vector<H5ColumnSP> &cols,
                                     H5DataType &convertedType);
    static bool createHdf5TypeColumns(H5DataType &srcType, vector<H5ColumnSP> &cols,
                                      H5DataType &convertedType);

    VectorSP createDolphinDBColumnVector(DATA_TYPE dt, int size, int cap);
    VectorSP createDolphinDBColumnVector(VectorSP destVec, int size, int cap);
    VectorSP createDolphinDBColumnVector(int size, int cap);

    DATA_TYPE srcType() const { return srcType_; }
    VectorSP colVec() { return colVec_; }
    int appendData(char *raw_data, int offset, int stride, int len, vector<char> &buffer);
    virtual ~TypeColumn() = default;
    virtual bool compatible(DATA_TYPE destType) const { return false; };
    virtual int h5size() const = 0;
    virtual DATA_TYPE packData(pack_info_t t) { return DT_VOID; }

  protected:
    virtual VectorSP createCompatiableVector(VectorSP destVec, DATA_TYPE destType, int size, int cap);
    int doAppend(void *data, int len, DATA_TYPE basicType);

    TypeColumn(DATA_TYPE srcType)
        : srcType_(srcType)
    {
    }

    DATA_TYPE srcType_ = DT_VOID;
    DATA_TYPE destType_ = DT_VOID;

    int srcTypeSize_ = 0;
    int destTypeSize_ = 0;

    VectorSP colVec_ = nullptr;
};

class IntegerColumn : public TypeColumn
{
  public:
    bool compatible(DATA_TYPE destType) const override;

  protected:
    IntegerColumn(DATA_TYPE srcType)
        : TypeColumn(srcType)
    {
    }
};

class CharColumn : public IntegerColumn
{
  public:
    CharColumn() : IntegerColumn(DT_CHAR) {}
    int h5size() const override { return sizeof(char); }
    DATA_TYPE packData(pack_info_t t) override;
};

class ShortColumn : public IntegerColumn
{
  public:
    ShortColumn() : IntegerColumn(DT_SHORT) {}
    int h5size() const override { return sizeof(short); }
    DATA_TYPE packData(pack_info_t t) override;
};

class IntColumn : public IntegerColumn
{
  public:
    IntColumn() : IntegerColumn(DT_INT) {}
    int h5size() const override { return sizeof(int); }
    DATA_TYPE packData(pack_info_t t) override;
};

class LLongColumn : public IntegerColumn
{
  public:
    LLongColumn() : IntegerColumn(DT_LONG) {}
    int h5size() const override { return sizeof(long long); }
    DATA_TYPE packData(pack_info_t t) override;
};

class FloatPointNumColumn : public TypeColumn
{
  public:
    bool compatible(DATA_TYPE destType) const override;

  protected:
    FloatPointNumColumn(DATA_TYPE srcType) : TypeColumn(srcType) {}
};

class FloatColumn : public FloatPointNumColumn
{
  public:
    FloatColumn() : FloatPointNumColumn(DT_FLOAT) {}
    int h5size() const override { return sizeof(float); }
    DATA_TYPE packData(pack_info_t t) override;
};

class DoubleColumn : public FloatPointNumColumn
{
  public:
    DoubleColumn() : FloatPointNumColumn(DT_DOUBLE) {}
    int h5size() const override { return sizeof(double); }
    DATA_TYPE packData(pack_info_t t) override;
};

class StringColumn : public TypeColumn
{
  public:
    bool compatible(DATA_TYPE destType) const override;

  protected:
    VectorSP createCompatiableVector(VectorSP destVec, DATA_TYPE destType, int size, int cap) override;
    StringColumn(DATA_TYPE srcType) : TypeColumn(srcType) {}
    SymbolBaseSP symBaseSP_;
};

class FixedStringColumn : public StringColumn
{
  public:
    FixedStringColumn(int size)
        : StringColumn(DT_STRING),
          size_(size) {}
    DATA_TYPE packData(pack_info_t t) override;
    int h5size() const override { return size_; }

  private:
    int size_;
};

class VariableStringColumn : public StringColumn
{
  public:
    VariableStringColumn() : StringColumn(DT_STRING) {}
    int h5size() const override { return sizeof(char *); }
    DATA_TYPE packData(pack_info_t t) override;
};

class SymbolColumn : public StringColumn
{
  public:
    void createEnumMap(hid_t nativeEnumId);
    VectorSP createCompatiableVector(VectorSP destVec, DATA_TYPE destType, int size, int cap) override;
    SymbolColumn(hid_t nativeEnumId) : StringColumn(DT_SYMBOL)
    {
        createEnumMap(nativeEnumId);
    }
    int h5size() const override { return baseSize_; }
    DATA_TYPE packData(pack_info_t t) override;

  private:
    FlatHashmap<long long, string> enumMap_;
    FlatHashmap<long long, int> symbolMap_;
    int baseSize_;
};

class UNIX64BitTimestampColumn : public TypeColumn
{
  public:
    bool compatible(DATA_TYPE destType) const override;
    DATA_TYPE packData(pack_info_t t) override;
    int h5size() const override
    {
        assert(sizeof(long long) == 8);
        return sizeof(long long);
    }
    UNIX64BitTimestampColumn() : TypeColumn(DT_TIMESTAMP) {}
};

class BoolColumn : public TypeColumn
{
  public:
    bool compatible(DATA_TYPE destType) const override;
    int h5size() const override { return sizeof(bool); }
    DATA_TYPE packData(pack_info_t t) override;
    BoolColumn() : TypeColumn(DT_BOOL) {}
};

class DatasetAppender : public Runnable
{

  public:
    void setColumns(H5ColumnSP *cols, size_t colNum, size_t eleByteLength)
    {
        cols_ = cols;
        colNum_ = colNum;
        eleByteLength_ = eleByteLength;
    }

    void updateRawData(char *rawData, size_t eleNum)
    {
        rawData_ = rawData;
        eleNum_ = eleNum;
    }

    virtual void run() override { append(); }
    virtual void append(){};

  protected:
    H5ColumnSP *cols_ = nullptr;
    size_t colNum_ = 0;
    size_t eleByteLength_ = 0;

    char *rawData_ = nullptr;
    size_t eleNum_ = 0;

    vector<char> appendBuffer_;
};

class SimpleDatasetAppender : public DatasetAppender
{
  public:
    void append() override
    {
        size_t numDataAppended = 0;
        size_t offset = 0;
        size_t stride = eleByteLength_ * colNum_;
        (void)stride;
        while (numDataAppended != eleNum_)
        {
            assert(eleNum_ > offset);
            size_t n = eleNum_ - offset;
            size_t t = n / colNum_;
            size_t dataLen = t + (size_t)(t * colNum_ < n);

            cols_[colIdx_]->appendData(rawData_, offset * eleByteLength_, stride, dataLen, appendBuffer_);

            numDataAppended += dataLen;
            colIdx_ = (colIdx_ + 1) % colNum_;
            offset++;
        }
    }

  private:
    size_t colIdx_ = 0;
};

class ComplexDatasetAppender : public DatasetAppender
{
  public:
    void append() override
    {
        size_t offset = 0;
        for (size_t i = 0; i != colNum_; i++)
        {
            cols_[i]->appendData(rawData_, offset, eleByteLength_, eleNum_, appendBuffer_);
            offset += cols_[i]->h5size();
        }
    }
};

struct vlen_mem
{
    std::vector<char> pre_alloc_buffer;
    std::list<char *> nature_alloc_ptr;
};

void freeVlenMemory(vlen_mem &vm);

class H5GeneralDataReader
{
  public:
    H5GeneralDataReader() {}
    H5GeneralDataReader(hid_t loc_id, int bufferByteLength, int vlen_str_buffer_len, hid_t mem_type_id);
    virtual void open(hid_t loc_id, int bufferByteLength, int vlen_str_buffer_len, hid_t mem_type_id);
    virtual size_t read();
    void swap_buffer(std::vector<char> &buf, vlen_mem &vm);
    size_t readbyRow(size_t offsetRow, size_t offsetCol);
    // size_t testRead(size_t offsetRow, size_t offsetCol, size_t &row_offset_after, size_t &col_offset_after);
    // size_t readbyCol();

    // template <typename T = void *>
    // inline T rawData() { return (T)buffer_.data(); };

    // inline bool reachColumnEnd() const
    // {
    //     return offsetRow_ == rowNum_ && offsetCol_ < colNum_;
    // }

    // inline bool reachRowEnd() const
    // {
    //     return offsetCol_ == colNum_ && offsetRow_ < rowNum_;
    // }

    virtual bool reachEnd() const {
      //  return reachColumnEnd() || reachRowEnd();
      return false;
    }

    // inline bool reachColumnEnd(size_t ro, size_t co) const
    // {
    //     return ro == rowNum_ && co < colNum_;
    // }

    // inline bool reachRowEnd(size_t ro, size_t co) const
    // {
    //     return co == colNum_ && ro < rowNum_;
    // }

    // size_t switchColumn(size_t col_idx);
    // size_t switchRow(size_t row_idx);
    inline size_t totalSize() const { return colNum_ * rowNum_ * elementByteLength_; }
    inline size_t columnNum() const { return colNum_; }
    inline size_t rowNum() const { return rowNum_; }
    inline size_t elementByteLength() const { return elementByteLength_; }
    inline size_t bufferByteLength() const { return buffer_.size(); }
    inline size_t preAllocVlenBufferByteLength() const { return vlenMem_.pre_alloc_buffer.capacity(); }
    inline size_t offsetRow() const { return offsetRow_; }
    inline size_t offsetCol() const { return offsetCol_; }

    virtual size_t elementNumReadOnceMax() const
    {
        return bufferByteLength() / elementByteLength();
    }

    inline size_t elementNumReadLast() const { return elementNumReadLast_; }
    ~H5GeneralDataReader() { freeVlenMemory(vlenMem_); }

  protected:
    virtual size_t prepareToRead(H5DataSpace &mem_space, H5DataSpace &file_space, size_t offsetRow, size_t offsetCol) const;
    void doRead(hid_t mem_space_id, hid_t file_space_id);
    bool discoveVlenString(hid_t type) const;
    void setXferProperty(bool has_vlen_str);

    hid_t locId_ = H5I_INVALID_HID;
    hid_t memTypeId_ = H5I_INVALID_HID;

    std::vector<char> buffer_;

    vlen_mem vlenMem_;

    bool hasVlenString_;
    H5S_class_t spaceClass_;
    H5Property xferProperty_;

    size_t offsetCol_ = 0;
    size_t offsetRow_ = 0;

    size_t colNum_ = 0;
    size_t rowNum_ = 0;

    size_t elementByteLength_ = 0;
    size_t elementNumReadLast_ = 0;
};

class H5BlockDataReader : public H5GeneralDataReader
{
  public:
    H5BlockDataReader(hid_t loc_id, size_t startElement, size_t endElement, int bufferByteLength, int vlen_str_buffer_len, hid_t mem_type_id);
    virtual void open(hid_t loc_id, int bufferByteLength, int vlen_str_buffer_len, hid_t mem_type_id);
    virtual size_t read();

  private:
    virtual bool reachEnd() const
    {
        return (offsetRow_ == endRow_ )
        // && offsetCol_ >= endCol_)
            || (offsetRow_ >= endRow_ );
            // && offsetCol_ == endCol_);
    }

    virtual size_t elementNumReadOnceMax() const
    {
        size_t startElement = offsetRow_ * colNum_ + offsetCol_;
        size_t elementNum = bufferByteLength() / elementByteLength();
        elementNum -= elementNum % colNum_;
        return std::min(elementNum, endElement_ - startElement);
    }

    size_t startElement_;
    size_t endElement_;
    size_t endRow_;
    size_t endCol_;
};
}

#endif /* HDF5_PLUGIN_H */
