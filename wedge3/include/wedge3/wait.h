#ifndef WEDGE3_WAIT_H
#define WEDGE3_WAIT_H

// Wait for signal, call add_monitor on the Step to wait for

#include "wedge3/main.h"
#include "wedge3/systems.h"

namespace wedge {

class WEDGE3_EXPORT Wait_Step : public Step
{
public:
	Wait_Step(Task *task);
	virtual ~Wait_Step();
	
	bool run();
	
	void done_signal(Step *step);

private:
	bool done;
};

}

#endif // WEDGE3_WAIT_H
