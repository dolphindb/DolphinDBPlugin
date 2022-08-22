/**
 * @file         ama_tools.h
 * @breif        TODO
 * @author       郭光葵
 * @mail         guoguangkui@archforce.com.cn
 * @created time Wed 29 Nov 2017 10:03:29 AM CST
 *
 * Copyright (c) 2018 Archforce Financial Technology.  All rights reserved.
 * Redistribution and use in source and binary forms, with or without  modification, are not permitted.
 * For more information about Archforce, welcome to archforce.cn.
 */

#ifndef AMA_TOOLS_H_
#define AMA_TOOLS_H_

#include <string>
#include "ama_struct.h"
#include "ama_export.h"

namespace amd { namespace ama {

class AMA_EXPORT Tools
{
public:
    static void SetUMSServers(UMSItem* cfg,  uint32_t index, const UMSItem& value);
    static void GetUMSServers(UMSItem* cfg,  uint32_t index, UMSItem& value);

    static SubscribeItem* CreateSubscribeItem(uint32_t cnt);
    static void DestroySubscribeItem(SubscribeItem* item);
    static SubscribeCategoryItem* CreateSubscribeCategoryItem(uint32_t cnt);
    static void DestroySubscribeCategoryItem(SubscribeCategoryItem* item);

    static void SetSubscribeItem(SubscribeItem* item,  uint32_t index, const SubscribeItem& value);
    static const SubscribeItem& GetSubscribeItemByIndex(const SubscribeItem* item, uint32_t index);
    static void SetSubscribeCategoryItem(SubscribeCategoryItem* item,  uint32_t index, const SubscribeCategoryItem& value);
    static const SubscribeCategoryItem& GetSubscribeCategoryItemByIndex(const SubscribeCategoryItem* item, uint32_t index);

    static SubscribeOrderBookItem* CreateSubscribeOrderBookItem(uint32_t cnt);
    static void DestroySubscribeOrderBookItem(SubscribeOrderBookItem *item);

    static void SetSubscribeOrderBookItem(SubscribeOrderBookItem* item,  uint32_t index, const SubscribeOrderBookItem& value);
    static const SubscribeOrderBookItem& GetSubscribeOrderBookItemByIndex(const SubscribeOrderBookItem* item, uint32_t index);

     static SubscribeDerivedDataItem* CreateSubscribeDerivedDataItem(uint32_t cnt);
    static void DestroySubscribeDerivedDataItem(SubscribeDerivedDataItem *item);

    static void SetSubscribeDerivedDataItem(SubscribeDerivedDataItem* item,  uint32_t index, const SubscribeDerivedDataItem& value);
    static const SubscribeDerivedDataItem& GetSubscribeDerivedDataItemByIndex(const SubscribeDerivedDataItem* item, uint32_t index);

    static CodeTableRecordList* CreateCodeTableRecordList();
    static void DestroyCodeTableRecordList(CodeTableRecordList* item);

    static ETFCodeTableRecordList* CreateETFCodeTableRecordList();
    static void DestroyETFCodeTableRecordList(ETFCodeTableRecordList* item);

    static SubCodeTableItem* CreateSubCodeTableItem(uint32_t cnt);
    static void DestroySubCodeTableItem(SubCodeTableItem* item);
    static const SubCodeTableItem& GetSubCodeTableItemByIndex(const SubCodeTableItem* item, uint32_t index);
    static void SetSubCodeTableItem(SubCodeTableItem* item,  uint32_t index, const SubCodeTableItem& value);

    static ETFItem* CreateETFItem(uint32_t cnt);
    static void DestroyETFItem(ETFItem* item);
    static void SetETFItem(ETFItem* item,  uint32_t index, const ETFItem& value);
    static const ETFItem& GetETFItemByIndex(const ETFItem* item, uint32_t index);

