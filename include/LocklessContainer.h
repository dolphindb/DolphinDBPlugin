#ifndef LOCKLESS_CONTAINER_H_
#define LOCKLESS_CONTAINER_H_

#include <sys/types.h>

#include <cstdlib>
#include <cstdint>
#include <cassert>
#include <climits>
#include <cstring>
#include <vector>
#include <atomic>
#include <string>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include <utility>

#include "Concurrent.h"
#include "HashmapUtil.h"
#include "FlatHashmap.h"
#include "Exceptions.h"


#define CACHE_LINE_SIZE  64 // 64 byte cache line on x86 and x86-64
#define CACHE_LINE_SCALE 6  // log base 2 of the cache line size

#define EXPECT_TRUE_INTERNAL(x)      __builtin_expect(!!(x), 1)
#define EXPECT_FALSE_INTERNAL(x)     __builtin_expect(!!(x), 0)

#ifndef NBD_SINGLE_THREADED

#define MAX_NUM_THREADS  32 // make this whatever you want, but make it a power of 2

#define SYNC_SWAP(addr,x)         __sync_lock_test_and_set(addr,x)
#define SYNC_CAS(addr,old,x)      __sync_val_compare_and_swap(addr,old,x)
#define SYNC_ADD(addr,n)          __sync_add_and_fetch(addr,n)
#define SYNC_FETCH_AND_OR(addr,x) __sync_fetch_and_or(addr,x)
#else// NBD_SINGLE_THREADED

#define MAX_NUM_THREADS  1

#define SYNC_SWAP(addr,x)         ({ __typeof__(*(addr)) _old = *(addr); *(addr)  = (x); _old; })
#define SYNC_CAS(addr,old,x)      ({ __typeof__(*(addr)) _old = *(addr); *(addr)  = (x); _old; })
//#define SYNC_CAS(addr,old,x)    ({ __typeof__(*(addr)) _old = *(addr); if ((old) == _old) { *(addr)  = (x); } _old; })
#define SYNC_ADD(addr,n)          ({ __typeof__(*(addr)) _old = *(addr); *(addr) += (n); _old; })
#define SYNC_FETCH_AND_OR(addr,x) ({ __typeof__(*(addr)) _old = *(addr); *(addr) |= (x); _old; })

#endif//NBD_SINGLE_THREADED

#define COUNT_TRAILING_ZEROS __builtin_ctz

#define MASK(n)     ((1ULL << (n)) - 1)

#define TRUE  1
#define FALSE 0

#ifdef BIT32
#define TAG1         (1U << 31)
#define TAG2         (1U << 30)
#else
#define TAG1         (1ULL << 63)
#define TAG2         (1ULL << 62)
#endif
#define TAG_VALUE(v, tag) ((v) |  tag)
#define IS_TAGGED(v, tag) ((v) &  tag)
#define STRIP_TAG(v, tag) ((v) & ~tag)


#ifdef BIT32
typedef uint32_t map_key_t;
typedef uint32_t map_val_t;
#else
typedef uint64_t map_key_t;
typedef uint64_t map_val_t;
#endif

#define CAS_EXPECT_DOES_NOT_EXIST (0)
#define CAS_EXPECT_EXISTS         (1)
#define CAS_EXPECT_WHATEVER       (2)


#define VOLATILE_DEREF(x) (*((volatile __typeof__(x))(x)))



#define DOES_NOT_EXIST 0
#define RETRY 3
// 1000000000000000000...
#define COPIED_VALUE  TAG_VALUE(DOES_NOT_EXIST, TAG1)
// 0111111111111111111...
#define TOMBSTONE STRIP_TAG(-1, TAG1)

#ifndef ENABLE_TRACE
#define TRACE(...) do { } while (0)
#else
#define TRACE(flag, format, v1, v2) printf(format, (size_t)(v1), (size_t)(v2));puts("")
#endif




typedef void  (*deleter_t)(void*);
typedef void* (*dupper_t)(void*);
typedef uint32_t (*hasher_t)(void*);
typedef bool (*equal_t)(void *, void*);
typedef void (*extract_t)(map_val_t, void*);
/* internal hash entry */
typedef struct entry {
    map_key_t key;
    map_val_t val;
} entry_t;

/* multi-threaded version of LocklessHashmap */
typedef struct hti hti_t;
typedef struct ht {
    std::atomic<hti_t *> hti;
    std::atomic<uint32_t> hti_copies;
    double density;
    int probe;
    deleter_t key_deleter;
    deleter_t val_deleter;
    dupper_t key_dupper;
    hasher_t key_hasher;
    equal_t key_equal;
    ht(deleter_t, deleter_t, dupper_t, hasher_t, equal_t);
    hazard_pointer_manager<hti_t> haz_manager;
    ~ht();
} hashtable_t;

