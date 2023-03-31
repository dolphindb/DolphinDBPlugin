#pragma once

#include <fstream>
#include <time.h>
#include <sys/timeb.h>
#include <thread>
#include <iostream>
#include <Concurrent.h>
#include <Logger.h>
#include <Util.h>
#include <ddbplugin/Plugin.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif // _WIN32

#include <HSNsqApi.h>

#define NUM_OF_ROUND 1

template <class T>
struct ObjectSizer
{
    inline int operator()(const T &obj)
    {
        return 1;
    }
};

template <class T>
struct ObjectUrgency
{
    inline bool operator()(const T &obj)
    {
        return false;
    }
};
struct SnapData
{
    CHSNsqSecuDepthMarketDataField *pSecuDepthMarketData;
    HSIntVolume *Bid1Volume;
    HSNum Bid1Count;
    HSNum MaxBid1Count;
    HSIntVolume *Ask1Volume;
    HSNum Ask1Count;
    HSNum MaxAsk1Count;
};

typedef GenericBoundedQueue<CHSNsqSecuTransactionTradeDataField *, ObjectSizer<CHSNsqSecuTransactionTradeDataField *>, ObjectUrgency<CHSNsqSecuTransactionTradeDataField *>> TradeQueue;
typedef GenericBoundedQueue<CHSNsqSecuTransactionEntrustDataField *, ObjectSizer<CHSNsqSecuTransactionEntrustDataField *>, ObjectUrgency<CHSNsqSecuTransactionEntrustDataField *>> EntrustQueue;
typedef GenericBoundedQueue<SnapData *, ObjectSizer<SnapData *>, ObjectUrgency<SnapData *>> SnapshotQueue;

