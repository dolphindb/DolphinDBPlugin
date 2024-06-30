/* Copyright 2013-2015 The MathWorks, Inc. */

#ifndef CoderAssumpTgtAppSvc_spec_h
#define CoderAssumpTgtAppSvc_spec_h

#ifdef BUILDING_LIBMWCODER_CODERASSUMPTGTAPPSVC

#define CODERASSUMPTGTAPPSVC_API                 DLL_EXPORT_SYM
#define CODERASSUMPTGTAPPSVC_API_EXTERN_C        extern "C" DLL_EXPORT_SYM

#elif defined(DLL_IMPORT_SYM)

#define CODERASSUMPTGTAPPSVC_API                 DLL_IMPORT_SYM
#define CODERASSUMPTGTAPPSVC_API_EXTERN_C        extern "C" DLL_IMPORT_SYM

#else

#define CODERASSUMPTGTAPPSVC_API                  extern
#ifdef __cplusplus
    #define CODERASSUMPTGTAPPSVC_API_EXTERN_C     extern "C"
#else
    #define CODERASSUMPTGTAPPSVC_API_EXTERN_C     extern
#endif
#endif

#endif

