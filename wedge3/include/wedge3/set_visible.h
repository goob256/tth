#ifndef WEDGE3_SET_VISIBLE_H
#define WEDGE3_SET_VISIBLE_H

#include "wedge3/main.h"
#include "wedge3/systems.h"

namespace wedge {

class Map_Entity;

class WEDGE3_EXPORT Set_Visible_Step : public Step
{
public:
	Set_Visible_Step(Map_Entity *entity, bool visible, Task *task);
	virtual ~Set_Visible_Step();

	void start();
	bool run();

private:
	Map_Entity *entity;
	bool visible;
};

}

#endif // WEDGE3_SET_VISIBLE_H
