#include <iostream>
#include <string>

#include "ama_struct.h"
#include "amdQuote.h"
#include "amdQuoteImp.h"
#include "amdSpiImp.h"
#include "ddbplugin/Plugin.h"

extern BackgroundResourceMap<AmdQuote> AMD_HANDLE_MAP;
extern bool STOP_TEST;
static const string AMD_SINGLETON_NAME = "amdQuote_map_key";

inline long long convertToAMDTime(long long dolphinTime, long long days = -1) {
    if (days == -1) {
        dolphinTime /= 1000000;
        days = dolphinTime / (24 * 60 * 60 * 1000);
    }
    dolphinTime = dolphinTime % (24 * 60 * 60 * 1000);
    int milliSecond = dolphinTime % 1000;
    dolphinTime = dolphinTime / 1000;
    int second = dolphinTime % 60;
    dolphinTime = dolphinTime / 60;
    int minute = dolphinTime % 60;
    dolphinTime = dolphinTime / 60;
    int hour = dolphinTime % 60;
    int year, month, day;
    Util::parseDate(days, year, month, day);
    return (((((((long long)year * 100) + month) * 100 + day) * 100 + hour) * 100 + minute) * 100 + second) * 1000 +
           milliSecond;
}

unordered_map<int, string> timeColumnNameForType = {
    {AMD_SNAPSHOT, "origTime"},       {AMD_FUND_SNAPSHOT, "origTime"},  {AMD_BOND_SNAPSHOT, "origTime"},
    {AMD_EXECUTION, "execTime"},      {AMD_FUND_EXECUTION, "execTime"}, {AMD_BOND_EXECUTION, "execTime"},
    {AMD_ORDER, "orderTime"},         {AMD_FUND_ORDER, "orderTime"},    {AMD_BOND_ORDER, "orderTime"},
    {AMD_INDEX, "origTime"},          {AMD_ORDER_QUEUE, "orderTime"},   {AMD_OPTION_SNAPSHOT, "orig_time"},
    {AMD_FUTURE_SNAPSHOT, "orig_time"},
#ifndef AMD_3_9_6
    {AMD_IOPV_SNAPSHOT, "orig_time"},
#endif
};

unordered_map<string, AMDDataType> NAME_TYPE = {
    {"snapshot", AMD_SNAPSHOT},   {"fundSnapshot", AMD_FUND_SNAPSHOT},   {"bondSnapshot", AMD_BOND_SNAPSHOT},
    {"execution", AMD_EXECUTION}, {"fundExecution", AMD_FUND_EXECUTION}, {"bondExecution", AMD_BOND_EXECUTION},
    {"order", AMD_ORDER},         {"fundOrder", AMD_FUND_ORDER},         {"bondOrder", AMD_BOND_ORDER},
    {"index", AMD_INDEX},         {"orderQueue", AMD_ORDER_QUEUE},       {"option", AMD_OPTION_SNAPSHOT},
    {"future", AMD_FUTURE_SNAPSHOT},
#ifndef AMD_3_9_6
    {"IOPV", AMD_IOPV_SNAPSHOT}
#endif
};

#include <cstdlib>
#include <time.h>

int getChannel(int market) {
    int randValue = rand();
    if (market == 101) {
        return 1 + randValue % 6;
    } else {
        return 2011 + randValue % 4;
    }
}

int getChannel(int market, AMDDataType type) {
    int randValue = rand();
    if (market == 101) {
        if (type == AMD_BOND_EXECUTION || type == AMD_BOND_ORDER) {
            return 801;
        }
        return 1 + randValue % 6;
    } else {
        if (type == AMD_EXECUTION || type == AMD_ORDER) {
            return 2011 + randValue % 4;
        } else if (type == AMD_FUND_EXECUTION || type == AMD_FUND_ORDER) {
            return 2021 + randValue % 4;
        } else {
            return 2031 + randValue % 4;
        }
    }
}

void fill(volatile int64_t *dest, int size, long long start) {
    for (int i = 0; i < size; ++i) {
        dest[i] = start + i;
    }
}

