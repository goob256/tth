#ifndef WEDGE3_SET_SOLID_H
#define WEDGE3_SET_SOLID_H

#include "wedge3/main.h"
#include "wedge3/systems.h"

namespace wedge {

class Map_Entity;

class WEDGE3_EXPORT Set_Solid_Step : public Step
{
public:
	Set_Solid_Step(Map_Entity *entity, bool solid, Task *task);
	virtual ~Set_Solid_Step();

	void start();
	bool run();

private:
	Map_Entity *entity;
	bool solid;
};

}

#endif // WEDGE3_SET_SOLID_H
