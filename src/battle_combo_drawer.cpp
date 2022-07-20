#include "wedge3/globals.h"
#include "wedge3/omnipresent.h"

#include "battle_game.h"
#include "battle_combo_drawer.h"
#include "combo.h"
#include "general.h"
#include "globals.h"
#include "milestones.h"

Battle_Combo_Drawer_Step::Battle_Combo_Drawer_Step(wedge::Task *task) :
	wedge::Step(task),
	end_draw(0),
	count(0),
	last_caption(-1),
	ended(false)
{
	next_caption();
}

Battle_Combo_Drawer_Step::~Battle_Combo_Drawer_Step()
{
}

void Battle_Combo_Drawer_Step::draw_fore()
{
	// this Step hooks OMNIPRESENT::draw_fore
	int mod = count % 2;
	count++;
	if (mod == 0) {
		OMNIPRESENT->hook_draw_last(this);
		return;
	}

	// draw cursors first
	if (cursors.size() > 0) {
		for (auto p : cursors) {
			TTH_GLOBALS->cursor->get_current_image()->draw(p.first, p.second);
		}
		cursors.clear();
	}
	if (multi_checkboxes.size() > 0) {
		for (auto m : multi_checkboxes) {
			m.second->get_current_image()->draw(m.first);
		}
		multi_checkboxes.clear();
	}

	if (GET_TICKS() >= end_draw) {
		if (ended == false) {
			ended = true;
			next_caption();
		}
		return;
	}
	
	gfx::Font::end_batches();

	const int EDGE_X = shim::screen_size.w * 0.05f;
	int num_lines = 3;
	int w = (shim::screen_size.w - EDGE_X*2);
	int h = shim::font->get_height() * num_lines + 3*2 + 3*2;
	int x = (shim::screen_size.w-w)/2;
	int y = (shim::screen_size.h-h)*5/6;

	SDL_Colour win_colour = shim::white;//shim::palette[38];
	win_colour.r *= 0.9f;
	win_colour.g *= 0.9f;
	win_colour.b *= 0.9f;
	win_colour.a *= 0.9f;
	//gfx::draw_filled_rectangle(win_colour, util::Point<float>(x, y), util::Size<float>(w, h));
	gfx::draw_9patch_tinted(win_colour, TTH_GLOBALS->gui_window, {float(x), float(y)}, {w, h});

	y += 3;

	SDL_Colour shadow = shim::black;
	shadow.r *= 0.25f;
	shadow.g *= 0.25f;
	shadow.b *= 0.25f;
	shadow.a *= 0.25f;
	
	TTH_GLOBALS->bold_font->enable_shadow(shadow, gfx::Font::DROP_SHADOW);
	shim::font->enable_shadow(shadow, gfx::Font::DROP_SHADOW);

	SDL_Colour col = selected_colour(shim::palette[34], shim::white);
	TTH_GLOBALS->bold_font->draw(col, caption, util::Point<int>(shim::screen_size.w/2-TTH_GLOBALS->bold_font->get_text_width(caption)/2, y));

	y += shim::font->get_height() + 3;

	TTH_GLOBALS->bold_font->disable_shadow();
	shim::font->disable_shadow();
}

void Battle_Combo_Drawer_Step::set(Combo c)
{
	this->c = c;
	end_draw = GET_TICKS() + 250;
	ended = false;
}

void Battle_Combo_Drawer_Step::set(std::string combo_name)
{
	if (get_combo_multi(combo_name) == true) {
		end_draw = 0;
	}
	else {
		this->combo_name = combo_name;
		end_name = GET_TICKS() + 250;
		end_draw = GET_TICKS() + 250;
		ended = false;
	}
}

void Battle_Combo_Drawer_Step::end_now()
{
	end_draw = 0;
}

void Battle_Combo_Drawer_Step::set_cursors(std::vector< std::pair<util::Point<float>, int> > cursors)
{
	this->cursors = cursors;
}

void Battle_Combo_Drawer_Step::set_multi_checkboxes(std::vector< std::pair<util::Point<float>, gfx::Sprite *> > multi_checkboxes)
{
	this->multi_checkboxes = multi_checkboxes;
}

void Battle_Combo_Drawer_Step::next_caption()
{
	int r;
	do {
		r = util::rand(0, 4);
		r += 186; // caption #1
	} while (r == last_caption);
	last_caption = r;
	caption = GLOBALS->game_t->translate(r);
}
