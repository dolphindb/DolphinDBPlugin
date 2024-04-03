#pragma once
#include <cstdint>
#include <cstring>
#include <string>
#include <algorithm>
#include <functional>
#include <ctype.h>
#include <cassert>
#include <stdexcept>

class DolphinString {
public:
    DolphinString() {
        constructInlineString(nullptr, 0);
    }

    DolphinString(const std::string & str) {
        constructString(str.data(), str.size());
    }

    DolphinString(const char * ptr, size_t len) {
        constructString(ptr, len);
    }

    DolphinString(const DolphinString & rhs) {
        constructString(rhs.data(), rhs.size());
    }

    DolphinString(const char * p) {
        size_t strLen = strlen(p);
        if (strLen <= INLINE_STR_MAX_LEN)
            constructInlineString(p, strLen);
        else
            constructNoninlineString(p, strLen);
    }

    //Assignment operator
    DolphinString & operator=(const DolphinString & rhs) {
        if (this != &rhs) {
            return assign(rhs.data(), rhs.size());
        }
        return (*this);
    }

    //Move-constructor
    DolphinString(DolphinString && rhs) noexcept {
        noninlineData = rhs.noninlineData;
        rhs.constructInlineString(nullptr, 0);
    }

    //Move-assignment operator
    DolphinString & operator=(DolphinString && rhs) noexcept {
        if (this != &rhs) {
            clear();
            noninlineData = rhs.noninlineData;
            rhs.constructInlineString(nullptr, 0);
        }
        return (*this);
    }

    //Move-assignment
    DolphinString& assign(DolphinString&& str){
        if (this != &str) {
            clear();
            noninlineData = str.noninlineData;
            str.constructInlineString(nullptr, 0);
        }
        return (*this);
    }

    inline DolphinString& assign(const DolphinString& str){
    	return assign(str.data(), str.size());
    }

    inline DolphinString& assign(const std::string& str){
    	return assign(str.data(), str.size());
    }

    inline DolphinString& assign(const char* str){
    	return assign(str, strlen(str));
    }

    DolphinString& assign(const char* str, size_t len);

    void append(const char* str, size_t length);

    ~DolphinString() {
        clear();
    }

public:
    void lower() {
        size_t sz = size();
        char * ptr = getData();
        std::transform(ptr, ptr + sz, ptr, [](char c) { return std::tolower(c); });
    }

    void upper() {
        size_t sz = size();
        char * ptr = getData();
        std::transform(ptr, ptr + sz, ptr, [](char c) { return std::toupper(c); });
    }

    void trim(std::function<int(int)> trimFunc = [](int c) { return std::isspace(c); }) {
        int sz = size();
        char * ptr = getData();
        int startIdx = 0;
        while (startIdx < sz && trimFunc(ptr[startIdx]))
            ++startIdx;
        int endIdx = sz - 1;
        while (endIdx >= 0 && trimFunc(ptr[endIdx]))
            --endIdx;
        if (endIdx < startIdx) {
            clear();
            constructInlineString(nullptr, 0);
            return;
        }
        int newSize = endIdx - startIdx + 1;
        if (newSize == sz) {
            return;
        }
        assert(newSize < sz);
        if (isInline() == false && newSize + 1 < INLINE_STR_CAP) {
            // noninline -> inline: take advantage of move assignment operator
            *this = std::move(DolphinString(ptr + startIdx, newSize));
            return;
        }
        memmove(ptr, ptr + startIdx, newSize);
        ptr[newSize] = '\0';
        setSize(newSize);
    }

    inline int compare(const std::string & str) const {
    	size_t lLen = size();
    	size_t rLen = str.size();
    	size_t len = lLen <= rLen ? lLen : rLen;
    	const unsigned char* lData = (const unsigned char*)data();
    	const unsigned char* rData = (const unsigned char*)str.data();
    	size_t i=0;
    	for(; i<len && lData[i] == rData[i]; ++i);

    	if(i < len)
    		return lData[i] < rData[i] ? -1 : 1;
    	else if(lLen == rLen)
    		return 0;
    	else
    		return lLen < rLen ? -1 : 1;
    }

    inline int compare(const char * str) const {
    	size_t len = size();
    	const unsigned char* lData = (const unsigned char*)data();
    	const unsigned char* rData = (const unsigned char*)str;
    	size_t i = 0;
    	while(i < len && lData[i] == rData[i] && rData[i] != 0) ++i;

    	if(i < len && rData[i] != 0)
    		return lData[i] < rData[i] ? -1 : 1;
    	else if(i == len && rData[i] == 0)
    		return 0;
    	else
    		return i == len ? -1 : 1;
    }

