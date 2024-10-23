#ifndef _HS_NSQ_STRUCT_H_
#define _HS_NSQ_STRUCT_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "./HSDataType.h"
#pragma pack(push,4)
struct CHSNsqRspInfoField
{
    /// 错误代码
    HSErrorID                     ErrorID;
    /// 错误提示
    HSErrorMsg                    ErrorMsg;
};

///客户登录
struct CHSNsqReqUserLoginField
{
    /// 账号
    HSAccountID                   AccountID;
    /// 密码
    HSPassword                    Password;
    /// 投资者端应用类别
    HSUserApplicationType         UserApplicationType;
    /// 投资者端应用信息
    HSUserApplicationInfo         UserApplicationInfo;
    /// 投资者Mac地址
    HSMacAddress                  MacAddress;
    /// 投资者IP地址
    HSIPAddress                   IPAddress;
};
///客户登录应答
struct CHSNsqRspUserLoginField
{
    /// 营业部号
    HSNum                         BranchID;
    /// 账号
    HSAccountID                   AccountID;
    /// 投资者姓名
    HSUserName                    UserName;
    /// 交易日
    HSDate                        TradingDay;
    /// 上个交易日
    HSDate                        PreTradingDay;
    /// 账单确认标志
    HSBillConfirmFlag             BillConfirmFlag;
    /// 会话编码
    HSSessionID                   SessionID;
    /// 投资者端应用类别
    HSUserApplicationType         UserApplicationType;
    /// 投资者端应用信息
    HSUserApplicationInfo         UserApplicationInfo;
    /// 风险等级
    HSRiskLevel                   RiskLevel;
    /// 投资者上次登陆的Mac地址
    HSMacAddress                  LastMacAddress;
    /// 投资者上次登陆的IP地址
    HSIPAddress                   LastIPAddress;
    /// 上次登录成功时间
    HSTime                        LastLoginTime;
    /// 郑商所当前时间
    HSTime                        CZCETime;
    /// 大商所当前时间
    HSTime                        DCETime;
    /// 上期所当前时间
    HSTime                        SHFETime;
    /// 中金所当前时间
    HSTime                        CFFEXTime;
    /// 能源中心当前时间
    HSTime                        INETime;
};


/// 行情订阅，取消订阅请求
struct CHSNsqReqDepthMarketDataField
{
    HSExchangeID                  ExchangeID;
    HSInstrumentID                InstrumentID;
};

///行情查询
struct CHSNsqReqQryDepthMarketDataField
{
    /// 交易所代码
    HSExchangeID                  ExchangeID;
    /// 合约代码
    HSInstrumentID                InstrumentID;
};


/// 行情订阅，取消订阅请求

//期货市场
struct CHSNsqReqFutuDepthMarketDataField
{
    HSExchangeID                  ExchangeID;
    HSInstrumentID                InstrumentID;
};

///期货行情信息
struct CHSNsqFutuDepthMarketDataField
{
    /// 交易日
    HSDate                        TradingDay;
    /// 合约代码
    HSInstrumentID                InstrumentID;
    /// 交易所代码
    HSExchangeID                  ExchangeID;
    /// 最新价
    HSPrice                       LastPrice;
    /// 昨结算价
    HSPrice                       PreSettlementPrice;
    /// 昨收盘价
    HSPrice                       PreClosePrice;
    /// 开盘价
    HSPrice                       OpenPrice;
    /// 最高价
    HSPrice                       HighestPrice;
    /// 最低价
    HSPrice                       LowestPrice;
    /// 成交数量
    HSIntVolume                   TradeVolume;
    /// 成交金额
    HSBalance                     TradeBalance;
    /// 总持量
    HSIntVolume                   OpenInterest;
    /// 收盘价
    HSPrice                       ClosePrice;
    /// 结算价
    HSPrice                       SettlementPrice;
    /// 涨停板价
    HSPrice                       UpperLimitPrice;
    /// 跌停板价
    HSPrice                       LowerLimitPrice;
    /// 最后更新时间
    HSTime                        UpdateTime;
    // 买卖盘
    ///五档申买价
    HSPrice                       BidPrice[5];
    ///五档申卖价
    HSPrice                       AskPrice[5];
    ///五档申买量
    HSIntVolume                   BidVolume[5];
    ///五档申卖量
    HSIntVolume                   AskVolume[5];
    /// 平均价格
    HSPrice                       AveragePrice;
    /// 昨持仓量
    HSIntVolume                   PreOpenInterest;
    /// 合约交易状态
    HSInstrumentTradeStatus       InstrumentTradeStatus;
    /// 昨虚实度(目前未填写)
    HSDelta                       PreDelta;
    /// 今虚实度(目前未填写)
    HSDelta                       CurDelta;
};


