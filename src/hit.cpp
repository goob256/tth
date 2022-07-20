#include <wedge3/battle_enemy.h>
#include <wedge3/battle_entity.h>
#include <wedge3/battle_player.h>
#include <wedge3/globals.h>

#include "globals.h"
#include "hit.h"

static void callback(void *data)
{
	auto h = static_cast<Hit_Step *>(data);
	h->set_done();
}

Hit_Step::Hit_Step(wedge::Battle_Entity *entity, int delay, wedge::Task *task) :
	wedge::Step(task),
	entity(entity),
	sprite(nullptr),
	done(false)
{
	// don't put this in start
	start_time = GET_TICKS() + delay;
	sprite = new gfx::Sprite("hit");
	
	gfx::Sprite *es = entity->get_sprite();
	if (es) {
		gfx::Image *img = es->get_current_image();
		util::Point<int> topleft, bottomright;
		img->get_bounds(topleft, bottomright);
		util::Size<int> sz(bottomright.x-topleft.x, bottomright.y-topleft.y);

		if (entity->get_type() == wedge::Battle_Entity::ENEMY) {
			auto e = dynamic_cast<wedge::Battle_Enemy *>(entity);
			draw_pos = e->get_position();
		}
		else {
			auto e = dynamic_cast<wedge::Battle_Player *>(entity);
			draw_pos = e->get_draw_pos();
		}
			
		draw_pos += topleft;
		draw_pos += sz/2;
		if (entity->get_inverse_x()) {
			draw_pos.x = shim::screen_size.w - draw_pos.x;
		}
		draw_pos -= sprite->get_current_image()->size/2;
		draw_pos += util::Point<int>(0, 2); // adjust for the sprite a bit
	}
	else {
		const int EDGE_X = shim::screen_size.w * 0.05f;
		const int EDGE_Y = shim::screen_size.h * 0.05f;

		int w = (shim::screen_size.w - EDGE_X*2) / 2;
		int h = shim::font->get_height() * 3 + 6; // 6 = border
		int y = shim::screen_size.h-EDGE_Y-h;
		int o = 4 + ((h - 6) / 3) / 2 - shim::font->get_height() / 2;
		util::Size<int> sz = sprite->get_current_image()->size;
		draw_pos = util::Point<float>(EDGE_X+w+WIN_BORDER-sz.w/2, y+o-(16-shim::font->get_height())/2+2-sz.h/2);
	}
}

Hit_Step::~Hit_Step()
{
	delete sprite;
}

void Hit_Step::start()
{
	gfx::Sprite *es = entity->get_sprite();
	if (es == nullptr) {
		TTH_GLOBALS->hit_sfx->play(false);
	}
	sprite->set_animation("hit", callback, this);
}

bool Hit_Step::run()
{
	if (done) {
		send_done_signal();
	}
	return !done;
}

void Hit_Step::draw()
{
	if (done) {
		return;
	}

	int flags;
	if (dynamic_cast<wedge::Battle_Player *>(entity)) {
		flags = gfx::Image::FLIP_H;
		// FIXME
		flags = 0;
	}
	else {
		flags = 0;
	}

	sprite->get_current_image()->draw(draw_pos, flags);
}

void Hit_Step::set_done()
{
	done = true;
}
