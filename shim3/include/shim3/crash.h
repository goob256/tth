#ifndef NOO_CRASH_H
#define NOO_CRASH_H

#include "shim3/main.h"

using namespace noo;

namespace noo {

namespace util {

void SHIM3_EXPORT start_crashdumps();
void SHIM3_EXPORT end_crashdumps();

} // End namespace util

} // End namespace noo

#endif // NOO_CRASH_H
