/* Copyright 2013 The MathWorks, Inc. */

#ifndef __XILUTILS_H__
#define __XILUTILS_H__

#include "mex.h" 

#ifndef EXTERN_C
    #ifdef __cplusplus
       
        #define EXTERN_C extern "C"
    #else
       
        #define EXTERN_C extern
    #endif
#endif

#ifndef LIBMWXILUTILS_API
   
    #define LIBMWXILUTILS_API
#endif

const int XIL_UTILS_SUCCESS = 0;
const int XIL_UTILS_ERROR = 1;

EXTERN_C LIBMWXILUTILS_API int xilMATLABUtilsCreate(void** const ppXILUtils);

EXTERN_C LIBMWXILUTILS_API mxArray* xilMATLABUtilsGetException(void* const pXILUtils);

EXTERN_C LIBMWXILUTILS_API void xilUtilsDestroy(void* const ppXILUtils);

EXTERN_C LIBMWXILUTILS_API int xilUtilsCallMATLAB(void* const
                                                  pXILUtils,
                                                  const uint8_T nlhs,
                                                  mxArray* plhs[],
                                                  const uint8_T nrhs,
                                                  mxArray* prhs[],
                                                  const char* functionName);

EXTERN_C LIBMWXILUTILS_API void xilUtilsHandleError(void* const pXILUtils,
                                                   const uint8_T nrhs,
                                                   mxArray* prhs[]);

#endif

