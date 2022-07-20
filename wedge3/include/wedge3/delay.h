#ifndef WEDGE3_DELAY_H
#define WEDGE3_DELAY_H

#include "wedge3/main.h"
#include "wedge3/systems.h"

namespace wedge {

class WEDGE3_EXPORT Delay_Step : public Step
{
public:
	Delay_Step(int millis, Task *task);
	virtual ~Delay_Step();
	
	void start();
	bool run();

private:
	int millis;
	int start_time;
};

}

#endif // WEDGE3_DELAY_H
