#ifndef _HS_NSQ_STRUCT_H_
#define _HS_NSQ_STRUCT_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "./HSDataType.h"
#pragma pack(push,4)
struct CHSNsqRspInfoField
{
    /// �������
    HSErrorID                     ErrorID;
    /// ������ʾ
    HSErrorMsg                    ErrorMsg;
};

///�ͻ���¼
struct CHSNsqReqUserLoginField
{
    /// �˺�
    HSAccountID                   AccountID;
    /// ����
    HSPassword                    Password;
    /// Ͷ���߶�Ӧ�����
    HSUserApplicationType         UserApplicationType;
    /// Ͷ���߶�Ӧ����Ϣ
    HSUserApplicationInfo         UserApplicationInfo;
    /// Ͷ����Mac��ַ
    HSMacAddress                  MacAddress;
    /// Ͷ����IP��ַ
    HSIPAddress                   IPAddress;
};
///�ͻ���¼Ӧ��
struct CHSNsqRspUserLoginField
{
    /// Ӫҵ����
    HSNum                         BranchID;
    /// �˺�
    HSAccountID                   AccountID;
    /// Ͷ��������
    HSUserName                    UserName;
    /// ������
    HSDate                        TradingDay;
    /// �ϸ�������
    HSDate                        PreTradingDay;
    /// �˵�ȷ�ϱ�־
    HSBillConfirmFlag             BillConfirmFlag;
    /// �Ự����
    HSSessionID                   SessionID;
    /// Ͷ���߶�Ӧ�����
    HSUserApplicationType         UserApplicationType;
    /// Ͷ���߶�Ӧ����Ϣ
    HSUserApplicationInfo         UserApplicationInfo;
    /// ���յȼ�
    HSRiskLevel                   RiskLevel;
    /// Ͷ�����ϴε�½��Mac��ַ
    HSMacAddress                  LastMacAddress;
    /// Ͷ�����ϴε�½��IP��ַ
    HSIPAddress                   LastIPAddress;
    /// �ϴε�¼�ɹ�ʱ��
    HSTime                        LastLoginTime;
    /// ֣������ǰʱ��
    HSTime                        CZCETime;
    /// ��������ǰʱ��
    HSTime                        DCETime;
    /// ��������ǰʱ��
    HSTime                        SHFETime;
    /// �н�����ǰʱ��
    HSTime                        CFFEXTime;
    /// ��Դ���ĵ�ǰʱ��
    HSTime                        INETime;
};


/// ���鶩�ģ�ȡ����������
struct CHSNsqReqDepthMarketDataField
{
    HSExchangeID                  ExchangeID;
    HSInstrumentID                InstrumentID;
};

///�����ѯ
struct CHSNsqReqQryDepthMarketDataField
{
    /// ����������
    HSExchangeID                  ExchangeID;
    /// ��Լ����
    HSInstrumentID                InstrumentID;
};


/// ���鶩�ģ�ȡ����������

//�ڻ��г�
struct CHSNsqReqFutuDepthMarketDataField
{
    HSExchangeID                  ExchangeID;
    HSInstrumentID                InstrumentID;
};

///�ڻ�������Ϣ
struct CHSNsqFutuDepthMarketDataField
{
    /// ������
    HSDate                        TradingDay;
    /// ��Լ����
    HSInstrumentID                InstrumentID;
    /// ����������
    HSExchangeID                  ExchangeID;
    /// ���¼�
    HSPrice                       LastPrice;
    /// ������
    HSPrice                       PreSettlementPrice;
    /// �����̼�
    HSPrice                       PreClosePrice;
    /// ���̼�
    HSPrice                       OpenPrice;
    /// ��߼�
    HSPrice                       HighestPrice;
    /// ��ͼ�
    HSPrice                       LowestPrice;
    /// �ɽ�����
    HSIntVolume                   TradeVolume;
    /// �ɽ����
    HSBalance                     TradeBalance;
    /// �ܳ���
    HSIntVolume                   OpenInterest;
    /// ���̼�
    HSPrice                       ClosePrice;
    /// �����
    HSPrice                       SettlementPrice;
    /// ��ͣ���
    HSPrice                       UpperLimitPrice;
    /// ��ͣ���
    HSPrice                       LowerLimitPrice;
    /// ������ʱ��
    HSTime                        UpdateTime;
    // ������
    ///�嵵�����
    HSPrice                       BidPrice[5];
    ///�嵵������
    HSPrice                       AskPrice[5];
    ///�嵵������
    HSIntVolume                   BidVolume[5];
    ///�嵵������
    HSIntVolume                   AskVolume[5];
    /// ƽ���۸�
    HSPrice                       AveragePrice;
    /// ��ֲ���
    HSIntVolume                   PreOpenInterest;
    /// ��Լ����״̬
    HSInstrumentTradeStatus       InstrumentTradeStatus;
    /// ����ʵ��(Ŀǰδ��д)
    HSDelta                       PreDelta;
    /// ����ʵ��(Ŀǰδ��д)
    HSDelta                       CurDelta;
};


