#ifndef FLAGHASHMAP_H_
#define FLAGHASHMAP_H_
#include <exception>
#include <memory>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <cstdio>
#include <cassert>
#include <memory.h>
#include <functional>

#include "HashmapUtil.h"

#ifndef CACHE_LINE_SIZE
#define CACHE_LINE_SIZE  64 // 64 byte cache line on x86 and x86-64
#endif

#define DEFAULT_HM_CAPACITY 2

struct prime_hash_policy {
    size_t index(uint64_t hash) const {
        return hash % cap_;
    }
    size_t capacity() const {
        return cap_;
    }
    prime_hash_policy(size_t cap, bool predicted = false){
        if (predicted == false) {
            cap_ = nextPrime(cap * 2);
        } else {
            cap_ = nextPrime(cap);
        }
    }
private:
    size_t nextPrime(size_t n) {
        size_t x = n;
        while (true) {
            x++;
            size_t i = 2;
            bool flag = true;
            while (i * i <= x && flag) {
                if (x % i == 0) {
                    flag = false;
                }
                ++i;
            }
            if (flag)
                return x;
        }
    }
    size_t cap_;
};

struct power2_hash_policy {
    size_t cap_; // # of slots
    static size_t next_power2(size_t n) {
        if (n == 0)
            return 1;
        size_t x = 1;
        while (x <= n) {
            x *= 2;
        }
        return x;
    }
    power2_hash_policy(size_t requiredCap, bool predicted = false): cap_(1){
        cap_ = next_power2(requiredCap);
        assert(cap_ == 0 || (cap_ & (cap_ - 1)) == 0);
    }
    size_t index(uint64_t hash) const {
        return hash & (cap_ - 1);
    }
    size_t capacity() const {
        return cap_;
    }
};

#define INSERT_GOOD        0
#define INSERT_NO_VACANCY  1
#define INSERT_KEY_EXISTS  2

template<typename Key,
         typename T,
         typename HashPolicy,
         typename Hasher,
         typename KeyEqual>
class FlatHashmapImpl {
public:
	typedef Key key_type;
	typedef T mapped_type;
    typedef Hasher key_hasher;
    typedef KeyEqual key_equal;
	typedef size_t size_type;
    typedef HashPolicy hash_policy;

    FlatHashmapImpl(const FlatHashmapImpl&)=delete;
    FlatHashmapImpl& operator=(const FlatHashmapImpl&)=delete;
    
    friend struct iterator;
    FlatHashmapImpl(size_t cap = DEFAULT_HM_CAPACITY, float probeLimitScalingFactor = 1.0f, bool predictedCap = false)
        : hash_policy_(cap, predictedCap) {
        initialize(hash_policy_.capacity(), probeLimitScalingFactor);
        endIterator = iterator(this, this->hash_policy_.capacity() + this->probe_limit_);
    }

    ~FlatHashmapImpl() {
        clear();
        myFree(headers_ptr_unaligned_);
        myFree(keys_ptr_unaligned_);
        myFree(values_ptr_unaligned_);
    }
    inline bool keyPresent(size_t idx) const {
        return this->headers[idx >> 5] & (1 << (idx & 31));
    }

    inline void setKeyPresent(size_t idx) {
        assert(keyPresent(idx) == false);
        this->headers[idx >> 5] |= (1 << (idx & 31));
    }

    inline void setKeyUnpresent(size_t idx) {
        this->headers[idx >> 5] &= ~(1 << (idx & 31));
    }

    int insertNoErase(const key_type & key, const mapped_type & value) {
        uint64_t hash = this->key_hasher_(key);
        size_t idx = this->hash_policy_.index(hash);
        size_t end = idx + this->probe_limit_;
        int ret = INSERT_NO_VACANCY;
        for(size_t i = idx; i < end; ++i) {
            if (keyPresent(i)) {
                if (this->key_equal_(this->keys[i], key)) {
                    ret = INSERT_KEY_EXISTS;
                    break;
                }
                //key unmatched
                continue;
            }
            setKeyPresent(i);
            new((char*)&this->keys[i]) key_type(key);
            new((char*)&this->values[i]) mapped_type(value);
            ++this->count_;
            ret = INSERT_GOOD;
            break;
        }
        return ret;
    }

    int insertNoErase(const key_type & key, const mapped_type & value, mapped_type ** recvPtr) {
        uint64_t hash = this->key_hasher_(key);
        size_t idx = this->hash_policy_.index(hash);
        size_t end = idx + this->probe_limit_;
        int ret = INSERT_NO_VACANCY;
        for(size_t i = idx; i < end; ++i) {
            if (keyPresent(i)) {
                if (this->key_equal_(this->keys[i], key)) {
                    ret = INSERT_KEY_EXISTS;
                    if (recvPtr != nullptr)
                        *recvPtr = &this->values[i];
                    break;
                }
                //key unmatched
                continue;
            }
            setKeyPresent(i);
            new((char*)&this->keys[i]) key_type(key);
            new((char*)&this->values[i]) mapped_type(value);
            ++this->count_;
            ret = INSERT_GOOD;
            if (recvPtr != nullptr)
                *recvPtr = &this->values[i];
            break;
        }
        return ret;
    }

    int insert(const key_type & key, const mapped_type & value) {
        uint64_t hash = this->key_hasher_(key);
        size_t idx = this->hash_policy_.index(hash);
        size_t end = idx + this->probe_limit_;
        int ret = INSERT_NO_VACANCY;
        int insertSlot = -1;
        // It's required to examine all probe_limit_ slots to finding the right insert position.
        // For example:
        // insert 1 => 1 _ _ _
        // insert 2 => 1 2 _ _
        // erase 1  => _ 2 _ _
        // insert 2 => 2 2 _ _  // wrong!
        for(size_t i = idx; i < end; ++i) {
            if (keyPresent(i)) {
                if (this->key_equal_(this->keys[i], key)) {
                    ret = INSERT_KEY_EXISTS;
                    break;
                }
            } else if (insertSlot == -1) {
                // insert at the first free slot
                insertSlot = i;
            }
        }
        if (insertSlot != -1 && ret != INSERT_KEY_EXISTS) {
            setKeyPresent(insertSlot);
            new((char*)&this->keys[insertSlot]) key_type(key);
            new((char*)&this->values[insertSlot]) mapped_type(value);
            ++this->count_;
            ret = INSERT_GOOD;
        }
        return ret;
    }

    int insert(const key_type & key, const mapped_type & value, mapped_type ** recvPtr) {
        uint64_t hash = this->key_hasher_(key);
        size_t idx = this->hash_policy_.index(hash);
        size_t end = idx + this->probe_limit_;
        int ret = INSERT_NO_VACANCY;
        int insertSlot = -1;
        for(size_t i = idx; i < end; ++i) {
            if (keyPresent(i)) {
                if (this->key_equal_(this->keys[i], key)) {
                    ret = INSERT_KEY_EXISTS;
                    if (recvPtr != nullptr)
                        *recvPtr = &this->values[i];
                    break;
                }
            } else if (insertSlot == -1) {
                insertSlot = i;
            }
        }
        if (insertSlot != -1 && ret != INSERT_KEY_EXISTS) {
            setKeyPresent(insertSlot);
            new((char*)&this->keys[insertSlot]) key_type(key);
            new((char*)&this->values[insertSlot]) mapped_type(value);
            ++this->count_;
            ret = INSERT_GOOD;
            if (recvPtr != nullptr)
                *recvPtr = &this->values[insertSlot];
        }
        return ret;
    }

    bool find(const key_type & key, mapped_type & recv) {
        uint64_t hash = this->key_hasher_(key);
        size_t idx = this->hash_policy_.index(hash);
        size_t end = idx + this->probe_limit_;
        bool found = false;
        for(size_t i = idx; i < end; ++i) {
            if (keyPresent(i) &&
                this->key_equal_(this->keys[i], key)) {
                recv = this->values[i];
                found = true;
                break;
            }
        }
        return found;
    }

