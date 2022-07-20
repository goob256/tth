#ifndef WEDGE3_FADE_H
#define WEDGE3_FADE_H

#include "wedge3/main.h"
#include "wedge3/systems.h"

namespace wedge {

class WEDGE3_EXPORT Fade_Step : public Step
{
public:
	Fade_Step(SDL_Colour colour, bool out, int length/*ms*/, Task *task); // in or out
	virtual ~Fade_Step();
	
	bool run();
	void start();

protected:
	SDL_Colour colour;
	bool out;
	int length;
	Uint32 start_time;
};

}

#endif // WEDGE3_FADE_H
