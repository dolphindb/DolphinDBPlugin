//
// Created by jccai on 19-5-24.
//

#include "Exchange.h"

using MappingUtils::Fields;

Config Exchange::config_;

Exchange::Exchange(std::string &&symbol, std::string &&creator, TableSP outputTable, TableSP depthOutputTable) : AbstractStreamEngine(__func__, symbol, creator), symbol_(symbol), outputTable_(outputTable), depthOutputTable_(depthOutputTable) {
    if (!config_.initialized) {
        throw RuntimeException("global config is not initialized");
    }
    if (config_.bookDepth_) {
        auto book = std::make_shared<DepthOrderBook>(symbol_, config_.bookDepth_);
        book->set_bbo_listener(this);
        book->set_depth_listener(this);
        book_ = book;
        depthOutput_.resize(6);
        // type level price aggregate_qty order_count market_price
        depthOutput_[0] = Util::createVector(DT_BOOL, config_.bookDepth_ * 2);
        depthOutput_[1] = Util::createVector(DT_INT, config_.bookDepth_ * 2);
        depthOutput_[2] = Util::createVector(DT_LONG, config_.bookDepth_ * 2);
        depthOutput_[3] = Util::createVector(DT_LONG, config_.bookDepth_ * 2);
        depthOutput_[4] = Util::createVector(DT_LONG, config_.bookDepth_ * 2);
        depthOutput_[5] = Util::createVector(DT_LONG, config_.bookDepth_ * 2);
        for (unsigned int i = 0; i < config_.bookDepth_ * 2; ++i) {
            depthOutput_[0]->setBool(i, i < config_.bookDepth_);
            depthOutput_[1]->setInt(i, i % config_.bookDepth_);
        }
    } else {
        book_ = std::make_shared<OrderBook>(symbol_);
    }
    book_->set_order_listener(this);
    book_->set_trade_listener(this);
    book_->set_order_book_listener(this);
    bufferedOutputTable_ = createTableFromModel(outputTable_);
    for (INDEX i = 0; i < outputTable_->columns(); ++i) {
        bufferedOutput_.push_back(Util::createVector(outputTable_->getColumnType(i), 1));
    }
}

bool Exchange::append(vector<ConstantSP> &values, INDEX &insertedRows, string &errMsg) {
    vector<ConstantSP> val;
    if (values[0]->isTable()) {
        for (INDEX i = 0; i < values[0]->columns(); ++i) {
            val.emplace_back(values[0]->getColumn(i));
        }
    } else {
        val = values;
    }
    LockGuard<Mutex> guard(&mutex_);
    auto rows = val[0]->size();
    for (INDEX i = 0; i < rows; ++i) {
        auto op = static_cast<OrderOperation>(val[config_.mapping_[Fields::op]]->getInt(i));
        auto symbol = static_cast<string>(val[config_.mapping_[Fields::symbol]]->getString(i));
        auto id = static_cast<uint64_t>(val[config_.mapping_[Fields::id]]->getLong(i));
        auto quantity = static_cast<uint64_t>(val[config_.mapping_[Fields::quantity]]->getLong(i));
        auto condition = static_cast<uint32_t>(val[config_.mapping_[Fields::condition]]->getInt(i));
        auto price = static_cast<uint64_t>(val[config_.mapping_[Fields::price]]->getLong(i));
        auto thresholdPrice = static_cast<uint64_t>(val[config_.mapping_[Fields::thresholdPrice]]->getLong(i));
        auto expiredTime = static_cast<uint64_t>(val[config_.mapping_[Fields::expiredTime]]->getLong(i));

        auto order = std::make_shared<book::Order>(std::move(symbol), id, quantity, condition, price, thresholdPrice, expiredTime);
        switch (op) {
            case book::ADD:
                book_->add(order);
                break;
            case book::MODIFY:
                book_->replace(order);
                break;
            case book::CANCEL:
                book_->cancel(id);
                break;
            default:
                errMsg = "unknown order operation";
        }
    }
    outputToOutput();
    cumMessages_ += rows;
    insertedRows = rows;
    return true;
}

TableSP Exchange::generateEngineStatTable() {
    static vector<string> colNames{"name", "user", "status", "lastErrMsg", "numRows"};
    vector<ConstantSP> cols;
    cols.push_back(Util::createVector(DT_STRING, 0));
    cols.push_back(Util::createVector(DT_STRING, 0));
    cols.push_back(Util::createVector(DT_STRING, 0));
    cols.push_back(Util::createVector(DT_STRING, 0));
    cols.push_back(Util::createVector(DT_LONG, 0));
    return Util::createTable(colNames, cols);
}