typedef struct hti {
    entry_t *table;
    hashtable_t *ht; // parent ht;
    std::atomic<hti *> next;
    void *unaligned_table_ptr; // system malloc doesn't guarentee cache-line alignment
    std::atomic<uint32_t> count; // TODO: make these counters distributed
    std::atomic<uint32_t> key_count;
    std::atomic<uint32_t> copy_scan;
    std::atomic<uint32_t> num_entries_copied;
    int probe;
    uint8_t scale;

    hti() :table(NULL), ht(NULL), next(NULL), unaligned_table_ptr(NULL), count(0), key_count(0), copy_scan(0), num_entries_copied(0), probe(0), scale(0) {}
    ~hti() {
        uint64_t end = (1ULL << scale) + probe;
        if (next.load() == NULL) { // the last table is responsible for freeing all kv pointers that haven't been erased from the hashtable
            for (uint64_t i = 0; i < end; ++i) {
                if (table[i].key) {
                    ht->key_deleter((void*)table[i].key); // strip off hash
                }
                // there are no values that have been copied to the next table.
                if (table[i].val != TOMBSTONE && table[i].val) {
                    ht->val_deleter((void *)table[i].val);
                }
            }
        } else { // check for toomstone values and delete its key
            for (uint64_t i = 0; i < end; ++i) {
                if (table[i].key) {
                    ht->key_deleter((void*)table[i].key);
                }
            }
        }
        
        mySmallFree(unaligned_table_ptr);
    }
} hti_t;
int           hti_help_copy (hti_t * hti);

map_val_t     ht_cas        (hashtable_t *ht, map_key_t key, map_val_t expected_val, map_val_t val, entry_t**holder);
map_val_t     ht_get        (hashtable_t *ht, map_key_t key, entry_t**holder);
map_val_t     ht_get        (hashtable_t *ht, map_key_t key, entry_t**holder, extract_t ext, void *ptr);
map_val_t     ht_remove     (hashtable_t *ht, map_key_t key, entry_t**holder);
size_t        ht_count      (hashtable_t *ht);
void          ht_print      (hashtable_t *ht, int verbose);
map_val_t     ht_val_internal (map_val_t internal_val);

/*
* A lockfree hashtable implementation inspired by the work of Cliff Click and Josh Dybnis.
*/
template<typename Key,
         typename T,
         typename Hash = murmur_hasher<Key>,
         typename KeyEqual = std::equal_to<Key>>
class LocklessHashmap {
public:
	typedef Key key_type;
	typedef T mapped_type;
    typedef Hash hasher;
    typedef KeyEqual key_equal;
	typedef size_t size_type;
	LocklessHashmap(): ht(key_entry_deleter, value_entry_deleter, key_entry_dupper, key_entry_hasher, key_entry_equal) {}
	bool find(const key_type & key, mapped_type & recv) {
		key_entry ks = make_key_entry(key);
		entry_t *holder;
		// fast path for non-existent entry
		value_entry* vp = (value_entry *)(ht_get(&this->ht, (map_key_t)&ks, &holder));
		if (vp == DOES_NOT_EXIST) {
			return false;
		}

		hprecord_guard<value_entry> hp(this->hm);
		do {
			hp.protect(vp);
			value_entry * cur = (value_entry*)ht_val_internal(__atomic_load_n(&holder->val, __ATOMIC_ACQUIRE));
			if (vp == cur)
				break;
			vp = cur;
		} while (true);
		// Now vp can be safely accessed
		// This protection makes sure that concurrent removal won't delete entries
		// that readers have references to.
		if (EXPECT_FALSE_INTERNAL(vp == (value_entry *)RETRY)) { // the value has been copied to the next table.
			return find(key, recv);
		} else if (vp == DOES_NOT_EXIST) {
			return false;
		}
		recv = vp->val;
		return true;
	}

	// Returns a renference to the value corresponds to the given key.
	// If the key is not in the table, a key-value pair with the default
	// value of the mapped_type is inserted into the table.
	// !!!This is unsafe to concurrnet read & write operations
	// as it returns value reference to the caller.
	mapped_type & operator[](const key_type &key) {
		key_entry ks = make_key_entry(key);
		entry_t *holder;
		map_val_t val = ht_get(&this->ht, (map_key_t)&ks, &holder);
		// I didn't use hazard pointer here because the semantic
		// of returning reference is already thread unsafe anyway.
		if (val == DOES_NOT_EXIST) {
            void * ptr = mySmallAlloc(sizeof(value_entry));
            assert(ptr);
			value_entry * vp = new(ptr) value_entry(mapped_type());
			ht_cas(&this->ht, (map_key_t)&ks, CAS_EXPECT_WHATEVER, (map_val_t)vp, &holder);
			return vp->val;
		}
		return reinterpret_cast<value_entry*>(val)->val;
	}

	// insert key-value pair {key, val} into the table.
	// return true if the insertion is successful.
	// return false if key-value pair is already in the table.
	bool insert(const key_type & key, const mapped_type & val) {
		key_entry ks = make_key_entry(key); // make a key string on heap
        void * ptr = mySmallAlloc(sizeof(value_entry));
        assert(ptr);
		value_entry * vp = new(ptr) value_entry(val);
		entry_t *holder;
		map_val_t old_val = ht_cas(&this->ht, (map_key_t)&ks, CAS_EXPECT_DOES_NOT_EXIST,(map_val_t)vp, &holder);
		if (old_val != DOES_NOT_EXIST) {
			// delete right away.
			value_entry_deleter(vp);
			return false;
		}
		return true;
	}

