#ifndef NOO_AUDIO_H
#define NOO_AUDIO_H

#include "shim3/main.h"

namespace noo {

namespace audio {

bool SHIM3_EXPORT static_start();
bool SHIM3_EXPORT start();
void SHIM3_EXPORT end();
int SHIM3_EXPORT millis_to_samples(int millis);
int SHIM3_EXPORT samples_to_millis(int samples, int freq = -1);
void SHIM3_EXPORT pause_sfx(bool paused);

} // End namespace audio

} // End namespace noo

#endif // NOO_AUDIO_H
