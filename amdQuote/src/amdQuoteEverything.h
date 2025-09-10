#pragma once

// gcc
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#pragma GCC diagnostic ignored "-Wattributes"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

#if __GNUC__ > 7
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#endif
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wtype-limits"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wvla"
#if __GNUC__ >= 9
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#endif
#if __GNUC__ >= 14
#pragma GCC diagnostic ignored "-Wunused-result"
#endif

#include "ama.h"
#include "ama_datatype.h"
#include "ama_tools.h"
#ifdef AMD_457
#include "ama_property.h"
#endif

#pragma GCC diagnostic pop

