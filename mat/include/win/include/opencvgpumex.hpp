///////////////////////////////////////////////////////////////////////////////
// This file exports utility functions to convert between OpenCV's cv::GpuMat
// and mxArray's containing GPU data.
//
// Copyright 2015 The MathWorks, Inc.
///////////////////////////////////////////////////////////////////////////////

#ifndef _OCVGPUMEX_
#define _OCVGPUMEX_

#ifndef LIBMWOCVGPUMEX_API
#    define LIBMWOCVGPUMEX_API
#endif

// no c linkage
#include "mex.h"
#include "tmwtypes.h"
#include "opencv2/opencv.hpp"
#include "opencv2/gpu/gpu.hpp"

///////////////////////////////////////////////////////////////////////////////
// ocvMxGpuArrayToGpuMat_DataType:
//   Creates cv::gpu::GpuMat from an mxArray conatining GPU data. 
//
//  Arguments:
//  ---------
//  in: Pointer to a MATLAB's mxArray conatining GPU data.
//      Supported data types are real_T (double), real32_T (single or float), 
//      uint8_T (uint8), uint16_T (uint16), uint32_T (uint32), int8_T (int8), 
//      int16_T (int16), int32_T (int32), or boolean_T (bool or logical).
//
//  Returns:
//  -------
//  OpenCV's smart pointer (cv::Ptr) to a cv::gpu::GpuMat object
//
//  Note:
//  ----
//  GpuMat only support 2D arrays. 
///////////////////////////////////////////////////////////////////////////////
extern LIBMWOCVGPUMEX_API cv::Ptr<cv::gpu::GpuMat> ocvMxGpuArrayToGpuMat_double(const mxArray * in);
extern LIBMWOCVGPUMEX_API cv::Ptr<cv::gpu::GpuMat> ocvMxGpuArrayToGpuMat_single(const mxArray * in);
extern LIBMWOCVGPUMEX_API cv::Ptr<cv::gpu::GpuMat> ocvMxGpuArrayToGpuMat_uint8(const mxArray * in);
extern LIBMWOCVGPUMEX_API cv::Ptr<cv::gpu::GpuMat> ocvMxGpuArrayToGpuMat_uint16(const mxArray * in);
extern LIBMWOCVGPUMEX_API cv::Ptr<cv::gpu::GpuMat> ocvMxGpuArrayToGpuMat_uint32(const mxArray * in);
extern LIBMWOCVGPUMEX_API cv::Ptr<cv::gpu::GpuMat> ocvMxGpuArrayToGpuMat_int8(const mxArray * in);
extern LIBMWOCVGPUMEX_API cv::Ptr<cv::gpu::GpuMat> ocvMxGpuArrayToGpuMat_int16(const mxArray * in);
extern LIBMWOCVGPUMEX_API cv::Ptr<cv::gpu::GpuMat> ocvMxGpuArrayToGpuMat_int32(const mxArray * in);
extern LIBMWOCVGPUMEX_API cv::Ptr<cv::gpu::GpuMat> ocvMxGpuArrayToGpuMat_bool(const mxArray * in);

///////////////////////////////////////////////////////////////////////////////
// ocvMxGpuArrayFromGpuMat_*:
//   Creates an mxArray from a cv::gpu::GpuMat.
//
//  Arguments:
//  ---------
//  in: Reference to OpenCV's cv::gpu::GpuMat object
//
//  Returns:
//  -------
//  Pointer to a MATLAB's mxArray conatining GPU data. 
//  Supported data types are real_T (double), real32_T (single or float), 
//  uint8_T (uint8), uint16_T (uint16), uint32_T (uint32), int8_T (int8), 
//  int16_T (int16), int32_T (int32), or boolean_T (bool or logical).
//
//  Note:
//  ----
//  GpuMat only support 2D arrays. 
///////////////////////////////////////////////////////////////////////////////

extern LIBMWOCVGPUMEX_API mxArray * ocvMxGpuArrayFromGpuMat_double(const cv::gpu::GpuMat & in);
extern LIBMWOCVGPUMEX_API mxArray * ocvMxGpuArrayFromGpuMat_single(const cv::gpu::GpuMat & in);
extern LIBMWOCVGPUMEX_API mxArray * ocvMxGpuArrayFromGpuMat_uint8(const cv::gpu::GpuMat & in);
extern LIBMWOCVGPUMEX_API mxArray * ocvMxGpuArrayFromGpuMat_uint16(const cv::gpu::GpuMat & in);
extern LIBMWOCVGPUMEX_API mxArray * ocvMxGpuArrayFromGpuMat_uint32(const cv::gpu::GpuMat & in);
extern LIBMWOCVGPUMEX_API mxArray * ocvMxGpuArrayFromGpuMat_int8(const cv::gpu::GpuMat & in);
extern LIBMWOCVGPUMEX_API mxArray * ocvMxGpuArrayFromGpuMat_int16(const cv::gpu::GpuMat & in);
extern LIBMWOCVGPUMEX_API mxArray * ocvMxGpuArrayFromGpuMat_int32(const cv::gpu::GpuMat & in);
extern LIBMWOCVGPUMEX_API mxArray * ocvMxGpuArrayFromGpuMat_bool(const cv::gpu::GpuMat & in);

#endif
