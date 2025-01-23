/*
* PUBLISHed header for libmclxlmain, the mclxlmain library.
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

#ifndef mclxlmain_published_api_h
#define mclxlmain_published_api_h

#ifndef LIBMCLXLMAIN_API
#  define LIBMCLXLMAIN_API
#endif

#ifdef __cplusplus
    extern "C" {
#endif


/* Register a MatLab Excel Builder component */
LIBMCLXLMAIN_API HRESULT mclRegisterMatLabComponent(const char* szModuleName,     /* DLL module handle */
                                const CLSID* clsid,           /* Class ID */
                                const GUID* libid,            /* GUID of TypeLib */
                                unsigned short wMajorRev,     /* Major rev of type lib */
                                unsigned short wMinorRev,     /* Minor rev of type lib */
                                const char* szFriendlyName,   /* Friendly Name */
                                const char* szVerIndProgID,   /* Programmatic */
                                const char* szProgID)         /* IDs */;


/* Unregister a MatLab Excel Builder component */
LIBMCLXLMAIN_API HRESULT mclUnRegisterMatLabComponent(const CLSID* clsid,            /* Class ID */
                                     const char* szVerIndProgID, /* Programmatic */
                                     const char* szProgID)       /* IDs */;


/* 
   Aquire global lock. Returns 0 for successful aquisition, -1 otherwise.
   If the global mutex is not initialized, or if the wait function fails,
   -1 is returned.
*/
LIBMCLXLMAIN_API int RequestGlobalLock(void);


/* 
   Release global lock. Returns 0. If the global mutex is not initialized,
   -1 is returned.
*/
LIBMCLXLMAIN_API int ReleaseGlobalLock(void);


LIBMCLXLMAIN_API HRESULT GetConversionFlags(IMWFlags* pFlags, MCLCONVERSION_FLAGS flags);


/* Converts an mxArray to a Variant */
LIBMCLXLMAIN_API int mxArray2Variant(const mxArray* px, VARIANT* pvar, const MCLCONVERSION_FLAGS flags);


/* Converts a Variant to an mxArray */
LIBMCLXLMAIN_API int Variant2mxArray(const VARIANT* pvar, mxArray** ppx, const MCLCONVERSION_FLAGS flags);


/* 
   If input VARIANT is VT_EMPTY or VT_ERROR && v->scode == DISP_E_PARAMNOTFOUND, returns true.
   Returns false otherwise. If input variant is *|VT_BYREF and reference is NULL, returns true.
   If input variant pointer is NULL, returns true.
*/
LIBMCLXLMAIN_API bool IsVisualBasicDefault(const VARIANT *v);


LIBMCLXLMAIN_API void InitConversionFlags(MCLCONVERSION_FLAGS flags);


LIBMCLXLMAIN_API const char* GetCOMErrorMessage(int ret);


LIBMCLXLMAIN_API bool mclComCheckMWComUtil();


LIBMCLXLMAIN_API bool mclComCheckMWComMgr();


#ifdef __cplusplus
    }	/* extern "C" */
#endif

#endif /* mclxlmain_h */
