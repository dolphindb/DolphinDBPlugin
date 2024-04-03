#ifndef HASHMAP_UTIL_H_
#define HASHMAP_UTIL_H_

#include <cstdint>
#include <cstdlib>
#include <string>
#include <unordered_set>
#include <vector>
#include <atomic>
#include <algorithm>
#include <memory.h>

#include "Concurrent.h"
#include "DolphinString.h"
#include "Guid.h"
#include "WideInteger.h"

void* myAlloc(size_t size);
void myFree(void * ptr);

void* mySmallAlloc(size_t size);
void mySmallFree(void * ptr);

//-----------------------------------------------------------------------------
// MurmurHash2, by Austin Appleby

// Note - This code makes a few assumptions about how your machine behaves -

// 1. We can read a 4-byte value from any address without crashing
// 2. sizeof(int) == 4

// And it has a few limitations -

// 1. It will not work incrementally.
// 2. It will not produce the same results on little-endian and big-endian
//    machines.

static inline uint32_t murmur32 (const char *key, int len)
{
	// 'm' and 'r' are mixing constants generated offline.
	// They're not really 'magic', they just happen to work well.

	const uint32_t m = 0x5bd1e995;
	const int r = 24;

	// Initialize the hash to a 'random' value
	uint32_t h = len;

	// Mix 4 bytes at a time into the hash

	const unsigned char *data = (const unsigned char *)key;

	while(len >= 4)
	{
		uint32_t k = *(uint32_t *)data;

		k *= m; 
		k ^= k >> r; 
		k *= m; 
		
		h *= m; 
		h ^= k;

		data += 4;
		len -= 4;
	}
	
	// Handle the last few bytes of the input array

	switch(len)
	{
	case 3: h ^= data[2] << 16;
	/* no break */
	case 2: h ^= data[1] << 8;
	/* no break */
	case 1: h ^= data[0];
	        h *= m;
	};

	// Do a few final mixes of the hash to ensure the last few
	// bytes are well-incorporated.

	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

	return h;
} 

static inline uint32_t murmur32_16b (const unsigned char* key)
{
	const uint32_t m = 0x5bd1e995;
	const int r = 24;
	uint32_t h = 16;

	uint32_t k1 = *(uint32_t*)(key);
	uint32_t k2 = *(uint32_t*)(key + 4);
	uint32_t k3 = *(uint32_t*)(key + 8);
	uint32_t k4 = *(uint32_t*)(key + 12);

	k1 *= m;
	k1 ^= k1 >> r;
	k1 *= m;

	k2 *= m;
	k2 ^= k2 >> r;
	k2 *= m;

	k3 *= m;
	k3 ^= k3 >> r;
	k3 *= m;

	k4 *= m;
	k4 ^= k4 >> r;
	k4 *= m;

	// Mix 4 bytes at a time into the hash
	h *= m;
	h ^= k1;
	h *= m;
	h ^= k2;
	h *= m;
	h ^= k3;
	h *= m;
	h ^= k4;

	// Do a few final mixes of the hash to ensure the last few
	// bytes are well-incorporated.
	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

	return h;
}

static inline uint32_t murmur32_8b (uint64_t key)
{
    // 'm' and 'r' are mixing constants generated offline.
    // They're not really 'magic', they just happen to work well.

    const uint32_t m = 0x5bd1e995;
    const int r = 24;

    // Initialize the hash to a 'random' value
    uint32_t h = 8;

    uint32_t k1 = (uint32_t)(key >> 32);
    uint32_t k2 = (uint32_t)key;

    k1 *= m; 
    k1 ^= k1 >> r; 
    k1 *= m; 

    k2 *= m; 
    k2 ^= k2 >> r; 
    k2 *= m; 

    // Mix 4 bytes at a time into the hash

    h *= m; 
    h ^= k1;
    h *= m; 
    h ^= k2;

    // Do a few final mixes of the hash to ensure the last few
    // bytes are well-incorporated.

    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return h;
}

static inline uint32_t murmur32_4b (uint32_t key)
{
    // 'm' and 'r' are mixing constants generated offline.
    // They're not really 'magic', they just happen to work well.

    const uint32_t m = 0x5bd1e995;
    const int r = 24;

    // Initialize the hash to a 'random' value
    uint32_t h = 4;

    uint32_t k = *(uint32_t *)&key;

    k *= m; 
    k ^= k >> r; 
    k *= m; 

    // Mix 4 bytes at a time into the hash

    h *= m; 
    h ^= k;

    // Do a few final mixes of the hash to ensure the last few
    // bytes are well-incorporated.

    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return h;
}

namespace std {
template<>
struct hash<DolphinString> {
	inline size_t operator()(const DolphinString& val) const{
		return murmur32(val.data(), val.size());
	}
};

template<>
struct hash<Guid> {
	inline size_t operator()(const Guid& val) const{
		return murmur32_16b(val.bytes());
	}
};

};