struct CHSNsqFutuInstrumentStaticInfoField {
    ///交易所代码
    HSExchangeID                  ExchangeID;
    /// 合约代码
    HSInstrumentID                InstrumentID;
    /// 合约名称
    HSInstrumentName              InstrumentName;
    ///类型
    HSSecurityType                SecurityType;
    ///昨收价
    HSPrice                       PreClosePrice;
    ///涨停价
    HSPrice                       UpperLimitPrice;
    ///跌停价
    HSPrice                       LowerLimitPrice;
    ///最小变动单位
    HSPrice                       PriceTick;
    ///合约最小交易量(买)
    HSNum                         BuyVolumeUnit;
    ///合约最小交易量(卖)
    HSNum                         SellVolumeUnit;
    /// 交易日期，格式为YYYYMMDD
    HSDate	                      TradeDate;
};

//现货市场
struct CHSNsqReqSecuDepthMarketDataField
{
    HSExchangeID                  ExchangeID;
    HSInstrumentID                InstrumentID;
};

struct CHSNsqSecuDepthMarketDataField
{
    ///交易所代码
    HSExchangeID                  ExchangeID;
    /// 合约代码
    HSInstrumentID                InstrumentID;
    ///业务类别
    ///HSSecurityType             SecurityType;
    ///最新价
    HSPrice                       LastPrice;
    ///昨收盘
    HSPrice                       PreClosePrice;
    ///今开盘
    HSPrice	                      OpenPrice;
    ///最高价
    HSPrice	                      HighPrice;
    ///最低价
    HSPrice                       LowPrice;
    ///今收盘(SH)
    HSPrice	                      ClosePrice;
    ///涨停价(SZ)
    HSPrice	                      UpperLimitPrice;
    ///跌停价(SZ)
    HSPrice	                      LowerLimitPrice;
    /// 交易日期，格式为YYYYMMDD
    HSDate	                      TradeDate;
    ///更新时间,格式为HHMMSSsss
    HSTime	                      UpdateTime;
    // 量额数据
    ///数量，为总成交量(单位与交易所一致)
    ///除了上海指数，债券与回购单位为手外，其他类型的单位都为股
    ///20220425日起，上海债券指数、上海债券快照单位为千元面额
    HSIntVolume                   TradeVolume;
    ///成交金额，为总成交金额(单位元，与交易所一致)
    HSBalance                     TradeBalance;
    ///当日均价=(HSBalance/TradeVolume)
    HSPrice                       AveragePrice;
    // 买卖盘
    ///十档申买价
    HSPrice                       BidPrice[10];
    ///十档申卖价
    HSPrice                       AskPrice[10];
    ///十档申买量
    HSIntVolume                   BidVolume[10];
    ///十档申卖量
    HSIntVolume                   AskVolume[10];
    // 额外数据
    ///成交笔数
    HSNum64                       TradesNum;
    ///当前交易状态说明
    HSInstrumentTradeStatus       InstrumentTradeStatus;
    ///委托买入总量(SH,SZ)
    HSIntVolume                   TotalBidVolume;
    ///委托卖出总量(SH,SZ)
    HSIntVolume                   TotalAskVolume;
    ///加权平均委买价格(SH,SZ)
    HSPrice                       MaBidPrice;
    ///加权平均委卖价格(SH,SZ)
    HSPrice                       MaAskPrice;
    ///债券加权平均委买价格(SH)
    HSPrice                       MaBondBidPrice;
    ///债券加权平均委卖价格(SH)
    HSPrice                       MaBondAskPrice;
    ///债券到期收益率(SH)
    HSRate                        YieldToMaturity;
    ///基金实时参考净值(SH,SZ)
    HSPrice                       IOPV;
    ///ETF申购笔数(SH,SZ)
    HSNum                         EtfBuycount;
    ///ETF赎回笔数(SH,SZ)
    HSNum                         EtfSellCount;
    ///ETF申购数量(SH,SZ)
    HSIntVolume                   EtfBuyVolume;
    ///ETF申购金额(SH)
    HSBalance                     EtfBuyBalance;
    ///ETF赎回数量(SH,SZ)
    HSIntVolume                   EtfSellVolume;
    ///ETF赎回金额(SH)
    HSBalance                     EtfSellBalance;
    ///权证执行的总数量(SH)
    HSIntVolume                   TotalWarrantExecVolume;
    ///权证跌停价格(元)(SH)
    HSPrice                       WarrantLowerPrice;
    ///权证涨停价格(元)(SH)
    HSPrice                       WarrantUpperPrice;
    ///买入撤单笔数(SH)
    HSNum                         CancelBuyNum;
    ///卖出撤单笔数(SH)
    HSNum                         CancelSellNum;
    ///买入撤单数量(SH)
    HSIntVolume                   CancelBuyVolume;
    ///卖出撤单数量(SH)
    HSIntVolume                   CancelSellVolume;
    ///买入撤单金额(SH)
    HSBalance                     CancelBuyValue;
    ///卖出撤单金额(SH)
    HSBalance                     CancelSellValue;
    ///买入总笔数(SH)
    HSNum                         TotalBuyNum;
    ///卖出总笔数(SH)
    HSNum                         TotalSellNum;
    ///买入委托成交最大等待时间(SH)
    HSDurationTime                DurationAfterBuy;
    ///卖出委托成交最大等待时间(SH)
    HSDurationTime                DurationAfterSell;
    ///买方委托价位数(SH)
    HSNum                         BidOrdersNum;
    ///卖方委托价位数(SH)
    HSNum                         AskOrdersNum;
    ///基金T-1日净值(SZ)
    HSPrice                       PreIOPV;
    ///频道代码(SZ)
    HSNum                         ChannelNo;

