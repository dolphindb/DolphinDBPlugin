#ifndef CVT_H_
#define CVT_H_
#include <stdexcept>
#include <string>
#ifdef WINDOWS
#include <windows.h>
#endif

enum class Encoding { UTF8, GB2312 };

namespace utf8 {
const uint16_t LEAD_SURROGATE_MIN = 0xd800u;
const uint16_t LEAD_SURROGATE_MAX = 0xdbffu;
const uint16_t TRAIL_SURROGATE_MIN = 0xdc00u;
const uint16_t TRAIL_SURROGATE_MAX = 0xdfffu;
const uint16_t LEAD_OFFSET = 0xd7c0u;             // LEAD_SURROGATE_MIN - (0x10000 >> 10)
const uint32_t SURROGATE_OFFSET = 0xfca02400u;    // 0x10000u - (LEAD_SURROGATE_MIN << 10) - TRAIL_SURROGATE_MIN

class RangeError : public std::exception {
   public:
    RangeError(std::string msg) : msg(msg) {}
    RangeError() : msg("utf8 range error") {}
    const char* what() const noexcept override { return msg.c_str(); }

   private:
    std::string msg;
};

template <typename u8_iterator>
int utf8length(const u8_iterator& it) {
    // refer to https://www.wikiwand.com/en/UTF-8
    uint8_t lead = *it;
    if (lead < 0x80u) {    // U+0000 - U+007F
        return 1;
    } else if ((lead >> 5u) == 0x6u) {    // U+0080 - U+07FF
        return 2;
    } else if ((lead >> 4u) == 0xeu) {    // U+0800 - U+FFFF
        return 3;
    } else if ((lead >> 3u) == 0x1eu) {    // U+10000 - U+10FFFF
        return 4;
    } else {
        throw RangeError();
    }
}

template <typename u8_iterator>
uint32_t u8NextCodepoint(u8_iterator& it, u8_iterator end) {
    int len = utf8length(it);
    uint32_t cp = 0;
    if (it == end) throw RangeError();
    if (len == 1) {
        cp = *it++;
    } else if (len == 2) {    // 110xxxxx 10xxxxxx
        cp = ((*it++) & (0x1fu));
        cp <<= 6u;
        if (it == end) throw RangeError();
        cp += ((*it++) & (0x3fu));
    } else if (len == 3) {    // 1110xxxx 10xxxxxx 10xxxxxx
        cp = ((*it++) & (0xfu));
        cp <<= 6u;
        if (it == end) throw RangeError();
        cp += ((*it++) & 0x3fu);
        cp <<= 6u;
        if (it == end) throw RangeError();
        cp += ((*it++) & 0x3fu);
    } else if (len == 4) {    // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        cp = ((*it++) & 0x7u);
        cp <<= 6u;
        if (it == end) throw RangeError();
        cp += ((*it++) & 0x3fu);
        cp <<= 6u;
        if (it == end) throw RangeError();
        cp += ((*it++) & 0x3fu);
        cp <<= 6u;
        if (it == end) throw RangeError();
        cp += ((*it++) & 0x3fu);
    }
    return cp;
}

template <typename u16_iterator>
uint32_t u16NextCodepoint(u16_iterator& it, u16_iterator end) {
    if (*it < 0xd800) {
        return *it++;
    } else {
        uint32_t cp = (*it++) - LEAD_SURROGATE_MIN;
        cp <<= 10u;
        if (it == end) throw RangeError();
        cp += (*it++) - TRAIL_SURROGATE_MIN + 0x10000;
        return cp;
    }
}

template <typename u16_iterator>
void u16append(u16_iterator& it, uint32_t codepoint) {
    if (codepoint > 0xffffu) {
        codepoint -= 0x10000u;
        *it++ = static_cast<uint16_t>((codepoint >> 10u) + 0xd800u);
        *it++ = static_cast<uint16_t>((codepoint & 0x3ffu) + 0xdc00u);
    } else {
        *it++ = codepoint;
    }
}

template <typename u8_iterator>
void u8append(u8_iterator& it, uint32_t codepoint) {
    if (codepoint < 0x80u) {
        *it++ = codepoint;
    } else if (codepoint < 0x800u) {
        *it++ = static_cast<uint8_t>(0xc0u | (codepoint >> 6u));
        *it++ = static_cast<uint8_t>(0x80u | (codepoint & 0x3fu));
    } else if (codepoint < 0x10000u) {
        *it++ = static_cast<uint8_t>(0xe0u | (codepoint >> 12u));
        *it++ = static_cast<uint8_t>(0x80u | ((codepoint >> 6u) & 0x3fu));
        *it++ = static_cast<uint8_t>(0x80u | (codepoint & 0x3fu));
    } else {
        *it++ = static_cast<uint8_t>(0xf0u | (codepoint >> 18u));
        *it++ = static_cast<uint8_t>(0x80u | ((codepoint >> 12u) & 0x3fu));
        *it++ = static_cast<uint8_t>(0x80u | ((codepoint >> 6u) & 0x3fu));
        *it++ = static_cast<uint8_t>(0x80u | (codepoint & 0x3fu));
    }
}

template <typename u8_iterator, typename u16_iterator>
void utf8to16(u8_iterator begin, u8_iterator end, u16_iterator result) {
    // utf8 -> codepoint -> utf16
    try {
        while (begin != end) {
            uint32_t cp = u8NextCodepoint(begin, end);
            u16append(result, cp);
        }
    } catch (RangeError& e) {
        // pass
    }
}

template <typename u8_iterator, typename u16_iterator>
void utf16to8(u16_iterator begin, u16_iterator end, u8_iterator result) {
    // utf16 -> codepoint -> utf8
    try {
        while (begin != end) {
            uint32_t cp = u16NextCodepoint(begin, end);
            u8append(result, cp);
        }
    } catch (RangeError& e) {
        // pass
    }
}
}    // namespace utf8

std::u16string utf8_to_utf16(const std::string& utf8);
std::u16string utf8_to_utf16(const char* first, const char* last);
std::string utf16_to_utf8(const std::u16string& utf16);
std::string utf16_to_utf8(const char16_t* from, const char16_t* to);

int gb2312_to_utf8(const char* src, char* utf8);
template <Encoding E>
int bytesToUTF8(const char* src, char* utf8);

#ifdef WINDOWS
template <>
inline int bytesToUTF8<Encoding::GB2312>(const char* src, char* utf8) {
    return gb2312_to_utf8(src, utf8);
}
#endif

#endif