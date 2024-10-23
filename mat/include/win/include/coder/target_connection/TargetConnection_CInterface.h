/* Copyright 2015 The MathWorks, Inc. */

#ifndef TargetConnection_CInterface_h
#define TargetConnection_CInterface_h

#include "coder_target_connection_spec.h"

CODER_TARGET_CONNECTION_EXPORT_EXTERN_C bool TargetConnectionCreate(
        void** const ppTargetConnections,
        void* const pConnectionOptions);

CODER_TARGET_CONNECTION_EXPORT_EXTERN_C void TargetConnectionDestroy(
        void* const pTargetConnections);

#endif
