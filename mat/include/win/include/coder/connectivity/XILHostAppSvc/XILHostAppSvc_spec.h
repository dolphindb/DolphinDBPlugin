/* Copyright 2015 The MathWorks, Inc. */

#ifndef XILHostAppSvc_spec_h
#define XILHostAppSvc_spec_h

#ifdef BUILDING_LIBMWCODER_XILHOSTAPPSVC

#define XILHOSTAPPSVC_API                 DLL_EXPORT_SYM
#define XILHOSTAPPSVC_API_EXTERN_C        extern "C" DLL_EXPORT_SYM

#elif defined(DLL_IMPORT_SYM)

#define XILHOSTAPPSVC_API                 DLL_IMPORT_SYM
#define XILHOSTAPPSVC_API_EXTERN_C        extern "C" DLL_IMPORT_SYM

#else

#define XILHOSTAPPSVC_API                  extern
#ifdef __cplusplus
    #define XILHOSTAPPSVC_API_EXTERN_C     extern "C"
#else
    #define XILHOSTAPPSVC_API_EXTERN_C     extern
#endif
#endif

#endif
