#include "plugin.h"
#include "PluginHbase.h"

#include "DolphinDBEverything.h"
#include "CoreConcept.h"
#include "ddbplugin/CommonInterface.h"

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/protocol/TCompactProtocol.h>
#include <thrift/transport/TTransportUtils.h>

namespace {

const string HBASE_PREFIX = "[Plugin::HBase]";
const string HBASE_CONNECTION_DESC = "hbase connection";

bool isNullOrNothing(const ConstantSP &arg) {
    return arg == nullptr || arg->isNull() || arg->isNothing();
}

} // namespace

ddb::ResourceMap<HBaseConnect> HBASE_CONNECTION_MAP(HBASE_PREFIX, HBASE_CONNECTION_DESC);
ConstantSP hbase_connect(Heap *heap, vector<ConstantSP> &args)
{
    string usage = "Usage: connect(host, port, [isFramed], [timeout]). ";

    HBaseTransportMode transportMode = HBaseTransportMode::Auto;
    int timeout = 5000;//default is 5000ms

    if (args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "host must be a string!");
    }
    if (args[1]->getType() != DT_INT || args[1]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "port must be an integer!");
    }
    if (args.size() >= 3) {
        if (!args[2]->isNull()) {
            if (args[2]->getType() != DT_BOOL || args[2]->getForm() != DF_SCALAR) {
                throw IllegalArgumentException(__FUNCTION__, usage + "isFramed must be a bool scalar.");
            }
            transportMode = args[2]->getBool() ? HBaseTransportMode::Framed : HBaseTransportMode::Buffered;
        }
    }
    if (args.size() == 4) {
        if (args[3]->getType() != DT_INT || args[3]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, usage + "timeout must be an integer!");
        }
        timeout = args[3]->getInt();
    }

    SmartPointer<HBaseConnect> conn = new HBaseConnect(args[0]->getString(), args[1]->getInt(), transportMode, timeout);
    FunctionDefSP onClose(Util::createSystemProcedure(
            "hbase connection onClose()", connectionOnCloseH, 1, 1));
    ConstantSP resource = Util::createResource(reinterpret_cast<long long>(conn.get()), HBASE_CONNECTION_DESC, onClose, heap->currentSession());
    HBASE_CONNECTION_MAP.safeAdd(resource, conn);

    return resource;
}

ConstantSP hbase_show_tables(Heap *heap, vector<ConstantSP> &args)
{
    std::ignore = heap;
    string usage = "Usage: showTables(conn). ";

    auto conn = HBASE_CONNECTION_MAP.safeGet(args[0]);
    return conn->showTablesH();
}

ConstantSP hbase_load(Heap *heap, vector<ConstantSP> &args)
{
    std::ignore = heap;
    string usage = "Usage: load(conn, tableName, [schema], [scanOptions]). ";

    auto conn = HBASE_CONNECTION_MAP.safeGet(args[0]);

    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "tableName must be a string!");
    }
    const string &tableName = args[1]->getString();
    ConstantSP schema;
    ConstantSP scanOptionsArg;

    if (args.size() > 2 && !isNullOrNothing(args[2])) {
        if (args[2]->getForm() == DF_TABLE) {
            schema = args[2];
        } else {
            throw IllegalArgumentException(__FUNCTION__, usage + "schema must be a table or null.");
        }
    }

    if (args.size() > 3 && !isNullOrNothing(args[3])) {
        if (args[3]->getForm() != DF_DICTIONARY) {
            throw IllegalArgumentException(__FUNCTION__, usage + "scanOptions must be a dictionary or null.");
        }
        scanOptionsArg = args[3];
    }

    if (schema.isNull()) {
        if (scanOptionsArg.isNull()) {
            return conn->loadH(tableName);
        }
        auto scanOptions = HBaseConnect::parseScanOptions(scanOptionsArg);
        return conn->loadH(tableName, TableSP(), scanOptions);
    }

    auto scanOptions = HBaseConnect::parseScanOptions(scanOptionsArg);
    return conn->loadH(tableName, schema, scanOptions);
}

ConstantSP hbase_delete_table(Heap *heap, vector<ConstantSP> &args)
{
    std::ignore = heap;
    string usage = "Usage: deleteTable(conn, tableNames). ";

    auto conn = HBASE_CONNECTION_MAP.safeGet(args[0]);

    if ((args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR) && args[1]->getForm() != DF_VECTOR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "tableName must be a string or string vector!");
    }
    if (args[1]->getForm() == DF_VECTOR) {
        for (int i = 0; i < args[1]->size(); ++i) {
            conn->deleteTableH(args[1]->getString(i));
        }
    } else {
        conn->deleteTableH(args[1]->getString());
    }
    return new Void();
}

ConstantSP hbase_get_row(Heap *heap, vector<ConstantSP> &args)
{
    std::ignore = heap;
    string usage = "Usage: getRow(conn, tableName, rowKey, [columnNames]). ";

    vector<string> columnNames;
    auto conn = HBASE_CONNECTION_MAP.safeGet(args[0]);

    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "tableName must be a string!");
    }
    if (args[2]->getType() != DT_STRING || args[2]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "rowKey must be a string!");
    }
    if (args.size() == 4) {
        if (args[3]->getType() != DT_STRING && (args[3]->getForm() != DF_SCALAR || args[3]->getForm() != DF_VECTOR))
            throw IllegalArgumentException(__FUNCTION__, usage + "columnName must be a string or string vector!");
        if (args[3]->getForm() == DF_SCALAR) {
            columnNames.emplace_back(args[3]->getString());
        } else {
            int columnSize = args[3]->size();
            for (int i = 0; i < columnSize; ++i) {
                columnNames.emplace_back(args[3]->getString(i));
            }
        }
    }

    return conn->getRowH(args[1]->getString(), args[2]->getString(), columnNames);
}
