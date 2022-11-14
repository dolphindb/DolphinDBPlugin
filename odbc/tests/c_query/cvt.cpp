#include "cvt.h"
#include <stdexcept>
#include <vector>

#if defined(__GNUC__) && (__GNUC__ < 5)
#else
#include <codecvt>
#include <locale>
#endif


std::u16string utf8_to_utf16(const char* first, const char* last) {
#if defined(__GNUC__) && (__GNUC__ < 5)
    std::u16string ret;
    try {
        utf8::utf8to16(first, last, std::back_inserter(ret));
    } catch(std::exception& ex) {
        // pass
    }
    return ret;
#else
    static thread_local std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conv;
    return conv.from_bytes(first, last);
#endif
}

std::u16string utf8_to_utf16(const std::string& utf8) {
    return utf8_to_utf16(utf8.c_str(), utf8.c_str() + utf8.size());
}

std::string utf16_to_utf8(const char16_t* first, const char16_t* last) {
#if defined(__GNUC__) && (__GNUC__ < 5)
    std::string ret;
    utf8::utf16to8(first, last, std::back_inserter(ret));
    return ret;
#else
    static thread_local std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conv;
    return conv.to_bytes(first, last);
#endif
}

std::string utf16_to_utf8(const std::u16string& utf16) {
    return utf16_to_utf8(utf16.c_str(), utf16.c_str() + utf16.size());
}

#ifdef WINDOWS
int gb2312_to_utf8(const char* gb2312, char* utf8) {
    int len = MultiByteToWideChar(CP_ACP, 0, gb2312, -1, NULL, 0);
    wchar_t *wstr = new wchar_t[len + 1];
    memset(wstr, 0, len + 1);
    MultiByteToWideChar(CP_ACP, 0, gb2312, -1, wstr, len);
    len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, utf8, len, NULL, NULL);
    if (wstr) delete[] wstr;
    return len;
}
#endif
