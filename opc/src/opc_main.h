#ifndef DEMO_H_
#define DEMO_H_

#include "CoreConcept.h"
#include "Logger.h"
#include "ddbplugin/CommonInterface.h"

extern "C" ConstantSP getOpcServerList(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP connectOpcServer(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP disconnect(Heap* heap, vector<ConstantSP>& arguments );
extern "C" ConstantSP readTag(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP writeTag(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP subscribeTag(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP endSub(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP getSubscriberStat(Heap* heap, vector<ConstantSP>& arguments);
#endif /* DEMO_H_ */