    bool findPointer(const key_type & key, mapped_type ** recvPtr) {
        uint64_t hash = this->key_hasher_(key);
        size_t idx = this->hash_policy_.index(hash);
        size_t end = idx + this->probe_limit_;
        bool found = false;
        for(size_t i = idx; i < end; ++i) {
            if (keyPresent(i) &&
                this->key_equal_(this->keys[i], key)) {
                *recvPtr = &this->values[i];
                found = true;
                break;
            }
        }
        return found;
    }

    bool findNoErase(const key_type & key, mapped_type & recv) {
        uint64_t hash = this->key_hasher_(key);
        size_t idx = this->hash_policy_.index(hash);
        size_t end = idx + this->probe_limit_;
        bool found = false;
        for(size_t i = idx; i < end && keyPresent(i); ++i) {
            if (this->keys[i] == key) {
                recv = this->values[i];
                found = true;
                break;
            }
        }
        return found;
    }

    bool findPointerNoErase(const key_type & key, mapped_type ** recvPtr) {
        uint64_t hash = this->key_hasher_(key);
        size_t idx = this->hash_policy_.index(hash);
        size_t end = idx + this->probe_limit_;
        bool found = false;
        for(size_t i = idx; i < end && keyPresent(i); ++i) {
            if (this->key_equal_(this->keys[i], key)) {
                *recvPtr = &this->values[i];
                found = true;
                break;
            }
        }
        return found;
    }

    bool erase(const key_type & key) {
        uint64_t hash = this->key_hasher_(key);
        size_t idx = this->hash_policy_.index(hash);
        size_t end = idx + this->probe_limit_;
        bool erased = false;
        for(size_t i = idx; i < end; ++i) {
            if (keyPresent(i) &&
                this->key_equal_(this->keys[i], key)) {
                this->keys[i].~key_type();
                this->values[i].~mapped_type();
                setKeyUnpresent(i);
                --this->count_;
                erased = true;
                break;
            }
        }
        return erased;
    }

    void clear() {
        size_t realCapacity = this->hash_policy_.capacity() + this->probe_limit_;
        for (size_t i = 0; i < realCapacity; ++i) {
            if (keyPresent(i) == false)
                continue;
            this->keys[i].~key_type();
            this->values[i].~mapped_type();
            setKeyUnpresent(i);
        }
        count_ = 0;
    }

    static FlatHashmapImpl* growFrom(const FlatHashmapImpl & from, bool noErase, float probeLimitScalingFactor, const uint64_t maxItemsHint, const uint64_t inserts) {
        uint64_t oldCap = from.hash_policy_.capacity();
        uint64_t newCap = oldCap + 1;
        bool predicted = false;
        if (maxItemsHint && inserts >= 50000) { // take 50000 samples
            assert(from.size() <= inserts);
            double uniqueKeysInsertsRatio = from.size() / (inserts + 0.0); 
            double expectedCapacity = (uniqueKeysInsertsRatio * maxItemsHint) / (from.size() / (oldCap + 0.0)) * 1.3; // expected # unique keys / load_factor
            newCap = std::max(expectedCapacity, newCap+0.0);
            //printf("keys %lu, inserts %lu, load_factor %0.2f, uniqueKeysInsertsRatio %0.2lf, predicted Capacity %lu\n", from.size(), inserts,from.size() / (oldCap + 0.0), uniqueKeysInsertsRatio, newCap);
            predicted = true;
        }
        std::unique_ptr<FlatHashmapImpl> to;
        //int failures = 0;
        while (true) {
            to.reset(new FlatHashmapImpl(newCap, probeLimitScalingFactor, predicted));
            if (noErase) {
                if (rehashNoErase(from, *to))
                    break;
            } else {
                if (rehash(from, *to))
                    break;
            }
            assert (to->hash_policy_.capacity() + 1 > newCap);
            //printf("%d, failed to grow at newCap %d\n",failures++, newCap);
            newCap = to->hash_policy_.capacity() + 1;
        }
        //printf("grew at size %lu from capcity %lu => %lu\n", from.size(), oldCap, to->hash_policy_.capacity());
        return to.release();
    }

    inline size_t size() const {
        return count_;
    }

    inline size_t capacity() const {
        return this->hash_policy_.capacity();
    }
    
    struct iterator {
        const key_type & key() {
            return impl->keys[idx];
        }
        const mapped_type & value() {
            return impl->values[idx];
        }
        iterator() : idx(-1), end(-1), impl(0){}
        iterator(FlatHashmapImpl<Key, T, HashPolicy, Hasher, KeyEqual>* impl_, int startIdx = -1):idx(startIdx), impl(impl_) {
            end = impl->hash_policy_.capacity() + impl->probe_limit_;
            this->next();
        }
        iterator & operator++() {
            this->next();
            return *this;
        }
        iterator operator++(int) {
            iterator old = *this;
            this->next();
            return old;
        }
        bool operator!=(const iterator & rhs) {
            return idx != rhs.idx;
        }
        bool operator==(const iterator & rhs) {
            return idx == rhs.idx;
        }
    private:
        void next() {
            while (idx < end && impl->keyPresent(++idx) == false);
        }
        int idx;
        int end;
        FlatHashmapImpl<Key, T, HashPolicy, Hasher, KeyEqual>* impl;
    };

    inline iterator begin() {
        return iterator(this);
    }
    inline iterator end() {
        return endIterator;
    }

private:
    iterator endIterator;
    static bool rehash(const FlatHashmapImpl & from, FlatHashmapImpl & to) {
        size_t fromCapacity = from.hash_policy_.capacity() + from.probe_limit_;
        assert(fromCapacity < to.hash_policy_.capacity() + to.probe_limit_);
        bool good = true;
        for (size_t i = 0; i < fromCapacity; ++i) {
            if (from.keyPresent(i) == false)
                continue;
            int ret = to.insert(from.keys[i], from.values[i]);
            assert(ret != INSERT_KEY_EXISTS);
            good = ret == INSERT_GOOD;
            if (good == false)
                break;
        }
        return good;
    }
    static bool rehashNoErase(const FlatHashmapImpl & from, FlatHashmapImpl & to) {
        size_t fromCapacity = from.hash_policy_.capacity() + from.probe_limit_;
        assert(fromCapacity < to.hash_policy_.capacity() + to.probe_limit_);
        bool good = true;
        for (size_t i = 0; i < fromCapacity; ++i) {
            if (from.keyPresent(i) == false)
                continue;
            int ret = to.insertNoErase(from.keys[i], from.values[i]);
            assert(ret != INSERT_KEY_EXISTS);
            good = ret == INSERT_GOOD;
            if (good == false)
                break;
        }
        return good;
    }
    void initialize(size_t cap, float probeLimitScalingFactor) {
        assert(probeLimitScalingFactor > 0);
        // probe_limit = log2(n)
        this->probe_limit_ = std::ceil(std::log2(cap)) * probeLimitScalingFactor;
        cap += probe_limit_; //allocate probe_limit_ more slots to avoid bound-checking
        size_t headers_sz = ((int)std::ceil(cap / 8.0) + sizeof(uint32_t) - 1) & ~(sizeof(uint32_t) - 1); // make headers_sz multiple of sizeof(uint32_t)
        assert(headers_sz % sizeof(uint32_t) == 0);
        size_t keys_sz = cap * sizeof(key_type);
        size_t vals_sz = cap * sizeof(mapped_type);
        this->count_ = 0;
        this->headers_ptr_unaligned_ = myAlloc(headers_sz + CACHE_LINE_SIZE - 1);
        this->keys_ptr_unaligned_ = myAlloc(keys_sz + CACHE_LINE_SIZE - 1);
        this->values_ptr_unaligned_ = myAlloc(vals_sz + CACHE_LINE_SIZE - 1);
        void* headers_ptr_aligned = (void *)(((size_t)this->headers_ptr_unaligned_ + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1));
        void* keys_ptr_aligned = (void *)(((size_t)this->keys_ptr_unaligned_ + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1));
        void* values_ptr_aligned = (void *)(((size_t)this->values_ptr_unaligned_ + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1));
        this->headers = new(headers_ptr_aligned) uint32_t[cap / 8];
        this->keys = (key_type *)keys_ptr_aligned;
        this->values = (mapped_type *)values_ptr_aligned;
        assert((size_t)headers_ptr_aligned % CACHE_LINE_SIZE == 0);
        assert((size_t)keys_ptr_aligned % CACHE_LINE_SIZE == 0);
        assert((size_t)values_ptr_aligned % CACHE_LINE_SIZE == 0);
        memset(this->headers_ptr_unaligned_, 0, headers_sz + CACHE_LINE_SIZE - 1);
    }