    ///匹配成交最近成交价(SZ 债券现券交易)
    HSPrice                       BondLastAuctionPrice;
    ///匹配成交成交量(SZ 债券现券交易)
    HSIntVolume                   BondAuctionVolume;
    ///匹配成交成交金额(SZ 债券现券交易)
    HSBalance                     BondAuctionBalance;
    ///最近价成交方式(SZ 债券现券交易)
    HSBondTradeType               BondLastTradeType;
    ///债券交易方式对应的交易状态(SZ 债券现券交易  0匹配成交 1协商成交 2点击成交 3询价成交 4竞买成交)
    HSInstrumentTradeStatus       BondTradeStatus[5];
    ///申买十档委托笔数
    HSNum                         BidNumOrders[10];
    ///申卖十档委托笔数
    HSNum                         AskNumOrders[10];
    ///预留
    char                          R1[16];
} ;

struct CHSNsqSecuDepthMarketDataPlusField
{
    ///本合约代码隶属的频道代码
    HSNum 	                    ChannelNo;
    ///交易所代码
    HSExchangeID                  ExchangeID;
    /// 合约代码
    HSInstrumentID                InstrumentID;
    ///最新价
    HSPrice                       LastPrice;
    ///今开盘
    HSPrice	                      OpenPrice;
    ///最高价
    HSPrice	                      HighPrice;
    ///最低价
    HSPrice                       LowPrice;
    /// 交易日期，格式为YYYYMMDD
    HSDate	                      TradeDate;
    ///更新时间,格式为HHMMSSsss
    HSTime	                      UpdateTime;
    // 量额数据
    ///数量，为总成交量（单位与交易所一致）
    HSIntVolume                   TradeVolume;
    ///成交金额，为总成交金额（单位元，与交易所一致）
    HSBalance                     TradeBalance;
    ///委托买入总量(SH,SZ)
    HSIntVolume                 TotalBidVolume;
    ///委托卖出总量(SH,SZ)
    HSIntVolume                 TotalAskVolume;
    ///成交笔数
    HSNum64                     TradesNum;
    // 买卖盘
    ///十档申买价
    HSPrice                     BidPrice[10];
    ///十档申买量
    HSIntVolume                 BidVolume[10];
    ///十档申卖价
    HSPrice                     AskPrice[10];
    ///十档申卖量
    HSIntVolume                 AskVolume[10];
} ;

struct CHSNsqSecuDepthMarketDataPlusStopNoticeField
{
    ///停止发送的频道代码
    HSNum 	                       ChannelNo;
} ;

