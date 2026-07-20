#include "pluginRedis.h"

#include "ScalarImp.h"
#include "ddbplugin/Plugin.h"
#include "hiredis/hiredis.h"
#include "redisConnection.h"
#include "redisStream.h"

const string REDIS_CONNECTION_NAME = "redis connection";
const string REDIS_PREFIX = "[Plugin::Redis] BackgroundResourceMap: ";
const vector<string> REDIS_STATUS_COLUMN_NAMES = {"token", "address", "createdTime", "channel"};
const vector<DATA_TYPE> REDIS_STATUS_COLUMN_TYPES = {DT_STRING, DT_STRING, DT_DATETIME, DT_STRING};
const int MAX_SUBSCRIBE_CHANNEL_NUM = 1024;
Mutex RELEASE_LOCK;

ddb::BackgroundResourceMap<RedisConnection> REDIS_HANDLE_MAP(REDIS_PREFIX, REDIS_CONNECTION_NAME);

static void doNothingOnClose(Heap *, vector<ConstantSP> &) {
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

ConstantSP redisPluginRun(Heap *, const vector<ConstantSP> &args) {
    checkHandle(args[0]);
    SmartPointer<RedisConnection> redisHandler = REDIS_HANDLE_MAP.safeGet(args[0]);
    checkHandleValid(redisHandler);
    return redisHandler->redisRun(args);
}

ConstantSP redisPluginBatchSet(Heap *, const vector<ConstantSP> &args) {
    checkHandle(args[0]);
    SmartPointer<RedisConnection> redisHandler = REDIS_HANDLE_MAP.safeGet(args[0]);
    checkHandleValid(redisHandler);

    if (args[1]->isScalar() && args[2]->isScalar()) {
        return redisHandler->redisRun({args[0], new String("SET"), args[1], args[2]}, "Set");
    }
    return redisHandler->redisBatchSet(args);
}

ConstantSP redisPluginBatchHashSet(Heap *, const vector<ConstantSP> &args) {
    checkHandle(args[0]);
    SmartPointer<RedisConnection> redisHandler = REDIS_HANDLE_MAP.safeGet(args[0]);
    checkHandleValid(redisHandler);
    return redisHandler->redisBatchHashSet(args);
}

ConstantSP redisPluginRelease(Heap *, const vector<ConstantSP> &args) {
    LockGuard<Mutex> _(&RELEASE_LOCK);
    checkHandle(args[0]);
    SmartPointer<RedisConnection> redisHandler = REDIS_HANDLE_MAP.safeGet(args[0]);
    redisHandler->stopListen();
    REDIS_HANDLE_MAP.safeRemove(args[0]);
    return new String("release finish.");
}

ConstantSP redisPluginReleaseAll(Heap *, const vector<ConstantSP> &) {
    LockGuard<Mutex> _(&RELEASE_LOCK);
    const vector<string> &names = REDIS_HANDLE_MAP.getHandleNames();
    int num = names.size();
    for (int i = 0; i < num; ++i) {
        ConstantSP handle = REDIS_HANDLE_MAP.getHandleByName(names[i]);
        checkHandle(handle);
        SmartPointer<RedisConnection> redisHandler = REDIS_HANDLE_MAP.safeGet(handle);
        redisHandler->stopListen();
        REDIS_HANDLE_MAP.safeRemove(handle);
    }
    return new String("releaseAll finish.");
}

ConstantSP redisGetHandle(Heap *, const vector<ConstantSP> &args) {
    if (args[0]->getForm() != DF_SCALAR || args[0]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] First argument must be a string scalar.");
    }
    return REDIS_HANDLE_MAP.getHandleByName(args[0]->getString());
}

