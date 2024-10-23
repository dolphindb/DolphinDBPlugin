/* Copyright 2015 The MathWorks, Inc. */

#ifndef RTIOStreamTgtAppSvcCInterf_h
#define RTIOStreamTgtAppSvcCInterf_h

#include "RTIOStreamTgtAppSvc_dll.hpp"
#include <stddef.h>

#if defined(_MSC_VER) && (_MSC_VER < 1600)
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
#elif defined(__LCC__)
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
#else
#include <stdint.h>
#endif
RTIOSTREAMTGTAPPSVC_API_C int8_t sendToCommSvc(const void *data, size_t size, size_t *size_sent);
RTIOSTREAMTGTAPPSVC_API_C int8_t receiveFromCommSvc(void *data, size_t size, size_t *size_sent);

#endif
