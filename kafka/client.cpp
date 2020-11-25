//
// Created by ypfan on 2020/11/20.
//

#include "client.h"

#include <utility>
#include <iostream>

void AppendTable::run() {

    auto consumer = (Consumer*)(consumer_->getLong());
    Message msg;
    TableSP result = handle_;
    int length = result->columns();
    vector<ConstantSP> parser_string;
    parser_string.emplace_back(Util::createConstant(DT_STRING));

    while(true){
        msg = consumer->poll(std::chrono::milliseconds(timeout_));
        if(msg){
            parser_string[0]->setString(string(msg.get_payload()));
            TableSP table_insert;
            try {
                auto parser_result = parser_->call(session_->getHeap().get(), parser_string);
                if(!parser_result->isTable()) {
                    cerr << "The parser should return a table." << endl;
                    return;
                }
                table_insert = parser_result;
                if(table_insert->columns()<length) {
                    cerr << "The columns of the table returned is smaller than the handler table." << endl;
                    return;
                }
                if(table_insert->columns()>length)
                    cerr << "The columns of the table returned is larger than the handler table, and the information may be ignored." << endl;
                vector<ConstantSP> dataToAppend;
                for(int i = 0;i<length;i++)
                    dataToAppend.emplace_back(table_insert->getColumn(i));
                INDEX insertedRowd;
                string errMsg;
                bool success = result->append(dataToAppend,insertedRowd,errMsg);
                if(!success) {
                    cerr << errMsg << endl;
                    return;
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