ConstantSP redisGetHandleStaus(Heap *, const vector<ConstantSP> &) {
    LockGuard<Mutex> _(&RELEASE_LOCK);
    const vector<string> &names = REDIS_HANDLE_MAP.getHandleNames();
    int num = names.size();
    TableSP statusTable = Util::createTable(REDIS_STATUS_COLUMN_NAMES, REDIS_STATUS_COLUMN_TYPES, 0, num);

    SmartPointer<RedisConnection> redisHandler;
    vector<ConstantSP> row(4);
    INDEX insertedRows;
    string errMsg;
    for (int i = 0; i < num; ++i) {
        redisHandler = REDIS_HANDLE_MAP.safeGetByName(names[i]);
        checkHandleValid(redisHandler);

        row[0] = new String(std::to_string(reinterpret_cast<long long>(redisHandler.get())));
        row[1] = new String(redisHandler->getAddress());
        row[2] = redisHandler->getCreatedTime().getValue();
        row[3] = new String(redisHandler->getSubscriptions());
        if (!statusTable->append(row, insertedRows, errMsg)) {
            throw RuntimeException("[Plugin::Redis] getStatus failed: " + errMsg);
        }
    }
    return statusTable;
}

ConstantSP redisBatchPush(Heap *, const vector<ConstantSP> &args) {
    checkHandle(args[0]);
    SmartPointer<RedisConnection> redisHandler = REDIS_HANDLE_MAP.safeGet(args[0]);
    checkHandleValid(redisHandler);
    return redisHandler->redisBatchPush(args);
}

ConstantSP redisBatchGet(Heap *, const vector<ConstantSP> &args) {
    checkHandle(args[0]);
    SmartPointer<RedisConnection> redisHandler = REDIS_HANDLE_MAP.safeGet(args[0]);
    checkHandleValid(redisHandler);
    return redisHandler->redisBatchGet(args);
}

ConstantSP redisSubscribe(Heap *heap, const vector<ConstantSP> &args) {
    checkHandle(args[0]);
    SmartPointer<RedisConnection> redisHandler = REDIS_HANDLE_MAP.safeGet(args[0]);
    checkHandleValid(redisHandler);

    if (args[1]->getType() != DT_STRING || (args[1]->getForm() != DF_SCALAR && args[1]->getForm() != DF_VECTOR)) {
        throw IllegalArgumentException(__FUNCTION__,
                                       "[Plugin::Redis] Argument channel must be a string scalar or a string vector.");
    }
    if (args[2]->getType() != DT_FUNCTIONDEF || args[2]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument handler must be a function defination.");
    }
    if (reinterpret_cast<FunctionDef *>(args[2].get())->getMaxParamCount() != 2 ||
        reinterpret_cast<FunctionDef *>(args[2].get())->getMinParamCount() != 2) {
        throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument handler must have two arguments.");
    }
    bool usePattern = false;
    if (args.size() > 3 && !args[3]->isNothing()) {
        if (args[3]->isNull()) {
            throw RuntimeException("[Plugin::Redis] Argument isPattern can't be NULL.");
        }
        if (args[3]->getType() != DT_BOOL || args[3]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument isPattern must be a bool scalar.");
        }
        usePattern = args[3]->getBool();
    }
    string password;
    if (args.size() > 4 && !args[4]->isNothing()) {
        if (args[4]->getType() != DT_STRING || args[4]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument password must be a string scalar.");
        }
        password = args[4]->getString();
    }

    int numChannels = args[1]->size();
    vector<string> channels(numChannels);
    if (args[1]->isScalar()) {
        channels[0] = args[1]->getString();
    } else {
        Vector *channelVec = reinterpret_cast<Vector *>(args[1].get());
        for (int i = 0; i < numChannels; ++i) {
            channels[i] = channelVec->getString(i);
        }
    }
    if (numChannels > MAX_SUBSCRIBE_CHANNEL_NUM) {
        throw RuntimeException("[Plugin::Redis] Can't subscribe more than 1024 channels in a single redis handler.");
    }

    redisHandler->subscribe(heap, channels, args[2], usePattern, password);
    return new Void();
}

