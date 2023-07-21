# DolphinDB Matching Engine

## Build

Build plugin
```
cmake . -DCMAKE_BUILD_TYPE=Release -DDOLPHINDB_LIB_DIR=/path/to/the/libDolphinDB.so -DDOLPHINDB_INC_DIR=/path/to/the/include
make && make install
```

Run test (gtest is required)

```
sudo apt-get install libgtest-dev #install gtest if not installed
sudo apt-get install cmake # install cmake if not installed
cd /usr/src/gtest
sudo cmake CMakeLists.txt
sudo make

#copy or symlink libgtest.a and libgtest_main.a to your /usr/lib folder
sudo cp *.a /usr/lib
```



```
cmake . -DCMAKE_BUILD_TYPE=Debug -DTEST=1
make
./MatchingEngine
```

## Example

DolphinDB script
```
/*
 * load matching engine plugin
 */
loadPlugin("/path/to/the/PluginMatchingEngine.txt")

/* 
 * input scheme: `op`symbol`id`quantity`condition`price`thresholdPrice`expiredTime
 *
 * op:              see the following explanation
 * symbol:          the symbol of the input orders, not checked (i.e. the symbol will not be checked inside the engine)
 * id:              the unique id of an order for identification, not checked
 * quantity:        the quantity of an order, must be a positive integer, not checked
 * condition:       see the following explanation
 * price:           the price of an order, must be a non-negative integer, 0 for market order, not checked
 * thresholdPrice:  the stop-loss/take-profit/trailing-stop price of an order, must be a positive integer, not checked
 * expiredTime:     the expiredTime of an order, 0 for never expired order, must be a non-negative integer, not checked
 */

/*
 * op: the operation type of the input order
 * 0-add, 1-modify(replace), 2-cancel
 */
ORDER_ADD = 0
ORDER_MOD = 1
ORDER_CAN = 2

/*
 * condition: the type of the order
 * SL for stop loss, TP for take profit, TS for trailing stop
 *
 * ...0    0     0     0     0     0     0     0
 *               |     |     |     |     |     |
 *             isTS  isTP  isSL  isIOC  isAON  isBuy
 * 
 * caution: stop-loss/take-profit/trailing-stop are still in progress, the behavior of using these orders are undefined
 */
ORDER_SEL = 0
ORDER_BUY = 1
ORDER_AON = 2
ORDER_IOC = 4
ORDER_SL  = 8
ORDER_TP  = 16
ORDER_TS  = 32

/*
 * e.g. a stoploss aon ioc bid order, condition = ORDER_BUY + ORDER_SL + ORDER_AON + ORDER_IOC
 */


/*
 * @brief setupGlobalConfig must be called before createExchange, it configures the following properties shared with all exchanges
 * @param inputScheme, the scheme of the orders, use the following scheme for now
 * @param mapping, leave it null is ok for now
 * @param pricePrecision, quantity/price are uint64, the cost keeps up to pricePrecision digits after the decimal point
 * @param bookDepth, the max depth of the depth order book
 * @return "Successful" if succeeded to setup, otherwise throw exceptions
 */
inputScheme = table(1:0, `op`symbol`id`quantity`condition`price`thresholdPrice`expiredTime, [INT,SYMBOL,LONG,LONG,INT,LONG,LONG,LONG])
pricePrecision = 0
bookDepth = 10
MatchingEngine::setupGlobalConfig(inputScheme, , pricePrecision, bookDepth)

/*
 * @brief createExchange create an matching engine for matching orders
 * @param symbol, the symbol of the input orders
 * @param outputTable, all events are output to this table, currently the schema is fixed(see the schema of the following output)
 * @return an object can be used as an handler in subscribeTable or a table object in insert
 */
sym = 'AAPL'
output = table(10000:0,`symbol`id`status`condition`quantity`filledQuantity`cost, [SYMBOL,LONG,STRING,INT,LONG,LONG,DOUBLE])
depthOutput = table(10000:0,`is_sell`level`price`aggregate_qty`order_count`market_price,[BOOL,INT,LONG,LONG,LONG,LONG])
exchange = MatchingEngine::createExchange(sym, output,depthOutput)

/*
 * insert into the object returned by createExchange to see the output
 */
insert into exchange values(ORDER_ADD,`AAPL,0,100,ORDER_BUY,100,0,0)
insert into exchange values(ORDER_ADD,`AAPL,1,100,ORDER_SEL,0,0,0)
output

/*
 * use the object returned by createExchange as the handler of the subscribeTable
 */
share streamTable(10000:0, `op`symbol`id`quantity`condition`price`thresholdPrice`expiredTime, [INT,SYMBOL,LONG,LONG,INT,LONG,LONG,LONG]) as publishOrders
subscribeTable(,`publishOrders,`matching,-1,append!{exchange},true)
insert into publishOrders values(ORDER_ADD,`AAPL,2,100,ORDER_BUY,100,0,0)
insert into publishOrders values(ORDER_CAN,`AAPL,2,100,ORDER_BUY,100,0,0)
insert into publishOrders values(ORDER_CAN,`AAPL,2,100,ORDER_BUY,100,0,0)   // cancel an cancelled order is ignored
insert into publishOrders values(ORDER_ADD,`AAPL,3,100,ORDER_BUY,100,0,0)
insert into publishOrders values(ORDER_ADD,`AAPL,4,100,ORDER_SEL,110,0,0)
insert into publishOrders values(ORDER_MOD,`AAPL,3,80,ORDER_BUY,0,0,0)      // modify the order and trigger a trade
output

/*
 * check the status of the exchange
 * currently only provides name, user, status, lastErrMsg, numRows
 */
getAggregatorStat()
```