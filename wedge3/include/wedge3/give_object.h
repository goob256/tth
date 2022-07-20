#ifndef WEDGE3_GIVE_OBJECT_H
#define WEDGE3_GIVE_OBJECT_H

#include "wedge3/main.h"
#include "wedge3/systems.h"

namespace wedge {

class WEDGE3_EXPORT Give_Object_Step : public Step
{
public:
	Give_Object_Step(Object object, Dialogue_Position dialogue_position, Task *task);
	virtual ~Give_Object_Step();

	void start();
	bool run();
	void done_signal(Step *step);

private:
	Object object;
	bool done;
	Dialogue_Position dialogue_position;
};

}

#endif // WEDGE3_GIVE_OBJECT_H
