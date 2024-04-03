/**
 * @file         ama_struct.h
 * @author       郭光葵
 * @mail         guoguangkui@archforce.com.cn
 * @created time Thu 21 Sep 2017 09:50:15 AM CST
 *
 * Copyright (c) 2018 Archforce Financial Technology.  All rights reserved.
 * Redistribution and use in source and binary forms, with or without  modification, are not permitted.
 * For more information about Archforce, welcome to archforce.cn.
 */

#ifndef AMD_AMA_STRUCT_H_
#define AMD_AMA_STRUCT_H_

#include "ama_datatype.h"
#include <vector>

#pragma pack(push)
#pragma pack(1)

namespace amd { namespace ama {

/*
    精度说明：
    本文中所有字段带有类型说明的相比如实际值有扩大倍数，详细倍数扩大规则如下(没有类型说明的就无扩大倍数)：
    类型:数量 (如成交量、报买量等)的扩大倍数是  10^4
    类型:价格 (如参考价、报买价等)的扩大倍数是  10^6
    类型:金额 (如拆借金额等)的扩大倍数是  10^5
    类型:比例 (如最新利率、开盘利率等)的扩大倍数  10^6
*/


/**
 * @name 连接UMS信息
 * @{ */
struct UMSItem
{
    char        local_ip[ConstField::kIPMaxLen];                    // 本地IP地址,为空默认为0.0.0.0
    char        server_ip[ConstField::kIPMaxLen];                   // 服务IP
    uint16_t    server_port;                                        // 服务端口
};
/**  @} */

/**
 * @name 配置定义
 * @{ */
struct Cfg
{
    //--------------------------------全局配置信息---------------------------------------------------------
    uint64_t    channel_mode;                                       // 通道模式的集合,请参考 ChannelMode,该配置为各通道模式的集合
    uint32_t    ha_mode;                                            // 高可用工作模式,请参考 HighAvailableMode
    int32_t     min_log_level;                                      // 日志最小级别,请参考 LogLevel
    bool        is_output_mon_data;                                 // 是否输出监控数据的配置,true-输出监控数据,false-不输出监控数据
    bool        is_thread_safe;                                     // 回调接口是否保证线程安全,true-启用线程安全模式执行回调接口,false-非线程安全模式执行回调接口

    bool        keep_order;                                         // 逐笔保序标志,true-开启保序,false-开启不保序
    uint32_t    keep_order_timeout_ms;                              // 逐笔保序超时时间(单位:毫秒),keep_order=true时有效
    bool        is_subscribe_full;                                  // 默认是否订阅全部数据,true-默认订阅全部,false-默认不订阅任何数据

    //-------------------------------UMS服务的连接信息------------------------------------------------------
    UMSItem     ums_servers[ConstField::kUMSItemLen];               // UMS的服务信息项,该信息不能超过8个
    uint32_t    ums_server_cnt;                                     // UMS的服务信息项个数, 小于1将启动失败

    char        username[ConstField::kUsernameLen];                 // 用户名
    char        password[ConstField::kPasswordLen];                 // 用户密码, 明文填入,密文使用

    uint32_t    tcp_compress_mode;                                  // TCP模式传输数据压缩标志,0:不压缩 1:自定义压缩 2:zstd压缩(仅TCP模式有效)

    //-------------------------------委托簿配置------------------------------------------------------
    uint8_t     enable_order_book;                                  // 0:不启用委托簿 1：客户端本地构建委托簿 2：服务端主动推送委托簿数据
    uint16_t    entry_size;                                         // 委托簿档位
    uint8_t     thread_num;                                         // 委托簿并行计算线程数
    uint8_t     order_queue_size;                                   // 每个价位委托揭示笔数,最高50
    uint32_t    order_book_deliver_interval_microsecond;            // 委托簿递交时间间隔(微秒级)
    Cfg()
    {
        channel_mode = 0;
        ha_mode = HighAvailableMode::kRegularDataFilter;
        min_log_level = LogLevel::kInfo;
        is_output_mon_data = false;
        is_thread_safe = false;
        keep_order = false;
        keep_order_timeout_ms = 3000;
        is_subscribe_full = false;
        ums_server_cnt = 0;
        tcp_compress_mode = 0;
        enable_order_book = OrderBookType::kNone;
        entry_size        = 10;
        thread_num        = 3;
        order_queue_size  = 0;
        order_book_deliver_interval_microsecond = 0;
    }
};
/**  @} */

/**
 * @name 订阅数据项定义
 * @{ */
struct SubscribeItem
{
    int32_t market;                                                 /* 市场类型,参考 MarketType(kHKEx 类型暂时不支持)
                                                                         为0表示订阅所有支持的市场. */
    uint64_t flag;                                                  // 各数据类型的集合,参考SubscribeDataType,为0表示订阅所有支持的数据类型
    char security_code[ConstField::kFutureSecurityCodeLen];         // 证券代码,为空表示订阅所有代码
};
/**  @} */

/**
 * @name 订阅代码表定义
 * @{ */
struct SubCodeTableItem
{
    int32_t market;                                                     // 市场类型,参考 MarketType, 为kNone表示查询所有支持的市场(代码表目前只支持上交所、深交所与北交所)
    char security_code[ConstField::kSecurityCodeLen];                   // 证券代码,为空表示查询所有代码
};
/**  @} */

/**
 * @name 订阅ETF代码表定义
 * @{ */
struct ETFItem
{
    int32_t market;                                                     // 市场类型,参考 MarketType, 为kNone表示查询所有支持的市场(目前只支持上交所与深交所)
    char security_code[ConstField::kSecurityCodeLen];                   // 证券代码,为空表示查询所有代码
};
/**  @} */

/**
 * @name 证券品种订阅数据项定义
 * @{ */
struct SubscribeCategoryItem
{
    int32_t market;                                                 /* 市场类型,参考 MarketType(kHKEx 类型暂时不支持)
                                                                         为0表示订阅所有支持的市场. */
    uint64_t data_type;                                             // 各数据证券数据类型的集合,参考SubscribeSecuDataType,为0表示订阅所有支持的证券数据类型
    uint64_t category_type;                                         // 各数据证券品种类型的集合,参考SubscribeCategoryType,为0表示订阅所有支持的证券品种数据类型
    char security_code[ConstField::kFutureSecurityCodeLen];         // 证券代码,为空表示订阅所有代码
};
/**  @} */

/**
 * @name 订阅委托簿数据项定义
 * @{ */
struct SubscribeOrderBookItem
{
    int32_t market;                                                 // 市场类型,参考 MarketType, 委托簿支持市场范围[kSSE/kSZSE]，其余市场暂时不支持
    uint64_t flag;                                                  // 各数据类型的集合,参考 SubscribeOrderBookDataType
    char security_code[ConstField::kSecurityCodeLen];               // 证券代码,仅支持单独订阅代码,订阅代码不能为空(服务端委托簿订阅有上限设置，订阅代码总数超过上限会导致订阅失败)
};
/**  @} */

/**
 * @name 订阅行情衍生数据项定义
 * @{ */
struct SubscribeDerivedDataItem
{
    int32_t market;                                                 // 市场类型,参考 MarketType, 行情衍生数据支持市场范围[kSSE/kSZSE]，其余市场暂时不支持
    char security_code[ConstField::kSecurityCodeLen];               // 证券代码(注意:不支持代码为空)
};
/**  @} */

/**
 * @name 代码表结构定义
 * 字段类型的扩大倍数参考 华锐高速行情平台AMD转码API开发指南中的 "精度说明"
 * @{ */
struct CodeTableRecord
{
    char security_code[ConstField::kFutureSecurityCodeLen];                 // 证券代码
    uint8_t market_type;                                                    // 证券市场
    char symbol[ConstField::kSymbolLen];                                    // 简称
    char english_name[ConstField::kSecurityAbbreviationLen];                // 英文名
    char security_type[ConstField::kMaxTypesLen];                           // 证券子类别
    char    currency[ConstField::kTypesLen];                                // 币种(CNY:人民币,HKD:港币,USD:美元,AUD:澳币,CAD:加币,JPY:日圆,SGD:新加坡币,GBP:英镑,EUR:欧元)
    uint8_t variety_category;                                               // 证券类别
    int64_t pre_close_price;                                                // 昨收价(类型:价格)
    int64_t close_price;                                                    // 收盘价(已弃用,固定赋值为0,类型:价格)
    char underlying_security_id[ConstField::kSecurityCodeLen];              // 标的代码 (仅期权/权证有效)
    char    contract_type[ConstField::kMaxTypesLen];                        // 合约类别 (仅期权有效)
    int64_t exercise_price;                                                 // 行权价(仅期权有效，类型:价格)
    uint32_t expire_date;                                                   // 到期日 (仅期权有效)
    int64_t high_limited;                                                   // 涨停价(类型:价格)
    int64_t low_limited;                                                    // 跌停价(类型:价格)
    char security_status[ConstField::kCodeTableSecurityStatusMaxLen];       // 产品状态标志
    //************************************产品状态标志*************************************************************
    //1:停牌,2:除权,3:除息,4:风险警示,5:退市整理期,6:上市首日,7:公司再融资,8:恢复上市首日,9:网络投票,10:增发股份上市
    //11:合约调整,12:暂停上市后协议转让,13:实施双转单调整,14:特定债券转让,15:上市初期,16:退市整理期首日
    int64_t price_tick;                                                     // 最小价格变动单位(类型:价格)
    int64_t buy_qty_unit;                                                   // 限价买数量单位(类型:数量)
    int64_t sell_qty_unit;                                                  // 限价卖数量单位(类型:数量)
    int64_t market_buy_qty_unit;                                            // 市价买数量单位(类型:数量)
    int64_t market_sell_qty_unit;                                           // 市价卖数量单位(类型:数量)
    int64_t buy_qty_lower_limit;                                            // 限价买数量下限(类型:数量)
    int64_t buy_qty_upper_limit;                                            // 限价买数量上限(类型:数量)
    int64_t sell_qty_lower_limit;                                           // 限价卖数量下限(类型:数量)
    int64_t sell_qty_upper_limit;                                           // 限价卖数量上限(类型:数量)
    int64_t market_buy_qty_lower_limit;                                     // 市价买数量下限 (类型:数量)
    int64_t market_buy_qty_upper_limit;                                     // 市价买数量上限 (类型:数量)
    int64_t market_sell_qty_lower_limit;                                    // 市价卖数量下限 (类型:数量)
    int64_t market_sell_qty_upper_limit;                                    // 市价卖数量上限 (类型:数量)
    uint32_t list_day;                                                      // 上市日期
    int64_t par_value;                                                      // 面值(类型:价格)
    int64_t outstanding_share;                                              // 总发行量(上交所不支持,类型:数量)
    int64_t public_float_share_quantity;                                    // 流通股数(上交所不支持,类型:数量)
    int64_t contract_multiplier;                                            // 对回购标准券折算率(类型:比例)
    char regular_share[ConstField::RegularShare];                           // 对应回购标准券(仅深交所)
    int64_t interest;                                                       // 应计利息(类型:汇率)
    int64_t coupon_rate;                                                    // 票面年利率(类型:比例)
};
/**  @} */

struct CodeTableRecordList
{
    uint32_t list_nums;
    CodeTableRecord* records;
};

/**
 * @name 成分股结构定义
 * 字段类型的扩大倍数参考 华锐高速行情平台AMD转码API开发指南中的 "精度说明"
 * @{ */
struct ConstituentStockInfo
{
    char security_code[ConstField::kSecurityCodeLen];                                   //成份证券代码
    uint8_t market_type;                                                                //成份证券所属市场(仅深圳有效,参考 MarketType)
    char underlying_symbol[ConstField::kSymbolETFLen];                                  //成份证券简称
    int64_t component_share;                                                            //成份证券数量(类型:数量)
    char substitute_flag;                                                               //现金替代标志
    //************************************深圳现金替代标志***************************************************************
    //0=禁止现金替代(必须有证券),1=可以进行现金替代(先用证券,证券不足时差额部分用现金替代),2=必须用现金替代
    //************************************上海现金替代标志***************************************************************
    //ETF 公告文件 1.0 版格式
    //0 –沪市不可被替代, 1 – 沪市可以被替代, 2 – 沪市必须被替代, 3 – 深市退补现金替代, 4 – 深市必须现金替代
    //5 – 非沪深市场成份证券退补现金替代(不适用于跨沪深港 ETF 产品), 6 – 非沪深市场成份证券必须现金替代(不适用于跨沪深港 ETF 产品)
    //ETF 公告文件 2.1 版格式
    //0 –沪市不可被替代, 1 – 沪市可以被替代, 2 – 沪市必须被替代, 3 – 深市退补现金替代, 4 – 深市必须现金替代
    //5 – 非沪深市场成份证券退补现金替代(不适用于跨沪深港 ETF 产品), 6 – 非沪深市场成份证券必须现金替代(不适用于跨沪深港 ETF 产品)
    //7 – 港市退补现金替代(仅适用于跨沪深港ETF产品), 8 – 港市必须现金替代(仅适用于跨沪深港ETF产品)
    int64_t premium_ratio;                                                              //溢价比例(类型:比例)
    int64_t discount_ratio;                                                             //折价比例(类型:比例)
    int64_t creation_cash_substitute;                                                   //申购替代金额(仅深圳有效,类型:金额)
    int64_t redemption_cash_substitute;                                                 //赎回替代金额(仅深圳有效,类型:金额)
    int64_t substitution_cash_amount;                                                   //替代总金额(仅上海有效,类型:金额)
    char underlying_security_id[ConstField::KUnderlyingSecurityID];                     //成份证券所属市场ID(暂时未启用,取值为空)
    char buy_or_sell_to_open;                                                           //期权期货买入开仓或卖出开仓(暂时未启用,取值为空)
    char reserved[ConstField::KReserved];                                               //预留字段(暂时未启用,取值为空)
};
/**  @} */


/**
 * @name ETF代码表结构定义
 * 字段类型的扩大倍数参考 华锐高速行情平台AMD转码API开发指南中的 "精度说明"
 * @{ */
struct ETFCodeTableRecord
{
    char security_code[ConstField::kSecurityCodeLen] ;                                  //证券代码
    int64_t creation_redemption_unit;                                                   //每个篮子对应的ETF份数(类型:数量)
    int64_t max_cash_ratio;                                                             //最大现金替代比例(类型:比例)
    char publish;                                                                       //是否发布 IOPV,Y=是, N=否
    char creation;                                                                      //是否允许申购,Y=是, N=否(仅深圳有效)
    char redemption;                                                                    //是否允许赎回,Y=是, N=否(仅深圳有效)
    char creation_redemption_switch;                                                    //申购赎回切换(仅上海有效,0 - 不允许申购/赎回, 1 - 申购和赎回皆允许, 2 - 仅允许申购, 3 - 仅允许赎回)
    int64_t record_num;                                                                 //深市成份证券数目(类型:数量)
    int64_t total_record_num;                                                           //所有成份证券数量(类型:数量)
    int64_t estimate_cash_component;                                                    //预估现金差额(类型:金额)
    int64_t trading_day;                                                                //当前交易日(格式:YYYYMMDD)
    int64_t pre_trading_day;                                                            //前一交易日(格式:YYYYMMDD)
    int64_t cash_component;                                                             //前一日现金差额(类型:金额)
    int64_t nav_per_cu;                                                                 //前一日最小申赎单位净值(类型:价格)
    int64_t nav;                                                                        //前一日基金份额净值(类型:价格)
    uint8_t market_type;                                                                //证券所属市场(参考 MarketType)
    char symbol[ConstField::kSymbolETFLen];                                             //基金名称(仅深圳有效)
    char fund_management_company[ConstField::kManagmentETFLen];                         //基金公司名称(仅深圳有效)
    char underlying_security_id[ConstField::kSecurityCodeLen];                          //拟合指数代码(仅深圳有效)
    char underlying_security_id_source[ConstField::KUnderlyingSecurityIDSource];        //拟合指数代码源(仅深圳有效)
    int64_t dividend_per_cu;                                                            //红利金额(类型:金额)
    int64_t creation_limit;                                                             //累计申购总额限制,为 0 表示没有限制(仅深圳有效,类型:数量)
    int64_t redemption_limit;                                                           //累计赎回总额限制,为 0 表示没有限制(仅深圳有效,类型:数量)
    int64_t creation_limit_per_user;                                                    //单个账户累计申购总额限制,为 0 表示没有限制(仅深圳有效,类型:数量)
    int64_t redemption_limit_per_user;                                                  //单个账户累计赎回总额限制,为 0 表示没有限制(仅深圳有效,类型:数量)
    int64_t net_creation_limit;                                                         //净申购总额限制,为 0 表示没有限制(仅深圳有效,类型:数量)
    int64_t net_redemption_limit;                                                       //净赎回总额限制,为 0 表示没有限制(仅深圳有效,类型:数量)
    int64_t net_creation_limit_per_user;                                                //单个账户净申购总额限制,为 0 表示没有限制(仅深圳有效,类型:数量)
    int64_t net_redemption_limit_per_user;                                              //单个账户净赎回总额限制,为 0 表示没有限制(仅深圳有效,类型:数量)
    char all_cash_flag;                                                                 //是否支持全现金申赎(暂时未启用,取值为空)
    char all_cash_amount[ConstField::AllCashAmount];                                    //全现金替代的总金额(暂时未启用,取值为空)
    char all_cash_premium_rate[ConstField::AllCashAremiumRate];                         //全现金替代的申购溢价比例(暂时未启用,取值为空)
    char all_cash_discount_rate[ConstField::AllCashDiscountRate];                       //全现金替代的赎回折价比例(暂时未启用,取值为空)
    char rtgs_flag;                                                                     //是否支持RTGS(暂时未启用,取值为空)
    char reserved[ConstField::KReserved];                                               //预留字段(暂时未启用,取值为空)
    std::vector<ConstituentStockInfo> constituent_stock_infos;                          //成分股信息
};
/**  @} */

struct ETFCodeTableRecordList
{
    uint32_t etf_list_nums;
    ETFCodeTableRecord* etf_records;
};


/**
 * @name IOPV快照数据信息结构定义
 * 字段类型的扩大倍数参考 华锐高速行情平台AMD转码API开发指南中的 "精度说明"
 * @{ */
struct MDIOPVSnapshot
{
    int32_t	market_type;                                                        // 市场类型
    char	security_code[ConstField::kSecurityCodeLen];                        // 证券代码
    int64_t	orig_time;                                                          // 最新快照时间
    int64_t	last_iopv;                                                          // 最新IOPV
    int64_t	bid_iopv[ConstField::kPositionLevelLen];                            // 买档位IOPV
    int64_t	offer_iopv[ConstField::kPositionLevelLen];                          // 卖档位IOPV
};

/**
 * @name 现货快照数据信息结构定义
 * 字段类型的扩大倍数参考 华锐高速行情平台AMD转码API开发指南中的 "精度说明"
 * @{ */
struct MDSnapshot
{
    int32_t market_type;                                                        // 市场类型
    char    security_code[ConstField::kSecurityCodeLen];                        // 证券代码
    int64_t orig_time;                                                          // 时间(YYYYMMDDHHMMSSsss)
    char    trading_phase_code[ConstField::kTradingPhaseCodeLen];               // 交易阶段代码
    //************************************上海现货行情交易状态***************************************************************
    //该字段为8位字符数组,左起每位表示特定的含义,无定义则填空格。
    //第0位:‘S’表示启动(开市前)时段,‘C’表示集合竞价时段,‘T’表示连续交易时段,
    //‘E’表示闭市时段 ,‘P’表示临时停牌,
    //‘M’表示可恢复交易的熔断(盘中集合竞价),‘N’表示不可恢复交易的熔断(暂停交易至闭市)
    //‘U’表示收盘集合竞价
    //第1位:‘0’表示此产品不可正常交易,‘1’表示此产品可正常交易。
    //第2位:‘0’表示未上市,‘1’表示已上市
    //第3位:‘0’表示此产品在当前时段不接受进行新订单申报,‘1’ 表示此产品在当前时段可接受进行新订单申报。

