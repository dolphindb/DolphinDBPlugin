/* Copyright 2013 The MathWorks, Inc. */

#ifndef _FOREGROUNDDETECTOR_
#define _FOREGROUNDDETECTOR_

#ifndef LIBMWFOREGROUNDDETECTOR_API
#    define LIBMWFOREGROUNDDETECTOR_API
#endif

#ifndef EXTERN_C
#  ifdef __cplusplus
#    define EXTERN_C extern "C"
#  else
#    define EXTERN_C extern
#  endif
#endif

#ifdef MATLAB_MEX_FILE
#include "tmwtypes.h"
#else
#include "rtwtypes.h"
#endif


EXTERN_C LIBMWFOREGROUNDDETECTOR_API 
void foregroundDetector_construct_double_double(void **ptr2ptrObj);
EXTERN_C LIBMWFOREGROUNDDETECTOR_API 
void foregroundDetector_construct_uint8_float(void **ptr2ptrObj);
EXTERN_C LIBMWFOREGROUNDDETECTOR_API
void foregroundDetector_construct_float_float(void **ptr2ptrObj);

EXTERN_C LIBMWFOREGROUNDDETECTOR_API
void foregroundDetector_step_double_double(void *ptrClass, 
                                           const double * inImage, 
                                           boolean_T *mask, 
                                           double learningRate);
 
EXTERN_C LIBMWFOREGROUNDDETECTOR_API
void foregroundDetector_step_uint8_float(void *ptrClass, 
                                         const uint8_T * inImage, 
                                         boolean_T *mask, 
                                         float learningRate);
 
EXTERN_C LIBMWFOREGROUNDDETECTOR_API 
void foregroundDetector_step_float_float(void *ptrClass, 
                                         const float * inImage, 
                                         boolean_T *mask, 
                                         float learningRate);


EXTERN_C LIBMWFOREGROUNDDETECTOR_API
void foregroundDetector_initialize_double_double(
    void *ptrClass,
    int32_T nDims,	
    int32_T *dims,
    int32_T numGaussians, 
    double initialVariance,
    double initialWeight, 
    double varianceThreshold,
    double minBGRatio);
							
EXTERN_C LIBMWFOREGROUNDDETECTOR_API
void foregroundDetector_initialize_uint8_float(
    void *ptrClass,
    int32_T numberOfDims,	
    int32_T *dims,
    int32_T numGaussians, 
    float initialVariance,
    float initialWeight, 
    float varianceThreshold,
    float minBGRatio);
							
EXTERN_C LIBMWFOREGROUNDDETECTOR_API
void foregroundDetector_initialize_float_float(
    void *ptrClass,
    int32_T numberOfDims,	
    int32_T *dims,
    int32_T numGaussians, 
    float initialVariance,
    float initialWeight, 
    float varianceThreshold,
    float minBGRatio);							


EXTERN_C LIBMWFOREGROUNDDETECTOR_API
void foregroundDetector_reset_double_double(void *ptrClass);
EXTERN_C LIBMWFOREGROUNDDETECTOR_API
void foregroundDetector_reset_uint8_float(void *ptrClass);
EXTERN_C LIBMWFOREGROUNDDETECTOR_API 
void foregroundDetector_reset_float_float(void *ptrClass);


EXTERN_C LIBMWFOREGROUNDDETECTOR_API 
void foregroundDetector_release_double_double(void *ptrClass);
EXTERN_C LIBMWFOREGROUNDDETECTOR_API 
void foregroundDetector_release_uint8_float(void *ptrClass);
EXTERN_C LIBMWFOREGROUNDDETECTOR_API
void foregroundDetector_release_float_float(void *ptrClass);

EXTERN_C LIBMWFOREGROUNDDETECTOR_API
void foregroundDetector_deleteObj_float_float(void *ptrClass);
EXTERN_C LIBMWFOREGROUNDDETECTOR_API 
void foregroundDetector_deleteObj_uint8_float(void *ptrClass);
EXTERN_C LIBMWFOREGROUNDDETECTOR_API
void foregroundDetector_deleteObj_double_double(void *ptrClass);

#endif


