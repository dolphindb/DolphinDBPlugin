//
// Created by jccai on 19-5-24.
//

#ifndef MATCHINGENGINE_CONFIG_H
#define MATCHINGENGINE_CONFIG_H

#include <CoreConcept.h>
#include <Util.h>
#include <string>

struct Config {
    Config() = default;

    bool validate() {
        if (inputScheme_.isNull()) {
            throw IllegalArgumentException("Config::validate", "missing inputScheme");
        }
        if (mapping_.empty()) {
            throw IllegalArgumentException("Config::validate", "missing fields");
        }
        if (pricePrecision_ < 0) {
            throw IllegalArgumentException("Config::validate", "invalid pricePrecision");
        }
        if (bookDepth_ < 0) {
            throw IllegalArgumentException("Config::validate", "invalid bookDepth");
        }
        initialized = true;
        return true;
    }

    bool initialized = false;
    TableSP inputScheme_;
    vector<string> inputColumnNames_;
    vector<int> mapping_;
    uint32_t pricePrecision_;
    uint32_t bookDepth_;
};

using ConfigPtr = std::shared_ptr<Config>;

inline TableSP createTableFromModel(const TableSP &model) {
    vector<ConstantSP> columns;
    vector<string> columnNames;
    for (INDEX i = 0; i < model->columns(); ++i) {
        columns.push_back(Util::createVector(model->getColumnType(i), 0, Util::BUF_SIZE));
        columnNames.push_back(model->getColumnName(i));
    }
    return Util::createTable(columnNames, columns);
}

namespace MappingUtils {

enum Fields { op, symbol, id, quantity, condition, price, thresholdPrice, expiredTime };

static const vector<string> fieldsNames = {
    "op", "symbol", "id", "quantity", "condition", "price", "thresholdPrice", "expiredTime",
};

inline const string &fieldsName(Fields ordinal) {
    return fieldsNames[ordinal];
}

}    // namespace MappingUtils

#endif    // MATCHINGENGINE_CONFIG_H