    //************************************深圳现货行情交易状态***************************************************************
    //第 0位:‘S’= 启动(开市前)‘O’= 开盘集合竞价‘T’= 连续竞价‘B’= 休市‘C’= 收盘集合竞价‘E’= 已闭市‘H’= 临时停牌‘A’= 盘后交易‘V’=波动性中断
    //第 1位:‘0’= 正常状态 ‘1’= 全天停牌
    int64_t pre_close_price;                                                    // 昨收价(类型:价格)
    int64_t open_price;                                                         // 开盘价(类型:价格)
    int64_t high_price;                                                         // 最高价(类型:价格)
    int64_t low_price;                                                          // 最低价(类型:价格)
    int64_t last_price;                                                         // 最新价(类型:价格)
    int64_t close_price;                                                        // 收盘价(类型:价格)
    int64_t bid_price[ConstField::kPositionLevelLen];                           // 申买价(类型:价格)
    int64_t bid_volume[ConstField::kPositionLevelLen];                          // 申买量(类型:数量)
    int64_t offer_price[ConstField::kPositionLevelLen];                         // 申卖价(类型:价格)
    int64_t offer_volume[ConstField::kPositionLevelLen];                        // 申卖量(类型:数量)
    int64_t num_trades;                                                         // 成交笔数
    int64_t total_volume_trade;                                                 // 成交总量(类型:数量)
    int64_t total_value_trade;                                                  // 成交总金额(类型:金额)
    int64_t total_bid_volume;                                                   // 委托买入总量(类型:数量)
    int64_t total_offer_volume;                                                 // 委托卖出总量(类型:数量)
    int64_t weighted_avg_bid_price;                                             // 加权平均为委买价格(类型:价格)
    int64_t weighted_avg_offer_price;                                           // 加权平均为委卖价格(类型:价格)
    int64_t IOPV;                                                               // IOPV净值估产(类型:价格)
    int64_t yield_to_maturity;                                                  // 到期收益率,实际值需除以10000000
    int64_t high_limited;                                                       // 涨停价(类型:价格)
    int64_t low_limited;                                                        // 跌停价(类型:价格)
    int64_t price_earning_ratio1;                                               // 市盈率1(仅深圳有效,类型:比例)
    int64_t price_earning_ratio2;                                               // 市盈率2(仅深圳有效,类型:比例)
    int64_t change1;                                                            // 升跌1(对比昨收价,仅深圳有效,类型:比例)
    int64_t change2;                                                            // 升跌2(对比上一笔, 仅深圳有效,类型:比例)
    int32_t channel_no;                                                         // 频道代码(仅深圳有效)
    char    md_stream_id[ConstField::kMDStreamIDMaxLen];
    char    instrument_status[ConstField::kTradingPhaseCodeLen];                 // 当前品种交易状态
    int64_t pre_close_iopv;                                                      // 基金T-1日收盘时刻IOPV(仅上海有效,类型:价格)
    int64_t alt_weighted_avg_bid_price;                                          // 债券加权平均委买价格(仅上海有效,类型:价格)
    int64_t alt_weighted_avg_offer_price;                                        // 债券加权平均委卖价格(仅上海有效,类型:价格)
    int64_t etf_buy_number;                                                      // ETF 申购笔数(仅上海有效)
    int64_t etf_buy_amount;                                                      // ETF 申购数量(仅上海有效,类型:数量)
    int64_t etf_buy_money;                                                       // ETF 申购金额(仅上海有效,类型:金额)
    int64_t etf_sell_number;                                                     // ETF 赎回笔数(仅上海有效)
    int64_t etf_sell_amount;                                                     // ETF 赎回数量(仅上海有效,类型:数量)
    int64_t etf_sell_money;                                                      // ETF 赎回金额(仅上海有效,类型:金额)
    int64_t total_warrant_exec_volume;                                           // 权证执行的总数量(仅上海有效,类型:数量)
    int64_t war_lower_price;                                                     // 债券质押式回购品种加权平均价(仅上海有效,类型:价格)
    int64_t war_upper_price;                                                     // 权证涨停价格(仅上海有效,类型:价格)
    int64_t withdraw_buy_number;                                                 // 买入撤单笔数(仅上海有效)
    int64_t withdraw_buy_amount;                                                 // 买入撤单数量(仅上海有效,类型:数量)
    int64_t withdraw_buy_money;                                                  // 买入撤单金额(仅上海有效,类型:金额)
    int64_t withdraw_sell_number;                                                // 卖出撤单笔数(仅上海有效)
    int64_t withdraw_sell_amount;                                                // 卖出撤单数量(仅上海有效,类型:数量)
    int64_t withdraw_sell_money;                                                 // 卖出撤单金额(仅上海有效,类型:金额)
    int64_t total_bid_number;                                                    // 买入总笔数(仅上海有效)
    int64_t total_offer_number;                                                  // 卖出总笔数(仅上海有效)
    int32_t bid_trade_max_duration;                                              // 买入委托成交最大等待时间(仅上海有效)
    int32_t offer_trade_max_duration;                                            // 卖出委托成交最大等待时间(仅上海有效)
    int32_t num_bid_orders;                                                      // 买方委托价位数(仅上海有效)
    int32_t num_offer_orders;                                                    // 卖方委托价位数(仅上海有效)
    int64_t last_trade_time;                                                     // 最近成交时间(为YYYYMMDDHHMMSSsss 仅上海00文件,LDDS生效)
    uint8_t variety_category;                                                    // 品种类别(取值参照 ama_datatype.h VarietyCategory)
};
/**  @} */

/**
 * @name 现货指数快照数据信息结构定义
 * 字段类型的扩大倍数参考 华锐高速行情平台AMD转码API开发指南中的 "精度说明"
 * @{ */
struct MDIndexSnapshot
{
    int32_t market_type;                                                        // 市场类型
    char    security_code[ConstField::kSecurityCodeLen];                        // 证券代码
    int64_t orig_time;                                                          // 时间(YYYYMMDDHHMMSSsss)
    char    trading_phase_code[ConstField::kTradingPhaseCodeLen];               // 交易阶段代码(仅深圳有效)
    //************************************深圳指数快照交易状态***************************************************************
    //第 0位:‘S’= 启动(开市前)‘O’= 开盘集合竞价‘T’= 连续竞价‘B’= 休市‘C’= 收盘集合竞价‘E’= 已闭市‘H’= 临时停牌‘A’= 盘后交易‘V’=波动性中断
    //第 1位:‘0’= 正常状态 ‘1’= 全天停牌
    int64_t pre_close_index;                                                     // 前收盘指数(类型:价格)
    int64_t open_index;                                                          // 今开盘指数(类型:价格)
    int64_t high_index;                                                          // 最高指数(类型:价格)
    int64_t low_index;                                                           // 最低指数(类型:价格)
    int64_t last_index;                                                          // 最新指数(类型:价格)
    int64_t close_index;                                                         // 收盘指数 (类型:价格)
    int64_t total_volume_trade;                                                  // 参与计算相应指数的交易数量(类型:数量)
    int64_t total_value_trade;                                                   // 参与计算相应指数的成交总金额(类型:金额)
    int32_t channel_no;                                                          // 频道代码
    char    md_stream_id[ConstField::kMDStreamIDMaxLen];                         // 行情类别
    uint8_t variety_category;                                                    // 品种类别(取值参照 ama_datatype.h VarietyCategory)
};
/**  @} */

/**
 * @name 现货逐笔委托数据信息结构定义
 * 字段类型的扩大倍数参考 华锐高速行情平台AMD转码API开发指南中的 "精度说明"
 * @{ */
struct MDTickOrder
{
    int32_t market_type;                                                // 市场类型
    char    security_code[ConstField::kSecurityCodeLen];                // 证券代码
    int32_t channel_no;                                                 // 频道号
    int64_t appl_seq_num;                                               // 频道索引
    int64_t order_time;                                                 // 时间(YYYYMMDDHHMMSSsss)
    int64_t order_price;                                                // 委托价格(类型:价格)
    int64_t order_volume;                                               // 深圳市场:委托数量, 上海市场:剩余委托数量(类型:数量)
    uint8_t side;                                                       // 买卖方向 深圳市场:(1-买 2-卖 G-借入 F-出借) 上海市场:(B:买单, S:卖单)
    uint8_t order_type;                                                 // 订单类别 深圳市场:(1-市价 2-限价 U-本方最优) 上海市场:(A:增加委托,D:删除委托)
    char    md_stream_id[ConstField::kMDStreamIDMaxLen];                // 行情类别(仅深圳有效)
    int64_t orig_order_no;                                              // 原始订单号
    int64_t biz_index;                                                  // 业务序号
    uint8_t variety_category;                                           // 品种类别(取值参照 ama_datatype.h VarietyCategory)
};
/**  @} */

/**
 * @name 现货逐笔成交数据信息结构定义
 * 字段类型的扩大倍数参考 华锐高速行情平台AMD转码API开发指南中的 "精度说明"
 * @{ */
struct MDTickExecution
{
    int32_t market_type;                                                // 市场类型
    char    security_code[ConstField::kSecurityCodeLen];                // 证券代码
    int64_t exec_time;                                                  // 时间(YYYYMMDDHHMMSSsss)
    int32_t channel_no;                                                 // 频道号
    int64_t appl_seq_num;                                               // 频道编号
    int64_t exec_price;                                                 // 成交价格(类型:价格)
    int64_t exec_volume;                                                // 成交数量(类型:数量)
    int64_t value_trade;                                                // 成交金额(类型:金额)
    int64_t bid_appl_seq_num;                                           // 买方委托索引
    int64_t offer_appl_seq_num;                                         // 卖方委托索引
    uint8_t side;                                                       // 买卖方向(仅上海有效 B-外盘,主动买 S-内盘,主动卖 N-未知)
    uint8_t exec_type;                                                  // 成交类型(深圳: 4-撤销 F-成交, 上海: F-成交)
    char    md_stream_id[ConstField::kMDStreamIDMaxLen];                // 行情类别(仅深圳有效)
    int64_t biz_index;                                                  // 业务序号
    uint8_t variety_category;                                           // 品种类别(取值参照 ama_datatype.h VarietyCategory)
};
/**  @} */

/**
 * @name 现货委托队列数据信息结构定义
 * 字段类型的扩大倍数参考 华锐高速行情平台AMD转码API开发指南中的 "精度说明"
 * @{ */
struct MDOrderQueue
{
    int32_t market_type;                                                    // 市场类型
    char    security_code[ConstField::kSecurityCodeLen];                    // 证券代码
    int64_t order_time;                                                     // 委托时间(YYYYMMDDHHMMSSsss)
    uint8_t side;                                                           // 买卖方向 (B-买 S-卖)
    int64_t order_price;                                                    // 委托价格(类型:价格)
    int64_t order_volume;                                                   // 订单数量(类型:数量)
    int32_t num_of_orders;                                                  // 总委托笔数
    int32_t items;                                                          // 明细个数
    int64_t volume[50];                                                     // 订单明细
    int32_t channel_no;                                                     // 频道号
    char    md_stream_id[ConstField::kMDStreamIDMaxLen];                    // 行情类别(仅深圳有效)
    uint8_t variety_category;                                               // 品种类别(取值参照 ama_datatype.h VarietyCategory)
};
/**  @} */

/**@
 * @name 期权基本信息
 * @{ */
struct MDOptionBasicInfo
{
    int32_t market_type;                                                    // 市场类型
    char security_id[ConstField::kSecurityCodeLen];                         // 期权代码
    char contract_id[ConstField::kContractIDLen];                           // 合约交易代码
    char contract_symbol[ConstField::kContractSymbolLen];                   // 期权合约简称
    char underlying_security_id[ConstField::kSecurityCodeLen];              // 标的证券代码
    char underlying_symbol[ConstField::kSecurityNameLen];                   // 基础证券代码名称
    char underlying_type[ConstField::kUnderlyingTypeLen];                   // 标的证券类型 EBS - ETF, ASH - A股
    char option_type;                                                       // 欧式美式 若为欧式期权,则本字段为“E”;若为美式期权,则本字段为“A”
    char call_or_put;                                                       // 认购认沽  认购,则本字段为“C”;若为认沽,则本字段为“P”
    uint32_t contract_multiplieri_unit;                                     // 合约单位 N11 经过除权除息调整后的合约单位
    uint64_t exercise_price;                                                // 期权行权价 N11(4) 经过除权除息调整后的期权行权价,精确到0.0001元
    uint32_t start_date;                                                    // 首个交易日 期权首个交易日,YYYYMMDD
    uint32_t end_date;                                                      // 最后交易日 期权最后交易日/行权日,YYYYMMDD
    uint32_t exercise_date;                                                 // 期权行权日 期权行权日,YYYYMMDD
    uint32_t delivery_date;                                                 // 行权交割日 行权交割日,默认为行权日的下一个交易日,YYYYMMDD
    uint32_t expire_date;                                                   // 期权到期日 期权到期日,YYYYMMDD
    char update_version;                                                    // 期权合约的版本号
    uint64_t total_long_position;                                           // 当前合约未平仓数 N12 单位是 (张)
    uint64_t pre_close_price;                                               // 昨日收盘价 N11(4) 单位:元(精确到0.0001元)
    uint64_t pre_settl_price;                                               // 昨日结算价 N11(4) 如遇除权除息则为调整后的结算价(合约上市首日填写参考价),单位:元(精确到0.0001元)
    uint64_t underlying_pre_close_price;                                    // 期权标的证券除权除息调整后的前收盘价格 N11(4) 单位:元(精确到0.0001元)
    char price_limit_type;                                                  // 涨跌幅限制类型 ‘N’有涨跌幅限制类型
    uint64_t high_limited;                                                  // 当日期权涨停价格 N11(4) 单位:元(精确到0.0001元)
    uint64_t low_limited;                                                   // 当日期权跌停价格 N11(4) 单位:元(精确到0.0001元)
    uint64_t margin_unit;                                                   // 单位保证金 N16(2) 当日持有一张合约所需要的保证金数量,精确到分
    uint32_t margin_ratio_param1;                                           // 保证金计算比例参数一 N6(2) 保证金计算参数,单位:%
    uint32_t margin_ratio_param2;                                           // 保证金计算比例参数二 N6(2) 保证金计算参数,单位:%
    uint64_t round_lot;                                                     // 整手数 N12 一手对应的合约数
    uint64_t lmt_ord_min_floor;                                             // 单笔限价申报下限 N12 单笔限价申报的申报张数下限
    uint64_t lmt_ord_max_floor;                                             // 单笔限价申报上限 N12 单笔限价申报的申报张数上限
    uint64_t mkt_ord_min_floor;                                             // 单笔市价申报下限 N12 单笔市价申报的申报张数下限
    uint64_t mkt_ord_max_floor;                                             // 单笔市价申报上限 N12 单笔市价申报的申报张数上限
    uint64_t tick_size;                                                     // 最小报价单位 N11(4) 单位:元,精确到0.0001元
    char security_status_flag[ConstField::kSecurityStatusFlagLen];          // 期权合约状态信息标签 该字段为8位字符数组,左起每位表示特定的含义,无定义则填空格。
                                                                            /*
                                                                                第1位:‘0’表示可开仓,‘1’表示限制卖出开仓(不.包括备兑开仓)和买入开仓。
                                                                                第2位:‘0’表示未连续停牌,‘1’表示连续停牌。(预留,暂填0)
                                                                                第3位:‘0’表示未临近到期日,‘1’表示距离到期日不足5个交易日。
                                                                                第4位:‘0’表示近期未做调整,‘1’表示最近5个交易日内合约发生过调整。
                                                                                第5位:‘A’表示当日新挂牌的合约,‘E’表示存续的合约
                                                                            */
};
/**  @} */

/**
 * @name 期权快照数据信息结构定义
 * 字段类型的扩大倍数参考 华锐高速行情平台AMD转码API开发指南中的 "精度说明"
 * @{ */
struct MDOptionSnapshot
{
    int32_t market_type;                                                    // 市场类型
    char    security_code[ConstField::kSecurityCodeLen];                    // 期权代码
    int64_t orig_time;                                                      // 时间(YYYYMMDDHHMMSSsss)
    int64_t pre_settle_price;                                               // 昨结算价(仅上海有效,类型:价格)
    int64_t pre_close_price;                                                // 昨收盘价(类型:价格)
    int64_t open_price;                                                     // 今开盘价(类型:价格)
    int64_t auction_price;                                                  // 动态参考价 (波动性中断参考价,类型:价格)
    int64_t auction_volume;                                                 // 虚拟匹配数量(类型:数量)
    int64_t high_price;                                                     // 最高价(类型:价格)
    int64_t low_price;                                                      // 最低价(类型:价格)
    int64_t last_price;                                                     // 最新价(类型:价格)
    int64_t close_price;                                                    // 收盘价(类型:价格)
    int64_t high_limited;                                                   // 涨停价(类型:价格)
    int64_t low_limited;                                                    // 跌停价(类型:价格)
    int64_t bid_price[5];                                                   // 申买价(类型:价格)
    int64_t bid_volume[5];                                                  // 申买量(类型:数量)
    int64_t offer_price[5];                                                 // 申卖价(类型:价格)
    int64_t offer_volume[5];                                                // 申卖量(类型:数量)
    int64_t settle_price;                                                   // 今日结算价(类型:价格)
    int64_t total_long_position;                                            // 总持仓量(类型:数量)
    int64_t total_volume_trade;                                             // 总成交数(类型:数量)
    int64_t total_value_trade;                                              // 总成交额(类型:金额)
    char    trading_phase_code[ConstField::kTradingPhaseCodeLen];           // 交易阶段代码
    //************************************上海期权交易状态***************************************************************
    //该字段为8位字符数组,左起每位表示特定的含义,无定义则填空格。
    //第0位:‘S’表示启动(开市前)时段,‘C’表示集合竞价时段,‘T’表示连续交易时段,
    //‘B’表示休市时段,‘E’表示闭市时段,‘V’表示波动性中断,‘P’表示临时停牌,
    //‘U’表示收盘集合竞价 ‘M’表示可恢复交易的熔断(盘中集合竞价),‘N’表示不可恢复交易的熔断(暂停交易至闭市)
    //第1位:‘0’表示未连续停牌,‘1’表示连续停牌。(预留,暂填空格)
    //第2位:‘0’表示不限制开仓,‘1’表示限制备兑开仓,‘2’表示卖出开仓,‘3’表示限制卖出开仓、备兑开仓,‘4’表示限制买入开仓,‘5’表示限制买入开仓、备兑开仓,‘6’表示限制买入开仓、卖出开仓,‘7’表示限制买入开仓、卖出开仓、备兑开仓
    //第3位:‘0’表示此产品在当前时段不接受进行新订单申报,‘1’ 表示此产品在当前时段可接受进行新订单申报。

