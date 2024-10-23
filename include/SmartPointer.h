/*
 * SmartPointer.h
 *
 *  Created on: Jun 24, 2012
 *      Author: dzhou
 */

#ifndef SMARTPOINTER_H_
#define SMARTPOINTER_H_

#include <atomic>
#include <cassert>
#include <memory>
#include <type_traits>
#include <typeinfo>
#include <utility>

#include "Exceptions.h"

#if defined(__GNUC__) && __GNUC__ >= 4
#define LIKELY(x) (__builtin_expect((x), 1))
#define UNLIKELY(x) (__builtin_expect((x), 0))
#else
#define LIKELY(x) (x)
#define UNLIKELY(x) (x)
#endif

class Counter {
public:
	constexpr Counter(void* p) noexcept: p_(p), obj_(nullptr), count_(0){}
	// reference: https://github.com/llvm/llvm-project/blob/release/15.x/libcxx/include/__memory/shared_ptr.h#L105
	int addRef() noexcept{ return atomic_fetch_add_explicit(&count_,1,std::memory_order_relaxed)+1;} //atomic operation
	int release() noexcept{return atomic_fetch_sub_explicit(&count_,1,std::memory_order_acq_rel)-1;} //atomic operation
	int getCount() const noexcept{return count_.load();}
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
using UniquePointer = std::unique_ptr<T>;

namespace Detail {
template<class>
struct IsUnboundedArray : std::false_type {};
template<class T>
struct IsUnboundedArray<T[]> : std::true_type {};

template<class>
struct IsBoundedArray : std::false_type {};
template<class T, std::size_t N>
struct IsBoundedArray<T[N]> : std::true_type {};
}  // namespace Detail
 
template<class T, class... Args>
typename std::enable_if<!std::is_array<T>::value, UniquePointer<T>>::type makeUnique(Args&&... args) {
    return UniquePointer<T>(new T(std::forward<Args>(args)...));
}
 
template<class T>
typename std::enable_if<Detail::IsUnboundedArray<T>::value, UniquePointer<T>>::type makeUnique(std::size_t n) {
    return UniquePointer<T>(new typename std::remove_extent<T>::type[n]());
}
 
template<class T, class... Args>
typename std::enable_if<Detail::IsBoundedArray<T>::value>::type makeUnique(Args&&...) = delete;

template <class T>
class SmartPointer {
public:
	SmartPointer(UniquePointer<T> ptr) : SmartPointer(ptr.release()) {}

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

	Counter* getCounter() const noexcept{return counterP_;}

	SmartPointer(const SmartPointer& sp) noexcept{
		counterP_=sp.counterP_;
		if (UNLIKELY(counterP_ == nullptr)) return;
		counterP_->addRef();
	}

	template <class U>
	SmartPointer(const SmartPointer<U>& sp) noexcept{
		static_assert(std::is_convertible<U*, T*>::value || std::is_base_of<U, T>::value, "U must be implicitly convertible to T or T must be a subclass of U");
		counterP_=sp.counterP_;
		if (UNLIKELY(counterP_ == nullptr)) return;
		// multi-inheritance is not supported in SmartPointer
		assert(static_cast<T*>((U*)(counterP_->p_)) == (T*)(counterP_->p_));
		counterP_->addRef();
	}

	template <class U>
	SmartPointer(SmartPointer<U> &&sp) noexcept {
		static_assert(std::is_convertible<U *, T *>::value || std::is_base_of<U, T>::value, "U must be implicitly convertible to T or T must be a subclass of U");
		counterP_=sp.counterP_;
		sp.counterP_=nullptr;
		if (UNLIKELY(counterP_ == nullptr)) return;
		// multi-inheritance is not supported in SmartPointer
		assert(static_cast<T *>((U *)(counterP_->p_)) == (T *)(counterP_->p_));
	}

	T& operator *() const noexcept{
		if (UNLIKELY(counterP_ == nullptr)) return *((T*)nullptr);
		return *((T*)counterP_->p_);
	}

	T* operator ->() const noexcept{
		if (UNLIKELY(counterP_ == nullptr)) return nullptr;
		return (T*)counterP_->p_;
	}

	void swap(SmartPointer& sp) noexcept{std::swap(counterP_, sp.counterP_);}

	SmartPointer& operator =(SmartPointer sp) noexcept{
		// copy and swap idiom
		swap(sp);
		return *this;
	}

	bool operator ==(const SmartPointer<T>& sp) const noexcept{
		return counterP_ == sp.counterP_;
	}

	bool operator !=(const SmartPointer<T>& sp) const noexcept{
		return !(*this == sp);
	}

	void clear(){*this = SmartPointer();}

	bool isNull() const noexcept{
		return counterP_ == nullptr || counterP_->p_ == nullptr;
	}

	int count() const noexcept{
		if (counterP_ == nullptr) return 0;
		return counterP_->getCount();
	}

	T* get() const noexcept{
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

private:
	template<class U> friend class SmartPointer;
	Counter* counterP_;
};

#endif /* SMARTPOINTER_H_ */
