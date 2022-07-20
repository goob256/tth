// FF4 GBA style battle transition

#include "wedge3/area.h"
#include "wedge3/area_game.h"
#include "wedge3/general.h"
#include "wedge3/globals.h"
#include "wedge3/map_entity.h"
#include "wedge3/omnipresent.h"

#include "general.h"
#include "globals.h"
#include "transition.h"

Transition_Step::Transition_Step(bool out, wedge::Task *task) :
	wedge::Step(task),
	out(out),
	count(0)
{
	length = LENGTH;
}

Transition_Step::~Transition_Step()
{
}

bool Transition_Step::run()
{
	Uint32 now = SDL_GetTicks();
	Uint32 elapsed = now - start_time;

	bool ret = elapsed < (Uint32)length;

	if (out && ret == false) {
		SDL_Colour colour = shim::black;
		gfx::clear(shim::black);
		gfx::draw_filled_rectangle(colour, {0, 0}, shim::screen_size);
		OMNIPRESENT->draw_fore();
		gfx::flip();
		SDL_Delay(LENGTH/3);
	}

	return ret;
}

void Transition_Step::draw_back()
{
}

void Transition_Step::draw_fore()
{
	// Every even, hook, every odd, draw (causes draws to happen in Omnipresent::draw_fore)
	int mod = count % 2;
	count++;
	if (mod == 0) {
		OMNIPRESENT->hook_draw_last(this);
		return;
	}

	Uint32 now = SDL_GetTicks();
	Uint32 elapsed = now - start_time;
	float p = MIN(1.0f, elapsed / (float)length);

	if (out == false) {
		p = 1.0f - p;
	}

	// slow fade
	p = p * p;

	if (out && BATTLE == nullptr) {
		wedge::Map_Entity *player = AREA->get_player(0);
		util::Point<int> pos = player->get_position();
		util::Point<float> offset = player->get_offset();
		util::Size<int> player_size = player->get_size();
		util::Point<float> sz(player_size.w / 2.0f, 1.0f - player_size.h / 2.0f);
		wedge::add_tiles(pos, offset, sz);
		util::Point<float> map_offset = AREA->get_current_area()->get_centred_offset(pos, offset, true);

		util::Point<float> centre = map_offset + pos * shim::tile_size + (shim::tile_size/2);

		gfx::Image *mug = TTH_GLOBALS->mug->get_current_image();

		const float max_scale = 20.0f;
		float scale = p * max_scale;

		SDL_Colour tint = shim::black;

		mug->stretch_region_tinted(tint, {0.0f, 0.0f}, static_cast< util::Size<float> >(mug->size), static_cast< util::Point<float> >(centre-scale*mug->size.w/2), util::Size<float>(scale*mug->size.w, scale*mug->size.h), 0);
	}
	else {
		SDL_Colour colour = shim::black;
		gfx::draw_filled_rectangle(make_translucent(colour, p), {0, 0}, shim::screen_size);
	}
}

void Transition_Step::start()
{
	start_time = SDL_GetTicks();
}

#ifdef _WIN32
void *Transition_Step::operator new(size_t i)
{
	return _mm_malloc(i,16);
}

void Transition_Step::operator delete(void* p)
{
	_mm_free(p);
}
#endif
