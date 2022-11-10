#pragma once

#include <fstream>
#include <time.h>
#include <sys/timeb.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif // _WIN32

#include <HSNsqApi.h>

#define NUM_OF_ROUND 1

class CHSNsqSpiImpl : public CHSNsqSpi
{

public:

	CHSNsqSpiImpl(CHSNsqApi* lpHSNsqApi)
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
	}

    ~CHSNsqSpiImpl() {}

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
    ///Others     :Bid1Volume买一队列数组, Bid1Count买一队列数组个数, MaxBid1Count买一总委托笔数
    ///           :Ask1Volume卖一队列数组, Ask1Count卖一队列数组个数, MaxAsk1Count卖一总委托笔数
	virtual void OnRtnSecuDepthMarketData(CHSNsqSecuDepthMarketDataField *pSecuDepthMarketData, HSIntVolume Bid1Volume[], HSNum Bid1Count, HSNum MaxBid1Count, HSIntVolume Ask1Volume[], HSNum Ask1Count, HSNum MaxAsk1Count);

    /// Description: 主推-现货盘后定价快照行情
    ///Others     :Bid1Volume买一队列数组, Bid1Count买一队列数组个数, MaxBid1Count买一总委托笔数
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
	virtual void OnRspOptDepthMarketDataSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	/// Description: 期权订阅取消-行情应答
	virtual void OnRspOptDepthMarketDataCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

	/// Description: 主推-期权行情
	virtual void OnRtnOptDepthMarketData(CHSNsqOptDepthMarketDataField *pOptDepthMarketData) ;

	 /// Description: 获取当前交易日合约应答
	 virtual void OnRspQryOptInstruments(CHSNsqOptInstrumentStaticInfoField *pOptInstrumentStaticInfo, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;
	 
	 /// Description: 获取合约的最新快照信息应答
	 virtual void OnRspQryOptDepthMarketData(CHSNsqOptDepthMarketDataField *pOptDepthMarketData, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;	 

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
private:
	CHSNsqApi * m_lpHSMdApi;
    bool        m_isConnected;
    bool        m_isLogined;
    bool        m_isAllInstrReady;
    int         m_iAllInstrCount;


    FILE* f_sh_stock_init;
    FILE* f_sz_stock_init;
    FILE* f_sh_opt_init;
    FILE* f_sz_opt_init;
};