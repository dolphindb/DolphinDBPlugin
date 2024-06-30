/* Copyright 2013 The MathWorks, Inc. */

#ifndef __XILCOMMSRTIOSTREAM_H__
#define __XILCOMMSRTIOSTREAM_H__

/* Shipping C interface to the rtiostream xilcomms service in the
 * libmwxilcomms_rtiostream shared library. */
#include "tmwtypes.h"
#include "mex.h"
#ifndef EXTERN_C
    #ifdef __cplusplus
        /* support C++ call sites */
        #define EXTERN_C                       extern "C"
    #else
        /* support C call sites */
        #define EXTERN_C                       extern
    #endif
#endif

/* detect shipping call site */
#ifndef LIBMWXILCOMMS_RTIOSTREAM_API
    /* default definition */
    #define LIBMWXILCOMMS_RTIOSTREAM_API
    #include "rtiostream_loadlib.h"        /* libH_type */
#else
    #include "rtiostreamutils/rtiostream_loadlib_h.hpp" /* libH_type */
#endif

const boolean_T XILCOMMS_RTIOSTREAM_ERROR = 1;
const boolean_T XILCOMMS_RTIOSTREAM_SUCCESS = 0;
    
typedef struct {
  libH_type libH;
  char * lib;
  mxArray * MATLABObject;
  double streamID;
  double recvTimeout;
  double sendTimeout;
  boolean_T isRtIOStreamCCall;
  mxClassID ioMxClassID;
  uint8_T ioDataSize;  
  size_t targetRecvBufferSizeBytes;
  size_t targetSendBufferSizeBytes;
} XIL_RtIOStreamData_T;

typedef struct {
  char* componentBlockPath;
  char* SILPILInterfaceFcnStr;
  int32_T inTheLoopType;
} SIL_DEBUGGING_DATA_T;

/* Create xilcomms service */
EXTERN_C LIBMWXILCOMMS_RTIOSTREAM_API boolean_T xilCommsCreate(
  void** const ppXILCommsRTIOStream,
  const void* const pRTIOStreamData,
  const void* const pSILDebuggingData,
  const int memUnitSizeBytes,
  const void* const pMemUnitXformer,
  void* const pXILUtils);

/* Destroy xilcomms service */
EXTERN_C LIBMWXILCOMMS_RTIOSTREAM_API void xilCommsDestroy(
  void* const pXILCommsRTIOStream);

/* Receive data and dispatch to owning application */
EXTERN_C LIBMWXILCOMMS_RTIOSTREAM_API boolean_T xilCommsRun(
  void* const pXILCommsRTIOStream,
  void* const pXILUtilsVoid);

/* Register an application with xilcomms */
EXTERN_C LIBMWXILCOMMS_RTIOSTREAM_API void xilCommsRegisterApplication(
  void* const pXILCommsRTIOStream,
  const void* const pApplication);

#endif
