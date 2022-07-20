#ifndef WEDGE3_STOP_SOUND_H
#define WEDGE3_STOP_SOUND_H

#include "wedge3/main.h"
#include "wedge3/systems.h"

namespace wedge {

class WEDGE3_EXPORT Stop_Sound_Step : public Step
{
public:
	Stop_Sound_Step(audio::Sound *sound, Task *task);
	virtual ~Stop_Sound_Step();

	bool run();

private:
	audio::Sound *sound;
};

}

#endif // WEDGE3_STOP_SOUND_H
