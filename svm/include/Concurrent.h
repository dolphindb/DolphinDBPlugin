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

#ifdef WINDOWS
	#include <winsock2.h>
    #include <windows.h>
    #define SleepMilliSecond Sleep
#else
    #include <pthread.h>
    #include <unistd.h>
	#include <semaphore.h>
    #define SleepMilliSecond sleep
#endif

class Thread;
class Runnable;
class CountDownLatch;
typedef SmartPointer<Thread> ThreadSP;
typedef SmartPointer<Runnable> RunnableSP;
typedef SmartPointer<CountDownLatch> CountDownLatchSP;

class Runnable{
public:
	Runnable();
	void start();
	virtual ~Runnable();
	bool isRunning(){return !idle_;}

protected:
	virtual void run()=0;

private:
	volatile bool idle_;
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

class RWLock{
public:
	RWLock();
	~RWLock();
	void acquireRead();
	void acquireWrite();
	void releaseRead();
	void releaseWrite();
	bool tryAcquireRead();
	bool tryAcqurieWrite();
private:
#ifdef WINDOWS
	SRWLOCK lock_;
#else
	pthread_rwlock_t lock_;
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
		if(acquireLock)
			res_->lock();
	}
	~LockGuard(){
		res_->unlock();
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
		if(acquireLock){
			if(exclusive_)
				res_->acquireWrite();
			else
				res_->acquireRead();
		}
	}
	~RWLockGuard(){
		if(exclusive_)
			res_->releaseWrite();
		else
			res_->releaseRead();
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
private:
	T* res_;
	bool exclusive_;
	bool locked_;
};

class CountDownLatch{
public:
	CountDownLatch(int count) : count_(count){}
	void wait();
	void countDown();
	int getCount() const;
private:
	mutable Mutex mutex_;
	ConditionalVariable condition_;
	int count_;
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

	int size(){
		LockGuard<Mutex> guard(&mutex_);
		return items_.size();
	}

	void clear(){
		LockGuard<Mutex> guard(&mutex_);
		while(!items_.empty())
			items_.pop();
	}

private:
	std::queue<T> items_;
	Mutex mutex_;
	ConditionalVariable empty_;
};

class Thread{
public:
	Thread(const RunnableSP& run);
	~Thread();
	void start();
	void join();
	bool isRunning(){return run_.isNull()?false:run_->isRunning();}
	static void sleep(int milliSeconds){
		SleepMilliSecond(milliSeconds);
	}

private:
	static void* startFunc(void* data){
		((Thread*)data)->run_->start();
		return data;
	}

	RunnableSP run_;
#ifdef WINDOWS
	HANDLE thread_;
	DWORD threadId_;
#else
	pthread_t thread_;
	pthread_attr_t attr_;
#endif
};

#endif /* CONCURRENT_H_ */