    inline int compare(const DolphinString & str) const {
    	size_t lLen = size();
    	size_t rLen = str.size();
    	size_t len = lLen <= rLen ? lLen : rLen;
    	const unsigned char* lData = (const unsigned char*)data();
    	const unsigned char* rData = (const unsigned char*)str.data();
    	size_t i=0;
    	for(; i<len && lData[i] == rData[i]; ++i);

    	if(i < len)
    		return lData[i] < rData[i] ? -1 : 1;
    	else if(lLen == rLen)
    		return 0;
    	else
    		return lLen < rLen ? -1 : 1;
    }

    bool operator==(const char * str) {
        return compare(str) == 0;
    }

    bool operator==(const std::string & str) {
        return compare(str) == 0;
    }

    bool operator<(const std::string & str) const {
        return compare(str) < 0;
    }

    bool operator<=(const std::string & str) const {
        return compare(str) <= 0;
    }

    bool operator>=(const std::string & str) const {
        return compare(str) >= 0;
    }

    bool operator>(const std::string & str) const {
        return compare(str) > 0;
    }

    bool operator==(const DolphinString & rhs) const {
    	size_t len = size();
    	if(len != rhs.size())
    		return false;
    	const char* lData = data();
    	const char* rData = rhs.data();
    	size_t i=0;
    	for(; i<len && lData[i] == rData[i]; ++i);
    	return i==len;
    }

    bool operator!=(const DolphinString & rhs) const {
        return compare(rhs) != 0;
    }

    bool operator<(const DolphinString & rhs) const {
        return compare(rhs) < 0;
    }

    bool operator<=(const DolphinString & rhs) const {
        return compare(rhs) <= 0;
    }

    bool operator>=(const DolphinString & rhs) const {
        return compare(rhs) >= 0;
    }

    bool operator>(const DolphinString & rhs) const {
        return compare(rhs) > 0;
    }

    DolphinString operator+(const DolphinString & rhs) const {
    	DolphinString tmp(*this);
    	tmp.append(rhs.data(), rhs.size());
        return tmp;
    }

    DolphinString operator+(const std::string & rhs) const {
    	DolphinString tmp(*this);
    	tmp.append(rhs.data(), rhs.size());
        return tmp;
    }

    DolphinString operator+(const char*  rhs) const {
    	DolphinString tmp(*this);
    	tmp.append(rhs, strlen(rhs));
        return tmp;
    }

    char & operator[](size_t idx) {
        return getData()[idx];
    }

    const char & operator[](size_t idx) const {
        return getData()[idx];
    }

    char & at(size_t idx) {
        if (idx >= size()) throw std::out_of_range("at(idx) out of range");
        return getData()[idx];
    }

    const char & at(size_t idx) const {
        if (idx >= size()) throw std::out_of_range("at(idx) out of range");
        return getData()[idx];
    }

    bool empty() const {
        return size() == 0;
    }
private:
    static size_t find(const char * s, const char * p, size_t pos) {
        auto startPtr = s + pos;
        auto res = strstr(startPtr, p);
        if (res == nullptr) return std::string::npos;
        else return res - startPtr + pos;
    }

    inline bool isLittleEndian(){ int x=1; return *(char *)&x == 1;}
public:

    // Return the position of the first character of the first match.
    // If no matches were found, the function returns string::npos.
    size_t find(const DolphinString & str, size_t pos = 0) const {
        return find(this->getData(), str.getData(), pos);
    }
    // Return the position of the first character of the first match.
    // If no matches were found, the function returns string::npos.
    size_t find(const char * str, size_t pos = 0) const {
        return find(this->getData(), str, pos);
    }

    DolphinString substr(size_t pos = 0, size_t len = std::string::npos) const {
        auto thisSize = size();
        if (pos >= thisSize) throw std::out_of_range("Out of bounds");
        return DolphinString(getData() + pos, std::min(thisSize - pos , len));
    }

    std::string getString() const {
        return std::string(getData(), size());
    }

    inline size_t size() const {
        return isInline() ? getInlineStringSize() : noninlineData.size_;
    }

    inline size_t length() const {
        return size();
    }

    inline char* getData() {
        return isInline() ? inlineData : noninlineData.ptr_;
    }

