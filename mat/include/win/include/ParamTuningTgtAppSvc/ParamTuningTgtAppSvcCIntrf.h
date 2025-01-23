/* Copyright 2014 MathWorks, Inc. */

#ifndef ParamTuningTgtAppSvcCIntrf_h
#define ParamTuningTgtAppSvcCIntrf_h

#include "ParamTuningTgtAppSvc_dll.hpp"

#ifdef SL_INTERNAL
  #include "simulinkcoder_capi/rtw_modelmap.h"
#else
  #include "rtw_modelmap.h"
#endif

PARAMTUNINGTGTAPPSVC_API_C int  startParamTuningTgtAppSvc();
PARAMTUNINGTGTAPPSVC_API_C int  terminateParamTuningTgtAppSvc();
PARAMTUNINGTGTAPPSVC_API_C void tunePendingParameterChanges(rtwCAPI_ModelMappingInfo *pMMI);

#endif
