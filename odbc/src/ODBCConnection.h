#include "DolphinDBODBC.h"
#include "nanodbcw/nanodbcw.h"
#include "nanodbc/nanodbc.h"
#ifdef LINUX
#include "dlfcn.h"
#endif

class ODBCBaseConnection;
typedef SmartPointer<ODBCBaseConnection> ODBCBaseConnectionSP;

class Defer {
public:
    Defer(std::function<void()> code): code(code) {}
    ~Defer() { code(); }
private:
    std::function<void()> code;
};

enum ODBCDataBaseType {
    ODBC_DBT_VOID,
    ODBC_DBT_MYSQL,
    ODBC_DBT_SQLSERVER,
    ODBC_DBT_SQLITE,
    ODBC_DBT_CLICKHOUSE,
    ODBC_DBT_ORACLE,
    ODBC_DBT_POSTGRSQL
};

typedef const char*         LPCSTR;

struct con_pair
{
    char            *keyword;
    char            *attribute;
    char            *identifier;
    struct con_pair *next;
};

struct con_struct
{
    int             count;
    struct con_pair *list;
};

static size_t INI_MAX_PROPERTY_VALUE = 1000;

extern "C" {
char *__find_lib_name( char *dsn, char *lib_name, char *driver_name );
int SQLGetPrivateProfileString( LPCSTR  pszSection,
                                LPCSTR  pszEntry,
                                LPCSTR  pszDefault,
                                LPSTR   pRetBuffer,
                                int     nRetBuffer,
                                LPCSTR  pszFileName
                              );
int __parse_connection_string( struct con_struct *con_str, char *str, int str_len );
void __release_conn( struct con_struct *con_str );
char * __get_attribute_value( struct con_struct * con_str, char * keyword );
}

class ODBCBaseConnection{
public:
    ODBCBaseConnection(ODBCDataBaseType dataBaseType)
        :dataBaseType_(dataBaseType) {}

    virtual ~ODBCBaseConnection(){}
    virtual void close(bool ignoreClosed) = 0; 

    virtual void connect(std::string connStr) = 0;
    ODBCDataBaseType getDataBaseType() { return dataBaseType_; }
    bool isClosed(){
        return closed_;
    }

    virtual ConstantSP odbcAppend(Heap* heap, TableSP t, const string& tableName, bool createTable, bool insertIgnore) = 0;
    virtual ConstantSP odbcQuery(Heap* heap, const TableSP& schemaTable, const FunctionDefSP& transform, int batchSize, const string& querySql) = 0;
    virtual void odbcExecute(const string &querySql, Heap *heap) = 0;
    static ConstantSP odbcGetConnection(Heap *heap, vector<ConstantSP> &args, const std::string &funcName);
    static bool compatible(DATA_TYPE dolphinType, int sqlType, int colSize);
    static ODBCDataBaseType getDataBaseType(const string &dataBase);
    static DATA_TYPE sqlType2DataType(int sqlType, long colSize, int cType);
    static string sqlTypeName(DATA_TYPE t, ODBCDataBaseType databaseType);
    static string getValueStr(string p, DATA_TYPE t, ODBCDataBaseType databaseType);
    
    static unordered_map<long long, ODBCBaseConnectionSP> ODBC_CONN_MAP;
    static Mutex ODBC_PLUGIN_LOCK;
    static unordered_map<string, ODBCDataBaseType> DBT_MAP;
    static Mutex CLICKHOUSE_LOCK;

protected:
    Mutex* getClickhouseLock(){
        if(dataBaseType_ == ODBC_DBT_CLICKHOUSE)
            return &CLICKHOUSE_LOCK;
        return nullptr;
    }
    ODBCDataBaseType dataBaseType_;
    bool closed_ =  true;
    Mutex lock;
};


template <typename NanConnection, typename NanTransaction, typename NanResult, typename NanTimestamp, typename NanDate, typename NanTime, typename NanStateMent, typename NanODBCFunc>
class OdbcConnection : public ODBCBaseConnection{
public:
    OdbcConnection(ODBCDataBaseType dataBaseType)
        :ODBCBaseConnection(dataBaseType){}

    virtual ~OdbcConnection(){
        close(true);
    }

    void connect(std::string connStr) override {
        LockGuard<Mutex> lock(&CLICKHOUSE_LOCK);
        nanoConn_ = new NanConnection(getODBCFunc().toString(connStr));
        std::string dbmsName;
        try {
            dbmsName = getODBCFunc().getString(nanoConn_->dbms_name());
        } catch (exception &e) {
            LOG_INFO("[PLUGIN::ODBC]: failed to get dbms name: ", e.what());
        }
        dbmsName = Util::lower(dbmsName);
        if (dbmsName == "clickhouse") {
            if (dataBaseType_ != ODBC_DBT_VOID && dataBaseType_ != ODBC_DBT_CLICKHOUSE) {
                throw RuntimeException(
                    "[PLUGIN::ODBC]: If clickhouse odbc drives, the dataBaseType must be set to clickhouse. ");
            }
            dataBaseType_ = ODBC_DBT_CLICKHOUSE;
        }
        closed_ = false;
    }
    void close(bool ignoreClosed) override{
        LockGuard<Mutex> lockClickhouse(getClickhouseLock());
        LockGuard<Mutex> lockGuard(&lock);
        if(isClosed()){
            if(ignoreClosed)return;
            else
                throw RuntimeException("[PLUGIN::ODBC]: odbc connection is closed. ");
        }
        closed_ = true;

        try{
            nanoConn_->disconnect();
        }catch(exception &e){
            LOG_ERR("[PLUGIN::ODBC]: nanodbc failed to disconnect : " + string(e.what()));
        }catch(...){
            LOG_ERR("[PLUGIN::ODBC]: nanodbc failed to disconnect. ");
        }
        try{
            nanoConn_->deallocate();
        }catch(exception &e){
            LOG_ERR("[PLUGIN::ODBC]: nanodbc failed to deallocate : " + string(e.what()));
        }catch(...){
            LOG_ERR("[PLUGIN::ODBC]: nanodbc failed to deallocate. ");
        }
        nanoConn_.clear();
    }

    bool isClosed(){
        return closed_ || !nanoConn_->connected();
    }

    static NanODBCFunc getODBCFunc(){
        return NanODBCFunc();
    }

    ConstantSP odbcAppend(Heap* heap, TableSP t, const string& tableName, bool createTable, bool insertIgnore);
    ConstantSP odbcQuery(Heap* heap, const TableSP& schemaTable, const FunctionDefSP& transform, int batchSize, const string& querySql);
    void odbcExecute(const string &querySql, Heap *heap);
    static TableSP odbcGetOrCreateTable(Heap *heap, const TableSP& schemaTable,
                                    const NanResult &results, vector<ConstantSP> &columnVecs,
                                    vector<string> &colNames, bool compatibility);
    static void getColNames(const NanResult &results, vector<std::string> &columnNames);
    static void getColVecAndType(const NanResult &results, vector<ConstantSP> &columnVecs,
                      vector<DATA_TYPE> &columnTypes);

private:
    void odbcExecute(NanTransaction &cp, const string &querySql, Heap *heap);
    SmartPointer<NanConnection> nanoConn_;

};

typedef union {
    long long longVal;
    int intVal;
    short shortVal;
    char charVal;
    double doubleVal;
    float floatVal;
    string *pointer;
    int intArray[2];
    float floatArray[2];
    short shortArray[4];
    char charArray[8];
} U8ForStringPoint;