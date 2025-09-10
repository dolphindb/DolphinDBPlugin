#ifndef REDIS_CONNECTION_H
#define REDIS_CONNECTION_H

#include "DolphinDBEverything.h"
#include "hiredis.h"

namespace ddb {

class RedisConnection {
  public:
    RedisConnection(redisContext *redisConnection, const string &ip, const int &port);
    ~RedisConnection();

    inline string getAddress() const { return address_; }
    inline DateTime getCreatedTime() const { return datetime_; }

    ConstantSP redisRun(const vector<ConstantSP> &args, const string& command = "redisRun");
    ConstantSP redisBatchSet(const vector<ConstantSP> &args);
    ConstantSP redisBatchHashSet(const vector<ConstantSP> &args);
    ConstantSP redisBatchPush(const vector<ConstantSP> &args);
    ConstantSP redisBatchGet(const vector<ConstantSP> &args);

  private:
    void checkReply(const redisReply *reply, const string &command);

  private:
    Mutex redisMutex_;
    redisContext *redisConnect_;
    string address_;
    DateTime datetime_;
};

}

#endif
