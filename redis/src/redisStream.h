#ifndef REDIS_STREAM_H
#define REDIS_STREAM_H

#include "DolphinDBEverything.h"
#include "Concurrent.h"
#include "hiredis/hiredis.h"

namespace ddb {

class RedisCommandArgs {
  public:
    RedisCommandArgs(const vector<string> &strs);
    inline const char **argv() { return argv_.data(); }
    inline const size_t *argvlen() { return argvlen_.data(); }
    inline int size() const { return static_cast<int>(argv_.size()); }

  private:
    vector<string> strs_;
    vector<const char *> argv_;
    vector<size_t> argvlen_;
};

class StreamSubscription {
  public:
    StreamSubscription(Heap *heap, const string &host, int port, const string &key, const string &group,
                       const string &consumer, const TableSP &output, const FunctionDefSP &handler,
                       const string &actionName, bool pending, int batchSize, bool autoAck);
    ~StreamSubscription();
    void stop();
    void connect(const string& password);
    void startRead(const ThreadSP &readThread);

  public:
    const string host_;
    const int port_{0};
    const string streamKey_;
    const string groupName_;
    const string consumerName_;
    TableSP output_;
    FunctionDefSP handler_;
    const string actionName_;
    const bool pending_{false};
    const int batchSize_{0};
    const bool autoAck_{true};
    const long long createTime_{0};
    ThreadSP readerThread_;
    SessionSP session_;

    Mutex statusLock_;
    bool isEnd_{false};
    redisContext *redisConn_{nullptr};
    long long processedMsgCount_{0};
    string status_{"OK"};
    string lastErrMsg_;
    long long lastFailedTime_{LONG_LONG_MIN};
};
typedef SmartPointer<StreamSubscription> StreamSubscriptionSP;

class StreamJobManager {
  public:
    static StreamJobManager *getInstance();
    inline Mutex *getMutex() { return &mutex_; }

    void addSubscription(const StreamSubscriptionSP &sub);
    void removeSubscription(const string &actionName);
    TableSP getStatusTable();

  private:
    StreamJobManager() = default;  // forbid constructor

    Mutex mutex_;
    unordered_map<string, StreamSubscriptionSP> subscriptions_;
};

// redis stream subscription
ConstantSP redisStreamSubscribeImpl(Heap *heap, const string &host, int port, const string &key, const string &group,
                                    const string &consumer, const TableSP &output, const FunctionDefSP &parser,
                                    const string &actionName, bool pending, int batchSize, bool autoAck,
                                    const string &password);
ConstantSP redisStreamUnsubscribeImpl(const string &actionName);
ConstantSP redisGetStreamJobStatImpl();

}  // namespace ddb

#endif