struct CHSNsqReqSecuTransactionRebuildField
{
    ///交易所代码
    HSExchangeID  	               ExchangeID;
    ///频道代码
    HSNum 	                       ChannelNo;

    // 双闭区间
    ///起始序号
    HSSeqNo     	               BeginSeqNo;
    ///结束序号
    HSSeqNo     	               EndSeqNo;
    ///重建类型，针对上交所。区分逐笔委托重建和逐笔成交重建，填值HS_TRANS_Entrust('2')请求逐笔委托重建，其余值对应逐笔成交重建
    HSTransType                    RebuildType;
} ;

struct CHSNsqSecuATPMarketDataField
{
    ///交易所代码
    HSExchangeID                  ExchangeID;
    /// 合约代码
    HSInstrumentID                InstrumentID;
    ///昨收盘
    HSPrice	                      PreClosePrice;
    ///今收盘价
    HSPrice	                      ClosePrice;
    /// 交易日期，格式为YYYYMMDD
    HSDate	                      TradeDate;
    ///更新时间,格式为HHMMSSsss
    HSTime	                      UpdateTime;
    ///当前交易状态说明
    HSInstrumentTradeStatus       InstrumentTradeStatus;
    // 量额数据
    ///盘后数量，单位都为股
    HSIntVolume	                  TradeVolume;
    ///盘后成交金额，为总成交金额(单位元，与交易所一致)
    HSBalance                     TradeBalance;

    ///盘后成交笔数
    HSNum64	                      TradesNum;

    ///盘后委托买入总量(SH)
    HSIntVolume                   TotalBidVolume;
    ///盘后委托卖出总量(SH)
    HSIntVolume                   TotalAskVolume;

    ///盘后买入撤单笔数(SH)
    HSNum                         CancelBuyNum;
    ///盘后卖出撤单笔数(SH)
    HSNum                         CancelSellNum;
    ///盘后买入撤单数量(SH)
    HSIntVolume                   CancelBuyVolume;
    ///盘后卖出撤单数量(SH)
    HSIntVolume                   CancelSellVolume;

    // 盘后买卖盘
    ///一档申买价
    HSPrice	                      BidPrice1;
    ///一档申卖价
    HSPrice	                      AskPrice1;
    ///一档申买量
    HSIntVolume	                  BidVolume1;
    ///一档申卖量
    HSIntVolume	                  AskVolume1;
    ///频道代码(SZ)
    HSNum                         ChannelNo;
    ///预留
    char                          R1[16];

};



///逐笔成交数据信息
struct CHSNsqSecuTransactionTradeDataField {
    ///交易所代码
    HSExchangeID                  ExchangeID;
    /// 合约代码
    HSInstrumentID                InstrumentID;
    ///逐笔行情数据标识
	///HS_TRF_Alone(逐笔独立编号)：表示逐笔成交与逐笔委托SeqNo字段独立编号
	///HS_TRF_Unified(逐笔统一编号):表示逐笔成交与逐笔委托SeqNo字段统一编号
    HSTransFlag                   TransFlag;
    ///消息序号
    ///SH:非合并逐笔 成交单独序号，在同一个ChannelNo内唯一,从1开始连续
    ///SH:债券逐笔、合并逐笔  成交与委托统一序号，在同一个ChannelNo内唯一，从1开始连续
    ///SZ:逐笔成交与委托统一序号，在同一个ChannelNo内唯一，从1开始连续
    HSSeqNo                       SeqNo;
    ///频道代码
    HSNum                         ChannelNo;
    ///成交日期
    HSDate                        TradeDate;
    ///成交时间
    HSTime                        TransactTime;
    ///成交价格
    HSPrice                       TrdPrice;
    ///成交量
    HSIntVolume                   TrdVolume;
    ///成交金额(仅适用上交所)
    HSBalance                     TrdMoney;
    ///买方订单号
    HSSeqNo                       TrdBuyNo;
    ///卖方订单号
    HSSeqNo                       TrdSellNo;
    /// SH: 内外盘标识('B':主动买; 'S':主动卖; 'N':未知)
    /// SZ: 成交标识('4':撤; 'F':成交)
    HSTrdType                     TrdBSFlag;
    /// SH: 逐笔成交与逐笔委托统一序号(仅适用上交所)
    HSSeqNo                       BizIndex;
};

