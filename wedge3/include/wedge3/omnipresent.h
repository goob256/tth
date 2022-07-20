#ifndef WEDGE3_OMNIPRESENT_GAME_H
#define WEDGE3_OMNIPRESENT_GAME_H

#include "wedge3/main.h"
#include "wedge3/systems.h"

namespace wedge {

class WEDGE3_EXPORT Omnipresent_Game : public Game
{
public:
	Omnipresent_Game();
	virtual ~Omnipresent_Game();
	
	virtual void draw_fore();
	virtual bool run();
	
	void draw_last();
	void draw_controls();

	void start_fade(SDL_Colour colour, int delay, int duration);
	void end_fade();

	void hook_draw_fore(Step *step);
	void hook_draw_last(Step *step);

	void set_hide_red_triangle(bool hide);

	void set_quit_game(bool quit);

	void set_triangle_pressed(bool pressed);
	void set_x_pressed(bool pressed);

private:
	bool buttons_enabled();

	bool fading;
	Uint32 fade_start;
	int fade_delay;
	int fade_duration;
	SDL_Colour fade_colour;
	std::vector<Step *> draw_fore_hooks;
	std::vector<Step *> draw_last_hooks;
	bool hide_red_triangle;
	bool quit_game;
	bool triangle_pressed;
	bool x_pressed;
};

}

#endif // WEDGE3_OMNIPRESENT_GAME_H
