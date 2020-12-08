//
// Created by ypfan on 2020/11/20.
//

#include "client.h"

#include <utility>
#include <iostream>

void AppendTable::run() {

    auto consumer = (Consumer*)(consumer_->getLong());
    Message msg;
    vector<ConstantSP> parser_string;
    parser_string.emplace_back(Util::createConstant(DT_STRING));

    while(true){
        msg = consumer->poll(std::chrono::milliseconds(timeout_));
        if(msg){
            parser_string[0]->setString(string(msg.get_payload()));
            try {
                auto parser_result = parser_->call(session_->getHeap().get(), parser_string);
                if(!parser_result->isTable()) {
                    cerr << "The parser should return a table." << endl;
                    return;
                }

                TableSP table_insert = parser_result;
                if(handle_->isTable()) {
                    TableSP result = handle_;
                    int length = result->columns();
                    if (table_insert->columns() < length) {
                        cerr << "The columns of the table returned is smaller than the handler table." << endl;
                        return;
                    }
                    if (table_insert->columns() > length)
                        cerr
                                << "The columns of the table returned is larger than the handler table, and the information may be ignored."
                                << endl;

                    if (result->isSegmentedTable()) {
                        vector<ConstantSP> args = {result, table_insert};
                        heap_->currentSession()->getFunctionDef("append!")->call(heap_, args);
                    } else {
                        INDEX insertedRows = table_insert->size();
                        string errMsg;
                        vector<ConstantSP> args = {table_insert};
                        bool add = result->append(args, insertedRows, errMsg);
                        if (!add) {
                            cerr << errMsg << endl;
                        }
                    }
                }
                else{
                    vector<ConstantSP> args = {table_insert};
                    ((FunctionDefSP)handle_)->call(heap_, args);
                }
            }
            catch (TraceableException &exception) {
                cerr << exception.what() << endl;
                continue;
            }
            catch(Exception &exception){
                cerr << exception.what() << endl;
                continue;
            }
        }
    }
}

SubConnection::SubConnection() {
    connected_ = false;
}

SubConnection::~SubConnection() {
    if (connected_) {
        connected_ = false;
    }
    std::cout<<"client: " << description_ << " is freed"<<std::endl;
}


SubConnection::SubConnection(Heap *heap, std::string description, const FunctionDefSP& parser, ConstantSP handle, ConstantSP consumer, int timeout)
        :description_(std::move(description)),heap_(heap) {
    connected_ = true;
    createTime_=Util::getEpochTime();
    session_ = heap->currentSession()->copy();
    session_->setUser(heap->currentSession()->getUser());

    SmartPointer<AppendTable> append = new AppendTable(heap, parser, this, handle, consumer, timeout);
    thread_ = new Thread(append);
    if (!thread_->isStarted()) {
        thread_->start();
    }
}