#ifndef NOO_MT_H
#define NOO_MT_H

#include "shim3/main.h"

using namespace noo;

namespace noo {

namespace util {

void SHIM3_EXPORT srand(uint32_t s);
uint32_t SHIM3_EXPORT rand();
uint32_t SHIM3_EXPORT rand(int min, int max_inclusive);

} // End namespace util

} // End namespace noo

#endif // NOO_MT_H
