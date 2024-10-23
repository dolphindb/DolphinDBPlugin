/*
 * Demo.h
 *
 *  Created on: Apr 19, 2017
 *      Author: dzhou
 */

#ifndef DEMO_H_
#define DEMO_H_

#include "CoreConcept.h"
#include "ddbplugin/CommonInterface.h"
extern "C" ConstantSP minmax(const ConstantSP& a, const ConstantSP& b);
extern "C" ConstantSP echo(Heap* heap, vector<ConstantSP>& arguments );

#endif /* DEMO_H_ */
