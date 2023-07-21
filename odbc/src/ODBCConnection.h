#include "DolphinDBODBC.h"

class OdbcConnection;

typedef SmartPointer<OdbcConnection> OdbcConnectionSP;

enum ODBCDataBaseType {
    ODBC_DBT_VOID,
    ODBC_DBT_MYSQL,
    ODBC_DBT_SQLSERVER,
    ODBC_DBT_SQLITE,
    ODBC_DBT_CLICKHOUSE,
    ODBC_DBT_ORACLE,
    ODBC_DBT_POSTGRSQL
};

class OdbcConnection {
public:
    OdbcConnection(nanodbc::connection* connection, ODBCDataBaseType dataBaseType)
        : nanoConn_(connection), dataBaseType_(dataBaseType) {}
    ODBCDataBaseType getDataBaseType() { return dataBaseType_; }
    void close(bool ignoreClosed){
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
        delete nanoConn_.release();
    }

    bool isClosed(){
        return closed_ || !nanoConn_->connected();
    }


    ConstantSP odbcAppend(Heap* heap, TableSP t, const string& tableName, bool createTable, bool insertIgnore);
    ConstantSP odbcQuery(Heap* heap, const TableSP& schemaTable, const FunctionDefSP& transform, int batchSize, const string& querySql);
    void odbcExecute(const string &querySql, Heap *heap);
    void odbcExecute(nanodbc::transaction &cp, const string &querySql, Heap *heap);
    static ConstantSP odbcGetConnection(Heap *heap, vector<ConstantSP> &args, const string &funcName);
    static TableSP odbcGetOrCreateTable(Heap *heap, const TableSP& schemaTable,
                                    const nanodbc::result &results, vector<ConstantSP> &columnVecs,
                                    vector<string> &colNames, bool compatibility);
    static ODBCDataBaseType getDataBaseType(const string &dataBase);
    static inline DATA_TYPE sqlType2DataType(int sqlType, long colSize, int cType);
    static bool compatible(DATA_TYPE dolphinType, int sqlType, int colSize);
    static void getColNames(const nanodbc::result &results, vector<std::string> &columnNames);
    static void getColVecAndType(const nanodbc::result &results, vector<ConstantSP> &columnVecs,
                      vector<DATA_TYPE> &columnTypes);
    static string sqlTypeName(DATA_TYPE t, ODBCDataBaseType databaseType);
    static string getValueStr(string p, DATA_TYPE t, ODBCDataBaseType databaseType);
    static unordered_map<long long, OdbcConnectionSP> ODBC_CONN_MAP;
    static Mutex ODBC_PLUGIN_LOCK;
    static unordered_map<string, ODBCDataBaseType> DBT_MAP;

private:
    std::unique_ptr<nanodbc::connection> nanoConn_;
    ODBCDataBaseType dataBaseType_;
    bool closed_ =  false;
    Mutex lock;

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