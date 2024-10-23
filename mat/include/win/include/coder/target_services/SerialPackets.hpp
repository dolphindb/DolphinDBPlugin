/* Copyright 2015 The MathWorks, Inc. */

#ifndef coder_tgtsvc_SerialPackets_hpp
#define coder_tgtsvc_SerialPackets_hpp

#include <stdint.h>
#include <stdlib.h>

#define SYNC_STRING_LITERAL "~~~~ synchronizing ~~~~ synchronizing ~~~~"

namespace coder { namespace tgtsvc {

enum PacketId {
    CONNECT_ID = 0,
    ACK_ID = 1,
    DATA_ID = 2
};

enum {
   
    MAX_SERIAL_PACKET_SIZE = 64,

    MAX_RX_WINDOW_SIZE = 128
};

struct Ack
{
    enum {
        ID = ACK_ID
    };

    Ack() : id_(ID), sequence_(0) {}

    uint8_t id_;
    uint8_t sequence_; 
};

struct DataHeader
{
    enum {
        ID = DATA_ID,
    };

    DataHeader() : id_(ID), sequence_(0), dataSize_(0), crc_(0) {}

    uint8_t *data() { return reinterpret_cast<uint8_t*>(this + 1); }
    const uint8_t *data() const { return const_cast<DataHeader*>(this)->data(); }

    uint8_t id_;
    uint8_t sequence_; 
    uint8_t dataSize_; 
    uint8_t crc_;      
};

struct Connect
{
    enum {
        ID = CONNECT_ID
    };

    Connect() : id_(ID), windowSize_(0) {}
    Connect(uint8_t id, uint8_t windowSize) :
        id_(id), windowSize_(windowSize) {}

    uint8_t id_;
    uint8_t windowSize_; 
};

}}

#endif
