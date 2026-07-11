#pragma once

#include "DolphinDBEverything.h"

#define PLUGIN_API(func) extern "C" __attribute__((visibility("default"))) ConstantSP func(Heap *heap, argsT &args)

PLUGIN_API(initialize);
