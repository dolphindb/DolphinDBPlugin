/* Copyright 2013 The MathWorks, Inc. */

#ifndef coder_tgtsvc_Aggregator_hpp
#define coder_tgtsvc_Aggregator_hpp

#include <utility>
#include <algorithm>

namespace coder { namespace tgtsvc {

template<size_t N>
class Aggregator
{
public:
	typedef std::pair<uint8_t *, size_t> ArrayRange;

	Aggregator() : in_(0), out_(0) {}

	bool empty() const { return (in_ == out_); }

	bool full() const {
		return (in_+1 == out_ || (out_ == 0 && in_ == N));
	}

	void clear() { in_ = out_ = 0; }

	size_t count() const {
		size_t r = in_ + N - out_;
		r = r<N ? r : r-N;
		return r;
	}

	size_t space() const { return N - count() - 1; }

	bool put(ArrayRange range) {
		if (space() < range.second) return false;

		size_t end = range.second < N-in_ ? range.second : N-in_;
		std::copy(range.first, range.first+end, &buff_[in_]);
		if (end == range.second) {
			size_t nin = in_ + end;
			in_ = nin<N ? nin : 0;
		} else {
		
			std::copy(range.first+end, range.first+range.second, buff_);
			in_ = range.second - end;
		}
		return true;
	}

	ArrayRange get() {
		ArrayRange r;
		r.first = buff_ + out_;
		if (out_ <= in_) {
			r.second = in_ - out_;
		} else {
			r.second = N - out_;
		}
		return r;
	}

	void release(size_t count) {
	
		size_t nout = out_ + count;
		out_ = nout<N ? nout : nout-N;
	}

private:
	size_t in_;       
	size_t out_;      
	uint8_t buff_[N];
};

}}

#endif