///逐笔委托数据信息
struct CHSNsqSecuTransactionEntrustDataField {
    ///交易所代码
    HSExchangeID                  ExchangeID;
    /// 合约代码
    HSInstrumentID                InstrumentID;
    ///逐笔行情数据标识
	///HS_TRF_Alone(逐笔独立编号)：表示逐笔成交与逐笔委托SeqNo字段独立编号
	///HS_TRF_Unified(逐笔统一编号):表示逐笔成交与逐笔委托SeqNo字段统一编号
    HSTransFlag                   TransFlag;
    ///消息序号
    ///SH:非合并逐笔 委托单独序号，在同一个ChannelNo内唯一,从1开始连续
    ///SH:债券逐笔、合并后逐笔,  成交与委托、状态订单统一序号，在同一个ChannelNo内唯一，从1开始连续
    ///SZ:逐笔成交与委托统一序号，在同一个ChannelNo内唯一，从1开始连续
    HSSeqNo                       SeqNo;
    ///频道代码
    HSNum                         ChannelNo;
    ///委托日期
    HSDate                        TradeDate;
    ///委托时间
    HSTime                        TransactTime;
    ///委托价格
    HSPrice                       OrdPrice;
    ///委托数量
    HSIntVolume                   OrdVolume;
    ///买卖方向
    /// SH: ('1':买单; '2':卖单)
    /// SZ: ('1':买; '2':卖; 'G':借入; 'F':出借)
    HSDirection                   OrdSide;
    ///订单类别
    /// SH: ('A':增加订单; 'D':删除订单；'S':产品状态订单)
    /// SZ: ('1':市价; '2':限价; 'U':本方最优)
    HSOrdType                     OrdType;
    // 产品状态订单状态(仅适用上交所产品状态订单, OrdType=='S')
    /// SH: ('1':ADD产品未上市, '2':START启动, '3':OCALL开市集合竞价, '4':TRADE连续自动撮合)
    ///     ('5':SUSP停牌, '6':CCALL收盘集合竞价, '7':CLOSE闭市, '8':ENDTR交易结束)
    HSTickStatusFlag              TickStatus;
    /// SH: 原始订单号(仅适用上交所)
    HSSeqNo                       OrdNo;
    /// SH: 逐笔成交与逐笔委托统一编号(仅适用上交所, 使用合并逐笔后该字段无意义)
    HSSeqNo                       BizIndex;
    /// SH: 已成交委托数量(仅适用上交所逐笔合并数据)
    HSIntVolume                   TrdVolume;
};

struct CHSNsqBondTradeInfo {
    ///涨停价
    HSPrice                       UpperLimitPrice;
    ///跌停价
    HSPrice                       LowerLimitPrice;
};

struct CHSNsqSecuInstrumentStaticInfoField {
    ///交易所代码
    HSExchangeID                  ExchangeID;
    /// 合约代码
    HSInstrumentID                InstrumentID;
    /// 合约名称
    HSInstrumentName              InstrumentName;
    ///证券类型
    HSSecurityType                SecurityType;
    ///昨收价
    HSPrice                       PreClosePrice;
    ///涨停价
    HSPrice                       UpperLimitPrice;
    ///跌停价
    HSPrice                       LowerLimitPrice;
    ///最小变动单位
    HSPrice                       PriceTick;
    ///合约最小交易量(买)
    HSNum                         BuyVolumeUnit;
    ///合约最小交易量(卖)
    HSNum                         SellVolumeUnit;
    /// 交易日期，格式为YYYYMMDD
    HSDate	                      TradeDate;
    ///证券子类型
    HSSubSecurityType             SubSecurityType;

    ///债券交易方式对应涨跌停价 (仅SZ, 0匹配成交 1协商成交 2点击成交 3询价成交 4竞买成交)
    CHSNsqBondTradeInfo           BondtradeInfo[5];
    ///预留标识字段
    HSNum64                       RsvFlag;
};


////以下是期权市场
//期权市场
struct CHSNsqReqOptDepthMarketDataField
{
    HSExchangeID                  ExchangeID;
    HSInstrumentID                InstrumentID;
};


