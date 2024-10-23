/* Copyright 2013-2015 The MathWorks, Inc. */

#ifndef coder_tgtsvc_Application_hpp
#define coder_tgtsvc_Application_hpp

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include "coder_target_services_spec.h"

namespace coder { namespace tgtsvc {

enum TSEStatus {
    TSE_SUCCESS = 0,
    TSE_ERROR = 1, 
    TSE_RESOURCE_UNAVAILABLE = 2 
};

class Message;

class CODER_TARGET_SERVICES_EXPORT_CLASS Application
{
public:

    enum {
        TO_ASYNC_QUEUE_ID = 1,
        PARAM_TUNING_ID   = 2,
        SIL_PIL_ID = 3,
        CODE_INSTRUMENTATION_ID = 4,
        CODER_ASSUMPTIONS_ID = 5,
        RTIOSTREAM_ID = 6,
        APPLICATION_COUNT = 7
    } ID;

    Application() {}
    virtual ~Application() {}

    virtual uint8_t id() = 0;

    virtual void handleMessage(Message *message) = 0;

	virtual void handleConnect(bool connected) = 0;

    static Application *findById(uint8_t id);

    static void dispatch(Message *message);

    static void connectionChanged(bool connected);

protected:
   
    void enable() {
        assert(id() < APPLICATION_COUNT);
        assert(registry_[id()] == NULL);
        registry_[id()] = this;
    }

    void disable() {
        assert(id() < APPLICATION_COUNT);
        assert(registry_[id()] != NULL);
        registry_[id()] = NULL;
    }

private:
    static Application *registry_[APPLICATION_COUNT];

	Application(const Application &);

	const Application &operator=(const Application &);
};

}}

#endif
