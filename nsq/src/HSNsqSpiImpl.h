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
#include "plugin_nsq.h"

#define NUM_OF_ROUND 1

static string NSQ_PREFIX = "[PLUGIN::NSQ]";

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
    Thread::sleep(40000);
    LOG(NSQ_PREFIX, " testEntrustInsertCapability run");
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());
    auto beginTime = ms.count();

    for (int i = 0; i < x; i++)
    {
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
    LOG(NSQ_PREFIX, " orders insert elapsed time is ", x / (endtime - beginTime) * 1000);
};
template <class CLASSTYPE>
void testSnapshotInsertCapability(int x, CLASSTYPE *opt)
{
    Thread::sleep(40000);
    LOG(NSQ_PREFIX, " testSnapshotInsertCapability run!!!!");
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());
    auto beginTime = ms.count();
    for (int i = 0; i < x; i++)
    {
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
    LOG(NSQ_PREFIX, " snapshot insert elapsed time is ", x / (endtime - beginTime) * 1000);
};
template <class CLASSTYPE>
void testTradeInsertCapability(int x, CLASSTYPE *opt)
{
    Thread::sleep(40000);
    LOG(NSQ_PREFIX, " testTradeInsertCapability run!!!!");
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());
    auto beginTime = ms.count();
    for (int i = 0; i < x; i++)
    {
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
    LOG(NSQ_PREFIX, " trade data insert elapsed time is ", x / (endtime - beginTime) * 1000);
};
template <class CLASSTYPE>
void tradeDataHandle(SmartPointer<TradeQueue> TQ, CLASSTYPE *opt, int batchSize)
{
    CHSNsqSecuTransactionTradeDataField *item;
    size_t lostData = 0;
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
            lostData = size + 1;
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
            LOG_ERR(NSQ_PREFIX, " TradeHandle thread failed to process ", lostData, " data because ", e.what());
        }
    }
};
template <class CLASSTYPE>
void entrustDataHandle(SmartPointer<EntrustQueue> SQ, CLASSTYPE *opt, int batchSize)
{
    CHSNsqSecuTransactionEntrustDataField *item;
    size_t lostData = 0;
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
            lostData = size + 1;
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
            LOG_ERR(NSQ_PREFIX, " TradeHandle thread failed to process ", lostData, " data because ", e.what());
        }
    }
};
template <class CLASSTYPE>
void snapshotDataHandle(SmartPointer<SnapshotQueue> SQ, CLASSTYPE *opt, int batchSize)
{
    SnapData *item;
    size_t lostData = 0;
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
            lostData = size + 1;
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
            LOG_ERR(NSQ_PREFIX, " TradeHandle thread failed to process ", lostData, " data because ", e.what());
        }
    }
};
class CHSNsqSpiImpl : public CHSNsqSpi
{

public:
    ThreadSP testEThread_;
    ThreadSP testSThread_;
    ThreadSP testTThread_;
    ThreadSP tradeThread1_;
    ThreadSP snapshotThread1_;
    ThreadSP entrustThread1_;
    ThreadSP tradeThread2_;
    ThreadSP snapshotThread2_;
    ThreadSP entrustThread2_;
    SmartPointer<TradeQueue> tradeBlockingQueue1_;
    SmartPointer<TradeQueue> tradeBlockingQueue2_;
    SmartPointer<SnapshotQueue> snapshotBlockingQueue1_;
    SmartPointer<SnapshotQueue> snapshotBlockingQueue2_;
    SmartPointer<EntrustQueue> entrustBlockingQueue1_;
    SmartPointer<EntrustQueue> entrustBlockingQueue2_;
    volatile bool queueRunFlag_ = true;
    const bool testForInsert_ = false;
    const size_t insertDataSize_ = 1000000;
    const size_t blockingQueueSize_ = 200000;

