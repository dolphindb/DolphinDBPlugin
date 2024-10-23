/* Copyright 2013-2015 The MathWorks, Inc. */

#ifndef XILHostAppSvc_CInterface_h
#define XILHostAppSvc_CInterface_h

#include "coder/connectivity/XILHostAppSvc/XILHostAppSvc.h"
#include "coder/connectivity/XILHostAppSvc/XILHostAppSvc_spec.h"

typedef struct {
    void* data;
    mwSize size;
} XIL_IOBuffer_T;

XILHOSTAPPSVC_API_EXTERN_C boolean_T xilHostAppSvcCreate(
        void** const ppXILHostAppSvc,
        void* const pCSVoid,
        void* const pXILUtils,                                          
        XIL_IOBuffer_T* const pIOBuffer,                                
        const uint8_T memUnitSize,                                      
        const uint8_T ioDataTypeSize,
        const boolean_T isUsingCommService);
       
XILHOSTAPPSVC_API_EXTERN_C const uint8_T xilHostAppSvcGetIsResponseComplete(
        void* const pXILHostAppSvc);

XILHOSTAPPSVC_API_EXTERN_C void xilHostAppSvcSetIsResponseComplete(
        void* const pXILHostAppSvc,                                                  
        const uint8_T val);

XILHOSTAPPSVC_API_EXTERN_C const mwSize xilHostAppSvcGetPayloadSizeForOneStep(
        void* const pXILHostAppSvc);

XILHOSTAPPSVC_API_EXTERN_C boolean_T xilHostAppSvcGrowIOBuffer(
        void* const pXILHostAppSvc,                                        
        XIL_IOBuffer_T* const IOBuffer,                                       
        const mwSize requiredSize,                                            
        const uint8_T isVerbose);

XILHOSTAPPSVC_API_EXTERN_C boolean_T xilHostAppSvcSend(
        void* const pXILHostAppSvc,                                      
        void* const src,                                              
        const mwSize size);

XILHOSTAPPSVC_API_EXTERN_C boolean_T xilHostAppSvcWaitForDataFromTarget(
        void* const pXILHostAppSvc);
      
XILHOSTAPPSVC_API_EXTERN_C void xilHostAppSvcDestroy(
        void* const pXILHostAppSvc);

#endif
