#ifndef KAFKA_CLIENT_H
#define KAFKA_CLIENT_H

#include "ddbplugin/ThreadedQueue.h"
#include "kafkaUtil.h"
#include "kafkaWrapper.h"

using namespace cppkafka;
using namespace KafkaUtil;

class SubConnection;
class AppendTable;
typedef SmartPointer<SubConnection> SubConnectionSP;
typedef SmartPointer<AppendTable> AppendTableSP;

class MessageWrapper {
  public:
    MessageWrapper() {}
    MessageWrapper(rawMessageWrapperSP ptr, long long reachTime) : rawMessage_(ptr), reachTime(reachTime) {}
    MessageWrapper(const MessageWrapper &) = default;
    MessageWrapper &operator=(const MessageWrapper &) = default;

  public:
    rawMessageWrapperSP rawMessage_;
    long long reachTime = LONG_LONG_MIN;
};

class AppendTable : public Runnable {
  public:
    AppendTable(Heap *heap, ConstantSP parser, ConstantSP handle, ConstantSP consumer, const string &actionName,
                SubJobAutoCommit autoCommit, bool msgAsTable, long long batchSize, long long throttle,
                long long queueDepth);
    ~AppendTable() override = default;
    void run() override;
    TableSP doParse(const rawMessageWrapperSP &msg);
    void doHandle(TableSP tableInsert);
    const StreamStatus &getStatus() const;
    void handleErr(const string &errMsg);

    void setFlag(bool flag) { flag_ = flag; }
    long long getBatchSize() const { return batchSize_; }
    float getThrottle() const { return double(throttle_) / 1000.0; }
    bool ifMsgAsTable() const { return msgAsTable_; }
    bool ifAutoCommit() const { return autoCommit_; }

  private:
    bool flag_ = true;
    bool autoCommit_ = false;
    bool msgAsTable_ = false;
    int timeout_ = 100;
    long long batchSize_ = 0;
    long long throttle_ = 0;
    long long localTimeGap_;
    string actionName_;
    SmartPointer<ThreadedQueue<MessageWrapper>> queue_;
    SmartPointer<Consumer> consumer_;
    SessionSP session_;
    ConstantSP parser_;
    ConstantSP handle_;
    ConstantSP consumerWrapper_;
    vector<ConstantSP> parserArgs_;
    StreamStatus status_;
};

class SubConnection {
  public:
    SubConnection() = default;
    SubConnection(Heap *heap, ConstantSP consumerWrapper, ConstantSP handler, ConstantSP parser,
                  const string &actionName, KafkaUtil::SubJobAutoCommit autoCommit, bool msgAsTable,
                  long long batchSize, long long throttle, long long queueDepth);
    ~SubConnection();

    string getDescription() const { return actionName_; }
    ConstantSP getConsumerHandle() const { return consumerWrapper_; }
    long long getCreateTime() const { return createTime_; }
    SessionSP getSession() { return session_; }
    const AppendTableSP &getAppendTable() const { return append_; };
    const StreamStatus &getStatus() { return append_->getStatus(); }

    void cancelThread() {
        append_->setFlag(false);
        thread_->join();
    }

  private:
    bool connected_ = false;
    long long createTime_ = LONG_LONG_MIN;
    std::string actionName_;
    Heap *heap_;
    ConstantSP consumerWrapper_;
    ThreadSP thread_;
    SessionSP session_;
    AppendTableSP append_;
};
#endif  // KAFKA_CLIENT_H
