#include "plugin_mysql.h"
#include "ddbplugin/Plugin.h"

using dolphindb::Connection;
using dolphindb::ConnectionSP;
using dolphindb::messageSP;
using std::cout;
using std::endl;

ConstantSP safeOp(const ConstantSP &arg, std::function<ConstantSP(Connection *)> &&f) {
    if (arg->getType() == DT_RESOURCE) {
        string desc = arg->getString();
        if(desc.find("mysql connection") != 0) {
            throw IllegalArgumentException(__FUNCTION__, "Invalid connection object.");
        }
        if (arg->getLong() == 0){
            throw IllegalArgumentException(__FUNCTION__, "Invalid connection handle.");
        }
        auto conn = (Connection *)(arg->getLong());
        return conn->connected() ? f(conn) : messageSP("Not connected yet.");
    } else {
        throw IllegalArgumentException(__FUNCTION__, "Must be a Resource Object.");
    }
}

vector<ConstantSP> getArgs(vector<ConstantSP> &args, size_t nMaxArgs) {
    auto ret = vector<ConstantSP>(nMaxArgs);
    for (size_t i = 0; i < nMaxArgs; ++i) {
        if (args.size() >= i + 1)
            ret[i] = args[i];
        else
            ret[i] = Util::createNullConstant(DT_VOID);
    }
    return ret;
}

static void mysqlConnectionOnClose(Heap *heap, vector<ConstantSP> &args) {
    Connection* conn = reinterpret_cast<Connection*>(args[0]->getLong());
    if(conn != nullptr){
        delete conn;
        args[0]->setLong(0);
    }
}

ConstantSP mysqlConnect(Heap *heap, vector<ConstantSP> &args) {
    std::string usage = "Usage: connect(host, port, user, password, db). ";
    // parse args first
    if (args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "host must be a string");
    }
	if (args[0]->getString().empty()) {
		throw IllegalArgumentException(__FUNCTION__, usage + "host can't be empty");
	}
    if (args[1]->getType() != DT_INT || args[1]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "port must be an integer");
    }
    if (args[2]->getType() != DT_STRING || args[2]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "user must be a string");
    }
	if (args[2]->getString().empty()) {
		throw IllegalArgumentException(__FUNCTION__, usage + "user can't be empty");
	}
    if (args[3]->getType() != DT_STRING || args[3]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "password must be a string");
    }
    if (args[4]->getType() != DT_STRING || args[4]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "db must be a string");
    }
    std::unique_ptr<Connection> cup(new Connection(args[0]->getString(), args[1]->getInt(), args[2]->getString(), args[3]->getString(), args[4]->getString()));
    std::string desc = "mysql connection to [";
    desc.append(cup->str()).append("]");
    dolphindb::littleEndian = Util::isLittleEndian();
    FunctionDefSP onClose(Util::createSystemProcedure("mysql connection onClose()", mysqlConnectionOnClose, 1, 1));
    return Util::createResource((long long)cup.release(), desc.data(), onClose, heap->currentSession());
}

ConstantSP mysqlTables(const ConstantSP &connection) {
    return safeOp(connection, [&](Connection *conn) { return conn->doQuery("show tables;"); });
}

ConstantSP mysqlSchema(const ConstantSP &connection, const ConstantSP &table) {
    if (table->getType() != DT_STRING)
        throw IllegalArgumentException(__FUNCTION__, "Usage：extractScheme(connection, table). table must be a string");
    return safeOp(connection, [&](Connection *conn) { return conn->extractSchema(table->getString()); });
}

ConstantSP mysqlLoad(Heap *heap, vector<ConstantSP> &arguments) {
    auto args = getArgs(arguments, 6);
    std::string usage = "Usage: load(connection, table_or_query, [schema], [startRow=0], [rowNum=ULONGLONG_MAX], [allowEmptyTable=FALSE]). ";

    std::string table;
    TableSP schema = nullptr;
    uint64_t startRow = 0, rowNum = std::numeric_limits<uint64_t>::max();
    bool allowEmptyTable=false;

    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "table_or_query must be a string");
    } else {
        table = args[1]->getString();
    }

    if (!args[2]->isNothing()) {
        if (!args[2]->isTable()) {
            throw IllegalArgumentException(__FUNCTION__, usage + "schema must be a table");
        }
        schema = args[2];
    }

    if (!args[3]->isNothing()) {
        if ((args[3]->getType() != DT_INT && args[3]->getType() != DT_LONG) || args[3]->getForm() != DF_SCALAR)
            throw IllegalArgumentException(__FUNCTION__, usage + "startRow must be a non-negative integer");
        if (args[3]->getInt() < 0)
            throw IllegalArgumentException(__FUNCTION__, usage + "startRow must be a non-negative integer");
        startRow = args[3]->getLong();
    }

    if (!args[4]->isNothing()) {
        if ((args[4]->getType() != DT_INT && args[4]->getType() != DT_LONG) || args[4]->getForm() != DF_SCALAR)
            throw IllegalArgumentException(__FUNCTION__, usage + "rowNum must be a integer");
        if (args[4]->getInt() < 0)
            throw IllegalArgumentException(__FUNCTION__, usage + "rowNum must be a non-negative integer");
        rowNum = args[4]->getLong();
    }
    if (!args[5]->isNothing()) {
        if ((args[5]->getType() != DT_BOOL || args[5]->getForm() != DF_SCALAR))
            throw IllegalArgumentException(__FUNCTION__, usage + "allowEmptyTable must be a bool");
        allowEmptyTable = args[5]->getBool();
    }

    return safeOp(args[0], [&](Connection *conn) { return conn->load(table, schema, startRow, rowNum , allowEmptyTable); });
}

