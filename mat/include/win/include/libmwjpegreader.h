/* Copyright 2015 The MathWorks, Inc. */

/* This file is used for imread code generation. The API is for internal 
 purposes and is subject to change. */

#ifndef _JPEGREADER_H_
#define _JPEGREADER_H_

#ifndef EXTERN_C
#  ifdef __cplusplus
#    define EXTERN_C extern "C"
#  else
#    define EXTERN_C extern
#  endif
#endif

#ifndef LIBMWJPEGREADER_API
#    define LIBMWJPEGREADER_API
#endif

#ifdef MATLAB_MEX_FILE
#include "tmwtypes.h"
#else
#include "rtwtypes.h"
#endif

EXTERN_C LIBMWJPEGREADER_API void jpegreader_getimagesize(
	const char *filename, 
    real64_T   *imgDims,
    int8_T     *fileStatus,
    int8_T     *colorSpaceStatus,
    real64_T   *libjpegMsgCode, 
    char       *libjpegErrWarnBuffer,
    int8_T     *errWarnFlag);

EXTERN_C LIBMWJPEGREADER_API void jpegreader_uint8(
	const char     *filename, 
	uint8_T        *img,
    const real64_T *imgDims,
    const real64_T  imgNumDims,
    int8_T         *fileStatus,
    int8_T         *libjpegReadDone,
    real64_T       *libjpegMsgCode, 
    char           *libjpegErrWarnBuffer,
    int8_T         *errWarnFlag);

#endif /* _JPEGREADER_H_ */
