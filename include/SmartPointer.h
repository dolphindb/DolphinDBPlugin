/*
 * SmartPointer.h
 *
 *  Created on: Jun 24, 2012
 *      Author: dzhou
 */

#ifndef SMARTPOINTER_H_
#define SMARTPOINTER_H_

#include <atomic>

class Counter {
public:
	Counter(void* p): p_(p), count_(0){}
	int addRef(){ return atomic_fetch_add(&count_,1)+1;} //atomic operation
	int release(){return atomic_fetch_sub(&count_,1)-1;} //atomic operation
	int getCount() const {return count_.load();}
	void* p_;

private:
	std::atomic<int> count_;
};


template <class T>
class SmartPointer {
public:
	SmartPointer(T* p=0): counterP_(new Counter(p)){
		counterP_->addRef();
	}

	SmartPointer(const SmartPointer<T>& sp){
		counterP_=sp.counterP_;
		counterP_->addRef();
	}

	template <class U>
	SmartPointer(const SmartPointer<U>& sp){
		counterP_=sp.counterP_;
		counterP_->addRef();
	}

	T& operator *() const{
		return *((T*)counterP_->p_);
	}

	T* operator ->() const{
		return (T*)counterP_->p_;
	}

	T& operator =(const SmartPointer<T>& sp){
		if(this==&sp)
			return *((T*)counterP_->p_);

		Counter* tmp = sp.counterP_;
		if(counterP_ == tmp)
			return *((T*)tmp->p_);
		tmp->addRef();

		//TODO: the below operation is not thread-safe. But it is safe if there is only one writer and multiple readers.
		Counter* oldCounter = counterP_;
		counterP_= tmp;

		if(oldCounter->release()==0){
			delete (T*)oldCounter->p_;
			delete oldCounter;
		}
		return *((T*)tmp->p_);
	}

	bool operator ==(const SmartPointer<T>& sp) const{
		return counterP_ == sp.counterP_;
	}

	bool operator !=(const SmartPointer<T>& sp) const{
		return counterP_ != sp.counterP_;
	}

	void clear(){
		Counter* tmp = new Counter(0);
		tmp->addRef();

		//TODO: the below operation is not thread-safe. But it is safe if there is only one writer and multiple readers.
		Counter* oldCounter = counterP_;
		counterP_= tmp;

		if(oldCounter->release()==0){
			delete (T*)oldCounter->p_;
			delete oldCounter;
		}
	}

	bool isNull() const{
		return counterP_->p_ == 0;
	}

	int count() const{
		return counterP_->getCount();
	}

	T* get() const{
		return (T*)counterP_->p_;
	}

	~SmartPointer(){
		if(counterP_->release()==0){
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
