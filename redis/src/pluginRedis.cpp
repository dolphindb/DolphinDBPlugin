#include "pluginRedis.h"
#include "ScalarImp.h"
#include "hiredis.h"
#include "ddbplugin/Plugin.h"

#include <unordered_set>
#include <ctime>

/* Structures and functions from hiredis.h as follows:
 *
 * structures: redisReply, redisContext
 * functions:  redisCommandArgv(), freeReplyObject(), redisFree() */

using std::string;
using std::vector;
using std::unordered_set;

class RedisConnection;

const int BUFFER_SIZE = 2048;
const string REDIS_CONNECTION_NAME = "redis connection";
const string REDIS_PREFIX = "[Plguin::Redis] BackgroundResourceMap: ";
const vector<string> REDIS_STATUS_COLUMN_NAMES = {"token", "address", "createdTime"};
const vector<DATA_TYPE> REDIS_STATUS_COLUMN_TYPES = {DT_STRING, DT_STRING, DT_DATETIME};

dolphindb::BackgroundResourceMap<RedisConnection> REDIS_HANDLE_MAP(REDIS_PREFIX, REDIS_CONNECTION_NAME);

Mutex redisMutex;

class RedisConnection {
public:
	RedisConnection(redisContext* redisConnection, const string& ip, const int& port) :
			redisConnect_(redisConnection) {
		address_ = ip + ":" + std::to_string(port);

		std::time_t t = std::time(0);
		std::tm* now = std::localtime(&t);
		if (now) {
			datetime_ = DateTime(1900 + now->tm_year, 1 + now->tm_mon, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
		}
	}

	~RedisConnection() {
		if (redisConnect_) {
			redisFree(redisConnect_);
		}
	}

	redisContext* getRawRedisConnection() const {
		return redisConnect_;
	}

	string getAddress() const {
		return address_;
	}

	DateTime getCreatedTime() const {
		return datetime_;
	}

private:
	redisContext* redisConnect_;
	string address_;
	DateTime datetime_;
};

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

static void doNothingOnClose(Heap *heap, vector<ConstantSP>& args) {
	// do nothing
}

ConstantSP redisPluginConnect(Heap *heap, const vector<ConstantSP>& args) {
	if (args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR) {
		throw IllegalArgumentException(__FUNCTION__, "[Plguin::Redis] Usage: connect(host, port), host must be string type.");
	}
	if (args[1]->getType() != DT_INT || args[1]->getForm() != DF_SCALAR) {
		throw IllegalArgumentException(__FUNCTION__, "[Plguin::Redis] Usage: connect(host, port), port must be int type.");
	}

	redisContext *conn = redisConnect(args[0]->getString().c_str(), args[1]->getInt());
	if (!conn || conn->err) {
		if (conn) {
			string errMsg = "[Plguin::Redis] Redis connection error: " + string(conn->errstr);
			redisFree(conn);
			throw RuntimeException(errMsg);
		} else {
			throw RuntimeException("[Plguin::Redis] Redis connection error: can't allocate redis context.");
		}
	}

	SmartPointer<RedisConnection> redisHandler = new RedisConnection(conn, args[0]->getString(), args[1]->getInt());
	FunctionDefSP onClose(Util::createSystemProcedure("redis connection onClose()", doNothingOnClose, 1, 1));
	ConstantSP resource = Util::createResource(reinterpret_cast<long long>(redisHandler.get()),
											   REDIS_CONNECTION_NAME, onClose,
								heap->currentSession());
	REDIS_HANDLE_MAP.safeAdd(resource, redisHandler, std::to_string(reinterpret_cast<long long>(
							 redisHandler.get())));
	return resource;
}

static ConstantSP convertRedisReply(const redisReply * const reply) {
	if (reply == nullptr) {
		return new Void();
	}

	switch (reply->type) {
		case REDIS_REPLY_ERROR:
			throw RuntimeException("[Plguin::Redis] Redis reply error: " + string(reply->str));

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

		case REDIS_REPLY_ARRAY:
		{
			size_t size = reply->elements;
			VectorSP vec = Util::createVector(DT_ANY, size);
			for (size_t i = 0; i < size; i++) {
				vec->set(i, convertRedisReply(reply->element[i]));
			}
			return vec;
		}

		default:
			throw RuntimeException("[Plguin::Redis] Not support this redis reply type: " + std::to_string(reply->type) + ".");
	}
}

ConstantSP redisPluginRun(Heap *heap, const vector<ConstantSP>& args) {
	if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != REDIS_CONNECTION_NAME) {
		throw IllegalArgumentException(__FUNCTION__, "[Plguin::Redis] First argument must be a redis handle.");
	}

	SmartPointer<RedisConnection> redisHandler = REDIS_HANDLE_MAP.safeGet(args[0]);
	if (redisHandler.isNull()) {
		throw RuntimeException("[Plguin::Redis] Invalid redis handle.");
	}
	redisContext* ctx = redisHandler->getRawRedisConnection();

	size_t sz = args.size();
	for (size_t i = 1; i < sz; i++) {
		if (args[i]->getForm() != DF_SCALAR || args[i]->getType() != DT_STRING) {
			throw IllegalArgumentException(__FUNCTION__, "[Plguin::Redis] argument " + std::to_string(i + 1) + " must be a string scalar.");
		}
	}

	int argsLen = sz - 1;
	vector<string> strings;
	strings.reserve(argsLen);
	for (size_t i = 1; i < sz; i++)	{
		strings.emplace_back(args[i]->getString());
	}

