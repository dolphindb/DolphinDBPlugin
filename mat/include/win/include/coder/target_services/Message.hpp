/* Copyright 2013 The MathWorks, Inc. */

#ifndef coder_tgtsvc_Message_hpp
#define coder_tgtsvc_Message_hpp

#include <stdint.h>
#include <stddef.h>
#include <new>
#include <assert.h>
#include "coder_target_services_spec.h"
#include"MessageStorage.hpp"

namespace coder { namespace tgtsvc {

class CODER_TARGET_SERVICES_EXPORT_CLASS Message : public MessageStorage
{
public:

    enum Priority {
        NORMAL_PRIORITY = 0,
		HIGH_PRIORITY = 1
	};

    enum {
       
        ABSOLUTE_MINIMUM_PAYLOAD = 48,
       
        ABSOLUTE_MAXIMUM_PAYLOAD = 2032
    };

    Message() {}
	~Message() {}

    void *operator new(size_t size);
    void operator delete(void *ptr) throw();

    void *operator new(size_t size, const std::nothrow_t &nothrow_value) throw();
    void operator delete (void* ptr, const std::nothrow_t& nothrow_constant) throw();

    void *operator new(size_t size, void *place) throw() { return ::operator new(size, place); }
    void operator delete(void *ptr, void *place) throw() { return ::operator delete(ptr, place); }

    static Message *alloc(uint16_t payloadSize);

    uint16_t payloadCapacity() const;

    static uint16_t maxPayloadCapacity();

    const MessageHeader &header() const  {
       
        assert(sizeof(MessageHeader) == 4);
        const MessageHeader *p = reinterpret_cast<const MessageHeader*>(payload() - sizeof(MessageHeader));
        assert((reinterpret_cast<size_t>(p) & 3) == 0);
        return *p;
    }
    MessageHeader &header() { return const_cast<MessageHeader&>(static_cast<const Message*>(this)->header()); }

    void header(const MessageHeader &h) {
        MessageHeader &mine = header();
        mine = h;
    }

    uint16_t payloadSize() const { return header().payloadSize(); }
    void payloadSize(uint16_t v) { header().payloadSize(v); }

    uint8_t appId() const { return header().appId(); }
    void appId(uint8_t v) { header().appId(v); }

    uint8_t appFun() const { return header().appFun(); }
    void appFun(uint8_t v) { header().appFun(v); }

	const uint8_t *payload() const {
        const uint8_t *ret = reinterpret_cast<const uint8_t*>(this+1);
        assert((size_t)ret % sizeof(void*) == 0);
        return ret;
    }
    uint8_t *payload() { return const_cast<uint8_t*>(static_cast<const Message*>(this)->payload()); }

    uint16_t transmitSize() const { return header().payloadSize() + (uint16_t)sizeof(MessageHeader); }

    const uint8_t *transmitStart() const { return payload() - sizeof(MessageHeader); }
    uint8_t *transmitStart() { return const_cast<uint8_t*>(static_cast<const Message*>(this)->transmitStart()); }

	static uint16_t memoryNeeded(uint16_t payloadSize) { return sizeof(Message) + payloadSize; }

private:
   
    void* operator new[] (std::size_t size);
    void operator delete[] (void* ptr) throw();
    void* operator new[] (std::size_t size, const std::nothrow_t& nothrow_value) throw();
    void operator delete[] (void* ptr, const std::nothrow_t& nothrow_constant) throw();
    void* operator new[] (std::size_t size, void* ptr) throw();
    void operator delete[] (void* ptr, void* voidptr2) throw();
};

}}

#endif
