#pragma once

#ifdef __linux__
#define LINUX
#elif defined(_WIN32)
#define WINDOWS
#endif

#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 4100 )
#elif defined(__clang__)
#pragma clang diagnostic push
// Too many to fix
#pragma clang diagnostic ignored "-Wdeprecated-copy-with-user-provided-copy"
#pragma clang diagnostic ignored "-Wignored-qualifiers"
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#pragma clang diagnostic ignored "-Wmismatched-tags"
#pragma clang diagnostic ignored "-Wnested-anon-types"
#pragma clang diagnostic ignored "-Wnull-dereference"
#pragma clang diagnostic ignored "-Woverloaded-virtual"
#pragma clang diagnostic ignored "-Wpessimizing-move"
#pragma clang diagnostic ignored "-Wtautological-constant-out-of-range-compare"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wvla-cxx-extension"
// Hazard Pointer in HashmapUtil.h looks like open-source software
#pragma clang diagnostic ignored "-Wbraced-scalar-init"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
// ConvertDeletedToEmptyAndFullToDeleted in FlatHashmap.h looks like open-source software
#pragma clang diagnostic ignored "-Wunneeded-internal-declaration"
// see comment for Logger.h::90
#pragma clang diagnostic ignored "-Wunused-value"
#else // gcc
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
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
#endif

#include "Logger.h"
#include "ScalarImp.h"
#include "SpecialConstant.h"

#if defined(_MSC_VER)
#pragma warning( pop )
#elif defined(__clang__)
#pragma clang diagnostic pop
#else // gcc
#pragma GCC diagnostic pop
#endif

#undef LINUX
#undef WINDOWS

using argsT = std::vector<ddb::ConstantSP>;
