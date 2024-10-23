/* Copyright 2011-2015 The MathWorks, Inc. */

#ifndef __XILUTILS_SL_H__
#define __XILUTILS_SL_H__

#ifndef EXTERN_C
    #ifdef __cplusplus
       
        #define EXTERN_C extern "C"
    #else
       
        #define EXTERN_C extern
    #endif
#endif

#ifndef LIBMWXILUTILS_SL_API
   
    #define LIBMWXILUTILS_SL_API
   
    #include "simstruc.h"
#else
    #ifndef MATLAB_MEX_FILE
        #define MATLAB_MEX_FILE
    #endif
   
    #include "simstruct/simstruc.h"
#endif

EXTERN_C LIBMWXILUTILS_SL_API int xilSimulinkUtilsCreate(void** const ppXILUtils,
                                                         SimStruct* const S);

EXTERN_C LIBMWXILUTILS_SL_API void xilSimulinkUtilsStaticErrorHandle(const uint8_T nrhs,
                                                                     mxArray* prhs[],
                                                                     SimStruct* const S);

#endif
