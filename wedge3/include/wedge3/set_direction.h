#ifndef WEDGE3_SET_DIRECTION_H
#define WEDGE3_SET_DIRECTION_H

#include "wedge3/main.h"
#include "wedge3/globals.h"
#include "wedge3/systems.h"

namespace wedge {

class Map_Entity;

class WEDGE3_EXPORT Set_Direction_Step : public Step
{
public:
	Set_Direction_Step(Map_Entity *entity, Direction direction, bool set_animation, bool moving, Task *task);
	virtual ~Set_Direction_Step();

	void start();
	bool run();

private:
	Map_Entity *entity;
	Direction direction;
	bool set_animation;
	bool moving;
};

}

#endif // WEDGE3_SET_DIRECTION_H