    static const MDSnapshot &GetDataByIndex(const MDSnapshot* snapshot, uint32_t index);
    static const MDOptionSnapshot &GetDataByIndex(const MDOptionSnapshot* snapshot, uint32_t index);
    static const MDHKTSnapshot &GetDataByIndex(const MDHKTSnapshot* snapshot, uint32_t index);
    static const MDIndexSnapshot &GetDataByIndex(const MDIndexSnapshot* snapshot, uint32_t index);
    static const MDTickOrder &GetDataByIndex(const MDTickOrder* tick, uint32_t index);
    static const MDTickExecution &GetDataByIndex(const MDTickExecution* tick, uint32_t index);
    static const MDOrderQueue &GetDataByIndex(const MDOrderQueue* orderqueue, uint32_t index);
    /*
    static const MDOptionBasicInfo &GetDataByIndex(const MDOptionBasicInfo* code, uint32_t index);
    */
    static const MDFutureSnapshot &GetDataByIndex(const MDFutureSnapshot* snapshot, uint32_t index);
    static const MDCSIIndexSnapshot &GetDataByIndex(const MDCSIIndexSnapshot* snapshot, uint32_t index);
    static const MDAfterHourFixedPriceSnapshot& GetDataByIndex(const MDAfterHourFixedPriceSnapshot* snapshots, uint32_t index);
    static const MDAfterHourFixedPriceTickExecution& GetDataByIndex(const MDAfterHourFixedPriceTickExecution* ticks, uint32_t index);
    static const MDIndicatorOfTradingVolumeSnapshot &GetDataByIndex(const MDIndicatorOfTradingVolumeSnapshot* snapshot, uint32_t index);
    static const MDCnIndexSnapshot &GetDataByIndex(const MDCnIndexSnapshot* snapshots, uint32_t index);
    static const MDRefinancingTickOrder& GetDataByIndex(const MDRefinancingTickOrder* ticks, uint32_t index);
    static const MDRefinancingTickExecution& GetDataByIndex(const MDRefinancingTickExecution* ticks, uint32_t index);
    static const MDNegotiableTickOrder& GetDataByIndex(const MDNegotiableTickOrder* ticks, uint32_t index);
    static const MDNegotiableTickExecution& GetDataByIndex(const MDNegotiableTickExecution* ticks, uint32_t index);
    static const MDHKTRealtimeLimit& GetDataByIndex(const MDHKTRealtimeLimit* limits, uint32_t index);
    static const MDHKMarketStatus& GetDataByIndex(const MDHKMarketStatus* limits, uint32_t index);
    static const MDHKTProductStatus& GetDataByIndex(const MDHKTProductStatus* status, uint32_t index);
    static const MDHKTVCM& GetDataByIndex(const MDHKTVCM* status, uint32_t index);
    static const MDNEEQSnapshot& GetDataByIndex(const MDNEEQSnapshot* snapshots, uint32_t index);
    static const MDNEEQSecurityInfo& GetDataByIndex(const MDNEEQSecurityInfo* infos, uint32_t index);
    static const MDNEEQNonPublicTransDeclaredInfo& GetDataByIndex(const MDNEEQNonPublicTransDeclaredInfo* infos, uint32_t index);
    static const MDNEEQHierarchicalInfo& GetDataByIndex(const MDNEEQHierarchicalInfo* infos, uint32_t index);
    static const MDNEEQNegotiableDeclaredInfo& GetDataByIndex(const MDNEEQNegotiableDeclaredInfo* infos, uint32_t index);
    static const MDNEEQMarketMakerDeclaredInfo& GetDataByIndex(const MDNEEQMarketMakerDeclaredInfo* infos, uint32_t index);
    static const MDNEEQNonPublicTransferDealInfo& GetDataByIndex(const MDNEEQNonPublicTransferDealInfo* infos, uint32_t index);
    static const MDOrderBook& GetDataByIndex(std::vector<MDOrderBook>& order_book, uint32_t index);
    static const MDOrderBookSnapshot &GetDataByIndex(const MDOrderBookSnapshot* snapshot, uint32_t index);
    static const MDIOPVSnapshot &GetDataByIndex(const MDIOPVSnapshot* snapshots, uint32_t index);
    static const CodeTableRecord& GetDataByIndex(const CodeTableRecord* records, uint32_t index);
    static const MDBondSnapshot &GetDataByIndex(const MDBondSnapshot* snapshot, uint32_t index);
    static const MDBondTickOrder &GetDataByIndex(const MDBondTickOrder* snapshot, uint32_t index);
    static const MDBondQuotedTickOrder &GetDataByIndex(const MDBondQuotedTickOrder* snapshot, uint32_t index);
    static const MDBondTickExecution &GetDataByIndex(const MDBondTickExecution* snapshot, uint32_t index);
    static const MDBondQuotedTickExecution &GetDataByIndex(const MDBondQuotedTickExecution* snapshot, uint32_t index);
    static const MDFundExpertSnapshot &GetDataByIndex(const MDFundExpertSnapshot* snapshot, uint32_t index);

    static const long long GetDataSize(std::vector<MDOrderBook>& order_book);
    static const SubTradingPhase& GetSubTradingPhaseByIndex(const SubTradingPhase* array_data, uint32_t index);

    static const long long& GetInt64DataByIndex(const long long* array_data, uint32_t index);

    static const ETFCodeTableRecord& GetDataByIndex(const ETFCodeTableRecord* records, uint32_t index);
    static const long long GetDataSize(std::vector<ConstituentStockInfo>& stocks);
    static const ConstituentStockInfo& GetDataByIndex(std::vector<ConstituentStockInfo>& stocks, uint32_t index);

    static const long long GetDataSize(std::vector<MDOrderBookItem>& book_item);
    static const MDOrderBookItem& GetDataByIndex(std::vector<MDOrderBookItem>& book_item, uint32_t index);

