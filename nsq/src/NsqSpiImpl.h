//
// Created by htxu on 11/20/2023.
//

#ifndef PLUGINNSQ_NSQSPIIMPL_H
#define PLUGINNSQ_NSQSPIIMPL_H

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <unistd.h>
#endif // _WIN32

#include "HSNsqApi.h"
#include "NsqQueues.h"

class CHSNsqSpiImpl : public CHSNsqSpi {

    /// Connection

    void OnFrontConnected() override;

    void OnRspUserLogin(CHSNsqRspUserLoginField *pRspUserLogin, CHSNsqRspInfoField *pRspInfo, int nRequestID,
                        bool bIsLast) override;

    void OnFrontDisconnected(int nResult) override;

    /// Snapshot

    void OnRspSecuDepthMarketDataSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

    void OnRspSecuDepthMarketDataCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

    void OnRtnSecuDepthMarketData(CHSNsqSecuDepthMarketDataField *pSecuDepthMarketData, HSIntVolume Bid1Volume[],
                                  HSNum Bid1Count, HSNum MaxBid1Count, HSIntVolume Ask1Volume[], HSNum Ask1Count,
                                  HSNum MaxAsk1Count) override;

    /// Trade and Entrust

    void OnRspSecuTransactionSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

    void OnRspSecuTransactionCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

    void OnRtnSecuTransactionTradeData(CHSNsqSecuTransactionTradeDataField *pSecuTransactionTradeData) override;

    void OnRtnSecuTransactionEntrustData(CHSNsqSecuTransactionEntrustDataField *pSecuTransactionEntrustData) override;

public:
    virtual ~CHSNsqSpiImpl() = default;

    SmartPointer<NsqQueues> queues_ = new NsqQueues();
    bool isConnected_ = false;
    string username_;
    string password_;
};


#endif //PLUGINNSQ_NSQSPIIMPL_H
