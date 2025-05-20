#pragma once

#if defined(_MSC_VER)
#pragma warning( push )
#elif defined(__clang__)
#pragma clang diagnostic push
#else // gcc
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

#include "librdkafka/rdkafka.h"
#include "cppkafka/cppkafka.h"

#if defined(_MSC_VER)
#pragma warning( pop )
#elif defined(__clang__)
#pragma clang diagnostic pop
#else // gcc
#pragma GCC diagnostic pop
#endif
