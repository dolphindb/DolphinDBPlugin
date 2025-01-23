/*
 * Concurrent.h
 *
 *  Created on: Jan 26, 2013
 *      Author: dzhou
 */

#ifndef CONCURRENT_H_
#define CONCURRENT_H_

#include "SmartPointer.h"
#include <vector>
#include <queue>
#include <cassert>

#ifdef WINDOWS
	#include <winsock2.h>
    #include <windows.h>
#else
    #include <pthread.h>
    #include <unistd.h>
    #include <sys/syscall.h>
	#include <semaphore.h>
#endif

class Thread;
class Runnable;
class CountDownLatch;
class Callback;
typedef SmartPointer<Thread> ThreadSP;
typedef SmartPointer<Runnable> RunnableSP;
typedef SmartPointer<Callback> CallbackSP;
typedef SmartPointer<CountDownLatch> CountDownLatchSP;

class Runnable{
public:
	Runnable();
	void start();
	virtual ~Runnable();
	bool isRunning();
	bool isStarted();
	bool isComplete();

protected:
	virtual void run()=0;

private:
	std::atomic<char> status_;
};

class Callback {
public:
	virtual ~Callback(){}
	virtual void execute() = 0;
};

class Mutex{
public:
	Mutex();
	~Mutex();
	void lock();
	bool tryLock();
	void unlock();

private:

#ifdef WINDOWS
	CRITICAL_SECTION mutex_;
#else
	pthread_mutexattr_t attr_;
	pthread_mutex_t mutex_;
#endif
	friend class ConditionalVariable;
};

// Reference: https://en.cppreference.com/w/cpp/thread/recursive_timed_mutex
class TimedMutex {
public:
	TimedMutex();
	~TimedMutex();
	void lock();
	bool tryLock();
	bool tryLockFor(int milli);
	void unlock();

private:
#ifdef WINDOWS
	HANDLE  mutex_;
#else
	pthread_mutexattr_t attr_;
	pthread_mutex_t mutex_;
#endif
	// TimedMutex CANNOT be used with condition variable
};

class RWLock{
public:
	RWLock(bool preferWrite=false);
	~RWLock();
	void acquireRead();
	void acquireWrite();
	void releaseRead();
	void releaseWrite();
	bool tryAcquireRead();
	bool tryAcquireWrite();
private:
#ifdef WINDOWS
	SRWLOCK lock_;
#else
	pthread_rwlock_t lock_;
    pthread_rwlockattr_t attr_;
#endif
};

class RWSpinLock{
public:
	RWSpinLock(){};
	~RWSpinLock(){};
	void acquireRead(){}
	void acquireWrite(){}
	void releaseRead(){}
	void releaseWrite(){}
private:

};

class ConditionalVariable{
public:
	ConditionalVariable();
	~ConditionalVariable();
	void wait(Mutex& mutex);
	bool wait(Mutex& mutex, int milliSeconds);
	void notify();
	void notifyAll();

private:
#ifdef WINDOWS
	CONDITION_VARIABLE conditionalVariable_;
#else
	pthread_cond_t conditionalVariable_;
#endif
};


template<class T>
class LockGuard{
public:
	LockGuard(T* res, bool acquireLock = true):res_(res){
		if(acquireLock && res_ != NULL) {
            res_->lock();
		}
	}

	~LockGuard() { unlock(); }

	void unlock() {
		if (res_ != NULL) {
			res_->unlock();
			res_ = NULL;
		}
	}

	void relock(T* res) {
		if (res) {
			res_ = res;
			res->lock();
		}
	}

	LockGuard(const LockGuard &) = delete;
	LockGuard& operator=(const LockGuard &) = delete;

	LockGuard& operator=(LockGuard &&other) noexcept {
		this->unlock();

		using std::swap;
		swap(res_, other.res_);

		return *this;
	}

private:
	T* res_;
};

template<class T>
class TryLockGuard{
public:
	TryLockGuard(T* res, bool acquireLock = true):res_(res), locked_(false){
		if(acquireLock)
			locked_ = res_->tryLock();
	}
	~TryLockGuard(){
		if(locked_)
			res_->unlock();
	}
	bool isLocked(){
		return locked_;
	}
private:
	T* res_;
	bool locked_;
};

