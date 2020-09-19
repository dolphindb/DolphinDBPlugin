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
extern "C" ConstantSP extractParquetSchema(const ConstantSP &filename);
extern "C" ConstantSP loadParquet(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP loadParquetEx(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP parquetDS(Heap *heap, vector<ConstantSP>& arguments);

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
    void close();
    std::shared_ptr<parquet::FileMetaData> fileMetadataReader(){return _parquet_reader->metadata();}
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
bool convertToDTdate(vector<int> &intValue, parquetTime times_t,vector<long long> &longValue);
bool convertToDTminute(vector<int> &intValue, parquetTime times_t,vector<long long> &longValue);
bool convertToDTmonth(vector<int> &intValue, parquetTime times_t,vector<long long> &longValue);
bool convertToDTdatetime(vector<int> &intValue, parquetTime times_t,vector<long long> &longValue);
bool convertToDTtime(vector<int> &intValue, parquetTime times_t,vector<long long> &longValue);
bool convertToDTtimestamp(vector<int> &intValue, parquetTime times_t,vector<long long> &longValue);
bool convertToDTsecond(vector<int> &intValue, parquetTime times_t,vector<long long> &longValue);
bool convertToDTnanotimestamp(vector<int> &intValue, parquetTime times_t,vector<long long> &longValue);
bool convertToDTnanotime(vector<int> &intValue, parquetTime times_t,vector<long long> &longValue);
parquetTime convertParquetTimes(const std::shared_ptr<const parquet::LogicalType>& logical_t, parquet::ConvertedType::type converted_t);
bool convertParquetToDolphindbChar(int col_idx,std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr,vector<char> &buffer);
bool convertParquetToDolphindbBool(int col_idx,std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr,vector<char> &buffer);
bool convertParquetToDolphindbInt(int col_idx,std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr,vector<int> &buffer,DATA_TYPE times_t);
bool convertParquetToDolphindbShort(int col_idx,std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr,vector<short> &buffer);
bool convertParquetToDolphindbLong(int col_idx,std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr,vector<long long> &buffer,DATA_TYPE times_t);
bool convertParquetToDolphindbFloat(int col_idx,std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr,vector<float> &buffer);
bool convertParquetToDolphindbDouble(int col_idx,std::shared_ptr<parquet::ColumnReader> column_reader,const parquet::ColumnDescriptor *col_descr,vector<double> &buffer );
bool convertParquetToDolphindbString(int col_idx,std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr,vector<string> &buffer, DATA_TYPE string_t);
ConstantSP loadParquet(const string &filename, const ConstantSP &schema, const ConstantSP &column, const int rowGroupStart, const int rowGroupNum);
ConstantSP loadParquet(ParquetReadOnlyFile*file, const ConstantSP &schema, const ConstantSP &column, const int rowGroupStart, const int rowGroupEnd);
ConstantSP parquetDS(const ConstantSP &filename,const ConstantSP &schema);
//ConstantSP loadFromParquetToDatabase(Heap *heap, vector<ConstantSP> &arguments);
vector<DistributedCallSP> generateParquetTasks(Heap* heap, ParquetReadOnlyFile *file, const TableSP &schema, const ConstantSP &column,
                                          const int rowGroupStart, const int rowGroupNum, const SystemHandleSP &db, const string &tableName, const ConstantSP &transform);

TableSP generateInMemoryParitionedTable(Heap *heap, const SystemHandleSP &db,
                                        const ConstantSP &tables, const ConstantSP &partitionNames);

ConstantSP loadParquetEx(Heap *heap, const SystemHandleSP &db, const string &tableName, const ConstantSP &partitionColumns,
                         const string &filename, const TableSP &schema, const ConstantSP &column, const int rowGroupStart, const int rowGroupNum, const ConstantSP &transform);
int getRowGroup(const string &filename);
void getParquetReadOnlyFile(Heap *heap, vector<ConstantSP> &arguments);
bool isDecimal(const parquet::ColumnDescriptor *col_descr);
}

#endif