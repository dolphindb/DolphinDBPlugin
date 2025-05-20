#ifndef FEATHER_PLUGIN_H
#define FEATHER_PLUGIN_H

#include <CoreConcept.h>
#include "ddbplugin/CommonInterface.h"
#include <Exceptions.h>
#include <FlatHashmap.h>
#include <ScalarImp.h>
#include <SysIO.h>
#include <Util.h>
#include <Types.h>

#include "arrow/type.h"
#include "arrow/type_fwd.h"

using ddb::ConstantSP;
using ddb::Heap;
using ddb::DATA_TYPE;
using std::vector;

extern "C" ConstantSP loadFeather(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP saveFeather(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP schemaFeather(Heap *heap, vector<ConstantSP> &arguments);


#endif