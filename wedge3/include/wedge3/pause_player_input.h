#ifndef WEDGE3_PAUSE_PLAYER_INPUT_H
#define WEDGE3_PAUSE_PLAYER_INPUT_H

#include "wedge3/main.h"
#include "wedge3/systems.h"

namespace wedge {

class WEDGE3_EXPORT Pause_Player_Input_Step : public Step
{
public:
	Pause_Player_Input_Step(bool paused, Task *this_task);
	virtual ~Pause_Player_Input_Step();
	
	void start();
	bool run();

private:
	bool paused;
};

}

#endif // WEDGE3_PAUSE_PLAYER_INPUT_H
