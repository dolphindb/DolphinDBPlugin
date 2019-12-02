#include "plugin_mysql.h"

using dolphindb::Connection;
using dolphindb::ConnectionSP;
using dolphindb::messageSP;
using std::cout;
using std::endl;

ConstantSP safeOp(const ConstantSP &arg, std::function<ConstantSP(Connection *)> &&f) {
    if (arg->getType() == DT_RESOURCE) {
        auto conn = (Connection *)(arg->getLong());
        return conn->connected() ? f(conn) : messageSP("Not connected yet.");
    } else {
        throw IllegalArgumentException(__FUNCTION__, "Invalid connection object.");
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
    delete (Connection *)(args[0]->getLong());
}

ConstantSP mysqlConnect(Heap *heap, vector<ConstantSP> &args) {
    std::string usage = "Usage: connect(host, port, user, password, db). ";
    // parse args first
    if (args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "host must be a string");
    }
    if (args[1]->getType() != DT_INT || args[1]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "port must be an integer");
    }
    if (args[2]->getType() != DT_STRING || args[2]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "user must be a string");
    }
    if (args[3]->getType() != DT_STRING || args[3]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "password must be a string");
    }
    if (args[4]->getType() != DT_STRING || args[4]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "db must be a string");
    }
    std::unique_ptr<Connection> cup(new Connection(args[0]->getString(), args[1]->getInt(), args[2]->getString(), args[3]->getString(), args[4]->getString()));
    const char *fmt = "mysql connection to [%s]";
    vector<char> descBuf(cup->str().size() + strlen(fmt));
    sprintf(descBuf.data(), fmt, cup->str().c_str());
    FunctionDefSP onClose(Util::createSystemProcedure("mysql connection onClose()", mysqlConnectionOnClose, 1, 1));
    return Util::createResource((long long)cup.release(), descBuf.data(), onClose, heap->currentSession());
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
    auto args = getArgs(arguments, 5);
    std::string usage = "Usage: load(connection, table_or_query, [schema], [startRow=0], [rowNum=ULONGLONG_MAX]). ";

    std::string table;
    TableSP schema = nullptr;
    uint64_t startRow = 0, rowNum = std::numeric_limits<uint64_t>::max();

    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "table_or_query must be a string");
    } else {
        table = args[1]->getString();
    }

    if (!args[2]->isNull()) {
        if (!args[2]->isTable()) {
            throw IllegalArgumentException(__FUNCTION__, usage + "schema must be a table");
        }
        schema = args[2];
    }

    if (!args[3]->isNull()) {
        if ((args[3]->getType() != DT_INT && args[3]->getType() != DT_LONG) || args[3]->getForm() != DF_SCALAR)
            throw IllegalArgumentException(__FUNCTION__, usage + "startRow must be a non-negative integer");
        if (args[3]->getInt() < 0)
            throw IllegalArgumentException(__FUNCTION__, usage + "startRow must be a non-negative integer");
        startRow = args[3]->getLong();
    }

    if (!args[4]->isNull()) {
        if ((args[4]->getType() != DT_INT && args[4]->getType() != DT_LONG) || args[4]->getForm() != DF_SCALAR)
            throw IllegalArgumentException(__FUNCTION__, usage + "rowNum must be a integer");
        if (args[4]->getInt() < 0)
            throw IllegalArgumentException(__FUNCTION__, usage + "rowNum must be a non-negative integer");
        rowNum = args[4]->getLong();
    }

    return safeOp(args[0], [&](Connection *conn) { return conn->load(table, schema, startRow, rowNum); });
}

