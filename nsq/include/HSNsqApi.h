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

///�ص�����
class CHSNsqSpi
{
public:
    /// Description: ���ͻ������̨��ʼ����ͨ�����ӣ����ӳɹ���˷������ص���
    virtual void OnFrontConnected(){};

    /// Description:���ͻ������̨ͨ�������쳣ʱ���÷��������á�
    /// Others     :ͨ��GetApiErrorMsg(nResult)��ȡ��ϸ������Ϣ��
    virtual void OnFrontDisconnected(int nResult){};


    /// Description:�ͻ���¼Ӧ��
    virtual void OnRspUserLogin(CHSNsqRspUserLoginField *pRspUserLogin, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast){};

    ////�������ڻ��ӿ�

    /// Description: �ڻ�����-����Ӧ��
    virtual void OnRspFutuDepthMarketDataSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

    /// Description: �ڻ�����ȡ��-����Ӧ��
    virtual void OnRspFutuDepthMarketDataCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

    /// Description: ����-�ڻ�����
    virtual void OnRtnFutuDepthMarketData(CHSNsqFutuDepthMarketDataField *pFutuDepthMarketData) {};

    /// Description: ��ȡ��ǰ�����պ�ԼӦ��
    virtual void OnRspQryFutuInstruments(CHSNsqFutuInstrumentStaticInfoField *pFutuInstrumentStaticInfo, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

    /// Description: ��ȡ��Լ�����¿�����ϢӦ��
    virtual void OnRspQryFutuDepthMarketData(CHSNsqFutuDepthMarketDataField *pFutuDepthMarketData, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};


    ////�������ֻ��ӿ�

    /// Description: ����-�ֻ���������Ӧ��
    virtual void OnRspSecuDepthMarketDataSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

    /// Description: ����ȡ��-�ֻ���������Ӧ��
    virtual void OnRspSecuDepthMarketDataCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

    /// Description: ����-�ֻ���������
    ///Others     :Bid1Volume��һ��������, Bid1Count��һ�����������, MaxBid1Count��һ��ί�б���
    ///           :Ask1Volume��һ��������, Ask1Count��һ�����������, MaxAsk1Count��һ��ί�б���
    virtual void OnRtnSecuDepthMarketData(CHSNsqSecuDepthMarketDataField *pSecuDepthMarketData, HSIntVolume Bid1Volume[], HSNum Bid1Count, HSNum MaxBid1Count, HSIntVolume Ask1Volume[], HSNum Ask1Count, HSNum MaxAsk1Count) {};


    /// Description: ����-�ֻ��̺󶨼ۿ�������
    ///Others     :Bid1Volume��һ��������, Bid1Count��һ�����������, MaxBid1Count��һ��ί�б���
    ///           :Ask1Volume��һ��������, Ask1Count��һ�����������, MaxAsk1Count��һ��ί�б���
    virtual void OnRtnSecuATPMarketData(CHSNsqSecuATPMarketDataField *pSecuDepthMarketData, HSIntVolume Bid1Volume[], HSNum Bid1Count, HSNum MaxBid1Count, HSIntVolume Ask1Volume[], HSNum Ask1Count, HSNum MaxAsk1Count) {};


    /// Description: ����-�ֻ��������Ӧ��
    virtual void OnRspSecuTransactionSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

    /// Description: ����ȡ��-�ֻ��������Ӧ��
    virtual void OnRspSecuTransactionCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

    /// Description: ����-�ֻ���ʳɽ�����
    virtual void OnRtnSecuTransactionTradeData(CHSNsqSecuTransactionTradeDataField *pSecuTransactionTradeData) {};

    /// Description: ����-�ֻ����ί������
    virtual void OnRtnSecuTransactionEntrustData(CHSNsqSecuTransactionEntrustDataField *pSecuTransactionEntrustData) {};


    /// Description: ����-�ֻ��̺�̶���ʳɽ�����
    virtual void OnRtnSecuATPTransactionTradeData(CHSNsqSecuTransactionTradeDataField *pSecuTransactionTradeData) {};

    /// Description: ����-��֤��ծȯ��ʳɽ�����
    virtual void OnRtnBondTransactionTradeData(CHSNsqBondTransactionTradeDataField* pSecuTransactionTradeData) {};

    /// Description: ����-��֤��ծȯ���ί������
    virtual void OnRtnBondTransactionEntrustData(CHSNsqBondTransactionEntrustDataField* pSecuTransactionEntrustData) {};


    /// Description: ��ȡ��ǰ�������ֻ���ԼӦ��
    virtual void OnRspQrySecuInstruments(CHSNsqSecuInstrumentStaticInfoField *pSecuInstrumentStaticInfo, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

    /// Description: ��ȡ�ֻ���Լ�����¿�����ϢӦ��
    virtual void OnRspQrySecuDepthMarketData(CHSNsqSecuDepthMarketDataField *pSecuDepthMarketData, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};


    ////��������Ȩ�ӿ�

    /// Description: ��Ȩ����-����Ӧ��
    virtual void OnRspOptDepthMarketDataSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

    /// Description: ��Ȩ����ȡ��-����Ӧ��
    virtual void OnRspOptDepthMarketDataCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

    /// Description: ����-��Ȩ����
    virtual void OnRtnOptDepthMarketData(CHSNsqOptDepthMarketDataField *pOptDepthMarketData) {};

    /// Description: ��ȡ��ǰ�����պ�ԼӦ��
    virtual void OnRspQryOptInstruments(CHSNsqOptInstrumentStaticInfoField *pOptInstrumentStaticInfo, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

    /// Description: ��ȡ��Լ�����¿�����ϢӦ��
    virtual void OnRspQryOptDepthMarketData(CHSNsqOptDepthMarketDataField *pOptDepthMarketData, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

    ////�������ֻ�����Plus�ӿ�

    /// Description: ����-�ֻ�����Plus����Ӧ��
    virtual void OnRspSecuDepthMarketDataPlusSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

    /// Description: ����ȡ��-�ֻ�����Plus����Ӧ��
    virtual void OnRspSecuDepthMarketDataPlusCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

    /// Description: ����-�ֻ�����Plus����
    virtual void OnRtnSecuDepthMarketDataPlus(CHSNsqSecuDepthMarketDataPlusField *pSecuDepthMarketDataPlus) {};

    /// Description: ����-�ֻ�����Plus����ֹ֪ͣͨ
    ///Others     :��������Plus�������ֽ������·���ĳ��ͨ����������ݴ��ڶ���ʱ��������Plus����������з���Զ�ʧ������ݵ��ؽ���
    ///           ���ؽ����֮ǰ��������Plus����������������;���ͨ����ֹ֪ͣͨ���Ӷ��������ص���
    ///           ���ؽ���ɺ󣬾���ͨ����ֹ֪ͣͨ��ֹͣ���ͣ�ͬʱ������Plus����������������Ӧͨ���ؽ������Ŀ���
    virtual void OnRtnSecuDepthMarketDataPlusStopNotice(CHSNsqSecuDepthMarketDataPlusStopNoticeField *pSecuDepthMarketDataPlusStopNotice) {};

    ////�������ֻ�����ؽ��ӿ�

    /// Description: �ؽ�Ӧ��-�ֻ��������
    virtual void OnRspSecuTransactionData(CHSNsqSecuTransactionDataField *pSecuTransactionData, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

    /// Description: �ؽ�Ӧ��ʱ-�ֻ�������ݣ����ص��߳��������ص��̲߳�ͬ��
    virtual void OnRspSecuTransactionDataTimeout(int nRequestID) {};


    ////�����Ǹ۹�ͨ�ӿ�

    /// Description: ��ȡ��ǰ�����պ�ԼӦ��
    virtual void OnRspQryHktInstruments(CHSNsqHktInstrumentStaticInfoField *pHktInstrumentStaticInfo, CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

    /// Description: ����-�۹�ͨ����
    virtual void OnRtnHktDepthMarketData(CHSNsqHktDepthMarketDataField *pHktDepthMarketData) {};

    ////������֪ͨ�ӿ�

    /// Description: ����-��̬��������ݳ�ʼ��֪ͨ
    /// Others ��sailfish_service����ĳ���г�/ҵ���ʼ���ɹ�ʱ(�������³�ʼ��)���·���֪ͨ��
    ///        SDK��¼��sailfish_service֮��sailfish_service��ʼ���ɹ���SDK�����յ���֪ͨ��
	///        �յ���֪ͨ�󣬿ɸ���ʹ�ó������»�ȡ��Ӧ�г�/ҵ��ľ�̬�����Ϣ��
    virtual void OnRtnInstrumentsDataChangeNotice(CHSNsqInstrumentsDataChangeNoticeField *pInstrumentsDataChangeNotice){};


    virtual ~CHSNsqSpi(){}
};




///����
class CHSNsqApi
{
public:
    /// Description: ɾ���ӿڶ�����
    virtual void ReleaseApi() = 0;

    /// Description: ��ʼ������
    ///              pszLicFile            ͨѶ֤��
    ///              pszSafeLevel          ��ȫ����
    ///              pszPwd                ͨѶ����
    ///              pszSslFile            SSL֤��
    ///              pszSslPwd             SSL����
    virtual int Init(const char *pszLicFile,const char *pszSafeLevel = "", const char *pszPwd = "", const char *pszSslFile = "", const char *pszSslPwd = "") = 0;


    /// Description:�ͻ���¼
    virtual int  ReqUserLogin(CHSNsqReqUserLoginField *pReqUserLogin, int nRequestID) = 0;


    /// Description: API�ͺ�̨�������ӣ����ӽ����ɹ������̻߳�ȴ��û��������߳��˳�
    /// Return     ��int 0��ʾ���ӽ����ɹ��������ʾʧ�ܣ�ͨ������GetApiErrorMsg���Ի�ȡ��ϸ������Ϣ
    virtual int Join() = 0;

    /// Description: ע���̨�����ַ
    /// Input      : pszFrontAddress            ��̨�����ַ  �磺��tcp://127.0.0.1:17001��
    /// Return     : int 0��ʾ���óɹ��������ʾʧ�ܣ�ͨ������GetApiErrorMsg���Ի�ȡ��ϸ������Ϣ
    virtual int RegisterFront(const char *pszFrontAddress) = 0;

    /// Description: ע��Fens�����ַ
    /// Input      : pszFensAddress            Fens�����ַ
    /// Input      : pszAccountID              �˺�
    /// Return     : int 0��ʾ���óɹ��������ʾʧ�ܣ�ͨ������GetApiErrorMsg���Ի�ȡ��ϸ������Ϣ
    virtual int RegisterFensServer(const char *pszFensAddress, const char *pszAccountID) = 0;// �ڻ��������͹�̨ר�ã�NSQû��

    /// Description: ע��ص��ӿ�
    /// Input      : pSpi            �����Իص��ӿ����ʵ��
    virtual void RegisterSpi(CHSNsqSpi *pSpi) = 0;


    ///�������ڻ���ؽӿ�  (�ݲ�֧��)

    /// Description: ����-�ڻ�������������
    /// Input      : pReqFutuDepthMarketDataSubscribe[]        ���鶩������ṹ������
    ///              nCount                                    ���������Լ������nCountΪ0����ʾ����pReqFutuDepthMarketDataSubscribe[0].ExchangeIDȫ�г�
    ///              nRequestID                                ������
    virtual int ReqFutuDepthMarketDataSubscribe(CHSNsqReqFutuDepthMarketDataField pReqFutuDepthMarketDataSubscribe[], int nCount, int nRequestID) = 0;

    /// Description: ����ȡ��-������������
    /// Input      : pReqFutuDepthMarketDataCancel[]        ���鶩������ṹ������
    ///              nCount                                 ���������Լ������nCountΪ0����ʾȡ������pReqFutuDepthMarketDataCancel[0].ExchangeIDȫ�г�
    ///              nRequestID                             ������
    virtual int ReqFutuDepthMarketDataCancel(CHSNsqReqFutuDepthMarketDataField pReqFutuDepthMarketDataCancel[], int nCount, int nRequestID) = 0;

    /// Description: ��ѯ-��ȡ��ǰ�����յĺ�Լ��Ϣ
    /// Input      : pReqQryFutuInstruments[]             �����ѯ����ṹ������
    ///              nCount 						      ��ѯ�����Լ������nCountΪ0����ʾ��ѯpReqQryFutuInstruments[0].ExchangeIDȫ�г�
    ///              nRequestID 					      ������
    virtual int ReqQryFutuInstruments(CHSNsqReqFutuDepthMarketDataField pReqQryFutuInstruments[], int nCount, int nRequestID) = 0;

    /// Description: ��ѯ-��ȡ�ڻ���Լ�����¿�����Ϣ
    /// Input      : pReqQryFutuDepthMarketData[]         �����ѯ����ṹ������
    ///              nCount 						      ��ѯ�����Լ������nCountΪ0����ʾ��ѯpReqQryFutuDepthMarketData[0].ExchangeIDȫ�г�
    ///              nRequestID 					      ������
    virtual int ReqQryFutuDepthMarketData(CHSNsqReqFutuDepthMarketDataField pReqQryFutuDepthMarketData[], int nCount, int nRequestID) = 0;



    ////�������ֻ��ӿ�

    /// Description: ����-�ֻ������������
    /// Input      : pReqSecuDepthMarketDataSubscribe[]         ���鶩������ṹ������
    ///              nCount 						      		���������Լ������nCountΪ0����ʾ����pReqSecuDepthMarketDataSubscribe[0].ExchangeIDȫ�г�
    ///              nRequestID 					      		������
    virtual int ReqSecuDepthMarketDataSubscribe(CHSNsqReqSecuDepthMarketDataField pReqSecuDepthMarketDataSubscribe[], int nCount, int nRequestID) = 0;

    /// Description: ����ȡ��-�ֻ������������
    /// Input      : pReqSecuDepthMarketDataCancel[]        ���鶩������ṹ������
    ///              nCount                                 ȡ�����������Լ������nCountΪ0����ʾȡ������pReqSecuDepthMarketDataCancel[0].ExchangeIDȫ�г�
    ///              nRequestID                             ������
    virtual int ReqSecuDepthMarketDataCancel(CHSNsqReqSecuDepthMarketDataField pReqSecuDepthMarketDataCancel[], int nCount, int nRequestID) = 0;

    /// Description: ����-�ֻ������������
    /// Input      : cTransType                             �������: '0'-��ʳɽ������ί��; '1'-��ʳɽ���'2'-���ί��
    ///              pReqSecuTransactionSubscribe[]         ���鶩������ṹ������
    ///              nCount                                 ���������Լ������nCountΪ0����ʾ����pReqSecuTransactionSubscribe[0].ExchangeIDȫ�г�
    ///              nRequestID                             ������
    virtual int ReqSecuTransactionSubscribe(HSTransType cTransType, CHSNsqReqSecuDepthMarketDataField pReqSecuTransactionSubscribe[], int nCount, int nRequestID) = 0;

    /// Description: ����ȡ��-�ֻ������������
    /// Input      : cTransType                             �������: '0'-��ʳɽ������ί��; '1'-��ʳɽ���'2'-���ί��
    ///              pReqSecuTransactionCancel[]            ���鶩������ṹ������
    ///              nCount                                 ȡ�����������Լ������nCountΪ0����ʾȡ������pReqSecuTransactionCancel[0].ExchangeIDȫ�г�
    ///              nRequestID                             ������
    virtual int ReqSecuTransactionCancel(HSTransType cTransType, CHSNsqReqSecuDepthMarketDataField pReqSecuTransactionCancel[], int nCount, int nRequestID) = 0;


    /// Description: ��ǰ�����մ�����Ϣ��ѯ
    /// Input      : pReqQrySecuInstruments[]               ������Ϣ��ѯ����ṹ������
    ///              nCount                                 ��ѯ�����Լ������nCountΪ0����ʾ��ѯpReqQrySecuInstruments[0].ExchangeIDȫ�г�
    ///              nRequestID                             ������
    /// Return     : int 0��ʾ����ɹ�(����ͨ��OnRspQrySecuInstruments����)�������ʾʧ�ܣ�ͨ������GetApiErrorMsg���Ի�ȡ��ϸ������Ϣ
    virtual int ReqQrySecuInstruments(CHSNsqReqSecuDepthMarketDataField pReqQrySecuInstruments[], int nCount, int nRequestID) = 0;

    /// Description: ��ѯ��������¿�����Ϣ  (�ݲ�֧��)
    /// Input      : pReqQrySecuDepthMarketData[]           ������Ϣ��ѯ����ṹ������
    ///              nCount                                 ��ѯ�����Լ������nCountΪ0����ʾ��ѯpReqQrySecuDepthMarketData[0].ExchangeIDȫ�г�
    ///              nRequestID                             ������
    virtual int ReqQrySecuDepthMarketData(CHSNsqReqSecuDepthMarketDataField pReqQrySecuDepthMarketData[], int nCount, int nRequestID) = 0;



    ////��������Ȩ�ӿ�
    /// Description: ����-��Ȩ��������
    /// Input      : pReqOptDepthMarketDataSubscribe[]    ���鶩������ṹ������
    ///              nCount                                 ���������Լ������nCountΪ0����ʾ����pReqOptDepthMarketDataSubscribe[0].ExchangeIDȫ�г�
    ///              nRequestID                             ������
    virtual int ReqOptDepthMarketDataSubscribe(CHSNsqReqOptDepthMarketDataField pReqOptDepthMarketDataSubscribe[], int nCount, int nRequestID) = 0;

    /// Description: ����ȡ��-��Ȩ�����������
    /// Input      : pReqOptDepthMarketDataCancel[]         ���鶩������ṹ������
    ///              nCount                                 ���������Լ������nCountΪ0����ʾȡ������pReqOptDepthMarketDataCancel[0].ExchangeIDȫ�г�
    ///              nRequestID                             ������
    virtual int ReqOptDepthMarketDataCancel(CHSNsqReqOptDepthMarketDataField pReqOptDepthMarketDataCancel[], int nCount, int nRequestID) = 0;

    /// Description: ��ǰ�����պ�Լ��Ϣ��ѯ
    /// Input      : pReqQryOptInstruments[]               ������Ϣ��ѯ����ṹ������
    ///              nCount                                 ��ѯ�����Լ������nCountΪ0����ʾ��ѯpReqQryOptInstruments[0].ExchangeIDȫ�г�
    ///              nRequestID                             ������
    /// Return     : int 0��ʾ����ɹ�(����ͨ��OnRspQryOptInstruments����)�������ʾʧ�ܣ�ͨ������GetApiErrorMsg���Ի�ȡ��ϸ������Ϣ
    virtual int ReqQryOptInstruments(CHSNsqReqOptDepthMarketDataField pReqQryOptInstruments[], int nCount, int nRequestID) = 0;

    /// Description: ��ѯ��Լ�����¿�����Ϣ (�ݲ�֧��)
    /// Input      : pReqQryOptDepthMarketData[]           ������Ϣ��ѯ����ṹ������
    ///              nCount                                 ��ѯ�����Լ������nCountΪ0����ʾ��ѯpReqQryOptDepthMarketData[0].ExchangeIDȫ�г�
    ///              nRequestID                             ������
    virtual int ReqQryOptDepthMarketData(CHSNsqReqOptDepthMarketDataField pReqQryOptDepthMarketData[], int nCount, int nRequestID) = 0;



    ///////////////////////////////////////////////////////////////////////////////////////
    /// Description: ��ô�����ϸ��Ϣ
    /// Input	   : nErrorCode 		   �����
    /// Return	   ��������Ϣ
    ///////////////////////////////////////////////////////////////////////////////////////
    virtual const char* GetApiErrorMsg(int nErrorCode) = 0;

    ////�������ֻ�����Plus�ӿ�

    /// Description: ����-�ֻ��������Plus����
    /// Input      : pReqSecuDepthMarketDataSubscribe[]         ���鶩������ṹ������
    ///              nCount 						      		���������Լ������nCountΪ0����ʾ����pReqSecuDepthMarketDataSubscribe[0].ExchangeIDȫ�г�
    ///              nRequestID 					      		������
    virtual int ReqSecuDepthMarketDataPlusSubscribe(CHSNsqReqSecuDepthMarketDataField pReqSecuDepthMarketDataSubscribe[], int nCount, int nRequestID) = 0;

    /// Description: ����ȡ��-�ֻ��������Plus����
    /// Input      : pReqSecuDepthMarketDataCancel[]        ���鶩������ṹ������
    ///              nCount                                 ȡ�����������Լ������nCountΪ0����ʾȡ������pReqSecuDepthMarketDataCancel[0].ExchangeIDȫ�г�
    ///              nRequestID                             ������
    virtual int ReqSecuDepthMarketDataPlusCancel(CHSNsqReqSecuDepthMarketDataField pReqSecuDepthMarketDataCancel[], int nCount, int nRequestID) = 0;

    ////�������ֻ�����ؽ��ӿ�

    /// Description: ����ؽ�����
    /// Input      : pReqSecuTransactionRebuild             �ؽ���Χ
    ///              nRequestID                             ������
    virtual int ReqSecuTransactionRebuild(CHSNsqReqSecuTransactionRebuildField *pReqSecuTransactionRebuild, int nRequestID) = 0;


    ////�����Ǹ۹�ͨ�ӿ�

    /// Description: ��ǰ�����պ�Լ��Ϣ��ѯ
    /// Input      : pReqQryHktInstruments[]                ������Ϣ��ѯ����ṹ������
    ///              nCount                                 ��ѯ�����Լ������nCountΪ0����ʾ��ѯpReqQryHktInstruments[0].ExchangeIDȫ�г�
    ///              nRequestID                             ������
    /// Return     : int 0��ʾ����ɹ�(����ͨ��OnRspQryHktInstruments����)�������ʾʧ�ܣ�ͨ������GetApiErrorMsg���Ի�ȡ��ϸ������Ϣ
    virtual int ReqQryHktInstruments(CHSNsqReqHktDepthMarketDataField pReqQryHktInstruments[], int nCount, int nRequestID) = 0;

protected:
    virtual ~CHSNsqApi(){};
};
#ifdef __cplusplus
extern "C"
{
#endif
    /// Description: ��ȡAPI�汾��
    /// Return     : API�汾��
    NSQ_API_EXPORT const char* GetNsqApiVersion();

    /// Description: ��������ӿ�
    /// Input      : pszFlowPath    ��־·��
    NSQ_API_EXPORT CHSNsqApi* NewNsqApi(const char *pszFlowPath);

    /// Description: ��������ӿ�
    /// Input      : pszFlowPath    ��־·��
    /// Input      : sdkCfgFilePath      �����ļ�·��
    NSQ_API_EXPORT CHSNsqApi* NewNsqApiExt(const char *pszFlowPath, const char* sdkCfgFilePath);

#ifdef __cplusplus
}
#endif
#endif