    static key_hasher key_hasher_;
    static key_equal key_equal_;
    hash_policy hash_policy_;
    void* headers_ptr_unaligned_;
    void* keys_ptr_unaligned_;
    void* values_ptr_unaligned_;
    uint32_t* headers;
    key_type* keys;
    mapped_type* values;
    size_t probe_limit_;
    size_t count_; // # elements in the table
};

template<typename Key,
         typename T,
         typename HashPolicy,
         typename Hasher,
         typename KeyEqual>
Hasher FlatHashmapImpl<Key, T, HashPolicy, Hasher, KeyEqual>::key_hasher_;
template<typename Key,
         typename T,
         typename HashPolicy,
         typename Hasher,
         typename KeyEqual>
KeyEqual FlatHashmapImpl<Key, T, HashPolicy, Hasher, KeyEqual>::key_equal_;


template<typename Key,
         typename T,
         typename HashPolicy = power2_hash_policy,
         typename Hasher = murmur_hasher<Key>,
         typename KeyEqual = std::equal_to<Key>>
class FlatHashmap {
public:
    typedef Key key_type;
	typedef T mapped_type;
    typedef Hasher key_hasher;
    typedef KeyEqual key_equal;
	typedef size_t size_type;
    typedef HashPolicy hash_policy;
    FlatHashmap(size_t initialCap = DEFAULT_HM_CAPACITY, float probeLimitScalingFactor = 1.0f)
        : impl(new FlatHashmapImpl<Key, T, HashPolicy, Hasher, KeyEqual>(initialCap, probeLimitScalingFactor)) {
            probeLimitScalingFactor_ = probeLimitScalingFactor;
        }
    
    inline bool find(const key_type & key, mapped_type & recv) {
        return impl->find(key, recv);
    }

    bool insert(const key_type & key, const mapped_type & value) {
        int ret = impl->insert(key, value);
        if (ret == INSERT_GOOD)
            return true;
        else if (ret == INSERT_KEY_EXISTS)
            return false;
        // ret == INSERT_NO_VACANCY
        auto to = impl->growFrom(*impl, false, probeLimitScalingFactor_, 0, 0);
        if (to == nullptr) {
            throw std::runtime_error("failed to grow hashmap");
        }
        impl.reset(to);
        // tail-recursive call
        return insert(key, value);
    }

    mapped_type& operator[] (const key_type & key) {
        mapped_type *recvPtr;
        bool found = impl->findPointer(key, &recvPtr);
        if (found == true)
            return *recvPtr;
        int ret = impl->insert(key, mapped_type(), &recvPtr);
        if (ret == INSERT_GOOD || ret == INSERT_KEY_EXISTS)
            return *recvPtr;
        // ret == INSERT_NO_VACANCY
        auto to = impl->growFrom(*impl, false, probeLimitScalingFactor_, 0, 0);
        if (to == nullptr) {
            throw std::runtime_error("failed to grow hashmap");
        }
        impl.reset(to);
        // tail-recursive call
        return operator[](key);
    }

    bool upsert(const key_type & key, const mapped_type & value) {
        mapped_type *recvPtr;
        bool found = impl->findPointer(key, &recvPtr);
        if (found == true) {
            *recvPtr = value;
            return false;
        }
        int ret = impl->insert(key, value);
        if (ret == INSERT_GOOD || ret == INSERT_KEY_EXISTS)
            return true;
        // ret == INSERT_NO_VACANCY
        auto to = impl->growFrom(*impl, false, probeLimitScalingFactor_, 0, 0);
        if (to == nullptr) {
            throw std::runtime_error("failed to grow hashmap");
        }
        impl.reset(to);
        // tail-recursive call
        return upsert(key, value);
    }

    inline bool erase(const key_type & key) {
        return impl->erase(key);
    }

    inline void clear() {
        impl->clear();
    }

    inline size_t size() {
        return impl->size();
    }

    inline size_t capacity() {
        return impl->capacity();
    }
    struct const_iterator {
        const key_type & key() {
            return it.key();
        }
        const mapped_type & value() {
            return it.value();
        }
        
        bool operator!=(const const_iterator & rhs) {
            return it != rhs.it;
        }
        bool operator==(const const_iterator & rhs) {
            return it == rhs.it;
        }
        const_iterator& operator++() {
            ++it;
            return *this;
        }
        const_iterator operator++(int) {
            const_iterator old = *this;
            ++it;
            return old;
        }
        const_iterator(FlatHashmapImpl<Key, T, HashPolicy, Hasher, KeyEqual> * impl): it(impl->begin()) {}
        const_iterator(const typename FlatHashmapImpl<Key, T, HashPolicy, Hasher, KeyEqual>::iterator & implIterator): it(implIterator) {}
    private:
        typename FlatHashmapImpl<Key, T, HashPolicy, Hasher, KeyEqual>::iterator it;
    };

    inline const_iterator begin() {
        return const_iterator(impl.get());
    }
    inline const_iterator end() {
        return const_iterator(impl->end());
    }
private:
    std::unique_ptr<FlatHashmapImpl<Key, T, HashPolicy, Hasher, KeyEqual>> impl;
    float probeLimitScalingFactor_;
};


template<typename Key,
         typename T,
         typename HashPolicy = power2_hash_policy,
         typename Hasher = murmur_hasher<Key>,
         typename KeyEqual = std::equal_to<Key>>
class IrremovableFlatHashmap {
public:
    typedef Key key_type;
	typedef T mapped_type;
    typedef Hasher key_hasher;
    typedef KeyEqual key_equal;
	typedef size_t size_type;
    typedef HashPolicy hash_policy;

    IrremovableFlatHashmap()
        : impl(new FlatHashmapImpl<Key, T, HashPolicy, Hasher,  KeyEqual>(DEFAULT_HM_CAPACITY, 1.0f)),
         inserts_(0), maxItemsHint_(0)
        {
            probeLimitScalingFactor_ = 1.0f;
        }
    IrremovableFlatHashmap(uint64_t maxItemsHint, float probeLimitScalingFactor = 1.0f)
        : impl(new FlatHashmapImpl<Key, T, HashPolicy, Hasher,  KeyEqual>(DEFAULT_HM_CAPACITY, probeLimitScalingFactor)),
           inserts_(0), maxItemsHint_(maxItemsHint), probeLimitScalingFactor_(probeLimitScalingFactor)
        {

        }

    inline bool find(const key_type & key, mapped_type & recv) {
        return impl->findNoErase(key, recv);
    }

