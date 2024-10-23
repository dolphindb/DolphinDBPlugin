/*
* PUBLISHed header for libmclcommain, the mclcommain library.
*
* Copyright 1984-2012 The MathWorks, Inc.
* All Rights Reserved.
*/

#if defined(_MSC_VER)
# pragma once
#endif
#if defined(__GNUC__) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ > 3))
# pragma once
#endif

#ifndef mclcommain_h
#define mclcommain_h


#ifndef MCLCOMMAIN_API
#  define MCLCOMMAIN_API 
#endif


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


#ifdef __cplusplus
    extern "C" {
#endif


/* Register a MatLab Excel Builder component */
EXTERN_C MCLCOMMAIN_API HRESULT mclRegisterMatLabComponent(const char* szModuleName,     /* DLL module handle */
                                const CLSID* clsid,           /* Class ID */
                                const GUID* libid,            /* GUID of TypeLib */
                                unsigned short wMajorRev,     /* Major rev of type lib */
                                unsigned short wMinorRev,     /* Minor rev of type lib */
                                const char* szFriendlyName,   /* Friendly Name */
                                const char* szVerIndProgID,   /* Programmatic */
                                const char* szProgID)         /* IDs */;


/* Unregister a MatLab Excel Builder component */
EXTERN_C MCLCOMMAIN_API HRESULT mclUnRegisterMatLabComponent(const CLSID* clsid,         /* Class ID */
                                  const char* szVerIndProgID, /* Programmatic */
                                  const char* szProgID)       /* IDs */;


/* 
   Aquire global lock. Returns 0 for successful aquisition, -1 otherwise.
   If the global mutex is not initialized, or if the wait function fails,
   -1 is returned.
*/
EXTERN_C MCLCOMMAIN_API int RequestGlobalLock(void);


/* 
   Release global lock. Returns 0. If the global mutex is not initialized,
   -1 is returned.
*/
EXTERN_C MCLCOMMAIN_API int ReleaseGlobalLock(void);


EXTERN_C MCLCOMMAIN_API HRESULT GetConversionFlags(IMWFlags* pFlags, MCLCONVERSION_FLAGS flags);


/* Converts an mxArray to a Variant */
EXTERN_C MCLCOMMAIN_API int mxArray2Variant(const mxArray* px, VARIANT* pvar, const MCLCONVERSION_FLAGS flags);


/* Converts a Variant to an mxArray */
EXTERN_C MCLCOMMAIN_API int Variant2mxArray(const VARIANT* pvar, mxArray** ppx, const MCLCONVERSION_FLAGS flags);


/* 
   If input VARIANT is VT_ERROR && v->scode == DISP_E_PARAMNOTFOUND, returns true. 
   Returns false otherwise. If input variant is *|VT_BYREF and reference is NULL, returns true. 
   If input variant pointer is NULL, returns true.
*/
EXTERN_C MCLCOMMAIN_API bool IsVisualBasicDefault(const VARIANT *v);


EXTERN_C MCLCOMMAIN_API void InitConversionFlags(MCLCONVERSION_FLAGS flags);


EXTERN_C MCLCOMMAIN_API const char* GetCOMErrorMessage(int ret);


EXTERN_C MCLCOMMAIN_API bool mclComCheckMWComUtil();


EXTERN_C MCLCOMMAIN_API bool mclComCheckMWComMgr();


#ifdef __cplusplus
    }	/* extern "C" */
#endif

#endif /* mclcommain_h */
