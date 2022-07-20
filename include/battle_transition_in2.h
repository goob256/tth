#ifndef BATTLE_TRANSITION_IN2_H
#define BATTLE_TRANSITION_IN2_H

#include <wedge3/main.h>
#include <wedge3/systems.h>

#include "transition.h"

class Battle_Transition_In2_Step : public Transition_Step
{
public:
	Battle_Transition_In2_Step(wedge::Task *task);
	virtual ~Battle_Transition_In2_Step();
	
	bool run();
};

#endif // BATTLE_TRANSITION_IN2_H
