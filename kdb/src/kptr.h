#ifndef _KPTR_H_
#define _KPTR_H_

#include <type_traits>
#include <memory>

#include "k.h"

//////////////////////////////////////////////////////////////////////////////

struct KDeleter {
    void operator()(K k) const;
};
using KPtr = std::unique_ptr<typename std::remove_pointer<K>::type, KDeleter>;

//////////////////////////////////////////////////////////////////////////////

#endif//_KPTR_H_