typedef std::hash<Guid> GuidHash;

template<class T>
struct murmur_hasher {
    inline uint64_t operator()(const T&);
};


template<>
struct murmur_hasher<std::string> {
    uint64_t operator()(const std::string & val){
    	return murmur32(val.data(), val.size());
    }
};

template<>
struct murmur_hasher<DolphinString> {
    uint64_t operator()(const DolphinString & val){
        return murmur32(val.getData(), val.size());
    }
};

template<>
struct murmur_hasher<Guid> {
    uint64_t operator()(const Guid & val){
        return murmur32_16b(val.bytes());
    }
};

template<>
struct murmur_hasher<bool> {
    uint64_t operator()(const bool & val);
};
template<>
struct murmur_hasher<char> {
    uint64_t operator()(const char & val);
};
template<>
struct murmur_hasher<signed char> {
    uint64_t operator()(const signed char & val);
};
template<>
struct murmur_hasher<unsigned char> {
    uint64_t operator()(const unsigned char & val);
};
template<>
struct murmur_hasher<char16_t> {
    uint64_t operator()(const char16_t & val);
};
template<>
struct murmur_hasher<char32_t> {
    uint64_t operator()(const char32_t & val);
};
template<>
struct murmur_hasher<wchar_t> {
    uint64_t operator()(const wchar_t & val);
};
template<>
struct murmur_hasher<short> {
    uint64_t operator()(const short & val);
};
template<>
struct murmur_hasher<unsigned short> {
    uint64_t operator()(const unsigned short & val);
};
template<>
struct murmur_hasher<int> {
    uint64_t operator()(const int & val);
};
template<>
struct murmur_hasher<unsigned int> {
    uint64_t operator()(const unsigned int & val);
};
template<>
struct murmur_hasher<long> {
    uint64_t operator()(const long & val);
};
template<>
struct murmur_hasher<unsigned long> {
    uint64_t operator()(const unsigned long & val);
};
template<>
struct murmur_hasher<long long> {
    uint64_t operator()(const long long & val);
};
template<>
struct murmur_hasher<unsigned long long> {
    uint64_t operator()(const unsigned long long & val);
};
template<>
struct murmur_hasher<float> {
    uint64_t operator()(const float & val);
};
template<>
struct murmur_hasher<double> {
    uint64_t operator()(const double & val);
};
template<>
struct murmur_hasher<wide_integer::int128> {
    uint64_t operator()(const wide_integer::int128 & val);
};
template<>
struct murmur_hasher<wide_integer::uint128> {
    uint64_t operator()(const wide_integer::uint128 & val);
};
template<class T>
struct murmur_hasher<T*> {
    uint64_t operator()(const T* val);
};

uint64_t XXHash64(const char *key, int len);

template<class T>
struct XXHasher {
    inline uint64_t operator()(const T&);
};


template<>
struct XXHasher<std::string> {
    uint64_t operator()(const std::string & val);
};

template<>
struct XXHasher<DolphinString> {
    uint64_t operator()(const DolphinString & val);
};

template<>
struct XXHasher<Guid> {
    uint64_t operator()(const Guid & val);
};

template<>
struct XXHasher<bool> {
    uint64_t operator()(const bool & val);
};
template<>
struct XXHasher<char> {
    uint64_t operator()(const char & val);
};
template<>
struct XXHasher<signed char> {
    uint64_t operator()(const signed char & val);
};
template<>
struct XXHasher<unsigned char> {
    uint64_t operator()(const unsigned char & val);
};
template<>
struct XXHasher<char16_t> {
    uint64_t operator()(const char16_t & val);
};
template<>
struct XXHasher<char32_t> {
    uint64_t operator()(const char32_t & val);
};
template<>
struct XXHasher<wchar_t> {
    uint64_t operator()(const wchar_t & val);
};
template<>
struct XXHasher<short> {
    uint64_t operator()(const short & val);
};
template<>
struct XXHasher<unsigned short> {
    uint64_t operator()(const unsigned short & val);
};
template<>
struct XXHasher<int> {
    uint64_t operator()(const int & val);
};
template<>
struct XXHasher<unsigned int> {
    uint64_t operator()(const unsigned int & val);
};
template<>
struct XXHasher<long> {
    uint64_t operator()(const long & val);
};
template<>
struct XXHasher<unsigned long> {
    uint64_t operator()(const unsigned long & val);
};
template<>
struct XXHasher<long long> {
    uint64_t operator()(const long long & val);
};
template<>
struct XXHasher<unsigned long long> {
    uint64_t operator()(const unsigned long long & val);
};
template<>
struct XXHasher<float> {
    uint64_t operator()(const float & val);
};
template<>
struct XXHasher<double> {
    uint64_t operator()(const double & val);
};
template<>
struct XXHasher<wide_integer::int128> {
    uint64_t operator()(const wide_integer::int128 & val);
};
template<>
struct XXHasher<wide_integer::uint128> {
    uint64_t operator()(const wide_integer::uint128 & val);
};
template<class T>
struct XXHasher<T*> {
    uint64_t operator()(const T* val);
};

