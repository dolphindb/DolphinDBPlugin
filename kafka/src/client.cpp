//
// Created by ypfan on 2020/11/20.
//

#include "client.h"

// #include "CoderBase.h"
#include <mutex>
#include <utility>
#include <iostream>

void AppendTable::run() {

    auto consumer = (Consumer*)(consumer_->getLong());
    Message msg;
    vector<ConstantSP> parser_string;
    parser_string.emplace_back(Util::createConstant(DT_STRING));
    map<string, string> configuration = consumer->get_configuration().get_all();
    bool commitFlag = false;
    auto it = configuration.find("enable.auto.commit");
    if(it != configuration.end() && it->second == "false"){
        commitFlag = true;
        LOG_INFO("[PluginKafka]: Actively commitkafka message");
    }


    while(flag_){
        msg = consumer->poll(std::chrono::milliseconds(timeout_));
        if(msg && msg.get_error() && msg.is_eof()) {
            continue;
        }
        if(msg){
            parser_string[0]->setString(string(msg.get_payload()));
            try {
                if(parser_->getType() == DT_FUNCTIONDEF) {
                    auto parser_result = ((FunctionDefSP)parser_)->call(session_->getHeap().get(), parser_string);
                    if(!parser_result->isTable()) {
                        LOG_ERR("The parser should return a table.");
                        continue;
                        //return;
                    }

                    TableSP table_insert = parser_result;
                    if(handle_->isTable()) {
                        TableSP result = handle_;
                        int length = result->columns();
                        if (table_insert->columns() < length) {
                            LOG_ERR("The columns of the table returned is smaller than the handler table.");
                            continue;
                            //return;
                        }
                        if (table_insert->columns() > length)
                            LOG_WARN("The columns of the table returned is larger than the handler table, and the information may be ignored.");

                        if (result->isSegmentedTable()) {
                            vector<ConstantSP> args = {result, table_insert};
                            session_->getFunctionDef("append!")->call(session_->getHeap().get(), args);
                        } else {
                            INDEX insertedRows = table_insert->size();
                            string errMsg;
                            vector<ConstantSP> args = {table_insert};
                            LockGuard<Mutex> _(result->getLock());
                            bool add = result->append(args, insertedRows, errMsg);
                            if (!add) {
                                LOG_ERR(errMsg);
                            }
                        }
                    }
                    else{
                        vector<ConstantSP> args = {table_insert};
						if (handle_->getType() == DT_FUNCTIONDEF)
                        	((FunctionDefSP)handle_)->call(session_->getHeap().get(), args);
						else
							LOG_ERR("Handle is not a function define.");
                    }
                } else if(parser_->getType() == DT_RESOURCE) {
                    ConstantSP content = Util::createTable({"string"}, parser_string);
                    // vector<ConstantSP> args = {parser_, {content}};
                    string name = "append";
                    vector<ConstantSP> args = {content};
                    parser_->callMethod(name, session_->getHeap().get(), args);
                    
                    // (TableSP)(parser_->getLong())
                    // heap_->currentSession()->getFunctionDef("append")->call(heap_, args);
                }
            }
            catch(std::exception &exception){
                LOG_ERR("[PluginKafka]:", exception.what());
                if(string(exception.what()).find("<DataNodeNotAvail>Server will shut down") != string::npos)
                    continue;
            }
            try{
                if(commitFlag)
                    consumer->commit(msg);
            }
            catch (exception& e){
                LOG_ERR("[PluginKafka]: Failed to commit message: ", e.what());
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
    LOG_INFO("client: " + description_ + " is freed");
}


SubConnection::SubConnection(Heap *heap, std::string description, const ConstantSP& parser, const ConstantSP& handle, const ConstantSP& consumer, int timeout)
    :description_(std::move(description)),heap_(heap) {
    connected_ = true;
    createTime_=Util::getEpochTime();
    session_ = heap->currentSession()->copy();
    session_->setUser(heap->currentSession()->getUser());

    SmartPointer<AppendTable> append = new AppendTable(heap, parser, this, handle, consumer, timeout);
    append_ = append;
    thread_ = new Thread(append);
    if (!thread_->isStarted()) {
        thread_->start();
    }
}
