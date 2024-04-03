#pragma once

#include "CoreConcept.h"
#include "pluginVersion.h"
#include "ScalarImp.h"

extern "C" 
{
    ConstantSP version(Heap *heap, vector<ConstantSP> &arguments){
        return new String(pluginVersion);
    }
}