    //************************************深圳期权交易状态***************************************************************
    //第 0位:S= 启动(开市前)‘O’= 开盘集合竞价‘T’= 连续竞价‘B’= 休市‘C’= 收盘集合竞价‘E’= 已闭市‘H’= 临时停牌‘A’= 盘后交易‘V’=波动性中断
    //第 1位:‘0’= 正常状态 ‘1’= 全天停牌
    int32_t channel_no;                                                     // 频道代码
    char    md_stream_id[ConstField::kMDStreamIDMaxLen];                    // 行情类别
    int64_t last_trade_time;                                                // 最近成交时间(为YYYYMMDDHHMMSSsss 仅上海03文件,LDDS生效)
    int64_t ref_price;                                                      // 参考价(仅深圳有效,类型:价格)
    uint8_t variety_category;                                               // 品种类别(取值参照 ama_datatype.h VarietyCategory)
    char    contract_type;                                                  // 合约类别
    int32_t expire_date;                                                    // 到期日
    char    underlying_security_code[ConstField::kSecurityCodeLen];         // 标的代码
    int64_t exercise_price;                                                 // 行权价(类型:价格)
};
/**  @} */

/**
 * @name 港股通快照行情
 * 字段类型的扩大倍数参考 华锐高速行情平台AMD转码API开发指南中的 "精度说明"
 * @{ */
struct MDHKTSnapshot
{
    int32_t market_type;                                                    // 市场类型
    char    security_code[ConstField::kSecurityCodeLen];                    // 港股代码
    int64_t orig_time;                                                      // 时间(YYYYMMDDHHMMSSsss)
    int64_t pre_close_price;                                                // 昨收价(类型:价格)
    int64_t nominal_price;                                                  // 按盘价(类型:价格)
    int64_t high_price;                                                     // 最高价(类型:价格)
    int64_t low_price;                                                      // 最低价(类型:价格)
    int64_t last_price;                                                     // 最新价(类型:价格)
    int64_t bid_price[5];                                                   // 申买价(类型:价格)
    int64_t bid_volume[5];                                                  // 申买量(类型:数量)
    int64_t offer_price[5];                                                 // 申卖价(类型:价格)
    int64_t offer_volume[5];                                                // 申卖量(类型:数量)
    int64_t total_volume_trade;                                             // 总成交数(类型:数量)
    int64_t total_value_trade;                                              // 总成交额(类型:金额)
    char    trading_phase_code[ConstField::kTradingPhaseCodeLen];           // 交易阶段代码
    //************************************上海港股通交易状态***************************************************************
    //该字段为8位字符数组,左起每位表示特定的含义,无定义则填空格。
    //第 0位:‘0’表示正常,‘1’表示暂停交易。
    //************************************深圳港股通交易状态***************************************************************
    //第 0位:‘S’= 启动(开市前)‘O’= 开盘集合竞价‘T’= 连续竞价‘B’= 休市‘C’= 收盘集合竞价‘E’= 已闭市‘H’= 临时停牌‘A’= 盘后交易‘V’=波动性中断
    //第 1位:‘0’= 正常状态 ‘1’= 全天停牌
    int32_t channel_no;                                                      // 频道代码
    char    md_stream_id[ConstField::kMDStreamIDMaxLen];                     // 行情类别
    int64_t ref_price;                                                       // 参考价格(类型:价格)
    int64_t high_limited;                                                    // 涨停价(类型:价格)
    int64_t low_limited;                                                     // 跌停价(类型:价格)
    int64_t bid_price_limit_up;                                              // 买盘上限价(类型:价格)
    int64_t bid_price_limit_down;                                            // 买盘下限价(类型:价格)
    int64_t offer_price_limit_up;                                            // 卖盘上限价(类型:价格)
    int64_t offer_price_limit_down;                                          // 卖盘下限价(类型:价格)
    uint8_t variety_category;                                                // 品种类别(取值参照 ama_datatype.h VarietyCategory)
};
/**  @} */

/**
 * @name 港股VCM数据
 * 字段类型的扩大倍数参考 华锐高速行情平台AMD转码API开发指南中的 "精度说明"
 * @{ */
struct MDHKTVCM
{
    int32_t market_type;                                                    // 市场类型
    char    security_code[ConstField::kSecurityCodeLen];                    // 港股代码
    int64_t orig_time;                                                      // 时间(YYYYMMDDHHMMSSsss)
    int64_t start_time;                                                     // 市调机制开始时间
    int64_t end_time;                                                       // 市调机制结束时间
    int64_t ref_price;                                                      // 市调机制参考价格(类型:价格)
    int64_t low_price;                                                      // 市调机制最低价格(类型:价格)
    int64_t high_price;                                                     // 市调机制最高价格(类型:价格)
    char    md_stream_id[ConstField::kMDStreamIDMaxLen];                    // 行情类别
    uint8_t variety_category;                                                // 品种类别(取值参照 ama_datatype.h VarietyCategory)
};
/**  @} */

/**
 * @name 盘后快照定义
 * 字段类型的扩大倍数参考 华锐高速行情平台AMD转码API开发指南中的 "精度说明"
 * @{ */
struct MDAfterHourFixedPriceSnapshot
{
    int32_t market_type;                                                    // 市场类型
    char    security_code[ConstField::kSecurityCodeLen];                    // 证券代码
    int64_t orig_time;                                                      // 时间(为YYYYMMDDHHMMSSsss)
    char    trading_phase_code[ConstField::kTradingPhaseCodeLen];           // 交易阶段代码
    //************************************上海盘后快照交易状态***************************************************************
    //该字段为8位字符数组,左起每位表示特定的含义,无定义则填空格。
    //第0位:‘I’表示启动(开市前)时段, ‘A’表示集中撮合时段,‘H’表示连续交易时段,‘D’表示闭市时段,‘F’表示停牌