ConstantSP mysqlLoadEx(Heap *heap, vector<ConstantSP> &arguments) {
    auto args = getArgs(arguments, 9);
    ConstantSP dbHandle = nullptr;
    std::string tableName, tableInMySQL;
    ConstantSP partitionColumns = nullptr;
    TableSP schema = nullptr;
    uint64_t startRow = 0, rowNum = std::numeric_limits<uint64_t>::max();

    std::string usage = "Usage: loadEx(connection,dbHandle,tableName,partitionColumns,tableInMySQL_or_query,[schema],[startRow=0],[rowNum=ULONGLONG_MAX],[transform]) ";

    if (!args[1]->isDatabase()) {
        throw IllegalArgumentException(__FUNCTION__, usage + "dbHandle must be a database handle");
    } else {
        dbHandle = args[1];
    }

    if (args[2]->getType() != DT_STRING || args[2]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "tableName must be a string");
    } else {
        tableName = args[2]->getString();
    }

    if (args[3]->isNothing()) {
        throw IllegalArgumentException(__FUNCTION__, usage + "partitionColumns is required.");
    } else if (args[3]->getType() != DT_STRING)
        throw IllegalArgumentException(__FUNCTION__, usage + "partitionColumns must be a string scalar/vector");
    else {
        partitionColumns = args[3];
    }

    if (args[4]->getType() != DT_STRING || args[4]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "tableInMySQL_or_query must be a string");
    } else {
        tableInMySQL = args[4]->getString();
    }

    if (!args[5]->isNothing()) {
        if (!args[5]->isTable()) {
            throw IllegalArgumentException(__FUNCTION__, usage + "schema must be a table");
        }
        schema = args[5];
    }

    if (!args[6]->isNothing()) {
        if ((args[6]->getType() != DT_INT && args[6]->getType() != DT_LONG) || args[6]->getForm() != DF_SCALAR)
            throw IllegalArgumentException(__FUNCTION__, usage + "startRow must be a non-negative integer");
        if (args[6]->getInt() < 0)
            throw IllegalArgumentException(__FUNCTION__, usage + "startRow must be a non-negative integer");
        startRow = args[6]->getLong();
    }

    if (!args[7]->isNothing()) {
        if ((args[7]->getType() != DT_INT && args[7]->getType() != DT_LONG) || args[7]->getForm() != DF_SCALAR)
            throw IllegalArgumentException(__FUNCTION__, usage + "rowNum must be a non-negative integer");
        if (args[7]->getInt() < 0)
            throw IllegalArgumentException(__FUNCTION__, usage + "rowNum must be a non-negative integer");
        rowNum = args[7]->getLong();
    }
    FunctionDefSP transform;
    if (!args[8]->isNothing()) {
        if (arguments[8]->getType() != DT_FUNCTIONDEF)
            throw IllegalArgumentException(__FUNCTION__, "transform must be a function.");
        transform = (FunctionDefSP)args[8];
        return safeOp(args[0], [&](Connection *conn) {
            return conn->loadEx(heap, dbHandle, tableName, partitionColumns, tableInMySQL, schema, startRow, rowNum, transform);
        });
    }
    return safeOp(args[0], [&](Connection *conn) {
        return conn->loadEx(heap, dbHandle, tableName, partitionColumns, tableInMySQL, schema, startRow, rowNum);
    });
    
}

