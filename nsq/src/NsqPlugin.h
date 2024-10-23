//
// Created by htxu on 11/20/2023.
//

#ifndef PLUGINNSQ_NSQPLUGIN_H
#define PLUGINNSQ_NSQPLUGIN_H


#include <CoreConcept.h>
#include "ddbplugin/CommonInterface.h"

extern "C" {

ConstantSP nsqConnect(Heap* heap, vector<ConstantSP>& args);
ConstantSP nsqClose(Heap *heap, vector<ConstantSP>& args);

ConstantSP nsqGetSchema(Heap *heap, vector<ConstantSP>& args);

ConstantSP nsqSubscribe(Heap* heap, vector<ConstantSP>& args);
ConstantSP nsqUnsubscribe(Heap* heap, vector<ConstantSP>& args);

ConstantSP nsqGetStatus(Heap *heap, vector<Constant>& args);

}


#endif //PLUGINNSQ_NSQPLUGIN_H
