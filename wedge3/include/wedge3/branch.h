#ifndef WEDGE3_BRANCH_H
#define WEDGE3_BRANCH_H

#include "wedge3/main.h"
#include "wedge3/systems.h"

namespace wedge {

class WEDGE3_EXPORT Branch_Step : public Step
{
public:
	Branch_Step(int *i, Game *game, std::vector< std::vector< std::vector< Step *> > > steps, Task *task);
	virtual ~Branch_Step();

	bool run();

private:
	int *i;
	Game *game;
	std::vector< std::vector< std::vector< Step *> > > steps;
};

}

#endif // WEDGE3_BRANCH_H
