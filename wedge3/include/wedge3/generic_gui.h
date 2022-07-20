#ifndef WEDGE3_GENERIC_GUI_H
#define WEDGE3_GENERIC_GUI_H

#include "wedge3/main.h"
#include "wedge3/systems.h"

namespace wedge {

class WEDGE3_EXPORT Generic_GUI_Step : public Step
{
public:
	Generic_GUI_Step(gui::GUI *gui, bool resize, Task *task);
	virtual ~Generic_GUI_Step();

	void start();
	bool run();

	void set_done(bool done);

private:
	bool done;
	gui::GUI *gui;
	bool resize;
};

}

#endif // WEDGE3_GENERIC_GUI_H
