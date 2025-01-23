/* Copyright 2013-2015 The MathWorks, Inc. */

#ifndef CodeInstrTgtAppSvc_CInterface_h
#define CodeInstrTgtAppSvc_CInterface_h

#include "CodeInstrTgtAppSvc.h"

CODEINSTRTGTAPPSVC_API_EXTERN_C uint8_T codeInstrTgtAppSvcCreate(void);

CODEINSTRTGTAPPSVC_API_EXTERN_C void codeInstrTgtAppSvcDestroy(void);

CODEINSTRTGTAPPSVC_API_EXTERN_C uint8_T codeInstrTgtAppSvcAllocBuffer(void** ppBuf,
        const uint16_T size);

CODEINSTRTGTAPPSVC_API_EXTERN_C IOUnit_T* codeInstrTgtAppSvcGetBufferDataPtr(void* pBufVoid);

CODEINSTRTGTAPPSVC_API_EXTERN_C uint8_T codeInstrTgtAppSvcSend(void* pBufVoid,
        const uint16_T payloadSize);

CODEINSTRTGTAPPSVC_API_EXTERN_C uint16_T codeInstrTgtAppSvcGetMaxPayloadCapacity(void);

#endif