    //************************************深圳盘后快照交易状态***************************************************************
    //第 0位:
    //‘S’= 启动(开市前)‘O’= 开盘集合竞价‘T’= 连续竞价‘B’= 休市‘C’= 收盘集合竞价‘E’= 已闭市‘H’= 临时停牌‘A’= 盘后交易‘V’=波动性中断
    //第 1位:‘0’= 正常状态 ‘1’= 全天停牌
    int64_t close_price;                                                    // 今日收盘价(仅上海有效,类型:价格)
    int64_t bid_price;                                                      // 申买价 (类型:价格)
    int64_t bid_volume;                                                     // 申买量(类型:数量)
    int64_t offer_price;                                                    // 申卖价 (类型:价格)
    int64_t offer_volume;                                                   // 申卖量(类型:数量)
    int64_t pre_close_price;                                                // 昨收价(类型:价格)
    int64_t num_trades;                                                     // 成交笔数
    int64_t total_volume_trade;                                             // 成交总量(类型:数量)
    int64_t total_value_trade;                                              // 成交总金额(类型:金额)
    int64_t total_bid_volume;                                               // 委托买入总量(仅上海有效,类型:数量)
    int64_t total_offer_volume;                                             // 委托卖出总量(仅上海有效,类型:数量)
    int32_t channel_no;                                                     // 频道代码
    char    md_stream_id[ConstField::kMDStreamIDMaxLen];                    // 行情类别
    uint8_t variety_category;                                               // 品种类别(取值参照 ama_datatype.h VarietyCategory)
};
/**  @} */

/**
 * @name 盘后逐笔成交定义 (仅上交所)
 * 字段类型的扩大倍数参考 华锐高速行情平台AMD转码API开发指南中的 "精度说明"
 * @{ */
struct MDAfterHourFixedPriceTickExecution 
{
    int32_t market_type;
    int64_t appl_seq_num;                                                   // 消息记录号
    char    security_code[ConstField::kSecurityCodeLen];                    // 证券代码
    int64_t exec_time;                                                      // 成交时间 (YYYYMMDDHHMMSSsss)
    int64_t exec_price;                                                     // 成交价格(类型:价格)
    int64_t exec_volume;                                                    // 成交数量(类型:数量)
    int64_t value_trade;                                                    // 成交金额(类型:金额)
    int64_t bid_appl_seq_num;                                               // 买方委托索引
    int64_t offer_appl_seq_num;                                             // 卖方委托索引
    uint8_t side;                                                           // 买卖方向(B-外盘,主动买 S-内盘,主动卖 N-未知)
    uint8_t exec_type;                                                      // 成交类型
    int32_t channel_no;                                                     // 频道代码
    uint8_t variety_category;                                                // 品种类别(取值参照 ama_datatype.h VarietyCategory)
};
/**  @} */

/**
* @name MDFutureSnapshot 期货快照数据结构
* 字段类型的扩大倍数参考 华锐高速行情平台AMD转码API开发指南中的 "精度说明"
* @{ */
struct MDFutureSnapshot
{
    int32_t market_type;                                                    // 市场类型
    char security_code[ConstField::kFutureSecurityCodeLen];                 // 合约代码
    int32_t action_day;                                                     // 业务日期
    int64_t orig_time;                                                      // 交易日 YYYYMMDDHHMMSSsss(ActionDay + UpdateTime + UpdateMillisec)
    char exchange_inst_id[ConstField::kExChangeInstIDLen];                  // 合约在交易所的代码
    int64_t last_price;                                                     // 最新价(类型:价格)
    int64_t pre_settle_price;                                               // 上次结算价(类型:价格)
    int64_t pre_close_price;                                                // 昨收价(类型:价格)
    int64_t pre_open_interest;                                              // 昨持仓量(类型:数量)
    int64_t open_price;                                                     // 开盘价(类型:价格)
    int64_t high_price;                                                     // 最高价(类型:价格)
    int64_t low_price;                                                      // 最低价(类型:价格)
    int64_t total_volume_trade;                                             // 数量(类型:数量)
    int64_t total_value_trade;                                              // 总成交金额(类型:金额)
    int64_t open_interest;                                                  // 持仓量(类型:数量)
    int64_t close_price;                                                    // 今收盘(类型:价格)
    int64_t settle_price;                                                   // 本次结算价(类型:价格)
    int64_t high_limited;                                                   // 涨停板价(类型:价格)
    int64_t low_limited;                                                    // 跌停板价(类型:价格)
    int64_t pre_delta;                                                      // 昨虚实度(类型:比例)
    int64_t curr_delta;                                                     // 今虚实度(类型:比例)
    int64_t bid_price[5];                                                   // 申买价(类型:价格)
    int64_t bid_volume[5];                                                  // 申买量(类型:数量)
    int64_t offer_price[5];                                                 // 申卖价(类型:价格)
    int64_t offer_volume[5];                                                // 申卖量(类型:数量)
    int64_t average_price;                                                  // 当日均价(类型:价格)
    int32_t trading_day;                                                    // 交易日期
    uint8_t variety_category;                                               // 品种类别(取值参照 ama_datatype.h VarietyCategory)
    char exchange_inst_groupid[ConstField::kSecurityCodeLen];               // 结算组代码
    int64_t his_high_price;                                                 // 历史最高价(类型:价格)
    int64_t his_low_price;                                                  // 历史最低价(类型:价格)
    int64_t latest_volume_trade;                                            // 最新成交量(类型:数量)
    int64_t init_volume_trade;                                              // 初始持仓量(类型:数量)
    int64_t change_volume_trade;                                            // 持仓量变化(类型:数量)
    int64_t bid_imply_volume;                                               // 申买推导量(类型:数量)
    int64_t offer_imply_volume;                                             // 申卖推导量(类型:数量)
    char arbi_type;                                                         // 策略类别
    char instrument_id_1[ConstField::kFutureSecurityCodeLen];               // 第一腿合约代码
    char instrument_id_2[ConstField::kFutureSecurityCodeLen];               // 第二腿合约代码
    char instrument_name[ConstField::kFutureSecurityCodeLen];               // 合约名称
    int64_t total_bid_volume_trade;                                         // 总买入量(类型:数量)
    int64_t total_ask_volume_trade;                                         // 总卖出量(类型:数量)
};
/**  @} */


/**
* @name MDCSIIndexSnapshot 中证指数行情信息(仅上交所)
* 字段类型的扩大倍数参考 华锐高速行情平台AMD转码API开发指南中的 "精度说明"
* @{ */
struct MDCSIIndexSnapshot
{
    int32_t market_type;                                                    // 市场类型
    char    security_code[ConstField::kSecurityCodeLen];                    // 证券代码
    int64_t orig_time;                                                      // 时间(YYYYMMDDHHMMSSsss)
    int64_t last_index;                                                     // 最新指数(类型:价格)
    int64_t open_index;                                                     // 今开盘指数(类型:价格)
    int64_t high_index;                                                     // 最高指数(类型:价格)
    int64_t low_index;                                                      // 最低指数(类型:价格)
    int64_t close_index;                                                    // 收盘指数(类型:价格)
    int64_t pre_close_index;                                                // 前收盘指数(类型:价格)
    int64_t change;                                                         // 涨跌(类型:比例)
    int64_t ratio_of_change;                                                // 涨跌幅(类型:比例)
    int64_t total_volume_trade;                                             // 成交量(类型:数量)
    int64_t total_value_trade;                                              // 总成交金额 (单位为万元,类型:金额)
    int64_t exchange_rate;                                                  // 汇率(类型:汇率)
    char    currency_symbol;                                                // 币种标志(0-人民币 1-港币 2-美元 3-台币 4-日元)
    int64_t close_index2;                                                   // 当日收盘2(类型:价格)
    int64_t close_index3;                                                   // 当日收盘3(类型:价格)
    uint8_t index_market;                                                   // 指数市场
    char    md_stream_id[ConstField::kMDStreamIDMaxLen];                    // 行情类别  JLLX
    uint8_t variety_category;                                               // 品种类别(取值参照 ama_datatype.h VarietyCategory)
};
/**  @} */

/**
* @name MDIndicatorOfTradingVolumeSnapshot 成交量统计指标快照行情(仅深交所)
* 字段类型的扩大倍数参考 华锐高速行情平台AMD转码API开发指南中的 "精度说明"
* @{ */
struct MDIndicatorOfTradingVolumeSnapshot
{
    int32_t market_type;                                                    // 市场类型
    char    security_code[ConstField::kSecurityCodeLen];                    // 证券代码
    int64_t orig_time;                                                      // 时间(YYYYMMDDHHMMSSsss)
    int64_t total_volume_trade;                                             // 总成交数(类型:数量)
    int64_t total_value_trade;                                              // 总成交额(类型:金额)
    int64_t pre_close_price;                                                // 昨收价(类型:价格)
    uint32_t stock_num;                                                     // 统计量指标样本个数
    char    trading_phase_code[ConstField::kTradingPhaseCodeLen];           // 交易阶段代码
    //************************************深圳成交量统计指标快照交易状态***************************************************************
    //第 0位:‘S’= 启动(开市前)‘O’= 开盘集合竞价‘T’= 连续竞价‘B’= 休市‘C’= 收盘集合竞价‘E’= 已闭市‘H’= 临时停牌‘A’= 盘后交易‘V’=波动性中断
    //第 1位:‘0’= 正常状态 ‘1’= 全天停牌
    int32_t channel_no;                                                     // 频道代码
    char    md_stream_id[ConstField::kMDStreamIDMaxLen];                    // 行情类别
    uint8_t variety_category;                                               // 品种类别(取值参照 ama_datatype.h VarietyCategory)
};

/**
* @name MDCnIndexSnapshot 国证指数快照行情 (仅深交所)
* 字段类型的扩大倍数参考 华锐高速行情平台AMD转码API开发指南中的 "精度说明"
* @{ */
struct MDCnIndexSnapshot
{
    int32_t market_type;                                                    // 市场类型
    char    security_code[ConstField::kSecurityCodeLen];                    // 证券代码
    int64_t orig_time;                                                      // 时间(YYYYMMDDHHMMSSsss)
    char    trading_phase_code[ConstField::kTradingPhaseCodeLen];           // 交易阶段代码
    //************************************深圳成交量统计指标快照交易状态***************************************************************
    //第 0位:‘S’= 启动(开市前)‘O’= 开盘集合竞价‘T’= 连续竞价‘B’= 休市‘C’= 收盘集合竞价‘E’= 已闭市‘H’= 临时停牌‘A’= 盘后交易‘V’=波动性中断
    //第 1位:‘0’= 正常状态 ‘1’= 全天停牌
    int64_t pre_close_index;                                                // 前收盘指数(类型:价格)
    int64_t open_index;                                                     // 今开盘指数(类型:价格)
    int64_t high_index;                                                     // 最高指数(类型:价格)
    int64_t low_index;                                                      // 最低指数(类型:价格)
    int64_t last_index;                                                     // 最新指数(类型:价格)
    int64_t close_index;                                                    // 收盘指数(类型:价格)
    int64_t close_index2;                                                   // 收盘指数2(类型:价格)
    int64_t close_index3;                                                   // 收盘指数3(类型:价格)
    int64_t total_volume_trade;                                             // 参与计算相应指数的交易数量(类型:数量)
    int64_t total_value_trade;                                              // 参与计算相应指数的成交总金额(类型:金额)
    int32_t channel_no;                                                     // 频道代码
    char    md_stream_id[ConstField::kMDStreamIDMaxLen];                    // 行情类别
    uint8_t variety_category;                                               // 品种类别(取值参照 ama_datatype.h VarietyCategory)
};
/**  @} */

/**
* @name MDRefinancingTickOrder 转融通证券出借逐笔委托(仅深交所)
* 字段类型的扩大倍数参考 华锐高速行情平台AMD转码API开发指南中的 "精度说明"
* @{ */
struct MDRefinancingTickOrder
{
    int32_t market_type;                                                    // 市场类型
    char    security_code[ConstField::kSecurityCodeLen];                    // 证券代码
    int32_t channel_no;                                                     // 频道编号
    int64_t appl_seq_num;                                                   // 消息记录号
    int64_t order_time;                                                     // 委托时间 (YYYYMMDDHHMMSSsss)
    int64_t order_price;                                                    // 委托价格(类型:价格)
    int64_t order_volume;                                                   // 委托数量(类型:数量)
    uint8_t side;                                                           // 买卖方向 (1-买 2-卖 G-借入 F-出借)
    uint16_t expiration_days;                                               // 期限
    uint8_t expiration_type;                                                // 期限类型(1-固定期限)
    char    md_stream_id[ConstField::kMDStreamIDMaxLen];                    // 行情类别
    uint8_t variety_category;                                               // 品种类别(取值参照 ama_datatype.h VarietyCategory)
};
/**  @} */

/**
* @name MDRefinancingTickExecution 转融通证券出借逐笔成交(仅深交所)
* 字段类型的扩大倍数参考 华锐高速行情平台AMD转码API开发指南中的 "精度说明"
* @{ */
struct MDRefinancingTickExecution
{
    int32_t market_type;                                                    // 市场类型
    char    security_code[ConstField::kSecurityCodeLen];                    // 证券代码
    int64_t exec_time;                                                      // 成交时间 YYYYMMDDHHMMSSsss
    int32_t channel_no;                                                     // 频道编号
    int64_t appl_seq_num;                                                   // 消息记录号
    int64_t exec_price;                                                     // 成交价格(类型:价格)
    int64_t exec_volume;                                                    // 成交数量(类型:数量)
    int64_t value_trade;                                                    // 成交金额(类型:金额)
    int64_t bid_appl_seq_num;                                               // 买方委托索引
    int64_t offer_appl_seq_num;                                             // 卖方委托索引
    uint8_t side;                                                           // 买卖方向
    uint8_t exec_type;                                                      // 成交类型(仅深圳有效 4-撤销 F-成交)
    char    md_stream_id[ConstField::kMDStreamIDMaxLen];                    // 行情类别
    uint8_t variety_category;                                               // 品种类别(取值参照 ama_datatype.h VarietyCategory)
};
/**  @} */

/**
* @name MDNegotiableTickOrder 协议交易逐笔委托(仅深交所)
* 字段类型的扩大倍数参考 华锐高速行情平台AMD转码API开发指南中的 "精度说明"
* @{ */
struct MDNegotiableTickOrder
{
    int32_t market_type;
    char    security_code[ConstField::kSecurityCodeLen];                    // 证券代码
    int32_t channel_no;                                                     // 频道编号
    int64_t appl_seq_num;                                                   // 消息记录号
    int64_t order_time;                                                     // 委托时间 (YYYYMMDDHHMMSSsss)
    int64_t order_price;                                                    // 委托价格(类型:价格)
    int64_t order_volume;                                                   // 委托数量(类型:数量)
    uint8_t side;                                                           // 买卖方向 (1-买 2-卖 G-借入 F-出借)
    char    confirm_id[ConstField::kConfirmIdLen];                          // 定价行情约定号; 为空表示是意向行情, 否则为定价行情
    char    contactor[ConstField::kContactorLen];                           // 联系人
    char    contact_info[ConstField::kContactInfoLen];                      // 联系方式
    char    md_stream_id[ConstField::kMDStreamIDMaxLen];                    // 行情类别
    uint8_t variety_category;                                               // 品种类别(取值参照 ama_datatype.h VarietyCategory)
};
/**  @} */

/**
* @name MDNegotiableTickExecution 协议交易逐笔成交(仅深交所)
* 字段类型的扩大倍数参考 华锐高速行情平台AMD转码API开发指南中的 "精度说明"
* @{ */
struct MDNegotiableTickExecution
{
    int32_t market_type;                                                    // 市场类型
    char    security_code[ConstField::kSecurityCodeLen];                    // 证券代码
    int64_t exec_time;                                                      // 成交时间 YYYYMMDDHHMMSSsss
    int32_t channel_no;                                                     // 频道编号
    int64_t appl_seq_num;                                                   // 消息记录号
    int64_t exec_price;                                                     // 成交价格(类型:价格)
    int64_t exec_volume;                                                    // 成交数量(类型:数量)
    int64_t value_trade;                                                    // 成交金额(类型:金额)
    int64_t bid_appl_seq_num;                                               // 买方委托索引
    int64_t offer_appl_seq_num;                                             // 卖方委托索引
    uint8_t side;                                                           // 买卖方向
    uint8_t exec_type;                                                      // 成交类型(仅深圳有效 4-撤销 F-成交)
    char    md_stream_id[ConstField::kMDStreamIDMaxLen];                    // 行情类别
    uint8_t variety_category;                                               // 品种类别(取值参照 ama_datatype.h VarietyCategory)
};
/**  @} */

/**
* @name MDHKTRealtimeLimit 港股通实时额度
* 字段类型的扩大倍数参考 华锐高速行情平台AMD转码API开发指南中的 "精度说明"
* @{ */
struct MDHKTRealtimeLimit
{
    int32_t market_type;                                                    // 市场类型
    int64_t orig_time;                                                      // 时间(YYYYMMDDHHMMSSsss)
    int64_t threshold_amount;                                               // 每日初始额度(类型:金额)
    int64_t pos_amt;                                                        // 日中剩余额度(类型:金额)
    char    amount_status;                                                  // 额度状态 (1-额度用完或其他原因全市场禁止买入 2-额度可用)
    int32_t channel_no;                                                     // 频道代码
    char    md_stream_id[ConstField::kMDStreamIDMaxLen];                    // 行情类别(仅上海有效)
    uint8_t variety_category;                                               // 品种类别(取值参照 ama_datatype.h VarietyCategory)
    char    mkt_status[ConstField::kTradingStatusLen];                      // 上交所港股通市场状态(上交所独有,来源于上交所文件行情)
};
/**  @} */

/**
* @name MDHKTProductStatus 港股通可接收订单并转发的产品状态数据
* @{ */
struct MDHKTProductStatus
{
    int32_t market_type;                                                    // 市场类型
    char    security_code[ConstField::kSecurityCodeLen];                    // 证券代码
    int64_t orig_time;                                                      // 时间(YYYYMMDDHHMMSSsss)
    char    trading_status1[ConstField::kTradingStatusLen];                 // 证券交易状态(整手订单)
    //************************************港股通整手订单************************************
    //该字段为8位字符数组,左起每位表示特定的含义,无定义则填空格。
    //第1位:‘0’表示限制买入,‘1’表示正常无此限制。
    //第2位:‘0’表示限制卖出,‘1’表示正常无此限制。
    char    trading_status2[ConstField::kTradingStatusLen];                 // 证券交易状态(零股订单)
    //************************************港股通零股订单************************************
    //该字段为8位字符数组,左起每位表示特定的含义,无定义则填空格。
    //第1位:‘0’表示限制买入,‘1’表示正常无此限制。
    //第2位:‘0’表示限制卖出,‘1’表示正常无此限制。
    int32_t channel_no;                                                     // 频道代码
    char    md_stream_id[ConstField::kMDStreamIDMaxLen];                    // 行情类别(仅上海有效)
    uint8_t variety_category;                                               // 品种类别(取值参照 ama_datatype.h VarietyCategory)
};
/**  @} */

/**
* @name MDHKMarketStatus 港股市场状态
* @{
* */
struct MDHKMarketStatus
{
    int32_t market_type;                                                    // 市场类型
    int64_t orig_time;                                                      // 时间(YYYYMMDDHHMMSSsss)
    char    trading_session_sub_id[ConstField::kTradingStatusLen];          // 市场状态
    uint8_t variety_category;                                               // 品种类别(取值参照 ama_datatype.h VarietyCategory)
};

/**
 * @name MDNEEQSnapshot 股转系统证券行情
 * 字段类型的扩大倍数参考 华锐高速行情平台AMD转码API开发指南中的 "精度说明"
 * @{ */
struct MDNEEQSnapshot
{
    int32_t market_type;                                                    // 市场类型
    char    security_code[ConstField::kSecurityCodeLen];                    // 证券代码
    int64_t orig_time;                                                      // 时间 CCYYMMDD + HHMMSS * 1000
    int64_t pre_close_price;                                                // 昨收价(类型:价格)
    int64_t open_price;                                                     // 开盘价(类型:价格)
    int64_t last_price;                                                     // 最新价(类型:价格)
    int64_t total_volume_trade;                                             // 成交总量(类型:数量)
    int64_t total_value_trade;                                              // 成交总金额(类型:金额)
    int64_t num_trades;                                                     // 成交笔数
    int64_t high_price;                                                     // 最高价(类型:价格)
    int64_t low_price;                                                      // 最低价(类型:价格)
    int64_t price_earning_ratio1;                                           // 市盈率1(类型:比例)
    int64_t price_earning_ratio2;                                           // 市盈率2(类型:比例)
    int64_t change1;                                                        // 升跌1(对比昨收价,类型:比例)
    int64_t change2;                                                        // 升跌2(对比上一笔,类型:比例)
    int64_t open_interest;                                                  // 合约持仓量(类型:数量)
    int64_t bid_price[5];                                                   // 申买价(类型:价格)
    int64_t bid_volume[5];                                                  // 申买量(类型:数量)
    int64_t offer_price[5];                                                 // 申卖价(类型:价格)
    int64_t offer_volume[5];                                                // 申卖量(类型:数量)
    int64_t index_factor;                                                   // 指数因子(类型:比例)
    char    trading_phase_code[ConstField::kTradingPhaseCodeLen];           // 交易阶段代码
    //************************************北交所证券行情状态***************************************************************
    //个位数存放收市行情标志(0:非收市行情;1:收市行情;2:盘后行情)
    //十位数存放正式行情与测试行情标志(0:正式行情;1:测试行情)
    uint8_t variety_category;                                               // 品种类别(取值参照 ama_datatype.h VarietyCategory)
};
/**  @} */

/**
* @name MDNEEQSecurityInfo 股转证券信息
* 字段类型的扩大倍数参考 华锐高速行情平台AMD转码API开发指南中的 "精度说明"
* @{ */
struct MDNEEQSecurityInfo
{
    int32_t market_type;                                                    // 市场类型
    char    security_code[ConstField::kSecurityCodeLen];                    // 证券代码
    int64_t orig_time;                                                      // 时间 CCYYMMDD + HHMMSSss * 10
    char    security_abbreviation[ConstField::kSecurityNameLen];            // 证券简称
    char    underlying_security[ConstField::kSecurityCodeLen];              // 基础证券
    char    ISIN[ConstField::kSecurityCodeLen];                             // ISIN编码
    int32_t trading_unit;                                                   // 交易单位
    char    industry_type[ConstField::kTypesLen];                           // 行业种类
    char    currency[ConstField::kTypesLen];                                // 货币种类 (00-人民币,02-美元)
    int64_t par_value;                                                      // 每股面值(类型:价格)
    int64_t general_capital;                                                // 总股本
    int64_t unrestricted_capital;                                           // 非限售股本
    int64_t last_year_earning;                                              // 上年每股收益(类型:价格)
    int64_t cur_year_earning;                                               // 本年每股收益(类型:价格)
    int64_t brokerage_rate;                                                 // 经手费率(类型:比例)
    int64_t stamp_duty_rate;                                                // 印花税率(类型:比例)
    int64_t transfer_fee_rate;                                              // 过户费率(类型:比例)
    char    listing_date[ConstField::kDateLen];                             // 挂牌日期
    char    value_date[ConstField::kDateLen];                               // 起息日
    char    expiring_date[ConstField::kDateLen];                            // 到期日
    int64_t every_limited;                                                  // 每笔限量(类型:数量)
    int32_t buy_amount_unit;                                                // 买数量单位(类型:数量)
    int32_t sell_amount_unit;                                               // 卖数量单位(类型:数量)
    int64_t mini_dec_amount;                                                // 最小申报数量(类型:数量)
    int32_t price_level;                                                    // 价格档位(类型:价格)
    int64_t first_trade_limit;                                              // 首笔交易限价参数(类型:价格)
    int64_t follow_trade_limit;                                             // 后续交易限价参数(类型:价格)
    uint8_t limit_param_nature;                                             // 限价参数性质
    int64_t high_limited;                                                   // 涨停价(类型:价格)
    int64_t low_limited;                                                    // 跌停价(类型:价格)
    int64_t block_trade_ceiling;                                            // 大宗交易价格上限(类型:价格)
    int64_t block_trade_floor;                                              // 大宗交易价格下限(类型:价格)
    char    component_mark;                                                 // 成分股标志
    int32_t conver_ratio;                                                   // 折合比例(类型:比例)
    char    trade_status;                                                   // 交易状态
    char    security_level;                                                 // 证券级别
    char    trade_type;                                                     // 交易类型
    int64_t market_maker_num;                                               // 做市商数量(类型:数量)
    char    suspen_sign;                                                    // 停牌标志
    char    ex_sign;                                                        // 除权除息标志
    char    net_vote_sign;                                                  // 网络投票标志
    char    other_buss_sign[ConstField::kTypesLen];                         // 其他业务标志
    char    record_update_time[ConstField::kTimeLen];                       // 记录更新时间
    uint8_t variety_category;                                               // 品种类别(取值参照 ama_datatype.h VarietyCategory)
};
/**  @} */

/**
 * @name MDNEEQNonPublicTransDeclaredInfo 股转非公开转让申报信息库
 * 字段类型的扩大倍数参考 华锐高速行情平台AMD转码API开发指南中的 "精度说明"
 * @{ */
struct MDNEEQNonPublicTransDeclaredInfo
{
    int32_t market_type;                                                    // 市场类型
    char    security_code[ConstField::kSecurityCodeLen];                    // 证券代码
    int64_t orig_time;                                                      // 时间 CCYYMMDD + HHMMSS * 1000
    char    transaction_unit[ConstField::kTypesLen];                        // 交易单元
    char    security_category[ConstField::kTypesLen];                       // 证券类别
    char    declare_category[ConstField::kTypesLen];                        // 申报类别
    int64_t declare_volume;                                                 // 申报数量(类型:数量)
    int64_t declare_price;                                                  // 申报价格(类型:价格)
    int32_t deal_agreement_num;                                             // 成交约定号
    char    declare_time[ConstField::kTimeLen];                             // 申报时间
    char    record_status;                                                  // 记录状态
    char    backup_sign;                                                    // 备用标志
    uint8_t variety_category;                                               // 品种类别(取值参照 ama_datatype.h VarietyCategory)
};
/**  @} */

/**
 * @name MDNEEQHierarchicalInfo 股转分层信息库
 * @{ */
struct MDNEEQHierarchicalInfo
{
    int32_t market_type;                                                    // 市场类型
    char    security_code[ConstField::kSecurityCodeLen];                    // 证券代码
    char    trade_date[ConstField::kDateLen];                               // 交易日期 CCYYMMDD
    char    security_abbreviation[ConstField::kSecurityAbbreviationLen];    // 证券简称
    char    layered_sign;                                                   // 分层标志
    char    layered_effective_date[ConstField::kDateLen];                   // 分层生效日期 CCYYMMDD
    char    backup_sign;                                                    // 备用标志
    uint8_t variety_category;                                               // 品种类别(取值参照 ama_datatype.h VarietyCategory)
};
/**  @} */

struct MDOrderBookItem
{
    int64_t price;                                                          // 价格(类型:价格)
    int64_t volume;                                                         // 总数量(类型:数量)
    int64_t order_queue_size;                                               // 委托队列大小
    int64_t order_queue[50];                                                // 委托队列数量, 最多揭示50笔
};

/**
* @name MDOrderBook 委托簿
* @{ */
struct MDOrderBook
{
    int32_t channel_no;                                                     // 频道号
    int32_t market_type;                                                    // 市场类型
    char    security_code[ConstField::kSecurityCodeLen];                    // 证券代码
    int64_t last_tick_time;                                                 // 最新逐笔生成时间
    int64_t last_snapshot_time;                                             // 最新快照生成时间
    int64_t last_tick_seq;                                                  // 最新逐笔序列号
    std::vector<MDOrderBookItem> bid_order_book;                            // 买委托簿
    std::vector<MDOrderBookItem> offer_order_book;                          // 卖委托簿
    int64_t total_num_trades;                                               // 基于委托簿演算的成交总笔数
    int64_t total_volume_trade;                                             // 基于委托簿演算的成交总量(类型:数量)
    int64_t total_value_trade;                                              // 基于委托簿演算的成交总金额(类型:金额)
    int64_t last_price;                                                     // 基于委托簿演算的最新价(类型:价格)
};
/**  @} */

/**
* @name MDOrderBookSnapshot 委托簿快照数据
* @{ */
struct MDOrderBookSnapshot
{
    uint8_t	market_type;                                        // 市场类型，参考公共数据字典市场类型(MarketType)
    uint8_t	variety_category;                                   // 品种类别，参照公共数据字典品种类型(VarietyCategory)
    char	security_code[ConstField::kSecurityCodeLen];        // 证券代码
    int64_t	last_tick_seq;                                      // 构建快照的最新逐笔记录号
    int32_t	channel_no;                                         // 构建快照的逐笔原始频道编号
    int64_t	orig_time;                                          // 基于委托簿演算的行情快照时间(最新逐笔记录号对应的逐笔时间)(为YYYYMMDDHHMMSSsss)
    int64_t	last_price;                                         // 基于委托簿演算的最新价(类型:价格)
    int64_t	total_num_trades;                                   // 基于委托簿演算的成交总笔数
    int64_t	total_volume_trade;                                 // 基于委托簿演算的成交总量(类型:数量)
    int64_t	total_value_trade;                                  // 基于委托簿演算的成交总金额(类型:金额)
    int64_t	total_bid_volume;                                   // 基于委托簿演算的委托买入总量(类型:数量)
    int64_t	total_offer_volume;                                 // 基于委托簿演算的委托卖出总量(类型:数量)
    int64_t	num_bid_orders;                                     // 基于委托簿演算的买方委托价位数
    int64_t	num_offer_orders;                                   // 基于委托簿演算的卖方委托价位数
    int64_t	bid_price[ConstField::kPositionLevelLen];           // 申买价(类型:价格)
    int64_t	bid_volume[ConstField::kPositionLevelLen];          // 申买量(类型:数量)
    int64_t	offer_price[ConstField::kPositionLevelLen];         // 申卖价(类型:价格)
    int64_t	offer_volume[ConstField::kPositionLevelLen];        // 申卖量(类型:数量)
};
/**  @} */


/**
 * @name MDNEEQNegotiableDeclaredInfo 北交所协议转让申报信息库
 * @{ */
struct MDNEEQNegotiableDeclaredInfo
{
    int32_t market_type;                                                    // 市场类型
    char    security_code[ConstField::kSecurityCodeLen];                    // 证券代码
    int64_t orig_time;                                                      // 时间 CCYYMMDD + HHMMSS * 1000
    char    transaction_unit[ConstField::kTypesLen];                        // 交易单元
    char    md_stream_id[ConstField::kMDStreamIDLen];                       // 业务类别
    int64_t declare_volume;                                                 // 申报数量(类型:数量)
    int64_t declare_price;                                                  // 申报价格(类型:价格)
    int32_t deal_agreement_num;                                             // 成交约定号
    char    declare_time[ConstField::kTimeLen];                             // 申报时间
    char    record_status;                                                  // 记录状态
    char    backup_sign;                                                    // 备用标志
    uint8_t variety_category;                                               // 品种类别(取值参照 ama_datatype.h VarietyCategory)
};
/**  @} */

/**
 * @name MDNEEQMarketMakerDeclaredInfo 北交所做市业务申报信息库
 * @{ */
struct MDNEEQMarketMakerDeclaredInfo
{
    int32_t market_type;                                                    // 市场类型
    char    security_code[ConstField::kSecurityCodeLen];                    // 证券代码
    int64_t orig_time;                                                      // 时间 CCYYMMDD + HHMMSS * 1000
    char    md_stream_id[ConstField::kMDStreamIDLen];                       // 业务类别
    int64_t declare_volume;                                                 // 申报数量(类型:数量)
    int64_t declare_price;                                                  // 申报价格(类型:价格)
    char    data_type;                                                      // 数据类型
    char    declare_time[ConstField::kTimeLen];                             // 申报时间
    int64_t backup_field;                                                   // 备用字段(预留)
    uint8_t variety_category;                                               // 品种类别(取值参照 ama_datatype.h VarietyCategory)
};
/**  @} */

/**
 * @name MDNEEQNonPublicTransferDealInfo 北交所非公开转让成交信息库
 * @{ */
struct MDNEEQNonPublicTransferDealInfo
{
    int32_t market_type;                                                    // 市场类型
    int64_t serial_num;                                                     // 序号
    char    security_code[ConstField::kSecurityCodeLen];                    // 证券代码
    int64_t orig_time;                                                      // 时间 CCYYMMDD + HHMMSS * 1000
    char    security_abbreviation[ConstField::kSecurityAbbreviationLen];    // 证券简称
    char    security_category[ConstField::kTypesLen];                       // 证券类别
    char    bid_transaction_unit[ConstField::kTypesLen];                    // 买入交易单元(预留)
    char    bid_transaction_unit_name[ConstField::kUnitName];               // 买入营业部名称/交易单元名称
    char    offer_transaction_unit[ConstField::kTypesLen];                  // 卖出交易单元(预留)
    char    offer_transaction_unit_name[ConstField::kUnitName];             // 卖出营业部名称/交易单元名称
    int64_t deal_volume;                                                    // 成交数量(类型:数量)
    int64_t deal_price;                                                     // 成交价格(类型:价格)
    char    deal_time[ConstField::kTimeLen];                                // 成交时间
    char    backup_sign;                                                    // 备用标志
    uint8_t variety_category;                                               // 品种类别(取值参照 ama_datatype.h VarietyCategory)
};
/**  @} */

/**
* @name SubTradingPhase 新债快照细分交易阶段数据结构
* @{
* */
struct SubTradingPhase
{
    char    sub_trading_phase_code[8];                                      // 交易方式所处的交易阶段代码
    uint8_t trading_type;                                                   // 交易方式
};

/**
* @name MDBondSnapshot 债券行情快照数据结构
* 字段类型的扩大倍数参考 华锐高速行情平台AMD转码API开发指南中的 "精度说明"
* @{
* */
struct MDBondSnapshot
{
    int32_t market_type;                                                    // 市场类型
    char    security_code[ConstField::kSecurityCodeLen];                    // 证券代码
    int64_t orig_time;                                                      // 时间(为YYYYMMDDHHMMSSsss)
    char    trading_phase_code[ConstField::kTradingPhaseCodeLen];           // 交易阶段代码
    //************************************上海债券快照交易状态***************************************************************
    //该字段为8位字符数组,左起每位表示特定的含义,无定义则填空格。
    //第0位:‘S’表示启动(开市前)时段,‘C’表示开盘集合竞价时段,‘T’表示连续交易时段,‘E’表示闭市时段,‘P’表示产品停牌
    //第1位:‘0’表示此产品不可正常交易,‘1’表示此产品可正常交易。
    //第2位:‘0’表示未上市,‘1’表示已上市
    //第3位:‘0’表示此产品在当前时段不接受进行新订单申报,‘1’ 表示此产品在当前时段可接受进行新订单申报。

