#ifndef NOO_ACHIEVEMENTS_H
#define NOO_ACHIEVEMENTS_H

#include "shim3/main.h"

namespace noo {

namespace util {

bool SHIM3_EXPORT achieve(void *id);
bool SHIM3_EXPORT show_achievements();

// FIXME: stuff for setting up achievements (store system-specific ids/etc)

} // End namespace util

} // End namespace noo

#endif // NOO_ACHIEVEMENTS_H
