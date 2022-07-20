#include "wedge3/battle_enemy.h"
#include "wedge3/battle_game.h"
#include "wedge3/battle_player.h"
#include "wedge3/globals.h"
#include "wedge3/stats.h"

using namespace wedge;

namespace wedge {

Battle_Enemy::Battle_Enemy(std::string name) :
	Battle_Entity(ENEMY, name),
	die_time(1000),
	experience(0),
	attack_sound(NULL),
	die_sound(NULL),
	started_attack(false),
	done_sprite(false),
	attack_hits_all(false),
	boss(false),
	use_death_shader(true),
	_use_death_sound(true),
	_remove_when_dead(true),
	play_attack_sound(true),
	click_zone_inc(0),
	floating(false)
{
}

Battle_Enemy::~Battle_Enemy()
{
	delete stats;
	delete sprite;
	delete attack_sound;
	delete die_sound;
}

bool Battle_Enemy::start()
{
	return true;
}

void Battle_Enemy::draw_enemy(SDL_Colour tint, gfx::Image *image, util::Point<float> draw_pos, float scale, int flags)
{
	auto active_turns = BATTLE->get_active_turns();

	bool acting = false;

	for (size_t i = 0; i < active_turns.size(); i++) {
		if (active_turns[i]->actor == this) {
			acting = true;
			break;
		}
	}

	if (acting) {
		draw_pos.y += 5;
	}

	bool dead = stats->hp <= 0;
	if (floating && !acting) {
		Uint32 t = GET_TICKS() % 1000;
		float p = t / 1000.0f;
		if (p < 0.5f) {
			p /= 0.5f;
			p = p * p;
		}
		else {
			p -= 0.5f;
			p /= 0.5f;
			p = p * p;
			p = 1.0f - p;
		}
		draw_pos.y -= p * 5.0f;
	}
	if (dead) {
		float p = (GET_TICKS()-dead_start)/(float)die_time;
		std::vector<int> to_draw;
		for (int y = 0; y < image->size.h; y++) {
			to_draw.push_back(y);
		}
		std::vector<float> times;
		std::vector<int> start_i;
		std::vector<int> max_i;
		int i;
		times.push_back(0.0f);
		times.push_back(0.333f);
		times.push_back(0.666f);
		start_i.push_back(0);
		start_i.push_back(0);
		start_i.push_back(0);
		max_i.push_back(4);
		max_i.push_back(2);
		max_i.push_back(0);
		float a = 1.0f - p;
		SDL_Colour c = tint;
		c.r *= a;
		c.g *= a;
		c.b *= a;
		c.a *= a;
		for (size_t t = 0; t < times.size(); t++) {
			float p2 = (p-times[t]) / 0.333f;
			if (p2 > 1.0f) {
				p2 = 1.0f;
			}
			int i = start_i[t];
			for (int y = 0; y < image->size.h; y++) {
				float p3 = y / (float)image->size.h;
				if (i == 0 && p3 < p2) {
					auto it = std::find(to_draw.begin(), to_draw.end(), y);
					if (it != to_draw.end()) {
						to_draw.erase(it);
					}
				}
				i++;
				if (i >= max_i[t]) {
					i = start_i[t];
				}
			}
		}
		image->start_batch();
		for (size_t i = 0; i < to_draw.size(); i++) {
			int y = to_draw[i];
			image->draw_region_tinted(c, {0.0f, (float)y}, {image->size.w, 1}, draw_pos+util::Point<float>(0.0f, y));
		}
		image->end_batch();
	}
	else {
		image->draw_tinted(tint, draw_pos, flags);
	}
}

void Battle_Enemy::draw_boss(SDL_Colour tint, gfx::Image *image, util::Point<float> draw_pos, float scale, int curr_frame, int num_frames, int flags)
{
	bool dead = stats->hp <= 0;
	image->draw_tinted(tint, draw_pos, flags);
}

void Battle_Enemy::draw()
{
	if (sprite) {
		gfx::Image *image = sprite->get_current_image();
		int flags;
		if (inverse_x) {
			flags = gfx::Image::FLIP_H;
		}
		else {
			flags = 0;
		}
		util::Point<int> pos;
		if (draw_as_player) {
			pos = get_sprite_pos();
			if (inverse_x) {
				pos.x = shim::screen_size.w - pos.x - image->size.w;
			}
		}
		else {
			if (inverse_x) {
				pos = util::Point<int>(shim::screen_size.w-position.x-image->size.w, position.y);
			}
			else {
				pos = position;
			}
		}
		if (global_flip_h) {
			if (flags == 0) {
				flags = gfx::Image::FLIP_H;
			}
			else {
				flags = 0;
			}
		}
		if (boss) {
			draw_boss(global_tint, image, pos, 1.0f, sprite->get_current_frame(), sprite->get_num_frames(), flags);
		}
		else {
			draw_enemy(global_tint, image, pos, 1.0f, flags);
		}
	}
}

bool Battle_Enemy::is_dead()
{
	return stats->hp <= 0 && int(GET_TICKS()-dead_start) >= die_time;
}

int Battle_Enemy::get_experience()
{
	return experience;
}

int Battle_Enemy::get_gold()
{
	return gold;
}

void Battle_Enemy::sprite_done()
{
	done_sprite = true;
}

void Battle_Enemy::set_position(util::Point<int> position)
{
	this->position = position;
}

util::Point<int> Battle_Enemy::get_position()
{
	return position;
}

void Battle_Enemy::play_die_sound()
{
	if (die_sound != NULL) {
		die_sound->play(false);
	}
	else if (_use_death_sound) {
		globals->enemy_die->play(false);
	}
}

Battle_Player *Battle_Enemy::rand_living_player()
{
	std::vector<Battle_Entity *> players = BATTLE->get_players();
	std::vector<Battle_Entity *> living;
	for (size_t i = 0; i < players.size(); i++) {
		if (players[i]->get_stats()->hp > 0) {
			living.push_back(players[i]);
		}
	}
	if (living.size() == 0) {
		return NULL;
	}
	return static_cast<Battle_Player *>(living[util::rand()%living.size()]);
}

void Battle_Enemy::set_dead()
{
	dead_start = GET_TICKS();
}

bool Battle_Enemy::remove_when_dead()
{
	return _remove_when_dead;
}

bool Battle_Enemy::use_death_sound()
{
	return _use_death_sound;
}

bool Battle_Enemy::take_hit(Battle_Entity *actor, int damage)
{
	bool dead = Battle_Entity::take_hit(actor, damage);
	if (dead) {
		play_die_sound();
		set_dead();
	}
	return dead;
}

int Battle_Enemy::get_click_zone_inc()
{
	return click_zone_inc;
}

util::Point<float> Battle_Enemy::get_sprite_pos()
{
       gfx::Image *current_image = sprite->get_current_image();
       util::Size<int> offset = util::Size<int>((shim::tile_size-current_image->size.w)/2, shim::tile_size-current_image->size.h);
       return position + util::Point<int>(offset.w, offset.h);
}

}