	//Return the approximate size of table.
	size_type size() const noexcept{
        hashtable_t *ht = const_cast<hashtable_t*>(&this->ht);
		return ht_count(ht);
	}

	// Erase the entry with key from the hashtable
	// return true if the deletion is successful,
	// false otherwise.
	bool erase(const key_type & key) {
		key_entry ks = make_key_entry(key);
		entry_t *holder;
		value_entry * oldvp = reinterpret_cast<value_entry*>(ht_remove(&this->ht, (map_key_t)&ks, &holder));
		//fast path for non-existent key
		if (oldvp == DOES_NOT_EXIST) {
			return false;
		}
		hprecord_guard<value_entry> hp(this->hm);
		// Since ht_remove is atomic internally, there are no concurrent deletions on the same value_entry pointer.
		// Simply retire the old value_entry pointer for later reclmation.
		hp.retire_node(oldvp);
		return true;
	}

    void clear() {
        /* copy entries to the last table */
        hashtable_t *ht = &this->ht;
        while (true) {
            hprecord_guard<hti_t> hp(ht->haz_manager);
            hti_t * hti, * next;
            do {
                hti = ht->hti.load(std::memory_order_acquire);
                hp.protect(hti);
            } while (hti != ht->hti.load(std::memory_order_acquire));
            next = hti->next;
            if (next == nullptr) {
                break;
            }
            int done = 0;
            // finish copying
            while ((done =hti_help_copy(hti)) == false);

            // Unlink fully copied tables.
            assert(next);
            //printf("clear done helping hti %p copy\n", hti);
            if (ht->hti.compare_exchange_strong(hti, next, std::memory_order_release)) {
                // retire hti
                hp.retire_node(hti);
            }
        }
        assert(ht->hti);
        hti_t * head = ht->hti;
        for (uint64_t i = 0; i < (1ULL << head->scale); ++i) {
            map_key_t key = reinterpret_cast<map_key_t>(head->table[i].key);
            if (key == DOES_NOT_EXIST)
                continue;
            entry_t* holder;
            map_val_t oldvp = ht_remove(ht, key, &holder);
            if (oldvp == DOES_NOT_EXIST)
                continue;
            hprecord_guard<value_entry> hp(this->hm);
            // Since ht_remove is atomic internally, there are no concurrent deletions on the same value_entry pointer.
            // Simply retire the old value_entry pointer for later reclmation.
            hp.retire_node(reinterpret_cast<value_entry*>(oldvp));
            //retired++;
        }
    }

	~LocklessHashmap() {
		// this->hm manages defered deletions on value_entry.
		// Upon destruction, this->hm.~hazard_pointer_manager<value_entry>()
		// will wait until all readers released the references to its managed
		// set of pointers and then free them.
		// Similarly this->ht manages defered deletions on tables used internally for copying.
	}

    void diagonose() {
        hm.diagonose();
    }
private:
	struct value_entry {
		mapped_type val;
		value_entry(const mapped_type & val_): val(val_) {}
	};
    struct key_entry {
        key_type val;
        key_entry(const key_type & val_): val(val_) {}
    };
    static key_entry make_key_entry(const key_type & key) {
		return key_entry(key);
	}
	static void value_entry_deleter(void * p) {
		if (p == nullptr) return;
		value_entry *vp = (value_entry*)p;
		vp->~value_entry();
        mySmallFree((void *)vp);
	}
    static void* key_entry_dupper(void *p) {
        if (p == nullptr) return nullptr;
        key_entry * kp = (key_entry*)p;
        void* ptr = mySmallAlloc(sizeof(key_entry));
        assert(ptr);
        key_entry * dkp = new(ptr) key_entry(kp->val);
        return dkp;
    }
    static void key_entry_deleter(void *p) {
        if (p == nullptr) return;
        key_entry *kp = (key_entry*)p;
        kp->~key_entry();
        mySmallFree((void *)kp);
    }
    static uint32_t key_entry_hasher(void *p) {
        if (p == nullptr) return 0;
        key_entry *kp = (key_entry*)p;
        return key_hasher_(kp->val);
    }
    static bool key_entry_equal(void *p1, void *p2) {
        key_entry *kp1 = (key_entry*)p1;
        key_entry *kp2 = (key_entry*)p2;
        return key_equal_(kp1->val, kp2->val);
    }
    static key_equal key_equal_;
    static hasher key_hasher_;
	hashtable_t ht;
	hazard_pointer_manager<value_entry> hm;
};
template<typename Key,
         typename T,
         typename Hash,
         typename KeyEqual>
KeyEqual LocklessHashmap<Key, T, Hash, KeyEqual>::key_equal_;
template<typename Key,
         typename T,
         typename Hash,
         typename KeyEqual>
Hash LocklessHashmap<Key, T, Hash, KeyEqual>::key_hasher_;



/*
* A lockfree hashtable implementation without key removal inspired by the work of Cliff Click and Josh Dybnis.
*/
template<typename Key,
         typename T,
         typename Hash = murmur_hasher<Key>,
         typename KeyEqual = std::equal_to<Key>>
