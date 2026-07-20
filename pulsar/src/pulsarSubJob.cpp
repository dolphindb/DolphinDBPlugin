//
// Created by htxu on 10/23/2023.
//

#include <Util.h>
#include <ddbplugin/PluginLogger.h>
#include <ScalarImp.h>

#include "pulsarSubJob.h"
#include "pulsarUtil.h"

/// PulsarSubJob

PulsarSubJob::PulsarSubJob(Heap *heap, const ConstantSP &client, const SmartPointer<pulsar::Consumer> &consumer,
                           const pulsar::ConsumerConfiguration& consumerConfig) :
consumerConfig_(consumerConfig), // copy the session as SessionSP in case session in the outer scope is destructed
session_(heap->currentSession()->copy()),
createTime_(Util::getEpochTime()),
client_(client),
consumer_(consumer) {

    session_->setUser(heap->currentSession()->getUser());
}

PulsarSubJob::~PulsarSubJob() = default;

void PulsarSubJob::stop() {
    // if already stopped
    if (endTime_ != LONG_LONG_MIN) {
        throw RuntimeException(PULSAR_PREFIX + "the job is already stopped");
    }

    // else
    endTime_ = Util::getEpochTime();
    try {
        consumer_->unsubscribe();
    } catch (exception &e) {
        throw RuntimeException(PULSAR_PREFIX + e.what());
    }
    client_.clear(); // counter--
}

long long PulsarSubJob::getCreateTime() const {
    return createTime_;
}

long long PulsarSubJob::getEndTime() const {
    return endTime_;
}

SessionSP PulsarSubJob::getSession() {
    return session_;
}

// Listener
void PulsarSubJob::listener(const SessionSP &session, const TableSP &table, const FunctionDefSP &parser,
                            pulsar::Consumer &consumer, const pulsar::Message &msg) {
    // avoid throw in the async thread
    try {
        /// init message vector to save message and to be used in parser
        vector<ConstantSP> msgVec;
        msgVec.emplace_back(new String(msg.getDataAsString()));
        if (parser->getParamCount() > 1) {
            msgVec.emplace_back(new String(msg.getPartitionKey()));
        }
        if (parser->getParamCount() > 2) {
            msgVec.emplace_back(new String(msg.getTopicName()));
        }

        /// get parsed message
        auto parsedMsg = parser->call(session->getHeap().get(), msgVec);
        if (!parsedMsg->isTable()) {
            LOG_ERR(PULSAR_PREFIX + "The parser should return a table.");
            acknowledge(consumer, msg);
            return;
        }

        // check number of columns
        auto tableSize = table->columns();
        auto msgSize = parsedMsg->columns();
        if (tableSize != msgSize) {
            LOG_ERR(PULSAR_PREFIX + "Column numbers of table and parsed message are different.");
            acknowledge(consumer, msg);
            return;
        }

        /// append to table
        if (table->isSegmentedTable()) {
            vector<ConstantSP> args = {table, parsedMsg};
            session->getFunctionDef("append!")->call(session->getHeap().get(), args);
        } else {
            vector<ConstantSP> args = {parsedMsg};
            auto insertedRows = parsedMsg->size();
            string errMsg;

            LockGuard<Mutex> _(table->getLock());
            auto result = table->append(args, insertedRows, errMsg);
            if (!result) {
                LOG_ERR(PULSAR_PREFIX + errMsg);
            }
        }

        acknowledge(consumer, msg);

    } catch (exception &e) {
        LOG_ERR(PULSAR_PREFIX + e.what());
    }
}

void PulsarSubJob::acknowledge(pulsar::Consumer &consumer, const pulsar::Message& msg) {
    try {
        auto result = consumer.acknowledge(msg);
        if (result != pulsar::ResultOk) {
            throw RuntimeException(string("Failed to acknowledge: ") + pulsar::strResult(result));
        }
    } catch (exception &e) {
        LOG_ERR(PULSAR_PREFIX + e.what());
    }
}
