#pragma once

#if defined(_MSC_VER)
#pragma warning( push )
#elif defined(__clang__)
#pragma clang diagnostic push
#else // gcc
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#include <open62541/client_config_default.h>
#include <open62541/client_highlevel.h>
#include <open62541/client_subscriptions.h>
#include <open62541/network_tcp.h>
#include <open62541/plugin/accesscontrol_default.h>
#include <open62541/plugin/log_stdout.h>
#include <open62541/plugin/pki_default.h>
#include <open62541/plugin/securitypolicy.h>
#include <open62541/plugin/securitypolicy_default.h>
#include <open62541/types.h>
#include <open62541/types_generated_handling.h>

#if defined(_MSC_VER)
#pragma warning( pop )
#elif defined(__clang__)
#pragma clang diagnostic pop
#else // gcc
#pragma GCC diagnostic pop
#endif
