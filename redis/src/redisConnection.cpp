#include "redisConnection.h"

#include <ctime>

const int BUFFER_SIZE = 4096;

class RedisReplyGuard {
  public:
    RedisReplyGuard(redisReply *r) : reply_(r) {}

    ~RedisReplyGuard() {
        if (reply_ != nullptr) {
            freeReplyObject(reply_);
        }
    }

  private:
    redisReply *reply_ = nullptr;
};

static ConstantSP convertRedisReply(const redisReply *const reply) {
    if (reply == nullptr) {
        return new Void();
    }

    switch (reply->type) {
        case REDIS_REPLY_ERROR:
            throw RuntimeException("[Plugin::Redis] Redis reply error: " + string(reply->str));

        case REDIS_REPLY_STRING:
        case REDIS_REPLY_STATUS:
        case REDIS_REPLY_BIGNUM:
        case REDIS_REPLY_VERB:
            return new String(DolphinString(reply->str, reply->len));

        case REDIS_REPLY_NIL:
            return new Void();

        case REDIS_REPLY_INTEGER:
            return new Long(reply->integer);

        case REDIS_REPLY_DOUBLE:
            return new Double(reply->dval);

        case REDIS_REPLY_BOOL:
            return new Bool(reply->integer);

        case REDIS_REPLY_ARRAY: {
            size_t size = reply->elements;
            VectorSP vec = Util::createVector(DT_ANY, size);
            for (size_t i = 0; i < size; i++) {
                vec->set(i, convertRedisReply(reply->element[i]));
            }
            return vec;
        }

        default:
            throw RuntimeException("[Plugin::Redis] Not support this redis reply type: " + std::to_string(reply->type) +
                                   ".");
    }
}

