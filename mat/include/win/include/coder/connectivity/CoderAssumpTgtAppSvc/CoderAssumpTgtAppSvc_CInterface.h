/* Copyright 2013-2015 The MathWorks, Inc. */

#ifndef CoderAssumpTgtAppSvc_CInterface_h
#define CoderAssumpTgtAppSvc_CInterface_h

#include "CoderAssumpTgtAppSvc_spec.h"
#include "rtwtypes.h"

#if defined (USING_CS_API) || defined(BUILDING_LIBMWCODER_CODERASSUMPTGTAPPSVC) 
    typedef uint8_T IOUnit_T;  
#else
    #include "xilcomms_rtiostream.h"
#endif

#define CODERASSUMP_RTIOSTREAM_BASED_SERVICE_ID 3

static const uint8_T CODERASSUMPTGTAPPSVC_ERROR = 0;
static const uint8_T CODERASSUMPTGTAPPSVC_SUCCESS = 1;

CODERASSUMPTGTAPPSVC_API_EXTERN_C uint8_T coderAssumpTgtAppSvcCreate(void);

CODERASSUMPTGTAPPSVC_API_EXTERN_C void coderAssumpTgtAppSvcDestroy(void);

CODERASSUMPTGTAPPSVC_API_EXTERN_C uint8_T coderAssumpTgtAppSvcAllocBuffer(void** ppBuf,
        const uint16_T size);

CODERASSUMPTGTAPPSVC_API_EXTERN_C IOUnit_T* coderAssumpTgtAppSvcGetBufferDataPtr(void* pBufVoid);

CODERASSUMPTGTAPPSVC_API_EXTERN_C uint8_T coderAssumpTgtAppSvcSend(void* pBufVoid,
        const uint16_T payloadSize);

CODERASSUMPTGTAPPSVC_API_EXTERN_C uint16_T coderAssumpTgtAppSvcGetMaxPayloadCapacity(void);

#endif
