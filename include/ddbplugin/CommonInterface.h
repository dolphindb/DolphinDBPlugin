#pragma once

#include "CoreConcept.h"
#include "pluginVersion.h"
#include "ScalarImp.h"

extern "C" 
{
    ConstantSP version(Heap *heap, vector<ConstantSP> &arguments){
        std::ignore = heap;
        std::ignore = arguments;
        return new String(pluginVersion);
    }
}
