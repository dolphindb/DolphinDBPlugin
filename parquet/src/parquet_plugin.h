#ifndef PARQUET_PLUGIN_H
#define PARQUET_PLUGIN_H

#include <vector>
#include <list>
#include <string>
#include <numeric>
#include <cassert>
#include <fstream>
#include <iostream>
#include <memory>
#include <atomic>

#include <Concurrent.h>
#include <CoreConcept.h>
#include "ddbplugin/CommonInterface.h"
#include <Exceptions.h>
#include <FlatHashmap.h>
#include <ScalarImp.h>
#include <SysIO.h>
#include <Util.h>
#include <parquet/api/schema.h>
#include <parquet/api/reader.h>
#include <parquet/api/writer.h>
#include <arrow/io/file.h>
#include "arrow/util/decimal.h"
#include "arrow/result.h"
#include "ddbplugin/Plugin.h"

using argsT = std::vector<ddb::ConstantSP>;

extern "C" ddb::ConstantSP extractParquetSchema(ddb::Heap *heap, argsT &arguments);
extern "C" ddb::ConstantSP loadParquet(ddb::Heap *heap, argsT &arguments);
extern "C" ddb::ConstantSP loadParquetHdfs(ddb::Heap *heap, argsT &arguments);
extern "C" ddb::ConstantSP loadParquetEx(ddb::Heap *heap, argsT &arguments);
extern "C" ddb::ConstantSP parquetDS(ddb::Heap *heap, argsT& arguments);
extern "C" ddb::ConstantSP saveParquet(ddb::Heap *heap, argsT& arguments);
extern "C" ddb::ConstantSP saveParquetHdfs(ddb::Heap *heap, argsT& arguments);
extern "C" ddb::ConstantSP setReadThreadNum(ddb::Heap *heap, argsT& arguments);
extern "C" ddb::ConstantSP getReadThreadNum(ddb::Heap *heap, argsT& arguments);

using namespace ddb;

namespace ParquetPluginImp
{

TableSP extractParquetSchema(const string &filename);
extern ConstantSP nullSP;

class ParquetReadOnlyFile
{
  public:
    ParquetReadOnlyFile() {};
    ParquetReadOnlyFile(const std::string &filename) { open(filename); }
    ParquetReadOnlyFile(const ParquetReadOnlyFile &rhs) = delete;
    ParquetReadOnlyFile &operator=(const ParquetReadOnlyFile &rhs) = delete;
    void open(const std::string &file_name);
    void open(std::shared_ptr<arrow::io::RandomAccessFile> source);
    void close();
    std::shared_ptr<parquet::FileMetaData> fileMetadataReader() {
		return (_parquet_reader.get() == nullptr) ? nullptr : _parquet_reader->metadata();
	}
    std::shared_ptr<parquet::RowGroupReader> rowReader(int i);
    ~ParquetReadOnlyFile() { close(); }
   
