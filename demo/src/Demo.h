#pragma once

#include "DolphinDBEverything.h"
#include "CoreConcept.h"
#include "ddbplugin/CommonInterface.h"

extern "C" {
ddb::ConstantSP minmax(const ddb::ConstantSP& a, const ddb::ConstantSP& b);
ddb::ConstantSP echo(ddb::Heap* heap, argsT &arguments );
}
