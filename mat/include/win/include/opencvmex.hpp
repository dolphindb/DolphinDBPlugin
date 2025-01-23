//////////////////////////////////////////////////////////////////////////////
// This file contains a set of utilities for marshalling data between 
// OpenCV and MATLAB. It supports only CPP-linkage.
//
// Copyright 2014 The MathWorks, Inc.
//  
//////////////////////////////////////////////////////////////////////////////

#ifndef _OCVMEX_
#define _OCVMEX_

#ifndef LIBMWOCVMEX_API
#    define LIBMWOCVMEX_API
#endif

#ifdef MATLAB_MEX_FILE
#include "tmwtypes.h"
#else
#include "rtwtypes.h"
#endif

// no c linkage
#include "mex.h"
#include "opencv2/opencv.hpp"
#include "opencv2/gpu/gpu.hpp"

/////////////////////////////////////////////////////////////////////////////////
// ocvCheckFeaturePointsStruct:
//  Checker for point feature data structure in MATLAB's struct.
//
//  Arguments:
//  ---------
//  in: Pointer to a MATLAB's structure mxArray that represents point feature. 
//      It has the following format:
//
//      Field Name  | Field Requirement | Field Data Type
//      -------------------------------------------------
//      Location    | Required          | Single
//      Scale       | Required          | Single
//      Metric      | Required          | Single
//      Orientation | Optional          | Single
//      Octave      | Optional          | int32
//      Misc        | Optional          | int32
/////////////////////////////////////////////////////////////////////////////////
extern LIBMWOCVMEX_API void ocvCheckFeaturePointsStruct(const mxArray *in);

//////////////////////////////////////////////////////////////////////////////
// ocvStructToKeyPoints:
//  Converts data structure for point feature from MATLAB's struct to 
//  OpenCV's KeyPoint vector
//
//  Arguments:
//  ---------
//  in: Pointer to a MATLAB's structure mxArray that represents point feature. 
//      Its format is explained in the description of ocvCheckFeaturePointsStruct 
//      function.
//
//  keypoints: Reference to OpenCV's cv::KeyPoint vector
//////////////////////////////////////////////////////////////////////////////
extern LIBMWOCVMEX_API void ocvStructToKeyPoints(const mxArray * in, 
                       cv::vector<cv::KeyPoint> &keypoints);

//////////////////////////////////////////////////////////////////////////////
// ocvKeyPointsToStruct:
//  Converts data structure for point feature from OpenCV's KeyPoint vector to
//  MATLAB's struct
//
//  Arguments:
//  ---------
//  in: Reference to OpenCV's KeyPoint vector
//
//  Returns:
//  -------
//  Pointer to a MATLAB's structure mxArray that represents point feature. 
//  Its format is explained in the description of ocvCheckFeaturePointsStruct 
//  function.
//////////////////////////////////////////////////////////////////////////////
extern LIBMWOCVMEX_API mxArray *ocvKeyPointsToStruct(cv::vector<cv::KeyPoint> &in);

//////////////////////////////////////////////////////////////////////////////
// ocvMxArrayToCvRect:
//  Converts data structure for rectangle from MATLAB's struct to 
//  OpenCV's CvRect
//  
//  Arguments:
//  ---------
//  in: Pointer to a MATLAB's structure mxArray that represents a rectangle.
//      The structure must have four scalar valued fields. 
//
//  Returns:
//  -------
//  OpenCV's CvRect
//////////////////////////////////////////////////////////////////////////////
extern LIBMWOCVMEX_API CvRect    ocvMxArrayToCvRect(const mxArray *in);