struct CHSNsqOptInstrumentStaticInfoField {
    /// 交易所代码
    HSExchangeID                  ExchangeID;
    /// 合约代码
    HSInstrumentID                InstrumentID;
    /// 合约交易代码
    HSInstrumentID                InstrumentTradeID;
    /// 合约名称
    HSInstrumentName              InstrumentName;
    /// 合约类型
    HSSecurityType                SecurityType;
    /// 标的证券代码
    HSInstrumentID                UnderlyingInstrID;
    /// 期权类型:认购认沽
    HSOptionsType                 OptionsType;
    ///期权行权方式:美式欧式
    HSExerciseStyle               ExerciseStyle;
    ///合约单位
    HSIntVolume                   ContractMultiplierUnit;
    /// 期权行权价
    HSPrice                       ExercisePrice;
    /// 首个交易日
    HSDate                        StartDate;
    /// 最后交易日日
    HSDate                        EndDate;
    /// 期权行权日
    HSDate                        ExerciseDate;
    /// 行权交割日
    HSDate                        DeliveryDate;
    /// 期权到期日
    HSDate                        ExpireDate;
    /// 当前合约未平仓数
    HSIntVolume                   TotalLongPosition;
    /// 昨收价
    HSPrice                       PreClosePrice;
    /// 昨结算价
    HSPrice                       PreSettlPrice;
    ///标的证券前收盘(SH)
    HSPrice                       UnderlyingClosePrice;
    ///涨停价
    HSPrice                       UpperLimitPrice;
    ///跌停价
    HSPrice                       LowerLimitPrice;
    ///单位保证金
    HSBalance                     MarginUnit;
    ///保证金计算比例参数一
    HSRate                        MarginRatioParam1;
    ///保证金计算比例参数二
    HSRate                        MarginRatioParam2;
    /// 合约乘数
    HSIntVolume                   VolumeMultiple;
    /// 限价单最小报单量(SH)
    HSIntVolume                   MinLimitOrderVolume;
    /// 限价单最大报单量
    HSIntVolume                   MaxLimitOrderVolume;
    /// 市价单最小报单量(SH)
    HSIntVolume                   MinMarketOrderVolume;
    /// 市价单最大报单量
    HSIntVolume                   MaxMarketOrderVolume;
    /// 最小变动价位
    HSPrice                       PriceTick;
    /// 交易日期，格式为YYYYMMDD
    HSDate	                      TradeDate;
};


struct CHSNsqOptDepthMarketDataField
{
    ///交易所代码
    HSExchangeID                  ExchangeID;
    /// 合约代码
    HSInstrumentID                InstrumentID;
    ///业务类别
    ///HSSecurityType             SecurityType;
    ///最新价
    HSPrice                       LastPrice;
    ///昨收盘
    HSPrice                       PreClosePrice;
    ///今开盘
    HSPrice                       OpenPrice;
    ///最高价
    HSPrice                       HighPrice;
    ///最低价
    HSPrice                       LowPrice;
    ///今收盘
    HSPrice                       ClosePrice;
    ///昨日持仓量(张)(目前未填写)
    HSIntVolume                   PreOpenInterest;
    ///持仓量(张)
    HSIntVolume                   OpenInterest;
    ///昨日结算价(SH)
    HSPrice                       PreSettlementPrice;
    ///今日结算价
    HSPrice                       SettlementPrice;
    ///涨停价(SZ)
    HSPrice                       UpperLimitPrice;
    ///跌停价(SZ)
    HSPrice                       LowerLimitPrice;
    /// 昨虚实度(目前未填写)
    HSDelta                       PreDelta;
    /// 今虚实度(目前未填写)
    HSDelta                       CurDelta;
    /// 交易日期，格式为YYYYMMDD
    HSDate                        TradeDate;
    ///更新时间,格式为HHMMSSsss
    HSTime                        UpdateTime;
    // 量额数据
    ///数量，为总成交量(单位张，与交易所一致)
    HSIntVolume                   TradeVolume;
    ///成交金额，为总成交金额(单位元，与交易所一致)
    HSBalance                     TradeBalance;
    ///当日均价(目前未填写)
    HSPrice                       AveragePrice;
    // 买卖盘
    ///十档申买价
    HSPrice                       BidPrice[10];
    ///十档申卖价
    HSPrice                       AskPrice[10];
    ///十档申买量
    HSIntVolume                   BidVolume[10];
    ///十档申卖量
    HSIntVolume                   AskVolume[10];
    // 额外数据
    ///成交笔数
    HSNum64                       TradesNum;
    ///当前交易状态说明
    HSInstrumentTradeStatus       InstrumentTradeStatus;
    /// 合约实时开仓限制(SH)
    HSOpenRestriction             OpenRestriction;
    ///波段性中断参考价(SH)
    HSPrice                       AuctionPrice;
    ///波段性中断集合竞价虚拟匹配量(SH)
    HSIntVolume                   AuctionVolume;
    ///最近询价时间(SH)(目前未填写)
    HSTime                        LastEnquiryTime;
    ///未平仓合约数(SH)
    HSIntVolume                   LeaveQty;
    ///频道代码(SZ)
    HSNum                         ChannelNo;
    ///预留
    char                          R1[16];

} ;