ConstantSP mysqlLoadEx(Heap *heap, vector<ConstantSP> &arguments) {
    auto args = getArgs(arguments, 8);
    ConstantSP dbHandle = nullptr;
    std::string tableName, tableInMySQL;
    ConstantSP partitionColumns = nullptr;
    TableSP schema = nullptr;
    uint64_t startRow = 0, rowNum = std::numeric_limits<uint64_t>::max();

    std::string usage = "Usage: loadEx(connection,dbHandle,tableName,partitionColumns,tableInMySQL_or_query,[schema],[startRow=0],[rowNum=ULONGLONG_MAX]) ";

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

    if (args[3]->isNull()) {
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

    if (!args[5]->isNull()) {
        if (!args[5]->isTable()) {
            throw IllegalArgumentException(__FUNCTION__, usage + "schema must be a table");
        }
    }
    schema = args[5];

    if (!args[6]->isNull()) {
        if ((args[6]->getType() != DT_INT && args[6]->getType() != DT_LONG) || args[6]->getForm() != DF_SCALAR)
            throw IllegalArgumentException(__FUNCTION__, usage + "startRow must be a non-negative integer");
        if (args[6]->getInt() < 0)
            throw IllegalArgumentException(__FUNCTION__, usage + "startRow must be a non-negative integer");
        startRow = args[6]->getLong();
    }

    if (!args[7]->isNull()) {
        if ((args[7]->getType() != DT_INT && args[7]->getType() != DT_LONG) || args[7]->getForm() != DF_SCALAR)
            throw IllegalArgumentException(__FUNCTION__, usage + "rowNum must be a non-negative integer");
        if (args[7]->getInt() < 0)
            throw IllegalArgumentException(__FUNCTION__, usage + "rowNum must be a non-negative integer");
        rowNum = args[7]->getLong();
    }

    return safeOp(args[0], [&](Connection *conn) { return conn->loadEx(heap, dbHandle, tableName, partitionColumns, tableInMySQL, schema, startRow, rowNum); });
}

/// Connection
namespace dolphindb {
Connection::Connection() {
}
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

ConstantSP Connection::load(const std::string &table_or_query, const TableSP &schema, const uint64_t &startRow, const uint64_t &rowNum) {
    LockGuard<Mutex> lk(&mtx_);
    if (isQuery(table_or_query)) {
        return MySQLExtractor(query(table_or_query)).extract(schema);
    } else {
        return MySQLExtractor(query("SELECT * FROM " + table_or_query + " LIMIT " + std::to_string(startRow) + "," + std::to_string(rowNum))).extract(schema);
    }
}

ConstantSP Connection::loadEx(Heap *heap, const ConstantSP &dbHandle, const std::string &tableName, const ConstantSP &partitionColumns, const std::string &MySQLTableName_or_query,
                              const TableSP &schema, const uint64_t &startRow, const uint64_t &rowNum) {
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
        MySQLExtractor(query(MySQLTableName_or_query)).extractEx(heap, ret, schema);
    } else {
        MySQLExtractor(query("SELECT * FROM " + MySQLTableName_or_query + " LIMIT " + std::to_string(startRow) + "," + std::to_string(rowNum))).extractEx(heap, ret, schema);
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
            colTypes->set(i, new String(getDolphinDBTypeStr(getDolphinDBType(res.typeAt(i), res.isUnsignedAt(i), res.isEnumAt(i), res.maxLengthAt(i)))));
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

TableSP MySQLExtractor::extract(const ConstantSP &schema) {
    try {
        auto res = query_.use();
        prepareForExtract(schema, res);
        TableSP t = Util::createTable(colNames_, dstColTypes_, 0, 0);

        realExtract(res, schema, t, [&](Pack &p) { growTable(t, p); });
        if (t->size() == 0) {
            throw RuntimeException("Empty result table.");
        }
        return t;
    } catch (mysqlxx::Exception &e) {
        throw RuntimeException(std::string(e.name()) + " " + std::string(e.displayText()));
    }
}

void MySQLExtractor::extractEx(Heap *heap, TableSP &t, const ConstantSP &schema) {
    auto res = query_.use();
    prepareForExtract(schema, res);
    realExtract(res, schema, t, [&](Pack &p) { growTableEx(t, p, heap); });
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
    dstColTypes_ = (schema.isNull() || schema->isNull()) ? srcColTypes_ : getDstType(schema);
    if (!compatible(dstColTypes_, srcColTypes_)) {
        throw IllegalArgumentException("extract", "Incompatible input schema");
    }
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

void MySQLExtractor::growTableEx(TableSP &t, Pack &p, Heap *heap) {
    realGrowTable(p, [&](vector<ConstantSP> &cols) {
        TableSP tmpTable = Util::createTable(colNames_, cols);
        vector<ConstantSP> args{t, tmpTable};
        heap->currentSession()->getFunctionDef("append!")->call(heap, args);
    });
}

void MySQLExtractor::realGrowTable(Pack &p, std::function<void(vector<ConstantSP> &)> &&callback) {
    auto buffers = p.data();
    auto &nulls = p.nullIndexies();
    int nCol = p.nCol();
    vector<ConstantSP> cols(nCol);
    int len = p.size();
    for (int idx = 0; idx < nCol; ++idx) {
        if (dstColTypes_[idx] == DT_SYMBOL)
            cols[idx] = Util::createVector(DT_STRING, 0, len);
        else
            cols[idx] = Util::createVector(dstColTypes_[idx], 0, len);

        VectorSP vec = cols[idx];
        char *colBuffer = buffers[idx];
        switch (dstColTypes_[idx]) {
            case DT_BOOL:
                vec->appendBool(colBuffer, len);
                break;
            case DT_CHAR:
                vec->appendChar(colBuffer, len);
                break;
            case DT_SHORT:
                vec->appendShort((short *)colBuffer, len);
                break;
            case DT_INT:
                vec->appendInt((int *)colBuffer, len);
                break;
            case DT_DATETIME:
            case DT_TIMESTAMP:
            case DT_NANOTIME:
            case DT_NANOTIMESTAMP:
            case DT_LONG:
                vec->appendLong((long long *)colBuffer, len);
                break;
            case DT_DATE:
            case DT_MONTH:
            case DT_TIME:
            case DT_MINUTE:
            case DT_SECOND:
                vec->appendInt((int *)colBuffer, len);
                break;
            case DT_FLOAT:
                vec->appendFloat((float *)colBuffer, len);
                break;
            case DT_DOUBLE:
                vec->appendDouble((double *)colBuffer, len);
                break;
            case DT_SYMBOL:
            case DT_STRING:
                vec->appendString((char **)colBuffer, len);
                break;
            default:
                throw NotImplementedException(__FUNCTION__, "todo");
        }
        // set null
        auto &colNulls = nulls[idx];
        if (!colNulls.empty()) {
            colNulls.emplace_back(ULONG_MAX);
            size_t start = colNulls[0];
            for (size_t i = 1; i < colNulls.size(); ++i) {
                if (colNulls[i] != colNulls[i - 1] + 1) {
                    vec->fill(start, colNulls[i - 1] - start + 1, Util::createNullConstant(dstColTypes_[idx]));
                    start = colNulls[i];
                }
            }
        }
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
            t == mysqlxx::MYSQL_TYPE_VARCHAR || t == mysqlxx::MYSQL_TYPE_ENUM) {
            ret[i] += 1;
        } else if (t == mysqlxx::MYSQL_TYPE_BLOB || t == mysqlxx::MYSQL_TYPE_TINY_BLOB ||
                   t == mysqlxx::MYSQL_TYPE_MEDIUM_BLOB || t == mysqlxx::MYSQL_TYPE_LONG_BLOB) {
            ret[i] = 3 * 2048 + 1;
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
        if (t == DT_VOID || t == DT_ANY || cat == NOTHING || cat == MIXED || cat == SYSTEM) {
            throw IllegalArgumentException(__FUNCTION__, "error schema");
        }
    }
    return ret;
}

/// Pack members
Pack::Pack(vector<DATA_TYPE> srcDt, vector<DATA_TYPE> dstDt, vector<size_t> maxStrLen, TableSP &resultTable, size_t cap) {
    init(srcDt, dstDt, resultTable, maxStrLen, cap);
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
    isNull_.resize(nCol_);
    maxStrLen_ = maxStrLen;

    auto rowStorage = getRowStorage(dstDt_, maxStrLen);
    auto allowedRow = DEFAULT_ALLOWED_MEM / rowStorage / DEFAULT_WORKSPACE_SIZE;
    capacity_ = capacity_ < allowedRow ? capacity_ : allowedRow;
    if (capacity_ == 0) {
        throw RuntimeException("Insufficient memory to load even one row, please check your MySQL "
                               "table, rowStorage: " + Util::convert(rowStorage / 1024 / 1024 / 1024) + "M");
    }

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
        }
    }
}

void Pack::clear() {
    size_ = 0;
    for (auto &nullvec : isNull_) {
        nullvec.clear();
    }
}

void Pack::append(const mysqlxx::Row &row) {
    try {
        for (size_t col = 0; col < nCol_; ++col) {
            char *dst = rawBuffers_[col] + size_ * typeLen_[col];
            auto val = row[col];
            bool notNull = true;
            switch (srcDt_[col]) {
                case DT_BOOL:
                    notNull &= parseScalar<bool>(dst, val, dstDt_[col]);
                    break;
                case DT_CHAR:
                    notNull &= parseScalar<char>(dst, val, dstDt_[col]);
                    break;
                case DT_SHORT:
                    notNull &= parseScalar<short>(dst, val, dstDt_[col]);
                    break;
                case DT_INT:
                    notNull &= parseScalar<int>(dst, val, dstDt_[col]);
                    break;
                case DT_LONG:
                    notNull &= parseScalar<long long>(dst, val, dstDt_[col]);
                    break;
                case DT_FLOAT:
                    notNull &= parseScalar<float>(dst, val, dstDt_[col]);
                    break;
                case DT_DOUBLE:
                    notNull &= parseScalar<double>(dst, val, dstDt_[col]);
                    break;
                case DT_TIMESTAMP:
                    notNull &= parseTimestamp(dst, val, dstDt_[col]);
                    break;
                case DT_NANOTIME:
                    notNull &= parseNanotime(dst, val, dstDt_[col]);
                    break;
                case DT_NANOTIMESTAMP:
                    notNull &= parseNanoTimestamp(dst, val, dstDt_[col]);
                    break;
                case DT_DATE:
                    notNull &= parseDate(dst, val, dstDt_[col]);
                    break;
                case DT_MONTH:
                    notNull &= parseMonth(dst, val, dstDt_[col]);
                    break;
                case DT_TIME:
                    notNull &= parseTime(dst, val, dstDt_[col]);
                    break;
                case DT_MINUTE:
                    notNull &= parseMinute(dst, val, dstDt_[col]);
                    break;
                case DT_SECOND:
                    notNull &= parseSecond(dst, val, dstDt_[col]);
                    break;
                case DT_DATETIME:
                    notNull &= parseDatetime(dst, val, dstDt_[col]);
                    break;
                case DT_SYMBOL:
                case DT_STRING:
                    notNull &= parseString(dst, val, maxStrLen_[col]);
                    break;
                default:
                    throw NotImplementedException(__FUNCTION__, "Not yet.");
            }
            if (!notNull) {
                isNull_[col].emplace_back(size_);
            }
        }
        ++size_;
        assert(size_ <= capacity_);
    } catch (mysqlxx::Exception &e) {
        throw RuntimeException("Something wrong happened with mysqlxx: " + std::string(e.name()) + " " + std::string(e.displayText()));
    }
}

Pack::~Pack() {
    for (size_t col = 0; col < nCol_; ++col) {
        if (dstDt_.size() > col && (dstDt_[col] == DT_STRING || dstDt_[col] == DT_SYMBOL)) {
            for (size_t j = 0; j < capacity_; ++j) {
                auto p = (char **)(rawBuffers_[col] + j * sizeof(char *));
                delete[](*p);
            }
        }
        if (rawBuffers_.size() > col)
            delete[] rawBuffers_[col];
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

bool parseDate(char *dst, const mysqlxx::Value &val, DATA_TYPE &dstDt) {
    int y, m, d;
    if (year(val, y) && month(val, m) && day(val, d)) {
        setter(dst, Util::countDays(y, m, d), dstDt);
        return true;
    } else {
        return false;
    }
}

bool parseMonth(char *dst, const mysqlxx::Value &val, DATA_TYPE &dstDt) {
    int y, m;
    if (year(val, y) && month(val, m)) {
        setter(dst, y * 12 + m - 1, dstDt);
        return true;
    } else {
        return false;
    }
}

bool parseTime(char *dst, const mysqlxx::Value &val, DATA_TYPE &dstDt) {
    int h, m, s;
    double fracSec = 0.0;
    if (hour(val, h) && minute(val, m) && second(val, s) && fractionalSecond(val, fracSec)) {
        setter(dst, countSeconds(h, m, s) * 1000 + static_cast<int>(fracSec * 1000), dstDt);
        return true;
    } else {
        return false;
    }
}

bool parseMinute(char *dst, const mysqlxx::Value &val, DATA_TYPE &dstDt) {
    int h, m;
    if (hour(val, h) && minute(val, m)) {
        setter(dst, h * 60 + m, dstDt);
        return true;
    } else {
        return false;
    }
}

bool parseSecond(char *dst, const mysqlxx::Value &val, DATA_TYPE &dstDt) {
    int h, m, s;
    if (hour(val, h) && minute(val, m) && second(val, s)) {
        setter(dst, countSeconds(h, m, s), dstDt);
        return true;
    } else {
        return false;
    }
}

bool parseDatetime(char *dst, const mysqlxx::Value &val, DATA_TYPE &dstDt) {
    long long ret;
    if (getUnixTimeStamp(val, ret)) {
        setter(dst, ret, dstDt);
        return true;
    } else {
        return false;
    }
}

bool parseTimestamp(char *dst, const mysqlxx::Value &val, DATA_TYPE &dstDt) {
    long long ret;
    double fracSec = 0.0;
    if (getUnixTimeStamp(val, ret) && fractionalSecond(val, fracSec)) {
        setter(dst, ret * 1000 + static_cast<long long>(fracSec * 1000.0), dstDt);
        return true;
    } else {
        return false;
    }
}

bool parseNanoTimestamp(char *dst, const mysqlxx::Value &val, DATA_TYPE &dstDt) {
    long long ret;
    double fracSec = 0.0;
    if (getUnixTimeStamp(val, ret) && fractionalSecond(val, fracSec)) {
        setter(dst, ret * 1000000000 + static_cast<long long>(fracSec * 1000000000), dstDt);
        return true;
    } else {
        return false;
    }
}

bool parseNanotime(char *dst, const mysqlxx::Value &val, DATA_TYPE &dstDt) {
    int h, m, s;
    double fracSec = 0.0;
    if (hour(val, h) && minute(val, m) && second(val, s) && fractionalSecond(val, fracSec)) {
        setter(dst, ((60ll * h + m) * 60ll + s) * 1000000000 + static_cast<long long>(fracSec * 1000000000), dstDt);
        return true;
    } else {
        return false;
    }
}

//////////////////////////////////// util
const char *getDolphinDBTypeStr(DATA_TYPE type) {
    return Util::getDataTypeString(type).c_str();
}

ConstantSP messageSP(const std::string &s) {
    auto message = Util::createConstant(DT_STRING);
    message->setString(s);
    return message;
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

DATA_TYPE getDolphinDBType(mysqlxx::enum_field_types type, bool isUnsigned, bool isEnum, int maxStrLen) {
    using namespace mysqlxx;
    switch (type) {
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
        case DT_DATETIME:
        case DT_TIMESTAMP:
        case DT_NANOTIME:
        case DT_NANOTIMESTAMP:
        case DT_LONG:
            return sizeof(long long);
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
        case DT_VOID:
        case DT_UUID:
        case DT_FUNCTIONDEF:
        case DT_HANDLE:
        case DT_CODE:
        case DT_DATASOURCE:
        case DT_RESOURCE:
        case DT_ANY:
        case DT_COMPRESS:
        case DT_DICTIONARY:
        case DT_OBJECT:
        default:
            throw IllegalArgumentException(__FUNCTION__, "type not supported yet.");
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

bool compatible(vector<DATA_TYPE> &dst, vector<DATA_TYPE> &src) {
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
    return true;
}

}    // namespace dolphindb