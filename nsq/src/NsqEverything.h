#pragma once

#if defined(_MSC_VER)
#pragma warning( push )
#elif defined(__clang__)
#pragma clang diagnostic push
#else // gcc
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#include "HSDataType.h"
#include "HSNsqStruct.h"
#include "HSNsqApi.h"

#if defined(_MSC_VER)
#pragma warning( pop )
#elif defined(__clang__)
#pragma clang diagnostic pop
#else // gcc
#pragma GCC diagnostic pop
#endif
