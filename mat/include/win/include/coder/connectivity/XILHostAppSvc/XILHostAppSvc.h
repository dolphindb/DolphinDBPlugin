/* Copyright 2013-2015 The MathWorks, Inc. */

#ifndef XILHostAppSvc_h
#define XILHostAppSvc_h

#include "tmwtypes.h"

static const boolean_T XILHOSTAPPSVC_ERROR = 0;
static const boolean_T XILHOSTAPPSVC_SUCCESS = 1;

#define XIL_RTIOSTREAM_BASED_SERVICE_ID 1
typedef enum {XIL_COMMAND_NOT_COMPLETE=0,
              XIL_COMMAND_COMPLETE,
              XIL_STEP_COMPLETE} XILCommandResponseType;
             
#endif
