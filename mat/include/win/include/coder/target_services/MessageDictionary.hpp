/* Copyright 2013 MathWorks, Inc. */

#include <coder/target_services/Message.hpp>
#include <coder/target_services/StatusFlags.hpp>

namespace coder { namespace tgtsvc {

enum {
    PING_MSG_ID               = 1,
    PING_RESPONSE_MSG_ID      = 2,
    CONNECT_MSG_ID            = 3,
    CONNECT_RESPONSE_MSG_ID   = 4,
    HEARTBEAT_MSG_ID          = 5,
    HEARTBEAT_RESPONSE_MSG_ID = 6,
    IN_TEST_START_MSG_ID      = 7,
    TEST_DATA_MSG_ID          = 8,
    OUT_TEST_START_MSG_ID     = 9,
    OUT_TEST_RESULT_MSG_ID    = 10,
    TEST_CONCLUDED_MSG_ID     = 11,
    COMM_SERVICE_ID = 0xFF
};

class PingMsg : public Message
{
public:
    enum {
        ID = PING_MSG_ID,
        PAYLOAD_SIZE = 0
    };

    PingMsg() {
        MessageHeader &h = header();
        h.payloadSize_ = PAYLOAD_SIZE;
        h.appId_ = COMM_SERVICE_ID;
        h.appFun_ = ID;
    }
};

class PingResponseMsg : public Message
{
public:
    enum {
        ID = PING_RESPONSE_MSG_ID,
        PAYLOAD_SIZE = 0
    };

    PingResponseMsg() {
        MessageHeader &h = header();
        h.payloadSize_ = PAYLOAD_SIZE;
        h.appId_ = COMM_SERVICE_ID;
        h.appFun_ = ID;
    }
};

class ConnectMsg : public Message
{
public:
    enum {
        ID = CONNECT_MSG_ID,
        PAYLOAD_SIZE = 0
    };

    ConnectMsg() {
        MessageHeader &h = header();
        h.payloadSize_ = PAYLOAD_SIZE;
        h.appId_ = COMM_SERVICE_ID;
        h.appFun_ = ID;
    }
};

class ConnectResponseMsg : public Message
{
public:
    enum {
        ID = CONNECT_RESPONSE_MSG_ID,
        PAYLOAD_SIZE = sizeof(uint16_t)
    };

    ConnectResponseMsg() {
        MessageHeader &h = header();
        h.payloadSize_ = PAYLOAD_SIZE;
        h.appId_ = COMM_SERVICE_ID;
        h.appFun_ = ID;
        maxPayload_ = maxPayloadCapacity();   
    }

    uint16_t maxPayload_;
};

class HeartbeatMsg : public Message
{
public:
    enum {
        ID = HEARTBEAT_MSG_ID,
        PAYLOAD_SIZE = 0
    };

    HeartbeatMsg() {
        MessageHeader &h = header();
        h.payloadSize_ = PAYLOAD_SIZE;
        h.appId_ = COMM_SERVICE_ID;
        h.appFun_ = ID;
    }
};

class HeartbeatResponseMsg : public Message
{
public:
    enum {
        ID = HEARTBEAT_RESPONSE_MSG_ID,
        PAYLOAD_SIZE = sizeof(coder::tgtsvc::StatusFlags)
    };

    HeartbeatResponseMsg() {
        MessageHeader &h = header();
        h.payloadSize_ = PAYLOAD_SIZE;
        h.appId_ = COMM_SERVICE_ID;
        h.appFun_ = ID;
    }

    coder::tgtsvc::StatusFlags statusFlags_;
};

class InTestStartMsg : public Message
{
public:
    enum {
        ID = IN_TEST_START_MSG_ID,
        PAYLOAD_SIZE = sizeof(uint16_t) + sizeof(uint32_t)
    };

    InTestStartMsg() {
        MessageHeader &h = header();
        h.payloadSize_ = PAYLOAD_SIZE;
        h.appId_ = COMM_SERVICE_ID;
        h.appFun_ = ID;
    }
    uint32_t testMsgCount_;
    uint16_t testMsgSize_;
};

class TestDataMsg : public Message
{
public:
    enum {
        ID = TEST_DATA_MSG_ID,
    };

    explicit TestDataMsg(uint16_t size) {
        MessageHeader &h = header();
        h.payloadSize_ = size;
        h.appId_ = COMM_SERVICE_ID;
        h.appFun_ = ID;
    }
private:
    TestDataMsg();
};

class OutTestStartMsg : public Message
{
public:
    enum {
        ID = OUT_TEST_START_MSG_ID,
        PAYLOAD_SIZE = sizeof(uint16_t) + sizeof(uint32_t)
    };

    OutTestStartMsg() {
        MessageHeader &h = header();
        h.payloadSize_ = PAYLOAD_SIZE;
        h.appId_ = COMM_SERVICE_ID;
        h.appFun_ = ID;
    }
    uint32_t testMsgCount_;
    uint16_t testMsgSize_;
};

class OutTestResultMsg : public Message
{
public:
    enum {
        ID = OUT_TEST_RESULT_MSG_ID,
        PAYLOAD_SIZE = 1
    };

    enum {
        RESULT_OK = 0
    };

    OutTestResultMsg() {
        MessageHeader &h = header();
        h.payloadSize_ = PAYLOAD_SIZE;
        h.appId_ = COMM_SERVICE_ID;
        h.appFun_ = ID;
        result_ = RESULT_OK;
    }

    uint8_t result_;
};

class TestConcludedMsg : public Message
{
public:
    enum {
        ID = TEST_CONCLUDED_MSG_ID,
        PAYLOAD_SIZE = 1
    };

    TestConcludedMsg() {
        MessageHeader &h = header();
        h.payloadSize_ = PAYLOAD_SIZE;
        h.appId_ = COMM_SERVICE_ID;
        h.appFun_ = ID;
    }

    uint8_t pad_;
};

}}
