#ifndef WEDGE3_RUMBLE_H
#define WEDGE3_RUMBLE_H

#include "wedge3/main.h"
#include "wedge3/systems.h"

namespace wedge {

class WEDGE3_EXPORT Rumble_Step : public Step
{
public:
	Rumble_Step(float force, Uint32 length, Task *task);
	virtual ~Rumble_Step();

	bool run();

private:
	float force;
	Uint32 length;
};

}

#endif // WEDGE3_RUMBLE_H
