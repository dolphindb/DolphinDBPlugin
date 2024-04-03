#ifndef ORC_PLUGIN_H
#define ORC_PLUGIN_H

#include <vector>
#include <string>
#include <memory>
#include <sstream>

#include <CoreConcept.h>
#include "ddbplugin/CommonInterface.h"
#include <ScalarImp.h>
#include <Util.h>
#include <orc/OrcFile.hh>
extern "C" ConstantSP extractORCSchema(const ConstantSP &filename);
extern "C" ConstantSP loadORC(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP loadORCEx(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP orcDS(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP loadORCHdfs(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP saveORC(Heap *heap, vector<ConstantSP> &arguments);

namespace ORCPluginImp
{

TableSP extractORCSchema(const string &filename);
extern ConstantSP nullSP;
extern const string STR_MIN;
static const string ORC_PREFIX = "[PLUGIN::ORC] ";

std::string getColumnType(const orc::Type *subtype);
bool getSchemaCol(const orc::Type &schema_descr, const ConstantSP &col_idx, vector<ConstantSP> &dolphindbCol);
void createNewVectorSP(vector<VectorSP> &dolphindb_v, const TableSP &tb);

TableSP appendColumnVecToTable(TableSP tb, vector<VectorSP> &colVec);
bool convertToDTnanotime(orc::TypeKind type, vector<long long> &buffer);
bool convertToDTnanotimestamp(orc::TypeKind type, vector<long long> &buffer);
bool convertToDTtimestamp(orc::TypeKind type, vector<long long> &buffer);
bool convertToDTdate(orc::TypeKind type, vector<int> &buffer);
bool convertToDTmonth(orc::TypeKind type, vector<int> &buffer);
bool convertToDTtime(orc::TypeKind type, vector<int> &buffer);
bool convertToDTsecond(orc::TypeKind type, vector<int> &buffer);
bool convertToDTminute(orc::TypeKind type, vector<int> &buffer);
bool convertToDTdatetime(orc::TypeKind type, vector<int> &buffer);
bool parsePartialDate(const string &str, bool containDelimitor, int& part1, int& part2);
bool dateParser(const string &str, int &intVal);
bool monthParser(const string &str, int &intVal);
bool timeParser(const string &str, int &intVal);
bool secondParser(const string &str, int &intVal);
bool minuteParser(const string &str, int &intVal);
bool datetimeParser(const string &str, int &intVal);
bool nanotimeParser(const string &str, long long &longVal);
bool nanotimestampParser(const string &str, long long &longVal);
bool timestampParser(const string &str, long long &longVal);
bool convertORCToDolphindbBool(int col_idx, orc::StructVectorBatch *root, orc::TypeKind type, vector<char> &buffer);
bool convertORCToDolphindbChar(int col_idx, orc::StructVectorBatch *root, orc::TypeKind type, vector<char> &buffer);
bool convertORCToDolphindbInt(int col_idx, orc::StructVectorBatch *root, orc::TypeKind type, vector<int> &buffer, DATA_TYPE times_t);
bool convertORCToDolphindbLong(int col_idx, orc::StructVectorBatch *root, orc::TypeKind type, vector<long long> &buffer, DATA_TYPE times_t);
bool convertORCToDolphindbShort(int col_idx, orc::StructVectorBatch *root, orc::TypeKind type, vector<short> &buffer);
bool convertORCToDolphindbFloat(int col_idx, orc::StructVectorBatch *root, orc::TypeKind type, vector<float> &buffer);
bool convertORCToDolphindbDouble(int col_idx, orc::StructVectorBatch *root, orc::TypeKind type, vector<double> &buffer);
bool convertORCToDolphindbString(int col_idx, orc::StructVectorBatch *root, orc::TypeKind type, vector<string> &buffer, DATA_TYPE string_t);

ConstantSP loadORC(const string &filename, const ConstantSP &schema, const ConstantSP &column, int rowStart, int rowNum);
ConstantSP loadORC(orc::Reader *reader, const ConstantSP &schema, const ConstantSP &column, int rowStart, int rowNum);
ConstantSP loadORCFromBuf(void *buffer, uint64_t length);
ConstantSP orcDS(const ConstantSP &filename, int chunkSize, const ConstantSP &schema, int skipRows);
ConstantSP saveORC(const TableSP &table, const string &fileName);
int getRowNum(const string &filename);
void getORCReader(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP loadFromORCToDatabase(Heap *heap, vector<ConstantSP> &arguments);
vector<DistributedCallSP> generateORCTasks(Heap* heap, const orc::Reader *reader, const TableSP &schema, const ConstantSP &column, const int rowStart, const int rowNum,
                                          const SystemHandleSP &db, const string &tableName, const ConstantSP &transform);

TableSP generateInMemoryPartitionedTable(Heap *heap, const SystemHandleSP &db,
                                        const ConstantSP &tables, const ConstantSP &partitionNames);

ConstantSP generatePartition(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP loadORCEx(Heap *heap, const SystemHandleSP &db, const string &tableName, const ConstantSP &partitionColumns,
                         const string &filename, const TableSP &schema, const ConstantSP &column, const int rowStart, const int rowNum, const ConstantSP &transform);
string getORCSchema(const TableSP &table);
}

#endif