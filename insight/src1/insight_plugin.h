#ifndef INSIGHT_PLUGIN_H
#define INSIGHT_PLUGIN_H

#include "CoreConcept.h"
#include "Util.h"
#include <vector>

extern "C" ConstantSP connectInsight(Heap *heap, std::vector<ConstantSP> &arguments);
extern "C" ConstantSP subscribe(Heap *heap, std::vector<ConstantSP> &arguments);
extern "C" ConstantSP unsubscribe(Heap *heap, std::vector<ConstantSP> &arguments);
extern "C" ConstantSP closeInsight(Heap *heap, std::vector<ConstantSP> &arguments);
extern "C" ConstantSP getSchema(Heap *heap, std::vector<ConstantSP> &arguments);
extern "C" ConstantSP getStatus(Heap *heap, std::vector<ConstantSP> &arguments);

static string PLUGIN_INSIGHT_PREFIX = "[PLUGIN::INSIGHT]: ";

class InsightStockTableMeta {
public:
    static std::vector<string> COLNAMES;
    static std::vector<DATA_TYPE> COLTYPES; 
};

class InsightIndexTableMeta {
public:
    static std::vector<string> COLNAMES;
    static std::vector<DATA_TYPE> COLTYPES;
};

class InsightFuturesTableMeta {
public:
    static std::vector<string> COLNAMES;
    static std::vector<DATA_TYPE> COLTYPES; 
};

class InsightTransactionTableMeta {
public:
    static std::vector<string> COLNAMES;
    static std::vector<DATA_TYPE> COLTYPES; 
};

class InsightOrderTableMeta {
public:
    static std::vector<string> COLNAMES;
    static std::vector<DATA_TYPE> COLTYPES;
};
#endif
