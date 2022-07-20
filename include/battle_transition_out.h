#ifndef BATTLE_TRANSITION_OUT_H
#define BATTLE_TRANSITION_OUT_H

#include <wedge3/main.h>
#include <wedge3/systems.h>

#include "transition.h"

class Battle_Transition_Out_Step : public Transition_Step
{
public:
	Battle_Transition_Out_Step(wedge::Task *task);
	virtual ~Battle_Transition_Out_Step();
	
	bool run();
};

#endif // BATTLE_TRANSITION_OUT_H
