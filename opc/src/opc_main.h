#ifndef DEMO_H_
#define DEMO_H_

#include "CoreConcept.h"

extern "C" ConstantSP getOpcServerList(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP connectOpcServer(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP disconnect(const ConstantSP& handle, const ConstantSP& b );
extern "C" ConstantSP readTag(Heap* heap, vector<ConstantSP>& arguments );
extern "C" ConstantSP writeTag(Heap* heap, vector<ConstantSP>& arguments );
extern "C" ConstantSP subscribeTag(Heap* heap, vector<ConstantSP>& arguments );
extern "C" ConstantSP endSub(const ConstantSP& handle, const ConstantSP& b );
#endif /* DEMO_H_ */
