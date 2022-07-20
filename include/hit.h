#ifndef HIT_H
#define HIT_H

#include <wedge3/main.h>
#include <wedge3/battle_entity.h>
#include <wedge3/systems.h>

class Hit_Step : public wedge::Step
{
public:
	Hit_Step(wedge::Battle_Entity *entity, int delay, wedge::Task *task);
	virtual ~Hit_Step();

	void start();
	bool run();
	void draw();

	void set_done();

private:
	Uint32 start_time;
	wedge::Battle_Entity *entity;
	gfx::Sprite *sprite;
	bool done;
	util::Point<float> draw_pos;
};

#endif // HIT_H
