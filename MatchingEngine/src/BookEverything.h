#pragma once

#if defined(_MSC_VER)
#pragma warning( push )
#elif defined(__clang__)
#pragma clang diagnostic push
#else // gcc
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wvla"
#pragma GCC diagnostic ignored "-Wtype-limits"
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

#include "book/depth_order_book.h"
#include "book/listener.h"
#include "book/order.h"

#if defined(_MSC_VER)
#pragma warning( pop )
#elif defined(__clang__)
#pragma clang diagnostic pop
#else // gcc
#pragma GCC diagnostic pop
#endif