    //************************************深圳债券快照交易状态***************************************************************
    //第 0位:‘S’= 启动(开市前)‘O’= 开盘集合竞价‘T’= 连续竞价‘B’= 休市‘C’= 收盘集合竞价‘E’= 已闭市‘H’= 临时停牌‘A’= 盘后交易‘V’=波动性中断
    //第 1位:‘0’= 正常状态 ‘1’= 全天停牌
    int64_t pre_close_price;                                                // 昨收价(类型:价格) 
    int64_t open_price;                                                     // 开盘价(类型:价格)
    int64_t high_price;                                                     // 最高价(类型:价格)
    int64_t low_price;                                                      // 最低价(类型:价格)
    int64_t last_price;                                                     // 最新价(类型:价格)
    int64_t close_price;                                                    // 收盘价(类型:价格)
    int64_t bid_price[ConstField::kPositionLevelLen];                       // 申买价(类型:价格)
    int64_t bid_volume[ConstField::kPositionLevelLen];                      // 申买量(类型:数量)
    int64_t offer_price[ConstField::kPositionLevelLen];                     // 申卖价(类型:价格)
    int64_t offer_volume[ConstField::kPositionLevelLen];                    // 申卖量(类型:数量)
    int64_t num_trades;                                                     // 成交笔数
    int64_t total_volume_trade;                                             // 成交总量(类型:数量)
    int64_t total_value_trade;                                              // 成交总金额(类型:金额)
    int64_t total_bid_volume;                                               // 委托买入总量(类型:数量)
    int64_t total_offer_volume;                                             // 委托卖出总量(类型:数量)
    int64_t weighted_avg_bid_price;                                         // 加权平均为委买价格(类型:价格)
    int64_t weighted_avg_offer_price;                                       // 加权平均为委卖价格(类型:价格)
    int64_t high_limited;                                                   // 涨停价(类型:价格)
    int64_t low_limited;                                                    // 跌停价(类型:价格)
    int64_t change1;                                                        // 升跌1(对比昨收价)(仅深圳有效,类型:比例)
    int64_t change2;                                                        // 升跌2(对比上一笔)(仅深圳有效,类型:比例)
    int64_t weighted_avg_bp;                                                // 加权平均利率涨跌BP(债券质押式回购)(仅深圳有效,类型:比例)
    int64_t pre_close_weighted_avg_price;                                   // 昨收盘加权平均价(债券质押式回购)(仅深圳有效,类型:比例)
    int64_t auct_last_price;                                                // 匹配成交最近价(仅深圳有效,类型:价格)
    uint8_t last_price_trading_type;                                        // 最近价成交方式(仅深圳有效)
    int32_t channel_no;                                                     // 频道代码(仅深圳有效)
    char    md_stream_id[ConstField::kMDStreamIDLen];                       // 行情类别
    char    instrument_status[ConstField::kTradingPhaseCodeLen];            // 当前品种交易状态(仅上海有效)
    //************************************交易状态***************************************************************
    //ADD---产品未上市          START---启动
    //OCALL---开市集合竞价      TRADE---连续自动撮合
    //SUSP---停牌              CLOSE---闭市
    //ENDTR---交易结束
    int64_t withdraw_buy_number;                                            // 买入撤单笔数(仅上海有效)
    int64_t withdraw_buy_amount;                                            // 买入撤单数量(仅上海有效,类型:数量)
    int64_t withdraw_buy_money;                                             // 买入撤单金额(仅上海有效,类型:金额)
    int64_t withdraw_sell_number;                                           // 卖出撤单笔数(仅上海有效)
    int64_t withdraw_sell_amount;                                           // 卖出撤单数量(仅上海有效,类型:数量)
    int64_t withdraw_sell_money;                                            // 卖出撤单金额(仅上海有效,类型:金额)
    int64_t total_bid_number;                                               // 买入总笔数(仅上海有效)
    int64_t total_offer_number;                                             // 卖出总笔数(仅上海有效)
    int32_t bid_trade_max_duration;                                         // 买入委托成交最大等待时间(仅上海有效)
    int32_t offer_trade_max_duration;                                       // 卖出委托成交最大等待时间(仅上海有效)
    int32_t num_bid_orders;                                                 // 买方委托价位数(仅上海有效)
    int32_t num_offer_orders;                                               // 卖方委托价位数(仅上海有效)
    int64_t last_trade_time;                                                // 最近成交时间(为YYYYMMDDHHMMSSsss 仅上海有效)
    int64_t weighted_avg_price;                                             // 加权平均价(类型:价格)
    uint32_t no_sub_trading_phase_code;                                     // 细分交易阶段个数(仅深圳有效)
    SubTradingPhase sub_trading_phase[ConstField::kSubTradingPhaseLen];     // 细分交易阶段信息(仅深圳有效)
    int64_t auct_volume_trade;                                              // 匹配成交成交量(仅深圳有效,类型:数量)
    int64_t auct_value_trade;                                               // 匹配成家成交金额(仅深圳有效,类型:金额)
    uint8_t variety_category;                                               // 品种类别(取值参照 ama_datatype.h VarietyCategory)
};
/**  @} */


/**
* @name MDBondTickOrder 债券逐笔委托数据结构
* 字段类型的扩大倍数参考 华锐高速行情平台AMD转码API开发指南中的 "精度说明"
* @{
* */
struct MDBondTickOrder
{
    int32_t market_type;
    int64_t appl_seq_num;                                                   // 消息记录号
    int32_t channel_no;                                                     // 频道编号
    char    security_code[ConstField::kSecurityCodeLen];                    // 证券代码
    int64_t order_time;                                                     // 委托时间 (YYYYMMDDHHMMSSsss)
    int64_t order_price;                                                    // 委托价格(类型:价格)
    int64_t order_volume;                                                   // 委托数量(类型:数量)
    uint8_t side;                                                           // 买卖方向 
    //************************************买卖方向***************************************************************
    //深圳市场: (1-买 2-卖 G-借入 F-出借) 
    //上海市场: 当order_type = 'S'时,此字段无意义,取值默认为'\0'. 当order_type = 'A' | order_type = 'D'时,意义如下:
    //(B:买单,S:卖单)
    uint8_t order_type;                                                     // 订单类别 深圳市场:(1-市价 2-限价 U-本方最优) 上海市场:(A:增加委托,D:删除委托,S:产品状态订单)
    char    md_stream_id[ConstField::kMDStreamIDLen];                       // 行情类别(仅深圳有效)
    char    product_status[ConstField::kTradingPhaseCodeLen];               // 产品状态(仅上海有效)
    //************************************产品状态***************************************************************
    //当order_type = 'A' | order_type = 'D'时,此字段无意义,取值默认为"\0". 当order_type = 'S'时,意义如下:
    //ADD---产品未上市          START---启动
    //OCALL---开市集合竞价      TRADE---连续自动撮合
    //SUSP---停牌              CLOSE---闭市
    //ENDTR---交易结束
    int64_t orig_order_no;                                                  //原始订单号
    uint8_t variety_category;                                               // 品种类别(取值参照 ama_datatype.h VarietyCategory)
};
/**  @} */

/**
* @name MDBondQuotedTickOrder 债券业务报价及大额逐笔委托数据结构(仅深交所)
* 字段类型的扩大倍数参考 华锐高速行情平台AMD转码API开发指南中的 "精度说明"
* @{
* */
struct MDBondQuotedTickOrder
{
    int32_t market_type;
    int64_t appl_seq_num;                                                   // 消息记录号
    int32_t channel_no;                                                     // 频道编号
    char    security_code[ConstField::kSecurityCodeLen];                    // 证券代码
    int64_t order_time;                                                     // 委托时间 (YYYYMMDDHHMMSSsss)
    int64_t order_price;                                                    // 委托价格(类型:价格)
    int64_t order_volume;                                                   // 委托数量(类型:数量)
    uint8_t side;                                                           // 买卖方向,(1-买 2-卖 G-借入 F-出借)
    char    md_stream_id[ConstField::kMDStreamIDLen];                       // 行情类别
    char    quote_id[ConstField::kQuoteIDLen];                              // 报价消息编号
    char    member_id[ConstField::kMemberIDLen];                            // 交易商代码
    char    investor_type[ConstField::kInvestorTypeLen];                    // 交易主体类型(01-自营 02-资管 03-机构经济 04-个人经济)
    char    investor_id[ConstField::kInvestorIDLen];                        // 交易主体代码
    char    investor_name[ConstField::kInvestorNameLen];                    // 客户名称
    char    trader_code[ConstField::kTraderCodeLen];                        // 交易员代码
    uint8_t settl_period;                                                   // 结算周期
    uint16_t settl_type;                                                    // 结算方式(103-多边净额 104-逐笔全额)
    uint8_t variety_category;                                               // 品种类别(取值参照 ama_datatype.h VarietyCategory)
    char     secondary_order_id[ConstField::kSecondaryOrderIDLen];          //竞买场次编号 (仅债券竞买逐笔数据有效)
    uint32_t bid_trans_type;                                                //竞买业务类别( 1=竞买预约申报 2=竞买发起申报 3=竞买应价申报 仅债券竞买逐笔数据有效)
    uint32_t bid_execinst_type;                                             //竞买成交方式( 1=单一主体中标 2=多主体单一价格中标 3=多主体多重价格中标 仅债券竞买逐笔数据有效)
    int64_t  lowlimit_price;                                                //价格下限 (仅债券竞买逐笔数据有效,类型:价格)
    int64_t  highlimit_price;                                               //价格上限( 0=无价格上限 仅债券竞买逐笔数据有效,类型:价格)
    int64_t  min_qty;                                                       //最低成交数量(仅债券竞买逐笔数据有效,类型:数量)
    uint32_t trade_date;                                                    //交易日期(YYYYMMDD 仅债券竞买逐笔数据有效)
};
/**  @} */

/**
* @name MDBondTickExecution 债券逐笔成交数据结构
* 字段类型的扩大倍数参考 华锐高速行情平台AMD转码API开发指南中的 "精度说明"
* @{
* */
struct MDBondTickExecution
{
    int32_t market_type;
    int64_t appl_seq_num;                                                   // 消息记录号
    int32_t channel_no;                                                     // 频道编号
    char    security_code[ConstField::kSecurityCodeLen];                    // 证券代码
    int64_t exec_time;                                                      // 成交时间 YYYYMMDDHHMMSSsss
    int64_t exec_price;                                                     // 成交价格(类型:价格)
    int64_t exec_volume;                                                    // 成交数量(类型:数量)
    int64_t value_trade;                                                    // 成交金额(仅上海有效,类型:金额)
    int64_t bid_appl_seq_num;                                               // 买方委托索引
    int64_t offer_appl_seq_num;                                             // 卖方委托索引
    uint8_t side;                                                           // 买卖方向(仅上海有效 B: 买单, S卖单)
    uint8_t exec_type;                                                      // 成交类型(深圳: 4-撤销 F-成交,上海: F-成交)
    char    md_stream_id[ConstField::kMDStreamIDLen];                       // 行情类别(仅深圳有效)
    uint8_t variety_category;                                               // 品种类别(取值参照 ama_datatype.h VarietyCategory)
};

/**
* @name MDBondQuotedTickExecution 债券业务报价及大额逐笔成交数据结构 (仅深交所)
* 字段类型的扩大倍数参考 华锐高速行情平台AMD转码API开发指南中的 "精度说明"
* @{
* */
struct MDBondQuotedTickExecution
{
    int32_t market_type;
    int64_t appl_seq_num;                                                   // 消息记录号
    int32_t channel_no;                                                     // 频道编号
    char    security_code[ConstField::kSecurityCodeLen];                    // 证券代码
    int64_t exec_time;                                                      // 成交时间 YYYYMMDDHHMMSSsss
    int64_t exec_price;                                                     // 成交价格(类型:价格)
    int64_t exec_volume;                                                    // 成交数量(类型:数量)
    int64_t bid_appl_seq_num;                                               // 买方委托索引
    int64_t offer_appl_seq_num;                                             // 卖方委托索引
    uint8_t exec_type;                                                      // 成交类型(4-撤销 F-成交)
    char    md_stream_id[ConstField::kMDStreamIDLen];                       // 行情类别
    uint8_t settl_period;                                                   // 结算周期
    uint16_t settl_type;                                                    // 结算方式(103-多边净额 104-逐笔全额)
    uint8_t variety_category;                                               // 品种类别(取值参照 ama_datatype.h VarietyCategory)
    char     secondary_order_id[ConstField::kSecondaryOrderIDLen];          // 竞买场次编号(仅债券竞买逐笔数据有效)
    uint32_t bid_execinst_type;                                             // 竞买成交方式( 1=单一主体中标 2=多主体单一价格中标 3=多主体多重价格中标 仅债券竞买逐笔数据有效)
    int64_t  margin_price;                                                  // 达成成交的边际价格( 竞买成交方式为多主体单一价格中标或多主体多重价格中标时用于揭示竞买成交的边际价格 仅债券竞买逐笔数据有效)
};
/**  @} */

/**
* @name MDFundExpertSnapshot 基金通快照数据结构
* 字段类型的扩大倍数参考 华锐高速行情平台AMD转码API开发指南中的 "精度说明"
* @{
* */
struct MDFundExpertSnapshot
{
    uint8_t market_type;                                                    // 市场类型
    uint8_t variety_category;                                               // 品种类别
    int32_t channel_no;                                                     // 频道代码(仅深交所)
    char md_stream_id[ConstField::kMDStreamIDMaxLen];                       // 行情类别
    char security_code[ConstField::kSecurityCodeLen];                       // 证券代码
    int64_t orig_time;                                                      // 行情发送时间(为YYYYMMDDHHMMSSsss)
    char trading_phase_code[ConstField::kTradingPhaseCodeLen];              // 交易阶段代码
    //************************************上海产品实时阶段及标志***************************************************************
    //该字段为8位字符串，左起每位表示特定的含义，无定义则填空格。
    //第 0 位：‘S’表示启动（开市前）时段，‘C’表示开盘集合竞价时段，‘T’表示连续交易时段，‘E’表示闭市时段，‘P’表示产品停牌，
    //‘M’表示可恢复交易的熔断时段（盘中集合竞价），‘N’表示不可恢复交易的熔断时段（暂停交易至闭市），‘U’表示收盘集合竞价时段。
    //第 1 位：‘0’表示此产品不可正常交易，‘1’表示此产品可正常交易，无意义填空格。
    //第 2 位：‘0’表示未上市，‘1’表示已上市。
    //第 3 位：‘0’表示此产品在当前时段不接受订单申报，‘1’ 表示此产品在当前时段可接受订单申报。无意义填空格。
    //************************************深圳交易阶段代码***************************************************************
    //第 0位：‘S’= 启动（开市前）‘T’= 连续竞价‘B’= 休市‘E’= 已闭市‘H’= 临时停牌
    //第 1位：‘0’= 正常状态 ‘1’= 全天停牌                 
    char instrument_status[ConstField::kTradingPhaseCodeLen];               // 当前品种交易状态(仅上交所)
    //************************************上海市场状态***************************************************************
    //0 = 全日收市 1 = 输入买卖盘(开盘集合竞价时段) 2 = 对盘(开盘集合竞价时段) 3 = 持续交易 4 = 对盘(收盘集合竞价时段)
    //5 = 输入买卖盘(收盘集合竞价时段) 7 = 暂停 100 = 未开市 101 = 不可取消(开盘集合竞价时段) 102 = ExchangeIntervention
    //103 = 收市 104 = 取消买卖盘 105 = 参考价定价(收盘集合竞价时段) 106 = 不可取消(收盘集合竞价时段) 107 = 随机收市(收盘集合竞价时段)
    //108=随机对盘（开盘集合竞价时段）
    int64_t pre_close_price;                                                // 昨收价(类型:价格)
    int64_t open_price;                                                     // SH-开盘价(仅上交所,类型:价格)
    int64_t high_price;                                                     // 最高价(类型:价格)
    int64_t low_price;                                                      // 最低价(类型:价格)
    int64_t last_price;                                                     // 最新价(类型:价格)
    int64_t per_price;                                                      // SH-加权平均价(仅上交所,类型:价格)
    int64_t close_price;                                                    // SH-今收盘价(仅上交所,类型:价格)
    int64_t bid_price[ConstField::kPositionLevelLen];                       // 申买价(做市商,类型:价格)
    int64_t bid_volume[ConstField::kPositionLevelLen];                      // 申买量(做市商, 类型:数量)
    int64_t offer_price[ConstField::kPositionLevelLen];                     // 申卖价(投资者,类型:价格)
    int64_t offer_volume[ConstField::kPositionLevelLen];                    // 申卖量(投资者, 类型:数量)
    int64_t num_trades;                                                     // 成交笔数
    int64_t total_volume_trade;                                             // 成交总量(类型:数量)
    int64_t total_value_trade;                                              // 成交总金额(类型:金额)
    int64_t market_maker_bid_volume;                                        // 委托买入总量(仅深交所, 做市商买入总量, 类型:数量)
    int64_t sh_investor_offer_volume;                                       // 委托卖出总量(仅深交所, 投资者卖出总量, 类型:数量)
    int64_t market_maker_bid_price;                                         // 加权平均委买价格(仅深交所, 做市商加权平均价,类型:价格)
    int64_t sh_investor_offer_price;                                        // 加权平均委卖价格(仅深交所, 投资者加权平均价,类型:价格)
    int64_t high_limited;                                                   // 涨停价(仅深交所, 类型:价格)
    int64_t low_limited;                                                    // 跌停价(仅深交所, 类型:价格)
    int64_t investor_bid_price;                                             // 投资者买入均价(仅上交所,类型:价格)
    int64_t investor_bid_volume;                                            // 投资者买入总量(仅上交所,类型:数量)
    int64_t investor_best_bid_price;                                        // 投资者买入最优价(仅上交所,类型:价格)
    int64_t investor_bid_volume_best_price;                                 // 投资者买入最优价数量(仅上交所,类型:数量)
    int64_t investor_offer_price;                                           // 投资者卖出均价(仅上交所,类型:价格)
    int64_t investor_offer_volume;                                          // 投资者卖出总量(仅上交所,类型:数量)
    int64_t investor_best_offer_price;                                      // 投资者卖出最优价(仅上交所,类型:价格)
    int64_t investor_offer_volume_best_price;                               // 投资者卖出最优价数量(仅上交所,类型:数量)
    int64_t iopv;                                                           // 基金 IOPV(仅上交所 MDStreamID=MD601 时存在该字段)
    int64_t last_trade_time;                                                // 最近成交时间(仅上交所)
};


/**  @} */

}; // end of ama
}; // end of amd

#pragma pack(pop)
#endif
