// SPDX-License-Identifier: Apache-2.0
// Copyright © 2025-2025 DolphinDB, Inc.
#pragma once

#include "DolphinDBEverything.h"
#include "ddbplugin/CommonInterface.h"
#include "ddbplugin/PluginLogger.h"
extern "C" ddb::ConstantSP setupGlobalConfig(ddb::Heap *heap, std::vector<ddb::ConstantSP> &args);
extern "C" ddb::ConstantSP createExchange(ddb::Heap *heap, std::vector<ddb::ConstantSP> &args);
extern "C" ddb::ConstantSP dropExchange(ddb::Heap *heap, std::vector<ddb::ConstantSP> &args);
