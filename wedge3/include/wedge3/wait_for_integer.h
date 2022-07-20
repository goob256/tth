#ifndef WEDGE3_WAIT_FOR_INTEGER_H
#define WEDGE3_WAIT_FOR_INTEGER_H

#include "wedge3/main.h"
#include "wedge3/systems.h"

namespace wedge {

class WEDGE3_EXPORT Wait_For_Integer_Step : public Step
{
public:
	Wait_For_Integer_Step(int *i, int desired_value, Task *task);
	virtual ~Wait_For_Integer_Step();
	
	bool run();

private:
	int *i;
	int desired_value;
};

}

#endif // WEDGE3_WAIT_FOR_INTEGER_H
