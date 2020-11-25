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
                table_insert = parser_->call(session_->getHeap().get(), parser_string);
            }
            catch (RuntimeException &exception) {
                cerr << exception.what() << endl;
                continue;
            }
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