struct CHSNsqFutuInstrumentStaticInfoField {
    ///����������
    HSExchangeID                  ExchangeID; 
    /// ��Լ����
    HSInstrumentID                InstrumentID;
    /// ��Լ����
    HSInstrumentName              InstrumentName;
    ///����
    HSSecurityType                SecurityType;
    ///���ռ�
    HSPrice                       PreClosePrice;
    ///��ͣ��
    HSPrice                       UpperLimitPrice;
    ///��ͣ��
    HSPrice                       LowerLimitPrice;
    ///��С�䶯��λ
    HSPrice                       PriceTick;
    ///��Լ��С������(��)
    HSNum                         BuyVolumeUnit;
    ///��Լ��С������(��)
    HSNum                         SellVolumeUnit;
    /// �������ڣ���ʽΪYYYYMMDD
    HSDate	                      TradeDate;
};

//�ֻ��г�
struct CHSNsqReqSecuDepthMarketDataField
{
    HSExchangeID                  ExchangeID;
    HSInstrumentID                InstrumentID;
};

struct CHSNsqSecuDepthMarketDataField
{
    ///����������
    HSExchangeID                  ExchangeID;
    /// ��Լ����
    HSInstrumentID                InstrumentID;
    ///ҵ�����
    ///HSSecurityType             SecurityType;
    ///���¼�
    HSPrice                       LastPrice;
    ///������
    HSPrice                       PreClosePrice;
    ///����
    HSPrice	                      OpenPrice;
    ///��߼�
    HSPrice	                      HighPrice;
    ///��ͼ�
    HSPrice                       LowPrice;
    ///������(SH)
    HSPrice	                      ClosePrice;
    ///��ͣ��(SZ)
    HSPrice	                      UpperLimitPrice;
    ///��ͣ��(SZ)
    HSPrice	                      LowerLimitPrice;
    /// �������ڣ���ʽΪYYYYMMDD
    HSDate	                      TradeDate;
    ///����ʱ��,��ʽΪHHMMSSsss
    HSTime	                      UpdateTime;
    // ��������
    ///������Ϊ�ܳɽ���(��λ�뽻����һ��)
    ///�����Ϻ�ָ����ծȯ��ع���λΪ���⣬�������͵ĵ�λ��Ϊ��
    ///20220425�����Ϻ�ծȯָ�����Ϻ�ծȯ���յ�λΪǧԪ���
    HSIntVolume                   TradeVolume;
    ///�ɽ���Ϊ�ܳɽ����(��λԪ���뽻����һ��)
    HSBalance                     TradeBalance;
    ///���վ���=(HSBalance/TradeVolume)
    HSPrice                       AveragePrice;
    // ������
    ///ʮ�������
    HSPrice                       BidPrice[10];
    ///ʮ��������
    HSPrice                       AskPrice[10];
    ///ʮ��������
    HSIntVolume                   BidVolume[10];
    ///ʮ��������
    HSIntVolume                   AskVolume[10];
    // ��������
    ///�ɽ�����
    HSNum64                       TradesNum;
    ///��ǰ����״̬˵��
    HSInstrumentTradeStatus       InstrumentTradeStatus;
    ///ί����������(SH,SZ)
    HSIntVolume                   TotalBidVolume;
    ///ί����������(SH,SZ)
    HSIntVolume                   TotalAskVolume;
    ///��Ȩƽ��ί��۸�(SH,SZ)
    HSPrice                       MaBidPrice;
    ///��Ȩƽ��ί���۸�(SH,SZ)
    HSPrice                       MaAskPrice;
    ///ծȯ��Ȩƽ��ί��۸�(SH)
    HSPrice                       MaBondBidPrice;
    ///ծȯ��Ȩƽ��ί���۸�(SH)
    HSPrice                       MaBondAskPrice;
    ///ծȯ����������(SH)
    HSRate                        YieldToMaturity;
    ///����ʵʱ�ο���ֵ(SH,SZ)
    HSPrice                       IOPV;
    ///ETF�깺����(SH,SZ)
    HSNum                         EtfBuycount;
    ///ETF��ر���(SH,SZ)
    HSNum                         EtfSellCount;
    ///ETF�깺����(SH,SZ)
    HSIntVolume                   EtfBuyVolume;
    ///ETF�깺���(SH)
    HSBalance                     EtfBuyBalance;
    ///ETF�������(SH,SZ)
    HSIntVolume                   EtfSellVolume;
    ///ETF��ؽ��(SH)
    HSBalance                     EtfSellBalance;
    ///Ȩִ֤�е�������(SH)
    HSIntVolume                   TotalWarrantExecVolume;
    ///Ȩ֤��ͣ�۸�(Ԫ)(SH)
    HSPrice                       WarrantLowerPrice;
    ///Ȩ֤��ͣ�۸�(Ԫ)(SH)
    HSPrice                       WarrantUpperPrice;
    ///���볷������(SH)
    HSNum                         CancelBuyNum;
    ///������������(SH)
    HSNum                         CancelSellNum;
    ///���볷������(SH)
    HSIntVolume                   CancelBuyVolume;
    ///������������(SH)
    HSIntVolume                   CancelSellVolume;
    ///���볷�����(SH)
    HSBalance                     CancelBuyValue;
    ///�����������(SH)
    HSBalance                     CancelSellValue;
    ///�����ܱ���(SH)
    HSNum                         TotalBuyNum;
    ///�����ܱ���(SH)
    HSNum                         TotalSellNum;
    ///����ί�гɽ����ȴ�ʱ��(SH)
    HSDurationTime                DurationAfterBuy;
    ///����ί�гɽ����ȴ�ʱ��(SH)
    HSDurationTime                DurationAfterSell;
    ///��ί�м�λ��(SH)
    HSNum                         BidOrdersNum;
    ///����ί�м�λ��(SH)
    HSNum                         AskOrdersNum;
    ///����T-1�վ�ֵ(SZ)
    HSPrice                       PreIOPV;
    ///Ƶ������(SZ)
    HSNum                         ChannelNo;