    bool insert(const key_type & key, const mapped_type & value) {
        int ret = impl->insertNoErase(key, value);
        ++inserts_;
        if (ret == INSERT_GOOD)
            return true;
        else if (ret == INSERT_KEY_EXISTS)
            return false;
        // ret == INSERT_NO_VACANCY
        auto to = impl->growFrom(*impl, true, probeLimitScalingFactor_, maxItemsHint_, inserts_);
        if (to == nullptr) {
            throw std::runtime_error("failed to grow hashmap");
        }
        impl.reset(to);
        --inserts_;
        // tail-recursive call
        return insert(key, value);
    }

    mapped_type& operator[] (const key_type & key) {
        ++inserts_;
        mapped_type *recvPtr;
        bool found = impl->findPointerNoErase(key, &recvPtr);
        if (found == true)
            return *recvPtr;
        int ret = impl->insertNoErase(key, mapped_type(), &recvPtr);
        if (ret == INSERT_GOOD) {
            return *recvPtr;
        } else if (ret == INSERT_KEY_EXISTS)
            return *recvPtr;
        // ret == INSERT_NO_VACANCY
        auto to = impl->growFrom(*impl, true, probeLimitScalingFactor_, maxItemsHint_, inserts_);
        if (to == nullptr) {
            throw std::runtime_error("failed to grow hashmap");
        }
        impl.reset(to);
        --inserts_;
        // tail-recursive call
        return operator[](key);
    }

    bool upsert(const key_type & key, const mapped_type & value) {
        mapped_type *recvPtr;
        bool found = impl->findPointerNoErase(key, &recvPtr);
        if (found == true) {
            *recvPtr = value;
            return false;
        }
        int ret = impl->insertNoErase(key, value);
        if (ret == INSERT_GOOD || ret == INSERT_KEY_EXISTS)
            return true;
        // ret == INSERT_NO_VACANCY
        auto to = impl->growFrom(*impl, false, probeLimitScalingFactor_, 0, 0);
        if (to == nullptr) {
            throw std::runtime_error("failed to grow hashmap");
        }
        impl.reset(to);
        // tail-recursive call
        return upsert(key, value);
    }

    inline void clear() {
        impl->clear();
    }

    inline size_t size() {
        return impl->size();
    }

    inline size_t capacity() {
        return impl->capacity();
    }

    struct const_iterator {
        const key_type & key() {
            return it.key();
        }
        const mapped_type & value() {
            return it.value();
        }
        
        bool operator!=(const const_iterator & rhs) {
            return it != rhs.it;
        }
        bool operator==(const const_iterator & rhs) {
            return it == rhs.it;
        }
        const_iterator & operator++() {
            ++it;
            return *this;
        }
        const_iterator operator++(int) {
            const_iterator old = *this;
            ++it;
            return old;
        }
        const_iterator(FlatHashmapImpl<Key, T, HashPolicy, Hasher, KeyEqual> * impl): it(impl->begin()) {}
        const_iterator(const typename FlatHashmapImpl<Key, T, HashPolicy, Hasher, KeyEqual>::iterator & implIterator): it(implIterator) {}
    private:
        typename FlatHashmapImpl<Key, T, HashPolicy, Hasher, KeyEqual>::iterator it;
    };

    inline const_iterator begin() {
        return const_iterator(impl.get());
    }
    inline const_iterator end() {
        return const_iterator(impl->end());
    }
private:
    std::unique_ptr<FlatHashmapImpl<Key, T, HashPolicy, Hasher, KeyEqual>> impl;
    uint64_t inserts_;
    const uint64_t maxItemsHint_;
    float probeLimitScalingFactor_;
};

/* Use bitmap to speed up hashmap for dense integral keys */
template<typename Key,
         typename T>
class FlatBitmap {
public:
    static_assert(std::is_integral<Key>::value, "Key must be one of integral types");
	typedef Key key_type;
	typedef T mapped_type;
	typedef size_t size_type;

    FlatBitmap(const FlatBitmap&)=delete;
    FlatBitmap& operator=(const FlatBitmap&)=delete;

    friend struct iterator;

    FlatBitmap(const key_type & minKey_, const key_type & maxKey_): minKey(minKey_), maxKey(maxKey_) {
        if (maxKey < minKey) {
            throw std::runtime_error("maxKey must be greater than or equal to minKey");
        }
        initialize();
        endIterator = iterator(this, this->cap);
    }

    ~FlatBitmap() {
        clear();
        myFree(keyBitmapPtrUnaligned);
        myFree(valuesPtrUnaligned);
    }

    inline bool keyPresent(size_t idx) const {
        return this->keyBitmap[idx >> 5] & (1 << (idx & 31));
    }

    inline void setKeyPresent(size_t idx) {
        assert(keyPresent(idx) == false);
        this->keyBitmap[idx >> 5] |= (1 << (idx & 31));
    }

    inline void setKeyUnpresent(size_t idx) {
        this->keyBitmap[idx >> 5] &= ~(1 << (idx & 31));
    }

    inline size_t key2Idx(const key_type & key) {
        return (int64_t)key - minKey;
    }

    inline const key_type idx2Key(int64_t idx) {
        return idx + minKey;
    }

    inline bool insert(const key_type & key, const mapped_type & value) {
        int64_t idx = key2Idx(key);
        if (!keyPresent(idx)) {
            setKeyPresent(idx);
            new((char*)&this->values[idx]) mapped_type(value);
            ++this->count;
            return true;
        }
        return false;
    }

    inline bool find(const key_type & key, mapped_type & recv) {
        if (key < this->minKey || key > this->maxKey)
            return false;
        int64_t idx = key2Idx(key);
        if (keyPresent(idx)) {
            recv = this->values[idx];
            return true;
        }
        return false;
    }

    inline mapped_type& operator[] (const key_type & key) {
        mapped_type *recvPtr;
        if (findPointer(key, &recvPtr) == false) {
            insert(key, mapped_type(), &recvPtr);
        }
        // tail-recursive call
        return *recvPtr;
    }

    inline bool erase(const key_type & key) {
        if (key < this->minKey || key > this->maxKey)
            return false;
        int64_t idx = key2Idx(key);
        if (keyPresent(idx)) {
            setKeyUnpresent(idx);
            this->values[idx].~mapped_type();
            --this->count;
            return true;
        }
        return false;
    }

    inline void clear() {
        size_t capcity = this->cap;
        for (size_t i = 0; i < capcity; ++i) {
            if (keyPresent(i) == false)
                continue;
            this->values[i].~mapped_type();
            setKeyUnpresent(i);
        }
        count = 0;
    }

    inline size_t size() const {
        return this->count;
    }

    inline size_t capacity() const {
        return this->cap;
    }

    struct iterator {
        const key_type key() {
            return impl->idx2Key(idx);
        }
        const mapped_type & value() {
            return impl->values[idx];
        }
        
        iterator & operator++() {
            this->next();
            return *this;
        }
        iterator operator++(int) {
            iterator old = *this;
            this->next();
            return old;
        }
        bool operator!=(const iterator & rhs) {
            return idx != rhs.idx;
        }
        bool operator==(const iterator & rhs) {
            return idx == rhs.idx;
        }
        friend FlatBitmap<Key,T>;
    private:
        iterator() : idx(-1), end(-1), impl(0){}
        iterator(FlatBitmap<Key, T>* impl_, int startIdx = -1): idx(startIdx), impl(impl_) {
            end = impl->cap;
            this->next();
        }
        void next() {
            while (idx < end && impl->keyPresent(++idx) == false);
        }
        int idx;
        int end;
        FlatBitmap<Key, T>* impl;
    };

    inline iterator begin() {
        return iterator(this);
    }
    inline iterator end() {
        return endIterator;
    }
private:
    bool findPointer(const key_type & key, mapped_type ** recvPtr) {
        if (key < this->minKey || key > this->maxKey)
            return false;
        int64_t idx = key2Idx(key);
        if (keyPresent(idx)) {
            *recvPtr = &this->values[idx];
            return true;
        }
        return false;
    }

