#include "pluginRedis.h"

#include "ScalarImp.h"
#include "ddbplugin/Plugin.h"
#include "hiredis.h"
#include "redisConnection.h"

const string REDIS_CONNECTION_NAME = "redis connection";
const string REDIS_PREFIX = "[Plugin::Redis] BackgroundResourceMap: ";
const vector<string> REDIS_STATUS_COLUMN_NAMES = {"token", "address", "createdTime"};
const vector<DATA_TYPE> REDIS_STATUS_COLUMN_TYPES = {DT_STRING, DT_STRING, DT_DATETIME};

dolphindb::BackgroundResourceMap<RedisConnection> REDIS_HANDLE_MAP(REDIS_PREFIX, REDIS_CONNECTION_NAME);

static void doNothingOnClose(Heap *heap, vector<ConstantSP> &args) {
    // do nothing
}

static void checkHandle(const ConstantSP &handle) {
    if (handle->getType() != DT_RESOURCE || handle->getString() != REDIS_CONNECTION_NAME) {
        throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] First argument must be a redis handle.");
    }
}

static void checkHandleValid(const SmartPointer<RedisConnection> &handle) {
    if (handle.isNull()) {
        throw RuntimeException("[Plugin::Redis] Invalid redis handle.");
    }
}

ConstantSP redisPluginConnect(Heap *heap, const vector<ConstantSP> &args) {
    if (args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__,
                                       "[Plugin::Redis] Usage: connect(host, port), host must be string type.");
    }
    if (args[1]->getType() != DT_INT || args[1]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__,
                                       "[Plugin::Redis] Usage: connect(host, port), port must be int type.");
    }

    redisContext *conn = redisConnect(args[0]->getString().c_str(), args[1]->getInt());
    if (!conn) {
        throw RuntimeException("[Plugin::Redis] Redis connection error: can't allocate redis context.");
    }
    if (conn->err) {
        string errMsg = "[Plugin::Redis] Redis connection error: " + string(conn->errstr);
        redisFree(conn);
        throw RuntimeException(errMsg);
    }

    SmartPointer<RedisConnection> redisHandler = new RedisConnection(conn, args[0]->getString(), args[1]->getInt());
    FunctionDefSP onClose(Util::createSystemProcedure("redis connection onClose()", doNothingOnClose, 1, 1));
    ConstantSP resource = Util::createResource(reinterpret_cast<long long>(redisHandler.get()), REDIS_CONNECTION_NAME,
                                               onClose, heap->currentSession());
    REDIS_HANDLE_MAP.safeAdd(resource, redisHandler, std::to_string(reinterpret_cast<long long>(redisHandler.get())));
    return resource;
}

ConstantSP redisPluginRun(Heap *heap, const vector<ConstantSP> &args) {
    checkHandle(args[0]);
    SmartPointer<RedisConnection> redisHandler = REDIS_HANDLE_MAP.safeGet(args[0]);
    checkHandleValid(redisHandler);
    return redisHandler->redisRun(args);
}

// TODO: use pipeline (redisAppendCommandArgv) to optimize it
ConstantSP redisPluginBatchSet(Heap *heap, const vector<ConstantSP> &args) {
    if (args[1]->isScalar() && args[2]->isScalar()) {
        return redisPluginRun(heap, {args[0], new String("SET"), args[1], args[2]});
    }

    checkHandle(args[0]);
    SmartPointer<RedisConnection> redisHandler = REDIS_HANDLE_MAP.safeGet(args[0]);
    checkHandleValid(redisHandler);
    return redisHandler->redisBatchSet(args);
}

ConstantSP redisPluginBatchHashSet(Heap *heap, const vector<ConstantSP> &args) {
    checkHandle(args[0]);
    SmartPointer<RedisConnection> redisHandler = REDIS_HANDLE_MAP.safeGet(args[0]);
    checkHandleValid(redisHandler);
    return redisHandler->redisBatchHashSet(args);
}

ConstantSP redisPluginRelease(const ConstantSP &handle) {
    checkHandle(handle);
    REDIS_HANDLE_MAP.safeRemove(handle);
    return new String("release finish.");
}

ConstantSP redisPluginReleaseAll() {
    REDIS_HANDLE_MAP.clear();
    return new String("releaseAll finish.");
}

ConstantSP redisGetHandle(Heap *heap, const vector<ConstantSP> &args) {
    if (args[0]->getForm() != DF_SCALAR || args[0]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] First argument must be a string scalar.");
    }
    return REDIS_HANDLE_MAP.getHandleByName(args[0]->getString());
}

ConstantSP redisGetHandleStaus() {
    const vector<string> &names = REDIS_HANDLE_MAP.getHandleNames();
    int num = names.size();
    TableSP statusTable = Util::createTable(REDIS_STATUS_COLUMN_NAMES, REDIS_STATUS_COLUMN_TYPES, 0, num);

    SmartPointer<RedisConnection> redisHandler;
    vector<ConstantSP> row(3);
    INDEX insertedRows;
    string errMsg;
    for (int i = 0; i < num; ++i) {
        redisHandler = REDIS_HANDLE_MAP.safeGetByName(names[i]);
        checkHandleValid(redisHandler);

        row[0] = new String(std::to_string(reinterpret_cast<long long>(redisHandler.get())));
        row[1] = new String(redisHandler->getAddress());
        row[2] = redisHandler->getCreatedTime().getValue();
        if (!statusTable->append(row, insertedRows, errMsg)) {
            throw RuntimeException("[Plugin::Redis] getStatus failed: " + errMsg);
        }
    }

    return statusTable;
}

ConstantSP redisBatchPush(Heap *heap, const vector<ConstantSP> &args) {
    checkHandle(args[0]);
    SmartPointer<RedisConnection> redisHandler = REDIS_HANDLE_MAP.safeGet(args[0]);
    checkHandleValid(redisHandler);
    return redisHandler->redisBatchPush(args);
}