extern "C" ConstantSP testAmdData(Heap *heap, vector<ConstantSP> &arguments) {
    string usage{"syntax should be one of: \"amdQuote::testData(dataType, lines)\", \"amdQuote::testData(dataType, data, lines)\": "};
    if (STOP_TEST) {
        return new Void();
    }
    if (arguments[0]->getType() != DT_STRING || arguments[0]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, usage + "dataType must be a string");
    if (arguments[1]->getForm() == DF_SCALAR && arguments[1]->getCategory() == INTEGRAL) {
        long long cnt = arguments[1]->getLong();
        if (arguments.size() > 2) {
            if (arguments[2]->getCategory() != INTEGRAL || arguments[2]->getForm() != DF_SCALAR)
                throw IllegalArgumentException(__FUNCTION__, usage + "lines must be a integral");
            cnt = arguments[2]->getInt();
        }
        string dataType = arguments[0]->getString();

        AMDDataType amdDataType;
        string timeColumnName;
        if (NAME_TYPE.count(dataType) != 0) {
            amdDataType = NAME_TYPE[dataType];
        } else {
            throw IllegalArgumentException(__FUNCTION__, usage + "invalid dataType: " + dataType);
        }
        if (timeColumnNameForType.count(amdDataType) != 0) {
            timeColumnName = timeColumnNameForType[amdDataType];
        } else {
            throw IllegalArgumentException(__FUNCTION__, usage + "invalid dataType: " + std::to_string(amdDataType));
        }

        SmartPointer<AmdQuote> instance = AMD_HANDLE_MAP.safeGetByName(AMD_SINGLETON_NAME);
        AMDSpiImp *amdSpi = instance->getAMDSpi();
        if (amdDataType == AMD_SNAPSHOT || amdDataType == AMD_FUND_SNAPSHOT) {
            for (int i = 0; i < cnt; ++i) {
                long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
                if (STOP_TEST) {
                    return new Void();
                }
                int bias = 0;
                amd::ama::MDSnapshot *snapshot = new amd::ama::MDSnapshot[1];
                snapshot[0].market_type = 101 + i % 2;
                snapshot[0].channel_no = getChannel(snapshot[0].market_type);
                snapshot[0].variety_category = amdDataType == AMD_SNAPSHOT ? 1 : 2;
                snapshot[0].orig_time = convertToAMDTime(time);
                string code = std::to_string(600000 + i);
                std::strcpy(snapshot[0].security_code, code.c_str());
                snapshot[0].trading_phase_code[0] = 'S';
                snapshot[0].trading_phase_code[1] = '0';
                snapshot[0].trading_phase_code[2] = '\0';
                snapshot[0].pre_close_price = i + bias++;
                snapshot[0].open_price = i + bias++;
                snapshot[0].high_price = i + bias++;
                snapshot[0].low_price = i + bias++;
                snapshot[0].last_price = i + bias++;
                snapshot[0].close_price = i + bias++;
                fill(snapshot[0].bid_price, amd::ama::ConstField::kPositionLevelLen, i);
                fill(snapshot[0].bid_volume, amd::ama::ConstField::kPositionLevelLen, i);
                fill(snapshot[0].offer_price, amd::ama::ConstField::kPositionLevelLen, i);
                fill(snapshot[0].offer_volume, amd::ama::ConstField::kPositionLevelLen, i);
                snapshot[0].num_trades = i + bias++;
                snapshot[0].total_volume_trade = i + bias++;
                snapshot[0].total_value_trade = i + bias++;
                snapshot[0].total_bid_volume = i + bias++;
                snapshot[0].total_offer_volume = i + bias++;
                snapshot[0].weighted_avg_bid_price = i + bias++;
                snapshot[0].weighted_avg_offer_price = i + bias++;
                snapshot[0].IOPV = i + bias++;
                snapshot[0].yield_to_maturity = i + bias++;
                snapshot[0].high_limited = i + bias++;
                snapshot[0].low_limited = i + bias++;
                snapshot[0].price_earning_ratio1 = i + bias++;
                snapshot[0].price_earning_ratio2 = i + bias++;
                snapshot[0].change1 = i + bias++;
                snapshot[0].change2 = i + bias++;
                snapshot[0].md_stream_id[0] = '\0';
                snapshot[0].instrument_status[0] = 'B';
                snapshot[0].pre_close_iopv = i + bias++;
                snapshot[0].alt_weighted_avg_bid_price = i + bias++;
                snapshot[0].alt_weighted_avg_offer_price = i + bias++;
                snapshot[0].etf_buy_number = i + bias++;
                snapshot[0].etf_buy_amount = i + bias++;
                snapshot[0].etf_buy_money = i + bias++;
                snapshot[0].etf_sell_number = i + bias++;
                snapshot[0].etf_sell_amount = i + bias++;
                snapshot[0].etf_sell_money = i + bias++;
                snapshot[0].total_warrant_exec_volume = i + bias++;
                snapshot[0].war_lower_price = i + bias++;
                snapshot[0].war_upper_price = i + bias++;
                snapshot[0].withdraw_buy_number = i + bias++;
                snapshot[0].withdraw_buy_amount = i + bias++;
                snapshot[0].withdraw_buy_money = i + bias++;
                snapshot[0].withdraw_sell_number = i + bias++;
                snapshot[0].withdraw_sell_amount = i + bias++;
                snapshot[0].withdraw_sell_money = i + bias++;
                snapshot[0].total_bid_number = i + bias++;
                snapshot[0].total_offer_number = i + bias++;
                snapshot[0].bid_trade_max_duration = i + bias++;
                snapshot[0].offer_trade_max_duration = i + bias++;
                snapshot[0].num_bid_orders = i + bias++;
                snapshot[0].num_offer_orders = i + bias++;
                snapshot[0].last_trade_time = i + bias++;

                amdSpi->pushSnapshotData(snapshot, 1, time);
                delete[] snapshot;
            }
        } else if (amdDataType == AMD_BOND_SNAPSHOT) {
            for (int i = 0; i < cnt; ++i) {
                long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
                if (STOP_TEST) {
                    return new Void();
                }
                amd::ama::MDBondSnapshot *snapshot = new amd::ama::MDBondSnapshot[1];
                int bias = 0;
                snapshot[0].market_type = 101 + i % 2;
                snapshot[0].channel_no = getChannel(snapshot[0].market_type);
                snapshot[0].variety_category = 3;
                snapshot[0].orig_time = convertToAMDTime(time);
                string code = std::to_string(600000 + i);
                std::strcpy(snapshot[0].security_code, code.c_str());
                snapshot[0].trading_phase_code[0] = 'S';
                snapshot[0].trading_phase_code[1] = '0';
                snapshot[0].trading_phase_code[2] = '\0';
                snapshot[0].pre_close_price = i + bias++;
                snapshot[0].open_price = i + bias++;
                snapshot[0].high_price = i + bias++;
                snapshot[0].low_price = i + bias++;
                snapshot[0].last_price = i + bias++;
                snapshot[0].close_price = i + bias++;
                fill(snapshot[0].bid_price, amd::ama::ConstField::kPositionLevelLen, i);
                fill(snapshot[0].bid_volume, amd::ama::ConstField::kPositionLevelLen, i);
                fill(snapshot[0].offer_price, amd::ama::ConstField::kPositionLevelLen, i);
                fill(snapshot[0].offer_volume, amd::ama::ConstField::kPositionLevelLen, i);
                snapshot[0].num_trades = i + bias++;
                snapshot[0].total_volume_trade = i + bias++;
                snapshot[0].total_value_trade = i + bias++;
                snapshot[0].total_bid_volume = i + bias++;
                snapshot[0].total_offer_volume = i + bias++;
                snapshot[0].weighted_avg_bid_price = i + bias++;
                snapshot[0].weighted_avg_offer_price = i + bias++;
                snapshot[0].high_limited = i + bias++;
                snapshot[0].low_limited = i + bias++;
                snapshot[0].change1 = i + bias++;
                snapshot[0].change2 = i + bias++;
                snapshot[0].md_stream_id[0] = '\0';
                snapshot[0].instrument_status[0] = 'C';
                snapshot[0].withdraw_buy_number = i + bias++;
                snapshot[0].withdraw_buy_amount = i + bias++;
                snapshot[0].withdraw_buy_money = i + bias++;
                snapshot[0].withdraw_sell_number = i + bias++;
                snapshot[0].withdraw_sell_amount = i + bias++;
                snapshot[0].withdraw_sell_money = i + bias++;
                snapshot[0].total_bid_number = i + bias++;
                snapshot[0].total_offer_number = i + bias++;
                snapshot[0].bid_trade_max_duration = i + bias++;
                snapshot[0].offer_trade_max_duration = i + bias++;
                snapshot[0].num_bid_orders = i + bias++;
                snapshot[0].num_offer_orders = i + bias++;
                snapshot[0].last_trade_time = i + bias++;

                amdSpi->pushBondSnapshotData(snapshot, 1, time);
                delete[] snapshot;
            }
        } else if (amdDataType == AMD_EXECUTION || amdDataType == AMD_FUND_EXECUTION) {
            for (int i = 0; i < cnt; ++i) {
                long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
                if (STOP_TEST) {
                    return new Void();
                }
                amd::ama::MDTickExecution *snapshot = new amd::ama::MDTickExecution[1];
                int bias = 0;
                string code = std::to_string(600000 + i);
                std::strcpy(snapshot[0].security_code, code.c_str());
                snapshot[0].market_type = 101 + i % 2;
                snapshot[0].channel_no = getChannel(snapshot[0].market_type, amdDataType);
                snapshot[0].variety_category = amdDataType == AMD_EXECUTION ? 1 : 2;
                snapshot[0].exec_time = convertToAMDTime(time);
                snapshot[0].appl_seq_num = i + bias++;
                snapshot[0].exec_price = i + bias++;
                snapshot[0].exec_volume = i + bias++;
                snapshot[0].value_trade = i + bias++;
                snapshot[0].bid_appl_seq_num = i + bias++;
                snapshot[0].offer_appl_seq_num = i + bias++;
                snapshot[0].side = 'B';
                snapshot[0].exec_type = 'F';
                snapshot[0].md_stream_id[0] = '\0';
                snapshot[0].biz_index = i + bias++;
                amdSpi->pushExecutionData(snapshot, 1, time);
                delete[] snapshot;
            }

        } else if (amdDataType == AMD_BOND_EXECUTION) {
            for (int i = 0; i < cnt; ++i) {
                long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
                if (STOP_TEST) {
                    return new Void();
                }
                amd::ama::MDBondTickExecution *snapshot = new amd::ama::MDBondTickExecution[1];
                int bias = 0;
                string code = std::to_string(600000 + i);
                std::strcpy(snapshot[0].security_code, code.c_str());
                snapshot[0].market_type = 101 + i % 2;
                snapshot[0].channel_no = getChannel(snapshot[0].market_type, amdDataType);
                snapshot[0].variety_category = 3;
                snapshot[0].exec_time = convertToAMDTime(time);
                snapshot[0].appl_seq_num = i + bias++;
                snapshot[0].exec_price = i + bias++;
                snapshot[0].exec_volume = i + bias++;
                snapshot[0].value_trade = i + bias++;
                snapshot[0].bid_appl_seq_num = i + bias++;
                snapshot[0].offer_appl_seq_num = i + bias++;
                snapshot[0].side = 'B';
                snapshot[0].exec_type = 'F';
                snapshot[0].md_stream_id[0] = '\0';
                amdSpi->pushBondExecutionData(snapshot, 1, time);
                delete[] snapshot;
            }
        } else if (amdDataType == AMD_ORDER || amdDataType == AMD_FUND_ORDER) {
            for (int i = 0; i < cnt; ++i) {
                long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
                if (STOP_TEST) {
                    return new Void();
                }
                amd::ama::MDTickOrder *snapshot = new amd::ama::MDTickOrder[1];
                int bias = 0;
                string code = std::to_string(600000 + i);
                std::strcpy(snapshot[0].security_code, code.c_str());
                snapshot[0].market_type = 101 + i % 2;
                snapshot[0].channel_no = getChannel(snapshot[0].market_type, amdDataType);
                snapshot[0].variety_category = amdDataType == AMD_ORDER ? 1 : 2;
                snapshot[0].order_time = convertToAMDTime(time);
                snapshot[0].appl_seq_num = i + bias++;
                snapshot[0].order_price = i + bias++;
                snapshot[0].order_volume = i + bias++;
                snapshot[0].side = 'B';
                snapshot[0].order_type = 'S';
                snapshot[0].md_stream_id[0] = '\0';
                snapshot[0].orig_order_no = i + bias++;
                snapshot[0].biz_index = i + bias++;
                amdSpi->pushOrderData(snapshot, 1, time);
                delete[] snapshot;
            }

        } else if (amdDataType == AMD_BOND_ORDER) {
            for (int i = 0; i < cnt; ++i) {
                long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
                if (STOP_TEST) {
                    return new Void();
                }
                amd::ama::MDBondTickOrder *snapshot = new amd::ama::MDBondTickOrder[1];
                int bias = 0;
                string code = std::to_string(600000 + i);
                std::strcpy(snapshot[0].security_code, code.c_str());
                snapshot[0].market_type = 101 + i % 2;
                snapshot[0].channel_no = getChannel(snapshot[0].market_type, amdDataType);
                snapshot[0].variety_category = 3;
                snapshot[0].order_time = convertToAMDTime(time);
                snapshot[0].appl_seq_num = i + bias++;
                snapshot[0].order_price = i + bias++;
                snapshot[0].order_volume = i + bias++;
                snapshot[0].side = 'B';
                snapshot[0].order_type = 'S';
                snapshot[0].md_stream_id[0] = '\0';
                snapshot[0].orig_order_no = i + bias++;
                amdSpi->pushBondOrderData(snapshot, 1, time);
                delete[] snapshot;
            }
        } else if (amdDataType == AMD_INDEX) {
            for (int i = 0; i < cnt; ++i) {
                long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
                if (STOP_TEST) {
                    return new Void();
                }
                amd::ama::MDIndexSnapshot *snapshot = new amd::ama::MDIndexSnapshot[1];
                int bias = 0;
                string code = std::to_string(600000 + i);
                std::strcpy(snapshot[0].security_code, code.c_str());
                snapshot[0].market_type = 101 + i % 2;
                snapshot[0].channel_no = getChannel(snapshot[0].market_type);
                snapshot[0].variety_category = 4;
                snapshot[0].trading_phase_code[0] = 'S';
                snapshot[0].trading_phase_code[1] = '0';
                snapshot[0].trading_phase_code[2] = '\0';
                snapshot[0].orig_time = convertToAMDTime(time);
                snapshot[0].pre_close_index = i + bias++;
                snapshot[0].open_index = i + bias++;
                snapshot[0].high_index = i + bias++;
                snapshot[0].low_index = i + bias++;
                snapshot[0].last_index = i + bias++;
                snapshot[0].close_index = i + bias++;
                snapshot[0].total_volume_trade = i + bias++;
                snapshot[0].total_value_trade = i + bias++;
                snapshot[0].md_stream_id[0] = '\0';
                amdSpi->pushIndexData(snapshot, 1, time);
                delete[] snapshot;
            }
        } else if (amdDataType == AMD_ORDER_QUEUE) {
            for (int i = 0; i < cnt; ++i) {
                long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
                if (STOP_TEST) {
                    return new Void();
                }
                amd::ama::MDOrderQueue *snapshot = new amd::ama::MDOrderQueue[1];
                int bias = 0;
                string code = std::to_string(600000 + i);
                std::strcpy(snapshot[0].security_code, code.c_str());
                snapshot[0].market_type = 101 + i % 2;
                snapshot[0].channel_no = getChannel(snapshot[0].market_type);
                snapshot[0].order_time = convertToAMDTime(time);
                snapshot[0].variety_category = 5;
                snapshot[0].side = 'B';
                snapshot[0].order_price = i + bias++;
                snapshot[0].order_volume = i + bias++;
                snapshot[0].num_of_orders = i + bias++;
                snapshot[0].items = i + bias++;
                fill(snapshot[0].volume, 50, i);
                snapshot[0].md_stream_id[0] = '\0';
                amdSpi->pushOrderQueueData(snapshot, 1, time);
                delete[] snapshot;
            }
        } else if (amdDataType == AMD_OPTION_SNAPSHOT) {
            for (int i = 0; i < cnt; ++i) {
                long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
                if (STOP_TEST) {
                    return new Void();
                }
                amd::ama::MDOptionSnapshot *snapshot = new amd::ama::MDOptionSnapshot[1];
                int bias = 0;
                string code = std::to_string(600000 + i);
                std::strcpy(snapshot[0].security_code, code.c_str());
                snapshot[0].market_type = 101 + i % 2;
                snapshot[0].channel_no = getChannel(snapshot[0].market_type);
                snapshot[0].variety_category = 6;
                snapshot[0].orig_time = convertToAMDTime(time);
                snapshot[0].pre_settle_price = i + bias++;
                snapshot[0].pre_close_price = i + bias++;
                snapshot[0].open_price = i + bias++;
                snapshot[0].auction_price = i + bias++;
                snapshot[0].auction_volume = i + bias++;
                snapshot[0].high_price = i + bias++;
                snapshot[0].low_price = i + bias++;
                snapshot[0].last_price = i + bias++;
                snapshot[0].close_price = i + bias++;
                snapshot[0].high_limited = i + bias++;
                snapshot[0].low_limited = i + bias++;
                fill(snapshot[0].bid_price, 5, i);
                fill(snapshot[0].bid_volume, 5, i);
                fill(snapshot[0].offer_price, 5, i);
                fill(snapshot[0].offer_volume, 5, i);
                snapshot[0].settle_price = i + bias++;
                snapshot[0].total_long_position = i + bias++;
                snapshot[0].total_volume_trade = i + bias++;
                snapshot[0].total_value_trade = i + bias++;
                snapshot[0].trading_phase_code[0] = 'S';
                snapshot[0].trading_phase_code[1] = '0';
                snapshot[0].trading_phase_code[2] = '\0';
                snapshot[0].md_stream_id[0] = '\0';
                snapshot[0].last_trade_time = i + bias++;
                snapshot[0].ref_price = i + bias++;
                snapshot[0].contract_type = i + bias++;
                snapshot[0].expire_date = i + bias++;
                snapshot[0].underlying_security_code[0] = 'O';
                snapshot[0].exercise_price = i + bias++;
                amdSpi->pushOptionData(snapshot, 1, time);
                delete[] snapshot;
            }
        } else if (amdDataType == AMD_FUTURE_SNAPSHOT) {
            for (int i = 0; i < cnt; ++i) {
                long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
                if (STOP_TEST) {
                    return new Void();
                }
                amd::ama::MDFutureSnapshot *snapshot = new amd::ama::MDFutureSnapshot[1];
                int bias = 0;
                string code = std::to_string(600000 + i);
                std::strcpy(snapshot[0].security_code, code.c_str());
                snapshot[0].market_type = (i % 3 == 0 ? 4: 101 + i % 2);
                snapshot[0].variety_category = 6;
                snapshot[0].action_day = convertToAMDTime(time) / 100000;
                snapshot[0].orig_time = convertToAMDTime(time);
                snapshot[0].exchange_inst_id[0] = '\0';
                snapshot[0].last_price = i + bias++;
                snapshot[0].pre_settle_price = i + bias++;
                snapshot[0].pre_close_price = i + bias++;
                snapshot[0].open_interest = i + bias++;
                snapshot[0].open_price = i + bias++;
                snapshot[0].high_price = i + bias++;
                snapshot[0].low_price = i + bias++;

                // TODO(slshen) fill all mock field
                snapshot[0].last_price = i + bias++;
                snapshot[0].close_price = i + bias++;
                snapshot[0].high_limited = i + bias++;
                snapshot[0].low_limited = i + bias++;
                fill(snapshot[0].bid_price, 5, i);
                fill(snapshot[0].bid_volume, 5, i);
                fill(snapshot[0].offer_price, 5, i);
                fill(snapshot[0].offer_volume, 5, i);
                snapshot[0].settle_price = i + bias++;
                snapshot[0].total_volume_trade = i + bias++;
                snapshot[0].total_value_trade = i + bias++;
                amdSpi->pushFutureData(snapshot, 1, time);
                delete[] snapshot;
            }
        }
#ifndef AMD_3_9_6
        else if (amdDataType == AMD_IOPV_SNAPSHOT) {
            for (int i = 0; i < cnt; ++i) {
                long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
                if (STOP_TEST) {
                    return new Void();
                }
                amd::ama::MDIOPVSnapshot *snapshot = new amd::ama::MDIOPVSnapshot[1];
                int bias = 0;
                string code = std::to_string(600000 + i);
                std::strcpy(snapshot[0].security_code, code.c_str());
                snapshot[0].market_type = 101 + i % 2;
                snapshot[0].orig_time = convertToAMDTime(time);
                snapshot[0].last_iopv = i + bias++;
                fill(snapshot[0].bid_iopv, amd::ama::ConstField::kPositionLevelLen, i);
                fill(snapshot[0].offer_iopv, amd::ama::ConstField::kPositionLevelLen, i);
                amdSpi->pushIOPVData(snapshot, 1, time);
                delete[] snapshot;
            }
        }
#endif
    } else if (arguments[1]->getForm() == DF_TABLE) {
        int cnt = 1;
        if (arguments.size() > 2) {
            if (arguments[2]->getType() != DT_INT || arguments[2]->getForm() != DF_SCALAR)
                throw IllegalArgumentException(__FUNCTION__, usage + "lines must be a int");
            cnt = arguments[2]->getInt();
        }
        string dataType = arguments[0]->getString();
        TableSP data = arguments[1];

        AMDDataType amdDataType;
        string timeColumnName;
        if (NAME_TYPE.count(dataType) != 0) {
            amdDataType = NAME_TYPE[dataType];
        } else
            throw IllegalArgumentException(__FUNCTION__, usage + "invalid dataType: " + dataType);
        if (timeColumnNameForType.count(amdDataType) != 0) {
            timeColumnName = timeColumnNameForType[amdDataType];
        } else
            throw IllegalArgumentException(__FUNCTION__, usage + "invalid dataType: " + std::to_string(amdDataType));

        if (!data->contain(timeColumnName))
            throw IllegalArgumentException(__FUNCTION__, usage + "data must contain a column named " + string(timeColumnName));
#ifndef AMD_3_9_6
        if (amdDataType != AMD_IOPV_SNAPSHOT && !data->contain("channelNo"))
            throw IllegalArgumentException(__FUNCTION__, usage + "data must contain a column named channelNo");
#else
        if (!data->contain("channelNo"))
            throw IllegalArgumentException(__FUNCTION__, usage + "data must contain a column named channelNo");
#endif
        if (!data->contain("market"))
            throw IllegalArgumentException(__FUNCTION__, usage + "data must contain a column named market");

        vector<string> colNames;
        vector<ConstantSP> cols;
        size_t colsNum = data->columns();
        for (size_t i = 0; i < colsNum; ++i) {
            colNames.push_back(data->getColumnName(i));
            cols.push_back(data->getColumn(i));
        }
        ConstantSP timeStampColumn = data->getColumn(timeColumnName);
        vector<ConstantSP> args = {timeStampColumn};
        VectorSP days = heap->currentSession()->getFunctionDef("date")->call(heap, args);
        cols.push_back(days);
        colNames.push_back("days");
        int rows = data->rows();
        data = Util::createTable(colNames, cols);

        ResultSet resultSet = data;
        int channelNoIndex = data->getColumnIndex("channelNo");
        int marketIndex = data->getColumnIndex("market");
        int dayIndex = data->getColumnIndex("days");
        int timeIndex = data->getColumnIndex(timeColumnName);

        SmartPointer<AmdQuote> instance = AMD_HANDLE_MAP.safeGetByName(AMD_SINGLETON_NAME);
        AMDSpiImp *amdSpi = instance->getAMDSpi();

        long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
        if (amdDataType == AMD_SNAPSHOT || amdDataType == AMD_FUND_SNAPSHOT) {
            for (int i = 0; i < cnt; ++i) {
                if (STOP_TEST) {
                    return new Void();
                }
                amd::ama::MDSnapshot *snapshot = new amd::ama::MDSnapshot[rows];
                int index = 0;
                resultSet.first();
                while (!resultSet.isAfterLast()) {
                    int count = cnt + index;
                    int bias = 0;
                    snapshot[index].channel_no = resultSet.getInt(channelNoIndex);
                    snapshot[index].market_type = resultSet.getInt(marketIndex);
                    snapshot[index].variety_category = amdDataType == AMD_SNAPSHOT ? 1 : 2;
                    snapshot[index].orig_time =
                        convertToAMDTime(resultSet.getLong(timeIndex), resultSet.getInt(dayIndex));
                    string code = std::to_string(600000 + count);
                    std::strcpy(snapshot[index].security_code, code.c_str());

                    snapshot[index].trading_phase_code[0] = 'S';
                    snapshot[index].trading_phase_code[1] = '0';
                    snapshot[index].trading_phase_code[2] = '\0';

                    snapshot[index].pre_close_price = count + bias++;
                    snapshot[index].open_price = count + bias++;
                    snapshot[index].high_price = count + bias++;
                    snapshot[index].low_price = count + bias++;
                    snapshot[index].last_price = count + bias++;
                    snapshot[index].close_price = count + bias++;
                    fill(snapshot[index].bid_price, amd::ama::ConstField::kPositionLevelLen, count);
                    fill(snapshot[index].bid_volume, amd::ama::ConstField::kPositionLevelLen, count);
                    fill(snapshot[index].offer_price, amd::ama::ConstField::kPositionLevelLen, count);
                    fill(snapshot[index].offer_volume, amd::ama::ConstField::kPositionLevelLen, count);
                    snapshot[index].num_trades = count + bias++;
                    snapshot[index].total_volume_trade = count + bias++;
                    snapshot[index].total_value_trade = count + bias++;
                    snapshot[index].total_bid_volume = count + bias++;
                    snapshot[index].total_offer_volume = count + bias++;
                    snapshot[index].weighted_avg_bid_price = count + bias++;
                    snapshot[index].weighted_avg_offer_price = count + bias++;
                    snapshot[index].IOPV = count + bias++;
                    snapshot[index].yield_to_maturity = count + bias++;
                    snapshot[index].high_limited = count + bias++;
                    snapshot[index].low_limited = count + bias++;
                    snapshot[index].price_earning_ratio1 = count + bias++;
                    snapshot[index].price_earning_ratio2 = count + bias++;
                    snapshot[index].change1 = count + bias++;
                    snapshot[index].change2 = count + bias++;
                    snapshot[index].md_stream_id[0] = '\0';
                    snapshot[index].instrument_status[0] = 'B';
                    snapshot[index].pre_close_iopv = count + bias++;
                    snapshot[index].alt_weighted_avg_bid_price = count + bias++;
                    snapshot[index].alt_weighted_avg_offer_price = count + bias++;
                    snapshot[index].etf_buy_number = count + bias++;
                    snapshot[index].etf_buy_amount = count + bias++;
                    snapshot[index].etf_buy_money = count + bias++;
                    snapshot[index].etf_sell_number = count + bias++;
                    snapshot[index].etf_sell_amount = count + bias++;
                    snapshot[index].etf_sell_money = count + bias++;
                    snapshot[index].total_warrant_exec_volume = count + bias++;
                    snapshot[index].war_lower_price = count + bias++;
                    snapshot[index].war_upper_price = count + bias++;
                    snapshot[index].withdraw_buy_number = count + bias++;
                    snapshot[index].withdraw_buy_amount = count + bias++;
                    snapshot[index].withdraw_buy_money = count + bias++;
                    snapshot[index].withdraw_sell_number = count + bias++;
                    snapshot[index].withdraw_sell_amount = count + bias++;
                    snapshot[index].withdraw_sell_money = count + bias++;
                    snapshot[index].total_bid_number = count + bias++;
                    snapshot[index].total_offer_number = count + bias++;
                    snapshot[index].bid_trade_max_duration = count + bias++;
                    snapshot[index].offer_trade_max_duration = count + bias++;
                    snapshot[index].num_bid_orders = count + bias++;
                    snapshot[index].num_offer_orders = count + bias++;
                    snapshot[index].last_trade_time = count + bias++;
                    index++;
                    resultSet.next();
                }
                amdSpi->pushSnapshotData(snapshot, rows, time);
                delete[] snapshot;
            }
        } else if (amdDataType == AMD_EXECUTION || amdDataType == AMD_FUND_EXECUTION) {
            for (int i = 0; i < cnt; ++i) {
                if (STOP_TEST) {
                    return new Void();
                }
                amd::ama::MDTickExecution *snapshot = new amd::ama::MDTickExecution[rows];
                int index = 0;
                resultSet.first();
                while (!resultSet.isAfterLast()) {
                    int count = cnt + index;
                    snapshot[index].channel_no = resultSet.getInt(channelNoIndex);
                    snapshot[index].market_type = resultSet.getInt(marketIndex);
                    snapshot[index].variety_category = amdDataType == AMD_EXECUTION ? 1 : 2;
                    snapshot[index].exec_time =
                        convertToAMDTime(resultSet.getLong(timeIndex), resultSet.getInt(dayIndex));
                    int bias = 0;
                    string code = std::to_string(600000 + count);
                    std::strcpy(snapshot[index].security_code, code.c_str());
                    snapshot[index].appl_seq_num = count + bias++;
                    snapshot[index].exec_price = count + bias++;
                    snapshot[index].exec_volume = count + bias++;
                    snapshot[index].value_trade = count + bias++;
                    snapshot[index].bid_appl_seq_num = count + bias++;
                    snapshot[index].offer_appl_seq_num = count + bias++;
                    snapshot[index].side = 'B';
                    snapshot[index].exec_type = 'F';
                    snapshot[index].md_stream_id[0] = '\0';
                    snapshot[index].biz_index = count + bias++;
                    index++;
                    resultSet.next();
                }
                amdSpi->pushExecutionData(snapshot, rows, time);
                delete[] snapshot;
            }
        } else if (amdDataType == AMD_ORDER || amdDataType == AMD_FUND_ORDER) {
            for (int i = 0; i < cnt; ++i) {
                if (STOP_TEST) {
                    return new Void();
                }
                amd::ama::MDTickOrder *snapshot = new amd::ama::MDTickOrder[rows];
                int index = 0;
                resultSet.first();
                while (!resultSet.isAfterLast()) {
                    int count = cnt + index;
                    snapshot[index].channel_no = resultSet.getInt(channelNoIndex);
                    snapshot[index].market_type = resultSet.getInt(marketIndex);
                    snapshot[index].variety_category = amdDataType == AMD_ORDER ? 1 : 2;
                    snapshot[index].order_time =
                        convertToAMDTime(resultSet.getLong(timeIndex), resultSet.getInt(dayIndex));

                    int bias = 0;
                    string code = std::to_string(600000 + count);
                    std::strcpy(snapshot[index].security_code, code.c_str());
                    snapshot[index].appl_seq_num = count + bias++;
                    snapshot[index].order_price = count + bias++;
                    snapshot[index].order_volume = count + bias++;
                    snapshot[index].side = 'B';
                    snapshot[index].order_type = 'S';
                    snapshot[index].md_stream_id[0] = '\0';
                    snapshot[index].orig_order_no = count + bias++;
                    snapshot[index].biz_index = count + bias++;
                    index++;
                    resultSet.next();
                }
                amdSpi->pushOrderData(snapshot, rows, time);
                delete[] snapshot;
            }
        } else if (amdDataType == AMD_BOND_SNAPSHOT) {
            for (int i = 0; i < cnt; ++i) {
                if (STOP_TEST) {
                    return new Void();
                }
                amd::ama::MDBondSnapshot *snapshot = new amd::ama::MDBondSnapshot[rows];
                int index = 0;
                resultSet.first();
                while (!resultSet.isAfterLast()) {
                    int bias = 0;
                    int count = cnt + index;
                    snapshot[index].channel_no = resultSet.getInt(channelNoIndex);
                    snapshot[index].market_type = resultSet.getInt(marketIndex);
                    snapshot[index].variety_category = 3;
                    snapshot[index].orig_time =
                        convertToAMDTime(resultSet.getLong(timeIndex), resultSet.getInt(dayIndex));
                    string code = std::to_string(600000 + count);
                    std::strcpy(snapshot[index].security_code, code.c_str());

                    snapshot[index].trading_phase_code[0] = 'S';
                    snapshot[index].trading_phase_code[1] = '0';
                    snapshot[index].trading_phase_code[2] = '\0';

                    snapshot[index].pre_close_price = count + bias++;
                    snapshot[index].open_price = count + bias++;
                    snapshot[index].high_price = count + bias++;
                    snapshot[index].low_price = count + bias++;
                    snapshot[index].last_price = count + bias++;
                    snapshot[index].close_price = count + bias++;
                    fill(snapshot[index].bid_price, amd::ama::ConstField::kPositionLevelLen, count);
                    fill(snapshot[index].bid_volume, amd::ama::ConstField::kPositionLevelLen, count);
                    fill(snapshot[index].offer_price, amd::ama::ConstField::kPositionLevelLen, count);
                    fill(snapshot[index].offer_volume, amd::ama::ConstField::kPositionLevelLen, count);
                    snapshot[index].num_trades = count + bias++;
                    snapshot[index].total_volume_trade = count + bias++;
                    snapshot[index].total_value_trade = count + bias++;
                    snapshot[index].total_bid_volume = count + bias++;
                    snapshot[index].total_offer_volume = count + bias++;
                    snapshot[index].weighted_avg_bid_price = count + bias++;
                    snapshot[index].weighted_avg_offer_price = count + bias++;
                    snapshot[index].high_limited = count + bias++;
                    snapshot[index].low_limited = count + bias++;
                    snapshot[index].change1 = count + bias++;
                    snapshot[index].change2 = count + bias++;
                    snapshot[index].md_stream_id[0] = '\0';
                    snapshot[index].instrument_status[0] = 'C';
                    snapshot[index].withdraw_buy_number = count + bias++;
                    snapshot[index].withdraw_buy_amount = count + bias++;
                    snapshot[index].withdraw_buy_money = count + bias++;
                    snapshot[index].withdraw_sell_number = count + bias++;
                    snapshot[index].withdraw_sell_amount = count + bias++;
                    snapshot[index].withdraw_sell_money = count + bias++;
                    snapshot[index].total_bid_number = count + bias++;
                    snapshot[index].total_offer_number = count + bias++;
                    snapshot[index].bid_trade_max_duration = count + bias++;
                    snapshot[index].offer_trade_max_duration = count + bias++;
                    snapshot[index].num_bid_orders = count + bias++;
                    snapshot[index].num_offer_orders = count + bias++;
                    snapshot[index].last_trade_time = count + bias++;
                    index++;
                    resultSet.next();
                }
                amdSpi->pushBondSnapshotData(snapshot, rows, time);
                delete[] snapshot;
            }
        } else if (amdDataType == AMD_BOND_EXECUTION) {
            for (int i = 0; i < cnt; ++i) {
                if (STOP_TEST) {
                    return new Void();
                }
                amd::ama::MDBondTickExecution *snapshot = new amd::ama::MDBondTickExecution[rows];
                int index = 0;
                resultSet.first();
                while (!resultSet.isAfterLast()) {
                    int count = cnt + index;
                    snapshot[index].channel_no = resultSet.getInt(channelNoIndex);
                    snapshot[index].market_type = resultSet.getInt(marketIndex);
                    snapshot[index].variety_category = 3;
                    snapshot[index].exec_time =
                        convertToAMDTime(resultSet.getLong(timeIndex), resultSet.getInt(dayIndex));
                    int bias = 0;
                    string code = std::to_string(600000 + count);
                    std::strcpy(snapshot[index].security_code, code.c_str());
                    snapshot[index].appl_seq_num = count + bias++;
                    snapshot[index].exec_price = count + bias++;
                    snapshot[index].exec_volume = count + bias++;
                    snapshot[index].value_trade = count + bias++;
                    snapshot[index].bid_appl_seq_num = count + bias++;
                    snapshot[index].offer_appl_seq_num = count + bias++;
                    snapshot[index].side = 'B';
                    snapshot[index].exec_type = 'F';
                    snapshot[index].md_stream_id[0] = '\0';
                    index++;
                    resultSet.next();
                }
                amdSpi->pushBondExecutionData(snapshot, rows, time);
                delete[] snapshot;
            }
        } else if (amdDataType == AMD_BOND_ORDER) {
            for (int i = 0; i < cnt; ++i) {
                if (STOP_TEST) {
                    return new Void();
                }
                amd::ama::MDBondTickOrder *snapshot = new amd::ama::MDBondTickOrder[rows];
                int index = 0;
                resultSet.first();
                while (!resultSet.isAfterLast()) {
                    int count = cnt + index;
                    snapshot[index].channel_no = resultSet.getInt(channelNoIndex);
                    snapshot[index].market_type = resultSet.getInt(marketIndex);
                    snapshot[index].variety_category = 3;
                    snapshot[index].order_time =
                        convertToAMDTime(resultSet.getLong(timeIndex), resultSet.getInt(dayIndex));

                    int bias = 0;
                    string code = std::to_string(600000 + count);
                    std::strcpy(snapshot[index].security_code, code.c_str());
                    snapshot[index].appl_seq_num = count + bias++;
                    snapshot[index].order_price = count + bias++;
                    snapshot[index].order_volume = count + bias++;
                    snapshot[index].side = 'B';
                    snapshot[index].order_type = 'S';
                    snapshot[index].md_stream_id[0] = '\0';
                    snapshot[index].orig_order_no = count + bias++;
                    index++;
                    resultSet.next();
                }
                amdSpi->pushBondOrderData(snapshot, rows, time);
                delete[] snapshot;
            }
        } else if (amdDataType == AMD_INDEX) {
            for (int i = 0; i < cnt; ++i) {
                if (STOP_TEST) {
                    return new Void();
                }
                amd::ama::MDIndexSnapshot *snapshot = new amd::ama::MDIndexSnapshot[rows];
                int index = 0;
                resultSet.first();
                while (!resultSet.isAfterLast()) {
                    int count = cnt + index;
                    snapshot[index].channel_no = resultSet.getInt(channelNoIndex);
                    snapshot[index].market_type = resultSet.getInt(marketIndex);
                    snapshot[index].orig_time =
                        convertToAMDTime(resultSet.getLong(timeIndex), resultSet.getInt(dayIndex));
                    int bias = 0;
                    string code = std::to_string(600000 + count);
                    std::strcpy(snapshot[index].security_code, code.c_str());
                    snapshot[index].variety_category = 4;
                    snapshot[index].trading_phase_code[0] = 'S';
                    snapshot[index].trading_phase_code[1] = '0';
                    snapshot[index].trading_phase_code[2] = '\0';
                    snapshot[index].pre_close_index = count + bias++;
                    snapshot[index].open_index = count + bias++;
                    snapshot[index].high_index = count + bias++;
                    snapshot[index].low_index = count + bias++;
                    snapshot[index].last_index = count + bias++;
                    snapshot[index].close_index = count + bias++;
                    snapshot[index].total_volume_trade = count + bias++;
                    snapshot[index].total_value_trade = count + bias++;
                    snapshot[index].md_stream_id[0] = '\0';
                    index++;
                    resultSet.next();
                }
                amdSpi->pushIndexData(snapshot, rows, time);
                delete[] snapshot;
            }
        } else if (amdDataType == AMD_ORDER_QUEUE) {
            for (int i = 0; i < cnt; ++i) {
                if (STOP_TEST) {
                    return new Void();
                }
                amd::ama::MDOrderQueue *snapshot = new amd::ama::MDOrderQueue[rows];
                int index = 0;
                resultSet.first();
                while (!resultSet.isAfterLast()) {
                    int count = cnt + index;
                    snapshot[index].channel_no = resultSet.getInt(channelNoIndex);
                    snapshot[index].market_type = resultSet.getInt(marketIndex);
                    snapshot[index].order_time =
                        convertToAMDTime(resultSet.getLong(timeIndex), resultSet.getInt(dayIndex));
                    int bias = 0;
                    string code = std::to_string(600000 + count);
                    std::strcpy(snapshot[index].security_code, code.c_str());
                    snapshot[index].variety_category = 5;
                    snapshot[index].side = 'B';
                    snapshot[index].order_price = count + bias++;
                    snapshot[index].order_volume = count + bias++;
                    snapshot[index].num_of_orders = count + bias++;
                    snapshot[index].items = count + bias++;
                    fill(snapshot[index].volume, 50, count);
                    snapshot[index].md_stream_id[0] = '\0';
                    index++;
                    resultSet.next();
                }
                amdSpi->pushOrderQueueData(snapshot, rows, time);
                delete[] snapshot;
            }
        } else if (amdDataType == AMD_OPTION_SNAPSHOT) {
            for (int i = 0; i < cnt; ++i) {
                if (STOP_TEST) {
                    return new Void();
                }
                amd::ama::MDOptionSnapshot *snapshot = new amd::ama::MDOptionSnapshot[rows];
                int index = 0;
                resultSet.first();
                while (!resultSet.isAfterLast()) {
                    int count = cnt + index;
                    snapshot[index].channel_no = resultSet.getInt(channelNoIndex);
                    snapshot[index].market_type = resultSet.getInt(marketIndex);
                    snapshot[index].variety_category = 6;
                    snapshot[index].orig_time =
                        convertToAMDTime(resultSet.getLong(timeIndex), resultSet.getInt(dayIndex));
                    int bias = 0;
                    string code = std::to_string(600000 + count);
                    std::strcpy(snapshot[index].security_code, code.c_str());
                    snapshot[index].pre_settle_price = count + bias++;
                    snapshot[index].pre_close_price = count + bias++;
                    snapshot[index].open_price = count + bias++;
                    snapshot[index].auction_price = count + bias++;
                    snapshot[index].auction_volume = count + bias++;
                    snapshot[index].high_price = count + bias++;
                    snapshot[index].low_price = count + bias++;
                    snapshot[index].last_price = count + bias++;
                    snapshot[index].close_price = count + bias++;
                    snapshot[index].high_limited = count + bias++;
                    snapshot[index].low_limited = count + bias++;
                    fill(snapshot[index].bid_price, 5, count);
                    fill(snapshot[index].bid_volume, 5, count);
                    fill(snapshot[index].offer_price, 5, count);
                    fill(snapshot[index].offer_volume, 5, count);
                    snapshot[index].settle_price = count + bias++;
                    snapshot[index].total_long_position = count + bias++;
                    snapshot[index].total_volume_trade = count + bias++;
                    snapshot[index].total_value_trade = count + bias++;
                    snapshot[index].trading_phase_code[0] = 'S';
                    snapshot[index].trading_phase_code[1] = '0';
                    snapshot[index].trading_phase_code[2] = '\0';
                    snapshot[index].md_stream_id[0] = '\0';
                    snapshot[index].last_trade_time = count + bias++;
                    snapshot[index].ref_price = count + bias++;
                    snapshot[index].contract_type = count + bias++;
                    snapshot[index].expire_date = count + bias++;
                    snapshot[index].underlying_security_code[0] = 'O';
                    snapshot[index].exercise_price = count + bias++;
                    index++;
                    resultSet.next();
                }
                amdSpi->pushOptionData(snapshot, rows, time);
                delete[] snapshot;
            }
        } else if (amdDataType == AMD_FUTURE_SNAPSHOT) {

            for (int i = 0; i < cnt; ++i) {
                if (STOP_TEST) {
                    return new Void();
                }
                amd::ama::MDFutureSnapshot *snapshot = new amd::ama::MDFutureSnapshot[rows];
                int index = 0;
                resultSet.first();
                while (!resultSet.isAfterLast()) {
                    int count = cnt + index;
                    int bias = 0;
                    string code = std::to_string(600000 + i);
                    std::strcpy(snapshot[index].security_code, code.c_str());
                    snapshot[index].market_type = resultSet.getInt(marketIndex);
                    snapshot[index].variety_category = 6;
                    snapshot[index].action_day = convertToAMDTime(time) / 100000;
                    snapshot[index].orig_time = convertToAMDTime(resultSet.getLong(timeIndex), resultSet.getInt(dayIndex));
                    snapshot[index].exchange_inst_id[0] = '\0';
                    snapshot[index].last_price = count + bias++;
                    snapshot[index].pre_settle_price = count + bias++;
                    snapshot[index].pre_close_price = count + bias++;
                    snapshot[index].open_interest = count + bias++;
                    snapshot[index].open_price = count + bias++;
                    snapshot[index].high_price = count + bias++;
                    snapshot[index].low_price = count + bias++;

                    // TODO(slshen) fill all mock field
                    snapshot[index].last_price = count + bias++;
                    snapshot[index].close_price = count + bias++;
                    snapshot[index].high_limited = count + bias++;
                    snapshot[index].low_limited = count + bias++;
                    fill(snapshot[index].bid_price, 5, i);
                    fill(snapshot[index].bid_volume, 5, i);
                    fill(snapshot[index].offer_price, 5, i);
                    fill(snapshot[index].offer_volume, 5, i);
                    snapshot[index].settle_price = count + bias++;
                    snapshot[index].total_volume_trade = count + bias++;
                    snapshot[index].total_value_trade = count + bias++;
                }
                amdSpi->pushFutureData(snapshot, rows, time);
                delete[] snapshot;
            }
        }
#ifndef AMD_3_9_6
        else if (amdDataType == AMD_IOPV_SNAPSHOT) {
            for (int i = 0; i < cnt; ++i) {
                if (STOP_TEST) {
                    return new Void();
                }
                amd::ama::MDIOPVSnapshot *snapshot = new amd::ama::MDIOPVSnapshot[rows];
                int index = 0;
                resultSet.first();
                while (!resultSet.isAfterLast()) {
                    int count = cnt + index;
                    snapshot[index].market_type = resultSet.getInt(marketIndex);
                    snapshot[index].orig_time =
                        convertToAMDTime(resultSet.getLong(timeIndex), resultSet.getInt(dayIndex));
                    int bias = 0;
                    string code = std::to_string(600000 + count);
                    std::strcpy(snapshot[index].security_code, code.c_str());
                    snapshot[index].last_iopv = count + bias++;
                    fill(snapshot[index].bid_iopv, amd::ama::ConstField::kPositionLevelLen, count);
                    fill(snapshot[index].offer_iopv, amd::ama::ConstField::kPositionLevelLen, count);
                    index++;
                    resultSet.next();
                }
                amdSpi->pushIOPVData(snapshot, rows, time);
                delete[] snapshot;
            }
        }
#endif
    } else {
        throw IllegalArgumentException(__FUNCTION__, usage + "invalid usage");
    }
    return new Void();
}