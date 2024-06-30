/*
 * @(#)mclmcrrt.h
 *
 *				apiproxy.published
 *				libmat_proxy.cpp
 *				libmwmclbase_proxy.cpp
 *				libmwmclmcr_proxy.cpp
 *				libmx_proxy.cpp
 */

#if defined(_MSC_VER)
# pragma once
#endif
#if defined(__GNUC__) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ > 3))
# pragma once
#endif

#ifndef mclmcrrt_h
#define mclmcrrt_h


/*
 * Copyright 1984-2003 The MathWorks, Inc.
 * All Rights Reserved.
 */



/* Copyright 2003-2006 The MathWorks, Inc. */

/* Only define EXTERN_C if it hasn't been defined already. This allows
 * individual modules to have more control over managing their exports.
 */
#ifndef EXTERN_C

#ifdef __cplusplus
  #define EXTERN_C extern "C"
#else
  #define EXTERN_C extern
#endif

#endif



#ifdef __LCC__
/* Must undefine EXTERN_C here (and redefine it later) because LCC's version
 * of windows.h has its own definition of EXTERN_C.
 */
#undef EXTERN_C
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __LCC__
#undef EXTERN_C
#define EXTERN_C extern
#endif

#ifndef _WIN32
typedef const struct _GUID *REFCLSID, *REFGUID;
typedef long HRESULT;
#endif



#  if defined( linux ) || defined( __linux ) || defined( __linux__ )
/* stdint.h must be included before sys/types.h or loadlibrary will fail.
 * Because matrix.h includes stdlib.h, which includes sys/types.h, stdint.h
 * must be included before any include of matrix.h (On Linux systems.)
 */
#include <stdint.h> 
#endif

#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))

#ifdef __cplusplus
extern "C"
{
#endif


#undef mclmcrInitialize2
#define mclmcrInitialize2 mclmcrInitialize2_proxy


#undef mclmcrInitialize
#define mclmcrInitialize mclmcrInitialize_proxy


#ifdef __cplusplus
}
#endif


#undef mclInitializeApplication
#define mclInitializeApplication mclInitializeApplication_proxy


#undef mclDisplayStartMessage
#define mclDisplayStartMessage mclDisplayStartMessage_proxy

#endif 

typedef void * MCREventHandlerArg;
typedef void (*MCREventHandlerFcn)(MCREventHandlerArg);
typedef enum
{   MCRStartEvent,
    MCRCompleteEvent
} mcrInitializationEventType;
typedef void * MCREventData;

EXTERN_C void mclDisplayStartMessage_proxy(mcrInitializationEventType eventType,MCREventHandlerFcn fcn,MCREventHandlerArg arg,MCREventData eventData);


#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetComponentInfo
#define mclGetComponentInfo mclGetComponentInfo_proxy
#endif 


EXTERN_C HRESULT mclGetComponentInfo_proxy(const char* lpszComponent, 
                                                      int nMajorRev, 
                                                      int nMinorRev, int nInfo, 
                                                      int nType, 
                                                      void** info);


#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetLIBIDInfo
#define mclGetLIBIDInfo mclGetLIBIDInfo_proxy
#endif 


EXTERN_C HRESULT mclGetLIBIDInfo_proxy(const char* lpszLIBID, 
                                                  int nMajorRev, int nMinorRev, 
                                                  int nInfo, void** info);


#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclRegisterServer
#define mclRegisterServer mclRegisterServer_proxy
#endif 


EXTERN_C HRESULT mclRegisterServer_proxy(const char* szModuleName,     
                                                    REFCLSID clsid,               
                                                    REFGUID libid,                
                                                    unsigned short wMajorRev,     
                                                    unsigned short wMinorRev,     
                                                    const char* szFriendlyName,   
                                                    const char* szVerIndProgID,   
                                                    const char* szProgID,         
                                                    const char* szThreadingModel);


#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGUIDFromString
#define mclGUIDFromString mclGUIDFromString_proxy
#endif 


EXTERN_C int mclGUIDFromString_proxy(const char* lpszGUID, struct _GUID* pguid);


#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclUnRegisterMatLabCOMComponent
#define mclUnRegisterMatLabCOMComponent mclUnRegisterMatLabCOMComponent_proxy
#endif 


EXTERN_C HRESULT mclUnRegisterMatLabCOMComponent_proxy(REFCLSID clsid,            
                                       const char* szVerIndProgID, 
                                       const char* szProgID);


#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclRegisterMatLabXLComponent
#define mclRegisterMatLabXLComponent mclRegisterMatLabXLComponent_proxy
#endif 


EXTERN_C HRESULT mclRegisterMatLabXLComponent_proxy(const char* szModuleName,    
                                    REFCLSID clsid,               
                                    REFGUID libid,                
                                    unsigned short wMajorRev,     
                                    unsigned short wMinorRev,     
                                    const char* szFriendlyName,   
                                    const char* szVerIndProgID,   
                                    const char* szProgID);


#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGUIDtochar
#define mclGUIDtochar mclGUIDtochar_proxy
#endif 


EXTERN_C void mclGUIDtochar_proxy(REFGUID guid, char* szGUID, int length);


#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclUnregisterServer
#define mclUnregisterServer mclUnregisterServer_proxy
#endif 


EXTERN_C HRESULT mclUnregisterServer_proxy(REFCLSID clsid,             
                           const char* szVerIndProgID, 
                           const char* szProgID);


#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclCLSIDtochar
#define mclCLSIDtochar mclCLSIDtochar_proxy
#endif 


EXTERN_C void mclCLSIDtochar_proxy(REFCLSID clsid, char* szCLSID, int length);


#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclFreeComponentInfo
#define mclFreeComponentInfo mclFreeComponentInfo_proxy
#endif 


EXTERN_C void mclFreeComponentInfo_proxy(void** info);


#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclUnRegisterMatLabXLComponent
#define mclUnRegisterMatLabXLComponent mclUnRegisterMatLabXLComponent_proxy
#endif 


EXTERN_C HRESULT mclUnRegisterMatLabXLComponent_proxy(REFCLSID clsid,             
                                      const char* szVerIndProgID, 
                                      const char* szProgID);


#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclRegisterMatLabCOMComponent
#define mclRegisterMatLabCOMComponent mclRegisterMatLabCOMComponent_proxy
#endif 


EXTERN_C HRESULT mclRegisterMatLabCOMComponent_proxy(const char* szModuleName,     
                                     REFCLSID clsid,               
                                     REFGUID libid,                
                                     unsigned short wMajorRev,     
                                     unsigned short wMinorRev,     
                                     const char* szFriendlyName,   
                                     const char* szVerIndProgID,   
                                     const char* szProgID);

#ifndef MW_CALL_CONV
#  ifdef _WIN32 
#      define MW_CALL_CONV __cdecl
#  else
#      define MW_CALL_CONV 
#  endif
#endif

/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef matOpen
#define matOpen matOpen_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef matClose
#define matClose matClose_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef matGetFp
#define matGetFp matGetFp_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef matPutVariable
#define matPutVariable matPutVariable_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef matPutVariableAsGlobal
#define matPutVariableAsGlobal matPutVariableAsGlobal_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef matGetVariable
#define matGetVariable matGetVariable_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef matGetNextVariable
#define matGetNextVariable matGetNextVariable_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef matGetNextVariableInfo
#define matGetNextVariableInfo matGetNextVariableInfo_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef matGetVariableInfo
#define matGetVariableInfo matGetVariableInfo_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef matDeleteVariable
#define matDeleteVariable matDeleteVariable_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef matGetDir
#define matGetDir matGetDir_proxy
#endif



/*#ifdef mat_h
#error "mclmcrrt.h must be included before mat.h. (Since mclmcrrt.h includes mat.h, additional inclusion is redundant.)"
#endif */
#define LIBMWMAT_API_EXTERN_C EXTERN_C
#include "mat.h"

/* Proxies for functions in mat.h */

EXTERN_C
MATFile * matOpen_proxy(const char *a0, const char *a1);

EXTERN_C
matError matClose_proxy(MATFile *a0);

EXTERN_C
FILE * matGetFp_proxy(MATFile *a0);

EXTERN_C
matError matPutVariable_proxy(MATFile *a0, const char *a1, 
    const mxArray *a2);

EXTERN_C
matError matPutVariableAsGlobal_proxy(MATFile *a0, const char *a1, 
    const mxArray *a2);

EXTERN_C
mxArray * matGetVariable_proxy(MATFile *a0, const char *a1);

EXTERN_C
mxArray * matGetNextVariable_proxy(MATFile *a0, const char **a1);

EXTERN_C
mxArray * matGetNextVariableInfo_proxy(MATFile *a0, const char **a1);

EXTERN_C
mxArray * matGetVariableInfo_proxy(MATFile *a0, const char *a1);

EXTERN_C
matError matDeleteVariable_proxy(MATFile *a0, const char *a1);

EXTERN_C
char ** matGetDir_proxy(MATFile *a0, int *a1);



#ifndef MW_CALL_CONV
#  ifdef _WIN32 
#      define MW_CALL_CONV __cdecl
#  else
#      define MW_CALL_CONV 
#  endif
#endif

/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclSetLastErrIdAndMsg
#define mclSetLastErrIdAndMsg mclSetLastErrIdAndMsg_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetLastErrorMessage
#define mclGetLastErrorMessage mclGetLastErrorMessage_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetStackTrace
#define mclGetStackTrace mclGetStackTrace_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclFreeStackTrace
#define mclFreeStackTrace mclFreeStackTrace_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclAcquireMutex
#define mclAcquireMutex mclAcquireMutex_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclReleaseMutex
#define mclReleaseMutex mclReleaseMutex_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclIsMCRInitialized
#define mclIsMCRInitialized mclIsMCRInitialized_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclIsJVMEnabled
#define mclIsJVMEnabled mclIsJVMEnabled_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetLogFileName
#define mclGetLogFileName mclGetLogFileName_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclIsNoDisplaySet
#define mclIsNoDisplaySet mclIsNoDisplaySet_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclInitializeApplicationInternal
#define mclInitializeApplicationInternal mclInitializeApplicationInternal_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclTerminateApplication
#define mclTerminateApplication mclTerminateApplication_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclIsMcc
#define mclIsMcc mclIsMcc_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef separatePathName
#define separatePathName separatePathName_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclFreeStrArray
#define mclFreeStrArray mclFreeStrArray_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclFreeArrayList
#define mclFreeArrayList mclFreeArrayList_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclCreateCellArrayFromArrayList
#define mclCreateCellArrayFromArrayList mclCreateCellArrayFromArrayList_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclCreateSharedCopy
#define mclCreateSharedCopy mclCreateSharedCopy_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclCreateEmptyArray
#define mclCreateEmptyArray mclCreateEmptyArray_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclCreateSimpleFunctionHandle
#define mclCreateSimpleFunctionHandle mclCreateSimpleFunctionHandle_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclMxSerialize
#define mclMxSerialize mclMxSerialize_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclMxDeserialize
#define mclMxDeserialize mclMxDeserialize_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclMxDestroyArray
#define mclMxDestroyArray mclMxDestroyArray_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclMxIsA
#define mclMxIsA mclMxIsA_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclMxIsRef
#define mclMxIsRef mclMxIsRef_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclMxRefIsA
#define mclMxRefIsA mclMxRefIsA_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclMxReleaseRef
#define mclMxReleaseRef mclMxReleaseRef_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclMxRefLocalMvm
#define mclMxRefLocalMvm mclMxRefLocalMvm_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclMxEnterNewArrayListContext
#define mclMxEnterNewArrayListContext mclMxEnterNewArrayListContext_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclMxApplyToAllArraysOnArrayList
#define mclMxApplyToAllArraysOnArrayList mclMxApplyToAllArraysOnArrayList_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclMxExitArrayListContext
#define mclMxExitArrayListContext mclMxExitArrayListContext_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclMakeMxArrayLocalScope
#define mclMakeMxArrayLocalScope mclMakeMxArrayLocalScope_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclRunMain
#define mclRunMain mclRunMain_proxy
#endif



/*#ifdef mclbase_h
#error "mclmcrrt.h must be included before mclbase.h. (Since mclmcrrt.h includes mclbase.h, additional inclusion is redundant.)"
#endif */
#define LIBMWMCLBASE_API_EXTERN_C EXTERN_C
#include "mclbase.h"

/* Proxies for functions in mclbase.h */

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
void mclSetLastErrIdAndMsg_proxy(const char *a0, const char *a1);

EXTERN_C
const char * mclGetLastErrorMessage_proxy();

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclGetStackTrace_proxy(char ***a0);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclFreeStackTrace_proxy(char ***a0, int a1);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
void mclAcquireMutex_proxy();

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
void mclReleaseMutex_proxy();

EXTERN_C
bool mclIsMCRInitialized_proxy();

EXTERN_C
bool mclIsJVMEnabled_proxy();

EXTERN_C
const char * mclGetLogFileName_proxy();

EXTERN_C
bool mclIsNoDisplaySet_proxy();

EXTERN_C
bool mclInitializeApplicationInternal_proxy(const char **a0, size_t a1);

