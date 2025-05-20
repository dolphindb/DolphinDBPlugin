#ifndef PUBLISHER_H_
#define PUBLISHER_H_

#include <string>

#include "DolphinDBEverything.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "TableImp.h"
#pragma GCC diagnostic pop

using std::string;

namespace ddb {

class PublishTable : public BasicTable {
  public:
    PublishTable(const vector<ConstantSP> &cols, const vector<string> &colNames, const ConstantSP &resource,
                 const string &topic, Heap *heap);

    virtual bool append(vector<ConstantSP> &values, INDEX &insertedRows, string &errMsg);

  private:
    SessionSP session_;
    ConstantSP resource_;
    string topic_;
    vector<ConstantSP> cols_;
    vector<string> colNames_;
};

}  // namespace ddb

#endif
