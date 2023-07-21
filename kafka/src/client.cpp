//
// Created by ypfan on 2020/11/20.
//

#include "client.h"
#include "Types.h"
#include "plugin_kafka.h"

// #include "CoderBase.h"
#include <cmath>
#include <exception>
#include <mutex>
#include <utility>
#include <iostream>

void AppendTable::run() {
    try {
    Consumer* consumer = nullptr;
    Message msg;
    bool commitFlag;
    vector<ConstantSP> parserString;
    parserString.emplace_back(Util::createConstant(DT_STRING));
    if(parser_->getType() == DT_FUNCTIONDEF)  {
        if(((FunctionDefSP)parser_)->getParamCount() == 2) {
            parserString.emplace_back(Util::createConstant(DT_STRING));
        } else if (((FunctionDefSP)parser_)->getParamCount() == 3) {
            parserString.emplace_back(Util::createConstant(DT_STRING));
            parserString.emplace_back(Util::createConstant(DT_STRING));
        }
    }
    long long consumerValue = consumerWrapper_->getLong();
    if(consumerValue != 0) {
        // consumer = (Consumer*)(consumerValue);  // verified in createSubJob()
        RWLockGuard<RWLock> lockGuard(NULL, false);
        getWrapperLockGuard<Consumer>(consumerWrapper_, lockGuard);
        consumer = getConnection<Consumer>(consumerWrapper_);
        map<string, string> configuration = consumer->get_configuration().get_all();
        commitFlag = false;
        auto it = configuration.find("enable.auto.commit");
        if(it != configuration.end() && it->second == "false"){
            commitFlag = true;
            LOG_INFO("[PluginKafka]: Actively commit kafka message");
        }
    } else {
        LOG_ERR(string("[PLUGIN::KAFKA] ") + "consumer has already been destructed.");
        return;
    }

    while(LIKELY(flag_&& consumer)){
        if(consumerWrapper_->getLong() == 0) {
            LOG_ERR(string("[PLUGIN::KAFKA] ") + "consumer has already been destructed.");
            return;
        }
        try {
            msg = consumer->poll(std::chrono::milliseconds(timeout_));
        } catch(std::exception& e) {
            LOG_ERR(string("[PLUGIN::KAFKA] ") + e.what());
        }
        if(msg && msg.get_error() && msg.is_eof()) {
            continue;
        }
        if(msg){
            try {
                parserString[0]->setString(string(msg.get_payload()));
                if(parser_->getType() == DT_FUNCTIONDEF) {
                    ConstantSP parserResult;
                    if(((FunctionDefSP)parser_)->getParamCount() == 1) {
                        parserResult = ((FunctionDefSP)parser_)->call(session_->getHeap().get(), parserString);
                        if(!parserResult->isTable()) {
                            LOG_ERR(string("[PLUGIN::KAFKA] ") + "The parser should return a table.");
                            continue;
                        }
                    } else if(((FunctionDefSP)parser_)->getParamCount() == 2) {
                        parserString[1]->setString(string(msg.get_key()));
                        parserResult = ((FunctionDefSP)parser_)->call(session_->getHeap().get(), parserString);
                        if(!parserResult->isTable()) {
                            LOG_ERR(string("[PLUGIN::KAFKA] ") + "The parser should return a table.");
                            continue;
                        }
                    } else {
                        parserString[1]->setString(string(msg.get_key()));
                        parserString[2]->setString(string(msg.get_topic()));
                        parserResult = ((FunctionDefSP)parser_)->call(session_->getHeap().get(), parserString);
                        if(!parserResult->isTable()) {
                            LOG_ERR(string("[PLUGIN::KAFKA] ") + "The parser should return a table.");
                            continue;
                        }
                    }

                    TableSP tableInsert = parserResult;
                    if(handle_->isTable()) {
                        TableSP result = handle_;
                        int length = result->columns();
                        if (tableInsert->columns() < length) {
                            LOG_ERR(string("[PLUGIN::KAFKA] ") + "The columns of the table returned is smaller than the handler table.");
                            continue;
                        }
                        if (tableInsert->columns() > length)
                            LOG_WARN(string("[PLUGIN::KAFKA] ") + "The columns of the table returned is larger than the handler table, and the information may be ignored.");

                        if (result->isSegmentedTable()) {
                            vector<ConstantSP> args = {result, tableInsert};
                            session_->getFunctionDef("append!")->call(session_->getHeap().get(), args);
                        } else {
                            INDEX insertedRows = tableInsert->size();
                            string errMsg;
                            vector<ConstantSP> args = {tableInsert};
                            LockGuard<Mutex> _(result->getLock());
                            bool add = result->append(args, insertedRows, errMsg);
                            if (!add) {
                                LOG_ERR(string("[PLUGIN::KAFKA] ") + errMsg);
                            }
                        }
                    }
                    else{
                        vector<ConstantSP> args = {tableInsert};
						if (handle_->getType() == DT_FUNCTIONDEF)
                            ((FunctionDefSP)handle_)->call(session_->getHeap().get(), args);
						else
							LOG_ERR(string("[PLUGIN::KAFKA] ") + "Handle is not a function define.");
                    }
                } else if(parser_->getForm() == DF_TABLE) {
                    vector<ConstantSP> tableArgs = {parserString[0]};
                    ConstantSP content = Util::createTable({"string"}, tableArgs);
                    string name = "parseAndHandle";
                    vector<ConstantSP> args = {content};
                    parser_->callMethod(name, session_->getHeap().get(), args);
                } else {
                    LOG_ERR("[PLUGIN::KAFKA] Invalid parser parameter.");
                }
            }
            catch(std::exception &exception){
                LOG_ERR(string("[PLUGIN::KAFKA] "), exception.what());
                if(string(exception.what()).find("<DataNodeNotAvail>Server will shut down") != string::npos)
                    continue;
            }
            try{
                try {
                    if(commitFlag)
                        consumer->commit(msg);
                } catch(std::exception& e) {
                    throw RuntimeException(string("[PLUGIN::KAFKA] ") + e.what());
                }
            }
            catch (exception& e){
                LOG_ERR(string("[PLUGIN::KAFKA] : Failed to commit message: "), e.what());
            }
        }
    }

    }catch(exception& ex) {
        LOG_ERR(string("[PLUGIN::KAFKA] : "), ex.what());
    } catch(...) {
        LOG_ERR(string("[PLUGIN::KAFKA] : Failed to commit message: "));
    }
}

SubConnection::~SubConnection() {
    if(thread_->isRunning()) {
        cancelThread();
    }
    if (connected_) {
        connected_ = false;
    }
    LOG_INFO("client: " + description_ + " is freed");
}


SubConnection::SubConnection(Heap *heap, const string& description, const ConstantSP& parser, const ConstantSP& handle, const ConstantSP& consumer, int timeout)
    :description_(std::move(description)),heap_(heap), consumerWrapper_(consumer) {
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