struct CHSNsqReqHktDepthMarketDataField
{
    /// 交易所代码
    HSExchangeID                  ExchangeID;
    /// 合约代码
    HSInstrumentID                InstrumentID;
};

struct CHSNsqHktInstrumentStaticInfoField
{
    /// 交易所代码
    HSExchangeID                  ExchangeID;
    /// 合约代码
    HSInstrumentID                InstrumentID;
    /// 合约名称
    HSInstrumentName              InstrumentName;
    /// 类型
    HSSecurityType                SecurityType;
    /// 昨收价
    HSPrice                       PreClosePrice;
    /// 合约最小交易量(买)
    HSNum                         BuyVolumeUnit;
    /// 合约最小交易量(卖)
    HSNum                         SellVolumeUnit;
    /// 交易日期，格式为YYYYMMDD
    HSDate                        TradeDate;
};


struct CHSNsqHktDepthMarketDataField
{
    /// 交易所代码
    HSExchangeID                  ExchangeID;
    /// 证券代码
    HSInstrumentID                InstrumentID;
    /// 最新价
    HSPrice	                      LastPrice;
    /// 昨收盘
    HSPrice	                      PreClosePrice;
    /// 最高价
    HSPrice	                      HighPrice;
    /// 最低价
    HSPrice	                      LowPrice;
    /// 按盘价(收盘后为收盘价)
    HSPrice	                      NomianlPrice;
    /// 交易日期，格式为YYYYMMDD
    HSDate	                      TradeDate;
    /// 更新时间,格式为HHMMSSsss
    HSTime	                      UpdateTime;
    /// 量额数据
    /// 数量，为总成交量(单位与交易所一致)
    /// 单位为股
    HSIntVolume	                  TradeVolume;
    /// 成交金额，为总成交金额(单位港元，与交易所一致)
    HSBalance                     TradeBalance;
    /// 买卖盘 目前只有1档数据
    /// 十档申买价
    HSPrice                       BidPrice[10];
    /// 十档申卖价
    HSPrice                       AskPrice[10];
    /// 十档申买量
    HSIntVolume                   BidVolume[10];
    /// 十档申卖量
    HSIntVolume                   AskVolume[10];
    /// 当前交易状态说明
    HSInstrumentTradeStatus       InstrumentTradeStatus;
    /// 港股通整手订单限制买入标识
    HSHktTradeLimit               BoardLotOrderBidLimit;
    /// 港股通整手订单限制卖出标识
    HSHktTradeLimit               BoardLotOrderAskLimit;
    /// 港股通零股订单限制买入标识
    HSHktTradeLimit               OddLotOrderBidLimit;
    /// 港股通零股订单限制卖出标识
    HSHktTradeLimit               OddLotOrderAskLimit;
    /// 频道代码(SZ)
    HSNum                         ChannelNo;
    /// 预留
    char                          R1[16];
};


///深圳 新债逐笔成交数据信息
struct CHSNsqBondTransactionTradeDataField {
    ///交易所代码
    HSExchangeID                  ExchangeID;
    /// 合约代码
    HSInstrumentID                InstrumentID;
    ///逐笔行情数据标识
    ///HS_TRF_Alone(逐笔独立编号)：表示逐笔成交与逐笔委托SeqNo字段独立编号
    ///HS_TRF_Unified(逐笔统一编号):表示逐笔成交与逐笔委托SeqNo字段统一编号
    HSTransFlag                   TransFlag;
    ///消息序号
    ///SZ:逐笔成交与委托统一序号，在同一个ChannelNo内唯一，从1开始连续
    HSSeqNo                       SeqNo;
    ///频道代码
    HSNum                         ChannelNo;
    ///成交日期
    HSDate                        TradeDate;
    ///成交时间
    HSTime                        TransactTime;
    ///成交价格
    HSPrice                       TrdPrice;
    ///成交量
    HSIntVolume                   TrdVolume;
    ///买方订单号
    HSSeqNo                       TrdBuyNo;
    ///卖方订单号
    HSSeqNo                       TrdSellNo;
    /// SZ: 成交标识('4':撤; 'F':成交)
    HSTrdType                     TrdBSFlag;
    /// SZ: 债券逐笔交易方式
    HSBondTradeType               TradeType;
	/// 扩展信息
    /// SZ: 结算周期
    HSSettlPeriod                 SettlPeriod;
    /// SZ: 结算方式
    HSSettlType                   SettlType;
    /// SZ: 竞买场次编号
    HSSecondaryOrderID            SecondaryOrderID;
    /// SZ: 竞买成交方式
    HSBondBidExecInstType         BidExecInstType;
    /// SZ: 达成成交的边际价格
    HSPrice                       MarginPrice;
};