    CHSNsqSpiImpl(CHSNsqApi *lpHSNsqApi)
    {
        lpHSMdApi = lpHSNsqApi;
        isConnected = false;
        isLogin = false;
        isAllInstrReady = false;
        iAllInstrCount = 0;

        fShStockInit = NULL;
        fSzStockInit = NULL;
        fShOptInit = NULL;
        fSzOptInit = NULL;
        tradeBlockingQueue1_ = new TradeQueue(blockingQueueSize_, ObjectSizer<CHSNsqSecuTransactionTradeDataField *>(), ObjectUrgency<CHSNsqSecuTransactionTradeDataField *>());
        tradeBlockingQueue2_ = new TradeQueue(blockingQueueSize_, ObjectSizer<CHSNsqSecuTransactionTradeDataField *>(), ObjectUrgency<CHSNsqSecuTransactionTradeDataField *>());
        entrustBlockingQueue1_ = new EntrustQueue(blockingQueueSize_, ObjectSizer<CHSNsqSecuTransactionEntrustDataField *>(), ObjectUrgency<CHSNsqSecuTransactionEntrustDataField *>());
        entrustBlockingQueue2_ = new EntrustQueue(blockingQueueSize_, ObjectSizer<CHSNsqSecuTransactionEntrustDataField *>(), ObjectUrgency<CHSNsqSecuTransactionEntrustDataField *>());
        snapshotBlockingQueue1_ = new SnapshotQueue(blockingQueueSize_, ObjectSizer<SnapData *>(), ObjectUrgency<SnapData *>());
        snapshotBlockingQueue2_ = new SnapshotQueue(blockingQueueSize_, ObjectSizer<SnapData *>(), ObjectUrgency<SnapData *>());
        try
        {
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
                testEThread_ = new Thread(testExecutorE);
                testSThread_ = new Thread(testExecutorS);
                testTThread_ = new Thread(testExecutorT);
                testEThread_->start();
                testSThread_->start();
                testTThread_->start();
            }
        }
        catch (exception &e)
        {
            LOG_ERR(NSQ_PREFIX, " Failed to create thread because ", e.what());
        }
    }

    virtual ~CHSNsqSpiImpl()
    {
        this->queueRunFlag_ = false;
        tradeThread1_->join();
        snapshotThread1_->join();
        entrustThread1_->join();
        tradeThread2_->join();
        snapshotThread2_->join();
        entrustThread2_->join();
    }

    virtual void OnFrontConnected();

    virtual void OnFrontDisconnected(int nResult);

    virtual void OnRspUserLogin(CHSNsqRspUserLoginField *pRspUserLogin, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    virtual void OnRspSecuDepthMarketDataSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    virtual void OnRspSecuDepthMarketDataCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    virtual void OnRtnSecuDepthMarketData(CHSNsqSecuDepthMarketDataField *pSecuDepthMarketData, HSIntVolume Bid1Volume[], HSNum Bid1Count, HSNum MaxBid1Count, HSIntVolume Ask1Volume[], HSNum Ask1Count, HSNum MaxAsk1Count);

    virtual void OnRtnSecuATPMarketData(CHSNsqSecuATPMarketDataField *pSecuDepthMarketData, HSIntVolume Bid1Volume[], HSNum Bid1Count, HSNum MaxBid1Count, HSIntVolume Ask1Volume[], HSNum Ask1Count, HSNum MaxAsk1Count);

    virtual void OnRspSecuTransactionSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    virtual void OnRspSecuTransactionCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    virtual void OnRtnSecuTransactionTradeData(CHSNsqSecuTransactionTradeDataField *pSecuTransactionTradeData);

    virtual void OnRtnSecuTransactionEntrustData(CHSNsqSecuTransactionEntrustDataField *pSecuTransactionEntrustData);

    virtual void OnRtnSecuATPTransactionTradeData(CHSNsqSecuTransactionTradeDataField *pSecuTransactionTradeData);

    virtual void OnRspQrySecuInstruments(CHSNsqSecuInstrumentStaticInfoField *pSecuInstrumentStaticInfo, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    virtual void OnRspOptDepthMarketDataSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    virtual void OnRspOptDepthMarketDataCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    virtual void OnRtnOptDepthMarketData(CHSNsqOptDepthMarketDataField *pOptDepthMarketData);

    virtual void OnRspQryOptInstruments(CHSNsqOptInstrumentStaticInfoField *pOptInstrumentStaticInfo, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    virtual void OnRspQryOptDepthMarketData(CHSNsqOptDepthMarketDataField *pOptDepthMarketData, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    virtual void OnRspSecuTransactionData(CHSNsqSecuTransactionDataField *pSecuTransactionData, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    virtual void OnRspSecuTransactionDataTimeout(int nRequestID);

    bool GetConnectStatus() { return isConnected; }

    bool GetLoginStatus() { return isLogin; }

    bool GetInstrumentsStatus() { return isAllInstrReady; }

    int GetInstrumentsCount() { return iAllInstrCount; }

    void asyncEntrustHandle(std::vector<CHSNsqSecuTransactionEntrustDataField *> &items);

    void asyncTradeHandle(std::vector<CHSNsqSecuTransactionTradeDataField *> &items);

    void asyncSnapshotHandle(std::vector<SnapData *> &items);

private:
    CHSNsqApi *lpHSMdApi;
    bool isConnected;
    bool isLogin;
    bool isAllInstrReady;
    int iAllInstrCount;

    FILE *fShStockInit;
    FILE *fSzStockInit;
    FILE *fShOptInit;
    FILE *fSzOptInit;
};