//////////////////////////////////////////////////////////////////////////////
// ocvCvRectToMxArray:
//  Converts data structure for rectangle from OpenCV's CvRect to
//  MATLAB's struct
//
//  Arguments:
//  ---------
//  in: Pointer to OpenCV's CvRect
//
//  Returns:
//  -------
//  Pointer to a MATLAB's structure mxArray that represents a rectangle. 
//  The structure has four scalar valued fields:
//     x, y, width, height.
//  Here x, y represent the coordinates of the upper left corner of the 
//  rectangle.
//////////////////////////////////////////////////////////////////////////////
extern LIBMWOCVMEX_API mxArray  *ocvCvRectToMxArray(const CvRect *in);

//////////////////////////////////////////////////////////////////////////////
// ocvCvBox2DToMxArray:
//  Converts data structure for rectangle from OpenCV's CvBox2D to
//  MATLAB's struct
//  NOTE:
//  in a CvBox2D structure, x and y represent the coordinates of the center 
//  of the rectangle and NOT the upper left corner of the rectangle as in
//  the CvRect structure or as in the MATLAB RECTANGLE function
//
//  Arguments:
//  ---------
//  in: Pointer to OpenCV's CvBox2D
//
//  Returns:
//  -------
//  Pointer to a MATLAB's structure mxArray that represents a rectangle. 
//  The structure has five scalar valued fields:
//     x_center, y_center, width, height, angle.
//  Here x_center, y_center represent the coordinates of the center of the 
//  rectangle.
//////////////////////////////////////////////////////////////////////////////
extern LIBMWOCVMEX_API mxArray  *ocvCvBox2DToMxArray(const CvBox2D *in);

///////////////////////////////////////////////////////////////////////////////
// cvRectToBoundingBox_T:
//  Converts vector<cv::Rect> to M-by-4 mxArray of bounding boxes
//
//  Arguments:
//  ---------
//  rects: Reference to OpenCV's vector<cv::Rect> 
//
//  Returns:
//  -------
//  Pointer to a MATLAB's mxArray having M-by-4 elements. 
//  Supported data types are real_T (double), real32_T (single or float), 
//  uint8_T (uint8), uint16_T (uint16), uint32_T (uint32), int8_T (int8), 
//  int16_T (int16), or int32_T (int32).
///////////////////////////////////////////////////////////////////////////////
extern LIBMWOCVMEX_API mxArray * ocvCvRectToBoundingBox_double(const std::vector<cv::Rect> & rects);
extern LIBMWOCVMEX_API mxArray * ocvCvRectToBoundingBox_single(const std::vector<cv::Rect> & rects);
extern LIBMWOCVMEX_API mxArray * ocvCvRectToBoundingBox_uint8(const std::vector<cv::Rect> & rects);
extern LIBMWOCVMEX_API mxArray * ocvCvRectToBoundingBox_uint16(const std::vector<cv::Rect> & rects);
extern LIBMWOCVMEX_API mxArray * ocvCvRectToBoundingBox_uint32(const std::vector<cv::Rect> & rects);
extern LIBMWOCVMEX_API mxArray * ocvCvRectToBoundingBox_int8(const std::vector<cv::Rect> & rects);
extern LIBMWOCVMEX_API mxArray * ocvCvRectToBoundingBox_int16(const std::vector<cv::Rect> & rects);
extern LIBMWOCVMEX_API mxArray * ocvCvRectToBoundingBox_int32(const std::vector<cv::Rect> & rects);

/////////////////////////////////////////////////////////////////////////////////
// ocvMxArrayToSize:
//  Converts 2-element mxArray to cv::Size.
//
//  Arguments:
//  ---------
//  in: Pointer to a MATLAB's mxArray having 2 elements.
//  rcInput: boolean flag indicates if input mxArray is [r c] or [x y].
//           By default its value is true. That means input mxArray is 
//           [r c] (i.e. [height width])
//
//  Returns:
//  -------
//  OpenCV's cv::Size

//  Note:
//  ----
//      Empty input ([]) will return cv::Size(0,0);
/////////////////////////////////////////////////////////////////////////////////
extern LIBMWOCVMEX_API cv::Size ocvMxArrayToSize(const mxArray * in, bool rcInput = true);

