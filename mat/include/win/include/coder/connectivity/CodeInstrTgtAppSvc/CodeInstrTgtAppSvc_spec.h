/* Copyright 2013-2015 The MathWorks, Inc. */

#ifndef CodeInstrTgtAppSvc_spec_h
#define CodeInstrTgtAppSvc_spec_h

#ifdef BUILDING_LIBMWCODER_CODEINSTRTGTAPPSVC

#define CODEINSTRTGTAPPSVC_API                 DLL_EXPORT_SYM
#define CODEINSTRTGTAPPSVC_API_EXTERN_C        extern "C" DLL_EXPORT_SYM

#elif defined(DLL_IMPORT_SYM)

#define CODEINSTRTGTAPPSVC_API                 DLL_IMPORT_SYM
#define CODEINSTRTGTAPPSVC_API_EXTERN_C        extern "C" DLL_IMPORT_SYM

#else

#define CODEINSTRTGTAPPSVC_API                  extern
#ifdef __cplusplus
    #define CODEINSTRTGTAPPSVC_API_EXTERN_C     extern "C"
#else
    #define CODEINSTRTGTAPPSVC_API_EXTERN_C     extern
#endif
#endif

#endif