    int insert(const key_type & key, const mapped_type & value, mapped_type ** recvPtr) {
        int64_t idx = key2Idx(key);
        *recvPtr = &this->values[idx];
        if (!keyPresent(idx)) {
            setKeyPresent(idx);
            new((char*)&this->values[idx]) mapped_type(value);
            ++this->count;
            return true;
        }
        return false;
    }


    void initialize() {
        cap = (int64_t)this->maxKey - (int64_t)this->minKey + 1;
        size_t bitmap_sz = ((int)std::ceil(cap / 8.0) + sizeof(uint32_t) - 1) & ~(sizeof(uint32_t) - 1); // make bitmap_sz multiple of sizeof(uint32_t)
        assert(bitmap_sz % sizeof(uint32_t) == 0);
        size_t vals_sz = cap * sizeof(mapped_type);
        this->count = 0;
        this->keyBitmapPtrUnaligned = myAlloc(bitmap_sz + CACHE_LINE_SIZE - 1);
        this->valuesPtrUnaligned = myAlloc(vals_sz + CACHE_LINE_SIZE - 1);
        void* bitmap_ptr_aligned = (void *)(((size_t)this->keyBitmapPtrUnaligned + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1));
        void* values_ptr_aligned = (void *)(((size_t)this->valuesPtrUnaligned + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1));
        this->keyBitmap = new(bitmap_ptr_aligned) uint32_t[cap / 8];
        this->values = (mapped_type *)values_ptr_aligned;
        assert((size_t)bitmap_ptr_aligned % CACHE_LINE_SIZE == 0);
        assert((size_t)values_ptr_aligned % CACHE_LINE_SIZE == 0);
        memset(this->keyBitmapPtrUnaligned, 0, bitmap_sz + CACHE_LINE_SIZE - 1);
        //cache warmup
        memset(this->valuesPtrUnaligned, 0, vals_sz + CACHE_LINE_SIZE - 1);
    }

    iterator endIterator;
    void* keyBitmapPtrUnaligned;
    void* valuesPtrUnaligned;
    uint32_t* keyBitmap;
    mapped_type* values;
    size_t count; // # elements in the table
    size_t cap;
    key_type minKey;
    key_type maxKey;
};

/* Use bitmap to speed up hashset and for dense integral types*/
template<typename Key>
class FlatBitset {
public:
    static_assert(std::is_integral<Key>::value, "Key must be one of integral types");
	typedef Key key_type;
	typedef size_t size_type;

    FlatBitset(const FlatBitset&)=delete;
    FlatBitset& operator=(const FlatBitset&)=delete;

    friend struct iterator;

    FlatBitset(const key_type & minKey_, const key_type & maxKey_): minKey(minKey_), maxKey(maxKey_) {
        if (!std::is_integral<Key>::value) {
            throw std::runtime_error("Only integral key types are supported");
        }
        initialize();
        endIterator = iterator(this, this->cap);
    }

    ~FlatBitset() {
        clear();
        myFree(keyBitmapPtrUnaligned);
    }

    inline bool keyPresent(int idx) const {
        return this->keyBitmap[idx >> 5] & (1 << (idx & 31));
    }

    inline void setKeyPresent(int idx) {
        assert(keyPresent(idx) == false);
        this->keyBitmap[idx >> 5] |= (1 << (idx & 31));
    }

    inline void setKeyUnpresent(int idx) {
        int scaledIdx = idx >> 5;
        int bitShiftInInt = idx & 31;
        this->keyBitmap[scaledIdx] &= ~(1 << bitShiftInInt);
    }

    inline int64_t key2Idx(const key_type & key) {
        return key - minKey;
    }

    inline const key_type idx2Key(int64_t idx) {
        return idx + minKey;
    }

    inline bool insert(const key_type & key) {
        int64_t idx = key2Idx(key);
        if (!keyPresent(idx)) {
            setKeyPresent(idx);
            ++this->count;
            return true;
        }
        return false;
    }

    inline bool find(const key_type & key) {
        if (key < this->minKey || key > this->maxKey)
            return false;
        int64_t idx = key2Idx(key);
        if (keyPresent(idx)) {
            return true;
        }
        return false;
    }

    inline bool erase(const key_type & key) {
        if (key < this->minKey || key > this->maxKey)
            return false;
        int64_t idx = key2Idx(key);
        if (keyPresent(idx)) {
            setKeyUnpresent(idx);
            --this->count;
            return true;
        }
        return false;
    }

    inline void clear() {
        size_t capcity = this->cap;
        for (size_t i = 0; i < capcity; ++i) {
            setKeyUnpresent(i);
        }
        count = 0;
    }

    inline size_t size() const {
        return this->count;
    }

    inline size_t capacity() const {
        return this->cap;
    }

    struct iterator {
        const key_type key() {
            return impl->idx2Key(idx);
        }
        iterator & operator++() {
            this->next();
            return *this;
        }
        iterator operator++(int) {
            iterator old = *this;
            this->next();
            return old;
        }
        bool operator!=(const iterator & rhs) {
            return idx != rhs.idx;
        }
        bool operator==(const iterator & rhs) {
            return idx == rhs.idx;
        }
        friend FlatBitset<Key>;
    private:
        iterator() : idx(-1), end(-1), impl(0){}
        iterator(FlatBitset<Key>* impl_, int startIdx = -1): idx(startIdx), impl(impl_) {
            end = impl->cap;
            this->next();
        }
        void next() {
            while (idx < end && impl->keyPresent(++idx) == false);
        }
        int idx;
        int end;
        FlatBitset<Key>* impl;
    };

    inline iterator begin() {
        return iterator(this);
    }
    inline iterator end() {
        return endIterator;
    }
private:
    void initialize() {
        cap = this->maxKey - this->minKey + 1;
        size_t bitmap_sz = ((int)std::ceil(cap / 8.0) + sizeof(uint32_t) - 1) & ~(sizeof(uint32_t) - 1); // make bitmap_sz multiple of sizeof(uint32_t)
        assert(bitmap_sz % sizeof(uint32_t) == 0);
        this->count = 0;
        this->keyBitmapPtrUnaligned = myAlloc(bitmap_sz + CACHE_LINE_SIZE - 1);
        void* bitmap_ptr_aligned = (void *)(((size_t)this->keyBitmapPtrUnaligned + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1));
        this->keyBitmap = new(bitmap_ptr_aligned) uint32_t[cap / 8];
        assert((size_t)bitmap_ptr_aligned % CACHE_LINE_SIZE == 0);
        memset(this->keyBitmapPtrUnaligned, 0, bitmap_sz + CACHE_LINE_SIZE - 1);
    }

    iterator endIterator;
    void* keyBitmapPtrUnaligned;
    uint32_t* keyBitmap;
    size_t count; // # elements in the table
    size_t cap;
    key_type minKey;
    key_type maxKey;
};


template<typename Key,
         typename HashPolicy,
         typename Hasher,
         typename KeyEqual>
class FlatHashsetImpl {
public:
	typedef Key key_type;
    typedef Hasher key_hasher;
    typedef KeyEqual key_equal;
	typedef size_t size_type;
    typedef HashPolicy hash_policy;

    FlatHashsetImpl(const FlatHashsetImpl&)=delete;
    FlatHashsetImpl& operator=(const FlatHashsetImpl&)=delete;
    
    friend struct iterator;
    FlatHashsetImpl(size_t cap = DEFAULT_HM_CAPACITY, float probeLimitScalingFactor = 1.0, bool predictedCap = false)
        : hash_policy_(cap, predictedCap){
        initialize(hash_policy_.capacity(), probeLimitScalingFactor);
        endIterator = iterator(this, this->hash_policy_.capacity() + this->probe_limit_);
    }

