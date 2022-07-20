#ifndef WEDGE3_PAUSE_SPRITE_H
#define WEDGE3_PAUSE_SPRITE_H

#include "wedge3/main.h"
#include "wedge3/systems.h"

namespace wedge {

class WEDGE3_EXPORT Pause_Sprite_Step : public Step
{
public:
	Pause_Sprite_Step(gfx::Sprite *sprite, bool paused, Task *task);
	virtual ~Pause_Sprite_Step();

	void start();
	bool run();

private:
	gfx::Sprite *sprite;
	bool paused;
};

}

#endif // WEDGE3_PAUSE_SPRITE_H
