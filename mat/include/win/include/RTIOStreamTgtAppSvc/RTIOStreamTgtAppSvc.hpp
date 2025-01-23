/* Copyright 2015 The MathWorks, Inc. */

#ifndef RTIOStreamTgtAppSvc_hpp
#define RTIOStreamTgtAppSvc_hpp

#include "RTIOStreamTgtAppSvc_dll.hpp"
#include "coder/target_services/Message.hpp"
#include "coder/target_services/Application.hpp"
#ifdef BUILDING_LIBMWCODER_RTIOSTREAMTGTAPPSVC
#  include "coder/target_services/CommService.hpp"
#else
#  include "CommService.hpp"
#endif
#include <coder/target_services/fifo.hpp>

#ifndef RTIOSTREAM_MAX_RX_BUFFER_SIZE
#define RTIOSTREAM_MAX_RX_BUFFER_SIZE 25
#endif

class RTIOSTREAMTGTAPPSVC_API RTIOStreamTgtAppSvc : public coder::tgtsvc::Application
{
public:
	RTIOStreamTgtAppSvc();
	virtual ~RTIOStreamTgtAppSvc();

	void handleMessage(coder::tgtsvc::Message *message);

    uint8_t id() { return(coder::tgtsvc::Application::RTIOSTREAM_ID); }

	virtual void handleConnect(bool connected) {};

	int8_t sendDataToCommService(const void * data, size_t size, size_t *sizeSent);
	int8_t receiveDataFromCommService(void * data, size_t size, size_t *sizeRcvd);

    static RTIOStreamTgtAppSvc & get_instance();

private:
    enum {
        RX_BUFFER_SIZE = RTIOSTREAM_MAX_RX_BUFFER_SIZE,
    };

    enum MessageType {
        MIDDLE_MESSAGE = 0x00,
        START_MESSAGE = 0x40,
        END_MESSAGE = 0x80,
        SINGLE_MESSAGE = 0xC0,
        DISTRESS_MESSAGE = 0x20
    };

    RTIOStreamTgtAppSvc(const RTIOStreamTgtAppSvc &);                 
	const RTIOStreamTgtAppSvc& operator=(RTIOStreamTgtAppSvc &);

    bool addToRXData(coder::tgtsvc::Message *bufPtr)
    {
        if (bufPtr != NULL && !fRXData.full())
        {
            fRXData.push(bufPtr);
            return true;
        }
        else
        {
            return false;
        }
    }
    coder::tgtsvc::Message * getRXDataFrontElement()
    {
        coder::tgtsvc::Message * tempRXBuffer = fRXData.front();
        return tempRXBuffer;
    }
    void popRXDataFrontElement()
    {
        coder::tgtsvc::Message * tempRXBuffer = getRXDataFrontElement();
        fRXData.pop();
        delete tempRXBuffer;
    }

private:
    coder::tgtsvc::detail::fifo<coder::tgtsvc::Message*, RX_BUFFER_SIZE> fRXData;
    size_t payloadSentInPreviousMessages;
};
#endif
