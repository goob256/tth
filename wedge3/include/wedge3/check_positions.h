#ifndef WEDGE3_CHECK_POSITIONS_H
#define WEDGE3_CHECK_POSITIONS_H

#include "wedge3/main.h"
#include "wedge3/systems.h"

namespace wedge {

class Map_Entity;

class WEDGE3_EXPORT Check_Positions_Step : public Step
{
public:
	Check_Positions_Step(std::vector<Map_Entity *> entities, std::vector< util::Point<int> > positions, bool check_for_zero_offset, Task *task);
	virtual ~Check_Positions_Step();

	bool run();

private:
	std::vector<Map_Entity *> entities;
	std::vector< util::Point<int> > positions;
	bool check_for_zero_offset;
};

}

#endif // WEDGE3_CHECK_POSITIONS_H