///逐笔委托数据信息
struct CHSNsqBondTransactionEntrustDataField {
    ///交易所代码
    HSExchangeID                  ExchangeID;
    /// 合约代码
    HSInstrumentID                InstrumentID;
    ///逐笔行情数据标识
    ///HS_TRF_Alone(逐笔独立编号)：表示逐笔成交与逐笔委托SeqNo字段独立编号
    ///HS_TRF_Unified(逐笔统一编号):表示逐笔成交与逐笔委托SeqNo字段统一编号
    HSTransFlag                   TransFlag;
    ///消息序号
    ///SZ:逐笔成交与委托统一序号，在同一个ChannelNo内唯一，从1开始连续
    HSSeqNo                       SeqNo;
    ///频道代码
    HSNum                         ChannelNo;
    ///委托日期
    HSDate                        TradeDate;
    ///委托时间
    HSTime                        TransactTime;
    ///委托价格
    HSPrice                       OrdPrice;
    ///委托数量
    HSIntVolume                   OrdVolume;
    ///买卖方向
    /// SZ: ('1':买; '2':卖; 'G':借入; 'F':出借)
    HSDirection                   OrdSide;
    ///订单类别
    /// SZ: ('1':市价; '2':限价; 'U':本方最优)(适用于匹配及质押式回购匹配成交逐笔)
    HSOrdType                     OrdType;
    ///债券逐笔交易方式
    HSBondTradeType               TradeType;
    /// 扩展信息
    /// SZ: 结算周期
    HSSettlPeriod                 SettlPeriod;
    /// SZ: 结算方式
    HSSettlType                   SettlType;
    /// SZ: 债券报价消息编号 (适用于点击成交行情)
    HSQuoteID                     QuoteID;
    /// SZ: 债券交易商代码
    HSMemberID                    MemberID;
    /// SZ: 债券交易主体类型
    HSInvestorType                InvestorType;
    /// SZ: 债券交易主体代码
    HSInvestorID                  InvestorID;
    /// SZ: 债券交易员代码
    HSTraderCode                  TraderCode;
    /// SZ: 竞买业务类别
    HSBondBidTransType            BidTransType;
    /// SZ: 竞买成交方式
    HSBondBidExecInstType         BidExecInstType;
    /// SZ: 竞买场次编号
    HSSecondaryOrderID            SecondaryOrderID;
    /// SZ: 备注
    HSBondMemo                    Memo;
    /// SZ: 价格上限
    HSPrice                       HighLimitPrice;
    /// SZ: 价格下限
    HSPrice                       LowLimitPrice;
    /// SZ: 最低成交数量
    HSIntVolume                   MinQty;
    /// SZ: 竞价报价交易日期(YYYYMMDD)
    HSDate                        BidTradeDate;
};


///重建应答的逐笔数据信息
struct CHSNsqSecuTransactionDataField {
    ///该通道下最新的序号
    HSSeqNo                 LatestSeqNo;
    ///重建返回的逐笔类型
    /// SZ: ('1':逐笔成交; '2':逐笔委托; '3':新债券逐笔成交; '4':新债券逐笔委托)
    /// SH: ('1':逐笔成交; '2':逐笔委托 )
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
    ///交易所代码
    HSExchangeID                  ExchangeID;
    ///业务类别
    HSMarketBizType               MarketBizType;
    ///日期，格式为YYYYMMDD
    HSDate                        TradeDate;
};

#pragma pack(pop)
#endif