template <class CLASSTYPE>
void testEntrustInsertCapability(int x, CLASSTYPE *opt)
{
    //std::this_thread::sleep_for(std::chrono::milliseconds(40000));
    Thread::sleep(40000);
    std::cout << "testEntrustInsertCapability run!!!!" << std::endl;
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());
    auto begintime = ms.count();

    for (int i = 0; i < x; i++)
    {
        // std::cout<<"has make one"<<std::endl;
        CHSNsqSecuTransactionEntrustDataField *mid = new CHSNsqSecuTransactionEntrustDataField();
        mid->ExchangeID[0] = '1';
        mid->ExchangeID[1] = '\0';
        mid->ExchangeID[2] = '\0';
        mid->ExchangeID[3] = '\0';
        mid->ExchangeID[4] = '\0';
        opt->OnRtnSecuTransactionEntrustData(mid);
        delete (mid);
    }
    std::chrono::milliseconds end = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());
    auto endtime = end.count();
    std::cout << "orders insert throught is " << x / (endtime - begintime) * 1000 << std::endl;
};
template <class CLASSTYPE>
void testSnapshotInsertCapability(int x, CLASSTYPE *opt)
{
    //std::this_thread::sleep_for(std::chrono::milliseconds(40000));
    Thread::sleep(40000);
    std::cout << "testSnapshotInsertCapability run!!!!" << std::endl;
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());
    auto begintime = ms.count();
    for (int i = 0; i < x; i++)
    {
        // std::cout<<"has make one"<<std::endl;
        SnapData *mid = new SnapData();
        mid->pSecuDepthMarketData = new CHSNsqSecuDepthMarketDataField();
        mid->pSecuDepthMarketData->ExchangeID[0] = '1';
        mid->pSecuDepthMarketData->ExchangeID[1] = '\0';
        mid->pSecuDepthMarketData->ExchangeID[2] = '\0';
        mid->pSecuDepthMarketData->ExchangeID[3] = '\0';
        mid->pSecuDepthMarketData->ExchangeID[4] = '\0';
        mid->Ask1Volume = new HSIntVolume[50];
        mid->Bid1Volume = new HSIntVolume[50];
        opt->OnRtnSecuDepthMarketData(mid->pSecuDepthMarketData, mid->Bid1Volume, mid->Bid1Count, mid->MaxBid1Count, mid->Ask1Volume, mid->Ask1Count, mid->MaxAsk1Count);
        delete mid->pSecuDepthMarketData;
        delete mid->Ask1Volume;
        delete mid->Bid1Volume;
        delete mid;
    }
    std::chrono::milliseconds end = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());

    auto endtime = end.count();
    std::cout << "snapshot insert throught is " << x / (endtime - begintime) * 1000 << std::endl;
};
template <class CLASSTYPE>
void testTradeInsertCapability(int x, CLASSTYPE *opt)
{
    //std::this_thread::sleep_for(std::chrono::milliseconds(40000));
    Thread::sleep(40000);
    std::cout << "testTradeInsertCapability run!!!!" << std::endl;
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());
    auto begintime = ms.count();
    for (int i = 0; i < x; i++)
    {
        // std::cout<<"has make one"<<std::endl;
        CHSNsqSecuTransactionTradeDataField *mid = new CHSNsqSecuTransactionTradeDataField();
        mid->ExchangeID[0] = '1';
        mid->ExchangeID[1] = '\0';
        mid->ExchangeID[2] = '\0';
        mid->ExchangeID[3] = '\0';
        mid->ExchangeID[4] = '\0';
        opt->OnRtnSecuTransactionTradeData(mid);
        delete (mid);
    }
    std::chrono::milliseconds end = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());
    auto endtime = end.count();
    std::cout << "tardedata insert throught is " << x / (endtime - begintime) * 1000 << std::endl;
};
template <class CLASSTYPE>
void tradeDataHandle(SmartPointer<TradeQueue> TQ, CLASSTYPE *opt, int batchSize)
{
    CHSNsqSecuTransactionTradeDataField *item;
    size_t lostdata = 0;
    while (opt->queueRunFlag_)
    {
        try
        {
            bool ret = TQ->blockingPop(item, 100);
            if (!ret)
            {
                continue;
            }
            auto size = TQ->size() >= batchSize ? batchSize : TQ->size();
            std::vector<CHSNsqSecuTransactionTradeDataField *> items;
            items.reserve(size + 1);
            lostdata = size + 1;
            items.push_back(std::move(item));
            if (size > 0)
                TQ->pop(items, size);
            opt->asyncTradeHandle(items);
            for (int i = 0; i < items.size(); i++)
            {
                delete items[i];
            }
        }
        catch (exception &e)
        {
            LOG_ERR("[PluginNsq] TradeHandle thread failed to process ", lostdata, " data because ", e.what());
        }
    }
};
template <class CLASSTYPE>
void entrustDataHandle(SmartPointer<EntrustQueue> SQ, CLASSTYPE *opt, int batchSize)
{
    CHSNsqSecuTransactionEntrustDataField *item;
    size_t lostdata = 0;
    while (opt->queueRunFlag_)
    {
        try
        {
            bool ret = SQ->blockingPop(item, 100);
            if (!ret)
            {
                continue;
            }
            auto size = SQ->size() >= batchSize ? batchSize : SQ->size();
            std::vector<CHSNsqSecuTransactionEntrustDataField *> items;
            items.reserve(size + 1);
            lostdata = size + 1;
            items.push_back(std::move(item));
            if (size > 0)
                SQ->pop(items, size);
            opt->asyncEntrustHandle(items);
            for (int i = 0; i < items.size(); i++)
            {
                delete items[i];
            }
        }
        catch (exception &e)
        {
            LOG_ERR("[PluginNsq] TradeHandle thread failed to process ", lostdata, " data because ", e.what());
        }
    }
};
template <class CLASSTYPE>
void snapshotDataHandle(SmartPointer<SnapshotQueue> SQ, CLASSTYPE *opt, int batchSize)
{
    SnapData *item;
    size_t lostdata = 0;
    while (opt->queueRunFlag_)
    {
        try
        {
            bool ret = SQ->blockingPop(item, 100);
            if (!ret)
            {
                continue;
            }
            auto size = SQ->size() >= batchSize ? batchSize : SQ->size();
            std::vector<SnapData *> items;
            items.reserve(size + 1);
            lostdata = size + 1;
            items.push_back(std::move(item));
            if (size > 0)
                SQ->pop(items, size);
            opt->asyncSnapshotHandle(items);
            for (int i = 0; i < items.size(); i++)
            {
                delete items[i]->pSecuDepthMarketData;
                delete items[i]->Ask1Volume;
                delete items[i]->Bid1Volume;
                delete items[i];
            }
        }
        catch (exception &e)
        {
            LOG_ERR("[PluginNsq] TradeHandle thread failed to process ", lostdata, " data because ", e.what());
        }
    }
};
class CHSNsqSpiImpl : public CHSNsqSpi
{

public:
    // 处理不同数据的后台线程
    ThreadSP testEthread_;
    ThreadSP testSthread_;
    ThreadSP testTthread_;
    ThreadSP tradeThread1_;
    ThreadSP snapshotThread1_;
    ThreadSP entrustThread1_;
    ThreadSP tradeThread2_;
    ThreadSP snapshotThread2_;
    ThreadSP entrustThread2_;
    // 存储回调数据的队列
    SmartPointer<TradeQueue> tradeBlockingQueue1_;
    SmartPointer<TradeQueue> tradeBlockingQueue2_;
    SmartPointer<SnapshotQueue> snapshotBlockingQueue1_;
    SmartPointer<SnapshotQueue> snapshotBlockingQueue2_;
    SmartPointer<EntrustQueue> entrustBlockingQueue1_;
    SmartPointer<EntrustQueue> entrustBlockingQueue2_;
    // 处理线程是否运行
    volatile bool queueRunFlag_ = true;
    // 是否打开测试线程
    const bool testForInsert_ = false;
    // 测试时需要插入的数据量
    const size_t insertDataSize_ = 1000000;
    // 队列的固定大小
    const size_t blockingQueueSize_ = 200000;