    inline const char* getData() const {
        return isInline() ? inlineData : noninlineData.ptr_;
    }

    inline char* data() {
        return isInline() ? inlineData : noninlineData.ptr_;
    }

    inline const char* data() const {
        return isInline() ? inlineData : noninlineData.ptr_;
    }

    inline const char * c_str() const {
        return getData();
    }

    inline size_t getCapacity() {
        if (isInline())
            return INLINE_STR_CAP - 1;
        if (isLittleEndian())
            return noninlineData.cap_ & ~(FLAG_UINT64_MASK);
        return noninlineData.cap_ >> FLAT_BIT_COUNT;
    }

    void reserve(size_t n);
    void resize(size_t n);

    //clear both data and allocated buffer.
    void clear();

    //clear data only and keep allocated buffer unchanged.
    inline void clearData(){
    	getData()[0] = 0;
    	setSize(0);
    }

    static constexpr int INLINE_STR_CAP = sizeof(char*) + sizeof(uint64_t) + sizeof(uint64_t);

private:
    // if the length of the string (including '\0') exceeds this threshold, we switch to buddy allocator.
    static constexpr unsigned BUDDY_ALLOC_THRESHOLD = 2047;
    static constexpr unsigned INLINE_STR_MAX_LEN = INLINE_STR_CAP - 2;
    static constexpr uint8_t FLAG_BYTE_MASK = 0x80;
    static constexpr uint8_t FLAT_BIT_COUNT = 1;
    static constexpr uint64_t FLAG_UINT64_MASK = 0x8000000000000000;

    void constructString(const char * p, size_t strLen) {
        if (strLen <= INLINE_STR_MAX_LEN)
            constructInlineString(p, strLen);
        else
            constructNoninlineString(p, strLen);
    }

    void constructString(const char * p) {
        size_t strLen = strlen(p);
        constructString(p, strLen);
    }

    void constructInlineString(const char * p, size_t strLen) {
        memset(inlineData, 0, sizeof(inlineData));
        markInlineBit();
        setSize(strLen);
        char* data = getData();
        if (p)  {
            memcpy(data, p, strLen);
            data[strLen] = '\0';
        }
        else {
            data[0] = '\0';
        }
    }

    void constructNoninlineString(const char * p, size_t strLen) {
        assert(strLen > INLINE_STR_MAX_LEN);
        assert(p);
        size_t newCap = strLen + 1;
        char * buf = allocateBuffer(newCap);
        if (buf == nullptr) throw std::bad_alloc();
        clearInlineBit();
        noninlineData.ptr_ = buf;
        setCapacity(newCap);
        setSize(strLen);
        memcpy(buf, p, strLen);
        buf[strLen] = '\0';
    }


    char * allocateBuffer(size_t& size);

    void releaseBuffer();

    inline void setCapacity(size_t cap) {
        if (isInline()) return;
        if (isLittleEndian())
            noninlineData.cap_ = cap & (~(1ULL << 63));
        else
            noninlineData.cap_ = (cap << 1);
    }

    inline void setSize(size_t size) {
        if (isInline()) setInlineStringSize(size);
        else noninlineData.size_ = size;
    }

    inline int getInlineFlagByte() const {
        return inlineData[INLINE_STR_CAP - 1] & FLAG_BYTE_MASK;
    }

    inline int getInlineBit() const {
        return (inlineData[INLINE_STR_CAP - 1] & FLAG_BYTE_MASK) >> (8 - FLAT_BIT_COUNT);
    }

    inline void markInlineBit() {
        inlineData[INLINE_STR_CAP - 1] |= 1 << 7;
    }

    inline void clearInlineBit() {
        inlineData[INLINE_STR_CAP - 1] &= ~FLAG_BYTE_MASK;
    }

    inline bool isInline() const {
        return getInlineBit() & 1;
    }

    inline size_t getInlineStringSize() const {
        assert(isInline());
        return static_cast<uint8_t>(inlineData[INLINE_STR_CAP - 1] & (~(FLAG_BYTE_MASK)));
    }

    inline void setInlineStringSize(size_t size) {
        assert(size <= INLINE_STR_MAX_LEN);
        inlineData[INLINE_STR_CAP - 1] = size | getInlineFlagByte();
    }

    union {
        struct {
            char* ptr_;
            uint64_t size_;
            uint64_t cap_;
        }noninlineData;
        char inlineData[INLINE_STR_CAP];
    };
};