  private:
    std::unique_ptr<parquet::ParquetFileReader>  _parquet_reader;

};
enum parquetTime{
  TimeMillis=0,
  TimeMicros,
  TimeNanos,
  Date,
  TimestampMillis,
  TimestampMicros,
  TimestampNanos,
  None
};
unordered_set<string> COMPRESSION_SET = {"snappy", "gzip", "zstd"};
std::string getDafaultColumnType(parquet::Type::type physical_t);
std::string getLayoutColumnType(std::shared_ptr<const  parquet::LogicalType>& logical_t,parquet::Type::type physical_t,parquet::SortOrder::type sort_order);
std::string getLayoutColumnType(parquet::ConvertedType::type converted_t,parquet::Type::type physical_t,parquet::SortOrder::type sort_order);
bool getSchemaCol(const parquet::SchemaDescriptor *schema_descr,const ConstantSP &col_idx,vector<ConstantSP> &dolpindbCol);
void createNewVectorSP(vector<VectorSP> &dolpindb_v,const TableSP &tb);

struct IndexArgs{
  vector<INDEX> index_;
  bool isArray_ = false;
  INDEX lastIndex_ = 0; // [left, right) last element right range
  INDEX cumSum_ = 0; // element count
};

void getIndex(vector<short> rep_level, int readCount, IndexArgs& indexArgs);

TableSP appendColumnVecToTable(TableSP tb, vector<VectorSP> &colVec);

parquetTime convertParquetTimes(const std::shared_ptr<const parquet::LogicalType>& logical_t, parquet::ConvertedType::type converted_t);

int convertParquetToDolphindbChar(int col_idx,std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr, char* buffer, size_t batchSize, bool& containNull, IndexArgs& indexArgs);

int convertParquetToDolphindbBool(int col_idx,std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr, char* buffer, size_t batchSize, bool& containNull, IndexArgs& indexArgs);

int convertParquetToDolphindbInt(int col_idx,std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr, int* buffer,DATA_TYPE times_t, size_t batchSize, bool& containNull, IndexArgs& indexArgs);

int convertParquetToDolphindbShort(int col_idx,std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr,short* buffer, size_t batchSize, bool& containNull, IndexArgs& indexArgs);

int convertParquetToDolphindbLong(int col_idx,std::shared_ptr<parquet::ColumnReader> column_reader, parquet::ColumnDescriptor *col_descr,long long* buffer,DATA_TYPE times_t, size_t batchSize, bool& containNull, IndexArgs& indexArgs);

int convertParquetToDolphindbFloat(int col_idx,std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr,float* buffer, size_t batchSize, bool& containNull, IndexArgs& indexArgs);

int convertParquetToDolphindbDouble(int col_idx,std::shared_ptr<parquet::ColumnReader> column_reader,const parquet::ColumnDescriptor *col_descr,double *buffer, size_t batchSize, bool& containNull, IndexArgs& indexArgs);

int convertParquetToDolphindbString(int col_idx,std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr,vector<string> &buffer, DATA_TYPE string_t, size_t batchSize, bool& containNull, IndexArgs& indexArgs);

ConstantSP loadParquetByFileName(Heap *heap, const string &filename, const ConstantSP &schema, const ConstantSP &column, const int rowGroupStart, const int rowGroupNum);

ConstantSP loadParquetByFilePtr(ParquetReadOnlyFile *file, Heap *heap, string fileName, const ConstantSP &schema, const ConstantSP &column, const int rowGroupStart, const int rowGroupEnd);

ConstantSP loadParquetHdfs(void *buffer, int64_t len);

ConstantSP parquetDS(const ConstantSP &filename,const ConstantSP &schema);

ConstantSP saveParquet(ConstantSP &tb, const string &filename, const string &compression);

ConstantSP saveParquetHdfs(ConstantSP &table);

std::shared_ptr<parquet::schema::GroupNode> getParquetSchemaFromDolphindb(const TableSP &table);

//ConstantSP loadFromParquetToDatabase(Heap *heap, vector<ConstantSP> &arguments);
vector<DistributedCallSP> generateParquetTasks(Heap* heap, ParquetReadOnlyFile *file, const TableSP &schema, const ConstantSP &column,
                                          const int rowGroupStart, const int rowGroupNum, DBHandleWrapper &db, const string &tableName, const ConstantSP &transform, const string& fileName);

TableSP generateInMemoryParitionedTable(Heap *heap, DBHandleWrapper &db,
                                        const ConstantSP &tables, const ConstantSP &partitionNames);

ConstantSP loadParquetEx(Heap *heap, DBHandleWrapper &db, const string &tableName, const ConstantSP &partitionColumns,
                         const string &filename, const TableSP &schema, const ConstantSP &column, const int rowGroupStart, const int rowGroupNum, const ConstantSP &transform);

int getRowGroup(const string &filename);

void getParquetReadOnlyFile(Heap *heap, vector<ConstantSP> &arguments);

inline bool isDecimal(const parquet::ColumnDescriptor *col_descr){
  if (!col_descr)
	return false;
  return col_descr->logical_type()->is_decimal() || (col_descr->converted_type()==parquet::ConvertedType::DECIMAL);
}

ConstantSP loadParquetColumn(ParquetReadOnlyFile *file, int batchRow, const VectorSP &dolphindbCol, int row,
                             int dolphinIndex, int arrowIndex, int offsetStart, int& totalRows, ConstantSP& indexCol);

ConstantSP loadParquetPloopFunc(Heap *heap, vector<ConstantSP> &arguments);

class ParquetPloopArgs : public Bool{
public:
    ParquetPloopArgs(const string &fileName, int batchRow, const vector<VectorSP> &dolphindbColVec, int rowGroupStart, const vector<int> &arrowIndexVec, int rowGroupEnd, const vector<ConstantSP> &indexDolphindbCol):
        fileName_(fileName), batchRow_(batchRow), dolphindbColVec_(dolphindbColVec), rowGroupStart_(rowGroupStart), arrowIndexVec_(arrowIndexVec), rowGroupEnd_(rowGroupEnd), indexDolphindbCol_(indexDolphindbCol){}
    
    virtual ~ParquetPloopArgs() {}
    std::string getFileName() const { return fileName_; }
    int getBatchRow() const { return batchRow_; }
    const vector<VectorSP>& getDolphindbColVec() const { return dolphindbColVec_; }
    int getRowGroupStart() const { return rowGroupStart_; }
    const vector<int>& getArrowIndexVec() const { return arrowIndexVec_; }
    int getRowGroupEnd() const { return rowGroupEnd_; }
    vector<ConstantSP>& getDolphindbIndexColVec() { return indexDolphindbCol_; }

    std::string fileName_;
    int batchRow_;
    vector<VectorSP> dolphindbColVec_;
    int rowGroupStart_;
    vector<int> arrowIndexVec_;
    int rowGroupEnd_;
    vector<ConstantSP> indexDolphindbCol_;
};

}


#endif