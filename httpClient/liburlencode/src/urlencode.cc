#include <urlencode.h>
#include <vector>

namespace urlencode {

using table_data::hexval;
using table_data::uri_encode_tbl;

size_t Encode(const char* src,
              const size_t len,
              char* dst,
              bool space_to_plus) {
  size_t cur = 0;

  for (size_t i = 0; i < len; i++) {
    const char octet = src[i];
    if (octet == ' ' && space_to_plus) {
      dst[cur++] = '+';
      continue;
    }

    const int32_t code = reinterpret_cast<const int32_t*>(
        uri_encode_tbl)[static_cast<unsigned char>(octet)];

    if (code) {
      *(reinterpret_cast<int32_t*>(&dst[cur])) = code;
      cur += 3;
    } else {
      dst[cur++] = octet;
    }
  }

  dst[cur] = 0;
  return cur;
}

size_t Decode(const char* src,
              const size_t len,
              char* dst,
              bool plus_to_space) {
  size_t i = 0;
  size_t j = 0;

  while (i < len) {
    int copy_char = 1;

    if (src[i] == '+' && plus_to_space) {
      dst[j] = ' ';
      i++;
      j++;
      continue;
    }

    if (src[i] == '%' && i + 2 < len) {
      const unsigned char v1 = hexval[static_cast<unsigned char>(src[i + 1])];
      const unsigned char v2 = hexval[static_cast<unsigned char>(src[i + 2])];

      /* skip invalid hex sequences */
      if ((v1 | v2) != 0xFF) {
        dst[j] = (v1 << 4) | v2;
        j++;
        i += 3;
        copy_char = 0;
      }
    }

    if (copy_char) {
      dst[j] = src[i];
      i++;
      j++;
    }
  }

  dst[j] = 0;
  return j;
}

string EncodeString(const string& data){
  int dataSize = data.size();
  std::vector<char> dstBuff(dataSize * 3);
  Encode(data.c_str(), dataSize, dstBuff.data(), false);
  return string(dstBuff.data());
}

}  // namespace urlencode
