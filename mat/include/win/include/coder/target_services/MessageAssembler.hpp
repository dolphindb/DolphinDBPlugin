/* Copyright 2013 The MathWorks, Inc. */

#ifndef coder_tgtsvc_MessageAssembler_hpp
#define coder_tgtsvc_MessageAssembler_hpp

#include <memory>

namespace coder { namespace tgtsvc {

struct MessageAssembler
{
    enum Return {
        SUCCESS,
        INCOMPLETE,
        NO_RESOURCES
    };

    MessageAssembler() : pos_(0) {}

    template <typename Iterator>
    Return assemble(Iterator &it, Iterator end) {
        while (it != end) {

            while (it != end && pos_ < sizeof(MessageHeader)) {
                headerAddr()[pos_++] = *it++;
            }
            if (pos_ < sizeof(MessageHeader)) break;

            if (!msg_) {
                msg_.reset(Message::alloc(hdr_.payloadSize()));
                if (!msg_) return NO_RESOURCES;
                msg_->header(hdr_);
            }

            while (it != end && pos_ < msg_->transmitSize()) {
                msg_->transmitStart()[pos_++] = *it++;
            }

            if (pos_ == msg_->transmitSize()) {
                pos_ = 0;
                return SUCCESS;
            }
        }
        return INCOMPLETE;
    }

    std::unique_ptr<Message> message() { return std::move(msg_); }

    void reset() {
        msg_.reset();
        pos_ = 0;
    }

private:
    std::unique_ptr<Message> msg_;
    size_t pos_;                  
    MessageHeader hdr_;           

    uint8_t *headerAddr() { return reinterpret_cast<uint8_t*>(&hdr_); }
};

}}

#endif
