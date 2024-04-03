#ifndef REDIS_CONNECTION_H
#define REDIS_CONNECTION_H

#include "CoreConcept.h"
#include "ScalarImp.h"
#include "hiredis.h"

class RedisConnection {
  public:
    RedisConnection(redisContext *redisConnection, const string &ip, const int &port);
    ~RedisConnection();

    inline string getAddress() const { return address_; }
    inline DateTime getCreatedTime() const { return datetime_; }

    ConstantSP redisRun(const vector<ConstantSP> &args);
    ConstantSP redisBatchSet(const vector<ConstantSP> &args);
    ConstantSP redisBatchHashSet(const vector<ConstantSP> &args);

  private:
    Mutex redisMutex_;
    redisContext *redisConnect_;
    string address_;
    DateTime datetime_;
};

#endif