    ///ƥ��ɽ�����ɽ���(SZ ծȯ��ȯ����)
    HSPrice                       BondLastAuctionPrice;
    ///ƥ��ɽ��ɽ���(SZ ծȯ��ȯ����)
    HSIntVolume                   BondAuctionVolume;
    ///ƥ��ɽ��ɽ����(SZ ծȯ��ȯ����)
    HSBalance                     BondAuctionBalance;
    ///����۳ɽ���ʽ(SZ ծȯ��ȯ����)
    HSBondTradeType               BondLastTradeType;
    ///ծȯ���׷�ʽ��Ӧ�Ľ���״̬(SZ ծȯ��ȯ����  0ƥ��ɽ� 1Э�̳ɽ� 2����ɽ� 3ѯ�۳ɽ� 4����ɽ�)
    HSInstrumentTradeStatus       BondTradeStatus[5];
    ///����ʮ��ί�б���
    HSNum                         BidNumOrders[10];
    ///����ʮ��ί�б���
    HSNum                         AskNumOrders[10];
    ///Ԥ��
    char                          R1[16];
} ;

struct CHSNsqSecuDepthMarketDataPlusField
{
    ///����Լ����������Ƶ������
    HSNum 	                    ChannelNo;
    ///����������
    HSExchangeID                  ExchangeID;
    /// ��Լ����
    HSInstrumentID                InstrumentID;
    ///���¼�
    HSPrice                       LastPrice;
    ///����
    HSPrice	                      OpenPrice;
    ///��߼�
    HSPrice	                      HighPrice;
    ///��ͼ�
    HSPrice                       LowPrice;
    /// �������ڣ���ʽΪYYYYMMDD
    HSDate	                      TradeDate;
    ///����ʱ��,��ʽΪHHMMSSsss
    HSTime	                      UpdateTime;
    // ��������
    ///������Ϊ�ܳɽ�������λ�뽻����һ�£�
    HSIntVolume                   TradeVolume;
    ///�ɽ���Ϊ�ܳɽ�����λԪ���뽻����һ�£�
    HSBalance                     TradeBalance;
    ///ί����������(SH,SZ)
    HSIntVolume                 TotalBidVolume;
    ///ί����������(SH,SZ)
    HSIntVolume                 TotalAskVolume;
    ///�ɽ�����
    HSNum64                     TradesNum;
    // ������
    ///ʮ�������
    HSPrice                     BidPrice[10];
    ///ʮ��������
    HSIntVolume                 BidVolume[10];
    ///ʮ��������
    HSPrice                     AskPrice[10];
    ///ʮ��������
    HSIntVolume                 AskVolume[10];
} ;

struct CHSNsqSecuDepthMarketDataPlusStopNoticeField
{
    ///ֹͣ���͵�Ƶ������
    HSNum 	                       ChannelNo;
} ;

struct CHSNsqReqSecuTransactionRebuildField
{
    ///����������
    HSExchangeID  	               ExchangeID;	
    ///Ƶ������
    HSNum 	                       ChannelNo;

