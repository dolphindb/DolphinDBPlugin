//
// Created by jccai on 19-5-24.
//

#ifndef MATCHINGENGINE_EXPORT_H
#define MATCHINGENGINE_EXPORT_H

#include "DolphinDBEverything.h"
#include "ddbplugin/CommonInterface.h"
#include "ddbplugin/PluginLoggerImp.h"
extern "C" ddb::ConstantSP setupGlobalConfig(ddb::Heap *heap, std::vector<ddb::ConstantSP> &args);
extern "C" ddb::ConstantSP createExchange(ddb::Heap *heap, std::vector<ddb::ConstantSP> &args);

#endif    // MATCHINGENGINE_EXPORT_H