    // long long recsum=0;
    CHSNsqSpiImpl(CHSNsqApi *lpHSNsqApi)
    {
        m_lpHSMdApi = lpHSNsqApi;
        m_isConnected = false;
        m_isLogined = false;
        m_isAllInstrReady = false;
        m_iAllInstrCount = 0;

        f_sh_stock_init = NULL;
        f_sz_stock_init = NULL;
        f_sh_opt_init = NULL;
        f_sz_opt_init = NULL;
        // 初始化队列，一共六种表，六个队列
        tradeBlockingQueue1_ = new TradeQueue(blockingQueueSize_, ObjectSizer<CHSNsqSecuTransactionTradeDataField *>(), ObjectUrgency<CHSNsqSecuTransactionTradeDataField *>());
        tradeBlockingQueue2_ = new TradeQueue(blockingQueueSize_, ObjectSizer<CHSNsqSecuTransactionTradeDataField *>(), ObjectUrgency<CHSNsqSecuTransactionTradeDataField *>());
        entrustBlockingQueue1_ = new EntrustQueue(blockingQueueSize_, ObjectSizer<CHSNsqSecuTransactionEntrustDataField *>(), ObjectUrgency<CHSNsqSecuTransactionEntrustDataField *>());
        entrustBlockingQueue2_ = new EntrustQueue(blockingQueueSize_, ObjectSizer<CHSNsqSecuTransactionEntrustDataField *>(), ObjectUrgency<CHSNsqSecuTransactionEntrustDataField *>());
        snapshotBlockingQueue1_ = new SnapshotQueue(blockingQueueSize_, ObjectSizer<SnapData *>(), ObjectUrgency<SnapData *>());
        snapshotBlockingQueue2_ = new SnapshotQueue(blockingQueueSize_, ObjectSizer<SnapData *>(), ObjectUrgency<SnapData *>());
        try
        {
            // 初始化线程，对应处理6种不同的表
            SmartPointer<dolphindb::Executor> tradeExecutor1 = new dolphindb::Executor(std::bind(&tradeDataHandle<CHSNsqSpiImpl>, tradeBlockingQueue1_, this, 4999));
            SmartPointer<dolphindb::Executor> tradeExecutor2 = new dolphindb::Executor(std::bind(&tradeDataHandle<CHSNsqSpiImpl>, tradeBlockingQueue2_, this, 4999));
            SmartPointer<dolphindb::Executor> entrustExecutor1 = new dolphindb::Executor(std::bind(&entrustDataHandle<CHSNsqSpiImpl>, entrustBlockingQueue1_, this, 4999));
            SmartPointer<dolphindb::Executor> entrustExecutor2 = new dolphindb::Executor(std::bind(&entrustDataHandle<CHSNsqSpiImpl>, entrustBlockingQueue2_, this, 4999));
            SmartPointer<dolphindb::Executor> snapshotExecutor1 = new dolphindb::Executor(std::bind(&snapshotDataHandle<CHSNsqSpiImpl>, snapshotBlockingQueue1_, this, 4999));
            SmartPointer<dolphindb::Executor> snapshotExecutor2 = new dolphindb::Executor(std::bind(&snapshotDataHandle<CHSNsqSpiImpl>, snapshotBlockingQueue2_, this, 4999));
            tradeThread1_ = new Thread(tradeExecutor1);
            tradeThread2_ = new Thread(tradeExecutor2);
            entrustThread1_ = new Thread(entrustExecutor1);
            entrustThread2_ = new Thread(entrustExecutor2);
            snapshotThread1_ = new Thread(snapshotExecutor1);
            snapshotThread2_ = new Thread(snapshotExecutor2);
            tradeThread1_->start();
            tradeThread2_->start();
            entrustThread1_->start();
            entrustThread2_->start();
            snapshotThread1_->start();
            snapshotThread2_->start();
            if (testForInsert_)
            {
                SmartPointer<dolphindb::Executor> testExecutorE = new dolphindb::Executor(std::bind(&testEntrustInsertCapability<CHSNsqSpiImpl>, insertDataSize_, this));
                SmartPointer<dolphindb::Executor> testExecutorS = new dolphindb::Executor(std::bind(&testSnapshotInsertCapability<CHSNsqSpiImpl>, insertDataSize_, this));
                SmartPointer<dolphindb::Executor> testExecutorT = new dolphindb::Executor(std::bind(&testTradeInsertCapability<CHSNsqSpiImpl>, insertDataSize_, this));
                testEthread_ = new Thread(testExecutorE);
                testSthread_ = new Thread(testExecutorS);
                testTthread_ = new Thread(testExecutorT);
                testEthread_->start();
                testSthread_->start();
                testTthread_->start();
            }
        }
        catch (exception &e)
        { // exception类位于<exception>头文件中
            LOG_ERR("[PluginNsq] Failed to create thread because ", e.what());
        }
    }

