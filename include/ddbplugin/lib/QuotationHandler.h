#pragma once
#include "CoreConcept.h"

namespace ddb {

class QuotationHandler {
  public:
    QuotationHandler();
    void setSchema(Heap *heap, TableSP schema, int keycolumnIndex = 0, int sourceTypeIndex = 3, int orderTypeIndex = 4);
    IO_ERR appendCombineQuotation(const string &quotation);
    IO_ERR appendSnapshotAndBar(const ConstantSP &dict);
    ConstantSP getTable();
    ConstantSP getSnapshotBarTableWithIndicator(const TableSP &indicator);
    ConstantSP getTickTableWithIndicator(const TableSP &indicator);
    ConstantSP getDict();
    ConstantSP getTickDict();
    ConstantSP getSnapshotBarDictWithIndicator(const TableSP &indicator);
    ConstantSP getTickDictWithIndicator(const TableSP &indicator);
    ConstantSP getEntrustTradeDictWithIndicator(const TableSP &entrustTb, const TableSP &tradeTb);
    void clear();
    inline int getSourceTypeIndex() const { return sourceTypeIndex_; }

  protected:
    vector<ConstantSP> cols_;
    vector<string> columnNames_;
    vector<DATA_TYPE> dt_;
    INDEX indexStart_;
    int keycolumnIndex_;
    int sourceTypeIndex_;
    int orderTypeIndex_;
};

}  // namespace ddb
