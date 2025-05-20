#ifndef HDF5_PLUGIN_IMP_H
#define HDF5_PLUGIN_IMP_H
#include <CoreConcept.h>
#include <Util.h>
#include <ScalarImp.h>
#include <Logger.h>
#include <hdf5_plugin_util.h>
#include <hdf5_plugin_obj.h>
#include <hdf5_plugin_type.h>

#include <H5Cpp.h>
#include <hdf5_hl.h>
#include <blosc_filter.h>
#include <list>
#include "ddbplugin/PluginLogger.h"

namespace H5PluginImp
{
struct hdf5_type_layout;
class H5DataType;
class TypeColumn;
using H5ColumnSP = SmartPointer<TypeColumn>;

static ConstantSP nullSP = Util::createNullConstant(DT_VOID);
extern const size_t insertThreshold;

class DatasetAppendRunner : public Runnable
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

    virtual void run() override {
      try {
        append();
      } catch (...) {
        PLUGIN_LOG_ERR(HDF5_LOG_PREFIX + "Error occurred when append data.");
      }
    }
    virtual void append(){};

  protected:
    H5ColumnSP *cols_ = nullptr;
    size_t colNum_ = 0;
    size_t eleByteLength_ = 0;

    char *rawData_ = nullptr;
    size_t eleNum_ = 0;

    vector<char> appendBuffer_;
};

class SimpleDatasetAppendRunner : public DatasetAppendRunner
{
  public:
    void append() override
    {
      // HACK weird code but work, keep it in case of larger trouble.
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

class ComplexDatasetAppendRunner : public DatasetAppendRunner
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
    size_t readByRow(size_t offsetRow, size_t offsetCol);
    virtual bool reachEnd() const {
      //  return reachColumnEnd() || reachRowEnd();
      return false;
    }
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

    /* unused function
      template <typename T = void *>
      inline T rawData() { return (T)buffer_.data(); };

      inline bool reachColumnEnd() const
      {
          return offsetRow_ == rowNum_ && offsetCol_ < colNum_;
      }

      inline bool reachRowEnd() const
      {
          return offsetCol_ == colNum_ && offsetRow_ < rowNum_;
      }


      inline bool reachColumnEnd(size_t ro, size_t co) const
      {
          return ro == rowNum_ && co < colNum_;
      }

      inline bool reachRowEnd(size_t ro, size_t co) const
      {
          return co == colNum_ && ro < rowNum_;
      }

      size_t testRead(size_t offsetRow, size_t offsetCol, size_t &row_offset_after, size_t &col_offset_after);
      size_t readByCol();
      size_t switchColumn(size_t col_idx);
      size_t switchRow(size_t row_idx);
    */

  protected:
    virtual size_t prepareToRead(H5DataSpace &mem_space, H5DataSpace &file_space, size_t offsetRow, size_t offsetCol) const;
    void doRead(hid_t mem_space_id, hid_t file_space_id);
    bool discoverVlenString(hid_t type) const;
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

typedef SmartPointer<DatasetAppendRunner> DatasetAppendRunnerSP;

void h5ls(const string &h5_path, vector<string> &objNames, vector<string> &objTypes);
TableSP h5read(const string &h5_path, const string &dataset_name, TableSP tb);
void h5lsTable(const string &filename, vector<string> &datasetName, vector<string> &datasetDims, vector<string> &dataType);
TableSP extractHDF5Schema(const string &filename, const string &datasetName);
ConstantSP loadHDF5(const string &filename, const string &datasetName, const ConstantSP &schema, size_t startRow, size_t rowNum);
ConstantSP loadHDF5Ex(Heap* heap, const SystemHandleSP &db, const string &tableName, const ConstantSP &partitionColumnNames, const string &filename,
                      const string &datasetName, const TableSP &schema,size_t startRow, size_t rowNum, const FunctionDefSP &transform=nullSP);
ConstantSP HDF5DS(const ConstantSP &filename, const ConstantSP &datasetName, const ConstantSP &schema, size_t dsNum);
ConstantSP saveHDF5(const TableSP &table, const string &fileName, const string &datasetName, bool append, unsigned stringMaxLength);
void appendHDF5(const TableSP &table, const string &fileName, const string &datasetName, unsigned stringMaxLength);
void writeHDF5(const TableSP &table, const string &fileName, const string &datasetName, unsigned stringMaxLength);
void extractDolphinDBSchema(const TableSP &table, size_t &type_size, char *field_names[], size_t *field_offset, hid_t *field_types, unsigned stringMaxLength);
void extractDolphinDBSchema(const TableSP &table, size_t &type_size, size_t *field_offset, size_t *field_sizes, unsigned stringMaxLength);
void extractDolphinDBData(const TableSP &table, const size_t &type_size, const size_t *field_offset, char *buf, unsigned stringMaxLength);

bool getHdf5ComplexLayout(hid_t type, std::vector<hdf5_type_layout> &layout);
bool getHdf5SimpleLayout(hid_t type, hdf5_type_layout &layout);
size_t getArrayElementNum(hid_t array);
const char *getHdf5NativeTypeStr(H5DataType &type);


TableSP readSimpleDataset(const hid_t set, H5DataType &type, const TableSP& tb, size_t startRow, size_t readRowNum, GroupInfo &groupInfo);
TableSP readComplexDataset(const hid_t set, H5DataType &type, const TableSP& tb, size_t startRow, size_t readRowNum, GroupInfo &groupInfo);

void getGroupAttribute(const H5::Group& group, const string& attribute, long long* value);
void getGroupAttribute(const H5::Group& group, const string& attribute, string& value);
void getDataSetAttribute(const H5::DataSet& dataset, const string& attribute, string& value);
void getRowAndColNum(const hid_t set, vector<size_t> &rowAndColNum);
bool createColumnVec(vector<H5ColumnSP> &cols, size_t num, size_t cap, H5DataType &src, const TableSP& dest);
bool createColumnVec(vector<H5ColumnSP> &cols, size_t cap, const TableSP& dest);
void doReadDataset_concurrent(H5GeneralDataReader &reader, const DatasetAppendRunnerSP& appendRunner, vector<H5ColumnSP> &cols, vector<ConstantSP> &colVec);
TableSP appendColumnVecToTable(const TableSP& tb, vector<ConstantSP> &colVec);
void generateComplexTypeBasedColsNameAndColsType(vector<string> &colsName, vector<string> &colsType, hid_t type);
bool getMemoryLayoutInArrayType(hid_t type, std::vector<hdf5_type_layout> &layout);
void _getMemoryLayoutInArrayType(hid_t type, std::vector<hdf5_type_layout> &layout, int belong_to);

}
#endif