    // ˫������
    ///��ʼ���
    HSSeqNo     	               BeginSeqNo;
    ///�������
    HSSeqNo     	               EndSeqNo;
    ///�ؽ����ͣ�����Ͻ������������ί���ؽ�����ʳɽ��ؽ�����ֵHS_TRANS_Entrust('2')�������ί���ؽ�������ֵ��Ӧ��ʳɽ��ؽ�
    HSTransType                    RebuildType;
} ;

struct CHSNsqSecuATPMarketDataField
{
    ///����������
    HSExchangeID                  ExchangeID;
    /// ��Լ����
    HSInstrumentID                InstrumentID;
    ///������
    HSPrice	                      PreClosePrice;
    ///�����̼�
    HSPrice	                      ClosePrice;
    /// �������ڣ���ʽΪYYYYMMDD
    HSDate	                      TradeDate;
    ///����ʱ��,��ʽΪHHMMSSsss
    HSTime	                      UpdateTime;
    ///��ǰ����״̬˵��
    HSInstrumentTradeStatus       InstrumentTradeStatus;
    // ��������
    ///�̺���������λ��Ϊ�� 
    HSIntVolume	                  TradeVolume;
    ///�̺�ɽ���Ϊ�ܳɽ����(��λԪ���뽻����һ��)
    HSBalance                     TradeBalance;

    ///�̺�ɽ�����
    HSNum64	                      TradesNum;

    ///�̺�ί����������(SH)
    HSIntVolume                   TotalBidVolume;
    ///�̺�ί����������(SH)
    HSIntVolume                   TotalAskVolume;

    ///�̺����볷������(SH)
    HSNum                         CancelBuyNum;
    ///�̺�������������(SH)
    HSNum                         CancelSellNum;
    ///�̺����볷������(SH)
    HSIntVolume                   CancelBuyVolume;
    ///�̺�������������(SH)
    HSIntVolume                   CancelSellVolume;

    // �̺�������
    ///һ�������
    HSPrice	                      BidPrice1;
    ///һ��������
    HSPrice	                      AskPrice1;
    ///һ��������
    HSIntVolume	                  BidVolume1;
    ///һ��������
    HSIntVolume	                  AskVolume1;
    ///Ƶ������(SZ)
    HSNum                         ChannelNo;
    ///Ԥ��
    char                          R1[16];

};



///��ʳɽ�������Ϣ
struct CHSNsqSecuTransactionTradeDataField {
    ///����������
    HSExchangeID                  ExchangeID;
    /// ��Լ����
    HSInstrumentID                InstrumentID;
    ///����������ݱ�ʶ
	///HS_TRF_Alone(��ʶ������)����ʾ��ʳɽ������ί��SeqNo�ֶζ������
	///HS_TRF_Unified(���ͳһ���):��ʾ��ʳɽ������ί��SeqNo�ֶ�ͳһ���
    HSTransFlag                   TransFlag;
    ///��Ϣ���
    ///SH:�Ǻϲ���� �ɽ�������ţ���ͬһ��ChannelNo��Ψһ,��1��ʼ����
    ///SH:ծȯ��ʡ��ϲ����  �ɽ���ί��ͳһ��ţ���ͬһ��ChannelNo��Ψһ����1��ʼ����
    ///SZ:��ʳɽ���ί��ͳһ��ţ���ͬһ��ChannelNo��Ψһ����1��ʼ����
    HSSeqNo                       SeqNo;
    ///Ƶ������
    HSNum                         ChannelNo;
    ///�ɽ�����
    HSDate                        TradeDate;
    ///�ɽ�ʱ��
    HSTime                        TransactTime;
    ///�ɽ��۸�
    HSPrice                       TrdPrice;
    ///�ɽ���
    HSIntVolume                   TrdVolume;
    ///�ɽ����(�������Ͻ���)
    HSBalance                     TrdMoney;
    ///�򷽶�����
    HSSeqNo                       TrdBuyNo;
    ///����������
    HSSeqNo                       TrdSellNo;
    /// SH: �����̱�ʶ('B':������; 'S':������; 'N':δ֪)
    /// SZ: �ɽ���ʶ('4':��; 'F':�ɽ�)
    HSTrdType                     TrdBSFlag;
    /// SH: ��ʳɽ������ί��ͳһ���(�������Ͻ���)
    HSSeqNo                       BizIndex;
};

