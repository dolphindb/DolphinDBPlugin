#pragma once

#include "CoreConcept.h"
#include "pluginVersion.h"
#include "ScalarImp.h"

#ifndef IS_MODULE
extern "C" {
    ddb::ConstantSP version(ddb::Heap *heap, std::vector<ddb::ConstantSP> &arguments){
        std::ignore = heap;
        std::ignore = arguments;
        return new ddb::String(pluginVersion);
    }
}   // extern "C"
#endif