template<class T>
class RWLockGuard{
public:
	RWLockGuard(T* res, bool exclusive, bool acquireLock = true):res_(res), exclusive_(exclusive){
		if(res != NULL && acquireLock){
			if(exclusive_)
				res_->acquireWrite();
			else
				res_->acquireRead();
		}
	}

	~RWLockGuard() { destroy(); }

	void upgrade(){
		if(res_ != NULL){
			if(exclusive_)
				return;
			else{
				res_->releaseRead();
				res_->acquireWrite();
				exclusive_ = true;
			}
		}
	}

	void unlock() {
		if (res_ != NULL) {
			if (exclusive_) {
				res_->releaseWrite();
			} else {
				res_->releaseRead();
			}
			res_ = NULL;
		}
	}

	void relock(T* res, bool exclusive) {
		res_ = res;
		exclusive_ = exclusive;
		if (res != NULL) {
			if(exclusive_) {
				res_->acquireWrite();
			}
			else {
				res_->acquireRead();
			}
		}
	}

	RWLockGuard(const RWLockGuard &) = delete;
	RWLockGuard& operator=(const RWLockGuard &) = delete;

	RWLockGuard& operator=(RWLockGuard &&other) noexcept {
		this->destroy();

		using std::swap;
		swap(res_, other.res_);
		swap(exclusive_, other.exclusive_);

		return *this;
	}

private:
	void destroy() {
		if (res_ != NULL) {
			if (exclusive_) {
				res_->releaseWrite();
			} else {
				res_->releaseRead();
			}
			res_ = NULL;
		}
	}

private:
	T* res_;
	bool exclusive_;
};

template<class T>
class TryRWLockGuard{
public:
	TryRWLockGuard(T* res, bool exclusive, bool acquireLock = true):res_(res), exclusive_(exclusive), locked_(false){
		if(acquireLock){
			if(exclusive_)
				locked_ = res_->tryAcquireWrite();
			else
				locked_ = res_->tryAcquireRead();
		}
	}
	~TryRWLockGuard(){
		if(locked_){
			if(exclusive_)
				res_->releaseWrite();
			else
				res_->releaseRead();
		}
	}

	bool isLocked() {
		return locked_;
	}

private:
	T* res_;
	bool exclusive_;
	bool locked_;
};

class CountDownLatch{
public:
	CountDownLatch(int count) : count_(count){}
	void wait();
	bool wait(int milliseconds);
	void countDown();
	void countDown(int n);
	int getCount() const;
	bool resetCount(int count);
	void clear();
	void setCallback(const CallbackSP& callback);

private:
	mutable Mutex mutex_;
	ConditionalVariable condition_;
	CallbackSP callback_;
	int count_;
};

template<class T>
class Future {
public:
	Future(): latch_(1) {}
	//Wait till the result is ready or the specified milliseconds timeout. Return whether the result is ready.
	bool wait(int milliseconds) { return latch_.wait(milliseconds); }
	//Wait till the result is ready.
	void wait() { latch_.wait(); }
	//Set the result. This function should be called exactly once.
	void set(const T & val) {
		val_ = val;
		latch_.countDown();
	}
	//Get the value as promised. Blocked if the result is not ready.
	T get() {
		latch_.wait();
		return val_;
	}
private:
	CountDownLatch latch_;
	T val_;
};

class Semaphore{
public:
	Semaphore(int resources);
	~Semaphore();
	void acquire();
	bool tryAcquire();
	void release();

private:
#ifdef WINDOWS
	HANDLE sem_;
#else
	sem_t sem_;
#endif
};

class ConditionalNotifier {
public:
	ConditionalNotifier() {}
	~ConditionalNotifier() {}
	void wait() { LockGuard<Mutex> guard(&mtx_); cv_.wait(mtx_); }
	bool wait(int milliSeconds) { LockGuard<Mutex> guard(&mtx_); return cv_.wait(mtx_, milliSeconds); }
	void notify() { cv_.notify(); }
	void notifyAll() { cv_.notifyAll(); }
private:
	ConditionalVariable cv_;
	Mutex mtx_;
};