EXTERN_C
bool mclTerminateApplication_proxy();

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool mclIsMcc_proxy();

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
void separatePathName_proxy(const char *a0, char *a1, size_t a2);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool mclFreeStrArray_proxy(char **a0, size_t a1);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
void mclFreeArrayList_proxy(int a0, mxArray **a1);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
mxArray * mclCreateCellArrayFromArrayList_proxy(int a0, mxArray **a1);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
mxArray * mclCreateSharedCopy_proxy(mxArray *a0);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
mxArray * mclCreateEmptyArray_proxy();

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
mxArray * mclCreateSimpleFunctionHandle_proxy(mxFunctionPtr a0);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
mxArray * mclMxSerialize_proxy(const mxArray *a0);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
mxArray * mclMxDeserialize_proxy(const void *a0, size_t a1);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
void mclMxDestroyArray_proxy(mxArray *a0, bool a1);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool mclMxIsA_proxy(mxArray *a0, const char *a1);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool mclMxIsRef_proxy(mxArray *a0);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool mclMxRefIsA_proxy(mxArray *a0, const char *a1);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
mxArray * mclMxReleaseRef_proxy(mxArray *a0);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
MVMID_t mclMxRefLocalMvm_proxy(mxArray *a0);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclMxEnterNewArrayListContext_proxy();

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
void mclMxApplyToAllArraysOnArrayList_proxy(mclMxArrayApplyFcn a0);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
void mclMxExitArrayListContext_proxy(int a0, mxArray **a1, int a2, 
    bool a3);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
void mclMakeMxArrayLocalScope_proxy(mxArray *a0);

EXTERN_C
int mclRunMain_proxy(mclMainFcnType a0, int a1, const char **a2);



#ifndef MW_CALL_CONV
#  ifdef _WIN32 
#      define MW_CALL_CONV __cdecl
#  else
#      define MW_CALL_CONV 
#  endif
#endif

/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetStreamFromArraySrc
#define mclGetStreamFromArraySrc mclGetStreamFromArraySrc_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclDestroyStream
#define mclDestroyStream mclDestroyStream_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetEmbeddedCtfStream
#define mclGetEmbeddedCtfStream mclGetEmbeddedCtfStream_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclInitializeComponentInstanceNonEmbeddedStandalone
#define mclInitializeComponentInstanceNonEmbeddedStandalone mclInitializeComponentInstanceNonEmbeddedStandalone_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclInitializeInstanceWithoutComponent
#define mclInitializeInstanceWithoutComponent mclInitializeInstanceWithoutComponent_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclInitializeComponentInstanceCtfFileToCache
#define mclInitializeComponentInstanceCtfFileToCache mclInitializeComponentInstanceCtfFileToCache_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclInitializeComponentInstanceEmbedded
#define mclInitializeComponentInstanceEmbedded mclInitializeComponentInstanceEmbedded_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclInitializeComponentInstanceWithCallbk
#define mclInitializeComponentInstanceWithCallbk mclInitializeComponentInstanceWithCallbk_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclInitializeComponentInstanceFromExtractedComponent
#define mclInitializeComponentInstanceFromExtractedComponent mclInitializeComponentInstanceFromExtractedComponent_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclInitializeComponentInstanceFromExtractedLocation
#define mclInitializeComponentInstanceFromExtractedLocation mclInitializeComponentInstanceFromExtractedLocation_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetDotNetComponentType
#define mclGetDotNetComponentType mclGetDotNetComponentType_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetMCCTargetType
#define mclGetMCCTargetType mclGetMCCTargetType_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef getStandaloneFileName
#define getStandaloneFileName getStandaloneFileName_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclStandaloneGenericMain
#define mclStandaloneGenericMain mclStandaloneGenericMain_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclStandaloneCtfxMain
#define mclStandaloneCtfxMain mclStandaloneCtfxMain_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclWaitForFiguresToDie
#define mclWaitForFiguresToDie mclWaitForFiguresToDie_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclcppGetLastError
#define mclcppGetLastError mclcppGetLastError_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclcppCreateError
#define mclcppCreateError mclcppCreateError_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclcppSetLastError
#define mclcppSetLastError mclcppSetLastError_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclcppErrorCheck
#define mclcppErrorCheck mclcppErrorCheck_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclCreateCharBuffer
#define mclCreateCharBuffer mclCreateCharBuffer_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetEps
#define mclGetEps mclGetEps_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetInf
#define mclGetInf mclGetInf_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetNaN
#define mclGetNaN mclGetNaN_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclIsFinite
#define mclIsFinite mclIsFinite_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclIsInf
#define mclIsInf mclIsInf_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclIsNaN
#define mclIsNaN mclIsNaN_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclIsIdentical
#define mclIsIdentical mclIsIdentical_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetEmptyArray
#define mclGetEmptyArray mclGetEmptyArray_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetMatrix
#define mclGetMatrix mclGetMatrix_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetArray
#define mclGetArray mclGetArray_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetNumericMatrix
#define mclGetNumericMatrix mclGetNumericMatrix_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetNumericArray
#define mclGetNumericArray mclGetNumericArray_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetScalarDouble
#define mclGetScalarDouble mclGetScalarDouble_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetScalarSingle
#define mclGetScalarSingle mclGetScalarSingle_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetScalarInt8
#define mclGetScalarInt8 mclGetScalarInt8_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetScalarUint8
#define mclGetScalarUint8 mclGetScalarUint8_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetScalarInt16
#define mclGetScalarInt16 mclGetScalarInt16_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetScalarUint16
#define mclGetScalarUint16 mclGetScalarUint16_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetScalarInt32
#define mclGetScalarInt32 mclGetScalarInt32_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetScalarUint32
#define mclGetScalarUint32 mclGetScalarUint32_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetScalarInt64
#define mclGetScalarInt64 mclGetScalarInt64_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetScalarUint64
#define mclGetScalarUint64 mclGetScalarUint64_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetCharMatrix
#define mclGetCharMatrix mclGetCharMatrix_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetCharArray
#define mclGetCharArray mclGetCharArray_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetScalarChar
#define mclGetScalarChar mclGetScalarChar_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetString
#define mclGetString mclGetString_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetCharMatrixFromStrings
#define mclGetCharMatrixFromStrings mclGetCharMatrixFromStrings_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetLogicalMatrix
#define mclGetLogicalMatrix mclGetLogicalMatrix_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetLogicalArray
#define mclGetLogicalArray mclGetLogicalArray_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetScalarLogical
#define mclGetScalarLogical mclGetScalarLogical_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetCellMatrix
#define mclGetCellMatrix mclGetCellMatrix_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetCellArray
#define mclGetCellArray mclGetCellArray_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetStructMatrix
#define mclGetStructMatrix mclGetStructMatrix_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetStructArray
#define mclGetStructArray mclGetStructArray_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetNumericSparse
#define mclGetNumericSparse mclGetNumericSparse_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetNumericSparseInferRowsCols
#define mclGetNumericSparseInferRowsCols mclGetNumericSparseInferRowsCols_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetLogicalSparse
#define mclGetLogicalSparse mclGetLogicalSparse_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetLogicalSparseInferRowsCols
#define mclGetLogicalSparseInferRowsCols mclGetLogicalSparseInferRowsCols_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclDeserializeArray
#define mclDeserializeArray mclDeserializeArray_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclcppGetArrayBuffer
#define mclcppGetArrayBuffer mclcppGetArrayBuffer_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclcppFeval
#define mclcppFeval mclcppFeval_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclcppArrayToString
#define mclcppArrayToString mclcppArrayToString_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclcppFreeString
#define mclcppFreeString mclcppFreeString_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclmxArray2ArrayHandle
#define mclmxArray2ArrayHandle mclmxArray2ArrayHandle_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclArrayHandle2mxArray
#define mclArrayHandle2mxArray mclArrayHandle2mxArray_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclMXArrayGetIndexArrays
#define mclMXArrayGetIndexArrays mclMXArrayGetIndexArrays_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclMXArrayGet
#define mclMXArrayGet mclMXArrayGet_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclMXArrayGetReal
#define mclMXArrayGetReal mclMXArrayGetReal_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclMXArrayGetImag
#define mclMXArrayGetImag mclMXArrayGetImag_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclMXArraySet
#define mclMXArraySet mclMXArraySet_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclMXArraySetReal
#define mclMXArraySetReal mclMXArraySetReal_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclMXArraySetImag
#define mclMXArraySetImag mclMXArraySetImag_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclMXArraySetLogical
#define mclMXArraySetLogical mclMXArraySetLogical_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclMxRefDestroyArray
#define mclMxRefDestroyArray mclMxRefDestroyArray_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclMxRefSerialize
#define mclMxRefSerialize mclMxRefSerialize_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclMxRefDeserialize
#define mclMxRefDeserialize mclMxRefDeserialize_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclMxRefMvmId
#define mclMxRefMvmId mclMxRefMvmId_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclHashNBytes
#define mclHashNBytes mclHashNBytes_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclWrite
#define mclWrite mclWrite_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclAddCanonicalPathMacro
#define mclAddCanonicalPathMacro mclAddCanonicalPathMacro_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclFevalInternal
#define mclFevalInternal mclFevalInternal_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetMaxPathLen
#define mclGetMaxPathLen mclGetMaxPathLen_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclmcrInitializeInternal2
#define mclmcrInitializeInternal2 mclmcrInitializeInternal2_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclmcrInitializeInternal
#define mclmcrInitializeInternal mclmcrInitializeInternal_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef deleteWcsStackPointer_hPtr
#define deleteWcsStackPointer_hPtr deleteWcsStackPointer_hPtr_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef initializeWcsStackPointer
#define initializeWcsStackPointer initializeWcsStackPointer_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef deleteWcsStackPointer
#define deleteWcsStackPointer deleteWcsStackPointer_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef allocWcsStackPointer
#define allocWcsStackPointer allocWcsStackPointer_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mwMbstowcs
#define mwMbstowcs mwMbstowcs_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef utf16_to_lcp_n_fcn
#define utf16_to_lcp_n_fcn utf16_to_lcp_n_fcn_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef utf16_strlen_fcn
#define utf16_strlen_fcn utf16_strlen_fcn_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef utf16_strncpy_fcn
#define utf16_strncpy_fcn utf16_strncpy_fcn_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef utf16_strdup_fcn
#define utf16_strdup_fcn utf16_strdup_fcn_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclSetGlobal
#define mclSetGlobal mclSetGlobal_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclIsStandaloneMode
#define mclIsStandaloneMode mclIsStandaloneMode_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclImpersonationFeval
#define mclImpersonationFeval mclImpersonationFeval_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetGlobal
#define mclGetGlobal mclGetGlobal_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetID
#define mclGetID mclGetID_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclMain
#define mclMain mclMain_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclMlfVFevalInternal
#define mclMlfVFevalInternal mclMlfVFevalInternal_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclRegisterEventFunction
#define mclRegisterEventFunction mclRegisterEventFunction_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclRegisterExternalFunction
#define mclRegisterExternalFunction mclRegisterExternalFunction_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetMCRVersion
#define mclGetMCRVersion mclGetMCRVersion_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetActiveID
#define mclGetActiveID mclGetActiveID_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclGetTempFileName
#define mclGetTempFileName mclGetTempFileName_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mclTerminateInstance
#define mclTerminateInstance mclTerminateInstance_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef stopImpersonationOnMCRThread
#define stopImpersonationOnMCRThread stopImpersonationOnMCRThread_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef ref_count_obj_addref
#define ref_count_obj_addref ref_count_obj_addref_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef ref_count_obj_release
#define ref_count_obj_release ref_count_obj_release_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef char_buffer_size
#define char_buffer_size char_buffer_size_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef char_buffer_get_buffer
#define char_buffer_get_buffer char_buffer_get_buffer_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef char_buffer_set_buffer
#define char_buffer_set_buffer char_buffer_set_buffer_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef char_buffer_compare_to
#define char_buffer_compare_to char_buffer_compare_to_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_classID
#define array_ref_classID array_ref_classID_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_deep_copy
#define array_ref_deep_copy array_ref_deep_copy_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_detach
#define array_ref_detach array_ref_detach_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_shared_copy
#define array_ref_shared_copy array_ref_shared_copy_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_serialize
#define array_ref_serialize array_ref_serialize_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_element_size
#define array_ref_element_size array_ref_element_size_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_number_of_elements
#define array_ref_number_of_elements array_ref_number_of_elements_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_number_of_nonzeros
#define array_ref_number_of_nonzeros array_ref_number_of_nonzeros_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_maximum_nonzeros
#define array_ref_maximum_nonzeros array_ref_maximum_nonzeros_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_number_of_dimensions
#define array_ref_number_of_dimensions array_ref_number_of_dimensions_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_get_dimensions
#define array_ref_get_dimensions array_ref_get_dimensions_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_number_of_fields
#define array_ref_number_of_fields array_ref_number_of_fields_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_get_field_name
#define array_ref_get_field_name array_ref_get_field_name_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_is_empty
#define array_ref_is_empty array_ref_is_empty_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_is_sparse
#define array_ref_is_sparse array_ref_is_sparse_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_is_numeric
#define array_ref_is_numeric array_ref_is_numeric_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_is_complex
#define array_ref_is_complex array_ref_is_complex_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_make_complex
#define array_ref_make_complex array_ref_make_complex_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_equals
#define array_ref_equals array_ref_equals_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_compare_to
#define array_ref_compare_to array_ref_compare_to_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_hash_code
#define array_ref_hash_code array_ref_hash_code_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_to_string
#define array_ref_to_string array_ref_to_string_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_row_index
#define array_ref_row_index array_ref_row_index_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_column_index
#define array_ref_column_index array_ref_column_index_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_get_int
#define array_ref_get_int array_ref_get_int_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_get_const_char
#define array_ref_get_const_char array_ref_get_const_char_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_getV_int
#define array_ref_getV_int array_ref_getV_int_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_getV_const_char
#define array_ref_getV_const_char array_ref_getV_const_char_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_set
#define array_ref_set array_ref_set_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_real
#define array_ref_real array_ref_real_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_imag
#define array_ref_imag array_ref_imag_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_get_numeric_mxDouble
#define array_ref_get_numeric_mxDouble array_ref_get_numeric_mxDouble_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_get_numeric_mxSingle
#define array_ref_get_numeric_mxSingle array_ref_get_numeric_mxSingle_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_get_numeric_mxInt8
#define array_ref_get_numeric_mxInt8 array_ref_get_numeric_mxInt8_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_get_numeric_mxUint8
#define array_ref_get_numeric_mxUint8 array_ref_get_numeric_mxUint8_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_get_numeric_mxInt16
#define array_ref_get_numeric_mxInt16 array_ref_get_numeric_mxInt16_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_get_numeric_mxUint16
#define array_ref_get_numeric_mxUint16 array_ref_get_numeric_mxUint16_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_get_numeric_mxInt32
#define array_ref_get_numeric_mxInt32 array_ref_get_numeric_mxInt32_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_get_numeric_mxUint32
#define array_ref_get_numeric_mxUint32 array_ref_get_numeric_mxUint32_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_get_numeric_mxInt64
#define array_ref_get_numeric_mxInt64 array_ref_get_numeric_mxInt64_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_get_numeric_mxUint64
#define array_ref_get_numeric_mxUint64 array_ref_get_numeric_mxUint64_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_get_char
#define array_ref_get_char array_ref_get_char_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_get_logical
#define array_ref_get_logical array_ref_get_logical_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_set_numeric_mxDouble
#define array_ref_set_numeric_mxDouble array_ref_set_numeric_mxDouble_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_set_numeric_mxSingle
#define array_ref_set_numeric_mxSingle array_ref_set_numeric_mxSingle_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_set_numeric_mxInt8
#define array_ref_set_numeric_mxInt8 array_ref_set_numeric_mxInt8_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_set_numeric_mxUint8
#define array_ref_set_numeric_mxUint8 array_ref_set_numeric_mxUint8_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_set_numeric_mxInt16
#define array_ref_set_numeric_mxInt16 array_ref_set_numeric_mxInt16_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_set_numeric_mxUint16
#define array_ref_set_numeric_mxUint16 array_ref_set_numeric_mxUint16_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_set_numeric_mxInt32
#define array_ref_set_numeric_mxInt32 array_ref_set_numeric_mxInt32_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_set_numeric_mxUint32
#define array_ref_set_numeric_mxUint32 array_ref_set_numeric_mxUint32_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_set_numeric_mxInt64
#define array_ref_set_numeric_mxInt64 array_ref_set_numeric_mxInt64_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_set_numeric_mxUint64
#define array_ref_set_numeric_mxUint64 array_ref_set_numeric_mxUint64_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_set_char
#define array_ref_set_char array_ref_set_char_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_ref_set_logical
#define array_ref_set_logical array_ref_set_logical_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_buffer_size
#define array_buffer_size array_buffer_size_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_buffer_get
#define array_buffer_get array_buffer_get_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_buffer_set
#define array_buffer_set array_buffer_set_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_buffer_add
#define array_buffer_add array_buffer_add_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_buffer_remove
#define array_buffer_remove array_buffer_remove_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_buffer_clear
#define array_buffer_clear array_buffer_clear_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_buffer_to_cell
#define array_buffer_to_cell array_buffer_to_cell_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef error_info_get_message
#define error_info_get_message error_info_get_message_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef error_info_get_stack_trace
#define error_info_get_stack_trace error_info_get_stack_trace_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_classID
#define array_handle_classID array_handle_classID_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_deep_copy
#define array_handle_deep_copy array_handle_deep_copy_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_detach
#define array_handle_detach array_handle_detach_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_shared_copy
#define array_handle_shared_copy array_handle_shared_copy_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_serialize
#define array_handle_serialize array_handle_serialize_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_element_size
#define array_handle_element_size array_handle_element_size_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_number_of_elements
#define array_handle_number_of_elements array_handle_number_of_elements_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_number_of_nonzeros
#define array_handle_number_of_nonzeros array_handle_number_of_nonzeros_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_maximum_nonzeros
#define array_handle_maximum_nonzeros array_handle_maximum_nonzeros_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_number_of_dimensions
#define array_handle_number_of_dimensions array_handle_number_of_dimensions_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_get_dimensions
#define array_handle_get_dimensions array_handle_get_dimensions_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_number_of_fields
#define array_handle_number_of_fields array_handle_number_of_fields_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_get_field_name
#define array_handle_get_field_name array_handle_get_field_name_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_is_empty
#define array_handle_is_empty array_handle_is_empty_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_is_sparse
#define array_handle_is_sparse array_handle_is_sparse_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_is_numeric
#define array_handle_is_numeric array_handle_is_numeric_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_is_complex
#define array_handle_is_complex array_handle_is_complex_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_make_complex
#define array_handle_make_complex array_handle_make_complex_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_equals
#define array_handle_equals array_handle_equals_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_compare_to
#define array_handle_compare_to array_handle_compare_to_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_hash_code
#define array_handle_hash_code array_handle_hash_code_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_to_string
#define array_handle_to_string array_handle_to_string_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_row_index
#define array_handle_row_index array_handle_row_index_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_column_index
#define array_handle_column_index array_handle_column_index_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_get_int
#define array_handle_get_int array_handle_get_int_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_get_const_char
#define array_handle_get_const_char array_handle_get_const_char_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_getV_int
#define array_handle_getV_int array_handle_getV_int_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_getV_const_char
#define array_handle_getV_const_char array_handle_getV_const_char_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_set
#define array_handle_set array_handle_set_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_real
#define array_handle_real array_handle_real_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_imag
#define array_handle_imag array_handle_imag_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_get_numeric_mxDouble
#define array_handle_get_numeric_mxDouble array_handle_get_numeric_mxDouble_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_get_numeric_mxSingle
#define array_handle_get_numeric_mxSingle array_handle_get_numeric_mxSingle_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_get_numeric_mxInt8
#define array_handle_get_numeric_mxInt8 array_handle_get_numeric_mxInt8_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_get_numeric_mxUint8
#define array_handle_get_numeric_mxUint8 array_handle_get_numeric_mxUint8_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_get_numeric_mxInt16
#define array_handle_get_numeric_mxInt16 array_handle_get_numeric_mxInt16_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_get_numeric_mxUint16
#define array_handle_get_numeric_mxUint16 array_handle_get_numeric_mxUint16_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_get_numeric_mxInt32
#define array_handle_get_numeric_mxInt32 array_handle_get_numeric_mxInt32_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_get_numeric_mxUint32
#define array_handle_get_numeric_mxUint32 array_handle_get_numeric_mxUint32_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_get_numeric_mxInt64
#define array_handle_get_numeric_mxInt64 array_handle_get_numeric_mxInt64_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_get_numeric_mxUint64
#define array_handle_get_numeric_mxUint64 array_handle_get_numeric_mxUint64_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_get_char
#define array_handle_get_char array_handle_get_char_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_get_logical
#define array_handle_get_logical array_handle_get_logical_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_set_numeric_mxDouble
#define array_handle_set_numeric_mxDouble array_handle_set_numeric_mxDouble_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_set_numeric_mxSingle
#define array_handle_set_numeric_mxSingle array_handle_set_numeric_mxSingle_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_set_numeric_mxInt8
#define array_handle_set_numeric_mxInt8 array_handle_set_numeric_mxInt8_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_set_numeric_mxUint8
#define array_handle_set_numeric_mxUint8 array_handle_set_numeric_mxUint8_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_set_numeric_mxInt16
#define array_handle_set_numeric_mxInt16 array_handle_set_numeric_mxInt16_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_set_numeric_mxUint16
#define array_handle_set_numeric_mxUint16 array_handle_set_numeric_mxUint16_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_set_numeric_mxInt32
#define array_handle_set_numeric_mxInt32 array_handle_set_numeric_mxInt32_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_set_numeric_mxUint32
#define array_handle_set_numeric_mxUint32 array_handle_set_numeric_mxUint32_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_set_numeric_mxInt64
#define array_handle_set_numeric_mxInt64 array_handle_set_numeric_mxInt64_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_set_numeric_mxUint64
#define array_handle_set_numeric_mxUint64 array_handle_set_numeric_mxUint64_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_set_char
#define array_handle_set_char array_handle_set_char_proxy
#endif



