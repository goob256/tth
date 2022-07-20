#ifndef WEDGE3_SET_MUSIC_VOLUME_H
#define WEDGE3_SET_MUSIC_VOLUME_H

#include "wedge3/main.h"
#include "wedge3/systems.h"

namespace wedge {

class WEDGE3_EXPORT Set_Music_Volume_Step : public Step
{
public:
	Set_Music_Volume_Step(float volume, Task *task);
	virtual ~Set_Music_Volume_Step();

	void start();
	bool run();

private:
	float volume;
};

}

#endif // WEDGE3_SET_MUSIC_VOLUME_H
