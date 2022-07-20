#ifndef WEDGE3_PAUSE_PRESSES_H
#define WEDGE3_PAUSE_PRESSES_H

#include "wedge3/main.h"
#include "wedge3/systems.h"

namespace wedge {

class WEDGE3_EXPORT Pause_Presses_Step : public Step
{
public:
	Pause_Presses_Step(bool paused, bool repeat_pressed, Task *this_task);
	virtual ~Pause_Presses_Step();
	
	void start();
	bool run();

private:
	bool paused;
	bool repeat_pressed;
};

}

#endif // WEDGE3_PAUSE_PRESSES_H
