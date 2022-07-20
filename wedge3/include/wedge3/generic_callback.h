#ifndef WEDGE3_GENERIC_CALLBACK_H
#define WEDGE3_GENERIC_CALLBACK_H

#include "wedge3/main.h"
#include "wedge3/systems.h"

namespace wedge {

class WEDGE3_EXPORT Generic_Callback_Step : public Step
{
public:
	Generic_Callback_Step(util::Callback callback, void *callback_data, Task *task);
	virtual ~Generic_Callback_Step();
	
	bool run();
	void done_signal(Step *step);

private:
	util::Callback callback;
	void *callback_data;
	bool done;
};

}

#endif // WEDGE3_GENERIC_CALLBACK_H