/// Connection
namespace dolphindb {
Connection::~Connection() {
}

Connection::Connection(std::string hostname, int port, std::string username, std::string password, std::string database)
    : host_(hostname), user_(username), password_(password), db_(database), port_(port) {
    try {
        connect(db_.c_str(), host_.c_str(), user_.c_str(), password_.c_str(), port_);
    } catch (mysqlxx::Exception &e) {
        throw RuntimeException("Failed to connect, error: " + std::string(e.name()) + " " + std::string(e.displayText()));
    }
}

bool Connection::isQuery(std::string query) {
    if (query.size() > 7) {
        auto s = query.substr(0, 7);
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return s == "select ";
    }
    return false;
}

ConstantSP Connection::doQuery(const std::string &str) {
    LockGuard<Mutex> lk(&mtx_);
    return MySQLExtractor(query(str)).extract();
}

ConstantSP Connection::load(const std::string &table_or_query, const TableSP &schema, const uint64_t &startRow, const uint64_t &rowNum,const bool &allowEmptyTable) {
    LockGuard<Mutex> lk(&mtx_);
    if (isQuery(table_or_query)) {
        return MySQLExtractor(query(table_or_query)).extract(schema,allowEmptyTable);
    } else {
        return MySQLExtractor(query("SELECT * FROM " + table_or_query + " LIMIT " + std::to_string(startRow) + "," + std::to_string(rowNum))).extract(schema,allowEmptyTable);
    }
}

ConstantSP Connection::loadEx(Heap *heap, const ConstantSP &dbHandle, const std::string &tableName, const ConstantSP &partitionColumns, const std::string &MySQLTableName_or_query,
                              const TableSP &schema, const uint64_t &startRow, const uint64_t &rowNum, const FunctionDefSP &transform) {
    LockGuard<Mutex> lk(&mtx_);
    DomainSP domain = static_cast<SystemHandleSP>(dbHandle)->getDomain();
    if (domain.isNull()) {
        throw IllegalArgumentException(__FUNCTION__, "Only partitioned database is supported");
    }
    if (domain->getPartitionType() == SEQ) {
        throw IllegalArgumentException(__FUNCTION__, "SEQ-based table not supported");
    }

    // check if table exists
    std::string addr = static_cast<SystemHandleSP>(dbHandle)->getDatabaseDir();
    TableSP ret = nullptr;

    bool tableExists = true;
    {
        vector<ConstantSP> args{new String(addr), new String(tableName)};
        tableExists &= heap->currentSession()->getFunctionDef("existsTable")->call(heap, args)->getBool();
    }

    if (tableExists) {
        // must use new String(addr) here, otherwise pointer in ConstantSP is
        // freed by someone for whatever reason.
        vector<ConstantSP> args{new String(addr), new String(tableName)};
        ret = heap->currentSession()->getFunctionDef("loadTable")->call(heap, args);
    } else {
        vector<ConstantSP> args{dbHandle, load(MySQLTableName_or_query, schema, 0, 1), new String(tableName), partitionColumns};
        ret = heap->currentSession()->getFunctionDef("createPartitionedTable")->call(heap, args);
    }

    if (isQuery(MySQLTableName_or_query)) {
        MySQLExtractor(query(MySQLTableName_or_query)).extractEx(heap, ret, transform, schema);
    } else {
        MySQLExtractor(query("SELECT * FROM " + MySQLTableName_or_query + " LIMIT " + std::to_string(startRow) + "," + std::to_string(rowNum))).extractEx(heap, ret, transform, schema);
    }

    return ret;
}

ConstantSP Connection::extractSchema(const std::string &table) {
    LockGuard<Mutex> lk(&mtx_);
    return MySQLExtractor(query()).extractSchema(table);
}

MySQLExtractor::MySQLExtractor(const mysqlxx::Query &q) : query_(q), emptyPackIdx_(DEFAULT_WORKSPACE_SIZE), fullPackIdx_(DEFAULT_WORKSPACE_SIZE), workspace_(DEFAULT_WORKSPACE_SIZE) {
}

MySQLExtractor::~MySQLExtractor() = default;

TableSP MySQLExtractor::extractSchema(const std::string &table) {
    try {
        query_.reset();
        query_ << "SELECT * FROM " << table << " LIMIT 0;";
        auto res = query_.use();
        int numFields = res.getNumFields();
        auto colNames = Util::createVector(DT_STRING, numFields, numFields);
        auto colTypes = Util::createVector(DT_STRING, numFields, numFields);
        auto colTypesMySQL = Util::createVector(DT_STRING, numFields, numFields);
        auto colRawType = Util::createVector(DT_STRING, numFields, numFields);

        for (int i = 0; i < numFields; ++i) {
            colNames->set(i, new String(res.nameAt(i)));
            colTypes->set(i, new String(Util::getDataTypeString(getDolphinDBType(res.typeAt(i), res.isUnsignedAt(i), res.isEnumAt(i), res.maxLengthAt(i)))));
            colRawType->set(i, new String(getMySQLTypeStr(res.typeAt(i))));
        }
        res.fetch();    // otherwise mysqlclient will hang

        {
            Query tmp(query_);
            tmp.reset();
            tmp << "DESCRIBE " << table;
            auto res = tmp.use();
            for (int i = 0; i < numFields; ++i) {
                auto row = res.fetch();
                colTypesMySQL->set(i, new String(row[1].data()));
            }
        }
#ifdef DEBUG
        return Util::createTable({"name", "type", "MySQL describe type", "mysqlxx returned type"}, {colNames, colTypes, colTypesMySQL, colRawType});
#else
        return Util::createTable({"name", "type", "MySQL describe type"}, {colNames, colTypes, colTypesMySQL});
#endif
    } catch (mysqlxx::Exception &e) {
        throw RuntimeException(std::string(e.name()) + " " + std::string(e.displayText()));
    }
}

TableSP MySQLExtractor::extract(const ConstantSP &schema ,const bool &allowEmptyTable) {
    try {
        auto res = query_.use();
        prepareForExtract(schema, res);
        TableSP t = Util::createTable(colNames_, dstColTypes_, 0, 0);

        realExtract(res, schema, t, [&](Pack &p) { growTable(t, p); });
        if (t->size() == 0 && !allowEmptyTable) {
            throw RuntimeException("Empty result table.");
        }
        return t;
    } catch (mysqlxx::Exception &e) {
        throw RuntimeException(std::string(e.name()) + " " + std::string(e.displayText()));
    }
}

void MySQLExtractor::extractEx(Heap *heap, TableSP &t, const FunctionDefSP &transform, const ConstantSP &schema) {
    try {
        auto res = query_.use();
        prepareForExtract(schema, res);
        realExtract(res, schema, t, [&](Pack &p) { growTableEx(t, p, heap, transform); });
    } catch (mysqlxx::Exception &e) {
        throw RuntimeException(std::string(e.name()) + " " + std::string(e.displayText()));
    }
}

void MySQLExtractor::realExtract(mysqlxx::UseQueryResult &res, const ConstantSP &schema, TableSP &resultTable, std::function<void(Pack &)> callback) {
    try {
        assert(emptyPackIdx_.size() == 0);
        assert(fullPackIdx_.size() == 0);
        for (int i = 0; i < (int)workspace_.size(); ++i) {
            workspace_[i].init(srcColTypes_, dstColTypes_, resultTable, maxStrLen_);
            emptyPackIdx_.push(i);
        }
    } catch (mysqlxx::Exception &e) {
        throw RuntimeException(std::string(e.name()) + " " + std::string(e.displayText()));
    }

    std::exception_ptr fetchRowException_ptr = nullptr;
    auto fetchRow = [&]() {
        try {
            while (auto row = res.fetch()) {
                if (workingPackIdx_ == -1) {
                    workerRequest();
                    // consumer tells me to stop
                    if (unlikely(workspace_[workingPackIdx_].full()))
                        return;
                }
                Pack &pack = workspace_[workingPackIdx_];
                pack.append(row);

                if (unlikely(pack.full())) {
                    workerRelease();
                }
            }
            if (workingPackIdx_ != -1)
                workerRelease();    // release tail rows

            // push empty pack, tell consumer to stop
            workerRequest();
            workerRelease();
            query_.reset();
        } catch (...) {
            if (workingPackIdx_ != -1)
                workerRelease();
            // push empty pack, tell consumer to stop
            workerRequest();
            workerRelease();
            fetchRowException_ptr = std::current_exception();
        }
    };

    std::exception_ptr growTableException_ptr = nullptr;
    auto growTable = [&]() {
        try {
            while (true) {
                consumerRequest();
                assert(consumingPackIdx_ != -1);
                Pack &pack = workspace_[consumingPackIdx_];
                if (pack.size() == 0) {
                    // get empty pack, terminate
                    return;
                }
                callback(pack);
                consumerRelease();
            }
        } catch (...) {
            // must haven't called pack.clear(), which never throw
            // push full pack, tell worker to stop
            consumerRelease();
            growTableException_ptr = std::current_exception();
        }
    };

    RunnableSP r1 = new Executor(fetchRow);
    RunnableSP r2 = new Executor(growTable);

    Thread t1(r1), t2(r2);
    t1.start();
    t2.start();
    t1.join();
    t2.join();
    if (fetchRowException_ptr)
        std::rethrow_exception(fetchRowException_ptr);
    if (growTableException_ptr)
        std::rethrow_exception(growTableException_ptr);
}

void MySQLExtractor::prepareForExtract(const ConstantSP &schema, mysqlxx::ResultBase &res) {
    colNames_ = getColNames(res);
    srcColTypes_ = getColTypes(res);
    maxStrLen_ = getMaxStrlen(res);
    dstColTypes_ = schema.isNull() ? srcColTypes_ : getDstType(schema);
    compatible(dstColTypes_, srcColTypes_);
}

void MySQLExtractor::workerRequest() {
    assert(workingPackIdx_ == -1);
    emptyPackIdx_.blockingPop(workingPackIdx_);
    assert(workingPackIdx_ != -1);
}

void MySQLExtractor::workerRelease() {
    assert(workingPackIdx_ != -1);
    fullPackIdx_.blockingPush(workingPackIdx_);
    workingPackIdx_ = -1;
}

void MySQLExtractor::consumerRequest() {
    assert(consumingPackIdx_ == -1);
    fullPackIdx_.blockingPop(consumingPackIdx_);
    assert(consumingPackIdx_ != -1);
}

void MySQLExtractor::consumerRelease() {
    assert(consumingPackIdx_ != -1);
    emptyPackIdx_.blockingPush(consumingPackIdx_);
    consumingPackIdx_ = -1;
}

void MySQLExtractor::growTable(TableSP &t, Pack &p) {
    realGrowTable(p, [&](vector<ConstantSP> &cols) {
        INDEX insertedRows;
        std::string errMsg;
        t->append(cols, insertedRows, errMsg);
        if (errMsg.size()) {
            throw RuntimeException(errMsg);
        }
    });
}

void MySQLExtractor::growTableEx(TableSP &t, Pack &p, Heap *heap, const FunctionDefSP &transform) {
    realGrowTable(p, [&](vector<ConstantSP> &cols) {
        TableSP tmpTable = Util::createTable(colNames_, cols);
        if(!transform.isNull()){
            vector<ConstantSP> arg={tmpTable};
            tmpTable = transform->call(heap, arg);
        }
        vector<ConstantSP> args{t, tmpTable};
        heap->currentSession()->getFunctionDef("append!")->call(heap, args);
    });
}

void MySQLExtractor::realGrowTable(Pack &p, std::function<void(vector<ConstantSP> &)> &&callback) {
    auto buffers = p.data();
    int nCol = p.nCol();
    vector<ConstantSP> cols(nCol);
    int len = p.size();
    for (int idx = 0; idx < nCol; ++idx) {
        if (dstColTypes_[idx] == DT_SYMBOL)
            cols[idx] = Util::createVector(DT_STRING, len, len);
        else
            cols[idx] = Util::createVector(dstColTypes_[idx], len, len);
        VectorSP vec = cols[idx];
        char *colBuffer = buffers[idx];
        switch (dstColTypes_[idx]) {
            case DT_BOOL:
            case DT_CHAR:
                vec->setChar(0, len, colBuffer);
                break;
            case DT_SHORT:
                vec->setShort(0, len, (short*)colBuffer);
                break;
            case DT_INT:
            case DT_DATETIME:
            case DT_DATE:
            case DT_MONTH:
            case DT_TIME:
            case DT_MINUTE:
            case DT_SECOND:
                vec->setInt(0, len, (int*)colBuffer);
                break;
            case DT_TIMESTAMP:
            case DT_NANOTIME:
            case DT_NANOTIMESTAMP:
            case DT_LONG:
                vec->setLong(0, len, (long long*)colBuffer);
                break;
            case DT_FLOAT:
                vec->setFloat(0, len, (float*)colBuffer);
                break;
            case DT_DOUBLE:
                vec->setDouble(0, len, (double*)colBuffer);
                break;
           case DT_SYMBOL:
            case DT_STRING:
                vec->setString(0, len, (char **)colBuffer);
                break;
            default:
                throw RuntimeException("The " + Util::getDataTypeString(dstColTypes_[idx]) + " type is not supported to realGrowTable");
        }
        if(p.containNull(idx))
            vec->setNullFlag(true);
    }
    callback(cols);
    p.clear();
}

vector<string> MySQLExtractor::getColNames(mysqlxx::ResultBase &res) {
    auto nCols = res.getNumFields();
    auto ret = vector<string>(nCols);
    for (size_t i = 0; i < nCols; ++i) {
        ret[i] = res.nameAt(i);
    }
    return ret;
}

vector<DATA_TYPE> MySQLExtractor::getColTypes(mysqlxx::ResultBase &res) {
    auto nCols = res.getNumFields();
    auto ret = vector<DATA_TYPE>(nCols);
    for (size_t i = 0; i < nCols; ++i) {
        ret[i] = getDolphinDBType(res.typeAt(i), res.isUnsignedAt(i), res.isEnumAt(i), res.maxLengthAt(i));
    }
    return ret;
}

// tinytext: 255
// text: 65535
// mediumtext: 16777215
// longtext: 4294967295

vector<size_t> MySQLExtractor::getMaxStrlen(mysqlxx::ResultBase &res) {
    int numFields = res.getNumFields();
    vector<size_t> ret(numFields, 0);
    for (int i = 0; i < numFields; ++i) {
        ret[i] = res.maxLengthAt(i);
        auto t = res.typeAt(i);
        if (t == mysqlxx::MYSQL_TYPE_STRING || t == mysqlxx::MYSQL_TYPE_VAR_STRING ||
            t == mysqlxx::MYSQL_TYPE_VARCHAR || t == mysqlxx::MYSQL_TYPE_ENUM || t == mysqlxx::MYSQL_TYPE_BIT) {
            ret[i] += 1;
        } else if (t == mysqlxx::MYSQL_TYPE_BLOB || t == mysqlxx::MYSQL_TYPE_TINY_BLOB ||
                   t == mysqlxx::MYSQL_TYPE_MEDIUM_BLOB || t == mysqlxx::MYSQL_TYPE_LONG_BLOB) {
            ret[i] = 65536;
        }
    }
    return ret;
}

// !? need more consideration on type conversion
vector<DATA_TYPE> MySQLExtractor::getDstType(const TableSP &schemaTable) {
    vector<DATA_TYPE> ret;
    auto schema = schemaTable->getColumn(1);
    for (int i = 0; i < schema->size(); ++i) {
        ret.emplace_back(Util::getDataType(schema->getString(i)));
    }

    for (auto &t : ret) {
        auto cat = Util::getCategory(t);
        if (cat != LOGICAL && cat != INTEGRAL && cat != FLOATING && cat != TEMPORAL && cat != LITERAL) {
            throw RuntimeException("The " + Util::getDataTypeString(t) + " type is not supported");
        }
    }
    return ret;
}

unsigned long long Pack::getRowStorage(vector<DATA_TYPE> types, vector<size_t> maxStrlen) {
    unsigned long long ret = 0;
    for (size_t i = 0; i < types.size(); ++i) {
        ret += typeLen(types[i]);
        if (types[i] == DT_STRING || types[i] == DT_SYMBOL) {
            ret += maxStrlen[i];
        }
    }
    return ret;
}

void Pack::init(vector<DATA_TYPE> srcDt, vector<DATA_TYPE> dstDt, TableSP &resultTable, vector<size_t> maxStrLen, size_t cap) {
    assert(srcDt.size() == dstDt.size());
    nCol_ = srcDt.size();
    size_ = 0;
    capacity_ = cap;
    srcDt_ = srcDt;
    dstDt_ = dstDt;
    typeLen_.resize(nCol_);
    rawBuffers_.resize(nCol_);
    maxStrLen_ = maxStrLen;
    containNull_.resize(nCol_, false);
    auto rowStorage = getRowStorage(dstDt_, maxStrLen);
    auto allowedRow = DEFAULT_ALLOWED_MEM / rowStorage;
    capacity_ = capacity_ < allowedRow ? capacity_ : allowedRow;
    if (capacity_ == 0) {
        throw RuntimeException("Insufficient memory to load even one row, please check your MySQL "
                               "table, rowStorage: " + Util::convert(rowStorage / 1024 / 1024) + "M");
    }
    initedCols_ = 0;
    for (size_t col = 0; col < nCol_; ++col) {
        typeLen_[col] = typeLen(dstDt_[col]);
        rawBuffers_[col] = new char[capacity_ * typeLen_[col]];

        // initialize
        if (dstDt_[col] == DT_STRING || dstDt_[col] == DT_SYMBOL) {
            assert(typeLen_[col] == sizeof(char *));
            for (size_t j = 0; j < capacity_; ++j) {
                // from char* to char**
                auto p = (char **)(rawBuffers_[col] + j * sizeof(char *));
                *p = new char[maxStrLen[col] + 1];
            }
                char *nullv = new char[1];
                nullv[0] = '\0';
                nullVal_.emplace_back(nullv);
        }
        else{
            char *nullv = new char[typeLen_[col]];
            getValNull(dstDt_[col], nullv);
            nullVal_.emplace_back(nullv);
        }
        initedCols_ ++;
    }
}

void Pack::clear() {
    size_ = 0;
}

void Pack::append(const mysqlxx::Row &row) {
    try {
        for (size_t col = 0; col < nCol_; ++col) {
            char *dst = rawBuffers_[col] + size_ * typeLen_[col];
            auto val = row[col];
            if(row.getTypeAt(col) == mysqlxx::MYSQL_TYPE_BIT)
                containNull_[col] = containNull_[col] | parseBit(dst, val,  dstDt_[col], nullVal_[col], typeLen_[col], maxStrLen_[col]);
            else
                switch (srcDt_[col]) {
                    case DT_BOOL:
                         containNull_[col] = containNull_[col] | parseScalar<bool>(dst, val, dstDt_[col], nullVal_[col], typeLen_[col]);
                         break;
                    case DT_CHAR:
                        containNull_[col] = containNull_[col] | parseScalar<char>(dst, val, dstDt_[col], nullVal_[col], typeLen_[col]);
                        break;
                    case DT_SHORT:
                        containNull_[col] = containNull_[col] | parseScalar<short>(dst, val, dstDt_[col], nullVal_[col], typeLen_[col]);
                        break;
                    case DT_INT:
                        containNull_[col] = containNull_[col] | parseScalar<int>(dst, val, dstDt_[col], nullVal_[col], typeLen_[col]);
                        break;
                    case DT_LONG:
                        containNull_[col] = containNull_[col] | parseScalar<long long>(dst, val, dstDt_[col], nullVal_[col], typeLen_[col]);
                        break;
                    case DT_FLOAT:
                        containNull_[col] = containNull_[col] | parseScalar<float>(dst, val, dstDt_[col], nullVal_[col], typeLen_[col]);
                        break;
                    case DT_DOUBLE:
                        containNull_[col] = containNull_[col] | parseScalar<double>(dst, val, dstDt_[col], nullVal_[col], typeLen_[col]);
                        break;
                    case DT_TIMESTAMP:
                        containNull_[col] = containNull_[col] | parseTimestamp(dst, val, dstDt_[col], nullVal_[col], typeLen_[col]);
                        break;
                    case DT_NANOTIME:
                         containNull_[col] = containNull_[col] | parseNanotime(dst, val, dstDt_[col], nullVal_[col], typeLen_[col]);
                         break;
                     case DT_NANOTIMESTAMP:
                         containNull_[col] = containNull_[col] | parseNanoTimestamp(dst, val, dstDt_[col], nullVal_[col], typeLen_[col]);
                         break;
                    case DT_DATE:
                        containNull_[col] = containNull_[col] | parseDate(dst, val, dstDt_[col], nullVal_[col], typeLen_[col]);
                        break;
                     case DT_MONTH:
                         containNull_[col] = containNull_[col] | parseMonth(dst, val, dstDt_[col], nullVal_[col], typeLen_[col]);
                         break;
                    case DT_TIME:
                        containNull_[col] = containNull_[col] | parseTime(dst, val, dstDt_[col], nullVal_[col], typeLen_[col]);
                        break;
                     case DT_MINUTE:
                         containNull_[col] = containNull_[col] | parseMinute(dst, val, dstDt_[col], nullVal_[col], typeLen_[col]);
                         break;
                     case DT_SECOND:
                         containNull_[col] = containNull_[col] | parseSecond(dst, val, dstDt_[col], nullVal_[col], typeLen_[col]);
                         break;
                    case DT_DATETIME:
                        containNull_[col] = containNull_[col] | parseDatetime(dst, val, dstDt_[col], nullVal_[col], typeLen_[col]);
                        break;
                    case DT_SYMBOL:
                    case DT_STRING:
                            containNull_[col] = containNull_[col] | parseString(dst, val, maxStrLen_[col]);
                        break;
                    default:
                        throw RuntimeException("The " + Util::getDataTypeString(srcDt_[col]) + " type is not supported to pack append");
                }
        }
        ++size_;
        assert(size_ <= capacity_);
    } catch (mysqlxx::Exception &e) {
        throw RuntimeException("Something wrong happened with mysqlxx: " + std::string(e.name()) + " " + std::string(e.displayText()));
    }
}

Pack::~Pack() {
    for (size_t col = 0; col < initedCols_; ++col) {
        if (dstDt_.size() > col && (dstDt_[col] == DT_STRING || dstDt_[col] == DT_SYMBOL)) {
            for (size_t j = 0; j < capacity_; ++j) {
                auto p = (char **)(rawBuffers_[col] + j * sizeof(char *));
                delete[](*p);
            }
        }
        if (rawBuffers_.size() > col)
            delete[] rawBuffers_[col];
        if (nullVal_.size() > col)
            delete[] nullVal_[col];
    }
}

bool Pack::parseString(char *dst, const mysqlxx::Value &val, size_t maxLen) {
    size_t len = std::min(val.size(), maxLen);
    memcpy(*((char **)dst), val.data(), len);
    (*((char **)dst))[len] = '\0';
    return !val.empty();
}

// parsers
// get year (from the value of the form '2011-01-01' or '2011-01-01 00:00:00').
inline int get(const char *data, int a, int b) {
    return (data[a] - '0') * 10 + (data[b] - '0');
}

inline int haveDate(const mysqlxx::Value &val) {
    return val.size() >= 10 && val.data()[4] == '-' && val.data()[7] == '-';
}

inline bool year(const mysqlxx::Value &val, int &ret) {
    auto data = val.data();
    if (haveDate(val)) {
        ret = (data[0] - '0') * 1000 + (data[1] - '0') * 100 + (data[2] - '0') * 10 + (data[3] - '0');
        return true;
    } else {
        return false;
    }
}

inline bool month(const mysqlxx::Value &val, int &ret) {
    if (haveDate(val)) {
        ret = get(val.data(), 5, 6);
        return true;
    } else {
        return false;
    }
}

inline bool day(const mysqlxx::Value &val, int &ret) {
    if (haveDate(val)) {
        ret = get(val.data(), 8, 9);
        return true;
    } else {
        return false;
    }
}

inline bool hour(const mysqlxx::Value &val, int &ret) {
    const char *data = val.data();
    if (val.size() >= 19) {
        ret = get(data, 11, 12);
        return true;
    } else if (val.size() > 2 && data[2] == ':') {
        ret = get(val.data(), 0, 1);
        return true;
    } else {
        return false;
    }
}

inline bool minute(const mysqlxx::Value &val, int &ret) {
    const char *data = val.data();
    if (val.size() >= 19) {
        ret = get(data, 14, 15);
        return true;
    } else if (val.size() > 2 && data[2] == ':') {
        ret = get(data, 3, 4);
        return true;
    } else {
        return false;
    }
}

inline bool second(const mysqlxx::Value &val, int &ret) {
    const char *data = val.data();
    if (val.size() >= 19) {
        ret = get(data, 17, 18);
        return true;
    } else if (val.size() > 2 && data[2] == ':') {
        ret = get(data, 6, 7);
        return true;
    } else {
        return false;
    }
}

// extract fraction second from '2019-01-22 18:05:24.123456' (size: [19 ~ 26])
//                           or '18:05:24.123456' (size [8 ~ 15])
inline bool haveFractionalSecond(const mysqlxx::Value &val) {
    if (val.size() > 20) {
        assert(val.data()[19] == '.');
        return true;
    }

    // '18:05:24.123456'
    if (val.size() > 9 && val.data()[8] == '.' && val.data()[2] == ':' && val.data()[5]) {
        assert(val.data()[8] == '.');
        return true;
    }
    return false;
}

inline bool extractFraction(const mysqlxx::Value &val, int start, double &ret) {
    ret = 0.0;
    double mul = 10.0;
    const char *data = val.data();
    for (int i = start; i < (int)val.size(); ++i) {
        if (data[i] < '0' || data[i] > '9')
            return false;
        ret += (data[i] - '0') / mul;
        mul *= 10.0;
    }
    return true;
}

inline bool fractionalSecond(const mysqlxx::Value &val, double &ret) {
    if (haveFractionalSecond(val)) {
        if (val.data()[8] == '.') {
            return extractFraction(val, 9, ret);
        } else {
            return extractFraction(val, 20, ret);
        }
    }
    return true;
}

inline int countSeconds(int h, int m, int s) {
    return (h * 60 + m) * 60 + s;
}

bool getUnixTimeStamp(const mysqlxx::Value &val, long long &ret) {
    int y, mon, d, h, m, s;
    if (year(val, y) && month(val, mon) && day(val, d) && hour(val, h) && minute(val, m) && second(val, s)) {
        ret = 86400LL * Util::countDays(y, mon, d) + countSeconds(h, m, s);
        return true;
    } else {
        return false;
    }
}

bool parseDate(char *dst, const mysqlxx::Value &val, DATA_TYPE &dstDt, char* nullVal, size_t len) {
    if(val.empty()){
        memcpy(dst, nullVal, len);
        return true;
    }
    int y, m, d;
    if (year(val, y) && month(val, m) && day(val, d)) {
        setter(dst, Util::countDays(y, m, d), dstDt);
        return false;
    } else {
        memcpy(dst, nullVal, len);
        return true;
    }
    return false;
}

