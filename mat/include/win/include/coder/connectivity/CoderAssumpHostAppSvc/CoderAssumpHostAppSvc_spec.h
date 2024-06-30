/* Copyright 2015 The MathWorks, Inc. */

#ifndef CoderAssumpHostAppSvc_spec_h
#define CoderAssumpHostAppSvc_spec_h

#ifdef BUILDING_LIBMWCODER_CODERASSUMPHOSTAPPSVC

#define CODERASSUMPHOSTAPPSVC_API                 DLL_EXPORT_SYM
#define CODERASSUMPHOSTAPPSVC_API_EXTERN_C        extern "C" DLL_EXPORT_SYM

#elif defined(DLL_IMPORT_SYM)

#define CODERASSUMPHOSTAPPSVC_API                 DLL_IMPORT_SYM
#define CODERASSUMPHOSTAPPSVC_API_EXTERN_C        extern "C" DLL_IMPORT_SYM

#else

#define CODERASSUMPHOSTAPPSVC_API                  extern
#ifdef __cplusplus
    #define CODERASSUMPHOSTAPPSVC_API_EXTERN_C     extern "C"
#else
    #define CODERASSUMPHOSTAPPSVC_API_EXTERN_C     extern
#endif
#endif

#endif
