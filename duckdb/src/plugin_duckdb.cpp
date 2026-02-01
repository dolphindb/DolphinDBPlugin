#include "plugin_duckdb.h"

#include "DecimalUtil.h"
#include "ddbplugin/Plugin.h"

using dolphindb::Connection;
using dolphindb::messageSP;
using std::cout;
using std::endl;

namespace dolphindb {

// Helper function for safe operations on connection
ConstantSP safeOp(const ConstantSP &arg, std::function<ConstantSP(Connection *)> &&f) {
    if (arg->getType() == DT_RESOURCE) {
        string desc = arg->getString();
        if (desc.find("duckdb connection") != 0) {
            throw IllegalArgumentException(__FUNCTION__, "Invalid connection object.");
        }
        if (arg->getLong() == 0) {
            throw IllegalArgumentException(__FUNCTION__, "Invalid connection handle.");
        }
        auto conn = (Connection *)(arg->getLong());
        if (conn->isClosed()) {
            throw RuntimeException("Invalid connection object.");
        }
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

ConstantSP messageSP(const std::string &s) {
    return new String(s);
}

// Connection cleanup callback
static void duckdbConnectionOnClose(Heap *heap, vector<ConstantSP> &args) {
    Connection *conn = reinterpret_cast<Connection *>(args[0]->getLong());
    if (conn != nullptr) {
        delete conn;
        args[0]->setLong(0);
    }
}

// Connect to DuckDB database
ConstantSP duckdbConnect(Heap *heap, vector<ConstantSP> &args) {
    std::string usage = "Usage: connect(dbpath). ";
    // Parse args
    if (args.size() != 1) {
        throw IllegalArgumentException(__FUNCTION__, usage + "Requires 1 argument.");
    }
    if (args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "dbpath must be a string");
    }
    if (args[0]->getString().empty()) {
        throw IllegalArgumentException(__FUNCTION__, usage + "dbpath can't be empty");
    }

    std::string dbpath = args[0]->getString();
    std::unique_ptr<Connection> cup(new Connection(dbpath));
    std::string desc = "duckdb connection to [";
    desc.append(cup->str()).append("]");
    dolphindb::littleEndian = Util::isLittleEndian();
    FunctionDefSP onClose(
        Util::createSystemProcedure("duckdb connection onClose()", duckdbConnectionOnClose, 1, 1));
    return Util::createResource((long long)cup.release(), desc.data(), onClose, heap->currentSession());
}

// Close connection
ConstantSP duckdbClose(Heap *heap, vector<ConstantSP> &args) {
    std::string usage = "Usage: close(connection).";
    if (args[0]->getType() != DT_RESOURCE || args[0]->getString().find("duckdb connection") != 0) {
        throw IllegalArgumentException(__FUNCTION__, usage + "Must be a duckdb resource object.");
    }
    if (args[0]->getLong() == 0) {
        throw IllegalArgumentException(__FUNCTION__, "Invalid connection handle.");
    }

    Connection *conn = reinterpret_cast<Connection *>(args[0]->getLong());
    if (conn != nullptr) {
        conn->close();
    }
    return new String("Connection is closed.");
}

// List all tables
ConstantSP duckdbTables(Heap *heap, vector<ConstantSP> &args) {
    return safeOp(args[0], [&](Connection *conn) { return conn->tables(); });
}

// Extract schema
ConstantSP duckdbSchema(Heap *heap, vector<ConstantSP> &args) {
    if (args.size() != 2) {
        throw IllegalArgumentException(__FUNCTION__, "Usage: extractSchema(connection, table_or_query)");
    }
    if (args[1]->getType() != DT_STRING)
        throw IllegalArgumentException(__FUNCTION__,
                                     "Usage: extractSchema(connection, table_or_query). table_or_query must be a string");
    return safeOp(args[0], [&](Connection *conn) { return conn->extractSchema(args[1]->getString()); });
}

// Load data
ConstantSP duckdbLoad(Heap *heap, vector<ConstantSP> &arguments) {
    auto args = getArgs(arguments, 6);
    std::string usage =
        "Usage: load(connection, table_or_query, [schema], [startRow=0], [rowNum=ULONGLONG_MAX], "
        "[allowEmptyTable=FALSE]). ";

    std::string table;
    TableSP schema = nullptr;
    uint64_t startRow = 0, rowNum = std::numeric_limits<uint64_t>::max();
    bool allowEmptyTable = false;

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

    return safeOp(args[0], [&](Connection *conn) {
        return conn->load(table, schema, startRow, rowNum, allowEmptyTable);
    });
}

// Connection implementation
Connection::Connection(std::string dbpath)
    : dbpath_(dbpath), isClosed_(false) {
    try {
        database_ = duckdb::make_uniq<duckdb::DuckDB>(dbpath);
        connection_ = duckdb::make_uniq<duckdb::Connection>(*database_);
    } catch (const std::exception &e) {
        throw RuntimeException("Failed to connect to DuckDB: " + std::string(e.what()));
    }
}

Connection::~Connection() {
    close();
}

void Connection::close() {
    if (!isClosed_) {
        connection_.reset();
        database_.reset();
        isClosed_ = true;
    }
}

bool Connection::isQuery(const std::string &str) {
    std::string upperStr = str;
    std::transform(upperStr.begin(), upperStr.end(), upperStr.begin(), ::toupper);
    return upperStr.find("SELECT") == 0 || upperStr.find("WITH") == 0 ||
           upperStr.find("EXPLAIN") == 0 || upperStr.find("DESCRIBE") == 0;
}

ConstantSP Connection::doQuery(const std::string &str) {
    try {
        auto result = connection_->Query(str);
        return createTableFromQueryResult(result);
    } catch (const std::exception &e) {
        throw RuntimeException("Query failed: " + std::string(e.what()));
    }
}

ConstantSP Connection::tables() {
    try {
        auto result = connection_->Query("SELECT table_name FROM information_schema.tables WHERE table_schema = 'main'");
        return createTableFromQueryResult(result);
    } catch (const std::exception &e) {
        throw RuntimeException("Failed to list tables: " + std::string(e.what()));
    }
}

ConstantSP Connection::extractSchema(const std::string &tableNameOrQuery) {
    try {
        std::string query;
        if (isQuery(tableNameOrQuery)) {
            query = "SELECT * FROM (" + tableNameOrQuery + ") AS subquery LIMIT 0";
        } else {
            query = "SELECT * FROM " + tableNameOrQuery + " LIMIT 0";
        }

        auto result = connection_->Query(query);
        if (result->HasError()) {
            throw RuntimeException("Failed to extract schema: " + result->GetError());
        }

        vector<duckdb::LogicalType> types = result->types;
        vector<string> names = result->names;

        vector<string> colNames{"name", "type"};
        vector<DATA_TYPE> colTypes{DT_STRING, DT_STRING};
        int size = names.size();

        TableSP table = Util::createTable(colNames, colTypes, size, size);
        VectorSP nameCol = table->getColumn(0);
        VectorSP typeCol = table->getColumn(1);

        for (size_t i = 0; i < names.size(); ++i) {
            nameCol->setString(i, names[i]);
            DATA_TYPE ddbType = getDolphinDBType(types[i]);
            typeCol->setString(i, getDolphinDBTypeStr(ddbType));
        }

        return table;
    } catch (const std::exception &e) {
        throw RuntimeException("Failed to extract schema: " + std::string(e.what()));
    }
}

ConstantSP Connection::createTableFromQueryResult(duckdb::unique_ptr<duckdb::MaterializedQueryResult> &result) {
    if (result->HasError()) {
        throw RuntimeException("Query error: " + result->GetError());
    }

    auto types = result->types;
    auto names = result->names;

    size_t colCount = types.size();

    // Get column types
    vector<DATA_TYPE> colTypes;
    vector<string> colNames;
    for (size_t i = 0; i < colCount; ++i) {
        DATA_TYPE ddbType = getDolphinDBType(types[i]);
        colTypes.push_back(ddbType);
        colNames.push_back(names[i]);
    }

    // Collect all data first
    vector<vector<ConstantSP>> allData(colCount);
    size_t totalRows = 0;

    // Process data in chunks
    while (true) {
        auto dataChunk = result->Fetch();
        if (!dataChunk || dataChunk->size() == 0) {
            break;
        }

        size_t chunkSize = dataChunk->size();
        totalRows += chunkSize;

        for (size_t colIdx = 0; colIdx < colCount; ++colIdx) {
            duckdb::Vector &duckVec = dataChunk->data[colIdx];
            DATA_TYPE ddbType = colTypes[colIdx];

            for (size_t rowIdx = 0; rowIdx < chunkSize; ++rowIdx) {
                duckdb::Value val = duckVec.GetValue(rowIdx);

                if (val.IsNull()) {
                    allData[colIdx].push_back(Util::createNullConstant(ddbType));
                } else {
                    // Convert DuckDB value to DolphinDB value
                    switch (ddbType) {
                    case DT_BOOL:
                        allData[colIdx].push_back(new Bool(val.GetValue<bool>()));
                        break;
                    case DT_CHAR:
                        allData[colIdx].push_back(new Char(static_cast<char>(val.GetValue<int8_t>())));
                        break;
                    case DT_SHORT:
                        allData[colIdx].push_back(new Short(val.GetValue<int16_t>()));
                        break;
                    case DT_INT:
                        allData[colIdx].push_back(new Int(val.GetValue<int32_t>()));
                        break;
                    case DT_LONG:
                        allData[colIdx].push_back(new Long(val.GetValue<int64_t>()));
                        break;
                    case DT_FLOAT:
                        allData[colIdx].push_back(new Float(val.GetValue<float>()));
                        break;
                    case DT_DOUBLE:
                        allData[colIdx].push_back(new Double(val.GetValue<double>()));
                        break;
                    case DT_STRING: {
                        std::string strVal = val.GetValue<std::string>();
                        allData[colIdx].push_back(new String(strVal));
                        break;
                    }
                    case DT_DATE: {
                        duckdb::date_t dateVal = val.GetValue<duckdb::date_t>();
                        int32_t days = dateVal.days;
                        allData[colIdx].push_back(new Int(days));
                        break;
                    }
                    case DT_TIME: {
                        duckdb::dtime_t timeVal = val.GetValue<duckdb::dtime_t>();
                        int32_t microseconds = timeVal.micros;
                        allData[colIdx].push_back(new Int(microseconds));
                        break;
                    }
                    case DT_TIMESTAMP:
                    case DT_DATETIME: {
                        duckdb::timestamp_t tsVal = val.GetValue<duckdb::timestamp_t>();
                        int64_t milliseconds = tsVal.value / 1000;
                        allData[colIdx].push_back(new Long(milliseconds));
                        break;
                    }
                    case DT_NANOTIMESTAMP: {
                        duckdb::timestamp_t tsVal = val.GetValue<duckdb::timestamp_t>();
                        int64_t nanoseconds = tsVal.value * 1000;
                        allData[colIdx].push_back(new Long(nanoseconds));
                        break;
                    }
                    default:
                        allData[colIdx].push_back(Util::createNullConstant(ddbType));
                        break;
                    }
                }
            }
        }
    }

    // Create table with collected data
    vector<ConstantSP> cols(colCount);
    for (size_t i = 0; i < colCount; ++i) {
        cols[i] = Util::createVector(colTypes[i], 0, totalRows);
        VectorSP vec = cols[i];
        for (size_t j = 0; j < totalRows; ++j) {
            vec->set(j, allData[i][j]);
        }
    }

    return Util::createTable(colNames, cols);
}

ConstantSP Connection::load(const std::string &table_or_query, const TableSP &schema,
                            const uint64_t &startRow, const uint64_t &rowNum,
                            const bool &allowEmptyTable) {
    try {
        std::string query;
        if (isQuery(table_or_query)) {
            query = "SELECT * FROM (" + table_or_query + ") AS subquery";
        } else {
            query = "SELECT * FROM " + table_or_query;
        }

        // Add LIMIT and OFFSET for pagination
        if (rowNum != std::numeric_limits<uint64_t>::max()) {
            query += " LIMIT " + std::to_string(rowNum);
        }
        if (startRow > 0) {
            query += " OFFSET " + std::to_string(startRow);
        }

        auto result = connection_->Query(query);
        if (result->HasError()) {
            throw RuntimeException("Query failed: " + result->GetError());
        }

        auto table = createTableFromQueryResult(result);

        if (!allowEmptyTable && table->size() == 0) {
            throw RuntimeException("Table is empty. If you want to load an empty table, please set allowEmptyTable=true.");
        }

        return table;
    } catch (const std::exception &e) {
        throw RuntimeException("Failed to load data: " + std::string(e.what()));
    }
}

// Type conversion functions
DATA_TYPE getDolphinDBType(duckdb::LogicalType duckdb_type) {
    auto id = duckdb_type.id();

    switch (id) {
    case duckdb::LogicalTypeId::BOOLEAN:
        return DT_BOOL;
    case duckdb::LogicalTypeId::TINYINT:
        return DT_CHAR;
    case duckdb::LogicalTypeId::SMALLINT:
        return DT_SHORT;
    case duckdb::LogicalTypeId::INTEGER:
        return DT_INT;
    case duckdb::LogicalTypeId::BIGINT:
        return DT_LONG;
    case duckdb::LogicalTypeId::UTINYINT:
        return DT_SHORT; // Convert unsigned tinyint to short
    case duckdb::LogicalTypeId::USMALLINT:
        return DT_INT; // Convert unsigned smallint to int
    case duckdb::LogicalTypeId::UINTEGER:
        return DT_LONG; // Convert unsigned int to long
    case duckdb::LogicalTypeId::UBIGINT:
        return DT_DOUBLE; // Convert unsigned bigint to double
    case duckdb::LogicalTypeId::FLOAT:
        return DT_FLOAT;
    case duckdb::LogicalTypeId::DOUBLE:
        return DT_DOUBLE;
    case duckdb::LogicalTypeId::VARCHAR:
        return DT_STRING;
    case duckdb::LogicalTypeId::DATE:
        return DT_DATE;
    case duckdb::LogicalTypeId::TIME:
        return DT_TIME;
    case duckdb::LogicalTypeId::TIMESTAMP:
        return DT_TIMESTAMP;
    case duckdb::LogicalTypeId::TIMESTAMP_TZ:
        return DT_NANOTIMESTAMP;
    case duckdb::LogicalTypeId::DECIMAL:
        return DT_DOUBLE; // Convert DECIMAL to DOUBLE
    case duckdb::LogicalTypeId::HUGEINT:
    case duckdb::LogicalTypeId::UHUGEINT:
        return DT_DOUBLE; // Convert huge integers to double
    default:
        return DT_STRING; // Default to string for unknown types
    }
}

const char *getDolphinDBTypeStr(DATA_TYPE dt) {
    switch (dt) {
    case DT_BOOL:
        return "BOOL";
    case DT_CHAR:
        return "CHAR";
    case DT_SHORT:
        return "SHORT";
    case DT_INT:
        return "INT";
    case DT_LONG:
        return "LONG";
    case DT_FLOAT:
        return "FLOAT";
    case DT_DOUBLE:
        return "DOUBLE";
    case DT_STRING:
        return "STRING";
    case DT_DATE:
        return "DATE";
    case DT_TIME:
        return "TIME";
    case DT_TIMESTAMP:
        return "TIMESTAMP";
    case DT_DATETIME:
        return "DATETIME";
    case DT_NANOTIMESTAMP:
        return "NANOTIMESTAMP";
    default:
        return "UNKNOWN";
    }
}

size_t typeLen(DATA_TYPE dt) {
    switch (dt) {
    case DT_BOOL:
        return 1;
    case DT_CHAR:
        return 1;
    case DT_SHORT:
        return 2;
    case DT_INT:
    case DT_FLOAT:
        return 4;
    case DT_LONG:
    case DT_DOUBLE:
    case DT_TIMESTAMP:
    case DT_NANOTIMESTAMP:
        return 8;
    case DT_STRING:
        return sizeof(std::string);
    default:
        return 8;
    }
}

bool compatible(DATA_TYPE dst, DATA_TYPE src) {
    if (dst == src)
        return true;
    if (dst == DT_DOUBLE && (src == DT_FLOAT || src == DT_INT || src == DT_LONG))
        return true;
    if (dst == DT_STRING)
        return true;
    return false;
}

void compatible(vector<DATA_TYPE> &dst, vector<DATA_TYPE> &src) {
    if (dst.size() != src.size()) {
        throw RuntimeException("Column count mismatch");
    }
    for (size_t i = 0; i < dst.size(); ++i) {
        if (!compatible(dst[i], src[i])) {
            throw RuntimeException("Type mismatch at column " + std::to_string(i));
        }
    }
}

} // namespace dolphindb
