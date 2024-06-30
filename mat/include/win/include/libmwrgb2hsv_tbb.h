/* Copyright 2015 The MathWorks, Inc. */
#ifndef _RGB2HSV_H_
#define _RGB2HSV_H_

#ifndef EXTERN_C
#  ifdef __cplusplus
#    define EXTERN_C extern "C"
#  else
#    define EXTERN_C extern
#  endif
#endif

#ifndef LIBMWRGB2HSV_TBB_API
#    define LIBMWRGB2HSV_TBB_API
#endif

#ifdef MATLAB_MEX_FILE
#include "tmwtypes.h"
#else
#include "rtwtypes.h"
#endif

EXTERN_C LIBMWRGB2HSV_TBB_API void rgb2hsv_tbb_real64(
	const real64_T* inputImage, 
	const real64_T numPixels,
	real64_T* outputImage);

EXTERN_C LIBMWRGB2HSV_TBB_API void rgb2hsv_tbb_real32(
	const real32_T* inputImage, 
	const real64_T numPixels,
	real32_T* outputImage);

EXTERN_C LIBMWRGB2HSV_TBB_API void rgb2hsv_tbb_uint16(
	const uint16_T* inputImage, 
	const real64_T numPixels,
	real64_T* outputImage);

EXTERN_C LIBMWRGB2HSV_TBB_API void rgb2hsv_tbb_uint8(
	const uint8_T* inputImage, 
	const real64_T numPixels,
	real64_T* outputImage);

#endif /* _RGB2HSV_H_ */