/////////////////////////////////////////////////////////////////////////////////
// ocvMxArrayToImage_T:
//  Fills up a given cv::Mat with the data from a given mxArray. 
//  The function transposes and interleaves column major mxArray data into 
//  row major cv::Mat. 
//
//  Arguments:
//  ---------
//  in: Pointer to a MATLAB's mxArray having column major data. 
//      Supported data types are real_T (double), real32_T (single or float), 
//      uint8_T (uint8), uint16_T (uint16), uint32_T (uint32), int8_T (int8), 
//      int16_T (int16), int32_T (int32), or boolean_T (bool or logical).
//  out: Reference to OpenCV's cv::Mat with row major data.
//
//  Note:
//  ----
//  - It supports 2D and 3D images.
//  - It reallocates memory for the cv::Mat if needed.
//  - This is not a generic matrix conversion routine!  For 3D images, it takes
//    into account that the OpenCV format uses BGR ordering and thus it  
//    manipulates the data to be compliant with that formatting.
/////////////////////////////////////////////////////////////////////////////////
extern LIBMWOCVMEX_API void ocvMxArrayToImage_double(const mxArray *in, cv::Mat &out);
extern LIBMWOCVMEX_API void ocvMxArrayToImage_single(const mxArray *in, cv::Mat &out);
extern LIBMWOCVMEX_API void ocvMxArrayToImage_uint8(const mxArray *in, cv::Mat &out);
extern LIBMWOCVMEX_API void ocvMxArrayToImage_uint16(const mxArray *in, cv::Mat &out);
extern LIBMWOCVMEX_API void ocvMxArrayToImage_uint32(const mxArray *in, cv::Mat &out);
extern LIBMWOCVMEX_API void ocvMxArrayToImage_int8(const mxArray *in, cv::Mat &out);
extern LIBMWOCVMEX_API void ocvMxArrayToImage_int16(const mxArray *in, cv::Mat &out);
extern LIBMWOCVMEX_API void ocvMxArrayToImage_int32(const mxArray *in, cv::Mat &out);
extern LIBMWOCVMEX_API void ocvMxArrayToImage_bool(const mxArray *in, cv::Mat &out);

/////////////////////////////////////////////////////////////////////////////////
// ocvMxArrayToImage_T:
//  Convert mxArray to cv::Mat (for images) 
//
//  Arguments:
//  ---------
//  in: Pointer to a MATLAB's mxArray having column major data.
//      Supported data types are real_T (double), real32_T (single or float), 
//      uint8_T (uint8), uint16_T (uint16), uint32_T (uint32), int8_T (int8), 
//      int16_T (int16), int32_T (int32), or boolean_T (bool or logical).
//  copyData: boolean flag indicates if data will be copied from the mxArray 
//            to the Mat.
//
//            Its default value is true. That means by default, the function
//            transposes and interleaves (in the case of RGB images) column
//            major mxArray data into row major cv::Mat object.
//
//            The copyData flag may be set to false, in which case no data is
//            copied from the mxArray to the Mat, instead, a new Mat wrapper is 
//            created, and used to point to the mxArray data. Because OpenCV is
//            row based and MATLAB is column based, the columns of the mxArray 
//            become the rows of the Mat. The copyData flag can only be false if
//            the image is 2D.
//
//  Returns:
//  -------
//  OpenCV's smart pointer (cv::Ptr) to a cv::Mat object
//
//  Note:
//  ----
//  Supports 2D and 3D images.
/////////////////////////////////////////////////////////////////////////////////
extern LIBMWOCVMEX_API cv::Ptr<cv::Mat> ocvMxArrayToImage_double(const mxArray *in, const bool copyData = true);
extern LIBMWOCVMEX_API cv::Ptr<cv::Mat> ocvMxArrayToImage_single(const mxArray *in, const bool copyData = true);
extern LIBMWOCVMEX_API cv::Ptr<cv::Mat> ocvMxArrayToImage_uint8(const mxArray *in, const bool copyData = true);
extern LIBMWOCVMEX_API cv::Ptr<cv::Mat> ocvMxArrayToImage_uint16(const mxArray *in, const bool copyData = true);
extern LIBMWOCVMEX_API cv::Ptr<cv::Mat> ocvMxArrayToImage_uint32(const mxArray *in, const bool copyData = true);
extern LIBMWOCVMEX_API cv::Ptr<cv::Mat> ocvMxArrayToImage_int8(const mxArray *in, const bool copyData = true);
extern LIBMWOCVMEX_API cv::Ptr<cv::Mat> ocvMxArrayToImage_int16(const mxArray *in, const bool copyData = true);
extern LIBMWOCVMEX_API cv::Ptr<cv::Mat> ocvMxArrayToImage_int32(const mxArray *in, const bool copyData = true);
extern LIBMWOCVMEX_API cv::Ptr<cv::Mat> ocvMxArrayToImage_bool(const mxArray *in, const bool copyData = true);

