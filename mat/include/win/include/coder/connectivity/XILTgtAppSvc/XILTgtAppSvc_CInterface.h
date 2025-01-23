/* Copyright 2013-2015 The MathWorks, Inc. */

#ifndef XILTgtAppSvc_CInterface_h
#define XILTgtAppSvc_CInterface_h

#include "XILTgtAppSvc.h"

XILTGTAPPSVC_API_EXTERN_C uint8_T xilTgtAppSvcCreate(void);

XILTGTAPPSVC_API_EXTERN_C void xilTgtAppSvcDestroy(void);

XILTGTAPPSVC_API_EXTERN_C uint8_T* xilTgtAppSvcGetReceivedData(uint16_T* size);

XILTGTAPPSVC_API_EXTERN_C void xilTgtAppSvcFreeLastReceivedData(void);

XILTGTAPPSVC_API_EXTERN_C uint8_T xilTgtAppSvcAllocBuffer(void** ppBuf,
        const uint16_T size);

XILTGTAPPSVC_API_EXTERN_C IOUnit_T* xilTgtAppSvcGetBufferDataPtr(void* pBufVoid);

XILTGTAPPSVC_API_EXTERN_C uint8_T xilTgtAppSvcSend(void* pBufVoid,
        const uint16_T payloadSize);

XILTGTAPPSVC_API_EXTERN_C uint16_T xilTgtAppSvcGetMaxPayloadCapacity(void);

#endif
