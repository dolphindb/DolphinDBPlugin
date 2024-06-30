/* Copyright 2013 The MathWorks, Inc. */

#ifndef coder_tgtsvc_MemoryServiceBase_hpp
#define coder_tgtsvc_MemoryServiceBase_hpp

#include <stdint.h>
#include <stdlib.h>
#include "coder_target_services_spec.h"
#include "SList.hpp"
#include "StatusFlags.hpp"

namespace coder { namespace tgtsvc {
   
namespace detail {

struct Chunk : public SListBaseHook<> {
    enum {
        POOL_INDEX_OFFSET = 0,
        ALLOCATED_OFFSET = 1,
        IS_ALLOCATED = 1
    };

    uint8_t poolIndex() const { return header()[POOL_INDEX_OFFSET]; }
    void poolIndex(uint8_t v) { header()[POOL_INDEX_OFFSET] = v; }

    bool allocated() const { return header()[ALLOCATED_OFFSET] == 1; }
    void allocated(bool v) { header()[ALLOCATED_OFFSET] = (uint8_t)v; }

    const uint8_t *header() const {
        const uint8_t *p = reinterpret_cast<const uint8_t*>(this-1);
        return p;
    }

    uint8_t *header() {
        return const_cast<uint8_t*>(static_cast<const Chunk*>(this)->header());
    }

    static Chunk *fromHeader(void *p) {
        uint8_t *b = static_cast<uint8_t*>(p);
        b += sizeof(void*);
        return reinterpret_cast<Chunk*>(b);
    }
};

}

template <class Derived>
class CODER_TARGET_SERVICES_EXPORT_CLASS MemoryServiceBase
{
public:
   
    explicit MemoryServiceBase(const uint16_t *poolSizes, uint8_t poolCnt) :
    poolSizes_(poolSizes), poolCount_(poolCnt)
    {
       
        for (uint8_t i=0; i<poolCount(); ++i) {
            assert(poolSize(i) % sizeof(void*) == 0);
            if (i>0) assert(poolSize(i) > poolSize(i-1));
            else assert(poolSize(i) >= sizeof(void*));
        }
    }

    ~MemoryServiceBase() {
    }

    uint16_t poolSize(uint8_t poolIdx) const {
        assert(poolIdx < poolCount_);
        return poolSizes_[poolIdx];
    }
    uint8_t poolCount() const { return poolCount_; }

    void *alloc(size_t request) throw() {

        uint8_t poolIdx = whichPool(request);
        detail::Chunk  *c = NULL;
        if (poolIdx < poolCount()) {

            c = static_cast<Derived*>(this)->popChunk(poolIdx);
            if (c == NULL) {
               
                c = static_cast<Derived*>(this)->allocChunk(poolIdx);
            }
        }

        if (c != NULL) {
            c->poolIndex(poolIdx);
            c->allocated(true);
        } else {
            StatusFlags::instance().set(StatusFlags::MEMORY_ALLOCATION_FAILED);
        }
        return c;
    }

    void free(void *p) {
        detail::Chunk *c = reinterpret_cast<detail::Chunk*>(p);
        assert(c != NULL && c->allocated() && c->poolIndex() < poolCount());
        c->allocated(false);
        static_cast<Derived*>(this)->pushChunk(c);
    }

    uint16_t capacity(const void *p) const {
        const detail::Chunk *c = reinterpret_cast<const detail::Chunk*>(p);
        assert(c != NULL && c->allocated() && c->poolIndex() < poolCount());
        uint8_t poolIdx = c->poolIndex();
        return poolSize(poolIdx);
    }

    uint16_t maxCapacity() const { return poolSize(poolCount()-1); }

private:
    const uint16_t *poolSizes_;
    uint8_t poolCount_;

    uint8_t whichPool(size_t requestSize) {
        uint8_t r = 0;
        while (r < poolCount() && poolSize(r) < requestSize) ++r;
        return r;
    }

	MemoryServiceBase(const MemoryServiceBase &cpy);

	MemoryServiceBase &operator=(const MemoryServiceBase &cpy);
};

}}

#endif
