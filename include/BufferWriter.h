#pragma once

#include "Types.h"
#include "SysIO.h"
#include <cstddef>

namespace ddb {

template<class T>
class BufferWriter{
public:
	BufferWriter(const T& out) : out_(out), buffer_(0), size_(0){}

	IO_ERR start(const char* buffer, std::size_t length){
		IO_ERR ret = OK;
		std::size_t actualWritten = 0;

		buffer_ = (char*)buffer;
		size_ = length;
		while((ret = out_->write(buffer_, size_, actualWritten))==OK  && actualWritten < size_){
			buffer_ += actualWritten;
			size_ -= actualWritten;
		}
		if(ret == NOSPACE){
			buffer_ += actualWritten;
			size_ -= actualWritten;
		}
		else
			size_ = 0;
		return ret;
	}

	IO_ERR resume(){
		IO_ERR ret = OK;
		std::size_t actualWritten = 0;

		while((ret = out_->write(buffer_, size_, actualWritten))==OK  && actualWritten < size_){
			buffer_ += actualWritten;
			size_ -= actualWritten;
		}
		if(ret == NOSPACE){
			buffer_ += actualWritten;
			size_ -= actualWritten;
		}
		else
			size_ = 0;
		return ret;
	}

	inline std::size_t size() const { return size_;}
	inline T getDataOutputStream() const { return out_;}

private:
	T out_;
	char* buffer_;
	std::size_t size_;
};

} // namespace ddb
