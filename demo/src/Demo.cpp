/*
 * Demo.cpp
 *
 *  Created on: Apr 19, 2017
 *      Author: dzhou
 */

#include "Demo.h"
#include "Util.h"

namespace ddb {

ConstantSP minmax(const ConstantSP& a, const ConstantSP& b)
{
	std::ignore = b;
	if(!a->isScalar() && !a->isArray())
		throw IllegalArgumentException("minmax","The argument for minmax function must be a scalar or vector.");
	ConstantSP result = Util::createVector(a->getType(), 2);
	if(a->isScalar()) {
		result->set(0, a);
		result->set(1, a);
	} else {
		result->set(0, ((Vector*)a.get())->min());
		result->set(1, ((Vector*)a.get())->max());
	}
	return result;
}

ConstantSP echo(Heap* heap, vector<ConstantSP>& arguments)
{
	std::ignore = heap;
	return arguments[0];
}

} // namespace ddb
