#include "publisher.h"

#include "ScalarImp.h"
#include "client.h"

PublishTable::PublishTable(const vector<ConstantSP> &cols, const vector<string> &colNames, const ConstantSP &resource,
                           const string &topic, Heap *heap)
    : BasicTable(cols, colNames), resource_(resource), topic_(topic), cols_(cols), colNames_(colNames) {
    session_ = heap->currentSession()->copy();
}

bool PublishTable::append(vector<ConstantSP> &values, INDEX &insertedRows, string &errMsg) {
    TableSP table;
    int numCols = colNames_.size();
    if (values.size() == 1 && values[0]->isTable()) {
        table = values[0];

        if (table->columns() != numCols) {
            throw RuntimeException("Invalid type of table to append.");
        }
    } else {
        table = Util::createTable(colNames_, values);
    }

    for (int i = 0; i < numCols; ++i) {
        if (table->getColumnType(i) != cols_[i]->getType()) {
            throw RuntimeException("Invalid type of values to append.");
        }
    }

    vector<ConstantSP> args = {resource_, new String(topic_), table};
    ConstantSP ret = mqttClientPub(session_->getHeap().get(), args);
    if (ret->getType() != DT_INT || ret->getInt() != MQTT_OK) {
        throw RuntimeException("MQTT publish failed.");
    }

    insertedRows = table->size();
    return true;
}