	vector<size_t> argvlen;
	argvlen.reserve(argsLen);
	vector<const char *> argv;
	argv.reserve(argsLen);
	for (size_t i = 0; i < strings.size(); i++)	{
		argv.push_back(strings[i].c_str());
		argvlen.push_back(strings[i].size());
	}

	LockGuard<Mutex> guard(&redisMutex);
	redisReply *reply = static_cast<redisReply *>(redisCommandArgv(ctx, argsLen, static_cast<const char **>(
												  argv.data()), static_cast<const size_t *>(argvlen.data())));
	if (ctx->err) {
		throw RuntimeException("[Plguin::Redis] Execute command failed: " + string(ctx->errstr) + ".");
	}

	RedisReplyGuard replyGuard(reply);
	return convertRedisReply(reply);
}

ConstantSP redisPluginBatchSet(Heap *heap, const vector<ConstantSP>& args) {
	if (args[1]->isScalar() && args[2]->isScalar()) {
		return redisPluginRun(heap, {args[0], new String("SET"), args[1], args[2]});
	}

	if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != REDIS_CONNECTION_NAME) {
		throw IllegalArgumentException(__FUNCTION__, "[Plguin::Redis] Argument conn must be a redis handle.");
	}
	if (args[1]->getForm() != DF_VECTOR || args[1]->getType() != DT_STRING) {
		throw IllegalArgumentException(__FUNCTION__, "[Plguin::Redis] Argument keys must be a string vector.");
	}
	if (args[2]->getForm() != DF_VECTOR || args[2]->getType() != DT_STRING) {
		throw IllegalArgumentException(__FUNCTION__, "[Plguin::Redis] Argument values must be a string vector.");
	}
	if (args[1]->size() != args[2]->size()) {
		throw IllegalArgumentException(__FUNCTION__, "[Plguin::Redis] Argument keys and values must have the same size.");
	}

	SmartPointer<RedisConnection> redisHandler = REDIS_HANDLE_MAP.safeGet(args[0]);
	if (redisHandler.isNull()) {
		throw RuntimeException("[Plguin::Redis] Invalid redis handle.");
	}
	redisContext* ctx = redisHandler->getRawRedisConnection();
	LockGuard<Mutex> guard(&redisMutex);

	char* keysBuffer[BUFFER_SIZE];
	char* valuesBuffer[BUFFER_SIZE];

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
			const char* setArgv[3] = {"SET", keysBuffer[i], valuesBuffer[i]};
			const size_t setArgvLen[3] = {3, strlen(keysBuffer[i]), strlen(valuesBuffer[i])};
			redisReply *reply = static_cast<redisReply *>(redisCommandArgv(ctx, 3, setArgv, setArgvLen));
			if (ctx->err || reply == nullptr) {
				throw RuntimeException("[Plguin::Redis] Execute command failed: " + string(ctx->errstr) + ".");
			}
			if (reply->type != REDIS_REPLY_STATUS || strncmp(reply->str, "OK", 2) != 0) {
				throw RuntimeException("[Plguin::Redis] Set failed: " + string(reply->str) + ".");
			}
		}
	}
	return new String("batchSet finish.");
}

ConstantSP redisPluginRelease(const ConstantSP& handle)
{
	if (handle->getType() != DT_RESOURCE || handle->getString() != REDIS_CONNECTION_NAME) {
		throw IllegalArgumentException(__FUNCTION__, "[Plguin::Redis] First argument must be a redis handle.");
	}
	REDIS_HANDLE_MAP.safeRemove(handle);
	return new String("release finish.");
}

ConstantSP redisPluginReleaseAll() {
	REDIS_HANDLE_MAP.clear();
	return new String("releaseAll finish.");
}

ConstantSP redisGetHandle(Heap *heap, const vector<ConstantSP>& args) {
	if (args[0]->getForm() != DF_SCALAR || args[0]->getType() != DT_STRING) {
		throw IllegalArgumentException(__FUNCTION__, "[Plguin::Redis] First argument must be a string scalar.");
	}
	return REDIS_HANDLE_MAP.getHandleByName(args[0]->getString());
}

ConstantSP redisGetHandleStaus() {
	const vector<string>& names = REDIS_HANDLE_MAP.getHandleNames();
	int num = names.size();
	TableSP statusTable = Util::createTable(REDIS_STATUS_COLUMN_NAMES, REDIS_STATUS_COLUMN_TYPES, 0, num);

	SmartPointer<RedisConnection> redisHandler;
	vector<ConstantSP> row(3);
	INDEX insertedRows;
	string errMsg;
	for (int i = 0; i < num; ++i) {
		redisHandler = nullptr;

		try {
			redisHandler = REDIS_HANDLE_MAP.safeGetByName(names[i]);
		} catch (const RuntimeException& e) {
			string err = e.what();
			size_t len = REDIS_PREFIX.size();
			if (err.size() < len || err.substr(0, len) != REDIS_PREFIX) {
				throw e;
			}
		}

		if (!redisHandler.isNull()) {
			row[0] = new String(std::to_string(reinterpret_cast<long long>(redisHandler.get())));
			row[1] = new String(redisHandler->getAddress());
			row[2] = redisHandler->getCreatedTime().getValue();

			if(!statusTable->append(row, insertedRows, errMsg)) {
				throw RuntimeException("[Plguin::Redis] getStatus failed: " + errMsg);               
			}
		}
	}

	return statusTable;
}
