#ifndef WEDGE3_GENERIC_IMMEDIATE_CALLBACK_H
#define WEDGE3_GENERIC_IMMEDIATE_CALLBACK_H

#include "wedge3/main.h"
#include "wedge3/systems.h"

namespace wedge {

class WEDGE3_EXPORT Generic_Immediate_Callback_Step : public Step
{
public:
	Generic_Immediate_Callback_Step(util::Callback callback, void *callback_data, Task *task);
	virtual ~Generic_Immediate_Callback_Step();

	void start();
	bool run();

private:
	util::Callback callback;
	void *callback_data;
};

}

#endif // WEDGE3_GENERIC_IMMEDIATE_CALLBACK_H
