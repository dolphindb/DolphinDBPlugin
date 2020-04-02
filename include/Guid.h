/*
 * Guid.h
 *
 *  Created on: Feb 29, 2020
 *      Author: dzhou
 */

#ifndef GUID_H_
#define GUID_H_

#include <string>
#include <string.h>

using std::string;

class Guid {
public:
	Guid(bool newGuid = false);
	Guid(unsigned char* guid){
		memcpy(uuid_, guid, 16);
	}
	Guid(const string& guid);
	Guid(const char* guid, int len);
	Guid(const Guid& copy){
		memcpy(uuid_, copy.uuid_, 16);
	}
	Guid(unsigned long long high, unsigned long long low){
#ifndef BIGENDIANNESS
		memcpy((char*)uuid_, (char*)&low, 8);
		memcpy((char*)uuid_ + 8, (char*)&high, 8);
#else
		memcpy((char*)uuid_, (char*)&high, 8);
		memcpy((char*)uuid_ + 8, (char*)&low, 8);
#endif
	}
	inline bool operator==(const Guid &other) const {
		const unsigned char* a = (const unsigned char*)uuid_;
		const unsigned char* b = (const unsigned char*)other.uuid_;
		return (*(long long*)a) == (*(long long*)b) && (*(long long*)(a+8)) == (*(long long*)(b+8));
	}
	inline bool operator!=(const Guid &other) const {
		const unsigned char* a = (const unsigned char*)uuid_;
		const unsigned char* b = (const unsigned char*)other.uuid_;
		return (*(long long*)a) != (*(long long*)b) || (*(long long*)(a+8)) != (*(long long*)(b+8));
	}
	inline bool operator<(const Guid &other) const {
		const unsigned char* a = (const unsigned char*)uuid_;
		const unsigned char* b = (const unsigned char*)other.uuid_;
#ifndef BIGENDIANNESS
		return (*(unsigned long long*)(a+8)) < (*(unsigned long long*)(b+8)) || ((*(unsigned long long*)(a+8)) == (*(unsigned long long*)(b+8)) && (*(unsigned long long*)a) < (*(unsigned long long*)b));
#else
		return (*(unsigned long long*)a) < (*(unsigned long long*)b) || ((*(unsigned long long*)a) == (*(unsigned long long*)b) && (*(unsigned long long*)(a+8)) < (*(unsigned long long*)(b+8)));
#endif
	}
	inline bool operator>(const Guid &other) const {
		const unsigned char* a = (const unsigned char*)uuid_;
		const unsigned char* b = (const unsigned char*)other.uuid_;
#ifndef BIGENDIANNESS
		return (*(unsigned long long*)(a+8)) > (*(unsigned long long*)(b+8)) || ((*(unsigned long long*)(a+8)) == (*(unsigned long long*)(b+8)) && (*(unsigned long long*)a) > (*(unsigned long long*)b));
#else
		return (*(unsigned long long*)a) > (*(unsigned long long*)b) || ((*(unsigned long long*)a) == (*(unsigned long long*)b) && (*(unsigned long long*)(a+8)) > (*(unsigned long long*)(b+8)));
#endif
	}
	inline bool operator<=(const Guid &other) const {
		const unsigned char* a = (const unsigned char*)uuid_;
		const unsigned char* b = (const unsigned char*)other.uuid_;
#ifndef BIGENDIANNESS
		return (*(unsigned long long*)(a+8)) < (*(unsigned long long*)(b+8)) || ((*(unsigned long long*)(a+8)) == (*(unsigned long long*)(b+8)) && (*(unsigned long long*)a) <= (*(unsigned long long*)b));
#else
		return (*(unsigned long long*)a) < (*(unsigned long long*)b) || ((*(unsigned long long*)a) == (*(unsigned long long*)b) && (*(unsigned long long*)(a+8)) <= (*(unsigned long long*)(b+8)));
#endif
	}
	inline bool operator>=(const Guid &other) const {
		const unsigned char* a = (const unsigned char*)uuid_;
		const unsigned char* b = (const unsigned char*)other.uuid_;
#ifndef BIGENDIANNESS
		return (*(unsigned long long*)(a+8)) > (*(unsigned long long*)(b+8)) || ((*(unsigned long long*)(a+8)) == (*(unsigned long long*)(b+8)) && (*(unsigned long long*)a) >= (*(unsigned long long*)b));
#else
		return (*(unsigned long long*)a) > (*(unsigned long long*)b) || ((*(unsigned long long*)a) == (*(unsigned long long*)b) && (*(unsigned long long*)(a+8)) >= (*(unsigned long long*)(b+8)));
#endif
	}
	inline int compare(const Guid &other) const { return (*this < other) ? -1 : (*this > other ? 1 : 0);}
	inline unsigned char operator[](int i) const { return uuid_[i];}
	inline bool isZero() const {
		const unsigned char* a = (const unsigned char*)uuid_;
		return (*(long long*)a) == 0 && (*(long long*)(a+8)) == 0;
	}
	inline bool isNull() const {
		const unsigned char* a = (const unsigned char*)uuid_;
		return (*(long long*)a) == 0 && (*(long long*)(a+8)) == 0;
	}
	inline bool isValid() const {
		const unsigned char* a = (const unsigned char*)uuid_;
		return (*(long long*)a) != 0 || (*(long long*)(a+8)) != 0;
	}
    string getString() const { return getString(uuid_);}
    inline const unsigned char* bytes() const { return uuid_;}
	static void toGuid(const unsigned char*, char* str);
	static bool fromGuid(const char* str, unsigned char* data);
    static string getString(const unsigned char* guid);
    static Guid ZERO;

private:
	unsigned char uuid_[16];
};



#endif /* GUID_H_ */