/*
 * Copyright (c) 2013, Marwan Burelle
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

// Hazard Pointer C++11 implem based on Michael (2004) article

// TODO:
// * cleaning, renaming ...
// * correct iterator for hprecord list
// * try to make hazard pointers look like smart pointers
// * more coherent use of inline/noexcept ...

template <typename T>
class hazard_pointer_manager;

// hprecord<T> a "block" of K hazard_pointer<T>
// Normaly provides through hazard_pointer_manager and hprecord_guard as a
// stores for managed shared pointers.
template <typename T>
class hprecord {
public:
	hprecord(hazard_pointer_manager<T> * mngr) : hp(0), next(0), manager(mngr), active(ATOMIC_FLAG_INIT) {}

	// A kind of try lock: if the record is available return true and lock it
	// for futher test.
	bool try_acquire() noexcept { return !active.test_and_set(); };

	// just release the active flag don't unset pointers
	void release() noexcept { active.clear(); }

	//give back the record and reset each stored pointers (making them
	// deletable)
	void retire() noexcept {
		hp = 0;
		active.clear();
	}

	inline void reset() noexcept {
		hp = 0;
	}

	// Those element should be accessible (at least through the guard.)
	T*     hp;
	std::vector<T*>       rlist;
	hprecord             *next;
	hazard_pointer_manager<T> * manager;
	~hprecord () {
        for (auto e : rlist) {
            e->~T();
            mySmallFree(e);
        }
        rlist.clear();
	}

private:
	std::atomic_flag      active;
};

// hazard_pointer_manager<T> manage a set of hprecord<T>
// responsible for the creation of records and their assignment
template <typename T>
class hazard_pointer_manager {
public:
	// Default constructor
	hazard_pointer_manager() :  H(0), head(0) {}

	// Ask for a record
	hprecord<T>* acquire() noexcept {
		hprecord<T>* cur = head.load();
		for (; cur; cur = cur->next) {
			if (!cur->try_acquire()) continue;
			return cur;
		}
		H.fetch_add(1);
		void * ptr = mySmallAlloc(sizeof(hprecord<T>));
		cur = new(ptr) hprecord<T>(this);
		cur->try_acquire();
		hprecord<T>      *oldhead = head.load();
		do
			cur->next = oldhead;
		while (!head.compare_exchange_strong(oldhead, cur));
		return cur;
	}

	void diagonose() {
		int total = 0;
		for (auto cur = this->begin(); cur != 0; cur = cur->next) {
			total += cur->rlist.size();
			printf("remaining %u\n", cur->rlist.size());
		}
		printf("total %d, H %d\n", total, H.load());
	}

	~hazard_pointer_manager() {
		int iterations = 0;
		while (true) {
			int hazards = 0;
			for (auto cur = this->begin(); cur != 0; cur = cur->next) {
				T *p = cur->hp;
				if (p != 0) {
					hazards++;
				}
			}
			iterations++;
			if (hazards != 0) {
				//printf("manager %p, still waiting for %d hazards, iterations %d\n",this, hazards, iterations);
			} else {
				//printf("manager %p, all gone, took %d iterations, head %p\n", this, iterations, head.load());
				break;
			}
		}

		while (head.load()) {
			hprecord<T> * ptr, * next;
			do {
				ptr = head.load();
				next = ptr->next;
			} while (head.compare_exchange_strong(ptr, next) == false);
			// wait until other thread has released this hazard pointer
			while (ptr->try_acquire() == false);
			// Potential ABA problem. But since it's in a destructor, there shouln't be any other threads accessing the manager object.
			//printf("manager %p deleting pointer %p\n", this, ptr);
			ptr->~hprecord<T>();
			mySmallFree((void *)ptr);
		}
	}
	// Iterations: we shouldn't use a classic iterator scheme since we need
	// specific atomic pointer access.

	// Access to the first element of the records list
	hprecord<T>* begin() noexcept {
		return head.load();
	}

	// how many pointer in the wait list before concrete deletion
	inline unsigned threshold() noexcept {
		unsigned    h = H.load(std::memory_order_relaxed);
		return 2*h + h/4;
	}


private:
	// Already built records
	mutable std::atomic<unsigned>         H;
	// head of the records list
	mutable std::atomic<hprecord<T>*>   head;

	// keep track of last usage record
	static thread_local void*      mine;
};


template <typename T>
class hprecord_guard {
public:
	hprecord_guard(hazard_pointer_manager<T>& _hpm) : ptr(_hpm.acquire()), hpm(_hpm) {}

	// No copy, no move !
	hprecord_guard(const hprecord_guard& r) = delete;
	hprecord_guard(hprecord_guard&& r) = delete;

	// Destructor: recycle the wrapped hprecord
	~hprecord_guard() { ptr->retire(); }

	// transparent access to the hp array
	void protect (T *p)  {
		ptr->hp = p;
		std::atomic_thread_fence(std::memory_order_seq_cst);
	}

	// reset hp pointers
	inline void reset()  { ptr->reset(); }

	// Try to delete a managed shared pointer.
	// This is the classic operation from original hazard pointer: we add the
	// pointer to a "wait list" and if the list is bigger than a given threshold
	// we try to concretely delete elements in the wait-list.
	void retire_node(T* node)  {
		ptr->rlist.push_back(node);
		if (ptr->rlist.size() >= hpm.threshold()) {
			scan();
			//help_scan();
		}
	}

	// try to delete all unused pointers
	// this is also a classical example from the original hazard pointers design
	void scan()  {
		std::unordered_set<T*>      plist;
		for (auto cur = hpm.begin(); cur != 0; cur = cur->next) {
			T *p = cur->hp;
			if (p != 0) plist.insert(p);
		}
		ptr->rlist.erase(std::remove_if(ptr->rlist.begin(), ptr->rlist.end(), [&](T* p) -> bool {
			if (plist.find(p) == plist.end()) {
                p->~T();
				mySmallFree(p);
				return true;
			}
			return false;
		}), ptr->rlist.end());
	}

	// try to delete unused pointers from free hprecords
	void help_scan()  {
		for (auto cur = hpm.begin(); cur != 0; cur = cur->next) {
			if (!cur->try_acquire()) continue;
			// steal pointers from other record to this record
			for (auto e : cur->rlist)
				ptr->rlist.push_back(e);
			cur->rlist.clear();
			if (ptr->rlist.size() >= hpm.threshold())
				scan();
			cur->release();
		}
	}

private:
	hprecord<T>                *ptr;
	hazard_pointer_manager<T>&  hpm;
};

template<typename T>
thread_local void* hazard_pointer_manager<T>::mine = 0;



#ifndef CACHE_LINE_SIZE
#define CACHE_LINE_SIZE  64 // 64 byte cache line on x86 and x86-64
#endif

#define INT64S_PER_CACHELINE (CACHE_LINE_SIZE / sizeof(int64_t))
#define INT64S_PER_CACHELINE_SCALE 4

static thread_local bool cpuIdInitialized;
static thread_local uint64_t cpuId;
template<int buckets = 32>
class DistributedCounter {
public:
    static_assert(buckets == 0 || (buckets & (buckets - 1)) == 0, "buckets must be a multiple of 2");

    DistributedCounter(int initVal = 0) {
        countArrayPtr = myAlloc(buckets * INT64S_PER_CACHELINE * sizeof(int64_t) + CACHE_LINE_SIZE - 1);
        memset(countArrayPtr, 0, buckets * INT64S_PER_CACHELINE * sizeof(int64_t) + CACHE_LINE_SIZE - 1);
        countArray = (int64_t *)(((size_t)countArrayPtr + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1));
        increment(initVal);
    }

    ~DistributedCounter() {
        myFree(countArrayPtr);
    }
    inline void increment(int v = 1) {
        __atomic_add_fetch(&countArray[arrayIndex() * INT64S_PER_CACHELINE], v, __ATOMIC_RELAXED);
        //countArray[arrayIndex() * INT64S_PER_CACHELINE] += v;
    }

    inline void decrement(int v = 1) {
        __atomic_sub_fetch(&countArray[arrayIndex() * INT64S_PER_CACHELINE], v, __ATOMIC_RELAXED);
        //countArray[arrayIndex() * INT64S_PER_CACHELINE] -= v;
    }

    int64_t get() {
        int64_t val = 0;
        for (int i = 0; i < totalINT64S; i += INT64S_PER_CACHELINE) {
            val += __atomic_load_n(&countArray[i], __ATOMIC_RELAXED);
        }
        return val;
    }

private:
    static constexpr int totalINT64S = buckets * INT64S_PER_CACHELINE;
    inline uint64_t getCPUId() {
        if (cpuIdInitialized == false) {
            cpuId = (uint64_t)std::hash<int>{}(Thread::getID());
            cpuIdInitialized = true;
            //printf("cpuid %lu, arrayIndex %d, pointer %p\n", cpuId, arrayIndex(), &countArray[arrayIndex() * INT64S_PER_CACHELINE]);
        }
        return cpuId;
    }

    inline int arrayIndex() {
        return getCPUId() & (buckets  - 1);
    }

    int64_t * countArray;
    void * countArrayPtr;
};
#endif