RedisConnection::RedisConnection(redisContext *redisConnection, const string &ip, const int &port)
    : redisConnect_(redisConnection) {
    address_ = ip + ":" + std::to_string(port);

    std::time_t t = std::time(0);
    std::tm *now = std::localtime(&t);
    if (now) {
        datetime_ =
            DateTime(1900 + now->tm_year, 1 + now->tm_mon, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
    }
}

RedisConnection::~RedisConnection() {
    if (redisConnect_) {
        redisFree(redisConnect_);
    }
}

ConstantSP RedisConnection::redisRun(const vector<ConstantSP> &args) {
    size_t sz = args.size();
    for (size_t i = 1; i < sz; i++) {
        if (args[i]->getForm() != DF_SCALAR || args[i]->getType() != DT_STRING) {
            throw IllegalArgumentException(
                __FUNCTION__, "[Plugin::Redis] argument " + std::to_string(i + 1) + " must be a string scalar.");
        }
    }
    LockGuard<Mutex> guard(&redisMutex_);

    int argsLen = sz - 1;
    vector<string> strings;
    strings.reserve(argsLen);
    for (size_t i = 1; i < sz; i++) {
        strings.emplace_back(args[i]->getString());
    }

    vector<size_t> argvlen;
    argvlen.reserve(argsLen);
    vector<const char *> argv;
    argv.reserve(argsLen);
    for (size_t i = 0; i < strings.size(); i++) {
        argv.push_back(strings[i].c_str());
        argvlen.push_back(strings[i].size());
    }

    redisReply *reply = static_cast<redisReply *>(redisCommandArgv(
        redisConnect_, argsLen, static_cast<const char **>(argv.data()), static_cast<const size_t *>(argvlen.data())));
    if (redisConnect_->err) {
        throw RuntimeException(
            "[Plugin::Redis] Execute command failed: " + string(redisConnect_->errstr) +
            ", this connection cannot be reused and you should release it and create a new connection.");
    }

    RedisReplyGuard replyGuard(reply);
    return convertRedisReply(reply);
}

// TODO: use pipeline (redisAppendCommandArgv) to optimize it
ConstantSP RedisConnection::redisBatchSet(const vector<ConstantSP> &args) {
    if (args[1]->getForm() != DF_VECTOR || args[1]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument keys must be a string vector.");
    }
    if (args[2]->getForm() != DF_VECTOR || args[2]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument values must be a string vector.");
    }
    if (args[1]->size() != args[2]->size()) {
        throw IllegalArgumentException(__FUNCTION__,
                                       "[Plugin::Redis] Argument keys and values must have the same size.");
    }
    LockGuard<Mutex> guard(&redisMutex_);

    char *keysBuffer[BUFFER_SIZE];
    char *valuesBuffer[BUFFER_SIZE];
    int sz = args[1]->size();
    int rest = sz, nread = 0, read = 0;
    for (; rest > 0; nread += read, rest = sz - nread) {
        if (rest >= BUFFER_SIZE) {
            args[1]->getString(nread, BUFFER_SIZE, keysBuffer);
            args[2]->getString(nread, BUFFER_SIZE, valuesBuffer);
            read = BUFFER_SIZE;
        } else {
            args[1]->getString(nread, rest, keysBuffer);
            args[2]->getString(nread, rest, valuesBuffer);
            read = rest;
        }

        for (int i = 0; i < read; i++) {
            const char *setArgv[3] = {"SET", keysBuffer[i], valuesBuffer[i]};
            const size_t setArgvLen[3] = {3, strlen(keysBuffer[i]), strlen(valuesBuffer[i])};
            redisReply *reply = static_cast<redisReply *>(redisCommandArgv(redisConnect_, 3, setArgv, setArgvLen));
            if (redisConnect_->err) {
                throw RuntimeException(
                    "[Plugin::Redis] Execute command failed: " + string(redisConnect_->errstr) +
                    ", this connection cannot be reused and you should release it and create a new connection.");
            }

            RedisReplyGuard replyGuard(reply);
            if (reply->type != REDIS_REPLY_STATUS || strncmp(reply->str, "OK", 2) != 0) {
                throw RuntimeException("[Plugin::Redis] Set failed: " + string(reply->str) + ".");
            }
        }
    }
    return new String("batchSet finish.");
}

ConstantSP RedisConnection::redisBatchHashSet(const vector<ConstantSP> &args) {
    if (args[1]->getForm() != DF_VECTOR || args[1]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument idCol must be a string vector.");
    }
    if (!args[2]->isTable() || ((Table *)args[2].get())->getTableType() != BASICTBL) {
        throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument tb must be a basic table.");
    }
    if (args[1]->size() != args[2]->size()) {
        throw IllegalArgumentException(__FUNCTION__,
                                       "[Plugin::Redis] Arguments idCol and tb must have the same num of rows.");
    }
    LockGuard<Mutex> guard(&redisMutex_);

    DolphinString *idBuffer[BUFFER_SIZE];
    DolphinString *valueBuffer[BUFFER_SIZE];
    DolphinString *valueBuffer2[BUFFER_SIZE];
    DolphinString *valueBuffer3[BUFFER_SIZE];
    DolphinString *valueBuffer4[BUFFER_SIZE];
    int numCols = ((Table *)args[2].get())->columns();
    for (int colIndex = 0; colIndex < numCols;) {
        bool useBatch = numCols - colIndex >= 4;
        INDEX start = 0;
        INDEX len = args[1]->size();
        VectorSP idCol = args[1];
        while (start < len) {
            int count = std::min(len - start, BUFFER_SIZE);
            DolphinString **ids = idCol->getStringConst(start, count, idBuffer);

            if (useBatch) {
                VectorSP col = ((Table *)args[2].get())->getColumn(colIndex);
                VectorSP col2 = ((Table *)args[2].get())->getColumn(colIndex + 1);
                VectorSP col3 = ((Table *)args[2].get())->getColumn(colIndex + 2);
                VectorSP col4 = ((Table *)args[2].get())->getColumn(colIndex + 3);
                if (col->getType() != DT_STRING || col2->getType() != DT_STRING || col3->getType() != DT_STRING ||
                    col4->getType() != DT_STRING) {
                    throw RuntimeException("[Plugin::Redis] The type of field column need to be string.");
                }
                const char *fieldName = ((Table *)args[2].get())->getColumnName(colIndex).c_str();
                const char *fieldName2 = ((Table *)args[2].get())->getColumnName(colIndex + 1).c_str();
                const char *fieldName3 = ((Table *)args[2].get())->getColumnName(colIndex + 2).c_str();
                const char *fieldName4 = ((Table *)args[2].get())->getColumnName(colIndex + 3).c_str();
                DolphinString **values = col->getStringConst(start, count, valueBuffer);
                DolphinString **values2 = col2->getStringConst(start, count, valueBuffer2);
                DolphinString **values3 = col3->getStringConst(start, count, valueBuffer3);
                DolphinString **values4 = col4->getStringConst(start, count, valueBuffer4);
                for (int i = 0; i < count; ++i) {
                    vector<const char *> argv(10);
                    vector<size_t> argvlen(10);
                    argv[0] = "HSET";
                    argvlen[0] = 4;
                    argv[1] = ids[i]->c_str();
                    argvlen[1] = strlen(argv[1]);

                    argv[2] = fieldName;
                    argvlen[2] = strlen(argv[2]);
                    argv[3] = values[i]->c_str();
                    argvlen[3] = strlen(argv[3]);

                    argv[4] = fieldName2;
                    argvlen[4] = strlen(argv[4]);
                    argv[5] = values2[i]->c_str();
                    argvlen[5] = strlen(argv[5]);

                    argv[6] = fieldName3;
                    argvlen[6] = strlen(argv[6]);
                    argv[7] = values3[i]->c_str();
                    argvlen[7] = strlen(argv[7]);

                    argv[8] = fieldName4;
                    argvlen[8] = strlen(argv[8]);
                    argv[9] = values4[i]->c_str();
                    argvlen[9] = strlen(argv[9]);
                    redisAppendCommandArgv(redisConnect_, 10, &argv[0], &argvlen[0]);
                }
            } else {
                VectorSP col = ((Table *)args[2].get())->getColumn(colIndex);
                if (col->getType() != DT_STRING) {
                    throw RuntimeException("[Plugin::Redis] The type of field column need to be string.");
                }
                const char *fieldName = ((Table *)args[2].get())->getColumnName(colIndex).c_str();
                DolphinString **values = col->getStringConst(start, count, valueBuffer);
                for (int i = 0; i < count; ++i) {
                    vector<const char *> argv(4);  // ["HSET", id, field, value]
                    vector<size_t> argvlen(4);
                    argv[0] = "HSET";
                    argvlen[0] = 4;
                    argv[1] = ids[i]->c_str();
                    argvlen[1] = strlen(argv[1]);

                    argv[2] = fieldName;
                    argvlen[2] = strlen(argv[2]);
                    argv[3] = values[i]->c_str();
                    argvlen[3] = strlen(argv[3]);
                    redisAppendCommandArgv(redisConnect_, 4, &argv[0], &argvlen[0]);
                }
            }

            for (int i = 0; i < count; ++i) {
                redisReply *reply;
                if (redisGetReply(redisConnect_, (void **)&reply) != REDIS_OK) {
                    throw RuntimeException(
                        "[Plugin::Redis] Failed to execute HSET command: " + string(redisConnect_->errstr) +
                        ", this connection cannot be reused and you should release it and create a new connection.");
                }
                if (redisConnect_->err) {
                    throw RuntimeException(
                        "[Plugin::Redis] Execute command failed: " + string(redisConnect_->errstr) +
                        ", this connection cannot be reused and you should release it and create a new connection.");
                }

                RedisReplyGuard replyGuard(reply);
                if (reply->type == REDIS_REPLY_ERROR) {
                    throw RuntimeException("[Plugin::Redis] HSET failed: " + string(reply->str) + ".");
                }
            }
            start += count;
        }
        colIndex = useBatch ? colIndex + 4 : colIndex + 1;
    }
    return new String("batchHashSet finish.");
}

static string checkArgsInBatchPush(const vector<ConstantSP> &args) {
    if (args[1]->getForm() != DF_VECTOR || args[1]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument keys must be a string vector.");
    }

    if (!args[2]->isVector()) {
        throw IllegalArgumentException(__FUNCTION__,
                                       "[Plugin::Redis] Argument values must be a two-level nested string vector.");
    }

    ConstantSP value;
    int size = args[2]->size();
    for (int i = 0; i < size; ++i) {
        value = args[2]->get(i);
        if (!value->isVector() || value->getType() != DT_STRING) {
            throw IllegalArgumentException(__FUNCTION__,
                                           "[Plugin::Redis] Argument values must be a two-level nested string vector.");
        }
        if (value->size() < 1) {
            throw IllegalArgumentException(__FUNCTION__,
                                           "[Plugin::Redis] The num of elements in values must greater than 0.");
        }
    }

    bool pushRight = true;
    if (args.size() > 3) {
        if (args[3]->getForm() != DF_SCALAR || args[3]->getType() != DT_BOOL) {
            throw IllegalArgumentException(__FUNCTION__, "[Plugin::Redis] Argument pushRight must be a bool scalar.");
        }
        pushRight = args[3]->getBool();
    }

    if (args[1]->size() != args[2]->size()) {
        throw IllegalArgumentException(__FUNCTION__,
                                       "[Plugin::Redis] Arguments keys and values must have the same num of elements.");
    }
    return pushRight ? "RPUSH" : "LPUSH";
}

ConstantSP RedisConnection::redisBatchPush(const vector<ConstantSP> &args) {
    string command = checkArgsInBatchPush(args);
    LockGuard<Mutex> guard(&redisMutex_);

    vector<const char *> argv(1);
    vector<size_t> argvlen(1);
    argv[0] = command.c_str();
    argvlen[0] = 5;

    DolphinString *keyBuffer[BUFFER_SIZE];
    DolphinString *valueBuffer[BUFFER_SIZE];

    INDEX len = args[1]->size();
    VectorSP keyVec = args[1];
    VectorSP outValueVec = args[2];
    INDEX start = 0;
    while (start < len) {
        int count = std::min(len - start, BUFFER_SIZE);
        DolphinString **keys = keyVec->getStringConst(start, count, keyBuffer);

        for (int i = 0; i < count; ++i) {
            VectorSP valueVec = outValueVec->get(start + i);
            int valueSize = valueVec->size();
            argv.resize(2 + valueSize);
            argvlen.resize(2 + valueSize);

            argv[1] = keys[i]->c_str();
            argvlen[1] = strlen(argv[1]);

            INDEX len2 = valueSize;
            INDEX start2 = 0;
            while (start2 < len2) {
                int count2 = std::min(len2 - start2, BUFFER_SIZE);
                DolphinString **values = valueVec->getStringConst(start2, count2, valueBuffer);

                for (int j = 0; j < count2; ++j) {
                    argv[2 + start2 + j] = values[j]->c_str();
                    argvlen[2 + start2 + j] = strlen(argv[2 + start2 + j]);
                }
                start2 += count2;
            }
            redisAppendCommandArgv(redisConnect_, argv.size(), &argv[0], &argvlen[0]);
        }

        for (int i = 0; i < count; ++i) {
            redisReply *reply;
            if (redisGetReply(redisConnect_, (void **)&reply) != REDIS_OK) {
                throw RuntimeException(
                    "[Plugin::Redis] Failed to execute " + command + " command: " + string(redisConnect_->errstr) +
                    ", this connection cannot be reused and you should release it and create a new connection");
            }
            if (redisConnect_->err) {
                throw RuntimeException(
                    "[Plugin::Redis] Execute command failed: " + string(redisConnect_->errstr) +
                    ", this connection cannot be reused and you should release it and create a new connection.");
            }

            RedisReplyGuard replyGuard(reply);
            if (reply->type == REDIS_REPLY_ERROR) {
                throw RuntimeException("[Plugin::Redis] " + command + " failed: " + string(reply->str) + ".");
            }
        }
        start += count;
    }
    return new Void();
}
