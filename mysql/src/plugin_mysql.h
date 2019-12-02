#ifndef PLUGIN_MYSQL_H
#define PLUGIN_MYSQL_H
#include <functional>
#include <iostream>
#include <limits>
#include <thread>
#include "Concurrent.h"
#include "CoreConcept.h"
#include "LocklessContainer.h"
#include "ScalarImp.h"
#include "Util.h"
#include "mysqlxx.h"
#ifdef DEBUG
#include <chrono>
typedef std::chrono::high_resolution_clock Clock;
using std::chrono::duration_cast;
using std::chrono::nanoseconds;
#endif

extern "C" ConstantSP mysqlConnect(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP mysqlSchema(const ConstantSP &connection, const ConstantSP &table);
extern "C" ConstantSP mysqlLoad(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP mysqlTables(const ConstantSP &connection);
extern "C" ConstantSP mysqlLoadEx(Heap *heap, vector<ConstantSP> &args);

namespace dolphindb {
const char *getDolphinDBTypeStr(DATA_TYPE mysql_type);
ConstantSP messageSP(const std::string &s);
vector<ConstantSP> getArgs(vector<ConstantSP> &args, size_t nMaxArgs);

bool parseTimestamp(char *dst, const mysqlxx::Value &val, DATA_TYPE &dstDt);
bool parseNanotime(char *dst, const mysqlxx::Value &val, DATA_TYPE &dstDt);
bool parseNanoTimestamp(char *dst, const mysqlxx::Value &val, DATA_TYPE &dstDt);
bool parseDate(char *dst, const mysqlxx::Value &val, DATA_TYPE &dstDt);
bool parseMonth(char *dst, const mysqlxx::Value &val, DATA_TYPE &dstDt);
bool parseTime(char *dst, const mysqlxx::Value &val, DATA_TYPE &dstDt);
bool parseMinute(char *dst, const mysqlxx::Value &val, DATA_TYPE &dstDt);
bool parseSecond(char *dst, const mysqlxx::Value &val, DATA_TYPE &dstDt);
bool parseDatetime(char *dst, const mysqlxx::Value &val, DATA_TYPE &dstDt);

const set<DATA_TYPE> time_type{DT_TIMESTAMP, DT_NANOTIME, DT_NANOTIMESTAMP, DT_DATE, DT_MONTH, DT_TIME, DT_MINUTE, DT_SECOND, DT_DATETIME};
bool compatible(DATA_TYPE dst, DATA_TYPE src);
bool compatible(vector<DATA_TYPE> &dst, vector<DATA_TYPE> &src);

class NotImplementedException : public exception {
   public:
    NotImplementedException(const string &functionName, const string &errMsg) : functionName_(functionName), errMsg_(errMsg) {}
    virtual const char *what() const throw() { return errMsg_.c_str(); }
    virtual ~NotImplementedException() throw() {}
    const string &getFunctionName() const { return functionName_; }

   private:
    const string functionName_;
    const string errMsg_;
};

DATA_TYPE getDolphinDBType(mysqlxx::enum_field_types t, bool isUnsigned, bool isEnum, int maxStrLen);
size_t typeLen(DATA_TYPE dt);

class Connection : public mysqlxx::Connection {
   private:
    std::string host_;
    std::string user_;
    std::string password_;
    std::string db_;
    int port_ = 3306;

   public:
    Connection();
    Connection(std::string hostname, int port, std::string username, std::string password, std::string database);

    ~Connection();
    ConstantSP doQuery(const std::string &str);
    ConstantSP extractSchema(const std::string &MySQLTableName);
    ConstantSP load(const std::string &table_or_query,
                    const TableSP &schema = nullptr,
                    const uint64_t &startRow = 0,
                    const uint64_t &rowNum = std::numeric_limits<uint64_t>::max());
    ConstantSP loadEx(Heap *heap,
                      const ConstantSP &dbHandle,
                      const std::string &tableName,
                      const ConstantSP &partitionColumns,
                      const std::string &MySQLTableName_or_query,
                      const TableSP &schema = nullptr,
                      const uint64_t &startRow = 0,
                      const uint64_t &rowNum = std::numeric_limits<uint64_t>::max());
    std::string str() { return user_ + "@" + host_ + ":" + std::to_string(port_) + "/" + db_; }

   private:
    bool isQuery(std::string);
    Mutex mtx_;
};

typedef SmartPointer<Connection> ConnectionSP;

const size_t DEFAULT_PACK_SIZE = 8192;
const unsigned long long DEFAULT_ALLOWED_MEM = 8ULL * 1024 * 1024 * 1024;
const size_t DEFAULT_WORKSPACE_SIZE = 3;
using mysqlxx::Query;
class Pack;

class MySQLExtractor {
   private:
    // only used in Executor
    class Executor : public Runnable {
       public:
        explicit Executor(std::function<void()> &&f) : realWork_(f) {}

       protected:
        void run() override { realWork_(); }

       private:
        std::function<void()> realWork_;
    };

   public:
    explicit MySQLExtractor(const Query &q);
    ~MySQLExtractor();
    TableSP extractSchema(const std::string &table);
    TableSP extract(const ConstantSP &schema = nullptr);
    void extractEx(Heap *heap, TableSP &t, const ConstantSP &schema = nullptr);
    void growTable(TableSP &t, Pack &p);
    void growTableEx(TableSP &t, Pack &p, Heap *heap);

   private:
    vector<DATA_TYPE> getDstType(const TableSP &schema);
    void realExtract(mysqlxx::UseQueryResult &res, const ConstantSP &schema, TableSP &resultTable, std::function<void(Pack &pack)>);
    void realGrowTable(Pack &p, std::function<void(vector<ConstantSP> &)> &&callback);
    vector<string> getColNames(mysqlxx::ResultBase &);
    vector<DATA_TYPE> getColTypes(mysqlxx::ResultBase &);
    vector<size_t> getMaxStrlen(mysqlxx::ResultBase &);
    void workerRequest();
    void workerRelease();
    void consumerRequest();
    void consumerRelease();
    void prepareForExtract(const ConstantSP &schema, mysqlxx::ResultBase &);

   private:
    vector<std::string> colNames_;
    vector<DATA_TYPE> srcColTypes_;
    vector<DATA_TYPE> dstColTypes_;
    vector<size_t> maxStrLen_;

    Query query_;
    BlockingBoundedQueue<int, true> emptyPackIdx_, fullPackIdx_;
    vector<Pack> workspace_;
    int workingPackIdx_ = -1;
    int consumingPackIdx_ = -1;
};

class Pack {
   public:
    Pack() : rawBuffers_(0), isNull_(0), typeLen_(0), nCol_(0), size_(0), capacity_(0), srcDt_(0), dstDt_(0){};
    explicit Pack(
        vector<DATA_TYPE> srcDt, vector<DATA_TYPE> dstDt, vector<size_t> maxStrlen, TableSP &resultTable, size_t cap = DEFAULT_PACK_SIZE);
    ~Pack();
    void init(vector<DATA_TYPE> srcDt, vector<DATA_TYPE> dstDt, TableSP &t, vector<size_t> maxStrlen, size_t cap = DEFAULT_PACK_SIZE);
    void append(const mysqlxx::Row &);
    void clear();

    vector<char *> data() { return rawBuffers_; }
    vector<vector<size_t>> &nullIndexies() { return isNull_; }
    bool full() { return size_ == capacity_; }
    size_t size() const { return size_; }
    size_t nCol() const { return nCol_; }

   private:
    bool parseString(char *dst, const mysqlxx::Value &val, size_t maxLen);
    unsigned long long getRowStorage(vector<DATA_TYPE> types, vector<size_t> maxStrlen);

   private:
    vector<char *> rawBuffers_;
    vector<vector<size_t>> isNull_;
    vector<size_t> typeLen_;
    vector<size_t> maxStrLen_;
    size_t nCol_;
    size_t size_;
    size_t capacity_;
    vector<DATA_TYPE> srcDt_, dstDt_;
};

template <typename T>
inline void setter(char *dst, T &&val, DATA_TYPE &dstDt) {
    switch (dstDt) {
        case DT_BOOL:
            *((bool *)dst) = static_cast<bool>(val);
            break;
        case DT_CHAR:
            *dst = static_cast<char>(val);
            break;
        case DT_SHORT:
            *((short *)dst) = static_cast<short>(val);
            break;
        case DT_DATETIME:
        case DT_TIMESTAMP:
        case DT_NANOTIME:
        case DT_NANOTIMESTAMP:
        case DT_LONG:
            *((long long *)dst) = static_cast<long long>(val);
            break;
        case DT_DATE:
        case DT_MONTH:
        case DT_TIME:
        case DT_MINUTE:
        case DT_SECOND:
        case DT_INT:
            *((int *)dst) = static_cast<int>(val);
            break;
        case DT_FLOAT:
            *((float *)dst) = static_cast<float>(val);
            break;
        case DT_DOUBLE:
            *((double *)dst) = static_cast<double>(val);
            break;
        default:
            throw RuntimeException("Type: " + Util::getDataTypeString(dstDt) + " in setter not handled.");
    }
}

template <typename T>
inline bool parseScalar(char *dst, const mysqlxx::Value &val, DATA_TYPE &dstDt) {
    if (dstDt == DT_STRING) {
        memcpy(*((char **)dst), val.data(), val.size());
        (*((char **)dst))[val.size()] = '\0';
    } else {
        try {
            setter(dst, val.get<T>(), dstDt);
        } catch (mysqlxx::CannotParseValue &e) {
            // cannot parse, set null
            return false;
        }
    }
    return !val.empty();
}

const char *getMySQLTypeStr(mysqlxx::enum_field_types dt);

}  // namespace dolphindb
#endif