ConstantSP redisUnsubscribe(Heap *, const vector<ConstantSP> &args) {
    checkHandle(args[0]);
    SmartPointer<RedisConnection> redisHandler = REDIS_HANDLE_MAP.safeGet(args[0]);
    checkHandleValid(redisHandler);

    if (args[1]->getType() != DT_STRING || (args[1]->getForm() != DF_SCALAR && args[1]->getForm() != DF_VECTOR)) {
        throw IllegalArgumentException(__FUNCTION__,
                                       "[Plugin::Redis] Argument channel must be a string scalar or a string vector.");
    }

    int numChannels = args[1]->size();
    vector<string> channels(numChannels);
    if (args[1]->isScalar()) {
        channels[0] = args[1]->getString();
    } else {
        Vector *channelVec = reinterpret_cast<Vector *>(args[1].get());
        for (int i = 0; i < numChannels; ++i) {
            channels[i] = channelVec->getString(i);
        }
    }
    redisHandler->unsubscribe(channels);
    return new Void();
}

ConstantSP redisStreamCreateGroup(Heap *, const vector<ConstantSP> &args) {
    checkHandle(args[0]);
    SmartPointer<RedisConnection> redisHandler = REDIS_HANDLE_MAP.safeGet(args[0]);
    checkHandleValid(redisHandler);

    if (args[1]->getForm() != DF_SCALAR || args[1]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument key must be a string scalar.");
    }
    if (args[2]->getForm() != DF_SCALAR || args[2]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument group must be a string scalar.");
    }
    string msgId = "$";
    if (args.size() > 3 && !args[3]->isNothing()) {
        if (args[3]->getForm() != DF_SCALAR || args[3]->getType() != DT_STRING) {
            throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument msgId must be a string scalar.");
        }
        msgId = args[3]->getString();
    }
    bool mkstream = false;
    if (args.size() > 4 && !args[4]->isNothing()) {
        if (args[4]->getForm() != DF_SCALAR || args[4]->getType() != DT_BOOL) {
            throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument mkStream must be a bool scalar.");
        }
        mkstream = args[4]->getBool();
    }
    return redisHandler->streamCreateGroup(args[1]->getString(), args[2]->getString(), msgId, mkstream);
}

ConstantSP redisStreamReadGroup(Heap *, const vector<ConstantSP> &args) {
    checkHandle(args[0]);
    SmartPointer<RedisConnection> redisHandler = REDIS_HANDLE_MAP.safeGet(args[0]);
    checkHandleValid(redisHandler);

    if (args[1]->getForm() != DF_SCALAR || args[1]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument key must be a string scalar.");
    }
    if (args[2]->getForm() != DF_SCALAR || args[2]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument group must be a string scalar.");
    }
    if (args[3]->getForm() != DF_SCALAR || args[3]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument consumer must be a string scalar.");
    }
    int count = 1;
    if (args.size() > 4 && !args[4]->isNothing()) {
        if (args[4]->getForm() != DF_SCALAR || args[4]->getType() != DT_INT) {
            throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument count must be an int scalar.");
        }
        count = args[4]->getInt();
    }
    bool pending = false;
    if (args.size() > 5 && !args[5]->isNothing()) {
        if (args[5]->getForm() != DF_SCALAR || args[5]->getType() != DT_STRING) {
            throw IllegalArgumentException(__FUNCTION__,
                                           "[Plugin::Redis] Argument readPending must be a string scalar.");
        }
        string pendingStr = args[5]->getString();
        if (pendingStr == "0") {
            pending = true;
        } else if (pendingStr != ">") {
            throw RuntimeException("[Plugin::Redis] Argument pending must be 0 or >.");
        }
    }
    int block = 0;
    if (args.size() > 6 && !args[6]->isNothing()) {
        if (args[6]->getForm() != DF_SCALAR || args[6]->getType() != DT_INT) {
            throw IllegalArgumentException(__FUNCTION__,
                                           "[Plugin::Redis] Argument maxWaitTime must be an int scalar.");
        }
        block = args[6]->getInt();
    }
    bool writePending = false;
    if (args.size() > 7 && !args[7]->isNothing()) {
        if (args[7]->getForm() != DF_SCALAR || args[7]->getType() != DT_BOOL) {
            throw IllegalArgumentException(__FUNCTION__,
                                           "[Plugin::Redis] Argument writeToPending must be a bool scalar.");
        }
        writePending = args[7]->getBool();
    }
    if (pending && (block > 0 || writePending)) {
        throw IllegalArgumentException(__FUNCTION__,
                                       "[Plugin::Redis] Can't set block or writeToPending when pending is true.");
    }

    return redisHandler->streamReadGroup(args[1]->getString(), args[2]->getString(), args[3]->getString(), count,
                                         pending, block, writePending);
}

