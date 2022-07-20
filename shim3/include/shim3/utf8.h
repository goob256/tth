#ifndef NOO_UTF8_H
#define NOO_UTF8_H

#include "shim3/main.h"

namespace noo {

namespace util {

SHIM3_EXPORT int utf8_len(std::string text);
SHIM3_EXPORT int utf8_len_bytes(std::string text, int char_count);
SHIM3_EXPORT Uint32 utf8_char_next(std::string text, int &offset);
SHIM3_EXPORT Uint32 utf8_char_offset(std::string text, int o);
SHIM3_EXPORT Uint32 utf8_char(std::string text, int i);
SHIM3_EXPORT std::string utf8_char_to_string(Uint32 ch);
SHIM3_EXPORT std::string utf8_substr(std::string s, int start, int count = -1);

} // End namespace util

}

#endif // NOO_UTF8_H
