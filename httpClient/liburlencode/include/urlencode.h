#ifndef INCLUDE_URLENCODE_H_
#define INCLUDE_URLENCODE_H_

#include <stdlib.h>
#include <string>
#include "urlencode_version.h"
using namespace std;

namespace urlencode {
namespace table_data {
extern const char uri_encode_tbl[256 * 4];
extern const unsigned char hexval[256];
}  // namespace table_data

size_t Encode(const char* src, const size_t len, char* dst, bool space_to_plus);
size_t Decode(const char* src, const size_t len, char* dst, bool plus_to_space);

string EncodeString(const string& data);

}  // namespace urlencode

#endif  // INCLUDE_URLENCODE_H_
