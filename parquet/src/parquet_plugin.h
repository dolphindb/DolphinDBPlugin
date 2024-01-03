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
extern "C" ConstantSP extractParquetSchema(const ConstantSP &filename);
extern "C" ConstantSP loadParquet(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP loadParquetHdfs(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP loadParquetEx(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP parquetDS(Heap *heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP saveParquet(Heap *heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP saveParquetHdfs(Heap *heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP setReadThreadNum(Heap *heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP getReadThreadNum(Heap *heap, vector<ConstantSP>& arguments);

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
std::string getDafaultColumnType(parquet::Type::type physical_t);
std::string getLayoutColumnType(std::shared_ptr<const  parquet::LogicalType>& logical_t,parquet::Type::type physical_t,parquet::SortOrder::type sort_order);
std::string getLayoutColumnType(parquet::ConvertedType::type converted_t,parquet::Type::type physical_t,parquet::SortOrder::type sort_order);
bool getSchemaCol(const parquet::SchemaDescriptor *schema_descr,const ConstantSP &col_idx,vector<ConstantSP> &dolpindbCol);
void createNewVectorSP(vector<VectorSP> &dolpindb_v,const TableSP &tb);

TableSP appendColumnVecToTable(TableSP tb, vector<VectorSP> &colVec);

parquetTime convertParquetTimes(const std::shared_ptr<const parquet::LogicalType>& logical_t, parquet::ConvertedType::type converted_t);

int convertParquetToDolphindbChar(int col_idx,std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr, char* buffer, size_t batchSize, bool& containNull);

int convertParquetToDolphindbBool(int col_idx,std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr, char* buffer, size_t batchSize, bool& containNull);

int convertParquetToDolphindbInt(int col_idx,std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr, int* buffer,DATA_TYPE times_t, size_t batchSize, bool& containNull);

int convertParquetToDolphindbShort(int col_idx,std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr,short* buffer, size_t batchSize, bool& containNull);

int convertParquetToDolphindbLong(int col_idx,std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr,long long* buffer,DATA_TYPE times_t, size_t batchSize, bool& containNull);

int convertParquetToDolphindbFloat(int col_idx,std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr,float* buffer, size_t batchSize, bool& containNull);

int convertParquetToDolphindbDouble(int col_idx,std::shared_ptr<parquet::ColumnReader> column_reader,const parquet::ColumnDescriptor *col_descr,double *buffer, size_t batchSize, bool& containNull);

int convertParquetToDolphindbString(int col_idx,std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr,vector<string> &buffer, DATA_TYPE string_t, size_t batchSize, bool& containNull);

ConstantSP loadParquetByFileName(Heap *heap, const string &filename, const ConstantSP &schema, const ConstantSP &column, const int rowGroupStart, const int rowGroupNum);

ConstantSP loadParquetByFilePtr(ParquetReadOnlyFile *file, Heap *heap, string fileName, const ConstantSP &schema, const ConstantSP &column, const int rowGroupStart, const int rowGroupEnd);

ConstantSP loadParquetHdfs(void *buffer, int64_t len);

ConstantSP parquetDS(const ConstantSP &filename,const ConstantSP &schema);

ConstantSP saveParquet(ConstantSP &tb, const string &filename);

ConstantSP saveParquetHdfs(ConstantSP &table);

std::shared_ptr<parquet::schema::GroupNode> getParquetSchemaFromDolphindb(const TableSP &table);

void writeBoolToParquet(parquet::BoolWriter *parquetCol, const VectorSP &dolphinCol);

void writeIntToParquet(parquet::Int32Writer *parquetCol, const VectorSP &dolphinCol, int (*transform)(int) = [](int v){return v;});

void writeLongToParquet(parquet::Int64Writer *parquetCol, const VectorSP &dolphinCol, long long (*transform)(long long) = [](long long v){return v;});

void writeFloatToParquet(parquet::FloatWriter *parquetCol, const VectorSP &dolphinCol);

void writeDoubleToParquet(parquet::DoubleWriter *parquetCol, const VectorSP &dolphinCol);

// void writeCharToParquet(parquet::FixedLenByteArrayWriter *parquetCol, const VectorSP &dolphinCol);

void writeStringToParquet(parquet::ByteArrayWriter *parquetCol, const VectorSP &dolphinCol);

//ConstantSP loadFromParquetToDatabase(Heap *heap, vector<ConstantSP> &arguments);
vector<DistributedCallSP> generateParquetTasks(Heap* heap, ParquetReadOnlyFile *file, const TableSP &schema, const ConstantSP &column,
                                          const int rowGroupStart, const int rowGroupNum, const SystemHandleSP &db, const string &tableName, const ConstantSP &transform, const string& fileName);

TableSP generateInMemoryParitionedTable(Heap *heap, const SystemHandleSP &db,
                                        const ConstantSP &tables, const ConstantSP &partitionNames);

ConstantSP loadParquetEx(Heap *heap, const SystemHandleSP &db, const string &tableName, const ConstantSP &partitionColumns,
                         const string &filename, const TableSP &schema, const ConstantSP &column, const int rowGroupStart, const int rowGroupNum, const ConstantSP &transform);

int getRowGroup(const string &filename);

void getParquetReadOnlyFile(Heap *heap, vector<ConstantSP> &arguments);

inline bool isDecimal(const parquet::ColumnDescriptor *col_descr){
  if (!col_descr)
	return false;
  return col_descr->logical_type()->is_decimal() || (col_descr->converted_type()==parquet::ConvertedType::DECIMAL);
}

ConstantSP loadParquetColumn(ParquetReadOnlyFile *file, int batchRow, const VectorSP &dolphindbCol, int row,
                             int dolphinIndex, int arrowIndex, int offsetStart, int& totalRows);

ConstantSP loadParquetPloopFunc(Heap *heap, vector<ConstantSP> &arguments);
}

#endif