/////////////////////////////////////////////////////////////////////////////////
// ocvMxArrayToMat_T:
//  Fills up a given cv::Mat with the data from a given mxArray. 
//  The function transposes and interleaves column major mxArray data into 
//  row major cv::Mat. 
//
//  Arguments:
//  ---------
//  in: Pointer to a MATLAB's mxArray having column major data. data can
//      be n-channel matrices.
//      Supported data types are real_T (double), real32_T (single or float), 
//      uint8_T (uint8), uint16_T (uint16), uint32_T (uint32), int8_T (int8), 
//      int16_T (int16), int32_T (int32), or boolean_T (bool or logical).
//  out: Reference to OpenCV's cv::Mat with row major data.
//
//  Note:
//  ----
//  - The function reallocates memory for the cv::Mat if needed.
//  - This is a generic matrix conversion routine for any number of channels.
/////////////////////////////////////////////////////////////////////////////////
extern LIBMWOCVMEX_API void ocvMxArrayToMat_double(const mxArray *in, cv::Mat &out);
extern LIBMWOCVMEX_API void ocvMxArrayToMat_single(const mxArray *in, cv::Mat &out);
extern LIBMWOCVMEX_API void ocvMxArrayToMat_uint8(const mxArray *in, cv::Mat &out);
extern LIBMWOCVMEX_API void ocvMxArrayToMat_uint16(const mxArray *in, cv::Mat &out);
extern LIBMWOCVMEX_API void ocvMxArrayToMat_uint32(const mxArray *in, cv::Mat &out);
extern LIBMWOCVMEX_API void ocvMxArrayToMat_int8(const mxArray *in, cv::Mat &out);
extern LIBMWOCVMEX_API void ocvMxArrayToMat_int16(const mxArray *in, cv::Mat &out);
extern LIBMWOCVMEX_API void ocvMxArrayToMat_int32(const mxArray *in, cv::Mat &out);
extern LIBMWOCVMEX_API void ocvMxArrayToMat_bool(const mxArray *in, cv::Mat &out);

