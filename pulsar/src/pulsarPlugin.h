//
// Created by htxu on 10/19/2023.
//

#ifndef PULSAR_PULSARPLUGIN_H
#define PULSAR_PULSARPLUGIN_H


#include "CoreConcept.h"
#include "ddbplugin/CommonInterface.h"

extern "C" {

ConstantSP pulsarClient(Heap *heap, vector<ConstantSP> &args);

ConstantSP pulsarProducer(Heap *heap, vector<ConstantSP> &args);
ConstantSP pulsarSend(Heap *heap, vector<ConstantSP> &args);

ConstantSP pulsarConsumer(Heap *heap, vector<ConstantSP> &args);
ConstantSP pulsarReceive(Heap *heap, vector<ConstantSP> &args);

ConstantSP pulsarCreateSubJob(Heap *heap, vector<ConstantSP> &args);
ConstantSP pulsarCancelSubJob(Heap *heap, vector<ConstantSP> &args);
ConstantSP pulsarGetJobStat(Heap *heap, vector<ConstantSP> &args);

ConstantSP pulsarGetConfig(Heap *heap, vector<ConstantSP> &args);

}

#endif //PULSAR_PULSARPLUGIN_H
