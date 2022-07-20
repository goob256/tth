#ifndef WEDGE3_PLAY_MUSIC_H
#define WEDGE3_PLAY_MUSIC_H

#include "wedge3/main.h"
#include "wedge3/systems.h"

namespace wedge {

class WEDGE3_EXPORT Play_Music_Step : public Step
{
public:
	Play_Music_Step(std::string name, Task *task);
	virtual ~Play_Music_Step();

	void start();
	bool run();

private:
	std::string name;
};

}

#endif // WEDGE3_PLAY_MUSIC_H