/////////////////////////////////////////////////////////////////////////////////
// ocvMxArrayToMat_T:
//  Convert mxArray to cv::Mat
//
//  Arguments:
//  ---------
//  in: Pointer to a MATLAB's mxArray having column major data. Data can
//      be n-channel matrices. 
//      Supported data types are real_T (double), real32_T (single or float), 
//      uint8_T (uint8), uint16_T (uint16), uint32_T (uint32), int8_T (int8), 
//      int16_T (int16), int32_T (int32), or boolean_T (bool or logical).
//  copyData: boolean flag indicates if data will be copied from the mxArray 
//            to the Mat.
//
//            Its default value is true. That means by default, the function
//            transposes and interleaves (in the case of RGB images) column
//            major mxArray data into row major cv::Mat object.
//
//            The copyData flag may be set to false, in which case no data is
//            copied from the mxArray to the Mat, instead, a new Mat wrapper is 
//            created, and used to point to the mxArray data. Because OpenCV is
//            row based and MATLAB is column based, the columns of the mxArray 
//            become the rows of the Mat. The copyData flag can only be false if
//            the image is 2D.
//
//  Returns:
//  -------
//  OpenCV's smart pointer (cv::Ptr) to a cv::Mat object
/////////////////////////////////////////////////////////////////////////////////
extern LIBMWOCVMEX_API cv::Ptr<cv::Mat> ocvMxArrayToMat_double(const mxArray *in, const bool copyData = true);
extern LIBMWOCVMEX_API cv::Ptr<cv::Mat> ocvMxArrayToMat_single(const mxArray *in, const bool copyData = true);
extern LIBMWOCVMEX_API cv::Ptr<cv::Mat> ocvMxArrayToMat_uint8(const mxArray *in, const bool copyData = true);
extern LIBMWOCVMEX_API cv::Ptr<cv::Mat> ocvMxArrayToMat_uint16(const mxArray *in, const bool copyData = true);
extern LIBMWOCVMEX_API cv::Ptr<cv::Mat> ocvMxArrayToMat_uint32(const mxArray *in, const bool copyData = true);
extern LIBMWOCVMEX_API cv::Ptr<cv::Mat> ocvMxArrayToMat_int8(const mxArray *in, const bool copyData = true);
extern LIBMWOCVMEX_API cv::Ptr<cv::Mat> ocvMxArrayToMat_int16(const mxArray *in, const bool copyData = true);
extern LIBMWOCVMEX_API cv::Ptr<cv::Mat> ocvMxArrayToMat_int32(const mxArray *in, const bool copyData = true);
extern LIBMWOCVMEX_API cv::Ptr<cv::Mat> ocvMxArrayToMat_bool(const mxArray *in, const bool copyData = true);

/////////////////////////////////////////////////////////////////////////////////
// ocvMxArrayFromImage_T:
//  Creates an mxArray from cv::Mat.
//
//  Arguments:
//  ---------
//  in: Reference to OpenCV's cv::Mat with row major data.
//  out: Pointer to a MATLAB's mxArray having column major data. 
//       Supported data types are real_T (double), real32_T (single or float), 
//       uint8_T (uint8), uint16_T (uint16), uint32_T (uint32), int8_T (int8), 
//       int16_T (int16), int32_T (int32), or boolean_T (bool or logical).
//
//  Note:
//  ----
//  This is not a generic matrix conversion routine!  For 3D images, it takes
//  into account that the OpenCV format uses BGR ordering and thus it  
//  manipulates the data to be compliant with that formatting.
/////////////////////////////////////////////////////////////////////////////////
extern LIBMWOCVMEX_API mxArray *ocvMxArrayFromImage_double(const cv::Mat &in);
extern LIBMWOCVMEX_API mxArray *ocvMxArrayFromImage_single(const cv::Mat &in);
extern LIBMWOCVMEX_API mxArray *ocvMxArrayFromImage_uint8(const cv::Mat &in);
extern LIBMWOCVMEX_API mxArray *ocvMxArrayFromImage_uint16(const cv::Mat &in);
extern LIBMWOCVMEX_API mxArray *ocvMxArrayFromImage_uint32(const cv::Mat &in);
extern LIBMWOCVMEX_API mxArray *ocvMxArrayFromImage_int8(const cv::Mat &in);
extern LIBMWOCVMEX_API mxArray *ocvMxArrayFromImage_int16(const cv::Mat &in);
extern LIBMWOCVMEX_API mxArray *ocvMxArrayFromImage_int32(const cv::Mat &in);
extern LIBMWOCVMEX_API mxArray *ocvMxArrayFromImage_bool(const cv::Mat &in);