    ~FlatHashsetImpl() {
        clear();
        myFree(headers_ptr_unaligned_);
        myFree(keys_ptr_unaligned_);
    }
    inline bool keyPresent(size_t idx) const {
        return this->headers[idx >> 5] & (1 << (idx & 31));
    }

    inline void setKeyPresent(size_t idx) {
        assert(keyPresent(idx) == false);
        this->headers[idx >> 5] |= (1 << (idx & 31));
    }

    inline void setKeyUnpresent(size_t idx) {
        this->headers[idx >> 5] &= ~(1 << (idx & 31));
    }

    int insertNoErase(const key_type & key) {
        uint32_t hash = this->key_hasher_(key);
        size_t idx = this->hash_policy_.index(hash);
        size_t end = idx + this->probe_limit_;
        int ret = INSERT_NO_VACANCY;
        for(size_t i = idx; i < end; ++i) {
            if (keyPresent(i)) {
                if (this->key_equal_(this->keys[i], key)) {
                    ret = INSERT_KEY_EXISTS;
                    break;
                }
                //key unmatched
                continue;
            }
            setKeyPresent(i);
            new((char*)&this->keys[i]) key_type(key);
            ++this->count_;
            ret = INSERT_GOOD;
            break;
        }
        return ret;
    }

    int insert(const key_type & key) {
        uint32_t hash = this->key_hasher_(key);
        size_t idx = this->hash_policy_.index(hash);
        size_t end = idx + this->probe_limit_;
        int ret = INSERT_NO_VACANCY;
        int insertSlot = -1;
        // It's required to examine all probe_limit_ slots to finding the right insert position.
        // For example:
        // insert 1 => 1 _ _ _
        // insert 2 => 1 2 _ _
        // erase 1  => _ 2 _ _
        // insert 2 => 2 2 _ _  // wrong!
        for(size_t i = idx; i < end; ++i) {
            if (keyPresent(i)) {
                if (this->key_equal_(this->keys[i], key)) {
                    ret = INSERT_KEY_EXISTS;
                    break;
                }
            } else if (insertSlot == -1) {
                // insert at the first free slot
                insertSlot = i;
            }
        }
        if (insertSlot != -1 && ret != INSERT_KEY_EXISTS) {
            setKeyPresent(insertSlot);
            new((char*)&this->keys[insertSlot]) key_type(key);
            ++this->count_;
            ret = INSERT_GOOD;
        }
        return ret;
    }

    bool find(const key_type & key) {
        uint32_t hash = this->key_hasher_(key);
        size_t idx = this->hash_policy_.index(hash);
        size_t end = idx + this->probe_limit_;
        bool found = false;
        for(size_t i = idx; i < end; ++i) {
            if (keyPresent(i) &&
                this->key_equal_(this->keys[i], key)) {
                found = true;
                break;
            }
        }
        return found;
    }

    bool findNoErase(const key_type & key) {
        uint32_t hash = this->key_hasher_(key);
        size_t idx = this->hash_policy_.index(hash);
        size_t end = idx + this->probe_limit_;
        bool found = false;
        for(size_t i = idx; i < end && keyPresent(i); ++i) {
            if (this->keys[i] == key) {
                found = true;
                break;
            }
        }
        return found;
    }

    bool erase(const key_type & key) {
        uint32_t hash = this->key_hasher_(key);
        size_t idx = this->hash_policy_.index(hash);
        size_t end = idx + this->probe_limit_;
        bool erased = false;
        for(size_t i = idx; i < end; ++i) {
            if (keyPresent(i) &&
                this->key_equal_(this->keys[i], key)) {
                this->keys[i].~key_type();
                setKeyUnpresent(i);
                --this->count_;
                erased = true;
                break;
            }
        }
        return erased;
    }

    void clear() {
        size_t realCapacity = this->hash_policy_.capacity() + this->probe_limit_;
        for (size_t i = 0; i < realCapacity; ++i) {
            if (keyPresent(i) == false)
                continue;
            this->keys[i].~key_type();
            setKeyUnpresent(i);
        }
        count_ = 0;
    }

    static FlatHashsetImpl* growFrom(const FlatHashsetImpl & from, bool noErase, float probeLimitScalingFactor, const uint64_t maxItemsHint, const uint64_t inserts) {
        size_t oldCap = from.hash_policy_.capacity();
        size_t newCap = oldCap + 1;
        bool predicted = false;
        if (maxItemsHint && inserts >= 50000) { // take 50000 samples
            assert(from.size() <= inserts);
            double uniqueKeysInsertsRatio = from.size() / (inserts + 0.0); 
            double expectedCapacity = (uniqueKeysInsertsRatio * maxItemsHint) / (from.size() / (oldCap + 0.0)); // expected # unique keys / load_factor
            newCap = std::max(expectedCapacity, newCap+0.0);
            //printf("keys %lu, inserts %lu, load_factor %0.2f, uniqueKeysInsertsRatio %0.2lf, predicted Capacity %lu\n", from.size(), inserts,from.size() / (oldCap + 0.0), uniqueKeysInsertsRatio, newCap);
            predicted = true;
        }
        std::unique_ptr<FlatHashsetImpl> to;
        //int failures = 0;
        while (true) {
            to.reset(new FlatHashsetImpl(newCap, probeLimitScalingFactor, predicted));
            if (noErase) {
                if (rehashNoErase(from, *to))
                    break;
            } else {
                if (rehash(from, *to))
                    break;
            }
            assert (to->hash_policy_.capacity() + 1 > newCap);
            //printf("%d, failed to grow at newCap %d\n",failures++, newCap);
            newCap = to->hash_policy_.capacity() + 1;
        }
        //printf("grew at size %lu from capcity %lu => %lu\n", from.size(), oldCap, to->hash_policy_.capacity());
        return to.release();
    }

    inline size_t size() const {
        return count_;
    }

    inline size_t capacity() const {
        return this->hash_policy_.capacity();
    }
    
    struct iterator {
        const key_type & key() {
            return impl->keys[idx];
        }
        iterator() : idx(-1), end(-1), impl(0){}
        iterator(FlatHashsetImpl<Key, HashPolicy, Hasher, KeyEqual>* impl_, int startIdx = -1):idx(startIdx), impl(impl_) {
            end = impl->hash_policy_.capacity() + impl->probe_limit_;
            this->next();
        }
        iterator & operator++() {
            this->next();
            return *this;
        }
        iterator operator++(int) {
            iterator old = *this;
            this->next();
            return old;
        }
        bool operator!=(const iterator & rhs) {
            return idx != rhs.idx;
        }
        bool operator==(const iterator & rhs) {
            return idx == rhs.idx;
        }
    private:
        void next() {
            while (idx < end && impl->keyPresent(++idx) == false);
        }
        int idx;
        int end;
        FlatHashsetImpl<Key, HashPolicy, Hasher, KeyEqual>* impl;
    };

