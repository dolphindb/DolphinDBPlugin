/*
 * SmartPointer.h
 *
 *  Created on: Jun 24, 2012
 *      Author: dzhou
 */

#ifndef SMARTPOINTER_H_
#define SMARTPOINTER_H_

#include <atomic>
#include <typeinfo>
#include <type_traits>
#include "Exceptions.h"
#include <assert.h>

#if defined(__GNUC__) && __GNUC__ >= 4
#define LIKELY(x) (__builtin_expect((x), 1))
#define UNLIKELY(x) (__builtin_expect((x), 0))
#else
#define LIKELY(x) (x)
#define UNLIKELY(x) (x)
#endif

class Counter {
public:
	Counter(void* p): p_(p), obj_(nullptr), count_(0){}
	// reference: https://github.com/llvm/llvm-project/blob/release/15.x/libcxx/include/__memory/shared_ptr.h#L105
	int addRef(){ return atomic_fetch_add_explicit(&count_,1,std::memory_order_relaxed)+1;} //atomic operation
	int release(){return atomic_fetch_sub_explicit(&count_,1,std::memory_order_acq_rel)-1;} //atomic operation
	int getCount() const {return count_.load();}
	void* p_;
	void* obj_;

private:
	std::atomic<int> count_;
};

class RefCountHelper {
public:
	virtual ~RefCountHelper() {}
	virtual void incRef(void* counter, void* pyObj) = 0;
	virtual void decRef(void* counter, void* pyObj) = 0;
	virtual void zeroHandler(void* counter) = 0;
public:
	static RefCountHelper* inst_;
};


template <class T>
class SmartPointer {
public:
	SmartPointer(T* p=0): counterP_(nullptr){
		if (UNLIKELY(p == nullptr)) return;
		counterP_ = new Counter(p);
		counterP_->addRef();
	}

	SmartPointer(T* p, Counter* counter): counterP_(counter){
		if (UNLIKELY(counterP_ == nullptr)) {
			counterP_ = new Counter(p);
		}
		counterP_->addRef();
	}

	Counter* getCounter() {
		if (UNLIKELY(counterP_ == nullptr)) {
			counterP_ = new Counter(nullptr);
			counterP_->addRef();
		}
		return counterP_;
	}

        Counter* getCounter() const {
                return counterP_;
	}

	SmartPointer(const SmartPointer<T>& sp){
		counterP_=sp.counterP_;
		if (UNLIKELY(counterP_ == nullptr)) return;
		counterP_->addRef();
	}

	template <class U>
	SmartPointer(const SmartPointer<U>& sp){
		static_assert(std::is_convertible<U*, T*>::value || std::is_base_of<U, T>::value, "U must be implicitly convertible to T or T must be a subclass of U");
		counterP_=sp.counterP_;
		if (UNLIKELY(counterP_ == nullptr)) return;
		// multi-inheritance is not supported in SmartPointer
		assert(static_cast<T*>((U*)(counterP_->p_)) == (T*)(counterP_->p_));
		counterP_->addRef();
	}

	T& operator *() const{
		if (UNLIKELY(counterP_ == nullptr)) return *((T*)nullptr);
		return *((T*)counterP_->p_);
	}

	T* operator ->() const{
		if (UNLIKELY(counterP_ == nullptr)) return nullptr;
		return (T*)counterP_->p_;
	}

	T& operator =(const SmartPointer<T>& sp){
		if(this==&sp)
			return *((T*)counterP_->p_);

		Counter* tmp = sp.counterP_;
		if (UNLIKELY(counterP_ == nullptr && tmp == nullptr))
			return *((T*)nullptr);
		if(counterP_ == tmp)
			return *((T*)tmp->p_);
		if (LIKELY(tmp != nullptr))
			tmp->addRef();

		Counter* oldCounter = counterP_;
		counterP_= tmp;

		if(LIKELY(oldCounter != nullptr) && oldCounter->release()==0){
			if (oldCounter->obj_) {
				RefCountHelper::inst_->zeroHandler((void*)oldCounter);
			}
			delete (T*)oldCounter->p_;
			delete oldCounter;
		}
		if (UNLIKELY(tmp == nullptr))
			return *((T*)nullptr);
		return *((T*)tmp->p_);
	}

	bool operator ==(const SmartPointer<T>& sp) const{
		return counterP_ == sp.counterP_;
	}

	bool operator !=(const SmartPointer<T>& sp) const{
		return counterP_ != sp.counterP_;
	}

	void clear(){
		Counter* oldCounter = counterP_;
		if(LIKELY(oldCounter != nullptr) && oldCounter->release()==0){
			if (oldCounter->obj_) {
                RefCountHelper::inst_->zeroHandler((void*)oldCounter);
            }
			delete (T*)oldCounter->p_;
			delete oldCounter;
		}
		counterP_ = nullptr;
	}

	bool isNull() const{
		return counterP_ == nullptr || counterP_->p_ == nullptr;
	}

	int count() const{
		if (counterP_ == nullptr) return 0;
		return counterP_->getCount();
	}

	T* get() const{
		if (UNLIKELY(counterP_ == nullptr)) return nullptr;
		return (T*)counterP_->p_;
	}

	template<typename Type>
	Type* getAs() const{
		Type* p = dynamic_cast<Type*>((T*)counterP_->p_);
		if (UNLIKELY(!p)){
			throw RuntimeException("cast from type<" + string(typeid(T).name()) + "> to type<" + string(typeid(Type).name()) + "> is not allowed");
		} 
		return p;
	}

	~SmartPointer(){
		if(LIKELY(counterP_ != nullptr) && counterP_->release()==0){
			if (counterP_->obj_) {
                RefCountHelper::inst_->zeroHandler((void*)counterP_);
            }
			delete static_cast<T*>(counterP_->p_);
			delete counterP_;
			counterP_=0;
		}
	}

    T& operator =(T* p){
        if(LIKELY(counterP_ != nullptr) && p == counterP_->p_)
            return *p;

        if(LIKELY(counterP_ != nullptr) && counterP_->release()==0) {
            delete static_cast<T *>(counterP_->p_);
            counterP_->p_ = p;
            counterP_->addRef();
        }
        else{
            counterP_ = new Counter(p);
            counterP_->addRef();
        }
        return *p;
    }

private:
	template<class U> friend class SmartPointer;
	Counter* counterP_;
};

#endif /* SMARTPOINTER_H_ */