/////////////////////////////////////////////////////////////////////////////////
// ocvMxArrayFromMat_T:
//  Creates an mxArray from cv::Mat.
//
//  Arguments:
//  ---------
//  in: Reference to OpenCV's cv::Mat with row major data.
//
//  Returns:
//  -------
//  Pointer to a MATLAB's mxArray having column major data.
//  Supported data types are real_T (double), real32_T (single or float), 
//  uint8_T (uint8), uint16_T (uint16), uint32_T (uint32), int8_T (int8), 
//  int16_T (int16), int32_T (int32), or boolean_T (bool or logical).
//
//  Note:
//  ----
//  This is a generic matrix conversion routine for any number of channels
//        It does not take into account the BGR ordering
/////////////////////////////////////////////////////////////////////////////////
extern LIBMWOCVMEX_API mxArray *ocvMxArrayFromMat_double(const cv::Mat &in);
extern LIBMWOCVMEX_API mxArray *ocvMxArrayFromMat_single(const cv::Mat &in);
extern LIBMWOCVMEX_API mxArray *ocvMxArrayFromMat_uint8(const cv::Mat &in);
extern LIBMWOCVMEX_API mxArray *ocvMxArrayFromMat_uint16(const cv::Mat &in);
extern LIBMWOCVMEX_API mxArray *ocvMxArrayFromMat_uint32(const cv::Mat &in);
extern LIBMWOCVMEX_API mxArray *ocvMxArrayFromMat_int8(const cv::Mat &in);
extern LIBMWOCVMEX_API mxArray *ocvMxArrayFromMat_int16(const cv::Mat &in);
extern LIBMWOCVMEX_API mxArray *ocvMxArrayFromMat_int32(const cv::Mat &in);
extern LIBMWOCVMEX_API mxArray *ocvMxArrayFromMat_bool(const cv::Mat &in);

/////////////////////////////////////////////////////////////////////////////////
// ocvMxArrayFromVector:
//  Converts numeric vector<T> to mxArray
//
//  Arguments:
//  ---------
//  v: Reference to vector<T>. Supported data types are real_T, real32_T, 
//     uint8_T, uint16_T, uint32_T, int8_T, int16_T, int32_T, or boolean_T
//
//  Returns:
//  -------
//  Pointer to a MATLAB's mxArray
/////////////////////////////////////////////////////////////////////////////////
extern LIBMWOCVMEX_API mxArray *ocvMxArrayFromVector(const std::vector<real_T> &v);
extern LIBMWOCVMEX_API mxArray *ocvMxArrayFromVector(const std::vector<real32_T> &v);
extern LIBMWOCVMEX_API mxArray *ocvMxArrayFromVector(const std::vector<uint8_T> &v);
extern LIBMWOCVMEX_API mxArray *ocvMxArrayFromVector(const std::vector<uint16_T> &v);
extern LIBMWOCVMEX_API mxArray *ocvMxArrayFromVector(const std::vector<uint32_T> &v);
extern LIBMWOCVMEX_API mxArray *ocvMxArrayFromVector(const std::vector<int8_T> &v);
extern LIBMWOCVMEX_API mxArray *ocvMxArrayFromVector(const std::vector<int16_T> &v);
extern LIBMWOCVMEX_API mxArray *ocvMxArrayFromVector(const std::vector<int32_T> &v);
extern LIBMWOCVMEX_API mxArray *ocvMxArrayFromVector(const std::vector<boolean_T> &v);

///////////////////////////////////////////////////////////////////////////////
// ocvMxArrayFromPoints2f:
//  Converts vector<cv::Points2f> to mxArray 
//
//  Arguments:
//  ---------
//  points: Reference to OpenCV's vector<cv::Points2f>
//
//  Returns:
//  -------
//  Pointer to a MATLAB's mxArray.
//  The mxArray has to be managed by the caller.
///////////////////////////////////////////////////////////////////////////////
extern mxArray *ocvMxArrayFromPoints2f(const std::vector<cv::Point2f> &points);

#endif
