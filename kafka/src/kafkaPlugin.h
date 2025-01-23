#ifndef KAFKA_PLUGIN_KAFKA_H
#define KAFKA_PLUGIN_KAFKA_H

#include "kafkaClient.h"
#include "kafkaUtil.h"

extern "C" {
ConstantSP kafkaProducer(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaProduce(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaProducerFlush(Heap *heap, vector<ConstantSP> &args);

ConstantSP kafkaConsumer(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaSubscribe(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaUnsubscribe(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaConsumerPoll(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaConsumerPollBatch(Heap *heap, vector<ConstantSP> &args);

ConstantSP kafkaCreateSubJob(Heap *heap, vector<ConstantSP> args);
ConstantSP kafkaGetJobStat(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaCancelSubJob(Heap *heap, vector<ConstantSP> args);
ConstantSP kafkaGetSubJobConsumer(Heap *heap, vector<ConstantSP> &args);

ConstantSP kafkaCommit(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaCommitTopic(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaConsumerAssign(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaConsumerUnassign(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaConsumerGetAssignment(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaConsumerPause(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaConsumerResume(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaGetOffsetInfo(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaGetOffset(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaGetOffsetsCommitted(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaGetOffsetPosition(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaGetMemberId(Heap *heap, vector<ConstantSP> &args);

ConstantSP kafkaGetMetadata(Heap *heap, vector<ConstantSP> &args);
}

#endif  // KAFKA_PLUGIN_KAFKA_H