    inline iterator begin() {
        return iterator(this);
    }
    inline iterator end() {
        return endIterator;
    }

private:
    iterator endIterator;
    static bool rehash(const FlatHashsetImpl & from, FlatHashsetImpl & to) {
        size_t fromCapacity = from.hash_policy_.capacity() + from.probe_limit_;
        assert(fromCapacity < to.hash_policy_.capacity() + to.probe_limit_);
        bool good = true;
        for (size_t i = 0; i < fromCapacity; ++i) {
            if (from.keyPresent(i) == false)
                continue;
            int ret = to.insert(from.keys[i]);
            assert(ret != INSERT_KEY_EXISTS);
            good = ret == INSERT_GOOD;
            if (good == false)
                break;
        }
        return good;
    }
    static bool rehashNoErase(const FlatHashsetImpl & from, FlatHashsetImpl & to) {
        size_t fromCapacity = from.hash_policy_.capacity() + from.probe_limit_;
        assert(fromCapacity < to.hash_policy_.capacity() + to.probe_limit_);
        bool good = true;
        for (size_t i = 0; i < fromCapacity; ++i) {
            if (from.keyPresent(i) == false)
                continue;
            int ret = to.insertNoErase(from.keys[i]);
            assert(ret != INSERT_KEY_EXISTS);
            good = ret == INSERT_GOOD;
            if (good == false)
                break;
        }
        return good;
    }
    void initialize(size_t cap, float probeLimitScalingFactor) {
        assert(probeLimitScalingFactor > 0);
        // probe_limit = log2(n)
        this->probe_limit_ = std::ceil(std::log2(cap)) * probeLimitScalingFactor;
        cap += probe_limit_; //allocate probe_limit_ more slots to avoid bound-checking
        size_t headers_sz = ((int)std::ceil(cap / 8.0) + sizeof(uint32_t) - 1) & ~(sizeof(uint32_t) - 1); // make headers_sz multiple of sizeof(uint32_t)
        assert(headers_sz % sizeof(uint32_t) == 0);
        size_t keys_sz = cap * sizeof(key_type);
        this->count_ = 0;
        this->headers_ptr_unaligned_ = myAlloc(headers_sz + CACHE_LINE_SIZE - 1);
        this->keys_ptr_unaligned_ = myAlloc(keys_sz + CACHE_LINE_SIZE - 1);
        void* headers_ptr_aligned = (void *)(((size_t)this->headers_ptr_unaligned_ + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1));
        void* keys_ptr_aligned = (void *)(((size_t)this->keys_ptr_unaligned_ + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1));
        this->headers = new(headers_ptr_aligned) uint32_t[cap / 8];
        this->keys = (key_type *)keys_ptr_aligned;
        assert((size_t)headers_ptr_aligned % CACHE_LINE_SIZE == 0);
        assert((size_t)keys_ptr_aligned % CACHE_LINE_SIZE == 0);
        memset(this->headers_ptr_unaligned_, 0, headers_sz + CACHE_LINE_SIZE - 1);
    }

    static key_hasher key_hasher_;
    static key_equal key_equal_;
    hash_policy hash_policy_;
    void* headers_ptr_unaligned_;
    void* keys_ptr_unaligned_;
    uint32_t* headers;
    key_type* keys;
    size_t probe_limit_;
    size_t count_; // # elements in the table
};

template<typename Key,
         typename HashPolicy,
         typename Hasher,
         typename KeyEqual>
Hasher FlatHashsetImpl<Key, HashPolicy, Hasher, KeyEqual>::key_hasher_;
template<typename Key,
         typename HashPolicy,
         typename Hasher,
         typename KeyEqual>
KeyEqual FlatHashsetImpl<Key, HashPolicy, Hasher, KeyEqual>::key_equal_;

template<typename Key,
         typename HashPolicy = power2_hash_policy,
         typename Hasher = murmur_hasher<Key>,
         typename KeyEqual = std::equal_to<Key>>
class FlatHashset {
public:
    typedef Key key_type;
    typedef Hasher key_hasher;
    typedef KeyEqual key_equal;
	typedef size_t size_type;
    typedef HashPolicy hash_policy;
    FlatHashset(size_t initialCap = DEFAULT_HM_CAPACITY, float probeLimitScalingFactor = 1.0f)
        : impl(new FlatHashsetImpl<Key, HashPolicy, Hasher, KeyEqual>(initialCap)) {
            probeLimitScalingFactor_ = probeLimitScalingFactor;
        }
    FlatHashset(float probeLimitScalingFactor)
        : impl(new FlatHashsetImpl<Key, HashPolicy, Hasher, KeyEqual>(DEFAULT_HM_CAPACITY, probeLimitScalingFactor)) {
            probeLimitScalingFactor_ = probeLimitScalingFactor;
        }

    inline bool find(const key_type & key) {
        return impl->find(key);
    }
    
    bool insert(const key_type & key) {
        int ret = impl->insert(key);
        if (ret == INSERT_GOOD)
            return true;
        else if (ret == INSERT_KEY_EXISTS)
            return false;
        // ret == INSERT_NO_VACANCY
        auto to = impl->growFrom(*impl, false, probeLimitScalingFactor_, 0, 0);
        if (to == nullptr) {
            throw std::runtime_error("failed to grow hashmap");
        }
        impl.reset(to);
        // tail-recursive call
        return insert(key);
    }

    inline bool erase(const key_type & key) {
        return impl->erase(key);
    }

    inline void clear() {
        impl->clear();
    }

    inline size_t size() {
        return impl->size();
    }

    inline size_t capacity() {
        return impl->capacity();
    }

    struct const_iterator {
        const key_type & key() {
            return it.key();
        }
        
        bool operator!=(const const_iterator & rhs) {
            return it != rhs.it;
        }
        bool operator==(const const_iterator & rhs) {
            return it == rhs.it;
        }
        const_iterator& operator++() {
            ++it;
            return *this;
        }
        const_iterator operator++(int) {
            const_iterator old = *this;
            ++it;
            return old;
        }
        const_iterator(FlatHashsetImpl<Key, HashPolicy, Hasher, KeyEqual> * impl): it(impl->begin()) {}
        const_iterator(const typename FlatHashsetImpl<Key, HashPolicy, Hasher, KeyEqual>::iterator & implIterator): it(implIterator) {}
    private:
        typename FlatHashsetImpl<Key, HashPolicy, Hasher, KeyEqual>::iterator it;
    };

    inline const_iterator begin() {
        return const_iterator(impl.get());
    }
    inline const_iterator end() {
        return const_iterator(impl->end());
    }
private:
    std::unique_ptr<FlatHashsetImpl<Key, HashPolicy, Hasher, KeyEqual>> impl;
    float probeLimitScalingFactor_;
};


template<typename Key,
         typename HashPolicy = power2_hash_policy,
         typename Hasher = murmur_hasher<Key>,
         typename KeyEqual = std::equal_to<Key>>
class IrremovableFlatHashset {
public:
    typedef Key key_type;
    typedef Hasher key_hasher;
    typedef KeyEqual key_equal;
	typedef size_t size_type;
    typedef HashPolicy hash_policy;
    IrremovableFlatHashset()
        : impl(new FlatHashsetImpl<Key, HashPolicy, Hasher,  KeyEqual>(DEFAULT_HM_CAPACITY, 1.0f)),
         inserts_(0), maxItemsHint_(0), probeLimitScalingFactor_(1.0f)
        {}
    IrremovableFlatHashset(uint64_t maxItemsHint, float probeLimitScalingFactor = 1.0f)
        : impl(new FlatHashsetImpl<Key, HashPolicy, Hasher,  KeyEqual>(DEFAULT_HM_CAPACITY, probeLimitScalingFactor)),
         inserts_(0), maxItemsHint_(maxItemsHint) , probeLimitScalingFactor_(probeLimitScalingFactor)
        {}

    inline bool find(const key_type & key) {
        return impl->findNoErase(key);
    }

    bool insert(const key_type & key) {
        int ret = impl->insertNoErase(key);
        ++inserts_;
        if (ret == INSERT_GOOD)
            return true;
        else if (ret == INSERT_KEY_EXISTS)
            return false;
        // ret == INSERT_NO_VACANCY
        auto to = impl->growFrom(*impl, true, probeLimitScalingFactor_, maxItemsHint_, inserts_);
        if (to == nullptr) {
            throw std::runtime_error("failed to grow hashmap");
        }
        impl.reset(to);
        --inserts_;
        // tail-recursive call
        return insert(key);
    }

    inline void clear() {
        impl->clear();
    }

    inline size_t size() {
        return impl->size();
    }

    inline size_t capacity() {
        return impl->capacity();
    }

    struct const_iterator {
        const key_type & key() {
            return it.key();
        }

