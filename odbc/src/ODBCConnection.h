#include "DolphinDBODBC.h"
#include "nanodbcw/nanodbcw.h"
#include "nanodbc/nanodbc.h"
#include "ddbplugin/PluginLogger.h"
#ifdef LINUX
#include "dlfcn.h"
#endif

class ODBCBaseConnection;
typedef SmartPointer<ODBCBaseConnection> ODBCBaseConnectionSP;
extern std::string PLUGIN_ODBC_STRING_PREFIX;


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
    ODBC_DBT_SQL_SERVER,
    ODBC_DBT_SQLITE,
    ODBC_DBT_CLICK_HOUSE,
    ODBC_DBT_ORACLE,
    ODBC_DBT_POST_GRE_SQL
};

inline std::string DataBaseToString(const ODBCDataBaseType cat) {
    switch (cat) {
        case ODBC_DBT_VOID: return "";
        case ODBC_DBT_MYSQL: return "MySQL";
        case ODBC_DBT_SQL_SERVER: return "SQL Server";
        case ODBC_DBT_SQLITE: return "SQLite";
        case ODBC_DBT_CLICK_HOUSE: return "ClickHouse";
        case ODBC_DBT_ORACLE: return "Oracle";
        case ODBC_DBT_POST_GRE_SQL: return "PostgreSQL";
        default: return "UNKNOWN(" + std::to_string(static_cast<int>(cat)) + ")";
    }
}

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

extern size_t INI_MAX_PROPERTY_VALUE;

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
    static Mutex CLICK_HOUSE_LOCK;

protected:
    Mutex* getClickHouseLock(){
        if(dataBaseType_ == ODBC_DBT_CLICK_HOUSE)
            return &CLICK_HOUSE_LOCK;
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
        :ODBCBaseConnection(dataBaseType){
            std::string dataBaseTypeString = DataBaseToString(dataBaseType);
            PLUGIN_LOG_INFO(PLUGIN_ODBC_STRING_PREFIX, dataBaseTypeString + " ODBC connection is created.");
        }

    virtual ~OdbcConnection(){
        close(true);
    }

    void connect(std::string connStr) override{
        Mutex* connectLock = (dataBaseType_ != ODBC_DBT_CLICK_HOUSE && dataBaseType_ != ODBC_DBT_VOID) ? nullptr : &CLICK_HOUSE_LOCK;
        if(connectLock != nullptr){
            PLUGIN_LOG_INFO(PLUGIN_ODBC_STRING_PREFIX, "Attempting to acquire global lock for ODBC connection " + std::to_string((long long)this) + ". ");
        }
        LockGuard<Mutex> lock(connectLock);
        PLUGIN_LOG_INFO(PLUGIN_ODBC_STRING_PREFIX, "Attempting to connect to database on ODBC connection " + std::to_string((long long)this) + ". ");
        nanoConn_ = new NanConnection(getODBCFunc().toString(connStr));
        PLUGIN_LOG_INFO(PLUGIN_ODBC_STRING_PREFIX, "Successfully connected to database on ODBC connection " + std::to_string((long long)this) + ". ");
        std::string dbmsName;
        try {
            dbmsName = getODBCFunc().getString(nanoConn_->dbms_name());
        } catch (exception &e) {
            PLUGIN_LOG_INFO(PLUGIN_ODBC_STRING_PREFIX, "Failed to get dbms name: ", e.what());
        }
        dbmsName = Util::lower(dbmsName);
        if (dbmsName == "clickhouse") {
            if (dataBaseType_ != ODBC_DBT_VOID && dataBaseType_ != ODBC_DBT_CLICK_HOUSE) {
                throw RuntimeException(
                    PLUGIN_ODBC_STRING_PREFIX + "If click house ODBC drives, the dataBaseType must be set to click house. ");
            }
            dataBaseType_ = ODBC_DBT_CLICK_HOUSE;
        }
        closed_ = false;
    }
    void close(bool ignoreClosed) override{
        if(dataBaseType_ == ODBC_DBT_CLICK_HOUSE){
            PLUGIN_LOG_INFO(PLUGIN_ODBC_STRING_PREFIX, "Attempting to acquire global lock for ODBC connection " + std::to_string((long long)this) + ". ");
        }
        LockGuard<Mutex> lockClickHouse(getClickHouseLock());
        PLUGIN_LOG_INFO(PLUGIN_ODBC_STRING_PREFIX, "Attempting to acquire lock for ODBC connection " + std::to_string((long long)this) + ". ");
        LockGuard<Mutex> lockGuard(&lock);
        PLUGIN_LOG_INFO(PLUGIN_ODBC_STRING_PREFIX, "Attempting to close the connection " + std::to_string((long long)this) + ". ");
        if(isClosed()){
            if(ignoreClosed)return;
            else{
                PLUGIN_LOG_ERR(PLUGIN_ODBC_STRING_PREFIX, "ODBC connection is closed. ");
                throw RuntimeException(PLUGIN_ODBC_STRING_PREFIX + "ODBC connection is closed. ");
            }
        }
        closed_ = true;

        try{
            nanoConn_->disconnect();
        }catch(exception &e){
            PLUGIN_LOG_ERR(PLUGIN_ODBC_STRING_PREFIX + "Failed to disconnect : " + string(e.what()));
        }catch(...){
            PLUGIN_LOG_ERR(PLUGIN_ODBC_STRING_PREFIX + "Failed to disconnect. ");
        }
        try{
            nanoConn_->deallocate();
        }catch(exception &e){
            PLUGIN_LOG_ERR(PLUGIN_ODBC_STRING_PREFIX + "Failed to deallocate : " + string(e.what()));
        }catch(...){
            PLUGIN_LOG_ERR(PLUGIN_ODBC_STRING_PREFIX + "Failed to deallocate. ");
        }
        nanoConn_.clear();
        PLUGIN_LOG_INFO(PLUGIN_ODBC_STRING_PREFIX, "Successfully closed the connection " + std::to_string((long long)this) + ". ");
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