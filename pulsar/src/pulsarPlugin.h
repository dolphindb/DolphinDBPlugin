#pragma once

#include "CoreConcept.h"
#include "ddbplugin/CommonInterface.h"
#include "ddbplugin/PluginLogger.h"

extern "C" {

ddb::ConstantSP pulsarClient(ddb::Heap *heap, argsT &args);

ddb::ConstantSP pulsarProducer(ddb::Heap *heap, argsT &args);
ddb::ConstantSP pulsarSend(ddb::Heap *heap, argsT &args);

ddb::ConstantSP pulsarConsumer(ddb::Heap *heap, argsT &args);
ddb::ConstantSP pulsarReceive(ddb::Heap *heap, argsT &args);

ddb::ConstantSP pulsarCreateSubJob(ddb::Heap *heap, argsT &args);
ddb::ConstantSP pulsarCancelSubJob(ddb::Heap *heap, argsT &args);
ddb::ConstantSP pulsarGetJobStat(ddb::Heap *heap, argsT &args);

ddb::ConstantSP pulsarGetConfig(ddb::Heap *heap, argsT &args);

}