class IrremovableLocklessHashmap {
public:
	typedef Key key_type;
	typedef T mapped_type;
    typedef Hash hasher;
    typedef KeyEqual key_equal;
	typedef size_t size_type;
	IrremovableLocklessHashmap(): ht(key_entry_deleter, value_entry_deleter, key_entry_dupper, key_entry_hasher, key_entry_equal) {}
	bool find(const key_type & key, mapped_type & recv) {
		key_entry ks = make_key_entry(key);
		entry_t *holder;
		// fast path for non-existent entry
		value_entry* vp = (value_entry *)(ht_get(&this->ht, (map_key_t)&ks, &holder, value_extract, &recv));
		if (vp == DOES_NOT_EXIST) {
			return false;
		}
		return true;
	}

	// Returns a renference to the value corresponds to the given key.
	// If the key is not in the table, a key-value pair with the default
	// value of the mapped_type is inserted into the table.
	// !!!This is unsafe to concurrnet read & write operations
	// as it returns value reference to the caller.
	mapped_type & operator[](const key_type &key) {
		key_entry ks = make_key_entry(key);
		entry_t *holder;
		map_val_t val = ht_get(&this->ht, (map_key_t)&ks, &holder);
		// I didn't use hazard pointer here because the semantic
		// of returning reference is already thread unsafe anyway.
		if (val == DOES_NOT_EXIST) {
            void * ptr = mySmallAlloc(sizeof(value_entry));
            assert(ptr);
			value_entry * vp = new(ptr) value_entry(mapped_type());
			ht_cas(&this->ht, (map_key_t)&ks, CAS_EXPECT_WHATEVER, (map_val_t)vp, &holder);
			return vp->val;
		}
		return reinterpret_cast<value_entry*>(val)->val;
	}

	// insert key-value pair {key, val} into the table.
	// return true if the insertion is successful.
	// return false if key-value pair is already in the table.
	bool insert(const key_type & key, const mapped_type & val) {
		key_entry ks = make_key_entry(key); // make a key string on heap
        void * ptr = mySmallAlloc(sizeof(value_entry));
        assert(ptr);
		value_entry * vp = new(ptr) value_entry(val);
		entry_t *holder;
		map_val_t old_val = ht_cas(&this->ht, (map_key_t)&ks, CAS_EXPECT_DOES_NOT_EXIST,(map_val_t)vp, &holder);
		if (old_val != DOES_NOT_EXIST) {
			// delete right away.
			value_entry_deleter(vp);
			return false;
		}
		return true;
	}

	//Return the approximate size of table.
	size_type size() const noexcept{
        hashtable_t *ht = const_cast<hashtable_t*>(&this->ht);
		return ht_count(ht);
	}

	~IrremovableLocklessHashmap() {
		// this->hm manages defered deletions on value_entry.
		// Upon destruction, this->hm.~hazard_pointer_manager<value_entry>()
		// will wait until all readers released the references to its managed
		// set of pointers and then free them.
		// Similarly this->ht manages defered deletions on tables used internally for copying.
	}

    void clear() {
        /* copy entries to the last table */
        hashtable_t *ht = &this->ht;
        while (true) {
            hprecord_guard<hti_t> hp(ht->haz_manager);
            hti_t * hti, * next;
            do {
                hti = ht->hti.load(std::memory_order_acquire);
                hp.protect(hti);
            } while (hti != ht->hti.load(std::memory_order_acquire));
            next = hti->next;
            if (next == nullptr) {
                break;
            }
            int done = 0;
            // finish copying
            while ((done =hti_help_copy(hti)) == false);

            // Unlink fully copied tables.
            assert(next);
            //printf("clear done helping hti %p copy\n", hti);
            if (ht->hti.compare_exchange_strong(hti, next, std::memory_order_release)) {
                // retire hti
                hp.retire_node(hti);
            }
        }
        assert(ht->hti);
        hti_t * head = ht->hti;
        for (uint64_t i = 0; i < (1ULL << head->scale); ++i) {
            map_key_t key = reinterpret_cast<map_key_t>(head->table[i].key);
            if (key == DOES_NOT_EXIST)
                continue;
            entry_t* holder;
            map_val_t oldvp = ht_remove(ht, key, &holder);
            if (oldvp == DOES_NOT_EXIST)
                continue;
            hprecord_guard<value_entry> hp(this->hm);
            // Since ht_remove is atomic internally, there are no concurrent deletions on the same value_entry pointer.
            // Simply retire the old value_entry pointer for later reclmation.
            hp.retire_node(reinterpret_cast<value_entry*>(oldvp));
            //retired++;
        }
    }

