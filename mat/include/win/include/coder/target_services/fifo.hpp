/* Copyright 2015 The MathWorks, Inc. */

#ifndef coder_tgtsvc_detail_fifo_hpp
#define coder_tgtsvc_detail_fifo_hpp

namespace coder { namespace tgtsvc { namespace detail {

template <typename value_type>
struct circular_iterator : public std::iterator<std::random_access_iterator_tag, value_type>
{
   
    explicit circular_iterator(value_type *it, value_type *begin, value_type *end) : it_(it), begin_(begin), end_(end) {}

    bool operator==(const circular_iterator<value_type> &rhs) const { return rhs.it_ == it_; }
    bool operator!=(const circular_iterator<value_type> &rhs) const { return !(rhs == *this); }

    circular_iterator<value_type> &operator+=(int dist) { advance(dist); return *this; }
    circular_iterator<value_type> &operator-=(int dist) { advance(-dist); return *this; }
    circular_iterator<value_type> &operator++() { advance(); return *this; }
    circular_iterator<value_type> &operator--() { retreat(); return *this; }
    circular_iterator<value_type>  operator++(int) { auto temp(*this); advance(); return temp; }
    circular_iterator<value_type>  operator--(int) { auto temp(*this); retreat(); return temp; }
    circular_iterator<value_type> operator+(ptrdiff_t dist) const { auto temp(*this); temp.advance(dist); return temp; }
    circular_iterator<value_type> operator-(ptrdiff_t dist) const { auto temp(*this); temp.advance(-dist); return temp; }

    ptrdiff_t operator-(const circular_iterator<value_type> &rhs) const {
        ptrdiff_t r = it_ - rhs.it_;
        if (r < 0) r += (end_ - begin_);
        return r;
    }

    value_type &operator*() { return *it_; }
    const value_type &operator*() const { return *it_; }

    value_type &operator[](size_t idx) {
        value_type *r = it_ + idx;
        ptrdiff_t wrap = end_ - begin_;
        while (r >= end_) { r -= wrap; }
        return *r;
    }

private:
    value_type *it_;
    value_type *begin_;
    value_type *end_;

    void advance() {
        value_type *out = ++it_;
        if (out == end_) out = begin_;
        it_ = out;
    }

    void advance(ptrdiff_t dist)
    {
        value_type *out = it_ + dist;
        ptrdiff_t wrap = end_ - begin_;
        while (out >= end_) { out -= wrap; }
        while (out < begin_) { out += wrap; }
        it_ = out;
    }

    void retreat() {
        value_type *out = --it_;
        if (out < begin_) out = end_ - 1;
        it_ = out;
    }
};

template<typename T, size_t N>
class fifo
{
public:

    typedef T value_type;

    struct carray {
        T *addr_;
        size_t size_;
    };

    fifo() : contents_(buff_), space_(buff_) {}

    bool empty() const { return space_ == contents_; }
    bool full() const { return contents_size() == N - 1; }
    void clear() { contents_ = buff_; space_ = buff_; }

    T front() const { return *contents_; }
    void pop() { contents_remove(1); }
    void push(const T &val) {
        *space_ = val;
        contents_add(1);
    }

    T &operator[](ptrdiff_t idx) { return contents_at(idx); }

    size_t contents_size() const {
        return contents_wrapped() ?  N + space_ - contents_ : space_ - contents_;
    }

    T &contents_at(size_t idx) {
        T *p = increment(const_cast<T*>(contents_), idx);
        return *p;
    }

    carray contents_carray() {
        carray r;
        r.addr_ = const_cast<T*>(contents_);
        r.size_ = contents_wrapped() ? buff_end() - contents_ : space_ - contents_;
        return r;
    }

    void contents_remove(size_t count) {
        contents_ = increment(const_cast<T*>(contents_), count);
    }

    void contents_add(size_t count) {
        space_ = increment(const_cast<T*>(space_), count);
    }

    size_t space_size() const { return N - contents_size() - 1; }

    T &space_at(size_t idx) {
        T *p = increment(const_cast<T*>(space_), idx);
        return *p;
    }

    carray space_carray() {
        carray r;
        r.addr_ = const_cast<T*>(space_);
        if (contents_wrapped()) {
            r.size_ = contents_ - space_ - 1;
        }
        else {
            r.size_ = buff_end() - space_;

            if (contents_ == buff_) --r.size_;
        }
        return r;
    }

    circular_iterator<value_type> contents_begin() { return circular_iterator<value_type>(const_cast<T*>(contents_), buff_, buff_ + N);	}
    circular_iterator<value_type> contents_end()   { return circular_iterator<value_type>(const_cast<T*>(space_), buff_, buff_ + N); }

    circular_iterator<value_type> space_begin() { return circular_iterator<value_type>(const_cast<T*>(space_), buff_, buff_ + N); }
    circular_iterator<value_type> space_end()   { return --contents_begin(); }

private:
    volatile T *contents_;
    volatile T *space_;
    T buff_[N];

    T *increment(T *p, size_t i=1) {
        p += i;
        p = p >= buff_end() ? p - N : p;
        return p;
    }

    T *buff_end() { return buff_ + N; }
    bool contents_wrapped() const { return space_ < contents_; }

};

}}}

#endif