template<class T>
class BoundedBlockingQueue{
public:
	BoundedBlockingQueue(size_t maxItems) : capacity_(maxItems), size_(0), head_(0), tail_(0){
		buf_ = new T[maxItems];
	}

	~BoundedBlockingQueue(){
		delete[] buf_;
	}

	void push(const T& item){
		lock_.lock();
		while(size_ >= capacity_)
			full_.wait(lock_);
		buf_[tail_] = item;
		tail_ = (tail_+1) % capacity_;
		++size_;

		if(size_ == 1)
			empty_.notifyAll();
		lock_.unlock();
	}

	void pop(T& item){
		lock_.lock();
		while(size_ == 0)
			empty_.wait(lock_);
		item = buf_[head_];
		buf_[head_] = T();
		head_ = (head_+1) % capacity_;
		--size_;

		if(size_ == capacity_ -1)
			full_.notifyAll();
		lock_.unlock();
	}

private:
	T* buf_;
	size_t capacity_;
	size_t size_;
	size_t head_;
	size_t tail_;
	Mutex lock_;
	ConditionalVariable full_;
	ConditionalVariable empty_;
};

template<class T>
class SynchronizedQueue{
public:
	SynchronizedQueue(){}
	void push(const T& item){
		LockGuard<Mutex> guard(&mutex_);
		items_.push(item);
		if(items_.size() == 1)
			empty_.notifyAll();
	}

	void pop(std::vector<T>& container, int n){
		LockGuard<Mutex> guard(&mutex_);
		int count = std::min((int)items_.size(), n);
		while(count>0){
			container.push_back(items_.front());
			items_.pop();
			--count;
		}
	}

	bool pop(T& item){
		LockGuard<Mutex> guard(&mutex_);
		if(items_.empty())
			return false;
		item = items_.front();
		items_.pop();
		return true;
	}

	bool peek(T& item){
		LockGuard<Mutex> guard(&mutex_);
		if(items_.empty())
			return false;
		item = items_.front();
		return true;
	}

	void blockingPop(T& item){
		LockGuard<Mutex> guard(&mutex_);
		while(items_.empty())
			empty_.wait(mutex_);
		item = items_.front();
		items_.pop();
	}

	bool blockingPop(T& item, int milliSeconds){
		LockGuard<Mutex> guard(&mutex_);
		while(items_.empty()){
			if(!empty_.wait(mutex_, milliSeconds))
				return false;
		}
		item = items_.front();
		items_.pop();
		return true;
	}

	void blockingPop(std::vector<T>& container, int n){
		LockGuard<Mutex> guard(&mutex_);
		while(items_.empty())
			empty_.wait(mutex_);
		int count = std::min((int)items_.size(), n);
		while(count>0){
			do {
				try {
					container.push_back(items_.front());
					break;
				} catch (...) {
					sleep(500);
				}
			} while (true);
			items_.pop();
			--count;
		}
	}

	void blockingPop(std::vector<T>& container, int n, int milliSeconds){
		LockGuard<Mutex> guard(&mutex_);
		while(items_.empty()) {
			if(!empty_.wait(mutex_, milliSeconds))
				return;
		}

		int count = std::min((int)items_.size(), n);
		while(count > 0) {
			container.push_back(items_.front());
			items_.pop();
			--count;
		}
 	}

	int size(){
		LockGuard<Mutex> guard(&mutex_);
		return items_.size();
	}

	void clear(){
		LockGuard<Mutex> guard(&mutex_);
		while(!items_.empty())
			items_.pop();
	}