/**This function is for INTERNAL USE ONLY.**/
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef array_handle_set_logical
#define array_handle_set_logical array_handle_set_logical_proxy
#endif



/*#ifdef mclmcr_h
#error "mclmcrrt.h must be included before mclmcr.h. (Since mclmcrrt.h includes mclmcr.h, additional inclusion is redundant.)"
#endif */
#define LIBMWMCLMCR_API_EXTERN_C EXTERN_C
#include "mclmcr.h"

/* Proxies for functions in mclmcr.h */

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
mclCtfStream mclGetStreamFromArraySrc_proxy(char *a0, int a1);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
void mclDestroyStream_proxy(mclCtfStream a0);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
mclCtfStream mclGetEmbeddedCtfStream_proxy(void *a0);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool mclInitializeComponentInstanceNonEmbeddedStandalone_proxy(
    HMCRINSTANCE *a0, const char *a1, const char *a2, mccTargetType a3, 
    mclOutputHandlerFcn a4, mclOutputHandlerFcn a5);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool mclInitializeInstanceWithoutComponent_proxy(HMCRINSTANCE *a0, 
    const char **a1, size_t a2, mclOutputHandlerFcn a3, 
    mclOutputHandlerFcn a4);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool mclInitializeComponentInstanceCtfFileToCache_proxy(HMCRINSTANCE *a0, 
    mclOutputHandlerFcn a1, mclOutputHandlerFcn a2, const char *a3);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool mclInitializeComponentInstanceEmbedded_proxy(HMCRINSTANCE *a0, 
    mclOutputHandlerFcn a1, mclOutputHandlerFcn a2, mclCtfStream a3);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool mclInitializeComponentInstanceWithCallbk_proxy(HMCRINSTANCE *a0, 
    mclOutputHandlerFcn a1, mclOutputHandlerFcn a2, 
    mclReadCtfStreamFcn a3, size_t a4);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool mclInitializeComponentInstanceFromExtractedComponent_proxy(
    HMCRINSTANCE *a0, mclOutputHandlerFcn a1, mclOutputHandlerFcn a2, 
    const char *a3);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool mclInitializeComponentInstanceFromExtractedLocation_proxy(
    HMCRINSTANCE *a0, mclOutputHandlerFcn a1, mclOutputHandlerFcn a2, 
    const char *a3);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclGetDotNetComponentType_proxy();

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclGetMCCTargetType_proxy(bool a0);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
const char * getStandaloneFileName_proxy(const char *a0, const char *a1);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool mclStandaloneGenericMain_proxy(size_t a0, const char **a1, 
    const char *a2, bool a3, void *a4);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool mclStandaloneCtfxMain_proxy(size_t a0, const char **a1);