ConstantSP redisStreamAck(Heap *, const vector<ConstantSP> &args) {
    checkHandle(args[0]);
    SmartPointer<RedisConnection> redisHandler = REDIS_HANDLE_MAP.safeGet(args[0]);
    checkHandleValid(redisHandler);

    if (args[1]->getForm() != DF_SCALAR || args[1]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument key must be a string scalar.");
    }
    if (args[2]->getForm() != DF_SCALAR || args[2]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument group must be a string scalar.");
    }
    if (args[3]->getForm() != DF_VECTOR || args[3]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument msgIds must be a string vector.");
    }
    return redisHandler->streamAck(args[1]->getString(), args[2]->getString(), args[3]);
}

ConstantSP redisStreamSubscribe(Heap *heap, const vector<ConstantSP> &args) {
    if (args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument host must be a string scalar.");
    }
    if (args[1]->getType() != DT_INT || args[1]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument port must be a int scalar.");
    }
    if (args[2]->getType() != DT_STRING || args[2]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument key must be a string scalar.");
    }
    if (args[3]->getType() != DT_STRING || args[3]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument group must be a string scalar.");
    }
    if (args[4]->getType() != DT_STRING || args[4]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument consumer must be a string scalar.");
    }
    if (!args[5]->isTable()) {
        throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument outputTable must be a table.");
    }
    if (args[6]->getType() != DT_FUNCTIONDEF || args[6]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument parser must be a function definition.");
    }
    FunctionDef *handler = reinterpret_cast<FunctionDef *>(args[6].get());
    if (handler->getMaxParamCount() != 1 || handler->getMinParamCount() != 1) {
        throw IllegalArgumentException(__FUNCTION__,
                                       "[Plugin::Redis] Handler function must have exactly one argument.");
    }
    if (args[7]->getType() != DT_STRING || args[7]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument actionName must be a string");
    }
    bool pending = false;
    if (args.size() > 8 && !args[8]->isNothing()) {
        if (args[8]->getForm() != DF_SCALAR || args[8]->getType() != DT_STRING) {
            throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument readPending must be a string scalar.");
        }
        string pendingStr = args[8]->getString();
        if (pendingStr == "0") {
            pending = true;
        } else if (pendingStr != ">") {
            throw RuntimeException("[Plugin::Redis] Argument pending must be 0 or >.");
        }
    }
    int batchSize = 100;
    if (args.size() > 9 && !args[9]->isNothing()) {
        if (args[9]->getType() != DT_INT || args[9]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument batchSize must be a int scalar.");
        }
        batchSize = args[9]->getInt();
    }
    bool autoAck = true;
    if (args.size() > 10 && !args[10]->isNothing()) {
        if (args[10]->getType() != DT_BOOL || args[10]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument autoAck must be a bool scalar.");
        }
        autoAck = args[10]->getBool();
    }
    string password;
    if (args.size() > 11 && !args[11]->isNothing()) {
        if (args[11]->getType() != DT_STRING || args[11]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument password must be a string scalar.");
        }
        password = args[11]->getString();
    }
    return redisStreamSubscribeImpl(heap, args[0]->getString(), args[1]->getInt(), args[2]->getString(),
                                         args[3]->getString(), args[4]->getString(), args[5], args[6],
                                         args[7]->getString(), pending, batchSize, autoAck, password);
}

ConstantSP redisStreamGetSubscribeStat(Heap *, const vector<ConstantSP> &) {
    return redisGetStreamJobStatImpl();
}

ConstantSP redisStreamUnsubscribe(Heap *, const vector<ConstantSP> &args) {
    if (args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument actionName must be a string scalar.");
    }
    return redisStreamUnsubscribeImpl(args[0]->getString());
}