	template<class Y>
	void removeItem(Y func){
		LockGuard<Mutex> guard(&mutex_);
		if(items_.empty())
			return;
		std::queue<T> newItem;
		while(!items_.empty()){
			T item;
			do {
				try {
					item = items_.front();
					break;
				} catch (...) {
					sleep(500);
				}
			} while (true);
			items_.pop();

			bool remove = false;
			try {
				if (func(item)) {
					remove = true;
				}
			} catch (...) {
				// ignore
			}
			if (!remove) {
				do {
					try {
						newItem.push(item);
						break;
					} catch (...) {
						sleep(500);
					}
				} while (true);
			}
		}
		items_.swap(newItem);
	}

private:
	void sleep(int milliSeconds) {
		if (milliSeconds <= 0)
			return;
#ifdef WINDOWS
		::Sleep(milliSeconds);
#else
		usleep(1000 * milliSeconds);
#endif
	}

private:
	std::queue<T> items_;
	Mutex mutex_;
	ConditionalVariable empty_;
};

template<class T, class Sizer, class Mandatory>
class GenericBoundedQueue{
public:
	GenericBoundedQueue(long long capacity, Sizer sizeFunc, Mandatory manFunc) : capacity_(capacity), size_(0), sizeFunc_(sizeFunc), manFunc_(manFunc){}

	bool push(const T& item){
		LockGuard<Mutex> guard(&mutex_);
		long long curSize = sizeFunc_(item);
		if(curSize <= 0)
			return false;

		if(!manFunc_(item) && size_ >= capacity_)
			return false;
		items_.push(item);
		size_ += curSize;
		if(items_.size() == 1)
			empty_.notifyAll();
		return true;
	}

	void blockingPush(const T& item){
		LockGuard<Mutex> guard(&mutex_);
		long long curSize = sizeFunc_(item);
		if(curSize <= 0)
			return;

		while(!manFunc_(item) && size_ >= capacity_)
			full_.wait(mutex_);
		items_.push(item);
		size_ += curSize;
		if(items_.size() == 1)
			empty_.notifyAll();
	}

	bool blockingPush(const T& item, int milliseconds){
		LockGuard<Mutex> guard(&mutex_);
		long long curSize = sizeFunc_(item);
		if(curSize <= 0)
			return true;

		while(!manFunc_(item) && size_ >= capacity_) {
			if (!full_.wait(mutex_, milliseconds)) {
				return false;
			}
		}
		items_.push(item);
		size_ += curSize;
		if(items_.size() == 1)
			empty_.notifyAll();
		return true;
	}

	void pop(std::vector<T>& container, long long n){
		assert(n > 0);
		LockGuard<Mutex> guard(&mutex_);
		if(size_ == 0)
			return;
		bool full = size_ >= capacity_;

		//at least pop out one item even if the sizeFunc(item) > n
		container.push_back(items_.front());
		long long curSize = sizeFunc_(container.back());
		items_.pop();
		size_ -= curSize;
		n -= curSize;

		while(n > 0 && size_ > 0){
			curSize = sizeFunc_(items_.front());
			if(curSize > n)
				break;
			container.push_back(items_.front());
			items_.pop();
			size_ -= curSize;
			n -= curSize;
		}
		if(full && size_ < capacity_)
			full_.notifyAll();
	}

	bool pop(T& item){
		LockGuard<Mutex> guard(&mutex_);
		if(size_ == 0)
			return false;
		item = items_.front();
		items_.pop();
		long long curSize = sizeFunc_(item);
		bool full = size_ >= capacity_;
		size_ -= curSize;
		if(full && size_ < capacity_)
			full_.notifyAll();
		return true;
	}

	bool peek(T& item){
		LockGuard<Mutex> guard(&mutex_);
		if(size_ == 0)
			return false;
		item = items_.front();
		return true;
	}

	void blockingPop(T& item){
		LockGuard<Mutex> guard(&mutex_);
		while(size_ == 0)
			empty_.wait(mutex_);
		item = items_.front();
		items_.pop();
		long long curSize = sizeFunc_(item);
		bool full = size_ >= capacity_;
		size_ -= curSize;
		if(full && size_ < capacity_)
			full_.notifyAll();
	}