///���ί��������Ϣ
struct CHSNsqSecuTransactionEntrustDataField {
    ///����������
    HSExchangeID                  ExchangeID;
    /// ��Լ����
    HSInstrumentID                InstrumentID;
    ///����������ݱ�ʶ
	///HS_TRF_Alone(��ʶ������)����ʾ��ʳɽ������ί��SeqNo�ֶζ������
	///HS_TRF_Unified(���ͳһ���):��ʾ��ʳɽ������ί��SeqNo�ֶ�ͳһ���
    HSTransFlag                   TransFlag;
    ///��Ϣ���
    ///SH:�Ǻϲ���� ί�е�����ţ���ͬһ��ChannelNo��Ψһ,��1��ʼ����
    ///SH:ծȯ��ʡ��ϲ������,  �ɽ���ί�С�״̬����ͳһ��ţ���ͬһ��ChannelNo��Ψһ����1��ʼ����
    ///SZ:��ʳɽ���ί��ͳһ��ţ���ͬһ��ChannelNo��Ψһ����1��ʼ����
    HSSeqNo                       SeqNo;
    ///Ƶ������
    HSNum                         ChannelNo;
    ///ί������
    HSDate                        TradeDate;
    ///ί��ʱ��
    HSTime                        TransactTime;
    ///ί�м۸�
    HSPrice                       OrdPrice;
    ///ί������
    HSIntVolume                   OrdVolume;
    ///��������
    /// SH: ('1':��; '2':����)
    /// SZ: ('1':��; '2':��; 'G':����; 'F':����)
    HSDirection                   OrdSide;
    ///�������
    /// SH: ('A':���Ӷ���; 'D':ɾ��������'S':��Ʒ״̬����)
    /// SZ: ('1':�м�; '2':�޼�; 'U':��������)
    HSOrdType                     OrdType;
    // ��Ʒ״̬����״̬(�������Ͻ�����Ʒ״̬����, OrdType=='S')
    /// SH: ('1':ADD��Ʒδ����, '2':START����, '3':OCALL���м��Ͼ���, '4':TRADE�����Զ����)
    ///     ('5':SUSPͣ��, '6':CCALL���̼��Ͼ���, '7':CLOSE����, '8':ENDTR���׽���)
    HSTickStatusFlag              TickStatus;
    /// SH: ԭʼ������(�������Ͻ���)
    HSSeqNo                       OrdNo;
    /// SH: ��ʳɽ������ί��ͳһ���(�������Ͻ���, ʹ�úϲ���ʺ���ֶ�������)
    HSSeqNo                       BizIndex;
    /// SH: �ѳɽ�ί������(�������Ͻ�����ʺϲ�����)
    HSIntVolume                   TrdVolume;
};

struct CHSNsqBondTradeInfo {
    ///��ͣ��
    HSPrice                       UpperLimitPrice;
    ///��ͣ��
    HSPrice                       LowerLimitPrice;
};

struct CHSNsqSecuInstrumentStaticInfoField {
    ///����������
    HSExchangeID                  ExchangeID; 
    /// ��Լ����
    HSInstrumentID                InstrumentID;
    /// ��Լ����
    HSInstrumentName              InstrumentName;
    ///֤ȯ����
    HSSecurityType                SecurityType;
    ///���ռ�
    HSPrice                       PreClosePrice;
    ///��ͣ��
    HSPrice                       UpperLimitPrice;
    ///��ͣ��
    HSPrice                       LowerLimitPrice;
    ///��С�䶯��λ
    HSPrice                       PriceTick;
    ///��Լ��С������(��)
    HSNum                         BuyVolumeUnit;
    ///��Լ��С������(��)
    HSNum                         SellVolumeUnit;
    /// �������ڣ���ʽΪYYYYMMDD
    HSDate	                      TradeDate;
    ///֤ȯ������
    HSSubSecurityType             SubSecurityType;

    ///ծȯ���׷�ʽ��Ӧ�ǵ�ͣ�� (��SZ, 0ƥ��ɽ� 1Э�̳ɽ� 2����ɽ� 3ѯ�۳ɽ� 4����ɽ�)
    CHSNsqBondTradeInfo           BondtradeInfo[5];
    ///Ԥ����ʶ�ֶ�  
    HSNum64                       RsvFlag;
};


////��������Ȩ�г�
//��Ȩ�г�
struct CHSNsqReqOptDepthMarketDataField
{
    HSExchangeID                  ExchangeID;
    HSInstrumentID                InstrumentID;
};


