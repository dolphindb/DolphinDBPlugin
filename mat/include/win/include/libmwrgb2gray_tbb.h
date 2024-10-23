/* Copyright 2014-2015 The MathWorks, Inc. */
#ifndef _RGB2GRAY_TBB_H_
#define _RGB2GRAY_TBB_H_

#ifndef EXTERN_C
#  ifdef __cplusplus
#    define EXTERN_C extern "C"
#  else
#    define EXTERN_C extern
#  endif
#endif

#ifndef LIBMWRGB2GRAY_TBB_API
#    define LIBMWRGB2GRAY_TBB_API
#endif

#ifdef MATLAB_MEX_FILE
#include "tmwtypes.h"
#else
#include "rtwtypes.h"
#endif

EXTERN_C LIBMWRGB2GRAY_TBB_API void rgb2gray_tbb_real64(
	const real64_T* inputImage,
	const real64_T numPixels,
	real64_T* outputImage);

EXTERN_C LIBMWRGB2GRAY_TBB_API void rgb2gray_tbb_real32(
	const real32_T* inputImage,
	const real64_T numPixels,
	real32_T* outputImage);

EXTERN_C LIBMWRGB2GRAY_TBB_API void rgb2gray_tbb_uint16(
	const uint16_T* inputImage,
	const real64_T numPixels,
	uint16_T* outputImage);

EXTERN_C LIBMWRGB2GRAY_TBB_API void rgb2gray_tbb_uint8(
	const uint8_T* inputImage,
	const real64_T numPixels,
	uint8_T* outputImage);

#endif /* _RGB2GRAY_TBB_H_ */
