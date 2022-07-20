#ifndef WEDGE3_SCREEN_SHAKE_H
#define WEDGE3_SCREEN_SHAKE_H

#include "wedge3/main.h"
#include "wedge3/systems.h"

namespace wedge {

class WEDGE3_EXPORT Screen_Shake_Step : public Step
{
public:
	Screen_Shake_Step(float amount, Uint32 length, Task *task);
	virtual ~Screen_Shake_Step();
	
	bool run();

	void set_cancelled(bool cancelled);

private:
	float amount;
	Uint32 length;
	bool cancelled;
};

}

#endif // WEDGE3_SCREEN_SHAKE_H