EXTERN_C
void mclWaitForFiguresToDie_proxy(HMCRINSTANCE a0);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclcppGetLastError_proxy(void **a0);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclcppCreateError_proxy(void **a0, const char *a1);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
void mclcppSetLastError_proxy(const char *a0);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclcppErrorCheck_proxy();

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclCreateCharBuffer_proxy(void **a0, const char *a1);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
double mclGetEps_proxy();

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
double mclGetInf_proxy();

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
double mclGetNaN_proxy();

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool mclIsFinite_proxy(double a0);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool mclIsInf_proxy(double a0);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool mclIsNaN_proxy(double a0);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool mclIsIdentical_proxy(mxArray *a0, mxArray *a1);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclGetEmptyArray_proxy(void **a0, mxClassID a1);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclGetMatrix_proxy(void **a0, mwSize a1, mwSize a2, mxClassID a3, 
    mxComplexity a4);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclGetArray_proxy(void **a0, mwSize a1, const mwSize *a2, 
    mxClassID a3, mxComplexity a4);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclGetNumericMatrix_proxy(void **a0, mwSize a1, mwSize a2, 
    mxClassID a3, mxComplexity a4);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclGetNumericArray_proxy(void **a0, mwSize a1, const mwSize *a2, 
    mxClassID a3, mxComplexity a4);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclGetScalarDouble_proxy(void **a0, mxDouble a1, mxDouble a2, 
    mxComplexity a3);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclGetScalarSingle_proxy(void **a0, mxSingle a1, mxSingle a2, 
    mxComplexity a3);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclGetScalarInt8_proxy(void **a0, mxInt8 a1, mxInt8 a2, 
    mxComplexity a3);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclGetScalarUint8_proxy(void **a0, mxUint8 a1, mxUint8 a2, 
    mxComplexity a3);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclGetScalarInt16_proxy(void **a0, mxInt16 a1, mxInt16 a2, 
    mxComplexity a3);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclGetScalarUint16_proxy(void **a0, mxUint16 a1, mxUint16 a2, 
    mxComplexity a3);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclGetScalarInt32_proxy(void **a0, mxInt32 a1, mxInt32 a2, 
    mxComplexity a3);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclGetScalarUint32_proxy(void **a0, mxUint32 a1, mxUint32 a2, 
    mxComplexity a3);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclGetScalarInt64_proxy(void **a0, mxInt64 a1, mxInt64 a2, 
    mxComplexity a3);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclGetScalarUint64_proxy(void **a0, mxUint64 a1, mxUint64 a2, 
    mxComplexity a3);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclGetCharMatrix_proxy(void **a0, mwSize a1, mwSize a2);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclGetCharArray_proxy(void **a0, mwSize a1, const mwSize *a2);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclGetScalarChar_proxy(void **a0, mxChar a1);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclGetString_proxy(void **a0, const char *a1);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclGetCharMatrixFromStrings_proxy(void **a0, mwSize a1, 
    const char **a2);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclGetLogicalMatrix_proxy(void **a0, mwSize a1, mwSize a2);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclGetLogicalArray_proxy(void **a0, mwSize a1, const mwSize *a2);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclGetScalarLogical_proxy(void **a0, mxLogical a1);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclGetCellMatrix_proxy(void **a0, mwSize a1, mwSize a2);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclGetCellArray_proxy(void **a0, mwSize a1, const mwSize *a2);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclGetStructMatrix_proxy(void **a0, mwSize a1, mwSize a2, int a3, 
    const char **a4);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclGetStructArray_proxy(void **a0, mwSize a1, const mwSize *a2, 
    int a3, const char **a4);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclGetNumericSparse_proxy(void **a0, mwSize a1, const mwSize *a2, 
    mwSize a3, const mwSize *a4, mwSize a5, const void *a6, 
    const void *a7, mwSize a8, mwSize a9, mwSize a10, mxClassID a11, 
    mxComplexity a12);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclGetNumericSparseInferRowsCols_proxy(void **a0, mwSize a1, 
    const mwSize *a2, mwSize a3, const mwSize *a4, mwSize a5, 
    const void *a6, const void *a7, mwSize a8, mxClassID a9, 
    mxComplexity a10);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclGetLogicalSparse_proxy(void **a0, mwSize a1, const mwIndex *a2, 
    mwSize a3, const mwIndex *a4, mwSize a5, const mxLogical *a6, 
    mwSize a7, mwSize a8, mwSize a9);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclGetLogicalSparseInferRowsCols_proxy(void **a0, mwSize a1, 
    const mwIndex *a2, mwSize a3, const mwIndex *a4, mwSize a5, 
    const mxLogical *a6, mwSize a7);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclDeserializeArray_proxy(void **a0, void **a1);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclcppGetArrayBuffer_proxy(void **a0, mwSize a1);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclcppFeval_proxy(HMCRINSTANCE a0, const char *a1, int a2, void **a3, 
    void *a4);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclcppArrayToString_proxy(const mxArray *a0, char **a1);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
void mclcppFreeString_proxy(char *a0);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclmxArray2ArrayHandle_proxy(void **a0, mxArray *a1);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclArrayHandle2mxArray_proxy(mxArray **a0, void *a1);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclMXArrayGetIndexArrays_proxy(mxArray **a0, mxArray **a1, 
    mxArray *a2);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclMXArrayGet_proxy(mxArray **a0, mxArray *a1, mwSize a2, 
    const mwIndex *a3);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclMXArrayGetReal_proxy(mxArray **a0, mxArray *a1, mwSize a2, 
    const mwIndex *a3);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclMXArrayGetImag_proxy(mxArray **a0, mxArray *a1, mwSize a2, 
    const mwIndex *a3);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclMXArraySet_proxy(mxArray *a0, mxArray *a1, mwSize a2, 
    const mwIndex *a3);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclMXArraySetReal_proxy(mxArray *a0, mxArray *a1, mwSize a2, 
    const mwIndex *a3);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclMXArraySetImag_proxy(mxArray *a0, mxArray *a1, mwSize a2, 
    const mwIndex *a3);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclMXArraySetLogical_proxy(mxArray *a0, mxArray *a1, mwSize a2, 
    const mwIndex *a3);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
void mclMxRefDestroyArray_proxy(mxArray *a0);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
mxArray * mclMxRefSerialize_proxy(mxArray *a0);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
mxArray * mclMxRefDeserialize_proxy(const void *a0, size_t a1, size_t a2);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
size_t mclMxRefMvmId_proxy(mxArray *a0);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
size_t mclHashNBytes_proxy(size_t a0, size_t a1, const char *a2);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclWrite_proxy(int a0, const void *a1, size_t a2);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
void mclAddCanonicalPathMacro_proxy(const char *a0, const char *a1);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool mclFevalInternal_proxy(HMCRINSTANCE a0, const char *a1, int a2, 
    mxArray **a3, int a4, mxArray **a5);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclGetMaxPathLen_proxy();

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool mclmcrInitializeInternal2_proxy(int a0);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool mclmcrInitializeInternal_proxy();

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
void deleteWcsStackPointer_hPtr_proxy(pwcsStackPointer a0);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
void initializeWcsStackPointer_proxy(pwcsStackPointer *a0);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
void deleteWcsStackPointer_proxy(pwcsStackPointer a0);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool allocWcsStackPointer_proxy(pwcsStackPointer *a0, int a1);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mwMbstowcs_proxy(pwcsStackPointer a0, const char *a1);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
void utf16_to_lcp_n_fcn_proxy(char *a0, int32_t *a1, const CHAR16_T *a2, 
    int32_t a3);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int32_t utf16_strlen_fcn_proxy(const CHAR16_T *a0);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
CHAR16_T * utf16_strncpy_fcn_proxy(CHAR16_T *a0, const CHAR16_T *a1, 
    int32_t a2);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
CHAR16_T * utf16_strdup_fcn_proxy(const CHAR16_T *a0);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool mclSetGlobal_proxy(HMCRINSTANCE a0, const char *a1, mxArray *a2);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool mclIsStandaloneMode_proxy();

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool mclImpersonationFeval_proxy(HMCRINSTANCE a0, const char *a1, int a2, 
    mxArray **a3, int a4, mxArray **a5, void *a6);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool mclGetGlobal_proxy(HMCRINSTANCE a0, const char *a1, mxArray **a2);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
long int mclGetID_proxy(HMCRINSTANCE a0);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int mclMain_proxy(HMCRINSTANCE a0, int a1, const char **a2, 
    const char *a3, int a4);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool mclMlfVFevalInternal_proxy(HMCRINSTANCE a0, const char *a1, int a2, 
    int a3, int a4, va_list a5);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool mclRegisterEventFunction_proxy(HMCRINSTANCE a0, const char *a1, 
    mclEventFunctionPtr a2);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool mclRegisterExternalFunction_proxy(HMCRINSTANCE a0, const char *a1, 
    mxFunctionPtr a2);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool mclGetMCRVersion_proxy(const char **a0);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
size_t mclGetActiveID_proxy();

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
char * mclGetTempFileName_proxy(char *a0);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool mclTerminateInstance_proxy(HMCRINSTANCE *a0);

