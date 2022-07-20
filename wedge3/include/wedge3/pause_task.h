#ifndef WEDGE3_PAUSE_TASK_H
#define WEDGE3_PAUSE_TASK_H

#include "wedge3/main.h"
#include "wedge3/systems.h"

namespace wedge {

class WEDGE3_EXPORT Pause_Task_Step : public Step
{
public:
	Pause_Task_Step(Task *task_to_pause, bool paused, Task *this_task);
	virtual ~Pause_Task_Step();
	
	void start();
	bool run();

private:
	Task *task_to_pause;
	bool paused;
};

}

#endif // WEDGE3_PAUSE_TASK_H
