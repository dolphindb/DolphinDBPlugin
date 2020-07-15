/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef UA_CONFIG_H_
#define UA_CONFIG_H_

/**
 * open62541 Version
 * ----------------- */
#define UA_OPEN62541_VER_MAJOR 0
#define UA_OPEN62541_VER_MINOR 0
#define UA_OPEN62541_VER_PATCH 0
#define UA_OPEN62541_VER_LABEL "" /* Release candidate label, etc. */
#define UA_OPEN62541_VER_COMMIT "v.1.0.1"

/**
 * Feature Options
 * ---------------
 * Changing the feature options has no effect on a pre-compiled library. */

#define UA_LOGLEVEL 300
#ifndef UA_ENABLE_AMALGAMATION
/* #undef UA_ENABLE_AMALGAMATION */
#endif
#define UA_ENABLE_METHODCALLS
#define UA_ENABLE_NODEMANAGEMENT
#define UA_ENABLE_SUBSCRIPTIONS
/* #undef UA_ENABLE_PUBSUB */
/* #undef UA_ENABLE_PUBSUB_ETH_UADP */
/* #undef UA_ENABLE_PUBSUB_DELTAFRAMES */
/* #undef UA_ENABLE_PUBSUB_INFORMATIONMODEL */
/* #undef UA_ENABLE_PUBSUB_INFORMATIONMODEL_METHODS */
#define UA_ENABLE_DA
#define UA_ENABLE_ENCRYPTION */
/* #undef UA_ENABLE_HISTORIZING */
/* #undef UA_ENABLE_MICRO_EMB_DEV_PROFILE */
/* #undef UA_ENABLE_EXPERIMENTAL_HISTORIZING */
/* #undef UA_ENABLE_SUBSCRIPTIONS_EVENTS */
/* #undef UA_ENABLE_JSON_ENCODING */

/* Multithreading */
/* #undef UA_ENABLE_MULTITHREADING */
/* #undef UA_ENABLE_IMMUTABLE_NODES */
#if defined(UA_ENABLE_MULTITHREADING) && !defined(UA_ENABLE_IMMUTABLE_NODES)
#error "The multithreading feature requires nodes to be immutable"
#endif

/* Advanced Options */
/* #undef UA_ENABLE_CUSTOM_NODESTORE */
#define UA_ENABLE_STATUSCODE_DESCRIPTIONS
#define UA_ENABLE_TYPENAMES
#define UA_ENABLE_NODESET_COMPILER_DESCRIPTIONS
/* #undef UA_ENABLE_DETERMINISTIC_RNG */
#define UA_ENABLE_DISCOVERY
/* #undef UA_ENABLE_DISCOVERY_MULTICAST */
/* #undef UA_ENABLE_QUERY */
/* #undef UA_ENABLE_MALLOC_SINGLETON */
#define UA_ENABLE_DISCOVERY_SEMAPHORE
/* #undef UA_ENABLE_UNIT_TEST_FAILURE_HOOKS */
/* #undef UA_ENABLE_VALGRIND_INTERACTIVE */
#define UA_VALGRIND_INTERACTIVE_INTERVAL 1000
#define UA_GENERATED_NAMESPACE_ZERO
/* #undef UA_ENABLE_PUBSUB_CUSTOM_PUBLISH_HANDLING */

/* #undef UA_PACK_DEBIAN */

/* Options for Debugging */
/* #undef UA_DEBUG */
/* #undef UA_DEBUG_DUMP_PKGS */

/**
 * Function Export
 * ---------------
 * On Win32: Define ``UA_DYNAMIC_LINKING`` and ``UA_DYNAMIC_LINKING_EXPORT`` in
 * order to export symbols for a DLL. Define ``UA_DYNAMIC_LINKING`` only to
 * import symbols from a DLL.*/
#define UA_DYNAMIC_LINKING

/* Shortcuts for extern "C" declarations */
#if !defined(_UA_BEGIN_DECLS)
# ifdef __cplusplus
#  define _UA_BEGIN_DECLS extern "C" {
# else
#  define _UA_BEGIN_DECLS
# endif
#endif
#if !defined(_UA_END_DECLS)
# ifdef __cplusplus
#  define _UA_END_DECLS }
# else
#  define _UA_END_DECLS
# endif
#endif

/* Select default architecture if non is selected through CMake or compiler define */
#if 1  && !defined(UA_ARCHITECTURE_ECOS) && !defined(UA_ARCHITECTURE_FREERTOSLWIP) && !defined(UA_ARCHITECTURE_POSIX) && !defined(UA_ARCHITECTURE_VXWORKS) && !defined(UA_ARCHITECTURE_WEC7) && !defined(UA_ARCHITECTURE_WIN32)
# ifdef _WIN32
#  define UA_ARCHITECTURE_WIN32
# else
#  define UA_ARCHITECTURE_POSIX
# endif
#endif

#include "posix/ua_architecture.h"

#endif /* UA_CONFIG_H_ */
