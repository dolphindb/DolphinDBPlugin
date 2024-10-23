/* Copyright 2013 The MathWorks, Inc. */

#ifndef coder_tgtsvc_StatusFlags_hpp
#define coder_tgtsvc_StatusFlags_hpp

#include <stdint.h>
#include "coder_target_services_spec.h"

namespace coder { namespace tgtsvc {

struct CODER_TARGET_SERVICES_EXPORT_CLASS StatusFlags
{
    enum Bit {
        MEMORY_ALLOCATION_FAILED = 0x01,
        COMM_SEND_FAILED         = 0x02,
        UNRECOGNIZED_MSG         = 0x04,
        MSG_APP_ID_OUT_OF_RANGE  = 0x08
    };

    void set(Bit b) { bits_ |= (uint32_t)b; }
    bool get(Bit b) const { return (bits_ & (uint32_t)b) != 0; }

    void clear() { bits_ = 0; }

    static StatusFlags &instance();

    uint32_t bits_;
};

}}

#endif