    ~CHSNsqSpiImpl()
    {
        this->queueRunFlag_ = false;
        tradeThread1_->join();
        snapshotThread1_->join();
        entrustThread1_->join();
        tradeThread2_->join();
        snapshotThread2_->join();
        entrustThread2_->join();
    }

    /// Description: 当客户端与后台开始建立通信连接，连接成功后此方法被回调。
    virtual void OnFrontConnected();

    /// Description:当客户端与后台通信连接异常时，该方法被调用。
    /// Others     :通过GetApiErrorMsg(nResult)获取详细错误信息。
    virtual void OnFrontDisconnected(int nResult);

    /// Description:客户登录
    virtual void OnRspUserLogin(CHSNsqRspUserLoginField *pRspUserLogin, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ////以下是现货接口

    /// Description: 订阅-现货快照行情应答
    virtual void OnRspSecuDepthMarketDataSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    /// Description: 订阅取消-现货快照行情应答
    virtual void OnRspSecuDepthMarketDataCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    /// Description: 主推-现货快照行情
    /// Others     :Bid1Volume买一队列数组, Bid1Count买一队列数组个数, MaxBid1Count买一总委托笔数
    ///           :Ask1Volume卖一队列数组, Ask1Count卖一队列数组个数, MaxAsk1Count卖一总委托笔数
    virtual void OnRtnSecuDepthMarketData(CHSNsqSecuDepthMarketDataField *pSecuDepthMarketData, HSIntVolume Bid1Volume[], HSNum Bid1Count, HSNum MaxBid1Count, HSIntVolume Ask1Volume[], HSNum Ask1Count, HSNum MaxAsk1Count);

    /// Description: 主推-现货盘后定价快照行情
    /// Others     :Bid1Volume买一队列数组, Bid1Count买一队列数组个数, MaxBid1Count买一总委托笔数
    ///           :Ask1Volume卖一队列数组, Ask1Count卖一队列数组个数, MaxAsk1Count卖一总委托笔数
    virtual void OnRtnSecuATPMarketData(CHSNsqSecuATPMarketDataField *pSecuDepthMarketData, HSIntVolume Bid1Volume[], HSNum Bid1Count, HSNum MaxBid1Count, HSIntVolume Ask1Volume[], HSNum Ask1Count, HSNum MaxAsk1Count);

    /// Description: 订阅-现货逐笔行情应答
    virtual void OnRspSecuTransactionSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    /// Description: 订阅取消-现货逐笔行情应答
    virtual void OnRspSecuTransactionCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    /// Description: 主推-现货逐笔成交行情
    virtual void OnRtnSecuTransactionTradeData(CHSNsqSecuTransactionTradeDataField *pSecuTransactionTradeData);

    /// Description: 主推-现货逐笔委托行情
    virtual void OnRtnSecuTransactionEntrustData(CHSNsqSecuTransactionEntrustDataField *pSecuTransactionEntrustData);

    /// Description: 主推-现货盘后固定逐笔成交行情
    virtual void OnRtnSecuATPTransactionTradeData(CHSNsqSecuTransactionTradeDataField *pSecuTransactionTradeData);

    /// Description: 获取当前交易日现货合约应答
    virtual void OnRspQrySecuInstruments(CHSNsqSecuInstrumentStaticInfoField *pSecuInstrumentStaticInfo, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    /// Description: 期权订阅-行情应答
    virtual void OnRspOptDepthMarketDataSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    /// Description: 期权订阅取消-行情应答
    virtual void OnRspOptDepthMarketDataCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    /// Description: 主推-期权行情
    virtual void OnRtnOptDepthMarketData(CHSNsqOptDepthMarketDataField *pOptDepthMarketData);

    /// Description: 获取当前交易日合约应答
    virtual void OnRspQryOptInstruments(CHSNsqOptInstrumentStaticInfoField *pOptInstrumentStaticInfo, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    /// Description: 获取合约的最新快照信息应答
    virtual void OnRspQryOptDepthMarketData(CHSNsqOptDepthMarketDataField *pOptDepthMarketData, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ////以下是现货逐笔重建接口

    /// Description: 重建应答-现货逐笔数据
    virtual void OnRspSecuTransactionData(CHSNsqSecuTransactionDataField *pSecuTransactionData, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    /// Description: 重建应答超时-现货逐笔数据（本回调线程与其他回调线程不同）
    virtual void OnRspSecuTransactionDataTimeout(int nRequestID);

    /// Description: 获取合TCP连接状态
    bool GetConnectStatus() { return m_isConnected; }

    /// Description: 获取登陆状态
    bool GetLoginStatus() { return m_isLogined; }

    /// Description: 获取静态代码表状态
    bool GetInstrumentsStatus() { return m_isAllInstrReady; }

    /// Description: 获取静态代码表数量
    int GetInstrumentsCount() { return m_iAllInstrCount; }

    // Description: 异步处理entrust表的数据
    void asyncEntrustHandle(std::vector<CHSNsqSecuTransactionEntrustDataField *> &items);

    // Description: 异步处理trade表的数据
    void asyncTradeHandle(std::vector<CHSNsqSecuTransactionTradeDataField *> &items);

    // Description: 异步处理snapshot表的数据
    void asyncSnapshotHandle(std::vector<SnapData *> &items);

private:
    CHSNsqApi *m_lpHSMdApi;
    bool m_isConnected;
    bool m_isLogined;
    bool m_isAllInstrReady;
    int m_iAllInstrCount;

    FILE *f_sh_stock_init;
    FILE *f_sz_stock_init;
    FILE *f_sh_opt_init;
    FILE *f_sz_opt_init;
};