struct CHSNsqOptInstrumentStaticInfoField {
    /// ����������
    HSExchangeID                  ExchangeID;
    /// ��Լ����
    HSInstrumentID                InstrumentID;
    /// ��Լ���״���
    HSInstrumentID                InstrumentTradeID;
    /// ��Լ����
    HSInstrumentName              InstrumentName;
    /// ��Լ����
    HSSecurityType                SecurityType;
    /// ���֤ȯ����
    HSInstrumentID                UnderlyingInstrID;
    /// ��Ȩ����:�Ϲ��Ϲ�
    HSOptionsType                 OptionsType;
    ///��Ȩ��Ȩ��ʽ:��ʽŷʽ
    HSExerciseStyle               ExerciseStyle;
    ///��Լ��λ
    HSIntVolume                   ContractMultiplierUnit;
    /// ��Ȩ��Ȩ��
    HSPrice                       ExercisePrice;
    /// �׸�������
    HSDate                        StartDate;
    /// ���������
    HSDate                        EndDate;
    /// ��Ȩ��Ȩ��
    HSDate                        ExerciseDate;
    /// ��Ȩ������
    HSDate                        DeliveryDate;
    /// ��Ȩ������
    HSDate                        ExpireDate;
    /// ��ǰ��Լδƽ����
    HSIntVolume                   TotalLongPosition;
    /// ���ռ�
    HSPrice                       PreClosePrice;
    /// ������
    HSPrice                       PreSettlPrice;
    ///���֤ȯǰ����(SH)
    HSPrice                       UnderlyingClosePrice;
    ///��ͣ��
    HSPrice                       UpperLimitPrice;
    ///��ͣ��
    HSPrice                       LowerLimitPrice;
    ///��λ��֤��
    HSBalance                     MarginUnit;
    ///��֤������������һ
    HSRate                        MarginRatioParam1;
    ///��֤��������������
    HSRate                        MarginRatioParam2;
    /// ��Լ����
    HSIntVolume                   VolumeMultiple;
    /// �޼۵���С������(SH)
    HSIntVolume                   MinLimitOrderVolume;
    /// �޼۵���󱨵���
    HSIntVolume                   MaxLimitOrderVolume;
    /// �м۵���С������(SH)
    HSIntVolume                   MinMarketOrderVolume;
    /// �м۵���󱨵���
    HSIntVolume                   MaxMarketOrderVolume;
    /// ��С�䶯��λ
    HSPrice                       PriceTick;
    /// �������ڣ���ʽΪYYYYMMDD
    HSDate	                      TradeDate;
};


struct CHSNsqOptDepthMarketDataField
{
    ///����������
    HSExchangeID                  ExchangeID;
    /// ��Լ����
    HSInstrumentID                InstrumentID;
    ///ҵ�����
    ///HSSecurityType             SecurityType;
    ///���¼�
    HSPrice                       LastPrice;
    ///������
    HSPrice                       PreClosePrice;
    ///����
    HSPrice                       OpenPrice;
    ///��߼�
    HSPrice                       HighPrice;
    ///��ͼ�
    HSPrice                       LowPrice;
    ///������
    HSPrice                       ClosePrice;
    ///���ճֲ���(��)(Ŀǰδ��д)
    HSIntVolume                   PreOpenInterest;
    ///�ֲ���(��)
    HSIntVolume                   OpenInterest;
    ///���ս����(SH)
    HSPrice                       PreSettlementPrice;
    ///���ս����
    HSPrice                       SettlementPrice;
    ///��ͣ��(SZ)
    HSPrice                       UpperLimitPrice;
    ///��ͣ��(SZ)
    HSPrice                       LowerLimitPrice;
    /// ����ʵ��(Ŀǰδ��д)
    HSDelta                       PreDelta;
    /// ����ʵ��(Ŀǰδ��д)
    HSDelta                       CurDelta;
    /// �������ڣ���ʽΪYYYYMMDD
    HSDate                        TradeDate;
    ///����ʱ��,��ʽΪHHMMSSsss
    HSTime                        UpdateTime;
    // ��������
    ///������Ϊ�ܳɽ���(��λ�ţ��뽻����һ��)
    HSIntVolume                   TradeVolume;
    ///�ɽ���Ϊ�ܳɽ����(��λԪ���뽻����һ��)
    HSBalance                     TradeBalance;
    ///���վ���(Ŀǰδ��д)
    HSPrice                       AveragePrice;
    // ������
    ///ʮ�������
    HSPrice                       BidPrice[10];
    ///ʮ��������
    HSPrice                       AskPrice[10];
    ///ʮ��������
    HSIntVolume                   BidVolume[10];
    ///ʮ��������
    HSIntVolume                   AskVolume[10];
    // ��������
    ///�ɽ�����
    HSNum64                       TradesNum;
    ///��ǰ����״̬˵��
    HSInstrumentTradeStatus       InstrumentTradeStatus;
    /// ��Լʵʱ��������(SH)
    HSOpenRestriction             OpenRestriction;
    ///�������жϲο���(SH)
    HSPrice                       AuctionPrice;
    ///�������жϼ��Ͼ�������ƥ����(SH)
    HSIntVolume                   AuctionVolume;
    ///���ѯ��ʱ��(SH)(Ŀǰδ��д)
    HSTime                        LastEnquiryTime;
    ///δƽ�ֺ�Լ��(SH)
    HSIntVolume                   LeaveQty;
    ///Ƶ������(SZ)
    HSNum                         ChannelNo;
    ///Ԥ��
    char                          R1[16];

} ;

