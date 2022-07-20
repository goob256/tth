#ifndef WEDGE3_SET_MILESTONE_COMPLETE_H
#define WEDGE3_SET_MILESTONE_COMPLETE_H

#include "wedge3/main.h"
#include "wedge3/systems.h"

namespace wedge {

class WEDGE3_EXPORT Set_Milestone_Complete_Step : public Step
{
public:
	Set_Milestone_Complete_Step(int milestone, bool complete, Task *task);
	virtual ~Set_Milestone_Complete_Step();

	void start();
	bool run();

private:
	int milestone;
	bool complete;
};

}

#endif // WEDGE3_SET_MILESTONE_COMPLETE_H
