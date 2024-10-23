/*
    Abandoned functions of kafka plugin
 */

#ifndef KAFKA_ABANDONED_H
#define KAFKA_ABANDONED_H

#include "kafkaUtil.h"
#include "kafkaWrapper.h"

extern "C" {
ConstantSP kafkaPollByteStream(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaPollDict(Heap *heap, vector<ConstantSP> &args);

ConstantSP kafkaSetConsumerTimeout(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaSetProducerTimeout(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaGetConsumerTimeout(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaGetProducerTimeout(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaGetBufferSize(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaSetBufferSize(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaGetMessageSize(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaSetMessageSize(Heap *heap, vector<ConstantSP> &args);

ConstantSP kafkaAsyncCommit(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaAsyncCommitTopic(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaStoreConsumedOffsets(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaStoreOffsets(Heap *heap, vector<ConstantSP> &args);

ConstantSP kafkaQueueLength(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaForToQueue(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaDisForToQueue(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaSetQueueTime(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaGetQueueTime(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaQueueConsume(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaQueueConsumeBatch(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaGetMainQueue(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaGetConsumerQueue(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaGetPartitionQueue(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaQueueEvent(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaGetEventName(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaEventGetMessages(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaGetEventMessageCount(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaEventGetError(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaEventGetPartition(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaEventGetPartitionList(Heap *heap, vector<ConstantSP> &args);
ConstantSP kafkaEventBool(Heap *heap, vector<ConstantSP> &args);
}

#endif