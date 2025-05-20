#include "DolphinDBEverything.h"
#include <CoreConcept.h>
#include<Util.h>
#include <ScalarImp.h>
#include "ddbplugin/CommonInterface.h"

using ddb::ConstantSP;
using ddb::Heap;
using std::vector;
extern "C" ConstantSP gpPlot(Heap *heap, vector<ConstantSP> &args);