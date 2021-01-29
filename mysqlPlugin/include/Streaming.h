#ifndef _STREAMING_H_
#define _STREAMING_H_
#include <functional>
#include <string>
#include <vector>
#include "Concurrent.h"
#include "DolphinDB.h"
#include "Util.h"
#ifdef _MSC_VER
#define EXPORT_DECL _declspec(dllexport)
#else
#define EXPORT_DECL 
#endif
namespace dolphindb {

template <typename T>
class BlockingQueue;

using Message = ConstantSP;
using MessageQueue = BlockingQueue<Message>;
using MessageQueueSP = SmartPointer<MessageQueue>;
using MessageHandler = std::function<void(Message)>;

extern char const *DEFAULT_ACTION_NAME;

template <typename T>
class EXPORT_DECL BlockingQueue {
public:
    explicit BlockingQueue(size_t maxItems)
        : buf_(new T[maxItems]), capacity_(maxItems), size_(0), head_(0), tail_(0) {}
    void push(const T &item) {
        lock_.lock();
        while (size_ >= capacity_) full_.wait(lock_);
        buf_[tail_] = item;
        tail_ = (tail_ + 1) % capacity_;
        ++size_;

        if (size_ == 1) empty_.notifyAll();
        lock_.unlock();
    }
    void emplace(T &&item) {
        lock_.lock();
        while (size_ >= capacity_) full_.wait(lock_);
        buf_[tail_] = std::move(item);
        tail_ = (tail_ + 1) % capacity_;
        ++size_;
        if (size_ == 1) empty_.notifyAll();
        lock_.unlock();
    }
    bool poll(T &item, int milliSeconds) {
        if (milliSeconds < 0) {
            pop(item);
            return true;
        }
        LockGuard<Mutex> guard(&lock_);
        while (size_ == 0) {
            if (!empty_.wait(lock_, milliSeconds)) return false;
        }
        item = std::move(buf_[head_]);
        buf_[head_] = T();
        head_ = (head_ + 1) % capacity_;
        --size_;
        if (size_ == capacity_ - 1) full_.notifyAll();
        return true;
    }
    void pop(T &item) {
        lock_.lock();
        while (size_ == 0) empty_.wait(lock_);
        item = std::move(buf_[head_]);
        buf_[head_] = T();
        head_ = (head_ + 1) % capacity_;
        --size_;
        if (size_ == capacity_ - 1) full_.notifyAll();
        lock_.unlock();
    }

private:
    std::unique_ptr<T[]> buf_;
    size_t capacity_;
    size_t size_;
    size_t head_;
    size_t tail_;
    Mutex lock_;
    ConditionalVariable full_;
    ConditionalVariable empty_;
};
class StreamingClientImpl;
class EXPORT_DECL StreamingClient {
public:
    explicit StreamingClient(int listeningPort);
    virtual ~StreamingClient();

protected:
    MessageQueueSP subscribeInternal(string host, int port, string tableName, string actionName = DEFAULT_ACTION_NAME,
                                     int64_t offset = -1, bool resubscribe = true, const VectorSP &filter = nullptr,
                                     bool msgAsTable = false, bool allowExists = false);
    void unsubscribeInternal(string host, int port, string tableName, string actionName = DEFAULT_ACTION_NAME);

private:
    std::unique_ptr<StreamingClientImpl> impl_;
};

class EXPORT_DECL ThreadedClient : private StreamingClient {
public:
    explicit ThreadedClient(int listeningPort);
    ~ThreadedClient() override = default;
    ThreadSP subscribe(string host, int port, const MessageHandler &handler, string tableName,
                       string actionName = DEFAULT_ACTION_NAME, int64_t offset = -1, bool resub = true,
                       const VectorSP &filter = nullptr, bool msgAsTable = false, bool allowExists = false);
    void unsubscribe(string host, int port, string tableName, string actionName = DEFAULT_ACTION_NAME);
};

class EXPORT_DECL ThreadPooledClient : private StreamingClient {
public:
    explicit ThreadPooledClient(int listeningPort, int threadCount);
    ~ThreadPooledClient() override = default;
    vector<ThreadSP> subscribe(string host, int port, const MessageHandler &handler, string tableName,
                               string actionName, int64_t offset = -1, bool resub = true,
                               const VectorSP &filter = nullptr, bool msgAsTable = false, bool allowExists = false);
    void unsubscribe(string host, int port, string tableName, string actionName = DEFAULT_ACTION_NAME);

private:
    int threadCount_;
};

class EXPORT_DECL PollingClient : private StreamingClient {
public:
    explicit PollingClient(int listeningPort);
    ~PollingClient() override = default;
    MessageQueueSP subscribe(string host, int port, string tableName, string actionName = DEFAULT_ACTION_NAME,
                             int64_t offset = -1, bool resub = true, const VectorSP &filter = nullptr,
                             bool msgAsTable = false, bool allowExists = false);
    void unsubscribe(string host, int port, string tableName, string actionName = DEFAULT_ACTION_NAME);
};

}  // namespace dolphindb
#endif  // _STREAMING_H_