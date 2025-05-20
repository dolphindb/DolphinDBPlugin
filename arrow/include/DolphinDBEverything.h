#pragma once

#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 4100 )
#elif defined(__clang__)
#pragma clang diagnostic push
// Too many to fix
#pragma clang diagnostic ignored "-Woverloaded-virtual"
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#pragma clang diagnostic ignored "-Wvla-cxx-extension"
#pragma clang diagnostic ignored "-Wnull-dereference"
#pragma clang diagnostic ignored "-Wmismatched-tags"
#pragma clang diagnostic ignored "-Wpessimizing-move"
// Hazard Pointer in HashmapUtil.h looks like open-source software
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#pragma clang diagnostic ignored "-Wbraced-scalar-init"
// ConvertDeletedToEmptyAndFullToDeleted in FlatHashmap.h looks like open-source software
#pragma clang diagnostic ignored "-Wunneeded-internal-declaration"
// see comment for Logger.h::90
#pragma clang diagnostic ignored "-Wunused-value"
#else // gcc
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#endif

#include "CoreConcept.h"
#include "ScalarImp.h"
#include "SpecialConstant.h"

#if defined(_MSC_VER)
#pragma warning( pop )
#elif defined(__clang__)
#pragma clang diagnostic pop
#else // gcc
#pragma GCC diagnostic pop
#endif
