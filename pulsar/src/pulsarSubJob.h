//
// Created by htxu on 10/23/2023.
//

#ifndef PLUGINPULSAR_PULSARSUBJOB_H
#define PLUGINPULSAR_PULSARSUBJOB_H


#include <SmartPointer.h>
#include <CoreConcept.h>

#include "pulsar/Client.h"

class PulsarSubJob {
public:
    PulsarSubJob(Heap *heap, const ConstantSP &client, const SmartPointer<pulsar::Consumer> &consumer,
                 const pulsar::ConsumerConfiguration& consumerConfig);
    ~PulsarSubJob();

    void stop();

    long long getCreateTime() const;
    long long getEndTime() const;
    SessionSP getSession();

    static void listener(const SessionSP &session, const TableSP &table, const FunctionDefSP &parser,
                         pulsar::Consumer &consumer, const pulsar::Message &msg);
    static void acknowledge(pulsar::Consumer &consumer, const pulsar::Message& msg);

    pulsar::ConsumerConfiguration consumerConfig_; // TODO: to be refactored

private:
    SessionSP session_;

    long long createTime_;
    long long endTime_ = LONG_LONG_MIN;

    // add count to SPs to run in the background
    ConstantSP client_;
    const SmartPointer<pulsar::Consumer> consumer_;
};


#endif //PLUGINPULSAR_PULSARSUBJOB_H