struct CHSNsqReqHktDepthMarketDataField
{
    /// ����������
    HSExchangeID                  ExchangeID;
    /// ��Լ����
    HSInstrumentID                InstrumentID;
};

struct CHSNsqHktInstrumentStaticInfoField 
{
    /// ����������
    HSExchangeID                  ExchangeID; 
    /// ��Լ����
    HSInstrumentID                InstrumentID;
    /// ��Լ����
    HSInstrumentName              InstrumentName;
    /// ����
    HSSecurityType                SecurityType;
    /// ���ռ�
    HSPrice                       PreClosePrice;
    /// ��Լ��С������(��)
    HSNum                         BuyVolumeUnit;
    /// ��Լ��С������(��)
    HSNum                         SellVolumeUnit;
    /// �������ڣ���ʽΪYYYYMMDD
    HSDate                        TradeDate;
};


struct CHSNsqHktDepthMarketDataField 
{
    /// ����������
    HSExchangeID                  ExchangeID;
    /// ֤ȯ����
    HSInstrumentID                InstrumentID;
    /// ���¼�
    HSPrice	                      LastPrice;
    /// ������
    HSPrice	                      PreClosePrice;
    /// ��߼�
    HSPrice	                      HighPrice;
    /// ��ͼ�
    HSPrice	                      LowPrice;
    /// ���̼�(���̺�Ϊ���̼�)
    HSPrice	                      NomianlPrice;
    /// �������ڣ���ʽΪYYYYMMDD
    HSDate	                      TradeDate;
    /// ����ʱ��,��ʽΪHHMMSSsss
    HSTime	                      UpdateTime;
    /// ��������
    /// ������Ϊ�ܳɽ���(��λ�뽻����һ��) 
    /// ��λΪ��
    HSIntVolume	                  TradeVolume;
    /// �ɽ���Ϊ�ܳɽ����(��λ��Ԫ���뽻����һ��)
    HSBalance                     TradeBalance;
    /// ������ Ŀǰֻ��1������
    /// ʮ�������
    HSPrice                       BidPrice[10];
    /// ʮ��������
    HSPrice                       AskPrice[10];
    /// ʮ��������
    HSIntVolume                   BidVolume[10];
    /// ʮ��������
    HSIntVolume                   AskVolume[10];
    /// ��ǰ����״̬˵��
    HSInstrumentTradeStatus       InstrumentTradeStatus;
    /// �۹�ͨ���ֶ������������ʶ 
    HSHktTradeLimit               BoardLotOrderBidLimit;
    /// �۹�ͨ���ֶ�������������ʶ
    HSHktTradeLimit               BoardLotOrderAskLimit;
    /// �۹�ͨ��ɶ������������ʶ
    HSHktTradeLimit               OddLotOrderBidLimit;
    /// �۹�ͨ��ɶ�������������ʶ
    HSHktTradeLimit               OddLotOrderAskLimit;
    /// Ƶ������(SZ)
    HSNum                         ChannelNo;
    /// Ԥ��
    char                          R1[16];
};


///���� ��ծ��ʳɽ�������Ϣ
struct CHSNsqBondTransactionTradeDataField {
    ///����������
    HSExchangeID                  ExchangeID;
    /// ��Լ����
    HSInstrumentID                InstrumentID;
    ///����������ݱ�ʶ
    ///HS_TRF_Alone(��ʶ������)����ʾ��ʳɽ������ί��SeqNo�ֶζ������
    ///HS_TRF_Unified(���ͳһ���):��ʾ��ʳɽ������ί��SeqNo�ֶ�ͳһ���
    HSTransFlag                   TransFlag;
    ///��Ϣ���
    ///SZ:��ʳɽ���ί��ͳһ��ţ���ͬһ��ChannelNo��Ψһ����1��ʼ����
    HSSeqNo                       SeqNo;
    ///Ƶ������
    HSNum                         ChannelNo;
    ///�ɽ�����
    HSDate                        TradeDate;
    ///�ɽ�ʱ��
    HSTime                        TransactTime;
    ///�ɽ��۸�
    HSPrice                       TrdPrice;
    ///�ɽ���
    HSIntVolume                   TrdVolume;
    ///�򷽶�����
    HSSeqNo                       TrdBuyNo;
    ///����������
    HSSeqNo                       TrdSellNo;
    /// SZ: �ɽ���ʶ('4':��; 'F':�ɽ�)
    HSTrdType                     TrdBSFlag;
    /// SZ: ծȯ��ʽ��׷�ʽ
    HSBondTradeType               TradeType;
	/// ��չ��Ϣ
    /// SZ: ��������
    HSSettlPeriod                 SettlPeriod;
    /// SZ: ���㷽ʽ
    HSSettlType                   SettlType;
    /// SZ: ���򳡴α��
    HSSecondaryOrderID            SecondaryOrderID;
    /// SZ: ����ɽ���ʽ
    HSBondBidExecInstType         BidExecInstType;
    /// SZ: ��ɳɽ��ı߼ʼ۸�
    HSPrice                       MarginPrice;
};

