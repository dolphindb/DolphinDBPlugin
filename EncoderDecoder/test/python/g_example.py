import example_pb2

def setInfo(inst):
    o_header = inst.header
    o_body = inst.body
    o_list = o_body.pbMarketDataList
    o_data1 = o_list.marketData.add()
    o_data2 = o_list.marketData.add()

    o_header.action = "action"
    o_header.clientID = "id"
    o_header.serialNo = "no"
    o_header.version = "ver"
    o_header.msgType = "type"
    o_header.sendingTime = "sendtime"
    # o_header.from = "from"
    o_header.operator = "operator"
    o_header.errorCode = "0"
    o_header.errorMsg = "none"

    o_body.qid = 1
    o_body.mdBookType = 2
    o_body.marketIndicator = "indicator"
    o_body.clOrdID = "id"
    o_body.transactTime = "trans-time"
    o_body.securityType = "sec-type"
    o_body.securityID = "sec-id"
    o_body.symbol = "symbol"
    o_body.marketDepth = 100
    o_body.mdSubBookType = "book"
    o_body.realTimeUndertakeFlag = "real"
    o_body.subjectPartyType = "sub"
    o_body.sendingTime = "sendtime_b"
    o_body.repoMethod = "repo"
    o_body.transactionMethod = "method"
    o_body.tradeMethod = "trade"
    o_body.bridgeDealIndic = "bridge"
    o_body.splitIndicator = "split"

    o_data1.mdEntryType = "md"
    o_data1.mdPriceLevel = 2
    o_data1.mdQuoteType = 3
    o_data1.quoteEntryID = "quote"
    o_data1.mdEntryDate = "entry_date"
    o_data1.mdEntryTime = "entry_time"
    o_data1.minQty = "min"
    o_data1.lastPx = "last"
    o_data1.mdEntryPx = "entryPx"
    o_data1.mdEntrySize = "size"
    o_data1.clearingMethod = 233
    o_data1.settlType = "type"
    o_data1.settlDate = "date"
    o_data1.b_DeliveryType = "b_Del"
    o_data1.s_DeliveryType = "s_Del"
    o_data1.settlCurrency = "sett"
    o_data1.settlCurrFxRate = "curr"
    o_data1.maturityYield = "mature"
    o_data1.partyID = "partyId"
    o_data1.traderID = "tradeId"
    o_data1.traderName = "tradeNum"
    o_data1.tradingAcctNumber = "acct"
    o_data1.tradeVolume = "vol"
    o_data1.unMatchQty = "match"
    o_data1.increasePositionValue = "pos"

    o_data2.mdEntryType = "md2"
    o_data2.mdPriceLevel = 22
    o_data2.mdQuoteType = 32
    o_data2.quoteEntryID = "quote2"
    o_data2.mdEntryDate = "entry_date2"
    o_data2.mdEntryTime = "entry_time2"
    o_data2.minQty = "min2"
    o_data2.lastPx = "last2"
    o_data2.mdEntryPx = "entryPx2"
    o_data2.mdEntrySize = "size2"
    o_data2.clearingMethod = 2332
    o_data2.settlType = "type2"
    o_data2.settlDate = "date2"
    o_data2.b_DeliveryType = "b_Del2"
    o_data2.s_DeliveryType = "s_Del2"
    o_data2.settlCurrency = "sett2"
    o_data2.settlCurrFxRate = "curr2"
    o_data2.maturityYield = "mature2"
    o_data2.partyID = "partyId2"
    o_data2.traderID = "tradeId2"
    o_data2.traderName = "tradeNum2"
    o_data2.tradingAcctNumber = "acct2"
    o_data2.tradeVolume = "vol2"
    o_data2.unMatchQty = "match2"
    o_data2.increasePositionValue = "pos2"
    # print(inst)
    return inst

msg = example_pb2.Pb_CfetsTradeMarketDataSubscribeReceiveMessage()
msg_cmpl = setInfo(msg)
print(msg_cmpl)
proto_info = msg_cmpl.SerializeToString()
print(proto_info)

def getInfo(wanted_info):
    print("header id:", wanted_info.header.clientID)
    print("body qid: ", wanted_info.body.qid)
    for data in wanted_info.body.pbMarketDataList.marketData:
        print("market level:", data.mdPriceLevel)

first_parsed = example_pb2.Pb_CfetsTradeMarketDataSubscribeReceiveMessage()
first_parsed.ParseFromString(proto_info)

getInfo(first_parsed)

with open("./data_example.txt", "wb") as file:
    file.write(proto_info)
