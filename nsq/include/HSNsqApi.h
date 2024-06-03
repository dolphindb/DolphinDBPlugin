#ifndef _HS_NSQ_API_H_
#define _HS_NSQ_API_H_

#include "HSNsqStruct.h"


#ifdef WIN32
    #define NSQ_API_EXPORT
#else
    #ifdef HSNSQAPI_EXPORTS
        #define NSQ_API_EXPORT __attribute__((visibility("default")))
    #else
        #define NSQ_API_EXPORT
    #endif
#endif

///回调虚类
class CHSNsqSpi
{
public:
    /// Description: 当客户端与后台开始建立通信连接，连接成功后此方法被回调。
    virtual void OnFrontConnected(){};

    /// Description:当客户端与后台通信连接异常时，该方法被调用。
    /// Others     :通过GetApiErrorMsg(nResult)获取详细错误信息。
    virtual void OnFrontDisconnected(int nResult){};


    /// Description:客户登录应答
    virtual void OnRspUserLogin(CHSNsqRspUserLoginField *pRspUserLogin, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast){};

    ////以下是期货接口

    /// Description: 期货订阅-行情应答
    virtual void OnRspFutuDepthMarketDataSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

    /// Description: 期货订阅取消-行情应答
    virtual void OnRspFutuDepthMarketDataCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

    /// Description: 主推-期货行情
    virtual void OnRtnFutuDepthMarketData(CHSNsqFutuDepthMarketDataField *pFutuDepthMarketData) {};

