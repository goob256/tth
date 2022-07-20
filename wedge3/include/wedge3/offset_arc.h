#ifndef WEDGE3_OFFSET_ARC_H
#define WEDGE3_OFFSET_ARC_H

#include "wedge3/main.h"
#include "wedge3/systems.h"

namespace wedge {

class Map_Entity;

class WEDGE3_EXPORT Offset_Arc_Step : public Step
{
public:
	Offset_Arc_Step(Map_Entity *entity, util::Point<float> start_offset, util::Point<float> end_offset, float height, Uint32 duration, Task *task);
	virtual ~Offset_Arc_Step();

	void start();
	bool run();

private:
	Map_Entity *entity;
	util::Point<float> start_offset;
	util::Point<float> end_offset;
	float height;
	Uint32 duration;
	Uint32 start_time;
};

}

#endif // WEDGE3_OFFSET_ARC_H