///���ί��������Ϣ
struct CHSNsqBondTransactionEntrustDataField {
    ///����������
    HSExchangeID                  ExchangeID;
    /// ��Լ����
    HSInstrumentID                InstrumentID;
    ///����������ݱ�ʶ
    ///HS_TRF_Alone(��ʶ������)����ʾ��ʳɽ������ί��SeqNo�ֶζ������
    ///HS_TRF_Unified(���ͳһ���):��ʾ��ʳɽ������ί��SeqNo�ֶ�ͳһ���
    HSTransFlag                   TransFlag;
    ///��Ϣ���
    ///SZ:��ʳɽ���ί��ͳһ��ţ���ͬһ��ChannelNo��Ψһ����1��ʼ����
    HSSeqNo                       SeqNo;
    ///Ƶ������
    HSNum                         ChannelNo;
    ///ί������
    HSDate                        TradeDate;
    ///ί��ʱ��
    HSTime                        TransactTime;
    ///ί�м۸�
    HSPrice                       OrdPrice;
    ///ί������
    HSIntVolume                   OrdVolume;
    ///��������
    /// SZ: ('1':��; '2':��; 'G':����; 'F':����)
    HSDirection                   OrdSide;
    ///�������
    /// SZ: ('1':�м�; '2':�޼�; 'U':��������)(������ƥ�估��Ѻʽ�ع�ƥ��ɽ����)
    HSOrdType                     OrdType;
    ///ծȯ��ʽ��׷�ʽ
    HSBondTradeType               TradeType;
    /// ��չ��Ϣ
    /// SZ: ��������
    HSSettlPeriod                 SettlPeriod;
    /// SZ: ���㷽ʽ
    HSSettlType                   SettlType;
    /// SZ: ծȯ������Ϣ��� (�����ڵ���ɽ�����)
    HSQuoteID                     QuoteID;
    /// SZ: ծȯ�����̴���
    HSMemberID                    MemberID;
    /// SZ: ծȯ������������
    HSInvestorType                InvestorType;
    /// SZ: ծȯ�����������
    HSInvestorID                  InvestorID;
    /// SZ: ծȯ����Ա����
    HSTraderCode                  TraderCode;
    /// SZ: ����ҵ�����
    HSBondBidTransType            BidTransType;
    /// SZ: ����ɽ���ʽ
    HSBondBidExecInstType         BidExecInstType;
    /// SZ: ���򳡴α��
    HSSecondaryOrderID            SecondaryOrderID;
    /// SZ: ��ע
    HSBondMemo                    Memo;
    /// SZ: �۸�����
    HSPrice                       HighLimitPrice;
    /// SZ: �۸�����
    HSPrice                       LowLimitPrice;
    /// SZ: ��ͳɽ�����
    HSIntVolume                   MinQty;
    /// SZ: ���۱��۽�������(YYYYMMDD)
    HSDate                        BidTradeDate;
};


///�ؽ�Ӧ������������Ϣ
struct CHSNsqSecuTransactionDataField {
    ///��ͨ�������µ����
    HSSeqNo                 LatestSeqNo;
    ///�ؽ����ص��������
    /// SZ: ('1':��ʳɽ�; '2':���ί��; '3':��ծȯ��ʳɽ�; '4':��ծȯ���ί��)
    /// SH: ('1':��ʳɽ�; '2':���ί�� )
    HSRebuildTransType      TransType;
    union {
        CHSNsqSecuTransactionTradeDataField     TradeData;
        CHSNsqSecuTransactionEntrustDataField   EntrustData;
        CHSNsqBondTransactionTradeDataField     BondTradeData;
        CHSNsqBondTransactionEntrustDataField   BondEntrustData;
    };
};

struct CHSNsqInstrumentsDataChangeNoticeField 
{
    ///����������
    HSExchangeID                  ExchangeID;
    ///ҵ�����
    HSMarketBizType               MarketBizType;
    ///���ڣ���ʽΪYYYYMMDD
    HSDate                        TradeDate;
};

#pragma pack(pop)
#endif
