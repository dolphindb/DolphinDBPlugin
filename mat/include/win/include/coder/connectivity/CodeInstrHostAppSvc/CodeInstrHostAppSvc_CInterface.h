/* Copyright 2013-2015 The MathWorks, Inc. */

#ifndef CodeInstrHostAppSvc_CInterface_h
#define CodeInstrHostAppSvc_CInterface_h

#include "coder/connectivity/CodeInstrHostAppSvc/CodeInstrHostAppSvc.h"
#include "coder/connectivity/CodeInstrHostAppSvc/CodeInstrHostAppSvc_spec.h"

typedef struct CodeInstrServiceData {
    const char* infoPath;
    const char* blockPath;
    const char* rootModel;
    size_t memUnitSize;
    bool isProfilingEnabled;
    size_t inTheLoopType;
    const char* silPilInterfaceFcn;
} CodeInstrServiceData_T;

CODEINSTRHOSTAPPSVC_API_EXTERN_C int codeInstrHostAppSvcCreate(
        void** const ppCodeInstrService,  
        const CodeInstrServiceData_T* const pData,
        void* const pCSVoid,                        
        const void* const pMemUnitXformer,                                
        const int idTypeSize,                                             
        void* const pXILUtils,                                            
        const int memUnitSize,
        const boolean_T isUsingCommService);
               
CODEINSTRHOSTAPPSVC_API_EXTERN_C void codeInstrHostAppSvcSetTime(
        void* const pCodeInstrService,                      
        const time_T simTime);
       
CODEINSTRHOSTAPPSVC_API_EXTERN_C void codeInstrHostAppSvcDestroy(
        void* const pCodeInstrService,
        const CodeInstrServiceData_T* const pData);

#endif

