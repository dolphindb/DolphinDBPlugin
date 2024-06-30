/* Copyright 2013 MathWorks, Inc. */

#ifndef coder_tgtsvc_CommServiceBase_hpp
#define coder_tgtsvc_CommServiceBase_hpp

#include <stddef.h>
#include "coder_target_services_spec.h"
#include "Application.hpp"
#include "Atomic.hpp"
#include "MessageDictionary.hpp"

namespace coder { namespace tgtsvc {

template <class Derived>
class CODER_TARGET_SERVICES_EXPORT_TEMPLATE CommServiceBase
{
public:
    enum TestState {
        NO_TEST,
        IN_TEST,
        OUT_TEST
    };

    CommServiceBase()
    {
        testCount_ = 0;
        testSize_ = 0;
        testState_ = NO_TEST;
        connected_ = false;
    }
    ~CommServiceBase() {
        Application::connectionChanged(false);
    }

    bool connected() const { return connected_; }

    TSEStatus sendMessage(Message *message, Message::Priority priority=Message::NORMAL_PRIORITY) {
        return static_cast<Derived*>(this)->sendMessage(message, priority);
    }

    void operator()()
    {
        switch (testState_) {
        case NO_TEST:
            break;

        case IN_TEST:
            if (testCount_ > 0) {
                Message *test = Message::alloc(testSize_);
                if (test != NULL) {
                    new (test) TestDataMsg(testSize_);
                    TSEStatus s = sendMessage(test);
                    if (s != TSE_SUCCESS) {
                        delete test;
                    } else {
                        --testCount_;
                    }
                }
            }
            if (testCount_ == 0) {
                TestConcludedMsg *tcm = new TestConcludedMsg();
                if (tcm != NULL) {
                    TSEStatus s = sendMessage(tcm);
                    if (s != TSE_SUCCESS) {
                        delete tcm;
                    } else {
                        testState_ = NO_TEST;
                    }
                }
            }
            break;

        case OUT_TEST:
            break;

        default:
            break;
        }
    }

protected:
    typename Atomic<bool>::type connected_;
    typename Atomic<uint16_t>::type testSize_;
    typename Atomic<TestState>::type testState_;
    typename Atomic<uint32_t>::type testCount_;

    void handleCSMessage(Message *message)
    {
        switch(message->header().appFun_) {

        case PingMsg::ID:
            new (message) PingResponseMsg;
            sendMessage(message);
            break;

        case PingResponseMsg::ID:
            delete message;
            break;

        case ConnectMsg::ID:
            new (message) ConnectResponseMsg;
            sendMessage(message);
            connected_ = true;
            Application::connectionChanged(connected_);
            break;

        case HeartbeatMsg::ID:
            {
                HeartbeatResponseMsg *hrm = new (message) HeartbeatResponseMsg;
                assert(hrm->payloadCapacity() > HeartbeatResponseMsg::PAYLOAD_SIZE);
                hrm->statusFlags_ = StatusFlags::instance();
                StatusFlags::instance().clear();
                sendMessage(message);
                break;
            }

        case InTestStartMsg::ID:
            {
                InTestStartMsg *itsm = static_cast<InTestStartMsg*>(message);
                testCount_ = itsm->testMsgCount_;
                testSize_ = itsm->testMsgSize_;
                testState_ = IN_TEST;
                delete message;
                break;
            }

        case OutTestStartMsg::ID:
            {
                OutTestStartMsg *otsm = static_cast<OutTestStartMsg*>(message);
                testCount_ = otsm->testMsgCount_;
                testSize_ = otsm->testMsgSize_;
                testState_ = OUT_TEST;
                delete message;
                break;
            }

        case TestDataMsg::ID:
            if (testState_ == OUT_TEST) {
                if (message->payloadSize() == testSize_) {
                    --testCount_;
                }
            }
            delete message;
            break;

        case TestConcludedMsg::ID:
            {
                if (testState_ == OUT_TEST) {
                    OutTestResultMsg *otrm = new (message) OutTestResultMsg();
                    if (testCount_ != 0) otrm->result_ = 0xFF;
                    TSEStatus s = sendMessage(otrm, Message::HIGH_PRIORITY);
                    if (s != TSE_SUCCESS) {
                        delete otrm;
                    } else {
                        testState_ = NO_TEST;
                    }
                }
            }
            break;

        default:
            StatusFlags::instance().set(StatusFlags::UNRECOGNIZED_MSG);
            delete message;
            break;
        }
    }

private:
   
	CommServiceBase(const CommServiceBase &cpy);

	CommServiceBase &operator=(const CommServiceBase &cpy);
};

}}
#endif
