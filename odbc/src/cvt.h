#ifndef CVT_H_
#define CVT_H_
#include <string>
#ifdef WINDOWS
#include <windows.h>
#else
#include <iconv.h>
#endif

enum class Encoding {
    UTF8,
    GB2312
};

std::u16string utf8_to_utf16(const std::string& utf8);
std::u16string utf8_to_utf16(const char* first, const char* last);
std::string utf16_to_utf8(const std::u16string& utf16);
std::string utf16_to_utf8(const char16_t* from, const char16_t* to);


int gb2312_to_utf8(const char* src, char* utf8);
template <Encoding E>
int bytesToUTF8(const char* src, char* utf8);

#ifdef WINDOWS
template <>
int bytesToUTF8<Encoding::GB2312>(const char* src, char* utf8) {
    return gb2312_to_utf8(src, utf8);
}
#endif

#endif