    void diagonose() {
        //hm.diagonose();
    }
private:
	struct value_entry {
		mapped_type val;
		value_entry(const mapped_type & val_): val(val_) {}
	};
    struct key_entry {
        key_type val;
        key_entry(const key_type & val_): val(val_) {}
    };
    static void value_extract(map_val_t mapval, void * valp) {
        value_entry *vp = reinterpret_cast<value_entry*>(mapval);
        *reinterpret_cast<value_entry*>(valp) = vp->val;
    }
    static key_entry make_key_entry(const key_type & key) {
		return key_entry(key);
	}
	static void value_entry_deleter(void * p) {
		if (p == nullptr) return;
		value_entry *vp = (value_entry*)p;
		vp->~value_entry();
        mySmallFree((void *)vp);
	}
    static void* key_entry_dupper(void *p) {
        if (p == nullptr) return nullptr;
        key_entry * kp = (key_entry*)p;
        void* ptr = mySmallAlloc(sizeof(key_entry));
        assert(ptr);
        key_entry * dkp = new(ptr) key_entry(kp->val);
        return dkp;
    }
    static void key_entry_deleter(void *p) {
        if (p == nullptr) return;
        key_entry *kp = (key_entry*)p;
        kp->~key_entry();
        mySmallFree((void *)kp);
    }
    static uint32_t key_entry_hasher(void *p) {
        if (p == nullptr) return 0;
        key_entry *kp = (key_entry*)p;
        return key_hasher_(kp->val);
    }
    static bool key_entry_equal(void *p1, void *p2) {
        key_entry *kp1 = (key_entry*)p1;
        key_entry *kp2 = (key_entry*)p2;
        return key_equal_(kp1->val, kp2->val);
    }
    static key_equal key_equal_;
    static hasher key_hasher_;
	hashtable_t ht;
    hazard_pointer_manager<value_entry> hm;
};
template<typename Key,
         typename T,
         typename Hash,
         typename KeyEqual>
KeyEqual IrremovableLocklessHashmap<Key, T, Hash, KeyEqual>::key_equal_;
template<typename Key,
         typename T,
         typename Hash,
         typename KeyEqual>
Hash IrremovableLocklessHashmap<Key, T, Hash, KeyEqual>::key_hasher_;


#define CACHE_ALIGNED __attribute__((aligned(CACHE_LINE_SIZE)))
#define DEFAULT_QUEUE_CAPACITY 4096

/*
* A lockfree bounded queue implementation using ring buffer & atomics.
* This queue only supports single-producer-single-consumer(SPSC) setup.
*/
template<typename T>
class SPSCLocklessBoundedQueue{
public:
    SPSCLocklessBoundedQueue(const uint64_t cap = DEFAULT_QUEUE_CAPACITY) 
        : head(0), tail(0), capacity(cap), buffer(new T[capacity]) {}

    bool push(const T& item) {
        uint64_t curTail = tail.load();
        uint64_t curHead = head.load();
        if (curTail - curHead >= capacity)
            return false;
        buffer[curTail % capacity] = item;
        tail.fetch_add(1);
        return true;
    }

    bool pop(T& item) {
        uint64_t curHead = head.load();
        uint64_t curTail = tail.load();
        if (curHead >= curTail)
            return false;
        item = buffer[curHead % capacity];
        head.fetch_add(1);
        return true;
    }
    int size() {
        return tail.load() - head.load();
    }
    ~SPSCLocklessBoundedQueue() {
        delete[] buffer;
    }
private:
    std::atomic<uint64_t> head CACHE_ALIGNED;
    std::atomic<uint64_t> tail CACHE_ALIGNED;
    const uint64_t capacity;
    T *buffer;
};