	bool blockingPop(T& item, int milliSeconds){
		LockGuard<Mutex> guard(&mutex_);
		while(size_ == 0){
			if(!empty_.wait(mutex_, milliSeconds))
				return false;
		}
		item = items_.front();
		items_.pop();
		long long curSize = sizeFunc_(item);
		bool full = size_ >= capacity_;
		size_ -= curSize;
		if(full && size_ < capacity_)
			full_.notifyAll();
		return true;
	}

	inline long long size() const {
		return size_;
	}

	inline long long capacity() const {
		return capacity_;
	}

	inline double occupancyLevel() const {
		return size_ / (double)capacity_;
	}

	int count(){
		LockGuard<Mutex> guard(&mutex_);
		return items_.size();
	}

	void clear(){
		LockGuard<Mutex> guard(&mutex_);
		while(!items_.empty())
			items_.pop();
		size_ = 0;
	}

	template<class Y>
	void removeItem(Y func){
		LockGuard<Mutex> guard(&mutex_);
		std::queue<T> newItem;
		if(items_.empty()){
			size_ = 0;
			return;
		}
		bool full = size_ >= capacity_;
		while(!items_.empty()){
			T item = items_.front();
			items_.pop();
			if(!func(item))
				newItem.push(item);
			else
				size_ -= sizeFunc_(item);
		}
		items_.swap(newItem);
		if(full && size_ < capacity_)
			full_.notifyAll();
	}

private:
	long long capacity_;
	long long size_;
	std::queue<T> items_;
	mutable Mutex mutex_;
	ConditionalVariable full_;
	ConditionalVariable empty_;
	Sizer sizeFunc_;
	Mandatory manFunc_;
};

class Thread{
public:
	Thread(const RunnableSP& run);
	~Thread();
	void start();
	void join();
	void detach();
	void cancel();
	bool isRunning(){return run_.isNull() ? false : run_->isRunning();}
	bool isComplete() {return run_.isNull()? false : run_->isComplete();}
	bool isStarted() {return run_.isNull()? false : run_->isStarted();}
	bool isCurrentThread();
	static void sleep(int milliSeconds);
	static int getID();
	static bool yield();

private:
	static void* startFunc(void* data);
	RunnableSP run_;
#ifdef WINDOWS
	HANDLE thread_;
	DWORD threadId_;
#else
	pthread_t thread_;
	pthread_attr_t attr_;
#endif
};

class MutexGroup{
public:
	MutexGroup(int size = 4099): lockGroup_(new Mutex[size]), size_(size){}
	~MutexGroup() { delete[] lockGroup_; }
	inline void lock(size_t h){ lockGroup_[h % size_].lock(); }
	inline bool tryLock(size_t h) { return lockGroup_[h % size_].tryLock(); }
	inline void unlock(size_t h) { lockGroup_[h % size_].unlock(); }
	inline int size() const { return size_;}

private:
	Mutex* lockGroup_;
	int size_;
};

class MutexGroupGuard {
public:
	MutexGroupGuard(MutexGroup * group, size_t h, bool acquireLock = true): h_(h), group_(group) {
		if (acquireLock)
			group_->lock(h_);
	}
	MutexGroupGuard():h_(0), group_(nullptr){}

	~MutexGroupGuard() { unlock(); }

	void unlock() {
		if (group_ != NULL) {
			group_->unlock(h_);
			group_ = NULL;
		}
	}

	void relock(MutexGroup * group){
		group_ = group;
		if(group_ != NULL){
			group_->lock(h_);
		}
	}

	MutexGroupGuard(const MutexGroupGuard &) = delete;
	MutexGroupGuard& operator=(const MutexGroupGuard &) = delete;
	MutexGroupGuard(MutexGroupGuard &&other) noexcept : h_(0), group_(nullptr) {
		using std::swap;
		swap(h_, other.h_);
		swap(group_, other.group_);
	}

	MutexGroupGuard& operator=(MutexGroupGuard &&other) noexcept {
		this->unlock();

		using std::swap;
		swap(h_, other.h_);
		swap(group_, other.group_);

		return *this;
	}

private:
	size_t h_;
	MutexGroup* group_;
};

#endif /* CONCURRENT_H_ */