    static std::string Serialize(const MDSnapshot& snapshot);
    static std::string Serialize(const MDOptionSnapshot& snapshot);
    static std::string Serialize(const MDHKTSnapshot& snapshot);
    static std::string Serialize(const MDIndexSnapshot& snapshot);
    static std::string Serialize(const MDTickOrder& tick);
    static std::string Serialize(const MDTickExecution& tick);
    static std::string Serialize(const MDOrderQueue& orderqueue);
    static std::string Serialize(const MDFutureSnapshot& snapshot);
    static std::string Serialize(const MDCSIIndexSnapshot& snapshot);
    /*
    static std::string Serialize(const MDOptionBasicInfo& basicinfo);
    */
    static std::string Serialize(const MDAfterHourFixedPriceSnapshot& snapshot);
    static std::string Serialize(const MDAfterHourFixedPriceTickExecution& tick);
    static std::string Serialize(const MDIndicatorOfTradingVolumeSnapshot& snapshot);
    static std::string Serialize(const MDCnIndexSnapshot& snapshot);
    static std::string Serialize(const MDRefinancingTickOrder& tick);
    static std::string Serialize(const MDRefinancingTickExecution& tick);
    static std::string Serialize(const MDNegotiableTickOrder& tick);
    static std::string Serialize(const MDNegotiableTickExecution& tick);
    static std::string Serialize(const MDHKTRealtimeLimit& limit);
    static std::string Serialize(const MDHKTProductStatus& status);
    static std::string Serialize(const MDHKMarketStatus& status);
    static std::string Serialize(const MDHKTVCM& status);
    static std::string Serialize(const MDNEEQSnapshot& snapshot);
    static std::string Serialize(const MDNEEQSecurityInfo& info);
    static std::string Serialize(const MDNEEQNonPublicTransDeclaredInfo& info);
    static std::string Serialize(const MDNEEQHierarchicalInfo& info);
    static std::string Serialize(const MDNEEQNegotiableDeclaredInfo& info);
    static std::string Serialize(const MDNEEQMarketMakerDeclaredInfo& info);
    static std::string Serialize(const MDNEEQNonPublicTransferDealInfo& info);
    static std::string Serialize(const MDOrderBookSnapshot& snapshot);
    static std::string Serialize(const MDIOPVSnapshot& snapshot);
    static std::string Serialize(const MDOrderBook& order_book);
    static std::string Serialize(const CodeTableRecord& record);
    static std::string Serialize(const MDBondSnapshot& snapshot);
    static std::string Serialize(const MDBondTickOrder& snapshot);
    static std::string Serialize(const MDBondQuotedTickOrder& snapshot);
    static std::string Serialize(const MDBondTickExecution& snapshot);
    static std::string Serialize(const MDBondQuotedTickExecution& snapshot);
    static std::string Serialize(const ETFCodeTableRecord& record);
    static std::string Serialize(const MDFundExpertSnapshot& snapshot);

    static void FreeMemory(MDSnapshot* snapshots);
    static void FreeMemory(MDOptionSnapshot* snapshots);
    static void FreeMemory(MDHKTSnapshot* snapshots);
    static void FreeMemory(MDIndexSnapshot* snapshots);
    static void FreeMemory(MDTickOrder* ticks);
    static void FreeMemory(MDTickExecution* ticks);
    static void FreeMemory(MDOrderQueue* orderqueues);
    static void FreeMemory(MDAfterHourFixedPriceSnapshot* snapshots);
    static void FreeMemory(MDAfterHourFixedPriceTickExecution* ticks);
    static void FreeMemory(MDFutureSnapshot* snapshots);
    static void FreeMemory(MDCSIIndexSnapshot* snapshots);
    static void FreeMemory(MDIndicatorOfTradingVolumeSnapshot* snapshots);
    static void FreeMemory(MDCnIndexSnapshot* snapshots);
    static void FreeMemory(MDRefinancingTickOrder* ticks);
    static void FreeMemory(MDRefinancingTickExecution* ticks);
    static void FreeMemory(MDNegotiableTickOrder* ticks);
    static void FreeMemory(MDNegotiableTickExecution* ticks);
    static void FreeMemory(MDHKTRealtimeLimit* limit);
    static void FreeMemory(MDHKMarketStatus* limit);
    static void FreeMemory(MDHKTProductStatus* status);
    static void FreeMemory(MDHKTVCM* status);
    static void FreeMemory(MDNEEQSnapshot* snapshot);
    static void FreeMemory(MDNEEQSecurityInfo* info);
    static void FreeMemory(MDNEEQNonPublicTransDeclaredInfo* info);
    static void FreeMemory(MDNEEQHierarchicalInfo* info);
    static void FreeMemory(MDNEEQNegotiableDeclaredInfo* info);
    static void FreeMemory(MDNEEQMarketMakerDeclaredInfo* info);
    static void FreeMemory(MDNEEQNonPublicTransferDealInfo* info);
    static void FreeMemory(CodeTableRecord* info);
    static void FreeMemory(ETFCodeTableRecord* info);
    static void FreeMemory(MDBondSnapshot* info);
    static void FreeMemory(MDBondTickOrder* info);
    static void FreeMemory(MDBondQuotedTickOrder* info);
    static void FreeMemory(MDBondTickExecution* info);
    static void FreeMemory(MDBondQuotedTickExecution* info);
    static void FreeMemory(MDOrderBookSnapshot* info);
    static void FreeMemory(MDIOPVSnapshot* info);
    static void FreeMemory(MDFundExpertSnapshot* info);
    static std::string GetEventLevelString(uint32_t level);
    static std::string GetEventCodeString(uint32_t level);
};


};
};


#endif //SWIG_WRAP_H_
