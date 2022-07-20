#ifndef WEDGE3_BATTLE_END_H
#define WEDGE3_BATTLE_END_H

#include "wedge3/main.h"
#include "wedge3/general.h"
#include "wedge3/systems.h"

namespace wedge {

class WEDGE3_EXPORT Battle_End_Step : public Step
{
public:
	Battle_End_Step(Task *task);
	virtual ~Battle_End_Step();
	
	bool run();
	void done_signal(Step *step);

private:
	int count;
};

}

#endif // WEDGE3_BATTLE_END_H
