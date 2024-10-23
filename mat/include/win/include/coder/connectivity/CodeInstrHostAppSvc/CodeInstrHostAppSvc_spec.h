/* Copyright 2015 The MathWorks, Inc. */

#ifndef CodeInstrHostAppSvc_spec_h
#define CodeInstrHostAppSvc_spec_h

#ifdef BUILDING_LIBMWCODER_CODEINSTRHOSTAPPSVC

#define CODEINSTRHOSTAPPSVC_API                 DLL_EXPORT_SYM
#define CODEINSTRHOSTAPPSVC_API_EXTERN_C        extern "C" DLL_EXPORT_SYM

#elif defined(DLL_IMPORT_SYM)

#define CODEINSTRHOSTAPPSVC_API                 DLL_IMPORT_SYM
#define CODEINSTRHOSTAPPSVC_API_EXTERN_C        extern "C" DLL_IMPORT_SYM

#else

#define CODEINSTRHOSTAPPSVC_API                  extern
#ifdef __cplusplus
    #define CODEINSTRHOSTAPPSVC_API_EXTERN_C     extern "C"
#else
    #define CODEINSTRHOSTAPPSVC_API_EXTERN_C     extern
#endif
#endif

#endif
