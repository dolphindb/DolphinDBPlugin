#pragma once

#if defined(_MSC_VER)
#pragma warning( push )
#elif defined(__clang__)
#pragma clang diagnostic push
#else // gcc
#pragma GCC diagnostic push
#if defined(__GNUC__) && (__GNUC__ >= 9)
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#endif
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#pragma GCC diagnostic ignored "-Wtype-limits"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wvla"
#pragma GCC diagnostic ignored "-Wpedantic"
#if defined(__GNUC__) && (__GNUC__ >= 5)
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough="
#endif
#endif

#include "CoreConcept.h"
#include "ScalarImp.h"
#include "StreamEngine.h"

#if defined(_MSC_VER)
#pragma warning( pop )
#elif defined(__clang__)
#pragma clang diagnostic pop
#else // gcc
#pragma GCC diagnostic pop
#endif
