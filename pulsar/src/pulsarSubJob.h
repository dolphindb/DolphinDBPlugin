#pragma once

#include "DolphinDBEverything.h"
#include <SmartPointer.h>
#include <CoreConcept.h>

#if defined(_MSC_VER)
#pragma warning( push )
#elif defined(__clang__)
#pragma clang diagnostic push
#else // gcc
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#include "pulsar/Client.h"

#if defined(_MSC_VER)
#pragma warning( pop )
#elif defined(__clang__)
#pragma clang diagnostic pop
#else // gcc
#pragma GCC diagnostic pop
#endif

namespace ddb {

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

} // namespace ddb