    /// Description: 获取当前交易日合约应答
    virtual void OnRspQryFutuInstruments(CHSNsqFutuInstrumentStaticInfoField *pFutuInstrumentStaticInfo, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

    /// Description: 获取合约的最新快照信息应答
    virtual void OnRspQryFutuDepthMarketData(CHSNsqFutuDepthMarketDataField *pFutuDepthMarketData, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};


    ////以下是现货接口

    /// Description: 订阅-现货快照行情应答
    virtual void OnRspSecuDepthMarketDataSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

    /// Description: 订阅取消-现货快照行情应答
    virtual void OnRspSecuDepthMarketDataCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

    /// Description: 主推-现货快照行情
    ///Others     :Bid1Volume买一队列数组, Bid1Count买一队列数组个数, MaxBid1Count买一总委托笔数
    ///           :Ask1Volume卖一队列数组, Ask1Count卖一队列数组个数, MaxAsk1Count卖一总委托笔数
    virtual void OnRtnSecuDepthMarketData(CHSNsqSecuDepthMarketDataField *pSecuDepthMarketData, HSIntVolume Bid1Volume[], HSNum Bid1Count, HSNum MaxBid1Count, HSIntVolume Ask1Volume[], HSNum Ask1Count, HSNum MaxAsk1Count) {};


    /// Description: 主推-现货盘后定价快照行情
    ///Others     :Bid1Volume买一队列数组, Bid1Count买一队列数组个数, MaxBid1Count买一总委托笔数
    ///           :Ask1Volume卖一队列数组, Ask1Count卖一队列数组个数, MaxAsk1Count卖一总委托笔数
    virtual void OnRtnSecuATPMarketData(CHSNsqSecuATPMarketDataField *pSecuDepthMarketData, HSIntVolume Bid1Volume[], HSNum Bid1Count, HSNum MaxBid1Count, HSIntVolume Ask1Volume[], HSNum Ask1Count, HSNum MaxAsk1Count) {};


    /// Description: 订阅-现货逐笔行情应答
    virtual void OnRspSecuTransactionSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

    /// Description: 订阅取消-现货逐笔行情应答
    virtual void OnRspSecuTransactionCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

    /// Description: 主推-现货逐笔成交行情
    virtual void OnRtnSecuTransactionTradeData(CHSNsqSecuTransactionTradeDataField *pSecuTransactionTradeData) {};

    /// Description: 主推-现货逐笔委托行情
    virtual void OnRtnSecuTransactionEntrustData(CHSNsqSecuTransactionEntrustDataField *pSecuTransactionEntrustData) {};


    /// Description: 主推-现货盘后固定逐笔成交行情
    virtual void OnRtnSecuATPTransactionTradeData(CHSNsqSecuTransactionTradeDataField *pSecuTransactionTradeData) {};

    /// Description: 主推-深证新债券逐笔成交行情
    virtual void OnRtnBondTransactionTradeData(CHSNsqBondTransactionTradeDataField* pSecuTransactionTradeData) {};

    /// Description: 主推-深证新债券逐笔委托行情
    virtual void OnRtnBondTransactionEntrustData(CHSNsqBondTransactionEntrustDataField* pSecuTransactionEntrustData) {};


    /// Description: 获取当前交易日现货合约应答
    virtual void OnRspQrySecuInstruments(CHSNsqSecuInstrumentStaticInfoField *pSecuInstrumentStaticInfo, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

    /// Description: 获取现货合约的最新快照信息应答
    virtual void OnRspQrySecuDepthMarketData(CHSNsqSecuDepthMarketDataField *pSecuDepthMarketData, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};


    ////以下是期权接口

    /// Description: 期权订阅-行情应答
    virtual void OnRspOptDepthMarketDataSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

    /// Description: 期权订阅取消-行情应答
    virtual void OnRspOptDepthMarketDataCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

    /// Description: 主推-期权行情
    virtual void OnRtnOptDepthMarketData(CHSNsqOptDepthMarketDataField *pOptDepthMarketData) {};

    /// Description: 获取当前交易日合约应答
    virtual void OnRspQryOptInstruments(CHSNsqOptInstrumentStaticInfoField *pOptInstrumentStaticInfo, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

    /// Description: 获取合约的最新快照信息应答
    virtual void OnRspQryOptDepthMarketData(CHSNsqOptDepthMarketDataField *pOptDepthMarketData, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

    ////以下是现货快照Plus接口

    /// Description: 订阅-现货快照Plus行情应答
    virtual void OnRspSecuDepthMarketDataPlusSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

    /// Description: 订阅取消-现货快照Plus行情应答
    virtual void OnRspSecuDepthMarketDataPlusCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

    /// Description: 主推-现货快照Plus行情
    virtual void OnRtnSecuDepthMarketDataPlus(CHSNsqSecuDepthMarketDataPlusField *pSecuDepthMarketDataPlus) {};

    /// Description: 主推-现货快照Plus行情停止通知
    ///Others     :当“快照Plus”服务发现交易所下发的某个通道的逐笔数据存在丢包时，“快照Plus”服务会自行发起对丢失逐笔数据的重建。
    ///           在重建完成之前，“快照Plus”服务会周期性推送具体通道的停止通知，从而触发本回调。
    ///           当重建完成后，具体通道的停止通知会停止推送，同时“快照Plus”服务会继续推送相应通道重建出来的快照
    virtual void OnRtnSecuDepthMarketDataPlusStopNotice(CHSNsqSecuDepthMarketDataPlusStopNoticeField *pSecuDepthMarketDataPlusStopNotice) {};

    ////以下是现货逐笔重建接口

    /// Description: 重建应答-现货逐笔数据
    virtual void OnRspSecuTransactionData(CHSNsqSecuTransactionDataField *pSecuTransactionData, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

    /// Description: 重建应答超时-现货逐笔数据（本回调线程与其他回调线程不同）
    virtual void OnRspSecuTransactionDataTimeout(int nRequestID) {};


    ////以下是港股通接口

    /// Description: 获取当前交易日合约应答
    virtual void OnRspQryHktInstruments(CHSNsqHktInstrumentStaticInfoField *pHktInstrumentStaticInfo, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

    /// Description: 主推-港股通行情
    virtual void OnRtnHktDepthMarketData(CHSNsqHktDepthMarketDataField *pHktDepthMarketData) {};

    ////以下是通知接口

    /// Description: 主推-静态代码表数据初始化通知
    /// Others 当sailfish_service服务某个市场/业务初始化成功时(包括重新初始化)会下发该通知。
    ///        SDK登录到sailfish_service之后sailfish_service初始化成功，SDK才能收到该通知。
	///        收到该通知后，可根据使用场景重新获取对应市场/业务的静态码表信息。
    virtual void OnRtnInstrumentsDataChangeNotice(CHSNsqInstrumentsDataChangeNoticeField *pInstrumentsDataChangeNotice){};


    virtual ~CHSNsqSpi(){}
};




///行情
class CHSNsqApi
{
public:
    /// Description: 删除接口对象本身
    virtual void ReleaseApi() = 0;

    /// Description: 初始化连接
    ///              pszLicFile            通讯证书
    ///              pszSafeLevel          安全级别
    ///              pszPwd                通讯密码
    ///              pszSslFile            SSL证书
    ///              pszSslPwd             SSL密码
    virtual int Init(const char *pszLicFile,const char *pszSafeLevel = "", const char *pszPwd = "", const char *pszSslFile = "", const char *pszSslPwd = "") = 0;


    /// Description:客户登录
    virtual int  ReqUserLogin(CHSNsqReqUserLoginField *pReqUserLogin, int nRequestID) = 0;


    /// Description: API和后台建立连接，连接建立成功后，主线程会等待用户操作子线程退出
    /// Return     ：int 0表示连接建立成功，否则表示失败，通过调用GetApiErrorMsg可以获取详细错误信息
    virtual int Join() = 0;

    /// Description: 注册后台网络地址
    /// Input      : pszFrontAddress            后台网络地址  如：”tcp://127.0.0.1:17001”
    /// Return     : int 0表示设置成功，否则表示失败，通过调用GetApiErrorMsg可以获取详细错误信息
    virtual int RegisterFront(const char *pszFrontAddress) = 0;

    /// Description: 注册Fens网络地址
    /// Input      : pszFensAddress            Fens网络地址
    /// Input      : pszAccountID              账号
    /// Return     : int 0表示设置成功，否则表示失败，通过调用GetApiErrorMsg可以获取详细错误信息
    virtual int RegisterFensServer(const char *pszFensAddress, const char *pszAccountID) = 0;// 期货恒生经纪柜台专用，NSQ没用

    /// Description: 注册回调接口
    /// Input      : pSpi            派生自回调接口类的实例
    virtual void RegisterSpi(CHSNsqSpi *pSpi) = 0;


    ///以下是期货相关接口  (暂不支持)

    /// Description: 订阅-期货单腿行情请求
    /// Input      : pReqFutuDepthMarketDataSubscribe[]        行情订阅请求结构体数组
    ///              nCount                                    订阅行情合约个数。nCount为0，表示订阅pReqFutuDepthMarketDataSubscribe[0].ExchangeID全市场
    ///              nRequestID                                请求编号
    virtual int ReqFutuDepthMarketDataSubscribe(CHSNsqReqFutuDepthMarketDataField pReqFutuDepthMarketDataSubscribe[], int nCount, int nRequestID) = 0;

    /// Description: 订阅取消-单腿行情请求
    /// Input      : pReqFutuDepthMarketDataCancel[]        行情订阅请求结构体数组
    ///              nCount                                 订阅行情合约个数。nCount为0，表示取消订阅pReqFutuDepthMarketDataCancel[0].ExchangeID全市场
    ///              nRequestID                             请求编号
    virtual int ReqFutuDepthMarketDataCancel(CHSNsqReqFutuDepthMarketDataField pReqFutuDepthMarketDataCancel[], int nCount, int nRequestID) = 0;

    /// Description: 查询-获取当前交易日的合约信息
    /// Input      : pReqQryFutuInstruments[]             行情查询请求结构体数组
    ///              nCount 						      查询行情合约个数。nCount为0，表示查询pReqQryFutuInstruments[0].ExchangeID全市场
    ///              nRequestID 					      请求编号
    virtual int ReqQryFutuInstruments(CHSNsqReqFutuDepthMarketDataField pReqQryFutuInstruments[], int nCount, int nRequestID) = 0;

    /// Description: 查询-获取期货合约的最新快照信息
    /// Input      : pReqQryFutuDepthMarketData[]         行情查询请求结构体数组
    ///              nCount 						      查询行情合约个数。nCount为0，表示查询pReqQryFutuDepthMarketData[0].ExchangeID全市场
    ///              nRequestID 					      请求编号
    virtual int ReqQryFutuDepthMarketData(CHSNsqReqFutuDepthMarketDataField pReqQryFutuDepthMarketData[], int nCount, int nRequestID) = 0;



    ////以下是现货接口

    /// Description: 订阅-现货行情快照请求
    /// Input      : pReqSecuDepthMarketDataSubscribe[]         行情订阅请求结构体数组
    ///              nCount 						      		订阅行情合约个数。nCount为0，表示订阅pReqSecuDepthMarketDataSubscribe[0].ExchangeID全市场
    ///              nRequestID 					      		请求编号
    virtual int ReqSecuDepthMarketDataSubscribe(CHSNsqReqSecuDepthMarketDataField pReqSecuDepthMarketDataSubscribe[], int nCount, int nRequestID) = 0;

    /// Description: 订阅取消-现货行情快照请求
    /// Input      : pReqSecuDepthMarketDataCancel[]        行情订阅请求结构体数组
    ///              nCount                                 取消订阅行情合约个数。nCount为0，表示取消订阅pReqSecuDepthMarketDataCancel[0].ExchangeID全市场
    ///              nRequestID                             请求编号
    virtual int ReqSecuDepthMarketDataCancel(CHSNsqReqSecuDepthMarketDataField pReqSecuDepthMarketDataCancel[], int nCount, int nRequestID) = 0;

    /// Description: 订阅-现货逐笔行情请求
    /// Input      : cTransType                             逐笔类型: '0'-逐笔成交和逐笔委托; '1'-逐笔成交；'2'-逐笔委托
    ///              pReqSecuTransactionSubscribe[]         行情订阅请求结构体数组
    ///              nCount                                 订阅行情合约个数。nCount为0，表示订阅pReqSecuTransactionSubscribe[0].ExchangeID全市场
    ///              nRequestID                             请求编号
    virtual int ReqSecuTransactionSubscribe(HSTransType cTransType, CHSNsqReqSecuDepthMarketDataField pReqSecuTransactionSubscribe[], int nCount, int nRequestID) = 0;

    /// Description: 订阅取消-现货逐笔行情请求
    /// Input      : cTransType                             逐笔类型: '0'-逐笔成交和逐笔委托; '1'-逐笔成交；'2'-逐笔委托
    ///              pReqSecuTransactionCancel[]            行情订阅请求结构体数组
    ///              nCount                                 取消订阅行情合约个数。nCount为0，表示取消订阅pReqSecuTransactionCancel[0].ExchangeID全市场
    ///              nRequestID                             请求编号
    virtual int ReqSecuTransactionCancel(HSTransType cTransType, CHSNsqReqSecuDepthMarketDataField pReqSecuTransactionCancel[], int nCount, int nRequestID) = 0;


    /// Description: 当前交易日代码信息查询
    /// Input      : pReqQrySecuInstruments[]               代码信息查询请求结构体数组
    ///              nCount                                 查询行情合约个数。nCount为0，表示查询pReqQrySecuInstruments[0].ExchangeID全市场
    ///              nRequestID                             请求编号
    /// Return     : int 0表示请求成功(数据通过OnRspQrySecuInstruments返回)，否则表示失败，通过调用GetApiErrorMsg可以获取详细错误信息
    virtual int ReqQrySecuInstruments(CHSNsqReqSecuDepthMarketDataField pReqQrySecuInstruments[], int nCount, int nRequestID) = 0;

    /// Description: 查询代码的最新快照信息  (暂不支持)
    /// Input      : pReqQrySecuDepthMarketData[]           代码信息查询请求结构体数组
    ///              nCount                                 查询行情合约个数。nCount为0，表示查询pReqQrySecuDepthMarketData[0].ExchangeID全市场
    ///              nRequestID                             请求编号
    virtual int ReqQrySecuDepthMarketData(CHSNsqReqSecuDepthMarketDataField pReqQrySecuDepthMarketData[], int nCount, int nRequestID) = 0;



    ////以下是期权接口
    /// Description: 订阅-期权行情请求
    /// Input      : pReqOptDepthMarketDataSubscribe[]    行情订阅请求结构体数组
    ///              nCount                                 订阅行情合约个数。nCount为0，表示订阅pReqOptDepthMarketDataSubscribe[0].ExchangeID全市场
    ///              nRequestID                             请求编号
    virtual int ReqOptDepthMarketDataSubscribe(CHSNsqReqOptDepthMarketDataField pReqOptDepthMarketDataSubscribe[], int nCount, int nRequestID) = 0;

    /// Description: 订阅取消-期权行情快照请求
    /// Input      : pReqOptDepthMarketDataCancel[]         行情订阅请求结构体数组
    ///              nCount                                 订阅行情合约个数。nCount为0，表示取消订阅pReqOptDepthMarketDataCancel[0].ExchangeID全市场
    ///              nRequestID                             请求编号
    virtual int ReqOptDepthMarketDataCancel(CHSNsqReqOptDepthMarketDataField pReqOptDepthMarketDataCancel[], int nCount, int nRequestID) = 0;

    /// Description: 当前交易日合约信息查询
    /// Input      : pReqQryOptInstruments[]               代码信息查询请求结构体数组
    ///              nCount                                 查询行情合约个数。nCount为0，表示查询pReqQryOptInstruments[0].ExchangeID全市场
    ///              nRequestID                             请求编号
    /// Return     : int 0表示请求成功(数据通过OnRspQryOptInstruments返回)，否则表示失败，通过调用GetApiErrorMsg可以获取详细错误信息
    virtual int ReqQryOptInstruments(CHSNsqReqOptDepthMarketDataField pReqQryOptInstruments[], int nCount, int nRequestID) = 0;

    /// Description: 查询合约的最新快照信息 (暂不支持)
    /// Input      : pReqQryOptDepthMarketData[]           代码信息查询请求结构体数组
    ///              nCount                                 查询行情合约个数。nCount为0，表示查询pReqQryOptDepthMarketData[0].ExchangeID全市场
    ///              nRequestID                             请求编号
    virtual int ReqQryOptDepthMarketData(CHSNsqReqOptDepthMarketDataField pReqQryOptDepthMarketData[], int nCount, int nRequestID) = 0;



    ///////////////////////////////////////////////////////////////////////////////////////
    /// Description: 获得错误详细信息
    /// Input	   : nErrorCode 		   错误号
    /// Return	   ：错误信息
    ///////////////////////////////////////////////////////////////////////////////////////
    virtual const char* GetApiErrorMsg(int nErrorCode) = 0;

    ////以下是现货快照Plus接口

    /// Description: 订阅-现货行情快照Plus请求
    /// Input      : pReqSecuDepthMarketDataSubscribe[]         行情订阅请求结构体数组
    ///              nCount 						      		订阅行情合约个数。nCount为0，表示订阅pReqSecuDepthMarketDataSubscribe[0].ExchangeID全市场
    ///              nRequestID 					      		请求编号
    virtual int ReqSecuDepthMarketDataPlusSubscribe(CHSNsqReqSecuDepthMarketDataField pReqSecuDepthMarketDataSubscribe[], int nCount, int nRequestID) = 0;

    /// Description: 订阅取消-现货行情快照Plus请求
    /// Input      : pReqSecuDepthMarketDataCancel[]        行情订阅请求结构体数组
    ///              nCount                                 取消订阅行情合约个数。nCount为0，表示取消订阅pReqSecuDepthMarketDataCancel[0].ExchangeID全市场
    ///              nRequestID                             请求编号
    virtual int ReqSecuDepthMarketDataPlusCancel(CHSNsqReqSecuDepthMarketDataField pReqSecuDepthMarketDataCancel[], int nCount, int nRequestID) = 0;

    ////以下是现货逐笔重建接口

    /// Description: 逐笔重建请求
    /// Input      : pReqSecuTransactionRebuild             重建范围
    ///              nRequestID                             请求编号
    virtual int ReqSecuTransactionRebuild(CHSNsqReqSecuTransactionRebuildField *pReqSecuTransactionRebuild, int nRequestID) = 0;


    ////以下是港股通接口

    /// Description: 当前交易日合约信息查询
    /// Input      : pReqQryHktInstruments[]                代码信息查询请求结构体数组
    ///              nCount                                 查询行情合约个数。nCount为0，表示查询pReqQryHktInstruments[0].ExchangeID全市场
    ///              nRequestID                             请求编号
    /// Return     : int 0表示请求成功(数据通过OnRspQryHktInstruments返回)，否则表示失败，通过调用GetApiErrorMsg可以获取详细错误信息
    virtual int ReqQryHktInstruments(CHSNsqReqHktDepthMarketDataField pReqQryHktInstruments[], int nCount, int nRequestID) = 0;

protected:
    virtual ~CHSNsqApi(){};
};
#ifdef __cplusplus
extern "C"
{
#endif
    /// Description: 获取API版本号
    /// Return     : API版本号
    NSQ_API_EXPORT const char* GetNsqApiVersion();

    /// Description: 创建行情接口
    /// Input      : pszFlowPath    日志路径
    NSQ_API_EXPORT CHSNsqApi* NewNsqApi(const char *pszFlowPath);

    /// Description: 创建行情接口
    /// Input      : pszFlowPath    日志路径
    /// Input      : sdkCfgFilePath      配置文件路径
    NSQ_API_EXPORT CHSNsqApi* NewNsqApiExt(const char *pszFlowPath, const char* sdkCfgFilePath);

#ifdef __cplusplus
}
#endif
#endif
