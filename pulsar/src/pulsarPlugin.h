// SPDX-License-Identifier: Apache-2.0
// Copyright © 2025-2025 DolphinDB, Inc.
#pragma once

#include "CoreConcept.h"
#include "ddbplugin/CommonInterface.h"
#include "ddbplugin/PluginLogger.h"

using namespace ddb;

extern "C" {

ConstantSP pulsarClient(Heap *heap, argsT &args);

ConstantSP pulsarProducer(Heap *heap, argsT &args);
ConstantSP pulsarSend(Heap *heap, argsT &args);

ConstantSP pulsarConsumer(Heap *heap, argsT &args);
ConstantSP pulsarReceive(Heap *heap, argsT &args);

ConstantSP pulsarCreateSubJob(Heap *heap, argsT &args);
ConstantSP pulsarCancelSubJob(Heap *heap, argsT &args);
ConstantSP pulsarGetJobStat(Heap *heap, argsT &args);

ConstantSP pulsarGetConfig(Heap *heap, argsT &args);

}