        bool operator!=(const const_iterator & rhs) {
            return it != rhs.it;
        }
        bool operator==(const const_iterator & rhs) {
            return it == rhs.it;
        }
        const_iterator & operator++() {
            ++it;
            return *this;
        }
        const_iterator operator++(int) {
            const_iterator old = *this;
            ++it;
            return old;
        }
        const_iterator(FlatHashsetImpl<Key, HashPolicy, Hasher, KeyEqual> * impl): it(impl->begin()) {}
        const_iterator(const typename FlatHashsetImpl<Key, HashPolicy, Hasher, KeyEqual>::iterator & implIterator): it(implIterator) {}
    private:
        typename FlatHashsetImpl<Key, HashPolicy, Hasher, KeyEqual>::iterator it;
    };

    inline const_iterator begin() {
        return const_iterator(impl.get());
    }
    inline const_iterator end() {
        return const_iterator(impl->end());
    }
private:
    std::unique_ptr<FlatHashsetImpl<Key, HashPolicy, Hasher, KeyEqual>> impl;
    uint64_t inserts_;
    const uint64_t maxItemsHint_;
    float probeLimitScalingFactor_;
};

template<int keyCounts, typename RawKeyType>
struct MultiCombinedKey {
    static_assert(std::is_integral<RawKeyType>::value, "RawKeyType must be one of integral types");
    RawKeyType keys[keyCounts];
    uint32_t idx;
    MultiCombinedKey():idx(0) {}

    template<typename Key, typename... Rest>
    MultiCombinedKey(const Key & key, Rest... rest):idx(0) {
        addKeys(key, rest...);
    }

    bool operator==(const MultiCombinedKey<keyCounts, RawKeyType> & rhs) const {
        return memcmp(keys, rhs.keys, sizeof(RawKeyType) * idx) == 0;
    }

    inline void addKeys() {}
    template<typename Key, typename... Rest>
    inline void addKeys(const Key & key, Rest... rest) {
        keys[idx++] = key;
        addKeys(rest...);
    }
    
};

template<int keyCounts, typename RawKeyType>
struct MultiCombinedKeyHasher{ 
    uint64_t operator()(const MultiCombinedKey<keyCounts, RawKeyType> & lhs) {
        return murmur32((const char *)&lhs.keys, sizeof(lhs.keys));
    }
};

template<int keyCounts, typename RawKeyType>
struct MultiCombinedKeyEqual{ 
    bool operator()(const MultiCombinedKey<keyCounts, RawKeyType> & lhs, const MultiCombinedKey<keyCounts, RawKeyType> & rhs) {
        return memcmp(lhs.keys, rhs.keys, sizeof(lhs.keys)) == 0;
    }
};


template<typename RawKeyType>
struct MultiCombinedKey<2, RawKeyType> {
    RawKeyType keys[2];

    MultiCombinedKey(){}

    template<typename Key1, typename Key2>
    MultiCombinedKey(const Key1 & key1, const Key2 & key2) {
        addKeys(key1, key2);
    }

    template<typename Key1, typename Key2>
    inline void addKeys(const Key1 & key1, const Key2 & key2) {
        keys[0] = key1;
        keys[1] = key2;
    }
    bool operator==(const MultiCombinedKey<2, RawKeyType> & rhs) const {
        return keys[0] == rhs.keys[0] && keys[1] == rhs.keys[1];
    }
};


template<>
struct MultiCombinedKey<2, uint32_t> {
    union{
        uint32_t keys[2];
        uint64_t bits64;
    };
    

    MultiCombinedKey(){}

    template<typename Key1, typename Key2>
    MultiCombinedKey(const Key1 & key1, const Key2 & key2) {
        addKeys(key1, key2);
    }

    template<typename Key1, typename Key2>
    inline void addKeys(const Key1 & key1, const Key2 & key2) {
        keys[0] = key1;
        keys[1] = key2;
    }
    bool operator==(const MultiCombinedKey<2, uint32_t> & rhs) const {
        return bits64 == rhs.bits64;
    }
};

template<typename RawKeyType>
struct MultiCombinedKey<3, RawKeyType> {
    RawKeyType keys[3];

    MultiCombinedKey() {}

    template<typename Key1, typename Key2, typename Key3>
    MultiCombinedKey(const Key1 & key1, const Key2 & key2, const Key3 & key3) {
        addKeys(key1, key2, key3);
    }

    template<typename Key1, typename Key2, typename Key3>
    inline void addKeys(const Key1 & key1, const Key2 & key2, const Key3 & key3) {
        keys[0] = key1;
        keys[1] = key2;
        keys[2] = key3;
    }
    bool operator==(const MultiCombinedKey<3, RawKeyType> & rhs) const {
        return memcmp(keys, rhs.keys, sizeof(keys)) == 0;
    }
};

typedef MultiCombinedKey<2, uint32_t> Double4BKey;
typedef MultiCombinedKey<3, uint32_t> Triple4BKey;
typedef MultiCombinedKeyHasher<2, uint32_t> Double4BKeyHasher;
typedef MultiCombinedKeyEqual<2, uint32_t> Double4BKeyEqual;
typedef MultiCombinedKeyHasher<3, uint32_t> Triple4BKeyHasher;
typedef MultiCombinedKeyEqual<3, uint32_t> Triple4BKeyEqual;

template<>
struct murmur_hasher<Double4BKey> {
    uint64_t operator()(const Double4BKey & key) {
        return MultiCombinedKeyHasher<2, uint32_t>{}(key);
    }
};

template<>
struct murmur_hasher<Triple4BKey> {
    uint64_t operator()(const Triple4BKey & key) {
        return MultiCombinedKeyHasher<3, uint32_t>{}(key);
    }
};

namespace std
{
    template<> struct hash<Double4BKey>
    {
        typedef Double4BKey argument_type;
        typedef std::size_t result_type;
        result_type operator()(argument_type const& s) const
        {
            return MultiCombinedKeyHasher<2, uint32_t>{}(s);
        }
    };
}

namespace std {
template<>
struct equal_to<Double4BKey> {
    bool operator()(const Double4BKey & key1, const Double4BKey & key2) const {
        return key1 == key2;
    }
};

template<>
struct equal_to<Triple4BKey> {
    bool operator()(const Triple4BKey & key1, const Triple4BKey & key2) const {
        return key1 == key2;
    }
};
}


typedef MultiCombinedKey<2, uint64_t> Double8BKey;
typedef MultiCombinedKey<3, uint64_t> Triple8BKey;
typedef MultiCombinedKeyHasher<2, uint64_t> Double8BKeyHasher;
typedef MultiCombinedKeyEqual<2, uint64_t> Double8BKeyEqual;
typedef MultiCombinedKeyHasher<3, uint64_t> Triple8BKeyHasher;
typedef MultiCombinedKeyEqual<3, uint64_t> Triple8BKeyEqual;

template<>
struct murmur_hasher<Double8BKey> {
    uint64_t operator()(const Double8BKey & key) {
        return MultiCombinedKeyHasher<2, uint64_t>{}(key);
    }
};

template<>
struct murmur_hasher<Triple8BKey> {
    uint64_t operator()(const Triple8BKey & key) {
        return MultiCombinedKeyHasher<3, uint64_t>{}(key);
    }
};

namespace std
{
    template<> struct hash<Double8BKey>
    {
        typedef Double8BKey argument_type;
        typedef std::size_t result_type;
        result_type operator()(argument_type const& s) const
        {
            return MultiCombinedKeyHasher<2, uint64_t>{}(s);
        }
    };
}

namespace std {
template<>
struct equal_to<Double8BKey> {
    bool operator()(const Double8BKey & key1, const Double8BKey & key2) const {
        return key1 == key2;
    }
};

template<>
struct equal_to<Triple8BKey> {
    bool operator()(const Triple8BKey & key1, const Triple8BKey & key2) const {
        return key1 == key2;
    }
};
}
#endif
