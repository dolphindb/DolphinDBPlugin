#pragma once

#include "NsqEverything.h"
#include "NsqConnection.h"

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <unistd.h>
#endif // _WIN32

#include "NsqQueues.h"

class CHSNsqSpiImpl : public CHSNsqSpi {
  public:
    explicit CHSNsqSpiImpl(std::shared_ptr<NsqConnection> conn) : conn_(std::move(conn)) {}
    ~CHSNsqSpiImpl() override = default;

    CHSNsqSpiImpl(const CHSNsqSpiImpl&) = delete;
    CHSNsqSpiImpl& operator=(const CHSNsqSpiImpl&) = delete;

    SmartPointer<NsqQueues> queues_ = new NsqQueues();
    bool isConnected_ = false;
    string username_;
    string password_;

  private:

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

    std::shared_ptr<NsqConnection> conn_;
};

extern std::shared_ptr<CHSNsqSpiImpl> g_spi; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
