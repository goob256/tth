#ifndef WEDGE3_STOP_MUSIC_H
#define WEDGE3_STOP_MUSIC_H

#include "wedge3/main.h"
#include "wedge3/systems.h"

namespace wedge {

class WEDGE3_EXPORT Stop_Music_Step : public Step
{
public:
	Stop_Music_Step(Task *task);
	virtual ~Stop_Music_Step();

	void start();
	bool run();
};

}

#endif // WEDGE3_STOP_MUSIC_H