/*
* A lockless unbouned multi-producer-multi-consumer queue
* implementation using linked list & cas & hazard-pointer.
*/
template<typename T>
class LocklessBoundlessQueue{
public:
    LocklessBoundlessQueue(){
        void * ptr = mySmallAlloc(sizeof(Node));
        if (ptr == nullptr) {
        	throw MemoryException();
        }
        head = new(ptr) Node();
        tail = head;
    }
    void push(const T& item) {
        void * ptr = mySmallAlloc(sizeof(Node));
        if (ptr == nullptr) {
        	throw MemoryException();
        }
        Node* newNode = new(ptr) Node(item);
        Node* curTail = tail;
        Node* expected;
        hprecord_guard<Node> hp(hazardManager);
        while (true) {
            do {
                expected = curTail;
                hp.protect(curTail);
            } while (expected != (curTail = __atomic_load_n(&tail, __ATOMIC_SEQ_CST)));
            // Now we are sure curTail won't get deleted, go ahead and access it.
            // Multiple threads compete to install node as the tail and only one will win the race
            if (__atomic_compare_exchange_n(&tail, &curTail, newNode, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
                // Winner connects the link.
                // Here a poping thread may see the queue as empty
                // while the node has been installed, but that's ok.
                assert(curTail->next == nullptr);
                curTail->next = newNode;
                break;
            }
        }
    }

    bool pop(T& item) {
        Node* curHead = head;
        Node* expected, *next;
        hprecord_guard<Node> hp1(hazardManager);
        hprecord_guard<Node> hp2(hazardManager);
        while (true) {
            do {
                expected = curHead;
                hp1.protect(curHead);
            } while (expected != (curHead = __atomic_load_n(&head, __ATOMIC_SEQ_CST)));
            // Now we are sure curHead won't get deleted and curHead will never be null, go ahead and access it.
            next = curHead->next;
            if (next == nullptr)
                break;
            // make sure next won't be deleted by other threads
            hp2.protect(next);
            if (next != __atomic_load_n(&curHead->next, __ATOMIC_SEQ_CST)) {
                continue;
            }
            if (__atomic_compare_exchange_n(&head, &curHead, next, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
                item = next->v;
                curHead->next = nullptr;
                hp1.retire_node(curHead);
                return true;
            }
        }
        return false;
    }
    int size() {
        throw std::runtime_error("size() is unsupported");
    }
    ~LocklessBoundlessQueue() { head->~Node(); mySmallFree(head); }
private:
    struct Node {
        T v;
        Node *next;
        Node(): next(nullptr) {;}
        Node(const T & val):v(val), next(nullptr) {}
        ~Node() {}
    };
    // head always points to the node that has been poped(intiailly points to a dummy node),
    // so the real head is head->next.
    // This way, we don't have to check pointers for NULLness and empty() could be implemented
    // by checking if head->next == nullptr.
    Node *head CACHE_ALIGNED;
    Node *tail CACHE_ALIGNED;
    hazard_pointer_manager<Node> hazardManager CACHE_ALIGNED;
};


/* 
* Nonblocking bounded queue implementation
* If SPSC(Single-Producer-Single-Consumer) is set to true,
* a specialized queue based on ring buffer will be used as storage
* which typically has much better performance than MPMC(Multi-Producer-Multi-Consumer). 
*
* TODO: implement MPMC ring buffer as in https://github.com/LMAX-Exchange/disruptor
*/
template<typename T, bool SPSC = false>
class NonblockingBoundedQueue{
public:
    NonblockingBoundedQueue(const int size = DEFAULT_QUEUE_CAPACITY);
    
    // push one item into the queue, return false if full.
    bool push(const T& item);

    // pop at most n elements without blocking
    int pop(std::vector<T>& container, int n);

    // pop one element
    // return false immediately without blocking if there are no elements available
    bool pop(T& item);

    // return the approximate size of the queue
    int size();
};

template<typename T>
class NonblockingBoundedQueue<T, true> {
public:
    NonblockingBoundedQueue(const int size = DEFAULT_QUEUE_CAPACITY): capacity(size), queue(size) {}

    bool push(const T& item) {
        return queue.push(item);
    }

    // pop at most n elements without blocking
    int pop(std::vector<T>& container, int n) {
        T item;
        int popped = 0;
        while (n > 0 && pop(item)) {
            container.push_back(item);
            --n;
            popped++;
        }
        return popped;
    }

    // pop one element
    // return false immediately without blocking if there are no elements available
    bool pop(T& item) {
        return queue.pop(item);
    }

    // return the approximate size of the queue
    int size() {
        return queue.size();
    }
    int cap() {
        return capacity;
    }
private:
    const int capacity;
    SPSCLocklessBoundedQueue<T> queue;
};

template<typename T>
class NonblockingBoundedQueue<T, false> {
public:
    NonblockingBoundedQueue(const int size = DEFAULT_QUEUE_CAPACITY): capacity(size), queue()  {}

    bool push(const T& item) {
        if (counter.load() >= capacity)
            return false;
        counter.fetch_add(1);
        queue.push(item);
        return true;
    }

    // pop at most n elements without blocking
    int pop(std::vector<T>& container, int n) {
        T item;
        int popped = 0;
        while (n > 0 && pop(item)) {
            container.push_back(item);
            --n;
            popped++;
        }
        return popped;
    }

    // pop one element
    // return false immediately without blocking if there are no elements available
    bool pop(T& item) {
        bool success = queue.pop(item);
        if (success) {
            counter.fetch_add(-1);
            return true;
        }
        return false;
    }

    // return the approximate size of the queue
    int size() {
        return counter.load();
    }

    int cap() {
        return capacity;
    }
private:
    const int capacity;
    LocklessBoundlessQueue<T> queue;
    std::atomic_int counter;
};

template<typename T>
class NonblockingBoundlessQueue{
public:
    NonblockingBoundlessQueue(){}
    bool push(const T& item) {
        queue.push(item);
        counter.fetch_add(1);
        return true;
    }

    // pop at most n elements without blocking, return # of elements popped.
    int pop(std::vector<T>& container, int n) {
        T item;
        int popped = 0;
        while (n > 0 && pop(item)) {
            container.push_back(item);
            --n;
            ++popped;
        }
        return popped;
    }

    // pop one element
    // return false immediately without blocking if there are no elements available
    bool pop(T& item) {
        bool success = queue.pop(item);
        if (success)
            counter.fetch_add(-1);
        return success;
    }

    // return the approximate size of the queue
    int size() {
        return counter.load();
    }

private:
    LocklessBoundlessQueue<T> queue;
    std::atomic_int counter;
};


/* 
* A blocking implementation of a bounded queue.
* This queue builds upon NonblockingBoundedQueue and blocks
* when queue is either full or empty.
* See NonblockingBoundedQueue for details.
*/
template<typename T, bool SPSC = false>
class BlockingBoundedQueue: public NonblockingBoundedQueue<T, SPSC>{
public:
    BlockingBoundedQueue(const int size = DEFAULT_QUEUE_CAPACITY): NonblockingBoundedQueue<T, SPSC>(size) {}
    // push item into the queue, block if full.
    void blockingPush(const T& item) {
        LockGuard<Mutex> lk(&mutex1);
        while (NonblockingBoundedQueue<T, SPSC>::push(item) == false) {
            cvNotFull.wait(mutex1);
        }
        cvNotEmpty.notify();
    }
        
    // push item into the queue, block if full.
    void blockingPop(T & item) {
        LockGuard<Mutex> lk(&mutex1);
        while (NonblockingBoundedQueue<T, SPSC>::pop(item) == false) {
            cvNotEmpty.wait(mutex1);
        }
        cvNotFull.notify();
    }

    // pop exactly n elements into container, wait if necessary.
    void blockingPop(std::vector<T> & container, int n) {
        LockGuard<Mutex> lk(&mutex2);
        while (n > 0) {
            int popped = NonblockingBoundedQueue<T>::pop(container, n);
            n -= popped;
            if (popped == 0) {
                cvNotEmpty.wait(mutex2);
            } else {
                cvNotFull.notifyAll();
            }
        }
    }
private:
    Mutex mutex1;
    ConditionalVariable cvNotFull;
    Mutex mutex2;
    ConditionalVariable cvNotEmpty;
};


template<typename T>
class BlockingBoundlessQueue: public NonblockingBoundlessQueue<T>{
public:
    BlockingBoundlessQueue(){}
    bool push(const T& item) {
        NonblockingBoundlessQueue<T>::push(item);
        cvNotEmpty.notifyAll();
        return true;
    }

    void blockingPop(T & item) {
        // fast path for non-empty case
        if (NonblockingBoundlessQueue<T>::pop(item)) {
            return;
        }
        LockGuard<Mutex> lk(&mutex);
        while (NonblockingBoundlessQueue<T>::pop(item) == false) {
            cvNotEmpty.wait(mutex);
        }
    }

    // pop up to n elements into container, but wait if the queue is empty.
    void blockingPop(std::vector<T> & container, int n) {
        // fast path if there are at least n elements available
        int popped = NonblockingBoundlessQueue<T>::pop(container, n);
        if (popped > 0) {
            return;
        }

        n -= popped;
        LockGuard<Mutex> lk(&mutex);
        while (n > 0) {
        	cvNotEmpty.wait(mutex);
            popped = NonblockingBoundlessQueue<T>::pop(container, n);
            n -= popped;
            if (popped > 0)
            	return;
        }
    }
private:
    Mutex mutex;
    ConditionalVariable cvNotEmpty;
};


class ReaderVersion {
public:
    ReaderVersion() : version(0) {}
    inline void depart() {
        version.decrement();
    }
    inline void arrive() {
        version.increment();
    }
    inline bool isEmpty() {
        return version.get() == 0;
    }
private:
    DistributedCounter<> version;
};

template<typename Key,
         typename T,
         typename HashPolicy = power2_hash_policy,
         typename Hasher = murmur_hasher<Key>,
         typename KeyEqual = std::equal_to<Key>>
class IrremovableLocklessFlatHashmap {
public:
    typedef Key key_type;
	typedef T mapped_type;
    typedef Hasher key_hasher;
    typedef KeyEqual key_equal;
	typedef size_t size_type;
    typedef HashPolicy hash_policy;
    IrremovableLocklessFlatHashmap()
     :versionIdx(0), leftRight(0) {}
    inline bool find(const key_type & key, mapped_type & recv) {
        int curVersionIdx = versionIdx.load();
        versions[curVersionIdx].arrive();
        bool found = maps[leftRight.load()].find(key, recv);
        versions[curVersionIdx].depart();
        return found;
    }

    void getEntries(std::vector<std::pair<key_type, mapped_type>> & vec) {
        int curVersionIdx = versionIdx.load();
        int curLeftRight = 0;
        versions[curVersionIdx].arrive();
        curLeftRight = leftRight.load();
        auto end = maps[curLeftRight].end();
        for(auto it = maps[curLeftRight].begin(); it != end; it++) {
            vec.push_back(std::make_pair(it.key(), it.value()));
        }
        versions[curVersionIdx].depart();
    }

    bool insert(const key_type & key, const mapped_type & value) {
        LockGuard<Mutex> guard(&writerMtx);
        int curLeftRight = leftRight.load();
        int curVersionIdx = versionIdx.load();
        bool inserted1 = maps[!curLeftRight].insert(key, value);
        leftRight.store(!curLeftRight);
        while(versions[!curVersionIdx].isEmpty() == false);
        versionIdx.store(!curVersionIdx);
        while(versions[curVersionIdx].isEmpty() == false);
        bool inserted2 = maps[curLeftRight].insert(key, value);
        assert(inserted1 == inserted2);
        return inserted1 && inserted2;
    }

    bool upsert(const key_type &key, const mapped_type & value) {
        LockGuard<Mutex> guard(&writerMtx);
        int curLeftRight = leftRight.load();
        int curVersionIdx = versionIdx.load();

        bool upserted1 = maps[!curLeftRight].upsert(key, value);

        leftRight.store(!curLeftRight);
        while(versions[!curVersionIdx].isEmpty() == false);
        versionIdx.store(!curVersionIdx);
        while(versions[curVersionIdx].isEmpty() == false);

        maps[curLeftRight].upsert(key, value);
        return upserted1;
    }

    inline void clear() {
        LockGuard<Mutex> guard(&writerMtx);
        int curLeftRight = leftRight.load();
        int curVersionIdx = versionIdx.load();
        maps[!curLeftRight].clear();
        leftRight.store(!curLeftRight);
        while(versions[!curVersionIdx].isEmpty() == false);
        versionIdx.store(!curVersionIdx);
        while(versions[curVersionIdx].isEmpty() == false);
        maps[curLeftRight].clear();
    }

    inline size_t size() {
        int curVersionIdx = versionIdx.load();
        versions[curVersionIdx].arrive();
        size_t sz = maps[leftRight.load()].size();
        versions[curVersionIdx].depart();
        return sz;
    }

private:
    Mutex writerMtx;
    IrremovableFlatHashmap<Key, T, HashPolicy, Hasher, KeyEqual> maps[2];
    std::atomic<int> versionIdx;
    std::atomic<int> leftRight;
    ReaderVersion versions[2];
};


template<typename Key,
         typename T,
         typename HashPolicy = power2_hash_policy,
         typename Hasher = murmur_hasher<Key>,
         typename KeyEqual = std::equal_to<Key>>
class LocklessFlatHashmap {
public:
    typedef Key key_type;
	typedef T mapped_type;
    typedef Hasher key_hasher;
    typedef KeyEqual key_equal;
	typedef size_t size_type;
    typedef HashPolicy hash_policy;
    LocklessFlatHashmap()
     :versionIdx(0), leftRight(0) {}
    inline bool find(const key_type & key, mapped_type & recv) {
        int curVersionIdx = versionIdx.load();
        versions[curVersionIdx].arrive();
        bool found = maps[leftRight.load()].find(key, recv);
        versions[curVersionIdx].depart();
        return found;
    }

    void getEntries(std::vector<std::pair<key_type, mapped_type>> & vec) {
        int curVersionIdx = versionIdx.load();
        int curLeftRight = 0;
        versions[curVersionIdx].arrive();
        curLeftRight = leftRight.load();
        auto end = maps[curLeftRight].end();
        for(auto it = maps[curLeftRight].begin(); it != end; it++) {
            vec.push_back(std::make_pair(it.key(), it.value()));
        }
        versions[curVersionIdx].depart();
    }

    bool insert(const key_type & key, const mapped_type & value) {
        bool inserted1 = false, inserted2 = false;
        LockGuard<Mutex> g(&writerMtx);

        int curLeftRight = leftRight.load();
        int curVersionIdx = versionIdx.load();
        inserted1 = maps[!curLeftRight].insert(key, value);
        leftRight.store(!curLeftRight);
        while(versions[!curVersionIdx].isEmpty() == false);
        versionIdx.store(!curVersionIdx);
        while(versions[curVersionIdx].isEmpty() == false);

        try {
            inserted2 = maps[curLeftRight].insert(key, value);
        } catch(...) {
            int curLeftRight = leftRight.load();
            int curVersionIdx = versionIdx.load();
            maps[!curLeftRight].erase(key);
            leftRight.store(!curLeftRight);
            while(versions[!curVersionIdx].isEmpty() == false);
            versionIdx.store(!curVersionIdx);
            while(versions[curVersionIdx].isEmpty() == false);
            maps[curLeftRight].erase(key);
            inserted1 = inserted2 = false;
			throw;
        }
        assert(inserted1 == inserted2);
        return inserted1;
    }

    bool upsert(const key_type &key, const mapped_type & value) {
        LockGuard<Mutex> g(&writerMtx);
        int curLeftRight = leftRight.load();
        int curVersionIdx = versionIdx.load();

        bool upserted1 = maps[!curLeftRight].upsert(key, value);

        leftRight.store(!curLeftRight);
        while(versions[!curVersionIdx].isEmpty() == false);
        versionIdx.store(!curVersionIdx);
        while(versions[curVersionIdx].isEmpty() == false);

        maps[curLeftRight].upsert(key, value);
        return upserted1;
    }

    bool erase(const key_type & key) {
        LockGuard<Mutex> g(&writerMtx);
        int curLeftRight = leftRight.load();
        int curVersionIdx = versionIdx.load();
        bool erased1 = maps[!curLeftRight].erase(key);
        leftRight.store(!curLeftRight);
        while(versions[!curVersionIdx].isEmpty() == false);
        versionIdx.store(!curVersionIdx);
        while(versions[curVersionIdx].isEmpty() == false);
        maps[curLeftRight].erase(key);
        return erased1;
    }

    inline void clear() {
        LockGuard<Mutex> g(&writerMtx);
        int curLeftRight = leftRight.load();
        int curVersionIdx = versionIdx.load();
        maps[!curLeftRight].clear();
        leftRight.store(!curLeftRight);
        while(versions[!curVersionIdx].isEmpty() == false);
        versionIdx.store(!curVersionIdx);
        while(versions[curVersionIdx].isEmpty() == false);
        maps[curLeftRight].clear();
    }

    inline size_t size() {
        int curVersionIdx = versionIdx.load();
        versions[curVersionIdx].arrive();
        size_t sz = maps[leftRight.load()].size();
        versions[curVersionIdx].depart();
        return sz;
    }
private:
    Mutex writerMtx;
    FlatHashmap<Key, T, HashPolicy, Hasher, KeyEqual> maps[2];
    std::atomic<int> versionIdx;
    std::atomic<int> leftRight;
    ReaderVersion versions[2];
};

#endif
