#ifndef PLUGIN_DUCKDB_H
#define PLUGIN_DUCKDB_H

#include <ddbplugin/pluginVersion.h>

#include <functional>
#include <iostream>
#include <limits>
#include <thread>
#include <memory>
#include <string>

#include "Concurrent.h"
#include "CoreConcept.h"
#include "ddbplugin/CommonInterface.h"
#include "LocklessContainer.h"
#include "ScalarImp.h"
#include "Util_wrapper.h"
#include "ddbplugin/PluginLoggerImp.h"

#ifdef DEBUG
#include <chrono>
typedef std::chrono::high_resolution_clock Clock;
using std::chrono::duration_cast;
using std::chrono::nanoseconds;
#endif

// DuckDB includes
#include "duckdb.hpp"

extern "C" ConstantSP duckdbConnect(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP duckdbClose(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP duckdbSchema(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP duckdbLoad(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP duckdbTables(Heap *heap, vector<ConstantSP> &args);

namespace dolphindb {

bool littleEndian = false;
const char *getDolphinDBTypeStr(DATA_TYPE dt);
ConstantSP messageSP(const std::string &s);
vector<ConstantSP> getArgs(vector<ConstantSP> &args, size_t nMaxArgs);

// Type conversion functions
bool parseTimestamp(char *dst, const duckdb::Value &val, DATA_TYPE &dstDt, char *nullVal, size_t len);
bool parseDate(char *dst, const duckdb::Value &val, DATA_TYPE &dstDt, char *nullVal, size_t len);
bool parseTime(char *dst, const duckdb::Value &val, DATA_TYPE &dstDt, char *nullVal, size_t len);
bool parseDecimal(char *dst, const duckdb::Value &val, DATA_TYPE &dstDt, char *nullVal, size_t len);

const set<DATA_TYPE> time_type{DT_TIMESTAMP, DT_NANOTIMESTAMP, DT_DATE, DT_TIME, DT_DATETIME};

bool compatible(DATA_TYPE dst, DATA_TYPE src);
void compatible(vector<DATA_TYPE> &dst, vector<DATA_TYPE> &src);

// Convert DuckDB type to DolphinDB type
DATA_TYPE getDolphinDBType(duckdb::LogicalType duckdb_type);
size_t typeLen(DATA_TYPE dt);

class Connection {
private:
    std::string dbpath_;
    std::unique_ptr<duckdb::DuckDB> database_;
    std::unique_ptr<duckdb::Connection> connection_;
    bool isClosed_;

public:
    Connection(std::string dbpath);
    ~Connection();

    ConstantSP doQuery(const std::string &str);
    ConstantSP extractSchema(const std::string &tableNameOrQuery);
    ConstantSP load(const std::string &table_or_query, const TableSP &schema = nullptr,
                    const uint64_t &startRow = 0,
                    const uint64_t &rowNum = std::numeric_limits<uint64_t>::max(),
                    const bool &allowEmptyTable = false);
    ConstantSP tables();

    std::string str() { return dbpath_; }
    void close();
    bool isClosed() const { return isClosed_; }
    bool connected() { return database_ != nullptr && connection_ != nullptr; }

private:
    bool isQuery(const std::string &str);
    ConstantSP createTableFromQueryResult(duckdb::unique_ptr<duckdb::MaterializedQueryResult> &result);
    ConstantSP convertDuckDBValueToDolphinDB(const duckdb::Value &value, DATA_TYPE target_type);
};

} // namespace dolphindb

#endif // PLUGIN_DUCKDB_H
