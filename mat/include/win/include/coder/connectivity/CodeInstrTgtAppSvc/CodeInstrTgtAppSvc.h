/* Copyright 2013-2015 The MathWorks, Inc. */

#ifndef CodeInstrTgtAppSvc_h
#define CodeInstrTgtAppSvc_h

#include "CodeInstrTgtAppSvc_spec.h"
#include "rtwtypes.h"

#if defined (USING_CS_API) || defined(BUILDING_LIBMWCODER_CODEINSTRTGTAPPSVC)
       
    typedef uint8_T IOUnit_T;   
#else
   
    #include "xilcomms_rtiostream.h"
#endif
       
static const boolean_T CODEINSTRTGTAPPSVC_ERROR = 0;
static const boolean_T CODEINSTRTGTAPPSVC_SUCCESS = 1;

#define CODEINSTR_RTIOSTREAM_BASED_SERVICE_ID 2
             
#endif
