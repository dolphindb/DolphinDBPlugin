/* Copyright 2015 The MathWorks, Inc. */

#ifndef RTIOStreamTgtAppSvc_dll_h
#define RTIOStreamTgtAppSvc_dll_h

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif

#ifdef SL_INTERNAL

#include "version.h"

#if defined(BUILDING_LIBMWCODER_RTIOSTREAMTGTAPPSVC)
#define RTIOSTREAMTGTAPPSVC_API DLL_EXPORT_SYM
#define RTIOSTREAMTGTAPPSVC_API_C EXTERN_C DLL_EXPORT_SYM
#else
#define RTIOSTREAMTGTAPPSVC_API DLL_IMPORT_SYM
#define RTIOSTREAMTGTAPPSVC_API_C EXTERN_C DLL_IMPORT_SYM
#endif

#else

#define RTIOSTREAMTGTAPPSVC_API
#define RTIOSTREAMTGTAPPSVC_API_C EXTERN_C

#endif

#endif