 bool parseMonth(char *dst, const mysqlxx::Value &val, DATA_TYPE &dstDt, char* nullVal, size_t len) {
     if(val.empty()){
         memcpy(dst, nullVal, len);
         return true;
     }
     int y, m;
     if (year(val, y) && month(val, m)) {
         setter(dst, y * 12 + m - 1, dstDt);
         return false;
     } else {
         memcpy(dst, nullVal, len);
         return true;
     }
     return false;
 }

bool parseTime(char *dst, const mysqlxx::Value &val, DATA_TYPE &dstDt, char* nullVal, size_t len) {
    if(val.empty()){
        memcpy(dst, nullVal, len);
        return true;
    }
    int h, m, s;
    double fracSec = 0.0;
    if (hour(val, h) && minute(val, m) && second(val, s) && fractionalSecond(val, fracSec)) {
        setter(dst, countSeconds(h, m, s) * 1000 + static_cast<int>(fracSec * 1000), dstDt);
        return false;
    } else {
        memcpy(dst, nullVal, len);
        return true;
    }
    return false;
}

bool parseMinute(char *dst, const mysqlxx::Value &val, DATA_TYPE &dstDt, char* nullVal, size_t len) {
    if(val.empty()){
        memcpy(dst, nullVal, len);
        return true;
    }
    int h, m;
    if (hour(val, h) && minute(val, m)) {
        setter(dst, h * 60 + m, dstDt);
        return false;
    } else {
        memcpy(dst, nullVal, len);
        return true;
    }
    return false;
}

bool parseSecond(char *dst, const mysqlxx::Value &val, DATA_TYPE &dstDt, char* nullVal, size_t len) {
    if(val.empty()){
        memcpy(dst, nullVal, len);
        return true;
    }
    int h, m, s;
    if (hour(val, h) && minute(val, m) && second(val, s)) {
        setter(dst, countSeconds(h, m, s), dstDt);
        return false;
    } else {
        memcpy(dst, nullVal, len);
        return true;
    }
    return false;
}

bool parseDatetime(char *dst, const mysqlxx::Value &val, DATA_TYPE &dstDt, char* nullVal, size_t len){
    if(val.empty()){
        memcpy(dst, nullVal, len);
        return true;
    }
    long long ret;
    if (getUnixTimeStamp(val, ret)) {
        setter(dst, ret, dstDt);
        return false;
    } else {
        memcpy(dst, nullVal, len);
        return true;
    }
    return false;
}

bool parseTimestamp(char *dst, const mysqlxx::Value &val, DATA_TYPE &dstDt, char* nullVal, size_t len) {
    if(val.empty()){
        memcpy(dst, nullVal, len);
        return true;
    }
    long long ret;
    double fracSec = 0.0;
    if (getUnixTimeStamp(val, ret) && fractionalSecond(val, fracSec)) {
        setter(dst, ret * 1000 + static_cast<long long>(fracSec * 1000.0), dstDt);
        return false;
    } else {
        memcpy(dst, nullVal, len);
        return true;
    }
    return false;
}

bool parseNanoTimestamp(char *dst, const mysqlxx::Value &val, DATA_TYPE &dstDt, char* nullVal, size_t len){
    if(val.empty()){
        memcpy(dst, nullVal, len);
        return true;
    }
    long long ret;
    double fracSec = 0.0;
    if (getUnixTimeStamp(val, ret) && fractionalSecond(val, fracSec)) {
        setter(dst, ret * 1000000000 + static_cast<long long>(fracSec * 1000000000), dstDt);
        return false;
    } else {
        memcpy(dst, nullVal, len);
        return true;
    }
    return false;
}

bool parseNanotime(char *dst, const mysqlxx::Value &val, DATA_TYPE &dstDt, char* nullVal, size_t len) {
    if(val.empty()){
        memcpy(dst, nullVal, len);
        return true;
    }
    int h, m, s;
    double fracSec = 0.0;
    if (hour(val, h) && minute(val, m) && second(val, s) && fractionalSecond(val, fracSec)) {
        setter(dst, ((60ll * h + m) * 60ll + s) * 1000000000 + static_cast<long long>(fracSec * 1000000000), dstDt);
        return false;
    } else {
        memcpy(dst, nullVal, len);
        return true;
    }
    return false;
}

static string bin2str(const char* tmp, size_t len, size_t maxStrLen)
{
    string data;
    unsigned char mask = 0x80;
    for (size_t i = 0; i < len; i++){
        for (short j = 0; j < 8; j++)
            if (tmp[i] & (mask >> j)){
                data += '1';
            }
            else
                data += '0';
    }
    return data.substr(len * 8 - maxStrLen);
}

bool parseBit(char *dst, const mysqlxx::Value &val, DATA_TYPE &dstDt, char* nullVal, size_t len, size_t maxStrLen){
    if(dstDt == DT_STRING){
        if (val.empty()){
            (*((char **)dst))[0] = '\0';
            return true;
        }    
        string bitStr(bin2str(val.data(), val.size(), maxStrLen - 1));
        memcpy(*((char **)dst), bitStr.data(), bitStr.size());
        (*((char **)dst))[bitStr.size()] = '\0';
        return false;
    }
    else{
        if(val.empty()){
            memcpy(dst, nullVal, len);
            return true;
        }
        else if(dstDt == DT_BOOL){
            const char *tmp = val.data();
            unsigned char mask = 0xFF;
            for (size_t i = 0; i < val.size(); i++)
                if (tmp[i] & mask){
                    bool bo = true;
                    memcpy(dst, &bo, sizeof(bool));
                    return false;
                }
            memset(dst, 0, len);
            return false;
        }
        else{
            if(len > val.length()){
                size_t emptyLen = len - val.length() ;
                memset(dst, 0, emptyLen);
                memcpy(dst + emptyLen, val.data(), val.length());
            }
            else if(len < val.length()){
                size_t valLen = val.length() - len;
                memcpy(dst, val.data() + valLen, len);
            }
            else{
                memcpy(dst, val.data(), len);
            }
            if(littleEndian){
                for (size_t i = 0; i < (len / 2); i++)
                    std::swap(dst[i], dst[len - 1 - i]);
            }
            return false;
        }
    }
}

//////////////////////////////////// util
// const char *getDolphinDBTypeStr(DATA_TYPE type) {
//     auto str = Util::getDataTypeString(type);
//     char* ptr = (char *)malloc(str.size()+1);
//     memcpy(ptr, str.c_str(), str.size()+1);
//     return (const char*)ptr;

// }

ConstantSP messageSP(const std::string &s) {
    auto message = Util::createConstant(DT_STRING);
    message->setString(s);
    return message;
}

// vector<ConstantSP> getArgs(vector<ConstantSP> &args, size_t nMaxArgs) {
//     auto ret = vector<ConstantSP>(nMaxArgs);
//     for (size_t i = 0; i < nMaxArgs; ++i) {
//         if (args.size() >= i + 1)
//             ret[i] = args[i];
//         else
//             ret[i] = Util::createNullConstant(DT_VOID);
//     }
//     return ret;
// }

DATA_TYPE getDolphinDBType(mysqlxx::enum_field_types type, bool isUnsigned, bool isEnum, int maxStrLen) {
    using namespace mysqlxx;
    switch (type) {
        case MYSQL_TYPE_BIT:
        {
            if(maxStrLen > 0 && maxStrLen <= 8)
                return DT_CHAR;
            else if(maxStrLen <= 16)
                return DT_SHORT;
            else if(maxStrLen <= 32)
                return DT_INT;
            else if(maxStrLen <= 64)
                return DT_LONG;
            else
                throw IllegalArgumentException(__FUNCTION__, "MySQL type " + std::string(getMySQLTypeStr(type)) + " wrong length : " + std::to_string(maxStrLen) + " .");
        }
        case MYSQL_TYPE_TINY:
            return isUnsigned ? DT_SHORT : DT_CHAR;
        case MYSQL_TYPE_SHORT:
            return isUnsigned ? DT_INT : DT_SHORT;
        case MYSQL_TYPE_INT24:
            return DT_INT;
        case MYSQL_TYPE_LONG:
            return isUnsigned ? DT_LONG : DT_INT;
        case MYSQL_TYPE_LONGLONG:
            return DT_LONG;
        case MYSQL_TYPE_DOUBLE:
        case MYSQL_TYPE_DECIMAL:
        case MYSQL_TYPE_NEWDECIMAL:
            return DT_DOUBLE;
        case MYSQL_TYPE_FLOAT:
            return DT_FLOAT;
        case MYSQL_TYPE_DATE:
            return DT_DATE;
        case MYSQL_TYPE_TIME:
            return DT_TIME;
        case MYSQL_TYPE_DATETIME:
            return DT_DATETIME;
        case MYSQL_TYPE_TIMESTAMP:
            return DT_TIMESTAMP;
        case MYSQL_TYPE_YEAR:
            return DT_INT;
        case MYSQL_TYPE_ENUM:
            return DT_SYMBOL;
        case MYSQL_TYPE_STRING:
        case MYSQL_TYPE_VAR_STRING:
        case MYSQL_TYPE_VARCHAR:
            if (isEnum || maxStrLen <= 30 * 3 + 1)    // For string type, mysql use 3 times length
                return DT_SYMBOL;
            else
                return DT_STRING;
        case MYSQL_TYPE_BLOB:
        case MYSQL_TYPE_TINY_BLOB:
        case MYSQL_TYPE_MEDIUM_BLOB:
        case MYSQL_TYPE_LONG_BLOB:
            return DT_STRING;
        default:
            throw IllegalArgumentException(__FUNCTION__, "MySQL type " + std::string(getMySQLTypeStr(type)) + " not supported yet.");
    }
}

const char *getMySQLTypeStr(mysqlxx::enum_field_types mysql_type) {
    using namespace mysqlxx;
#define cm(x) \
    case x:   \
        return #x
    switch (mysql_type) {
        cm(MYSQL_TYPE_DECIMAL);
        cm(MYSQL_TYPE_TINY);
        cm(MYSQL_TYPE_SHORT);
        cm(MYSQL_TYPE_LONG);
        cm(MYSQL_TYPE_FLOAT);
        cm(MYSQL_TYPE_DOUBLE);
        cm(MYSQL_TYPE_NULL);
        cm(MYSQL_TYPE_TIMESTAMP);
        cm(MYSQL_TYPE_LONGLONG);
        cm(MYSQL_TYPE_INT24);
        cm(MYSQL_TYPE_DATE);
        cm(MYSQL_TYPE_TIME);
        cm(MYSQL_TYPE_DATETIME);
        cm(MYSQL_TYPE_YEAR);
        cm(MYSQL_TYPE_NEWDATE);
        cm(MYSQL_TYPE_VARCHAR);
        cm(MYSQL_TYPE_BIT);
        cm(MYSQL_TYPE_TIMESTAMP2);
        cm(MYSQL_TYPE_DATETIME2);
        cm(MYSQL_TYPE_TIME2);
        cm(MYSQL_TYPE_JSON);
        cm(MYSQL_TYPE_NEWDECIMAL);
        cm(MYSQL_TYPE_ENUM);
        cm(MYSQL_TYPE_SET);
        cm(MYSQL_TYPE_TINY_BLOB);
        cm(MYSQL_TYPE_MEDIUM_BLOB);
        cm(MYSQL_TYPE_LONG_BLOB);
        cm(MYSQL_TYPE_BLOB);
        cm(MYSQL_TYPE_VAR_STRING);
        cm(MYSQL_TYPE_STRING);
        cm(MYSQL_TYPE_GEOMETRY);
        cm(MAX_NO_FIELD_TYPES);
        default:
            return "INVALID TYPE";
    }
#undef cm
}

size_t typeLen(DATA_TYPE dt) {
    switch (dt) {
        case DT_BOOL:
            return sizeof(bool);
        case DT_CHAR:
            return sizeof(char);
        case DT_SHORT:
            return sizeof(short);
        case DT_INT:
            return sizeof(int);
        case DT_TIMESTAMP:
        case DT_NANOTIME:
        case DT_NANOTIMESTAMP:
        case DT_LONG:
            return sizeof(long long);
        case DT_DATETIME:
        case DT_DATE:
        case DT_MONTH:
        case DT_TIME:
        case DT_MINUTE:
        case DT_SECOND:
            return sizeof(int);
        case DT_FLOAT:
            return sizeof(float);
        case DT_DOUBLE:
            return sizeof(double);
        case DT_SYMBOL:
        case DT_STRING:
            return sizeof(char *);
        // case DT_VOID:
        // case DT_UUID:
        // case DT_FUNCTIONDEF:
        // case DT_HANDLE:
        // case DT_CODE:
        // case DT_DATASOURCE:
        // case DT_RESOURCE:
        // case DT_ANY:
        // case DT_COMPRESS:
        // case DT_DICTIONARY:
        // case DT_OBJECT:
        default:
                throw RuntimeException("The " + Util::getDataTypeString(dt) + " type is not supported");
    }
}

bool compatible(DATA_TYPE dst, DATA_TYPE src) {
    // could convert any type to STRING
    if (dst == DT_STRING)
        return true;
    if (dst == DT_SYMBOL && src == DT_STRING)
        return true;

    auto c1 = Util::getCategory(dst), c2 = Util::getCategory(src);
    set<DATA_CATEGORY> num{LOGICAL, INTEGRAL, FLOATING};
    // set<DATA_TYPE > time{DT_TIME, DT_TIMESTAMP, DT_NANOTIME,
    // DT_NANOTIMESTAMP}; from TIMESTAMP to DATETIME, DATE, TIME, NANODATETIME,
    // NANOTIMESTAMP, NANOTIME from DATETIME to TIMESTAMP, DATE, TIME,
    // NANODATETIME, NANOTIMESTAMP, NANOTIME {DT_TIMESTAMP, DT_NANOTIME,
    // DT_NANOTIMESTAMP, DT_DATE, DT_MONTH, DT_TIME, DT_MINUTE, DT_SECOND,
    // DT_DATETIME};

    if (src == dst) {
        return true;
    } else if (src == DT_TIMESTAMP && time_type.count(dst)) {
        return true;
    } else if (src == DT_DATETIME && time_type.count(dst)) {
        return true;
    } else if (src == DT_TIME && dst != DT_NANOTIME && dst != DT_MINUTE && dst != DT_SECOND) {
        return false;
    } else if (c1 == c2) {
        return true;
    } else if (num.count(c1) && num.count(c2)) {
        return true;
    }
    return false;
}

void compatible(vector<DATA_TYPE> &dst, vector<DATA_TYPE> &src) {
    if (dst.size() != src.size()) {
        throw IllegalArgumentException(__FUNCTION__, "dst and src schema have different size");
    }
    for (size_t i = 0; i < dst.size(); ++i) {
        std::string errMsg = "Couldn't convert from " + Util::getDataTypeString(src[i]) + " to " + Util::getDataTypeString(dst[i]) + " at column " + std::to_string(i);
        if (!compatible(dst[i], src[i])) {
            throw IllegalArgumentException(__FUNCTION__, errMsg);
        }

        if (time_type.count(dst[i])) {
            src[i] = dst[i];
        }
    }
}

}    // namespace dolphindb