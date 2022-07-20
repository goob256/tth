#ifndef WEDGE3_SLIDE_ENTITY_H
#define WEDGE3_SLIDE_ENTITY_H

#include "wedge3/main.h"
#include "wedge3/systems.h"

namespace wedge {

class Map_Entity;

class WEDGE3_EXPORT Slide_Entity_Step : public Step
{
public:
	Slide_Entity_Step(Map_Entity *entity, util::Point<int> destination_tile, float speed, Task *task);
	virtual ~Slide_Entity_Step();

	void start();
	bool run();

private:
	Map_Entity *entity;
	util::Point<int> destination_tile;
	float speed;
	util::Point<int> start_pos;
	util::Point<float> start_offset;
};

}

#endif // WEDGE3_SLIDE_ENTITY_H
