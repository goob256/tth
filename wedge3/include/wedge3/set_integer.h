#ifndef WEDGE3_SET_INTEGER_H
#define WEDGE3_SET_INTEGER_H

#include "wedge3/main.h"
#include "wedge3/systems.h"

namespace wedge {

class WEDGE3_EXPORT Set_Integer_Step : public Step
{
public:
	Set_Integer_Step(int *i, int value, Task *task);
	virtual ~Set_Integer_Step();

	bool run();

private:
	int *i;
	int value;
};

}

#endif // WEDGE3_SET_INTEGER_H
