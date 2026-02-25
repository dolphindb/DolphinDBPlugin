#ifndef REDIS_CONNECTION_H
#define REDIS_CONNECTION_H

#include "DolphinDBEverything.h"
#include "hiredis/hiredis.h"

class RedisReplyGuard {
  public:
    RedisReplyGuard(redisReply *r) : reply_(r) {}
    ~RedisReplyGuard() {
        if (reply_ != nullptr) {
            freeReplyObject(reply_);
        }
    }

    RedisReplyGuard(const RedisReplyGuard &) = delete;
    RedisReplyGuard &operator=(const RedisReplyGuard &) = delete;

  private:
    redisReply *reply_ = nullptr;
};

ConstantSP convertRedisReply(const redisReply *reply);

class DummyOutput : public Output {
  public:
    virtual bool timeElapsed(long long nanoSeconds) { return true; }
    virtual bool write(const ConstantSP &obj) { return true; }
    virtual bool message(const string &msg) { return true; }
    virtual void enableIntermediateMessage(bool enabled) {}
    virtual IO_ERR done() { return OK; }
    virtual IO_ERR done(const string &errMsg) { return OK; }
    virtual bool start() { return true; }
    virtual bool start(const string &message) { return true; }
    virtual IO_ERR writeReady() { return OK; }
    virtual ~DummyOutput() {}
    virtual OUTPUT_TYPE getOutputType() const { return STDOUT; }
    virtual void close() {}
    virtual void setWindow(INDEX index, INDEX size) {};
    virtual IO_ERR flush() { return OK; }
};

struct RedisMsg {
    string channel_;
    string msg_;
};

const string CONN_ERR = ", this connection cannot be reused and you should release it and create a new connection.";

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
    // redis stream
    ConstantSP streamCreateGroup(const string &key, const string &group, const string &msgId, bool mkstream);
    ConstantSP streamReadGroup(const string &key, const string &group, const string &consumer, int count, bool pending,
                               int block, bool writePending);
    ConstantSP streamAck(const string &key, const string &group, const VectorSP &msgIds);
    void subscribe(Heap *heap, const vector<string> &channels, const FunctionDefSP &callback, bool isPattern,
                   const string &password);
    void unsubscribe(const vector<string> &channels);
    string getSubscriptions();
    void stopListen();

  private:
    void checkReply(const redisReply *reply, const string &command);
    void checkIsSub(const string& funcName) const;
    string checkSubReplyHelper(const redisReply *reply) const;
    string checkSubReply(int numChannels) const;
    void initSubThreads(Heap *heap);
    bool reconnect(const timeval &timeout);
    void buildSubCommand(const vector<string> &channels, bool isSub);

  private:
    Mutex redisMutex_;
    redisContext *redisConnect_;
    string address_;
    DateTime datetime_;

    bool isClosed_{false};
    SessionSP session_;
    FunctionDefSP callback_;
    ThreadSP listener_;
    ThreadSP handler_;
    SynchronizedQueue<RedisMsg> msgQueue_;
    
    Mutex channelMutex_;
    bool isPattern_{false};
    bool isChanged_{false};
    int numChannels_{0};
    string channelStr_;
    unordered_set<string> channels_;
    string password_;
};

#endif