/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
void stopImpersonationOnMCRThread_proxy();

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int ref_count_obj_addref_proxy(class ref_count_obj *a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int ref_count_obj_release_proxy(class ref_count_obj *a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
size_t char_buffer_size_proxy(class char_buffer *a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
const char * char_buffer_get_buffer_proxy(class char_buffer *a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int char_buffer_set_buffer_proxy(class char_buffer *a0, const char *a1);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int char_buffer_compare_to_proxy(class char_buffer *a0, 
    class char_buffer *a1);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
mxClassID array_ref_classID_proxy(class array_ref *a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
class array_ref * array_ref_deep_copy_proxy(class array_ref *a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
void array_ref_detach_proxy(class array_ref *a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
class array_ref * array_ref_shared_copy_proxy(class array_ref *a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
class array_ref * array_ref_serialize_proxy(class array_ref *a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
size_t array_ref_element_size_proxy(class array_ref *a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
mwSize array_ref_number_of_elements_proxy(class array_ref *a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
mwSize array_ref_number_of_nonzeros_proxy(class array_ref *a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
mwSize array_ref_maximum_nonzeros_proxy(class array_ref *a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
mwSize array_ref_number_of_dimensions_proxy(class array_ref *a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
class array_ref * array_ref_get_dimensions_proxy(class array_ref *a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_ref_number_of_fields_proxy(class array_ref *a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
class char_buffer * array_ref_get_field_name_proxy(class array_ref *a0, 
    int a1);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool array_ref_is_empty_proxy(class array_ref *a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool array_ref_is_sparse_proxy(class array_ref *a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool array_ref_is_numeric_proxy(class array_ref *a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool array_ref_is_complex_proxy(class array_ref *a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_ref_make_complex_proxy(class array_ref *a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool array_ref_equals_proxy(class array_ref *a0, class array_ref *a1);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_ref_compare_to_proxy(class array_ref *a0, class array_ref *a1);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_ref_hash_code_proxy(class array_ref *a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
class char_buffer * array_ref_to_string_proxy(class array_ref *a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
class array_ref * array_ref_row_index_proxy(class array_ref *a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
class array_ref * array_ref_column_index_proxy(class array_ref *a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
class array_ref * array_ref_get_int_proxy(class array_ref *a0, mwSize a1, 
    const mwIndex *a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
class array_ref * array_ref_get_const_char_proxy(class array_ref *a0, 
    const char *a1, mwSize a2, const mwIndex *a3);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
class array_ref * array_ref_getV_int_proxy(class array_ref *a0, 
    mwSize a1, va_list a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
class array_ref * array_ref_getV_const_char_proxy(class array_ref *a0, 
    const char *a1, mwSize a2, va_list a3);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_ref_set_proxy(class array_ref *a0, class array_ref *a1);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
class array_ref * array_ref_real_proxy(class array_ref *a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
class array_ref * array_ref_imag_proxy(class array_ref *a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_ref_get_numeric_mxDouble_proxy(class array_ref *a0, 
    mxDouble *a1, mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_ref_get_numeric_mxSingle_proxy(class array_ref *a0, 
    mxSingle *a1, mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_ref_get_numeric_mxInt8_proxy(class array_ref *a0, mxInt8 *a1, 
    mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_ref_get_numeric_mxUint8_proxy(class array_ref *a0, mxUint8 *a1, 
    mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_ref_get_numeric_mxInt16_proxy(class array_ref *a0, mxInt16 *a1, 
    mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_ref_get_numeric_mxUint16_proxy(class array_ref *a0, 
    mxUint16 *a1, mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_ref_get_numeric_mxInt32_proxy(class array_ref *a0, mxInt32 *a1, 
    mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_ref_get_numeric_mxUint32_proxy(class array_ref *a0, 
    mxUint32 *a1, mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_ref_get_numeric_mxInt64_proxy(class array_ref *a0, mxInt64 *a1, 
    mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_ref_get_numeric_mxUint64_proxy(class array_ref *a0, 
    mxUint64 *a1, mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_ref_get_char_proxy(class array_ref *a0, mxChar *a1, mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_ref_get_logical_proxy(class array_ref *a0, mxLogical *a1, 
    mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_ref_set_numeric_mxDouble_proxy(class array_ref *a0, 
    const mxDouble *a1, mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_ref_set_numeric_mxSingle_proxy(class array_ref *a0, 
    const mxSingle *a1, mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_ref_set_numeric_mxInt8_proxy(class array_ref *a0, 
    const mxInt8 *a1, mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_ref_set_numeric_mxUint8_proxy(class array_ref *a0, 
    const mxUint8 *a1, mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_ref_set_numeric_mxInt16_proxy(class array_ref *a0, 
    const mxInt16 *a1, mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_ref_set_numeric_mxUint16_proxy(class array_ref *a0, 
    const mxUint16 *a1, mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_ref_set_numeric_mxInt32_proxy(class array_ref *a0, 
    const mxInt32 *a1, mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_ref_set_numeric_mxUint32_proxy(class array_ref *a0, 
    const mxUint32 *a1, mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_ref_set_numeric_mxInt64_proxy(class array_ref *a0, 
    const mxInt64 *a1, mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_ref_set_numeric_mxUint64_proxy(class array_ref *a0, 
    const mxUint64 *a1, mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_ref_set_char_proxy(class array_ref *a0, const mxChar *a1, 
    mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_ref_set_logical_proxy(class array_ref *a0, const mxLogical *a1, 
    mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
mwSize array_buffer_size_proxy(class array_buffer *a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
class array_ref * array_buffer_get_proxy(class array_buffer *a0, 
    mwIndex a1);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_buffer_set_proxy(class array_buffer *a0, mwIndex a1, 
    class array_ref *a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_buffer_add_proxy(class array_buffer *a0, class array_ref *a1);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_buffer_remove_proxy(class array_buffer *a0, mwIndex a1);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_buffer_clear_proxy(class array_buffer *a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
class array_ref * array_buffer_to_cell_proxy(class array_buffer *a0, 
    mwIndex a1, mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
const char * error_info_get_message_proxy(class error_info *a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
size_t error_info_get_stack_trace_proxy(class error_info *a0, char ***a1);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
mxClassID array_handle_classID_proxy(array_handle a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
array_handle array_handle_deep_copy_proxy(array_handle a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
void array_handle_detach_proxy(array_handle a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
array_handle array_handle_shared_copy_proxy(array_handle a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
array_handle array_handle_serialize_proxy(array_handle a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
size_t array_handle_element_size_proxy(array_handle a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
mwSize array_handle_number_of_elements_proxy(array_handle a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
mwSize array_handle_number_of_nonzeros_proxy(array_handle a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
mwSize array_handle_maximum_nonzeros_proxy(array_handle a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
mwSize array_handle_number_of_dimensions_proxy(array_handle a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
array_handle array_handle_get_dimensions_proxy(array_handle a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_handle_number_of_fields_proxy(array_handle a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
class char_buffer * array_handle_get_field_name_proxy(array_handle a0, 
    int a1);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool array_handle_is_empty_proxy(array_handle a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool array_handle_is_sparse_proxy(array_handle a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool array_handle_is_numeric_proxy(array_handle a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool array_handle_is_complex_proxy(array_handle a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_handle_make_complex_proxy(array_handle a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
bool array_handle_equals_proxy(array_handle a0, array_handle a1);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_handle_compare_to_proxy(array_handle a0, array_handle a1);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_handle_hash_code_proxy(array_handle a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
class char_buffer * array_handle_to_string_proxy(array_handle a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
array_handle array_handle_row_index_proxy(array_handle a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
array_handle array_handle_column_index_proxy(array_handle a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
array_handle array_handle_get_int_proxy(array_handle a0, mwSize a1, 
    const mwIndex *a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
array_handle array_handle_get_const_char_proxy(array_handle a0, 
    const char *a1, mwSize a2, const mwIndex *a3);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
array_handle array_handle_getV_int_proxy(array_handle a0, mwSize a1, 
    va_list a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
array_handle array_handle_getV_const_char_proxy(array_handle a0, 
    const char *a1, mwSize a2, va_list a3);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_handle_set_proxy(array_handle a0, array_handle a1);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
array_handle array_handle_real_proxy(array_handle a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
array_handle array_handle_imag_proxy(array_handle a0);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_handle_get_numeric_mxDouble_proxy(array_handle a0, 
    mxDouble *a1, mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_handle_get_numeric_mxSingle_proxy(array_handle a0, 
    mxSingle *a1, mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_handle_get_numeric_mxInt8_proxy(array_handle a0, mxInt8 *a1, 
    mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_handle_get_numeric_mxUint8_proxy(array_handle a0, mxUint8 *a1, 
    mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_handle_get_numeric_mxInt16_proxy(array_handle a0, mxInt16 *a1, 
    mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_handle_get_numeric_mxUint16_proxy(array_handle a0, 
    mxUint16 *a1, mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_handle_get_numeric_mxInt32_proxy(array_handle a0, mxInt32 *a1, 
    mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_handle_get_numeric_mxUint32_proxy(array_handle a0, 
    mxUint32 *a1, mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_handle_get_numeric_mxInt64_proxy(array_handle a0, mxInt64 *a1, 
    mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_handle_get_numeric_mxUint64_proxy(array_handle a0, 
    mxUint64 *a1, mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_handle_get_char_proxy(array_handle a0, mxChar *a1, mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_handle_get_logical_proxy(array_handle a0, mxLogical *a1, 
    mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_handle_set_numeric_mxDouble_proxy(array_handle a0, 
    const mxDouble *a1, mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_handle_set_numeric_mxSingle_proxy(array_handle a0, 
    const mxSingle *a1, mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_handle_set_numeric_mxInt8_proxy(array_handle a0, 
    const mxInt8 *a1, mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_handle_set_numeric_mxUint8_proxy(array_handle a0, 
    const mxUint8 *a1, mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_handle_set_numeric_mxInt16_proxy(array_handle a0, 
    const mxInt16 *a1, mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_handle_set_numeric_mxUint16_proxy(array_handle a0, 
    const mxUint16 *a1, mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_handle_set_numeric_mxInt32_proxy(array_handle a0, 
    const mxInt32 *a1, mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_handle_set_numeric_mxUint32_proxy(array_handle a0, 
    const mxUint32 *a1, mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_handle_set_numeric_mxInt64_proxy(array_handle a0, 
    const mxInt64 *a1, mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_handle_set_numeric_mxUint64_proxy(array_handle a0, 
    const mxUint64 *a1, mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_handle_set_char_proxy(array_handle a0, const mxChar *a1, 
    mwSize a2);
#endif /* __cplusplus */

#ifdef __cplusplus /* Only available in C++ */
/**This function is for INTERNAL USE ONLY.**/
EXTERN_C
int array_handle_set_logical_proxy(array_handle a0, const mxLogical *a1, 
    mwSize a2);
#endif /* __cplusplus */


#if !defined(MW_BUILD_ARCHIVES)
#ifdef __cplusplus
extern "C"
{
#endif


#undef mclMlfVFeval
#define mclMlfVFeval mclMlfVFeval_proxy


#ifdef __cplusplus
}
#endif
#endif


#ifdef __cplusplus
extern "C"
{
#endif


#if !defined(MW_BUILD_ARCHIVES)
#undef mclFeval
#define mclFeval mclFeval_proxy
#endif


EXTERN_C
bool MW_CALL_CONV mclFeval_proxy(HMCRINSTANCE a0, const char *a1, int a2,
                                 mxArray **a3, int a4, mxArray **a5);



#ifdef __cplusplus
}
#endif


#ifdef __cplusplus
extern "C"
{
#endif


#if !defined(MW_BUILD_ARCHIVES)
#undef mclMlfFeval
#define mclMlfFeval mclMlfFeval_proxy
#endif


EXTERN_C
bool MW_CALL_CONV mclMlfFeval_proxy(HMCRINSTANCE a0, const char *a1,
		  	            int a2, int a3, int a4, ...);


#ifdef __cplusplus
}
#endif

#ifndef MW_CALL_CONV
#  ifdef _WIN32 
#      define MW_CALL_CONV __cdecl
#  else
#      define MW_CALL_CONV 
#  endif
#endif

#if defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetNumberOfDimensions_700
#define mxGetNumberOfDimensions_700 mxGetNumberOfDimensions_700_proxy
#endif
#endif   /* defined(MX_COMPAT_32) */
#if defined(__linux__)
#if defined(MX_COMPAT_32)
#undef mxGetNumberOfDimensions
#define mxGetNumberOfDimensions mxGetNumberOfDimensions_700
#endif
#endif




#if defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetDimensions_700
#define mxGetDimensions_700 mxGetDimensions_700_proxy
#endif
#endif   /* defined(MX_COMPAT_32) */
#if defined(__linux__)
#if defined(MX_COMPAT_32)
#undef mxGetDimensions
#define mxGetDimensions mxGetDimensions_700
#endif
#endif




#if defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetIr_700
#define mxGetIr_700 mxGetIr_700_proxy
#endif
#endif   /* defined(MX_COMPAT_32) */
#if defined(__linux__)
#if defined(MX_COMPAT_32)
#undef mxGetIr
#define mxGetIr mxGetIr_700
#endif
#endif




#if defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetJc_700
#define mxGetJc_700 mxGetJc_700_proxy
#endif
#endif   /* defined(MX_COMPAT_32) */
#if defined(__linux__)
#if defined(MX_COMPAT_32)
#undef mxGetJc
#define mxGetJc mxGetJc_700
#endif
#endif




#if defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetNzmax_700
#define mxGetNzmax_700 mxGetNzmax_700_proxy
#endif
#endif   /* defined(MX_COMPAT_32) */
#if defined(__linux__)
#if defined(MX_COMPAT_32)
#undef mxGetNzmax
#define mxGetNzmax mxGetNzmax_700
#endif
#endif




#if defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxSetNzmax_700
#define mxSetNzmax_700 mxSetNzmax_700_proxy
#endif
#endif   /* defined(MX_COMPAT_32) */
#if defined(__linux__)
#if defined(MX_COMPAT_32)
#undef mxSetNzmax
#define mxSetNzmax mxSetNzmax_700
#endif
#endif




#if defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetFieldByNumber_700
#define mxGetFieldByNumber_700 mxGetFieldByNumber_700_proxy
#endif
#endif   /* defined(MX_COMPAT_32) */
#if defined(__linux__)
#if defined(MX_COMPAT_32)
#undef mxGetFieldByNumber
#define mxGetFieldByNumber mxGetFieldByNumber_700
#endif
#endif




#if defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetCell_700
#define mxGetCell_700 mxGetCell_700_proxy
#endif
#endif   /* defined(MX_COMPAT_32) */
#if defined(__linux__)
#if defined(MX_COMPAT_32)
#undef mxGetCell
#define mxGetCell mxGetCell_700
#endif
#endif




#if defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxSetM_700
#define mxSetM_700 mxSetM_700_proxy
#endif
#endif   /* defined(MX_COMPAT_32) */
#if defined(__linux__)
#if defined(MX_COMPAT_32)
#undef mxSetM
#define mxSetM mxSetM_700
#endif
#endif




#if defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxSetIr_700
#define mxSetIr_700 mxSetIr_700_proxy
#endif
#endif   /* defined(MX_COMPAT_32) */
#if defined(__linux__)
#if defined(MX_COMPAT_32)
#undef mxSetIr
#define mxSetIr mxSetIr_700
#endif
#endif




#if defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxSetJc_700
#define mxSetJc_700 mxSetJc_700_proxy
#endif
#endif   /* defined(MX_COMPAT_32) */
#if defined(__linux__)
#if defined(MX_COMPAT_32)
#undef mxSetJc
#define mxSetJc mxSetJc_700
#endif
#endif




#if defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxCalcSingleSubscript_700
#define mxCalcSingleSubscript_700 mxCalcSingleSubscript_700_proxy
#endif
#endif   /* defined(MX_COMPAT_32) */
#if defined(__linux__)
#if defined(MX_COMPAT_32)
#undef mxCalcSingleSubscript
#define mxCalcSingleSubscript mxCalcSingleSubscript_700
#endif
#endif




#if defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxSetCell_700
#define mxSetCell_700 mxSetCell_700_proxy
#endif
#endif   /* defined(MX_COMPAT_32) */
#if defined(__linux__)
#if defined(MX_COMPAT_32)
#undef mxSetCell
#define mxSetCell mxSetCell_700
#endif
#endif




#if defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxSetFieldByNumber_700
#define mxSetFieldByNumber_700 mxSetFieldByNumber_700_proxy
#endif
#endif   /* defined(MX_COMPAT_32) */
#if defined(__linux__)
#if defined(MX_COMPAT_32)
#undef mxSetFieldByNumber
#define mxSetFieldByNumber mxSetFieldByNumber_700
#endif
#endif




#if defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetField_700
#define mxGetField_700 mxGetField_700_proxy
#endif
#endif   /* defined(MX_COMPAT_32) */
#if defined(__linux__)
#if defined(MX_COMPAT_32)
#undef mxGetField
#define mxGetField mxGetField_700
#endif
#endif




#if defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxSetField_700
#define mxSetField_700 mxSetField_700_proxy
#endif
#endif   /* defined(MX_COMPAT_32) */
#if defined(__linux__)
#if defined(MX_COMPAT_32)
#undef mxSetField
#define mxSetField mxSetField_700
#endif
#endif




#if defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetProperty_700
#define mxGetProperty_700 mxGetProperty_700_proxy
#endif
#endif   /* defined(MX_COMPAT_32) */
#if defined(__linux__)
#if defined(MX_COMPAT_32)
#undef mxGetProperty
#define mxGetProperty mxGetProperty_700
#endif
#endif




#if defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxSetProperty_700
#define mxSetProperty_700 mxSetProperty_700_proxy
#endif
#endif   /* defined(MX_COMPAT_32) */
#if defined(__linux__)
#if defined(MX_COMPAT_32)
#undef mxSetProperty
#define mxSetProperty mxSetProperty_700
#endif
#endif




#if defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxCreateNumericMatrix_700
#define mxCreateNumericMatrix_700 mxCreateNumericMatrix_700_proxy
#endif
#endif   /* defined(MX_COMPAT_32) */
#if defined(__linux__)
#if defined(MX_COMPAT_32)
#undef mxCreateNumericMatrix
#define mxCreateNumericMatrix mxCreateNumericMatrix_700
#endif
#endif




#if defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxSetN_700
#define mxSetN_700 mxSetN_700_proxy
#endif
#endif   /* defined(MX_COMPAT_32) */
#if defined(__linux__)
#if defined(MX_COMPAT_32)
#undef mxSetN
#define mxSetN mxSetN_700
#endif
#endif




#if defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxSetDimensions_700
#define mxSetDimensions_700 mxSetDimensions_700_proxy
#endif
#endif   /* defined(MX_COMPAT_32) */
#if defined(__linux__)
#if defined(MX_COMPAT_32)
#undef mxSetDimensions
#define mxSetDimensions mxSetDimensions_700
#endif
#endif




#if defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxCreateNumericArray_700
#define mxCreateNumericArray_700 mxCreateNumericArray_700_proxy
#endif
#endif   /* defined(MX_COMPAT_32) */
#if defined(__linux__)
#if defined(MX_COMPAT_32)
#undef mxCreateNumericArray
#define mxCreateNumericArray mxCreateNumericArray_700
#endif
#endif




#if defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxCreateCharArray_700
#define mxCreateCharArray_700 mxCreateCharArray_700_proxy
#endif
#endif   /* defined(MX_COMPAT_32) */
#if defined(__linux__)
#if defined(MX_COMPAT_32)
#undef mxCreateCharArray
#define mxCreateCharArray mxCreateCharArray_700
#endif
#endif




#if defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxCreateDoubleMatrix_700
#define mxCreateDoubleMatrix_700 mxCreateDoubleMatrix_700_proxy
#endif
#endif   /* defined(MX_COMPAT_32) */
#if defined(__linux__)
#if defined(MX_COMPAT_32)
#undef mxCreateDoubleMatrix
#define mxCreateDoubleMatrix mxCreateDoubleMatrix_700
#endif
#endif




#if defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxCreateLogicalArray_700
#define mxCreateLogicalArray_700 mxCreateLogicalArray_700_proxy
#endif
#endif   /* defined(MX_COMPAT_32) */
#if defined(__linux__)
#if defined(MX_COMPAT_32)
#undef mxCreateLogicalArray
#define mxCreateLogicalArray mxCreateLogicalArray_700
#endif
#endif




#if defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxCreateLogicalMatrix_700
#define mxCreateLogicalMatrix_700 mxCreateLogicalMatrix_700_proxy
#endif
#endif   /* defined(MX_COMPAT_32) */
#if defined(__linux__)
#if defined(MX_COMPAT_32)
#undef mxCreateLogicalMatrix
#define mxCreateLogicalMatrix mxCreateLogicalMatrix_700
#endif
#endif




#if defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxCreateSparse_700
#define mxCreateSparse_700 mxCreateSparse_700_proxy
#endif
#endif   /* defined(MX_COMPAT_32) */
#if defined(__linux__)
#if defined(MX_COMPAT_32)
#undef mxCreateSparse
#define mxCreateSparse mxCreateSparse_700
#endif
#endif




#if defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxCreateSparseLogicalMatrix_700
#define mxCreateSparseLogicalMatrix_700 mxCreateSparseLogicalMatrix_700_proxy
#endif
#endif   /* defined(MX_COMPAT_32) */
#if defined(__linux__)
#if defined(MX_COMPAT_32)
#undef mxCreateSparseLogicalMatrix
#define mxCreateSparseLogicalMatrix mxCreateSparseLogicalMatrix_700
#endif
#endif




#if defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetNChars_700
#define mxGetNChars_700 mxGetNChars_700_proxy
#endif
#endif   /* defined(MX_COMPAT_32) */
#if defined(__linux__)
#if defined(MX_COMPAT_32)
#undef mxGetNChars
#define mxGetNChars mxGetNChars_700
#endif
#endif




#if defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetString_700
#define mxGetString_700 mxGetString_700_proxy
#endif
#endif   /* defined(MX_COMPAT_32) */
#if defined(__linux__)
#if defined(MX_COMPAT_32)
#undef mxGetString
#define mxGetString mxGetString_700
#endif
#endif




#if defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxCreateStringFromNChars_700
#define mxCreateStringFromNChars_700 mxCreateStringFromNChars_700_proxy
#endif
#endif   /* defined(MX_COMPAT_32) */
#if defined(__linux__)
#if defined(MX_COMPAT_32)
#undef mxCreateStringFromNChars
#define mxCreateStringFromNChars mxCreateStringFromNChars_700
#endif
#endif




#if defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxCreateCharMatrixFromStrings_700
#define mxCreateCharMatrixFromStrings_700 mxCreateCharMatrixFromStrings_700_proxy
#endif
#endif   /* defined(MX_COMPAT_32) */
#if defined(__linux__)
#if defined(MX_COMPAT_32)
#undef mxCreateCharMatrixFromStrings
#define mxCreateCharMatrixFromStrings mxCreateCharMatrixFromStrings_700
#endif
#endif




#if defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxCreateCellMatrix_700
#define mxCreateCellMatrix_700 mxCreateCellMatrix_700_proxy
#endif
#endif   /* defined(MX_COMPAT_32) */
#if defined(__linux__)
#if defined(MX_COMPAT_32)
#undef mxCreateCellMatrix
#define mxCreateCellMatrix mxCreateCellMatrix_700
#endif
#endif




#if defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxCreateCellArray_700
#define mxCreateCellArray_700 mxCreateCellArray_700_proxy
#endif
#endif   /* defined(MX_COMPAT_32) */
#if defined(__linux__)
#if defined(MX_COMPAT_32)
#undef mxCreateCellArray
#define mxCreateCellArray mxCreateCellArray_700
#endif
#endif




#if defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxCreateStructMatrix_700
#define mxCreateStructMatrix_700 mxCreateStructMatrix_700_proxy
#endif
#endif   /* defined(MX_COMPAT_32) */
#if defined(__linux__)
#if defined(MX_COMPAT_32)
#undef mxCreateStructMatrix
#define mxCreateStructMatrix mxCreateStructMatrix_700
#endif
#endif




#if defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxCreateStructArray_700
#define mxCreateStructArray_700 mxCreateStructArray_700_proxy
#endif
#endif   /* defined(MX_COMPAT_32) */
#if defined(__linux__)
#if defined(MX_COMPAT_32)
#undef mxCreateStructArray
#define mxCreateStructArray mxCreateStructArray_700
#endif
#endif




/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxMalloc
#define mxMalloc mxMalloc_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxCalloc
#define mxCalloc mxCalloc_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxFree
#define mxFree mxFree_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxRealloc
#define mxRealloc mxRealloc_proxy
#endif



#if !defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetNumberOfDimensions_730
#define mxGetNumberOfDimensions_730 mxGetNumberOfDimensions_730_proxy
#endif
#endif   /* !defined(MX_COMPAT_32) */
#if defined(__linux__)
#if !defined(MX_COMPAT_32)
#undef mxGetNumberOfDimensions
#define mxGetNumberOfDimensions mxGetNumberOfDimensions_730
#endif
#endif




#if !defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetDimensions_730
#define mxGetDimensions_730 mxGetDimensions_730_proxy
#endif
#endif   /* !defined(MX_COMPAT_32) */
#if defined(__linux__)
#if !defined(MX_COMPAT_32)
#undef mxGetDimensions
#define mxGetDimensions mxGetDimensions_730
#endif
#endif




/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetM
#define mxGetM mxGetM_proxy
#endif



#if !defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetIr_730
#define mxGetIr_730 mxGetIr_730_proxy
#endif
#endif   /* !defined(MX_COMPAT_32) */
#if defined(__linux__)
#if !defined(MX_COMPAT_32)
#undef mxGetIr
#define mxGetIr mxGetIr_730
#endif
#endif




#if !defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetJc_730
#define mxGetJc_730 mxGetJc_730_proxy
#endif
#endif   /* !defined(MX_COMPAT_32) */
#if defined(__linux__)
#if !defined(MX_COMPAT_32)
#undef mxGetJc
#define mxGetJc mxGetJc_730
#endif
#endif




#if !defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetNzmax_730
#define mxGetNzmax_730 mxGetNzmax_730_proxy
#endif
#endif   /* !defined(MX_COMPAT_32) */
#if defined(__linux__)
#if !defined(MX_COMPAT_32)
#undef mxGetNzmax
#define mxGetNzmax mxGetNzmax_730
#endif
#endif




#if !defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxSetNzmax_730
#define mxSetNzmax_730 mxSetNzmax_730_proxy
#endif
#endif   /* !defined(MX_COMPAT_32) */
#if defined(__linux__)
#if !defined(MX_COMPAT_32)
#undef mxSetNzmax
#define mxSetNzmax mxSetNzmax_730
#endif
#endif




/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetFieldNameByNumber
#define mxGetFieldNameByNumber mxGetFieldNameByNumber_proxy
#endif



#if !defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetFieldByNumber_730
#define mxGetFieldByNumber_730 mxGetFieldByNumber_730_proxy
#endif
#endif   /* !defined(MX_COMPAT_32) */
#if defined(__linux__)
#if !defined(MX_COMPAT_32)
#undef mxGetFieldByNumber
#define mxGetFieldByNumber mxGetFieldByNumber_730
#endif
#endif




#if !defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetCell_730
#define mxGetCell_730 mxGetCell_730_proxy
#endif
#endif   /* !defined(MX_COMPAT_32) */
#if defined(__linux__)
#if !defined(MX_COMPAT_32)
#undef mxGetCell
#define mxGetCell mxGetCell_730
#endif
#endif




/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetClassID
#define mxGetClassID mxGetClassID_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetData
#define mxGetData mxGetData_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxSetData
#define mxSetData mxSetData_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxIsNumeric
#define mxIsNumeric mxIsNumeric_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxIsCell
#define mxIsCell mxIsCell_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxIsLogical
#define mxIsLogical mxIsLogical_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxIsScalar
#define mxIsScalar mxIsScalar_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxIsChar
#define mxIsChar mxIsChar_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxIsStruct
#define mxIsStruct mxIsStruct_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxIsOpaque
#define mxIsOpaque mxIsOpaque_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxIsFunctionHandle
#define mxIsFunctionHandle mxIsFunctionHandle_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxIsObject
#define mxIsObject mxIsObject_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetImagData
#define mxGetImagData mxGetImagData_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxSetImagData
#define mxSetImagData mxSetImagData_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxIsComplex
#define mxIsComplex mxIsComplex_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxIsSparse
#define mxIsSparse mxIsSparse_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxIsDouble
#define mxIsDouble mxIsDouble_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxIsSingle
#define mxIsSingle mxIsSingle_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxIsInt8
#define mxIsInt8 mxIsInt8_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxIsUint8
#define mxIsUint8 mxIsUint8_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxIsInt16
#define mxIsInt16 mxIsInt16_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxIsUint16
#define mxIsUint16 mxIsUint16_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxIsInt32
#define mxIsInt32 mxIsInt32_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxIsUint32
#define mxIsUint32 mxIsUint32_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxIsInt64
#define mxIsInt64 mxIsInt64_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxIsUint64
#define mxIsUint64 mxIsUint64_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetNumberOfElements
#define mxGetNumberOfElements mxGetNumberOfElements_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetPr
#define mxGetPr mxGetPr_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxSetPr
#define mxSetPr mxSetPr_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetPi
#define mxGetPi mxGetPi_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxSetPi
#define mxSetPi mxSetPi_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetChars
#define mxGetChars mxGetChars_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetUserBits
#define mxGetUserBits mxGetUserBits_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxSetUserBits
#define mxSetUserBits mxSetUserBits_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetScalar
#define mxGetScalar mxGetScalar_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxIsFromGlobalWS
#define mxIsFromGlobalWS mxIsFromGlobalWS_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxSetFromGlobalWS
#define mxSetFromGlobalWS mxSetFromGlobalWS_proxy
#endif



#if !defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxSetM_730
#define mxSetM_730 mxSetM_730_proxy
#endif
#endif   /* !defined(MX_COMPAT_32) */
#if defined(__linux__)
#if !defined(MX_COMPAT_32)
#undef mxSetM
#define mxSetM mxSetM_730
#endif
#endif




/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetN
#define mxGetN mxGetN_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxIsEmpty
#define mxIsEmpty mxIsEmpty_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetFieldNumber
#define mxGetFieldNumber mxGetFieldNumber_proxy
#endif



#if !defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxSetIr_730
#define mxSetIr_730 mxSetIr_730_proxy
#endif
#endif   /* !defined(MX_COMPAT_32) */
#if defined(__linux__)
#if !defined(MX_COMPAT_32)
#undef mxSetIr
#define mxSetIr mxSetIr_730
#endif
#endif




#if !defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxSetJc_730
#define mxSetJc_730 mxSetJc_730_proxy
#endif
#endif   /* !defined(MX_COMPAT_32) */
#if defined(__linux__)
#if !defined(MX_COMPAT_32)
#undef mxSetJc
#define mxSetJc mxSetJc_730
#endif
#endif




/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetElementSize
#define mxGetElementSize mxGetElementSize_proxy
#endif



#if !defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxCalcSingleSubscript_730
#define mxCalcSingleSubscript_730 mxCalcSingleSubscript_730_proxy
#endif
#endif   /* !defined(MX_COMPAT_32) */
#if defined(__linux__)
#if !defined(MX_COMPAT_32)
#undef mxCalcSingleSubscript
#define mxCalcSingleSubscript mxCalcSingleSubscript_730
#endif
#endif




/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetNumberOfFields
#define mxGetNumberOfFields mxGetNumberOfFields_proxy
#endif



#if !defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxSetCell_730
#define mxSetCell_730 mxSetCell_730_proxy
#endif
#endif   /* !defined(MX_COMPAT_32) */
#if defined(__linux__)
#if !defined(MX_COMPAT_32)
#undef mxSetCell
#define mxSetCell mxSetCell_730
#endif
#endif




#if !defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxSetFieldByNumber_730
#define mxSetFieldByNumber_730 mxSetFieldByNumber_730_proxy
#endif
#endif   /* !defined(MX_COMPAT_32) */
#if defined(__linux__)
#if !defined(MX_COMPAT_32)
#undef mxSetFieldByNumber
#define mxSetFieldByNumber mxSetFieldByNumber_730
#endif
#endif




#if !defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetField_730
#define mxGetField_730 mxGetField_730_proxy
#endif
#endif   /* !defined(MX_COMPAT_32) */
#if defined(__linux__)
#if !defined(MX_COMPAT_32)
#undef mxGetField
#define mxGetField mxGetField_730
#endif
#endif




#if !defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxSetField_730
#define mxSetField_730 mxSetField_730_proxy
#endif
#endif   /* !defined(MX_COMPAT_32) */
#if defined(__linux__)
#if !defined(MX_COMPAT_32)
#undef mxSetField
#define mxSetField mxSetField_730
#endif
#endif




#if !defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetProperty_730
#define mxGetProperty_730 mxGetProperty_730_proxy
#endif
#endif   /* !defined(MX_COMPAT_32) */
#if defined(__linux__)
#if !defined(MX_COMPAT_32)
#undef mxGetProperty
#define mxGetProperty mxGetProperty_730
#endif
#endif




#if !defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxSetProperty_730
#define mxSetProperty_730 mxSetProperty_730_proxy
#endif
#endif   /* !defined(MX_COMPAT_32) */
#if defined(__linux__)
#if !defined(MX_COMPAT_32)
#undef mxSetProperty
#define mxSetProperty mxSetProperty_730
#endif
#endif




/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetClassName
#define mxGetClassName mxGetClassName_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxIsClass
#define mxIsClass mxIsClass_proxy
#endif



#if !defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxCreateNumericMatrix_730
#define mxCreateNumericMatrix_730 mxCreateNumericMatrix_730_proxy
#endif
#endif   /* !defined(MX_COMPAT_32) */
#if defined(__linux__)
#if !defined(MX_COMPAT_32)
#undef mxCreateNumericMatrix
#define mxCreateNumericMatrix mxCreateNumericMatrix_730
#endif
#endif




/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxCreateUninitNumericMatrix
#define mxCreateUninitNumericMatrix mxCreateUninitNumericMatrix_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxCreateUninitNumericArray
#define mxCreateUninitNumericArray mxCreateUninitNumericArray_proxy
#endif



#if !defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxSetN_730
#define mxSetN_730 mxSetN_730_proxy
#endif
#endif   /* !defined(MX_COMPAT_32) */
#if defined(__linux__)
#if !defined(MX_COMPAT_32)
#undef mxSetN
#define mxSetN mxSetN_730
#endif
#endif




#if !defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxSetDimensions_730
#define mxSetDimensions_730 mxSetDimensions_730_proxy
#endif
#endif   /* !defined(MX_COMPAT_32) */
#if defined(__linux__)
#if !defined(MX_COMPAT_32)
#undef mxSetDimensions
#define mxSetDimensions mxSetDimensions_730
#endif
#endif




/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxDestroyArray
#define mxDestroyArray mxDestroyArray_proxy
#endif



#if !defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxCreateNumericArray_730
#define mxCreateNumericArray_730 mxCreateNumericArray_730_proxy
#endif
#endif   /* !defined(MX_COMPAT_32) */
#if defined(__linux__)
#if !defined(MX_COMPAT_32)
#undef mxCreateNumericArray
#define mxCreateNumericArray mxCreateNumericArray_730
#endif
#endif




#if !defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxCreateCharArray_730
#define mxCreateCharArray_730 mxCreateCharArray_730_proxy
#endif
#endif   /* !defined(MX_COMPAT_32) */
#if defined(__linux__)
#if !defined(MX_COMPAT_32)
#undef mxCreateCharArray
#define mxCreateCharArray mxCreateCharArray_730
#endif
#endif




#if !defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxCreateDoubleMatrix_730
#define mxCreateDoubleMatrix_730 mxCreateDoubleMatrix_730_proxy
#endif
#endif   /* !defined(MX_COMPAT_32) */
#if defined(__linux__)
#if !defined(MX_COMPAT_32)
#undef mxCreateDoubleMatrix
#define mxCreateDoubleMatrix mxCreateDoubleMatrix_730
#endif
#endif




/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetLogicals
#define mxGetLogicals mxGetLogicals_proxy
#endif



#if !defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxCreateLogicalArray_730
#define mxCreateLogicalArray_730 mxCreateLogicalArray_730_proxy
#endif
#endif   /* !defined(MX_COMPAT_32) */
#if defined(__linux__)
#if !defined(MX_COMPAT_32)
#undef mxCreateLogicalArray
#define mxCreateLogicalArray mxCreateLogicalArray_730
#endif
#endif




#if !defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxCreateLogicalMatrix_730
#define mxCreateLogicalMatrix_730 mxCreateLogicalMatrix_730_proxy
#endif
#endif   /* !defined(MX_COMPAT_32) */
#if defined(__linux__)
#if !defined(MX_COMPAT_32)
#undef mxCreateLogicalMatrix
#define mxCreateLogicalMatrix mxCreateLogicalMatrix_730
#endif
#endif




/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxCreateLogicalScalar
#define mxCreateLogicalScalar mxCreateLogicalScalar_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxIsLogicalScalar
#define mxIsLogicalScalar mxIsLogicalScalar_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxIsLogicalScalarTrue
#define mxIsLogicalScalarTrue mxIsLogicalScalarTrue_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxCreateDoubleScalar
#define mxCreateDoubleScalar mxCreateDoubleScalar_proxy
#endif



#if !defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxCreateSparse_730
#define mxCreateSparse_730 mxCreateSparse_730_proxy
#endif
#endif   /* !defined(MX_COMPAT_32) */
#if defined(__linux__)
#if !defined(MX_COMPAT_32)
#undef mxCreateSparse
#define mxCreateSparse mxCreateSparse_730
#endif
#endif




#if !defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxCreateSparseLogicalMatrix_730
#define mxCreateSparseLogicalMatrix_730 mxCreateSparseLogicalMatrix_730_proxy
#endif
#endif   /* !defined(MX_COMPAT_32) */
#if defined(__linux__)
#if !defined(MX_COMPAT_32)
#undef mxCreateSparseLogicalMatrix
#define mxCreateSparseLogicalMatrix mxCreateSparseLogicalMatrix_730
#endif
#endif




#if !defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetNChars_730
#define mxGetNChars_730 mxGetNChars_730_proxy
#endif
#endif   /* !defined(MX_COMPAT_32) */
#if defined(__linux__)
#if !defined(MX_COMPAT_32)
#undef mxGetNChars
#define mxGetNChars mxGetNChars_730
#endif
#endif




#if !defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetString_730
#define mxGetString_730 mxGetString_730_proxy
#endif
#endif   /* !defined(MX_COMPAT_32) */
#if defined(__linux__)
#if !defined(MX_COMPAT_32)
#undef mxGetString
#define mxGetString mxGetString_730
#endif
#endif




/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxArrayToString
#define mxArrayToString mxArrayToString_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxArrayToUTF8String
#define mxArrayToUTF8String mxArrayToUTF8String_proxy
#endif



#if !defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxCreateStringFromNChars_730
#define mxCreateStringFromNChars_730 mxCreateStringFromNChars_730_proxy
#endif
#endif   /* !defined(MX_COMPAT_32) */
#if defined(__linux__)
#if !defined(MX_COMPAT_32)
#undef mxCreateStringFromNChars
#define mxCreateStringFromNChars mxCreateStringFromNChars_730
#endif
#endif




/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxCreateString
#define mxCreateString mxCreateString_proxy
#endif



#if !defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxCreateCharMatrixFromStrings_730
#define mxCreateCharMatrixFromStrings_730 mxCreateCharMatrixFromStrings_730_proxy
#endif
#endif   /* !defined(MX_COMPAT_32) */
#if defined(__linux__)
#if !defined(MX_COMPAT_32)
#undef mxCreateCharMatrixFromStrings
#define mxCreateCharMatrixFromStrings mxCreateCharMatrixFromStrings_730
#endif
#endif




#if !defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxCreateCellMatrix_730
#define mxCreateCellMatrix_730 mxCreateCellMatrix_730_proxy
#endif
#endif   /* !defined(MX_COMPAT_32) */
#if defined(__linux__)
#if !defined(MX_COMPAT_32)
#undef mxCreateCellMatrix
#define mxCreateCellMatrix mxCreateCellMatrix_730
#endif
#endif




#if !defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxCreateCellArray_730
#define mxCreateCellArray_730 mxCreateCellArray_730_proxy
#endif
#endif   /* !defined(MX_COMPAT_32) */
#if defined(__linux__)
#if !defined(MX_COMPAT_32)
#undef mxCreateCellArray
#define mxCreateCellArray mxCreateCellArray_730
#endif
#endif




#if !defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxCreateStructMatrix_730
#define mxCreateStructMatrix_730 mxCreateStructMatrix_730_proxy
#endif
#endif   /* !defined(MX_COMPAT_32) */
#if defined(__linux__)
#if !defined(MX_COMPAT_32)
#undef mxCreateStructMatrix
#define mxCreateStructMatrix mxCreateStructMatrix_730
#endif
#endif




#if !defined(MX_COMPAT_32)
/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxCreateStructArray_730
#define mxCreateStructArray_730 mxCreateStructArray_730_proxy
#endif
#endif   /* !defined(MX_COMPAT_32) */
#if defined(__linux__)
#if !defined(MX_COMPAT_32)
#undef mxCreateStructArray
#define mxCreateStructArray mxCreateStructArray_730
#endif
#endif




/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxDuplicateArray
#define mxDuplicateArray mxDuplicateArray_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxSetClassName
#define mxSetClassName mxSetClassName_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxAddField
#define mxAddField mxAddField_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxRemoveField
#define mxRemoveField mxRemoveField_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetEps
#define mxGetEps mxGetEps_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetInf
#define mxGetInf mxGetInf_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxGetNaN
#define mxGetNaN mxGetNaN_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxIsFinite
#define mxIsFinite mxIsFinite_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxIsInf
#define mxIsInf mxIsInf_proxy
#endif



/* Map original name to unique proxy layer name. */
#if !(defined(__APPLE__) && (defined(__arm__) || defined(__arm64__)))
#undef mxIsNaN
#define mxIsNaN mxIsNaN_proxy
#endif



/*#ifdef matrix_h
#error "mclmcrrt.h must be included before matrix.h. (Since mclmcrrt.h includes matrix.h, additional inclusion is redundant.)"
#endif */
#define LIBMWMATRIX_API_EXTERN_C EXTERN_C
#include "matrix.h"

/* Proxies for functions in matrix.h */

EXTERN_C
int mxGetNumberOfDimensions_700_proxy(const mxArray *a0);

EXTERN_C
const int * mxGetDimensions_700_proxy(const mxArray *a0);

EXTERN_C
int * mxGetIr_700_proxy(const mxArray *a0);

EXTERN_C
int * mxGetJc_700_proxy(const mxArray *a0);

EXTERN_C
int mxGetNzmax_700_proxy(const mxArray *a0);

EXTERN_C
void mxSetNzmax_700_proxy(mxArray *a0, int a1);

EXTERN_C
mxArray * mxGetFieldByNumber_700_proxy(const mxArray *a0, int a1, int a2);

EXTERN_C
mxArray * mxGetCell_700_proxy(const mxArray *a0, int a1);

EXTERN_C
void mxSetM_700_proxy(mxArray *a0, int a1);

EXTERN_C
void mxSetIr_700_proxy(mxArray *a0, int *a1);

EXTERN_C
void mxSetJc_700_proxy(mxArray *a0, int *a1);

EXTERN_C
int mxCalcSingleSubscript_700_proxy(const mxArray *a0, int a1, 
    const int *a2);

EXTERN_C
void mxSetCell_700_proxy(mxArray *a0, int a1, mxArray *a2);

EXTERN_C
void mxSetFieldByNumber_700_proxy(mxArray *a0, int a1, int a2, 
    mxArray *a3);

EXTERN_C
mxArray * mxGetField_700_proxy(const mxArray *a0, int a1, const char *a2);

EXTERN_C
void mxSetField_700_proxy(mxArray *a0, int a1, const char *a2, 
    mxArray *a3);

EXTERN_C
mxArray * mxGetProperty_700_proxy(const mxArray *a0, const int a1, 
    const char *a2);

EXTERN_C
void mxSetProperty_700_proxy(mxArray *a0, int a1, const char *a2, 
    const mxArray *a3);

EXTERN_C
mxArray * mxCreateNumericMatrix_700_proxy(int a0, int a1, mxClassID a2, 
    mxComplexity a3);

EXTERN_C
void mxSetN_700_proxy(mxArray *a0, int a1);

EXTERN_C
int mxSetDimensions_700_proxy(mxArray *a0, const int *a1, int a2);

EXTERN_C
mxArray * mxCreateNumericArray_700_proxy(int a0, const int *a1, 
    mxClassID a2, mxComplexity a3);

EXTERN_C
mxArray * mxCreateCharArray_700_proxy(int a0, const int *a1);

EXTERN_C
mxArray * mxCreateDoubleMatrix_700_proxy(int a0, int a1, mxComplexity a2);

EXTERN_C
mxArray * mxCreateLogicalArray_700_proxy(int a0, const int *a1);

EXTERN_C
mxArray * mxCreateLogicalMatrix_700_proxy(int a0, int a1);

EXTERN_C
mxArray * mxCreateSparse_700_proxy(int a0, int a1, int a2, 
    mxComplexity a3);

EXTERN_C
mxArray * mxCreateSparseLogicalMatrix_700_proxy(int a0, int a1, int a2);

EXTERN_C
void mxGetNChars_700_proxy(const mxArray *a0, char *a1, int a2);

EXTERN_C
int mxGetString_700_proxy(const mxArray *a0, char *a1, int a2);

EXTERN_C
mxArray * mxCreateStringFromNChars_700_proxy(const char *a0, int a1);

EXTERN_C
mxArray * mxCreateCharMatrixFromStrings_700_proxy(int a0, 
    const char **a1);

EXTERN_C
mxArray * mxCreateCellMatrix_700_proxy(int a0, int a1);

EXTERN_C
mxArray * mxCreateCellArray_700_proxy(int a0, const int *a1);

EXTERN_C
mxArray * mxCreateStructMatrix_700_proxy(int a0, int a1, int a2, 
    const char **a3);

EXTERN_C
mxArray * mxCreateStructArray_700_proxy(int a0, const int *a1, int a2, 
    const char **a3);

EXTERN_C
void * mxMalloc_proxy(size_t a0);

EXTERN_C
void * mxCalloc_proxy(size_t a0, size_t a1);

EXTERN_C
void mxFree_proxy(void *a0);

EXTERN_C
void * mxRealloc_proxy(void *a0, size_t a1);

EXTERN_C
size_t mxGetNumberOfDimensions_730_proxy(const mxArray *a0);

EXTERN_C
const size_t * mxGetDimensions_730_proxy(const mxArray *a0);

EXTERN_C
size_t mxGetM_proxy(const mxArray *a0);

EXTERN_C
size_t * mxGetIr_730_proxy(const mxArray *a0);

EXTERN_C
size_t * mxGetJc_730_proxy(const mxArray *a0);

EXTERN_C
size_t mxGetNzmax_730_proxy(const mxArray *a0);

EXTERN_C
void mxSetNzmax_730_proxy(mxArray *a0, size_t a1);

EXTERN_C
const char * mxGetFieldNameByNumber_proxy(const mxArray *a0, int a1);

EXTERN_C
mxArray * mxGetFieldByNumber_730_proxy(const mxArray *a0, size_t a1, 
    int a2);

EXTERN_C
mxArray * mxGetCell_730_proxy(const mxArray *a0, size_t a1);

EXTERN_C
mxClassID mxGetClassID_proxy(const mxArray *a0);

EXTERN_C
void * mxGetData_proxy(const mxArray *a0);

EXTERN_C
void mxSetData_proxy(mxArray *a0, void *a1);

EXTERN_C
bool mxIsNumeric_proxy(const mxArray *a0);

EXTERN_C
bool mxIsCell_proxy(const mxArray *a0);

EXTERN_C
bool mxIsLogical_proxy(const mxArray *a0);

EXTERN_C
bool mxIsScalar_proxy(const mxArray *a0);

EXTERN_C
bool mxIsChar_proxy(const mxArray *a0);

EXTERN_C
bool mxIsStruct_proxy(const mxArray *a0);

EXTERN_C
bool mxIsOpaque_proxy(const mxArray *a0);

EXTERN_C
bool mxIsFunctionHandle_proxy(const mxArray *a0);

EXTERN_C
bool mxIsObject_proxy(const mxArray *a0);

EXTERN_C
void * mxGetImagData_proxy(const mxArray *a0);

EXTERN_C
void mxSetImagData_proxy(mxArray *a0, void *a1);

EXTERN_C
bool mxIsComplex_proxy(const mxArray *a0);

EXTERN_C
bool mxIsSparse_proxy(const mxArray *a0);

EXTERN_C
bool mxIsDouble_proxy(const mxArray *a0);

EXTERN_C
bool mxIsSingle_proxy(const mxArray *a0);

EXTERN_C
bool mxIsInt8_proxy(const mxArray *a0);

EXTERN_C
bool mxIsUint8_proxy(const mxArray *a0);

EXTERN_C
bool mxIsInt16_proxy(const mxArray *a0);

EXTERN_C
bool mxIsUint16_proxy(const mxArray *a0);

EXTERN_C
bool mxIsInt32_proxy(const mxArray *a0);

EXTERN_C
bool mxIsUint32_proxy(const mxArray *a0);

EXTERN_C
bool mxIsInt64_proxy(const mxArray *a0);

EXTERN_C
bool mxIsUint64_proxy(const mxArray *a0);

EXTERN_C
size_t mxGetNumberOfElements_proxy(const mxArray *a0);

EXTERN_C
double * mxGetPr_proxy(const mxArray *a0);

EXTERN_C
void mxSetPr_proxy(mxArray *a0, double *a1);

EXTERN_C
double * mxGetPi_proxy(const mxArray *a0);

EXTERN_C
void mxSetPi_proxy(mxArray *a0, double *a1);

EXTERN_C
mxChar * mxGetChars_proxy(const mxArray *a0);

EXTERN_C
int mxGetUserBits_proxy(const mxArray *a0);

EXTERN_C
void mxSetUserBits_proxy(mxArray *a0, int a1);

EXTERN_C
double mxGetScalar_proxy(const mxArray *a0);

EXTERN_C
bool mxIsFromGlobalWS_proxy(const mxArray *a0);

EXTERN_C
void mxSetFromGlobalWS_proxy(mxArray *a0, bool a1);

EXTERN_C
void mxSetM_730_proxy(mxArray *a0, size_t a1);

EXTERN_C
size_t mxGetN_proxy(const mxArray *a0);

EXTERN_C
bool mxIsEmpty_proxy(const mxArray *a0);

EXTERN_C
int mxGetFieldNumber_proxy(const mxArray *a0, const char *a1);

EXTERN_C
void mxSetIr_730_proxy(mxArray *a0, size_t *a1);

EXTERN_C
void mxSetJc_730_proxy(mxArray *a0, size_t *a1);

EXTERN_C
size_t mxGetElementSize_proxy(const mxArray *a0);

EXTERN_C
size_t mxCalcSingleSubscript_730_proxy(const mxArray *a0, size_t a1, 
    const size_t *a2);

EXTERN_C
int mxGetNumberOfFields_proxy(const mxArray *a0);

EXTERN_C
void mxSetCell_730_proxy(mxArray *a0, size_t a1, mxArray *a2);

EXTERN_C
void mxSetFieldByNumber_730_proxy(mxArray *a0, size_t a1, int a2, 
    mxArray *a3);

EXTERN_C
mxArray * mxGetField_730_proxy(const mxArray *a0, size_t a1, 
    const char *a2);

EXTERN_C
void mxSetField_730_proxy(mxArray *a0, size_t a1, const char *a2, 
    mxArray *a3);

EXTERN_C
mxArray * mxGetProperty_730_proxy(const mxArray *a0, const size_t a1, 
    const char *a2);

EXTERN_C
void mxSetProperty_730_proxy(mxArray *a0, size_t a1, const char *a2, 
    const mxArray *a3);

EXTERN_C
const char * mxGetClassName_proxy(const mxArray *a0);

EXTERN_C
bool mxIsClass_proxy(const mxArray *a0, const char *a1);

EXTERN_C
mxArray * mxCreateNumericMatrix_730_proxy(size_t a0, size_t a1, 
    mxClassID a2, mxComplexity a3);

EXTERN_C
mxArray * mxCreateUninitNumericMatrix_proxy(size_t a0, size_t a1, 
    mxClassID a2, mxComplexity a3);

EXTERN_C
mxArray * mxCreateUninitNumericArray_proxy(size_t a0, size_t *a1, 
    mxClassID a2, mxComplexity a3);

EXTERN_C
void mxSetN_730_proxy(mxArray *a0, size_t a1);

EXTERN_C
int mxSetDimensions_730_proxy(mxArray *a0, const size_t *a1, size_t a2);

EXTERN_C
void mxDestroyArray_proxy(mxArray *a0);

EXTERN_C
mxArray * mxCreateNumericArray_730_proxy(size_t a0, const size_t *a1, 
    mxClassID a2, mxComplexity a3);

EXTERN_C
mxArray * mxCreateCharArray_730_proxy(size_t a0, const size_t *a1);

EXTERN_C
mxArray * mxCreateDoubleMatrix_730_proxy(size_t a0, size_t a1, 
    mxComplexity a2);

EXTERN_C
mxLogical * mxGetLogicals_proxy(const mxArray *a0);

EXTERN_C
mxArray * mxCreateLogicalArray_730_proxy(size_t a0, const size_t *a1);

EXTERN_C
mxArray * mxCreateLogicalMatrix_730_proxy(size_t a0, size_t a1);

EXTERN_C
mxArray * mxCreateLogicalScalar_proxy(bool a0);

EXTERN_C
bool mxIsLogicalScalar_proxy(const mxArray *a0);

EXTERN_C
bool mxIsLogicalScalarTrue_proxy(const mxArray *a0);

EXTERN_C
mxArray * mxCreateDoubleScalar_proxy(double a0);

EXTERN_C
mxArray * mxCreateSparse_730_proxy(size_t a0, size_t a1, size_t a2, 
    mxComplexity a3);

EXTERN_C
mxArray * mxCreateSparseLogicalMatrix_730_proxy(size_t a0, size_t a1, 
    size_t a2);

EXTERN_C
void mxGetNChars_730_proxy(const mxArray *a0, char *a1, size_t a2);

EXTERN_C
int mxGetString_730_proxy(const mxArray *a0, char *a1, size_t a2);

EXTERN_C
char * mxArrayToString_proxy(const mxArray *a0);

EXTERN_C
char * mxArrayToUTF8String_proxy(const mxArray *a0);

EXTERN_C
mxArray * mxCreateStringFromNChars_730_proxy(const char *a0, size_t a1);

EXTERN_C
mxArray * mxCreateString_proxy(const char *a0);

EXTERN_C
mxArray * mxCreateCharMatrixFromStrings_730_proxy(size_t a0, 
    const char **a1);

EXTERN_C
mxArray * mxCreateCellMatrix_730_proxy(size_t a0, size_t a1);

EXTERN_C
mxArray * mxCreateCellArray_730_proxy(size_t a0, const size_t *a1);

EXTERN_C
mxArray * mxCreateStructMatrix_730_proxy(size_t a0, size_t a1, int a2, 
    const char **a3);

EXTERN_C
mxArray * mxCreateStructArray_730_proxy(size_t a0, const size_t *a1, 
    int a2, const char **a3);

EXTERN_C
mxArray * mxDuplicateArray_proxy(const mxArray *a0);

EXTERN_C
int mxSetClassName_proxy(mxArray *a0, const char *a1);

EXTERN_C
int mxAddField_proxy(mxArray *a0, const char *a1);

EXTERN_C
void mxRemoveField_proxy(mxArray *a0, int a1);

EXTERN_C
double mxGetEps_proxy();

EXTERN_C
double mxGetInf_proxy();

EXTERN_C
double mxGetNaN_proxy();

EXTERN_C
bool mxIsFinite_proxy(double a0);

EXTERN_C
bool mxIsInf_proxy(double a0);

EXTERN_C
bool mxIsNaN_proxy(double a0);


#if defined(__APPLE__) && (defined(__arm__) || defined(__arm64__))
EXTERN_C bool mclmcrInitialize2_proxy(int mode);
EXTERN_C bool mclmcrInitialize_proxy(void);
EXTERN_C bool mclInitializeApplication_proxy(const char **options, size_t count);
#endif

#endif /* mclmcrrt_h */
