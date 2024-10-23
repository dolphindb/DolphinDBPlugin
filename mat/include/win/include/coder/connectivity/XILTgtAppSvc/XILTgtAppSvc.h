/* Copyright 2013-2015 The MathWorks, Inc. */

#ifndef XILTgtAppSvc_h
#define XILTgtAppSvc_h

#include "XILTgtAppSvc_spec.h"
#include "rtwtypes.h"

#if defined (USING_CS_API) || defined(BUILDING_LIBMWCODER_XILTGTAPPSVC)
       
    typedef uint8_T IOUnit_T;   
#else
   
    #include "xilcomms_rtiostream.h"
#endif
       
static const boolean_T XILTGTAPPSVC_ERROR = 0;
static const boolean_T XILTGTAPPSVC_SUCCESS = 1;

#define XIL_RTIOSTREAM_BASED_SERVICE_ID 1
typedef enum {XIL_COMMAND_NOT_COMPLETE=0,
              XIL_COMMAND_COMPLETE,
              XIL_STEP_COMPLETE} XILCommandResponseType;
             
#endif