void Exchange::initEngineStat() {
    engineStat_.push_back(new String(engineName_));
    engineStat_.push_back(new String(engineUser_));
    engineStat_.push_back(new String(status_));
    engineStat_.push_back(new String(lastErrMsg_));
    engineStat_.push_back(new Long(cumMessages_));
}

void Exchange::updateEngineStat() {
    LockGuard<Mutex> guard(&mutex_);
    engineStat_[2]->setString(status_);
    engineStat_[3]->setString(lastErrMsg_);
    engineStat_[4]->setLong(cumMessages_);
}

ConstantSP Exchange::setupGlobalConfig(Heap *heap, vector<ConstantSP> &args) {
    static string funcName = "setupGlobalConfig";
    static string syntax = "Usage: " + funcName + "(inputScheme, mapping, pricePrecision, bookDepth).";

    // check inputScheme
    if (!args[0]->isTable()) {
        throw IllegalArgumentException("setupGlobalConfig", "inputScheme must be a table");
    }
    TableSP inputScheme = args[0];

    // check mapping
    DictionarySP fieldMap;
    if (!args[1]->isNothing() && !args[1]->isDictionary()) {
        throw IllegalArgumentException("setupGlobalConfig", "mapping must be a dictionary");
    } else if (args[1]->isNothing()) {
        fieldMap = nullptr;
    } else {
        fieldMap = args[1];
    }

    // check pricePrecision
    uint32_t pricePrecision = 3;
    if (args.size() >= 3) {
        if (args[2]->isNull() || args[2]->getCategory() != INTEGRAL) {
            throw IllegalArgumentException("setupGlobalConfig", "pricePrecision must be a non-negative integer");
        }
        pricePrecision = args[2]->getLong();
    }

    // check bookDepth
    uint32_t bookDepth = 10;
    if (args.size() >= 4) {
        if (args[3]->isNull() || args[3]->getCategory() != INTEGRAL) {
            throw IllegalArgumentException("setupGlobalConfig", "bookDepth must be a non-negative integer");
        }
        bookDepth = args[3]->getLong();
    }

    Config config;
    config.inputScheme_ = inputScheme;
    if (fieldMap.isNull()) {
        for (auto &field : MappingUtils::fieldsNames) {
            config.mapping_.emplace_back(inputScheme->getColumnIndex(field));
        }
    } else {
        for (auto &field : MappingUtils::fieldsNames) {
            config.mapping_.emplace_back(inputScheme->getColumnIndex(fieldMap->getMember(field)->getString()));
        }
    }

    config.pricePrecision_ = std::pow(10, pricePrecision);
    config.bookDepth_ = bookDepth;
    for (INDEX i = 0; i < inputScheme->columns(); ++i) {
        config.inputColumnNames_.emplace_back(inputScheme->getColumnName(i));
    }
    config.validate();
    Exchange::config_ = config;
    return new String("Successful");
}

ConstantSP Exchange::createExchange(Heap *heap, vector<ConstantSP> &args) {
    static string funcName = "createExchange";
    static string syntax = "Usage: " + funcName + "(symbol, outputTable, depthOutputTable).";

    // check symbol
    if (args[0]->getType() != DT_STRING) {
        throw IllegalArgumentException("createExchange", "symbol must be a string or symbol");
    }

    // check duplicate
    string symbol = args[0]->getString();
    if (!StreamEngineManager::instance().find(symbol).isNull()) {
        throw IllegalArgumentException("createExchange", "Exchange for symbol " + symbol + " already exists");
    }

    // check outputTable
    if (!args[1]->isTable()) {
        throw IllegalArgumentException("createExchange", "outputTable must be a table");
    }

    if(!args[2]->isTable()) {
        throw IllegalArgumentException("createExchange", "depthOutTable must be a table");
    }

    AbstractStreamEngineSP exchange = new Exchange(std::move(symbol), heap->currentSession()->getUser()->getUserId(), args[1], args[2]);
    exchange->initEngineStat();
    StreamEngineManager::instance().insert(exchange);
    return exchange;
}

ConstantSP setupGlobalConfig(Heap *heap, vector<ConstantSP> &args) {
    return Exchange::setupGlobalConfig(heap, args);
}

ConstantSP createExchange(Heap *heap, vector<ConstantSP> &args) {
    return Exchange::createExchange(heap, args);
}
