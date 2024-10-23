/* Copyright 2013-2015 The MathWorks, Inc. */

#ifndef XILTgtAppSvc_spec_h
#define XILTgtAppSvc_spec_h

#ifdef BUILDING_LIBMWCODER_XILTGTAPPSVC

#define XILTGTAPPSVC_API                 DLL_EXPORT_SYM
#define XILTGTAPPSVC_API_EXTERN_C        extern "C" DLL_EXPORT_SYM

#elif defined(DLL_IMPORT_SYM)

#define XILTGTAPPSVC_API                 DLL_IMPORT_SYM
#define XILTGTAPPSVC_API_EXTERN_C        extern "C" DLL_IMPORT_SYM

#else

#define XILTGTAPPSVC_API                  extern
#ifdef __cplusplus
    #define XILTGTAPPSVC_API_EXTERN_C     extern "C"
#else
    #define XILTGTAPPSVC_API_EXTERN_C     extern
#endif
#endif

#endif

