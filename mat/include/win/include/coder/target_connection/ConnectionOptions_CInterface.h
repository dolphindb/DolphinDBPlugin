/* Copyright 2015 The MathWorks, Inc. */

#ifndef ConnectionOptions_CInterface_h
#define ConnectionOptions_CInterface_h

#include <stdint.h>
#include "coder_target_connection_spec.h"

CODER_TARGET_CONNECTION_EXPORT_EXTERN_C bool ConnectionOptionsCreate(
        void** const ppConnectionOptions);

CODER_TARGET_CONNECTION_EXPORT_EXTERN_C void ConnectionOptionsDestroy(
        void* const pConnectionOptions);

CODER_TARGET_CONNECTION_EXPORT_EXTERN_C void ConnectionOptionsSetTransport(
        void* const pConnectionOptions,
        int32_t val);

CODER_TARGET_CONNECTION_EXPORT_EXTERN_C void ConnectionOptionsSetIPPort(
        void* const pConnectionOptions,
        uint16_t val);

CODER_TARGET_CONNECTION_EXPORT_EXTERN_C void ConnectionOptionsSetHostName(
        void* const pConnectionOptions,
        const char* c);

CODER_TARGET_CONNECTION_EXPORT_EXTERN_C void ConnectionOptionsSetSerialPort(
        void* const pConnectionOptions,
        const char* c);

CODER_TARGET_CONNECTION_EXPORT_EXTERN_C void ConnectionOptionsSetBaudRate(
        void* const pConnectionOptions,
        uint32_t b);

#endif
